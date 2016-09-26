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

#ifndef RF_CALCCURVATURE_H
#define RF_CALCCURVATURE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class CalcCurvatureImpl;

    /**
     * \brief Calculate curvature of a terrain.
     *
     */
    class RF_API CalcCurvature : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::CalcCurvature)

        CalcCurvatureImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        CalcCurvature(const CalcCurvature&);
        CalcCurvature& operator=(const CalcCurvature&);

    protected:
        virtual bool  execute();

    public:
        CalcCurvature();
        virtual ~CalcCurvature();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::CalcCurvature, RF_API)

#endif

