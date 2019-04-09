/*
/*
  Created by: Stuart Mead
  Creation date: $Date$
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

*/


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
#include "Mesh/Geometry/boundingsphere.h"

#include "volcanoutils.h"

#include "ogr_spatialref.h"

#include "volcanoplugin.h"
#include "deformtosphere.h"


namespace RF
{
    /**
     * \internal
     */
    class DeformToSphereImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::DeformToSphereImpl)

    public:
        DeformToSphere&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< GDALDatasetH > demDataset_;
        CSIRO::DataExecution::SimpleInput< CSIRO::Mesh::MeshModelInterface > scoopModel_;
        CSIRO::DataExecution::SimpleInput< int > selectedNode_;
        CSIRO::DataExecution::SimpleOutput< GDALDatasetH > cutDEM_;
		CSIRO::DataExecution::SimpleInput< QString > cutDatasetName_;
        CSIRO::DataExecution::SimpleOutput< GDALDatasetH > heightRaster_;
		CSIRO::DataExecution::SimpleInput< QString > heightDatasetName_;


        DeformToSphereImpl(DeformToSphere& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    DeformToSphereImpl::DeformToSphereImpl(DeformToSphere& op) :
        op_(op),
        demDataset_("DEM dataset",  op_),
        scoopModel_("Scoop models",  op_),
        selectedNode_("Selected node",  op_),
        cutDEM_("Cut DEM",  op_),
		cutDatasetName_("Cut DEM filename", op_),
        heightRaster_("Height Raster",  op_),
		heightDatasetName_("Height raster filename", op_)
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
    bool DeformToSphereImpl::execute()
    {
        const GDALDatasetH&                    demDataset   = *demDataset_;
        const CSIRO::Mesh::MeshModelInterface& scoopModel   = *scoopModel_;
        const int&                            selectedNode = *selectedNode_;
		QString&      cutDatasetname = *cutDatasetName_;
		QString&	  heightDatasetname = *heightDatasetName_;
        GDALDatasetH&                    cutDEM       = *cutDEM_;
        GDALDatasetH&                    heightRaster = *heightRaster_;
        
		GDALAllRegister();

		GDALRasterBandH demBand = GDALGetRasterBand(demDataset, 1);

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
		GDALGetGeoTransform(demDataset, transform);
		GDALInvGeoTransform(transform, invTransform);

		std::cout << QString("Read in dem raster band!") + "\n";

		const CSIRO::Mesh::MeshNodesInterface& nodes = scoopModel.getNodes();

		if (selectedNode > nodes.size()) {
			std::cout << QString("ERROR: Selected node is outside model range!") + "\n";
			return false;
		}

		//Iterate until node is found
		int count = 0;
		CSIRO::Mesh::MeshNodesInterface::const_iterator it = nodes.begin(); 
		while (count != selectedNode && it != nodes.end()) {
			++it;
			++count;
		}
		if (it == nodes.end()) {
			std::cout << QString("ERROR: Did not find node! Count is %1").arg(count) + "\n";
			return false;
		}
		
		double radius, volume, angle, fosBish;
		CSIRO::Mesh::NodeStateHandle radState = nodes.getStateHandle("radius_m");
		CSIRO::Mesh::NodeStateHandle volState = nodes.getStateHandle("vol_m^3");
		CSIRO::Mesh::NodeStateHandle angleState = nodes.getStateHandle("angle");
		CSIRO::Mesh::NodeStateHandle fosState = nodes.getStateHandle("F_Bish");
                
		//Get node positon
		CSIRO::Mesh::Vector3D pos = nodes.getPosition(*it);
		nodes.getState(*it, radState, radius);
		nodes.getState(*it, volState, volume);
		nodes.getState(*it, angleState, angle);
		nodes.getState(*it, fosState, fosBish);
		//Define bounding sphere and box
		CSIRO::Mesh::BoundingSphere bounds = CSIRO::Mesh::BoundingSphere(pos, radius);
		CSIRO::Mesh::Vector3D max, min;
		bounds.getBoundingBox(min, max);

                //SRM TEMP - Write out to a csvFile!
                std::ofstream ofs;
		QString csv = cutDatasetname + ".csv";
                ofs.open(csv.toLocal8Bit().constData(), std::ofstream::out);
                ofs << "Node, Radius (m), Volume (m^3), Angle (deg), F_Bish" << "\n";
		ofs << selectedNode << "," << radius << "," << volume << "," << angle << "," << fosBish;
                ofs.close();
		
		//Copy array to output
		float * outputDem;
		outputDem = new float[GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand)];
		memcpy(outputDem, demData, sizeof(float)*GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand));
		//Create the heightfile DEM
		float * heightDem;
		heightDem = new float[GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand)];
		//Memset doesnt work!!!
		for (int ii = 0; ii < GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand); ++ii) {
			heightDem[ii] = (float)0.0;
		}
		std::cout << QString("Allocated memory, found node!") + "\n";
		

		//
		//Coefficients between Pixel Line and Projected (Yp, Xp space)
		//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
		//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
		
		CSIRO::Mesh::Vector3d cellPos;
		double zVal;
		double rsq = radius*radius;
		
		//Loop through DEM
		//Coefficients between Pixel Line and Projected (Yp, Xp space)
		//Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
		//Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
		
		for (int i = 0; i < GDALGetRasterBandXSize(demBand)*GDALGetRasterBandYSize(demBand); ++i) {
			int y = floorf(i / GDALGetRasterBandXSize(demBand));
			int x = i - (y * GDALGetRasterBandXSize(demBand));
			double x_loc = transform[0] + (float)x * transform[1];
			double y_loc = transform[3] + (float)y*transform[5];
			if (x_loc > min.x && x_loc < max.x && y_loc > min.y && y_loc < max.y) {
				cellPos.x = transform[0] + x*transform[1] + y*transform[2];
				cellPos.y = transform[3] + x*transform[4] + y*transform[5];
				cellPos.z = demData[i];
				if (bounds.contains(cellPos)) {
					//Calculate Z value (minimum) 
					zVal = pos.z + sqrt(pow(fabs(cellPos.x - pos.x), 2) + pow(fabs(cellPos.y - pos.y), 2) + rsq) ;
					outputDem[i] = (float)zVal - (2*radius);
					heightDem[i] = (float)(demData[i] - outputDem[i]);
				}
			}
		}

		
		//Write out - cutDem and heightRaster

		cutDEM = GDALCreate(GDALGetDriverByName("GTiff"), //GDALGetDatasetDriver(demDataset),
			cutDatasetname.toLocal8Bit().constData(),
			GDALGetRasterXSize(demDataset), GDALGetRasterYSize(demDataset),
			1,
			GDT_Float32,
			NULL);
		GDALRasterBandH cutDemBand = GDALGetRasterBand(cutDEM, 1);
		GDALSetGeoTransform(cutDEM, transform);
	    GDALSetProjection(cutDEM, GDALGetProjectionRef(demDataset));
		/*
		if (GDALSetProjection(cutDEM, GDALGetProjectionRef(demDataset)) != CE_None)
		{
			std::cout << QString("WARNING: Output projection cannot be set, setting to WGS84") + "\n";
			OGRSpatialReferenceH hSRS;
			hSRS = OSRNewSpatialReference(NULL);
			OSRSetWellKnownGeogCS(hSRS, "WGS84");
			char *gsR = NULL;
			OSRExportToWkt(hSRS, &gsR);
			if (GDALSetProjection(cutDEM, gsR) != CE_None)
			{
				std::cout << QString("ERROR: Could not set projection to WGS84") + "\n";
				return false;
			}
		}
		*/
		GDALSetRasterNoDataValue(cutDemBand, dstNoDataValue);
		CPLErr error;
		error = GDALRasterIO(cutDemBand, GF_Write,
			0, 0,
			GDALGetRasterBandXSize(demBand), GDALGetRasterBandYSize(demBand),
			outputDem,
			GDALGetRasterBandXSize(demBand), GDALGetRasterBandYSize(demBand),
			GDT_Float32,
			0, 0);

		if (error != CE_None)
		{
			std::cout << QString("ERROR: GDALRasterIO write operation failed.") + "\n";
		}

		//GDALClose(cutDEM);
		
                //Write out height data
		error = writeRasterData(heightRaster, GDALGetDriverByName("GTiff"), transform,
			GDALGetRasterBandXSize(demBand), GDALGetRasterBandYSize(demBand), 0.0,
			heightDem, heightDatasetname.toLocal8Bit().constData(), GDALGetProjectionRef(demDataset));

		if (error != CPLErr::CE_None) {
			std::cout << QString("ERROR: GDALRaster write operation failed.") + "\n";
		}

		heightRaster = GDALOpen(heightDatasetname.toLocal8Bit().constData(), GA_ReadOnly);
		
        return true;
    }


    /**
     *
     */
    DeformToSphere::DeformToSphere() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< DeformToSphere >::getInstance(),
            tr("DeformToScoopsSphere"))
    {
        pImpl_ = new DeformToSphereImpl(*this);
    }


    /**
     *
     */
    DeformToSphere::~DeformToSphere()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  DeformToSphere::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(DeformToSphere, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

