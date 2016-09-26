#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"



#include "boundsofraster.h"
#include "volcanoplugin.h"
#include "projtooffset.h"


namespace RF
{
    /**
     * \internal
     */
    class ProjToOffsetImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ProjToOffsetImpl)

    public:
        ProjToOffset&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >        dataRasterDataset_;
        CSIRO::DataExecution::TypedObject< RF::BoundsofRaster >  dataWindowBounds_;
        CSIRO::DataExecution::TypedObject< int >                 dataXOffset_;
        CSIRO::DataExecution::TypedObject< int >                 dataYOffset_;
        CSIRO::DataExecution::TypedObject< int >                 dataXLength_;
        CSIRO::DataExecution::TypedObject< int >                 dataYLength_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputRasterDataset_;
        CSIRO::DataExecution::InputScalar inputWindowBounds_;
        CSIRO::DataExecution::Output      outputXOffset_;
        CSIRO::DataExecution::Output      outputYOffset_;
        CSIRO::DataExecution::Output      outputXLength_;
        CSIRO::DataExecution::Output      outputYLength_;


        ProjToOffsetImpl(ProjToOffset& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    ProjToOffsetImpl::ProjToOffsetImpl(ProjToOffset& op) :
        op_(op),
        dataRasterDataset_(),
        dataWindowBounds_(),
        dataXOffset_(),
        dataYOffset_(),
        dataXLength_(),
        dataYLength_(),
        inputRasterDataset_("Raster dataset", dataRasterDataset_, op_),
        inputWindowBounds_("Window bounds", dataWindowBounds_, op_),
        outputXOffset_("X offset", dataXOffset_, op_),
        outputYOffset_("Y offset", dataYOffset_, op_),
        outputXLength_("X length", dataXLength_, op_),
        outputYLength_("Y length", dataYLength_, op_)
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
    bool ProjToOffsetImpl::execute()
    {
        GDALDatasetH&       rasterDataset = *dataRasterDataset_;
        RF::BoundsofRaster& windowBounds  = *dataWindowBounds_;
        int&                xOffset       = *dataXOffset_;
        int&                yOffset       = *dataYOffset_;
        int&                xLength       = *dataXLength_;
        int&                yLength       = *dataYLength_;
        

        double adfGeotransform[6];
        
        GDALGetGeoTransform(rasterDataset, adfGeotransform);

        double dfULX, dfULY, dfLRX, dfLRY;

        dfULX = windowBounds.getWesternBound();
        dfULY = windowBounds.getSouthernBound();
        dfLRX = windowBounds.getEasternBound();
        dfLRY = windowBounds.getNorthernBound();

        //Calc offsets
        xOffset = (int) floor((dfULX - adfGeotransform[0]) / adfGeotransform[1] + 0.001);
        yOffset = (int)  floor((dfULY - adfGeotransform[3]) / adfGeotransform[5] + 0.001);
        xLength = (int) ((dfLRX - dfULX) / adfGeotransform[1] + 0.5);
        yLength = (int) ((dfLRY - dfULY) / adfGeotransform[5] + 0.5);
        

        return true;
    }


    /**
     *
     */
    ProjToOffset::ProjToOffset() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< ProjToOffset >::getInstance(),
            tr("Calculate cell values from projection window"))
    {
        pImpl_ = new ProjToOffsetImpl(*this);
    }


    /**
     *
     */
    ProjToOffset::~ProjToOffset()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  ProjToOffset::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(ProjToOffset, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

