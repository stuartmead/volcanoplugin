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

#ifndef RF_ENERGYCONE_H
#define RF_ENERGYCONE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class EnergyConeImpl;

    /**
     * \brief Calculates Sheridan's energy cone.
     *
     */
    class RF_API EnergyCone : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::EnergyCone)

        EnergyConeImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        EnergyCone(const EnergyCone&);
        EnergyCone& operator=(const EnergyCone&);

    protected:
        virtual bool  execute();

    public:
        EnergyCone();
        virtual ~EnergyCone();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::EnergyCone, RF_API)

#endif

