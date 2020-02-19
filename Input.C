#include "Input.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MArrayDataBuilder.h>
#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MNodeMessage.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>

#include <HAPI/HAPI.h>

#include "AssetNode.h"
#include "InputMesh.h"
#include "InputCurve.h"
#include "InputParticle.h"
#include "hapiutil.h"
#include "util.h"

Inputs::Inputs(HAPI_NodeId nodeId) :
    myNodeId(nodeId)
{
}

Inputs::~Inputs()
{
    for(AssetInputVector::iterator iter = myAssetInputs.begin();
            iter != myAssetInputs.end();
            iter++)
    {
        delete *iter;
        *iter = NULL;
    }
    myAssetInputs.clear();
}

MStatus
Inputs::compute(MDataBlock &dataBlock)
{
    MStatus status;

    MArrayDataHandle inputArrayHandle = dataBlock.outputArrayValue(AssetNode::input, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MArrayDataBuilder inputArrayBuilder = inputArrayHandle.builder(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i = 0; i < myAssetInputs.size(); ++i)
    {
        MDataHandle inputHandle = inputArrayBuilder.addElement(i, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);


        MDataHandle inputNameHandle = inputHandle.child(AssetNode::inputName);

        HAPI_StringHandle nameSH;
        HAPI_GetNodeInputName(
                Util::theHAPISession.get(),
                myNodeId,
                i,
                &nameSH);

        inputNameHandle.set(Util::HAPIString(nameSH));
    }

    status = inputArrayHandle.set(inputArrayBuilder);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return MStatus::kSuccess;
}

void Inputs::setNumInputs(int numInputs)
{
    // delete any Input that will be removed
    for(unsigned int i = numInputs; i < myAssetInputs.size(); i++)
    {
        Input* &assetInput = myAssetInputs[i];

        if(assetInput)
        {
            delete assetInput;
            assetInput = NULL;
        }
    }

    myAssetInputs.resize(numInputs, NULL);
}

Input::Input() :
    myUnlockNormals(0),
    myMatPerFace(0),
    myAllowFacetSet(0),
    myPreserveScale(false),
    myGeometryNodeId(-1)
{
}

Input::~Input()
{
    removeNameChangedCallback();
}

Input*
Input::createAssetInput(AssetInputType assetInputType)
{
    Input* assetInput = NULL;
    switch(assetInputType)
    {
        case AssetInputType_Mesh:
            assetInput = new InputMesh();
            break;
        case AssetInputType_Curve:
            assetInput = new InputCurve();
            break;
        case AssetInputType_Particle:
            assetInput = new InputParticle();
            break;
        case AssetInputType_Invalid:
            break;
    }
    return assetInput;
}

void
Input::addNameChangedCallback(MObject &node)
{
    MStatus status;

    if(myNameChangedCallbackNode == node)
        return;

    removeNameChangedCallback();

    MObject curNode = node;
    while(true)
    {
        MCallbackId callback = MNodeMessage::addNameChangedCallback(
                curNode,
                nameChangedCallback,
                this,
                &status
                );
        CHECK_MSTATUS(status);
        myNameChangedCallbackIds.append(callback);

        if(!curNode.hasFn(MFn::kDagNode))
            break;

        MFnDagNode nodeFn(curNode);
        if(nodeFn.parentCount() != 1)
            break;

        curNode = nodeFn.parent(0);
    }

    myNameChangedCallbackNode = node;
}

void
Input::removeNameChangedCallback()
{
    for(unsigned int i = 0; i < myNameChangedCallbackIds.length(); i++)
    {
        CHECK_MSTATUS(MMessage::removeCallback(myNameChangedCallbackIds[i]));
    }
    myNameChangedCallbackIds.clear();
}

void
Input::nameChangedCallback(
        MObject &node, const MString &str, void *clientData
        )
{
    Input* input = (Input*)clientData;
    MString cmd;
    cmd.format("dgdirty ^1s", input->myGeoPlug.name());
    MGlobal::executeCommand(cmd);
}

void
Input::setInputName(
        HAPI_AttributeOwner owner, int count,
        const MPlug &plug
        )
{
    myGeoPlug = plug;
    MPlug sourcePlug = Util::plugSource(myGeoPlug);
    MObject sourceNodeObj = sourcePlug.node();

    addNameChangedCallback(sourceNodeObj);

    MString name = Util::getNodeName(sourceNodeObj);

    if(owner == HAPI_ATTROWNER_PRIM)
    {
        CHECK_HAPI(hapiSetPrimAttribute(
                    myGeometryNodeId, 0,
                    1,
                    "name",
                    std::vector<const char*>(count, name.asChar())
                    ));
    }
    else if(owner == HAPI_ATTROWNER_POINT)
    {
        CHECK_HAPI(hapiSetPointAttribute(
                    myGeometryNodeId, 0,
                    1,
                    "name",
                    std::vector<const char*>(count, name.asChar())
                    ));
    }
}

void Input::setUnlockNormals(bool unlockNormals)
{
    myUnlockNormals = unlockNormals;
}


void Input::setMatPerFace(bool matPerFace)
{
    myMatPerFace = matPerFace;
}

void Input::setAllowFacetSet(bool allowFacetSet)
{
    myAllowFacetSet = allowFacetSet;
}

void Input::setPreserveScale(bool preserveScale)
{
    myPreserveScale = preserveScale;
}

void
Input::setInputTransform(MDataHandle &dataHandle)
{
    MMatrix transformMatrix = dataHandle.asMatrix();

    float matrix[16];

    if (myPreserveScale)
    {
        MTransformationMatrix tmat(transformMatrix);
        MVector translation = tmat.getTranslation(MSpace::kWorld) * 0.01;
        tmat.setTranslation(translation, MSpace::kWorld);
        
        tmat.asMatrix().get(reinterpret_cast<float(*)[4]>(matrix));
    }
    else
    {
        transformMatrix.get(reinterpret_cast<float(*)[4]>(matrix));
    }

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler(
            Util::theHAPISession.get(),
            matrix,
            HAPI_SRT,
            HAPI_XYZ,
            &transformEuler
            );

    CHECK_HAPI(HAPI_SetObjectTransform(
            Util::theHAPISession.get(),
            transformNodeId(),
            &transformEuler
            ));
}
void
Input::setInputComponents(
                MDataBlock &dataBlock,
                const MPlug &geoPlug,
                const MPlug &compPlug,
                const MPlug &primGroupPlug,
                const MPlug &pointGroupPlug
		)
{
}
