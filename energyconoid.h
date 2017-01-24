/*
Created by: Stuart Mead
Creation date: 2017-01-20

Released under BSD 3 clause.
Use it however you want, but I cannot guarantee it is right.
Also don't use my name, the name of collaborators and my/their affiliations
as endorsement.

*/

/**
 * \file
 */

#ifndef RF_ENERGYCONOID_H
#define RF_ENERGYCONOID_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class EnergyConoidImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API EnergyConoid : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EnergyConoid)

        EnergyConoidImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        EnergyConoid(const EnergyConoid&);
        EnergyConoid& operator=(const EnergyConoid&);

    protected:
        virtual bool  execute();

    public:
        EnergyConoid();
        virtual ~EnergyConoid();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::EnergyConoid, RF_API)

#endif

