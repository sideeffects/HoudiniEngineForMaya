#ifndef __AssetSync_h__
#define __AssetSync_h__

#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>

class AssetSync
{
    public:
	AssetSync();
	virtual ~AssetSync();

    public:
	virtual MStatus doIt() = 0;
	virtual MStatus undoIt() = 0;
	virtual MStatus redoIt() = 0;
};

#endif
