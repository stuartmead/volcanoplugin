/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/

#include <cassert>
#include <iostream>

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/DataObjects/typeddatafactory.h"


#include "volcanoplugin.h"
#include "boundsofraster.h"


namespace RF
{
    using namespace RF;
    using namespace CSIRO::DataExecution;

    /**
     * \internal
     */
    class BoundsofRasterImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::BoundsofRasterImpl)

        void setObjects();

    public:
        BoundsofRaster&  owner_;

        // Data objects
        TypedObject< QString >  coordinateSystem_;
        TypedObject< double >   northernBound_;
        TypedObject< double >   easternBound_;
        TypedObject< double >   southernBound_;
        TypedObject< double >   westernBound_;
        TypedObject< double >   eWResolution_;
        TypedObject< double >   nSResolution_;


        BoundsofRasterImpl(BoundsofRaster& owner);
        BoundsofRasterImpl(BoundsofRaster& owner, const BoundsofRasterImpl& other);
    };


    /**
     *
     */
    BoundsofRasterImpl::BoundsofRasterImpl(BoundsofRaster& owner) :
        owner_(owner),
        coordinateSystem_(),
        northernBound_(),
        easternBound_(),
        southernBound_(),
        westernBound_(),
        eWResolution_(),
        nSResolution_()
    {
        setObjects();
    }


    /**
     *
     */
    BoundsofRasterImpl::BoundsofRasterImpl(BoundsofRaster& owner, const BoundsofRasterImpl& other) :
        owner_(owner),
        coordinateSystem_(other.coordinateSystem_),
        northernBound_(other.northernBound_),
        easternBound_(other.easternBound_),
        southernBound_(other.southernBound_),
        westernBound_(other.westernBound_),
        eWResolution_(other.eWResolution_),
        nSResolution_(other.nSResolution_)
    {
        setObjects();
    }


    /**
     *
     */
    void  BoundsofRasterImpl::setObjects()
    {
        owner_.add("Coordinate System", coordinateSystem_);
        owner_.add("Northern Bound", northernBound_);
        owner_.add("Eastern Bound", easternBound_);
        owner_.add("Southern Bound", southernBound_);
        owner_.add("Western Bound", westernBound_);
        owner_.add("EW Resolution", eWResolution_);
        owner_.add("NS Resolution", nSResolution_);

    }


    //==========================//


    /**
     *
     */
    BoundsofRaster::BoundsofRaster() :
        CSIRO::DataExecution::ObjectGroup()
    {
        pImpl_ = new BoundsofRasterImpl(*this);
    }


    /**
     *
     */
    BoundsofRaster::BoundsofRaster(const BoundsofRaster& other) :
        CSIRO::DataExecution::ObjectGroup()
    {
        pImpl_ = new BoundsofRasterImpl(*this, *other.pImpl_);
    }


    /**
     *
     */
    BoundsofRaster::~BoundsofRaster()
    {
        delete pImpl_;
    }


    /**
     * Cloning
     */
    BoundsofRaster* BoundsofRaster::clone() const
    {
        return new BoundsofRaster(*this);
    }


    /**
     * Comparison
     */
    bool BoundsofRaster::operator==(const BoundsofRaster& rhs) const
    {
        if (&rhs == this)
            return true;

        if ( !(*pImpl_->coordinateSystem_ == *rhs.pImpl_->coordinateSystem_) ) { return false; }
        if ( !(*pImpl_->northernBound_ == *rhs.pImpl_->northernBound_) ) { return false; }
        if ( !(*pImpl_->easternBound_ == *rhs.pImpl_->easternBound_) ) { return false; }
        if ( !(*pImpl_->southernBound_ == *rhs.pImpl_->southernBound_) ) { return false; }
        if ( !(*pImpl_->westernBound_ == *rhs.pImpl_->westernBound_) ) { return false; }
        if ( !(*pImpl_->eWResolution_ == *rhs.pImpl_->eWResolution_) ) { return false; }
        if ( !(*pImpl_->nSResolution_ == *rhs.pImpl_->nSResolution_) ) { return false; }

        return true;
    }


    /**
     * Assignment
     */
    BoundsofRaster& BoundsofRaster::operator=(const BoundsofRaster& rhs)
    {
        // Check for self assignment
        if (&rhs == this)
            return *this;

        // Clear the current contents; we're about to delete the impl, so we
        // don't want any dangling pointers to our dataobjects.
        clear();

        BoundsofRasterImpl* impl = pImpl_;
        pImpl_ = new BoundsofRasterImpl(*this, *rhs.pImpl_);
        delete impl;
        impl = 0;

        return *this;
    }


    /**
     *
     */
    const QString&  BoundsofRaster::getCoordinateSystem() const
    {
        return *pImpl_->coordinateSystem_;
    }


    /**
     *
     */
    void  BoundsofRaster::setCoordinateSystem(const QString& coordinateSystem)
    {
        *pImpl_->coordinateSystem_ = coordinateSystem;
    }


    /**
     *
     */
    double  BoundsofRaster::getNorthernBound() const
    {
        return *pImpl_->northernBound_;
    }


    /**
     *
     */
    void  BoundsofRaster::setNorthernBound(double northernBound)
    {
        *pImpl_->northernBound_ = northernBound;
    }


    /**
     *
     */
    double  BoundsofRaster::getEasternBound() const
    {
        return *pImpl_->easternBound_;
    }


    /**
     *
     */
    void  BoundsofRaster::setEasternBound(double easternBound)
    {
        *pImpl_->easternBound_ = easternBound;
    }


    /**
     *
     */
    double  BoundsofRaster::getSouthernBound() const
    {
        return *pImpl_->southernBound_;
    }


    /**
     *
     */
    void  BoundsofRaster::setSouthernBound(double southernBound)
    {
        *pImpl_->southernBound_ = southernBound;
    }


    /**
     *
     */
    double  BoundsofRaster::getWesternBound() const
    {
        return *pImpl_->westernBound_;
    }


    /**
     *
     */
    void  BoundsofRaster::setWesternBound(double westernBound)
    {
        *pImpl_->westernBound_ = westernBound;
    }


    /**
     *
     */
    double  BoundsofRaster::getEWResolution() const
    {
        return *pImpl_->eWResolution_;
    }


    /**
     *
     */
    void  BoundsofRaster::setEWResolution(double eWResolution)
    {
        *pImpl_->eWResolution_ = eWResolution;
    }


    /**
     *
     */
    double  BoundsofRaster::getNSResolution() const
    {
        return *pImpl_->nSResolution_;
    }


    /**
     *
     */
    void  BoundsofRaster::setNSResolution(double nSResolution)
    {
        *pImpl_->nSResolution_ = nSResolution;
    }


}


DEFINE_WORKSPACE_DATA_FACTORY(RF::BoundsofRaster, RF::VolcanoPlugin::getInstance())
DEFINE_WORKSPACE_DERIVEDTOBASEADAPTOR(RF::BoundsofRaster, CSIRO::DataExecution::ObjectGroup, RF::VolcanoPlugin::getInstance())


