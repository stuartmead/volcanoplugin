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

#ifndef RF_SCOOPREADER_H
#define RF_SCOOPREADER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class ScoopReaderImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API ScoopReader : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScoopReader)

        ScoopReaderImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        ScoopReader(const ScoopReader&);
        ScoopReader& operator=(const ScoopReader&);

    protected:
        virtual bool  execute();

    public:
        ScoopReader();
        virtual ~ScoopReader();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::ScoopReader, RF_API)

#endif

