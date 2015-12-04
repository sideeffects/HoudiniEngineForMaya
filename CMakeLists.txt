cmake_minimum_required( VERSION 2.8 )

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
# plugin
########################################
add_library( plugin SHARED ${SRCS} )

set_target_properties(
    plugin
    PROPERTIES
    OUTPUT_NAME houdiniEngine
    )

# Maya libraries
target_link_libraries( plugin ${maya_libraries} )

# Houdini Engine libraries
target_link_libraries( plugin HoudiniEngine )
