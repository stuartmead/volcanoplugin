/**
 * \file
 */

#ifndef RF_SCOOPCOUNTER_H
#define RF_SCOOPCOUNTER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ScoopCounterImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API ScoopCounter : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScoopCounter)

        ScoopCounterImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ScoopCounter(const ScoopCounter&);
        ScoopCounter& operator=(const ScoopCounter&);

    protected:
        virtual bool  execute();

    public:
        ScoopCounter();
        virtual ~ScoopCounter();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ScoopCounter, RF_API)

#endif

