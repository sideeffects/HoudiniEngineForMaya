#include <maya/MDGModifier.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>

#include <maya/MFnDagNode.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "util.h"

//minimum cook time to show the progress bar, in milliseconds
#define MIN_COOKTIME_FOR_PROGRESSBAR	1000 
//=============================================================================
// HAPIError
//=============================================================================

HAPIError::HAPIError() throw()
    : exception()
    , myMessage("")
{}

HAPIError::HAPIError( const HAPIError & error ) throw()
    : exception()
    , myMessage(error.myMessage)
{}

HAPIError::HAPIError( MString msg ) throw()
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
    MString name = getString(parm.nameSH);
    if(parm.isChildOfMultiParm)
    {
        name = replaceString(name, "#", "_");
    }

    // If it's a button, add a suffix so that we can distinguish it while
    // populating the AE
    if(parm.type == HAPI_PARMTYPE_BUTTON)
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
    if (hasHAPICallFailed(stat))
    {
        int bufLen;
        HAPI_GetStatusStringBufLength(HAPI_STATUS_RESULT, &bufLen);
        char * buffer = new char[bufLen];
        HAPI_GetStatusString(HAPI_STATUS_RESULT, buffer);
        throw HAPIError(buffer);
    }
}

//TODO: For each of the ifdefs, implement them for other platforms (mac and linux)
void pumpmsgs()
{
#ifdef _WIN32
    MSG msg;
    
    while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0 )
    {	    	
	TranslateMessage(&msg);
	DispatchMessage(&msg);	
    }        
#endif
}

void 
Util::showProgressWindow( const MString & title, const MString & status, int progress )
{
#ifdef _WIN32
    MString cmdStr = "progressWindow -t \"";
    cmdStr += title;
    cmdStr += "\" -progress ";
    cmdStr += progress;
    cmdStr += " -status \"";
    cmdStr += status;
    cmdStr += "\" -ii false";
    MGlobal::executeCommand( cmdStr );
    pumpmsgs();    
#endif
}

void 
Util::updateProgressWindow( const MString & status, int progress )
{
#ifdef _WIN32
    MString cmdStr;
    cmdStr = "progressWindow -e -progress ";	    	    
    cmdStr += progress;

    cmdStr += " -st \"";
    cmdStr += status;
    cmdStr += "\"";
        	    
    MGlobal::executeCommand( cmdStr );	
    pumpmsgs();
#endif    
}

void 
Util::hideProgressWindow()
{
#ifdef _WIN32
    MString cmdStr = "progressWindow -endProgress";
    MGlobal::executeCommand( cmdStr );
    pumpmsgs();
#endif
    
}

void
Util::statusCheckLoop()
{        
    bool showProgressWindow = false;
    HAPI_State state = HAPI_STATE_STARTING_LOAD;    
    int currState = (int) state;
    int currCookCount = 0;
    int totalCookCount = 1;       
    
#ifdef _WIN32
    int startTime = 0;
    
    startTime = ::GetTickCount();
#endif

    int elapsedTime = 0;
    while ( state != HAPI_STATE_READY )
    {
	    HAPI_GetStatus( HAPI_STATUS_STATE, &currState );
	    state = (HAPI_State) currState;

	    
#ifdef _WIN32
	    elapsedTime = (int)::GetTickCount() - startTime;
#endif

	    if( elapsedTime > MIN_COOKTIME_FOR_PROGRESSBAR && !showProgressWindow )
	    {
		MString title("Houdini");
		MString status("Working...");    
		Util::showProgressWindow( title, status, 0 );
		showProgressWindow = true;

	    }

	    int percent = 0;
	    if ( state == HAPI_STATE_COOKING )
	    {
		    HAPI_GetCookingCurrentCount( &currCookCount ); 
		    HAPI_GetCookingTotalCount( &totalCookCount );		    
		    percent = (int) ( (float) currCookCount*100 / (float) totalCookCount);
	    }
	    else
	    {		    
		    percent = (int)((elapsedTime / 1000) % 100);
	    }
	    
	    int statusBufSize = 0;
	    HAPI_GetStatusStringBufLength( HAPI_STATUS_STATE, 
					     &statusBufSize );

	    char * statusBuf = NULL;

	    if( statusBufSize > 0 )
	    {
		statusBuf = new char[ statusBufSize ];	        
		HAPI_GetStatusString( HAPI_STATUS_STATE, statusBuf );		
	    }
	        	    	    

	    if( statusBuf != NULL && showProgressWindow )
	    {
		MString statusStr = statusBuf;
		updateProgressWindow( statusStr, percent );
#ifdef _WIN32
		::Sleep( 100 );
#endif

	    }	    

	    if(statusBuf)
	    {
		delete[] statusBuf;
	    }
    }    

    if( showProgressWindow )
	Util::hideProgressWindow();    
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
    HAPI_GetAttributeInfo( assetId, objectId, geoId, partId, name.asChar(), owner, &attr_info);

    MStringArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    int * data = new int[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    HAPI_GetAttributeStrData( assetId, objectId, geoId, partId, name.asChar(),
            &attr_info, data, 0, attr_info.count);

    for (int j=0; j<size; j++){
        ret.append(Util::getString(data[j]));
    }

    delete[] data;

    return ret;
}

int
Util::findParm(std::vector<HAPI_ParmInfo>& parms, MString name, int instanceNum)
{
    for (size_t i = 0; i < parms.size(); i++)
    {
        if(getString(parms[i].nameSH) == name
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
