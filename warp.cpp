#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_multiproc.h"

#include "volcanoplugin.h"
#include "warp.h"


namespace RF
{
    /**
     * \internal
     */
    class WarpImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::WarpImpl)

    public:
        Warp&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataSourceDataset_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataReprojectionDataset_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDestinationDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputSourceDataset_;
        CSIRO::DataExecution::InputScalar inputReprojectionDataset_;
        CSIRO::DataExecution::InputScalar inputOutputFileName_;
        CSIRO::DataExecution::Output outputDestinationDataset_;


        WarpImpl(Warp& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    WarpImpl::WarpImpl(Warp& op) :
        op_(op),
        dataSourceDataset_(),
        dataReprojectionDataset_(),
        dataOutputFileName_(),
        dataDestinationDataset_(),
        inputSourceDataset_("Source dataset", dataSourceDataset_, op_),
        inputReprojectionDataset_("Reprojection dataset", dataReprojectionDataset_, op_),
        inputOutputFileName_("Output raster name", dataOutputFileName_, op_),
        outputDestinationDataset_("Destination dataset", dataDestinationDataset_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

        inputSourceDataset_.setDescription("Dataset to be reprojected");
        inputReprojectionDataset_.setDescription("Dataset with projection system to use");
    }


    /**
     *
     */
    bool WarpImpl::execute()
    {
        GDALDatasetH& sourceDataset       = *dataSourceDataset_;
        GDALDatasetH& reprojectionDataset = *dataReprojectionDataset_;
        GDALDatasetH& destinationDataset  = *dataDestinationDataset_;
        QString&          outputRasterFilename = *dataOutputFileName_;
        GDALAllRegister();

        //Get source data type
        GDALDataType srcDatatype = GDALGetRasterDataType(GDALGetRasterBand(sourceDataset,1));

        //Get projection systems

        const char *sourceWorldCoords, *destWorldCoords = NULL;

        sourceWorldCoords = GDALGetProjectionRef(sourceDataset);
        destWorldCoords = GDALGetProjectionRef(reprojectionDataset);

        if (sourceWorldCoords == NULL && !strlen(sourceWorldCoords) > 0)
        {
            std::cout << QString("ERROR: Source coordinate system cannot be read in") + "\n";
            return false;
        }
        if (destWorldCoords == NULL && !strlen(destWorldCoords) > 0)
        {
            std::cout << QString("ERROR: Destination coordinate system cannot be read in") + "\n";
            return false;
        }
        
        //Create a transformer from src to dest

        void *hTransformArg;

        hTransformArg = GDALCreateGenImgProjTransformer(sourceDataset, sourceWorldCoords, NULL, destWorldCoords, FALSE, 0, 1);

        //Get output bounds etc
        double adfDstGeoTransform[6];

        int nPixels = 0, nlines = 0;
        
        GDALSuggestedWarpOutput(sourceDataset, GDALGenImgProjTransform, hTransformArg, adfDstGeoTransform, &nPixels, &nlines);

        GDALDestroyGenImgProjTransformer(hTransformArg);

        //Create output dataset
        if (outputRasterFilename.isEmpty())
        {
            std::cout << QString("ERROR: You need to define a filename for the output raster") + "\n";
            return false;
        }

        destinationDataset = GDALCreate(GDALGetDatasetDriver(reprojectionDataset),
                                        outputRasterFilename.toLocal8Bit().constData(),
                                        nPixels, nlines,
                                        1, srcDatatype, NULL);

        GDALSetProjection(destinationDataset, destWorldCoords);
        GDALSetGeoTransform(destinationDataset, adfDstGeoTransform);

        //Now warp

        GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

        psWarpOptions->hSrcDS = sourceDataset;
        psWarpOptions->hDstDS = destinationDataset;
        psWarpOptions->nBandCount = 1;
        psWarpOptions->panSrcBands = (int *) CPLMalloc(sizeof(int));
        psWarpOptions->panSrcBands[0] = 1;
        psWarpOptions->panDstBands = (int *) CPLMalloc(sizeof(int));
        psWarpOptions->panDstBands[0] = 1;

        //Transformer
        psWarpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(sourceDataset,
                                                                            GDALGetProjectionRef(sourceDataset),
                                                                            destinationDataset,
                                                                            GDALGetProjectionRef(destinationDataset),
                                                                            FALSE, 0.0, 1);
        psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

        //Execute warp
        GDALWarpOperation warpOperation;

        warpOperation.Initialize(psWarpOptions);
        warpOperation.ChunkAndWarpImage(0, 0,
                                        GDALGetRasterXSize(destinationDataset),
                                        GDALGetRasterYSize(destinationDataset));
        GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
        GDALDestroyWarpOptions( psWarpOptions);

        return true;
    }


    /**
     *
     */
    Warp::Warp() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< Warp >::getInstance(),
            tr("Reproject to raster"))
    {
        pImpl_ = new WarpImpl(*this);
    }


    /**
     *
     */
    Warp::~Warp()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  Warp::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(Warp, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

