#include "InputMesh.h"

#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MDataBlock.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnComponentListData.h>

#include "hapiutil.h"
#include "types.h"
#include "util.h"

InputMesh::InputMesh() :
    Input()
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    HAPI_NodeId nodeId;
    CHECK_HAPI(HAPI_CreateInputNode(
        Util::theHAPISession.get(),
        &nodeId,
        NULL
        ));
    if(!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating input asset."));
    }

    HAPI_NodeInfo nodeInfo;
    HAPI_GetNodeInfo(
        Util::theHAPISession.get(),
        nodeId,
        &nodeInfo
        );

    setTransformNodeId(nodeInfo.parentId);
    setGeometryNodeId(nodeId);
}

InputMesh::~InputMesh()
{
    CHECK_HAPI(HAPI_DeleteNode(
                Util::theHAPISession.get(),
                geometryNodeId()
                ));
}

Input::AssetInputType
InputMesh::assetInputType() const
{
    return Input::AssetInputType_Mesh;
}

void
InputMesh::setInputComponents(
        MDataBlock &dataBlock,
        const MPlug &geoPlug,
        const MPlug &compPlug
        )
{
    // extract mesh data from Maya
    MDataHandle compHandle = dataBlock.inputValue(compPlug);
    MObject compList = compHandle.data();
    MFnComponentListData compListFn( compList );

    // extract mesh data from Maya
    MDataHandle meshHandle = dataBlock.inputValue(geoPlug);
    MObject meshObj = meshHandle.asMesh();
    MFnMesh meshFn(meshObj);

    unsigned i;
    int j;
    MIntArray faceIds;
    MIntArray vertIds;
    MIntArray edgeIds;
    for( i = 0; i < compListFn.length(); i++ )
    {
        MObject comp = compListFn[i];
	if( comp.apiType() == MFn::kMeshPolygonComponent) {
            MFnSingleIndexedComponent siComp( comp );
            for( j = 0; j < siComp.elementCount(); j++ )
                faceIds.append( siComp.element(j) );
	}
	if( comp.apiType() == MFn::kMeshVertComponent) {
            MFnSingleIndexedComponent siComp( comp );
            for( j = 0; j < siComp.elementCount(); j++ )
                vertIds.append( siComp.element(j) );
	}

	if( comp.apiType() == MFn::kMeshEdgeComponent) {
	    // should convert the edge component to a point group in some meaningful way
            MFnSingleIndexedComponent siComp( comp );
            for( j = 0; j < siComp.elementCount(); j++ )
                edgeIds.append( siComp.element(j) );
	}
	if( comp.apiType() == MFn::kMeshVtxFaceComponent) {
	    // unfortunately, setAttr doesn't support vertexFace components
	    // for a componentList attr: may need a command to set these
            // MFnDoubleIndexedComponent doComp( comp );
	}
    }
    std::vector<int> groupMembership;
    
    if(faceIds.length() > 0) {
          int numFaces = meshFn.numPolygons();
	  HAPI_GroupType groupType = HAPI_GROUPTYPE_PRIM;

	  groupMembership.resize(numFaces);
	  std::fill(groupMembership.begin(), groupMembership.end(), 0);

	  for( i = 0; i < faceIds.length(); i++)
	  {
	      groupMembership[faceIds[i]] = 1;
          }
	  
	  MString primGroupName = "inputPrimitiveComponent";
          CHECK_HAPI(HAPI_AddGroup(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    primGroupName.asChar()
                    ));

          CHECK_HAPI(HAPI_SetGroupMembership(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    primGroupName.asChar(),
                    &groupMembership[0],
                    0, groupMembership.size()
                   ));
    }
    if(vertIds.length() > 0) {
         int numVerts = meshFn.numVertices();
	 HAPI_GroupType groupType = HAPI_GROUPTYPE_POINT;

	 groupMembership.resize(numVerts);
	 std::fill(groupMembership.begin(), groupMembership.end(), 0);

	 for( i = 0; i < vertIds.length(); i++)
	 {
	     groupMembership[vertIds[i]] = 1;
         }
	  
	 MString primGroupName = "inputPointComponent";
         CHECK_HAPI(HAPI_AddGroup(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    primGroupName.asChar()
                    ));

         CHECK_HAPI(HAPI_SetGroupMembership(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    primGroupName.asChar(),
                    &groupMembership[0],
                    0, groupMembership.size()
                    ));
    }
    HAPI_CommitGeo(
        Util::theHAPISession.get(),
        geometryNodeId()
        );
}

void
InputMesh::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
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
    partInfo.id = 0;
    partInfo.faceCount        = vertexCount.size();
    partInfo.vertexCount      = vertexList.size();
    partInfo.pointCount       = meshFn.numVertices();

    // Set the data
    HAPI_SetPartInfo(
            Util::theHAPISession.get(),
            geometryNodeId(), 0,
            &partInfo
            );
    HAPI_SetFaceCounts(
            Util::theHAPISession.get(),
            geometryNodeId(), 0,
            &vertexCount[0],
            0, partInfo.faceCount
            );
    HAPI_SetVertexList(
            Util::theHAPISession.get(),
            geometryNodeId(), 0,
            &vertexList[0],
            0, partInfo.vertexCount
            );

    // Set position attributes.
    processPoints(meshFn);

    // normals
    processNormals(meshObj, meshFn, vertexCount);

    // UVs
    processUVs(meshFn, vertexCount, vertexList);

    // Colors and Alphas
    processColorSets(meshFn, vertexCount, vertexList);

    processSets(plug, meshFn);

    setInputName(
            HAPI_ATTROWNER_PRIM, partInfo.faceCount,
            plug
            );

    // Commit it
    HAPI_CommitGeo(
            Util::theHAPISession.get(),
            geometryNodeId()
            );
}

bool
InputMesh::processPoints(
        const MFnMesh &meshFn
        )
{
    CHECK_HAPI(hapiSetPointAttribute(
            geometryNodeId(), 0,
            3,
            "P",
            rawArray(
                meshFn.getRawPoints(NULL),
                meshFn.numVertices() * 3
                )
            ));

    return true;
}

bool
InputMesh::processNormals(
        const MObject &meshObj,
        const MFnMesh &meshFn,
        std::vector<int> vertexCount
        )
{
    // get normal IDs
    MIntArray normalCounts;
    MIntArray normalIds;
    meshFn.getNormalIds(normalCounts, normalIds);

    if(!normalIds.length())
    {
        return false;
    }

    // reverse winding order
    Util::reverseWindingOrder(normalIds, vertexCount);

    // get normal values
    const float* rawNormals = meshFn.getRawNormals(NULL);

    // build the per-vertex normals
    std::vector<int> lockedNormals(normalIds.length());
    std::vector<float> vertexNormals;
    vertexNormals.reserve(normalIds.length() * 3);
    for(unsigned int i = 0; i < normalIds.length(); ++i)
    {
        if(meshFn.isNormalLocked(normalIds[i]))
        {
            lockedNormals[i] = 1;
        }

        vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 0]);
        vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 1]);
        vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 2]);
    }

    // add and set it to HAPI
    CHECK_HAPI(hapiSetVertexAttribute(
                geometryNodeId(), 0,
                1,
                "maya_locked_normal",
                lockedNormals
                ));
    CHECK_HAPI(hapiSetVertexAttribute(
            geometryNodeId(), 0,
            3,
            "N",
            vertexNormals
            ));

    // hard/soft edges
    {
        std::vector<int> hardEdges(meshFn.numFaceVertices());

        int polygonVertexOffset = 0;
        for(MItMeshPolygon itMeshPolygon(meshObj);
                !itMeshPolygon.isDone(); itMeshPolygon.next())
        {
            MIntArray edges;
            itMeshPolygon.getEdges(edges);
            int numVertices = edges.length();

            for(int i = 0; i < numVertices; i++)
            {
                if(!meshFn.isEdgeSmooth(edges[i]))
                {
                    // first vertex in the Houdini winding order
                    int polygonVertexIndex = polygonVertexOffset + (i + 1) % numVertices;
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
                    geometryNodeId(), 0,
                    1,
                    "maya_hard_edge",
                    hardEdges
                    ));
    }


    return true;
}

bool
InputMesh::processUVs(
        const MFnMesh &meshFn,
        std::vector<int> vertexCount,
        std::vector<int> vertexList
        )
{
    MString currentUVSetName = meshFn.currentUVSetName();

    MStringArray uvSetNames;
    meshFn.getUVSetNames(uvSetNames);

    MStringArray mappedUVAttributeNames;
    mappedUVAttributeNames.setLength(uvSetNames.length());

    for(unsigned int uvSetIndex = 0;
            uvSetIndex < uvSetNames.length(); uvSetIndex++)
    {
        const MString uvSetName = uvSetNames[uvSetIndex];

        const MString uvAttributeName
            = Util::getAttrLayerName("uv", uvSetIndex);
        const MString uvNumberAttributeName
            = Util::getAttrLayerName("uvNumber", uvSetIndex);

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
        for(unsigned int i = 0; i < uvCounts.length(); ++i)
        {
            if(uvCounts[i] == vertexCount[i])
            {
                // has UVs assigned
                for(int j = 0; j < uvCounts[i]; ++j)
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
                for(int j = 0; j < vertexCount[i]; ++j)
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
                geometryNodeId(), 0,
                3,
                uvAttributeName.asChar(),
                vertexUVs
                ));
        CHECK_HAPI(hapiSetVertexAttribute(
                geometryNodeId(), 0,
                1,
                uvNumberAttributeName.asChar(),
                vertexUVNumbers
                ));
    }

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_uv_current",
            currentUVSetName
            ));

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_uv_name",
            uvSetNames
            ));

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_uv_mapped_uv",
            mappedUVAttributeNames
            ));

    return true;
}

bool
InputMesh::processColorSets(
        const MFnMesh &meshFn,
        std::vector<int> vertexCount,
        std::vector<int> vertexList
        )
{
    MStringArray currentColorSetName(1, meshFn.currentColorSetName());

    MStringArray colorSetNames;
    meshFn.getColorSetNames(colorSetNames);

    MStringArray mappedCdNames;
    MStringArray mappedAlphaNames;
    mappedCdNames.setLength(colorSetNames.length());
    mappedAlphaNames.setLength(colorSetNames.length());

    MColor defaultUnsetColor;
    MColorArray colors;
    std::vector<float> buffer;
    for(unsigned int i = 0; i < colorSetNames.length(); i++)
    {
        const MString colorSetName = colorSetNames[i];

        bool hasColor = false;
        bool hasAlpha = false;
        {
            MFnMesh::MColorRepresentation colorSetRepresentation =
                meshFn.getColorRepresentation(colorSetName);

            switch(colorSetRepresentation)
            {
                case MFnMesh::kAlpha:
                    hasAlpha = true;
                    break;
                case MFnMesh::kRGB:
                    hasColor = true;
                    break;
                case MFnMesh::kRGBA:
                    hasColor = true;
                    hasAlpha = true;
                    break;
            }
        }

        CHECK_MSTATUS(const_cast<MFnMesh&>(meshFn).getFaceVertexColors(
                    colors,
                    &colorSetName,
                    &defaultUnsetColor
                    ));

        // reverse winding order
        Util::reverseWindingOrder(colors, vertexCount);

        if(hasColor)
        {
            const MString colorAttributeName
                = Util::getAttrLayerName("Cd", i);

            mappedCdNames[i] = colorAttributeName;

            buffer = Util::reshapeArray<
                3,
                0, 3,
                0, 4,
                std::vector<float>
                >(colors);

            // add and set Cd
            CHECK_HAPI(hapiSetVertexAttribute(
                    geometryNodeId(), 0,
                    3,
                    colorAttributeName.asChar(),
                    buffer
                    ));
        }

        if(hasAlpha)
        {
            const MString alphaAttributeName
                = Util::getAttrLayerName("Alpha", i);

            mappedAlphaNames[i] = alphaAttributeName;

            buffer = Util::reshapeArray<
                1,
                0, 1,
                3, 4,
                std::vector<float>
                >(colors);

            // add and set Alpha
            CHECK_HAPI(hapiSetVertexAttribute(
                    geometryNodeId(), 0,
                    1,
                    alphaAttributeName.asChar(),
                    buffer
                    ));
        }
    }

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_colorset_current",
            currentColorSetName
            ));

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_colorset_name",
            colorSetNames
            ));

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_colorset_mapped_Cd",
            mappedCdNames
            ));

    CHECK_HAPI(hapiSetDetailAttribute(
            geometryNodeId(), 0,
            "maya_colorset_mapped_Alpha",
            mappedAlphaNames
            ));

    return true;
}

bool
InputMesh::processSets(
        const MPlug &plug,
        const MFnMesh &meshFn
        )
{
    MStatus status;

    // The source node may not be actually mesh node.
    MPlug srcPlug = Util::plugSource(plug);
    if(srcPlug.isNull())
    {
        return false;
    }

    // XXX: instance number
    MDagPath srcDagPath = MDagPath::getAPathTo(srcPlug.node());

    // Sets and Members
    MFnMesh srcNodeFn(srcDagPath, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MObjectArray sets;
    MObjectArray comps;
    // XXX: instance number
    srcNodeFn.getConnectedSetsAndMembers(0, sets, comps, false);

    MStringArray sgNames;
    MObjectArray sgCompObjs;

    std::vector<int> groupMembership;
    for(int setIndex = 0; setIndex < (int) sets.length(); setIndex++)
    {
        const MObject &setObj = sets[setIndex];
        const MObject &compObj = comps[setIndex];

        MFnDependencyNode setFn(setObj, &status);
        CHECK_MSTATUS(status);

        // Filter out unwanted sets. This mimics setFilterScript().
        MString setType = setFn.typeName();
        if(setType == "shadingEngine")
        {
            sgNames.append(setFn.name());
            sgCompObjs.append(compObj);
            continue;
        }

        if(!(setType == "objectSet" ||
                    setType == "textureBakeSet" ||
                    setType == "vertexBakeSet" ||
                    setType == "creaseSet" ||
                    setType == "character"))
        {
            continue;
        }

        if(setFn.findPlug("verticesOnlySet").asBool()
                || setFn.findPlug("edgesOnlySet").asBool()
                || setFn.findPlug("facetsOnlySet").asBool()
                || setFn.findPlug("editPointsOnlySet").asBool()
                || setFn.findPlug("renderableOnlySet").asBool()
          )
        {
            continue;
        }

        HAPI_GroupType groupType;

        if(compObj.isNull())
        {
            groupType = HAPI_GROUPTYPE_PRIM;
            groupMembership.resize(meshFn.numPolygons());

            std::fill(groupMembership.begin(), groupMembership.end(), 1);
        }
        else
        {
            MFnSingleIndexedComponent componentFn(compObj, &status);
            CHECK_MSTATUS(status);

            switch(componentFn.componentType())
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
            for(int i = 0; i < componentFn.elementCount(); i++)
            {
                groupMembership[componentFn.element(i)] = 1;
            }
        }

        MString setName = setFn.name();
        // If the set is in a namespace, the name will contain a colon.
        setName = Util::sanitizeStringForNodeName(setName);

        CHECK_HAPI(HAPI_AddGroup(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    setName.asChar()
                    ));

        CHECK_HAPI(HAPI_SetGroupMembership(
                    Util::theHAPISession.get(),
                    geometryNodeId(), 0,
                    groupType,
                    setName.asChar(),
                    &groupMembership[0],
                    0, groupMembership.size()
                    ));
    }

    processShadingGroups(meshFn, sgNames, sgCompObjs);

    return true;
}

bool
InputMesh::processShadingGroups(
        const MFnMesh &meshFn,
        const MStringArray &sgNames,
        const MObjectArray &sgCompObjs
        )
{
    MStatus status;

    if(sgCompObjs.length() == 1 && sgCompObjs[0].isNull())
    {
        CHECK_HAPI(hapiSetDetailAttribute(
                    geometryNodeId(), 0,
                    "maya_shading_group",
                    const_cast<MStringArray&>(sgNames)[0]
                    ));
    }
    else
    {
        MString defaultShader;

        std::vector<const char*> sgNamePerComp;
        sgNamePerComp.resize(meshFn.numPolygons(), defaultShader.asChar());

        for(int i = 0; i < (int) sgNames.length(); i++)
        {
            const char* sgName = sgNames[i].asChar();
            const MObject &sgCompObj = sgCompObjs[i];

            assert(!sgCompObj.isNull());

            MFnSingleIndexedComponent componentFn(sgCompObj, &status);
            CHECK_MSTATUS(status);

            assert(componentFn.componentType()
                    == MFn::kMeshPolygonComponent);

            for(int j = 0; j < componentFn.elementCount(); j++)
            {
                sgNamePerComp[componentFn.element(j)] = sgName;
            }
        }

        CHECK_HAPI(hapiSetPrimAttribute(
                    geometryNodeId(), 0,
                    1,
                    "maya_shading_group",
                    sgNamePerComp
                    ));
    }

    return true;
}
