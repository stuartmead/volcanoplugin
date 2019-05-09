#include <cassert>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Workspace/DataExecution/DataObjects/derivedtobaseadaptor.h"

#include "DataAnalysis/DataStructures/array3d.h"
#include "DataAnalysis/DataStructures/array3dtyped.h"
#include "DataAnalysis/DataStructures/array3ddata.h"

#include <vtkSmartPointer.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkImageIterator.h>

#include "volcanoplugin.h"
#include "vtireader.h"


namespace RF
{
    /**
     * \internal
     */
    class VTIReaderImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::VTIReaderImpl)

    public:
        VTIReader&  op_;

        // Data objects
		CSIRO::DataExecution::TypedObject< CSIRO::DataAnalysis::Array3dScalar> vTIData_;

        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< QString > fileName_;
		CSIRO::DataExecution::SimpleInput < bool >	info_;
		CSIRO::DataExecution::SimpleInput< QString > component_;
        CSIRO::DataExecution::Output outputVTI_;


        VTIReaderImpl(VTIReader& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    VTIReaderImpl::VTIReaderImpl(VTIReader& op) :
        op_(op),
        fileName_("File Name",  op_),
		info_("Print out information only", false, op_),
		component_("Component name", "mag_sus", op_),
        outputVTI_("VTI data", vTIData_, op_)
    {
        // Make sure all of our inputs have data by default. If your operation accepts a
        // large data structure as input, you may wish to remove this call and replace it
        // with constructors for each input in the initialisation list above.
        op_.ensureHasData();

        // Recommend setting a description of the operation and each input / output here:
        // op_.setDescription(tr("My operation does this, that and this other thing."));
        // input1_.input_.setDescription(tr("Used for such and such."));
        // output1_.output_.setDescription(tr("Results of the blah-di-blah."));
		info_.input_.setDescription(tr("Print out information on file only, useful to identify components"));
    }


    /**
     *
     */
    bool VTIReaderImpl::execute()
    {
        const QString&                            fileName = *fileName_;
		const QString&							component = *component_;
		//CSIRO::DataAnalysis::Array3dScalar & vTIData  = *vTIData_;

		//Initialise

		auto reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
        
		reader->SetFileName(fileName.toStdString().c_str());
		reader->Update();
		
		vtkSmartPointer<vtkImageData> rawData = vtkSmartPointer<vtkImageData>::New();

		rawData = reader->GetOutput();

		//Set size of data
		unsigned int* dims = (unsigned int*) rawData->GetDimensions();
		std::cout << QString("Reading in data with dimensions of %1, %2, %3 \n").arg(dims[0]).arg(dims[1]).arg(dims[2]);
		
		//Create the matrix
		CSIRO::DataAnalysis::Array3dDoubleData matrix;
		matrix.resize(dims[0], dims[1], dims[2], 0.0);

		
		//Get Point data
		vtkPointData *pd = rawData->GetPointData();
		if (pd)
		{
			std::cout << QString("It has %1 components\n").arg(pd->GetNumberOfArrays());
			for (int i = 0; i < pd->GetNumberOfArrays(); i++)
			{
				std::cout << "\tArray " << i
					<< " is named "
					<< (pd->GetArrayName(i) ? pd->GetArrayName(i) : "NULL")
					<< std::endl;
			}
		}
		if (*info_) { return true; }
		
		vtkDataArray *val_array = pd->GetArray(component.toStdString().c_str());
		
		vtkIdType count = 0;
		for (int z = 0; z < dims[2]; z++) 
		{
			for (int y = 0; y < dims[1]; y++)
			{
				for (int x = 0; x < dims[0]; x++)
				{
					double v = val_array->GetComponent(count, 0);
					matrix(x, y, z) = v;
					++count;
					if (count % 1000 == 0) {
						std::cout << QString("Value is %1 \n").arg(v);
					}
				}
			}
		}
		
		CSIRO::DataAnalysis::Array3dTypedAdaptor <CSIRO::DataAnalysis::Array3dDoubleData, double> myMatrix;
		myMatrix.setData(&matrix, false);
		
		CSIRO::DataAnalysis::Array3dScalar& scalarMat = myMatrix;
		
		//vTIData_.setData(&scalarMat, false);

		vTIData_.setData(&scalarMat, false);

		std::cout << QString("Output array is %1 long\n").arg(vTIData_->size());
		std::cout << QString("A value is %1 \n").arg(scalarMat(50,50,50));

		//vTIData.setCell(0, 0, 0, 300.0);
		//vTIData.setCell(1, 1, 1, 300.0);
		
        return true;
    }


    /**
     *
     */
    VTIReader::VTIReader() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< VTIReader >::getInstance(),
            tr("VTIReader"))
    {
        pImpl_ = new VTIReaderImpl(*this);
    }


    /**
     *
     */
    VTIReader::~VTIReader()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  VTIReader::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(VTIReader, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("FileIO/Readers"))