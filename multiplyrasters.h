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

#ifndef RF_MULTIPLYRASTERS_H
#define RF_MULTIPLYRASTERS_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class MultiplyRastersImpl;

    /**
     * \brief Multiplies two rasters together.
     *
     */
    class RF_API MultiplyRasters : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::MultiplyRasters)

        MultiplyRastersImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        MultiplyRasters(const MultiplyRasters&);
        MultiplyRasters& operator=(const MultiplyRasters&);

    protected:
        virtual bool  execute();

    public:
        MultiplyRasters();
        virtual ~MultiplyRasters();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::MultiplyRasters, RF_API)

#endif

