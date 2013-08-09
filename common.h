#ifndef __common_h__
#define __common_h__

#include <maya/MObject.h>

class AssetNodeAttributes
{
    public:
        static MObject fileNameAttr;
        static MObject parmsModified;
        static MObject timeInput;

        static MObject assetType;

        static MObject output;

        static MObject objects;
        static MObject objectName;
        static MObject metaData;
        static MObject mesh;
        
        static MObject transform;
        static MObject translateAttr;
        static MObject translateAttrX;
        static MObject translateAttrY;
        static MObject translateAttrZ;
        static MObject rotateAttr;
        static MObject rotateAttrX;
        static MObject rotateAttrY;
        static MObject rotateAttrZ;
        static MObject scaleAttr;
        static MObject scaleAttrX;
        static MObject scaleAttrY;
        static MObject scaleAttrZ;

        static MObject material;
        static MObject materialExists;
        static MObject texturePath;
        static MObject ambientAttr;
        static MObject diffuseAttr;
        static MObject specularAttr;
        static MObject alphaAttr;

        //static MObject numObjects;
        static MObject instancers;
        static MObject instancerData;
        static MObject instancedObjectNames;

};

#endif
