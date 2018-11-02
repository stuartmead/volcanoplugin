/**
 * \file
 */

#ifndef RF_ELLIPSEPROPERTIES_H
#define RF_ELLIPSEPROPERTIES_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class EllipsePropertiesImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API EllipseProperties : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EllipseProperties)

        EllipsePropertiesImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        EllipseProperties(const EllipseProperties&);
        EllipseProperties& operator=(const EllipseProperties&);

    protected:
        virtual bool  execute();

    public:
        EllipseProperties();
        virtual ~EllipseProperties();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::EllipseProperties, RF_API)

#endif

