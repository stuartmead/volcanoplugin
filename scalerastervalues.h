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

#ifndef RF_SCALERASTERVALUES_H
#define RF_SCALERASTERVALUES_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ScaleRasterValuesImpl;

    /**
     * \brief Rescales raster values. 
     *
     */
    class RF_API ScaleRasterValues : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScaleRasterValues)

        ScaleRasterValuesImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ScaleRasterValues(const ScaleRasterValues&);
        ScaleRasterValues& operator=(const ScaleRasterValues&);

    protected:
        virtual bool  execute();

    public:
        ScaleRasterValues();
        virtual ~ScaleRasterValues();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ScaleRasterValues, RF_API)

#endif

