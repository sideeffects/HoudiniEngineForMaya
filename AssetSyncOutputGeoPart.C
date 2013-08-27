#include "AssetSyncOutputGeoPart.h"

#include <maya/MDagModifier.h>
#include <maya/MDGModifier.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "util.h"

MStatus
AssetSyncOutputGeoPart::update()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
    return stat;
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
	    MString objectName = plug.child(AssetNode::objectName).asString();
	    int separatorIndex = objectName.rindexW('/');
	    objName = objectName.substringW(0, separatorIndex-1);
	    partName = objectName.substringW(separatorIndex + 1, objectName.numChars() - 1);
	}

        MString objFullPath = "|" + MFnDependencyNode(assetTransform).name() +
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
            objNode = fnDag.create("transform", objName, assetTransform, &stat);
            Util::checkMayaStatus(stat);
        }
        objectTransform = objNode;

	if (partTransform.isNull())
	{
	    partTransform = dag.createNode("transform", objectTransform, &stat);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.renameNode(partTransform, partName);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    meshNode = dag.createNode("mesh", partTransform, &stat);
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.renameNode(meshNode, partName + "Shape");
	    CHECK_MSTATUS_AND_RETURN_IT(stat);

	    stat = dag.doIt();
	    CHECK_MSTATUS_AND_RETURN_IT(stat);
        }

        // Materials
        MPlug materialPlug = plug.child(AssetNode::material);
        if (materialNode.isNull())
        {
            cmd = "shadingNode -asShader phong";
	    stat = MGlobal::executeCommand(cmd, result); // phong node
            Util::checkMayaStatus(stat);
            materialNode = Util::findNodeByName(result);
        }

        if (seNode.isNull())
        {
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
            }
        }

        bool matExists = materialPlug.child(AssetNode::materialExists).asBool();
        if (matExists)
        {
            MString texturePath = materialPlug.child(AssetNode::texturePath).asString();
            if (texturePath != "")
            {
                if (fileNode.isNull())
                {
                    cmd = "shadingNode -asTexture file";
		    stat = MGlobal::executeCommand(cmd, result); // file node
		    Util::checkMayaStatus(stat);
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
        MPlug transformPlug = plug.child(AssetNode::transform);

        src = transformPlug.child(AssetNode::translateAttr);
        dest = MFnDependencyNode(partTransform).findPlug("translate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNode::rotateAttr);
        dest = MFnDependencyNode(partTransform).findPlug("rotate", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        src = transformPlug.child(AssetNode::scaleAttr);
        dest = MFnDependencyNode(partTransform).findPlug("scale", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Mesh
        src = plug.child(AssetNode::mesh);
        dest = MFnDependencyNode(meshNode).findPlug("inMesh", true);
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Materials
        MPlug materialPlug = plug.child(AssetNode::material);
        bool matExists = materialPlug.child(AssetNode::materialExists).asBool();
        if (matExists)
        {
            src = materialPlug.child(AssetNode::specularAttr);
            dest = MFnDependencyNode(materialNode).findPlug("specularColor");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            src = materialPlug.child(AssetNode::alphaAttr);
            dest = MFnDependencyNode(materialNode).findPlug("transparency");
            stat = dg.connect(src, dest);
            Util::checkMayaStatus(stat);

            MString texturePath = materialPlug.child(AssetNode::texturePath).asString();
            if (texturePath == "")
            {
                src = materialPlug.child(AssetNode::diffuseAttr);
                dest = MFnDependencyNode(materialNode).findPlug("color");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);

                src = materialPlug.child(AssetNode::ambientAttr);
                dest = MFnDependencyNode(materialNode).findPlug("ambientColor");
                stat = dg.connect(src, dest);
                Util::checkMayaStatus(stat);
            }
            else
            {
                src = materialPlug.child(AssetNode::texturePath);
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
