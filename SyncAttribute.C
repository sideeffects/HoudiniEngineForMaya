#include "SyncAttribute.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPlugArray.h>
#include <maya/MRampAttribute.h>

#include <HAPI/HAPI.h>

#include "Asset.h"
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

    //MFnCompoundAttribute* parentAttrFn = myAttrFns.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(!invisible)
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
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(!invisible)
    {
        if(parmInfo.rampType != HAPI_RAMPTYPE_INVALID)
        {
            MString attrName = Util::getAttrNameFromParm(parmInfo);

            MObject attrObj;
            if(parmInfo.rampType == HAPI_RAMPTYPE_FLOAT)
            {
                attrObj = MRampAttribute::createCurveRamp(
                        attrName,
                        attrName
                        );
            }
            else if(parmInfo.rampType == HAPI_RAMPTYPE_COLOR)
            {
                attrObj = MRampAttribute::createColorRamp(
                        attrName,
                        attrName
                        );
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
            MString attrName = Util::getAttrNameFromParm(parmInfo);
            MString label = Util::HAPIString(parmInfo.labelSH);

            MFnNumericAttribute sizeAttrFn;
            sizeAttrFn.create(attrName + "__multiSize", attrName + "__multiSize", MFnNumericData::kInt);
            sizeAttrFn.setNiceNameOverride(label);
            parentAttrFn->addChild(sizeAttrFn.object());

            attrFn = new MFnCompoundAttribute();
            MObject compoundAttrObj = attrFn->create(attrName, attrName);
            attrFn->setNiceNameOverride(label);
            attrFn->setArray(true);
            attrFn->setUsesArrayDataBuilder(true);
        }
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

    // for ramp, no need to create anything
    if(parmInfo.isChildOfMultiParm
            && parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)
    {
        invisible = true;
    }

    if(!invisible && !parmInfo.invisible)
    {
        MObject attrObj;

        if((parmInfo.type == HAPI_PARMTYPE_INT
                    || parmInfo.type == HAPI_PARMTYPE_BUTTON
                    || parmInfo.type == HAPI_PARMTYPE_STRING
                    || parmInfo.type == HAPI_PARMTYPE_PATH_FILE
                    || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO
                    || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE)
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
                case HAPI_PARMTYPE_BUTTON:
                case HAPI_PARMTYPE_FLOAT:
                case HAPI_PARMTYPE_COLOR:
                case HAPI_PARMTYPE_TOGGLE:
                    attrObj = createNumericAttr(parmInfo);
                    break;
                case HAPI_PARMTYPE_STRING:
                case HAPI_PARMTYPE_PATH_FILE:
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
configureStringAttribute(
        MFnTypedAttribute &tAttr,
        const HAPI_ParmInfo &parm)
{
    if(HAPI_ParmInfo_IsPath(&parm))
    {
        tAttr.setUsedAsFilename(true);

        MString filterString = Util::HAPIString(parm.typeInfoSH);
        if(filterString.length())
        {
            filterString = filterString + "(" + filterString + ")";
            tAttr.addToCategory("hapiParmFile_filter" + filterString);
        }

        switch(parm.permissions)
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

    if(size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for(int i=0; i<size; i++)
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
            configureStringAttribute(tAttr, parm);

            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(attrName, attrName, MFnData::kString);
    tAttr.setNiceNameOverride(niceName);
    tAttr.setStorable(true);
    configureStringAttribute(tAttr, parm);

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
    switch(parm.type)
    {
        case HAPI_PARMTYPE_TOGGLE:
            type = MFnNumericData::kBoolean;
            break;
        case HAPI_PARMTYPE_INT:
        case HAPI_PARMTYPE_BUTTON:
            type = MFnNumericData::kInt;
            break;
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOR:
            type = MFnNumericData::kFloat;
            break;
        default:
            break;
    }

    if(type == MFnNumericData::kInvalid)
    {
        return result;
    }

    if(parm.type == HAPI_PARMTYPE_COLOR
            && parm.size == 3)
    {
        result = nAttr.createColor(attrName, attrName);
        nAttr.setNiceNameOverride(niceName);
        return result;
    }

    if(parm.size > 1)
    {
        result = cAttr.create(attrName, attrName);
        cAttr.setNiceNameOverride(niceName);
        for(int i = 0; i < parm.size; i++)
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
    if(parm.hasMin)
        nAttr.setMin(parm.min);
    if(parm.hasMax)
        nAttr.setMax(parm.max);
    if(parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if(parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

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

    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parm.choiceCount];
    HAPI_GetParmChoiceLists(
            Util::theHAPISession.get(),
            myNodeInfo.id,
            choiceInfos,
            parm.choiceIndex, parm.choiceCount
            );

    int enumIndex = 0;

    // Button menu items have a dummy field at the beginning.
    if(parm.type == HAPI_PARMTYPE_BUTTON)
    {
        eAttr.addField(niceName, enumIndex++);
    }

    for(int i = 0; i < parm.choiceCount; i++)
    {
        MString field = Util::HAPIString(choiceInfos[i].labelSH);
        eAttr.addField(field, static_cast<short>(enumIndex++));
    }

    delete[] choiceInfos;
    return result;
}

static void
getConnectedChildrenPlugs(MPlugArray &connections, const MPlug &parentPlug)
{
    unsigned int numChildren = parentPlug.numChildren();
    for(unsigned int i = 0; i < numChildren; i++)
    {
        MPlug childPlug = parentPlug.child(i);

        // as destination
        {
            MPlugArray connectedPlugs;
            childPlug.connectedTo(connectedPlugs, true, false);

            unsigned int connectedPlugsLength = connectedPlugs.length();
            for(unsigned int j = 0; j < connectedPlugsLength; ++j)
            {
                connections.append(connectedPlugs[j]);
                connections.append(childPlug);
            }
        }

        // as source
        {
            MPlugArray connectedPlugs;
            childPlug.connectedTo(connectedPlugs, false, true);

            unsigned int connectedPlugsLength = connectedPlugs.length();
            for(unsigned int j = 0; j < connectedPlugsLength; ++j)
            {
                connections.append(childPlug);
                connections.append(connectedPlugs[j]);
            }
        }

        if(childPlug.isCompound())
        {
            getConnectedChildrenPlugs(connections, childPlug);
        }
    }
}

SyncAttribute::SyncAttribute(
        const MObject &assetNodeObj
        ) :
    SubCommandAsset(assetNodeObj)
{
}

SyncAttribute::~SyncAttribute()
{
}

MStatus
SyncAttribute::doIt()
{
    MStatus status;

    GET_COMMAND_ASSET_OR_RETURN_FAIL();

    MFnDagNode assetNodeFn(myAssetNodeObj);

    HAPI_NodeInfo nodeInfo = asset->myNodeInfo;

    // Save the current state of the parameters
    MStringArray setAttrCmds;
    MStringArray connectAttrCmds;
    {
        MPlug houdiniAssetParmPlug = assetNodeFn.findPlug(Util::getParmAttrPrefix(), &status);
        if(status)
        {
            // Save the parameter values
            houdiniAssetParmPlug.getSetAttrCmds(setAttrCmds, MPlug::kAll, true);

            // Save the connections
            {
                MPlugArray connections;
                getConnectedChildrenPlugs(connections, houdiniAssetParmPlug);

                MString connectAttrFormat = "connectAttr ^1s ^2s;";
                unsigned int connectionsLength = connections.length();
                for(unsigned int i = 0; i < connectionsLength; i += 2)
                {
                    MString connectAttrCmd;
                    connectAttrCmd.format(connectAttrFormat,
                            connections[i].name(),
                            connections[i+1].name());
                    connectAttrCmds.append(connectAttrCmd);
                }
            }
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

    if(nodeInfo.parmCount != 0)
    {
        std::vector<HAPI_ParmInfo> parmInfos;
        parmInfos.resize(nodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                nodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        // create root attribute
        MFnCompoundAttribute attrFn;
        houdiniAssetParmObj = attrFn.create(
                Util::getParmAttrPrefix(),
                Util::getParmAttrPrefix()
                );

        CreateAttrOperation operation(
                reinterpret_cast<MFnCompoundAttribute*>(&reinterpret_cast<char&>(attrFn)),
                nodeInfo
                );
        Util::walkParm(parmInfos, operation);

        if(attrFn.numChildren())
        {
            myDGModifier.addAttribute(myAssetNodeObj, houdiniAssetParmObj);
        }
    }

    // Restore the parameter values
    status = myDGModifier.commandToExecute("select " + assetNodeFn.fullPathName());
    for(unsigned int i = 0; i< setAttrCmds.length(); i++)
    {
        status = myDGModifier.commandToExecute(setAttrCmds[i]);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // Restore the connections
    unsigned int connectAttrCmdsLength = connectAttrCmds.length();
    for(unsigned int i = 0; i < connectAttrCmdsLength; ++i)
    {
        status = myDGModifier.commandToExecute(connectAttrCmds[i]);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return redoIt();
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
