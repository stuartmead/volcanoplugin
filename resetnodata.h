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

#ifndef RF_RESETNODATA_H
#define RF_RESETNODATA_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ResetNoDataImpl;

    /**
     * \brief Change raster NODATA spec.
     *
     */
    class RF_API ResetNoData : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ResetNoData)

        ResetNoDataImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ResetNoData(const ResetNoData&);
        ResetNoData& operator=(const ResetNoData&);

    protected:
        virtual bool  execute();

    public:
        ResetNoData();
        virtual ~ResetNoData();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ResetNoData, RF_API)

#endif

