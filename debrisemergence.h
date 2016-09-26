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

#ifndef RF_DEBRISEMERGENCE_H
#define RF_DEBRISEMERGENCE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class DebrisEmergenceImpl;

    /**
     * \brief Takashi's debris flow emergence criteria - not sure this is right.
     *
     */
    class RF_API DebrisEmergence : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::DebrisEmergence)

        DebrisEmergenceImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        DebrisEmergence(const DebrisEmergence&);
        DebrisEmergence& operator=(const DebrisEmergence&);

    protected:
        virtual bool  execute();

    public:
        DebrisEmergence();
        virtual ~DebrisEmergence();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::DebrisEmergence, RF_API)

#endif

