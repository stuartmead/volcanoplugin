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

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "volcanoplugin.h"

#include "volcanoplugin.h"
#include "getpixelvalue.h"


namespace RF
{
    /**
     * \internal
     */
    class GetPixelValueImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::GetPixelValueImpl)

    public:
        GetPixelValue&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDatabase_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< double >        dataXLocation_;
        CSIRO::DataExecution::TypedObject< double >        dataYLocation_;
        CSIRO::DataExecution::TypedObject< double >        dataValue_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDatabase_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputXLocation_;
        CSIRO::DataExecution::InputScalar inputYLocation_;
        CSIRO::DataExecution::Output      outputValue_;


        GetPixelValueImpl(GetPixelValue& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    GetPixelValueImpl::GetPixelValueImpl(GetPixelValue& op) :
        op_(op),
        dataGDALDatabase_(),
        dataRasterBand_(),
        dataXLocation_(),
        dataYLocation_(),
        dataValue_(),
        inputGDALDatabase_("GDAL Database", dataGDALDatabase_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputXLocation_("X location", dataXLocation_, op_),
        inputYLocation_("Y location", dataYLocation_, op_),
        outputValue_("Value", dataValue_, op_)
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
    bool GetPixelValueImpl::execute()
    {
        GDALDatasetH& gDALDatabase = *dataGDALDatabase_;
        int&          rasterBand   = *dataRasterBand_;
        double&       xLocation    = *dataXLocation_;
        double&       yLocation    = *dataYLocation_;
        double&       value        = *dataValue_;
        

        if (rasterBand > GDALGetRasterCount(gDALDatabase))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDatabase)).arg(rasterBand) + "\n";
            return false;
        }

        GDALRasterBandH hBand = GDALGetRasterBand(gDALDatabase, rasterBand);

        double transform[6], invTransform[6];
        GDALGetGeoTransform(gDALDatabase,transform);
        GDALInvGeoTransform(transform, invTransform);

        int pixel, line;

        pixel = (int) floor(invTransform[0] + invTransform[1] * xLocation + invTransform[2] * yLocation);

        line = (int) floor(invTransform[3] + invTransform[4] * xLocation + invTransform[5] * yLocation);

        float pixVal;

        GDALRasterIO( hBand, GF_Read,
            pixel, line, //X Y location in cells
            1, 1, //X Y length in cells
            &pixVal, //value
            1, 1,
            GDT_Float32, //Type
            0, 0); //Scanline stuff

        value = pixVal;

        return true;
    }


    /**
     *
     */
    GetPixelValue::GetPixelValue() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< GetPixelValue >::getInstance(),
            tr("Get pixel value"))
    {
        pImpl_ = new GetPixelValueImpl(*this);
    }


    /**
     *
     */
    GetPixelValue::~GetPixelValue()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  GetPixelValue::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(GetPixelValue, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

