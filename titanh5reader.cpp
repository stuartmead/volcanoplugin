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
#include <algorithm>

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
#include "h5utils.h"
#include "titanh5reader.h"


namespace RF
{
    /**
     * \internal
     */
    class TitanH5ReaderImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::TitanH5ReaderImpl)

    public:
        TitanH5Reader&  op_;

        // Data objects


        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< QString > fileName_;
        CSIRO::DataExecution::SimpleOutput< CSIRO::Mesh::MeshModelInterface > mesh_;


        TitanH5ReaderImpl(TitanH5Reader& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    TitanH5ReaderImpl::TitanH5ReaderImpl(TitanH5Reader& op) :
        op_(op),
        fileName_("File name",  op_),
        mesh_("Mesh",  op_)
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
    bool TitanH5ReaderImpl::execute()
    {
        const QString&                         fileName = *fileName_;
        CSIRO::Mesh::MeshModelInterface& mesh     = *mesh_;
        
		mesh.clear();

		CSIRO::Mesh::MeshNodesInterface&    nodes = mesh.getNodes();
		CSIRO::Mesh::MeshElementsInterface& elems = mesh.getElements(CSIRO::Mesh::ElementType::Tri::getInstance());

		HdfFile hdfFile = HdfFile(fileName);
		HdfGroup geom = hdfFile.group("Mesh");
		HdfGroup props = hdfFile.group("Properties");

		//Parse mesh
		HdfDataset point = geom.dataset("Points");
		
		QVector<hsize_t> nNds = point.dims();
		QVector<float> points = point.readArray();

		std::vector<CSIRO::Mesh::NodeHandle> nodeLookup;
		int it = 0;
		for (int i = 0; i < nNds.at(0); ++i)
		{
			CSIRO::Mesh::NodeHandle nh = nodes.add(CSIRO::Mesh::Vector3D(points[nNds[1] * i],
				points[nNds[1] * i + 1],
				points[nNds[1] * i + 1]));
				nodeLookup.push_back(nh);
		}
		
		HdfDataset conns = geom.dataset("Connections");
		
		QVector<hsize_t> nElm = conns.dims();
		QVector<int> elem_conn = conns.readArrayInt();
		
		//States for elements

		float htRef = 0.0;
		CSIRO::Mesh::ElementStateHandle pH = elems.addState("Pile height", htRef);
		CSIRO::Mesh::ElementStateHandle xM = elems.addState("X momentum", htRef);
		CSIRO::Mesh::ElementStateHandle yM = elems.addState("Y momentum", htRef);
		CSIRO::Mesh::ElementStateHandle mM = elems.addState("Momentum mag", htRef);

		HdfDataset pHeight = props.dataset("PILE_HEIGHT");
		HdfDataset momX = props.dataset("XMOMENTUM");
		HdfDataset momY = props.dataset("YMOMENTUM");

		QVector<float> pileVals = pHeight.readArray();
		QVector<float> xVals = momX.readArray();
		QVector<float> yVals = momY.readArray();

		for (int e = 0; e < nElm.at(0); ++e)
		{
			QVector<uint> Nid(nElm.at(1));
			for (int ni = 0; ni < nElm.at(1); ++ni)
			{
				Nid[ni] = elem_conn[nElm[1] * e + ni];
			}
			CSIRO::Mesh::ElementHandle eh = elems.add(nodeLookup[Nid[1]], nodeLookup[Nid[2]], nodeLookup[Nid[3]], nodeLookup[Nid[4]]);
			elems.setState(eh, pH, pileVals[e]);
			elems.setState(eh, xM, xVals[e]);
			elems.setState(eh, yM, yVals[e]);
			elems.setState(eh, mM, sqrt(xVals[e] * xVals[e] + yVals[e] * yVals[e]));
		}



        return true;
    }


    /**
     *
     */
    TitanH5Reader::TitanH5Reader() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< TitanH5Reader >::getInstance(),
            tr("Titan2D file reader"))
    {
        pImpl_ = new TitanH5ReaderImpl(*this);
    }


    /**
     *
     */
    TitanH5Reader::~TitanH5Reader()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  TitanH5Reader::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(TitanH5Reader, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("FileIO/Readers"))

