#include <cassert>
#include <fstream>
#include <iostream>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/simpleoperationio.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Workspace/DataExecution/DataObjects/datafactorytraits.h"

#include "DataAnalysis/DataStructures/array3d.h"
#include "DataAnalysis/DataStructures/array3dtyped.h"
#include "DataAnalysis/DataStructures/array3ddata.h"
#include "DataAnalysis/DataStructures/transferfunction2d.h"

#include "Rendering/SceneComponents/Shaders/volumeshader.h"
#include "Rendering/SceneComponents/Shaders/volumeshaderbuilder.h"

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
		//CSIRO::DataExecution::TypedObject<CSIRO::DataAnalysis::Array3dScalar> vTIData_;

        // Inputs and outputs
        CSIRO::DataExecution::SimpleInput< QString > fileName_;
		CSIRO::DataExecution::SimpleInput < bool >	info_;
		CSIRO::DataExecution::SimpleInput< QString > component_;
        //CSIRO::DataExecution::InputScalar inputVtiData_;
		CSIRO::DataExecution::SimpleInputOutput<CSIRO::DataAnalysis::Array3dScalar> vTIData_;
		CSIRO::DataExecution::SimpleInput< QString > outputFileName_;

		//Create category thresholds
		CSIRO::DataExecution::SimpleInputArray < double >	catThreshold_;
		CSIRO::DataExecution::SimpleInputArray < double >	catCee_;
		CSIRO::DataExecution::SimpleInputArray < double >	catPhi_;
		CSIRO::DataExecution::SimpleInputArray < double >	catGamt_;

        VTIReaderImpl(VTIReader& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
	VTIReaderImpl::VTIReaderImpl(VTIReader& op) :
		op_(op),
		//vTIData_(),
		fileName_("File Name", op_),
		outputFileName_("Output file name", op_),
		info_("Print out information only", false, op_),
		component_("Component name", "mag_sus", op_),
		//inputVtiData_("VTI data", vTIData_, op_),
		vTIData_("VTI data", op_),
		catThreshold_("Category thresholds", op_),
		catCee_("Category cohesions", op_),
		catPhi_("Category angle", op_),
		catGamt_("Category unit weight", op_)
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
		
		//Check the vectors are only length 3 - fix up later!
		if (catThreshold_.size() != 3 || catCee_.size() != 3 || catPhi_.size() != 3 || catGamt_.size() != 3) {
			std::cout << QString("Error: Category thresholds or values are not of size 3 - check inputs \n");
			return false;
		}

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

		double* origin = rawData->GetOrigin();
		double* spacing = rawData->GetSpacing();
		double* bounds = rawData->GetBounds();

		std::cout << QString("Origin is %1, %2, %3 \n").arg(origin[0]).arg(origin[1]).arg(origin[2]);
		std::cout << QString("Spacing is %1, %2, %3 \n").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);
		std::cout << QString("Bounds are %1, %2, %3, %4, %5, %6 \n").arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]);

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
					/*
					if (count % 10000 == 0) {
						std::cout << QString("Value is %1 \n").arg(v);
					}*/
				}
			}
		}

		CSIRO::DataAnalysis::Array3dTypedAdaptor <CSIRO::DataAnalysis::Array3dDoubleData, double> myMatrix;
		myMatrix.setData(&matrix, false);
		CSIRO::DataAnalysis::Array3dScalar& scalarMat = myMatrix;
		
		std::cout << QString("Output array is %1 long\n").arg(scalarMat.size());
		std::cout << QString("A value is %1 \n").arg(scalarMat(50, 50, 50));

		//Write out to a scoops IJZ file
		std::ofstream outfile(outputFileName_->toLocal8Bit().constData());

		//Initial items
		outfile << "#3D material property file\n";
		outfile << "#Generated in workspace\n";
		outfile << "#i j z cee phi gamt\n";
		outfile << "coords\n";
		outfile << "ijz\n";

		double zval;
		double cee, phi, gamt;
		//Now loop through matrix: x, y, z order
		for (int x = 0; x < dims[0]; x++) {
			for (int y = 0; y < dims[1]; y++) {
				for (int z = 0; z < dims[2]; z++) {
					zval = scalarMat(x, y, z);
					if (zval >= 0.0) {
						//Output co-ords
						outfile << "\t" << x + 1 << "\t" << y + 1 << "\t" << origin[2] + ((double)z*spacing[2]);
						//Now categorise
						if (zval > catThreshold_[2]) { //Highest value
							cee = catCee_[2];
							phi = catPhi_[2];
							gamt = catGamt_[2];
						}
						else if (zval <= catThreshold_[2] && zval > catThreshold_[1]) {//Mid value
							cee = catCee_[1];
							phi = catPhi_[1];
							gamt = catGamt_[1];
						}
						else { //Low value
							cee = catCee_[0];
							phi = catPhi_[0];
							gamt = catGamt_[0];
						}
						//Output values
						outfile << "\t" << cee;
						outfile << "\t" << phi;
						outfile << "\t"	<< gamt << "\n";
					}

				}
			}
		}
		
		outfile.close();


		/* Fuck knows how this is supposed to work
		CSIRO::DataAnalysis::Array3dTypedAdaptor <CSIRO::DataAnalysis::Array3dDoubleData, double> myMatrix;

		myMatrix.setData(&matrix, false);

		//auto* adapter = new CSIRO::DataAnalysis::Array3dTypedAdaptor <CSIRO::DataAnalysis::Array3dDoubleData, double>();
		//adapter->setData(&matrix, false);
		

		//Works
		CSIRO::DataAnalysis::Array3dScalar& scalarMat = myMatrix;
		std::cout << QString("Output array is %1 long\n").arg(scalarMat.size());
		std::cout << QString("A value is %1 \n").arg(scalarMat(50, 50, 50));
		vTIData_.data_.setData(&scalarMat, true);
		*/

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