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

#ifndef RF_GDALINFO_H
#define RF_GDALINFO_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"

namespace RF
{
    class GDALinfoImpl;

    /**
     * \brief Gets info of a raster file using GDALinfo
     *
     */
    class RF_API GDALinfo : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::GDALinfo)

        GDALinfoImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        GDALinfo(const GDALinfo&);
        GDALinfo& operator=(const GDALinfo&);

    protected:
        virtual bool  execute();

    public:
        GDALinfo();
        virtual ~GDALinfo();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::GDALinfo, RF_API)

#endif

