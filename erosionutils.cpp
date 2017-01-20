/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

  This is a whole bunch of utility functions for erosion/shallow landsliding for lahars.
*/


#include <stdlib.h>
#include <math.h>
#include <vector>

#include "gdal.h"
#include "gdal_priv.h"

#include "volcanoutils.h"
#include "erosionutils.h"

#include "Workspace/DataExecution/DataObjects/typeddatafactory.h"

/***************************************************************
Recursive flow algebra
***************************************************************/
//Utility to check out of bounds
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

//Calculate upstream proportion of flow
float upstreamProportion(float* flowWindow, int neighBour, int* ijlookup, int nXsize, int nYsize, float srcNodataValue)
    {
    /*
    0   1   2
    3   4   5
    6   7   8
    */

    if (checkOutofBounds(ijlookup[neighBour*2], ijlookup[(neighBour*2)+1], nXsize, nYsize))        
    {
        float fDir = flowWindow[(ijlookup[neighBour*2]*nXsize)+ijlookup[(neighBour*2)+1]];
        int neighbourList[8] = {0,1,2,3,5,6,7,8};
        int flowCell = neighbourList[neighBour];
        int quadrantList[8] = {6,5,4,7,3,0,1,2};


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


/*
ComputeFlowAlgebraValues: Compute the value using the processing algorithims
*/
void ComputeFlowAlgebraValues(float srcNoDataValue, float dstNoDataValue,
                                        int i, int j, int nXSize, int nYSize,
                                        float* flowDirections,
                                        float* algData,
                                        bool* algCalc,
                                        double* transform,
                                        GenericRecursiveFlowAlgebraAlg pfnAlg,
                                        void* pData)
{
        //Get cell Index
        int cellIndex = (i*nXSize) + j;
        //Check it hasnt been calculated
        if(!algCalc[cellIndex])
        {
            if (flowDirections[cellIndex] == srcNoDataValue)
            {
                algCalc[cellIndex] = true;
                algData[cellIndex] = 0;
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
                for (int neighbourCell = 0; neighbourCell < 8; ++neighbourCell)
                {
                    if (checkOutofBounds(algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1], nXSize, nYSize))
                    {
                        float flowProp = upstreamProportion(flowDirections, neighbourCell, *algDataIJLookup, nXSize, nYSize, srcNoDataValue);
                        if (flowProp > 0)
                        {
                            ComputeFlowAlgebraValues(srcNoDataValue, dstNoDataValue, algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1],
                                                        nXSize, nYSize, flowDirections, algData, algCalc, transform, pfnAlg, pData);
                        }
                    }
                }
                float algValue = pfnAlg(i, j, nXSize, nYSize, flowDirections, algData, dstNoDataValue, srcNoDataValue, pData); 
                algData[cellIndex] = algValue;
                algCalc[cellIndex] = true;
            }
        }
}

void ComputeFailureAlgebraValues(float srcNoDataValue, float dstNoDataValue,
                                        int i, int j, int nXSize, int nYSize,
                                        float* flowDirections,
                                        float* failureDepth,
                                        float* algData,
                                        bool* algCalc,
                                        double* transform,
                                        GenericRecursiveFailureAlgebraAlg pfnAlg,
                                        void* pData)
{
        
		//Get cell Index
        int cellIndex = (i*nXSize) + j;
        //Check it hasnt been calculated
        if(!algCalc[cellIndex])
        {
            if (flowDirections[cellIndex] == srcNoDataValue)
            {
                algCalc[cellIndex] = true;
                algData[cellIndex] = 0;
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
                for (int neighbourCell = 0; neighbourCell < 8; ++neighbourCell)
                {
                    if (checkOutofBounds(algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1], nXSize, nYSize))
                    {
                        float flowProp = upstreamProportion(flowDirections, neighbourCell, *algDataIJLookup, nXSize, nYSize, srcNoDataValue);
                        if (flowProp > 0)
                        {
                            ComputeFailureAlgebraValues(srcNoDataValue, dstNoDataValue, algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1],
                                                        nXSize, nYSize, flowDirections, failureDepth, algData, algCalc, transform, pfnAlg, pData);
                        }
                    }
                }
                float algValue = pfnAlg(i, j, nXSize, nYSize, flowDirections, failureDepth, algData, dstNoDataValue, srcNoDataValue, pData); 
                algData[cellIndex] = algValue;
                algCalc[cellIndex] = true;
            }
        }
}

void ComputePropertyAlgebraValues(float srcNoDataValue, float dstNoDataValue,
                                        int i, int j, int nXSize, int nYSize,
                                        float* flowDirections,
                                        float* upstreamProperty,
                                        float* algData,
                                        bool* algCalc,
                                        double* transform,
                                        GenericRecursivePropertyAlgebraAlg pfnAlg,
                                        void* pData)
{
        
        //Get cell Index
        int cellIndex = (i*nXSize) + j;
        //Check it hasnt been calculated
        if(!algCalc[cellIndex])
        {
            if (flowDirections[cellIndex] == srcNoDataValue)
            {
                algCalc[cellIndex] = true;
                algData[cellIndex] = 0;
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
                for (int neighbourCell = 0; neighbourCell < 8; ++neighbourCell)
                {
                    if (checkOutofBounds(algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1], nXSize, nYSize))
                    {
                        float flowProp = upstreamProportion(flowDirections, neighbourCell, *algDataIJLookup, nXSize, nYSize, srcNoDataValue);
                        if (flowProp > 0)
                        {
                            ComputePropertyAlgebraValues(srcNoDataValue, dstNoDataValue, algDataIJLookup[neighbourCell][0], algDataIJLookup[neighbourCell][1],
                                                        nXSize, nYSize, flowDirections, upstreamProperty, algData, algCalc, transform, pfnAlg, pData);
                        }
                    }
                }
                float algValue = pfnAlg(i, j, nXSize, nYSize, flowDirections, upstreamProperty, algData, dstNoDataValue, srcNoDataValue, pData); 
                algData[cellIndex] = algValue;
                algCalc[cellIndex] = true;
            }
        }
}

//Utility program
std::vector<float> FloatArraytoVector(float* fIn)
{
    std::vector<float> vec;
    for (int a = 0; a < 9; ++a)
    {
        vec.push_back(fIn[a]);
    }
    return vec;
}

void GenericGetIJWindow(int i, int j, int nXsize, int nYsize, float* inputData, float* outputWindow)
{
    //Use a 3x3 window over each cell for calcs
    //Middle cell is [4]
    //  
    //  0 1 2
    //  3 4 5
    //  6 7 8
    int algDataIJLookup[9][2] = {i-1,j-1,//0
                        i-1,j,//1
                        i-1,j+1,//2
                        i,j-1,//3
                        i,j,//4
                        i,j+1,//5
                        i+1,j-1,//6
                        i+1,j,//7
                        i+1,j+1};//8

    for (int cell = 0; cell < 9; ++cell)
    {
        if (checkOutofBounds(algDataIJLookup[cell][0],algDataIJLookup[cell][1], nXsize, nYsize))
        {
            outputWindow[cell] = inputData[(algDataIJLookup[cell][0]*nXsize) + algDataIJLookup[cell][1]];
        }
        else
        {
            outputWindow[cell] = 0;
        }
    }
}

CPLErr GenericRecursiveFlowAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursiveFlowAlgebraAlg pfnAlg,
                                                void* pData)
{
    CPLErr eErr;

    int i,j;//Cell index

    
    int nXSize = GDALGetRasterBandXSize(srcFlowBand);//Get length of raster
    int nYSize = GDALGetRasterBandYSize(srcFlowBand);
    
    float srcNoDataValue;
    int srcNoData;

    srcNoDataValue = (float) GDALGetRasterNoDataValue(srcFlowBand, &srcNoData);
    float dstNoDataValue = srcNoDataValue;

    float* flowDirection = new float[nXSize*nYSize];
    bool* algCalc = new bool[nXSize*nYSize]();//Initialise as 0
    float* algData = new float[nXSize*nYSize];
    std::cout << QString("Created empty output arrays") + "\n";
    //Read the inputData
    GDALRasterIO(srcFlowBand, GF_Read,
            0, 0, //X,Y offset in cells
            nXSize, nYSize, //X,Y length in cells
            flowDirection, //data
            nXSize, nYSize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);

    std::cout << QString("Read input array") + "\n";
        

    for (i = 0; i < nYSize; ++i)//rows
    {
        for (j = 0; j < nXSize; ++j)//cols
        {
            ComputeFlowAlgebraValues(srcNoDataValue, dstNoDataValue, i, j, nXSize, nYSize,
                                        flowDirection, algData, algCalc, transform, pfnAlg, pData);
        }
    }

    //Write
    eErr = GDALRasterIO (dstAlgBand,
                            GF_Write,
                            0,0,
                            nXSize,nYSize,
                            algData,
                            nXSize,nYSize,
                            GDT_Float32,
                            0,0);

    return eErr;
}

CPLErr GenericRecursiveFailureAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH failFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursiveFailureAlgebraAlg pfnAlg,
                                                void* pData)
{
    CPLErr eErr;

    int i,j;//Cell index

    
    int nXSize = GDALGetRasterBandXSize(srcFlowBand);//Get length of raster
    int nYSize = GDALGetRasterBandYSize(srcFlowBand);
    
    float srcNoDataValue;
    int srcNoData;

    srcNoDataValue = (float) GDALGetRasterNoDataValue(srcFlowBand, &srcNoData);
    float dstNoDataValue = srcNoDataValue;

    float* flowDirection = new float[nXSize*nYSize];
    float* failureDepth = new float[nXSize*nYSize];
    bool* algCalc = new bool[nXSize*nYSize]();//Initialise as 0
    float* algData = new float[nXSize*nYSize];
    std::cout << QString("Created empty output arrays") + "\n";
    //Read the inputData
    GDALRasterIO(srcFlowBand, GF_Read,
            0, 0, //X,Y offset in cells
            nXSize, nYSize, //X,Y length in cells
            flowDirection, //data
            nXSize, nYSize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);

    GDALRasterIO(failFlowBand, GF_Read,
        0, 0, //X,Y offset in cells
        nXSize, nYSize, //X,Y length in cells
        failureDepth, //data
        nXSize, nYSize, //Number of cells in new dataset
        GDT_Float32, //Type
        0, 0);

    std::cout << QString("Read input arrays of size %1, %2").arg(nXSize).arg(nYSize) + "\n";


    for (i = 0; i < nYSize; ++i)//rows
    {
        for (j = 0; j < nXSize; ++j)//cols
        {
            ComputeFailureAlgebraValues(srcNoDataValue, dstNoDataValue, i, j, nXSize, nYSize,
                                        flowDirection, failureDepth, algData, algCalc, transform, pfnAlg, pData);
        }
    }

    //Write
    eErr = GDALRasterIO (dstAlgBand,
                            GF_Write,
                            0,0,
                            nXSize,nYSize,
                            algData,
                            nXSize,nYSize,
                            GDT_Float32,
                            0,0);

    return eErr;
}

CPLErr GenericRecursivePropertyAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH propFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursivePropertyAlgebraAlg pfnAlg,
                                                void* pData)
{
    CPLErr eErr;

    int i,j;//Cell index

    
    int nXSize = GDALGetRasterBandXSize(srcFlowBand);//Get length of raster
    int nYSize = GDALGetRasterBandYSize(srcFlowBand);
    
    float srcNoDataValue;
    int srcNoData;

    srcNoDataValue = (float) GDALGetRasterNoDataValue(srcFlowBand, &srcNoData);
    float dstNoDataValue = srcNoDataValue;

    float* flowDirection = new float[nXSize*nYSize];
    float* upstreamProperty = new float[nXSize*nYSize];
    bool* algCalc = new bool[nXSize*nYSize]();//Initialise as 0
    float* algData = new float[nXSize*nYSize];
    std::cout << QString("Created empty output arrays") + "\n";
    //Read the inputData
    GDALRasterIO(srcFlowBand, GF_Read,
            0, 0, //X,Y offset in cells
            nXSize, nYSize, //X,Y length in cells
            flowDirection, //data
            nXSize, nYSize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);

    GDALRasterIO(propFlowBand, GF_Read,
        0, 0, //X,Y offset in cells
        nXSize, nYSize, //X,Y length in cells
        upstreamProperty, //data
        nXSize, nYSize, //Number of cells in new dataset
        GDT_Float32, //Type
        0, 0);

    std::cout << QString("Read input arrays of size %1, %2").arg(nXSize).arg(nYSize) + "\n";


    for (i = 0; i < nYSize; ++i)//rows
    {
        for (j = 0; j < nXSize; ++j)//cols
        {
            ComputePropertyAlgebraValues(srcNoDataValue, dstNoDataValue, i, j, nXSize, nYSize,
                                        flowDirection, upstreamProperty, algData, algCalc, transform, pfnAlg, pData);
        }
    }

    //Write
    eErr = GDALRasterIO (dstAlgBand,
                            GF_Write,
                            0,0,
                            nXSize,nYSize,
                            algData,
                            nXSize,nYSize,
                            GDT_Float32,
                            0,0);

    return eErr;
}

/***************************************************************
Takahashi - this one doesn't make sense.
***************************************************************/

void* TakashiCreateInputData(double grainconc, double rhograin, double rho, double frictAngle)
{
    TakashiAlgdata* pData = 
        (TakashiAlgdata*)CPLMalloc(sizeof(TakashiAlgdata));

    pData->grainConc = grainconc;
    pData->frictAngle = frictAngle;
    pData->rhoGrain = rhograin;
    pData->rho = rho;

    return pData;
}


float TakashiEmergenceAlg(float* afWin, float dstNoDataValue, void* pData)
{
    const double degreesToRadians = M_PI / 180.0;
    TakashiAlgdata* psData = (TakashiAlgdata*)pData;

    double tanphi = tan(degreesToRadians*psData->frictAngle);

    float criterion = ((psData->grainConc*(psData->rhoGrain-psData->rho))/(psData->grainConc*(psData->rhoGrain-psData->rho) + 3*psData->rho))*tanphi;

    float fos = criterion/(afWin[4]*degreesToRadians);

    return fos;

}

/**************************************************************
Upslope flow area/accumulation
**************************************************************/

void* RecursiveCreateInputData(double* transform)
{
    RecursiveUpstreamAlgData* pData =
        (RecursiveUpstreamAlgData*)CPLMalloc(sizeof(RecursiveUpstreamAlgData));
        
    pData->nsres = transform[5];
    pData->ewres = transform[1];
    return pData;
}


float RecursiveUpstreamFlowAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData)
{
    RecursiveUpstreamAlgData* psData = (RecursiveUpstreamAlgData*) pData;
    
    double ewres = psData->ewres;
    double nsres = psData->nsres;
    
    float accumulatedFlow = nsres*ewres;
    accumulatedFlow = -accumulatedFlow;

    
    float* aFlowWindow = new float[9];
    float* aAlgWindow = new float[9];

    GenericGetIJWindow(i,j,nXsize, nYsize, flowDirections, aFlowWindow);
    GenericGetIJWindow(i,j,nXsize, nYsize, algData, aAlgWindow);

    int algDataIJLookup[9][2] = {i-1,j-1,//0
                            i-1,j,//1
                            i-1,j+1,//2
                            i,j-1,//3
                            i,j,//4
                            i,j+1,//5
                            i+1,j-1,//6
                            i+1,j,//7
                            i+1,j+1};//8

    int aIJLookup[8][2] = {i-1,j-1,//0
                        i-1,j,//1
                        i-1,j+1,//2
                        i,j-1,//3
                        i,j+1,//5
                        i+1,j-1,//6
                        i+1,j,//7
                        i+1,j+1};//8


    int neighbourCount = 0;
    for (int cell = 0; cell < 9; ++cell)
    {
        if (cell != 4)
        {
            if (checkOutofBounds(algDataIJLookup[cell][0], algDataIJLookup[cell][1], nXsize, nYsize))
            {
                float upProp = upstreamProportion(flowDirections, neighbourCount, *aIJLookup, nXsize, nYsize, srcNoDataValue);
                if (upProp > 0)
                {
                    accumulatedFlow += upProp*aAlgWindow[cell];
                }
            }
            ++neighbourCount;
        }
    }
    return accumulatedFlow;
}

/**************************************************************
Iverson FOS Landslide triggering by infiltration
***************************************************************/

double response(double t)
{
    double respFun = 0.0;
    if (t > 0)
    {
        respFun = sqrt(t/M_PI)*exp(-1/t)-erfc(1/sqrt(t));
    }
    return respFun;
}

double normaliseTime(double t, double Z, double D_eff)
{
    double nTime = 0;
    if (Z > 0)
    {
        nTime = t/(pow(Z,2)/D_eff);
    }
    return nTime; 
}

double determineResponse(double t, double duration, double Z, double D_eff)
{
    double resp = 0;
    if (t <= duration)
    {
        resp = response(normaliseTime(t,Z,D_eff));
    }
    else
    {
        resp = response(normaliseTime(t,Z,D_eff)) - response(normaliseTime(t,Z,D_eff) - normaliseTime(duration,Z,D_eff));
    }
    return resp;
}

double determineUnsteadyFOS(double time, double duration, double waterWeight, double satSoilWeight, double frictionAngleRad, double slopeAngleRad, double IzkSat, double Z, double D_eff)
{
    double fos = 0;
    fos = -(waterWeight/satSoilWeight)*(tan(frictionAngleRad)/(sin(slopeAngleRad)*cos(slopeAngleRad)))*IzkSat*determineResponse(time,duration,Z, D_eff);
    return fos;
}

/**************************************************************
Upslope failure volume calc
**************************************************************/

float RecursiveUpstreamFailureAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* failureDepth, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData)
{
    RecursiveUpstreamAlgData* psData = (RecursiveUpstreamAlgData*) pData;
    
    double ewres = psData->ewres;
    double nsres = psData->nsres;
    
    
    float* aFlowWindow = new float[9];
    float* aFailureWindow = new float[9];
    float* aAlgWindow = new float[9];

    GenericGetIJWindow(i,j,nXsize, nYsize, flowDirections, aFlowWindow);
    GenericGetIJWindow(i,j,nXsize, nYsize, failureDepth, aFailureWindow);
    GenericGetIJWindow(i,j,nXsize, nYsize, algData, aAlgWindow);

    int algDataIJLookup[9][2] = {i-1,j-1,//0
                            i-1,j,//1
                            i-1,j+1,//2
                            i,j-1,//3
                            i,j,//4
                            i,j+1,//5
                            i+1,j-1,//6
                            i+1,j,//7
                            i+1,j+1};//8

    int aIJLookup[8][2] = {i-1,j-1,//0
                        i-1,j,//1
                        i-1,j+1,//2
                        i,j-1,//3
                        i,j+1,//5
                        i+1,j-1,//6
                        i+1,j,//7
                        i+1,j+1};//8

    float accumulatedVol = (nsres*ewres)*MAX(aFailureWindow[4], 0);
    accumulatedVol = -accumulatedVol;
    
    int neighbourCount = 0;
    for (int cell = 0; cell < 9; ++cell)
    {
        if (cell != 4)
        {
            if (checkOutofBounds(algDataIJLookup[cell][0], algDataIJLookup[cell][1], nXsize, nYsize))
            {
                float upProp = upstreamProportion(flowDirections, neighbourCount, *aIJLookup, nXsize, nYsize, srcNoDataValue);
                if (upProp > 0)
                {
                    accumulatedVol += upProp*aAlgWindow[cell];
                }
            }
            ++neighbourCount;
        }
    }
    return accumulatedVol;
}

/**************************************************************
Upslope property calc
**************************************************************/

float RecursiveUpstreamPropAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* upstreamProperty, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData)
{
    RecursiveUpstreamAlgData* psData = (RecursiveUpstreamAlgData*) pData;
    
    double ewres = psData->ewres;
    double nsres = psData->nsres;
    
    
    float* aFlowWindow = new float[9];
    float* aPropertyWindow = new float[9];
    float* aAlgWindow = new float[9];

    GenericGetIJWindow(i,j,nXsize, nYsize, flowDirections, aFlowWindow);
    GenericGetIJWindow(i,j,nXsize, nYsize, upstreamProperty, aPropertyWindow);
    GenericGetIJWindow(i,j,nXsize, nYsize, algData, aAlgWindow);

    int algDataIJLookup[9][2] = {i-1,j-1,//0
                            i-1,j,//1
                            i-1,j+1,//2
                            i,j-1,//3
                            i,j,//4
                            i,j+1,//5
                            i+1,j-1,//6
                            i+1,j,//7
                            i+1,j+1};//8

    int aIJLookup[8][2] = {i-1,j-1,//0
                        i-1,j,//1
                        i-1,j+1,//2
                        i,j-1,//3
                        i,j+1,//5
                        i+1,j-1,//6
                        i+1,j,//7
                        i+1,j+1};//8

    float upstreamValue = aPropertyWindow[4];
    
    int neighbourCount = 0;
    for (int cell = 0; cell < 9; ++cell)
    {
        if (cell != 4)
        {
            if (checkOutofBounds(algDataIJLookup[cell][0], algDataIJLookup[cell][1], nXsize, nYsize))
            {
                float upProp = upstreamProportion(flowDirections, neighbourCount, *aIJLookup, nXsize, nYsize, srcNoDataValue);
                if (upProp > 0)
                {
                    upstreamValue += upProp*aAlgWindow[cell];
                }
            }
            ++neighbourCount;
        }
    }
    return upstreamValue;
}
