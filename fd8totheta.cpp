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
#include <map>


#include <qstring.h>



#include "Workspace/Application/LanguageUtils/streamqstring.h"

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"


#include "volcanoplugin.h"
#include "volcanoplugin.h"
#include "fd8totheta.h"


namespace RF
{
    /**
     * \internal
     */
    class FD8toThetaImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FD8toThetaImpl)

    public:
        FD8toTheta&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFD8Dataset_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< QString >  dataDestinationFileName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataAngleDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputFD8Dataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputDestinationFileName_;
        CSIRO::DataExecution::Output      outputAngleDataset_;


        FD8toThetaImpl(FD8toTheta& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    FD8toThetaImpl::FD8toThetaImpl(FD8toTheta& op) :
        op_(op),
        dataFD8Dataset_(),
        dataRasterBand_(1),
        dataDestinationFileName_(),
        dataAngleDataset_(),
        inputFD8Dataset_("FD8 dataset", dataFD8Dataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputDestinationFileName_("Destination file name", dataDestinationFileName_, op_),
        outputAngleDataset_("Angle dataset", dataAngleDataset_, op_)
    {
    }


    /**
     *
     */
    bool FD8toThetaImpl::execute()
    {
        GDALDatasetH& fD8Dataset   = *dataFD8Dataset_;
        GDALDatasetH& angleDataset = *dataAngleDataset_;
        int&          rasterBand          = *dataRasterBand_;
        QString& destinationFileName = *dataDestinationFileName_;
        
                //Get raster input band and transform
        GDALAllRegister();

        GDALRasterBandH hBand;
        if(rasterBand > GDALGetRasterCount(fD8Dataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(fD8Dataset)).arg(rasterBand) + "\n";
            return false;
        }
        hBand = GDALGetRasterBand(fD8Dataset, rasterBand);

        double transform[6];
        GDALGetGeoTransform(fD8Dataset,transform);

        //Setup
        int srcNoData;
        float dstNoDataValue;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);

        int * fD8Data;
        fD8Data = new int [GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

        GDALRasterIO( hBand, GF_Read,
            0,0,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            fD8Data,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            GDT_Int32,
            0,0);

        //Create output raster
        float * thetaData;
        thetaData = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

        //Create a map
        std::map<int, float> directionMap;
        directionMap[1] = 0;
        directionMap[2] = 7*M_PI_4;
        directionMap[4] = 3*M_PI_2;
        directionMap[8] = 5*M_PI_4;
        directionMap[16] = M_PI;
        directionMap[32] = 3*M_PI_4;
        directionMap[64] = M_PI_2;
        directionMap[128] = M_PI_4;


        for (int it = 0; it < GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand); ++it)
        {
            if (fD8Data[it] == 1 || fD8Data[it] == 2 || fD8Data[it] == 4 || fD8Data[it] == 8 || fD8Data[it] == 16 || fD8Data[it] == 32 || fD8Data[it] == 64 || fD8Data[it] == 128)
            {
                thetaData[it] = directionMap[fD8Data[it]];
            }
            else
            {
                thetaData[it] = dstNoDataValue;
            }
        }

        angleDataset = GDALCreate( GDALGetDatasetDriver(fD8Dataset),
                                destinationFileName.toLocal8Bit().constData(),
                                GDALGetRasterXSize(fD8Dataset), GDALGetRasterYSize(fD8Dataset),
                                1,
                                GDT_Float32, NULL);

        GDALRasterBandH destBand = GDALGetRasterBand(angleDataset, 1);
        GDALSetGeoTransform(angleDataset, transform);
        GDALSetProjection(angleDataset, GDALGetProjectionRef(fD8Dataset));
        GDALSetRasterNoDataValue(destBand, dstNoDataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        thetaData,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);


        return true;
    }


    /**
     *
     */
    FD8toTheta::FD8toTheta() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< FD8toTheta >::getInstance(),
            tr("Convert FD8 directions to radians"))
    {
        pImpl_ = new FD8toThetaImpl(*this);
    }


    /**
     *
     */
    FD8toTheta::~FD8toTheta()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  FD8toTheta::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(FD8toTheta, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

