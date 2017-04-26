#ifndef __OutputObject_h__
#define __OutputObject_h__

#include <maya/MStatus.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>

#include <HAPI/HAPI.h>

class Asset;

class OutputObject {

    public:
        enum ObjectType {
            OBJECT_TYPE_GEOMETRY,
            OBJECT_TYPE_INSTANCER
        };

        // static creator
        static OutputObject* createObject(
                HAPI_NodeId nodeId
                );

        OutputObject(HAPI_NodeId nodeId);
        virtual ~OutputObject();

        int getId();
        MString getName();

        void setObjectInfo(const HAPI_ObjectInfo &objectInfo);

        virtual ObjectType type() = 0;

        bool        isVisible() const;
        bool        isInstanced() const;

    public:
        bool myIsInstanced;

    protected:
        HAPI_NodeId myNodeId;

        HAPI_NodeInfo myNodeInfo;
        HAPI_ObjectInfo myObjectInfo;

        int myLastCookCount;
};

#endif
