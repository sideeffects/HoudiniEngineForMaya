#include "AssetInputParticle.h"

#include <maya/MFnAttribute.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MDoubleArray.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MStringArray.h>
#include <maya/MVectorArray.h>

#include "util.h"

AssetInputParticle::AssetInputParticle(int assetId, int inputIdx) :
    AssetInput(assetId, inputIdx),
    myInputAssetId(0)
{
    HAPI_CreateGeoInput(myAssetId, myInputIdx, &myInputInfo);
}

AssetInputParticle::~AssetInputParticle()
{
}

AssetInput::AssetInputType
AssetInputParticle::assetInputType() const
{
    return AssetInput::AssetInputType_Particle;
}

void
AssetInputParticle::setInputTransform(MDataHandle &dataHandle)
{
    MMatrix transformMatrix = dataHandle.asMatrix();

    float matrix[16];
    transformMatrix.get(reinterpret_cast<float(*)[4]>(matrix));

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler(matrix, HAPI_SRT, HAPI_XYZ, &transformEuler);
    HAPI_SetObjectTransform(myInputAssetId, myInputInfo.objectId, transformEuler);
}

void
AssetInputParticle::setAttributePointData(
        const char* attributeName,
        HAPI_StorageType storage,
        int count,
        int tupleSize,
        void* data
        )
{
    HAPI_AttributeInfo attributeInfo;
    attributeInfo.exists = true;
    attributeInfo.owner = HAPI_ATTROWNER_POINT;
    attributeInfo.storage = storage;
    attributeInfo.count = count;
    attributeInfo.tupleSize = tupleSize;

    HAPI_AddAttribute(
            myInputAssetId, myInputInfo.objectId, myInputInfo.geoId,
            attributeName,
            &attributeInfo
            );

    switch(storage)
    {
        case HAPI_STORAGETYPE_FLOAT:
            HAPI_SetAttributeFloatData(
                    myInputAssetId, myInputInfo.objectId, myInputInfo.geoId,
                    attributeName,
                    &attributeInfo,
                    static_cast<float*>(data),
                    0, count
                    );
            break;
        case HAPI_STORAGETYPE_INT:
            HAPI_SetAttributeIntData(
                    myInputAssetId, myInputInfo.objectId, myInputInfo.geoId,
                    attributeName,
                    &attributeInfo,
                    static_cast<int*>(data),
                    0, count
                    );
            break;
        default:
            break;
    }
}

void
AssetInputParticle::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MStatus status;

    // get particle node
    MObject particleObj;
    {
        MPlugArray plugArray;
        plug.connectedTo(plugArray, true, false, &status);
        CHECK_MSTATUS(status);

        if(plugArray.length() == 1)
        {
            particleObj = plugArray[0].node();
        }
    }

    if(particleObj.isNull())
    {
        return;
    }

    MFnParticleSystem particleFn(particleObj);

    // get original particle node
    MObject originalParticleObj;
    // status parameter is needed due to a bug in Maya API
    if(particleFn.isDeformedParticleShape(&status))
    {
        originalParticleObj = particleFn.originalParticleShape(&status);
    }
    else
    {
        originalParticleObj = particleObj;
    }

    MFnParticleSystem originalParticleFn(originalParticleObj);

    // set up part info
    HAPI_PartInfo partInfo;
    HAPI_PartInfo_Init(&partInfo);
    partInfo.id = 0;
    partInfo.faceCount = 0;
    partInfo.vertexCount = 0;
    partInfo.pointCount = particleFn.count();

    HAPI_SetPartInfo(
            myInputAssetId, myInputInfo.objectId, myInputInfo.geoId,
            &partInfo
            );

    // set per-particle attributes
    {
        // shared buffers for passing data
        MVectorArray vectorArray;
        MDoubleArray doubleArray;
        char* data = new char[sizeof(float) * 3 * particleFn.count()];

        // id
        {
            MIntArray ids;
            // Must get the IDs from the original particle node. Maya will
            // crash if we try to get the IDs from the deformed particle node.
            originalParticleFn.particleIds(ids);

            ids.get(reinterpret_cast<int*>(data));

            setAttributePointData(
                    "id",
                    HAPI_STORAGETYPE_INT,
                    particleFn.count(),
                    1,
                    data
                    );
        }

        // 0: vector
        // 1: double
        for(int i = 0; i < 2; i++)
        {
            int tupleSize;
            MString getAttributesCommand = "particle -q ";
            switch(i)
            {
                case 0:
                    tupleSize = 3;
                    getAttributesCommand += "-perParticleVector ";
                    break;
                case 1:
                    tupleSize = 1;
                    getAttributesCommand += "-perParticleDouble ";
                    break;
            }

            // query the original particle for names of the per-particle attributes
            getAttributesCommand += originalParticleFn.fullPathName();

            // get the per-particle attribute names
            MStringArray attributeNames;
            MGlobal::executeCommand(getAttributesCommand, attributeNames);

            for(unsigned int j = 0; j < attributeNames.length(); j++)
            {
                const MString attributeName = attributeNames[j];

                MObject attributeObj = originalParticleFn.attribute(attributeName);
                if(attributeObj.isNull())
                {
                    continue;
                }

                // mimics "listAttr -v -w" from AEokayAttr
                MFnAttribute attributeFn(attributeObj);
                if(!(!attributeFn.isHidden() && attributeFn.isWritable()))
                {
                    continue;
                }

                // get the per-particle data
                switch(i)
                {
                    case 0:
                        if(attributeName == "position")
                        {
                            // Need to use position() so that we get the right
                            // positions in the case of deformed particles.
                            particleFn.position(vectorArray);
                        }
                        else
                        {
                            // Maya will automatically use the original
                            // particle node in the case of deformed particles.
                            particleFn.getPerParticleAttribute(
                                    attributeName,
                                    vectorArray
                                    );
                        }

                        vectorArray.get(reinterpret_cast<float(*)[3]>(data));
                        break;
                    case 1:
                        // Maya will automatically use the original
                        // particle node in the case of deformed particles.
                        particleFn.getPerParticleAttribute(
                                attributeName,
                                doubleArray
                                );

                        doubleArray.get(reinterpret_cast<float(*)>(data));
                        break;
                }

                // map the parameter name
                const char* parameterName = attributeName.asChar();
                switch(i)
                {
                    case 0:
                        if(strcmp(parameterName, "position") == 0)
                        {
                            parameterName = "P";
                        }
                        else if(strcmp(parameterName, "velocity") == 0)
                        {
                            parameterName = "v";
                        }
                        else if(strcmp(parameterName, "acceleration") == 0)
                        {
                            parameterName = "force";
                        }
                        else if(strcmp(parameterName, "rgbPP") == 0)
                        {
                            parameterName = "Cd";
                        }
                        break;
                    case 1:
                        if(strcmp(parameterName, "opacityPP") == 0)
                        {
                            parameterName = "Alpha";
                        }
                        else if(strcmp(parameterName, "radiusPP") == 0)
                        {
                            parameterName = "pscale";
                        }
                        break;
                }

                setAttributePointData(
                        parameterName,
                        HAPI_STORAGETYPE_FLOAT,
                        particleFn.count(),
                        tupleSize,
                        data
                        );
            }

            switch(i)
            {
                case 0:
                    vectorArray.clear();
                    break;
                case 1:
                    doubleArray.clear();
                    break;
            }
        }

        delete [] data;
    }

    // Commit it
    HAPI_CommitGeo(myInputAssetId, myInputInfo.objectId, myInputInfo.geoId);
}
