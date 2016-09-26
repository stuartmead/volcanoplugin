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

#ifndef RF_RASTERSUM_H
#define RF_RASTERSUM_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class RasterSumImpl;

    /**
     * \brief Add up two rasters.
     *
     */
    class RF_API RasterSum : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RasterSum)

        RasterSumImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        RasterSum(const RasterSum&);
        RasterSum& operator=(const RasterSum&);

    protected:
        virtual bool  execute();

    public:
        RasterSum();
        virtual ~RasterSum();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::RasterSum, RF_API)

#endif

