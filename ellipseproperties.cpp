#include <cassert>

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"
#include <iostream>

#include <qstring.h>
#include "volcanoplugin.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QImage>

#include "volcanoutils.h"
#include "ellipseproperties.h"


namespace RF
{
    /**
     * \internal
     */
    class EllipsePropertiesImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EllipsePropertiesImpl)

    public:
        EllipseProperties&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > heightRaster_;
		CSIRO::DataExecution::SimpleInput< int > heightThreshold_;
        CSIRO::DataExecution::SimpleOutput< double > xLocation_;
		CSIRO::DataExecution::SimpleOutput< double > yLocation_;
		CSIRO::DataExecution::SimpleOutput< int > width_;
		CSIRO::DataExecution::SimpleOutput< int > height_;
		CSIRO::DataExecution::SimpleOutput< double > angle_;
		


        EllipsePropertiesImpl(EllipseProperties& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    EllipsePropertiesImpl::EllipsePropertiesImpl(EllipseProperties& op) :
        op_(op),
        heightRaster_("Height raster",  op_),
		heightThreshold_("Height threshold", op_, 80),
		xLocation_("X - centre location (cell)", op_),
		yLocation_("Y - centre location (cell)", op_),
		width_("Ellipse width", op_),
		height_("Ellipse height", op_),
		angle_("Ellipse rotation angle", op_)

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
    bool EllipsePropertiesImpl::execute()
    {
        const GDALDatasetH& heightRaster = *heightRaster_;
		const int&			heightThreshold = *heightThreshold_;

		float *data;
		float dstNodataValue;

		//Collect data
		data = getRasterData(heightRaster, dstNodataValue);

		//Convert to a mat
		cv::Mat inputMat(GDALGetRasterYSize(heightRaster), GDALGetRasterXSize(heightRaster), CV_32F, data);
		//Convert to a single channel 8 bit image for canny
		cv::Mat inputGrey(inputMat.size(), CV_8U);
		inputMat.convertTo(inputGrey, CV_8U);
		

		//Run canny edge detection
		cv::Mat edges;
		cv::Canny(inputGrey, edges, heightThreshold, 3 * heightThreshold);
		cv::Mat thresholdedOutput;
		edges.convertTo(thresholdedOutput, CV_8U);

		
			
		//Find the contours
		std::vector< std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> heirarchy;

		cv::findContours(thresholdedOutput, contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
		std::cout << QString("Found %1 contours \n").arg(contours.size());
		//Create vector of ellipses
		cv::RotatedRect maxEllipse;

		//Loop through ellipses to find largest
		int area = 0;
		for (int i = 0; i < contours.size(); i++) {
			if (contours[i].size() > 50) {
				if (cv::fitEllipse(cv::Mat(contours[i])).boundingRect().area() > area) {
					maxEllipse = cv::fitEllipse(cv::Mat(contours[i]));
				}
			}
		}

		//Output these details for conversion
		//Cell AREA CO-ORDS
		std::cout << QString("CELL AREA COORDINATES:\n");
		std::cout << QString("Maximum ellipse is within a %1 width, %2 height and %3 cell area rectangle \n").arg(maxEllipse.boundingRect().width).arg(maxEllipse.boundingRect().height).arg(maxEllipse.boundingRect().area());
		std::cout << QString("The rotation angle is %1, at %2 x and %3 y \n").arg(maxEllipse.angle).arg(maxEllipse.center.x).arg(maxEllipse.center.y);

		//Convert to geographic coordinates
		//Get Transform and invert to get pixel/line
		//double transform[6], invTransform[6];
		double * transform;
		double * invTransform;
		transform = new double[6];
		invTransform = new double[6];
		GDALGetGeoTransform(heightRaster, transform);
		//Coefficients between Pixel Line and Projected (Yp, Xp space)
		//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
		//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
				
		//GDALInvGeoTransform(transform, invTransform);
		*xLocation_ = transform[0] + maxEllipse.center.x * transform[1] + maxEllipse.center.y * transform[2];
		*yLocation_ = transform[3] + maxEllipse.center.x * transform[4] + maxEllipse.center.y * transform[5];
		*width_ = fabs((float(maxEllipse.boundingRect().width) * transform[1])/2);
		*height_ = fabs((float(maxEllipse.boundingRect().height) * transform[5])/2);
		*angle_ = maxEllipse.angle;

        return true;
    }


    /**
     *
     */
    EllipseProperties::EllipseProperties() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< EllipseProperties >::getInstance(),
            tr("Fit ellipse to raster"))
    {
        pImpl_ = new EllipsePropertiesImpl(*this);
    }


    /**
     *
     */
    EllipseProperties::~EllipseProperties()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  EllipseProperties::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(EllipseProperties, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

