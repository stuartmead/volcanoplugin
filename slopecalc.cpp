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
#include "slopecalc.h"


namespace RF
{
    /**
     * \internal
     */
    class SlopeCalcImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::SlopeCalcImpl)

    public:
        SlopeCalc&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject<RF::SlopeAlgType> dataSlopeAlgorithim_;
        CSIRO::DataExecution::TypedObject< double >         dataScale_;
        CSIRO::DataExecution::TypedObject< bool >          dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< bool >          dataPercentSlope_;
        CSIRO::DataExecution::TypedObject< QString >       dataSlopeName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataSlopeRaster_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputSlopeAlgorithim_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar inputScale_;
        CSIRO::DataExecution::InputScalar inputPercentSlope_;
        CSIRO::DataExecution::InputScalar inputSlopeName_;
        CSIRO::DataExecution::Output      outputSlopeRaster_;


        SlopeCalcImpl(SlopeCalc& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    SlopeCalcImpl::SlopeCalcImpl(SlopeCalc& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataSlopeAlgorithim_(RF::SlopeAlgType::THORNE),
        dataComputeEdges_(1),
        dataSlopeRaster_(),
        dataScale_(1),
        dataPercentSlope_(0),
        dataSlopeName_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputSlopeAlgorithim_("Slope Algorithim", dataSlopeAlgorithim_, op_),
        inputComputeEdges_("Compute edges", dataComputeEdges_, op_),
        inputScale_("Scale", dataScale_, op_),
        inputPercentSlope_("Return slope as a percentage", dataPercentSlope_, op_),
        inputSlopeName_("Slope raster filename", dataSlopeName_, op_),
        outputSlopeRaster_("Slope raster", dataSlopeRaster_, op_)
    {
        inputScale_.setDescription("Scale is the ratio of vertical units to horizontal, for Feet:Latlong use scale=370400, for Meters:LatLong use scale=111120");
    }


    /**
     *
     */
    bool SlopeCalcImpl::execute()
    {
        GDALDatasetH& gDALDataset       = *dataGDALDataset_;
        int&          rasterBand        = *dataRasterBand_;
        RF::SlopeAlgType& slopeAlg      = *dataSlopeAlgorithim_;
        bool&         computeEdges      = *dataComputeEdges_;
        QString&      dstFilename       = *dataSlopeName_;
        GDALDatasetH& slopeRaster       = *dataSlopeRaster_;
        
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
        
        int slopeFormat = 1;
        if (*dataPercentSlope_)
        {
            slopeFormat = 0;
        }

        pData = GDALCreateSlopeData(transform, *dataScale_, slopeFormat);

        GDALGeneric3x3ProcessingAlg pfnAlg = NULL;

        if (slopeAlg == RF::SlopeAlgType::THORNE)
        {
            pfnAlg = GDALSlopeHornAlg;
        }
        else if (slopeAlg == RF::SlopeAlgType::ZEVTHORNE)
        {
            pfnAlg = GDALSlopeZevenbergenThorneAlg;
        }


        slopeRaster = GDALCreate ( GDALGetDatasetDriver(gDALDataset),
                                    dstFilename.toLocal8Bit().constData(),
                                    GDALGetRasterXSize(gDALDataset),
                                    GDALGetRasterYSize(gDALDataset),
                                    1,
                                    GDT_Float32, NULL);

        GDALRasterBandH destBand = GDALGetRasterBand(slopeRaster, 1);
        GDALSetGeoTransform(slopeRaster, transform);
        GDALSetProjection(slopeRaster, GDALGetProjectionRef(gDALDataset));

        GDALSetRasterNoDataValue(destBand, dstNoDataValue);

        GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);

                                    

        return true;
    }


    /**
     *
     */
    SlopeCalc::SlopeCalc() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< SlopeCalc >::getInstance(),
            tr("Calculate Slope"))
    {
        pImpl_ = new SlopeCalcImpl(*this);
    }


    /**
     *
     */
    SlopeCalc::~SlopeCalc()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  SlopeCalc::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(SlopeCalc, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

