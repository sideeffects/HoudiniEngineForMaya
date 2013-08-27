#include "AssetSyncOutputGeoPart.h"

#include <maya/MDagModifier.h>
#include <maya/MDGModifier.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncOutputGeoPart::AssetSyncOutputGeoPart(
	const MPlug &outputPlug,
	const MObject &assetTransform
	) :
    myOutputPlug(outputPlug),
    myAssetTransform(assetTransform)
{
}

AssetSyncOutputGeoPart::~AssetSyncOutputGeoPart()
{
}

MStatus
AssetSyncOutputGeoPart::doIt()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
    return stat;
}

MStatus
AssetSyncOutputGeoPart::undoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputGeoPart::redoIt()
{
    return MStatus::kSuccess;
}

// Check if necessary nodes exist, and if not, will create them
MStatus
AssetSyncOutputGeoPart::updateNodes()
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
	MString objName;
	MString partName;
	{
	    MString objectName = myOutputPlug.child(AssetNode::objectName).asString();
	    int separatorIndex = objectName.rindexW('/');
	    objName = objectName.substringW(0, separatorIndex-1);
	    partName = objectName.substringW(separatorIndex + 1, objectName.numChars() - 1);
	}

        MString objFullPath = "|" + MFnDependencyNode(myAssetTransform).name() +
                              "|" + objName;
        cmd = "objExists " + objFullPath;
        int exists = 0;
        stat = MGlobal::executeCommand(cmd, exists);
        Util::checkMayaStatus(stat);

        MObject objNode;
        if (exists)
        {
            objNode = Util::findNodeByName(objFullPath);
        }
        else
        {
            objNode = fnDag.create("transform", objName, myAssetTransform, &stat);
            Util::checkMayaStatus(stat);
        }
        myObjectTransform = objNode;

	if (myPartTransform.isNull())
	{
	    myPartTransform = dag.createNode("transform", myObjectTransform, &stat);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.renameNode(myPartTransform, partName);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    myMeshNode = dag.createNode("mesh", myPartTransform, &stat);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.renameNode(myMeshNode, partName + "Shape");
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.doIt();
	    CHECK_MSTATUS_AND_RETURN_IT(stat);
        }

        // Materials
        MPlug materialPlug = myOutputPlug.child(AssetNode::material);
        if (myMaterialNode.isNull())
        {
            cmd = "shadingNode -asShader phong";
	    stat = MGlobal::executeCommand(cmd, result); // phong node
            Util::checkMayaStatus(stat);
            myMaterialNode = Util::findNodeByName(result);
        }

        if (mySeNode.isNull())
        {
            stat = MGlobal::selectByName(MFnDagNode(myPartTransform).fullPathName(), MGlobal::kReplaceList);
            Util::checkMayaStatus(stat);

            cmd = "hyperShade -assign " + MFnDependencyNode(myMaterialNode).name();
	    stat = MGlobal::executeCommand(cmd);
            Util::checkMayaStatus(stat);

            MPlugArray connectedPlugs;
            bool hasConnections = MFnDependencyNode(myMaterialNode).findPlug("outColor").connectedTo(
                    connectedPlugs, false, true, &stat);
            Util::checkMayaStatus(stat);
            if (hasConnections)
            {
                mySeNode = connectedPlugs[0].node();
            }
        }

        bool matExists = materialPlug.child(AssetNode::materialExists).asBool();
        if (matExists)
        {
            MString texturePath = materialPlug.child(AssetNode::texturePath).asString();
            if (texturePath != "")
            {
                if (myFileNode.isNull())
                {
                    cmd = "shadingNode -asTexture file";
		    stat = MGlobal::executeCommand(cmd, result); // file node
		    Util::checkMayaStatus(stat);
                    myFileNode = Util::findNodeByName(result);
                }
            }
            else
            {
                if (!myFileNode.isNull())
                {
                    stat = dg.deleteNode(myFileNode);
                    Util::checkMayaStatus(stat);
                }
            }
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

//Once you know all the nodes exist, then make sure all proper connections exist
MStatus
AssetSyncOutputGeoPart::updateConnections()
{
    MDGModifier dg;
    MPlug src, dest;
    MStatus stat;

    try
    {
        // Transform
        MPlug transformPlug = myOutputPlug.child(AssetNode::transform);

        src = transformPlug.child(AssetNode::translateAttr);
        dest = MFnDependencyNode(myPartTransform).findPlug("translate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNode::rotateAttr);
        dest = MFnDependencyNode(myPartTransform).findPlug("rotate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNode::scaleAttr);
        dest = MFnDependencyNode(myPartTransform).findPlug("scale", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Mesh
        src = myOutputPlug.child(AssetNode::mesh);
        dest = MFnDependencyNode(myMeshNode).findPlug("inMesh", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Materials
        MPlug materialPlug = myOutputPlug.child(AssetNode::material);
        bool matExists = materialPlug.child(AssetNode::materialExists).asBool();
        if (matExists)
        {
            src = materialPlug.child(AssetNode::specularAttr);
            dest = MFnDependencyNode(myMaterialNode).findPlug("specularColor");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            src = materialPlug.child(AssetNode::alphaAttr);
            dest = MFnDependencyNode(myMaterialNode).findPlug("transparency");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            MString texturePath = materialPlug.child(AssetNode::texturePath).asString();
            if (texturePath == "")
            {
                src = materialPlug.child(AssetNode::diffuseAttr);
                dest = MFnDependencyNode(myMaterialNode).findPlug("color");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = materialPlug.child(AssetNode::ambientAttr);
                dest = MFnDependencyNode(myMaterialNode).findPlug("ambientColor");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);
            }
            else
            {
                src = materialPlug.child(AssetNode::texturePath);
                dest = MFnDependencyNode(myFileNode).findPlug("fileTextureName");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = MFnDependencyNode(myFileNode).findPlug("outColor");
                dest = MFnDependencyNode(myMaterialNode).findPlug("color");
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
