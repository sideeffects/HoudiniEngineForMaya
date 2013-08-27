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
#include "AssetNode.h"
#include "util.h"

#include "AssetSyncOutputGeoPart.h"
#include "AssetSyncOutputInstance.h"

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
        myAssetNode = dg.createNode("houdiniAsset", &stat);
        Util::checkMayaStatus(stat);
        MPlug plug( myAssetNode, AssetNode::fileNameAttr);
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
	dest = MPlug( myAssetNode, AssetNode::timeInput );
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
    MStatus stat;
    try
    {
        // Create the objs/meshes
        MPlug objectsPlug( myAssetNode, AssetNode::objects );
        int objCount = objectsPlug.evaluateNumElements(&stat);
        Util::checkMayaStatus(stat);

	MString title = "Houdini";
	MString status = "Composing Objects...";
	Util::showProgressWindow( title, status, 0 );
        for (int i=0; i<objCount; i++)
        {
	    Util::updateProgressWindow( status, (int) ( (float)i*100.0f / (float) objCount) );
            MPlug elemPlug = objectsPlug[i];
            createAssetSyncOutputGeoPart(elemPlug, myAssetTransform);
        }
	Util::hideProgressWindow();

        // Instancers
        MPlug instancersPlug(myAssetNode, AssetNode::instancers);
        int instCount = instancersPlug.numElements(&stat);
        Util::checkMayaStatus(stat);

	status = "Instancing Objects...";
	Util::showProgressWindow( title, status, 0 );
        for (int i=0; i<instCount; i++)
        {
	    Util::updateProgressWindow( status, (int) ( (float)i*100.0f / (float) instCount) );
            MPlug elemPlug = instancersPlug[i];
            createAssetSyncOutputInstance(elemPlug, myAssetTransform);
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


AssetSyncOutputGeoPart*
AssetManager::getObjectGroup(MPlug& plug)
{
    try
    {
        int index = plug.logicalIndex();
        // This throws an exception if index is out of bounds
        AssetSyncOutputGeoPart* ret = myAssetSyncOutputGeoParts.at(index);
        return ret;
    }
    catch (...)
    {
        return NULL;
    }
}


AssetSyncOutputInstance*
AssetManager::getInstGroup(MPlug& plug)
{
    try
    {
        int index = plug.logicalIndex();
        // This throws an exception if index is out of bounds
        AssetSyncOutputInstance* ret = myAssetSyncOutputInstances.at(index);
        return ret;
    }
    catch (...)
    {
        return NULL;
    }
}


MStatus
AssetManager::createAssetSyncOutputGeoPart(MPlug& plug, MObject& assetTransform)
{
    AssetSyncOutputGeoPart* objGroup = getObjectGroup(plug);
    if (objGroup == NULL)
    {
        objGroup = new AssetSyncOutputGeoPart(plug, assetTransform);
        myAssetSyncOutputGeoParts.push_back(objGroup);
    }

    return objGroup->doIt();
}


MStatus
AssetManager::createAssetSyncOutputInstance(MPlug& plug, MObject& assetTransform)
{
    AssetSyncOutputInstance* instGroup = getInstGroup(plug);
    if (instGroup == NULL)
    {
        instGroup = new AssetSyncOutputInstance(plug, assetTransform);
        myAssetSyncOutputInstances.push_back(instGroup);
    }

    return instGroup->doIt();
}
