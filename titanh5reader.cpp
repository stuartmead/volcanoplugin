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
#include<QMap>

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
        CSIRO::DataExecution::SimpleInput< double > htThresh_;
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
	htThresh_("Height threshold for velocity", op_),
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
		CSIRO::Mesh::MeshElementsInterface& elems = mesh.getElements(CSIRO::Mesh::ElementType::Quad::getInstance());

		HdfFile hdata = HdfFile(fileName);
        if(!hdata.isValid())
        {
            std::cout << QString("ERROR: HDF file is invalid!\n");
            return false;
        }

		HdfGroup geom = hdata.group("Mesh");
        if(!geom.isValid())
        {
            std::cout << QString("ERROR: Geometry mesh group is invalid!\n");
            return false;
        }
		HdfGroup props = hdata.group("Properties");
        if(!hdata.isValid())
        {
            std::cout << QString("ERROR: Property group is invalid!\n");
            return false;
        }
        
		//Parse mesh
		HdfDataset point = geom.dataset("Points");
        if(!point.isValid())
        {
            std::cout << QString("ERROR: Point dataset is invalid!\n");
            return false;
        }
		
		QVector<hsize_t> nNds = point.dims();
        std::cout << QString("Point dimensions are %1, %2").arg(nNds.at(0)).arg(nNds.at(1)) + "\n";
		QVector<float> points = point.readArray();

        QMap<int, CSIRO::Mesh::NodeHandle> nodeMap;
		CSIRO::Mesh::Vector3D vec;
        int it = 0;
        for (int i = 0; i < nNds.at(0); ++i)
		{
            vec.x = points[nNds[1]*i];
            vec.y = points[nNds[1]*i+1];
            vec.z = points[nNds[1]*i+2];     
			CSIRO::Mesh::NodeHandle nh = nodes.add(vec);
            nodeMap[i] = nh;
		}
		
		HdfDataset conns =  geom.dataset("Connections");
        if (!conns.isValid())
        {
            std::cout << QString("ERROR: cannot load connections dataset \n");
            return false;
        }
        HdfDataset pHeight = props.dataset("PILE_HEIGHT");
        if (!pHeight.isValid())
        {
            std::cout << QString("ERROR: cannot load pile height dataset\n");
            return false;
        }
		HdfDataset momX = props.dataset("XMOMENTUM");
		if (!momX.isValid())
        {
            std::cout << QString("ERROR: cannot load pile height dataset\n");
            return false;
        }
        HdfDataset momY = props.dataset("YMOMENTUM");
		if (!momY.isValid())
        {
            std::cout << QString("ERROR: cannot load pile height dataset\n");
            return false;
        }

		QVector<hsize_t> nElm = conns.dims();
        std::cout << QString("Element dimensions are %1, %2").arg(nElm.at(0)).arg(nElm.at(1)) + "\n";
		QVector<int> elem_conn = conns.readArrayInt();

		//States for elements
		double htRef = 0.0;
		CSIRO::Mesh::ElementStateHandle pH = elems.addState("Pile height", htRef);
        if(!pH.isValid())
        {
            std::cout << QString("ERROR: Pile height state not added.\n");
            return false;
        }
		CSIRO::Mesh::ElementStateHandle xM = elems.addState("X momentum", htRef);
		CSIRO::Mesh::ElementStateHandle yM = elems.addState("Y momentum", htRef);
		CSIRO::Mesh::ElementStateHandle mMag = elems.addState("Momentum mag", htRef);
		CSIRO::Mesh::ElementStateHandle vMag = elems.addState("Velocity mag", htRef);

		QVector<float> pileVals = pHeight.readArray();
		QVector<float> xVals = momX.readArray();
		QVector<float> yVals = momY.readArray();
/*
       for (int e = 0; e < nElm.at(0); ++e)
       {
           elements[e].setId(e);
           elements[e].setEType(Element::E4Q);
           QVector<uint> idx(nElm.at(1));
               for (int fi=0; fi < nElm.at(1); ++fi)
               {
                   idx[fi] = elem_conn[nElm[1]*e + fi];
               }
        elements[e].setP(idx.data());
    }

*/
		for (int e = 0; e < nElm.at(0); ++e)
		{
			QVector<uint> Nid(nElm.at(1));
			for (int ni = 0; ni < nElm.at(1); ++ni)
			{
				Nid[ni] = elem_conn[nElm[1] * e + ni];
			}
			CSIRO::Mesh::ElementHandle eh = elems.add(nodeMap[Nid[0]], nodeMap[Nid[1]], nodeMap[Nid[2]], nodeMap[Nid[3]]);
			elems.setState(eh, pH, (double)pileVals[e]);
			elems.setState(eh, xM, (double)xVals[e]);
			elems.setState(eh, yM, (double)yVals[e]);
			elems.setState(eh, mMag, (double)sqrt(xVals[e] * xVals[e] + yVals[e] * yVals[e]));
			if (pileVals[e] > *htThresh_)
			{
				elems.setState(eh, vMag, (double)(sqrt(xVals[e] * xVals[e] + yVals[e] * yVals[e]))/pileVals[e]);
			} else
			{
				elems.setState(eh, vMag, (double)0.0);
			}
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

