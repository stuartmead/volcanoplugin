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

#include <Qvector>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "gdal.h"

#include "volcanoplugin.h"
#include "rasterbandsummary.h"


namespace RF
{
    /**
     * \internal
     */
    class RasterBandSummaryImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RasterBandSummaryImpl)

    public:
        RasterBandSummary&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >           dataBandNumber_;
        CSIRO::DataExecution::TypedObject< double >        dataMinimum_;
        CSIRO::DataExecution::TypedObject< double >        dataMaximum_;
        CSIRO::DataExecution::TypedObject< QVector<int> >  dataHistogram_;
        CSIRO::DataExecution::TypedObject< QVector<double> >  dataHistogramValues_;
        CSIRO::DataExecution::TypedObject< int >           dataNumberOfBins_;
        CSIRO::DataExecution::TypedObject< double >        dataHistmin_;
        CSIRO::DataExecution::TypedObject< double >        dataHistmax_;

        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputBandNumber_;
        CSIRO::DataExecution::Output      outputMinimum_;
        CSIRO::DataExecution::Output      outputMaximum_;
        CSIRO::DataExecution::Output      outputHistogram_;
        CSIRO::DataExecution::Output      outputHistogramValues_;
        CSIRO::DataExecution::InputScalar inputNumberOfBins_;
        CSIRO::DataExecution::InputScalar inputHistmin_;
        CSIRO::DataExecution::InputScalar inputHistmax_;

        RasterBandSummaryImpl(RasterBandSummary& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    RasterBandSummaryImpl::RasterBandSummaryImpl(RasterBandSummary& op) :
        op_(op),
        dataGDALDataset_(),
        dataBandNumber_(1),
        dataMinimum_(),
        dataMaximum_(),
        dataHistogram_(),
        dataHistogramValues_(),
        dataNumberOfBins_(50),
        dataHistmin_(-1),
        dataHistmax_(-1),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputBandNumber_("Band number", dataBandNumber_, op_),
        outputMinimum_("Minimum", dataMinimum_, op_),
        outputMaximum_("Maximum", dataMaximum_, op_),
        outputHistogram_("Histogram", dataHistogram_, op_),
        outputHistogramValues_("Histogram values", dataHistogramValues_, op_),
        inputNumberOfBins_("Number of bins", dataNumberOfBins_, op_),
        inputHistmin_("Histogram minimum", dataHistmin_, op_),
        inputHistmax_("Histogram maximum", dataHistmax_, op_)
    {
        inputHistmin_.setDescription("Minimum value for plotting histogram, use a negative value to auto choose range");
        inputHistmax_.setDescription("Maximum value for plotting histogram, use a negative value to auto choose range");
    }


    /**
     *
     */
    bool RasterBandSummaryImpl::execute()
    {
        GDALDatasetH& gDALDataset  = *dataGDALDataset_;
        int&          bandNumber   = *dataBandNumber_;
        double&       minimum      = *dataMinimum_;
        double&       maximum      = *dataMaximum_;
        QVector<int>& histogram    = *dataHistogram_;
        QVector< double >& histogramValues = *dataHistogramValues_;
        int&          numberOfBins = *dataNumberOfBins_;
        

        dataHistogram_->clear();
        dataHistogramValues_->clear();

        
                
        if(bandNumber > GDALGetRasterCount(gDALDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDataset)).arg(bandNumber) + "\n";
            return false;
        }

        GDALRasterBandH hBand = GDALGetRasterBand(gDALDataset, bandNumber);
        
        //Calc min/max
        double minmax[2];
        GDALComputeRasterMinMax(hBand, true, minmax);
        minimum = minmax[0];
        maximum = minmax[1];

        //Histogram
        int * histo;
        histo = new int [numberOfBins];

        if (*dataHistmin_ >= 0 && *dataHistmax_ > *dataHistmin_)
        {
            minmax[0] = *dataHistmin_;
            minmax[1] = *dataHistmax_;
        }

        if(GDALGetRasterHistogram(hBand,minmax[0], minmax[1], numberOfBins, histo, FALSE, TRUE, GDALDummyProgress, NULL)==CE_Failure)
        {
            std::cout << QString("WARNING: Something went wrong creating raster histogram, the vector will be empty") + "\n";
        }
        

        for (int i = 0; i < numberOfBins; ++i)
        {
            histogram.push_back(histo[i]);
            histogramValues.push_back(i*((minmax[1]-minmax[0])/numberOfBins));
        }

        


        return true;
    }


    /**
     *
     */
    RasterBandSummary::RasterBandSummary() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< RasterBandSummary >::getInstance(),
            tr("Compute raster summary statistics"))
    {
        pImpl_ = new RasterBandSummaryImpl(*this);
    }


    /**
     *
     */
    RasterBandSummary::~RasterBandSummary()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  RasterBandSummary::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(RasterBandSummary, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

