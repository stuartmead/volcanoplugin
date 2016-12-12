
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/


#include <QString>
#include <QStringList>

#include "Workspace/DataExecution/DataObjects/datafactorytraits.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "Workspace/DataExecution/DataObjects/typeddatafactory.h"
#include "Workspace/Widgets/enumcomboboxfactory.h"

#include "gdal.h"

#include "volcanoplugin.h"
#include "ellipticalpile.h"
#include "totalupstreamproperty.h"
#include "getpixelvalue.h"
#include "uplsopefailurevolume.h"
#include "multiplyrasters.h"
#include "boxfilter.h"
#include "smooth.h"
#include "scalerastervalues.h"
#include "projtooffset.h"
#include "warp.h"
#include "energycone.h"
#include "iversonfailurevolume.h"
#include "rasterdifference.h"
#include "rastersum.h"
#include "fd8totheta.h"
#include "resetnodata.h"
#include "subsetdata.h"
#include "upslopearea.h"
#include "angletovectorstate.h"
#include "flowrouting.h"
#include "filldepressions.h"
#include "create3dmodel.h"
#include "debrisemergence.h"
#include "calccurvature.h"
#include "aspectcalc.h"
#include "volcanoutils.h"
#include "slopecalc.h"
#include "rastertoimage.h"
#include "rasterbandsummary.h"
#include "gdalinfo.h"

#include "boundsofraster.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


namespace RF
{
    /**
     * \internal
     */
    class VolcanoPluginImpl
    {
    public:
        // You can add, remove or modify anything in here without
        // breaking binary compatibility. It starts as empty, but
        // leave it here in case at some time in the future you
        // want to add some data for your plugin without breaking
        // binary compatibility.
    };


    /**
     *
     */
    VolcanoPlugin::VolcanoPlugin() :
            CSIRO::Application::WorkspacePlugin("www.riskfrontiers.com.au/volcano",
                                                "Volcano Analysis",
                                                TOSTRING(VOLCANO_PLUGIN_VERSION))
    {
        pImpl_ = new VolcanoPluginImpl;
    }


    /**
     *
     */
    VolcanoPlugin::~VolcanoPlugin()
    {
        delete pImpl_;
    }


    /**
     * \return  The singleton instance of this plugin.
     */
    VolcanoPlugin&  VolcanoPlugin::getInstance()
    {
        // This is a Singleton pattern. There will only ever be one
        // instance of the plugin across the entire application.
        static VolcanoPlugin plugin;
        return plugin;
    }


    /**
     *
     */
    bool  VolcanoPlugin::setup()
    {
        // Add your data factories like this:
        //addFactory( CSIRO::DataExecution::DataFactoryTraits<MyDataType>::getInstance() );
        addFactory(CSIRO::DataExecution::DataFactoryTraits<GDALDatasetH>::getInstance());
        addFactory(CSIRO::DataExecution::DataFactoryTraits<BoundsofRaster>::getInstance());
        addFactory(CSIRO::DataExecution::DataFactoryTraits<RF::SlopeAlgType>::getInstance());
        addFactory(CSIRO::DataExecution::DataFactoryTraits<RF::CurvatureType>::getInstance());
        addFactory(CSIRO::DataExecution::DataFactoryTraits<RF::BoundsofRaster>::getInstance());
        
        // Add your operation factories like this:
        //addFactory( CSIRO::DataExecution::OperationFactoryTraits<MyOperation>::getInstance() );
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<GDALinfo>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<RasterBandSummary>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<RastertoImage>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<SlopeCalc>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<AspectCalc>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<CalcCurvature>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<DebrisEmergence>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<Create3dModel>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<FillDepressions>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<FlowRouting>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<AngletoVectorState>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<UpslopeArea>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<SubsetData>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<ResetNoData>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<FD8toTheta>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<RasterDifference>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<RasterSum>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<IversonFailureVolume>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<EnergyCone>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<Warp>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<ProjToOffset>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<ScaleRasterValues>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<Smooth>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<BoxFilter>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<MultiplyRasters>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<UplsopeFailureVolume>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<GetPixelValue>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<TotalUpstreamProperty>::getInstance());
        addFactory(CSIRO::DataExecution::OperationFactoryTraits<EllipticalPile>::getInstance());

        // Add your widget factories like this:
        //addFactory( MyNamespace::MyWidgetFactory::getInstance() );
        static CSIRO::Widgets::EnumComboBoxFactory<RF::SlopeAlgType> slopeAlgWidgetFact;
        addFactory(slopeAlgWidgetFact);
        static CSIRO::Widgets::EnumComboBoxFactory<RF::CurvatureType> curvatureTypeWidgetFact;
        addFactory(curvatureTypeWidgetFact);
        static CSIRO::Widgets::EnumComboBoxFactory<RF::FlowDirType> flowDirtTypeWidgetFact;
        addFactory(flowDirtTypeWidgetFact);

        return true;
    }


    /**
     *
     */
    const CSIRO::DataExecution::OperationFactory*  VolcanoPlugin::getAliasedOperationFactory(const QString& opType) const
    {
        // If you rename an operation, you can provide backwards
        // compatibility using something like this (don't forget to
        // include namespaces in the names if relevant):
        //if (opType == "SomeOperationName")
        //    return &CSIRO::DataExecution::OperationFactoryTraits<NewOperationName>::getInstance();

        // If you make use of opType, you can delete the following Q_UNUSED line
        Q_UNUSED(opType);

        // If we get here, opType is not something we renamed, so return a
        // a null pointer to tell the caller
        return static_cast<const CSIRO::DataExecution::OperationFactory*>(0);
    }


    /**
     *
     */
    const CSIRO::DataExecution::DataFactory*  VolcanoPlugin::getAliasedDataFactory(const QString& dataType) const
    {
        // If you rename a data type, you can provide backwards
        // compatibility using something like this (don't forget to
        // include namespaces in the names if relevant):
        //if (dataType == "SomeDataType")
        //    return &CSIRO::DataExecution::DataFactoryTraits<NewDataType>::getInstance();

        // If you make use of dataType, you can delete the following Q_UNUSED line
        Q_UNUSED(dataType);

        // If we get here, dataType is not something we renamed, so return a
        // a null pointer to tell the caller
        return static_cast<const CSIRO::DataExecution::DataFactory*>(0);
    }
    
    
    /**
     *
     */
    QStringList  VolcanoPlugin::getCustomWidgetPaths() const
    {
        QStringList result;
        result.push_back("widgets:Volcano Analysis");
        return result;
    }
    
}

#ifndef CSIRO_STATIC_BUILD
extern "C"
{
    CSIRO_EXPORTSPEC CSIRO::Application::WorkspacePlugin* getWorkspacePlugin()
    {
        return &RF::VolcanoPlugin::getInstance();
    }
    
    /**
     *	\return The version string for the Workspace build we've been built against
     */
    CSIRO_EXPORTSPEC const char* builtAgainstWorkspace()
    {
        #define STRINGIFY(x) #x
        #define TOSTRING(x) STRINGIFY(x)
        return TOSTRING(CSIRO_WORKSPACE_VERSION_CHECK);
    }
}

DEFINE_WORKSPACE_DATA_FACTORY(GDALDatasetH, RF::VolcanoPlugin::getInstance())

#endif
