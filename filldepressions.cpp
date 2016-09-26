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
#include "filldepressions.h"


namespace RF
{
    /**
     * \internal
     */
    class FillDepressionsImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FillDepressionsImpl)

    public:
        FillDepressions&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< bool >          dataComputeEdges_;
        CSIRO::DataExecution::TypedObject< QString >       dataDestinationFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputComputeEdges_;
        CSIRO::DataExecution::InputScalar      inputDestinationFileName_;
        CSIRO::DataExecution::Output      inputOutputDataset_;


        FillDepressionsImpl(FillDepressions& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    FillDepressionsImpl::FillDepressionsImpl(FillDepressions& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataComputeEdges_(true),
        dataDestinationFileName_(),
        dataOutputDataset_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputComputeEdges_("Compute Edges", dataComputeEdges_, op_),
        inputDestinationFileName_("Destination File Name", dataDestinationFileName_, op_),
        inputOutputDataset_("Output Dataset", dataOutputDataset_, op_)
    {
    }


    /**
     *
     */
    bool FillDepressionsImpl::execute()
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

        GDALGeneric3x3ProcessingAlg pfnAlg = NULL;

        pfnAlg = GDALFillDepressionsAlg;

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

        GDALGeneric3x3Processing(hBand, destBand, pfnAlg, pData, computeEdges);

        return true;
    }


    /**
     *
     */
    FillDepressions::FillDepressions() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< FillDepressions >::getInstance(),
            tr("Fill sinks in raster"))
    {
        pImpl_ = new FillDepressionsImpl(*this);
    }


    /**
     *
     */
    FillDepressions::~FillDepressions()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  FillDepressions::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(FillDepressions, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

