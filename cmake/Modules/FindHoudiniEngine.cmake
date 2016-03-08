# Output variables:
#   HoudiniEngine_FOUND
#   HoudiniEngine_INCLUDE_DIRS
#   HoudiniEngine_LIBRARIES
#   HoudiniEngine_LIBRARY_DIR
#   HoudiniEngine_BINARY_DIR
#
# Hints for finding package:
#   HoudiniEngine_ROOT
#   HoudiniEngine_INCLUDEDIR
#   HoudiniEngine_LIBRARYDIR
#   HoudiniEngine_BINARYDIR

set( _houdiniengine_lib HAPIL )

#if ( NOT HoudiniEngine_FIND_VERSION )
#    message( FATAL_ERROR "Houdini Engine version is not specified." )
#endif ()

########################################
# Determine search directories
########################################
# HoudiniEngine_INCLUDEDIR
if ( DEFINED HoudiniEngine_INCLUDEDIR )
    list( APPEND _houdiniengine_include_search_dirs "${HoudiniEngine_INCLUDEDIR}" )
endif ()

# HoudiniEngine_LIBRARYDIR
if ( DEFINED HoudiniEngine_LIBRARYDIR )
    list( APPEND _houdiniengine_library_search_dirs "${HoudiniEngine_LIBRARYDIR}" )
endif ()

# HoudiniEngine_BINARYDIR
if ( DEFINED HoudiniEngine_BINARYDIR )
    list( APPEND _houdiniengine_binary_search_dirs "${HoudiniEngine_BINARYDIR}" )
endif ()

# HoudiniEngine_ROOT
if ( DEFINED HoudiniEngine_ROOT )
    set( _houdiniengine_root ${HoudiniEngine_ROOT} )
#else ()
#    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
#        set( _houdiniengine_root "/opt/hfs${HoudiniEngine_FIND_VERSION}" )
#    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
#        set( _houdiniengine_root "$ENV{ProgramFiles}/Side\ Effects\ Software/Houdini\ ${HoudiniEngine_FIND_VERSION}/bin" )
#    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
#        set( _houdiniengine_root "/Applications/Houdini ${HoudiniEngine_FIND_VERSION}/Houdini.framework/Versions/Current/Resources" )
#    endif ()
endif ()
if ( _houdiniengine_root )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_library_search_dirs "${_houdiniengine_root}/dsolib" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_library_search_dirs "${_houdiniengine_root}/custom/houdini/dsolib" )
        list( APPEND _houdiniengine_binary_search_dirs "${_houdiniengine_root}/bin" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_library_search_dirs "${_houdiniengine_root}/../Libraries" )
    endif ()
endif ()

########################################
# Find
########################################
find_path(
    HoudiniEngine_INCLUDE_DIRS
    HAPI/HAPI.h
    HINTS ${_houdiniengine_include_search_dirs}
    )
list( APPEND _houdiniengine_required_vars HoudiniEngine_INCLUDE_DIRS )

# Houdini libraries on Windows are lib<name>.a
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif ()
find_library(
    HoudiniEngine_${_houdiniengine_lib}
    NAMES ${_houdiniengine_lib}
    HINTS ${_houdiniengine_library_search_dirs}
    )
list( APPEND _houdiniengine_required_vars HoudiniEngine_${_houdiniengine_lib} )
list( APPEND HoudiniEngine_LIBRARIES ${HoudiniEngine_${_houdiniengine_lib}} )
get_filename_component(
    HoudiniEngine_LIBRARY_DIR
    ${HoudiniEngine_${_houdiniengine_lib}}
    DIRECTORY
    )
list( APPEND _houdiniengine_required_vars HoudiniEngine_LIBRARY_DIR )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    find_path(
        HoudiniEngine_BINARY_DIR
        libHAPI.dll
        HINTS ${_houdiniengine_binary_search_dirs}
        )
    list( APPEND _houdiniengine_required_vars HoudiniEngine_BINARY_DIR )
endif ()

########################################
# Create library target
########################################
add_library( HoudiniEngine SHARED IMPORTED )

if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set_target_properties(
        HoudiniEngine
        PROPERTIES
            IMPORTED_LOCATION ${HoudiniEngine_${_houdiniengine_lib}}
        )
else ()
    # IMPORTED_IMPLIB is used on Windows.
    set_target_properties(
        HoudiniEngine
        PROPERTIES
        IMPORTED_IMPLIB ${HoudiniEngine_${_houdiniengine_lib}}
        )
endif ()

set_target_properties(
    HoudiniEngine
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${HoudiniEngine_INCLUDE_DIRS}"
    )

########################################
# Handle standard arguments
########################################
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    HoudiniEngine
    DEFAULT_MSG
    ${_houdiniengine_required_vars}
    )
