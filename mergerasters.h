/**
 * \file
 */

#ifndef RF_MERGERASTERS_H
#define RF_MERGERASTERS_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class MergeRastersImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API MergeRasters : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::MergeRasters)

        MergeRastersImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        MergeRasters(const MergeRasters&);
        MergeRasters& operator=(const MergeRasters&);

    protected:
        virtual bool  execute();

    public:
        MergeRasters();
        virtual ~MergeRasters();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::MergeRasters, RF_API)

#endif

