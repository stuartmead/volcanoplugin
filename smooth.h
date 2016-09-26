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

#ifndef RF_SMOOTH_H
#define RF_SMOOTH_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class SmoothImpl;

    /**
     * \brief Smooths terrain models using OpenCV algos.
     *
     */
    class RF_API Smooth : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::Smooth)

        SmoothImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        Smooth(const Smooth&);
        Smooth& operator=(const Smooth&);

    protected:
        virtual bool  execute();

    public:
        Smooth();
        virtual ~Smooth();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::Smooth, RF_API)

#endif

