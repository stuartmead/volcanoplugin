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


#include "volcanoplugin.h"
#include "erosionutils.h"
#include "volcanoutils.h"
#include "upslopearea.h"


namespace RF
{
    /**
     * \internal
     */
    class UpslopeAreaImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UpslopeAreaImpl)

    public:
        UpslopeArea&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFlowDirectionDataset_;
        CSIRO::DataExecution::TypedObject< int >                dataRasterBand_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataAccumulationDataset_;
        CSIRO::DataExecution::TypedObject< QString >            dataSlopeName_;

        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputFlowDirectionDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputSlopeName_;
        CSIRO::DataExecution::Output      outputAccumulationDataset_;


        UpslopeAreaImpl(UpslopeArea& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    UpslopeAreaImpl::UpslopeAreaImpl(UpslopeArea& op) :
        op_(op),
        dataFlowDirectionDataset_(),
        dataRasterBand_(1),
        dataSlopeName_(),
        dataAccumulationDataset_(),
        inputFlowDirectionDataset_("Flow direction dataset", dataFlowDirectionDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputSlopeName_("Raster Name", dataSlopeName_, op_),
        outputAccumulationDataset_("Accumulation dataset", dataAccumulationDataset_, op_)
    {
    }
        
    /**
     *
     */
    bool UpslopeAreaImpl::execute()
    {
        GDALDatasetH& flowDirectionDataset = *dataFlowDirectionDataset_;
        int&               rasterBand    = *dataRasterBand_;
        GDALDatasetH& accumulationDataset  = *dataAccumulationDataset_;
        QString&           slopeName     = *dataSlopeName_;

        
        GDALAllRegister();

        GDALRasterBandH fBand;

        if (rasterBand > GDALGetRasterCount(flowDirectionDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(flowDirectionDataset)).arg(rasterBand) + "\n";
            return false;
        }

        fBand = GDALGetRasterBand(flowDirectionDataset, rasterBand);;

        double transform[6];
        GDALGetGeoTransform(flowDirectionDataset,transform);

        
        //Setup
        float srcNodataValue;
        int srcNoData;

        srcNodataValue = (float)GDALGetRasterNoDataValue(fBand, &srcNoData);
        float dstNoDataValue = srcNodataValue;
        

        void *pData;

        pData = RecursiveCreateInputData(transform);

        GenericRecursiveFlowAlgebraAlg pfnAlg = RecursiveUpstreamFlowAlg;




        
        int xSize = GDALGetRasterBandXSize(fBand);
        int ySize = GDALGetRasterBandYSize(fBand);
      
        accumulationDataset = GDALCreate( GDALGetDatasetDriver(flowDirectionDataset),
                                            slopeName.toLocal8Bit().constData(),
                                            GDALGetRasterXSize(flowDirectionDataset),
                                            GDALGetRasterYSize(flowDirectionDataset),
                                            1,
                                            GDT_Float32, NULL);
        
        GDALRasterBandH algBand = GDALGetRasterBand(accumulationDataset, 1);
        GDALSetGeoTransform(accumulationDataset, transform);
        GDALSetProjection(accumulationDataset, GDALGetProjectionRef(flowDirectionDataset));
        GDALSetRasterNoDataValue(algBand, dstNoDataValue);

        GenericRecursiveFlowAlgebraProcessor(fBand, algBand, transform, pfnAlg, pData);
  
        return true;
    }

    
    /**
     *
     */
    UpslopeArea::UpslopeArea() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< UpslopeArea >::getInstance(),
            tr("Calculate upslope flow area"))
    {
        pImpl_ = new UpslopeAreaImpl(*this);
    }


    /**
     *
     */
    UpslopeArea::~UpslopeArea()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  UpslopeArea::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(UpslopeArea, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

