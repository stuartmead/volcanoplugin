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
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
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

