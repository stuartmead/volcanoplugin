#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"

#include "volcanoplugin.h"
#include "multiplyrasters.h"


namespace RF
{
    /**
     * \internal
     */
    class MultiplyRastersImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::MultiplyRastersImpl)

    public:
        MultiplyRasters&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataRaster1_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataRaster2_;
        CSIRO::DataExecution::TypedObject< QString >  dataOutputRasterFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputRaster1_;
        CSIRO::DataExecution::InputScalar inputRaster2_;
        CSIRO::DataExecution::InputScalar inputOutputRasterFileName_;
        CSIRO::DataExecution::Output      outputOutputRaster_;


        MultiplyRastersImpl(MultiplyRasters& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    MultiplyRastersImpl::MultiplyRastersImpl(MultiplyRasters& op) :
        op_(op),
        dataRaster1_(),
        dataRaster2_(),
        dataOutputRasterFileName_(),
        dataOutputRaster_(),
        inputRaster1_("Raster 1", dataRaster1_, op_),
        inputRaster2_("Raster 2", dataRaster2_, op_),
        inputOutputRasterFileName_("Output raster file name", dataOutputRasterFileName_, op_),
        outputOutputRaster_("Output raster", dataOutputRaster_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input_.setDescription(tr("Used for such and such."));
        // output_.setDescription(tr("Results of the blah-di-blah."));
    }


    /**
     *
     */
    bool MultiplyRastersImpl::execute()
    {
        GDALDatasetH& raster1              = *dataRaster1_;
        GDALDatasetH& raster2              = *dataRaster2_;
        QString& outputRasterFileName = *dataOutputRasterFileName_;
        GDALDatasetH& outputRaster         = *dataOutputRaster_;
        
        GDALAllRegister();

        GDALRasterBandH hBand1, hBand2;

        hBand1 = GDALGetRasterBand(raster1,1);
        hBand2 = GDALGetRasterBand(raster2,1);

        double transform1[6];
        GDALGetGeoTransform(raster1, transform1);

        double transform2[6];
        GDALGetGeoTransform(raster2, transform2);

        for (int i = 0; i < 6; ++i)
        {
            if (transform1[i] != transform2[i])
            {
                std::cout << QString("ERROR: Transforms are not equal between rasters. Index %1 is %2 for raster 1 but %3 for raster 2").arg(i).arg(transform1[i]).arg(transform2[i]) + "\n";
                return false;
            }
        }

        int srcNodata;
        float dstNodataValue;

        dstNodataValue = (float) GDALGetRasterNoDataValue(hBand1, &srcNodata);
        
        float * data1;
        float * data2;
        data1 = new float [GDALGetRasterBandXSize(hBand1)*GDALGetRasterBandYSize(hBand1)];
        data2 = new float [GDALGetRasterBandXSize(hBand2)*GDALGetRasterBandYSize(hBand2)];

        GDALRasterIO(hBand1, GF_Read,
                0,0,
                GDALGetRasterBandXSize(hBand1),GDALGetRasterBandYSize(hBand1),
                data1,
                GDALGetRasterBandXSize(hBand1),GDALGetRasterBandYSize(hBand1),
                GDT_Float32,
                0,0);

        GDALRasterIO(hBand2, GF_Read,
                0,0,
                GDALGetRasterBandXSize(hBand2),GDALGetRasterBandYSize(hBand2),
                data2,
                GDALGetRasterBandXSize(hBand2),GDALGetRasterBandYSize(hBand2),
                GDT_Float32,
                0,0);

        //Multiply Rasters
        float * multipliedRaster;
        multipliedRaster = new float [GDALGetRasterBandXSize(hBand1)*GDALGetRasterBandYSize(hBand1)];

        for (int i = 0; i < GDALGetRasterBandXSize(hBand1)*GDALGetRasterBandYSize(hBand1); ++i)
        {
            multipliedRaster[i] = data1[i]*data2[i];
        }

        outputRaster = GDALCreate(GDALGetDatasetDriver(raster1),
                                    outputRasterFileName.toLocal8Bit().constData(),
                                    GDALGetRasterXSize(raster1),
                                    GDALGetRasterYSize(raster1),
                                    1,
                                    GDT_Float32,
                                    NULL);

        GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
        GDALSetGeoTransform(outputRaster, transform1);

        GDALSetProjection(outputRaster, GDALGetProjectionRef(raster1));
        GDALSetRasterNoDataValue(destBand, dstNodataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(hBand1), GDALGetRasterBandYSize(hBand1),
                        multipliedRaster,
                        GDALGetRasterBandXSize(hBand1), GDALGetRasterBandYSize(hBand1),
                        GDT_Float32,
                        0,0);

        return true;
    }


    /**
     *
     */
    MultiplyRasters::MultiplyRasters() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< MultiplyRasters >::getInstance(),
            tr("Multiply two rasters"))
    {
        pImpl_ = new MultiplyRastersImpl(*this);
    }


    /**
     *
     */
    MultiplyRasters::~MultiplyRasters()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  MultiplyRasters::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(MultiplyRasters, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

