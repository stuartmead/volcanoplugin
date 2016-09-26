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

#ifndef RF_SLOPECALC_H
#define RF_SLOPECALC_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class SlopeCalcImpl;

    /**
     * \brief Calculates slope using GDAL algos.
     *
     */
    class RF_API SlopeCalc : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::SlopeCalc)

        SlopeCalcImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        SlopeCalc(const SlopeCalc&);
        SlopeCalc& operator=(const SlopeCalc&);

    protected:
        virtual bool  execute();

    public:
        SlopeCalc();
        virtual ~SlopeCalc();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::SlopeCalc, RF_API)

#endif

