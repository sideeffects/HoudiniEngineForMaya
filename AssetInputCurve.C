#include "AssetInputCurve.h"

#include <sstream>

#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>

#include <HAPI/HAPI.h>

#include "util.h"

AssetInputCurve::AssetInputCurve(int assetId, int inputIdx) :
    AssetInput( assetId, inputIdx )
{
    int curveAssetId;
    HAPI_CreateCurve(&curveAssetId);

    HAPI_GetAssetInfo(curveAssetId, &myCurveAssetInfo);
    HAPI_GetNodeInfo(myCurveAssetInfo.nodeId, &myCurveNodeInfo);

    HAPI_ConnectAssetGeometry(myCurveAssetInfo.id, 0, myAssetId, myInputIdx);
}

AssetInputCurve::~AssetInputCurve()
{
    HAPI_DisconnectAssetGeometry(myAssetId, myInputIdx);

    HAPI_DestroyAsset(myCurveAssetInfo.id);
}

AssetInputCurve::AssetInputType
AssetInputCurve::assetInputType() const
{
    return AssetInput::AssetInputType_Curve;
}

void
AssetInputCurve::setInputTransform(MDataHandle &dataHandle)
{
    MMatrix transformMatrix = dataHandle.asMatrix();

    float matrix[16];
    transformMatrix.get(reinterpret_cast<float(*)[4]>(matrix));

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler(matrix, HAPI_SRT, HAPI_XYZ, &transformEuler);
    HAPI_SetObjectTransform(myCurveAssetInfo.id, 0, transformEuler);
}

void
AssetInputCurve::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MDataHandle dataHandle = dataBlock.inputValue(plug);

    MObject inputMesh = dataHandle.asNurbsCurve();
    if(inputMesh.isNull())
    {
	return;
    }

    // find coords parm
    std::vector<HAPI_ParmInfo> parms(myCurveNodeInfo.parmCount);
    HAPI_GetParameters(myCurveNodeInfo.id, &parms[0], 0, myCurveNodeInfo.parmCount);
    int typeParmIndex = Util::findParm(parms, "type");
    int coordsParmIndex = Util::findParm(parms, "coords");
    int orderParmIndex = Util::findParm(parms, "order");
    int closeParmIndex = Util::findParm(parms, "close");
    if(coordsParmIndex < 0
	    || coordsParmIndex < 0
	    || orderParmIndex < 0
	    || closeParmIndex < 0)
    {
	return;
    }

    const HAPI_ParmInfo &typeParm = parms[typeParmIndex];
    const HAPI_ParmInfo &coordsParm = parms[coordsParmIndex];
    const HAPI_ParmInfo &orderParm = parms[orderParmIndex];
    const HAPI_ParmInfo &closeParm = parms[closeParmIndex];

    MFnNurbsCurve fnCurve(inputMesh);

    // type
    {
	HAPI_ParmChoiceInfo* choices = new HAPI_ParmChoiceInfo[typeParm.choiceCount];
	HAPI_GetParmChoiceLists(myCurveNodeInfo.id, choices,
		typeParm.choiceIndex, typeParm.choiceCount);

	int nurbsIdx = -1;
	for(int i = 0; i < typeParm.choiceCount; i++)
	{
	    if(Util::getString(choices[i].valueSH) == "nurbs")
	    {
		nurbsIdx = i;
		break;
	    }
	}

	delete [] choices;

	if(nurbsIdx < 0)
	{
	    return;
	}

	HAPI_SetParmIntValues(myCurveNodeInfo.id, &nurbsIdx,
		typeParm.intValuesIndex, 1);
    }

    // coords
    {
	MPointArray cvs;
	fnCurve.getCVs(cvs);

	// Maya has fnCurve.degree() more cvs in it's data definition
	// than houdini for periodic curves--but they are conincident
	// with the first ones. Houdini ignores them, so we don't
	// output them.
	int num_houdini_cvs = cvs.length();
	if (fnCurve.form() == MFnNurbsCurve::kPeriodic)
	    num_houdini_cvs -= fnCurve.degree();

	std::ostringstream coords;
	for(unsigned int i = 0; i < (unsigned int) num_houdini_cvs; i++)
	{
	    const MPoint &pt = cvs[i];

	    coords << pt.x << "," << pt.y << "," << pt.z << " ";
	}
	HAPI_SetParmStringValue(myCurveNodeInfo.id, coords.str().c_str(), coordsParm.id, coordsParm.stringValuesIndex);
    }

    // order
    {
	int order = fnCurve.degree() + 1;

	HAPI_SetParmIntValues(myCurveNodeInfo.id, &order, orderParm.intValuesIndex, 1);
    }

    // periodicity
    {
	int close = fnCurve.form() == MFnNurbsCurve::kPeriodic;
	HAPI_SetParmIntValues(myCurveNodeInfo.id, &close, closeParm.intValuesIndex, 1);
    }
}
