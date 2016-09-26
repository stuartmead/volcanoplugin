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
#include "erosionutils.h"
#include "debrisemergence.h"


namespace RF
{
    /**
     * \internal
     */
    class DebrisEmergenceImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::DebrisEmergenceImpl)

    public:
        DebrisEmergence&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< double >        dataGrainConcentration_;
        CSIRO::DataExecution::TypedObject< double >        dataGrainDensity_;
        CSIRO::DataExecution::TypedObject< double >        dataFluidDensity_;
        CSIRO::DataExecution::TypedObject< double >        dataFrictionAngle_;
        CSIRO::DataExecution::TypedObject< bool >          dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputGrainConcentration_;
        CSIRO::DataExecution::InputScalar inputGrainDensity_;
        CSIRO::DataExecution::InputScalar inputFluidDensity_;
        CSIRO::DataExecution::InputScalar inputFrictionAngle_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar inputOutputRasterName_;
        CSIRO::DataExecution::Output      outputOutputRaster_;


        DebrisEmergenceImpl(DebrisEmergence& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    DebrisEmergenceImpl::DebrisEmergenceImpl(DebrisEmergence& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataGrainConcentration_(0.7),
        dataGrainDensity_(2650),
        dataFluidDensity_(1000),
        dataFrictionAngle_(36),
        dataComputeEdges_(true),
        dataOutputRasterName_(),
        dataOutputRaster_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputGrainConcentration_("Grain concentration", dataGrainConcentration_, op_),
        inputGrainDensity_("Grain density", dataGrainDensity_, op_),
        inputFluidDensity_("Fluid density", dataFluidDensity_, op_),
        inputFrictionAngle_("Friction angle", dataFrictionAngle_, op_),
        inputComputeEdges_("Compute Edges", dataComputeEdges_, op_),
        inputOutputRasterName_("Output Raster Name", dataOutputRasterName_, op_),
        outputOutputRaster_("Output Raster", dataOutputRaster_, op_)
    {
    }


    /**
     *
     */
    bool DebrisEmergenceImpl::execute()
    {
        GDALDatasetH& gDALDataset        = *dataGDALDataset_;
        int&          rasterBand         = *dataRasterBand_;
        double&       grainConcentration = *dataGrainConcentration_;
        double&       grainDensity       = *dataGrainDensity_;
        double&       fluidDensity       = *dataFluidDensity_;
        double&       frictionAngle      = *dataFrictionAngle_;
        bool&         computeEdges       = *dataComputeEdges_;
        QString&      outputRasterName   = *dataOutputRasterName_;
        GDALDatasetH& outputRaster       = *dataOutputRaster_;
        

        //Get raster input band and transform
        GDALAllRegister();

        GDALRasterBandH hBand;
        if(rasterBand > GDALGetRasterCount(gDALDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDataset)).arg(rasterBand) + "\n";
            return false;
        }
        hBand = GDALGetRasterBand(gDALDataset, rasterBand);

        double transform[6];
        GDALGetGeoTransform(gDALDataset,transform);

        //Setup
        int srcNoData;
        float dstNoDataValue;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);

        void *pData;

        pData = TakashiCreateInputData(grainConcentration, grainDensity, fluidDensity, frictionAngle);

        GDALGeneric3x3ProcessingAlg pfnAlg = TakashiEmergenceAlg;

        outputRaster = GDALCreate ( GDALGetDatasetDriver(gDALDataset),
                            outputRasterName.toLocal8Bit().constData(),
                            GDALGetRasterXSize(gDALDataset),
                            GDALGetRasterYSize(gDALDataset),
                            1,
                            GDT_Float32, NULL);

        GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
        GDALSetGeoTransform(outputRaster, transform);
        GDALSetProjection(outputRaster, GDALGetProjectionRef(gDALDataset));
        GDALSetRasterNoDataValue(destBand, dstNoDataValue);

        GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);            

        return true;
    }


    /**
     *
     */
    DebrisEmergence::DebrisEmergence() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< DebrisEmergence >::getInstance(),
            tr("Calculate debris flow emergence FOS"))
    {
        pImpl_ = new DebrisEmergenceImpl(*this);
    }


    /**
     *
     */
    DebrisEmergence::~DebrisEmergence()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  DebrisEmergence::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(DebrisEmergence, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

