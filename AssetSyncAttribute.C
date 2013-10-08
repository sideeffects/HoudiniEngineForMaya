#include "AssetSyncAttribute.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <HAPI/HAPI.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncAttribute::AssetSyncAttribute(
	const MObject &assetNodeObj,
	bool removeExistingAttributes
	) :
    myAssetNodeObj(assetNodeObj),
    myRemoveExistingAttributes( removeExistingAttributes )
{
    MFnDependencyNode assetNodeFn(myAssetNodeObj);

    AssetNode* assetNode = dynamic_cast<AssetNode*>(assetNodeFn.userNode());
    myNodeInfo = assetNode->getAsset()->myNodeInfo;
}

AssetSyncAttribute::~AssetSyncAttribute()
{
}

MStatus
AssetSyncAttribute::doIt()
{
    MStatus status;

    if( myRemoveExistingAttributes )
	removeAllParameterAttributes();
    buildParms();

    return redoIt();
}

MStatus
AssetSyncAttribute::undoIt()
{
    MStatus status;

    status = myDGModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncAttribute::redoIt()
{
    MStatus status;

    status = myDGModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

void
AssetSyncAttribute::addAttrTo(MObject& child, MObject* parent)
{
    if (NULL == parent)
    {
        myDGModifier.addAttribute(myAssetNodeObj, child);
        return;
    }

    MFnCompoundAttribute cAttr(*parent);
    cAttr.addChild(child);
}

void
AssetSyncAttribute::removeAllParameterAttributes()
{
    MFnDependencyNode fnNode( myAssetNodeObj );
    int numAttrs = fnNode.attributeCount();
    MObjectArray attrsToRemove;
    for( int ii = 0; ii < numAttrs; ii++ )
    {
	MObject attr = fnNode.attribute( ii );
	MFnAttribute fnAttr( attr );
	MString attrName = fnAttr.name();
	if( attrName.length() < Util::getParmAttrPrefix().length() )
	    continue;

	if( attrName.substring( 0, Util::getParmAttrPrefix().length() - 1 ) == Util::getParmAttrPrefix() )
	{
	    if( fnAttr.parent() == MObject::kNullObj )
		attrsToRemove.append( attr );
	}
    }

    int numAttrsToRemove = attrsToRemove.length();
    for( int ii = 0; ii < numAttrsToRemove; ii++ )
    {
	MObject attr = attrsToRemove[ ii ];
	myDGModifier.removeAttribute( myAssetNodeObj, attr );	
		
    }
    myDGModifier.doIt();
}

MObject
AssetSyncAttribute::createAttr(HAPI_ParmInfo& parm)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnGenericAttribute gAttr;
    MFnTypedAttribute tAttr;

    MString label = Util::getString(parm.labelSH);
    MString attrName = Util::getAttrNameFromParm(parm);

    MObject result;

    // Other types
    switch(parm.type)
    {
        case HAPI_PARMTYPE_FOLDERLIST:
        case HAPI_PARMTYPE_FOLDER:
            result = cAttr.create(attrName, attrName);
            cAttr.setStorable(true);
            cAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_SEPARATOR:
            result = gAttr.create(attrName, attrName);
            gAttr.setHidden(true);
            gAttr.setStorable(false);
            gAttr.setReadable(false);
            gAttr.setWritable(false);
            gAttr.setConnectable(false);
            gAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_INT:
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOUR:
        case HAPI_PARMTYPE_TOGGLE:
            result = createNumericAttr(parm, attrName, attrName, label);
            break;
        case HAPI_PARMTYPE_STRING:
        case HAPI_PARMTYPE_FILE:
            result = createStringAttr(parm, attrName, attrName, label);
            break;
        default:
            result = nAttr.create(attrName, attrName, MFnNumericData::kFloat, 1.0);
            nAttr.setStorable(true);
            nAttr.setNiceNameOverride(label);
            break;
    }

    return result;
}

MObject
AssetSyncAttribute::createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    int size = parm.size;

    MObject result;

    if (size > 1)
    {
        result = cAttr.create(longName, shortName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = tAttr.create(ln, sn, MFnData::kString);
            tAttr.setNiceNameOverride(nn);
            tAttr.setStorable(true);
            if (parm.type == HAPI_PARMTYPE_FILE)
                tAttr.setUsedAsFilename(true);
            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(longName, shortName, MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setNiceNameOverride(niceName);
    if (parm.type == HAPI_PARMTYPE_FILE)
        tAttr.setUsedAsFilename(true);

    return result;
}

MObject
AssetSyncAttribute::createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnEnumAttribute eAttr;

    MObject result;
    int size = parm.size;
    int choiceCount = parm.choiceCount;

    // Choice list
    if (choiceCount > 0)
    {
        result = eAttr.create(longName, shortName);
        eAttr.setStorable(true);
        eAttr.setNiceNameOverride(niceName);

        HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[choiceCount];
        HAPI_GetParmChoiceLists(myNodeInfo.id, choiceInfos, parm.choiceIndex, choiceCount);
        for (int i=0; i<choiceCount; i++)
        {
            MString field = Util::getString(choiceInfos[i].labelSH);
            eAttr.addField(field, static_cast<short>(i));
        }

	delete[] choiceInfos;
        return result;
    }



    MFnNumericData::Type type;
    if (parm.type == HAPI_PARMTYPE_TOGGLE)
    {
        type = MFnNumericData::kBoolean;
    }
    else if (parm.type == HAPI_PARMTYPE_INT)
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kInt; break;
            case 2: type = MFnNumericData::k2Int; break;
            case 3: type = MFnNumericData::k3Int; break;
            default: type = MFnNumericData::kInt; break;
        }
    } else
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kFloat; break;
            case 2: type = MFnNumericData::k2Float; break;
            case 3: type = MFnNumericData::k3Float; break;
            default: type = MFnNumericData::kFloat; break;
        }
    }

    if (size > 3) {
        result = cAttr.create(longName, shortName);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = nAttr.create(ln, sn, type);
            nAttr.setNiceNameOverride(nn);
            cAttr.addChild(child);
        }
        return result;
    }

    if (parm.type == HAPI_PARMTYPE_COLOUR)
        result = nAttr.createColor(longName, shortName);
    else
        result = nAttr.create(longName, shortName, type);
    nAttr.setStorable(true);
    nAttr.setNiceNameOverride(niceName);

    // TODO: support min/max for all sizes
    if (parm.hasMin)
        nAttr.setMin(parm.min);
    if (parm.hasMax)
        nAttr.setMax(parm.max);
    if (parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if (parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

    return result;
}

void
AssetSyncAttribute::buildParms()
{

    // PARMS
    int parmCount = myNodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myNodeInfo.id, parmInfos, 0, parmCount);

    int index = 0;
    while (index < parmCount)
    {
        int consumed = buildAttrTree(parmInfos, NULL, index, index+1);
        index += consumed;
    }

    delete[] parmInfos;

}

int
AssetSyncAttribute::buildAttrTree(HAPI_ParmInfo* parmInfos, MObject* parent, int current, int start)
{
    HAPI_ParmInfo parm = parmInfos[current];

    if (parm.type == HAPI_PARMTYPE_FOLDERLIST)
    {
        int offset = parm.size;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(parmInfos, parent, i, start + offset);
            offset += count;
        }
        return offset + 1;
    }

    if (parm.type == HAPI_PARMTYPE_FOLDER)
    {
        MObject result = createAttr(parm);
        int offset = 0;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(parmInfos, &result, start + offset, start + offset + 1);
            offset += count;
        }
        addAttrTo(result, parent);
        return offset;
    }

    MObject result = createAttr(parm);
    addAttrTo(result, parent);
    return 1;
}
