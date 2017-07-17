#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"

#include "mergerasters.h"
#include "opencv2/opencv.hpp"


namespace RF
{
    /**
     * \internal
     */
    class MergeRastersImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::MergeRastersImpl)

    public:
        MergeRasters&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > baseLayer_;
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > overlayLayer_;
        CSIRO::DataExecution::SimpleInput< double > baseWeight_;
        CSIRO::DataExecution::SimpleInput< double > overlayWeighting_;
		CSIRO::DataExecution::SimpleInput< QString > mergedDatasetFilename_;
        CSIRO::DataExecution::SimpleOutput< GDALDatasetH > mergedDataset_;
		

        MergeRastersImpl(MergeRasters& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    MergeRastersImpl::MergeRastersImpl(MergeRasters& op) :
        op_(op),
        baseLayer_("Base layer",  op_),
        overlayLayer_("Overlay layer",  op_),
        baseWeight_("Base weighting", 0.1,  op_),
		overlayWeighting_("Overlay weighting", 0.9,  op_),
		mergedDatasetFilename_("Merged dataset filename", op_),
        mergedDataset_("Merged dataset",  op_)
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
    bool MergeRastersImpl::execute()
    {
        const GDALDatasetH& baseLayer        = *baseLayer_;
        const GDALDatasetH& overlayLayer     = *overlayLayer_;
        const double&       baseWeight       = *baseWeight_;
        const double&       overlayWeighting = *overlayWeighting_;
        GDALDatasetH& mergedDataset    = *mergedDataset_;
		QString& filename = *mergedDatasetFilename_;
        
		GDALRasterBandH baseBand = GDALGetRasterBand(baseLayer, 1);

		float *baseData;
		baseData = new float[GDALGetRasterBandXSize(baseLayer)*GDALGetRasterBandYSize(baseLayer)];

		GDALRasterIO(baseBand, GF_Read,
			0, 0,
			GDALGetRasterBandXSize(baseBand), GDALGetRasterBandYSize(baseBand),
			baseData,
			GDALGetRasterBandXSize(baseBand), GDALGetRasterBandYSize(baseBand),
			GDT_Float32,
			0, 0);

		int srcNoData;
		float srcNoDataValue;

		srcNoDataValue = (float)GDALGetRasterNoDataValue(baseBand, &srcNoData);

		//Overlay
		GDALRasterBandH overlayBand = GDALGetRasterBand(overlayLayer, 1);

		float *overlayData;
		overlayData = new float[GDALGetRasterBandXSize(overlayLayer)*GDALGetRasterBandYSize(overlayLayer)];

		GDALRasterIO(overlayBand, GF_Read,
			0, 0,
			GDALGetRasterBandXSize(overlayBand), GDALGetRasterBandYSize(overlayBand),
			overlayData,
			GDALGetRasterBandXSize(overlayBand), GDALGetRasterBandYSize(overlayBand),
			GDT_Float32,
			0, 0);


		double baseTransform[6], overlayTransform[6];
		GDALGetGeoTransform(baseLayer, baseTransform);
		GDALGetGeoTransform(overlayLayer, overlayTransform);
		/******Geo Transform is
		adfGeoTransform[0] /* top left x
		adfGeoTransform[1] /* w-e pixel resolution
		adfGeoTransform[2] /* 0
		adfGeoTransform[3] /* top left y
		adfGeoTransform[4] /* 0
		adfGeoTransform[5] /* n-s pixel resolution (negative value)
		*/

		//Check that overlay is same size or smaller
		/*
		if (baseTransform[0] < overlayTransform[0] || //X min smaller
			baseTransform[0] + baseTransform[1] * GDALGetRasterBandXSize(baseBand) < overlayTransform[0] + overlayTransform[1] * GDALGetRasterBandXSize(overlayBand)) //X max
		{
			std::cout << QString("ERROR: Base raster is not bigger or equal to overlay raster in X") + "\n";
			return false;
		}
		if (baseTransform[3] < overlayTransform[3] || //Y min smaller
			baseTransform[3] + baseTransform[5] * GDALGetRasterBandYSize(baseBand) < overlayTransform[3] + overlayTransform[5] * GDALGetRasterBandYSize(overlayBand)) //X max
		{
			std::cout << QString("ERROR: Base raster is not bigger or equal to overlay raster in Y") + "\n";
			return false;
		}
		*/
		//Convert float array to an openCV mat (assuming rows = Y)
		cv::Mat baseMat(GDALGetRasterBandYSize(baseBand), GDALGetRasterBandXSize(baseBand), CV_32F, baseData);
		cv::Mat overlayMat(GDALGetRasterBandYSize(overlayBand), GDALGetRasterBandXSize(overlayBand), CV_32F, overlayData);

		//Resize (most likely downsample) overlay to baseData size
		int overlayResizeY = floor((overlayTransform[5] * GDALGetRasterBandYSize(overlayBand)) / baseTransform[5]);
		int overlayResizeX = floor((overlayTransform[1] * GDALGetRasterBandXSize(overlayBand)) / baseTransform[1]);
		std::cout << QString("Resizing to %1 x and %2 y").arg(overlayResizeX).arg(overlayResizeY) + "\n";
		cv::Mat overlayMatSize(overlayResizeY, overlayResizeY, CV_32F);
		cv::resize(overlayMat, overlayMatSize, overlayMatSize.size(), 0, 0, CV_INTER_LINEAR);

		//Work out ROI (base/anchor location)
		int x_off = floor((overlayTransform[0] - baseTransform[0]) / baseTransform[1]);
		int y_off = floor((overlayTransform[3] - baseTransform[3]) / baseTransform[5]);
		cv::Rect roi = cv::Rect(x_off, y_off, overlayMatSize.rows, overlayMatSize.cols);
		
		//Now do linear addition
		cv::Mat outputMat = baseMat.clone();
		cv::addWeighted(outputMat(roi), baseWeight, overlayMatSize, overlayWeighting, 0.0, outputMat(roi));
		
		mergedDataset = GDALCreate(GDALGetDatasetDriver(baseLayer),
			filename.toLocal8Bit().constData(),
			GDALGetRasterXSize(baseLayer), GDALGetRasterYSize(baseLayer),
			1,
			GDT_Float32, NULL);

		GDALRasterBandH destBand = GDALGetRasterBand(mergedDataset, 1);
		GDALSetGeoTransform(mergedDataset, baseTransform);
		GDALSetProjection(mergedDataset, GDALGetProjectionRef(baseLayer));
		GDALSetRasterNoDataValue(destBand, srcNoDataValue);

		GDALRasterIO(destBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(baseBand), GDALGetRasterBandYSize(baseBand),
			outputMat.data,
			GDALGetRasterBandXSize(baseBand), GDALGetRasterBandYSize(baseBand),
			GDT_Float32,
			0, 0);
		    
        return true;
    }


    /**
     *
     */
    MergeRasters::MergeRasters() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< MergeRasters >::getInstance(),
            tr("Merge rasters"))
    {
        pImpl_ = new MergeRastersImpl(*this);
    }


    /**
     *
     */
    MergeRasters::~MergeRasters()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  MergeRasters::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(MergeRasters, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

