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

#ifndef RF_RASTERTOIMAGE_H
#define RF_RASTERTOIMAGE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class RastertoImageImpl;

    /**
     * \brief Converts raster to QImage
     *
     */
    class RF_API RastertoImage : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RastertoImage)

        RastertoImageImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        RastertoImage(const RastertoImage&);
        RastertoImage& operator=(const RastertoImage&);

    protected:
        virtual bool  execute();

    public:
        RastertoImage();
        virtual ~RastertoImage();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::RastertoImage, RF_API)

#endif

