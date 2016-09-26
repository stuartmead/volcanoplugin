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

#ifndef RF_PROJTOOFFSET_H
#define RF_PROJTOOFFSET_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ProjToOffsetImpl;

    /**
     * \brief Changes the GDAL projection stuff I think?
     *
     */
    class RF_API ProjToOffset : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ProjToOffset)

        ProjToOffsetImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ProjToOffset(const ProjToOffset&);
        ProjToOffset& operator=(const ProjToOffset&);

    protected:
        virtual bool  execute();

    public:
        ProjToOffset();
        virtual ~ProjToOffset();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ProjToOffset, RF_API)

#endif

