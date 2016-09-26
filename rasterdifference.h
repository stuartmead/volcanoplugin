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

#ifndef RF_RASTERDIFFERENCE_H
#define RF_RASTERDIFFERENCE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class RasterDifferenceImpl;

    /**
     * \brief Subtract two rasters
     *
     */
    class RF_API RasterDifference : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RasterDifference)

        RasterDifferenceImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        RasterDifference(const RasterDifference&);
        RasterDifference& operator=(const RasterDifference&);

    protected:
        virtual bool  execute();

    public:
        RasterDifference();
        virtual ~RasterDifference();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::RasterDifference, RF_API)

#endif

