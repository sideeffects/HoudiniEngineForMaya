#include <maya/MDGModifier.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>

#include <maya/MFnDagNode.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <memory>

#include "util.h"

//=============================================================================
// HAPIError
//=============================================================================

HAPIError::HAPIError() throw()
    : exception()
    , myMessage("")
{}

HAPIError::HAPIError(const HAPIError & error) throw()
    : exception()
    , myMessage(error.myMessage)
{}

HAPIError::HAPIError(MString msg) throw()
    : exception()
    , myMessage(msg)
{}

const char *
HAPIError::what() const throw()
{
    myBuffer = "******************** HAPI Error ********************\n";
    myBuffer += myMessage;
    myBuffer += "\n****************************************************";
    return myBuffer.asChar();
}

//=============================================================================
// Util
//=============================================================================
void
Util::displayInfoForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayInfo(typeName + ": " + message);
}

void
Util::displayWarningForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayWarning(typeName + ": " + message);
}

void
Util::displayErrorForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayError(typeName + ": " + message);
}

MString
Util::getString(int handle)
{
    int bufLen;
    HAPI_GetStringBufLength(handle, &bufLen);
    char * buffer = new char[bufLen];
    HAPI_GetString(handle, buffer, bufLen);

    MString ret(buffer);
    delete[] buffer;

    return ret;
}

MString
Util::getAttrNameFromParm(const HAPI_ParmInfo &parm)
{
    MString name = getString(parm.templateNameSH);
    if(parm.isChildOfMultiParm)
    {
        name = replaceString(name, "#", "_");
    }

    // If it's a button, add a suffix so that we can distinguish it while
    // populating the AE
    if(parm.type == HAPI_PARMTYPE_BUTTON && parm.choiceCount == 0)
    {
        name += "__button";
    }

    return getParmAttrPrefix() + "_" + name;
}

MString
Util::getParmAttrPrefix()
{
    MString ret = "houdiniAssetParm";
    return ret;
}

bool
Util::hasHAPICallFailed(HAPI_Result stat)
{
    return stat > 0;
}

void
Util::checkHAPIStatus(HAPI_Result stat)
{
    if(hasHAPICallFailed(stat))
    {
        int bufLen;
        HAPI_GetStatusStringBufLength(
            HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS, &bufLen);
        char * buffer = new char[bufLen];
        HAPI_GetStatusString(HAPI_STATUS_CALL_RESULT, buffer);
        throw HAPIError(buffer);
    }
}

MString
Util::escapeString(const MString &str)
{
    MString escapedStr;

    for(unsigned int i = 0; i < str.length(); i++)
    {
        MString ch = str.substringW(i, i);
        if(ch == "\n")
        {
            escapedStr += "\\n";
        }
        else if(ch == "\t")
        {
            escapedStr += "\\t";
        }
        else if(ch == "\b")
        {
            escapedStr += "\\b";
        }
        else if(ch == "\r")
        {
            escapedStr += "\\r";
        }
        else if(ch == "\f")
        {
            escapedStr += "\\f";
        }
        else if(ch == "\v")
        {
            escapedStr += "\\v";
        }
        else if(ch == "\a")
        {
            escapedStr += "\\a";
        }
        else if(ch == "\\")
        {
            escapedStr += "\\\\";
        }
        else if(ch == "\"")
        {
            escapedStr += "\\\"";
        }
        else if(ch == "\'")
        {
            escapedStr += "\\\'";
        }
        else
        {
            escapedStr += ch;
        }
    }

    return escapedStr;
}

Util::ProgressBar::ProgressBar(double waitTimeBeforeShowing) :
    myWaitTimeBeforeShowing(waitTimeBeforeShowing),
    myIsShowing(false)
{
}

Util::ProgressBar::~ProgressBar()
{
}

void
Util::ProgressBar::beginProgress()
{
    myTimer.beginTimer();
}

void
Util::ProgressBar::updateProgress(
        int progress,
        int maxProgress,
        const MString &status
        )
{
    if(!isShowing()
            && elapsedTime() > myWaitTimeBeforeShowing)
    {
        showProgress();
        myIsShowing = true;
    }

    if(!isShowing())
    {
        return;
    }

    displayProgress(progress, maxProgress, status);
}

void
Util::ProgressBar::endProgress()
{
    if(isShowing())
    {
        hideProgress();
        myIsShowing = false;
    }

    myTimer.endTimer();
}

bool
Util::ProgressBar::isShowing() const
{
    return myIsShowing;
}

double
Util::ProgressBar::elapsedTime()
{
    myTimer.endTimer();
    return myTimer.elapsedTime();
}

MString
Util::ProgressBar::elapsedTimeString()
{
    const int time = (int) elapsedTime();

    const int hours = time / (60 * 60) % 60;
    const int minutes = time / 60 % 60;
    const int seconds = time % 60;

    MString timeString;

    if(hours > 0)
    {
        if(hours < 10)
        {
            timeString += "0";
        }

        timeString += hours;
        timeString += ":";
    }

    if(minutes < 10)
    {
        timeString += "0";
    }
    timeString += minutes;
    timeString += ":";

    if(seconds < 10)
    {
        timeString += "0";
    }
    timeString += seconds;

    return timeString;
}

void
Util::ProgressBar::showProgress()
{
}

void
Util::ProgressBar::displayProgress(
        int progress,
        int maxProgress,
        const MString &status
        )
{
}

void
Util::ProgressBar::hideProgress()
{
}

Util::MainProgressBar::MainProgressBar(double waitTimeBeforeShowing) :
    ProgressBar(waitTimeBeforeShowing)
{
}

Util::MainProgressBar::~MainProgressBar()
{
}

void
Util::MainProgressBar::showProgress()
{
    CHECK_MSTATUS(MGlobal::executeCommand("progressBar -edit"
                " -beginProgress"
                " $gMainProgressBar"));
}

void
Util::MainProgressBar::displayProgress(
        int progress,
        int maxProgress,
        const MString &status
        )
{
    if(progress == -1 || maxProgress == -1)
    {
        // unknown progress
        const int maxTicks = 100;
        const double maxTime = 2.0;
        progress = ((int)(elapsedTime() * maxTicks / maxTime)) % maxTicks;
        maxProgress = maxTicks;
    }

    MString cmd;
    CHECK_MSTATUS(cmd.format(
                "progressBar -edit -progress ^1s"
                " -maxValue ^2s"
                " -status \"(^3s) ^4s\""
                " $gMainProgressBar",
                MString() + progress,
                MString() + maxProgress,
                elapsedTimeString(),
                escapeString(status)
                ));
    CHECK_MSTATUS(MGlobal::executeCommand(cmd));
}

void
Util::MainProgressBar::hideProgress()
{
    CHECK_MSTATUS(MGlobal::executeCommand("progressBar -edit -endProgress"
                " $gMainProgressBar"));
}

bool
Util::statusCheckLoop()
{
    HAPI_State state = HAPI_STATE_STARTING_LOAD;
    int currState = (int) state;
    int currCookCount = -1;
    int totalCookCount = -1;

    std::auto_ptr<ProgressBar> progressBar;
    progressBar = std::auto_ptr<ProgressBar>(new MainProgressBar());

    progressBar->beginProgress();

    while(state > HAPI_STATE_MAX_READY_STATE)
    {
            HAPI_GetStatus(HAPI_STATUS_COOK_STATE, &currState);
            state = (HAPI_State) currState;

            if(state == HAPI_STATE_COOKING)
            {
                    HAPI_GetCookingCurrentCount(&currCookCount);
                    HAPI_GetCookingTotalCount(&totalCookCount);
            }
            else
            {
                currCookCount = -1;
                totalCookCount = -1;
            }

            int statusBufSize = 0;
            HAPI_GetStatusStringBufLength(
                HAPI_STATUS_COOK_STATE, HAPI_STATUSVERBOSITY_ERRORS,
                &statusBufSize);

            char * statusBuf = NULL;

            if(statusBufSize > 0)
            {
                statusBuf = new char[statusBufSize];
                HAPI_GetStatusString(HAPI_STATUS_COOK_STATE, statusBuf);
            }

            progressBar->updateProgress(currCookCount, totalCookCount, statusBuf);

            if(statusBuf)
            {
                delete[] statusBuf;
            }

#ifdef _WIN32
            Sleep(100);
#else
            usleep(100);
#endif
    }

    progressBar->endProgress();

    if(state == HAPI_STATE_READY_WITH_FATAL_ERRORS
        || state == HAPI_STATE_READY_WITH_COOK_ERRORS)
    {
        return false;
    }

    return true;
}

MObject
Util::findNodeByName(const MString& name)
{
    MSelectionList selection;
    selection.add(name);

    MObject ret;

    if(selection.length())
        selection.getDependNode(0, ret);
    return ret;
}

MObject
Util::findDagChild(const MFnDagNode &dag, const MString &name)
{
    MObject childObj;

    for(unsigned int i = 0; i < dag.childCount(); i++)
    {
        MObject currObj = dag.child(i);
        MFnDependencyNode currFn(currObj);
        if(currFn.name() == name)
        {
            childObj = currObj;
            break;
        }
    }

    return childObj;
}

MStatus
Util::createNodeByModifierCommand(
        MDGModifier &dgModifier,
        const MString &command,
        MObject &object,
        unsigned int index
        )
{
    MStatus status;

    // clear current selection
    // this ensures we'll get an error if command doesn't result in a selection
    MGlobal::clearSelectionList();

    // run the command
    status = dgModifier.commandToExecute(command);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = dgModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // get the selection
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);
    if(selection.length() <= index)
    {
        return MStatus::kInvalidParameter;
    }

    status = selection.getDependNode(index, object);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MString
Util::replaceString(const MString &str, const MString &searchStr, const MString &replaceStr)
{
    MString remaining = str;
    MString result;

    for(;;)
    {
        int length = remaining.numChars();
        int index = remaining.indexW(searchStr);
        if(index < 0)
        {
            index = length;
        }
        if(index > 0)
        {
            result += remaining.substringW(0, index - 1);
        }
        if(index == length)
        {
            break;
        }
        result += replaceStr;
        remaining = remaining.substringW(index + searchStr.numChars(), length - 1);;
    }

    return result;
}

MStringArray
Util::getAttributeStringData(int assetId,
                            int objectId,
                            int geoId,
                            int partId,
                            HAPI_AttributeOwner owner,
                            const MString & name)
{
    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    HAPI_GetAttributeInfo(assetId, objectId, geoId, partId, name.asChar(), owner, &attr_info);

    MStringArray ret;
    if(!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    int * data = new int[size];
    // zero the array
    for(int j=0; j<size; j++){
        data[j] = 0;
    }
    HAPI_GetAttributeStringData(assetId, objectId, geoId, partId, name.asChar(),
            &attr_info, data, 0, attr_info.count);

    for(int j=0; j<size; j++){
        ret.append(Util::getString(data[j]));
    }

    delete[] data;

    return ret;
}

int
Util::findParm(std::vector<HAPI_ParmInfo>& parms, MString name, int instanceNum)
{
    for(size_t i = 0; i < parms.size(); i++)
    {
        if(getString(parms[i].templateNameSH) == name
                && (instanceNum < 0 || parms[i].instanceNum == instanceNum)
          )
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

Util::WalkParmOperation::WalkParmOperation()
{
}

Util::WalkParmOperation::~WalkParmOperation()
{
}

void
Util::WalkParmOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
}

void
Util::WalkParmOperation::popFolder()
{
}

void
Util::WalkParmOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
}

void
Util::WalkParmOperation::nextMultiparm()
{
}

void
Util::WalkParmOperation::popMultiparm()
{
}

void
Util::WalkParmOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
}

int
walkParmOne(
        const HAPI_ParmInfo* parmInfos,
        Util::WalkParmOperation &operation
        );

int
walkParmMultiple(
        const HAPI_ParmInfo* parmInfos,
        Util::WalkParmOperation &operation,
        unsigned int count
        );

int
walkParmOne(
        const HAPI_ParmInfo* parmInfos,
        Util::WalkParmOperation &operation
        )
{
    int consumed = 0;

    const HAPI_ParmInfo &parmInfo = *parmInfos;
    consumed++;

    if(parmInfo.type == HAPI_PARMTYPE_FOLDERLIST)
    {
        const HAPI_ParmInfo* folderParmInfos = parmInfos + 1 + parmInfo.size;

        // loop over the HAPI_PARMTYPE_FOLDER parms
        for(int i = 0; i < parmInfo.size; i++)
        {
            const HAPI_ParmInfo &folderParmInfo = parmInfos[1 + i];
            consumed++;

            operation.pushFolder(folderParmInfo);

            //folderAttrFn,
            int folderConsumed = walkParmMultiple(
                    folderParmInfos,
                    operation,
                    folderParmInfo.size
                    );
            folderParmInfos += folderConsumed;
            consumed += folderConsumed;

            operation.popFolder();
        }
    }
    else if(parmInfo.type == HAPI_PARMTYPE_MULTIPARMLIST)
    {
        operation.pushMultiparm(parmInfo);

        const HAPI_ParmInfo* multiparmInfos = parmInfos + 1;

        // The parameters of the multiparm won't be listed until there is at
        // least one instance of the multiparm. We create the attributes using
        // the first instance. After creating the attributes with the first
        // instance, we still need to walk through the rest of the instances to
        // find the end.
        for(int i = 0; i < parmInfo.instanceCount; i++)
        {
            int multiparmConsumed = walkParmMultiple(
                    multiparmInfos,
                    operation,
                    parmInfo.instanceLength
                    );
            multiparmInfos += multiparmConsumed;
            consumed += multiparmConsumed;

            operation.nextMultiparm();
        }

        operation.popMultiparm();
    }
    else
    {
        operation.leaf(parmInfo);
    }

    return consumed;
}

int
walkParmMultiple(
        const HAPI_ParmInfo* parmInfos,
        Util::WalkParmOperation &operation,
        unsigned int count
        )
{
    unsigned int consumed = 0;

    for(unsigned int i = 0; i < count; i++)
    {
        int oneConsumed = walkParmOne(parmInfos + consumed, operation);
        consumed += oneConsumed;
    }

    return consumed;
}

void
Util::walkParm(const std::vector<HAPI_ParmInfo> &parmInfos, Util::WalkParmOperation &operation)
{
    size_t index = 0;

    const HAPI_ParmInfo* walkParmInfos = &parmInfos.front();

    while(index < parmInfos.size())
    {
        size_t consumed = walkParmOne(walkParmInfos, operation);
        walkParmInfos += consumed;
        index += consumed;
    }
}
