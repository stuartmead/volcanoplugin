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

#ifndef RF_FILLDEPRESSIONS_H
#define RF_FILLDEPRESSIONS_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class FillDepressionsImpl;

    /**
     * \brief Fills 1 cell depressions in rasters.
     *
     */
    class RF_API FillDepressions : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::FillDepressions)

        FillDepressionsImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        FillDepressions(const FillDepressions&);
        FillDepressions& operator=(const FillDepressions&);

    protected:
        virtual bool  execute();

    public:
        FillDepressions();
        virtual ~FillDepressions();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::FillDepressions, RF_API)

#endif

