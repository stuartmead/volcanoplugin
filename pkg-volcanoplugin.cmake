# =========================================================================
#
#   Revision:       $Revision: 18437 $
#   Last changed:   $Date: 2012-01-05 16:21:57 +1100 (Thu, 05 Jan 2012) $
#
#   Copyright 2003-2013 by:
#
#     Commonwealth Scientific and Industrial Research Organisation (CSIRO)
#     Division of Computational Informatics
#
#     For further information, contact:
#         CFD (CMIS)
#         Private Bag 33,
#         Clayton South MDC,
#         Clayton, Vic, 3169
#         AUSTRALIA
#
#   This copyright notice must be included with all copies of the source code.
#
# ===========================================================================

IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   MESSAGE(FATAL_ERROR "CMake >= 2.6.0 required")
ENDIF()
CMAKE_POLICY(PUSH)
CMAKE_POLICY(VERSION 2.6)

GET_TARGET_PROPERTY(volcanoplugin_location volcanoplugin LOCATION)
if (NOT VOLCANO_SOURCE_DIR AND NOT volcanoplugin_location AND NOT TARGET volcanoplugin)
    # Commands may need to know the format version.
    SET(CMAKE_IMPORT_FILE_VERSION 1)

    # Compute the installation prefix relative to this file.
    GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
    GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
    GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

    # Create imported target volcanoplugin
    ADD_LIBRARY(volcanoplugin SHARED IMPORTED)
    if (WIN32)
        find_file(IMPORTLIB volcanoplugin.lib PATHS "${_IMPORT_PREFIX}/lib/Plugins"
                  PATH_SUFFIXES  Release RelWithDebInfo MinSizeRel Debug NO_DEFAULT_PATHS)
        SET_TARGET_PROPERTIES(volcanoplugin PROPERTIES
                              IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/Plugins/volcanoplugin.dll"
                              IMPORTED_IMPLIB   "${IMPORTLIB}")
        unset(IMPORTLIB CACHE)
    elseif(APPLE)
        SET_TARGET_PROPERTIES(volcanoplugin PROPERTIES
                              IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/Plugins/libvolcanoplugin.@VOLCANO_PLUGIN_SOVERSION@.dylib"
                              IMPORTED_SONAME "libvolcanoplugin.@VOLCANO_PLUGIN_SOVERSION@.dylib")
    else()
        SET_TARGET_PROPERTIES(volcanoplugin PROPERTIES
                              IMPORTED_LOCATION "${_IMPORT_PREFIX}/lib/Plugins/libvolcanoplugin.so.@VOLCANO_PLUGIN_SOVERSION@"
                              IMPORTED_SONAME "libvolcanoplugin.so.@VOLCANO_PLUGIN_SOVERSION@")
    endif()
    SET_TARGET_PROPERTIES(volcanoplugin PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES "workspace")

    # Commands beyond this point should not need to know the version.
    SET(CMAKE_IMPORT_FILE_VERSION)
endif()
UNSET(volcanoplugin_location)

CMAKE_POLICY(POP)
