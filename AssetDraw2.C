#include <maya/MTypes.h>
#if MAYA_API_VERSION >= 20180000

#include <maya/MString.h>
#include <maya/MFileObject.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MModelMessage.h>
#include <maya/MSelectionList.h>
#include <maya/MFnMessageAttribute.h>

#define MNoVersionString
#define MNoPluginEntry
#include <maya/MFnPlugin.h>

#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MEvaluationNode.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
#include <maya/MShaderManager.h>
#include <maya/MHWGeometry.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPointArray.h>

#include <unordered_map>
#include <signal.h>
#include <limits>

#include "MayaTypeID.h"
#include "AssetNode.h"
#include "AssetDraw2.h"
#include "hapiutil.h"

static std::unordered_map<MShaderInstance*,AssetDrawGeometryOverride*> 
theShaderToOverrideMap;

// Ctor of AssetDrawTraits
AssetDrawTraits::AssetDrawTraits()
{
}

namespace
{
    // Viewport 2.0 specific data
    //
    const MString colorParameterName_ = "solidColor";
    const MString wireframeItemName_  = "houdiniDrawWires";
    const MString shadedItemName_     = "houdiniDrawTriangles";

    // Maintain a mini cache for 3d solid shaders in order to reuse the shader
    // instance whenever possible. This can allow Viewport 2.0 optimization e.g.
    // the GPU instancing system and the consolidation system to be leveraged.
    //
    struct MColorHash
    {
	std::size_t
	operator()(const MColor& color) const
	{
	    std::size_t seed = 0;
	    CombineHashCode(seed, color.r);
	    CombineHashCode(seed, color.g);
	    CombineHashCode(seed, color.b);
	    CombineHashCode(seed, color.a);
	    return seed;
	}

	void
	CombineHashCode(std::size_t& seed, float v) const
	{
	    std::hash<float> hasher;
	    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
    };

    std::unordered_map<MColor, MHWRender::MShaderInstance*, MColorHash> the3dSolidShaders;
    std::unordered_map<MColor, MHWRender::MShaderInstance*, MColorHash> the3dMaterialShaders;

#if 1
    void
    printTechniques(const MStringArray& techniqueNames)
    {
	for( size_t i=0; i<techniqueNames.length(); ++i)
	{
	    const MString &s = techniqueNames[i];
	    printf("technique[%d] = %s\n", (int)i, s.asChar());
	}
    }

    void
    printShaderParameters(MHWRender::MShaderInstance *test)
    {
	const char * parameterTypes[] {
	    "kInvalid",
	    "kBoolean",
	    "kInteger",
	    "kFloat",
	    "kFloat2",
	    "kFloat3",
	    "kFloat4",
	    "kFloat4x4Row",
	    "kFloat4x4Col",
	    "kTexture1",
	    "kTexture2",
	    "kTexture3",
	    "kTextureCube",
	    "kSampler"
	};

	MStringArray l;
	test->parameterList(l);
	for (unsigned int i=0; i<l.length(); ++i)
	{
	    int pt = (int)test->parameterType(l[i]);
	    printf("%s %s\n",parameterTypes[pt],l[i].asChar());
	}
    }
#endif

    MHWRender::MShaderInstance*
    get3dSolidShader(const MColor& color, bool &newshader)
    {
	// Return the shader instance if exists.
	auto it = the3dSolidShaders.find(color);
	if (it != the3dSolidShaders.end())
	    return it->second;


	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
	    return nullptr;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr)
	    return nullptr;

	MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
	    MHWRender::MShaderManager::k3dSolidShader);

	if (!shader)
	    return nullptr;

	float solidColor[] = { color.r, color.g, color.b, 1.0f };
	shader->setParameter(colorParameterName_, solidColor);

	the3dSolidShaders[color] = shader;

	newshader = true;
	return shader;
    }

    MHWRender::MShaderInstance*
    get3dDefaultMaterialShader(const MColor& color, bool &newshader)
    {
	// Return the shader instance if exists.
	auto it = the3dMaterialShaders.find(color);
	if (it != the3dMaterialShaders.end())
	    return it->second;


	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
	    return nullptr;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr)
	    return nullptr;

	MHWRender::MShaderInstance* shader = shaderMgr->getStockShader(
	    MHWRender::MShaderManager::k3dDefaultMaterialShader);


	if (!shader)
	    return nullptr;

	//printShaderParameters(shader);

	//kFloat4 diffuseColor
	float diffuseColor[] = { 0.7, 0.7, 0.7, 1.0f };
	shader->setParameter("diffuseColor", diffuseColor);

	the3dMaterialShaders[color] = shader;

	newshader = true;
	return shader;
    }


    MStatus
    releaseShaders()
    {
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if (!renderer)
	    return MS::kFailure;

	const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
	if (!shaderMgr)
	    return MS::kFailure;

	for (auto it = the3dSolidShaders.begin(); it != the3dSolidShaders.end(); it++)
	    shaderMgr->releaseShader(it->second);

	for (auto it = the3dMaterialShaders.begin(); it != the3dMaterialShaders.end(); it++)
	    shaderMgr->releaseShader(it->second);

	the3dSolidShaders.clear();
	the3dMaterialShaders.clear();
	return MS::kSuccess;

    }
} // anonymous namespace

MTypeId AssetDraw::id( MayaTypeID_HoudiniAssetDraw );
MString	AssetDraw::drawDbClassification("drawdb/geometry/houdiniDraw");
MString	AssetDraw::drawRegistrantId("houdiniDrawPlugin");

AssetDraw::AssetDraw()
: myOutputDeform(/*topo=*/true,/*normal=*/true, /*skippoints=*/true, /*uvs=*/true)
{
}
AssetDraw::~AssetDraw()
{
}

MStatus
AssetDraw::compute( const MPlug& plug, MDataBlock& data )
{
    AssetDrawTraits &traits = AssetDraw::theTraits;

    MString plugName = plug.name();

    if (plug.attribute() == traits.output)
    {
	// Pull on the output plug ourself to trigger cooking
	// Store a pointer to the output for access during drawing
	MPlug inPlug(thisMObject(), traits.inputNodeId);

	int i=0;
	inPlug.getValue(i);
        data.setClean(plug);

	MPlugArray plugs;
	if (!inPlug.connectedTo(plugs,/*asDst=*/true,/*asSrc=*/false))
	    return MS::kFailure;

	AssetNode *pAssetNode = dynamic_cast<AssetNode*>(
	    MFnDependencyNode(plugs[0].node()).userNode());
	if (!pAssetNode)
	    return MS::kFailure;

	Asset *a = pAssetNode->getAsset();
	if (!a || !a->isValid())
	    return MS::kFailure;

	OutputObject *obj = a->getOutputObject(0);

	size_t n = 0;
	if (!myOutputDeform.compute(obj,n))
	    return MS::kFailure;

	return MS::kSuccess;
    }
    return MS::kUnknownParameter;
}

bool
AssetDraw::isBounded() const
{
    return true;
}

MBoundingBox
AssetDraw::boundingBox() const
{
    static MBoundingBox theReallyBigBox(
        MPoint( std::numeric_limits<double>::min(),
                std::numeric_limits<double>::min(),
                std::numeric_limits<double>::min())
                ,
        MPoint( std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max())
                );

    return theReallyBigBox;
}

MSelectionMask
AssetDraw::getShapeSelectionMask() const
{
    return MSelectionMask("houdiniDrawSelection");
}

// Called before this node is evaluated by Evaluation Manager
MStatus
AssetDraw::preEvaluation( const MDGContext& context,
    const MEvaluationNode& evaluationNode)
{
    AssetDrawTraits &traits = AssetDraw::theTraits;
    if (context.isNormal())
    {
	MStatus status;
	if( (evaluationNode.dirtyPlugExists(traits.inputNodeId, &status) && status) ||
	    (evaluationNode.dirtyPlugExists(traits.output, &status) && status) )
	{
	    MHWRender::MRenderer::setGeometryDrawDirty(thisMObject());
	}
    }
    return MStatus::kSuccess;
}

void*
AssetDraw::creator()
{
    return new AssetDraw();
}



// Viewport 2.0 override implementation

AssetDrawGeometryOverride::AssetDrawGeometryOverride(const MObject& obj)
: MHWRender::MPxGeometryOverride(obj)
, mLocatorNode(obj)
, myOutput(nullptr)
, myShaderInstance(nullptr)
, myLightCount(0)
{
}

AssetDrawGeometryOverride::~AssetDrawGeometryOverride()
{
    releaseShader();
}

void
AssetDrawGeometryOverride::releaseShader()
{
    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
    if (!renderer)
	return;
    const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr)
	return;

    const MHWRender::MTextureManager* textureMgr = renderer->getTextureManager();
    if (!textureMgr)
	return;

    // Release the existing shader
    if (myShaderInstance)
    {
	shaderMgr->releaseShader(myShaderInstance);
	theShaderToOverrideMap.erase(myShaderInstance);
    }

    myShaderInstance = nullptr;
    myShaderNode = MObject();
    myShaderFile = MString();

    for (size_t i=0; i<myTextures.size(); ++i)
    {
	if (myTextures[i])
	    textureMgr->releaseTexture(myTextures[i]);
    }
    myTextures.clear();
    myTextureParms.clear();
    myLightCount = 0;
}
const static MString kLightColorF = "Light%dColor";
const static MString kLightTypeF = "Light%dType";
const static MString kLightIntensityF = "Light%dIntensity";
const static MString kLightPosF = "Light%dPos";
const static MString kLightDirF = "Light%dDir";

struct LightNames
{
    MString kLightColor;
    MString kLightType;
    MString kLightIntensity;
    MString kLightPos;
    MString kLightDir;
};

enum ELightType
{
	eInvalidLight,
	eUndefinedLight,
	eSpotLight,
	ePointLight,	
	eDirectionalLight,
	eAmbientLight,
	eVolumeLight,
	eAreaLight,
	eDefaultLight,
	eLightCount
};
// Convert Maya light type to glslShader light type
ELightType
getLightType(const MHWRender::MLightParameterInformation* lightParam)
{
    ELightType type = eUndefinedLight;
    MString lightType = lightParam->lightType();

    // The 3rd letter of the light name is a perfect hash,
    // so let's cut on the number of string comparisons.
    if (lightType.length() <= 2)
	return type;

    char c2 = lightType.asChar()[2];
    switch (c2)
    {
    case 'o':
	//if (STRICMP(lightType.asChar(),"spotLight") == 0)
	type = eSpotLight;
	break;

    case 'r':
	//if (STRICMP(lightType.asChar(),"directionalLight") == 0)
	{
	    // The headlamp used in the "Use default lighting" mode
	    // does not have the same set of attributes as a regular
	    // directional light, so we must disambiguate them
	    // otherwise we might not know how to fetch shadow data
	    // from the regular kind.
	    if (lightParam->lightPath().isValid())
		type = eDirectionalLight;
	    else
		type = eDefaultLight;
	}
	break;

    case 'i':
	//if (STRICMP(lightType.asChar(),"pointLight") == 0)
	    type = ePointLight;
	break;

    case 'b':
	//if (STRICMP(lightType.asChar(),"ambientLight") == 0)
	    type = eAmbientLight;
	break;

    case 'l':
	//if (STRICMP(lightType.asChar(),"volumeLight") == 0)
	    type = eVolumeLight;
	break;

    case 'e':
	//if (STRICMP(lightType.asChar(),"areaLight") == 0)
	    type = eAreaLight;
	break;
    }
    return type;
}

void
AssetDraw2PreCallback(MDrawContext& ctx,
    const MRenderItemList& renderItemList, MShaderInstance *sh)
{
    AssetDrawGeometryOverride *drawOverride = theShaderToOverrideMap[sh];
    drawOverride->preDrawCallback(ctx,renderItemList,sh);
}

void
AssetDrawGeometryOverride::preDrawCallback(MDrawContext& ctx,
    const MRenderItemList& renderItemList, MShaderInstance *sh)
{
    sh->bind(ctx);
    MStatus status;

    //Prebuild a list of shader parameter names for each light index
    static LightNames names[32];
    static bool initNames = true;
    if (initNames)
    {
	char buf[128];
	for( int i=0; i<32; ++i)
	{
	    LightNames &namei = names[i];
#define INIT_NAME(X) \
	    sprintf(buf, X ## F.asChar(), i); \
	    namei.X.set(buf)
	    INIT_NAME(kLightColor);
	    INIT_NAME(kLightType);
	    INIT_NAME(kLightIntensity);
	    INIT_NAME(kLightPos);
	    INIT_NAME(kLightDir);
	}
	initNames=false;
    }

    // the set parameter calls somehow hold the pointers
    // so we'll use a buffer to hold the values until 
    // updateParameters is called.
    float zero4[4] = {0.f,0.f,0.f,0.f};

    // Run over the active lights
    size_t n = ctx.numberOfActiveLights();
    if (n>myLightCount)
	n = myLightCount;

    for (size_t i=0; i<n; ++i)
    {
	MLightParameterInformation* li = ctx.getLightParameterInformation(i);
	if (!li)
	    continue;

	MStringArray parms;
	li->parameterList(parms);
	for (size_t j=0; j<parms.length(); ++j)
	{
	    const MString &p = parms[j];
	    printf("Light%d%s\n",(int)i,p.asChar());
	}

	MIntArray ivals;
	MFloatArray fvals;
	MFloatVector vec;

	LightNames &namei = names[i];
	bool enabled = true;
	{
	    status = li->getParameter(
		MLightParameterInformation::kLightEnabled,ivals);
	    if (status==MS::kSuccess)
		enabled = ivals[0];
	}
	if (!enabled)
	{
	    sh->setParameter(namei.kLightColor, zero4);
	    sh->setParameter(namei.kLightIntensity, 0.f);
	    sh->setParameter(namei.kLightType, 0);//not sure of this value
	    continue;
	}

	ELightType lightType = getLightType(li);
	printf("%s->%d\n",li->lightType().asChar(), (int)lightType);
	sh->setParameter(namei.kLightType, (int)lightType);

	status = li->getParameter(
	    MLightParameterInformation::kWorldPosition,fvals);
	if (status==MS::kSuccess)
	{
	    sh->setParameter(namei.kLightPos,&fvals[0]);
	}

	status = li->getParameter(
	    MLightParameterInformation::kWorldDirection,fvals);
	if (status==MS::kSuccess)
	{
	    sh->setParameter(namei.kLightDir,&fvals[0]);
	}

	status = li->getParameter(
	    MLightParameterInformation::kIntensity,fvals);
	if (status==MS::kSuccess)
	    sh->setParameter(namei.kLightIntensity,fvals[0]);

	status = li->getParameter(
	    MLightParameterInformation::kColor,fvals);
	if (status==MS::kSuccess)
	{
	    //printf("setParm%d %s %f %f %f\n", (int)i, namei.kLightColor.asChar(), fvals[0], fvals[1], fvals[2]);
	    sh->setParameter(namei.kLightColor,&fvals[0]);
	}
    }
    // Disable all the remaining lights
    for (size_t i=n; i<myLightCount; ++i)
    {
	LightNames &namei = names[i];
	sh->setParameter(namei.kLightColor, zero4);
	sh->setParameter(namei.kLightIntensity, 0.f);
	sh->setParameter(namei.kLightType, 0);//not sure of this value
    }

    // Force texture parms, to avoid flickering
    /*for (size_t i=0; i<myTextures.size(); ++i)
    {
	MHWRender::MTexture *texture = myTextures[i];
	const MString &pname = myTextureParms[i];

	// Set the shader parameter
	MHWRender::MTextureAssignment assign;
	assign.texture = texture;
	sh->setParameter(pname,assign);
    }*/

    sh->updateParameters(ctx);

    sh->unbind(ctx);
}

MHWRender::MShaderInstance*
AssetDrawGeometryOverride::getShader(const MDagPath& path, bool &newshader)

{
    MStatus status;
    AssetDrawTraits &traits = AssetDraw::theTraits;

    MPlug shaderPlug(path.node(),traits.shader);
    MPlug shaderFilePlug(path.node(),traits.shaderfile);

    MObject shaderNode = shaderPlug.source().node();
    MString shaderFile = shaderFilePlug.asString();
    if (myShaderNode==shaderNode && myShaderFile==shaderFile)
	return myShaderInstance;

    releaseShader();

    if (shaderNode.isNull())
	return nullptr;
    

    MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
    if (!renderer)
	return nullptr;
    const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
    if (!shaderMgr)
	return nullptr;
    MHWRender::MTextureManager* textureMgr = renderer->getTextureManager();
    if (!textureMgr)
	return nullptr;


    MFnDependencyNode shaderDep(shaderNode);
    CHECK_MSTATUS(status);

    MHWRender::MShaderInstance* shader = nullptr;

    size_t lightCount = 0;
    // Try building a shader from the file attribute
    if (shaderFile.length()>0)
    {
	MFileObject shaderFileObject;
	shaderFileObject.setRawFullName(shaderFile);
	
	MString folder = shaderFileObject.resolvedPath();
	printf("folder=%s\n", folder.asChar());

	MHWRender::MShaderCompileMacro *macros = nullptr;
	const unsigned int numMacros = 0;
	bool useEffectCache = false;
	MShaderInstance::DrawCallback preCb = AssetDraw2PreCallback;

	MStringArray techniques;
	shaderMgr->getEffectsTechniques(shaderFile, techniques,
	    macros, numMacros, useEffectCache);
	printTechniques(techniques);

	std::unordered_map<std::string,size_t> prefixes;
	for (size_t i=0; i<32; ++i)
	{
	    std::string key;

	    key = "Light";
	    key += std::to_string(i);
	    prefixes[key] = i;

	    key = "light";
	    key += std::to_string(i);
	    prefixes[key] = i;
	}

	if (techniques.length()>0)
	{
	    const MString &technique = techniques[0];
	    shader = shaderMgr->getEffectsFileShader(shaderFile,technique,
		macros, numMacros, useEffectCache, preCb);

	    // set textures
	    if (shader)
	    {
		MStringArray l;
		shader->parameterList(l);

		// count the number of lights by looking at the parameter
		// names and light index
		for (unsigned int i=0; i<l.length(); ++i)
		{
		    size_t lightIndex = 0;

		    const MString & pname = l[i];
		    std::string s = pname.asChar();

		    std::string key;
		    std::unordered_map<std::string,size_t>::const_iterator it;
		    
		    key = s.substr(0,7);
		    it = prefixes.find(key);
		    if (it==prefixes.end())
		    {
			key = s.substr(0,6);
			it = prefixes.find(key);
		    }
		    if (it==prefixes.end())
			continue;

		    lightIndex = it->second;
		    if (lightIndex+1>lightCount)
			lightCount = lightIndex+1;
		}
		printf("lightCount -> %d\n", (int)lightCount);

		// Set parameters
		for (unsigned int i=0; i<l.length(); ++i)
		{
		    // Support only 2D textures
		    const MString & pname = l[i];
		    MShaderInstance::ParameterType pt = shader->parameterType(pname);
		    if (pt!=MShaderInstance::kTexture2)
			continue;

		    // Heuristic to get a full filename from the resource name
		    // We might want to expose a texture folder on AssetDraw
		    // as a string parameter.
		    // I could only get the name of the file without the folder
		    // from the shader file.
		    MString resourceName = shader->resourceName(pname,status);
		    printf("texture %s -> %s\n",
			pname.asChar(), resourceName.asChar());

		    if (resourceName.length()==0)
			continue;
		    
		    // resolve the texture path
		    MString texturePath;
		    {
			MFileObject textureFileObject;
			textureFileObject.setRawPath(folder);
			textureFileObject.setName(resourceName);

			texturePath = textureFileObject.resolvedFullName();
		    }

		    // Get a texture
		    // TODO add mipmap control
		    MHWRender::MTextureArguments args(texturePath);
		    MHWRender::MTexture *texture =
			textureMgr->acquireTexture(args);
		    if (!texture)
			continue;

		    // Set the shader parameter
		    MHWRender::MTextureAssignment assign;
		    assign.texture = texture;
		    shader->setParameter(pname,assign);

		    // TODO use shared textures if the paths are the same
		    myTextures.push_back(texture);
		    myTextureParms.push_back(pname);
		}
	    }
	}
    }

    
    // Fallback to the connected shader
    if (!shader)
	shader = shaderMgr->getShaderFromNode(shaderNode, path);

    // No shader at this point is a failure.
    if (!shader)
	return nullptr;

    // Debug shader parameters
    printShaderParameters(shader);

    newshader = true;
    myShaderInstance = shader;
    theShaderToOverrideMap[myShaderInstance] = this;
    myShaderNode = shaderNode;
    myShaderFile = shaderFile;
    myLightCount = lightCount;
    return shader;
}

MHWRender::DrawAPI
AssetDrawGeometryOverride::supportedDrawAPIs() const
{
    // this plugin supports both GL and DX
    return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile);
}

void
AssetDrawGeometryOverride::updateDG()
{
    myOutput = nullptr;

    AssetDrawTraits &traits = AssetDraw::theTraits;

    // Pull on the output plug ourself to trigger cooking
    // Store a pointer to the output for access during drawing
    MPlug plug(mLocatorNode, traits.output);
    int i=0;
    plug.getValue(i);

    MString plugName = plug.name();

    AssetDraw *pAssetDraw2 = dynamic_cast<AssetDraw*>(
	MFnDependencyNode(mLocatorNode).userNode());
    if (!pAssetDraw2)
	return;

    myOutput = &pAssetDraw2->myOutputDeform;

}

bool
AssetDrawGeometryOverride::isIndexingDirty(const MHWRender::MRenderItem &item)
{
    return myOutput ? myOutput->myTopoDirty : false;
}

bool
AssetDrawGeometryOverride::isStreamDirty(const MHWRender::MVertexBufferDescriptor &desc)
{
    return myOutput ? myOutput->myPos.myDirty||myOutput->myTopoDirty : false;
}

void
AssetDrawGeometryOverride::updateRenderItems( const MDagPath& path,
    MHWRender::MRenderItemList& list )
{
    if (!myOutput)
	return;

    bool newshader=false;
    MColor color = MHWRender::MGeometryUtilities::wireframeColor(path);
    MHWRender::MShaderInstance* shader0 = get3dSolidShader(color,newshader);
    if (!shader0)
	return;

    MHWRender::MShaderInstance* shader1 = get3dDefaultMaterialShader(color,newshader);
    if (!shader1)
	return;

    MHWRender::MShaderInstance* shaderSG = getShader(path,newshader);
    if (shaderSG)
	shader1 = shaderSG;

    if (newshader)
	myOutput->myTopoDirty = true;

    unsigned int depthPriority;
    switch (MHWRender::MGeometryUtilities::displayStatus(path))
    {
	case MHWRender::kLead:
	case MHWRender::kActive:
	case MHWRender::kHilite:
	case MHWRender::kActiveComponent:
	    depthPriority = MHWRender::MRenderItem::sActiveWireDepthPriority;
	    break;
	default:
	    depthPriority = MHWRender::MRenderItem::sDormantFilledDepthPriority;
	    break;
    }

    MHWRender::MRenderItem* wireframeItem = NULL;

    int index = 0;

    index = list.indexOf(wireframeItemName_);
    if (index < 0)
    {
        wireframeItem = MHWRender::MRenderItem::Create(
            wireframeItemName_,
            MHWRender::MRenderItem::DecorationItem,
            MHWRender::MGeometry::kLines);
        wireframeItem->setDrawMode(MHWRender::MGeometry::kWireframe);
        list.append(wireframeItem);
    }
    else
    {
        wireframeItem = list.itemAt(index);
    }

    if(wireframeItem)
    {
	wireframeItem->setShader(shader0);
	wireframeItem->depthPriority(depthPriority);
	wireframeItem->enable(true);
    }

    MHWRender::MRenderItem* shadedItem = NULL;

    index = list.indexOf(shadedItemName_);
    if (index < 0)
    {
	shadedItem = MHWRender::MRenderItem::Create(
            shadedItemName_,
            MHWRender::MRenderItem::MaterialSceneItem,
            MHWRender::MGeometry::kTriangles);

	shadedItem->setDrawMode((MHWRender::MGeometry::DrawMode)
	    (MHWRender::MGeometry::kShaded | MHWRender::MGeometry::kTextured));

        list.append(shadedItem);
    }
    else
    {
	shadedItem = list.itemAt(index);
    }

    if(shadedItem)
    {
	shadedItem->setShader(shader1);
	shadedItem->depthPriority(depthPriority);
	shadedItem->enable(true);
    }
}
void
AssetDrawGeometryOverride::populateGeometry(
    const MHWRender::MGeometryRequirements& requirements,
    const MHWRender::MRenderItemList& renderItems,
    MHWRender::MGeometry& data)
{
    if (!myOutput)
	return;

    MHWRender::MVertexBuffer* verticesBuffer = NULL;
    MHWRender::MVertexBuffer* normalsBuffer = NULL;
    MHWRender::MVertexBuffer* texturesBuffer = NULL;
    MHWRender::MVertexBuffer* tangentsBuffer = NULL;
    MHWRender::MVertexBuffer* bitangentsBuffer = NULL;

    float* vertices = NULL;
    float* normals = NULL;
    float* textures = NULL;
    float* tangents = NULL;
    float* bitangents = NULL;

    const MHWRender::MVertexBufferDescriptorList& vertexBufferDescriptorList =
	requirements.vertexRequirements();

    const int numberOfVertexRequirments = vertexBufferDescriptorList.length();

    const HAPI_PartInfo &info = myOutput->myPartInfo;
    int pointCount = myOutput->myPos.myIsValid ? info.pointCount : 0;

    MHWRender::MVertexBufferDescriptor vertexBufferDescriptor;
    for (int requirementNumber = 0; 
	requirementNumber < numberOfVertexRequirments; ++requirementNumber)
    {
	if (!vertexBufferDescriptorList.getDescriptor(
	    requirementNumber, vertexBufferDescriptor))
	    continue;

	switch (vertexBufferDescriptor.semantic())
	{
	    case MHWRender::MGeometry::kPosition:
	    {
		if (!verticesBuffer)
		{
		    verticesBuffer = data.createVertexBuffer(vertexBufferDescriptor);
		    if (verticesBuffer)
			vertices = (float*)verticesBuffer->acquire(pointCount,/*writeOnly=*/true);
		}
		break;
	    }

	    case MHWRender::MGeometry::kNormal:
	    {
		if (!normalsBuffer)
		{
		    normalsBuffer = data.createVertexBuffer(vertexBufferDescriptor);
		    if (normalsBuffer)
			normals = (float*)normalsBuffer->acquire(pointCount,/*writeOnly=*/true);
		}
		break;
	    }

	    case MHWRender::MGeometry::kTexture:
	    {
		if (!texturesBuffer && myOutput->myTopoDirty)
		{
		    texturesBuffer = data.createVertexBuffer(vertexBufferDescriptor);
		    if (texturesBuffer)
			textures = (float*)texturesBuffer->acquire(pointCount,/*writeOnly=*/true);
		}
		break;
	    }

	    case MHWRender::MGeometry::kTangent:
	    {
		if (!tangentsBuffer && myOutput->myTopoDirty)
		{
		    tangentsBuffer = data.createVertexBuffer(vertexBufferDescriptor);
		    if (tangentsBuffer)
			tangents = (float*)tangentsBuffer->acquire(pointCount,/*writeOnly=*/true);
		}
		break;
	    }

	    case MHWRender::MGeometry::kBitangent:
	    {
		if (!bitangentsBuffer && myOutput->myTopoDirty)
		{
		    bitangentsBuffer = data.createVertexBuffer(vertexBufferDescriptor);
		    if (bitangentsBuffer)
			bitangents = (float*)bitangentsBuffer->acquire(pointCount,/*writeOnly=*/true);
		}
		break;
	    }
	    default:
		// do nothing for stuff we don't understand
		break;
	    }
    }

    if(vertices)
    {
	if (!myOutput->myPos.myIsValid)
	{
	    float *dst = vertices;
	    memset(dst, 0, sizeof(float)*3 * pointCount);
	}
	else if (myOutput->myDelayedPointCount>0)
	{
	    float *dst = vertices;
	    myOutput->getDelayedPointAttribute("P",myOutput->myPos,pointCount,dst);
	}
	else
	{
	    if (myOutput->myPos.myIsDouble)
	    {
		float *dst = vertices;
		double *src = &(myOutput->myPos.myData64[0]);
		if (src)
		{
		    for (int i=0; i<pointCount; ++i)
		    {
			*(dst++) = *(src++);
			*(dst++) = *(src++);
			*(dst++) = *(src++);
		    }
		}
		else
		{
		    memset(dst, 0, sizeof(float)*3 * pointCount);
		}


	    }
	    else
	    {
		float *src = &(myOutput->myPos.myData32[0]);
		if (src)
		{
		    memcpy(vertices, src, sizeof(float)*3 * pointCount);
		}
		else
		{
		    memset(vertices, 0, sizeof(float)*3 * pointCount);
		}
	    }
	}
    }

    if(verticesBuffer && vertices)
    {
	verticesBuffer->commit(vertices);
	myOutput->myPos.myDirty = false;
    }

    if(normals)
    {
	if (!myOutput->myNormal.myIsValid)
	{
	    float *dst = normals;
	    for (int i=0; i<pointCount; ++i)
	    {
		*(dst++) = 0;
		*(dst++) = 1;
		*(dst++) = 0;
	    }
	}
	else if (myOutput->myDelayedPointCount>0)
	{
	    float *dst = normals;
	    myOutput->getDelayedPointAttribute("N",myOutput->myNormal,pointCount,dst);
	}
	else if (myOutput->myNormal.myIsDouble)
	{
	    float *dst = normals;
	    double *src = &(myOutput->myNormal.myData64[0]);
	    if (src)
	    {
		for (int i=0; i<pointCount; ++i)
		{
		    *(dst++) = *(src++);
		    *(dst++) = *(src++);
		    *(dst++) = *(src++);
		}
	    }
	    else
	    {
		for (int i=0; i<pointCount; ++i)
		{
		    *(dst++) = 0;
		    *(dst++) = 1;
		    *(dst++) = 0;
		}
	    }
	}
	else
	{
	    float *src = &(myOutput->myNormal.myData32[0]);
	    if (src)
	    {
		memcpy(normals, src, sizeof(float)*3 * pointCount);
	    }
	    else
	    {
		float *dst = normals;
		for (int i=0; i<pointCount; ++i)
		{
		    *(dst++) = 0;
		    *(dst++) = 1;
		    *(dst++) = 0;
		}
	    }
	}
    }

    if(normalsBuffer && normals)
    {
	normalsBuffer->commit(normals);
	myOutput->myNormal.myDirty = false;
    }

    if(textures)
    {
	if (!myOutput->myTexture.myIsValid)
	{
	    float *dst = textures;
	    for (int i=0; i<pointCount; ++i)
	    {
		*(dst++) = 0;
		*(dst++) = 1;
	    }
	}
	else if (myOutput->myDelayedPointCount>0)
	{
	    float *dst = textures;
	    myOutput->getDelayedPointAttribute("uv",myOutput->myTexture,pointCount,dst);
	}
	else if (myOutput->myTexture.myIsDouble)
	{
	    float *dst = textures;
	    double *src = &(myOutput->myTexture.myData64[0]);
	    if (src)
	    {
		for (int i=0; i<pointCount; ++i)
		{
		    *(dst++) = *(src++);
		    *(dst++) = *(src++);
		}
	    }
	    else
	    {
		for (int i=0; i<pointCount; ++i)
		{
		    *(dst++) = 0;
		    *(dst++) = 1;
		}
	    }
	}
	else
	{
	    float *src = &(myOutput->myTexture.myData32[0]);
	    if (src)
	    {
		memcpy(textures, src, sizeof(float)*2 * pointCount);
	    }
	    else
	    {
		memset(textures, 0, sizeof(float)*2 * pointCount);
	    }
	}
    }

    if(texturesBuffer && textures)
    {
	texturesBuffer->commit(textures);
	myOutput->myTexture.myDirty = false;
    }

    for (int ri=0; ri < renderItems.length(); ++ri)
    {
	const MHWRender::MRenderItem* item = renderItems.itemAt(ri);
	if (!item)
	    continue;

	if (!myOutput->myTopoValid)
	    continue;

	MHWRender::MIndexBuffer* indexBuffer = data.createIndexBuffer(MHWRender::MGeometry::kUnsignedInt32);

	int faceCount = info.faceCount;
	if (item->name() == wireframeItemName_ )
	{
	    int numIndex = faceCount*6;

	    unsigned int* indices = (unsigned int*)indexBuffer->acquire(numIndex,/*writeOnly=*/true);

	    unsigned int* dst = indices;
	    int* src = &myOutput->myVertexList[0];
	    int* faces = &myOutput->myFaceCounts[0];
	    if (src)
	    {
		for (int i=0; i<faceCount; ++i)
		{
		    *(dst++) = (unsigned int)(src[0]);
		    *(dst++) = (unsigned int)(src[1]);

		    *(dst++) = (unsigned int)(src[1]);
		    *(dst++) = (unsigned int)(src[2]);

		    *(dst++) = (unsigned int)(src[2]);
		    *(dst++) = (unsigned int)(src[0]);
		    src += *(faces++);
		}
	    }
	    else
	    {
		for (int i=0; i<faceCount; ++i)
		{
		    *(dst++) = (unsigned int)(0);
		    *(dst++) = (unsigned int)(0);

		    *(dst++) = (unsigned int)(0);
		    *(dst++) = (unsigned int)(0);

		    *(dst++) = (unsigned int)(0);
		    *(dst++) = (unsigned int)(0);
		}
	    }

	    indexBuffer->commit(indices);
	}
	else if (item->name() == shadedItemName_ )
	{
	    int numIndex = faceCount*3;

	    unsigned int* indices = (unsigned int*)indexBuffer->acquire(numIndex,/*writeOnly=*/true);

	    unsigned int* dst = indices;
	    int* src = &myOutput->myVertexList[0];
	    int* faces = &myOutput->myFaceCounts[0];

	    if (src)
	    {
		for (int i=0; i<faceCount; ++i)
		{
		    *(dst++) = (unsigned int)(src[0]);
		    *(dst++) = (unsigned int)(src[1]);
		    *(dst++) = (unsigned int)(src[2]);
		    src += *(faces++);
		}
	    }
	    else
	    {
		for (int i=0; i<faceCount; ++i)
		{
		    *(dst++) = (unsigned int)0;
		    *(dst++) = (unsigned int)0;
		    *(dst++) = (unsigned int)0;
		}
	    }

	    indexBuffer->commit(indices);
	}

	item->associateWithIndexBuffer(indexBuffer);
	myOutput->myTopoDirty = false;
    }
}

//---------------------------------------------------------------------------
// Plugin Registration
//---------------------------------------------------------------------------

MStatus
AssetDraw::initialize()
{
    AssetDrawTraits &traits = theTraits;

    MFnNumericAttribute nAttr;
    MFnMessageAttribute mAttr;
    MFnTypedAttribute tAttr;
    MStatus stat;

    // input/inputNodeId
    traits.inputNodeId = nAttr.create(
        "inputNodeId", "inputNodeId", MFnNumericData::kInt, -1);
    nAttr.setCached(false);
    nAttr.setStorable(false);
    stat = addAttribute( traits.inputNodeId );

    // shader message
    traits.shader = mAttr.create("shader", "shader");
    stat = addAttribute( traits.shader );

    // shaderfile filepath
    traits.shaderfile = tAttr.create(
        "shaderfile", "shaderfile", MFnData::kString);
    tAttr.setUsedAsFilename(true);
    stat = addAttribute( traits.shaderfile );

    // output
    traits.output = nAttr.create(
	"output", "output", MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    stat = addAttribute( traits.output );

    attributeAffects( traits.inputNodeId, traits.output );

    return MS::kSuccess;
}

MStatus
AssetDraw::initializePlugin( MFnPlugin &plugin, MObject obj )
{
    MStatus   status;

    status = plugin.registerNode(
	"houdiniDraw2",
	AssetDraw::id,
	AssetDraw::creator,
	AssetDraw::initialize,
	MPxNode::kLocatorNode,
	&AssetDraw::drawDbClassification);

    if (!status)
    {
	status.perror("registerNode");
	return status;
    }

    status = MHWRender::MDrawRegistry::registerGeometryOverrideCreator(
	AssetDraw::drawDbClassification,
	AssetDraw::drawRegistrantId,
	AssetDrawGeometryOverride::Creator);

    if (!status)
    {
	status.perror("registerDrawOverrideCreator");
	return status;
    }

    // Register a custom selection mask with priority 2 (same as locators
    // by default).
    MSelectionMask::registerSelectionType("houdiniDrawSelection", 2);
    status = MGlobal::executeCommand("selectType -byName \"houdiniDrawSelection\" 1");
    return status;
}

MStatus
AssetDraw::uninitializePlugin( MFnPlugin &plugin, MObject obj)
{
    MStatus   status;

    status = MHWRender::MDrawRegistry::deregisterGeometryOverrideCreator(
	    AssetDraw::drawDbClassification,
	    AssetDraw::drawRegistrantId);
    if (!status)
    {
	status.perror("deregisterDrawOverrideCreator");
	return status;
    }

    status = releaseShaders();
    if (!status)
    {
	status.perror("releaseShaders");
	return status;
    }

    status = plugin.deregisterNode( AssetDraw::id );
    if (!status)
    {
	status.perror("deregisterNode");
	return status;
    }

    // Deregister custom selection mask
    MSelectionMask::deregisterSelectionType("houdiniDrawSelection");

    return status;
}



AssetDrawTraits AssetDraw::theTraits;

#endif
