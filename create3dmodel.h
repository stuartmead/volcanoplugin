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

#ifndef RF_CREATE3DMODEL_H
#define RF_CREATE3DMODEL_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class Create3dModelImpl;

    /**
     * \brief Create a 3d model from raster file.
     *
     */
    class RF_API Create3dModel : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::Create3dModel)

        Create3dModelImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        Create3dModel(const Create3dModel&);
        Create3dModel& operator=(const Create3dModel&);

    protected:
        virtual bool  execute();

    public:
        Create3dModel();
        virtual ~Create3dModel();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::Create3dModel, RF_API)

#endif

