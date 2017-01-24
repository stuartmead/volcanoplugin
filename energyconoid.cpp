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
		CSIRO::DataExecution::TypedObject< double >        dataSettlingVelocity_;
		CSIRO::DataExecution::TypedObject< double >        dataFroude_;
		CSIRO::DataExecution::TypedObject< double >        dataConcentration_;
		CSIRO::DataExecution::TypedObject< double >        dataVolume_;
		CSIRO::DataExecution::TypedObject< double >        dataParticleDensity_;
		CSIRO::DataExecution::TypedObject< double >        dataAtmosphereDensity_;
		CSIRO::DataExecution::TypedObject< double >        dataXLocation_;
		CSIRO::DataExecution::TypedObject< double >        dataYLocation_;
		CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
		CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputRaster_;
		CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;


		// Inputs and outputs
		CSIRO::DataExecution::InputScalar inputElevationDataset_;
		CSIRO::DataExecution::InputScalar inputSettlingVelocity_;
		CSIRO::DataExecution::InputScalar inputFroude_;
		CSIRO::DataExecution::InputScalar inputConcentration_;
		CSIRO::DataExecution::InputScalar inputVolume_;
		CSIRO::DataExecution::InputScalar inputParticleDensity_;
		CSIRO::DataExecution::InputScalar inputAtmosphereDensity_;
		CSIRO::DataExecution::InputScalar inputXLocation_;
		CSIRO::DataExecution::InputScalar inputYLocation_;
		CSIRO::DataExecution::InputScalar inputOutputRasterName_;
		CSIRO::DataExecution::Output      outputOutputRaster_;
		CSIRO::DataExecution::InputScalar inputRasterBand_;

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
		dataSettlingVelocity_(0.25),
		dataFroude_(1.18),
		dataConcentration_(0.015),
		dataVolume_(52.8e6),
		dataParticleDensity_(800.0),
		dataAtmosphereDensity_(1.225),
		dataXLocation_(-1.0),
		dataYLocation_(-1.0),
		dataOutputRasterName_(),
		dataOutputRaster_(),
		dataRasterBand_(1),
		inputElevationDataset_("Elevation Dataset", dataElevationDataset_, op_),
		inputSettlingVelocity_("Settling velocity", dataSettlingVelocity_, op_),
		inputFroude_("Froude number", dataFroude_, op_),
		inputConcentration_("Particle concentration", dataConcentration_, op_),
		inputVolume_("Total volume", dataVolume_, op_),
		inputParticleDensity_("Particle density", dataParticleDensity_, op_),
		inputAtmosphereDensity_("Atmosphere density", dataAtmosphereDensity_, op_),
		inputXLocation_("X location", dataXLocation_, op_),
		inputYLocation_("Y location", dataYLocation_, op_),
		inputOutputRasterName_("Output Raster name", dataOutputRasterName_, op_),
		outputOutputRaster_("Output Raster", dataOutputRaster_, op_),
		inputRasterBand_("Raster Band", dataRasterBand_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

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
	
    bool EnergyConoidImpl::execute()
    {
		GDALDatasetH& elevationDataset = *dataElevationDataset_;
		QString&      outputRasterName = *dataOutputRasterName_;
		GDALDatasetH& outputRaster = *dataOutputRaster_;
		int&          rasterBand = *dataRasterBand_;

		GDALAllRegister();

		GDALRasterBandH hBand;

		if (rasterBand > GDALGetRasterCount(elevationDataset))
		{
			std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(elevationDataset)).arg(rasterBand) + "\n";
		}

		hBand = GDALGetRasterBand(elevationDataset, rasterBand);

		int srcNodata;
		float dstNodataValue;

		dstNodataValue = (float)GDALGetRasterNoDataValue(hBand, &srcNodata);
		std::cout << QString("Input nodata value for energy cone is %1").arg(dstNodataValue) + "\n";

		float * elevation;
		elevation = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

		GDALRasterIO(hBand, GF_Read,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			elevation,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			GDT_Float32,
			0, 0);

		/*
		START - Get constants for energy conoid model (Imax, C, gp', \lambda)
		From Esposti Ongaro et al. (2016) 'A fast, calibrated model for pyroclastic 
			density current kinematics and hazard' JVGR 327, pp. 257-272

		gp' = [(rho_p - rho_a)/rho_a]*g
		lambda = w_s/Fr(A^3gp')^1/2
		Imax = (5*phi^1/2/lambda)^2/5
		C = ws^1/3*Fr^2/3*phi^1/3*gp'^1/3
		*/

		//gp', the reduced gravity in the current, controlled by concentration and particle density
		//Eq. 3 in Esposito Ongaro
		//NOTE: In Esposti Ongaro, gp' is defined as g'= phi[(rho_p - rho_a)/rho_a]g = phi*gp'
		//Dividing by phi, you get gp = [(rho_p - rho_a)/rho_a]*g, the same as Hallworth et al. (1998)
		double gravity = 9.807;
		double gp = ((*dataParticleDensity_ - *dataAtmosphereDensity_) / *dataAtmosphereDensity_);

		//lambda, a constant relating the reduction in particle concentration to distance
		//Eq. 5 in Esposito Ongaro
		//Need to use 2/3 * vol - essentially assuming a square
		//double lambda = *dataSettlingVelocity_ / (*dataFroude_ * sqrt(pow(*dataVolume_, 2.0 / 3.0)*gp));

		//Imax, the current runout in Cylindrical co-ords
		//Eq. 6 in Esposito Ongaro (cartesian) double Imax = pow((5 * sqrt(*dataConcentration_)) / lambda, 2.0 / 5.0);
		//Using Neri's
		double Imax = pow(8 * sqrt(*dataConcentration_)* *dataFroude_ * sqrt(gp) * pow(*dataVolume_, 3.0 / 2.0)* pow(*dataSettlingVelocity_, -1.0), 1.0 / 4.0);

		std::cout << QString("Maximum runout is %1 metres.").arg(Imax) + "\n";

		//C, a decay constant
		//Eq. 13 in Esposito Ongaro	double C = pow(*dataSettlingVelocity_, 1.0 / 3.0)*pow(*dataFroude_, 2.0 / 3.0)*pow(*dataConcentration_, 1.0 / 3.0)*pow(gp, 1.0 / 3.0);
		//Using Neri's 
		double C = pow(*dataFroude_ * *dataFroude_ * *dataSettlingVelocity_ * *dataConcentration_ * gp, 1.0 / 3.0) / 2;

		
		/*
		STOP - Get constants for energy conoid model
		*/

		/*
		Loop through cells in the raster, calculating hmax (eq. 12 in Esposito Ongaro)
		*/
		float * energyConoid;
		energyConoid = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

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
		float pixElev;

		GDALRasterIO(hBand, GF_Read,
			pixelX, pixelY,
			1, 1,
			&pixElev,
			1, 1,
			GDT_Float32,
			0, 0);

		std::cout << QString("Elevation at initiation point is %1 metres.").arg(pixElev) + "\n";

		for (int i = 0; i < GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand); ++i)
		{
			int L = floorf(i / GDALGetRasterBandXSize(hBand));
			int P = i - (L*GDALGetRasterBandXSize(hBand));

			double xl = transform[0] + P*transform[1] + L*transform[2];
			double yl = transform[3] + P*transform[4] + L*transform[5];

			//Distance (x/Imax)
			double dist = ecludianDistance2D(*dataXLocation_, *dataYLocation_, xl, yl) / Imax;

			//Calc x cosh^2 arctanh(x^2) = x / 1 - x^4
			double denom = dist / (1 - pow(dist, 4.0));

			double hmax = (1 / (2 * gravity)) * pow((C * pow(Imax, 1.0 / 3.0) ) / dist , 2.0);
			
			energyConoid[i] = (float)std::max(0.0, hmax - pixElev - elevation[i]); //(float)std::max(0.0, hmax - fabs(elevation[i] - pixElev));

		}

		outputRaster = GDALCreate(GDALGetDatasetDriver(elevationDataset),
			outputRasterName.toLocal8Bit().constData(),
			GDALGetRasterXSize(elevationDataset),
			GDALGetRasterYSize(elevationDataset),
			1,
			GDT_Float32,
			NULL);

		GDALRasterBandH destBand = GDALGetRasterBand(outputRaster, 1);
		GDALSetGeoTransform(outputRaster, transform);


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

		GDALSetRasterNoDataValue(destBand, dstNodataValue);

		GDALRasterIO(destBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			energyConoid,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
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

