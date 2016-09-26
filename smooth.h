/**
 * \file
 */

#ifndef RF_SMOOTH_H
#define RF_SMOOTH_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class SmoothImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API Smooth : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::Smooth)

        SmoothImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        Smooth(const Smooth&);
        Smooth& operator=(const Smooth&);

    protected:
        virtual bool  execute();

    public:
        Smooth();
        virtual ~Smooth();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::Smooth, RF_API)

#endif

