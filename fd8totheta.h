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

#ifndef RF_FD8TOTHETA_H
#define RF_FD8TOTHETA_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class FD8toThetaImpl;

    /**
     * \brief Converts FD8 (8 flow direction rasters) into a flow angle.
     *
     */
    class RF_API FD8toTheta : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FD8toTheta)

        FD8toThetaImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        FD8toTheta(const FD8toTheta&);
        FD8toTheta& operator=(const FD8toTheta&);

    protected:
        virtual bool  execute();

    public:
        FD8toTheta();
        virtual ~FD8toTheta();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::FD8toTheta, RF_API)

#endif

