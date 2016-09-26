
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
   
  Revision:       $Revision: $
  Last changed:   $Date: $
  Last changed by: Stuart Mead

  Copyright Risk Frontiers 2014, Faculty of Science, Macquarie University, NSW 2109, Australia.

  For further information, contact:
          Stuart Mead
          Building E7A
          Dept. of Environment & Geography
          Macquarie University
          North Ryde NSW 2109

  This copyright notice must be included with all copies of the source code.

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
     * \brief Put a one-line description of your plugin here
     *
     * Add a more detailed description of your plugin here
     * or remove these lines if the brief description above
     * is sufficient.
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
