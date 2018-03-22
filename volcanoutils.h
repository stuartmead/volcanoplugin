 /*******************************************************************************
 * Copyright (c) 2006, 2009 Matthew Perry 
 * Copyright (c) 2009-2013, Even Rouault <even dot rouault at mines-paris dot org>
 * Portions derived from GRASS 4.1 (public domain) See 
 * http://trac.osgeo.org/ticket/2975 for more information regarding 
 * history of this code
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

  This is the header file for a bunch of utility functions needed in the plugin.
*/

#ifndef RF_VOLCANOUTILS_H
#define RF_VOLCANOUTILS_H

#include <iostream>

#include <qstring.h>

#include "Workspace/Application/LanguageUtils/streamqstring.h"


#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#include "volcanoplugin.h"

#include "Workspace/DataExecution/DataObjects/datafactorytraits.h"
#include "Workspace/DataExecution/DataObjects/enumtointadaptor.h"

namespace RF
{
    enum SlopeAlgType
    {
        THORNE,
        ZEVTHORNE
    };

    enum CurvatureType
    {
        PROFILE,
        CONTOUR,
        TANGENTIAL
    };

    enum FlowDirType
    {
        DINF,
        D8
    };

	enum FuzzyMembershipType
	{
		CONSTANT,
		LINEAR,
		GAUSSIAN
	};
}

namespace CSIRO
{
    namespace DataExecution
    {
        template <> inline void getEnumNames<RF::SlopeAlgType>(QStringList& names)
        {
            names.push_back("Thorne");
            names.push_back("Zevenbergen-Thorne");
        }

        template <> inline void getEnumNames<RF::CurvatureType>(QStringList& names)
        {
            names.push_back("Profile");
            names.push_back("Contour/Planform");
            names.push_back("Tangential");
        }

        template <> inline void getEnumNames<RF::FlowDirType>(QStringList& names)
        {
            names.push_back("D-infinity");
            names.push_back("D-8");
        }

		template <> inline void getEnumNames<RF::FuzzyMembershipType>(QStringList& names)
		{
			names.push_back("Constant");
			names.push_back("Linear");
			names.push_back("Gaussian");
		}
    }
}



typedef float (*GDALGeneric3x3ProcessingAlg) (float* pafWindow, float fDstNoDataValue, void* pData);//A datatype contatining the 3 line buffer, nodata value and data

//Generic value computer for 3x3 windows
static float ComputeVal(bool srcNoData, float srcNoDataValue,
                            float* afWin, float dstNoDataValue,
                            GDALGeneric3x3ProcessingAlg pfnAlg,
                            void* pData,
                            bool computeEdges);

//3x3 Processor
CPLErr GDALGeneric3x3Processing (GDALRasterBandH srcBand,
                                   GDALRasterBandH dstBand,
                                   GDALGeneric3x3ProcessingAlg pfnAlg,
                                   void* pData,
                                   bool computeEdges);//Could use progress



float * getRasterData(GDALDatasetH raster, float dstNodataValue,
	int xOffset = 0, int yOffset = 0, int xLength = 0, int yLength = 0, double scaleFactor = 1.0, int bandNo = 1);


/***************************************
SLOPE
***************************************/

//Slope calculation struct
typedef struct
{
    double nsres;
    double ewres;
    double scale;
    int slopeFormat;//0 = percent, 1 = degrees
} GDALSlopeAlgData;

//Generate slope data
void*  GDALCreateSlopeData(double* adfGeoTransform,
                           double scale,
                           int slopeFormat);

//Horn Algorithim: Horn, B. K. P. (1981). Hill Shading and the Reflectance Map.  IEEE 69(1)
float GDALSlopeHornAlg (float* afWin, float dstNoDataValue, void* pData);

//ZevenbergenThorneSlopeAlgorithim
float GDALSlopeZevenbergenThorneAlg (float* afWin, float dsNoDataValue, void* pData);

/****************************************************
ASPECT
****************************************************/
typedef struct
{
    bool useAngleAsAzimuth;
} GDALAspectAlgData;

//Create data struct
void*  GDALCreateAspectData(bool useAngleAsAzimuth);

    //Horn-Like algo
float GDALAspectAlg (float* afWin, float dstNoDataValue, void* pData);

//ZevenBergenThorneAlgo
float GDALAspectZevenbergenThorneAlg (float* afWin, float dstNoDataValue, void* pData);

/***************************************************
CURVATURE
***************************************************/
typedef struct
{
    double nsres;
    double ewres;
} GDALCurvatureAlgData;

//Create struct
void* GDALCreateCurvatureData(double* adfTransform);

//Profile curvature
float GDALCurvatureProfileAlg(float* afWin, float dstNoDataValue, void* pData);
//Contour/Planform curvature
float GDALCurvatureContourAlg(float* afWin, float dstNoDataValue, void* pData);
//Tangential curvature
float GDALCurvatureTangentAlg(float* afWin, float dstNoDataValue, void* pData);

/***************************************************
FLOWDIR
***************************************************/
typedef struct
{
    double nsres;
    double ewres;
} GDALFlowDirAlgData;

struct facet
{
    double s[2];
    double fdir;
    int e1;
    int e2;
    int ac;
    int af;
    double s_mag;
};

void* GDALCreateFlowDirectionData(double* adfTransform);

float GDALFlowDirectionInfAlg(float* afWin, float dstNoDataValue, void* pData);

float GDALFlowDirection8Alg(float* afWin, float dstNoDataValue, void* pData);

float GDALFlowDirection8IterativeAlg(float* afWin, float dstNoDataValue, void* pData);

float GDALFillDepressionsAlg(float* afWin, float dstNoDataValue, void* pData);



DECLARE_WORKSPACE_DATA_FACTORY(RF::SlopeAlgType, RF_API)
DECLARE_WORKSPACE_ENUMTOINTADAPTOR(RF::SlopeAlgType, RF_API)

DECLARE_WORKSPACE_DATA_FACTORY(RF::CurvatureType, RF_API)
DECLARE_WORKSPACE_ENUMTOINTADAPTOR(RF::CurvatureType, RF_API)

DECLARE_WORKSPACE_DATA_FACTORY(RF::FlowDirType, RF_API)
DECLARE_WORKSPACE_ENUMTOINTADAPTOR(RF::FlowDirType, RF_API)

DECLARE_WORKSPACE_DATA_FACTORY(RF::FuzzyMembershipType, RF_API)
DECLARE_WORKSPACE_ENUMTOINTADAPTOR(RF::FuzzyMembershipType, RF_API)
#endif
