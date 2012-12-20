#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>
#include <maya/MSceneMessage.h>

#include "assetNodeMonitor.h"
#include "util.h"
#include "common.h"


//=============================================================================
// Public
//=============================================================================
AssetNodeMonitor::AssetNodeMonitor(AssetManager* m)
    : myManager(m)
    , myNode(m->getAssetNode())
    , attrChangedCBId(0)
    , nodeDirtyCBId(0)
    , sceneNewCBId(0)
    , sceneOpenCBId(0)
{
}

AssetNodeMonitor::~AssetNodeMonitor() {};


MStatus
AssetNodeMonitor::watch()
{

    MStatus stat;
    try {
        attrChangedCBId = MNodeMessage::addAttributeChangedCallback(myNode,
                &attributeChangedCB, this, &stat);
        Util::checkMayaStatus(stat);

        sceneNewCBId = MSceneMessage::addCallback(MSceneMessage::kBeforeNew,
                &sceneOpenedCB, this, &stat);
        Util::checkMayaStatus(stat);

        sceneOpenCBId = MSceneMessage::addCallback(MSceneMessage::kBeforeOpen,
                &sceneOpenedCB, this, &stat);
        Util::checkMayaStatus(stat);

    }
    catch (MayaError& e)
    {
        return e.status();
    }
}


MStatus
AssetNodeMonitor::stop()
{
    MStatus stat;
    try
    {
        if (attrChangedCBId){
            stat = MMessage::removeCallback(attrChangedCBId);
            Util::checkMayaStatus(stat);
        }
        if (nodeDirtyCBId){
            stat = MMessage::removeCallback(nodeDirtyCBId);
            Util::checkMayaStatus(stat);
        }
        if (sceneNewCBId){
            stat = MMessage::removeCallback(sceneNewCBId);
            Util::checkMayaStatus(stat);
        }
        if (sceneOpenCBId){
            stat = MMessage::removeCallback(sceneOpenCBId);
            Util::checkMayaStatus(stat);
        }
    }
    catch (MayaError& e)
    {
        return e.status();
    }
}


AssetManager*
AssetNodeMonitor::getManager()
{
    return myManager;
}


//=============================================================================
// Private
//=============================================================================
void
AssetNodeMonitor::attachNodeDirtyCallback()
{
    MStatus stat;
    nodeDirtyCBId = MNodeMessage::addNodeDirtyPlugCallback(myNode, &nodeDirtyPlugCB, this, &stat);
    Util::checkMayaStatus(stat);
}


void
AssetNodeMonitor::detachNodeDirtyCallback()
{
    MStatus stat = MNodeMessage::removeCallback(nodeDirtyCBId);
    Util::checkMayaStatus(stat);
}


//=============================================================================
// Callbacks
//=============================================================================
void
AssetNodeMonitor::attributeChangedCB(MNodeMessage::AttributeMessage msg, MPlug& plug,
        MPlug& otherPlug, void* clientData)
{
    if ((msg & MNodeMessage::kAttributeSet && plug != AssetNodeAttributes::fileNameAttr) ||
        (msg & MNodeMessage::kConnectionBroken) && (msg & MNodeMessage::kIncomingDirection))
    {
        cerr << "Attr changed: " << plug.name() << " //// " << plug.asFloat() << endl;

        AssetNodeMonitor* mon = (AssetNodeMonitor*) clientData;
        mon->getManager()->update();
    }
    else if ((msg & MNodeMessage::kConnectionMade) && (msg & MNodeMessage::kIncomingDirection))
    {
        // node is not dirty yet, need to pass force compute cycle to
        // node dirty callback
        cerr << "Attr connection made: " << plug.name() << endl;
        AssetNodeMonitor* mon = (AssetNodeMonitor*) clientData;
        mon->attachNodeDirtyCallback();
    }
}


void
AssetNodeMonitor::nodeDirtyPlugCB(MObject& node, MPlug& plug, void* clientData)
{
    cerr << "Node dirty: " << plug.name() << " |||| " << plug.asFloat() << endl;

    AssetNodeMonitor* mon = (AssetNodeMonitor*) clientData;
    mon->getManager()->update();

    // detach the callback because we only need to do this once when new 
    // input connection is made
    mon->detachNodeDirtyCallback();
}


void
AssetNodeMonitor::sceneOpenedCB(void* clientData)
{
    cerr << "Scene opened" << endl;
    AssetNodeMonitor* mon = (AssetNodeMonitor*) clientData;
    mon->stop();
}
