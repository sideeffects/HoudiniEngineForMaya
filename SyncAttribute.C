#include "SyncAttribute.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPlugArray.h>
#include <maya/MRampAttribute.h>

#include "HoudiniApi.h"

#include "Asset.h"
#include "AssetNode.h"
#include "util.h"

class CreateAttrOperation : public Util::WalkParmOperation
{
public:
    CreateAttrOperation(MFnCompoundAttribute *attrFn,
                        const HAPI_NodeInfo &nodeInfo);
    ~CreateAttrOperation();

    virtual void pushFolder(const HAPI_ParmInfo &parmInfo);
    virtual void popFolder();

    virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
    virtual void popMultiparm();

    virtual void leaf(const HAPI_ParmInfo &parmInfo);

private:
    std::vector<MFnCompoundAttribute *> myAttrFns;
    std::vector<bool> myInvisibles;
    std::vector<const HAPI_ParmInfo *> myParentParmInfos;

    const HAPI_NodeInfo &myNodeInfo;

    MObject createStringAttr(const HAPI_ParmInfo &parm);
    MObject createNumericAttr(const HAPI_ParmInfo &parm);
    MObject createEnumAttr(const HAPI_ParmInfo &parm);

    void handleParmTags(const HAPI_ParmInfo &parm, MFnAttribute &attr,
                        const MString attrName) const;
};

CreateAttrOperation::CreateAttrOperation(MFnCompoundAttribute *attrFn,
                                         const HAPI_NodeInfo &nodeInfo)
    : myNodeInfo(nodeInfo)
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
    MFnCompoundAttribute *attrFn = NULL;
    bool invisible               = myInvisibles.back() || parmInfo.invisible;

    // MFnCompoundAttribute* parentAttrFn = myAttrFns.back();
    // const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if (!invisible)
    {
        attrFn = new MFnCompoundAttribute();

        MString attrName = Util::getAttrNameFromParm(parmInfo);
        MString niceName = Util::HAPIString(parmInfo.labelSH);

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
    MFnCompoundAttribute *attrFn = myAttrFns.back();
    bool invisible               = myInvisibles.back();

    myAttrFns.pop_back();
    myInvisibles.pop_back();
    myParentParmInfos.pop_back();

    MFnCompoundAttribute *parentAttrFn = myAttrFns.back();

    if (!invisible)
    {
        // Maya will crash if there is a compound attribute with no children.
        // As a temporary workaround, avoid creating the attribute if there are
        // no children. Eventually, we should avoid using compound attributes
        // to represent folders.
        if (attrFn->numChildren() > 0)
        {
            parentAttrFn->addChild(attrFn->object());
        }

        delete attrFn;
    }
}

static void getParmTags(const HAPI_NodeInfo &myNodeInfo, const HAPI_ParmInfo &parm,
                        std::vector<MString> &tagNames, std::vector<MString> &tagValues)
{
    tagNames.reserve(parm.tagCount);
    tagValues.reserve(parm.tagCount);

    for (int i = 0; i < parm.tagCount; i++)
    {
        HAPI_StringHandle tagNameSH;
        HoudiniApi::GetParmTagName(Util::theHAPISession.get(), myNodeInfo.id, parm.id,
                            i, &tagNameSH);

        MString tagName = Util::HAPIString(tagNameSH);

        HAPI_StringHandle tagValueSH;
        HoudiniApi::GetParmTagValue(Util::theHAPISession.get(), myNodeInfo.id, parm.id,
                             tagName.asChar(), &tagValueSH);

        MString tagValue = Util::HAPIString(tagValueSH);

        tagNames.emplace_back(tagName.toLowerCase());
        tagValues.emplace_back(tagValue);
    }
}

void
CreateAttrOperation::handleParmTags(const HAPI_ParmInfo &parm, MFnAttribute &attr,
                                    const MString attrName) const
{
    std::vector<MString> tagNames;
    std::vector<MString> tagValues;

    getParmTags(myNodeInfo, parm, tagNames, tagValues);

    for (size_t i = 0; i < tagNames.size(); i++)
    {
        if (tagNames[i] == "sidefx::maya_component_selection_type")
        {
            // Parm supports component selection
            if (tagValues[i] == "vertex")
                attr.addToCategory("hapiParmString_selectVertex");
            else if (tagValues[i] == "edge")
                attr.addToCategory("hapiParmString_selectEdge");
            else if (tagValues[i] == "face")
                attr.addToCategory("hapiParmString_selectFace");
            else if (tagValues[i] == "uv")
                attr.addToCategory("hapiParmString_selectUV");
            else
            {
                DISPLAY_WARNING("Unknown selection type \"^1s\" requested for "
                                "\"^2s\". Valid types are: \"vertex\", \"edge\", "
                                "\"face\" and \"uv\".\n", tagValues[i], attrName);
            }
        }
        if (tagNames[i] == "sidefx::maya_parm_affects_others")
        {
            // Parm affects others, so we must force an attribute sync
            attr.addToCategory("hapiParm_affectsOthers");
        }
        else if (tagNames[i] == "sidefx::maya_parm_syncs_asset")
        {
            // Parm causes an asset sync
            attr.addToCategory("hapiParm_syncsAsset");
        }
        else if (tagNames[i] == "sidefx::maya_parm_callback_func")
        {
            // Parm supplied code to be executed. We add the prefix so that we
            // can parse what type of tag it is from a single string.
            attr.addToCategory(MAYA_PARM_CALLBACK_FUNC_PREFIX + tagValues[i]);
        }
        else if (tagNames[i] == "sidefx::maya_parm_callback_func_language")
        {
            // The language of the callback function, which defaults to MEL. 
            // See comment above as to why we use a prefix.
            MString tag_ms = tagValues[i].toLowerCase();
            std::string tag(tag_ms.asChar());
            MString lang;

            if (tag.rfind("python", 0) == 0)
                lang = "python";
            else if (tag.rfind("mel", 0) == 0)
                lang = "mel";
            else
            {
                DISPLAY_WARNING("Unknown callback function language: \"^1s\". "
                                "Valid types are: \"mel\" and \"python\".\n",
                                tag_ms);
                continue;
            }

            attr.addToCategory(MAYA_PARM_CALLBACK_FUNC_LANG_PREFIX + tag_ms);
        }
    }
}

void
CreateAttrOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute *attrFn = NULL;
    bool invisible               = myInvisibles.back() || parmInfo.invisible;

    MFnCompoundAttribute *parentAttrFn = myAttrFns.back();
    // const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if (!invisible)
    {
        MString attrName = "";

        if (parmInfo.rampType != HAPI_RAMPTYPE_INVALID)
        {
            attrName = Util::getAttrNameFromParm(parmInfo);

            MObject attrObj;
            if (parmInfo.rampType == HAPI_RAMPTYPE_FLOAT)
            {
                attrObj = MRampAttribute::createCurveRamp(attrName, attrName);
            }
            else if (parmInfo.rampType == HAPI_RAMPTYPE_COLOR)
            {
                attrObj = MRampAttribute::createColorRamp(attrName, attrName);
            }

            attrFn = new MFnCompoundAttribute(attrObj);

            // Workaround a bug when saving/loading scene files. When the first
            // point of the ramp is at default values (i.e. all zeroes), Maya
            // would skip writing the values out to file. Then, when the file is
            // loaded back, the first point would disappear. Workaround this bug
            // by changing the default position to an impossible value, so that
            // Maya will always write out the values.
            {
                MFnNumericAttribute posAttrFn(attrFn->child(0));
                CHECK_MSTATUS(posAttrFn.setDefault(-1.0f));
            }

            MString niceName = Util::HAPIString(parmInfo.labelSH);
            attrFn->setNiceNameOverride(niceName);

            parentAttrFn->addChild(attrObj);
        }
        else
        {
            attrName = Util::getAttrNameFromParm(parmInfo);
            MString label    = Util::HAPIString(parmInfo.labelSH);

            MFnNumericAttribute sizeAttrFn;
            sizeAttrFn.create(attrName + "__multiSize",
                              attrName + "__multiSize", MFnNumericData::kInt);
            sizeAttrFn.setNiceNameOverride(label);
            parentAttrFn->addChild(sizeAttrFn.object());

            attrFn                  = new MFnCompoundAttribute();
            MObject compoundAttrObj = attrFn->create(attrName, attrName);
            attrFn->setNiceNameOverride(label);
            attrFn->setArray(true);
            attrFn->setUsesArrayDataBuilder(true);
        }

        if (attrFn)
            handleParmTags(parmInfo, *attrFn, attrName);
    }

    myAttrFns.push_back(attrFn);
    myInvisibles.push_back(invisible);
    myParentParmInfos.push_back(&parmInfo);
}

void
CreateAttrOperation::popMultiparm()
{
    MFnCompoundAttribute *attrFn = myAttrFns.back();
    bool invisible               = myInvisibles.back();

    myAttrFns.pop_back();
    myInvisibles.pop_back();
    myParentParmInfos.pop_back();

    MFnCompoundAttribute *parentAttrFn = myAttrFns.back();

    if (!invisible)
    {
        // Maya will crash if there is a compound attribute with no children.
        // As a temporary workaround, avoid creating the attribute if there are
        // no children.
        if (attrFn->numChildren() > 0)
        {
            parentAttrFn->addChild(attrFn->object());
        }

        delete attrFn;
    }
}

void
CreateAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MFnCompoundAttribute *attrFn         = myAttrFns.back();
    bool invisible                       = myInvisibles.back();
    const HAPI_ParmInfo *&parentParmInfo = myParentParmInfos.back();

    // for multiparm, only build the first instance
    if (parmInfo.isChildOfMultiParm &&
        parmInfo.instanceNum != parentParmInfo->instanceStartOffset)
    {
        invisible = true;
    }

    // for ramp, no need to create anything
    if (parmInfo.isChildOfMultiParm &&
        parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)
    {
        invisible = true;
    }

    if (!invisible && !parmInfo.invisible)
    {
        MObject attrObj;

        if ((parmInfo.type == HAPI_PARMTYPE_INT ||
             parmInfo.type == HAPI_PARMTYPE_BUTTON ||
             parmInfo.type == HAPI_PARMTYPE_STRING ||
             parmInfo.type == HAPI_PARMTYPE_PATH_FILE ||
             parmInfo.type == HAPI_PARMTYPE_PATH_FILE_DIR ||
             parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO ||
             parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE) &&
            parmInfo.choiceCount > 0)
        {
            attrObj = createEnumAttr(parmInfo);
        }
        else
        {
            switch (parmInfo.type)
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
            case HAPI_PARMTYPE_BUTTON:
            case HAPI_PARMTYPE_FLOAT:
            case HAPI_PARMTYPE_COLOR:
            case HAPI_PARMTYPE_TOGGLE:
            case HAPI_PARMTYPE_NODE:
                attrObj = createNumericAttr(parmInfo);
                break;
            case HAPI_PARMTYPE_STRING:
            case HAPI_PARMTYPE_PATH_FILE:
            case HAPI_PARMTYPE_PATH_FILE_DIR:
            case HAPI_PARMTYPE_PATH_FILE_GEO:
            case HAPI_PARMTYPE_PATH_FILE_IMAGE:
                attrObj = createStringAttr(parmInfo);
                break;
            default:
                break;
            }
        }

        attrFn->addChild(attrObj);
    }
}

static void
configureStringAttribute(MFnTypedAttribute &tAttr, const HAPI_ParmInfo &parm)
{
    if (HoudiniApi::ParmInfo_IsPath(&parm))
    {
        tAttr.setUsedAsFilename(true);

        if (parm.typeInfoSH)
        {
            MString filterString = Util::HAPIString(parm.typeInfoSH);
            if (filterString.length())
            {
                filterString = filterString + "(" + filterString + ")";
                tAttr.addToCategory("hapiParmFile_filter" + filterString);
            }
        }
        switch (parm.permissions)
        {
        case HAPI_PERMISSIONS_READ_WRITE:
        case HAPI_PERMISSIONS_WRITE_ONLY:
            tAttr.addToCategory("hapiParmFile_write");
            break;
        case HAPI_PERMISSIONS_READ_ONLY:
            tAttr.addToCategory("hapiParmFile_read");
            break;
        default:
            break;
        }
        if (parm.type == HAPI_PARMTYPE_PATH_FILE_DIR)
        {
            tAttr.addToCategory("hapiParmFile_directory");
        }
    }
}

MObject
CreateAttrOperation::createStringAttr(const HAPI_ParmInfo &parm)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::HAPIString(parm.labelSH);

    int size = parm.size;

    MObject result;

    if (size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for (int i = 0; i < size; i++)
        {
            MString childAttrName = attrName + "__tuple" + i;
            MString childNiceName = niceName + " " + i;

            MObject child = tAttr.create(
                childAttrName, childAttrName, MFnData::kString);
            tAttr.setNiceNameOverride(childNiceName);
            tAttr.setStorable(true);
            configureStringAttribute(tAttr, parm);
            handleParmTags(parm, tAttr, childAttrName);

            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(attrName, attrName, MFnData::kString);
    tAttr.setNiceNameOverride(niceName);
    tAttr.setStorable(true);
    configureStringAttribute(tAttr, parm);

    handleParmTags(parm, tAttr, attrName);

    return result;
}

MObject
CreateAttrOperation::createNumericAttr(const HAPI_ParmInfo &parm)
{
    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::HAPIString(parm.labelSH);

    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;

    MObject result;

    MFnNumericData::Type type = MFnNumericData::kInvalid;
    switch (parm.type)
    {
    case HAPI_PARMTYPE_TOGGLE:
        type = MFnNumericData::kBoolean;
        break;
    case HAPI_PARMTYPE_INT:
    case HAPI_PARMTYPE_BUTTON:
    case HAPI_PARMTYPE_NODE:
        type = MFnNumericData::kInt;
        break;
    case HAPI_PARMTYPE_FLOAT:
    case HAPI_PARMTYPE_COLOR:
        type = MFnNumericData::kFloat;
        break;
    default:
        break;
    }

    if (type == MFnNumericData::kInvalid)
    {
        return result;
    }

    if (parm.type == HAPI_PARMTYPE_COLOR && parm.size == 3)
    {
        result = nAttr.createColor(attrName, attrName);
        nAttr.setNiceNameOverride(niceName);
        nAttr.setChannelBox(true);
        handleParmTags(parm, nAttr, attrName);
        return result;
    }

    if (parm.size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setNiceNameOverride(niceName);
        for (int i = 0; i < parm.size; i++)
        {
            MString childAttrName = attrName + "__tuple" + i;
            MString childNiceName = niceName + " " + i;
            MObject child = nAttr.create(childAttrName, childAttrName, type);
            nAttr.setNiceNameOverride(childNiceName);
            nAttr.setChannelBox(true);
            handleParmTags(parm, nAttr, childAttrName);
            cAttr.addChild(child);
        }
        return result;
    }

    result = nAttr.create(attrName, attrName, type);
    nAttr.setNiceNameOverride(niceName);
    nAttr.setChannelBox(true);

    if (parm.type == HAPI_PARMTYPE_NODE)
    {
        // NODE parms attributes don't need to be stored for their value
        // since they are only meaningful if connected
        // however if Maya chooses to optimize storing a compound with
        // all numeric elements, a non-storable element confuses the
        // optimization and can cause data loss
        nAttr.setDefault(-1);
        nAttr.setDisconnectBehavior(MFnAttribute::kReset);
        nAttr.setCached(false);
        nAttr.setStorable(true);
    }

    // TODO: support min/max for all sizes
    if (parm.hasMin)
        nAttr.setMin(parm.min);
    if (parm.hasMax)
        nAttr.setMax(parm.max);
    if (parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if (parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

    handleParmTags(parm, nAttr, attrName);

    return result;
}

MObject
CreateAttrOperation::createEnumAttr(const HAPI_ParmInfo &parm)
{
    MString attrName = Util::getAttrNameFromParm(parm);
    MString niceName = Util::HAPIString(parm.labelSH);

    MFnEnumAttribute eAttr;

    MObject result = eAttr.create(attrName, attrName);
    eAttr.setStorable(true);
    eAttr.setNiceNameOverride(niceName);

    HAPI_ParmChoiceInfo *choiceInfos =
        new HAPI_ParmChoiceInfo[parm.choiceCount];
    HoudiniApi::GetParmChoiceLists(Util::theHAPISession.get(), myNodeInfo.id,
                            choiceInfos, parm.choiceIndex, parm.choiceCount);

    int enumIndex = 0;

    // Button menu items have a dummy field at the beginning.
    if (parm.type == HAPI_PARMTYPE_BUTTON)
    {
        eAttr.addField(niceName, enumIndex++);
    }

    for (int i = 0; i < parm.choiceCount; i++)
    {
        MString field = Util::HAPIString(choiceInfos[i].labelSH);
        eAttr.addField(field, static_cast<short>(enumIndex++));
    }
    eAttr.setChannelBox(true);
    handleParmTags(parm, eAttr, attrName);
    delete[] choiceInfos;
    return result;
}

static void
getConnectedChildrenPlugs(MPlugArray &connections, const MPlug &plug)
{
    std::vector<MPlug> plugsToTraverse;
    plugsToTraverse.push_back(plug);

    while (plugsToTraverse.size())
    {
        const MPlug currentPlug = plugsToTraverse.back();
        plugsToTraverse.pop_back();

        // as destination
        {
            MPlug srcPlug = Util::plugSource(currentPlug);

            if (!srcPlug.isNull())
            {
                connections.append(srcPlug);
                connections.append(currentPlug);
            }
        }

        // as source
        {
            MPlugArray connectedPlugs = Util::plugDestination(currentPlug);

            unsigned int connectedPlugsLength = connectedPlugs.length();
            for (unsigned int j = 0; j < connectedPlugsLength; ++j)
            {
                connections.append(currentPlug);
                connections.append(connectedPlugs[j]);
            }
        }

        if (currentPlug.isArray())
        {
            for (unsigned int i = currentPlug.numElements(); i-- > 0;)
            {
                plugsToTraverse.push_back(
                    currentPlug.elementByPhysicalIndex(i));
            }
        }
        else if (currentPlug.isCompound())
        {
            for (unsigned int i = currentPlug.numChildren(); i-- > 0;)
            {
                plugsToTraverse.push_back(currentPlug.child(i));
            }
        }
    }
}

static void
unlockChildPlugs(MPlug &plug)
{
    std::vector<MPlug> plugsToTraverse;
    plugsToTraverse.push_back(plug);

    while (plugsToTraverse.size())
    {
        MPlug currentPlug = plugsToTraverse.back();
        plugsToTraverse.pop_back();

        // as destination
        {
            if (currentPlug.isLocked())
            {
                // Ideally we woud do this with executeCommand since it needs to
                // be undoable but any attempt to access the attr from after the
                // undo triggers updating the attr state from the parms so you'd
                // never actually see the unlock getting undone, so skip the
                // extra overhead for now if we change the way we do the locking
                // for disabled parms,  this may need to be re-enabled again.

                // MString unlockFormat = "setAttr -l false ^1s";
                // MString setAttrCmd;
                // setAttrCmd.format(unlockFormat, currentPlug.name());
                // MGlobal::executeCommand(setAttrCmd);
                currentPlug.setLocked(false);
            }
        }

        if (currentPlug.isArray())
        {
            for (unsigned int i = currentPlug.numElements(); i-- > 0;)
            {
                plugsToTraverse.push_back(
                    currentPlug.elementByPhysicalIndex(i));
            }
        }
        else if (currentPlug.isCompound())
        {
            for (unsigned int i = currentPlug.numChildren(); i-- > 0;)
            {
                plugsToTraverse.push_back(currentPlug.child(i));
            }
        }
    }
}

SyncAttribute::SyncAttribute(const MObject &assetNodeObj)
    : SubCommandAsset(assetNodeObj)
{
}

SyncAttribute::~SyncAttribute() {}

MStatus
SyncAttribute::doIt()
{
    MStatus status;

    GET_COMMAND_ASSET_OR_RETURN_FAIL();

    MFnDagNode assetNodeFn(myAssetNodeObj);
    AssetNode *assetNode = dynamic_cast<AssetNode *>(assetNodeFn.userNode());

    MObject houdiniAssetParmObj =
        assetNodeFn.attribute(Util::getParmAttrPrefix());

    // Save the values
    assetNode->setParmValues();

    // Save the connections
    MStringArray connectAttrCmds;
    if (!houdiniAssetParmObj.isNull())
    {
        MPlugArray connections;
        MPlug parmPlug = assetNodeFn.findPlug(
            houdiniAssetParmObj, true, &status);
        getConnectedChildrenPlugs(connections, parmPlug);
        unlockChildPlugs(parmPlug);

        MString connectAttrFormat      = "connectAttr ^1s ^2s;";
        unsigned int connectionsLength = connections.length();
        for (unsigned int i = 0; i < connectionsLength; i += 2)
        {
            MString connectAttrCmd;
            connectAttrCmd.format(connectAttrFormat, connections[i].name(),
                                  connections[i + 1].name());
            connectAttrCmds.append(connectAttrCmd);
        }
    }

    // delete existing attribute
    if (!houdiniAssetParmObj.isNull())
    {
        myDGModifier.removeAttribute(myAssetNodeObj, houdiniAssetParmObj);
        myDGModifier.doIt();
    }

    HAPI_NodeInfo nodeInfo = asset->getNodeInfo();
    if (nodeInfo.parmCount != 0)
    {
        std::vector<HAPI_ParmInfo> parmInfos;
        parmInfos.resize(nodeInfo.parmCount);
        HoudiniApi::GetParameters(Util::theHAPISession.get(), nodeInfo.id,
                           &parmInfos[0], 0, parmInfos.size());

        // create root attribute
        MFnCompoundAttribute attrFn;
        houdiniAssetParmObj = attrFn.create(
            Util::getParmAttrPrefix(), Util::getParmAttrPrefix());
        attrFn.setInternal(true);

        CreateAttrOperation operation(reinterpret_cast<MFnCompoundAttribute *>(
                                          &reinterpret_cast<char &>(attrFn)),
                                      nodeInfo);
        Util::walkParm(parmInfos, operation);

        if (attrFn.numChildren())
        {
            myDGModifier.addAttribute(myAssetNodeObj, houdiniAssetParmObj);
        }
    }

    // Restore the connections
    unsigned int connectAttrCmdsLength = connectAttrCmds.length();
    for (unsigned int i = 0; i < connectAttrCmdsLength; ++i)
    {
        status = myDGModifier.commandToExecute(connectAttrCmds[i]);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // execute all the DG commands for recreating the attributes
    // and connections. Then restore the attribute values once
    // the attributes exist again.

    status = myDGModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);
    assetNode->getParmValues();

    return MStatus::kSuccess;
}

MStatus
SyncAttribute::undoIt()
{
    MStatus status;

    status = myDGModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncAttribute::redoIt()
{
    MStatus status;

    status = myDGModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool
SyncAttribute::isUndoable() const
{
    return true;
}

