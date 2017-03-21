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

#ifndef RF_TITANH5READER_H
#define RF_TITANH5READER_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class TitanH5ReaderImpl;

    /**
     * \brief Put a one-line description of your operation here
     *
     * Add a more detailed description of your operation here
     * or remove these lines if the brief description above
     * is sufficient.
     */
    class RF_API TitanH5Reader : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::TitanH5Reader)

        TitanH5ReaderImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        TitanH5Reader(const TitanH5Reader&);
        TitanH5Reader& operator=(const TitanH5Reader&);

    protected:
        virtual bool  execute();

    public:
        TitanH5Reader();
        virtual ~TitanH5Reader();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::TitanH5Reader, RF_API)

#endif

