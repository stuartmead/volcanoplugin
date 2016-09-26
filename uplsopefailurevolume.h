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

#ifndef RF_UPLSOPEFAILUREVOLUME_H
#define RF_UPLSOPEFAILUREVOLUME_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class UplsopeFailureVolumeImpl;

    /**
     * \brief Calculates total upstream volume of failure from a failure depth raster.
     *
     */
    class RF_API UplsopeFailureVolume : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UplsopeFailureVolume)

        UplsopeFailureVolumeImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        UplsopeFailureVolume(const UplsopeFailureVolume&);
        UplsopeFailureVolume& operator=(const UplsopeFailureVolume&);

    protected:
        virtual bool  execute();

    public:
        UplsopeFailureVolume();
        virtual ~UplsopeFailureVolume();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::UplsopeFailureVolume, RF_API)

#endif

