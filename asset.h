#ifndef ASSET_H
#define ASSET_H

#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>

#include <HAPI.h>

#include "object.h"

class Asset {
    public:
        Asset(MString otlFilePath, MObject node);
        ~Asset();

        void init();

        MObjectArray getParmAttributes();
        Object** getObjects();
        Object* findObjectByName(MString name);
        Object* findObjectById(int id);

        // Getters for infos
        HAPI_ObjectInfo getObjectInfo(int id);
        HAPI_Transform getTransformInfo(int id);
        HAPI_MaterialInfo getMaterialInfo(int id);

        MStatus compute(const MPlug& plug, MDataBlock& data);

        MIntArray getParmIntValues(HAPI_ParmInfo& parm);
        MFloatArray getParmFloatValues(HAPI_ParmInfo& parm);
        MStringArray getParmStringValues(HAPI_ParmInfo& parm);

    public:
        HAPI_AssetInfo info;
        int numVisibleObjects;
        int numObjects;


    private:

        void update();

        void addAttrTo(MObject& child, MObject* parent);
        void buildParms();
        int buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start);

        MObject createAttr(HAPI_ParmInfo& parm);
        MObject createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);
        MObject createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);

        void computeAssetInputs(const MPlug& plug, MDataBlock& data);
        void computeInstancerObjects(const MPlug& plug, MDataBlock& data);
        void computeGeometryObjects(const MPlug& plug, MDataBlock& data);


    private:
        MObject node;
        MObject mayaInputs;
        Object** objects;

        // Arrays of infos that can be accessed when updating objects,
        // keeping them here avoids getting them for individual object.
        HAPI_ObjectInfo* objectInfos;
        HAPI_Transform* transformInfos;
        HAPI_MaterialInfo* materialInfos;

        //Object* visibleObjects;
        MObjectArray parmAttributes;

};

#endif
