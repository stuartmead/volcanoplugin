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

#ifndef RF_RASTERBANDSUMMARY_H
#define RF_RASTERBANDSUMMARY_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class RasterBandSummaryImpl;

    /**
     * \brief Returns summary of raster data (inc. histograms)
     *
     */
    class RF_API RasterBandSummary : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RasterBandSummary)

        RasterBandSummaryImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        RasterBandSummary(const RasterBandSummary&);
        RasterBandSummary& operator=(const RasterBandSummary&);

    protected:
        virtual bool  execute();

    public:
        RasterBandSummary();
        virtual ~RasterBandSummary();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::RasterBandSummary, RF_API)

#endif

