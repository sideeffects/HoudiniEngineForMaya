#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFileObject.h>
#include <maya/MSelectionList.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>

#include "assetManager.h"
#include "common.h"
#include "util.h"


std::vector<AssetManager*> AssetManager::managers;

void
AssetManager::createManager(const MString& filePath)
{
    AssetManager* m = new AssetManager(filePath);
    managers.push_back(m);
    m->init();
}

AssetManager::AssetManager()
{
}


AssetManager::AssetManager(const MString& filePath)
    :filePath(filePath)
{
}


AssetManager::~AssetManager()
{
}


void
AssetManager::init()
{
    MDGModifier dg;
    MDagModifier dag;
    MFnDagNode fnDag;
    MStatus stat;
    MPlug src;
    MPlug dest;

    try
    {
        // Create the asset node
        assetNode = dg.createNode("hAsset", &stat);
        Util::checkMayaStatus(stat);
        MPlug plug(assetNode, AssetNodeAttributes::fileNameAttr);
        stat = dg.newPlugValueString(plug, filePath);

        MFileObject file;
        file.setRawFullName(filePath);
        MString fileName = file.resolvedName();
        int dotIndex = fileName.rindexW('.');
        MString assetName = fileName.substringW(0, dotIndex-1);
        assetName += "_asset";
        MFnDependencyNode fnDN(assetNode);
        fnDN.setName(assetName, &stat);
        Util::checkMayaStatus(stat);

        MObject topTransform = fnDag.create("transform", assetName + "_transform", MObject::kNullObj, &stat);
        Util::checkMayaStatus(stat);

        // DGModifier do it
        stat = dg.doIt();
        Util::checkMayaStatus(stat);

        plug.setLocked(true);

        // Create the objs/meshes
        MPlug objectsPlug(assetNode, AssetNodeAttributes::objects);
        int objCount = objectsPlug.evaluateNumElements(&stat);
        Util::checkMayaStatus(stat);

        for (int i=0; i<objCount; i++)
        {
            MPlug elemPlug = objectsPlug[i];
            MString name = elemPlug.child(AssetNodeAttributes::objectName).asString();
            MObject transformNode = fnDag.create("mesh", name, MObject::kNullObj, &stat);
            Util::checkMayaStatus(stat);

            partNodes.append(transformNode);
        }

        // Connect the objects
        for (int i=0; i<objCount; i++)
        {
            MPlug elemPlug = objectsPlug[i];

            // Transform
            MObject transformNode = partNodes[i];
            MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);

            src = transformPlug.child(AssetNodeAttributes::translateAttr);
            cerr << src.name() << endl;
            dest = MFnDependencyNode(transformNode).findPlug("translate", true);
            stat = dag.connect(src, dest);
            Util::checkMayaStatus(stat);

            src = transformPlug.child(AssetNodeAttributes::rotateAttr);
            dest = MFnDependencyNode(transformNode).findPlug("rotate", true);
            stat = dag.connect(src, dest);
            Util::checkMayaStatus(stat);

            src = transformPlug.child(AssetNodeAttributes::scaleAttr);
            dest = MFnDependencyNode(transformNode).findPlug("scale", true);
            stat = dag.connect(src, dest);
            Util::checkMayaStatus(stat);

            // Mesh
            MObject meshNode = MFnDagNode(transformNode).child(0, &stat);
            Util::checkMayaStatus(stat);
            src = elemPlug.child(AssetNodeAttributes::mesh);
            dest = MFnDependencyNode(meshNode).findPlug("inMesh", true);
            stat = dag.connect(src, dest);
            Util::checkMayaStatus(stat);

        }
        stat = dag.doIt();
        Util::checkMayaStatus(stat);


        // Parent nodes
        for (int i=0; i<objCount; i++)
        {
            MFnDependencyNode fnDN(partNodes[i]);
            MString nodeName = fnDN.name();

            int usIndex = nodeName.rindexW('_');
            MString objName = nodeName.substringW(0, usIndex-1);

            MObject objNode = Util::findNodeByName(objName);
            if (objNode.isNull())
            {
                objNode = fnDag.create("transform", objName, topTransform, &stat);
                Util::checkMayaStatus(stat);
                stat = dag.reparentNode(objNode, topTransform);
                Util::checkMayaStatus(stat);
            }

            stat = dag.reparentNode(partNodes[i], objNode);
            Util::checkMayaStatus(stat);

        }
        stat = dag.doIt();
        Util::checkMayaStatus(stat);


        // Create the materials
        for (int i=0; i<objCount; i++)
        {
            MPlug elemPlug = objectsPlug[i];
            MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);
            bool matExists = materialPlug.child(AssetNodeAttributes::materialExists).asBool();

            MString result;
            MString cmd;

            cmd = "shadingNode -asShader phong";
            result = Util::executeCommand(cmd); // phong node
            MObject phongNode = Util::findNodeByName(result);
            materialNodes.append(phongNode);

            stat = MGlobal::selectByName(MFnDependencyNode(partNodes[i]).name(), MGlobal::kReplaceList);
            Util::checkMayaStatus(stat);

            cmd = "hyperShade -assign " + result;
            cerr << cmd << endl;
            result = Util::executeCommand(cmd); // shadingEngine node
            MObject seNode = Util::findNodeByName(result);
            seNodes.append(seNode);

            if (matExists)
            {

                // connect attributes

                src = materialPlug.child(AssetNodeAttributes::specularAttr);
                dest = MFnDependencyNode(phongNode).findPlug("specularColor");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = materialPlug.child(AssetNodeAttributes::alphaAttr);
                dest = MFnDependencyNode(phongNode).findPlug("transparency");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                MString texturePath = materialPlug.child(AssetNodeAttributes::texturePath).asString();
                cerr << "texturePath: " << texturePath << endl;
                if (texturePath == "")
                {
                    src = materialPlug.child(AssetNodeAttributes::diffuseAttr);
                    dest = MFnDependencyNode(phongNode).findPlug("color");
                    stat = dg.connect(src, dest);
                    Util::checkMayaStatus(stat);

                    src = materialPlug.child(AssetNodeAttributes::ambientAttr);
                    dest = MFnDependencyNode(phongNode).findPlug("ambientColor");
                    stat = dg.connect(src, dest);
                    Util::checkMayaStatus(stat);
                }
                else
                {
                    cmd = "shadingNode -asTexture file";
                    cerr << texturePath << endl;
                    result = Util::executeCommand(cmd); // file node
                    MObject fileNode = Util::findNodeByName(result);
                    fileNodes.append(fileNode);

                    // connect attributes
                    src = materialPlug.child(AssetNodeAttributes::texturePath);
                    dest = MFnDependencyNode(fileNode).findPlug("fileTextureName");
                    stat = dg.connect(src, dest);
                    Util::checkMayaStatus(stat);

                    src = MFnDependencyNode(fileNode).findPlug("outColor");
                    dest = MFnDependencyNode(phongNode).findPlug("color");
                    stat = dg.connect(src, dest);
                    Util::checkMayaStatus(stat);
                }
            }
        }
        stat = dg.doIt();
        Util::checkMayaStatus(stat);


        // Create instancers
        MPlug instancersPlug(assetNode, AssetNodeAttributes::instancers);
        int instCount = instancersPlug.numElements(&stat);
        cerr << "instCount: " << instCount << endl;
        Util::checkMayaStatus(stat);

        for (int i=0; i<instCount; i++)
        {
            MPlug elemPlug = instancersPlug[i];

            MObject instancer = fnDN.create("instancer", "instancer", &stat);
            Util::checkMayaStatus(stat);
            instancerNodes.append(instancer);

            stat = dg.newPlugValueInt(MFnDependencyNode(instancer).findPlug("rau"), 1);
            Util::checkMayaStatus(stat);

            src = elemPlug.child(AssetNodeAttributes::instancerData);
            dest = MFnDependencyNode(instancer).findPlug("inputPoints");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            MPlug instancedNames = elemPlug.child(AssetNodeAttributes::instancedObjectNames);
            int ionCount = instancedNames.numElements();
            for (int j=0; j<ionCount; j++)
            {
                MString name = instancedNames[j].asString();

                src = MFnDependencyNode(Util::findNodeByName(name)).findPlug("matrix");
                dest = MFnDependencyNode(instancer).findPlug("inputHierarchy").elementByLogicalIndex(j);
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                MString cmd = "hide " + name;
                Util::executeCommand(cmd);
            }


            stat = dag.reparentNode(instancer, topTransform);
            Util::checkMayaStatus(stat);

        }
        stat = dg.doIt();
        Util::checkMayaStatus(stat);
        stat = dag.doIt();
        Util::checkMayaStatus(stat);

    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
    }

}


void
AssetManager::update()
{
}


ObjectNodeGroup*
AssetManager::getObjectGroup(MPlug& plug)
{
    try
    {
        int index = plug.logicalIndex();
        ObjectNodeGroup* ret = objectNodeGroups.at(index);
        return ret;
    }
    catch (...)
    {
        return NULL;
    }
}


MStatus
AssetManager::createObjectNodeGroup(MPlug& plug)
{
    ObjectNodeGroup* objGroup = getObjectGroup(plug);
    if (objGroup == NULL)
    {
        objGroup = new ObjectNodeGroup();
        objectNodeGroups.push_back(objGroup);
    }

    return objGroup->update();
}


MStatus
ObjectNodeGroup::update()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
}


MStatus
ObjectNodeGroup::updateNodes()
{
    MStatus stat;
    MFnDagNode fnDag;
    MDGModifier dg;
    MString cmd, result;

    try
    {
        if (partNode.isNull())
        {
            MString name = plug.child(AssetNodeAttributes::objectName).asString();
            // Creates a mesh node and its parent, returns the parent to me.
            partNode = fnDag.create("mesh", name, MObject::kNullObj, &stat);
            Util::checkMayaStatus(stat);
        }

        MPlug materialPlug = plug.child(AssetNodeAttributes::material);
        if (materialNode.isNull())
        {
            MString result;
            MString cmd;

            cmd = "shadingNode -asShader phong";
            result = Util::executeCommand(cmd); // phong node
            materialNode = Util::findNodeByName(result);
        }

        if (seNode.isNull())
        {
            stat = MGlobal::selectByName(MFnDependencyNode(partNode).name(), MGlobal::kReplaceList);
            Util::checkMayaStatus(stat);

            cmd = "hyperShade -assign " + result;
            result = Util::executeCommand(cmd); // shadingEngine node
            seNode = Util::findNodeByName(result);
        }

        bool matExists = materialPlug.child(AssetNodeAttributes::materialExists).asBool();
        if (matExists)
        {
            MString texturePath = materialPlug.child(AssetNodeAttributes::texturePath).asString();
            if (texturePath != "")
            {
                if (fileNode.isNull())
                {
                    cmd = "shadingNode -asTexture file";
                    result = Util::executeCommand(cmd); // file node
                    fileNode = Util::findNodeByName(result);
                }
            }
            else
            {
                if (!fileNode.isNull())
                {
                    stat = dg.deleteNode(fileNode);
                    Util::checkMayaStatus(stat);
                    stat = dg.doIt();
                    Util::checkMayaStatus(stat);
                }
            }
        }

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        return MS::kFailure;
    }
}


MStatus
ObjectNodeGroup::updateConnections()
{
    return MS::kSuccess;
}


MStatus
InstNodeGroup::update()
{
    return MS::kSuccess;
}
