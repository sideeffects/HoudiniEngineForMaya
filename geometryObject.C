#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "asset.h"
#include "geometryObject.h"
#include "geometryPart.h"
#include "util.h"
#include "common.h"

GeometryObject::GeometryObject() {}


GeometryObject::GeometryObject(int assetId, int objectId)
    :Object(assetId, objectId)
{
}


void
GeometryObject::init()
{
    // Do a full update
    Object::init();

    int partCount = myGeoInfo.partCount;
    myParts = new GeometryPart[partCount];

    for (int i=0; i<partCount; i++)
    {
        MString partName = getName() + "_partShape" + (i+1);
        cerr << "&&&&&&&&&&&& partName: " << partName << endl;
        myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, partName, myGeoInfo, myObjectControl );
    }
}


GeometryObject::~GeometryObject()
{
    delete[] myParts;
}


Object::ObjectType
GeometryObject::type()
{
    return Object::OBJECT_TYPE_GEOMETRY;
}


//void
//GeometryObject::updateFaceCounts()
//{
    //int numFaceCount = geoInfo.faceCount;
    //if (numFaceCount > 0)
    //{
        //int myFaceCounts[numFaceCount];
        //HAPI_GetFaceCounts(assetId, objectInfo.id, 0, myFaceCounts, 0, numFaceCount);
        //MIntArray result(myFaceCounts, numFaceCount);

        //faceCounts = result;
    //}
    ////cerr << "facecounts: " << faceCounts << endl;
//}


//void
//GeometryObject::updateVertexList()
//{
    //int numVertexCount = geoInfo.vertexCount;
    //if (numVertexCount > 0)
    //{
        //int myVertexList[numVertexCount];
        //HAPI_GetVertexList(assetId, objectInfo.id, 0, myVertexList, 0, numVertexCount);
        //MIntArray result(myVertexList, numVertexCount);
        ////cerr << "vertextList: " << endl;
        ////cerr << result << endl;
        //Util::reverseWindingOrderInt(result, faceCounts);

        //vertexList = result;
    //}
//}


//void
//GeometryObject::updatePoints()
//{
    //MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_POINT, "P");
    //// make a maya point array, assume 3 tuple
    //MFloatPointArray result;
    //int i = 0;
    //int len = data.length();
    //while (i < len)
    //{
        //MFloatPoint v(data[i], data[i+1], data[i+2]);
        //result.append(v);
        //i = i+3;
    //}

    //points = result;
//}


//void
//GeometryObject::updateNormals()
//{
    //MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_POINT, "N");
    //// make a maya vector array, assume 3 tuple
    //MVectorArray result;

    //if (data.length() > 0)
    //{
        //int i = 0;
        //int len = data.length();
        //while (i < len)
        //{
            //MFloatVector v(data[i], data[i+1], data[i+2]);
            //result.append(v);
            //i = i+3;
        //}
    //}

    //normals = result;
//}


//void
//GeometryObject::updateUVs()
//{
    //MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_VERTEX, "uv");
    //// split UVs into two arrays, assume 3 tuple
    //MFloatArray Us;
    //MFloatArray Vs;

    //if (data.length() > 0)
    //{
        //int i = 0;
        //int len = data.length();
        //while (i < len)
        //{
            //Us.append(data[i]);
            //Vs.append(data[i+1]);
            //i = i+3;
        //}
        //Util::reverseWindingOrderFloat(Us, faceCounts);
        //Util::reverseWindingOrderFloat(Vs, faceCounts);
    //}

    //us = Us;
    //vs = Vs;
//}


void
GeometryObject::update()
{
    int oldPartCount = myGeoInfo.partCount;
    Object::update();
    int partCount = myGeoInfo.partCount;

    // if partCount changed, we clear out the array and make a new one.
    if (oldPartCount != partCount)
    {
        delete[] myParts;
        myParts = new GeometryPart[partCount];
        for (int i=0; i<partCount; i++)
        {
            MString partName = getName() + "_partShape" + (i+1);
            cerr << "&&&&&&&&&&&& partName: " << partName << endl;
            myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, partName, myGeoInfo, myObjectControl);
        }
    }
    else
    {
        for (int i=0; i<partCount; i++)
        {
            myParts[i].setGeoInfo( myGeoInfo );
        }
    }

    //updateFaceCounts();
    //updateVertexList();
    //updatePoints();
    //updateNormals();
    //updateUVs();

}


MStatus
GeometryObject::compute(MDataHandle& handle)
{
    //update();

    //// Don't output mesh for invisible geos
    //if (!objectInfo.isVisible && !isInstanced)
        //return MS::kFailure;

    //// Don't output mesh for degenerate geos
    //if (geoInfo.pointCount == 0 || geoInfo.faceCount == 0|| geoInfo.vertexCount == 0)
        //return MS::kFailure;

    //cerr << "compute object: " << getName() << endl;
    //// Get plugs
    //MDataHandle objectNameHandle = handle.child(AssetNodeAttributes::objectName);
    //MDataHandle meshHandle = handle.child(AssetNodeAttributes::mesh);
    //MDataHandle transformHandle = handle.child(AssetNodeAttributes::transform);
    //MDataHandle materialHandle = handle.child(AssetNodeAttributes::material);

    //// Object name
    //objectNameHandle.set(Util::getString(objectInfo.nameSH));

    //// Mesh
    //MObject newMeshData = createMesh();
    //meshHandle.set(newMeshData);

    //// Transform and materials
    //updateTransform(transformHandle);
    //updateMaterial(materialHandle);

    //objectNameHandle.setClean();
    //meshHandle.setClean();
    //handle.setClean();

    return MS::kSuccess;
}


MStatus
GeometryObject::computeParts(MArrayDataBuilder* builder, int* index)
{
    update();

    // TODO: this may be temporary until HAPI supports actual groups
    if ( myNeverBuilt || myObjectInfo.haveGeosChanged)
    {
        MStatus stat;

        // Don't output mesh for invisible geos
        if (! myObjectInfo.isVisible && ! myIsInstanced)
            return MS::kFailure;

        // TODO: right now assume one geo
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = builder->addElement(*index);
            stat = myParts[i].compute(h);

            if (MS::kSuccess == stat)
            {
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = h.child(AssetNodeAttributes::transform);
                    updateTransform(t);
                }
                cerr << *index << " index++: " + myParts[i].myPartName << endl;
                (*index)++;
            }
        }

        myNeverBuilt = false;
    }
    else
    {
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            if ( myParts[i].hasMesh() )
            {
                MDataHandle h = builder->addElement(*index);
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = h.child(AssetNodeAttributes::transform);
                    updateTransform(t);
                }
                (*index)++;
            }
        }
    }

    return MS::kSuccess;
}


MStatus
GeometryObject::setClean(MPlug& plug, MDataBlock& data)
{
    data.setClean(plug);
    data.setClean(plug.child(AssetNodeAttributes::mesh));
    data.setClean(plug.child(AssetNodeAttributes::objectName));

    MPlug transformPlug = plug.child(AssetNodeAttributes::transform);
    data.setClean(transformPlug.child(AssetNodeAttributes::translateAttr));
    data.setClean(transformPlug.child(AssetNodeAttributes::rotateAttr));
    data.setClean(transformPlug.child(AssetNodeAttributes::scaleAttr));

    MPlug materialPlug = plug.child(AssetNodeAttributes::material);
    data.setClean(materialPlug.child(AssetNodeAttributes::materialExists));
    data.setClean(materialPlug.child(AssetNodeAttributes::ambientAttr));
    data.setClean(materialPlug.child(AssetNodeAttributes::diffuseAttr));
    data.setClean(materialPlug.child(AssetNodeAttributes::specularAttr));
    data.setClean(materialPlug.child(AssetNodeAttributes::texturePath));

    return MS::kSuccess;
}


//MObject
//GeometryObject::createMesh()
//{

    ////cerr << "Creating mesh... " << Util::getString(objectInfo.nameSH) << endl;
    //// Mesh data
    //MFnMeshData dataCreator;
    //MObject outData = dataCreator.create();

    //// create mesh
    //MFnMesh meshFS;
    //MObject newMesh = meshFS.create(points.length(), faceCounts.length(),
            //points, faceCounts, vertexList, outData);

    //// set normals
    //if (normals.length() > 0)
    //{
        //MIntArray vlist;
        //for (int j=0; j<points.length(); j++)
            //vlist.append(j);
        //meshFS.setVertexNormals(normals, vlist);
    //}

    //// set UVs
    //if (us.length() > 0)
    //{
        //meshFS.setUVs(us, vs);
        //MIntArray uvIds;
        //for (int j=0; j<vertexList.length(); j++)
            //uvIds.append(j);
        //meshFS.assignUVs(faceCounts, uvIds);
    //}

    //return outData;

//}


void GeometryObject::updateTransform(MDataHandle& handle)
{
    cerr << "Update transform: " << getName() << endl;
    MDataHandle translateHandle = handle.child(AssetNodeAttributes::translateAttr);
    MDataHandle rotateHandle = handle.child(AssetNodeAttributes::rotateAttr);
    MDataHandle scaleHandle = handle.child(AssetNodeAttributes::scaleAttr);

    HAPI_GetObjectTransforms( myAssetId, 5, &myTransformInfo, myObjectId, 1 );
    //transformInfo = objectControl->getTransformInfo(objectId);

    // convert to euler angle
    MEulerRotation r = MQuaternion(myTransformInfo.rotationQuaternion[0],
            myTransformInfo.rotationQuaternion[1], myTransformInfo.rotationQuaternion[2],
            myTransformInfo.rotationQuaternion[3]).asEulerRotation();

    translateHandle.set3Double((double)( myTransformInfo.position[0]), (double) myTransformInfo.position[1], (double) myTransformInfo.position[2]);
    rotateHandle.set3Double((double)r[0], (double)r[1], (double)r[2]);
    scaleHandle.set3Double((double) myTransformInfo.scale[0], (double) myTransformInfo.scale[1], (double) myTransformInfo.scale[2]);

    translateHandle.setClean();
    rotateHandle.setClean();
    scaleHandle.setClean();
    handle.setClean();

}


//void
//GeometryObject::updateMaterial(MDataHandle& handle)
//{
    //MDataHandle matExistsHandle = handle.child(AssetNodeAttributes::materialExists);
    //MDataHandle ambientHandle = handle.child(AssetNodeAttributes::ambientAttr);
    //MDataHandle diffuseHandle = handle.child(AssetNodeAttributes::diffuseAttr);
    //MDataHandle specularHandle = handle.child(AssetNodeAttributes::specularAttr);
    //MDataHandle texturePathHandle = handle.child(AssetNodeAttributes::texturePath);

    ////cerr << "Updating materials .................." << endl;

    //if (geoInfo.materialId < 0)
    //{
        //matExistsHandle.set(false);
    //} else
    //{
        //// get material info
        //int matId = geoInfo.materialId;
        //materialInfo = objectControl->getMaterialInfo(matId);

        //matExistsHandle.set(true);

        //ambientHandle.set3Float(materialInfo.ambient[0], materialInfo.ambient[1], materialInfo.ambient[2]);
        //diffuseHandle.set3Float(materialInfo.diffuse[0], materialInfo.diffuse[1], materialInfo.diffuse[2]);
        //specularHandle.set3Float(materialInfo.specular[0], materialInfo.specular[1], materialInfo.specular[2]);
        //MString texturePath = Util::getString(materialInfo.textureFilePathSH);
        //texturePathHandle.set(texturePath);
    //}

    
    //handle.setClean();
    //matExistsHandle.setClean();
    //ambientHandle.setClean();
    //diffuseHandle.setClean();
    //specularHandle.setClean();
    //texturePathHandle.setClean();
//}
