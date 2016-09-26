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

#ifndef RF_IVERSONFAILUREVOLUME_H
#define RF_IVERSONFAILUREVOLUME_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class IversonFailureVolumeImpl;

    /**
     * \brief Calculates failure depth using the Iverson (2000) method.
     *
     */
    class RF_API IversonFailureVolume : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::IversonFailureVolume)

        IversonFailureVolumeImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        IversonFailureVolume(const IversonFailureVolume&);
        IversonFailureVolume& operator=(const IversonFailureVolume&);

    protected:
        virtual bool  execute();

    public:
        IversonFailureVolume();
        virtual ~IversonFailureVolume();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::IversonFailureVolume, RF_API)

#endif

