#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFileObject.h>
#include <maya/MSelectionList.h>
#include <maya/MPlugArray.h>
#include <maya/MStatus.h>
#include <maya/MGlobal.h>

#include "AssetManager.h"
#include "common.h"
#include "util.h"


std::vector<AssetManager*> AssetManager::myManagers;

AssetManager*
AssetManager::createManager(const MString& filePath)
{
    AssetManager* m = new AssetManager(filePath);
    myManagers.push_back(m);
    m->init();
    return m;
}


AssetManager::AssetManager()
{
}


AssetManager::AssetManager(const MString& filePath)
    : myFilePath(filePath)
{
}


AssetManager::~AssetManager()
{
    // TODO: destroy stuff
}


MObject
AssetManager::getAssetNode()
{
    return myAssetNode;
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
        myAssetNode = dg.createNode("hAsset", &stat);
        Util::checkMayaStatus(stat);
        MPlug plug( myAssetNode, AssetNodeAttributes::fileNameAttr);
        stat = dg.newPlugValueString(plug, myFilePath);

        MFileObject file;
        file.setRawFullName( myFilePath );
        MString fileName = file.resolvedName();
        int dotIndex = fileName.rindexW('.');
        MString assetName = fileName.substringW(0, dotIndex-1);
        assetName += "_asset";
        MFnDependencyNode fnDN( myAssetNode );
        fnDN.setName(assetName, &stat);
        Util::checkMayaStatus(stat);

        myAssetTransform = fnDag.create("transform", assetName + "_transform", MObject::kNullObj, &stat);
        Util::checkMayaStatus(stat);

        MString t = "time1";
	MObject timeNode = Util::findNodeByName(t);
	src = MFnDependencyNode(timeNode).findPlug("outTime");
	dest = MPlug( myAssetNode, AssetNodeAttributes::timeInput );
	dg.connect(src, dest);

        // DGModifier do it
        stat = dg.doIt();
        Util::checkMayaStatus(stat);

        plug.setLocked(true);

        stat = update();
        Util::checkMayaStatus(stat);

        // Select the newly created asset
        stat = MGlobal::select( myAssetNode, MGlobal::kReplaceList );
        Util::checkMayaStatus(stat);

    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
    }

}


MStatus
AssetManager::update()
{
    cerr << "AssetManager::update() called" << endl;
    MStatus stat;
    try
    {
        // Create the objs/meshes
        MPlug objectsPlug( myAssetNode, AssetNodeAttributes::objects );
        int objCount = objectsPlug.evaluateNumElements(&stat);
        Util::checkMayaStatus(stat);

	MString title = "Houdini";
	MString status = "Composing Objects...";
	Util::showProgressWindow( title, status, 0 );
        for (int i=0; i<objCount; i++)
        {
	    Util::updateProgressWindow( status, (int) ( (float)i*100.0f / (float) objCount) );
            MPlug elemPlug = objectsPlug[i];
            createObjectNodeGroup(elemPlug, myAssetTransform);
        }
	Util::hideProgressWindow();

        // Instancers
        MPlug instancersPlug(myAssetNode, AssetNodeAttributes::instancers);
        int instCount = instancersPlug.numElements(&stat);
        Util::checkMayaStatus(stat);

	status = "Instancing Objects...";
	Util::showProgressWindow( title, status, 0 );
        for (int i=0; i<instCount; i++)
        {
	    Util::updateProgressWindow( status, (int) ( (float)i*100.0f / (float) instCount) );
            MPlug elemPlug = instancersPlug[i];
            createInstNodeGroup(elemPlug, myAssetTransform);
        }
	Util::hideProgressWindow();

        // TODO: delete extra node groups

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
        return MS::kFailure;
    }
}


ObjectNodeGroup*
AssetManager::getObjectGroup(MPlug& plug)
{
    try
    {
        int index = plug.logicalIndex();
        // This throws an exception if index is out of bounds
        ObjectNodeGroup* ret = myObjectNodeGroups.at(index);
        return ret;
    }
    catch (...)
    {
        return NULL;
    }
}


InstNodeGroup*
AssetManager::getInstGroup(MPlug& plug)
{
    try
    {
        int index = plug.logicalIndex();
        // This throws an exception if index is out of bounds
        InstNodeGroup* ret = myInstNodeGroups.at(index);
        return ret;
    }
    catch (...)
    {
        return NULL;
    }
}


MStatus
AssetManager::createObjectNodeGroup(MPlug& plug, MObject& assetTransform)
{
    ObjectNodeGroup* objGroup = getObjectGroup(plug);
    if (objGroup == NULL)
    {
        objGroup = new ObjectNodeGroup();
        objGroup->plug = plug;
        objGroup->assetTransform = assetTransform;
        myObjectNodeGroups.push_back(objGroup);
    }

    return objGroup->update();
}


MStatus
AssetManager::createInstNodeGroup(MPlug& plug, MObject& assetTransform)
{
    InstNodeGroup* instGroup = getInstGroup(plug);
    if (instGroup == NULL)
    {
        instGroup = new InstNodeGroup();
        instGroup->plug = plug;
        instGroup->assetTransform = assetTransform;
        myInstNodeGroups.push_back(instGroup);
    }

    return instGroup->update();
}


MStatus
ObjectNodeGroup::update()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
    return stat;
}

// Check if necessary nodes exist, and if not, will create them
MStatus
ObjectNodeGroup::updateNodes()
{
    MStatus stat;
    MFnDagNode fnDag;
    MDGModifier dg;
    MDagModifier dag;
    MString cmd, result;

    try
    {

        // Check to see if the object transform has been created, if so,
        // then add the part transform under it, if not, then create the
        // object transform then add the part transform
        MString partName = plug.child(AssetNodeAttributes::objectName).asString();

        int usIndex = partName.rindexW('_');
        MString objName = partName.substringW(0, usIndex-1);

        MString objFullPath = "|" + MFnDependencyNode(assetTransform).name() +
                              "|" + objName;
        cerr << objFullPath << endl;
        cmd = "objExists " + objFullPath;
        int exists = 0;
        stat = MGlobal::executeCommand(cmd, exists);
        Util::checkMayaStatus(stat);
        cerr << exists << endl;

        MObject objNode;
        if (exists)
        {
            objNode = Util::findNodeByName(objFullPath);
        }
        else
        {
            objNode = fnDag.create("transform", objName, assetTransform, &stat);
            Util::checkMayaStatus(stat);
        }
        objectTransform = objNode;

        if (partTransform.isNull())
        {
            cerr << "createing mesh" << endl;
            // Creates a mesh node and its parent, returns the parent to me.
            partTransform = fnDag.create("mesh", partName, MObject::kNullObj, &stat);
            Util::checkMayaStatus(stat);
            meshNode = MFnDagNode(partTransform).child(0, &stat);
            Util::checkMayaStatus(stat);
            stat = dag.reparentNode(partTransform, objectTransform);
            Util::checkMayaStatus(stat);
            stat = dag.doIt();
            Util::checkMayaStatus(stat);
        }

        // Materials
        MPlug materialPlug = plug.child(AssetNodeAttributes::material);
        if (materialNode.isNull())
        {
            cerr << "createing mateiral" << endl;
            cmd = "shadingNode -asShader phong";
            result = Util::executeCommand(cmd); // phong node
            materialNode = Util::findNodeByName(result);
        }

        if (seNode.isNull())
        {
            cerr << "createing se: " << MFnDagNode(partTransform).fullPathName() << endl;
            stat = MGlobal::selectByName(MFnDagNode(partTransform).fullPathName(), MGlobal::kReplaceList);
            Util::checkMayaStatus(stat);

            cmd = "hyperShade -assign " + MFnDependencyNode(materialNode).name();
	    stat = MGlobal::executeCommand(cmd);
            Util::checkMayaStatus(stat);

            MPlugArray connectedPlugs;
            bool hasConnections = MFnDependencyNode(materialNode).findPlug("outColor").connectedTo(
                    connectedPlugs, false, true, &stat);
            Util::checkMayaStatus(stat);
            if (hasConnections)
            {
                seNode = connectedPlugs[0].node();
                cerr << "seNode: " << MFnDependencyNode(seNode).name() << endl;
            }
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
                }
            }
        }
        cerr << "check" << endl;
        stat = dg.doIt();
        Util::checkMayaStatus(stat);

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
        return MS::kFailure;
    }
}

//Once you know all the nodes exist, then make sure all proper connections exist
MStatus
ObjectNodeGroup::updateConnections()
{
    MDGModifier dg;
    MPlug src, dest;
    MStatus stat;

    try
    {
        // Transform
        MPlug transformPlug = plug.child(AssetNodeAttributes::transform);

        src = transformPlug.child(AssetNodeAttributes::translateAttr);
        cerr << src.name() << endl;
        dest = MFnDependencyNode(partTransform).findPlug("translate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNodeAttributes::rotateAttr);
        dest = MFnDependencyNode(partTransform).findPlug("rotate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNodeAttributes::scaleAttr);
        dest = MFnDependencyNode(partTransform).findPlug("scale", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Mesh
        src = plug.child(AssetNodeAttributes::mesh);
        dest = MFnDependencyNode(meshNode).findPlug("inMesh", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Materials
        MPlug materialPlug = plug.child(AssetNodeAttributes::material);
        bool matExists = materialPlug.child(AssetNodeAttributes::materialExists).asBool();
        if (matExists)
        {
            src = materialPlug.child(AssetNodeAttributes::specularAttr);
            dest = MFnDependencyNode(materialNode).findPlug("specularColor");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            src = materialPlug.child(AssetNodeAttributes::alphaAttr);
            dest = MFnDependencyNode(materialNode).findPlug("transparency");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            MString texturePath = materialPlug.child(AssetNodeAttributes::texturePath).asString();
            cerr << "texturePath: " << texturePath << endl;
            if (texturePath == "")
            {
                src = materialPlug.child(AssetNodeAttributes::diffuseAttr);
                dest = MFnDependencyNode(materialNode).findPlug("color");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = materialPlug.child(AssetNodeAttributes::ambientAttr);
                dest = MFnDependencyNode(materialNode).findPlug("ambientColor");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);
            }
            else
            {
                src = materialPlug.child(AssetNodeAttributes::texturePath);
                dest = MFnDependencyNode(fileNode).findPlug("fileTextureName");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = MFnDependencyNode(fileNode).findPlug("outColor");
                dest = MFnDependencyNode(materialNode).findPlug("color");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);
            }
        }
       Util::checkMayaStatus(stat);

        stat = dg.doIt();
        Util::checkMayaStatus(stat);

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
        return MS::kFailure;
    }

}


MStatus
InstNodeGroup::update()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
    return stat;
}


MStatus
InstNodeGroup::updateNodes()
{
    MStatus stat;
    MDagModifier dag;
    MFnDependencyNode fnDN;

    try
    {
        // Create the instancer node
        if (instancerNode.isNull())
        {
            instancerNode = fnDN.create("instancer", "instancer", &stat);
            Util::checkMayaStatus(stat);
        }

        // Set the rotation units to radians
        stat = dag.newPlugValueInt(MFnDependencyNode(instancerNode).findPlug("rau"), 1);
        Util::checkMayaStatus(stat);

        stat = dag.reparentNode(instancerNode, assetTransform);
        Util::checkMayaStatus(stat);

        stat = dag.doIt();
        Util::checkMayaStatus(stat);

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
        return MS::kFailure;
    }

}


MStatus
InstNodeGroup::updateConnections()
{
    MStatus stat;
    MPlug src, dest;
    MDGModifier dg;

    try
    {
        // Connect the input points
        src = plug.child(AssetNodeAttributes::instancerData);
        dest = MFnDependencyNode(instancerNode).findPlug("inputPoints");
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Connect the instanced objects
        MPlug instancedNames = plug.child(AssetNodeAttributes::instancedObjectNames);
        int ionCount = instancedNames.numElements();
        for (int i=0; i<ionCount; i++)
        {
            MString name = "|" + MFnDependencyNode(assetTransform).name() +
                "|" + instancedNames[i].asString();

            src = MFnDependencyNode(Util::findNodeByName(name)).findPlug("matrix");
            dest = MFnDependencyNode(instancerNode).findPlug("inputHierarchy").elementByLogicalIndex(i);
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            MString cmd = "hide " + name;

	    stat = MGlobal::executeCommand(cmd);
	    Util::checkMayaStatus(stat);
        }

        stat = dg.doIt();
        Util::checkMayaStatus(stat);

        return MS::kSuccess;
    }
    catch (MayaError& e)
    {
        cerr << e.what() << endl;
        return MS::kFailure;
    }
}
