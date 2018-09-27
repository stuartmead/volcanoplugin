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
#include <QString>
#include <QImage>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

//OpenCV stuff
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/photo/photo.hpp"

#include "volcanoplugin.h"
#include "deionise.h"


namespace RF
{
	/**
	* \internal
	*/
	class DeioniseImpl
	{
		// Allow string translation to work properly
		Q_DECLARE_TR_FUNCTIONS(RF::DeioniseImpl)

	public:
		Deionise&  op_;

		// Data objects
		CSIRO::DataExecution::TypedObject< QString >  dataInputImage_;
		CSIRO::DataExecution::TypedObject< int >     dataH_;
		CSIRO::DataExecution::TypedObject< int >     dataSearchWindow_;
		CSIRO::DataExecution::TypedObject< int >     dataBlockSize_;
		CSIRO::DataExecution::TypedObject< QImage >  dataOutputImage_;


		// Inputs and outputs
		CSIRO::DataExecution::InputScalar inputInputImage_;
		CSIRO::DataExecution::InputScalar inputH_;
		CSIRO::DataExecution::InputScalar inputSearchWindow_;
		CSIRO::DataExecution::InputScalar inputBlockSize_;
		CSIRO::DataExecution::Output      outputOutputImage_;


		DeioniseImpl(Deionise& op);

		bool  execute();
		void  logText(const QString& msg) { op_.logText(msg); }
	};


	/**
	*
	*/
	DeioniseImpl::DeioniseImpl(Deionise& op) :
		op_(op),
		dataInputImage_(),
		dataH_(3),
		dataSearchWindow_(21),
		dataBlockSize_(7),
		dataOutputImage_(),
		inputInputImage_("Input image filename", dataInputImage_, op_),
		inputH_("h value", dataH_, op_),
		inputSearchWindow_("Search window", dataSearchWindow_, op_),
		inputBlockSize_("Block size", dataBlockSize_, op_),
		outputOutputImage_("Output image", dataOutputImage_, op_)
	{
		// Make sure all of our inputs have data by default. If your operation accepts a
		// large data structure as input, you may wish to remove this call and replace it
		// with constructors for each input in the initialisation list above.
		op_.ensureHasData();

		inputH_.setDescription("Parameter regulating filter strength. Big h value perfectly removes noise but also removes image details, smaller h value preserves details but also preserves some noise");
		inputSearchWindow_.setDescription("Size in pixels of the window that is used to compute weighted average for given pixel. Should be odd.");
		inputBlockSize_.setDescription("Size in pixels of the template patch that is used to compute weights. Should be odd. Recommended value 7 pixels");

		// Recommend setting a description of the operation and each input / output here:
		// op_.setDescription(tr("My operation does this, that and this other thing."));
		// input_.setDescription(tr("Used for such and such."));
		// output_.setDescription(tr("Results of the blah-di-blah."));
	}


	/**
	*
	*/
	bool DeioniseImpl::execute()
	{

		QString& image = *dataInputImage_;
		
		const cv::Mat img = cv::imread(image.toStdString(), cv::IMREAD_GRAYSCALE);

		cv::Mat outImg = cv::Mat(img.rows, img.cols, img.type());

		//Run deinonise
		cv::fastNlMeansDenoising(img, outImg, *dataH_, *dataSearchWindow_, *dataBlockSize_);

		cv::cvtColor(outImg, outImg, cv::COLOR_GRAY2RGB);
		
		QImage qtmp((uchar*)outImg.data, outImg.cols, outImg.rows, outImg.step, QImage::Format_RGB888);
		QImage qtmp2(qtmp);
		qtmp2.detach();
		*dataOutputImage_ = qtmp2;


		return true;
	}


	/**
	*
	*/
	Deionise::Deionise() :
		CSIRO::DataExecution::Operation(
			CSIRO::DataExecution::OperationFactoryTraits< Deionise >::getInstance(),
			tr("Deionise dataset"))
	{
		pImpl_ = new DeioniseImpl(*this);
	}


	/**
	*
	*/
	Deionise::~Deionise()
	{
		delete pImpl_;
	}


	/**
	*
	*/
	bool  Deionise::execute()
	{
		return pImpl_->execute();
	}
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(Deionise,
	RF::VolcanoPlugin::getInstance(),
	CSIRO::DataExecution::Operation::tr("Geospatial"))

