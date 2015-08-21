#include "Input.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MArrayDataBuilder.h>
#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>

#include <HAPI/HAPI.h>

#include "InputAsset.h"
#include "InputMesh.h"
#include "InputCurve.h"
#include "InputParticle.h"
#include "util.h"

MObject Inputs::input;
MObject Inputs::inputName;
MObject Inputs::inputTransform;
MObject Inputs::inputGeo;

MObject
Inputs::createInputAttribute()
{
    MFnCompoundAttribute cAttr;
    MFnMatrixAttribute mAttr;
    MFnGenericAttribute gAttr;
    MFnTypedAttribute tAttr;

    Inputs::inputTransform = mAttr.create(
            "inputTransform", "inputTransform"
            );

    Inputs::inputName = tAttr.create(
            "inputName", "inputName",
            MFnData::kString);

    Inputs::inputGeo = gAttr.create(
            "inputGeo", "inputGeo"
            );
    gAttr.addDataAccept(MFnData::kIntArray);
    gAttr.addDataAccept(MFnData::kMesh);
    gAttr.addDataAccept(MFnData::kNurbsCurve);
    gAttr.addDataAccept(MFnData::kVectorArray);

    Inputs::input = cAttr.create(
            "input", "input"
            );
    cAttr.addChild(Inputs::inputName);
    cAttr.addChild(Inputs::inputTransform);
    cAttr.addChild(Inputs::inputGeo);
    cAttr.setDisconnectBehavior(MFnAttribute::kReset);
    cAttr.setInternal(true);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    return Inputs::input;
}

Inputs::Inputs(int assetId) :
    myAssetId(assetId)
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

    MArrayDataHandle inputArrayHandle = dataBlock.outputArrayValue(Inputs::input, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MArrayDataBuilder inputArrayBuilder = inputArrayHandle.builder(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i = 0; i < myAssetInputs.size(); ++i)
    {
        MDataHandle inputHandle = inputArrayBuilder.addElement(i, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);


        MDataHandle inputNameHandle = inputHandle.child(Inputs::inputName);

        HAPI_StringHandle nameSH;
        HAPI_GetInputName(
                Util::theHAPISession.get(),
                myAssetId,
                i,
                HAPI_INPUT_GEOMETRY,
                &nameSH);

        inputNameHandle.set(
                Util::getString(nameSH)
                );
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

void
Inputs::setInput(
        int inputIdx,
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MStatus status;

    bool isValidInput = true;

    MPlug inputGeoPlug = plug.child(Inputs::inputGeo);

    // Check if the plug is actually connected. The MDataHandle could contain
    // valid geometry from previous connection.
    if(isValidInput)
    {
        // is the inputGeo connected
        MPlugArray plugArray;
        inputGeoPlug.connectedTo(plugArray, true, false);

        isValidInput = plugArray.length() == 1;
    }

    MDataHandle geoDataHandle;
    if(isValidInput)
    {
        // has valid data
        geoDataHandle = dataBlock.inputValue(inputGeoPlug, &status);

        isValidInput = status;
    }

    Input* &assetInput = myAssetInputs[inputIdx];

    if(!isValidInput)
    {
        HAPI_DisconnectAssetGeometry(
                Util::theHAPISession.get(),
                myAssetId,
                inputIdx
                );

        delete assetInput;
        assetInput = NULL;
        return;
    }

    // determine the new input type
    Input::AssetInputType newAssetInputType = Input::AssetInputType_Invalid;
    if(geoDataHandle.type() == MFnData::kIntArray)
    {
        newAssetInputType = Input::AssetInputType_Asset;
    }
    else if(geoDataHandle.type() == MFnData::kMesh)
    {
        newAssetInputType = Input::AssetInputType_Mesh;
    }
    else if(geoDataHandle.type() == MFnData::kNurbsCurve)
    {
        newAssetInputType = Input::AssetInputType_Curve;
    }
    else if(geoDataHandle.type() == MFnData::kVectorArray)
    {
        newAssetInputType = Input::AssetInputType_Particle;
    }

    // if the existing input doesn't match the new input type, delete it
    if(assetInput && assetInput->assetInputType() != newAssetInputType)
    {
        delete assetInput;
        assetInput = NULL;
    }

    // create Input if necessary
    if(!assetInput)
    {
        assetInput = Input::createAssetInput(myAssetId, inputIdx, newAssetInputType);
    }

    if(!assetInput)
    {
        return;
    }

    // set input transform
    MPlug transformPlug = plug.child(Inputs::inputTransform);
    MDataHandle transformHandle = dataBlock.inputValue(transformPlug);
    assetInput->setInputTransform(transformHandle);

    // set input geo
    MPlug geoPlug = plug.child(Inputs::inputGeo);
    assetInput->setInputGeo(dataBlock, geoPlug);
}

Input::Input(int assetId, int inputIdx) :
    myAssetId(assetId),
    myInputIdx(inputIdx)
{
}

Input::~Input()
{
}

Input*
Input::createAssetInput(int assetId, int inputIdx, AssetInputType assetInputType)
{
    Input* assetInput = NULL;
    switch(assetInputType)
    {
        case AssetInputType_Asset:
            assetInput = new InputAsset(assetId, inputIdx);
            break;
        case AssetInputType_Mesh:
            assetInput = new InputMesh(assetId, inputIdx);
            break;
        case AssetInputType_Curve:
            assetInput = new InputCurve(assetId, inputIdx);
            break;
        case AssetInputType_Particle:
            assetInput = new InputParticle(assetId, inputIdx);
            break;
        case AssetInputType_Invalid:
            break;
    }
    return assetInput;
}

template<
    typename T,
    HAPI_StorageType storageType,
    HAPI_Result (SetAttribute)(const HAPI_Session * session,
            HAPI_AssetId asset_id,
            HAPI_ObjectId object_id,
            HAPI_GeoId geo_id,
            const char * name,
            const HAPI_AttributeInfo * attr_info,
            T * data_array,
            int start, int length)
>
void
setDetailAttribute_internal(
        int inputAssetId,
        int inputObjectId,
        int inputGeoId,
        const char* attributeName,
        T* value,
        int tupleSize
        )
{
    HAPI_AttributeInfo attributeInfo;
    attributeInfo.exists = true;
    attributeInfo.owner = HAPI_ATTROWNER_DETAIL;
    attributeInfo.storage = storageType;
    attributeInfo.count = 1;
    attributeInfo.tupleSize = tupleSize;

    HAPI_AddAttribute(
            Util::theHAPISession.get(),
            inputAssetId, inputObjectId, inputGeoId,
            attributeName,
            &attributeInfo
            );

    CHECK_HAPI(SetAttribute(
                Util::theHAPISession.get(),
                inputAssetId, inputObjectId, inputGeoId,
                attributeName,
                &attributeInfo,
                value,
                0, 1
                ));
}

template<>
void
Input::setDetailAttribute(
        int inputAssetId,
        int inputObjectId,
        int inputGeoId,
        const char* attributeName,
        const MStringArray &value
        )
{
    if(value.length() == 0)
        return;

    std::vector<const char*> converted_value;
    converted_value.resize(value.length());
    for(unsigned int i = 0; i < value.length(); i++)
    {
        converted_value[i] = value[i].asChar();
    }

    setDetailAttribute_internal<
        const char*,
        HAPI_STORAGETYPE_STRING,
        HAPI_SetAttributeStringData
            >(
                    inputAssetId,
                    inputObjectId,
                    inputGeoId,
                    attributeName,
                    &converted_value[0],
                    converted_value.size()
             );
}

void
Input::setInputPlugMetaData(
        const MPlug &plug,
        int inputAssetId,
        int inputObjectId,
        int inputGeoId
        )
{
    // maya_source_node
    {
        MString shapeName;

        MPlugArray plugs;
        if(plug.connectedTo(plugs, true, false))
        {
            assert(plugs.length() == 1);
            shapeName = MFnDependencyNode(plugs[0].node()).name();
        }

        if(shapeName.length())
        {
            MStringArray values;
            values.append(shapeName);

            setDetailAttribute(
                    inputAssetId,
                    inputObjectId,
                    inputGeoId,
                    "maya_source_node",
                    values
                    );
        }
    }
}
