#include "InputMesh.h"

#include <maya/MDagPath.h>
#include <maya/MDataBlock.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

#include "hapiutil.h"
#include "types.h"
#include "util.h"

InputMesh::InputMesh() : Input()
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    HAPI_NodeId nodeId;
    CHECK_HAPI(HAPI_CreateInputNode(Util::theHAPISession.get(), &nodeId, NULL));
    if (!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating input asset."));
    }

    HAPI_NodeInfo nodeInfo;
    HAPI_GetNodeInfo(Util::theHAPISession.get(), nodeId, &nodeInfo);

    setTransformNodeId(nodeInfo.parentId);
    setGeometryNodeId(nodeId);
}

InputMesh::~InputMesh()
{
    if (!Util::theHAPISession.get())
        return;
    CHECK_HAPI(HAPI_DeleteNode(Util::theHAPISession.get(), geometryNodeId()));
}

Input::AssetInputType
InputMesh::assetInputType() const
{
    return Input::AssetInputType_Mesh;
}

void
InputMesh::setInputComponents(MDataBlock &dataBlock,
                              const MPlug &geoPlug,
                              const MPlug &compPlug,
                              const MPlug &primGroupPlug,
                              const MPlug &pointGroupPlug)
{
    // extract component list data from Maya
    MDataHandle compHandle = dataBlock.inputValue(compPlug);
    MObject compList       = compHandle.data();
    MFnComponentListData compListFn(compList);

    // extract mesh data from Maya
    MDataHandle meshHandle = dataBlock.inputValue(geoPlug);
    MObject meshObj        = meshHandle.asMesh();
    MFnMesh meshFn(meshObj);

    unsigned i;
    int j;
    MIntArray faceIds;
    MIntArray vertIds;
    MIntArray edgeIds;
    for (i = 0; i < compListFn.length(); i++)
    {
        MObject comp = compListFn[i];
        if (comp.apiType() == MFn::kMeshPolygonComponent)
        {
            MFnSingleIndexedComponent siComp(comp);
            for (j = 0; j < siComp.elementCount(); j++)
                faceIds.append(siComp.element(j));
        }
        if (comp.apiType() == MFn::kMeshVertComponent)
        {
            MFnSingleIndexedComponent siComp(comp);
            for (j = 0; j < siComp.elementCount(); j++)
                vertIds.append(siComp.element(j));
        }

        if (comp.apiType() == MFn::kMeshEdgeComponent)
        {
            // should convert the edge component to a point group in some
            // meaningful way
            MFnSingleIndexedComponent siComp(comp);
            for (j = 0; j < siComp.elementCount(); j++)
                edgeIds.append(siComp.element(j));
        }
        if (comp.apiType() == MFn::kMeshVtxFaceComponent)
        {
            // unfortunately, setAttr doesn't support vertexFace components
            // for a componentList attr: may need a command to set these
            // MFnDoubleIndexedComponent doComp( comp );
        }
    }
    std::vector<int> groupMembership;

    if (faceIds.length() > 0)
    {
        int numFaces             = meshFn.numPolygons();
        HAPI_GroupType groupType = HAPI_GROUPTYPE_PRIM;

        groupMembership.resize(numFaces);
        std::fill(groupMembership.begin(), groupMembership.end(), 0);

        for (i = 0; i < faceIds.length(); i++)
        {
            groupMembership[faceIds[i]] = 1;
        }
        MString primGroupName = primGroupPlug.asString();
        if (primGroupName == "")
        {
            primGroupName = "inputPrimitiveComponent";
        }

        CHECK_HAPI(HAPI_AddGroup(Util::theHAPISession.get(), geometryNodeId(),
                                 0, groupType, primGroupName.asChar()));

        CHECK_HAPI(HAPI_SetGroupMembership(
            Util::theHAPISession.get(), geometryNodeId(), 0, groupType,
            primGroupName.asChar(), &groupMembership[0], 0,
            groupMembership.size()));
    }
    if (vertIds.length() > 0)
    {
        int numVerts             = meshFn.numVertices();
        HAPI_GroupType groupType = HAPI_GROUPTYPE_POINT;

        groupMembership.resize(numVerts);
        std::fill(groupMembership.begin(), groupMembership.end(), 0);

        for (i = 0; i < vertIds.length(); i++)
        {
            groupMembership[vertIds[i]] = 1;
        }

        MString pointGroupName = pointGroupPlug.asString();
        if (pointGroupName == "")
        {
            pointGroupName = "inputPointComponent";
        }
        CHECK_HAPI(HAPI_AddGroup(Util::theHAPISession.get(), geometryNodeId(),
                                 0, groupType, pointGroupName.asChar()));

        CHECK_HAPI(HAPI_SetGroupMembership(
            Util::theHAPISession.get(), geometryNodeId(), 0, groupType,
            pointGroupName.asChar(), &groupMembership[0], 0,
            groupMembership.size()));
    }
    HAPI_CommitGeo(Util::theHAPISession.get(), geometryNodeId());
}

void
InputMesh::setInputGeo(MDataBlock &dataBlock, const MPlug &plug)
{
    MDataHandle dataHandle = dataBlock.inputValue(plug);

    // extract mesh data from Maya
    MObject meshObj = dataHandle.asMesh();

    MFnMesh meshFn(meshObj);

    // get face data
    std::vector<int> vertexCount;
    std::vector<int> vertexList;
    {
        MIntArray mayaVertexCount;
        MIntArray mayaVertexList;
        meshFn.getVertices(mayaVertexCount, mayaVertexList);

        vertexCount.resize(mayaVertexCount.length());
        mayaVertexCount.get(&vertexCount[0]);

        vertexList.resize(mayaVertexList.length());
        mayaVertexList.get(&vertexList[0]);
    }
    Util::reverseWindingOrder(vertexList, vertexCount);

    // set up part info
    HAPI_PartInfo partInfo;
    HAPI_PartInfo_Init(&partInfo);
    partInfo.id          = 0;
    partInfo.faceCount   = vertexCount.size();
    partInfo.vertexCount = vertexList.size();
    partInfo.pointCount  = meshFn.numVertices();

    // Set the data
    HAPI_SetPartInfo(
        Util::theHAPISession.get(), geometryNodeId(), 0, &partInfo);
    HAPI_SetFaceCounts(Util::theHAPISession.get(), geometryNodeId(), 0,
                       &vertexCount[0], 0, partInfo.faceCount);
    HAPI_SetVertexList(Util::theHAPISession.get(), geometryNodeId(), 0,
                       &vertexList[0], 0, partInfo.vertexCount);

    // Set position attributes.
    processPoints(meshFn);

    // HACK: For some reason if processSets is called after processUVs, the part
    //       size and the membership in Maya can get out of sync when custom
    //       sets are utilized, which results in a crash.
    processSets(plug, meshFn);

    // normals
    processNormals(meshObj, meshFn, vertexCount);

    // UVs
    processUVs(meshFn, vertexCount, vertexList);

    // Colors and Alphas
    processColorSets(meshFn, vertexCount, vertexList);

    setInputName(HAPI_ATTROWNER_PRIM, partInfo.faceCount, plug);

    // Commit it
    HAPI_CommitGeo(Util::theHAPISession.get(), geometryNodeId());
}

bool
InputMesh::processPoints(const MFnMesh &meshFn)
{
    const float *rawPoints = meshFn.getRawPoints(NULL);

    if (myPreserveScale)
    {
        float *scaledPoints = new float[meshFn.numVertices() * 3];

        for (int i = 0; i < meshFn.numVertices(); i++)
        {
            //     float *scaledPoint = scaledPoints[i + 3];

            scaledPoints[(i * 3) + 0] = rawPoints[(i * 3) + 0] * 0.01f;
            scaledPoints[(i * 3) + 1] = rawPoints[(i * 3) + 1] * 0.01f;
            scaledPoints[(i * 3) + 2] = rawPoints[(i * 3) + 2] * 0.01f;
        }

        // send scaled points to houdini
        CHECK_HAPI(hapiSetPointAttribute(
            geometryNodeId(), 0, 3, "P",
            rawArray(scaledPoints, meshFn.numVertices() * 3)));

        delete[] scaledPoints;
    }
    else
    {
        CHECK_HAPI(hapiSetPointAttribute(
            geometryNodeId(), 0, 3, "P",
            rawArray(meshFn.getRawPoints(NULL), meshFn.numVertices() * 3)));
    }

    return true;
}

bool
InputMesh::processNormals(const MObject &meshObj,
                          const MFnMesh &meshFn,
                          std::vector<int> vertexCount)
{
    // get normal IDs
    MIntArray normalCounts;
    MIntArray normalIds;
    meshFn.getNormalIds(normalCounts, normalIds);

    if (myUnlockNormals || !normalIds.length())
    {
        // if there are no normals being set on the input
        // delete any left over from the previous input
        HAPI_AttributeInfo attributeInfo;
        attributeInfo.exists    = true;
        attributeInfo.owner     = HAPI_ATTROWNER_VERTEX;
        attributeInfo.storage   = HAPI_STORAGETYPE_INT;
        attributeInfo.count     = 1;
        attributeInfo.tupleSize = 1;

        HAPI_DeleteAttribute(Util::theHAPISession.get(), geometryNodeId(), 0,
                             "maya_locked_normal", &attributeInfo);
        attributeInfo.storage   = HAPI_STORAGETYPE_FLOAT;
        attributeInfo.count     = 1;
        attributeInfo.tupleSize = 3;
        HAPI_DeleteAttribute(Util::theHAPISession.get(), geometryNodeId(), 0,
                             "N", &attributeInfo);

        return false;
    }

    // reverse winding order
    Util::reverseWindingOrder(normalIds, vertexCount);

    // get normal values
    const float *rawNormals = meshFn.getRawNormals(NULL);

    // build the per-vertex normals
    std::vector<int> lockedNormals(normalIds.length());
    std::vector<float> vertexNormals(normalIds.length() * 3);
    for (unsigned int i = 0; i < normalIds.length(); ++i)
    {
        if (meshFn.isNormalLocked(normalIds[i]))
        {
            lockedNormals[i] = 1;
        }

        vertexNormals[i * 3 + 0] = (rawNormals[normalIds[i] * 3 + 0]);
        vertexNormals[i * 3 + 1] = (rawNormals[normalIds[i] * 3 + 1]);
        vertexNormals[i * 3 + 2] = (rawNormals[normalIds[i] * 3 + 2]);
    }

    // add and set it to HAPI
    CHECK_HAPI(hapiSetVertexAttribute(
        geometryNodeId(), 0, 1, "maya_locked_normal", lockedNormals));
    CHECK_HAPI(
        hapiSetVertexAttribute(geometryNodeId(), 0, 3, "N", vertexNormals));

    // hard/soft edges
    {
        std::vector<int> hardEdges(meshFn.numFaceVertices());

        int polygonVertexOffset = 0;
        for (MItMeshPolygon itMeshPolygon(meshObj); !itMeshPolygon.isDone();
             itMeshPolygon.next())
        {
            MIntArray edges;
            itMeshPolygon.getEdges(edges);
            int numVertices = edges.length();

            for (int i = 0; i < numVertices; i++)
            {
                if (!meshFn.isEdgeSmooth(edges[i]))
                {
                    // first vertex in the Houdini winding order
                    int polygonVertexIndex = polygonVertexOffset +
                                             (i + 1) % numVertices;
                    hardEdges[polygonVertexIndex] = 1;
                }
                else
                {
                    // default is already 0
                }
            }
            polygonVertexOffset += edges.length();
        }
        assert(polygonVertexOffset == meshFn.numFaceVertices());

        // reverse winding order
        Util::reverseWindingOrder(hardEdges, vertexCount);

        CHECK_HAPI(hapiSetVertexAttribute(
            geometryNodeId(), 0, 1, "maya_hard_edge", hardEdges));
    }

    return true;
}

bool
InputMesh::processUVs(const MFnMesh &meshFn,
                      std::vector<int> vertexCount,
                      std::vector<int> vertexList)
{
    MString currentUVSetName = meshFn.currentUVSetName();

    MStringArray uvSetNames;
    meshFn.getUVSetNames(uvSetNames);

    MStringArray mappedUVAttributeNames;
    mappedUVAttributeNames.setLength(uvSetNames.length());

    for (unsigned int uvSetIndex = 0; uvSetIndex < uvSetNames.length();
         uvSetIndex++)
    {
        const MString uvSetName = uvSetNames[uvSetIndex];

        const MString uvAttributeName = Util::getAttrLayerName(
            "uv", uvSetIndex);
        const MString uvNumberAttributeName = Util::getAttrLayerName(
            "uvNumber", uvSetIndex);

        mappedUVAttributeNames[uvSetIndex] = uvAttributeName;

        // get UV IDs
        MIntArray uvCounts;
        MIntArray uvIds;
        meshFn.getAssignedUVs(uvCounts, uvIds, &uvSetName);

        // reverse winding order
        Util::reverseWindingOrder(uvIds, uvCounts);

        // get UV values
        MFloatArray uArray;
        MFloatArray vArray;
        meshFn.getUVs(uArray, vArray, &uvSetName);

        // build the per-vertex UVs
        std::vector<float> vertexUVs;
        std::vector<int> vertexUVNumbers;
        vertexUVs.reserve(vertexList.size() * 3);
        unsigned int uvIdIndex = 0;
        for (unsigned int i = 0; i < uvCounts.length(); ++i)
        {
            if (uvCounts[i] == vertexCount[i])
            {
                // has UVs assigned
                for (int j = 0; j < uvCounts[i]; ++j)
                {
                    vertexUVs.push_back(uArray[uvIds[uvIdIndex]]);
                    vertexUVs.push_back(vArray[uvIds[uvIdIndex]]);
                    vertexUVs.push_back(0);
                    vertexUVNumbers.push_back(uvIds[uvIdIndex]);

                    uvIdIndex++;
                }
            }
            else
            {
                // no UVs assigned
                for (int j = 0; j < vertexCount[i]; ++j)
                {
                    vertexUVs.push_back(0);
                    vertexUVs.push_back(0);
                    vertexUVs.push_back(0);
                    vertexUVNumbers.push_back(0);
                }
            }
        }

        // add and set it to HAPI
        CHECK_HAPI(hapiSetVertexAttribute(
            geometryNodeId(), 0, 3, uvAttributeName.asChar(), vertexUVs));
        CHECK_HAPI(hapiSetVertexAttribute(geometryNodeId(), 0, 1,
                                          uvNumberAttributeName.asChar(),
                                          vertexUVNumbers));
    }
#if MAYA_API_VERSION > 201600
    // now remove any TEXTURE type parms that no longer correspond
    // to uvsets on the input
    // This seems more complicated but less of a performance hit  than deleting
    // and recreating all the attributes every time we pull on the inputs.

    HAPI_CommitGeo(Util::theHAPISession.get(), geometryNodeId());
    HAPI_CookOptions cook_options = HAPI_CookOptions_Create();
    // cook the input geo so that the group counts are updated on the geoInfo
    HAPI_CookNode(Util::theHAPISession.get(), geometryNodeId(), &cook_options);

    HAPI_PartInfo partInfo;
    CHECK_HAPI(HAPI_GetPartInfo(
        Util::theHAPISession.get(), geometryNodeId(), 0, &partInfo));

    // on the inputs, we know that UV parms are called uv[0-9]*
    // and that uvNumbers have the corresponding names
    // so we just check the names and don't bother checking the types

    int attributeCount = partInfo.attributeCounts[HAPI_ATTROWNER_VERTEX];
    if (attributeCount > 0)
    {
        std::vector<HAPI_StringHandle> attributeNames(attributeCount);
        HAPI_GetAttributeNames(Util::theHAPISession.get(), geometryNodeId(), 0,
                               HAPI_ATTROWNER_VERTEX, &attributeNames[0],
                               attributeCount);
        for (int j = 0; j < attributeCount; j++)
        {
            MString attrMStrName      = Util::HAPIString(attributeNames[j]);
            const char *attributeName = attrMStrName.asChar();
            if (!strncmp(attributeName, "uv", 2) &&
                strncmp(attributeName, "uvNumber", 8))
            {
                if (mappedUVAttributeNames.indexOf(attrMStrName) < 0)
                {
                    HAPI_AttributeInfo attributeInfo;
                    attributeInfo.exists    = true;
                    attributeInfo.owner     = HAPI_ATTROWNER_VERTEX;
                    attributeInfo.storage   = HAPI_STORAGETYPE_FLOAT;
                    attributeInfo.count     = 1;
                    attributeInfo.tupleSize = 3;

                    HAPI_DeleteAttribute(Util::theHAPISession.get(),
                                         geometryNodeId(), 0, attributeName,
                                         &attributeInfo);
                    char vertNumName[16] = "uvNumber";
                    strcat(vertNumName, attributeName + 2);

                    attributeInfo.storage   = HAPI_STORAGETYPE_INT;
                    attributeInfo.count     = 1;
                    attributeInfo.tupleSize = 1;
                    HAPI_DeleteAttribute(Util::theHAPISession.get(),
                                         geometryNodeId(), 0, vertNumName,
                                         &attributeInfo);
                }
            }
        }
    }
#endif

    // update the attribute mappiing parms

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_uv_current", currentUVSetName));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_uv_name", uvSetNames));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_uv_mapped_uv", mappedUVAttributeNames));

    return true;
}

bool
InputMesh::processColorSets(const MFnMesh &meshFn,
                            std::vector<int> vertexCount,
                            std::vector<int> vertexList)
{
    MStringArray currentColorSetName(1, meshFn.currentColorSetName());

    MStringArray colorSetNames;
    meshFn.getColorSetNames(colorSetNames);

    MStringArray mappedCdNames;
    MStringArray mappedAlphaNames;
    MStringArray colorReps;
    mappedCdNames.setLength(colorSetNames.length());
    mappedAlphaNames.setLength(colorSetNames.length());
    colorReps.setLength(colorSetNames.length());

    MColor defaultUnsetColor;
    MColorArray colors;
    std::vector<float> buffer;
    for (unsigned int i = 0; i < colorSetNames.length(); i++)
    {
        const MString colorSetName = colorSetNames[i];

        bool hasColor = false;
        bool hasAlpha = false;
        {
            MFnMesh::MColorRepresentation colorSetRepresentation =
                meshFn.getColorRepresentation(colorSetName);

            switch (colorSetRepresentation)
            {
            case MFnMesh::kAlpha:
                hasAlpha     = true;
                colorReps[i] = "A";
                break;
            case MFnMesh::kRGB:
                hasColor     = true;
                colorReps[i] = "RGB";
                break;
            case MFnMesh::kRGBA:
                colorReps[i] = "RGBA";
                hasColor     = true;
                hasAlpha     = true;
                break;
            }
        }
        CHECK_MSTATUS(const_cast<MFnMesh &>(meshFn).getFaceVertexColors(
            colors, &colorSetName, &defaultUnsetColor));

        // reverse winding order
        Util::reverseWindingOrder(colors, vertexCount);

        if (hasColor)
        {
            const MString colorAttributeName = Util::getAttrLayerName("Cd", i);

            mappedCdNames[i] = colorAttributeName;

            buffer =
                Util::reshapeArray<3, 0, 3, 0, 4, std::vector<float>>(colors);

            // add and set Cd
            CHECK_HAPI(hapiSetVertexAttribute(
                geometryNodeId(), 0, 3, colorAttributeName.asChar(), buffer));
        }

        if (hasAlpha)
        {
            const MString alphaAttributeName = Util::getAttrLayerName(
                "Alpha", i);

            mappedAlphaNames[i] = alphaAttributeName;

            buffer =
                Util::reshapeArray<1, 0, 1, 3, 4, std::vector<float>>(colors);

            // add and set Alpha
            CHECK_HAPI(hapiSetVertexAttribute(
                geometryNodeId(), 0, 1, alphaAttributeName.asChar(), buffer));
        }
    }
#if MAYA_API_VERSION > 201600
    // now remove any color and type parms that are no longer mapped
    // This seems more complicated but less of a performance hit  than deleting
    // and recreating all the attributes every time we pull on the inputs. need
    // to commit to update the parms and cook to update the parm counts

    HAPI_CommitGeo(Util::theHAPISession.get(), geometryNodeId());
    HAPI_CookOptions cook_options = HAPI_CookOptions_Create();
    // cook the input geo so that the group counts are updated on the geoInfo
    HAPI_CookNode(Util::theHAPISession.get(), geometryNodeId(), &cook_options);

    HAPI_PartInfo partInfo;
    CHECK_HAPI(HAPI_GetPartInfo(
        Util::theHAPISession.get(), geometryNodeId(), 0, &partInfo));

    // on the inputs, we know that color parms are called Cd[0-9]*
    // and that mapped Alpha are called Alpha[0-9]
    // so we just check the names and don't bother checking the types

    int attributeCount = partInfo.attributeCounts[HAPI_ATTROWNER_VERTEX];
    if (attributeCount > 0)
    {
        std::vector<HAPI_StringHandle> attributeNames(attributeCount);
        HAPI_GetAttributeNames(Util::theHAPISession.get(), geometryNodeId(), 0,
                               HAPI_ATTROWNER_VERTEX, &attributeNames[0],
                               attributeCount);
        for (int j = 0; j < attributeCount; j++)
        {
            MString attrMStrName      = Util::HAPIString(attributeNames[j]);
            const char *attributeName = attrMStrName.asChar();
            if (!strncmp(attributeName, "Cd", 2))
            {
                if (mappedCdNames.indexOf(attrMStrName) < 0)
                {
                    HAPI_AttributeInfo attributeInfo;
                    attributeInfo.exists    = true;
                    attributeInfo.owner     = HAPI_ATTROWNER_VERTEX;
                    attributeInfo.storage   = HAPI_STORAGETYPE_FLOAT;
                    attributeInfo.count     = 1;
                    attributeInfo.tupleSize = 3;

                    HAPI_DeleteAttribute(Util::theHAPISession.get(),
                                         geometryNodeId(), 0, attributeName,
                                         &attributeInfo);
                }
            }
            if (!strncmp(attributeName, "Alpha", 2))
            {
                if (mappedAlphaNames.indexOf(attrMStrName) < 0)
                {
                    HAPI_AttributeInfo attributeInfo;
                    attributeInfo.exists    = true;
                    attributeInfo.owner     = HAPI_ATTROWNER_VERTEX;
                    attributeInfo.storage   = HAPI_STORAGETYPE_FLOAT;
                    attributeInfo.count     = 1;
                    attributeInfo.tupleSize = 1;

                    HAPI_DeleteAttribute(Util::theHAPISession.get(),
                                         geometryNodeId(), 0, attributeName,
                                         &attributeInfo);
                }
            }
        }
    }
#endif

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_colorset_current", currentColorSetName));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_colorset_name", colorSetNames));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_colorset_mapped_Cd", mappedCdNames));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_colorset_mapped_Alpha", mappedAlphaNames));

    CHECK_HAPI(hapiSetDetailAttribute(
        geometryNodeId(), 0, "maya_colorRep", colorReps));

    return true;
}

bool
InputMesh::processSets(const MPlug &plug, const MFnMesh &meshFn)
{
    MStatus status;

    // The source node may not be actually mesh node.
    MPlug srcPlug = Util::plugSource(plug);
    if (srcPlug.isNull())
    {
        return false;
    }

    // XXX: instance number
    // MDagPath srcDagPath = MDagPath::getAPathTo(srcPlug.node());
    MDagPath srcDagPath;
    MDagPath::getAPathTo(srcPlug.node(), srcDagPath);

    // Sets and Members
    MFnMesh srcNodeFn(srcDagPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MObjectArray sets;
    MObjectArray comps;
    std::vector<std::string> setNamesUsed;
    // XXX: instance number
    srcNodeFn.getConnectedSetsAndMembers(0, sets, comps, false);

    MStringArray sgNames;
    MObjectArray sgCompObjs;

    std::vector<int> groupMembership;
    for (int setIndex = 0; setIndex < (int)sets.length(); setIndex++)
    {
        const MObject &setObj  = sets[setIndex];
        const MObject &compObj = comps[setIndex];

        MFnDependencyNode setFn(setObj, &status);
        CHECK_MSTATUS(status);

        // Filter out unwanted sets. This mimics setFilterScript().
        MString setType = setFn.typeName();
        if (setType == "shadingEngine")
        {
            sgNames.append(setFn.name());
            sgCompObjs.append(compObj);
            continue;
        }

        if (!(setType == "objectSet" || setType == "textureBakeSet" ||
              setType == "vertexBakeSet" || setType == "creaseSet" ||
              setType == "character"))
        {
            continue;
        }

        // these are mostly hidden set types that are used internally
        // by various maya tools - so we skip them to avoid conflicts
        if (setFn.findPlug("verticesOnlySet", true).asBool() ||
            setFn.findPlug("edgesOnlySet", true).asBool() ||
            setFn.findPlug("editPointsOnlySet", true).asBool() ||
            setFn.findPlug("renderableOnlySet", true).asBool())
        {
            continue;
        }

        // BUT ... they're documemented, so some customers have
        // found and used some of them - We will provide options
        // to allow them if required
        if (!myAllowFacetSet && setFn.findPlug("facetsOnlySet", true).asBool())
        {
            // if facet sets are not allowed they might have been before
            // delete the group for the facet set in case it exists already

            MString setName = setFn.name();
            setName         = Util::sanitizeStringForNodeName(setName);
            CHECK_HAPI(HAPI_DeleteGroup(Util::theHAPISession.get(),
                                        geometryNodeId(), 0,
                                        HAPI_GROUPTYPE_PRIM, setName.asChar()));
            continue;
        }

        HAPI_GroupType groupType;

        if (compObj.isNull())
        {
            groupType = HAPI_GROUPTYPE_PRIM;
            groupMembership.resize(meshFn.numPolygons());

            std::fill(groupMembership.begin(), groupMembership.end(), 1);
        }
        else
        {
            MFnSingleIndexedComponent componentFn(compObj, &status);
            CHECK_MSTATUS(status);

            switch (componentFn.componentType())
            {
            case MFn::kMeshPolygonComponent:
                groupType = HAPI_GROUPTYPE_PRIM;
                groupMembership.resize(meshFn.numPolygons());
                break;
            case MFn::kMeshVertComponent:
                groupType = HAPI_GROUPTYPE_POINT;
                groupMembership.resize(meshFn.numVertices());
                break;
            default:
                continue;
                break;
            }

            std::fill(groupMembership.begin(), groupMembership.end(), 0);
            for (int i = 0; i < componentFn.elementCount(); i++)
            {
                groupMembership[componentFn.element(i)] = 1;
            }
        }

        MString setName = setFn.name();
        // If the set is in a namespace, the name will contain a colon.
        setName                = Util::sanitizeStringForNodeName(setName);
        std::string setNameStr = setName.asChar();
        Util::markItemNameUsed(setNameStr, setNamesUsed);

        CHECK_HAPI(HAPI_AddGroup(Util::theHAPISession.get(), geometryNodeId(),
                                 0, groupType, setName.asChar()));

        CHECK_HAPI(HAPI_SetGroupMembership(
            Util::theHAPISession.get(), geometryNodeId(), 0, groupType,
            setName.asChar(), &groupMembership[0], 0, groupMembership.size()));
    }
    // now remove any groups that no longer correspond to sets on the input

    // Commit it (even though we only care about pre-existing groups to delete
    // if we don't commit the geo the first time through, bad stuff happens
    HAPI_CommitGeo(Util::theHAPISession.get(), geometryNodeId());

    HAPI_CookOptions cook_options = HAPI_CookOptions_Create();
    // cook the input geo so that the group counts are updated on the geoInfo
    HAPI_CookNode(Util::theHAPISession.get(), geometryNodeId(), &cook_options);
    HAPI_PartInfo partInfo;
    HAPI_GeoInfo geoInfo;
    CHECK_HAPI(HAPI_GetGeoInfo(
        Util::theHAPISession.get(), geometryNodeId(), &geoInfo));
    CHECK_HAPI(HAPI_GetPartInfo(
        Util::theHAPISession.get(), geometryNodeId(), 0, &partInfo));

    if (geoInfo.pointGroupCount > 0)
    {
        std::vector<HAPI_StringHandle> groupNames(geoInfo.pointGroupCount);
        HAPI_GetGroupNames(Util::theHAPISession.get(), geometryNodeId(),
                           HAPI_GROUPTYPE_POINT, &groupNames[0],
                           geoInfo.pointGroupCount);
        for (int j = 0; j < geoInfo.pointGroupCount; j++)
        {
            MString groupName        = Util::HAPIString(groupNames[j]);
            std::string groupNameStr = groupName.asChar();
            if (!Util::isItemNameUsed(groupNameStr, setNamesUsed))
            {
                CHECK_HAPI(HAPI_DeleteGroup(
                    Util::theHAPISession.get(), geometryNodeId(), 0,
                    HAPI_GROUPTYPE_POINT, groupName.asChar()));
            }
        }
    }
    if (geoInfo.primitiveGroupCount > 0)
    {
        std::vector<HAPI_StringHandle> groupNames(geoInfo.primitiveGroupCount);
        HAPI_GetGroupNames(Util::theHAPISession.get(), geometryNodeId(),
                           HAPI_GROUPTYPE_PRIM, &groupNames[0],
                           geoInfo.primitiveGroupCount);
        for (int j = 0; j < geoInfo.primitiveGroupCount; j++)
        {
            MString groupName        = Util::HAPIString(groupNames[j]);
            std::string groupNameStr = groupName.asChar();
            if (!Util::isItemNameUsed(groupNameStr, setNamesUsed))
            {
                CHECK_HAPI(HAPI_DeleteGroup(
                    Util::theHAPISession.get(), geometryNodeId(), 0,
                    HAPI_GROUPTYPE_PRIM, groupName.asChar()));
            }
        }
    }

    processShadingGroups(meshFn, sgNames, sgCompObjs);

    return true;
}

bool
InputMesh::processShadingGroups(const MFnMesh &meshFn,
                                const MStringArray &sgNames,
                                const MObjectArray &sgCompObjs)
{
    MStatus status;

    // Normally, if there is no per face shader assignment
    // we use a detail attribute to keep track of the maya shading group
    // However, if we anticipate merging the input objects, we can
    // promote the shading group info to primitive attributes so that
    // it will survive the merge.

    if (sgCompObjs.length() == 1 && sgCompObjs[0].isNull())
    {
        if (!myMatPerFace)
        {
            CHECK_HAPI(hapiSetDetailAttribute(
                geometryNodeId(), 0, "maya_shading_group",
                const_cast<MStringArray &>(sgNames)[0]));
        }
        else
        {
            std::vector<const char *> sgNamePerComp;
            sgNamePerComp.resize(meshFn.numPolygons(), sgNames[0].asChar());
            CHECK_HAPI(hapiSetPrimAttribute(
                geometryNodeId(), 0, 1, "maya_shading_group", sgNamePerComp));
        }
    }
    else
    {
        MString defaultShader;

        std::vector<const char *> sgNamePerComp;
        sgNamePerComp.resize(meshFn.numPolygons(), defaultShader.asChar());

        for (int i = 0; i < (int)sgNames.length(); i++)
        {
            const char *sgName       = sgNames[i].asChar();
            const MObject &sgCompObj = sgCompObjs[i];

            assert(!sgCompObj.isNull());

            MFnSingleIndexedComponent componentFn(sgCompObj, &status);
            CHECK_MSTATUS(status);

            assert(componentFn.componentType() == MFn::kMeshPolygonComponent);

            for (int j = 0; j < componentFn.elementCount(); j++)
            {
                sgNamePerComp[componentFn.element(j)] = sgName;
            }
        }

        CHECK_HAPI(hapiSetPrimAttribute(
            geometryNodeId(), 0, 1, "maya_shading_group", sgNamePerComp));
    }

    return true;
}
