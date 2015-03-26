#ifndef __CurveMeshInputNode_h__
#define __CurveMeshInputNode_h__

#include <maya/MPxTransform.h>
#include <maya/MTypeId.h>

class MPlugArray;

#include <vector>

class CurveMeshInputNode: public MPxTransform
{
    public:
        CurveMeshInputNode();
        virtual ~CurveMeshInputNode();

        virtual void postConstructor();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);

        virtual bool getInternalValueInContext(
                const MPlug &plug,
                MDataHandle &dataHandle,
                MDGContext &ctx);

        virtual bool setInternalValueInContext(
                const MPlug &plug,
                const MDataHandle &dataHandle,
                MDGContext &ctx
                );

        virtual int internalArrayCount( const MPlug &plug,
                                        const MDGContext &ctx) const;

        virtual void copyInternalData(MPxNode* node);

        virtual MStatus setDependentsDirty( const MPlug& plugBeingDirtied,
                                            MPlugArray& affectedPlugs);

        static void* creator();
        static MStatus initialize();

    public:
        static MString typeName;
        static MTypeId typeId;

    public:
        static MObject theInputCurves;
        static MObject theOutputObjectMetaData;
};

#endif
