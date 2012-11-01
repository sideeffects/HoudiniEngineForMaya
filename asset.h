#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>

#include <HAPI.h>

#include "object.h"

class Asset {
    public:
        Asset(MString otlFilePath);
        MObjectArray getParmAttributes();
        Object* getObjects();
        Object* getVisibleObjects();

        MIntArray getParmIntValues(HAPI_ParmInfo& parm);
        MFloatArray getParmFloatValues(HAPI_ParmInfo& parm);
        MStringArray getParmStringValues(HAPI_ParmInfo& parm);

    public:
        HAPI_AssetInfo info;
        int numVisibleObjects;

        // test
        bool materialEnabled;

    private:
        void addAttrTo(MObject& child, MObject* parent);
        void buildParms();
        int buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start);

        MObject createAttr(HAPI_ParmInfo& parm);
        MObject createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);
        MObject createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);

    private:
        Object* objects;
        Object* visibleObjects;
        MObjectArray parmAttributes;

};
