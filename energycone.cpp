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

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "ogr_spatialref.h"

#include "volcanoplugin.h"
#include "energycone.h"


namespace RF
{
    /**
     * \internal
     */
    class EnergyConeImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EnergyConeImpl)

    public:
        EnergyCone&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataElevationDataset_;
        CSIRO::DataExecution::TypedObject< double >        dataMultiplier_;
        CSIRO::DataExecution::TypedObject< double >        dataAddHeight_;
        CSIRO::DataExecution::TypedObject< double >        dataXLocation_;
        CSIRO::DataExecution::TypedObject< double >        dataYLocation_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputElevationDataset_;
        CSIRO::DataExecution::InputScalar inputMultiplier_;
        CSIRO::DataExecution::InputScalar inputAddHeight_;
        CSIRO::DataExecution::InputScalar inputXLocation_;
        CSIRO::DataExecution::InputScalar inputYLocation_;
        CSIRO::DataExecution::InputScalar inputOutputRasterName_;
        CSIRO::DataExecution::Output      outputOutputRaster_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;


        EnergyConeImpl(EnergyCone& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    EnergyConeImpl::EnergyConeImpl(EnergyCone& op) :
        op_(op),
        dataElevationDataset_(),
        dataMultiplier_(1.0),
        dataAddHeight_(0.0),
        dataXLocation_(-1.0),
        dataYLocation_(-1.0),
        dataOutputRasterName_(),
        dataOutputRaster_(),
        dataRasterBand_(1),
        inputElevationDataset_("Elevation Dataset", dataElevationDataset_, op_),
        inputMultiplier_("Multiplier", dataMultiplier_, op_),
        inputAddHeight_("Additional height", dataAddHeight_, op_),
        inputXLocation_("X location", dataXLocation_, op_),
        inputYLocation_("Y location", dataYLocation_, op_),
        inputOutputRasterName_("Output Raster name", dataOutputRasterName_, op_),
        outputOutputRaster_("Output Raster", dataOutputRaster_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_)
    {
    }


    /**
     *
     */

    float ecludianDistance(float X, float Y, float Z, float x, float y, float z)
    {
        return sqrt(pow(fabs(X-x),2)+pow(fabs(Y-y),2)+pow(fabs(Z-z),2));
    }

    bool EnergyConeImpl::execute()
    {
        GDALDatasetH& elevationDataset = *dataElevationDataset_;
        double&       multiplier       = *dataMultiplier_;
        double&       xLocation    = *dataXLocation_;
        double&       yLocation    = *dataYLocation_;
        QString&      outputRasterName = *dataOutputRasterName_;
        GDALDatasetH& outputRaster     = *dataOutputRaster_;
        int&          rasterBand       = *dataRasterBand_;
        
        GDALAllRegister();

        GDALRasterBandH hBand;

        if (rasterBand > GDALGetRasterCount(elevationDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(elevationDataset)).arg(rasterBand) + "\n";
        }

        hBand = GDALGetRasterBand(elevationDataset, rasterBand);

        double transform[6], invTransform[6];
        GDALGetGeoTransform(elevationDataset, transform);
        GDALInvGeoTransform(transform, invTransform);
        
        int srcNodata;
        float dstNodataValue;

        dstNodataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNodata);
        std::cout << QString("Input nodata value for energy cone is %1").arg(dstNodataValue) + "\n";

        float * elevation;
        elevation = new float [GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

        GDALRasterIO(hBand, GF_Read,
                        0,0,
                        GDALGetRasterBandXSize(hBand),GDALGetRasterBandYSize(hBand),
                        elevation,
                        GDALGetRasterBandXSize(hBand),GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);


        float * energyCone;
        energyCone = new float [GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

        float maxElev = elevation[0];//first value
        int cells[2];

        //Find max cell
        for (int i = 1; i < GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand); ++i)
        {
            if (elevation[i] > maxElev && elevation[i] != dstNodataValue)
            {
                maxElev = elevation[i];
                cells[1] = floorf(i/GDALGetRasterBandXSize(hBand));
                cells[0] = i - (cells[1]*GDALGetRasterBandXSize(hBand));
            }
        }

        std::cout << QString("Max elevation is %1 at cell %2, %3").arg(maxElev).arg(cells[0]).arg(cells[1]) + "\n";


        if (xLocation >= 0.0 && yLocation >= 0.0)
        {
            cells[0] = (int) xLocation;
            cells[1] = (int) yLocation;
            maxElev = elevation[(cells[1]*GDALGetRasterBandXSize(hBand))+cells[0]];
            std::cout << QString("Elevation is %1 at cell %2, %3").arg(maxElev).arg(cells[0]).arg(cells[1]) + "\n";
        }
       
        maxElev = maxElev + *dataAddHeight_;

        for (int i = 0; i < GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand); ++i)
        {
            int y = floorf(i/GDALGetRasterBandXSize(hBand));
            int x = i - (y*GDALGetRasterBandXSize(hBand));
            if ((cells[0]-x)+(cells[1]-y)+(maxElev-elevation[i]) <= 0)
            {
                energyCone[i] = 0.0;//dstNodataValue;
            }
            else
            {
                 energyCone[i] = std::max((float) 0.0, (float) (maxElev - (ecludianDistance(cells[0]*transform[1],-cells[1]*transform[5],maxElev,x*transform[1],-y*transform[5],elevation[i])*multiplier)) - elevation[i]);
            }
            /*
            if (i % 100000 == 0)
            {
                std::cout << QString("Ecludian distance is %1").arg( energyCone[i]) + "\n";
            }
            */
            if (energyCone[i] < 0)
                std::cout << QString("Negative energy cone value, elevation is %1, energy cone is %2").arg(elevation[i]).arg(energyCone[i]) + "\n";
        }

        outputRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
                                    outputRasterName.toLocal8Bit().constData(),
                                    GDALGetRasterXSize(elevationDataset),
                                    GDALGetRasterYSize(elevationDataset),
                                    1,
                                    GDT_Float32,
                                    NULL);

        GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
        GDALSetGeoTransform(outputRaster, transform);

        
        if(GDALSetProjection(outputRaster, GDALGetProjectionRef(elevationDataset)) != CE_None)
        {
            std::cout << QString("WARNING: Output projection cannot be set, setting to WGS84") + "\n";
            OGRSpatialReferenceH hSRS;
            hSRS = OSRNewSpatialReference(NULL);
            OSRSetWellKnownGeogCS(hSRS, "WGS84");
            char *gsR = NULL;
            OSRExportToWkt(hSRS, &gsR);
            if(GDALSetProjection(outputRaster, gsR) != CE_None)
            {
                std::cout << QString("ERROR: Could not set projection to WGS84") + "\n";
                return false;
            }
        }
          
        GDALSetRasterNoDataValue(destBand, dstNodataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        energyCone,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);

        return true;
    }


    /**
     *
     */
    EnergyCone::EnergyCone() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< EnergyCone >::getInstance(),
            tr("Calculate Energy Cone"))
    {
        pImpl_ = new EnergyConeImpl(*this);
    }


    /**
     *
     */
    EnergyCone::~EnergyCone()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  EnergyCone::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(EnergyCone, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

