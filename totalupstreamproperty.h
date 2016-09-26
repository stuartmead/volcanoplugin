/**
 * \file
 */

#ifndef RF_TOTALUPSTREAMPROPERTY_H
#define RF_TOTALUPSTREAMPROPERTY_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class TotalUpstreamPropertyImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API TotalUpstreamProperty : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::TotalUpstreamProperty)

        TotalUpstreamPropertyImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        TotalUpstreamProperty(const TotalUpstreamProperty&);
        TotalUpstreamProperty& operator=(const TotalUpstreamProperty&);

    protected:
        virtual bool  execute();

    public:
        TotalUpstreamProperty();
        virtual ~TotalUpstreamProperty();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::TotalUpstreamProperty, RF_API)

#endif

