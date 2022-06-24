#include "OutputDeform.h"
#include "OutputObject.h"
#include "OutputGeometryObject.h"
#include "OutputGeometry.h"
#include "OutputGeometryPart.h"
#include "hapiutil.h"


bool
OutputDeform::getDelayedPointAttribute(
	const char *name, OutputDeformCache &cache, size_t n,
	float *buf)
{
    const HAPI_Session *session = Util::theHAPISession.get();
    return getPointAttribute(session,myDelayedNodeId,myDelayedPartId, name, cache, n, buf);
}

bool
OutputDeform::getPointAttribute(const HAPI_Session *session, 
	const HAPI_NodeId &nodeId, const HAPI_PartId &partId,
	const char *name, OutputDeformCache &cache, size_t n,
	float *buf
	)
{

    HAPI_Result hapiResult;
    if (!cache.myNeed)
	return false;

    HAPI_AttributeInfo attrInfo;
    hapiResult = HoudiniApi::GetAttributeInfo(session, nodeId, partId, name,
	HAPI_ATTROWNER_POINT, &attrInfo);

    if (HAPI_FAIL(hapiResult))
	return false;

    if (!attrInfo.exists)
	return false;

    const int stride = cache.myStride;
    int numComponents = 3;
    if (attrInfo.storage==HAPI_STORAGETYPE_FLOAT64)
    {
	cache.myIsDouble = true;
	cache.myData64.resize(n*numComponents);
	cache.myData32.resize(0);

	if (buf)
	{
	    float *dst = buf;
	    double *src = &(cache.myData64[0]);

	    {
		for (size_t i=0; i<n; ++i)
		{
		    *(dst++) = *(src++);
		    *(dst++) = *(src++);
		    *(dst++) = *(src++);
		}
	    }
	}
    }
    else if (buf!=nullptr && stride==2)
    {
	cache.myIsDouble = false;
	cache.myData32.resize(n*numComponents);
	cache.myData64.resize(0);
	hapiResult = HoudiniApi::GetAttributeFloatData( session, nodeId, partId, name,
	    &attrInfo, -1, /*data_array=*/&(cache.myData32[0]),
	    /*start=*/0, /*length=*/n );

	float *dst = buf;
	float *src = &(cache.myData32[0]);

	for (size_t i=0; i<n; ++i)
	{
	    *(dst++) = *(src++);
	    *(dst++) = *(src++);
	    src++;
	}
    }
    else if (buf!=nullptr)
    {
	cache.myIsDouble = false;
	cache.myData32.resize(0);
	cache.myData64.resize(0);

	hapiResult = HoudiniApi::GetAttributeFloatData( session, nodeId, partId, name,
	    &attrInfo, -1, /*data_array=*/buf,
	    /*start=*/0, /*length=*/n );
    }
    else
    {
	cache.myIsDouble = false;
	cache.myData32.resize(n*numComponents);
	cache.myData64.resize(0);
	hapiResult = HoudiniApi::GetAttributeFloatData( session, nodeId, partId, name,
	    &attrInfo, -1, /*data_array=*/&(cache.myData32[0]),
	    /*start=*/0, /*length=*/n );
    
    }

    if (HAPI_FAIL(hapiResult))
	return false;

    cache.myDirty = true;
    cache.myIsValid = true;
    return true;
}

bool
OutputDeform::compute(OutputObject *obj, size_t &n)
{
    myPos.myIsValid = false;
    myNormal.myIsValid = false;
    myTexture.myIsValid = false;
    myDelayedPointCount = 0;
    myDelayedNodeId = 0;
    myDelayedPartId = 0;

    OutputGeometryObject *gobj = dynamic_cast<OutputGeometryObject*>(obj);
    if (!gobj)
	return false;

    gobj->update();
    if (gobj->myGeos.size()<1)
	return false;

    OutputGeometry *geo = gobj->myGeos[0];
    if (!geo)
	return false;

    if (geo->myParts.size()<1)
	return false;

    OutputGeometryPart *part = geo->myParts[0];
    if (!part)
	return false;

    HAPI_NodeId nodeId = part->myNodeId;
    HAPI_PartId partId = part->myPartId;

    const HAPI_Session *session = Util::theHAPISession.get();
    HAPI_Result hapiResult;

    HAPI_PartInfo partInfo;
    hapiResult = HoudiniApi::GetPartInfo(session, nodeId, partId, &partInfo );
    if (HAPI_FAIL(hapiResult))
	return false;

    unsigned int pointCount =  (unsigned int)partInfo.pointCount;

    // Clamp the point count in case the houdini geo has less points than the
    // maya one.
    if (n>pointCount || n==0)
	n = pointCount;

    if (!mySkipPointAttributes)
    {
	if (!getPointAttribute(session,nodeId,partId,"P",myPos, n))
	{
	    myPos.myDirty = true;
	    myPos.myIsValid = false;

	    myNormal.myDirty = true;
	    myNormal.myIsValid = false;

	    myTexture.myDirty = true;
	    myTexture.myIsValid = false;
	    return false;
	}

	// N is optionnal
	if ( !getPointAttribute(session,nodeId,partId,"N",myNormal, n) )
	{
	    myNormal.myDirty = true;
	    myNormal.myIsValid = false;
	}

	// uv is optionnal
	if ( !getPointAttribute(session,nodeId,partId,"uv",myTexture, n) )
	{
	    myTexture.myDirty = true;
	    myTexture.myIsValid = false;
	}
    }
    else
    {
	myDelayedPointCount = n;
	myDelayedNodeId = nodeId;
	myDelayedPartId = partId;

	myPos.myDirty = true;
	myPos.myIsValid = true;

	myNormal.myDirty = true;
	myNormal.myIsValid = true;

	myTexture.myDirty = true;
	myTexture.myIsValid = true;
    }

    if (myNeedTopo)
    {
	// Quick check for topology change
	if (!myTopoChanged)
	{
	    if (myPartInfo.faceCount != partInfo.faceCount ||
		myPartInfo.vertexCount != partInfo.vertexCount )
	    {
		myTopoChanged = true;
	    }
	}

	if (myTopoChanged)
	{
	    myTopoValid = false;
	    myFaceCounts.resize(partInfo.faceCount);

	    hapiResult = HoudiniApi::GetFaceCounts(session, nodeId, partId,
			       &myFaceCounts.front(), 0, partInfo.faceCount);
	    if (HAPI_FAIL(hapiResult))
		return false;

	    myVertexList.resize(partInfo.vertexCount);
	    hapiResult = HoudiniApi::GetVertexList(session, nodeId, partId,
			       &myVertexList.front(), 0,
			       partInfo.vertexCount);
	    if (HAPI_FAIL(hapiResult))
		return false;

	    Util::reverseWindingOrder(myVertexList, myFaceCounts);

	    myTopoChanged = false;
	    myTopoDirty = true;
	    myTopoValid = true;
	}
    }


    // Keep the last part info
    memcpy(&myPartInfo,&partInfo,sizeof(HAPI_PartInfo));

    return true;
}


