/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/


#include <cassert>
#include <iostream>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

//#include "Geospatial/boundingbox.h"

#include "gdal.h"
#include "gdal_alg.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_multiproc.h"

#include "volcanoplugin.h"
#include "boundsofraster.h"
#include "gdalinfo.h"


namespace RF
{
    /**
     * \internal
     */
    class GDALinfoImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::GDALinfoImpl)

    public:
        GDALinfo&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< QString >                         dataFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >                    dataRaster_;
        CSIRO::DataExecution::TypedObject< QString >                         dataDriver_;
        CSIRO::DataExecution::TypedObject< int >                             dataSizex_;
        CSIRO::DataExecution::TypedObject< int >                             dataSizey_;
        CSIRO::DataExecution::TypedObject< QString >                         dataProjection_;
        CSIRO::DataExecution::TypedObject< RF::BoundsofRaster >				 dataBboix_;
        CSIRO::DataExecution::TypedObject< double >                          dataCellSizeX_;
        CSIRO::DataExecution::TypedObject< double >                          dataCellSizeY_;
        CSIRO::DataExecution::TypedObject< int >                             dataOriginX_;
        CSIRO::DataExecution::TypedObject< int >                             dataOriginY_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >                    dataGDALdataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputFileName_;
        CSIRO::DataExecution::InputScalar inputRaster_;
        CSIRO::DataExecution::Output      outputDriver_;
        CSIRO::DataExecution::Output      outputSizex_;
        CSIRO::DataExecution::Output      outputSizey_;
        CSIRO::DataExecution::Output      outputProjection_;
        CSIRO::DataExecution::Output      outputBboix_;
        CSIRO::DataExecution::Output      outputCellSizeX_;
        CSIRO::DataExecution::Output      outputCellSizeY_;
        CSIRO::DataExecution::Output      outputOriginX_;
        CSIRO::DataExecution::Output      outputOriginY_;
        CSIRO::DataExecution::Output      outputGDALdataset_;

        GDALinfoImpl(GDALinfo& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    GDALinfoImpl::GDALinfoImpl(GDALinfo& op) :
        op_(op),
        dataFileName_(),
        dataRaster_(),
        dataDriver_(),
        dataSizex_(),
        dataSizey_(),
        dataProjection_(),
        dataBboix_(),
        dataCellSizeX_(),
        dataCellSizeY_(),
        dataOriginX_(),
        dataOriginY_(),
        dataGDALdataset_(),
        inputFileName_("File name", dataFileName_, op_),
        inputRaster_("Raster dataset", dataRaster_, op_),
        outputDriver_("Driver", dataDriver_, op_),
        outputSizex_("Cells in X", dataSizex_, op_),
        outputSizey_("Cells in Y", dataSizey_, op_),
        outputProjection_("Projection", dataProjection_, op_),
        outputBboix_("Bounding Box", dataBboix_, op_),
        outputCellSizeX_("Cell Size X", dataCellSizeX_, op_),
        outputCellSizeY_("Cell Size Y", dataCellSizeY_, op_),
        outputOriginX_("Origin X", dataOriginX_, op_),
        outputOriginY_("Origin Y", dataOriginY_, op_),
        outputGDALdataset_("GDAL Dataset Handle", dataGDALdataset_, op_)
    {
        inputFileName_.setDescription("Dataset to read from file");
        inputRaster_.setDescription("Raster for information. If file name is empty, this will be used to obtain the information");
    }


    /**
     *
     */
    bool GDALinfoImpl::execute()
    {
        QString&                        fileName   = *dataFileName_;
        QString&                        driver     = *dataDriver_;
        GDALDatasetH&                   iDataset    = *dataRaster_;
        int&                            sizex      = *dataSizex_;
        int&                            sizey      = *dataSizey_;
        QString&                        projection = *dataProjection_;
        RF::BoundsofRaster&				bbox      = *dataBboix_;
        double&                         cellSizeX  = *dataCellSizeX_;
        double&                         cellSizeY  = *dataCellSizeY_;
        int&                            originX    = *dataOriginX_;
        int&                            originY    = *dataOriginY_;
        
        //Register
        GDALAllRegister();

        if (!fileName.isEmpty())
        {
            //Open dataset.
            iDataset = GDALOpen(fileName.toLatin1(), GA_ReadOnly);
        }

        *dataGDALdataset_ = iDataset;
        //*dataGdalHandle_=iDataset;

        if (iDataset == NULL)
        {
            std::cout << QString("ERROR: unable to open %1").arg(fileName) + "\n";
        }

                

        //Driver and Size
        GDALDriverH driverG = GDALGetDatasetDriver(iDataset);
        driver = GDALGetDriverLongName(driverG);

        sizex = GDALGetRasterXSize(iDataset);
        sizey = GDALGetRasterYSize(iDataset);

        //Projection
        char *cprojection = (char *) GDALGetProjectionRef(iDataset);
        
                OGRSpatialReferenceH hSRS = OSRNewSpatialReference(NULL);

        if (OSRImportFromWkt(hSRS, &cprojection) == CE_None)
        {
            char        *pszPrettyWkt = NULL;

            OSRExportToPrettyWkt( hSRS, &pszPrettyWkt, FALSE );
            projection = pszPrettyWkt;
            CPLFree( pszPrettyWkt );
        }
        else
        {
            projection = cprojection;
        }

        char *crs = NULL;
        OSRExportToProj4(hSRS, &crs);
        bbox.setCoordinateSystem("EPSG:4167");
                
        std::cout << QString("Coordinate System is: \n %1").arg(projection) + "\n";

		std::cout << QString("Projection: %1").arg(OSRGetAuthorityCode(hSRS, NULL)) + "\n";//OSRGetAttrValue(hSRS, "PROJECTION", 0)
		

        //Origin, x and y
        double transform[6];
        GDALGetGeoTransform(iDataset, transform);
        /******Geo Transform is
        adfGeoTransform[0] /* top left x 
        adfGeoTransform[1] /* w-e pixel resolution 
        adfGeoTransform[2] /* 0 
        adfGeoTransform[3] /* top left y 
        adfGeoTransform[4] /* 0 
        adfGeoTransform[5] /* n-s pixel resolution (negative value)
        */
        originX = transform[0];
        originY = transform[3];
        cellSizeX = transform[1];
        cellSizeY = -transform[5];

        //Boundingbox

        if( cprojection != NULL && strlen(cprojection) > 0 )
        {
        OGRSpatialReferenceH hProj, hLatLong = NULL;

        hProj = OSRNewSpatialReference( cprojection );
        if( hProj != NULL )
            hLatLong = OSRCloneGeogCS( hProj );

        if( hLatLong != NULL )
        {
            CPLPushErrorHandler( CPLQuietErrorHandler );
            OGRCoordinateTransformationH hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
            CPLPopErrorHandler();
            
            OSRDestroySpatialReference( hLatLong );
        }

        if( hProj != NULL )
            OSRDestroySpatialReference( hProj );
        }


        //minx
        bbox.setWesternBound(transform[0]); //x is 0
        //miny
        bbox.setSouthernBound(transform[3]);
        //maxx
        bbox.setEasternBound(transform[0]+transform[1]*sizex);
        //maxy
        bbox.setNorthernBound(transform[3] + transform[5]*sizey);
        //Resolution
        bbox.setEWResolution(transform[1]);
        bbox.setNSResolution(-transform[5]);

        int rasterCount = GDALGetRasterCount(*dataGDALdataset_);
        std::cout << QString("Raster has %1 bands").arg(rasterCount) + "\n";
        
        
        int srcNoData;
        float srcNoDataValue;

        srcNoDataValue = (float) GDALGetRasterNoDataValue(GDALGetRasterBand(*dataGDALdataset_, 1), &srcNoData);

        std::cout << QString("Band nodata value is %1").arg(srcNoDataValue) + "\n";

        return true;
    }


    /**
     *
     */
    GDALinfo::GDALinfo() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< GDALinfo >::getInstance(),
            tr("GetGeoDataInfo"))
    {
        pImpl_ = new GDALinfoImpl(*this);
    }


    /**
     *
     */
    GDALinfo::~GDALinfo()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  GDALinfo::execute()
    {
        return pImpl_->execute();
    }
}

using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(GDALinfo, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

