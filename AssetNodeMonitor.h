#ifndef __AssetNodeMonitor_h__
#define __AssetNodeMonitor_h__

#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>
#include <maya/MNodeMessage.h>
#include <maya/MDGModifier.h>

#include "AssetManager.h"

class AssetNodeMonitor
{
    public:
        AssetNodeMonitor(AssetManager* m);
        virtual ~AssetNodeMonitor();

        MStatus watch();
        MStatus stop();

        AssetManager* getManager();

    private:
        void attachNodeDirtyCallback();
        void detachNodeDirtyCallback();

        static void attributeChangedCB(MNodeMessage::AttributeMessage msg, MPlug& plug,
                MPlug& otherPlug, void* clientData);
        static void nodeDirtyPlugCB(MObject& node, MPlug& plug, void* clientData);
        static void sceneOpenedCB(void* clientData);


    private:
        MObject myNode;
        AssetManager* myManager;

        MCallbackId myAttrChangedCBId;
        MCallbackId myNodeDirtyCBId;
        MCallbackId mySceneNewCBId;
        MCallbackId mySceneOpenCBId;
};

#endif
