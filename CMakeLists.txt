cmake_minimum_required( VERSION 2.8.12 )

project( Maya )

set( MAYA_VERSION 2014 CACHE STRING "" )

set(
    SRCS
    AssetNode.C
    AssetCommand.C
    SubCommand.C
    AssetSubCommandLoadAsset.C
    AssetSubCommandSync.C
    EngineCommand.C
    Asset.C
    Input.C
    InputAsset.C
    InputMesh.C
    InputCurve.C
    InputParticle.C
    OutputObject.C
    util.C
    OutputGeometryObject.C
    OutputInstancerObject.C
    OutputGeometry.C
    OutputGeometryPart.C
    SyncAttribute.C
    SyncOutputObject.C
    SyncOutputGeometryPart.C
    SyncOutputInstance.C
    FluidGridConvert.C
    plugin.C
    CurveMeshInputNode.C
    )

########################################
# Module structure
########################################
set( install_module_dir "maya${MAYA_VERSION}" )
set( install_plug_ins_dir "${install_module_dir}/plug-ins" )
set( install_module_absolute_dir "${CMAKE_INSTALL_PREFIX}/${install_module_dir}" )

########################################
# Find packages
########################################
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules" )

set(
    maya_libraries
    OpenMaya
    OpenMayaAnim
    OpenMayaFX
    )
find_package( Maya ${MAYA_VERSION} EXACT REQUIRED ${maya_libraries} )

find_package( HoudiniEngine REQUIRED )

########################################
# Relative paths
########################################
file(
    RELATIVE_PATH
    install_plug_ins_dir_to_houdiniengine_lib_dir
    "${CMAKE_INSTALL_PREFIX}/${install_plug_ins_dir}"
    "${HoudiniEngine_LIBRARY_DIR}"
    )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    file(
        RELATIVE_PATH
        install_module_dir_to_houdiniengine_bin_dir
        "${install_module_absolute_dir}"
        "${HoudiniEngine_BINARY_DIR}"
        )
endif ()

########################################
# plugin
########################################
add_library( plugin SHARED ${SRCS} )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set( plugin_suffix ".so" )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set( plugin_suffix ".mll" )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    set( plugin_suffix ".bundle" )
endif ()

set_target_properties(
    plugin
    PROPERTIES
    OUTPUT_NAME houdiniEngine
    PREFIX ""
    SUFFIX "${plugin_suffix}"
    )

# Compile definitions
set( compile_definitions )
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    list(
        APPEND compile_definitions
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
        )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
endif ()
target_compile_definitions( plugin PRIVATE ${compile_definitions} )

# Compile options
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set(
        compile_options
        -Wall
        )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set(
        compile_options
        -W4
        # Enabled warnings
        -w14101 # unreferenced local variable
        -w14996 # Deprecation usage
        # Warnings treated as errors
        -we4265 # 'class' has virtual functions but destructor is not virtual
        -we4700 # uninitialized local variable used
        -we4715 # not all control paths return a value
        -we4717 # recursive on all control paths, function will cause runtime stack overflow
        -we4263 # member func does not override any base class virt member func
        -we4266 # no override for virt member func from base 'type'; func is hidden
        -we4390 # empty controlled statement found; is this the intent?
        -we4407 # cast between different pointer to member representations, compiler may generate incorrect code
        # Disabled warnings
        -wd4100 # unreferenced formal parameter
        -wd4244 # conversion of floating point type possible loss of data
        -wd4267 # conversion of integer type possible loss of data
        -wd4389 # signed/unsigned mismatch
        )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    set(
        compile_options
        -Wall
        )
endif ()
target_compile_options( plugin PRIVATE ${compile_options} )

# Maya libraries
target_link_libraries( plugin ${maya_libraries} )

# Houdini Engine libraries
target_link_libraries( plugin HoudiniEngine )

# Setup rpath
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set_target_properties(
        plugin
        PROPERTIES
        INSTALL_RPATH "$ORIGIN/${install_plug_ins_dir_to_houdiniengine_lib_dir}"
        )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    set_target_properties(
        plugin
        PROPERTIES
        MACOSX_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        )
endif ()

########################################
# Module files
########################################
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    # Set the PATH variable for dynamic library.
    set(
        MODULE_DESCRIPTION_ENVIRONMENTS
        "${MODULE_DESCRIPTION_ENVIRONMENTS}PATH +:= ${install_module_dir_to_houdiniengine_bin_dir}\n"
        )
    set(
        MODULE_DESCRIPTION_ABSOLUTE_ENVIRONMENTS
        "${MODULE_DESCRIPTION_ABSOLUTE_ENVIRONMENTS}PATH += ${install_module_absolute_dir}/${install_module_dir_to_houdiniengine_bin_dir}\n"
        )
endif ()

# HOUDINI_DSO_EXCLUDE_PATTERN
# Block out Houdini plugins that would load Houdini's UI libraries.
set(
    houdini_dso_exclude_pattern
    "HOUDINI_DSO_EXCLUDE_PATTERN={ROP_OpenGL,COP2_GPULighting,COP2_GPUFog,COP2_GPUEnvironment,COP2_GPUZComposite,COP2_EnableGPU,SHOP_OGL,OBJ_ReLight,VEX_OpRender}*"
    )
set(
    MODULE_DESCRIPTION_ENVIRONMENTS
    "${MODULE_DESCRIPTION_ENVIRONMENTS}${houdini_dso_exclude_pattern}\n"
    )
set(
    MODULE_DESCRIPTION_ABSOLUTE_ENVIRONMENTS
    "${MODULE_DESCRIPTION_ABSOLUTE_ENVIRONMENTS}${houdini_dso_exclude_pattern}\n"
    )

# Generate module files
configure_file( module_description.in "houdiniEngine-maya${MAYA_VERSION}" )
configure_file( module_description_absolute.in "${install_module_dir}/houdiniEngine-maya${MAYA_VERSION}" )

########################################
# Install
########################################
install(
    TARGETS plugin
    DESTINATION "${install_plug_ins_dir}"
    )

install(
    DIRECTORY scripts
    DESTINATION "${install_module_dir}"
    )

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/houdiniEngine-maya${MAYA_VERSION}"
    DESTINATION "."
    )

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${install_module_dir}/houdiniEngine-maya${MAYA_VERSION}"
    DESTINATION "${install_module_dir}"
    )