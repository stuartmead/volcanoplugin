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

#ifndef RF_FLOWROUTING_H
#define RF_FLOWROUTING_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class FlowRoutingImpl;

    /**
     * \brief Runs flow routing algs. A bit broken.
     *
     */
    class RF_API FlowRouting : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FlowRouting)

        FlowRoutingImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        FlowRouting(const FlowRouting&);
        FlowRouting& operator=(const FlowRouting&);

    protected:
        virtual bool  execute();

    public:
        FlowRouting();
        virtual ~FlowRouting();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::FlowRouting, RF_API)

#endif

