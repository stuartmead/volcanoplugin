#include <cassert>
#include <iostream>
#include <fstream>

#include <qstring.h>
#include "Workspace/Application/LanguageUtils/streamqstring.h"

#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Mesh/DataStructures/MeshModelInterface/meshmodelinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshnodesinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshelementsinterface.h"
#include "Mesh/Geometry/boundingplane.h"
#include "Mesh/Geometry/boundingsphere.h"

#include "volcanoutils.h"

#include "ogr_spatialref.h"

#include "volcanoplugin.h"
#include "scoopcounter.h"


namespace RF
{
    /**
     * \internal
     */
    class ScoopCounterImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScoopCounterImpl)

    public:
        ScoopCounter&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< CSIRO::Mesh::BoundingPlane > boundingPlane1_;
        CSIRO::DataExecution::SimpleInput< CSIRO::Mesh::BoundingPlane > boundingPlane2_;
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > dEMDataset_;
        CSIRO::DataExecution::SimpleInput< CSIRO::Mesh::MeshModelInterface > scoopModel_;
        CSIRO::DataExecution::SimpleOutput< int > numberWithinRegion_;


        ScoopCounterImpl(ScoopCounter& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    ScoopCounterImpl::ScoopCounterImpl(ScoopCounter& op) :
        op_(op),
        boundingPlane1_("Bounding plane 1",  op_),
        boundingPlane2_("Bounding plane 2",  op_),
        dEMDataset_("DEM dataset",  op_),
        scoopModel_("Scoop model",  op_),
        numberWithinRegion_("Number within region",  op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input1_.input_.setDescription(tr("Used for such and such."));
        // output1_.output_.setDescription(tr("Results of the blah-di-blah."));
    }


    /**
     *
     */
    bool ScoopCounterImpl::execute()
    {
        const CSIRO::Mesh::BoundingPlane&      boundingPlane1     = *boundingPlane1_;
        const CSIRO::Mesh::BoundingPlane&      boundingPlane2     = *boundingPlane2_;
        const GDALDatasetH&                    dEMDataset         = *dEMDataset_;
        const CSIRO::Mesh::MeshModelInterface& scoopModel         = *scoopModel_;
        int&                             numberWithinRegion = *numberWithinRegion_;
        
		//Set number to zero
		numberWithinRegion = 0;

		GDALAllRegister();

		GDALRasterBandH demBand = GDALGetRasterBand(dEMDataset, 1);

		//Setup
		int srcNoData;
		float dstNoDataValue;

		dstNoDataValue = (float)GDALGetRasterNoDataValue(demBand, &srcNoData);

		std::cout << QString("GDAL data type is %1").arg(GDALGetRasterDataType(demBand)) + "\n";

		float * demData;
		demData = new float[GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand)];

		GDALRasterIO(demBand, GF_Read,
			0, 0,
			GDALGetRasterBandXSize(demBand), GDALGetRasterBandYSize(demBand),
			demData,
			GDALGetRasterBandXSize(demBand), GDALGetRasterBandYSize(demBand),
			GDALGetRasterDataType(demBand),
			0, 0);


		//Get Transform and invert to get pixel/line
		//double transform[6], invTransform[6];
		double * transform;
		double * invTransform;
		transform = new double[6];
		invTransform = new double[6];
		GDALGetGeoTransform(dEMDataset, transform);
		GDALInvGeoTransform(transform, invTransform);

		std::cout << QString("Read in dem raster band!") + "\n";


		//Now get scoop model
		const CSIRO::Mesh::MeshNodesInterface& nodes = scoopModel.getNodes();
		CSIRO::Mesh::Vector3D pos;
		CSIRO::Mesh::Vector3d cellPos;

		CSIRO::Mesh::NodeStateHandle radState = nodes.getStateHandle("radius_m");
		CSIRO::Mesh::NodeStateHandle volState = nodes.getStateHandle("vol_m^3");
		CSIRO::Mesh::NodeStateHandle angleState = nodes.getStateHandle("angle");
		CSIRO::Mesh::NodeStateHandle fosState = nodes.getStateHandle("F_Bish");
		

		double radius, volume, angle, fosBish;
		//Loop through the nodes
		for (CSIRO::Mesh::MeshNodesInterface::const_iterator it = nodes.begin(); it != nodes.end(); ++it) 
		{
			pos = nodes.getPosition(*it);
			nodes.getState(*it, radState, radius);
			nodes.getState(*it, volState, volume);
			nodes.getState(*it, angleState, angle);
			nodes.getState(*it, fosState, fosBish);
			//Define bounding sphere and box
			CSIRO::Mesh::BoundingSphere bounds = CSIRO::Mesh::BoundingSphere(pos, radius);
			CSIRO::Mesh::Vector3D max, min;
			bounds.getBoundingBox(min, max);

			//Check if sphere is within the two planes
			if (boundingPlane1.contains(min) || boundingPlane1.contains(max) || boundingPlane1.contains(pos) ||
				boundingPlane2.contains(min) || boundingPlane2.contains(max) || boundingPlane2.contains(pos)) {
				//Loop through the DEM cells
				int i = 0;
				do {
					int y = floorf(i / GDALGetRasterBandXSize(demBand));
					int x = i - (y * GDALGetRasterBandXSize(demBand));
					double x_loc = transform[0] + (float)x * transform[1];
					double y_loc = transform[3] + (float)y*transform[5];
					if (x_loc > min.x && x_loc < max.x && y_loc > min.y && y_loc < max.y) {
						cellPos.x = transform[0] + x*transform[1] + y*transform[2];
						cellPos.y = transform[3] + x*transform[4] + y*transform[5];
						cellPos.z = demData[i];
						if (bounds.contains(cellPos)) {
							++numberWithinRegion;
						}
					}
					++i;
				} while (!bounds.contains(cellPos) && i < GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand));
			}
		}




        return true;
    }


    /**
     *
     */
    ScoopCounter::ScoopCounter() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< ScoopCounter >::getInstance(),
            tr("Count scoops in region"))
    {
        pImpl_ = new ScoopCounterImpl(*this);
    }


    /**
     *
     */
    ScoopCounter::~ScoopCounter()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  ScoopCounter::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(ScoopCounter, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

