#include "SyncOutputGeometryPart.h"

#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnTypedAttribute.h>

#include "AssetNode.h"
#include "SyncOutputMaterial.h"
#include "util.h"

#include <string>
#include <algorithm>
#include <map>

static MObject
createAttributeFromDataHandle(
        const MString &attributeName,
        const MDataHandle &dataHandle
        )
{
    MObject attribute;

    bool isNumeric = false;
    bool isNull = false;
    dataHandle.isGeneric(isNumeric, isNull);

    if(isNumeric)
    {
        // This is a singleton generic type. It is not MFnNumericData, and
        // needs to be handled differently. This is stored internally as a
        // double, and there's no way to determine the original type.
        MFnNumericAttribute numericAttribute;
        attribute = numericAttribute.create(
                attributeName,
                attributeName,
                MFnNumericData::kDouble
                );
    }
    else if(dataHandle.numericType() != MFnNumericData::kInvalid)
    {
        MFnNumericAttribute numericAttribute;
        attribute = numericAttribute.create(
                attributeName,
                attributeName,
                dataHandle.numericType()
                );
    }
    else if(dataHandle.type() != MFnData::kInvalid)
    {
        MFnTypedAttribute typedAttribute;
        attribute = typedAttribute.create(
                attributeName,
                attributeName,
                dataHandle.type()
                );
    }

    return attribute;
}

SyncOutputGeometryPart::SyncOutputGeometryPart(
        const MPlug &outputPlug,
        const MObject &objectTransform
        ) :
    myOutputPlug(outputPlug),
    myObjectTransform(objectTransform),
    myIsInstanced(false)
{
}

SyncOutputGeometryPart::~SyncOutputGeometryPart()
{
}

MStatus
SyncOutputGeometryPart::doIt()
{
    MStatus status;

    MString partName = myOutputPlug.child(AssetNode::outputPartName).asString();
    if(partName.length())
    {
        partName = Util::sanitizeStringForNodeName(partName);
    }
    else
    {
        partName = "emptyPart";
    }

    MFnDagNode objectTransformFn(myObjectTransform);

    myPartTransform = Util::findDagChild(objectTransformFn, partName);
    assert(myPartTransform.isNull()); // TODO: does the transform ever exist?

    // create myPartTransform
    myPartTransform = myDagModifier.createNode("transform", myObjectTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename myPartTransform
    status = myDagModifier.renameNode(myPartTransform, partName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // create part
    status = createOutputPart(
            myObjectTransform,
            partName
            );

    return redoIt();
}

MStatus
SyncOutputGeometryPart::doItPost(
        SyncOutputGeometryPart *const *syncParts
        )
{
    // A second pass for setting up the nodes. This second pass has access to
    // all the nodes that were created in the first pass.

    MPlug hasInstancerPlug = myOutputPlug.child(AssetNode::outputPartHasInstancer);
    if(hasInstancerPlug.asBool())
    {
        createOutputInstancerPost(
                myOutputPlug.child(AssetNode::outputPartInstancer),
                syncParts
                );
    }

    return redoIt();
}

MStatus
SyncOutputGeometryPart::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool
SyncOutputGeometryPart::isUndoable() const
{
    return true;
}

MStatus
SyncOutputGeometryPart::createOutputPart(
        const MObject &objectTransform,
        const MString &partName
        )
{
    MStatus status;

    // create mesh
    MPlug hasMeshPlug = myOutputPlug.child(AssetNode::outputPartHasMesh);
    if(hasMeshPlug.asBool())
    {
        status = createOutputMesh(
                partName,
                myOutputPlug.child(AssetNode::outputPartMesh)
                );
    }

    // create particle
    MPlug particleExistsPlug = myOutputPlug.child(AssetNode::outputPartHasParticles);
    if(particleExistsPlug.asBool())
    {
        status = createOutputParticle(
                partName,
                myOutputPlug.child(AssetNode::outputPartParticle)
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // create curves
    MPlug curveIsBezier = myOutputPlug.child(AssetNode::outputPartCurvesIsBezier);
    createOutputCurves(myOutputPlug.child(AssetNode::outputPartCurves),
                       partName,
                       curveIsBezier.asBool());

    // create instancer
    MPlug hasInstancerPlug = myOutputPlug.child(AssetNode::outputPartHasInstancer);
    if(hasInstancerPlug.asBool())
    {
        createOutputInstancer(
                partName,
                myOutputPlug.child(AssetNode::outputPartInstancer)
                );
    }

    // doIt
    // Need to do it here right away because otherwise the top level
    // transform won't be shared.
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputMesh(
        const MString &partName,
        const MPlug &meshPlug
        )
{
    MStatus status;

    // create mesh
    MObject meshShape = myDagModifier.createNode("mesh", myPartTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename mesh
    status = myDagModifier.renameNode(meshShape, partName + "Shape");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDagNode partMeshFn(meshShape, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set mesh.displayColors
    myDagModifier.newPlugValueBool(
            partMeshFn.findPlug("displayColors"),
            true
            );

    // set mesh.currentColorSet
    // Connecting outputPartMeshCurrentColorSet to currentColorSet doesn't seem
    // to trigger updates.
    myDagModifier.newPlugValueString(
            partMeshFn.findPlug("currentColorSet"),
            meshPlug.child(AssetNode::outputPartMeshCurrentColorSet).asString()
            );

    // set mesh.currentUVSet
    // Connecting outputPartMeshCurrentUV to currentUVSet doesn't seem to
    // trigger updates.
    myDagModifier.newPlugValueString(
            partMeshFn.findPlug("currentUVSet"),
            meshPlug.child(AssetNode::outputPartMeshCurrentUV).asString()
            );

    // connect mesh attributes
    {
        MPlug srcPlug;
        MPlug dstPlug;

        //srcPlug = meshPlug.child(AssetNode::outputPartMeshCurrentColorSet);
        //dstPlug = partMeshFn.findPlug("currentColorSet");
        //status = myDagModifier.connect(srcPlug, dstPlug);
        //CHECK_MSTATUS_AND_RETURN_IT(status);

        //srcPlug = meshPlug.child(AssetNode::outputPartMeshCurrentUV);
        //dstPlug = partMeshFn.findPlug("currentUVSet");
        //status = myDagModifier.connect(srcPlug, dstPlug);
        //CHECK_MSTATUS_AND_RETURN_IT(status);

        srcPlug = meshPlug.child(AssetNode::outputPartMeshData);
        dstPlug = partMeshFn.findPlug("inMesh");
        status = myDagModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    MPlug mayaSGAttributePlug;
    createOutputExtraAttributes(
            meshShape,
            &mayaSGAttributePlug
            );

    // createOutputExtraAttributes() seems to cause something in the DG to be
    // dirty/invalid. We need to force the mesh to evaluate before running the
    // "sets" command. Otherwise, the "sets" command would fail when there are
    // extra attributes.
    partMeshFn.findPlug("outMesh").asMObject();

    createOutputGroups(
            meshShape
            );

    // material
    {
        std::vector<bool> hasMaterials;
        typedef std::pair<MObject, MIntArray*> MaterialComponent;
        std::vector<MaterialComponent> materialComponents;

        // if there are material ids
        MPlug materialIdsPlug = meshPlug.parent().child(AssetNode::outputPartMaterialIds);
        const MFnIntArrayData materialIdsData(materialIdsPlug.asMObject());
        if(materialIdsData.length())
        {
            std::map<int, MaterialComponent> materialComponentsMap;

            if(hasMaterials.size() < materialIdsData.length())
            {
                hasMaterials.resize(materialIdsData.length());
            }

            // gather material ids
            for(size_t i = 0; i < materialIdsData.length(); i++)
            {
                if(hasMaterials[i])
                {
                    continue;
                }

                int materialId = materialIdsData[i];
                if(materialId == -1)
                {
                    continue;
                }

                hasMaterials[i] = true;

                std::pair<std::map<int, MaterialComponent>::iterator, bool> r
                    = materialComponentsMap.insert(
                            std::pair<int, MaterialComponent>(
                                materialId, MaterialComponent()));
                // if first time seeing the material id
                if(r.second)
                {
                    MaterialComponent &materialComponent = r.first->second;
                    // create the material
                    materialComponent.first = SyncOutputMaterial::createOutputMaterial(
                            myDagModifier,
                            myOutputPlug.node(),
                            materialId);
                    materialComponent.second = new MIntArray();
                }

                r.first->second.second->append(i);
            }

            for(std::map<int, MaterialComponent>::iterator iter
                    = materialComponentsMap.begin();
                    iter != materialComponentsMap.end(); iter++)
            {
                materialComponents.push_back(iter->second);
            }
        }

        // if there are maya_shading_group
        if(!mayaSGAttributePlug.isNull())
        {
            MPlug mayaSGOwnerPlug = mayaSGAttributePlug.child(
                    AssetNode::outputPartExtraAttributeOwner
                    );
            MPlug mayaSGDataPlug = mayaSGAttributePlug.child(
                    AssetNode::outputPartExtraAttributeData
                    );

            MString owner = mayaSGOwnerPlug.asString();

            if(owner == "detail")
            {
                MString mayaSG = mayaSGDataPlug.asString();

                MIntArray* components = NULL;

                if(hasMaterials.size())
                {
                    components = new MIntArray();
                    for(size_t i = 0; i < hasMaterials.size(); i++)
                    {
                        if(hasMaterials[i])
                        {
                            continue;
                        }

                        hasMaterials[i] = true;

                        components->append(i);
                    }
                }

                materialComponents.push_back(MaterialComponent(
                            Util::findNodeByName(mayaSG.asChar(),
                                MFn::kShadingEngine),
                            components));
            }
            else if(owner == "primitive")
            {
                MFnStringArrayData mayaSGData(mayaSGDataPlug.asMObject());
                MStringArray mayaSG = mayaSGData.array();

                std::map<const char*, MaterialComponent> materialComponentsMap;

                if(hasMaterials.size() < mayaSG.length())
                {
                    hasMaterials.resize(mayaSG.length());
                }

                // gather shading group name
                for(size_t i = 0; i < mayaSG.length(); i++)
                {
                    if(hasMaterials[i])
                    {
                        continue;
                    }

                    const char* sgName = mayaSG[i].asChar();
                    if(!sgName || !sgName[0])
                    {
                        continue;
                    }

                    hasMaterials[i] = true;

                    std::pair<std::map<const char*, MaterialComponent>::iterator, bool> r
                        = materialComponentsMap.insert(
                                std::pair<const char*, MaterialComponent>(
                                    sgName, MaterialComponent()));
                    // if first time seeing the shading group
                    if(r.second)
                    {
                        MaterialComponent &materialComponent = r.first->second;
                        materialComponent.first = Util::findNodeByName(sgName,
                                MFn::kShadingEngine);
                        materialComponent.second = new MIntArray();
                    }

                    r.first->second.second->append(i);
                }

                for(std::map<const char*, MaterialComponent>::iterator iter
                        = materialComponentsMap.begin();
                        iter != materialComponentsMap.end(); iter++)
                {
                    materialComponents.push_back(iter->second);
                }
            }
        }

        // faces with no materials
        MObject defaultMaterialObj = Util::findNodeByName("initialShadingGroup",
                MFn::kShadingEngine);
        if(hasMaterials.size())
        {
            MIntArray* components = new MIntArray();
            for(size_t i = 0; i < hasMaterials.size(); i++)
            {
                if(hasMaterials[i])
                {
                    continue;
                }

                components->append(i);
            }

            if(components->length())
            {
                materialComponents.push_back(MaterialComponent(
                            defaultMaterialObj,
                            components));
            }
            else
            {
                delete components;
            }
        }

        // if no materials was assigned at all
        if(!materialComponents.size())
        {
            materialComponents.push_back(MaterialComponent(
                        defaultMaterialObj, static_cast<MIntArray*>(NULL)));
        }

        // assign materials
        for(std::vector<MaterialComponent>::iterator iter
                = materialComponents.begin();
                iter != materialComponents.end(); iter++)
        {
            MObject materialObj = iter->first;
            MIntArray *components = iter->second;

            if(materialObj.isNull())
            {
                materialObj = defaultMaterialObj;
            }

            MObject componentObj;
            //do per-face assignment if components is null
            if(components)
            {
                MFnSingleIndexedComponent componentFn;
                componentObj = componentFn.create(MFn::kMeshPolygonComponent);
                componentFn.addElements(*components);
            }

            MDagPath partMeshDag;
            partMeshFn.getPath(partMeshDag);

            MString assignCommand = "sets -e -forceElement "
                + MFnDependencyNode(materialObj).name();

            MSelectionList selectionList;
            MStringArray selectionStrings;
            selectionList.add(partMeshDag, componentObj);
            selectionList.getSelectionStrings(selectionStrings);
            for(unsigned int i = 0; i < selectionStrings.length(); i++)
            {
                assignCommand += " " + selectionStrings[i];
            }

            myDagModifier.commandToExecute(assignCommand);

            delete components;
        }
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputCurves(
        MPlug curvesPlug,
        const MString &partName,
        bool isBezier
        )
{
    MStatus status;

    int numCurves = curvesPlug.evaluateNumElements();
    for(int i=0; i<numCurves; i++)
    {
        MPlug curvePlug = curvesPlug[i];

        MString curveTransformName;
        MString curveShapeName;
        curveTransformName.format("curve^1s", MString() + (i + 1));
        curveShapeName.format("curveShape^1s", MString() + (i + 1));

        // create curve transform
        MObject curveTransform = myDagModifier.createNode(
                "transform",
                myPartTransform,
                &status
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // rename curve transform
        status = myDagModifier.renameNode(curveTransform, curveTransformName);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // create curve shape
        MObject curveShape = myDagModifier.createNode(
                isBezier ? "bezierCurve" : "nurbsCurve",
                curveTransform,
                &status
                );
        CHECK_MSTATUS(status);

        // rename curve shape
        status = myDagModifier.renameNode(curveShape, curveShapeName);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDependencyNode curveShapeFn(curveShape, &status);
        MPlug dstPlug = curveShapeFn.findPlug("create");
        CHECK_MSTATUS(status);

        myDagModifier.connect(curvePlug, dstPlug);
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputParticle(
        const MString &partName,
        const MPlug &particlePlug
        )
{
    MStatus status;

    // create nParticle
    MObject particleShapeObj = myDagModifier.createNode(
            "nParticle",
            myPartTransform,
            &status
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename particle
    status = myDagModifier.renameNode(particleShapeObj, partName + "Shape");
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    myDagModifier.commandToExecute(
            "setupNParticleConnections " +
            MFnDagNode(myPartTransform).fullPathName()
            );

    MPlug srcPlug;
    MPlug dstPlug;

    MFnDependencyNode particleShapeFn(particleShapeObj);

    // connect nParticleShape attributes
    srcPlug = particlePlug.child(AssetNode::outputPartParticleCurrentTime);
    dstPlug = particleShapeFn.findPlug("currentTime");
    status = myDagModifier.connect(srcPlug, dstPlug);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    srcPlug = particlePlug.child(AssetNode::outputPartParticlePositions);
    dstPlug = particleShapeFn.findPlug("positions");
    status = myDagModifier.connect(srcPlug, dstPlug);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    srcPlug = particlePlug.child(AssetNode::outputPartParticleArrayData);
    dstPlug = particleShapeFn.findPlug("cacheArrayData");
    status = myDagModifier.connect(srcPlug, dstPlug);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set particleRenderType to points
    status = myDagModifier.newPlugValueInt(
            particleShapeFn.findPlug("particleRenderType"),
            3);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set playFromCache to true
    status = myDagModifier.newPlugValueBool(
            particleShapeFn.findPlug("playFromCache"),
            true);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    createOutputExtraAttributes(particleShapeObj);

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputInstancer(
        const MString &partName,
        const MPlug &instancerPlug
        )
{
    MStatus status;

    MFnDependencyNode assetNodeFn(instancerPlug.node());

    MPlug useInstancerNodePlug = assetNodeFn.findPlug(AssetNode::useInstancerNode);
    bool useInstancerNode = useInstancerNodePlug.asBool();

    MPlug partsPlug = instancerPlug.child(AssetNode::outputPartInstancerParts);
    MFnIntArrayData partsData(partsPlug.asMObject());
    MIntArray parts = partsData.array();

    // Particle instancer
    if(useInstancerNode)
    {
        myPartShapes.setLength(parts.length());

        for(unsigned int i = 0; i < parts.length(); i++)
        {
            // create the instancer node
            myPartShapes[i] = myDagModifier.createNode("instancer", myPartTransform, &status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            MFnDependencyNode instancerFn(myPartShapes[i], &status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // set the rotation units to radians
            status = myDagModifier.newPlugValueInt(instancerFn.findPlug("rotationAngleUnits"), 1);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // connect instancer directly to assetnode
            status = myDagModifier.connect(
                instancerPlug.child(AssetNode::outputPartInstancerArrayData),
                instancerFn.findPlug("inputPoints"));

            createOutputExtraAttributes(
                    myPartShapes[i]
                    );
        }
    }
    else
    {
        MPlug transformPlug = instancerPlug.child(AssetNode::outputPartInstancerTransform);

        unsigned int numTransforms = transformPlug.numElements();

        myPartShapes.setLength(numTransforms);

        for(unsigned int i = 0; i < numTransforms; i++)
        {
            MPlug transformElemPlug = transformPlug[i];
            MPlug translatePlug = transformElemPlug.child(AssetNode::outputPartInstancerTranslate);
            MPlug rotatePlug = transformElemPlug.child(AssetNode::outputPartInstancerRotate);
            MPlug scalePlug = transformElemPlug.child(AssetNode::outputPartInstancerScale);

            translatePlug.selectAncestorLogicalIndex(i, AssetNode::outputPartInstancerTransform);
            rotatePlug.selectAncestorLogicalIndex(i, AssetNode::outputPartInstancerTransform);
            scalePlug.selectAncestorLogicalIndex(i, AssetNode::outputPartInstancerTransform);

            myPartShapes[i] = myDagModifier.createNode("transform", myPartTransform, &status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            status = myDagModifier.renameNode(myPartShapes[i], partName + "_" + i);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            MFnDependencyNode instanceTransform(myPartShapes[i]);

            // connect translate
            status = myDagModifier.connect(
                    translatePlug,
                    instanceTransform.findPlug("translate"));
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // connect rotate
            status = myDagModifier.connect(
                    rotatePlug,
                    instanceTransform.findPlug("rotate"));
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // connect scale
            status = myDagModifier.connect(
                    scalePlug,
                    instanceTransform.findPlug("scale"));
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputInstancerPost(
        const MPlug &instancerPlug,
        SyncOutputGeometryPart *const *syncParts
        )
{
    MStatus status;

    MFnDependencyNode assetNodeFn(instancerPlug.node());

    MPlug useInstancerNodePlug = assetNodeFn.findPlug(AssetNode::useInstancerNode);
    bool useInstancerNode = useInstancerNodePlug.asBool();

    MPlug partsPlug = instancerPlug.child(AssetNode::outputPartInstancerParts);
    MFnIntArrayData partsData(partsPlug.asMObject());
    MIntArray parts = partsData.array();

    // Particle instancer
    if(useInstancerNode)
    {
        for(unsigned int i = 0; i < parts.length(); i++)
        {
            MFnDependencyNode instancerFn(myPartShapes[i], &status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            MPlug inputHierarchyPlug = instancerFn.findPlug("inputHierarchy");

            const MObject &instancedPartTransform = syncParts[parts[i]]->partTransform();
            MFnDependencyNode instancedPartTransformFn(instancedPartTransform);

            // set objectTransform hidden
            status = myDagModifier.newPlugValueInt(
                    instancedPartTransformFn.findPlug("visibility"),
                    0);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // connect inputHierarchy
            status = myDagModifier.connect(
                    instancedPartTransformFn.findPlug("matrix"),
                    inputHierarchyPlug.elementByLogicalIndex(0));
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }
    }
    else
    {
        MPlug transformPlug = instancerPlug.child(AssetNode::outputPartInstancerTransform);

        unsigned int numTransforms = transformPlug.numElements();

        for(unsigned int i = 0; i < parts.length(); i++)
        {
            SyncOutputGeometryPart* const &syncPart = syncParts[parts[i]];

            const MObject &instancedPartTransform = syncPart->partTransform();
            MFnDagNode instancedPartTransformFn(instancedPartTransform);

            for(unsigned int j = 0; j < numTransforms; j++)
            {
                MFnDagNode instanceTransformFn(myPartShapes[j]);

                MString command;

                // When we ever see the part for the very first time, we want to
                // reparent it from the default location. After that, we can
                // simply instance it.
                if(!syncPart->isInstanced())
                {
                    syncPart->setIsInstanced(true);

                    command.format("parent -relative ^1s ^2s;",
                            instancedPartTransformFn.fullPathName(),
                            instanceTransformFn.fullPathName());
                }
                else
                {
                    command.format("parent -relative -addObject ^1s ^2s;",
                            instancedPartTransformFn.fullPathName(),
                            instanceTransformFn.fullPathName());
                }

                status = myDagModifier.commandToExecute(command);
                CHECK_MSTATUS_AND_RETURN_IT(status);

                status = myDagModifier.doIt();
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
        }
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputExtraAttributes(
        const MObject &dstNode,
        MPlug* mayaSGAttributePlug
        )
{
    MStatus status;

    MFnDependencyNode dstNodeFn(dstNode);

    MPlug extraAttributesPlug = myOutputPlug.child(
            AssetNode::outputPartExtraAttributes
            );

    bool isParticle = dstNode.hasFn(MFn::kParticle);

    int numExtraAttributes = extraAttributesPlug.numElements();
    for(int i = 0; i < numExtraAttributes; i++)
    {
        MPlug extraAttributePlug = extraAttributesPlug[i];
        MPlug extraAttributeNamePlug = extraAttributePlug.child(
                AssetNode::outputPartExtraAttributeName
                );
        MPlug extraAttributeOwnerPlug = extraAttributePlug.child(
                AssetNode::outputPartExtraAttributeOwner
                );
        MPlug extraAttributeDataTypePlug = extraAttributePlug.child(
                AssetNode::outputPartExtraAttributeDataType
                );
        MPlug extraAttributeTuplePlug = extraAttributePlug.child(
                AssetNode::outputPartExtraAttributeTuple
                );
        MPlug extraAttributeDataPlug = extraAttributePlug.child(
                AssetNode::outputPartExtraAttributeData
                );

        MString dstAttributeName = extraAttributeNamePlug.asString();

        MString owner = extraAttributeOwnerPlug.asString();
        MString dataType = extraAttributeDataTypePlug.asString();
        int tuple = extraAttributeTuplePlug.asInt();

        bool isPerParticleAttribute = false;
        if(isParticle)
        {
            // Not every attribute type can be supported on a particle node.
            bool canSupportAttribute = false;

            if(owner == "detail")
            {
                // Make sure the data is not array type.
                if((dataType == "float"
                            || dataType == "double"
                            || dataType == "int"
                            || dataType == "long")
                        && tuple <= 3)
                {
                    canSupportAttribute = true;
                }
                else if(dataType == "string"
                        && tuple == 1)
                {
                    canSupportAttribute = true;
                }
            }
            else if(owner == "primitive")
            {
                // Shouldn't be possible.
            }
            else if(owner == "point")
            {
                // Particles don't support int and string attributes. Also,
                // make sure the one particle maps to one element of the array.
                if((dataType == "double" && tuple == 1)
                        || (dataType == "double" && tuple == 3))
                {
                    isPerParticleAttribute = true;
                    canSupportAttribute = true;

                    // This should match the list in
                    // OutputGeometryPart::computeParticle().
                    if(dstAttributeName == "Cd")
                    {
                        dstAttributeName = "rgbPP";
                    }
                    else if(dstAttributeName == "Alpha")
                    {
                        dstAttributeName = "opacityPP";
                    }
                    else if(dstAttributeName == "pscale")
                    {
                        dstAttributeName = "radiusPP";
                    }
                }
            }
            else if(owner == "vertex")
            {
                // Shouldn't be possible.
            }

            if(!canSupportAttribute)
            {
                DISPLAY_WARNING(
                        "The particle node:\n"
                        "    ^1s\n"
                        "cannot support the attribute:\n"
                        "    owner: ^2s, name: ^3s, type: ^4s, tuple: ^5s\n"
                        "from the plug:"
                        "    ^6s\n",
                        dstNodeFn.name(),
                        owner, dstAttributeName, dataType, MString() + tuple,
                        extraAttributeDataPlug.name()
                        );
                continue;
            }
        }

        if((owner == "primitive" || owner == "detail")
                && dstAttributeName == "maya_shading_group")
        {
            if(mayaSGAttributePlug)
            {
                *mayaSGAttributePlug = extraAttributePlug;
            }
            continue;
        }

        // Some special cases to remap certain attributes.
        if(owner != "detail" && dstAttributeName == "v")
        {
            // "v" means velocity in Houdini. However, in Maya, "v" happens to
            // be the short name of the "visibility" attribute. Instead of
            // connecting velocity to visibility, which would be very confusing
            // for the user, we rename "v" to "velocity".
            dstAttributeName = "velocity";
        }

        MObject dstAttribute = dstNodeFn.attribute(dstAttributeName);

        if(owner == "detail")
        {
            // Use existing attribute if it exists.
        }
        else
        {
            if(!dstAttribute.isNull())
            {
                // If the attribute already exists, add a prefix. This avoids
                // name conflict when outputting attributes with the same name.
                dstAttributeName = owner + "_" + dstAttributeName;
                dstAttribute = MObject::kNullObj;
            }
        }

        // If it doesn't exist, create it.
        if(dstAttribute.isNull())
        {
            MDataHandle dataHandle = extraAttributeDataPlug.asMDataHandle();

            dstAttribute = createAttributeFromDataHandle(
                    dstAttributeName,
                    dataHandle
                    );

            if(!dstAttribute.isNull())
            {
                CHECK_MSTATUS(myDagModifier.addAttribute(
                            dstNode,
                            dstAttribute
                            ));
                CHECK_MSTATUS(myDagModifier.doIt());
            }
        }

        // If there's still no attribute, skip it.
        if(dstAttribute.isNull())
        {
            DISPLAY_WARNING(
                    "Cannot create attribute on the node:\n"
                    "    ^1s\n"
                    "for the attribute:\n"
                    "    owner: ^2s, name: ^3s, type: ^4s, tuple: ^5s\n"
                    "from the plug:\n"
                    "    ^6s\n",
                    dstNodeFn.name(),
                    owner, dstAttributeName, dataType, MString() + tuple,
                    extraAttributeDataPlug.name()
                    );

            continue;
        }

        // Per-particle attributes does not need to be, and cannot be,
        // connected.
        if(isParticle && isPerParticleAttribute)
        {
            continue;
        }

        MPlug dstPlug(dstNode, dstAttribute);
        CHECK_MSTATUS(myDagModifier.connect(
                    extraAttributeDataPlug,
                    dstPlug
                    ));
        status = (myDagModifier.doIt());
        if(!status)
        {
            DISPLAY_WARNING(
                    "Failed to connect:\n"
                    "    ^1s\n"
                    "    to:\n"
                    "    ^2s\n",
                    extraAttributeDataPlug.name(),
                    dstPlug.name()
                    );
            CHECK_MSTATUS(status);
        }
    }

    return status;
}

MStatus
SyncOutputGeometryPart::createOutputGroups(
        const MObject &dstNode
        )
{
    MStatus status;

    MDagPath dagPath;
    MDagPath::getAPathTo(dstNode, dagPath);

    // This is needed to make sure that the geometry connection are setup.
    // Otherwise, set components can't be assigned.
    status = myDagModifier.doIt();
    CHECK_MSTATUS(status);

    MPlug groupsPlug = myOutputPlug.child(
            AssetNode::outputPartGroups
            );

    int numGroups = groupsPlug.numElements();
    for(int i = 0; i < numGroups; i++)
    {
        MPlug groupPlug = groupsPlug[i];

        MPlug groupNamePlug = groupPlug.child(
                AssetNode::outputPartGroupName
                );
        MPlug groupTypePlug = groupPlug.child(
                AssetNode::outputPartGroupType
                );
        MPlug groupMembersPlug = groupPlug.child(
                AssetNode::outputPartGroupMembers
                );

        MString setName = groupNamePlug.asString();

        MFn::Type componentType = (MFn::Type) groupTypePlug.asInt();

        MObject setObj = Util::findNodeByName(setName, MFn::kSet);
        if(setObj.isNull())
        {
            status = Util::createNodeByModifierCommand(
                    myDagModifier,
                    "select -noExpand `sets "
                    "-name \"" + setName + "\"`",
                    setObj
                    );
            CHECK_MSTATUS(status);
        }

        MObject groupMembersObj = groupMembersPlug.asMObject();
        MFnIntArrayData groupMembersDataFn(groupMembersObj, &status);
        CHECK_MSTATUS(status);

        MFnSingleIndexedComponent componentFn;
        MObject componentObj = componentFn.create(componentType);

        MIntArray componentArray = groupMembersDataFn.array();
        componentFn.addElements(componentArray);

        MString assignCommand = "sets -e -forceElement "
            + MFnDependencyNode(setObj).name();

        MSelectionList selectionList;
        MStringArray selectionStrings;
        selectionList.add(dagPath, componentObj);
        selectionList.getSelectionStrings(selectionStrings);
        for(unsigned int i = 0; i < selectionStrings.length(); i++)
        {
            assignCommand += " " + selectionStrings[i];
        }

        myDagModifier.commandToExecute(assignCommand);
    }

    return MStatus::kSuccess;
}
