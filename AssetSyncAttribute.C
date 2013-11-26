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

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
        virtual void popMultiparm();

        virtual void leaf(const HAPI_ParmInfo &parmInfo);

    private:
        std::vector<MFnCompoundAttribute*> myAttrFns;
        std::vector<bool> myInvisibles;
        std::vector<const HAPI_ParmInfo*> myParentParmInfos;

        const HAPI_NodeInfo &myNodeInfo;

        MObject createStringAttr(const HAPI_ParmInfo &parm);
        MObject createNumericAttr(const HAPI_ParmInfo &parm);
        MObject createEnumAttr(const HAPI_ParmInfo &parm);
};

CreateAttrOperation::CreateAttrOperation(
        MFnCompoundAttribute* attrFn,
        const HAPI_NodeInfo &nodeInfo
        ) :
    myNodeInfo(nodeInfo)
{
    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(false);
    myParentParmInfos.push_back(NULL);
}

CreateAttrOperation::~CreateAttrOperation()
{
    myAttrFns.pop_back();
    myInvisibles.pop_back();
    myParentParmInfos.pop_back();
}

void
CreateAttrOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute* attrFn = NULL;
    bool invisible = myInvisibles.back() || parmInfo.invisible;

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();
    const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(!invisible)
    {
        attrFn = new MFnCompoundAttribute();

        MString attrName = Util::getAttrNameFromParm(parmInfo);
        MString niceName = Util::getString(parmInfo.labelSH);

        attrFn->create(attrName, attrName);
        attrFn->setNiceNameOverride(niceName);
    }

    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(invisible);
    myParentParmInfos.push_back(&parmInfo);
}

void
CreateAttrOperation::popFolder()
{
    MFnCompoundAttribute* attrFn = myAttrFns.back();
    bool invisible = myInvisibles.back();

    myAttrFns.pop_back();
    myInvisibles.pop_back();
    myParentParmInfos.pop_back();

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();

    if(!invisible)
    {
        // Maya will crash if there is a compound attribute with no children.
        // As a temporary workaround, avoid creating the attribute if there are
        // no children. Eventually, we should avoid using compound attributes
        // to represent folders.
        if(attrFn->numChildren() > 0)
        {
            parentAttrFn->addChild(attrFn->object());
        }

        delete attrFn;
    }
}

void
CreateAttrOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute* attrFn = NULL;
    bool invisible = myInvisibles.back() || parmInfo.invisible;

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();
    const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(!invisible)
    {
        MString attrName = Util::getAttrNameFromParm(parmInfo);
	MString label = Util::getString(parmInfo.labelSH);

	MFnNumericAttribute sizeAttrFn;
	sizeAttrFn.create(attrName + "__multiSize", attrName + "__multiSize", MFnNumericData::kInt);
	sizeAttrFn.setNiceNameOverride(label);
        sizeAttrFn.setInternal(true);
	parentAttrFn->addChild(sizeAttrFn.object());

        attrFn = new MFnCompoundAttribute();
        MObject compoundAttrObj = attrFn->create(attrName, attrName);
	attrFn->setNiceNameOverride(label);
        attrFn->setArray(true);
        attrFn->setUsesArrayDataBuilder(true);
    }

    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(invisible);
    myParentParmInfos.push_back(&parmInfo);
}

void
CreateAttrOperation::popMultiparm()
{
    MFnCompoundAttribute* attrFn = myAttrFns.back();
    bool invisible = myInvisibles.back();

    myAttrFns.pop_back();
    myInvisibles.pop_back();
    myParentParmInfos.pop_back();

    MFnCompoundAttribute* parentAttrFn = myAttrFns.back();

    if(!invisible)
    {
        // Maya will crash if there is a compound attribute with no children.
        // As a temporary workaround, avoid creating the attribute if there are
        // no children.
        if(attrFn->numChildren() > 0)
        {
            parentAttrFn->addChild(attrFn->object());
        }

        delete attrFn;
    }
}

void
CreateAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute* attrFn = myAttrFns.back();
    bool invisible = myInvisibles.back();
    const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    // for multiparm, only build the first instance
    if(parmInfo.isChildOfMultiParm
            && parmInfo.instanceNum != parentParmInfo->instanceStartOffset)
    {
        invisible = true;
    }

    if(!invisible && !parmInfo.invisible)
    {
        MObject attrObj;

        if((parmInfo.type == HAPI_PARMTYPE_INT || parmInfo.type == HAPI_PARMTYPE_STRING)
                && parmInfo.choiceCount > 0)
        {
            attrObj = createEnumAttr(parmInfo);
        }
        else
        {
            switch(parmInfo.type)
            {
                case HAPI_PARMTYPE_SEPARATOR:
                    {
                        MFnGenericAttribute gAttr;

                        MString attrName = Util::getAttrNameFromParm(parmInfo);

                        attrObj = gAttr.create(attrName, attrName);
                        gAttr.setHidden(true);
                        gAttr.setStorable(false);
                        gAttr.setReadable(false);
                        gAttr.setWritable(false);
                        gAttr.setConnectable(false);
                        gAttr.setNiceNameOverride("Separator");
                    }
                    break;
                case HAPI_PARMTYPE_INT:
                case HAPI_PARMTYPE_FLOAT:
                case HAPI_PARMTYPE_COLOUR:
                case HAPI_PARMTYPE_TOGGLE:
                    attrObj = createNumericAttr(parmInfo);
                    break;
                case HAPI_PARMTYPE_STRING:
                case HAPI_PARMTYPE_FILE:
                    attrObj = createStringAttr(parmInfo);
                    break;
                default:
                    break;
            }
        }

        attrFn->addChild(attrObj);
    }
}

MObject
CreateAttrOperation::createStringAttr(const HAPI_ParmInfo &parm)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::getString(parm.labelSH);

    int size = parm.size;

    MObject result;

    if (size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString childAttrName = attrName + "__tuple" + i;
            MString childNiceName = niceName + " " + i;
            MObject child = tAttr.create(
                    childAttrName,
                    childAttrName,
                    MFnData::kString
                    );
            tAttr.setNiceNameOverride(childNiceName);
            tAttr.setStorable(true);
            if (parm.type == HAPI_PARMTYPE_FILE)
                tAttr.setUsedAsFilename(true);
            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(attrName, attrName, MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setNiceNameOverride(niceName);
    if (parm.type == HAPI_PARMTYPE_FILE)
        tAttr.setUsedAsFilename(true);

    return result;
}

MObject
CreateAttrOperation::createNumericAttr(const HAPI_ParmInfo &parm)
{
    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::getString(parm.labelSH);

    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;

    MObject result;

    MFnNumericData::Type type = MFnNumericData::kInvalid;
    switch(parm.type)
    {
        case HAPI_PARMTYPE_TOGGLE:
            type = MFnNumericData::kBoolean;
            break;
        case HAPI_PARMTYPE_INT:
            type = MFnNumericData::kInt;
            break;
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOUR:
            type = MFnNumericData::kFloat;
            break;
        default:
            break;
    }

    if(type == MFnNumericData::kInvalid)
    {
        return result;
    }

    if(parm.type == HAPI_PARMTYPE_COLOUR
            && parm.size == 3)
    {
        result = nAttr.createColor(attrName, attrName);
        return result;
    }

    if(parm.size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setNiceNameOverride(niceName);
        for (int i = 0; i < parm.size; i++)
        {
            MString childAttrName = attrName + "__tuple" + i;
            MString childNiceName = niceName + " " + i;
            MObject child = nAttr.create(
                    childAttrName,
                    childAttrName,
                    type
                    );
            nAttr.setNiceNameOverride(childNiceName);
            cAttr.addChild(child);
        }
        return result;
    }

    result = nAttr.create(attrName, attrName, type);
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

MObject
CreateAttrOperation::createEnumAttr(const HAPI_ParmInfo &parm)
{
    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::getString(parm.labelSH);

    MFnEnumAttribute eAttr;

    MObject result = eAttr.create(attrName, attrName);
    eAttr.setStorable(true);
    eAttr.setNiceNameOverride(niceName);

    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parm.choiceCount];
    HAPI_GetParmChoiceLists(myNodeInfo.id, choiceInfos, parm.choiceIndex, parm.choiceCount);
    for (int i = 0; i < parm.choiceCount; i++)
    {
        MString field = Util::getString(choiceInfos[i].labelSH);
        eAttr.addField(field, static_cast<short>(i));
    }

    delete[] choiceInfos;
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

    MFnDagNode assetNodeFn(myAssetNodeObj);

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

        if(attrFn.numChildren())
        {
            myDGModifier.addAttribute(myAssetNodeObj, houdiniAssetParmObj);
        }
    }

    // restore old parameter values
    status = myDGModifier.commandToExecute("select " + assetNodeFn.fullPathName());
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
