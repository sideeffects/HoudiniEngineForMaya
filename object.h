#ifndef OBJECT_H
#define OBJECT_H

#include <maya/MStatus.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>

#include <HAPI.h>

class Asset;

class Object {


    public:
        enum ObjectType {
            OBJECT_TYPE_GEOMETRY,
            OBJECT_TYPE_INSTANCER
        };

        // static creator
        static Object* createObject(int assetId, int objectId, Asset* objControl);

        Object();
        Object(int assetId, int objectId);
        MObject createMesh();

        virtual int getId();
        virtual MString getName();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);
        virtual ObjectType type() = 0;

    public:
        Asset* objectControl;
        bool isInstanced;

    protected:
        virtual void update();

        // Utility
        virtual MFloatArray getAttributeFloatData(HAPI_AttributeOwner owner, MString name);
        virtual MStringArray getAttributeStringData(HAPI_AttributeOwner owner, MString name);
        virtual void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts);
        virtual void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts);
        virtual void printAttributes(HAPI_AttributeOwner owner);

    protected:
        HAPI_ObjectInfo objectInfo;
        HAPI_GeoInfo geoInfo;
        int assetId;
        int objectId;

};

#endif
