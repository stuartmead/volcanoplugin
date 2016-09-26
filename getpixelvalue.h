/**
 * \file
 */

#ifndef RF_GETPIXELVALUE_H
#define RF_GETPIXELVALUE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class GetPixelValueImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API GetPixelValue : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::GetPixelValue)

        GetPixelValueImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        GetPixelValue(const GetPixelValue&);
        GetPixelValue& operator=(const GetPixelValue&);

    protected:
        virtual bool  execute();

    public:
        GetPixelValue();
        virtual ~GetPixelValue();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::GetPixelValue, RF_API)

#endif

