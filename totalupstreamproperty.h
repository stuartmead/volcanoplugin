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

#ifndef RF_TOTALUPSTREAMPROPERTY_H
#define RF_TOTALUPSTREAMPROPERTY_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class TotalUpstreamPropertyImpl;

    /**
     * \brief Calculates upstream value of any arbitrary property.
     *
     */
    class RF_API TotalUpstreamProperty : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::TotalUpstreamProperty)

        TotalUpstreamPropertyImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        TotalUpstreamProperty(const TotalUpstreamProperty&);
        TotalUpstreamProperty& operator=(const TotalUpstreamProperty&);

    protected:
        virtual bool  execute();

    public:
        TotalUpstreamProperty();
        virtual ~TotalUpstreamProperty();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::TotalUpstreamProperty, RF_API)

#endif

