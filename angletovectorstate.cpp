/*
Created by: Stuart Mead
  Creation date: 2014-02-03
   
  Revision:       $Revision: $
  Last changed:   $Date: $
  Last changed by: Stuart Mead

  Copyright Risk Frontiers 2014, Faculty of Science, Macquarie University, NSW 2109, Australia.

  For further information, contact:
          Stuart Mead
          Building E7A
          Dept. of Environment & Geography
          Macquarie University
          North Ryde NSW 2109

  This copyright notice must be included with all copies of the source code.
*/

#include <cassert>
#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Mesh/DataStructures/MeshModelInterface/meshmodelinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshnodesinterface.h"
#include "Mesh/Geometry/vector3d.h"


#include "volcanoplugin.h"
#include "angletovectorstate.h"


namespace RF
{
    /**
     * \internal
     */
    class AngletoVectorStateImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::AngletoVectorStateImpl)

    public:
        AngletoVectorState&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< CSIRO::Mesh::MeshModelInterface >  dataModel_;
        CSIRO::DataExecution::TypedObject< QString >                          dataAngleStateName_;
        CSIRO::DataExecution::TypedObject< QString >                          dataVectorStateName_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputModel_;
        CSIRO::DataExecution::InputScalar inputAngleStateName_;
        CSIRO::DataExecution::InputScalar inputVectorStateName_;
        CSIRO::DataExecution::Output      outputModel_;


        AngletoVectorStateImpl(AngletoVectorState& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    AngletoVectorStateImpl::AngletoVectorStateImpl(AngletoVectorState& op) :
        op_(op),
        dataModel_(),
        dataAngleStateName_(),
        dataVectorStateName_(),
        inputModel_("Model", dataModel_, op_),
        inputAngleStateName_("Angle state name", dataAngleStateName_, op_),
        inputVectorStateName_("Vector state name", dataVectorStateName_, op_),
        outputModel_("Model", dataModel_, op_, true)
    {
    }


    /**
     *
     */
    bool AngletoVectorStateImpl::execute()
    {
        CSIRO::Mesh::MeshModelInterface& model           = *dataModel_;
        QString&                         angleStateName  = *dataAngleStateName_;
        QString&                         vectorStateName = *dataVectorStateName_;
        
        CSIRO::Mesh::MeshNodesInterface& nodes = model.getNodes();

        CSIRO::Mesh::NodeStateHandle angleState = nodes.getStateHandle(angleStateName);

        CSIRO::Mesh::NodeStateHandle vectorState = nodes.addState<CSIRO::Mesh::Vector3D>(vectorStateName,CSIRO::Mesh::Vector3D());

        double theta;
        for (CSIRO::Mesh::MeshNodesInterface::iterator i = nodes.begin(); i != nodes.end(); ++i)
        {
            CSIRO::Mesh::NodeHandle node = *i;
            CSIRO::Mesh::Vector3D vec;
            nodes.getState(node, angleState, theta);
            vec.x = cos(theta);
            vec.y = sin(theta);
            nodes.setState(node, vectorState, vec);
        }
        

        return true;
    }


    /**
     *
     */
    AngletoVectorState::AngletoVectorState() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< AngletoVectorState >::getInstance(),
            tr("Convert angle to Vector State"))
    {
        pImpl_ = new AngletoVectorStateImpl(*this);
    }


    /**
     *
     */
    AngletoVectorState::~AngletoVectorState()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  AngletoVectorState::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(AngletoVectorState, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

