
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

/**
 * \file
 */

#ifndef RF_VOLCANOPLUGIN_H
#define RF_VOLCANOPLUGIN_H

#define EXPORT_GDAL_DATATYPES

#include "Workspace/Application/workspaceplugin.h"

#include "gdal.h"

#include "volcanoplugin_api.h"


namespace RF
{
    class VolcanoPluginImpl;

    /**
     * \brief Workspace plugin containing GIS utility programs for lahar/volcano research
     *
     */
    class RF_API VolcanoPlugin : public CSIRO::Application::WorkspacePlugin
    {
        VolcanoPluginImpl*  pImpl_;

        VolcanoPlugin();
        ~VolcanoPlugin();

        // Prevent copying and assignment
        VolcanoPlugin(const VolcanoPlugin&);
        VolcanoPlugin& operator=(const VolcanoPlugin&);

    protected:
        virtual const CSIRO::DataExecution::DataFactory*       getAliasedDataFactory(const QString& dataType) const;
        virtual const CSIRO::DataExecution::OperationFactory*  getAliasedOperationFactory(const QString& opType) const;

    public:
        static VolcanoPlugin&  getInstance();
        
        QStringList  getCustomWidgetPaths() const;

        virtual bool  setup();
    };
}

DECLARE_WORKSPACE_DATA_FACTORY(GDALDatasetH, EXPORT_GDAL_DATATYPES)

#endif
