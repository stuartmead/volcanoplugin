/**
 * \file
 */

#ifndef RF_VTIREADER_H
#define RF_VTIREADER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class VTIReaderImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API VTIReader : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::VTIReader)

        VTIReaderImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        VTIReader(const VTIReader&);
        VTIReader& operator=(const VTIReader&);

    protected:
        virtual bool  execute();

    public:
        VTIReader();
        virtual ~VTIReader();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::VTIReader, RF_API)

#endif

