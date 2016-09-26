/**
 * \file
 */

#ifndef RF_PROJTOOFFSET_H
#define RF_PROJTOOFFSET_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ProjToOffsetImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API ProjToOffset : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ProjToOffset)

        ProjToOffsetImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ProjToOffset(const ProjToOffset&);
        ProjToOffset& operator=(const ProjToOffset&);

    protected:
        virtual bool  execute();

    public:
        ProjToOffset();
        virtual ~ProjToOffset();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ProjToOffset, RF_API)

#endif

