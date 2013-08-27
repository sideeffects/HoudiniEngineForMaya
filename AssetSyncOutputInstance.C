#include "AssetSyncOutputInstance.h"

#include <maya/MDagModifier.h>
#include <maya/MGlobal.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncOutputInstance::AssetSyncOutputInstance(
	const MPlug &outputPlug,
	const MObject &assetTransform
	) :
    myOutputPlug(outputPlug),
    myAssetTransform(assetTransform)
{
}

AssetSyncOutputInstance::~AssetSyncOutputInstance()
{
}

MStatus
AssetSyncOutputInstance::doIt()
{
    MStatus stat;

    stat = updateNodes();
    stat = updateConnections();
    return stat;
}

MStatus
AssetSyncOutputInstance::undoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputInstance::redoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputInstance::updateNodes()
{
    MStatus stat;
    MDagModifier dag;
    MFnDependencyNode fnDN;

    try
    {
        // Create the instancer node
        if (myInstancerNode.isNull())
        {
            myInstancerNode = fnDN.create("instancer", "instancer", &stat);
            Util::checkMayaStatus(stat);
        }

        // Set the rotation units to radians
        stat = dag.newPlugValueInt(MFnDependencyNode(myInstancerNode).findPlug("rau"), 1);
        Util::checkMayaStatus(stat);

        stat = dag.reparentNode(myInstancerNode, myAssetTransform);
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
AssetSyncOutputInstance::updateConnections()
{
    MStatus stat;
    MPlug src, dest;
    MDGModifier dg;

    try
    {
        // Connect the input points
        src = myOutputPlug.child(AssetNode::instancerData);
        dest = MFnDependencyNode(myInstancerNode).findPlug("inputPoints");
        stat = dg.connect(src, dest);
        Util::checkMayaStatus(stat);

        // Connect the instanced objects
        MPlug instancedNames = myOutputPlug.child(AssetNode::instancedObjectNames);
        int ionCount = instancedNames.numElements();
        for (int i=0; i<ionCount; i++)
        {
            MString name = "|" + MFnDependencyNode(myAssetTransform).name() +
                "|" + instancedNames[i].asString();

            src = MFnDependencyNode(Util::findNodeByName(name)).findPlug("matrix");
            dest = MFnDependencyNode(myInstancerNode).findPlug("inputHierarchy").elementByLogicalIndex(i);
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
