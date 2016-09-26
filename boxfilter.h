/**
 * \file
 */

#ifndef RF_BOXFILTER_H
#define RF_BOXFILTER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class BoxFilterImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API BoxFilter : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::BoxFilter)

        BoxFilterImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        BoxFilter(const BoxFilter&);
        BoxFilter& operator=(const BoxFilter&);

    protected:
        virtual bool  execute();

    public:
        BoxFilter();
        virtual ~BoxFilter();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::BoxFilter, RF_API)

#endif

