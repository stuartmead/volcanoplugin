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

#include < Qimage >

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"
#include "subsetdata.h"


namespace RF
{
    /**
     * \internal
     */
    class SubsetDataImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::SubsetDataImpl)

    public:
        SubsetData&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDatabase_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterNumber_;
        CSIRO::DataExecution::TypedObject< int >           dataXOffset_;
        CSIRO::DataExecution::TypedObject< int >           dataYOffset_;
        CSIRO::DataExecution::TypedObject< int >           dataXLength_;
        CSIRO::DataExecution::TypedObject< int >           dataYLength_;
        CSIRO::DataExecution::TypedObject< double >        dataScaleFactor_;
		CSIRO::DataExecution::TypedObject< bool >		   dataWriteOut_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDatabase_;
        CSIRO::DataExecution::InputScalar inputRasterNumber_;
        CSIRO::DataExecution::InputScalar inputXOffset_;
        CSIRO::DataExecution::InputScalar inputYOffset_;
        CSIRO::DataExecution::InputScalar inputXLength_;
        CSIRO::DataExecution::InputScalar inputYLength_;
        CSIRO::DataExecution::InputScalar inputScaleFactor_;
		CSIRO::DataExecution::InputScalar inputWriteOut_; 
        CSIRO::DataExecution::Output      outputOutputRaster_;
        CSIRO::DataExecution::InputScalar inputOutputRasterName_;


        SubsetDataImpl(SubsetData& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    SubsetDataImpl::SubsetDataImpl(SubsetData& op) :
        op_(op),
        dataGDALDatabase_(),
        dataRasterNumber_(1),
        dataXOffset_(),
        dataYOffset_(),
        dataXLength_(),
        dataYLength_(),
        dataScaleFactor_(1.0),
		dataWriteOut_(false),
        dataOutputRaster_(),
        dataOutputRasterName_(),
        inputGDALDatabase_("GDAL Database", dataGDALDatabase_, op_),
        inputRasterNumber_("Raster Number", dataRasterNumber_, op_),
        inputXOffset_("X offset", dataXOffset_, op_),
        inputYOffset_("Y offset", dataYOffset_, op_),
        inputXLength_("X length", dataXLength_, op_),
        inputYLength_("Y length", dataYLength_, op_),
        inputScaleFactor_("Scale factor", dataScaleFactor_, op_),
		inputWriteOut_("Write as .tiff file", dataWriteOut_, op_),
        outputOutputRaster_("Output raster", dataOutputRaster_, op_),
        inputOutputRasterName_("Output raster name", dataOutputRasterName_, op_)
    {
    }


    /**
     *
     */
    bool SubsetDataImpl::execute()
    {
        GDALDatasetH& gDALDatabase     = *dataGDALDatabase_;
        int&          rasterNumber     = *dataRasterNumber_;
        int&          xOffset          = *dataXOffset_;
        int&          yOffset          = *dataYOffset_;
        int&          xLength          = *dataXLength_;
        int&          yLength          = *dataYLength_;
        double&       scaleFactor      = *dataScaleFactor_;
        GDALDatasetH& outputRaster     = *dataOutputRaster_;
        QString&      outputRasterName = *dataOutputRasterName_;

        
        if(rasterNumber > GDALGetRasterCount(gDALDatabase))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDatabase)).arg(rasterNumber) + "\n";
            return false;
        }

        GDALRasterBandH hBand = GDALGetRasterBand(gDALDatabase, rasterNumber);

        double sizes[2];
        if (xLength <= 0)
        {
            sizes[0] = GDALGetRasterBandXSize(hBand) - xOffset;
        }
        else
        {
            sizes[0] = xLength;
        }

        if (yLength <= 0)
        {
            sizes[1] = GDALGetRasterBandYSize(hBand) - yOffset;
        }
        else
        {
            sizes[1] = yLength;
        }

        int scaleXsize, scaleYsize;
        scaleXsize = floor(sizes[0]/scaleFactor);
        scaleYsize = floor(sizes[1]/scaleFactor);

        double transform[6];
        GDALGetGeoTransform(gDALDatabase,transform);
		
		/*
		//Geotransform to convert from cellspace (P,L) to geographical space
		//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2]; 
		//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
		In a north up image, padfTransform[1] is the pixel width, and 
		padfTransform[5] is the pixel height. 
		The upper left corner of the upper left pixel is at 
		position (padfTransform[0],padfTransform[3]).
		*/

        std::cout << QString("Downscaled X cellsize is %1, Y cellsize is %2").arg((sizes[0]/scaleXsize)*transform[1]).arg((sizes[1]/scaleYsize)*-transform[5]) + "\n";

		//Change the transform
		transform[0] = transform[0] + xOffset*transform[1]; //X origin is original + xOffset times (orig) cellsize
		transform[3] = transform[3] + yOffset*transform[5]; //Similar for Y
		transform[1] = (sizes[0] / scaleXsize)*transform[1]; //Cellsize x
		transform[5] = (sizes[1] / scaleYsize)*transform[5]; //Cellsize y


        float *data;
        data = new float [scaleYsize*scaleXsize];

        std::cout << QString("Raster type is %1").arg(GDALGetDataTypeName(GDALGetRasterDataType(hBand))) + "\n";
		


        GDALRasterIO(hBand, GF_Read,
            xOffset, yOffset, //X,Y offset in cells
            sizes[0], sizes[1], //X,Y length in cells
            data, //data
            scaleXsize, scaleYsize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);//Scanline stuff (for interleaving)

		      

        //Now write data to new raster
        outputRaster = GDALCreate( GDALGetDatasetDriver(gDALDatabase),
                                    outputRasterName.toLocal8Bit().constData(),
                                    scaleXsize, scaleYsize,
                                    1,
                                    GDT_Float32, NULL);

        int srcNoData;
        double dstNodataValue;

        dstNodataValue = GDALGetRasterNoDataValue(hBand, &srcNoData);
        GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
		GDALSetGeoTransform(outputRaster, transform);
        GDALSetProjection(outputRaster, GDALGetProjectionRef(gDALDatabase));
        GDALSetRasterNoDataValue(destBand, dstNodataValue);

        GDALRasterIO( destBand, GF_Write,
                        0,0,
                        scaleXsize, scaleYsize,
                        data,
                        scaleXsize, scaleYsize,
                        GDT_Float32,
                        0,0);
        
		if (*dataWriteOut_ == true)
		{
			std::cout << QString("Writing out tiff grid file.") + "\n";
			const char *pszFormat = "GTiff";
			GDALDatasetH ascOut = GDALCreate(GDALGetDriverByName(pszFormat),
				outputRasterName.append(".tiff").toLocal8Bit().constData(),
				scaleXsize, scaleYsize,
				1,
				GDT_Float32, NULL);
			GDALRasterBandH ascBand = GDALGetRasterBand(ascOut, 1);


			GDALSetGeoTransform(ascOut, transform);
			GDALSetProjection(ascOut, GDALGetProjectionRef(gDALDatabase));
			GDALSetRasterNoDataValue(destBand, dstNodataValue);

			GDALRasterIO(ascBand, GF_Write,
				0, 0,
				scaleXsize, scaleYsize,
				data,
				scaleXsize, scaleYsize,
				GDT_Float32,
				0, 0);
			GDALClose(ascOut);
		}


        return true;
    }


    /**
     *
     */
    SubsetData::SubsetData() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< SubsetData >::getInstance(),
            tr("Extract Subset of GDALDatabase"))
    {
        pImpl_ = new SubsetDataImpl(*this);
    }


    /**
     *
     */
    SubsetData::~SubsetData()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  SubsetData::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(SubsetData, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

