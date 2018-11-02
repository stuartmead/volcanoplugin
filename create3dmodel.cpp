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

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "Mesh/DataStructures/MeshModelInterface/meshmodelinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshnodesinterface.h"
#include "Mesh/DataStructures/MeshModelInterface/meshelementsinterface.h"


#include "volcanoplugin.h"
#include "create3dmodel.h"


namespace RF
{
    /**
     * \internal
     */
    class Create3dModelImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::Create3dModelImpl)

    public:
        Create3dModel&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >                         dataElevationDataset_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >                         dataPropertyDatasets_;
        CSIRO::DataExecution::TypedObject< QString >                              dataPropertyNames_;
        CSIRO::DataExecution::TypedObject< int >                                  dataRasterBand_;
        CSIRO::DataExecution::TypedObject< int >                              dataNXOff_;
        CSIRO::DataExecution::TypedObject< int >                              dataNYOff_;
        CSIRO::DataExecution::TypedObject< int >                              dataXSize_;
        CSIRO::DataExecution::TypedObject< int >                              dataYSize_;
        CSIRO::DataExecution::TypedObject< double >                           dataScaleFactor_;
        CSIRO::DataExecution::TypedObject< CSIRO::Mesh::MeshModelInterface >      dataMesh_;
        CSIRO::DataExecution::TypedObject< bool >                                 dataCreateElements_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputElevationDataset_;
        CSIRO::DataExecution::InputArray  inputPropertyDatasets_;
        CSIRO::DataExecution::InputArray  inputPropertyNames_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputNXOff_;
        CSIRO::DataExecution::InputScalar inputNYOff_;
        CSIRO::DataExecution::InputScalar inputXSize_;
        CSIRO::DataExecution::InputScalar inputYSize_;
        CSIRO::DataExecution::InputScalar inputScaleFactor_;
        CSIRO::DataExecution::Output      outputMesh_;
        CSIRO::DataExecution::InputScalar inputCreateElements_;


        Create3dModelImpl(Create3dModel& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    Create3dModelImpl::Create3dModelImpl(Create3dModel& op) :
        op_(op),
        dataElevationDataset_(),
        dataPropertyDatasets_(),
        dataPropertyNames_(),
        dataRasterBand_(1),
        dataNXOff_(0),
        dataNYOff_(0),
        dataXSize_(-1),
        dataYSize_(-1),
        dataScaleFactor_(1),
        dataMesh_(),
        dataCreateElements_(),
        inputElevationDataset_("Elevation Dataset", dataElevationDataset_, op_),
        inputRasterBand_("Raster band", dataRasterBand_, op_),
        inputPropertyDatasets_("Property Datasets", dataPropertyDatasets_, op_),
        inputPropertyNames_("Property Names", dataPropertyNames_, op_),
        inputNXOff_("X offset", dataNXOff_, op_),
        inputNYOff_("Y offset", dataNYOff_, op_),
        inputXSize_("X size", dataXSize_, op_),
        inputYSize_("Y size", dataYSize_, op_),
        inputScaleFactor_("Scale factor", dataScaleFactor_, op_),
        outputMesh_("Mesh", dataMesh_, op_),
        inputCreateElements_("Create Elements", dataCreateElements_, op_)
    {
    }


    /**
     *
     */
    bool Create3dModelImpl::execute()
    {
        GDALDatasetH&                        elevationDataset = *dataElevationDataset_;
        CSIRO::Mesh::MeshModelInterface&     mesh             = *dataMesh_;
        bool&                                createElements   = *dataCreateElements_;
        int&                                 rasterBand       = *dataRasterBand_;
        int&                             nXOff         = *dataNXOff_;
        int&                             nYOff         = *dataNYOff_;
        int&                             xSize         = *dataXSize_;
        int&                             ySize         = *dataYSize_;
        double&                          scaleFactor   = *dataScaleFactor_;
        
        //Alwasy clear meshdata

        dataMesh_->clear();

        //Get raster input band and transform
        GDALAllRegister();

        GDALRasterBandH hBand;
        if(rasterBand > GDALGetRasterCount(elevationDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(elevationDataset)).arg(rasterBand) + "\n";
            return false;
        }
        hBand = GDALGetRasterBand(elevationDataset, rasterBand);

        //Read Elevation dataset
        double sizes[2];
        if (xSize <= 0)
        {
            sizes[0] = GDALGetRasterBandXSize(hBand)-nXOff;
        }
        else
        {
            sizes[0] = xSize;
        }

        if (ySize <= 0)
        {
            sizes[1] = GDALGetRasterBandYSize(hBand)-nYOff;
        }
        else
        {
            sizes[1] = ySize;
        }

        int scaleXsize, scaleYsize;
        scaleXsize = floor(sizes[0]/scaleFactor);
        scaleYsize = floor(sizes[1]/scaleFactor);

        double transform[6];
        GDALGetGeoTransform(elevationDataset,transform);

        std::cout << QString("Downscaled X cellsize is %1, Y cellsize is %2").arg((sizes[0]/scaleXsize)*transform[1]).arg((sizes[1]/scaleYsize)*-transform[5]) + "\n";

        float *data;
        data = new float [scaleXsize*scaleYsize];

        GDALRasterIO(hBand, GF_Read,
            nXOff, nYOff, //X,Y offset in cells
            sizes[0], sizes[1], //X,Y length in cells
            data, //data
            scaleXsize, scaleYsize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);//Scanline stuff (for interleaving)

        CSIRO::Mesh::MeshNodesInterface&    nodes = mesh.getNodes();
        CSIRO::Mesh::MeshElementsInterface& elems = mesh.getElements(CSIRO::Mesh::ElementType::Tri::getInstance());

         //Now read the other rasters
        std::vector<GDALDatasetH*> propertyRasters;
        std::vector<QString*> names;
        for (unsigned rasters = 0; rasters < inputPropertyDatasets_.size(); ++rasters)
        {
            propertyRasters.push_back(&inputPropertyDatasets_.getInput(rasters).getDataObject().getRawData<GDALDatasetH>());
            names.push_back(&inputPropertyNames_.getInput(rasters).getDataObject().getRawData<QString>());
        }
               
        
        double xposn = transform[0];
        double yposn = transform[3];
        std::vector<CSIRO::Mesh::NodeHandle> nodeLookup;
		int it = 0;
        for (int i = 0; i < scaleYsize; ++i)//Line
        {
            for (int j = 0; j < scaleXsize; ++j)//Pixel
            {
                CSIRO::Mesh::NodeHandle nh = nodes.add(CSIRO::Mesh::Vector3D(transform[0] + j*transform[1] + i*transform[2],//xposn+(i*((sizes[0]/scaleXsize)*transform[1]))
                transform[3] + j*transform[4] + i*transform[5],//yposn+(j*((sizes[1]/scaleYsize)*-transform[5])),
                data[it]));
                nodeLookup.push_back(nh);
                /*/Deal with states here
                for (int k = 0; k < propertyHandles.size(); ++k)
                {
                    nodes.setState(nh,propertyHandles.at(i),propertyData.at(k)[it]);
                }*/
                ++it;
            }
        }

		
        for (unsigned rData = 0; rData < propertyRasters.size(); ++rData)
        {
			int rasterCount = GDALGetRasterCount(*propertyRasters[rData]);
			
			if (rasterCount > 1) 
			{
				for (int sit = 1; sit <= rasterCount; sit++) {

					float *scrData = new float[scaleXsize*scaleYsize];
					std::cout << QString("Property raster %1, name %2, band %3").arg(rData).arg(*names[rData]).arg(sit) + "\n";
					GDALRasterBandH pBand = GDALGetRasterBand(*propertyRasters[rData], sit);
					//Read
					GDALRasterIO(pBand, GF_Read,
						nXOff, nYOff, //X,Y offset in cells
						sizes[0], sizes[1], //X,Y length in cells
						scrData, //data
						scaleXsize, scaleYsize, //Number of cells in new dataset
						GDT_Float32, //Type
						0, 0);//Scanline stuff (for interleaving)

					QString name = *names[rData];
					name.append(QString::number(sit));
					CSIRO::Mesh::NodeStateHandle nsh = nodes.addState<int>(name, 0);

					if (nodes.hasState(name))
					{
						for (int nit = 0; nit < nodeLookup.size(); ++nit)
						{
							nodes.setState(nodeLookup[nit], nsh, (int)scrData[nit]);
						}
					}
					else
					{
						std::cout << QString("WARNING: Nodes do not have newly added state %1").arg(rData) + "\n";
					}
				}
			}
			else {
				
				float *scrData = new float[scaleXsize*scaleYsize];
				std::cout << QString("Property raster %1, name %2").arg(rData).arg(*names[rData]) + "\n";
				GDALRasterBandH pBand = GDALGetRasterBand(*propertyRasters[rData], 1);
				//Read
				GDALRasterIO(pBand, GF_Read,
					nXOff, nYOff, //X,Y offset in cells
					sizes[0], sizes[1], //X,Y length in cells
					scrData, //data
					scaleXsize, scaleYsize, //Number of cells in new dataset
					GDT_Float32, //Type
					0, 0);//Scanline stuff (for interleaving)

				CSIRO::Mesh::NodeStateHandle nsh = nodes.addState<double>(*names[rData], 0.0);

				if (nodes.hasState(*names[rData]))
				{
					for (int nit = 0; nit < nodeLookup.size(); ++nit)
					{
						nodes.setState(nodeLookup[nit], nsh, (double)scrData[nit]);
					}
				}
				else
				{
					std::cout << QString("WARNING: Nodes do not have newly added state %1").arg(rData) + "\n";
				}
			}

        }




        if (createElements == true)
        {
            //Create elements
            double nrowelem = scaleYsize - 1;
            double ncolelem = scaleXsize - 1;

            for (unsigned i = 0; i != nrowelem; ++i)
            {
                int arrayno = i * scaleXsize;
                for (unsigned r = 0; r != ncolelem; ++r)
                {
                    int index0 = r + arrayno;
                    int index1 = (r + 1) + arrayno;
                    int index2 = (scaleXsize + r) + arrayno;
                    elems. add(nodeLookup[index0],nodeLookup[index1],nodeLookup[index2]);
                }
            }
            //Second pass
            for (unsigned i = 0; i != nrowelem; ++i)
            {
                int arrayno = i * scaleXsize;
                for (unsigned r = 0; r != ncolelem; ++r)
                {
                    int index0 = (scaleXsize + r) + arrayno;
                    int index1 = (scaleXsize + (r + 1)) + arrayno;
                    int index2 = (r+1) + arrayno;
                    elems. add(nodeLookup[index0],nodeLookup[index1],nodeLookup[index2]);
                }
            }

            
        }



        return true;
    }


    /**
     *
     */
    Create3dModel::Create3dModel() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< Create3dModel >::getInstance(),
            tr("Create 3d Model from DTM"))
    {
        pImpl_ = new Create3dModelImpl(*this);
    }


    /**
     *
     */
    Create3dModel::~Create3dModel()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  Create3dModel::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(Create3dModel, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

