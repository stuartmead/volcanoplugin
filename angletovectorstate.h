/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

/**
 * \file
 */

#ifndef RF_ANGLETOVECTORSTATE_H
#define RF_ANGLETOVECTORSTATE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class AngletoVectorStateImpl;

    /**
     * \brief Converts an angle to a vector state.
     *
     */
    class RF_API AngletoVectorState : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::AngletoVectorState)

        AngletoVectorStateImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        AngletoVectorState(const AngletoVectorState&);
        AngletoVectorState& operator=(const AngletoVectorState&);

    protected:
        virtual bool  execute();

    public:
        AngletoVectorState();
        virtual ~AngletoVectorState();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::AngletoVectorState, RF_API)

#endif

