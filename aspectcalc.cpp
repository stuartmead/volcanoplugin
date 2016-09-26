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
#include "aspectcalc.h"


namespace RF
{
    /**
     * \internal
     */
    class AspectCalcImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::AspectCalcImpl)

    public:
        AspectCalc&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >      dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >               dataRasterBand_;
        CSIRO::DataExecution::TypedObject< RF::SlopeAlgType >  dataAspectAlgorithim_;
        CSIRO::DataExecution::TypedObject< bool >              dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< QString >           dataOutputRasterFilename_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >      dataOutputGDALDataset_;
        CSIRO::DataExecution::TypedObject< bool >              dataUseAngeAsAzimuth_;
        CSIRO::DataExecution::TypedObject< bool >              dataSetFlatAresToNODATA_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputAspectAlgorithim_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar  inputOutputRasterFilename_;
        CSIRO::DataExecution::Output      outputOutputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputUseAngeAsAzimuth_;
        CSIRO::DataExecution::InputScalar inputSetFlatAresToNODATA_;


        AspectCalcImpl(AspectCalc& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    AspectCalcImpl::AspectCalcImpl(AspectCalc& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataAspectAlgorithim_(RF::SlopeAlgType::THORNE),
        dataComputeEdges_(1),
        dataOutputRasterFilename_(),
        dataOutputGDALDataset_(),
        dataUseAngeAsAzimuth_(TRUE),
        dataSetFlatAresToNODATA_(TRUE),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputAspectAlgorithim_("Aspect Algorithim", dataAspectAlgorithim_, op_),
        inputComputeEdges_("Compute Edges", dataComputeEdges_, op_),
        inputOutputRasterFilename_("Output Raster Filename", dataOutputRasterFilename_, op_),
        outputOutputGDALDataset_("Output GDAL Dataset", dataOutputGDALDataset_, op_),
        inputUseAngeAsAzimuth_("Use angle as azimuth", dataUseAngeAsAzimuth_, op_),
        inputSetFlatAresToNODATA_("Set flat ares to NODATA", dataSetFlatAresToNODATA_, op_)
    {
    }


    /**
     *
     */
    bool AspectCalcImpl::execute()
    {
        GDALDatasetH&     gDALDataset          = *dataGDALDataset_;
        int&              rasterBand           = *dataRasterBand_;
        RF::SlopeAlgType& aspectAlgorithim     = *dataAspectAlgorithim_;
        bool&             computeEdges         = *dataComputeEdges_;
        QString&          outputRasterFilename = *dataOutputRasterFilename_;
        GDALDatasetH&     outputGDALDataset    = *dataOutputGDALDataset_;
        bool&             useAngeAsAzimuth     = *dataUseAngeAsAzimuth_;
        bool&             setFlatAresToNODATA  = *dataSetFlatAresToNODATA_;
        
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
        bool dstNoData = true;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);

        if (!setFlatAresToNODATA)
        {
            dstNoDataValue = 0.0;
        }

        void* pData = GDALCreateAspectData(useAngeAsAzimuth);

        GDALGeneric3x3ProcessingAlg pfnAlg = NULL;

        if ( aspectAlgorithim == RF::SlopeAlgType::THORNE)
        {
            pfnAlg = GDALAspectAlg;
        }
        else if (aspectAlgorithim == RF::SlopeAlgType::ZEVTHORNE)
        {
            pfnAlg = GDALAspectZevenbergenThorneAlg;
        }

        outputGDALDataset = GDALCreate ( GDALGetDatasetDriver(gDALDataset),
                            outputRasterFilename.toLocal8Bit().constData(),
                            GDALGetRasterXSize(gDALDataset),
                            GDALGetRasterYSize(gDALDataset),
                            1,
                            GDT_Float32, NULL);
        
        GDALRasterBandH destBand = GDALGetRasterBand(outputGDALDataset, 1);
        GDALSetGeoTransform(outputGDALDataset, transform);
        GDALSetProjection(outputGDALDataset, GDALGetProjectionRef(gDALDataset));

        GDALSetRasterNoDataValue(destBand, dstNoDataValue);


        GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);
        

        return true;
    }


    /**
     *
     */
    AspectCalc::AspectCalc() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< AspectCalc >::getInstance(),
            tr("Calculate Slope Aspect"))
    {
        pImpl_ = new AspectCalcImpl(*this);
    }


    /**
     *
     */
    AspectCalc::~AspectCalc()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  AspectCalc::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(AspectCalc, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

