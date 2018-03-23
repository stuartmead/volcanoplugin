/*
/*
  Created by: Stuart Mead
  Creation date: $Date$
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"
#include "volcanoutils.h"

#include "volcanoplugin.h"
#include "fuzzylocation.h"

//Include opencv processes
#include "opencv2/imgproc/imgproc.hpp"


namespace RF
{
    /**
     * \internal
     */
    class FuzzyLocationImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FuzzyLocationImpl)

    public:
        FuzzyLocation&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > categoricalMap_;
		CSIRO::DataExecution::SimpleInput< bool > threshold_;
		CSIRO::DataExecution::SimpleInput< double > thresholdVal_;
		CSIRO::DataExecution::SimpleInput< int > kernelSize_;
        CSIRO::DataExecution::TypedObject< RF::FuzzyMembershipType > memberType_;
        CSIRO::DataExecution::SimpleInput< QString > outputName_;
        CSIRO::DataExecution::SimpleOutput< GDALDatasetH > outputDataset_;

		CSIRO::DataExecution::InputScalar inputMemberType_;

        FuzzyLocationImpl(FuzzyLocation& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    FuzzyLocationImpl::FuzzyLocationImpl(FuzzyLocation& op) :
        op_(op),
        categoricalMap_("Categorical map",  op_),
		threshold_("Threshold data", op_),
		thresholdVal_("Threshold value", op_),
		kernelSize_("Kernel size", op_),
        memberType_(RF::FuzzyMembershipType::GAUSSIAN),
        outputName_("Output dataset name",  op_),
        outputDataset_("Output dataset",  op_),
		inputMemberType_("Membership weighting", memberType_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();
		threshold_.input_.setDescription(tr("Create a boolean threshold for everything above the given value."));
        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input1_.input_.setDescription(tr("Used for such and such."));
        // output1_.output_.setDescription(tr("Results of the blah-di-blah."));
    }


    /**
     *
     */
    bool FuzzyLocationImpl::execute()
    {
        const GDALDatasetH&     categoricalMap = *categoricalMap_;
        const RF::FuzzyMembershipType& memberType     = *memberType_;
        const QString&          outputName     = *outputName_;
		const int& kernelSize = *kernelSize_;
        GDALDatasetH&     outputDataset  = *outputDataset_;
        
		//Check kernel size is odd
		if (!(kernelSize & 1)) {
			std::cout << QString("ERROR: Kernel size must be odd-numbered, is %1").arg(kernelSize % 2) + "\n";
			return false;
		}

		GDALRasterBandH hBand = GDALGetRasterBand(categoricalMap, 1);

		float *data;
		data = new float[GDALGetRasterBandXSize(hBand)*GDALGetRasterBandYSize(hBand)];

		GDALRasterIO(hBand, GF_Read,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			data,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			GDT_Float32,
			0, 0);

		int srcNoData;
		float srcNoDataValue;

		srcNoDataValue = (float)GDALGetRasterNoDataValue(hBand, &srcNoData);

		//Convert float array to an openCV mat (assuming rows = Y)
		cv::Mat dataToMat(GDALGetRasterBandYSize(hBand), GDALGetRasterBandXSize(hBand), CV_32F, data);

		//Thresholding
		if (*threshold_) {
			cv::threshold(dataToMat.clone(), dataToMat, *thresholdVal_, 1.0, CV_THRESH_BINARY);
		}

		//Create dilated (Category 1) and eroded (Category 0) mat
		cv::Mat dilationCategory(dataToMat.size(), dataToMat.type());
		cv::Mat erosionCategory(dataToMat.size(), dataToMat.type());

		//Create kernel input for erosion and dilation
		cv::Mat kernel = cv::Mat::zeros(cv::Size(kernelSize,kernelSize), CV_32F);
		//Create dilation and erosion doubles
		double dilation, erosion;

		if (memberType == RF::FuzzyMembershipType::LINEAR) {
			double length = (int)(kernelSize / 2); //Cast to int should floor without negative infinite
			double dist = sqrt(length*length + length*length);
			double m = -1.0 / dist; //Calculate slope, is negative to go from 0 to distance
			//Generate kernel
			for (int ki = 0; ki < kernel.rows; ++ki) {
				double rlength = fabs(ki - (int)(kernelSize / 2)); //Cast to int should floor without negative infinite
				for (int kj = 0; kj < kernel.cols; ++kj) {
					double clength = fabs(kj - (int)(kernelSize / 2));
					//Calculate distance
					double kdist = sqrt(rlength*rlength + clength*clength);
					kernel.at<float>(ki, kj) = float(m*kdist + 1.0);
				}
			}
			std::cout << "Linear Mat = " << std::endl << kernel << std::endl;
			//Now convolve the kernels
			for (int i = 0; i < dataToMat.rows; ++i) {
				int rowStart = std::max(i - ((kernelSize - 1) / 2), 0);
				int rowEnd = std::min(i + ((kernelSize - 1) / 2) + 1, dataToMat.rows);
				for (int j = 0; j < dataToMat.cols; ++j) {
					int colStart = std::max(j - ((kernelSize - 1) / 2), 0);
					int colEnd = std::min(j + ((kernelSize - 1) / 2) + 1, dataToMat.cols);

					cv::Mat R = dataToMat(cv::Range(rowStart, rowEnd), cv::Range(colStart, colEnd));

					if (R.total() != kernel.total()) { /*
													   if (rowStart == 0) {
													   if (colStart == 0) {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
													   cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   else if (colEnd == dataToMat.cols) {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
													   cv::Range(0, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   else {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
													   cv::Range::all()), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   }
													   else if (rowEnd == dataToMat.rows) {
													   if (colStart == 0) {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
													   cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   else if (colEnd == dataToMat.cols) {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
													   cv::Range(0, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   else {
													   cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
													   cv::Range::all()), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   }
													   else if (colStart == 0) {
													   //Rowstart and end cases dealth with
													   cv::minMaxLoc(R.mul(kernel(cv::Range::all(), //Rows
													   cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   else if (colEnd == dataToMat.cols) {
													   cv::minMaxLoc(R.mul(kernel(cv::Range::all(), //Rows
													   cv::Range(0, R.size().width)), scaleFactor), //Cols
													   &erosion, &dilation, NULL, NULL);
													   }
													   */
						dilation = 0;
						erosion = 0;
					}
					else {
						cv::minMaxLoc(R.mul(kernel), &erosion, &dilation, NULL, NULL);
					}

					dilationCategory.at<float>(i, j) = float(dilation);
					erosionCategory.at<float>(i, j) = float(erosion);
				}
			}
		}
		else if (memberType == RF::FuzzyMembershipType::CONSTANT) {
			kernel = cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, kernel.size());
			//Create dilation band
			cv::dilate(dataToMat, dilationCategory, kernel);
			//Create erosion band
			cv::erode(dataToMat, erosionCategory, kernel);
		}
		else if (memberType == RF::FuzzyMembershipType::GAUSSIAN) {
			kernel = cv::getGaussianKernel(kernelSize, -1, CV_32F) * cv::getGaussianKernel(kernelSize, -1, CV_32F).t();
			float scaleFactor = 1.0 / kernel.at<float>((kernelSize - 1) / 2, (kernelSize - 1) / 2);
			//std::cout << "Mat = " << std::endl << kernel << std::endl;
			//Run convolution
			for (int i = 0; i < dataToMat.rows; ++i) {
				int rowStart = std::max(i - ((kernelSize - 1) / 2), 0);
				int rowEnd = std::min(i + ((kernelSize - 1) / 2) + 1, dataToMat.rows);
				for (int j = 0; j < dataToMat.cols; ++j) {
					int colStart = std::max(j - ((kernelSize - 1) / 2), 0);
					int colEnd = std::min(j + ((kernelSize - 1) / 2) + 1, dataToMat.cols);

					cv::Mat R = dataToMat(cv::Range(rowStart, rowEnd), cv::Range(colStart, colEnd));
					
					if (R.total() != kernel.total()) { /*
						if (rowStart == 0) {
							if (colStart == 0) {
								cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
									cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL); 
							}
							else if (colEnd == dataToMat.cols) {
								cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
									cv::Range(0, R.size().width)), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL);
							}
							else {
								cv::minMaxLoc(R.mul(kernel(cv::Range(kernelSize - R.size().height, R.size().height), //Rows
									cv::Range::all()), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL);
							}
						}
						else if (rowEnd == dataToMat.rows) {
							if (colStart == 0) {
								cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
									cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL);
							}
							else if (colEnd == dataToMat.cols) {
								cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
									cv::Range(0, R.size().width)), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL);
							}
							else {
								cv::minMaxLoc(R.mul(kernel(cv::Range(0, R.size().height), //Rows
									cv::Range::all()), scaleFactor), //Cols
									&erosion, &dilation, NULL, NULL);
							}
						}
						else if (colStart == 0) {
							//Rowstart and end cases dealth with
							cv::minMaxLoc(R.mul(kernel(cv::Range::all(), //Rows
								cv::Range(kernelSize - R.size().width, R.size().width)), scaleFactor), //Cols
								&erosion, &dilation, NULL, NULL);
						}
						else if (colEnd == dataToMat.cols) {
							cv::minMaxLoc(R.mul(kernel(cv::Range::all(), //Rows
								cv::Range(0, R.size().width)), scaleFactor), //Cols
								&erosion, &dilation, NULL, NULL);
						}
						*/
						dilation = 0;
						erosion = 0;
					}
					else {
						cv::minMaxLoc(R.mul(kernel, scaleFactor), &erosion, &dilation, NULL, NULL);
					}
					
					dilationCategory.at<float>(i, j) = float(dilation);
					erosionCategory.at<float>(i, j) = float(erosion);
				}
			}

		}

		outputDataset = GDALCreate(GDALGetDriverByName("GTiff"),//GDALGetDatasetDriver(categoricalMap),
			outputName.toLocal8Bit().constData(),
			GDALGetRasterXSize(categoricalMap), GDALGetRasterYSize(categoricalMap),
			2,//Dilation an derosion map
			GDT_Float32, NULL);
		double transform[6];
		GDALGetGeoTransform(categoricalMap, transform);

		GDALRasterBandH dilationBand = GDALGetRasterBand(outputDataset, 1);
		GDALRasterBandH erosionBand = GDALGetRasterBand(outputDataset, 2);
		GDALSetGeoTransform(outputDataset, transform);
		GDALSetProjection(outputDataset, GDALGetProjectionRef(categoricalMap));

		GDALSetRasterNoDataValue(dilationBand, srcNoDataValue);


		GDALRasterIO(dilationBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			dilationCategory.data,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			GDT_Float32,
			0, 0);

		GDALRasterIO(erosionBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			erosionCategory.data,
			GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
			GDT_Float32,
			0, 0);

		GDALClose(outputDataset);

        return true;
    }


    /**
     *
     */
    FuzzyLocation::FuzzyLocation() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< FuzzyLocation >::getInstance(),
            tr("Fuzzy location classifier"))
    {
        pImpl_ = new FuzzyLocationImpl(*this);
    }


    /**
     *
     */
    FuzzyLocation::~FuzzyLocation()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  FuzzyLocation::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(FuzzyLocation, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

