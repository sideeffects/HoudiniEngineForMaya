
#include "Geo.h"
#include "GeometryPart.h"
#include "Object.h"
#include "util.h"
#include "AssetNode.h"


Geo::~Geo() 
{    
}


Geo::Geo( int assetId, int objectId, int geoId, Object * parentObject) :
    myParentObject ( parentObject ),    
    myAssetId ( assetId ),
    myObjectId ( objectId ),
    myGeoId ( geoId )
{    
    HAPI_GeoInfo_Init( &myGeoInfo );

    // Do a full update, ignoring what has changed
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {        
        hstat = HAPI_GetGeoInfo( myAssetId, myObjectId, myGeoId, &myGeoInfo);
        Util::checkHAPIStatus(hstat);

        if( myGeoInfo.type == HAPI_GEOTYPE_DEFAULT || 
            myGeoInfo.type == HAPI_GEOTYPE_INTERMEDIATE )
        {
            int partCount = myGeoInfo.partCount;
            myParts.clear();
            myParts.reserve( partCount );
    
            HAPI_ObjectInfo objectInfo = myParentObject->getObjectInfo();

            for ( int ii = 0; ii < partCount; ii++)
            {
                myParts.push_back( GeometryPart( myAssetId, 
                                                 myObjectId, 
                                                 myGeoId, 
                                                 ii, 
                                                 objectInfo, 
                                                 myGeoInfo
                                                 ) );
            }
        }

    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
    }
}

void
Geo::update()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        // update geometry
        if ( myParentObject->getObjectInfo().haveGeosChanged )
        {	    
            HAPI_GeoInfo oldGeoInfo = myGeoInfo;
            hstat = HAPI_GetGeoInfo( myAssetId, myObjectId, myGeoId, &myGeoInfo );
            Util::checkHAPIStatus(hstat);

            if( myGeoInfo.type == HAPI_GEOTYPE_DEFAULT ||
                myGeoInfo.type == HAPI_GEOTYPE_INTERMEDIATE )
            {                
                int partCount = myGeoInfo.partCount;

                // if partCount changed, we clear out the array and make a new one.
                if (oldGeoInfo.partCount != partCount)
                {
                    myParts.clear();
                    myParts.reserve( partCount );
                    HAPI_ObjectInfo objectInfo = myParentObject->getObjectInfo();
                    for ( int ii = 0; ii < partCount; ii++ )
                    {	                
                        myParts.push_back( GeometryPart( myAssetId, 
                                                         myObjectId, 
                                                         myGeoId, 
                                                         ii, 
                                                         objectInfo, 
                                                         myGeoInfo
                                                         ));
                    }
                }
                
            }
        }

    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        HAPI_GeoInfo_Init( &myGeoInfo );
    }
    
}


MStatus 
Geo::compute(MDataHandle &geoHandle, bool &needToSyncOutputs)
{
    MStatus stat;    

    MDataHandle geoNameHandle = geoHandle.child( AssetNode::outputGeoName );
    geoNameHandle.setString(Util::getString( myGeoInfo.nameSH));
    geoNameHandle.setClean();

    MDataHandle isTemplatedHandle = geoHandle.child( AssetNode::outputGeoIsTemplated );
    isTemplatedHandle.setBool( myGeoInfo.isTemplated );
    isTemplatedHandle.setClean();

    MDataHandle isDisplayGeoHandle = geoHandle.child( AssetNode::outputGeoIsDisplayGeo );
    isDisplayGeoHandle.setBool( myGeoInfo.isDisplayGeo );
    isDisplayGeoHandle.setClean();

    MDataHandle partsHandle = geoHandle.child(AssetNode::outputParts);
    MArrayDataHandle partsArrayHandle(partsHandle);
    MArrayDataBuilder partsBuilder = partsArrayHandle.builder();
    if(partsBuilder.elementCount() != myGeoInfo.partCount)
    {
        needToSyncOutputs = true;
    }

    if( myGeoInfo.type == HAPI_GEOTYPE_DEFAULT || 
        myGeoInfo.type == HAPI_GEOTYPE_INTERMEDIATE )
    { 
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = partsBuilder.addElement(i);
            stat = myParts[i].compute(h, needToSyncOutputs);
            CHECK_MSTATUS_AND_RETURN( stat, MS::kFailure );
        
        }    

        int partBuilderSizeCheck = partsBuilder.elementCount();
        if (partBuilderSizeCheck > myGeoInfo.partCount)
        {
            for (int i = myGeoInfo.partCount; i< partBuilderSizeCheck; i++)
	    {
	        stat = partsBuilder.removeElement(i);
	        CHECK_MSTATUS_AND_RETURN( stat, MS::kFailure );
	    }
        }
    }

    partsArrayHandle.set(partsBuilder);

    return MS::kSuccess;
}

MStatus 
Geo::setClean( MPlug& geoPlug, MDataBlock& data )
{
    MPlug partsPlug = geoPlug.child( AssetNode::outputParts );
    for (int jj=0; jj < myGeoInfo.partCount; jj++)
    {
	MPlug partPlug = partsPlug[ jj ];
	data.setClean( partPlug.child( AssetNode::outputPartName ) );
	data.setClean( partPlug.child( AssetNode::outputPartMesh ) );

	data.setClean( partPlug.child( AssetNode::outputPartMaterial ) );
	data.setClean( partPlug.child( AssetNode::outputPartMaterialExists ) );
	data.setClean( partPlug.child( AssetNode::outputPartTexturePath ) );
	data.setClean( partPlug.child( AssetNode::outputPartAmbientColor ) );
	data.setClean( partPlug.child( AssetNode::outputPartDiffuseColor ) );
	data.setClean( partPlug.child( AssetNode::outputPartSpecularColor ) );
	data.setClean( partPlug.child( AssetNode::outputPartAlphaColor ) );
    }
    return MS::kSuccess;
}
