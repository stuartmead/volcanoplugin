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
#include "calccurvature.h"


namespace RF
{
    /**
     * \internal
     */
    class CalcCurvatureImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::CalcCurvatureImpl)

    public:
        CalcCurvature&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >       dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >                dataRasterBand_;
        CSIRO::DataExecution::TypedObject< RF::CurvatureType >  dataCurvatureType_;
        CSIRO::DataExecution::TypedObject< bool >               dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< QString >            dataSlopeName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >       dataSlopeRaster_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputCurvatureType_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar inputSlopeName_;
        CSIRO::DataExecution::Output      outputSlopeRaster_;


        CalcCurvatureImpl(CalcCurvature& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    CalcCurvatureImpl::CalcCurvatureImpl(CalcCurvature& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataCurvatureType_(RF::CurvatureType::PROFILE),
        dataComputeEdges_(1),
        dataSlopeName_(),
        dataSlopeRaster_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputCurvatureType_("Curvature Type", dataCurvatureType_, op_),
        inputComputeEdges_("Compute Edges", dataComputeEdges_, op_),
        inputSlopeName_("Slope Name", dataSlopeName_, op_),
        outputSlopeRaster_("Slope Raster", dataSlopeRaster_, op_)
    {
    }


    /**
     *s
     */
    bool CalcCurvatureImpl::execute()
    {
        GDALDatasetH&      gDALDataset   = *dataGDALDataset_;
        int&               rasterBand    = *dataRasterBand_;
        RF::CurvatureType& curvatureType = *dataCurvatureType_;
        bool&              computeEdges  = *dataComputeEdges_;
        QString&           slopeName     = *dataSlopeName_;
        GDALDatasetH&      slopeRaster   = *dataSlopeRaster_;
        
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

        pData = GDALCreateCurvatureData(transform);

        GDALGeneric3x3ProcessingAlg pfnAlg = NULL;

        if (curvatureType == RF::CurvatureType::PROFILE)
        {
            pfnAlg = GDALCurvatureProfileAlg;
        }
        else if (curvatureType == RF::CurvatureType::CONTOUR)
        {
            pfnAlg = GDALCurvatureContourAlg;
        }
        else if (curvatureType == RF::CurvatureType::TANGENTIAL)
        {
            pfnAlg = GDALCurvatureTangentAlg;
        }

        slopeRaster = GDALCreate (GDALGetDatasetDriver(gDALDataset),
                                    slopeName.toLocal8Bit().constData(),
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
    CalcCurvature::CalcCurvature() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< CalcCurvature >::getInstance(),
            tr("Calculate Curvature"))
    {
        pImpl_ = new CalcCurvatureImpl(*this);
    }


    /**
     *
     */
    CalcCurvature::~CalcCurvature()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  CalcCurvature::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(CalcCurvature, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

