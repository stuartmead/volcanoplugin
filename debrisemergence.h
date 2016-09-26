/*
Created by: Stuart Mead
  Creation date: 2014-02-03
   
  Revision:       $Revision: $
  Last changed:   $Date: $
  Last changed by: Stuart Mead

  Copyright Risk Frontiers 2014, Faculty of Science, Macquarie University, NSW 2109, Australia.

  For further information, contact:
          Stuart Mead
          Building E7A
          Dept. of Environment & Geography
          Macquarie University
          North Ryde NSW 2109

  This copyright notice must be included with all copies of the source code.
*/

/**
 * \file
 */

#ifndef RF_DEBRISEMERGENCE_H
#define RF_DEBRISEMERGENCE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class DebrisEmergenceImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API DebrisEmergence : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::DebrisEmergence)

        DebrisEmergenceImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        DebrisEmergence(const DebrisEmergence&);
        DebrisEmergence& operator=(const DebrisEmergence&);

    protected:
        virtual bool  execute();

    public:
        DebrisEmergence();
        virtual ~DebrisEmergence();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::DebrisEmergence, RF_API)

#endif

