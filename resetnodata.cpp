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
#include "resetnodata.h"


namespace RF
{
    /**
     * \internal
     */
    class ResetNoDataImpl
    {
        // Allow string translation to work properly
        Q_DECLARE_TR_FUNCTIONS(RF::ResetNoDataImpl)

    public:
        ResetNoData&  op_;

        // Data objects
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataGDALDataset_;
        CSIRO::DataExecution::TypedObject< int >           dataRasterBand_;
        CSIRO::DataExecution::TypedObject< QString >  dataDestinationFileName_;
        CSIRO::DataExecution::TypedObject< double >        dataMinimumCellValue_;
        CSIRO::DataExecution::TypedObject< GDALDatasetH >  dataOutputDataset_;


        // Inputs and outputs
        CSIRO::DataExecution::InputScalar inputGDALDataset_;
        CSIRO::DataExecution::InputScalar inputRasterBand_;
        CSIRO::DataExecution::InputScalar inputDestinationFileName_;
        CSIRO::DataExecution::InputScalar inputMinimumCellValue_;
        CSIRO::DataExecution::Output      outputOutputDataset_;


        ResetNoDataImpl(ResetNoData& op);

        bool  execute();
        void  logText(const QString& msg)   { op_.logText(msg); }
    };


    /**
     *
     */
    ResetNoDataImpl::ResetNoDataImpl(ResetNoData& op) :
        op_(op),
        dataGDALDataset_(),
        dataRasterBand_(1),
        dataDestinationFileName_(),
        dataMinimumCellValue_(0.0),
        dataOutputDataset_(),
        inputGDALDataset_("GDAL Dataset", dataGDALDataset_, op_),
        inputRasterBand_("Raster Band", dataRasterBand_, op_),
        inputDestinationFileName_("Destination file name", dataDestinationFileName_, op_),
        inputMinimumCellValue_("Minimum cell value", dataMinimumCellValue_, op_),
        outputOutputDataset_("Output Dataset", dataOutputDataset_, op_)
    {
    }


    /**
     *
     */
    bool ResetNoDataImpl::execute()
    {
        GDALDatasetH& gDALDataset         = *dataGDALDataset_;
        int&          rasterBand          = *dataRasterBand_;
        QString& destinationFileName = *dataDestinationFileName_;
        double&       minimumCellValue    = *dataMinimumCellValue_;
        GDALDatasetH& outputDataset       = *dataOutputDataset_;
        
        if(rasterBand > GDALGetRasterCount(gDALDataset))
        {
            std::cout << QString("ERROR: Not enough raster bands, number of bands is %1, band selected is %2").arg(GDALGetRasterCount(gDALDataset)).arg(rasterBand) + "\n";
            return false;
        }

        GDALRasterBandH hBand = GDALGetRasterBand(gDALDataset, rasterBand);

        float *data;
        data = new float [GDALGetRasterBandYSize(hBand)*GDALGetRasterBandXSize(hBand)];

        GDALRasterIO( hBand, GF_Read,
            0,0,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            data,
            GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
            GDT_Float32,
            0,0);

        int srcNoData;
        float srcNoDataValue;

        srcNoDataValue = (float) GDALGetRasterNoDataValue(hBand, &srcNoData);
        std::cout << QString("Band nodata value is %1").arg(srcNoDataValue) + "\n";
        
        for (int it = 0; it < GDALGetRasterBandYSize(hBand)*GDALGetRasterBandXSize(hBand); ++it)
        {
                if (data[it] <= minimumCellValue)
                {
                    data[it] = srcNoDataValue;
                }
        }

        outputDataset = GDALCreate( GDALGetDatasetDriver(gDALDataset),
                                destinationFileName.toLocal8Bit().constData(),
                                GDALGetRasterXSize(gDALDataset), GDALGetRasterYSize(gDALDataset),
                                1,
                                GDT_Float32, NULL);
        double transform[6];
        GDALGetGeoTransform(gDALDataset,transform);

        GDALRasterBandH destBand = GDALGetRasterBand(outputDataset, 1);
        GDALSetGeoTransform(outputDataset, transform);
        GDALSetProjection(outputDataset, GDALGetProjectionRef(gDALDataset));
        GDALSetRasterNoDataValue(destBand, srcNoDataValue);

        GDALRasterIO(destBand, GF_Write,
                        0,0,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        data,
                        GDALGetRasterBandXSize(hBand), GDALGetRasterBandYSize(hBand),
                        GDT_Float32,
                        0,0);

        return true;
    }


    /**
     *
     */
    ResetNoData::ResetNoData() :
        CSIRO::DataExecution::Operation(
            CSIRO::DataExecution::OperationFactoryTraits< ResetNoData >::getInstance(),
            tr("Set cells to noData"))
    {
        pImpl_ = new ResetNoDataImpl(*this);
    }


    /**
     *
     */
    ResetNoData::~ResetNoData()
    {
        delete pImpl_;
    }


    /**
     *
     */
    bool  ResetNoData::execute()
    {
        return pImpl_->execute();
    }
}


using namespace RF;
DEFINE_WORKSPACE_OPERATION_FACTORY(ResetNoData, 
                                   RF::VolcanoPlugin::getInstance(),
                                   CSIRO::DataExecution::Operation::tr("Geospatial"))

