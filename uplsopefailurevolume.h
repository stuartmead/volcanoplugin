/**
 * \file
 */

#ifndef RF_UPLSOPEFAILUREVOLUME_H
#define RF_UPLSOPEFAILUREVOLUME_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class UplsopeFailureVolumeImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API UplsopeFailureVolume : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UplsopeFailureVolume)

        UplsopeFailureVolumeImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        UplsopeFailureVolume(const UplsopeFailureVolume&);
        UplsopeFailureVolume& operator=(const UplsopeFailureVolume&);

    protected:
        virtual bool  execute();

    public:
        UplsopeFailureVolume();
        virtual ~UplsopeFailureVolume();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::UplsopeFailureVolume, RF_API)

#endif

