#include "AssetSyncAttribute.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <HAPI/HAPI.h>

#include "AssetNode.h"
#include "util.h"

class CreateAttrOperation : public Util::WalkParmOperation
{
    public:
        CreateAttrOperation(
                MFnCompoundAttribute* attrFn,
                const HAPI_NodeInfo &nodeInfo
                );
        ~CreateAttrOperation();

        virtual void pushFolder(const HAPI_ParmInfo &parmInfo);
        virtual void popFolder();

        virtual void leaf(const HAPI_ParmInfo &parmInfo);

    private:
        std::vector<MFnCompoundAttribute*> myAttrFns;
        std::vector<bool> myInvisibles;

        const HAPI_NodeInfo &myNodeInfo;

        MObject createAttr(const HAPI_ParmInfo &parm);
        MObject createStringAttr(const HAPI_ParmInfo &parm, MString& longName, MString& shortName, MString& niceName);
        MObject createNumericAttr(const HAPI_ParmInfo &parm, MString& longName, MString& shortName, MString& niceName);
};

CreateAttrOperation::CreateAttrOperation(
        MFnCompoundAttribute* attrFn,
        const HAPI_NodeInfo &nodeInfo
        ) :
    myNodeInfo(nodeInfo)
{
    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(false);
}

CreateAttrOperation::~CreateAttrOperation()
{
    myAttrFns.pop_back();
    myInvisibles.pop_back();
}

void
CreateAttrOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute* attrFn = NULL;
    bool invisible = myInvisibles.back() || parmInfo.invisible;

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();

    if(!invisible)
    {
        MObject folderAttrObj = createAttr(parmInfo);

        attrFn = new MFnCompoundAttribute(folderAttrObj);
    }

    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(invisible);
}

void
CreateAttrOperation::popFolder()
{
    MFnCompoundAttribute* attrFn = myAttrFns.back();
    bool invisible = myInvisibles.back();

    myAttrFns.pop_back();
    myInvisibles.pop_back();

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();

    if(!invisible)
    {
        parentAttrFn->addChild(attrFn->object());

        delete attrFn;
    }
}

void
CreateAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute* attrFn = myAttrFns.back();
    bool invisible = myInvisibles.back();

    if(!invisible && !parmInfo.invisible)
    {
        MObject attrObj = createAttr(parmInfo);
        attrFn->addChild(attrObj);
    }
}

MObject
CreateAttrOperation::createAttr(const HAPI_ParmInfo &parm)
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
            gAttr.setNiceNameOverride("Separator");
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
CreateAttrOperation::createStringAttr(const HAPI_ParmInfo &parm, MString& longName, MString& shortName, MString& niceName)
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
CreateAttrOperation::createNumericAttr(const HAPI_ParmInfo &parm, MString& longName, MString& shortName, MString& niceName)
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

AssetSyncAttribute::AssetSyncAttribute(
        const MObject &assetNodeObj
        ) :
    myAssetNodeObj(assetNodeObj)
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

    MFnDependencyNode assetNodeFn(myAssetNodeObj);

    // save the existing parameter values
    MStringArray setAttrCmds;
    {
        MPlug houdiniAssetParmPlug = assetNodeFn.findPlug(Util::getParmAttrPrefix(), &status);
        if(status)
        {
            houdiniAssetParmPlug.getSetAttrCmds(setAttrCmds, MPlug::kAll, true);
        }
    }

    MObject houdiniAssetParmObj;

    // delete existing attribute
    houdiniAssetParmObj = assetNodeFn.attribute(Util::getParmAttrPrefix());
    if(!houdiniAssetParmObj.isNull())
    {
        myDGModifier.removeAttribute(myAssetNodeObj, houdiniAssetParmObj);
        myDGModifier.doIt();
    }

    {
        std::vector<HAPI_ParmInfo> parmInfos;
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(myNodeInfo.id, &parmInfos[0], 0, parmInfos.size());

        // create root attribute
        MFnCompoundAttribute attrFn;
        houdiniAssetParmObj = attrFn.create(
                Util::getParmAttrPrefix(),
                Util::getParmAttrPrefix()
                );

        CreateAttrOperation operation(
                reinterpret_cast<MFnCompoundAttribute*>(&reinterpret_cast<char&>(attrFn)),
                myNodeInfo
                );
        Util::walkParm(parmInfos, operation);
    }

    myDGModifier.addAttribute(myAssetNodeObj, houdiniAssetParmObj);

    // restore old parameter values
    status = myDGModifier.commandToExecute("select " + assetNodeFn.name());
    for(unsigned int i = 0; i< setAttrCmds.length(); i++)
    {
        status = myDGModifier.commandToExecute(setAttrCmds[i]);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

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
