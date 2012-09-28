#include <string.h>

#include <maya/MPxNode.h> 

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>

#include <HAPI.h>


class Asset: public MPxNode
{
    public:
        Asset();
        virtual ~Asset(); 

        virtual MStatus compute( const MPlug& plug, MDataBlock& data );

        static void* creator();
        static MStatus initialize();

    public:
        static MObject input1;
        static MObject input2;
        static MObject output;
        static MTypeId id;

    private:
        MObject createMesh(MObject& outData);
        HAPI_AssetInfo* assetInfo;

};
