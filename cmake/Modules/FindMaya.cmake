# Output variables:
#   Maya_FOUND
#   Maya_INCLUDE_DIRS
#   Maya_LIBRARIES
#
# Hints for finding package:
#   Maya_ROOT
#   Maya_INCLUDEDIR
#   Maya_LIBRARYDIR

# All the Maya libraries link with Foundation.
set(
    _maya_libraries
    Foundation
    ${Maya_FIND_COMPONENTS}
    )

if ( NOT Maya_FIND_VERSION )
    message( FATAL_ERROR "Maya version is not specified." )
endif ()

list( APPEND _maya_versions 2016.5 )
list( APPEND _maya_versions 2016 )
list( APPEND _maya_versions 2015 )
list( APPEND _maya_versions 2014 )
list( FIND _maya_versions ${Maya_FIND_VERSION} _maya_version_index )
if ( ${_maya_version_index} EQUAL -1 )
    message( FATAL_ERROR "Unknown Maya version. ${Maya_FIND_VERSION}" )
endif ()

########################################
# Determine search directories
########################################
# Maya_INCLUDEDIR
if ( DEFINED Maya_INCLUDEDIR )
    list( APPEND _maya_include_search_dirs "${Maya_INCLUDEDIR}" )
endif ()

# Maya_LIBRARYDIR
if ( DEFINED Maya_LIBRARYDIR )
    list( APPEND _maya_library_search_dirs "${Maya_LIBRARYDIR}" )
endif ()

# Maya_ROOT
if ( DEFINED Maya_ROOT )
    set( _maya_root ${Maya_ROOT} )
else ()
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        if ( ${Maya_FIND_VERSION} LESS 2016 )
            set( _maya_root "/usr/autodesk/maya${Maya_FIND_VERSION}-x64" )
        else ()
            set( _maya_root "/usr/autodesk/maya${Maya_FIND_VERSION}" )
        endif ()
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        set( _maya_root "C:/Program\ Files/Autodesk/Maya${Maya_FIND_VERSION}" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        set( _maya_root "/Applications/Autodesk/maya${Maya_FIND_VERSION}" )
    endif ()
endif ()
if ( _maya_root )
    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        list( APPEND _maya_include_search_dirs "${_maya_root}/include" )
        list( APPEND _maya_library_search_dirs "${_maya_root}/lib" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        list( APPEND _maya_include_search_dirs "${_maya_root}/include" )
        list( APPEND _maya_library_search_dirs "${_maya_root}/lib" )
    elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
        if ( ${Maya_FIND_VERSION} EQUAL 2016.5 )
            list( APPEND _maya_include_search_dirs "${_maya_root}/include" )
        else ()
            list( APPEND _maya_include_search_dirs "${_maya_root}/devkit/include" )
        endif ()
        list( APPEND _maya_library_search_dirs "${_maya_root}/Maya.app/Contents/MacOS" )
    endif ()
endif ()

########################################
# Find
########################################
find_path(
    Maya_INCLUDE_DIRS
    maya/MFn.h
    HINTS ${_maya_include_search_dirs}
    )
list( APPEND _maya_required_vars Maya_INCLUDE_DIRS )

foreach ( lib ${_maya_libraries} )
    find_library(
        Maya_${lib}
        NAMES ${lib}
        HINTS ${_maya_library_search_dirs}
        NO_DEFAULT_PATH
        )
    list( APPEND _maya_required_vars Maya_${lib} )
    list( APPEND Maya_LIBRARIES ${Maya_${lib}} )
endforeach ()

########################################
# Create library targets
########################################
set(
    _maya_compile_definitions
    _BOOL
    REQUIRE_IOSTREAM
    )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    list(
        APPEND _maya_compile_definitions
        OSMac_
        OSMac_MachO_
        )
endif ()

foreach ( lib ${_maya_libraries} )
    # SHARED so that IMPORTED_NO_SONAME works
    add_library( ${lib} SHARED IMPORTED )

    if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        set_target_properties(
            ${lib}
            PROPERTIES
                IMPORTED_LOCATION "${Maya_${lib}}"
                # Maya 2015 and older libraries do not have SONAME on Linux. This
                # would cause cmake to pass in absolute paths to the linker, which
                # we don't want.
                IMPORTED_NO_SONAME TRUE
            )
    else ()
        # IMPORTED_IMPLIB is used on Windows.
        set_target_properties(
            ${lib}
            PROPERTIES
                IMPORTED_IMPLIB "${Maya_${lib}}"
            )
    endif ()

    set_target_properties(
        ${lib}
        PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "${_maya_compile_definitions}"
            INTERFACE_INCLUDE_DIRECTORIES "${Maya_INCLUDE_DIRS}"
        )

    # All the Maya libraries link with Foundation.
    if ( NOT ${lib} EQUAL "Foundation" )
        set_target_properties(
            ${lib}
            PROPERTIES
                INTERFACE_LINK_LIBRARIES "${Maya_Foundation}"
            )
    endif ()
endforeach ()

########################################
# Handle standard arguments
########################################
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    Maya
    DEFAULT_MSG
    ${_maya_required_vars}
    )
