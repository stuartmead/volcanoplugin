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

#ifndef RF_UPSLOPEAREA_H
#define RF_UPSLOPEAREA_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class UpslopeAreaImpl;

    /**
     * \brief Calculate upslope area from a cell
     *
     */
    class RF_API UpslopeArea : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UpslopeArea)

        UpslopeAreaImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        UpslopeArea(const UpslopeArea&);
        UpslopeArea& operator=(const UpslopeArea&);

    protected:
        virtual bool  execute();

    public:
        UpslopeArea();
        virtual ~UpslopeArea();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::UpslopeArea, RF_API)

#endif

