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


#include "volcanoplugin.h"
#include "erosionutils.h"
#include "volcanoutils.h"
#include "upslopearea.h"


namespace RF
{
    /**
     * \internal
     */
    class UpslopeAreaImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::UpslopeAreaImpl)

    public:
        UpslopeArea&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataFlowDirectionDataset_;
        CSIRO::DataExecution::TypedObject< int >                dataRasterBand_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataAccumulationDataset_;
        CSIRO::DataExecution::TypedObject< QString >            dataSlopeName_;

        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputFlowDirectionDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputSlopeName_;
        CSIRO::DataExecution::Output      outputAccumulationDataset_;


        UpslopeAreaImpl(UpslopeArea& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    UpslopeAreaImpl::UpslopeAreaImpl(UpslopeArea& op) :
        op_(op),
        dataFlowDirectionDataset_(),
        dataRasterBand_(1),
        dataSlopeName_(),
        dataAccumulationDataset_(),
        inputFlowDirectionDataset_("Flow direction dataset", dataFlowDirectionDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputSlopeName_("Raster Name", dataSlopeName_, op_),
        outputAccumulationDataset_("Accumulation dataset", dataAccumulationDataset_, op_)
    {
    }
        
/*
bool checkOutofBounds(int i, int j, int nXsize, int nYsize)
{
    if (i >= nYsize)
        return false;
    if (i < 0)
        return false;
    if (j >= nXsize)
        return false;
    if (j < 0)
        return false;
    return true;
}
*/
/*
float upstreamProportion(float* flowWindow, int neighBour, int* ijlookup, int nXsize, int nYsize, float srcNodataValue)
    {
    /*
    0   1   2
    3   4   5
    6   7   8
    */
/*
    if (checkOutofBounds(ijlookup[neighBour*2], ijlookup[(neighBour*2)+1], nXsize, nYsize))        
    {
        float fDir = flowWindow[(ijlookup[neighBour*2]*nXsize)+ijlookup[(neighBour*2)+1]];
        int neighbourList[8] = {0,1,2,3,5,6,7,8};
        int flowCell = neighbourList[neighBour];
        int quadrantList[8] = {6,5,4,7,3,0,1,2};


        //std::cout << QString("Flow direction of cell %1, %2 (window index %3) is %4 radians").arg(ijlookup[neighBour*2]).arg(ijlookup[(neighBour*2)+1]).arg((ijlookup[neighBour*2]*nXsize)+ijlookup[(neighBour*2)+1]).arg(fDir) + "\n";
        if (fDir == -1 || fDir == srcNodataValue)
        {
            return 0.0;
        }
        else if (neighbourList[neighBour] != 3 && fDir > quadrantList[neighBour]*M_PI_4 && fDir < (quadrantList[neighBour]+2)*M_PI_4)
        {
            if ( fabs(fDir - (quadrantList[neighBour]+1)*M_PI_4) < 1e-06)//is cardinal
            {
                return 1.0;
            }
            else if (fabs(fDir - (quadrantList[neighBour]+2)*M_PI_4) < 1e-06)//is not our cardinal
            {
                return 0.0;
            }
            else if (fabs(fDir - quadrantList[neighBour]*M_PI_4) < 1e-06)//is not our cardinal
            {
                return 0.0;
            }
            else if (fDir - (quadrantList[neighBour]+1)*M_PI_4 < M_PI_4)//is in first
            {
                return (fDir-(quadrantList[neighBour]*M_PI_4))/M_PI_4;
            }
            else
            {
                return (((quadrantList[neighBour]+2)*M_PI_4)-fDir)/M_PI_4;
            
            }
        }
        else if (neighbourList[neighBour] == 3)
        {
            if (fabs(fDir) < 1e-06)//Cardinal 0 degree
            {
                return 1.0;
            }
            else if (fabs(fDir - 2*M_PI) < 1e-06)//Cardinal 2pi
            {
                return 1.0;
            }
            else if (fabs(fDir - M_PI_4) < 1e-06)//not our cardinal
            {
                return 0.0;
            }
            else if (fabs(fDir - 7*M_PI_4) < 1e-06)//not our cardinal
            {
                return 0.0;
            }
            else if (fDir > 7*M_PI_4)
            {
                return (fDir - 2*M_PI)/M_PI_4;
            }
            else if (fDir < M_PI_4)
            {
                return (M_PI_4 - fDir)/M_PI_4;
            }
            else
            {
                return 0.0;
            }
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        return 0.0;
    }
}
*/
/*
void calcUpstreamArea(float* flowWindow, bool* calculated, float* accumulated, double* transform, int i, int j, int xSize, int ySize, float srcNodataValue)
{
    if (!calculated[(i*xSize)+j])
    {
        if (flowWindow[(i*xSize+j)] == srcNodataValue)
        {
            calculated[(i*xSize)+j] = true;
            accumulated[(i*xSize)+j] = 0;            
        }
        else
        {
            int algDataIJLookup[8][2] = {i-1,j-1,
                        i-1,j,
                        i-1,j+1,
                        i,j-1,
                        i,j+1,
                        i+1,j-1,
                        i+1,j,
                        i+1,j+1};
            float accumupstream = transform[1] * transform[5];//Calculate cell area
            accumupstream = -accumupstream;
            for (int neighBour = 0; neighBour < 8; ++neighBour)
            {
                if (checkOutofBounds(algDataIJLookup[neighBour][0], algDataIJLookup[neighBour][1], xSize, ySize))
                {
                    float flowProp = upstreamProportion(flowWindow, neighBour, *algDataIJLookup, xSize, ySize, srcNodataValue);
                    if ( flowProp > 0)
                    {
                        calcUpstreamArea(flowWindow, calculated, accumulated, transform, algDataIJLookup[neighBour][0], algDataIJLookup[neighBour][1], xSize, ySize, srcNodataValue);
                        accumupstream += (flowProp*accumulated[(algDataIJLookup[neighBour][0]*xSize)+algDataIJLookup[neighBour][1]]);
                    }
                }
             }
            accumulated[(i*xSize)+j]=accumupstream;
            calculated[(i*xSize)+j]=true;
        }
    }
}
*/
    /**
     *
     */
    bool UpslopeAreaImpl::execute()
    {
        GDALDatasetH& flowDirectionDataset = *dataFlowDirectionDataset_;
        int&               rasterBand    = *dataRasterBand_;
        GDALDatasetH& accumulationDataset  = *dataAccumulationDataset_;
        QString&           slopeName     = *dataSlopeName_;

        
        GDALAllRegister();

        GDALRasterBandH fBand;

        if (rasterBand > GDALGetRasterCount(flowDirectionDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(flowDirectionDataset)).arg(rasterBand) + "\n";
            return false;
        }

        fBand = GDALGetRasterBand(flowDirectionDataset, rasterBand);;

        double transform[6];
        GDALGetGeoTransform(flowDirectionDataset,transform);

        
        //Setup
        float srcNodataValue;
        int srcNoData;

        srcNodataValue = (float)GDALGetRasterNoDataValue(fBand, &srcNoData);
        float dstNoDataValue = srcNodataValue;
        

        void *pData;

        pData = RecursiveCreateInputData(transform);

        GenericRecursiveFlowAlgebraAlg pfnAlg = RecursiveUpstreamFlowAlg;




        
        int xSize = GDALGetRasterBandXSize(fBand);
        int ySize = GDALGetRasterBandYSize(fBand);
        /*
        float* accumulation = new float[xSize*ySize];
        bool* calculated = new bool[xSize*ySize]();

        float* flowWindow = new float[xSize*ySize];
        
        GDALRasterIO(fBand, GF_Read,
                        0,0,
                        xSize, ySize,
                        flowWindow,
                        xSize, ySize,
                        GDT_Float32,
                        0,0);

        int neighbourList[8] = {0,1,2,3,5,6,7,8};

        for (int i = 0; i < ySize; ++i)
        {
            for (int j = 0; j < xSize; ++j)
            {
                calcUpstreamArea(flowWindow, calculated, accumulation, transform, i, j, xSize, ySize, srcNodataValue);
            }
        }
        
        //GenericRecursiveFlowAlgebraAlg pfnAlg = RecursiveUpstreamFlowAlg;
        */

        accumulationDataset = GDALCreate( GDALGetDatasetDriver(flowDirectionDataset),
                                            slopeName.toLocal8Bit().constData(),
                                            GDALGetRasterXSize(flowDirectionDataset),
                                            GDALGetRasterYSize(flowDirectionDataset),
                                            1,
                                            GDT_Float32, NULL);
        
        GDALRasterBandH algBand = GDALGetRasterBand(accumulationDataset, 1);
        GDALSetGeoTransform(accumulationDataset, transform);
        GDALSetProjection(accumulationDataset, GDALGetProjectionRef(flowDirectionDataset));
        GDALSetRasterNoDataValue(algBand, dstNoDataValue);

        GenericRecursiveFlowAlgebraProcessor(fBand, algBand, transform, pfnAlg, pData);
        /*

        GDALRasterIO(algBand, GF_Write,
                        0,0,
                        xSize,ySize,
                        accumulation,
                        xSize,ySize,
                        GDT_Float32,
                        0,0);


        //GenericRecursiveFlowAlgebraProcessor(fBand,algBand,pfnAlg,pData);
                                            
        delete[] accumulation;
        delete[] calculated;
        delete[] flowWindow;
        */
        return true;
    }

    
    /**
     *
     */
    UpslopeArea::UpslopeArea() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< UpslopeArea >::getInstance(),
            tr("Calculate upslope flow area"))
    {
        pImpl_ = new UpslopeAreaImpl(*this);
    }


    /**
     *
     */
    UpslopeArea::~UpslopeArea()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  UpslopeArea::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(UpslopeArea, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

