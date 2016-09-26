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

#ifndef RF_ASPECTCALC_H
#define RF_ASPECTCALC_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class AspectCalcImpl;

    /**
     * \brief Calculates aspect of a slope - makes cool hillshadey effects.
     *
     */
    class RF_API AspectCalc : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::AspectCalc)

        AspectCalcImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        AspectCalc(const AspectCalc&);
        AspectCalc& operator=(const AspectCalc&);

    protected:
        virtual bool  execute();

    public:
        AspectCalc();
        virtual ~AspectCalc();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::AspectCalc, RF_API)

#endif

