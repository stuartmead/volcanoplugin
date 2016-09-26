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
#include "scalerastervalues.h"


namespace RF
{
    /**
     * \internal
     */
    class ScaleRasterValuesImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScaleRasterValuesImpl)

    public:
        ScaleRasterValues&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataInputDataset_;
        CSIRO::DataExecution::TypedObject< double >        dataScalingFactor_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputDataset_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputDatasetName_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputInputDataset_;
        CSIRO::DataExecution::InputScalar inputScalingFactor_;
        CSIRO::DataExecution::Output      outputOutputDataset_;
        CSIRO::DataExecution::InputScalar inputOutputDatasetName_;


        ScaleRasterValuesImpl(ScaleRasterValues& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    ScaleRasterValuesImpl::ScaleRasterValuesImpl(ScaleRasterValues& op) :
        op_(op),
        dataInputDataset_(),
        dataScalingFactor_(1.0),
        dataOutputDataset_(),
        dataOutputDatasetName_(),
        inputInputDataset_("Input dataset", dataInputDataset_, op_),
        inputScalingFactor_("Scaling factor", dataScalingFactor_, op_),
        outputOutputDataset_("Output dataset", dataOutputDataset_, op_),
        inputOutputDatasetName_("Output dataset name", dataOutputDatasetName_, op_)
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
    bool ScaleRasterValuesImpl::execute()
    {
        GDALDatasetH& inputDataset      = *dataInputDataset_;
        double&       scalingFactor     = *dataScalingFactor_;
        GDALDatasetH& outputDataset     = *dataOutputDataset_;
        QString&      outputDatasetName = *dataOutputDatasetName_;
        
            
        GDALRasterBandH hBand = GDALGetRasterBand(inputDataset, 1);

        float *data;
        data = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

        GDALRasterIO( hBand, GF_Read,
            0,0,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            data,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            GDT_Float32,
            0,0);
        
        int srcNoData;
        float srcNoDataValue;

        srcNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);
        std::cout << QString("Band nodata value is %1").arg(srcNoDataValue) + "\n";
        
        for (int it = 0; it < GDALGetRasterBandYSize(hBand)*GDALGetRasterBandXSize(hBand); ++it)
        {
            
            data[it] = data[it]*scalingFactor;            
        }

                outputDataset = GDALCreate( GDALGetDatasetDriver(inputDataset),
                                outputDatasetName.toLocal8Bit().constData(),
                                GDALGetRasterXSize(inputDataset), GDALGetRasterYSize(inputDataset),
                                1,
                                GDT_Float32, NULL);
        double transform[6];
        GDALGetGeoTransform(inputDataset,transform);

        GDALRasterBandH destBand = GDALGetRasterBand(outputDataset, 1);
        GDALSetGeoTransform(outputDataset, transform);
        GDALSetProjection(outputDataset, GDALGetProjectionRef(inputDataset));
        GDALSetRasterNoDataValue(destBand, srcNoDataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        data,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);

       
        return true;
    }


    /**
     *
     */
    ScaleRasterValues::ScaleRasterValues() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< ScaleRasterValues >::getInstance(),
            tr("Scale raster z values"))
    {
        pImpl_ = new ScaleRasterValuesImpl(*this);
    }


    /**
     *
     */
    ScaleRasterValues::~ScaleRasterValues()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  ScaleRasterValues::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(ScaleRasterValues, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

