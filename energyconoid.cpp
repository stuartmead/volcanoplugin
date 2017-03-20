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
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"


#include "ogr_spatialref.h"

#include "volcanoplugin.h"
#include "energyconoid.h"
#include "volcanoutils.h"


namespace RF
{
    /**
     * \internal
     */
    class EnergyConoidImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EnergyConoidImpl)

    public:
        EnergyConoid&  op_;

		// Data objects
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataElevationDataset_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataSlopeDataset_;
		CSIRO::DataExecution::TypedObject< double >        dataSettlingVelocity_;
		CSIRO::DataExecution::TypedObject< double >        dataFroude_;
		CSIRO::DataExecution::TypedObject< double >        dataConcentration_;
		CSIRO::DataExecution::TypedObject< double >        dataVolume_;
		CSIRO::DataExecution::TypedObject< double >        dataParticleDensity_;
		CSIRO::DataExecution::TypedObject< double >        dataAtmosphereDensity_;
		CSIRO::DataExecution::TypedObject< double >        dataXLocation_;
		CSIRO::DataExecution::TypedObject< double >        dataYLocation_;
		CSIRO::DataExecution::TypedObject< bool >		   dataAxisymmetric_;
		CSIRO::DataExecution::TypedObject< double >		   dataRotation_;
		CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataEnergyConoid_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDyPressure_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataDepositMass_;
		CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
		CSIRO::DataExecution::TypedObject< double >		   dataReducedGravity_;
		CSIRO::DataExecution::TypedObject< double >		   dataImax_;

		// Inputs and outputs
		CSIRO::DataExecution::InputScalar inputElevationDataset_;
		CSIRO::DataExecution::InputScalar inputSlopeDataset_;
		CSIRO::DataExecution::InputScalar inputSettlingVelocity_;
		CSIRO::DataExecution::InputScalar inputFroude_;
		CSIRO::DataExecution::InputScalar inputConcentration_;
		CSIRO::DataExecution::InputScalar inputVolume_;
		CSIRO::DataExecution::InputScalar inputParticleDensity_;
		CSIRO::DataExecution::InputScalar inputAtmosphereDensity_;
		CSIRO::DataExecution::InputScalar inputXLocation_;
		CSIRO::DataExecution::InputScalar inputYLocation_;
		CSIRO::DataExecution::InputScalar inputAxisymmetric_;
		CSIRO::DataExecution::InputScalar inputRotation_;
		CSIRO::DataExecution::InputScalar inputOutputRasterName_;
		CSIRO::DataExecution::Output      outputOutputRaster_;
		CSIRO::DataExecution::Output	  outputEnergyConoid_;
		CSIRO::DataExecution::Output	  outputDyPressure_;
		CSIRO::DataExecution::Output	  outputDepositMass_;
		CSIRO::DataExecution::InputScalar inputRasterBand_;
		CSIRO::DataExecution::Output	  outputReducedGravity_;
		CSIRO::DataExecution::Output	  outputImax_;

        EnergyConoidImpl(EnergyConoid& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


	/**
     *
     */
    EnergyConoidImpl::EnergyConoidImpl(EnergyConoid& op) :
		op_(op),
		dataElevationDataset_(),
		dataSlopeDataset_(),
		dataSettlingVelocity_(0.25),
		dataFroude_(1.18),
		dataConcentration_(0.015),
		dataVolume_(52.8e6),
		dataParticleDensity_(800.0),
		dataAtmosphereDensity_(1.225),
		dataXLocation_(-1.0),
		dataYLocation_(-1.0),
		dataAxisymmetric_(true),
		dataRotation_(1.0),
		dataOutputRasterName_(),
		dataOutputRaster_(),
		dataEnergyConoid_(),
		dataDyPressure_(),
		dataDepositMass_(),
		dataRasterBand_(1),
		dataReducedGravity_(),
		dataImax_(),
		inputElevationDataset_("Elevation Dataset", dataElevationDataset_, op_),
		inputSlopeDataset_("(Optional) Slope dataset", dataSlopeDataset_, op_),
		inputSettlingVelocity_("Settling velocity", dataSettlingVelocity_, op_),
		inputFroude_("Froude number", dataFroude_, op_),
		inputConcentration_("Particle concentration", dataConcentration_, op_),
		inputVolume_("Total volume", dataVolume_, op_),
		inputRotation_("Number of collapse sectors", dataRotation_, op_),
		inputParticleDensity_("Particle density", dataParticleDensity_, op_),
		inputAtmosphereDensity_("Atmosphere density", dataAtmosphereDensity_, op_),
		inputXLocation_("X location", dataXLocation_, op_),
		inputYLocation_("Y location", dataYLocation_, op_),
		inputAxisymmetric_("Axisymmetric current", dataAxisymmetric_, op_),
		inputOutputRasterName_("Output Raster name", dataOutputRasterName_, op_),
		outputOutputRaster_("Output Raster", dataOutputRaster_, op_),
		outputEnergyConoid_("Energy conoid", dataEnergyConoid_, op_),
		outputDyPressure_("Dynamic pressure", dataDyPressure_, op_),
		outputDepositMass_("Deposit mass", dataDepositMass_, op_),
		outputReducedGravity_("Reduced gravity", dataReducedGravity_, op_),
		outputImax_("Maximum runout", dataImax_, op_),
		inputRasterBand_("Raster Band", dataRasterBand_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();
		inputVolume_.setDescription(tr("Total volume of released material"));
		inputRotation_.setDescription(tr("Numer of collapse sectors radially, volume per \
					sector will be calculated as Vol/(2pi/N), where N is the number of sectors"));
		inputSlopeDataset_.setDescription(tr("Slope angle dataset for energy conoid on a slope"));
        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input1_.input_.setDescription(tr("Used for such and such."));
        // output1_.output_.setDescription(tr("Results of the blah-di-blah."));
    }


    /**
     *
     */

	float ecludianDistance2D(float X, float Y, float x, float y)
	{
		return sqrt(pow(fabs(X - x), 2) + pow(fabs(Y - y), 2));
	}

	double l_max(double phi, double fr, double A, double w_s, double g_p, bool axisymmetric = true)
	{
		if (axisymmetric)
		{
			return{
				pow((8 * sqrt(phi)) /
				(pow(fr*sqrt(g_p*pow(2 * A,3.0)),-1.0)*w_s), 1.0/4.0)
			};
		}
		else
		{
			return{
				pow((5 * sqrt(phi)) /
				(pow(fr*sqrt(g_p*pow(A,3.0)),-1.0)*w_s), 2.0 / 5.0)
			};
		}
	}

	double h_max_cyl(double grav, double C, double linf, double x) //Cylindrical h_max
	{
		return{
			(1 / (2 * grav))*
			pow(0.5*C*pow(linf, 1.0 / 3.0)*(sqrt(1 - pow(x,4.0)) / x)
				,2.0)
		};
	}
	
    bool EnergyConoidImpl::execute()
    {
		GDALDatasetH& elevationDataset = *dataElevationDataset_;
		QString&      outputRasterName = *dataOutputRasterName_;
		GDALDatasetH& outputRaster = *dataOutputRaster_;
		int&          rasterBand = *dataRasterBand_;

		GDALAllRegister();
		
		float dstNodataValue;
		float * elevation = getRasterData(elevationDataset, dstNodataValue);

		float * slope;
		if (inputSlopeDataset_.connected())
		{
			slope = getRasterData(*dataSlopeDataset_, dstNodataValue);
		}
		
		/*START - Get constants for energy conoid model (Imax, C, gp', \lambda)
		From Esposti Ongaro et al. (2016) 'A fast, calibrated model for pyroclastic 
			density current kinematics and hazard' JVGR 327, pp. 257-272

		gp' = [(rho_p - rho_a)/rho_a]*g
		C = ws^1/3*Fr^2/3*phi^1/3*gp'^1/3
		*/

		//gp', the reduced gravity in the current, controlled by concentration and particle density
		//Eq. 3 in Esposito Ongaro
		//NOTE: In Esposti Ongaro, gp' is defined as g'= phi[(rho_p - rho_a)/rho_a]g = phi*gp'
		//Dividing by phi, you get gp = [(rho_p - rho_a)/rho_a]*g, the same as Hallworth et al. (1998)
		double gravity = 9.807;
		double gp = ((*dataParticleDensity_ - *dataAtmosphereDensity_) / *dataAtmosphereDensity_)*gravity;

		*dataReducedGravity_ = gp;


		double A = *dataVolume_ / ((2 * M_PI) / *dataRotation_);

		//C, a decay constant
		//Eq. 13 in Esposito Ongaro	
		double C = pow(*dataSettlingVelocity_, 1.0 / 3.0)*pow(*dataFroude_, 2.0 / 3.0)*pow(*dataConcentration_, 1.0 / 3.0)*pow(gp, 1.0 / 3.0);

		//Calculate lmax
		double lmax = l_max(*dataConcentration_, *dataFroude_, A, *dataSettlingVelocity_, gp, *dataAxisymmetric_);

		std::cout << QString("Maximum runout is %1 metres.").arg(lmax) + "\n";

		*dataImax_ = lmax;
							
		/*
		STOP - Get constants for energy conoid model
		*/

		/*
		Loop through cells in the raster, calculating hmax (eq. 12 in Esposito Ongaro)
		*/
		float * energyConoid;
		energyConoid = new float[GDALGetRasterXSize(elevationDataset)*GDALGetRasterYSize(elevationDataset)];
		float * elevDiff;
		elevDiff = new float[GDALGetRasterXSize(elevationDataset)*GDALGetRasterYSize(elevationDataset)];
		float * dyPressure;
		dyPressure = new float[GDALGetRasterXSize(elevationDataset)*GDALGetRasterYSize(elevationDataset)];
		float * depositMass;
		depositMass = new float[GDALGetRasterXSize(elevationDataset)*GDALGetRasterYSize(elevationDataset)];

		//Get geotransform to convert from cellspace (P,L) to geographical space
		//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2]; 
		//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
		double transform[6];
		GDALGetGeoTransform(elevationDataset, transform);

		//Get elevation of the pixel
		//Get the inverse geotransform
		double invTransform[6];
		GDALInvGeoTransform(transform, invTransform);
		int pixelX = (int)floor(invTransform[0] + invTransform[1] * *dataXLocation_ + invTransform[2] * *dataYLocation_);
		int pixelY = (int)floor(invTransform[3] + invTransform[4] * *dataXLocation_ + invTransform[5] * *dataYLocation_);
		
		float * pixElev = getRasterData(elevationDataset, dstNodataValue, pixelX, pixelY,
			1, 1);

		std::cout << QString("Elevation at initiation point is %1 metres.").arg(*pixElev) + "\n";

		for (int i = 0; i < GDALGetRasterXSize(elevationDataset)*GDALGetRasterYSize(elevationDataset); ++i)
		{
			int L = floorf(i / GDALGetRasterXSize(elevationDataset));
			int P = i - (L*GDALGetRasterXSize(elevationDataset));

			double xl = transform[0] + P*transform[1] + L*transform[2];
			double yl = transform[3] + P*transform[4] + L*transform[5];

			//Distance (x/lmax)
			double dist = ecludianDistance2D(*dataXLocation_, *dataYLocation_, xl, yl) / lmax;
			
			if (inputSlopeDataset_.connected())
			{
				gp = ((*dataParticleDensity_ - *dataAtmosphereDensity_) / *dataAtmosphereDensity_)*(gravity*cos(slope[i]*DEG2RAD));
				C = pow(*dataSettlingVelocity_, 1.0 / 3.0)*pow(*dataFroude_, 2.0 / 3.0)*pow(*dataConcentration_, 1.0 / 3.0)*pow(gp, 1.0 / 3.0);

				//Calculate lmax
				lmax = l_max(*dataConcentration_, *dataFroude_, A, *dataSettlingVelocity_, gp, *dataAxisymmetric_);

				energyConoid[i] = h_max_cyl(gravity*cos(slope[i]*DEG2RAD), C, lmax, dist);
			}
			else {
				energyConoid[i] = h_max_cyl(gravity, C, lmax, dist);
			}
			elevDiff[i] = std::max((float) 0.0, energyConoid[i] + *pixElev - elevation[i]);
			//Dynamic Pressure (Eq. 10 Esposti Ongaro)
			//((*dataConcentration_* *dataParticleDensity_)+((1-*dataConcentration_) * *dataAtmosphereDensity_))*gravity*std::max((float) 0.0, energyConoid[i]);
			dyPressure[i] = (std::max((float) 0.0, energyConoid[i]) * 2 * gravity) * 0.5 * ((*dataConcentration_* *dataParticleDensity_) +
				((1 - *dataConcentration_) * *dataAtmosphereDensity_));

			//Deposit mass for axisymmetric currents, eq. B.10 Esposti Ongaro
			double lambda_c = *dataSettlingVelocity_ / (*dataFroude_*sqrt(pow(2 * A, 3.0)*gp));
			depositMass[i] = pow(sqrt(*dataConcentration_) - (0.125*lambda_c*pow(std::min(dist, 1.0)*lmax, 4.0))
				, 2.0);

		}
		
		outputRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
			outputRasterName.toLocal8Bit().constData(),
			GDALGetRasterXSize(elevationDataset),
			GDALGetRasterYSize(elevationDataset),
			1,
			GDT_Float32,
			NULL);

		GDALDatasetH& massRaster = *dataDepositMass_;
		massRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
			"deposit_mass",
			GDALGetRasterXSize(elevationDataset),
			GDALGetRasterYSize(elevationDataset),
			1,
			GDT_Float32,
			NULL);

		GDALDatasetH& conoidRaster = *dataEnergyConoid_;
		conoidRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
			"energy_conoid",
			GDALGetRasterXSize(elevationDataset),
			GDALGetRasterYSize(elevationDataset),
			1,
			GDT_Float32,
			NULL);

		GDALDatasetH& dynamicPressureRaster = *dataDyPressure_;

		dynamicPressureRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
			"dynamic_pressure",
			GDALGetRasterXSize(elevationDataset),
			GDALGetRasterYSize(elevationDataset),
			1,
			GDT_Float32,
			NULL);

		GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
		GDALSetGeoTransform(outputRaster, transform);

		GDALRasterBandH conoidDestBand = GDALGetRasterBand(conoidRaster, 1);
		GDALSetGeoTransform(conoidRaster, transform);

		GDALRasterBandH dyPressureBand = GDALGetRasterBand(dynamicPressureRaster, 1);
		GDALSetGeoTransform(dynamicPressureRaster, transform);

		GDALRasterBandH depositMassBand = GDALGetRasterBand(massRaster, 1);
		GDALSetGeoTransform(massRaster, transform);
		GDALSetProjection(massRaster, GDALGetProjectionRef(elevationDataset));

		if (GDALSetProjection(outputRaster, GDALGetProjectionRef(elevationDataset)) != CE_None)
		{
			std::cout << QString("WARNING: Output projection cannot be set, setting to WGS84") + "\n";
			OGRSpatialReferenceH hSRS;
			hSRS = OSRNewSpatialReference(NULL);
			OSRSetWellKnownGeogCS(hSRS, "WGS84");
			char *gsR = NULL;
			OSRExportToWkt(hSRS, &gsR);
			if (GDALSetProjection(outputRaster, gsR) != CE_None)
			{
				std::cout << QString("ERROR: Could not set projection to WGS84") + "\n";
				return false;
			}
		}

		if (GDALSetProjection(conoidRaster, GDALGetProjectionRef(elevationDataset)) != CE_None)
		{
			std::cout << QString("WARNING: Output projection cannot be set, setting to WGS84") + "\n";
			OGRSpatialReferenceH hSRS;
			hSRS = OSRNewSpatialReference(NULL);
			OSRSetWellKnownGeogCS(hSRS, "WGS84");
			char *gsR = NULL;
			OSRExportToWkt(hSRS, &gsR);
			if (GDALSetProjection(conoidRaster, gsR) != CE_None)
			{
				std::cout << QString("ERROR: Could not set projection to WGS84") + "\n";
				return false;
			}
		}

		if (GDALSetProjection(dynamicPressureRaster, GDALGetProjectionRef(elevationDataset)) != CE_None)
		{
			std::cout << QString("WARNING: Output projection cannot be set, setting to WGS84") + "\n";
			OGRSpatialReferenceH hSRS;
			hSRS = OSRNewSpatialReference(NULL);
			OSRSetWellKnownGeogCS(hSRS, "WGS84");
			char *gsR = NULL;
			OSRExportToWkt(hSRS, &gsR);
			if (GDALSetProjection(dynamicPressureRaster, gsR) != CE_None)
			{
				std::cout << QString("ERROR: Could not set projection to WGS84") + "\n";
				return false;
			}
		}


		GDALSetRasterNoDataValue(destBand, dstNodataValue);
		GDALSetRasterNoDataValue(conoidDestBand, dstNodataValue);
		GDALSetRasterNoDataValue(dyPressureBand, dstNodataValue);
		GDALSetRasterNoDataValue(depositMassBand, dstNodataValue);

		GDALRasterIO(depositMassBand, GF_Write,
			0, 0,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset),
			depositMass,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset),
			GDT_Float32,
			0, 0);

		GDALRasterIO(destBand, GF_Write,
			0, 0,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset),
			elevDiff,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset), 
			GDT_Float32,
			0, 0);

		GDALRasterIO(conoidDestBand, GF_Write,
			0, 0,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset), 
			energyConoid,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset), 
			GDT_Float32,
			0, 0);

		GDALRasterIO(dyPressureBand, GF_Write,
			0, 0,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset),
			dyPressure,
			GDALGetRasterXSize(elevationDataset), GDALGetRasterYSize(elevationDataset),
			GDT_Float32,
			0, 0);
 
        return true;
    }


    /**
     *
     */
    EnergyConoid::EnergyConoid() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< EnergyConoid >::getInstance(),
            tr("Box model energy cone"))
    {
        pImpl_ = new EnergyConoidImpl(*this);
    }


    /**
     *
     */
    EnergyConoid::~EnergyConoid()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  EnergyConoid::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(EnergyConoid, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

