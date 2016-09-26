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
#include "totalupstreamproperty.h"
#include "erosionutils.h"

namespace RF
{
    /**
     * \internal
     */
    class TotalUpstreamPropertyImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::TotalUpstreamPropertyImpl)

    public:
        TotalUpstreamProperty&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataSlopeDirectionDataset_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataPropertyDataset_;
        CSIRO::DataExecution::TypedObject< QString >       dataOutputRasterName_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataUpstreamProperties_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputSlopeDirectionDataset_;
        CSIRO::DataExecution::InputScalar inputPropertyDataset_;
        CSIRO::DataExecution::InputScalar inputOutputRasterName_;
        CSIRO::DataExecution::Output      outputUpstreamProperties_;


        TotalUpstreamPropertyImpl(TotalUpstreamProperty& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    TotalUpstreamPropertyImpl::TotalUpstreamPropertyImpl(TotalUpstreamProperty& op) :
        op_(op),
        dataSlopeDirectionDataset_(),
        dataPropertyDataset_(),
        dataOutputRasterName_(),
        dataUpstreamProperties_(),
        inputSlopeDirectionDataset_("Slope direction dataset", dataSlopeDirectionDataset_, op_),
        inputPropertyDataset_("Property dataset", dataPropertyDataset_, op_),
        inputOutputRasterName_("Output Raster Name", dataOutputRasterName_, op_),
        outputUpstreamProperties_("Upstream properties", dataUpstreamProperties_, op_)
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
    bool TotalUpstreamPropertyImpl::execute()
    {
        GDALDatasetH& slopeDirectionDataset = *dataSlopeDirectionDataset_;
        GDALDatasetH& propertyDataset       = *dataPropertyDataset_;
        QString&      outputRasterName      = *dataOutputRasterName_;
        GDALDatasetH& upstreamProperties    = *dataUpstreamProperties_;
        
        GDALRasterBandH fBand, propertyBand;

        fBand = GDALGetRasterBand(slopeDirectionDataset, 1);
        propertyBand = GDALGetRasterBand(propertyDataset, 1);

        double transformf[6];
        GDALGetGeoTransform(slopeDirectionDataset, transformf);
        
        double transformd[6];
        GDALGetGeoTransform(propertyDataset, transformd);

        for (int i = 0; i < 6; ++i)
        {
            if (transformf[i] != transformd[i])
            {
                std::cout << QString("ERROR: Transforms are not equal between rasters. Index %1 is %2 for flow direction but %3 for property").arg(i).arg(transformf[i]).arg(transformd[i]) + "\n";
                //return false;
            }
        }

        //Setup
        float srcNodataValue;
        int srcNoData;

        srcNodataValue = (float)GDALGetRasterNoDataValue(fBand, &srcNoData);
        float dstNoDataValue = srcNodataValue;
        

        void *pData;

        pData = RecursiveCreateInputData(transformf);

        GenericRecursivePropertyAlgebraAlg pfnAlg = RecursiveUpstreamPropAlg;

        int xSize = GDALGetRasterBandXSize(fBand);
        int ySize = GDALGetRasterBandYSize(fBand);
        float* propertyValues = new float[xSize*ySize];


        GDALRasterIO(propertyBand, GF_Read,
            0, 0, //X,Y offset in cells
            xSize, ySize, //X,Y length in cells
            propertyValues, //data
            ySize, xSize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);

            upstreamProperties = GDALCreate( GDALGetDatasetDriver(slopeDirectionDataset),
                                        outputRasterName.toLocal8Bit().constData(),
                                        GDALGetRasterXSize(slopeDirectionDataset),
                                        GDALGetRasterYSize(slopeDirectionDataset),
                                        1,
                                        GDT_Float32, NULL);

            GDALRasterBandH algBand = GDALGetRasterBand(upstreamProperties, 1);
            GDALSetGeoTransform(upstreamProperties, transformf);
            GDALSetProjection(upstreamProperties, GDALGetProjectionRef(slopeDirectionDataset));
            GDALSetRasterNoDataValue(algBand, dstNoDataValue);

            GenericRecursivePropertyAlgebraProcessor(fBand, propertyBand, algBand, transformf, pfnAlg, pData);

        return true;
    }


    /**
     *
     */
    TotalUpstreamProperty::TotalUpstreamProperty() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< TotalUpstreamProperty >::getInstance(),
            tr("Calculate sum of upstream values"))
    {
        pImpl_ = new TotalUpstreamPropertyImpl(*this);
    }


    /**
     *
     */
    TotalUpstreamProperty::~TotalUpstreamProperty()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  TotalUpstreamProperty::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(TotalUpstreamProperty, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

