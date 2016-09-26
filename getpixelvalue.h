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

#ifndef RF_GETPIXELVALUE_H
#define RF_GETPIXELVALUE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class GetPixelValueImpl;

    /**
     * \brief Gets value of one raster pixel/cell.
     *
     */
    class RF_API GetPixelValue : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::GetPixelValue)

        GetPixelValueImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        GetPixelValue(const GetPixelValue&);
        GetPixelValue& operator=(const GetPixelValue&);

    protected:
        virtual bool  execute();

    public:
        GetPixelValue();
        virtual ~GetPixelValue();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::GetPixelValue, RF_API)

#endif

