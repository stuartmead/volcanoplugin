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

*/

#include <stdlib.h>
#include <math.h>

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"


#include "Workspace/DataExecution/DataObjects/typeddatafactory.h"

#include "volcanoutils.h"


#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif

#define INTERPOL(a,b) ((srcNoData && (ARE_REAL_EQUAL(a, srcNoDataValue) || ARE_REAL_EQUAL(b, srcNoDataValue))) ? srcNoDataValue : 2 * (a) - (b))

#ifndef ROOT_2
#define ROOT_2 sqrtf(2)
#endif

/*
Write out raster band
*/
CPLErr writeRasterData(GDALDatasetH dataset, GDALDriverH driver, double * transform, int rasterXsize, int rasterYsize, float noDataValue, float * data, const char * filename) 
{
	//Create dataset - only works for 1 band
	dataset = GDALCreate(driver, filename, rasterXsize, rasterYsize, 1, GDT_Float32, NULL);
	if (dataset == NULL) {
		std::cout << QString("ERROR: Cannot create GDAL dataset %1").arg(filename) + "\n";
		return CPLErr::CE_Failure;
	}
	GDALRasterBandH band = GDALGetRasterBand(dataset, 1);
	if (band == NULL) {
		std::cout << QString("ERROR: Cannot access dataset %1 band").arg(filename) + "\n";
		return CPLErr::CE_Failure;
	}
	
	if (GDALSetGeoTransform(dataset, transform) != CPLErr::CE_None)
	{
		std::cout << QString("ERROR: Cannot create GDAL dataset %1 transform").arg(filename) + "\n";
		std::cout << QString("Transform is %1, %2, %3, %4, %5, %6").arg(transform[0]).arg(transform[1])
			.arg(transform[2]).arg(transform[3]).arg(transform[4]).arg(transform[5]) + "\n";
		return CPLErr::CE_Failure;
	}
	if (GDALSetRasterNoDataValue(band, noDataValue) != CPLErr::CE_None) 
	{
		std::cout << QString("ERROR: Cannot create assign NoData to dataset %1").arg(filename) + "\n";
		return CPLErr::CE_Failure;
	}
	CPLErr error = GDALRasterIO(band, GF_Write,
		0, 0,
		rasterXsize, rasterYsize,
		data,
		rasterXsize, rasterYsize,
		GDT_Float32,
		0, 0);
	GDALClose(dataset);
	return error;
}

/*
getRasterData: Return a float array from the dataset 
*/
float * getRasterData(GDALDatasetH raster, float dstNodataValue,
	int xOffset, int yOffset, int xLength, int yLength, double scaleFactor, int bandNo)
{
	if (bandNo > GDALGetRasterCount(raster))
	{
		std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(raster)).arg(bandNo) + "\n";
	}

	double sizes[2];
	if (xLength <= 0)
	{
		sizes[0] = GDALGetRasterXSize(raster) - xOffset;
	}
	else
	{
		sizes[0] = xLength;
	}

	if (yLength <= 0)
	{
		sizes[1] = GDALGetRasterYSize(raster) - yOffset;
	}
	else
	{
		sizes[1] = yLength;
	}

	GDALRasterBandH band = GDALGetRasterBand(raster, bandNo);
	
	dstNodataValue = GDALGetRasterNoDataValue(band, NULL);

	float * data = new float[GDALGetRasterYSize(raster)*GDALGetRasterXSize(raster)];

	if (GDALRasterIO(band, GF_Read,
		xOffset, yOffset, //X,Y offset in cells
		sizes[0], sizes[1], //X,Y length in cells
		data, //data
		floor(sizes[0] / scaleFactor), floor(sizes[1] / scaleFactor), //Number of cells in new dataset
		GDT_Float32, //Type
		0, 0) != CE_None)
	{
		std::cout << QString("Error: There was an issue reading the raster band") + "\n";
    }
	else
	{
		return data;
	}
}

/*
ComputeVal: Computes the value using a processing algorithim defined here - checking for nodata
*/
static float ComputeVal(bool srcNoData, float srcNoDataValue,
                            float* afWin, float dstNoDataValue,
                            GDALGeneric3x3ProcessingAlg pfnAlg,
                            void* pData,
                            bool computeEdges)
{
    if (srcNoData && ARE_REAL_EQUAL(afWin[4], srcNoDataValue))//Do not calculate if no data  
    {
        return dstNoDataValue;
    }
    else 
    {
        if (srcNoData)
        {
            for (int k=0; k<9; k++)
            {
                if (ARE_REAL_EQUAL(afWin[k], srcNoDataValue))//Check each cell for nodata
                {
                    if (computeEdges)
                    {
                        afWin[k] = afWin[4]; //Set the edge to the centre cell value (ok for gradient)
                    }
                    else
                    {
                        return dstNoDataValue;
                    }
                }
            }
         }
        return pfnAlg(afWin, dstNoDataValue, pData); //Run the calculations
    }
}
/*
3x3 grid processor for algorithms
*/
CPLErr GDALGeneric3x3Processing (GDALRasterBandH srcBand,
                                   GDALRasterBandH dstBand,
                                   GDALGeneric3x3ProcessingAlg pfnAlg,
                                   void* pData,
                                   bool computeEdges)
{
    CPLErr eErr;
    float *pafThreeLineWin; //3 line input buffer
    float *pafOutputBuf; //1 line dest buffer
    int i,j; //Cell index

    int srcNoData, dstNoData; //Really a bool
    float srcNoDataValue, dstNoDataValue;

    srcNoDataValue = (float) GDALGetRasterNoDataValue(srcBand, &srcNoData);
    dstNoDataValue = (float) GDALGetRasterNoDataValue(dstBand, &dstNoData);

    if (!dstNoData)
    {
         dstNoDataValue = 0.0;
    }

    int nXSize = GDALGetRasterBandXSize(srcBand); //Get length of raster
    int nYSize = GDALGetRasterBandYSize(srcBand);

    pafOutputBuf = new float [nXSize];
    pafThreeLineWin = new float [3*(nXSize + 1)];

    //Use a 3x3 window over each cell for calcs
    //Middle cell is [4]
    //
    //  0 1 2
    //  3 4 5
    //  6 7 8

    //First 2 lines
    for (i = 0; i < 2 && i < nYSize; i++)
    {
        GDALRasterIO ( srcBand,
                        GF_Read,
                        0,i,
                        nXSize,1,
                        pafThreeLineWin + i * nXSize,
                        nXSize, 1,
                        GDT_Float32,
                        0, 0);
    }

    if (computeEdges && nXSize >= 2 && nYSize >=2) //If compute edges is on, we need to interpolate the first line
    {
        for (j=0; j < nXSize; j++)
        {
            float afWin[9];
            int jmin = (j == 0) ? j : j - 1;
            int jmax = (j == nXSize-1) ? j : j + 1;

            afWin[0] = INTERPOL(pafThreeLineWin[jmin], pafThreeLineWin[nXSize + jmin]);
            afWin[1] = INTERPOL(pafThreeLineWin[j], pafThreeLineWin[nXSize + j]);
            afWin[2] = INTERPOL(pafThreeLineWin[jmax], pafThreeLineWin[nXSize + jmax]);
            afWin[3] = pafThreeLineWin[jmin];
            afWin[4] = pafThreeLineWin[j];
            afWin[5] = pafThreeLineWin[jmax];
            afWin[6] = pafThreeLineWin[nXSize + jmin];
            afWin[7] = pafThreeLineWin[nXSize + j];
            afWin[8] = pafThreeLineWin[nXSize + jmax];

            pafOutputBuf[j] = ComputeVal(srcNoData, srcNoDataValue,
                                            afWin, dstNoDataValue,
                                            pfnAlg, pData, computeEdges);
        }
        GDALRasterIO(dstBand, GF_Write,
                    0, 0, nXSize, 1,
                    pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);
    }
    else //No edges
    {
        for (j = 0; j < nXSize; j++)
        {
            pafOutputBuf[j] = dstNoDataValue;
        }
        GDALRasterIO(dstBand, GF_Write,
                    0, 0, nXSize, 1,
                    pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);
    
        if (nYSize > 1)
        {
            GDALRasterIO(dstBand, GF_Write,
                        0, nYSize - 1, nXSize, 1,
                        pafOutputBuf, nXSize, 1, GDT_Float32, 0, 0);
        }
    }

    //Now do the rest of the lines

    //Linebuffer offsets
    int nLine1Off = 0*nXSize;
    int nLine2Off = 1*nXSize;
    int nLine3Off = 2*nXSize;

    for (i = 1; i < nYSize - 1; i++)
    {
        //Read line 3
        eErr = GDALRasterIO (srcBand, GF_Read,
                                0, i + 1,
                                nXSize, 1,
                                pafThreeLineWin + nLine3Off,
                                nXSize, 1,
                                GDT_Float32,
                                0, 0);
        if (eErr != CE_None)
        {
            //Error handling here
        }

        
        if (computeEdges && nXSize >=2)
        {
            float afWin[9];

            //For the first cell, need to interpolate cell 0,3,6
            j = 0;
            afWin[0] = INTERPOL(pafThreeLineWin[nLine1Off + j], pafThreeLineWin[nLine1Off + j+1]);
            afWin[1] = pafThreeLineWin[nLine1Off + j];
            afWin[2] = pafThreeLineWin[nLine1Off + j+1];
            afWin[3] = INTERPOL(pafThreeLineWin[nLine2Off + j], pafThreeLineWin[nLine2Off + j+1]);
            afWin[4] = pafThreeLineWin[nLine2Off + j];
            afWin[5] = pafThreeLineWin[nLine2Off + j+1];
            afWin[6] = INTERPOL(pafThreeLineWin[nLine3Off + j], pafThreeLineWin[nLine3Off + j+1]);
            afWin[7] = pafThreeLineWin[nLine3Off + j];
            afWin[8] = pafThreeLineWin[nLine3Off + j+1];

            pafOutputBuf[j] = ComputeVal(srcNoData, srcNoDataValue,
                                         afWin, dstNoDataValue,
                                         pfnAlg, pData, computeEdges);
            
            //For the last cell, need to interpolate cell 2,5,8
            j = nXSize - 1;
            afWin[0] = pafThreeLineWin[nLine1Off + j-1];
            afWin[1] = pafThreeLineWin[nLine1Off + j];
            afWin[2] = INTERPOL(pafThreeLineWin[nLine1Off + j], pafThreeLineWin[nLine1Off + j-1]);
            afWin[3] = pafThreeLineWin[nLine2Off + j-1];
            afWin[4] = pafThreeLineWin[nLine2Off + j];
            afWin[5] = INTERPOL(pafThreeLineWin[nLine2Off + j], pafThreeLineWin[nLine2Off + j-1]);
            afWin[6] = pafThreeLineWin[nLine3Off + j-1];
            afWin[7] = pafThreeLineWin[nLine3Off + j];
            afWin[8] = INTERPOL(pafThreeLineWin[nLine3Off + j], pafThreeLineWin[nLine3Off + j-1]);

            pafOutputBuf[j] = ComputeVal(srcNoData, srcNoDataValue,
                                         afWin, dstNoDataValue,
                                         pfnAlg, pData, computeEdges);

        }
        else //No edges
        {
            // Exclude the edges
            pafOutputBuf[0] = dstNoDataValue;
            if (nXSize > 1)
                pafOutputBuf[nXSize - 1] = dstNoDataValue;
        }

        //Now loop through the rest of the cells
        for (j = 1; j < nXSize - 1; j++)
        {
            float afWin[9];
            afWin[0] = pafThreeLineWin[nLine1Off + j-1];
            afWin[1] = pafThreeLineWin[nLine1Off + j];
            afWin[2] = pafThreeLineWin[nLine1Off + j+1];
            afWin[3] = pafThreeLineWin[nLine2Off + j-1];
            afWin[4] = pafThreeLineWin[nLine2Off + j];
            afWin[5] = pafThreeLineWin[nLine2Off + j+1];
            afWin[6] = pafThreeLineWin[nLine3Off + j-1];
            afWin[7] = pafThreeLineWin[nLine3Off + j];
            afWin[8] = pafThreeLineWin[nLine3Off + j+1];

            pafOutputBuf[j] = ComputeVal(srcNoData, srcNoDataValue,
                                            afWin, dstNoDataValue,
                                            pfnAlg, pData, computeEdges);
        }

        //Now write the line to the buffer

        eErr = GDALRasterIO(dstBand, GF_Write,
                              0,i,//i is line
                              nXSize, 1,//Row by row
                              pafOutputBuf,
                              nXSize, 1,
                              GDT_Float32,
                              0,0);

        if (eErr != CE_None)
        {
            //do error handling here
        }

        //Move lines
        int nTemp = nLine1Off;
        nLine1Off = nLine2Off;
        nLine2Off = nLine3Off;
        nLine3Off = nTemp;
    }

    if(computeEdges && nXSize >= 2 && nYSize >=2) //Write last line
    {
        for(j=0; j < nXSize; j++)
        {
            float afWin[9];
            int jmin = (j == 0) ? j : j - 1;
            int jmax = (j == nXSize - 1) ? j : j + 1;

            //Bottom row needs to be interpolated
            afWin[0] = pafThreeLineWin[nLine1Off + jmin];
            afWin[1] = pafThreeLineWin[nLine1Off + j];
            afWin[2] = pafThreeLineWin[nLine1Off + jmax];
            afWin[3] = pafThreeLineWin[nLine2Off + jmin];
            afWin[4] = pafThreeLineWin[nLine2Off + j];
            afWin[5] = pafThreeLineWin[nLine2Off + jmax];
            afWin[6] = INTERPOL(pafThreeLineWin[nLine2Off + jmin], pafThreeLineWin[nLine1Off + jmin]);
            afWin[7] = INTERPOL(pafThreeLineWin[nLine2Off + j],    pafThreeLineWin[nLine1Off + j]);
            afWin[8] = INTERPOL(pafThreeLineWin[nLine2Off + jmax], pafThreeLineWin[nLine1Off + jmax]);

            pafOutputBuf[j] = ComputeVal(srcNoData, srcNoDataValue,
                                            afWin, dstNoDataValue,
                                            pfnAlg, pData, computeEdges);
        }
        GDALRasterIO(dstBand, GF_Write,
                        0, i,
                        nXSize, 1,
                        pafOutputBuf,
                        nXSize, 1,
                        GDT_Float32,
                        0, 0);
    }
    
    return eErr;
}


/*
Horn Slope algorithim
*/
float GDALSlopeHornAlg (float* afWin, float dstNoDataValue, void* pData)
{
    const double radiansToDegrees = 180.0 / M_PI;
    GDALSlopeAlgData* psData = (GDALSlopeAlgData*)pData;
    double dx, dy, key;
    
    dx = ((afWin[0] + afWin[3] + afWin[3] + afWin[6]) - 
          (afWin[2] + afWin[5] + afWin[5] + afWin[8]))/psData->ewres;

    dy = ((afWin[6] + afWin[7] + afWin[7] + afWin[8]) - 
          (afWin[0] + afWin[1] + afWin[1] + afWin[2]))/psData->nsres;

    key = (dx * dx + dy * dy);

    if (psData->slopeFormat == 1) 
        return (float) (atan(sqrt(key) / (8*psData->scale)) * radiansToDegrees);
    else
        return (float) (100*(sqrt(key) / (8*psData->scale)));
}
/*
Zevenbergen-Thorne Slope algorithim
*/
float GDALSlopeZevenbergenThorneAlg (float* afWin, float dsNoDataValue, void* pData)
{
    const double radiansToDegrees = 180.0 / M_PI;
    GDALSlopeAlgData* psData = (GDALSlopeAlgData*)pData;
    double dx, dy, key;
    
    dx = (afWin[3] - afWin[5])/psData->ewres;

    dy = (afWin[7] - afWin[1])/psData->nsres;

    key = (dx * dx + dy * dy);

    if (psData->slopeFormat == 1) 
        return (float) (atan(sqrt(key) / (2*psData->scale)) * radiansToDegrees);
    else
        return (float) (100*(sqrt(key) / (2*psData->scale)));
}
/*
Slope data functions
*/
void*  GDALCreateSlopeData(double* adfGeoTransform,
                           double scale,
                           int slopeFormat)
{
    GDALSlopeAlgData* pData =
        (GDALSlopeAlgData*)CPLMalloc(sizeof(GDALSlopeAlgData));
        
    pData->nsres = adfGeoTransform[5];
    pData->ewres = adfGeoTransform[1];
    pData->scale = scale;
    pData->slopeFormat = slopeFormat;
    return pData;
}
/*
Horne like aspect
*/
float GDALAspectAlg (float* afWin, float dstNoDataValue, void* pData)
{
    const double degreesToRadians = M_PI / 180.0;
    GDALAspectAlgData* psData = (GDALAspectAlgData*)pData;
    double dx, dy;
    float aspect;
    
    dx = ((afWin[2] + afWin[5] + afWin[5] + afWin[8]) -
          (afWin[0] + afWin[3] + afWin[3] + afWin[6]));

    dy = ((afWin[6] + afWin[7] + afWin[7] + afWin[8]) - 
          (afWin[0] + afWin[1] + afWin[1] + afWin[2]));

    aspect = (float) (atan2(dy,-dx) / degreesToRadians);

    if (dx == 0 && dy == 0)
    {
        /* Flat area */
        aspect = dstNoDataValue;
    } 
    else if ( psData->useAngleAsAzimuth )
    {
        if (aspect > 90.0) 
            aspect = 450.0f - aspect;
        else
            aspect = 90.0f - aspect;
    }
    else
    {
        if (aspect < 0)
            aspect += 360.0;
    }

    if (aspect == 360.0) 
        aspect = 0.0;

    return aspect;
}

/*
Zev-Thorne
*/
float GDALAspectZevenbergenThorneAlg (float* afWin, float dstNoDataValue, void* pData)
{
    const double degreesToRadians = M_PI / 180.0;
    GDALAspectAlgData* psData = (GDALAspectAlgData*)pData;
    double dx, dy;
    float aspect;
    
    dx = (afWin[5] - afWin[3]);

    dy = (afWin[7] - afWin[1]);

    aspect = (float) (atan2(dy,-dx) / degreesToRadians);

    if (dx == 0 && dy == 0)
    {
        /* Flat area */
        aspect = dstNoDataValue;
    } 
    else if ( psData->useAngleAsAzimuth)
    {
        if (aspect > 90.0) 
            aspect = 450.0f - aspect;
        else
            aspect = 90.0f - aspect;
    }
    else
    {
        if (aspect < 0)
            aspect += 360.0;
    }

    if (aspect == 360.0) 
        aspect = 0.0;

    return aspect;
}

//Create data
void*  GDALCreateAspectData(bool useAngleAsAzimuth)
{
    GDALAspectAlgData* pData =
        (GDALAspectAlgData*)CPLMalloc(sizeof(GDALAspectAlgData));
        
    pData->useAngleAsAzimuth = useAngleAsAzimuth;
    return pData;
}


/*
Curvature
*/
void* GDALCreateCurvatureData(double* adfTransform)
{
    GDALCurvatureAlgData* pData =
        (GDALCurvatureAlgData*)CPLMalloc(sizeof(GDALCurvatureAlgData));
        
    pData->nsres = adfTransform[5];
    pData->ewres = adfTransform[1];
    return pData;
}

float GDALCurvatureProfileAlg(float* afWin, float dstNoDataValue, void*pData)
{
    double zx, zy, zxx, zyy, zxy, z2x, z2y;
    GDALCurvatureAlgData* psData = (GDALCurvatureAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = psData->nsres;

    zx = (afWin[2] + afWin[5] + afWin[8] - afWin[0] - afWin[3] - afWin[6])/(6*ewres);
    zy = (afWin[0] + afWin[1] +afWin[2] - afWin[6] - afWin[7] - afWin[8])/(6*nsres);
    zxx = (afWin[0] + afWin[2] + afWin[3] + afWin[5] + afWin[6] + afWin[8] - 2*(afWin[1]+afWin[4]+afWin[7]))/(3*pow(ewres, 2));
    zyy = (afWin[0] + afWin[1] + afWin[2] + afWin[6] + afWin[7] + afWin[8] - 2*(afWin[3]+afWin[4]+afWin[5]))/(3*pow(nsres, 2));
    zxy = (afWin[2] + afWin[6] - afWin[0] - afWin[8])/(4*(pow(ewres,2)));
    z2x = pow(zx,2);
    z2y = pow(zy,2);

    if (zx == 0 && zy == 0)
    {
        /* Flat area */
        return dstNoDataValue;
    }
    else
    {
        return (float) ((zxx*z2x)+(2*zxy*zx*zy)+(zyy*z2y))/((z2x + z2y)*pow(sqrt(abs(1+z2x+z2y)),3));
    }
}

float GDALCurvatureContourAlg(float* afWin, float dstNoDataValue, void* pData)
{
    double zx, zy, zxx, zyy, zxy, z2x, z2y;
    GDALCurvatureAlgData* psData = (GDALCurvatureAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = psData->nsres;

    zx = (afWin[2] + afWin[5] + afWin[8] - afWin[0] - afWin[3] - afWin[6])/(6*ewres);
    zy = (afWin[0] + afWin[1] +afWin[2] - afWin[6] - afWin[7] - afWin[8])/(6*nsres);
    zxx = (afWin[0] + afWin[2] + afWin[3] + afWin[5] + afWin[6] + afWin[8] - 2*(afWin[1]+afWin[4]+afWin[7]))/(3*pow(ewres, 2));
    zyy = (afWin[0] + afWin[1] + afWin[2] + afWin[6] + afWin[7] + afWin[8] - 2*(afWin[3]+afWin[4]+afWin[5]))/(3*pow(nsres, 2));
    zxy = (afWin[2] + afWin[6] - afWin[0] - afWin[8])/(4*(pow(ewres,2)));
    z2x = pow(zx,2);
    z2y = pow(zy,2);

    if (zx == 0 && zy == 0)
    {
        /* Flat area */
        return dstNoDataValue;
    }
    else
    {
        return (float) ((zxx*z2x)-(2*zxy*zx*zy)+(zyy*z2y))/pow(sqrt(abs(z2x+z2y)),3);
    }
}

float GDALCurvatureTangentAlg(float* afWin, float dstNoDataValue, void* pData)
{
    double zx, zy, zxx, zyy, zxy, z2x, z2y;
    GDALCurvatureAlgData* psData = (GDALCurvatureAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = psData->nsres;

    zx = (afWin[2] + afWin[5] + afWin[8] - afWin[0] - afWin[3] - afWin[6])/(6*ewres);
    zy = (afWin[0] + afWin[1] +afWin[2] - afWin[6] - afWin[7] - afWin[8])/(6*nsres);
    zxx = (afWin[0] + afWin[2] + afWin[3] + afWin[5] + afWin[6] + afWin[8] - 2*(afWin[1]+afWin[4]+afWin[7]))/(3*pow(ewres, 2));
    zyy = (afWin[0] + afWin[1] + afWin[2] + afWin[6] + afWin[7] + afWin[8] - 2*(afWin[3]+afWin[4]+afWin[5]))/(3*pow(nsres, 2));
    zxy = (afWin[2] + afWin[6] - afWin[0] - afWin[8])/(4*(pow(ewres,2)));
    z2x = pow(zx,2);
    z2y = pow(zy,2);

    if (zx == 0 && zy == 0)
    {
        /* Flat area */
        return dstNoDataValue;
    }
    else
    {
    return (float) ((zxx*z2x)-(2*zxy*zx*zy)+(zyy*z2y))/((z2x+z2y)+sqrt(abs(1+z2x+z2y)));
    }
}

/*
Flow direction
*/
void* GDALCreateFlowDirectionData(double* adfTransform)
{
    GDALFlowDirAlgData* pData = (GDALFlowDirAlgData*)CPLMalloc(sizeof(GDALFlowDirAlgData));

    pData->nsres = adfTransform[5];
    pData->ewres = adfTransform[1];

    return pData;
}

float GDALFlowDirectionInfAlg(float* afWin, float dstNoDataValue, void* pData)
{
    const double radiansToDegrees = 180.0 / M_PI;

    GDALFlowDirAlgData* psData = (GDALFlowDirAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = psData->nsres;
    nsres = -nsres;

    struct facet facets[8];
    
    facets[0].e1 = 5;
    facets[0].e2 = 2;
    facets[0].ac = 0;
    facets[0].af = 1;

    facets[1].e1 = 1;
    facets[1].e2 = 2;
    facets[1].ac = 1;
    facets[1].af = -1;

    facets[2].e1 = 1;
    facets[2].e2 = 0;
    facets[2].ac = 1;
    facets[2].af = 1;

    facets[3].e1 = 3;
    facets[3].e2 = 0;
    facets[3].ac = 2;
    facets[3].af = -1;

    facets[4].e1 = 3;
    facets[4].e2 = 6;
    facets[4].ac = 2;
    facets[4].af = 1;

    facets[5].e1 = 7;
    facets[5].e2 = 6;
    facets[5].ac = 3;
    facets[5].af = -1;

    facets[6].e1 = 7;
    facets[6].e2 = 8;
    facets[6].ac = 3;
    facets[6].af = 1;

    facets[7].e1 = 5;
    facets[7].e2 = 8;
    facets[7].ac = 4;
    facets[7].af = -1;

    //Facets initialised loop through
    double flowDir = -9999;
    double maxS = -999999;
    int maxFacet = -1;
    for (int i = 0; i < 8; ++i)
    {
        //Calculate slope for each facet
        facets[i].s[0] = (afWin[4] - afWin[facets[i].e1])/ewres;//SRM
        facets[i].s[1] = (afWin[facets[i].e1] - afWin[facets[i].e2])/nsres;//SRM

        
        facets[i].fdir = atan2(facets[i].s[1],facets[i].s[0]);
        if (facets[i].fdir <= 0)
        {
            facets[i].fdir = 0;
            facets[i].s_mag = facets[i].s[0];
        }
        else if (facets[i].fdir > atan(nsres/ewres))
        {
            facets[i].fdir = atan(nsres/ewres);
            facets[i].s_mag = (afWin[4] - afWin[facets[i].e2])/sqrt(abs(pow(facets[i].s[0],2) + pow(facets[i].s[1],2)));
        }
        else
        {
            facets[i].fdir = atan2(facets[i].s[1],facets[i].s[0]);
            facets[i].s_mag = sqrt(abs(pow(facets[i].s[0],2)+pow(facets[i].s[1],2)));//THis will break if I have a -slope but IDK.

        }
        

        if(facets[i].s_mag > maxS)
        {
            maxS = facets[i].s_mag;
            flowDir = facets[i].fdir;
            maxFacet = i;
        }
  
    }
    
    double ccwEastangle;

    if (maxS > 0)
    {
        ccwEastangle = (facets[maxFacet].af*flowDir) + (facets[maxFacet].ac*(M_PI/2));
    }
    else
    {
        ccwEastangle = -1;
    }

    return ccwEastangle;
}

float GDALFlowDirection8Alg(float* afWin, float dstNoDataValue, void* pData)
{
    double* dWDrop;

    const double radiansToDegrees = 180.0 / M_PI;

    GDALFlowDirAlgData* psData = (GDALFlowDirAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = -psData->nsres;

    dWDrop = new double [9];
    double dWDropmax = 0;
    for (int i = 0; i < 9; ++i)
    {
        if (i % 2 == 0 && i != 4)
        {
            dWDrop[i] = (afWin[4] - afWin[i])/ROOT_2;
            if (dWDrop[i] > dWDropmax)
            {
                dWDropmax = dWDrop[i];
            }
        }
        else
        {
            dWDrop[i] = (afWin[4] - afWin[i]);
            if (dWDrop[i] > dWDropmax)
            {
                dWDropmax = dWDrop[i];
            }
        }
    }

    int flowDir = 0;
    int flowDCodes[9] = {1 , 2, 4, 128, 0, 8, 64, 32, 16};

    int flowAngles[9] = {32, 64, 128, 
                        16, -1, 1,
                        8, 4, 2};

    /*float flowAngles[9] = {3*M_PI_4, M_PI_2, M_PI_4,
                         M_PI, -1, 0,
                         5*M_PI_4, 3*M_PI_2, 7*M_PI_4};*/
    int nNeighbours = 0;
    std::vector<int> maxCell;
    for (int i = 0; i < 9; ++i)
    {
        if (dWDrop[i] >= dWDropmax)
        {
            ++nNeighbours;
            flowDir = flowDir + flowDCodes[i];//should give a unique value.
            maxCell.push_back(i);
        }
    }

    float direction;
    if (nNeighbours == 0)
    {
        std::cout << QString("ERROR: Condition 1 cell, ensure depressions have been filled") + "\n";
        direction = -1;
    }
    else if (nNeighbours == 1)
    {
        //Condition 2 cells - only 1 neighbour.
        direction = flowAngles[maxCell[0]];
    }
    else if (nNeighbours > 1 && dWDropmax > 0.0)
    {
        //Condition 3 cells - 2 neighbours with equal drop - look up table
        if (maxCell.size() < 3)//2 neighbours - arbitrary
        {
            direction = flowAngles[maxCell[0]];
        }
        else
        {
            if (maxCell[0] == 0 && maxCell[1] == 1 && maxCell[2] == 2)
                direction = flowAngles[1];
            if (maxCell[0] == 1 && maxCell[1] == 2 && maxCell[2] == 5)
                direction = flowAngles[2];
            if (maxCell[0] == 2 && maxCell[1] == 5 && maxCell[2] == 8)
                direction = flowAngles[5];
            if (maxCell[0] == 5 && maxCell[1] == 7 && maxCell[2] == 8)
                direction = flowAngles[8];
            if (maxCell[0] == 6 && maxCell[1] == 7 && maxCell[2] == 8)
                direction = flowAngles[7];
            if (maxCell[0] == 3 && maxCell[1] == 6 && maxCell[2] == 7)
                direction = flowAngles[6];
            if (maxCell[0] == 0 && maxCell[1] == 3 && maxCell[2] == 6)
                direction = flowAngles[3];
            if (maxCell[0] == 0 && maxCell[1] == 1 && maxCell[2] == 3)
                direction = flowAngles[0];
        }
    }
    else if (nNeighbours > 1 && dWDropmax == 0.0)
    {
        //Condition 4 cells - 0 drop cells
        direction = 0;
        for (int dirIt = 0; dirIt < maxCell.size(); ++dirIt)
        {
            direction += maxCell[dirIt];
        }
    }

   return direction;
}

float GDALFlowDirection8IterativeAlg(float* afWin, float dstNoDataValue, void* pData)
{
    const double radiansToDegrees = 180.0 / M_PI;

    GDALFlowDirAlgData* psData = (GDALFlowDirAlgData*)pData;

    double ewres = psData->ewres;
    double nsres = -psData->nsres;
    int flowAngles[9] = {32, 64, 128, 
                        16, -1, 1,
                        8, 4, 2};
    int flowIntoAngles[9] = { 2, 4, 8,
                              1, 0, 16,
                              128, 64, 32
                            }; 

    /*float flowAngles[9] = {3*M_PI_4, M_PI_2, M_PI_4,
                         M_PI, -1, 0,
                         5*M_PI_4, 3*M_PI_2, 7*M_PI_4};*/
    /*float flowIntoAngles[9] = {7*M_PI_4, 3*M_PI_2, 5*M_PI_4,
                             0, -1, M_PI,
                             M_PI_4,M_PI_2, 3*M_PI_4};*/
    if (afWin[4] != 1 && afWin[4] != 2 && afWin[4] != 4 && afWin[4] != 8 && afWin[4] != 16 && afWin[4] != 32 && afWin[4] != 64 && afWin[4] != 128 && afWin[4] > -1)
    {
        float direction;
        for (int i = 0; i < 9; ++i)
        {
            if (i != 4)
            {
                if (afWin[i] == 1 || afWin[i] == 2 || afWin[i] == 4 || afWin[i] == 8 || afWin[i] == 16 || afWin[i] == 32 || afWin[i] == 64 || afWin[i] == 128)//FLOW IS RESOLVED
                {
                    if(afWin[i] != flowIntoAngles[i])//doesnt flow to our cell.
                    {
                        direction = flowAngles[i];
                    }
                }
            }
        }
        return direction;
    }
    else 
    {
        return afWin[4];
    }
}

float GDALFillDepressionsAlg(float* afWin, float dstNoDataValue, void* pData)
{
    float low = afWin[0];
    int minElement = 4;

    for (int i = 0; i < 9; ++i)
    {
        if (afWin[i] < afWin[4])
        {
            minElement = i;
        }
        if (afWin[i] < low && i != 4)
        {
            low = afWin[i];
        }
    }
    if (minElement == 4)
    {
        return low;
    }
    else
    {
        return afWin[4];
    }
}


DEFINE_WORKSPACE_DATA_FACTORY(RF::SlopeAlgType, RF::VolcanoPlugin::getInstance())
DEFINE_WORKSPACE_ENUMTOINTADAPTOR(RF::SlopeAlgType, RF::VolcanoPlugin::getInstance())

DEFINE_WORKSPACE_DATA_FACTORY(RF::CurvatureType, RF::VolcanoPlugin::getInstance())
DEFINE_WORKSPACE_ENUMTOINTADAPTOR(RF::CurvatureType, RF::VolcanoPlugin::getInstance())

DEFINE_WORKSPACE_DATA_FACTORY(RF::FlowDirType, RF::VolcanoPlugin::getInstance())
DEFINE_WORKSPACE_ENUMTOINTADAPTOR(RF::FlowDirType, RF::VolcanoPlugin::getInstance())

DEFINE_WORKSPACE_DATA_FACTORY(RF::FuzzyMembershipType, RF::VolcanoPlugin::getInstance())
DEFINE_WORKSPACE_ENUMTOINTADAPTOR(RF::FuzzyMembershipType, RF::VolcanoPlugin::getInstance())