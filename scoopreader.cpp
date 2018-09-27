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
#include <fstream> //To read - old school!
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Mesh/DataStructures/MeshModelInterface/meshmodelinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshnodesinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshelementsinterface.h"

#include "volcanoplugin.h"
#include "scoopreader.h"


namespace RF
{
    /**
     * \internal
     */
    class ScoopReaderImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ScoopReaderImpl)

    public:
        ScoopReader&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< QString > fileName_;
        CSIRO::DataExecution::SimpleOutput< CSIRO::Mesh::MeshModelInterface > scoopModel_;


        ScoopReaderImpl(ScoopReader& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    ScoopReaderImpl::ScoopReaderImpl(ScoopReader& op) :
        op_(op),
        fileName_("File Name",  op_),
        scoopModel_("Scoop model",  op_)
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
    bool ScoopReaderImpl::execute()
    {
        const QString&                         fileName   = *fileName_;
        CSIRO::Mesh::MeshModelInterface& scoopModel = *scoopModel_;
        
		if (fileName.isNull()) {
			std::cout << "ERROR: No file name";
			return false;
		}
		
		//Clear the mesh, collect pointers to model
		scoopModel.clear();
		CSIRO::Mesh::MeshNodesInterface& nodes = scoopModel.getNodes();

		//Collect file
		std::ifstream okcFile(fileName.toLocal8Bit().constData());
		if (!okcFile) {
			std::cout << "ERROR: Cannot open file!";
		}
		
		std::string str;
		int nDims, nPts;
		//Line 1: Number of dimensions, number of points, points x dimensions
		okcFile >> nDims >> nPts >> str;
		std::cout << QString("Reading in %1 points with %2 dimensions").arg(nPts).arg(nDims) + "\n";
		std::vector<QString> names;
		std::vector<CSIRO::Mesh::NodeStateHandle> stateHandles;
		std::string name;
		//Allocate states: THIS TURNS INT TO DOUBLE AS WE DONT KNOW FORMAT
		for (int i = 0; i < nDims; ++i) {
			okcFile >> name;			
			names.push_back(QString::fromStdString(name));			
		}

		//Now skip the next nDim lines
		for (int i = 0; i < nDims; ++i) {
			okcFile >> str >> str >> str;
		}

		//Create state handles - ignore first 3 dims
		for (int i = 3; i < names.size(); ++i) {
			stateHandles.push_back(nodes.addState<double>(names.at(i), 0.0));
		}


		double x_loc, y_loc, z_loc, stateValue;
		//Now read each node
		while (okcFile.good()) {
			okcFile >> x_loc >> y_loc >> z_loc;
			CSIRO::Mesh::NodeHandle nh = nodes.add(CSIRO::Mesh::Vector3D(x_loc, y_loc, z_loc));
			for (std::vector<CSIRO::Mesh::NodeStateHandle>::iterator it = stateHandles.begin(); it != stateHandles.end(); ++it) {
				okcFile >> stateValue;
				nodes.setState(nh, *it, stateValue);
			}
		}
		
		//Done!


        return true;
    }


    /**
     *
     */
    ScoopReader::ScoopReader() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< ScoopReader >::getInstance(),
            tr("Read OKC file"))
    {
        pImpl_ = new ScoopReaderImpl(*this);
    }


    /**
     *
     */
    ScoopReader::~ScoopReader()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  ScoopReader::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(ScoopReader, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("FileIO/Readers"))

