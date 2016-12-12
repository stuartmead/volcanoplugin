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

#include < Qimage >

#include "cpl_conv.h"
#include "cpl_multiproc.h"

#include "Workspace/Application/LanguageUtils/streamqstring.h"
#include "Workspace/DataExecution/DataObjects/typedobject.h"
#include "Workspace/DataExecution/InputOutput/inputscalar.h"
#include "Workspace/DataExecution/InputOutput/inputarray.h"
#include "Workspace/DataExecution/InputOutput/output.h"
#include "Workspace/DataExecution/Operations/typedoperationfactory.h"

#include "DataAnalysis/Color/colorscale.h"


#include "volcanoplugin.h"
#include "rastertoimage.h"


namespace RF
{
    /**
     * \internal
     */
    class RastertoImageImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::RastertoImageImpl)

    public:
        RastertoImage&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >                     dataRasterDataset_;
        CSIRO::DataExecution::TypedObject< int >                              dataBandNumber_;
        CSIRO::DataExecution::TypedObject< CSIRO::DataAnalysis::ColorScale >  dataColorMapper_;
        CSIRO::DataExecution::TypedObject< int >                              dataNXOff_;
        CSIRO::DataExecution::TypedObject< int >                              dataNYOff_;
        CSIRO::DataExecution::TypedObject< int >                              dataXSize_;
        CSIRO::DataExecution::TypedObject< int >                              dataYSize_;
        CSIRO::DataExecution::TypedObject< double >                           dataScaleFactor_;
        CSIRO::DataExecution::TypedObject< QImage >                           dataRasterImage_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputRasterDataset_;
        CSIRO::DataExecution::InputScalar inputBandNumber_;
        CSIRO::DataExecution::InputScalar inputColorMapper_;
        CSIRO::DataExecution::InputScalar inputNXOff_;
        CSIRO::DataExecution::InputScalar inputNYOff_;
        CSIRO::DataExecution::InputScalar inputXSize_;
        CSIRO::DataExecution::InputScalar inputYSize_;
        CSIRO::DataExecution::InputScalar inputScaleFactor_;
        CSIRO::DataExecution::Output      outputRasterImage_;


        RastertoImageImpl(RastertoImage& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    RastertoImageImpl::RastertoImageImpl(RastertoImage& op) :
        op_(op),
        dataRasterDataset_(),
        dataBandNumber_(1),
        dataColorMapper_(),
        dataNXOff_(0),
        dataNYOff_(0),
        dataXSize_(-1),
        dataYSize_(-1),
        dataScaleFactor_(1),
        dataRasterImage_(),
        inputRasterDataset_("Raster Dataset", dataRasterDataset_, op_),
        inputBandNumber_("Band number", dataBandNumber_, op_),
        inputColorMapper_("Color Mapper", dataColorMapper_, op_),
        inputNXOff_("X offset", dataNXOff_, op_),
        inputNYOff_("Y offset", dataNYOff_, op_),
        inputXSize_("X size", dataXSize_, op_),
        inputYSize_("Y size", dataYSize_, op_),
        inputScaleFactor_("Scale factor", dataScaleFactor_, op_),
        outputRasterImage_("Raster Image", dataRasterImage_, op_)
    {
    }


    /**
     *
     */
    bool RastertoImageImpl::execute()
    {
        GDALDatasetH&                    rasterDataset = *dataRasterDataset_;
        int&                             bandNumber   = *dataBandNumber_;
        CSIRO::DataAnalysis::ColorScale& colorMapper   = *dataColorMapper_;
        int&                             nXOff         = *dataNXOff_;
        int&                             nYOff         = *dataNYOff_;
        int&                             xSize         = *dataXSize_;
        int&                             ySize         = *dataYSize_;
        double&                          scaleFactor   = *dataScaleFactor_;
        QImage&                          rasterImage   = *dataRasterImage_;
        
        if(bandNumber > GDALGetRasterCount(rasterDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(rasterDataset)).arg(bandNumber) + "\n";
            return false;
        }

        GDALRasterBandH hBand = GDALGetRasterBand(rasterDataset, bandNumber);
        
        double sizes[2];
        if (xSize <= 0)
        {
            sizes[0] = GDALGetRasterBandXSize(hBand) - nXOff;
        }
        else
        {
            sizes[0] = xSize;
        }

        if (ySize <= 0)
        {
            sizes[1] = GDALGetRasterBandYSize(hBand) - nYOff;
        }
        else
        {
            sizes[1] = ySize;
        }

        int scaleXsize, scaleYsize;
        scaleXsize = floor(sizes[0]/scaleFactor);//Needs to include nxoff
        scaleYsize = floor(sizes[1]/scaleFactor);//Needs to include nyoff

        double transform[6];
        GDALGetGeoTransform(rasterDataset,transform);

        std::cout << QString("X cellsize is %1, Y cellsize is %2").arg((sizes[0]/scaleXsize)*transform[1]).arg((sizes[1]/scaleYsize)*-transform[5]) + "\n";

        float *data;
        data = new float [scaleYsize*scaleXsize];

        std::cout << QString("Raster type is %1").arg(GDALGetDataTypeName(GDALGetRasterDataType(hBand))) + "\n";




        GDALRasterIO(hBand, GF_Read,
            nXOff, nYOff, //X,Y offset in cells
            sizes[0], sizes[1], //X,Y length in cells
            data, //data
            scaleXsize, scaleYsize, //Number of cells in new dataset
            GDT_Float32, //Type
            0, 0);//Scanline stuff (for interleaving)


        //Now convert float array to qimage
        rasterImage = QImage(QSize(scaleXsize,scaleYsize), QImage::Format_ARGB32);
        int it = 0;//Iterator for float data
        for (int i = 0; i < scaleYsize; ++i)
        {
            for (int j = 0; j < scaleXsize; ++j)
            {
                QColor c = colorMapper.mapToColor(data[it]);
                rasterImage.setPixel(j,i,qRgba(c.red(),c.green(),c.blue(),c.alpha()));
                ++it;
            }
        }

        delete[] data;



        return true;
    }


    /**
     *
     */
    RastertoImage::RastertoImage() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< RastertoImage >::getInstance(),
            tr("Plot raster"))
    {
        pImpl_ = new RastertoImageImpl(*this);
    }


    /**
     *
     */
    RastertoImage::~RastertoImage()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  RastertoImage::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(RastertoImage, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

