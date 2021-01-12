#ifndef __AssetDraw_h__
#define __AssetDraw_h__

#include <maya/MTypes.h>
#if MAYA_API_VERSION >= 20180000

#include "Asset.h"
#include "OutputDeform.h"

#include <maya/MPxLocatorNode.h>
#include <maya/MPxSurfaceShape.h>
#include <maya/MPxGeometryOverride.h>
#include <maya/MShaderManager.h>


class MFnPlugin;

class AssetDrawTraits
{
public:
    AssetDrawTraits();
    MObject inputNodeId;
    MObject shader;
    MObject output;
    MObject shaderfile;
};

class AssetDraw : public MPxLocatorNode
{
public:
    static AssetDrawTraits theTraits;
    AssetDraw();
    ~AssetDraw() override;

    static void *creator();
    static MStatus initialize();

    static MStatus initializePlugin( MFnPlugin &plugin, MObject obj );
    static MStatus uninitializePlugin( MFnPlugin &plugin, MObject obj );


    MStatus compute( const MPlug& plug, MDataBlock& data ) override;
    MStatus computeOutput();

    void draw( M3dView & view, 
	const MDagPath & path,
	M3dView::DisplayStyle style,
	M3dView::DisplayStatus status ) override{}

    bool isBounded() const override;
    MBoundingBox boundingBox() const override;

    MSelectionMask getShapeSelectionMask() const override;

    MStatus preEvaluation(const MDGContext& context, 
	const MEvaluationNode& evaluationNode) override;

    SchedulingType schedulingType() const override {return SchedulingType::kParallel;}

    static MTypeId id;
    static MString drawDbClassification;
    static MString drawRegistrantId;

    OutputDeform myOutputDeform;

private:

};

// Viewport 2.0 override implementation
class AssetDrawGeometryOverride : public MHWRender::MPxGeometryOverride
{
public:
    static MHWRender::MPxGeometryOverride* Creator(const MObject& obj)
    {
	return new AssetDrawGeometryOverride(obj);
    }

    ~AssetDrawGeometryOverride() override;

    MHWRender::DrawAPI supportedDrawAPIs() const override;

    bool hasUIDrawables() const override { return false; }

    void updateDG() override;
    bool isIndexingDirty(const MHWRender::MRenderItem &item) override;
    bool isStreamDirty(const MHWRender::MVertexBufferDescriptor &desc) override;

    void updateRenderItems(const MDagPath &path, MHWRender::MRenderItemList& list) override;
    void populateGeometry(const MHWRender::MGeometryRequirements &requirements, const MHWRender::MRenderItemList &renderItems, MHWRender::MGeometry &data) override;
    void cleanUp() override {};
	
    bool traceCallSequence() const override
    {
	return false; // Return true if internal tracing is desired.
    }

    void handleTraceMessage( const MString &message ) const override
    {
	MGlobal::displayInfo("houdiniDrawGeometryOverride: " + message);
	fputs("houdiniDrawGeometryOverride: ",stderr);
	fputs(message.asChar(),stderr);
	fputs("\n",stderr);
    }

    void preDrawCallback(MDrawContext& ctx, const MRenderItemList& renderItemList, MShaderInstance *sh);

private:
    AssetDrawGeometryOverride(const MObject& obj);

    MHWRender::MShaderInstance* getShader(const MDagPath& path, bool &newshader);
    void releaseShader();

    MObject mLocatorNode;
    OutputDeform *myOutput;

    MObject myShaderNode;
    MString myShaderFile;
    MShaderInstance *myShaderInstance;
    std::vector<MHWRender::MTexture*> myTextures;
    std::vector<MString> myTextureParms;
    size_t myLightCount;
};
#endif
#endif


