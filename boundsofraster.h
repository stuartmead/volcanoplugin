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

#ifndef RF_BOUNDSOFRASTER_H
#define RF_BOUNDSOFRASTER_H

#include <QCoreApplication>

#include "Workspace/DataExecution/DataObjects/datafactorytraits.h"
#include "Workspace/DataExecution/DataObjects/objectgroup.h"
#include "Workspace/DataExecution/DataObjects/derivedtobaseadaptor.h"

#include "volcanoplugin.h"


// Forward declarations so that our get / set functions work.


namespace RF
{
    class BoundsofRasterImpl;

    /**
     * \brief Determines the bounds of the raster.
     *
     */
    class RF_API BoundsofRaster : public CSIRO::DataExecution::ObjectGroup
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::BoundsofRaster)

        BoundsofRasterImpl*  pImpl_;

    public:
        BoundsofRaster();
        BoundsofRaster(const BoundsofRaster& other);
        virtual ~BoundsofRaster();

        // Clones our data type
        virtual BoundsofRaster* clone() const;

        // Optional, but handy operators
        bool operator==(const BoundsofRaster& rhs) const;
        BoundsofRaster& operator=(const BoundsofRaster& rhs);

        // Get / set functions for modifying data members in code
        const QString& getCoordinateSystem() const;
        void setCoordinateSystem(const QString& coordinateSystem);

        double getNorthernBound() const;
        void setNorthernBound(double northernBound);

        double getEasternBound() const;
        void setEasternBound(double easternBound);

        double getSouthernBound() const;
        void setSouthernBound(double southernBound);

        double getWesternBound() const;
        void setWesternBound(double westernBound);

        double getEWResolution() const;
        void setEWResolution(double eWResolution);

        double getNSResolution() const;
        void setNSResolution(double nSResolution);


    };
}

DECLARE_WORKSPACE_DATA_FACTORY(RF::BoundsofRaster, RF_API)
DECLARE_WORKSPACE_DERIVEDTOBASEADAPTOR(RF::BoundsofRaster, CSIRO::DataExecution::ObjectGroup, RF_API)

#endif

