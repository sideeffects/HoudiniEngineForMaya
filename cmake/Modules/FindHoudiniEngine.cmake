# Output variables:
#   HoudiniEngine_FOUND
#   HoudiniEngine_INCLUDE_DIRS
#   HoudiniEngine_BINARY_DIR
#
# Hints for finding package:
#   HoudiniEngine_ROOT
#   HoudiniEngine_INCLUDEDIR
#   HoudiniEngine_BINARYDIR

# set( _houdiniengine_lib HAPIL )
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set( _houdiniengine_hars_executable HARS )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set( _houdiniengine_hars_executable HARS.exe )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    set( _houdiniengine_hars_executable HARS )
endif ()

########################################
# Determine search directories
########################################
# HoudiniEngine_INCLUDEDIR
if ( DEFINED HoudiniEngine_INCLUDEDIR )
    list( APPEND _houdiniengine_include_search_dirs "${HoudiniEngine_INCLUDEDIR}" )
endif ()

# HoudiniEngine_BINARYDIR
if ( DEFINED HoudiniEngine_BINARYDIR )
    list( APPEND _houdiniengine_binary_search_dirs "${HoudiniEngine_BINARYDIR}" )
endif ()

# HoudiniEngine_ROOT
if ( DEFINED HoudiniEngine_ROOT )
    set( _houdiniengine_root ${HoudiniEngine_ROOT} )
endif ()
if ( _houdiniengine_root )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_binary_search_dirs "${_houdiniengine_root}/bin" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_binary_search_dirs "${_houdiniengine_root}/bin" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        list( APPEND _houdiniengine_include_search_dirs "${_houdiniengine_root}/toolkit/include" )
        list( APPEND _houdiniengine_binary_search_dirs "${_houdiniengine_root}/bin" )
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

find_path(
    HoudiniEngine_BINARY_DIR
    ${_houdiniengine_hars_executable}
    HINTS ${_houdiniengine_binary_search_dirs}
    )
list( APPEND _houdiniengine_required_vars HoudiniEngine_BINARY_DIR )

########################################
# Handle standard arguments
########################################
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    HoudiniEngine
    DEFAULT_MSG
    ${_houdiniengine_required_vars}
    )

