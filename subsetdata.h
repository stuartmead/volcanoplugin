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

#ifndef RF_SUBSETDATA_H
#define RF_SUBSETDATA_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
    class SubsetDataImpl;

    /**
     * \brief Subsets data using GDAL extraction
     *
     */
    class RF_API SubsetData : public CSIRO::DataExecution::Operation
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::SubsetData)

        SubsetDataImpl*  pImpl_;

        // Prevent copy and assignment - these should not be implemented
        SubsetData(const SubsetData&);
        SubsetData& operator=(const SubsetData&);

    protected:
        virtual bool  execute();

    public:
        SubsetData();
        virtual ~SubsetData();
    };
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::SubsetData, RF_API)

#endif

