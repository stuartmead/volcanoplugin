/**
 * \file
 */

#ifndef RF_SCALERASTERVALUES_H
#define RF_SCALERASTERVALUES_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ScaleRasterValuesImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API ScaleRasterValues : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScaleRasterValues)

        ScaleRasterValuesImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ScaleRasterValues(const ScaleRasterValues&);
        ScaleRasterValues& operator=(const ScaleRasterValues&);

    protected:
        virtual bool  execute();

    public:
        ScaleRasterValues();
        virtual ~ScaleRasterValues();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ScaleRasterValues, RF_API)

#endif

