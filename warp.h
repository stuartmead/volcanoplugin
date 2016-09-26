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

#ifndef RF_WARP_H
#define RF_WARP_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class WarpImpl;

    /**
     * \brief Warp a dataset using GDAL
     *
     */
    class RF_API Warp : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::Warp)

        WarpImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        Warp(const Warp&);
        Warp& operator=(const Warp&);

    protected:
        virtual bool  execute();

    public:
        Warp();
        virtual ~Warp();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::Warp, RF_API)

#endif

