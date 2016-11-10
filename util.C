#include <maya/MDGModifier.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
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

namespace Util
{
std::auto_ptr<HAPISession> theHAPISession;

void
displayInfoForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayInfo(typeName + ": " + message);
}

void
displayWarningForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayWarning(typeName + ": " + message);
}

void
displayErrorForNode(
        const MString &typeName,
        const MString &message
        )
{
    MGlobal::displayError(typeName + ": " + message);
}

MString
getAttrNameFromParm(const HAPI_ParmInfo &parm)
{
    MString name = HAPIString(parm.templateNameSH);
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
    else if(parm.type == HAPI_PARMTYPE_FOLDER)
    {
        name += "__folder";
    }
    else if(parm.type == HAPI_PARMTYPE_NODE)
    {
        name += "__node";
    }
    else if(parm.rampType != HAPI_RAMPTYPE_INVALID)
    {
        name += "__ramp";
    }

    return getParmAttrPrefix() + "_" + name;
}

MString
getAttrNameFromParm(
        const HAPI_ParmInfo &parm,
        const HAPI_ParmInfo *parentParm
        )
{
    if(parm.isChildOfMultiParm
            && parentParm
            && parentParm->rampType != HAPI_RAMPTYPE_INVALID)
    {
        // Map the parameters of a Houdini ramp to the equivalent attributes of
        // a Maya ramp.
        MString name = HAPIString(parm.templateNameSH);
        name = replaceString(name, "#", "_");

        if(endsWith(name, "pos"))
        {
            name = getAttrNameFromParm(*parentParm) + "_Position";
        }
        else if(endsWith(name, "value"))
        {
            name = getAttrNameFromParm(*parentParm) + "_FloatValue";
        }
        else if(endsWith(name, "c"))
        {
            name = getAttrNameFromParm(*parentParm) + "_Color";
        }
        else if(endsWith(name, "interp"))
        {
            name = getAttrNameFromParm(*parentParm) + "_Interp";
        }

        return name;
    }

    return getAttrNameFromParm(parm);
}

MString
getParmAttrPrefix()
{
    MString ret = "houdiniAssetParm";
    return ret;
}

bool
hasHAPICallFailed(HAPI_Result stat)
{
    return stat > 0;
}

PythonInterpreterLock::PythonInterpreterLock()
{
    HAPI_PythonThreadInterpreterLock(
            theHAPISession.get(),
            true
            );
}

PythonInterpreterLock::~PythonInterpreterLock()
{
    HAPI_PythonThreadInterpreterLock(
            theHAPISession.get(),
            false
            );
}

bool
startsWith(const MString &str, const MString &start)
{
    unsigned int strLength = str.length();
    unsigned int startLength = start.length();
    if(strLength < startLength)
        return false;

    return str.substring(0, startLength - 1) == start;
}

bool
endsWith(const MString &str, const MString &end)
{
    unsigned int strLength = str.length();
    unsigned int endLength = end.length();
    if(strLength < endLength)
        return false;

    return str.substring(strLength - endLength, strLength - 1) == end;
}

MString
escapeString(const MString &str)
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

ProgressBar::ProgressBar(double waitTimeBeforeShowing) :
    myWaitTimeBeforeShowing(waitTimeBeforeShowing),
    myIsShowing(false)
{
}

ProgressBar::~ProgressBar()
{
}

void
ProgressBar::beginProgress()
{
    myTimer.beginTimer();
}

void
ProgressBar::updateProgress(
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
ProgressBar::endProgress()
{
    if(isShowing())
    {
        hideProgress();
        myIsShowing = false;
    }

    myTimer.endTimer();
}

bool
ProgressBar::isInterrupted()
{
    if(!isShowing())
    {
        return false;
    }

    return checkInterrupted();
}

bool
ProgressBar::isShowing() const
{
    return myIsShowing;
}

double
ProgressBar::elapsedTime()
{
    myTimer.endTimer();
    return myTimer.elapsedTime();
}

MString
ProgressBar::elapsedTimeString()
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
ProgressBar::showProgress()
{
}

void
ProgressBar::displayProgress(
        int progress,
        int maxProgress,
        const MString &status
        )
{
}

void
ProgressBar::hideProgress()
{
}

bool
ProgressBar::checkInterrupted()
{
    return false;
}

MainProgressBar::MainProgressBar(double waitTimeBeforeShowing) :
    ProgressBar(waitTimeBeforeShowing)
{
}

MainProgressBar::~MainProgressBar()
{
}

void
MainProgressBar::showProgress()
{
    CHECK_MSTATUS(MGlobal::executeCommand("progressBar -edit"
                " -beginProgress"
                " -isInterruptable true"
                " $gMainProgressBar"));
}

void
MainProgressBar::displayProgress(
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

bool
MainProgressBar::checkInterrupted()
{
    int interrupted;

    CHECK_MSTATUS(MGlobal::executeCommand("progressBar -query"
                " -isCancelled"
                " $gMainProgressBar",
                interrupted));

    return interrupted != 0;
}

void
MainProgressBar::hideProgress()
{
    CHECK_MSTATUS(MGlobal::executeCommand("progressBar -edit -endProgress"
                " $gMainProgressBar"));
}

LogProgressBar::LogProgressBar(
        double timeBetweenLog,
        double waitTimeBeforeShowing
        ) :
    myTimeBetweenLog(timeBetweenLog),
    myLastPrintedTime(0.0)
{
}

LogProgressBar::~LogProgressBar()
{
}

void
LogProgressBar::showProgress()
{
    myComputation.beginComputation();
}

void
LogProgressBar::displayProgress(
        int progress,
        int maxProgress,
        const MString &status
        )
{
    const double elapsedTimeTemp = elapsedTime();

    if(elapsedTimeTemp < myLastPrintedTime + myTimeBetweenLog)
    {
        return;
    }

    MString progressString;
    if(progress != -1 && maxProgress != -1)
    {
        progressString.format("(^1s%) ",
                MString() + (int) ((progress / (float) maxProgress) * 100.0f));
    }

    MString message;
    message.format(
            "^1s(^2s) ^3s",
            progressString,
            elapsedTimeString(),
            status
            );
    MGlobal::displayInfo(message);

    myLastPrintedTime = elapsedTimeTemp;
}

bool
LogProgressBar::checkInterrupted()
{
    return myComputation.isInterruptRequested();
}

void
LogProgressBar::hideProgress()
{
    myComputation.endComputation();
}

bool
statusCheckLoop(bool wantMainProgressBar)
{
    HAPI_State state = HAPI_STATE_STARTING_LOAD;
    int currState = (int) state;
    int currCookCount = -1;
    int totalCookCount = -1;

    std::auto_ptr<ProgressBar> progressBar;
    if(MGlobal::mayaState() == MGlobal::kInteractive
            && wantMainProgressBar)
    {
        progressBar = std::auto_ptr<ProgressBar>(new MainProgressBar());
    }
    else
    {
        progressBar = std::auto_ptr<ProgressBar>(new LogProgressBar());
    }

    progressBar->beginProgress();

    while(state > HAPI_STATE_MAX_READY_STATE)
    {
            HAPI_GetStatus(
                    theHAPISession.get(),
                    HAPI_STATUS_COOK_STATE,
                    &currState
                    );
            state = (HAPI_State) currState;

            if(state == HAPI_STATE_COOKING)
            {
                    HAPI_GetCookingCurrentCount(
                            theHAPISession.get(),
                            &currCookCount
                            );
                    HAPI_GetCookingTotalCount(
                            theHAPISession.get(),
                            &totalCookCount
                            );
            }
            else
            {
                currCookCount = -1;
                totalCookCount = -1;
            }

            int statusBufSize = 0;
            HAPI_GetStatusStringBufLength(
                theHAPISession.get(),
                HAPI_STATUS_COOK_STATE,
                HAPI_STATUSVERBOSITY_ERRORS,
                &statusBufSize
                );

            char * statusBuf = NULL;

            if(statusBufSize > 0)
            {
                statusBuf = new char[statusBufSize];
                HAPI_GetStatusString(
                    theHAPISession.get(),
                    HAPI_STATUS_COOK_STATE,
                    statusBuf,
                    statusBufSize
                    );
            }

            progressBar->updateProgress(currCookCount, totalCookCount, statusBuf);

            if(statusBuf)
            {
                delete[] statusBuf;
            }

            if(progressBar->isInterrupted())
            {
                HAPI_Interrupt( theHAPISession.get() );
            }

#ifdef _WIN32
            Sleep(1);
#else
            usleep(1000);
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

MString
getNodeName(const MObject &nodeObj)
{
    MStatus status;

    MString name;

    if(nodeObj.hasFn(MFn::kDagNode))
    {
        MFnDagNode nodeFn(nodeObj, &status);
        CHECK_MSTATUS(status);

        name = nodeFn.fullPathName();
    }
    else if(nodeObj.hasFn(MFn::kDependencyNode))
    {
        MFnDependencyNode nodeFn(nodeObj, &status);
        CHECK_MSTATUS(status);

        name = nodeFn.name();
    }

    return name;
}

MObject
findNodeByName(const MString& name)
{
    MSelectionList selection;
    selection.add(name);

    MObject ret;

    if(selection.length())
        selection.getDependNode(0, ret);
    return ret;
}

MObject
findDagChild(const MFnDagNode &dag, const MString &name)
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
createNodeByModifierCommand(
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
replaceString(const MString &str, const MString &searchStr, const MString &replaceStr)
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

MString
sanitizeStringForNodeName(const MString &str)
{
    MString result = str;
    assert(result.length() > 0);

    result = replaceString(result, ":", "_");
    result = replaceString(result, ".", "_");
    result = replaceString(result, "/", "_");
    result = replaceString(result, " ", "_");

    // check that the first character is not a number
    if(result.substringW(0,0).isInt())
    {
        result = "_" + result.substringW(1, result.length() - 1);
    }

    return result;
}

int
findParm(std::vector<HAPI_ParmInfo>& parms, MString name, int instanceNum)
{
    for(size_t i = 0; i < parms.size(); i++)
    {
        if(HAPIString(parms[i].templateNameSH) == name
                && (instanceNum < 0 || parms[i].instanceNum == instanceNum)
          )
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

WalkParmOperation::WalkParmOperation()
{
}

WalkParmOperation::~WalkParmOperation()
{
}

void
WalkParmOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
}

void
WalkParmOperation::popFolder()
{
}

void
WalkParmOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
}

void
WalkParmOperation::nextMultiparm()
{
}

void
WalkParmOperation::popMultiparm()
{
}

void
WalkParmOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
}

int
walkParmOne(
        const HAPI_ParmInfo* parmInfos,
        WalkParmOperation &operation
        );

int
walkParmMultiple(
        const HAPI_ParmInfo* parmInfos,
        WalkParmOperation &operation,
        unsigned int count
        );

int
walkParmOne(
        const HAPI_ParmInfo* parmInfos,
        WalkParmOperation &operation
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
        WalkParmOperation &operation,
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
walkParm(const std::vector<HAPI_ParmInfo> &parmInfos, WalkParmOperation &operation)
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

MPlug
plugSource(const MPlug &plug)
{
    MStatus status;

    MPlugArray connectedPlugs;
    plug.connectedTo(connectedPlugs, true, false, &status);
    CHECK_MSTATUS(status);

    if(!connectedPlugs.length())
    {
        return MPlug();
    }

    assert(connectedPlugs.length() == 1);

    return connectedPlugs[0];
}

MPlugArray
plugDestination(const MPlug &plug)
{
    MStatus status;

    MPlugArray connectedPlugs;
    plug.connectedTo(connectedPlugs, true, false, &status);
    CHECK_MSTATUS(status);

    if(!connectedPlugs.length())
    {
        return MPlugArray();
    }

    return connectedPlugs;
}
}
