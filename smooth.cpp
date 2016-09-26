#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

//OpenCV stuff
#include "opencv2/imgproc/imgproc.hpp"

#include "volcanoplugin.h"
#include "smooth.h"


namespace RF
{
    /**
     * \internal
     */
    class SmoothImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::SmoothImpl)

    public:
        Smooth&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataInputDataset_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputDatasetFilename_;
        CSIRO::DataExecution::TypedObject< double >        dataKernelSize_;
        CSIRO::DataExecution::TypedObject< double >        dataSigma_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputInputDataset_;
        CSIRO::DataExecution::InputScalar inputOutputDatasetFilename_;
        CSIRO::DataExecution::InputScalar inputKernelSize_;
        CSIRO::DataExecution::InputScalar inputSigma_;
        CSIRO::DataExecution::Output      outputOutputDataset_;


        SmoothImpl(Smooth& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    SmoothImpl::SmoothImpl(Smooth& op) :
        op_(op),
        dataInputDataset_(),
        dataOutputDatasetFilename_(),
        dataKernelSize_(),
        dataSigma_(),
        dataOutputDataset_(),
        inputInputDataset_("Input dataset", dataInputDataset_, op_),
        inputOutputDatasetFilename_("Output dataset filename", dataOutputDatasetFilename_, op_),
        inputKernelSize_("Kernel Size", dataKernelSize_, op_),
        inputSigma_("Sigma", dataSigma_, op_),
        outputOutputDataset_("Output dataset", dataOutputDataset_, op_)
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
    bool SmoothImpl::execute()
    {
        GDALDatasetH& inputDataset          = *dataInputDataset_;
        QString&      outputDatasetFilename = *dataOutputDatasetFilename_;
        double&       kernelSize            = *dataKernelSize_;
        double&       sigma                 = *dataSigma_;
        GDALDatasetH& outputDataset         = *dataOutputDataset_;
        
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
        
        //Convert float array to an openCV mat (assuming rows = Y)
        cv::Mat dataToMat(GDALGetRasterBandYSize(hBand),GDALGetRasterBandXSize(hBand), CV_32F, data);
        cv::Mat blurredData = dataToMat.clone();

        cv::GaussianBlur(dataToMat, blurredData, cv::Size(kernelSize, kernelSize), sigma);

        outputDataset = GDALCreate( GDALGetDatasetDriver(inputDataset),
                                outputDatasetFilename.toLocal8Bit().constData(),
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
                        blurredData.data,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);
        return true;
    }


    /**
     *
     */
    Smooth::Smooth() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< Smooth >::getInstance(),
            tr("Smooth dataset"))
    {
        pImpl_ = new SmoothImpl(*this);
    }


    /**
     *
     */
    Smooth::~Smooth()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  Smooth::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(Smooth, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

