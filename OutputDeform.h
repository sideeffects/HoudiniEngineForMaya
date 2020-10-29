#ifndef __OutputDeform_h__
#define __OutputDeform_h__

#include <vector>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>

#include <HAPI/HAPI_Common.h>

class OutputObject;

class OutputDeformCache
{
public:
    OutputDeformCache()
    : myIsDouble(false)
    , myDirty(true)
    , myNeed(true)
    , myIsValid(false)
    , myStride(-1)
    {}

    std::vector<float> myData32;
    std::vector<double> myData64;
    bool myIsDouble;
    bool myDirty;
    bool myNeed;
    bool myIsValid;
    int myStride;

};

class OutputDeform
{
public:
    OutputDeform(bool topo=false, bool normal=false, bool skippoints=false, bool uvs=false)
    : myNeedTopo(topo)
    , myTopoChanged(true)
    , myTopoDirty(true)
    , myTopoValid(false)
    , mySkipPointAttributes(skippoints)

    , myDelayedPointCount(0)
    , myDelayedNodeId(0)
    , myDelayedPartId(0)
    {
	myNormal.myNeed = normal;
	myTexture.myNeed = uvs;
	myTexture.myStride = 2; // Pull uv vector as vector2
    }

    bool getPointAttribute(const HAPI_Session *session, 
	const HAPI_NodeId &nodeId, const HAPI_PartId &partId,
	const char *name, OutputDeformCache &cache, size_t n,
	float *buf=nullptr);

    bool getDelayedPointAttribute(
	const char *name, OutputDeformCache &cache, size_t n,
	float *buf);

    // Compute the positions of the first part of obj
    bool compute(OutputObject *obj, size_t &n);

    OutputDeformCache myPos;
    OutputDeformCache myNormal;
    OutputDeformCache myTexture;

    std::vector<int> myFaceCounts;
    std::vector<int> myVertexList;

    HAPI_PartInfo myPartInfo;

    bool myNeedTopo;
    bool myTopoChanged;
    bool myTopoDirty;   // Dirty flag for the drawing code, The draw code resets it.
    bool myTopoValid;   // Dirty flag for the drawing code, The draw code resets it.
    bool mySkipPointAttributes; // Delay the copy of the point attributes

    size_t myDelayedPointCount;
    HAPI_NodeId myDelayedNodeId;
    HAPI_PartId myDelayedPartId;
};

#endif
