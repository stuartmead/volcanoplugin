/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

#include <cassert>

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"
#include "uplsopefailurevolume.h"
#include "erosionutils.h"


namespace RF
{
    /**
     * \internal
     */
    class UplsopeFailureVolumeImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UplsopeFailureVolumeImpl)

    public:
        UplsopeFailureVolume&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFlowDirectionDataset_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFailureDepthDataset_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterFilename_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataAccumulatedFailureVolume_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputFlowDirectionDataset_;
        CSIRO::DataExecution::InputScalar inputFailureDepthDataset_;
        CSIRO::DataExecution::InputScalar inputOutputRasterFilename_;
        CSIRO::DataExecution::Output      outputAccumulatedFailureVolume_;


        UplsopeFailureVolumeImpl(UplsopeFailureVolume& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    UplsopeFailureVolumeImpl::UplsopeFailureVolumeImpl(UplsopeFailureVolume& op) :
        op_(op),
        dataFlowDirectionDataset_(),
        dataFailureDepthDataset_(),
        dataOutputRasterFilename_(),
        dataAccumulatedFailureVolume_(),
        inputFlowDirectionDataset_("Flow direction dataset", dataFlowDirectionDataset_, op_),
        inputFailureDepthDataset_("Failure depth dataset", dataFailureDepthDataset_, op_),
        inputOutputRasterFilename_("Output raster filename", dataOutputRasterFilename_, op_),
        outputAccumulatedFailureVolume_("Accumulated failure volume", dataAccumulatedFailureVolume_, op_)
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
    bool UplsopeFailureVolumeImpl::execute()
    {
        GDALDatasetH& flowDirectionDataset     = *dataFlowDirectionDataset_;
        GDALDatasetH& failureDepthDataset      = *dataFailureDepthDataset_;
        QString&      outputRasterFilename     = *dataOutputRasterFilename_;
        GDALDatasetH& accumulatedFailureVolume = *dataAccumulatedFailureVolume_;
        
		GDALRasterBandH fBand;
		GDALRasterBandH depthBand;

        fBand = GDALGetRasterBand(flowDirectionDataset, 1);
        depthBand = GDALGetRasterBand(failureDepthDataset, 1);

        double transformf[6];
        GDALGetGeoTransform(flowDirectionDataset, transformf);
		
		double transformd[6];
        GDALGetGeoTransform(failureDepthDataset, transformd);

        for (int i = 0; i < 6; ++i)
        {
            if (transformf[i] != transformd[i])
            {
                std::cout << QString("ERROR: Transforms are not equal between rasters. Index %1 is %2 for flow direction but %3 for depth").arg(i).arg(transformf[i]).arg(transformd[i]) + "\n";
                //return false;
            }
        }

        //Setup
        float srcNodataValue;
        int srcNoData;

        srcNodataValue = (float)GDALGetRasterNoDataValue(fBand, &srcNoData);
        float dstNoDataValue = srcNodataValue;
        

        void *pData;

        pData = RecursiveCreateInputData(transformf);

        GenericRecursiveFailureAlgebraAlg pfnAlg = RecursiveUpstreamFailureAlg;

        int xSize = GDALGetRasterBandXSize(fBand);
        int ySize = GDALGetRasterBandYSize(fBand);
        float* failureDepth = new float[xSize*ySize];

        GDALRasterIO(depthBand, GF_Read,
            0, 0, //X,Y offset in cells
            xSize, ySize, //X,Y length in cells
            failureDepth, //data
            ySize, xSize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);

        accumulatedFailureVolume = GDALCreate( GDALGetDatasetDriver(flowDirectionDataset),
                                            outputRasterFilename.toLocal8Bit().constData(),
                                            GDALGetRasterXSize(flowDirectionDataset),
                                            GDALGetRasterYSize(flowDirectionDataset),
                                            1,
                                            GDT_Float32, NULL);
        
        GDALRasterBandH algBand = GDALGetRasterBand(accumulatedFailureVolume, 1);
        GDALSetGeoTransform(accumulatedFailureVolume, transformf);
        GDALSetProjection(accumulatedFailureVolume, GDALGetProjectionRef(flowDirectionDataset));
        GDALSetRasterNoDataValue(algBand, dstNoDataValue);

        GenericRecursiveFailureAlgebraProcessor(fBand, depthBand, algBand, transformf, pfnAlg, pData);

        return true;
    }


    /**
     *
     */
    UplsopeFailureVolume::UplsopeFailureVolume() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< UplsopeFailureVolume >::getInstance(),
            tr("Calculate upslope failure volume"))
    {
        pImpl_ = new UplsopeFailureVolumeImpl(*this);
    }


    /**
     *
     */
    UplsopeFailureVolume::~UplsopeFailureVolume()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  UplsopeFailureVolume::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(UplsopeFailureVolume, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

