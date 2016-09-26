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
#include <qvector.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "erosionutils.h"
#include "volcanoplugin.h"
#include "iversonfailurevolume.h"


namespace RF
{
    /**
     * \internal
     */
    class IversonFailureVolumeImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::IversonFailureVolumeImpl)

    public:
        IversonFailureVolume&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >   dataSlopeAngleDataset_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >   dataDepthOfDeposit_;
        CSIRO::DataExecution::TypedObject< int >            dataDepthIncrement_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >   dataWaterTableDepth_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataWaterInflux_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataFrictionAngle_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataCohesion_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataSaturatedSoilWeight_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataWaterWeight_;
        CSIRO::DataExecution::TypedObject< double >        dataHydraulicConductivity_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataHydralicDiffusivity_;//Dataset
        CSIRO::DataExecution::TypedObject< double >        dataRainfallIntensity_;
        CSIRO::DataExecution::TypedObject< double >        dataRainfallDuration_;
        CSIRO::DataExecution::TypedObject< double >        dataTotalTime_;
        CSIRO::DataExecution::TypedObject< QString >       dataFailureName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFailureDepth_;
        CSIRO::DataExecution::TypedObject< QVector<double> >  dataXVec_;
        CSIRO::DataExecution::TypedObject< QVector<double> >  dataYVec_;



        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputSlopeAngleDataset_;
        CSIRO::DataExecution::InputScalar inputDepthOfDeposit_;
        CSIRO::DataExecution::InputScalar inputDepthIncrement_;
        CSIRO::DataExecution::InputScalar inputWaterTableDepth_;
        CSIRO::DataExecution::InputScalar inputWaterInflux_;
        CSIRO::DataExecution::InputScalar inputFrictionAngle_;
        CSIRO::DataExecution::InputScalar inputCohesion_;
        CSIRO::DataExecution::InputScalar inputSaturatedSoilWeight_;
        CSIRO::DataExecution::InputScalar inputWaterWeight_;
        CSIRO::DataExecution::InputScalar inputHydraulicConductivity_;
        CSIRO::DataExecution::InputScalar inputHydralicDiffusivity_;
        CSIRO::DataExecution::InputScalar inputRainfallIntensity_;
        CSIRO::DataExecution::InputScalar inputRainfallDuration_;
        CSIRO::DataExecution::InputScalar inputTotalTime_;
        CSIRO::DataExecution::InputScalar inputFailureName_;
        CSIRO::DataExecution::Output      outputFailureDepth_;
        CSIRO::DataExecution::Output      outputXVec_;
        CSIRO::DataExecution::Output      outputYVec_;


        IversonFailureVolumeImpl(IversonFailureVolume& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    IversonFailureVolumeImpl::IversonFailureVolumeImpl(IversonFailureVolume& op) :
        op_(op),
        dataSlopeAngleDataset_(),
        dataDepthOfDeposit_(),
        dataDepthIncrement_(20),
        dataWaterTableDepth_(),
        dataWaterInflux_(),
        dataFrictionAngle_(),
        dataCohesion_(),
        dataSaturatedSoilWeight_(),
        dataWaterWeight_(),
        dataHydraulicConductivity_(),
        dataHydralicDiffusivity_(),
        dataRainfallIntensity_(),
        dataRainfallDuration_(),
        dataTotalTime_(),
        dataFailureName_(),
        dataFailureDepth_(),
        dataXVec_(),
        dataYVec_(),
        inputSlopeAngleDataset_("Slope angle dataset", dataSlopeAngleDataset_, op_),
        inputDepthOfDeposit_("Depth of deposit", dataDepthOfDeposit_, op_),
        inputDepthIncrement_("Number of depth increments", dataDepthIncrement_, op_),
        inputWaterTableDepth_("Water table depth", dataWaterTableDepth_, op_),
        inputWaterInflux_("Steady state water influx", dataWaterInflux_, op_),
        inputFrictionAngle_("Friction angle", dataFrictionAngle_, op_),
        inputCohesion_("Cohesion", dataCohesion_, op_),
        inputSaturatedSoilWeight_("Saturated soil weight", dataSaturatedSoilWeight_, op_),
        inputWaterWeight_("Water weight", dataWaterWeight_, op_),
        inputHydraulicConductivity_("Hydraulic Conductivity", dataHydraulicConductivity_, op_),
        inputHydralicDiffusivity_("Hydralic Diffusivity", dataHydralicDiffusivity_, op_),
        inputRainfallIntensity_("Rainfall Intensity", dataRainfallIntensity_, op_),
        inputRainfallDuration_("Rainfall Duration", dataRainfallDuration_, op_),
        inputTotalTime_("Total time", dataTotalTime_, op_),
        inputFailureName_("Output raster name", dataFailureName_, op_),
        outputFailureDepth_("Failure depth", dataFailureDepth_, op_),
        outputXVec_("X Vector", dataXVec_, op_),
        outputYVec_("Y Vector", dataYVec_, op_)
    {
    }


    /**
     *
     */
    
    double degreesToRadians(double theta)
    {
        return theta * (M_PI/180);
    }

    bool IversonFailureVolumeImpl::execute()
    {
        GDALDatasetH& slopeAngleDataset     = *dataSlopeAngleDataset_;
        GDALDatasetH& depthOfDeposit        = *dataDepthOfDeposit_;
        int&          zIncrements           = *dataDepthIncrement_;
        GDALDatasetH& waterTableDepth       = *dataWaterTableDepth_;
        double&       waterInflux           = *dataWaterInflux_;
        double&       frictionAngle         = *dataFrictionAngle_;
        double&       cohesion              = *dataCohesion_;
        double&       saturatedSoilWeight   = *dataSaturatedSoilWeight_;
        double&       waterWeight           = *dataWaterWeight_;
        double&       hydraulicConductivity = *dataHydraulicConductivity_;
        double&       hydralicDiffusivity   = *dataHydralicDiffusivity_;
        double&       rainfallIntensity     = *dataRainfallIntensity_;
        double&       rainfallDuration      = *dataRainfallDuration_;
        double&       totalTime             = *dataTotalTime_;
        GDALDatasetH& failureDepth          = *dataFailureDepth_;
        QVector<double>& xVec                = *dataXVec_;
        QVector<double>& yVec                = *dataYVec_;
        
        dataXVec_->clear();
        dataYVec_->clear();

        GDALAllRegister();

        //Read the slope angle dataset
        GDALRasterBandH slopeBand;
        
        if (1 > GDALGetRasterCount(slopeAngleDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(slopeAngleDataset)).arg(1) + "\n";
            return false;
        }

        slopeBand = GDALGetRasterBand(slopeAngleDataset, 1);

        double transform[6];
        GDALGetGeoTransform(slopeAngleDataset, transform);

        int srcNoData;
        float dstNoDataValue;

        dstNoDataValue = (float) GDALGetRasterNoDataValue(slopeBand, &srcNoData);

        float * angleData;
        angleData = new float [GDALGetRasterBandXSize(slopeBand)*GDALGetRasterBandYSize(slopeBand)];

        GDALRasterIO(slopeBand, GF_Read,
                    0, 0,
                    GDALGetRasterBandXSize(slopeBand), GDALGetRasterBandYSize(slopeBand),
                    angleData,
                    GDALGetRasterBandXSize(slopeBand), GDALGetRasterBandYSize(slopeBand),
                    GDT_Float32,
                    0,0);

        //Read the depth dataset

        GDALRasterBandH depthBand;
        
        if (1 > GDALGetRasterCount(depthOfDeposit))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(depthOfDeposit)).arg(1) + "\n";
            return false;
        }

        depthBand = GDALGetRasterBand(depthOfDeposit, 1);

        float * depthData;
        depthData = new float [GDALGetRasterBandXSize(depthBand)*GDALGetRasterBandYSize(depthBand)];

        GDALRasterIO(depthBand, GF_Read,
                    0, 0,
                    GDALGetRasterBandXSize(depthBand), GDALGetRasterBandYSize(depthBand),
                    depthData,
                    GDALGetRasterBandXSize(depthBand), GDALGetRasterBandYSize(depthBand),
                    GDT_Float32,
                    0,0);

        float depthNodataValue;
        depthNodataValue = (float) GDALGetRasterNoDataValue(depthBand, &srcNoData);
        

        //Read the water table depth dataset

        GDALRasterBandH waterTableBand;

        if (1 > GDALGetRasterCount(waterTableDepth))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(waterTableDepth)).arg(1) + "\n";
            return false;
        }

        waterTableBand = GDALGetRasterBand(waterTableDepth, 1);

        float * wtDepth;
        wtDepth = new float [GDALGetRasterBandXSize(waterTableBand)*GDALGetRasterBandYSize(waterTableBand)];

        GDALRasterIO(waterTableBand, GF_Read,
            0, 0,
            GDALGetRasterBandXSize(waterTableBand), GDALGetRasterBandYSize(waterTableBand),
            wtDepth,
            GDALGetRasterBandXSize(waterTableBand), GDALGetRasterBandYSize(waterTableBand),
            GDT_Float32,
            0,0);

        float wtNodataValue;

        wtNodataValue = (float) GDALGetRasterNoDataValue(waterTableBand, &srcNoData);

        double slopeAngleRad, frictAngleRad,frictionFOS,cohesionFOS,porePressureFOS, unsteadyFOS, depositThickness, waterTable;
        double runOff = 0;

        /**********************************
        Test Response
        ***********************************/
        /*
        for (int i = 0; i < 10000; ++i)
        {
            double t = i*0.1;
            if (t <= 10.0)
            {
                yVec.push_back(response(t));
            }
            else
            {
                yVec.push_back(response(t)-response(t-10.0));
            }
            xVec.push_back(t);
            if (i%1000 == 0)
            {
                std::cout << QString("Response at i %1 t %2 is %3").arg(i).arg(t).arg(response(t)-response(t-0.1)) + "\n";
            }
        }
        */
               


        /*************************************
        Test FOS
        *************************************/
        /*
        double Ff = tan(degreesToRadians(frictionAngle))/tan(degreesToRadians(45));
        //std::cout << QString("Friction FOS is %1").arg(Ff) + "\n";
        double izKsat;
        if (rainfallIntensity > hydraulicConductivity)
        {
            izKsat = 1;
            runOff += rainfallIntensity - hydraulicConductivity;
        } else
        {
            izKsat = rainfallIntensity/hydraulicConductivity;
        }
        double D_eff = 4*hydralicDiffusivity*pow(cos(degreesToRadians(45)),2);
        //std::cout << QString("Deff is %1").arg(D_eff) + "\n";
        for (int i = 0; i < Z.size(); ++i)
        {
            double Fc = cohesion/(saturatedSoilWeight*Z[i]*sin(degreesToRadians(45))*cos(degreesToRadians(45)));
          //  std::cout << QString("Cohesion FOS is %1").arg(Fc) + "\n";
            double backgroundHead = pow(cos(degreesToRadians(45)),2)*(1-(waterTableDepth/Z[i]));
            double Fw = (backgroundHead*Z[i]*waterWeight*tan(degreesToRadians(frictionAngle)))/(saturatedSoilWeight*Z[i]*sin(degreesToRadians(45))*cos(degreesToRadians(45)));
            //std::cout << QString("FW FOS is %1").arg(Fw) + "\n";
            double unsteadyFOS = determineUnsteadyFOS(totalTime, rainfallDuration, waterWeight, saturatedSoilWeight, degreesToRadians(frictionAngle), degreesToRadians(45), izKsat, Z[i], D_eff);
            //std::cout << QString("Unsteady FOS is %1").arg(unsteadyFOS) + "\n";
            xVec.push_back(Ff+Fc-Fw+unsteadyFOS);
            yVec.push_back(Z[i]);
        }
        */
        ////std::vector<std::vector<double>> fosVector;

        float * failureDepthData;
        failureDepthData = new float [GDALGetRasterBandXSize(slopeBand)*GDALGetRasterBandYSize(slopeBand)];

        for (int i = 0; i < GDALGetRasterBandXSize(slopeBand)*GDALGetRasterBandYSize(slopeBand); ++i)
        {
            if (angleData[i] == dstNoDataValue || depthData[i] == depthNodataValue || wtDepth[i] == wtNodataValue)
            {
                failureDepthData[i] = dstNoDataValue;
            }
            else
            {
                slopeAngleRad = degreesToRadians(angleData[i]);
                depositThickness = depthData[i];
                waterTable = wtDepth[i];
                frictAngleRad = degreesToRadians(frictionAngle);
                frictionFOS = tan(frictAngleRad)/tan(slopeAngleRad);
                double D_eff = 4*hydralicDiffusivity*pow(cos(slopeAngleRad),2);
                //Determine izKz
                double izKsat;
                if (rainfallIntensity > hydraulicConductivity)
                {
                    izKsat = 1;
                    runOff += rainfallIntensity - hydraulicConductivity;
                } else
                {
                    izKsat = rainfallIntensity/hydraulicConductivity;
                }
                //Initialise depth increment
                std::vector<double> Z;
                double inc = (1/(float)zIncrements)*depositThickness;
                for (int zLev = 0; zLev < zIncrements; ++zLev)
                {
                    Z.push_back(zLev * inc);
                }
                
                /*if (i % 100000 == 0)
                {
                    std::cout << QString("Depth is %1").arg( depthData[i]) + "\n";
                    std::cout << QString("Increment is %1, zlev[0] is %2").arg(inc).arg(Z[0]) + "\n";
                    std::cout << QString("Calculated as 1/%1 * %2").arg(zIncrements).arg(depositThickness) + "\n";
                }*/

                std::vector<double> fosCell;
                float failure = -1.0f; //This is very bad
                for (int zCalc = 0; zCalc < Z.size(); ++zCalc)
                {
                    double depth = Z[zCalc];
                    double backgroundHead = pow(cos(slopeAngleRad),2)*(1-(waterTable/depth));
                    cohesionFOS = cohesion/(saturatedSoilWeight*depth*sin(slopeAngleRad)*cos(slopeAngleRad));
                    porePressureFOS = (backgroundHead*depth*waterWeight*tan(frictAngleRad))/(saturatedSoilWeight*depth*sin(slopeAngleRad)*cos(slopeAngleRad));
                    unsteadyFOS = determineUnsteadyFOS(totalTime, rainfallDuration, waterWeight, saturatedSoilWeight, frictAngleRad, slopeAngleRad, izKsat, depth, D_eff);
                    double totalFOS = frictionFOS+cohesionFOS-porePressureFOS+unsteadyFOS;
                    /*if (i % 100000 == 0)
                    {
                        std::cout << QString("Background head is %1").arg( backgroundHead) + "\n";
                        std::cout << QString("Friction FOS is %1").arg(frictionFOS) + "\n";
                        std::cout << QString("Cohesion FOS is %1").arg(cohesionFOS) + "\n";
                        std::cout << QString("Pore FOS is %1").arg(porePressureFOS) + "\n";
                        std::cout << QString("Unsteady FOS is %1").arg(unsteadyFOS) + "\n";
                    }*/

                    if (totalFOS < 1)
                    {
                        if (depth > failure)
                        {
                            failure = depth;
                        }
                    }
                    fosCell.push_back(frictionFOS+cohesionFOS+porePressureFOS+unsteadyFOS);
                }
                failureDepthData[i] = failure;
				/*
				if (i % 100000 == 0)
                {
                    std::cout << QString("Depth is %1").arg( depthData[i]) + "\n";
                    std::cout << QString("Failure depth is %1").arg(failureDepthData[i]) + "\n";
                }
				*/
            }
        }

        //Write failure depth

        failureDepth = GDALCreate(GDALGetDatasetDriver(slopeAngleDataset),
                                    dataFailureName_->toLocal8Bit().constData(),
                                    GDALGetRasterXSize(slopeAngleDataset),
                                    GDALGetRasterYSize(slopeAngleDataset),
                                    1,
                                    GDT_Float32,
                                    NULL);
        GDALRasterBandH destBand = GDALGetRasterBand(failureDepth, 1);
        GDALSetGeoTransform(failureDepth, transform);
        GDALSetProjection(failureDepth, GDALGetProjectionRef(slopeAngleDataset));

        GDALSetRasterNoDataValue(destBand, dstNoDataValue);

        GDALRasterIO(destBand, GF_Write,
                0,0,
                GDALGetRasterBandXSize(slopeBand), GDALGetRasterBandYSize(slopeBand),
                failureDepthData,
                GDALGetRasterBandXSize(slopeBand), GDALGetRasterBandYSize(slopeBand),
                GDT_Float32,
                0,0);
 

        return true;
    }


    /**
     *
     */
    IversonFailureVolume::IversonFailureVolume() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< IversonFailureVolume >::getInstance(),
            tr("Calculate failure depth"))
    {
        pImpl_ = new IversonFailureVolumeImpl(*this);
    }


    /**
     *
     */
    IversonFailureVolume::~IversonFailureVolume()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  IversonFailureVolume::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(IversonFailureVolume, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

