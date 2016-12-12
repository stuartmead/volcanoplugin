/**
 * \file
 */

#ifndef RF_ELLIPTICALPILE_H
#define RF_ELLIPTICALPILE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class EllipticalPileImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API EllipticalPile : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EllipticalPile)

        EllipticalPileImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        EllipticalPile(const EllipticalPile&);
        EllipticalPile& operator=(const EllipticalPile&);

    protected:
        virtual bool  execute();

    public:
        EllipticalPile();
        virtual ~EllipticalPile();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::EllipticalPile, RF_API)

#endif

