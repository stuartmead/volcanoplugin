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

#ifndef RF_EROSIONUTILS_H
#define RF_EROSIONUTILS_H

#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#include "volcanoutils.h"
#include "volcanoplugin.h"

#include "Workspace/DataExecution/DataObjects/datafactorytraits.h"
#include "Workspace/DataExecution/DataObjects/enumtointadaptor.h"

/***************************************************************
Recursive flow algebra
***************************************************************/

//Generic datatype for flow algebra algorithim conatining: Flow angles, flowCalcs from algebra, nodata, program data (constants), and extra neighbouring windows of data
typedef float (*GenericRecursiveFlowAlgebraAlg) (int i, int j, int nXsize, int nYsize, float* aFlowWindow, float* aAlgWindow, float fDstNoDataValue, float fSrcNoDataValue, void* pData);

//Generic datatype for flow algebra algorithim conatining: Flow angles, failure volume, flowCalcs from algebra, nodata, program data (constants), and extra neighbouring windows of data
typedef float (*GenericRecursiveFailureAlgebraAlg) (int i, int j, int nXsize, int nYsize, float* aFlowWindow, float* aFailureWindow, float* aAlgWindow, float fDstNoDataValue, float fSrcNoDataValue, void* pData);
//Generic datatype for flow algebra algorithim conatining: Flow angles, flowCalcs from algebra, nodata, program data (constants), and extra neighbouring windows of data
typedef float (*GenericRecursivePropertyAlgebraAlg) (int i, int j, int nXsize, int nYsize, float* aFlowWindow, float* aPropWindow, float* aAlgWindow, float fDstNoDataValue, float fSrcNoDataValue, void* pData);


//Utility to check if you are out of bounds.
bool checkOutofBounds(int i, int j, int nXsize, int nYsize);

//Calculate upstream flow proportion
float upstreamProportion(float* flowWindow, int neighBour, int* ijlookup, int nXsize, int nYsize, float srcNodataValue);

//Utility function to convert floats to vector
std::vector<float> FloatArraytoVector(float* fIn);


//Compute the values using the algo
void ComputeFlowAlgebraValues(float srcNoDataValue, float dstNoDataValue,
                                        int i, int j, int nXSize, int nYSize,
                                        float* flowDirections,
                                        float* algData,
                                        bool* algCalc,
                                        double* transform,
                                        GenericRecursiveFlowAlgebraAlg pfnAlg,
                                        void* pData);

//Generic recursive processor
CPLErr GenericRecursiveFlowAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursiveFlowAlgebraAlg pfnAlg,
                                                void* pData);

CPLErr GenericRecursiveFailureAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH failFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursiveFailureAlgebraAlg pfnAlg,
                                                void* pData);

CPLErr GenericRecursivePropertyAlgebraProcessor(GDALRasterBandH srcFlowBand,
                                                GDALRasterBandH propFlowBand,
                                                GDALRasterBandH dstAlgBand,
                                                double* transform,
                                                GenericRecursivePropertyAlgebraAlg pfnAlg,
                                                void* pData);

void GenericGetIJWindow(int i, int j, int nXsize, int nYsize, float* inputData, float* outputWindow);

/****************************************************************
Takashi's debris flow initiation criteria
****************************************************************/
//Generate data
typedef struct
{
    double grainConc;
    double rhoGrain;
    double rho;
    double frictAngle;
} TakashiAlgdata;

void* TakashiCreateInputData(double grainconc, double rhograin, double rho, double frictAngle);

//Failure
float TakashiEmergenceAlg(float* afWin, float dstNoDataValue, void* pData);

/*****************************************************************
Recursive upstream flow accumulation
*****************************************************************/
typedef struct
{
    double nsres;
    double ewres;
} RecursiveUpstreamAlgData;

void* RecursiveCreateInputData(double* transform);

float RecursiveUpstreamFlowAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData);

/**************************************************************
Iverson FOS Landslide triggering by infiltration
***************************************************************/

double response(double t);

double normaliseTime(double t, double Z, double D_eff);

double determineResponse(double t, double duration, double Z, double D_eff);

double determineUnsteadyFOS(double time, double duration, double waterWeight, double satSoilWeight, double frictionAngleRad, double slopeAngleRad, double IzkSat, double Z, double D_eff);

//Lovingly plagiarised from john cook.
double erf(double x);

double erfc(double x);

void testErf();

/**************************************************************
Upslope failure volume calc
**************************************************************/

float RecursiveUpstreamFailureAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* failureDepth, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData);

/**************************************************************
Upslope property values calc
**************************************************************/

float RecursiveUpstreamPropAlg (int i, int j, int nXsize, int nYsize, float* flowDirections, float* upstreamProperty, float* algData, float dstNoDataValue, float srcNoDataValue, void* pData);


#endif
