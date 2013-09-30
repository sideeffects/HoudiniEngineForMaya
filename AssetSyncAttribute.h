#ifndef __AssetSyncAttribute_h__
#define __AssetSyncAttribute_h__

#include "AssetSync.h"

#include <maya/MDGModifier.h>
#include <maya/MObject.h>

#include <HAPI/HAPI.h>

class AssetSyncAttribute : public AssetSync
{
    public:
	AssetSyncAttribute(
		const MObject &assetNodeObj
		);
	virtual ~AssetSyncAttribute();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	void addAttrTo(MObject& child, MObject* parent);

        MObject createAttr(HAPI_ParmInfo& parm);
        MObject createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);
        MObject createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName);

        void buildParms();
        int buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start);

    protected:
	const MObject myAssetNodeObj;

	HAPI_NodeInfo myNodeInfo;

	MDGModifier myDGModifier;
};

#endif
