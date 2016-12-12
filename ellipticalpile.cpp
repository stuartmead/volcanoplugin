#include <cassert>
#include <iostream>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

//OpenCV core for ellipse
#include "opencv2/imgproc.hpp"

#include "qimage.h"

#include "volcanoplugin.h"
#include "ellipticalpile.h"


namespace RF
{
    /**
     * \internal
     */
    class EllipticalPileImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EllipticalPileImpl)

    public:
        EllipticalPile&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< double >        dataCenterX_;
        CSIRO::DataExecution::TypedObject< double >        dataCenterY_;
        CSIRO::DataExecution::TypedObject< double >        dataMajorLen_;
        CSIRO::DataExecution::TypedObject< double >        dataMinorLen_;
        CSIRO::DataExecution::TypedObject< double >        dataAngle_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataBaseDataSet_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputEllipse_;
		CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
		CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
		CSIRO::DataExecution::TypedObject< double >        dataVolume_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputCenterX_;
        CSIRO::DataExecution::InputScalar inputCenterY_;
        CSIRO::DataExecution::InputScalar inputMajorLen_;
        CSIRO::DataExecution::InputScalar inputMinorLen_;
        CSIRO::DataExecution::InputScalar inputAngle_;
        CSIRO::DataExecution::InputScalar inputBaseDataSet_;
		CSIRO::DataExecution::InputScalar inputOutputRasterName_;
		CSIRO::DataExecution::Output      outputOutputEllipse_;
		CSIRO::DataExecution::InputScalar inputRasterBand_;
		CSIRO::DataExecution::InputScalar inputVolume_;


        EllipticalPileImpl(EllipticalPile& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    EllipticalPileImpl::EllipticalPileImpl(EllipticalPile& op) :
        op_(op),
        dataCenterX_(),
        dataCenterY_(),
        dataMajorLen_(),
        dataMinorLen_(),
        dataAngle_(),
        dataBaseDataSet_(),
        dataOutputEllipse_(),
        dataVolume_(),
	    inputCenterX_("Center X", dataCenterX_, op_),
        inputCenterY_("Center Y", dataCenterY_, op_),
        inputMajorLen_("Major axis length", dataMajorLen_, op_),
        inputMinorLen_("Minor axis length", dataMinorLen_, op_),
        inputAngle_("Angle", dataAngle_, op_),
        inputBaseDataSet_("BaseDataSet", dataBaseDataSet_, op_),
        outputOutputEllipse_("Output Ellipse", dataOutputEllipse_, op_),
		inputOutputRasterName_("Output Raster name", dataOutputRasterName_, op_),
		inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputVolume_("Target volume", dataVolume_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input_.setDescription(tr("Used for such and such."));
        // output_.setDescription(tr("Results of the blah-di-blah."));
    }


    /**
     *
     */
    bool EllipticalPileImpl::execute()
    {
        double&       centerX       = *dataCenterX_;
        double&       centerY       = *dataCenterY_;
        double&       majorLen      = *dataMajorLen_;
        double&       minorLen      = *dataMinorLen_;
        double&       angle         = *dataAngle_;
        GDALDatasetH& baseDataSet   = *dataBaseDataSet_;
		QString&      outputRasterName = *dataOutputRasterName_;
		GDALDatasetH& outputEllipse = *dataOutputEllipse_;
		int&          rasterBand = *dataRasterBand_;
        double&       volume        = *dataVolume_;
        

		//Solve for max height of ellipse (h = 3*(2*Vol)/(4*pi*a*b))
		double height = 3 * ((volume*2) / (4 * M_PI*majorLen*minorLen));
		std::cout << QString("Maximum height of ellipse is %1 metres.").arg(height) + "\n";

		//Start with the raster
		GDALRasterBandH hBand;

		if (rasterBand > GDALGetRasterCount(baseDataSet))
		{
			std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(baseDataSet)).arg(rasterBand) + "\n";
		}

		hBand = GDALGetRasterBand(baseDataSet, rasterBand);

		/*
		Coefficients between Pixel Line and Projected (Yp, Xp space)
		Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
		Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
		*/
		
		//Get Transform and invert to get pixel/line
		double transform[6], invTransform[6];
		GDALGetGeoTransform(baseDataSet, transform);
		GDALInvGeoTransform(transform, invTransform);

		//Work out the pixel centre, pixel size of data
		cv::Point pixelCentre; 
		pixelCentre.x = (int)floor(invTransform[0] + invTransform[1] * centerX + invTransform[2] * centerY);
		pixelCentre.y = (int)floor(invTransform[3] + invTransform[4] * centerX + invTransform[5] * centerY);

		cv::Size axes((int) floor(majorLen/transform[1]),
						(int) floor(minorLen/transform[1]));


		//Nodata stuff
		int srcNoData;
		float srcNoDataValue;
		srcNoDataValue = (float)GDALGetRasterNoDataValue(hBand, &srcNoData);

		//Create elliptical raster array
		float * ellipseRaster;
		ellipseRaster = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

		
		//Convert float array to an openCV mat (assuming rows = Y)
		cv::Mat dataToMat(GDALGetRasterBandYSize(hBand), GDALGetRasterBandXSize(hBand), CV_32F, ellipseRaster);
					
		//Create ellipse
		cv::ellipse(dataToMat,
			pixelCentre,
			axes,
			angle,
			0, 360,
			cv::Scalar(10.0), cv::FILLED);


		
		double radAngle = M_PI - (angle * DEG2RAD);
		double bigA, bigB, z;
		std::cout << QString("Rotating ellipse by %1 radians.").arg(radAngle) + "\n";

		//Loop through the Mat and set heights
		for (int y = 0; y < dataToMat.rows; y++)
		{
			for (int x = 0; x < dataToMat.cols; x++)
			{
				if (dataToMat.at<float>(y, x) > 0.0)//value within ellipse
				{
					//Work in cell coords
					double xdistance = abs(pixelCentre.x - x);
					double ydistance = abs(pixelCentre.y - y);

					//Calculate values A and B (see http://math.stackexchange.com/questions/426150/what-is-the-general-equation-of-the-ellipse-that-is-not-in-the-origin-and-rotate)
					bigA = pow(((xdistance)*cos(radAngle) + (ydistance)*sin(radAngle)), 2) / pow(axes.width, 2);
					bigB = pow(((xdistance)*sin(radAngle) + (ydistance)*cos(radAngle)), 2) / pow(axes.height, 2);
					z = sqrt(pow(height, 2)*(1 - bigA - bigB));
					dataToMat.at<float>(y, x) = (float)z;

				}
			}
		}

		//Now write to the new gdalDataset

		outputEllipse = GDALCreate(GDALGetDatasetDriver(baseDataSet),
			outputRasterName.toLocal8Bit().constData(),
			GDALGetRasterXSize(baseDataSet), GDALGetRasterYSize(baseDataSet),
			1,
			GDT_Float32, NULL);

		GDALRasterBandH destBand = GDALGetRasterBand(outputEllipse, 1);
		GDALSetGeoTransform(outputEllipse, transform);
		GDALSetProjection(outputEllipse, GDALGetProjectionRef(baseDataSet));
		GDALSetRasterNoDataValue(destBand, srcNoDataValue);

		CPLErr error;
		error = GDALRasterIO(destBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			dataToMat.data,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			GDT_Float32,
			0, 0);

		if (error != CE_None)
		{
			std::cout << QString("ERROR: GDALRasterIO write operation failed.") + "\n";
		}

        return true;
    }


    /**
     *
     */
    EllipticalPile::EllipticalPile() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< EllipticalPile >::getInstance(),
            tr("Create Elliptical Pile"))
    {
        pImpl_ = new EllipticalPileImpl(*this);
    }


    /**
     *
     */
    EllipticalPile::~EllipticalPile()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  EllipticalPile::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(EllipticalPile, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

