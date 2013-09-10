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
// MayaError
//=============================================================================
MayaError::MayaError() throw()
    : exception()
{}

MayaError::MayaError( const MayaError & error ) throw()
    : exception()
    , myStat(error.myStat)
{}

MayaError::MayaError( MStatus stat ) throw()
    : exception()
    , myStat(stat)
{}

const char *
MayaError::what() const throw()
{
    myBuffer = "******************** Maya Error ********************\n";
    myBuffer += myStat.errorString();
    myBuffer += "\n****************************************************";
    return myBuffer.asChar();
}

MStatus
MayaError::status() { return myStat; }


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
    MString parmName = Util::getString(parm.nameSH);
    // folder parameters can return names witih spaces
    parmName = Util::replaceString(parmName, " ", "_");
    MString name = "houdiniAssetParm_" + parmName;
    return name;
}

void
Util::reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            int temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
}


void
Util::reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            float temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
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


void
Util::checkMayaStatus(MStatus stat)
{
    if (stat.error())
    {
        throw MayaError(stat);
    }
}


void
Util::printHAPIStatus(HAPI_Result stat)
{
    if (hasHAPICallFailed(stat))
    {
        int bufLen;
        HAPI_GetStatusStringBufLength(HAPI_STATUS_RESULT, &bufLen);
        char * buffer = new char[bufLen];
        HAPI_GetStatusString(HAPI_STATUS_RESULT, buffer);

        MString str;
        str += "******************** Maya Error ********************\n";
        str += buffer;
        str += "\n****************************************************";

        cerr << str << endl;
    }
}


void
Util::printMayaStatus(MStatus stat)
{
    if (stat.error())
    {
        cerr << stat << endl;
    }
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

    while(true)
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

int
Util::findParm(std::vector<HAPI_ParmInfo>& parms, MString name)
{
    for (size_t i = 0; i < parms.size(); i++)
    {
	MString current_parm_name = getString(parms[i].nameSH);
	if(parms[i].isChildOfMultiParm)
	{
	    current_parm_name = replaceString(current_parm_name, "#", MString() + parms[i].instanceNum);
	}

	if (current_parm_name == name)
	{
	    return static_cast<int>(i);
	}
    }

    return -1;
}
