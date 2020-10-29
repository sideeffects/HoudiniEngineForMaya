#ifndef __AssetDraw2_h__
#define __AssetDraw2_h__

#include <maya/MTypes.h>
#if MAYA_API_VERSION >= 20180000

#include "Asset.h"
#include "OutputDeform.h"

#include <maya/MPxLocatorNode.h>
#include <maya/MPxSurfaceShape.h>
#include <maya/MPxGeometryOverride.h>


class MFnPlugin;

class AssetDraw2Traits
{
public:
    AssetDraw2Traits();
    MObject inputNodeId;
    MObject shader;
    MObject output;
    MObject shaderfile;
};

class AssetDraw2 : public MPxLocatorNode
{
public:
    static AssetDraw2Traits theTraits;
    AssetDraw2();
    ~AssetDraw2() override;

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

    MSelectionMask getShapeSelectionMask() const override;

    MStatus preEvaluation(const MDGContext& context, 
	const MEvaluationNode& evaluationNode) override;

    static MTypeId id;
    static MString drawDbClassification;
    static MString drawRegistrantId;

    OutputDeform myOutputDeform;

private:

};

// Viewport 2.0 override implementation
class AssetDraw2GeometryOverride : public MHWRender::MPxGeometryOverride
{
public:
    static MHWRender::MPxGeometryOverride* Creator(const MObject& obj)
    {
	return new AssetDraw2GeometryOverride(obj);
    }

    ~AssetDraw2GeometryOverride() override;

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
	MGlobal::displayInfo("houdiniDraw2GeometryOverride: " + message);
	fputs("houdiniDraw2GeometryOverride: ",stderr);
	fputs(message.asChar(),stderr);
	fputs("\n",stderr);
    }

private:
    AssetDraw2GeometryOverride(const MObject& obj);

    MHWRender::MShaderInstance* getShader(const MDagPath& path, bool &newshader);
    void releaseShader();

    MObject mLocatorNode;
    OutputDeform *myOutput;

    MObject myShaderNode;
    MString myShaderFile;
    MShaderInstance *myShaderInstance;
};
#endif
#endif


