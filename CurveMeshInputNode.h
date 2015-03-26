#ifndef __CurveMeshInputNode_h__
#define __CurveMeshInputNode_h__

#include <maya/MPxTransform.h>
#include <maya/MTypeId.h>
#include <HAPI/HAPI.h>

class MPlugArray;

#include <vector>

class CurveMeshInputNode: public MPxTransform
{
    public:
        CurveMeshInputNode();
        virtual ~CurveMeshInputNode();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);

        virtual void copyInternalData(MPxNode* node);

        static void* creator();
        static MStatus initialize();

        static MString typeName;
        static MTypeId typeId;

    private:
        HAPI_AssetId myAssetId;

        static MObject theInputCurves;
        static MObject theOutputObjectMetaData;
};

#endif
