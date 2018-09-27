/*
/*
  Created by: Stuart Mead
  Creation date: $Date$
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

/**
 * \file
 */

#ifndef RF_DEFORMTOSPHERE_H
#define RF_DEFORMTOSPHERE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class DeformToSphereImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API DeformToSphere : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::DeformToSphere)

        DeformToSphereImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        DeformToSphere(const DeformToSphere&);
        DeformToSphere& operator=(const DeformToSphere&);

    protected:
        virtual bool  execute();

    public:
        DeformToSphere();
        virtual ~DeformToSphere();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::DeformToSphere, RF_API)

#endif

