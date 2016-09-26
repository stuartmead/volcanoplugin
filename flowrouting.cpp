/*
Created by: Stuart Mead
  Creation date: 2014-02-03
   
  Revision:       $Revision: $
  Last changed:   $Date: $
  Last changed by: Stuart Mead

  Copyright Risk Frontiers 2014, Faculty of Science, Macquarie University, NSW 2109, Australia.

  For further information, contact:
          Stuart Mead
          Building E7A
          Dept. of Environment & Geography
          Macquarie University
          North Ryde NSW 2109

  This copyright notice must be included with all copies of the source code.
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
#include "volcanoutils.h"
#include "flowrouting.h"


namespace RF
{
    /**
     * \internal
     */
    class FlowRoutingImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FlowRoutingImpl)

    public:
        FlowRouting&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< RF::FlowDirType> dataFlowDirType_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< bool >          dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< QString >       dataDestinationFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputFlowDirType_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar      outputDestinationFileName_;
        CSIRO::DataExecution::Output      outputOutputDataset_;


        FlowRoutingImpl(FlowRouting& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    FlowRoutingImpl::FlowRoutingImpl(FlowRouting& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataComputeEdges_(true),
        dataFlowDirType_(RF::DINF),
        dataDestinationFileName_(),
        dataOutputDataset_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputFlowDirType_("Flow direction calculation", dataFlowDirType_, op_),
        inputComputeEdges_("Compute Edges", dataComputeEdges_, op_),
        outputDestinationFileName_("Destination File Name", dataDestinationFileName_, op_),
        outputOutputDataset_("Output Dataset", dataOutputDataset_, op_)
    {
    }


    /**
     *
     */
    bool FlowRoutingImpl::execute()
    {
        GDALDatasetH& gDALDataset         = *dataGDALDataset_;
        int&          rasterBand          = *dataRasterBand_;
        bool&         computeEdges        = *dataComputeEdges_;
        QString&      destinationFileName = *dataDestinationFileName_;
        GDALDatasetH& outputDataset       = *dataOutputDataset_;
        

        GDALAllRegister();

        GDALRasterBandH hBand;
        if(rasterBand > GDALGetRasterCount(gDALDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDataset)).arg(rasterBand) + "\n";
            return false;
        }
        hBand = GDALGetRasterBand(gDALDataset, rasterBand);

        double transform[6];
        GDALGetGeoTransform(gDALDataset, transform);

        //Setup
        int srcNoData;
        float dstNoDataValue;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);

        void *pData;

        pData = GDALCreateFlowDirectionData(transform);

        GDALGeneric3x3ProcessingAlg pfnAlg = NULL;

        if (*dataFlowDirType_==RF::FlowDirType::DINF)
        {
            outputDataset = GDALCreate(GDALGetDatasetDriver(gDALDataset),
                            destinationFileName.toLocal8Bit().constData(),
                            GDALGetRasterXSize(gDALDataset),
                            GDALGetRasterYSize(gDALDataset),
                            1,
                            GDT_Float32, NULL);
        
            GDALRasterBandH destBand = GDALGetRasterBand(outputDataset, 1);
            GDALSetGeoTransform(outputDataset, transform);
            GDALSetProjection(outputDataset, GDALGetProjectionRef(gDALDataset));

            GDALSetRasterNoDataValue(destBand, dstNoDataValue);
            pfnAlg = GDALFlowDirectionInfAlg;
            GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);
        }
        else if (*dataFlowDirType_==RF::FlowDirType::D8)
        {
             outputDataset = GDALCreate(GDALGetDatasetDriver(gDALDataset),
                            destinationFileName.toLocal8Bit().constData(),
                            GDALGetRasterXSize(gDALDataset),
                            GDALGetRasterYSize(gDALDataset),
                            1,
                            GDT_Int32, NULL);
        
            GDALRasterBandH destBand = GDALGetRasterBand(outputDataset, 1);
            GDALSetGeoTransform(outputDataset, transform);
            GDALSetProjection(outputDataset, GDALGetProjectionRef(gDALDataset));

            GDALSetRasterNoDataValue(destBand, dstNoDataValue);

            pfnAlg = GDALFlowDirection8Alg;
            GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);
            
            int count = 0;
            int countUndef = 0;
            int *data;
            data = new int [GDALGetRasterBandYSize(destBand)*GDALGetRasterBandXSize(destBand)];

            GDALRasterIO( destBand, GF_Read,
                0,0,
                GDALGetRasterBandXSize(destBand), GDALGetRasterBandYSize(destBand),
                data,
                GDALGetRasterBandXSize(destBand), GDALGetRasterBandYSize(destBand),
                GDT_Int32,
                0,0);
            for (int it = 0; it < GDALGetRasterBandYSize(destBand)*GDALGetRasterBandXSize(destBand); ++it)
            {
                if (data[it]!= 1 && data[it]!= 2 && data[it]!= 4 && data[it]!= 8 && data[it]!= 16 && data[it]!= 32 && data[it]!= 64 && data[it]!= 128 && data[it] > 0)
                {
                    ++count;
                    if (data[it] > 0)
                    {
                        ++countUndef;
                    }
                }
            }
            std::cout << QString("Still have %1 undefined flow points, %2 of which are not nodata").arg(count).arg(countUndef) + "\n";
            delete[] data;
            
            while (count > 0)
            {
                GDALGeneric3x3ProcessingAlg itAlg = GDALFlowDirection8IterativeAlg;
                GDALGeneric3x3Processing(destBand, destBand, itAlg, pData, computeEdges);

                count = 0;
                float *data;
                data = new float [GDALGetRasterBandYSize(destBand)*GDALGetRasterBandXSize(destBand)];
                GDALRasterIO( destBand, GF_Read,
                    0,0,
                    GDALGetRasterBandXSize(destBand), GDALGetRasterBandYSize(destBand),
                    data,
                    GDALGetRasterBandXSize(destBand), GDALGetRasterBandYSize(destBand),
                    GDT_Float32,
                    0,0);
                
                for (int it = 0; it < GDALGetRasterBandYSize(hBand)*GDALGetRasterBandXSize(hBand); ++it)
                {
                    if (data[it]!= 1 && data[it]!= 2 && data[it]!= 4 && data[it]!= 8 && data[it]!= 16 && data[it]!= 32 && data[it]!= 64 && data[it]!= 128 && data[it] > 0)
                    {
                        ++count;
                    }
                }
                delete[] data;
                if (!computeEdges && count == (GDALGetRasterBandYSize(hBand)+GDALGetRasterBandXSize(hBand))*2)
                {
                    count = 0;
                }
                std::cout << QString("Still have %1 undefined flow points").arg(count) + "\n";
            }            
        }
        

        return true;
    }


    /**
     *
     */
    FlowRouting::FlowRouting() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< FlowRouting >::getInstance(),
            tr("Determine flow direction"))
    {
        pImpl_ = new FlowRoutingImpl(*this);
    }


    /**
     *
     */
    FlowRouting::~FlowRouting()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  FlowRouting::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(FlowRouting, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

