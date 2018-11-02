#/*
#  Created by: Stuart Mead/Workspace
#  Creation date: 2014-02-03
#  
#  Released under BSD 3 clause.
#  Use it however you want, but I cannot guarantee it is right.
#  Also don't use my name, the name of collaborators and my/their affiliations
#  as endorsement.
#
#*/


project(VOLCANO)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    find_path(WORKSPACE_CMAKE_ROOT Workspace_CMake_Root.txt
              HINTS ${CMAKE_SOURCE_DIR} ENV WORKSPACE_CMAKE_ROOT
              DOC "Directory containing the Workspace_CMake_Root.txt file")
    if (NOT WORKSPACE_CMAKE_ROOT)
        message(FATAL_ERROR "Please set WORKSPACE_CMAKE_ROOT to the directory containing the file Workspace_CMake_Root.txt")
    endif()
    include( ${WORKSPACE_CMAKE_ROOT}/CMakeLists.txt NO_POLICY_SCOPE )
endif()

cmake_minimum_required(VERSION 3.0)


include_directories(${VOLCANO_SOURCE_DIR})
if (NOT ${VOLCANO_SOURCE_DIR} STREQUAL ${VOLCANO_BINARY_DIR})
    include_directories(${VOLCANO_BINARY_DIR})
endif()


set(VOLCANO_PLUGIN_VERSION 0.1.0)
string(REGEX MATCH "^[0-9]+" VOLCANO_PLUGIN_SOVERSION ${VOLCANO_PLUGIN_VERSION})

# Add other Qt modules as required by your plugin - eg. QtNetwork, QtOpenGL, QtSql
find_package(Qt5Gui)
find_package(Qt5Core)
find_package(Qt5Widgets)
set(QT_LIBRARIES Qt5::Core;Qt5::Widgets)


#set(GEOSPATIAL_PLUGIN_DIR "" CACHE PATH "Geospatial plugin directory")
find_package(GDAL)

#set(GDAL_DIR "" CACHE PATH "GDAL library directory")

#set(GDAL_INCLUDE_DIR "" CACHE PATH "GDAL include directory")
include_directories(${GDAL_INCLUDE_DIR})

set(OpenCV_DIR "" CACHE PATH "The location of the build directory in your OpenCV directory")
find_package(OpenCV REQUIRED)

find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

#set(HDF5_DIR "" CACHE PATH "The location of hdf5 cmake files")
#include_directories(${HDF5_DIR}/include)
##link_directories(${HDF5_DIR}/lib)
#link_directories(${HDF5_DIR}/bin)

#link_directories(${GEOSPATIAL_PLUGIN_DIR})
link_directories(${GDAL_DIR})

set(HEADERS
    ${VOLCANO_SOURCE_DIR}/ellipseproperties.h
    ${VOLCANO_SOURCE_DIR}/deformtosphere.h
    ${VOLCANO_SOURCE_DIR}/scoopreader.h
    ${VOLCANO_SOURCE_DIR}/fuzzylocation.h
    ${VOLCANO_SOURCE_DIR}/mergerasters.h
    #${VOLCANO_SOURCE_DIR}/titanh5reader.h
    ${VOLCANO_SOURCE_DIR}/energyconoid.h
    ${VOLCANO_SOURCE_DIR}/ellipticalpile.h
    ${VOLCANO_SOURCE_DIR}/totalupstreamproperty.h
    ${VOLCANO_SOURCE_DIR}/getpixelvalue.h
    ${VOLCANO_SOURCE_DIR}/uplsopefailurevolume.h
    ${VOLCANO_SOURCE_DIR}/multiplyrasters.h
	${VOLCANO_SOURCE_DIR}/deionise.h
    ${VOLCANO_SOURCE_DIR}/boxfilter.h
    ${VOLCANO_SOURCE_DIR}/smooth.h
    ${VOLCANO_SOURCE_DIR}/scalerastervalues.h
    ${VOLCANO_SOURCE_DIR}/projtooffset.h
    ${VOLCANO_SOURCE_DIR}/warp.h
    ${VOLCANO_SOURCE_DIR}/boundsofraster.h
    ${VOLCANO_SOURCE_DIR}/energycone.h
    ${VOLCANO_SOURCE_DIR}/iversonfailurevolume.h
    ${VOLCANO_SOURCE_DIR}/rasterdifference.h
    ${VOLCANO_SOURCE_DIR}/rastersum.h
    ${VOLCANO_SOURCE_DIR}/fd8totheta.h
    ${VOLCANO_SOURCE_DIR}/resetnodata.h
    ${VOLCANO_SOURCE_DIR}/subsetdata.h
    ${VOLCANO_SOURCE_DIR}/upslopearea.h
    ${VOLCANO_SOURCE_DIR}/angletovectorstate.h
    ${VOLCANO_SOURCE_DIR}/flowrouting.h
    ${VOLCANO_SOURCE_DIR}/filldepressions.h
    ${VOLCANO_SOURCE_DIR}/create3dmodel.h
    ${VOLCANO_SOURCE_DIR}/debrisemergence.h
    ${VOLCANO_SOURCE_DIR}/calccurvature.h
    ${VOLCANO_SOURCE_DIR}/aspectcalc.h
    ${VOLCANO_SOURCE_DIR}/slopecalc.h
    ${VOLCANO_SOURCE_DIR}/rastertoimage.h
    ${VOLCANO_SOURCE_DIR}/rasterbandsummary.h
    ${VOLCANO_SOURCE_DIR}/gdalinfo.h
    ${VOLCANO_SOURCE_DIR}/volcanoplugin_api.h
    ${VOLCANO_SOURCE_DIR}/volcanoplugin.h
    ${VOLCANO_SOURCE_DIR}/volcanoutils.h
	${VOLCANO_SOURCE_DIR}/h5utils.h
    ${VOLCANO_SOURCE_DIR}/erosionutils.h
    ${VOLCANO_SOURCE_DIR}/boundsofraster.h
)

set(INSTALL_HEADERS
    ${VOLCANO_SOURCE_DIR}/ellipseproperties.h
    ${VOLCANO_SOURCE_DIR}/deformtosphere.h
    ${VOLCANO_SOURCE_DIR}/scoopreader.h
    ${VOLCANO_SOURCE_DIR}/fuzzylocation.h
    ${VOLCANO_SOURCE_DIR}/mergerasters.h
    #${VOLCANO_SOURCE_DIR}/titanh5reader.h
    ${VOLCANO_SOURCE_DIR}/energyconoid.h
    ${VOLCANO_SOURCE_DIR}/ellipticalpile.h
    ${VOLCANO_SOURCE_DIR}/totalupstreamproperty.h
    ${VOLCANO_SOURCE_DIR}/getpixelvalue.h
    ${VOLCANO_SOURCE_DIR}/uplsopefailurevolume.h
    ${VOLCANO_SOURCE_DIR}/multiplyrasters.h
	${VOLCANO_SOURCE_DIR}/deionise.h
    ${VOLCANO_SOURCE_DIR}/boxfilter.h
    ${VOLCANO_SOURCE_DIR}/smooth.h
    ${VOLCANO_SOURCE_DIR}/scalerastervalues.h
    ${VOLCANO_SOURCE_DIR}/projtooffset.h
    ${VOLCANO_SOURCE_DIR}/warp.h
    ${VOLCANO_SOURCE_DIR}/boundsofraster.h
    ${VOLCANO_SOURCE_DIR}/energycone.h
    ${VOLCANO_SOURCE_DIR}/iversonfailurevolume.h
    ${VOLCANO_SOURCE_DIR}/rasterdifference.h
    ${VOLCANO_SOURCE_DIR}/rastersum.h
    ${VOLCANO_SOURCE_DIR}/fd8totheta.h
    ${VOLCANO_SOURCE_DIR}/resetnodata.h
    ${VOLCANO_SOURCE_DIR}/subsetdata.h
    ${VOLCANO_SOURCE_DIR}/upslopearea.h
    ${VOLCANO_SOURCE_DIR}/angletovectorstate.h
    ${VOLCANO_SOURCE_DIR}/flowrouting.h
    ${VOLCANO_SOURCE_DIR}/filldepressions.h
    ${VOLCANO_SOURCE_DIR}/create3dmodel.h
    ${VOLCANO_SOURCE_DIR}/debrisemergence.h
    ${VOLCANO_SOURCE_DIR}/calccurvature.h
    ${VOLCANO_SOURCE_DIR}/aspectcalc.h
    ${VOLCANO_SOURCE_DIR}/slopecalc.h
    ${VOLCANO_SOURCE_DIR}/rastertoimage.h
    ${VOLCANO_SOURCE_DIR}/rasterbandsummary.h
    ${VOLCANO_SOURCE_DIR}/gdalinfo.h
    ${VOLCANO_SOURCE_DIR}/volcanoplugin_api.h
    ${VOLCANO_SOURCE_DIR}/volcanoplugin.h
    ${VOLCANO_SOURCE_DIR}/volcanoutils.h
    ${VOLCANO_SOURCE_DIR}/erosionutils.h
    ${VOLCANO_SOURCE_DIR}/boundsofraster.h
)

set(MOC_HEADERS
)

set(SOURCES
    ${VOLCANO_SOURCE_DIR}/ellipseproperties.cpp
    ${VOLCANO_SOURCE_DIR}/deformtosphere.cpp
    ${VOLCANO_SOURCE_DIR}/scoopreader.cpp
    ${VOLCANO_SOURCE_DIR}/fuzzylocation.cpp
    ${VOLCANO_SOURCE_DIR}/mergerasters.cpp
    #${VOLCANO_SOURCE_DIR}/titanh5reader.cpp
    ${VOLCANO_SOURCE_DIR}/energyconoid.cpp
    ${VOLCANO_SOURCE_DIR}/ellipticalpile.cpp
    ${VOLCANO_SOURCE_DIR}/totalupstreamproperty.cpp
    ${VOLCANO_SOURCE_DIR}/getpixelvalue.cpp
    ${VOLCANO_SOURCE_DIR}/uplsopefailurevolume.cpp
    ${VOLCANO_SOURCE_DIR}/multiplyrasters.cpp
    ${VOLCANO_SOURCE_DIR}/deionise.cpp
	${VOLCANO_SOURCE_DIR}/boxfilter.cpp
    ${VOLCANO_SOURCE_DIR}/smooth.cpp
    ${VOLCANO_SOURCE_DIR}/scalerastervalues.cpp
    ${VOLCANO_SOURCE_DIR}/projtooffset.cpp
    ${VOLCANO_SOURCE_DIR}/warp.cpp
    ${VOLCANO_SOURCE_DIR}/boundsofraster.cpp
    ${VOLCANO_SOURCE_DIR}/energycone.cpp
    ${VOLCANO_SOURCE_DIR}/iversonfailurevolume.cpp
    ${VOLCANO_SOURCE_DIR}/rasterdifference.cpp
    ${VOLCANO_SOURCE_DIR}/rastersum.cpp
    ${VOLCANO_SOURCE_DIR}/fd8totheta.cpp
    ${VOLCANO_SOURCE_DIR}/resetnodata.cpp
    ${VOLCANO_SOURCE_DIR}/subsetdata.cpp
    ${VOLCANO_SOURCE_DIR}/upslopearea.cpp
    ${VOLCANO_SOURCE_DIR}/angletovectorstate.cpp
    ${VOLCANO_SOURCE_DIR}/flowrouting.cpp
    ${VOLCANO_SOURCE_DIR}/filldepressions.cpp
    ${VOLCANO_SOURCE_DIR}/create3dmodel.cpp
    ${VOLCANO_SOURCE_DIR}/debrisemergence.cpp
    ${VOLCANO_SOURCE_DIR}/calccurvature.cpp
    ${VOLCANO_SOURCE_DIR}/aspectcalc.cpp
    ${VOLCANO_SOURCE_DIR}/slopecalc.cpp
    ${VOLCANO_SOURCE_DIR}/rastertoimage.cpp
    ${VOLCANO_SOURCE_DIR}/rasterbandsummary.cpp
    ${VOLCANO_SOURCE_DIR}/gdalinfo.cpp
    ${VOLCANO_SOURCE_DIR}/volcanoplugin.cpp
    ${VOLCANO_SOURCE_DIR}/volcanoutils.cpp
    ${VOLCANO_SOURCE_DIR}/erosionutils.cpp
)

set(UI_SOURCES
)

# The next line is used by the simple application generator wizard
# add_subdirectory(${VOLCANO_SOURCE_DIR}/Application)

# The below line can be used to import sub-directories
# include( ${VOLCANO_SOURCE_DIR}/SubDir/CMakeLists.txt )

qtx_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})
qtx_wrap_ui(UIC_SOURCES ${UI_SOURCES})

add_definitions(-DVOLCANO_PLUGIN_VERSION=${VOLCANO_PLUGIN_VERSION})

include_directories(${OpenCV_INCLUDE_DIRS})

add_library(volcanoplugin ${SOURCES} ${HEADERS} ${MOC_SOURCES} ${UIC_SOURCES})
target_link_libraries(volcanoplugin dataanalysisplugin meshplugin workspace ${HDF5_LIBRARIES} ${GDAL_LIBRARIES} ${OpenCV_LIBS} ${QT_LIBRARIES})

set_target_properties(volcanoplugin PROPERTIES
    DEFINE_SYMBOL RF_EXPORT
    VERSION ${VOLCANO_PLUGIN_VERSION}
    SOVERSION ${VOLCANO_PLUGIN_SOVERSION}
)

setTargetOutputDirectory(volcanoplugin ${CSIRO_INSTALL_AREA}/lib/Plugins)
configure_file(pkg-volcanoplugin.cmake ${CSIRO_INSTALL_AREA}/cmake/Exports/pkg-volcanoplugin.cmake @ONLY)

# Copy our install headers into the install directory so that others can build against our plugin.
foreach(inFile ${INSTALL_HEADERS})
    string(REGEX REPLACE "(${VOLCANO_SOURCE_DIR}/)(.*)" "${CSIRO_INSTALL_AREA}/include/Volcano/\\2" outFile "${inFile}")
    configure_file(${inFile} ${outFile} COPYONLY)
endforeach(inFile)

