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

#ifndef RF_BOXFILTER_H
#define RF_BOXFILTER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class BoxFilterImpl;

    /**
     * \brief Run an OpenCV box filter across data.
     *
     */
    class RF_API BoxFilter : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::BoxFilter)

        BoxFilterImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        BoxFilter(const BoxFilter&);
        BoxFilter& operator=(const BoxFilter&);

    protected:
        virtual bool  execute();

    public:
        BoxFilter();
        virtual ~BoxFilter();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::BoxFilter, RF_API)

#endif

