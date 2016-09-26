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
#include "volcanoplugin.h"
#include "rasterdifference.h"


namespace RF
{
    /**
     * \internal
     */
    class RasterDifferenceImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RasterDifferenceImpl)

    public:
        RasterDifference&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDataset1_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDataset2_;
        CSIRO::DataExecution::TypedObject< double >        dataMinimum_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDifference_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand2_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputDataset1_;
        CSIRO::DataExecution::InputScalar inputDataset2_;
        CSIRO::DataExecution::InputScalar inputMinimum_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputOutputFileName_;
        CSIRO::DataExecution::Output      outputDifference_;
        CSIRO::DataExecution::InputScalar inputRasterBand2_;


        RasterDifferenceImpl(RasterDifference& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    RasterDifferenceImpl::RasterDifferenceImpl(RasterDifference& op) :
        op_(op),
        dataDataset1_(),
        dataDataset2_(),
        dataMinimum_(0.0),
        dataRasterBand_(1),
        dataOutputFileName_(),
        dataDifference_(),
        dataRasterBand2_(1),
        inputDataset1_("Dataset 1", dataDataset1_, op_),
        inputDataset2_("Dataset 2", dataDataset2_, op_),
        inputMinimum_("Minimum value", dataMinimum_, op_),
        inputRasterBand_("Raster band 1", dataRasterBand_, op_),
        inputOutputFileName_("Output file name", dataOutputFileName_, op_),
        outputDifference_("Difference", dataDifference_, op_),
        inputRasterBand2_("Raster band 2", dataRasterBand2_, op_)
    {
    }


    /**
     *
     */
    bool RasterDifferenceImpl::execute()
    {
        GDALDatasetH& dataset1       = *dataDataset1_;
        GDALDatasetH& dataset2       = *dataDataset2_;
        int&          rasterBand     = *dataRasterBand_;
        QString&      outputFileName = *dataOutputFileName_;
        GDALDatasetH& difference     = *dataDifference_;
        int&          rasterBand2    = *dataRasterBand2_;
        

        GDALAllRegister();

        GDALRasterBandH band1;
        GDALRasterBandH band2;

        if (rasterBand > GDALGetRasterCount(dataset1))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(dataset1)).arg(rasterBand) + "\n";
            return false;
        }

        if (rasterBand2 > GDALGetRasterCount(dataset2))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(dataset2)).arg(rasterBand2) + "\n";
            return false;
        }

        band1 = GDALGetRasterBand(dataset1, rasterBand);
        band2 = GDALGetRasterBand(dataset2, rasterBand2);

        //Setup
        int srcNoData;
        float dstNoDataValue;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(band1, &srcNoData);

        std::cout << QString("GDAL data type is %1").arg(GDALGetRasterDataType(band1)) + "\n";

        float * band1Data;
        band1Data = new float [GDALGetRasterBandXSize(band1)*GDALGetRasterBandYSize(band1)];
        float * band2Data;
        band2Data = new float [GDALGetRasterBandXSize(band2)*GDALGetRasterBandYSize(band2)];

       GDALRasterIO( band1, GF_Read,
            0,0,
            GDALGetRasterBandXSize(band1), GDALGetRasterBandYSize(band1),
            band1Data,
            GDALGetRasterBandXSize(band1), GDALGetRasterBandYSize(band1),
            GDALGetRasterDataType(band1),
            0,0);
      
        GDALRasterIO( band2, GF_Read,
            0,0,
            GDALGetRasterBandXSize(band2), GDALGetRasterBandYSize(band2),
            band2Data,
            GDALGetRasterBandXSize(band2), GDALGetRasterBandYSize(band2),
            GDALGetRasterDataType(band2),
            0,0);
           
        std::cout << QString("Read in raster bands") + "\n";

        
        if (GDALGetRasterBandXSize(band1) != GDALGetRasterBandXSize(band2))
        {
            std::cout << QString("ERROR: Raster band x sizes are not equal, are %1, %2").arg(GDALGetRasterBandXSize(band1)).arg(GDALGetRasterBandXSize(band2)) + "\n";
            return false;
        }
        if (GDALGetRasterBandYSize(band1) != GDALGetRasterBandYSize(band2))
        {
            std::cout << QString("ERROR: Raster band y sizes are not equal, are %1, %2").arg(GDALGetRasterBandYSize(band1)).arg(GDALGetRasterBandYSize(band2)) + "\n";
            return false;
        }

        
        float * ouputData;
        ouputData = new float [GDALGetRasterBandXSize(band1)*GDALGetRasterBandYSize(band1)];
        
        for (int it = 0; it < GDALGetRasterBandXSize(band1)*GDALGetRasterBandYSize(band1); ++it)
        {
            if (*dataMinimum_ < band1Data[it] - band2Data[it])
            {
                ouputData[it] = band1Data[it] - band2Data[it];
            }
            else
            {
                ouputData[it] = (float)*dataMinimum_;
            }
            
        }
        

        difference = GDALCreate(GDALGetDatasetDriver(dataset1),
                                outputFileName.toLocal8Bit().constData(),
                                GDALGetRasterXSize(dataset1), GDALGetRasterYSize(dataset1),    
                                1,
                                GDT_Float32, NULL);

       
                                
    
        double transform[6];
        GDALGetGeoTransform(dataset1, transform);
        GDALSetGeoTransform(difference, transform);
        GDALSetProjection(difference, GDALGetProjectionRef(dataset1));
        

        GDALRasterBandH destBand = GDALGetRasterBand(difference, 1);
        GDALSetRasterNoDataValue(destBand, dstNoDataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(band1), GDALGetRasterBandYSize(band1),
                        ouputData,
                        GDALGetRasterBandXSize(band1), GDALGetRasterBandYSize(band1),
                        GDT_Float32,
                        0,0);
                        

        return true;
    }


    /**
     *
     */
    RasterDifference::RasterDifference() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< RasterDifference >::getInstance(),
            tr("Difference between two rasters"))
    {
        pImpl_ = new RasterDifferenceImpl(*this);
    }


    /**
     *
     */
    RasterDifference::~RasterDifference()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  RasterDifference::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(RasterDifference, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

