/**
 * \file
 */

#ifndef RF_MULTIPLYRASTERS_H
#define RF_MULTIPLYRASTERS_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class MultiplyRastersImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API MultiplyRasters : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::MultiplyRasters)

        MultiplyRastersImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        MultiplyRasters(const MultiplyRasters&);
        MultiplyRasters& operator=(const MultiplyRasters&);

    protected:
        virtual bool  execute();

    public:
        MultiplyRasters();
        virtual ~MultiplyRasters();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::MultiplyRasters, RF_API)

#endif

