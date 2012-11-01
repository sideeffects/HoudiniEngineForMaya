#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MGlobal.h>

#include "asset.h"
#include "util.h"

Asset::Asset(MString otlFilePath)
{
    // test
    materialEnabled = true;

    const char* filename = otlFilePath.asChar();
    info.minVerticesPerPrimitive = 3;
    info.maxVerticesPerPrimitive = 20;

    if (materialEnabled)
    {
        MString texturePath;
        MGlobal::executeCommand("workspace -q -rd;", texturePath);
        texturePath += "sourceimages";
        HAPI_LoadOTLFile(filename, texturePath.asChar(), &info);
    } else
        HAPI_LoadOTLFile(filename, NULL, &info);


    // meshes
    int objCount = info.objectCount;
    //HAPI_ObjectInfo myObjects[objCount];
    //HAPI_GetObjects(info.id, myObjects, 0, objCount);

    // materials
    //int matCount = info.materialCount;
    //cerr << "matCount: " << matCount << endl;
    //HAPI_MaterialInfo myMaterials[matCount];
    //HAPI_GetMaterials(info.id, myMaterials, 0, matCount);

    //// debug print
    //HAPI_MaterialInfo mat = myMaterials[0];
    //cerr << "ambient:";
    //for (int i=0; i<4; i++)
        //cerr << " " << mat.ambient[i];
    //cerr << endl;

    //cerr << "diffuse:";
    //for (int i=0; i<4; i++)
        //cerr << " " << mat.diffuse[i];
    //cerr << endl;

    //cerr << "specular:";
    //for (int i=0; i<4; i++)
        //cerr << " " << mat.specular[i];
    //cerr << endl;
    

    objects = new Object[objCount];
    numVisibleObjects = 0;
    for (int i=0; i<objCount; i++)
    {
        //cerr << "matID: " << myObjects[i].materialId;
        objects[i] = Object(i, info.id);
        if (objects[i].isVisible())
            numVisibleObjects++;
    }

    visibleObjects = new Object[numVisibleObjects];
    int index = 0;
    for (int i=0; i<objCount; i++)
    {
        if (objects[i].isVisible())
        {
            visibleObjects[index] = objects[i];
            index++;
        }
    }

    // build parms
    buildParms();
}


MObjectArray
Asset::getParmAttributes()
{
    return parmAttributes;
}


Object*
Asset::getObjects()
{
    return objects;
}


Object*
Asset::getVisibleObjects()
{
    return visibleObjects;
}


void
Asset::addAttrTo(MObject& child, MObject* parent)
{
    if (NULL == parent)
    {
        parmAttributes.append(child);
        return;
    }

    MFnCompoundAttribute cAttr(*parent);
    cAttr.addChild(child);
}


MObject
Asset::createAttr(HAPI_ParmInfo& parm)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnGenericAttribute gAttr;
    MFnTypedAttribute tAttr;

    MString label = Util::getString(parm.labelSH);
    MString name = Util::getString(parm.nameSH);

    MString shortName = MString("_parm") + parm.id + "_";
    MString longName = MString("_H_parm") + parm.id + "_";

    MObject result;

    // Other types
    switch(parm.type)
    {
        case HAPI_PARMTYPE_FOLDERLIST:
        case HAPI_PARMTYPE_FOLDER:
            result = cAttr.create(longName, shortName);
            cAttr.setStorable(true);
            cAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_SEPARATOR:
            result = gAttr.create(longName, shortName);
            gAttr.setHidden(true);
            gAttr.setStorable(false);
            gAttr.setReadable(false);
            gAttr.setWritable(false);
            gAttr.setConnectable(false);
            gAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_INT:
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOUR:
        case HAPI_PARMTYPE_TOGGLE:
            result = createNumericAttr(parm, longName, shortName, label);
            break;
        case HAPI_PARMTYPE_STRING:
        case HAPI_PARMTYPE_FILE:
            result = createStringAttr(parm, longName, shortName, label);
            break;
        default:
            result = nAttr.create(longName, shortName, MFnNumericData::kFloat, 1.0);
            nAttr.setStorable(true);
            nAttr.setNiceNameOverride(label);
            break;
    }

    return result;
}


MObject
Asset::createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    int size = parm.size;

    MObject result;

    if (size > 1)
    {
        result = cAttr.create(longName, shortName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = tAttr.create(ln, sn, MFnData::kString);
            tAttr.setNiceNameOverride(nn);
            tAttr.setStorable(true);
            if (parm.type == HAPI_PARMTYPE_FILE)
                tAttr.setUsedAsFilename(true);
            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(longName, shortName, MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setNiceNameOverride(niceName);
    if (parm.type == HAPI_PARMTYPE_FILE)
        tAttr.setUsedAsFilename(true);

    return result;
}


MObject
Asset::createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnEnumAttribute eAttr;

    MObject result;
    int size = parm.size;
    int choiceCount = parm.choiceCount;

    // Choice list
    if (choiceCount > 0)
    {
        cerr << "create enum: " << niceName << "type: " << parm.type << endl;
        result = eAttr.create(longName, shortName);
        eAttr.setStorable(true);
        eAttr.setNiceNameOverride(niceName);

        HAPI_ParmChoiceInfo choiceInfos[choiceCount];
        HAPI_GetParmChoiceLists(info.id, choiceInfos, parm.choiceIndex, choiceCount);
        for (int i=0; i<choiceCount; i++)
        {
            MString field = Util::getString(choiceInfos[i].labelSH);
            eAttr.addField(field, i);
        }

        return result;
    }



    MFnNumericData::Type type;
    if (parm.type == HAPI_PARMTYPE_TOGGLE)
    {
        type = MFnNumericData::kBoolean;
    }
    else if (parm.type == HAPI_PARMTYPE_INT)
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kInt; break;
            case 2: type = MFnNumericData::k2Int; break;
            case 3: type = MFnNumericData::k3Int; break;
            default: type = MFnNumericData::kInt; break;
        }
    } else
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kFloat; break;
            case 2: type = MFnNumericData::k2Float; break;
            case 3: type = MFnNumericData::k3Float; break;
            default: type = MFnNumericData::kFloat; break;
        }
    }

    if (size > 3) {
        result = cAttr.create(longName, shortName);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = nAttr.create(ln, sn, type);
            nAttr.setNiceNameOverride(nn);
            cAttr.addChild(child);
        }
        return result;
    }

    if (parm.type == HAPI_PARMTYPE_COLOUR)
        result = nAttr.createColor(longName, shortName);
    else
        result = nAttr.create(longName, shortName, type);
    nAttr.setStorable(true);
    nAttr.setNiceNameOverride(niceName);

    // TODO: support min/max for all sizes
    if (parm.hasMin)
        nAttr.setMin(parm.min);
    if (parm.hasMax)
        nAttr.setMin(parm.max);
    if (parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if (parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

    return result;
}


void
Asset::buildParms()
{

    // PARMS
    int parmCount = info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(info.id, myParmInfos, 0, parmCount);

    int index = 0;
    while (index < parmCount)
    {
        int consumed = buildAttrTree(myParmInfos, NULL, index, index+1);
        index += consumed;
    }


}


int
Asset::buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start)
{
    HAPI_ParmInfo parm = myParmInfos[current];

    if (parm.type == HAPI_PARMTYPE_FOLDERLIST)
    {
        int offset = parm.size;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(myParmInfos, parent, i, start + offset);
            offset += count;
        }
        return offset + 1;
    }

    if (parm.type == HAPI_PARMTYPE_FOLDER)
    {
        MObject result = createAttr(parm);
        int offset = 0;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(myParmInfos, &result, start + offset, start + offset + 1);
            offset += count;
        }
        addAttrTo(result, parent);
        return offset;
    }

    MObject result = createAttr(parm);
    addAttrTo(result, parent);
    return 1;
}


MIntArray
Asset::getParmIntValues(HAPI_ParmInfo& parm)
{
    int index = parm.intValuesIndex;
    int size = parm.size;
    int values[size];
    HAPI_GetParmIntValues(info.id, values, index, size);

    MIntArray ret(values, size);
    return ret;
}


MFloatArray
Asset::getParmFloatValues(HAPI_ParmInfo& parm)
{
    int index = parm.floatValuesIndex;
    int size = parm.size;
    float values[size];
    HAPI_GetParmFloatValues(info.id, values, index, size);

    MFloatArray ret(values, size);
    return ret;
}


MStringArray
Asset::getParmStringValues(HAPI_ParmInfo& parm)
{
    int index = parm.stringValuesIndex;
    int size = parm.size;
    int handles[size];
    HAPI_GetParmStringValues(info.id, handles, index, size);

    MStringArray ret;
    for (int i=0; i<size; i++)
    {
        MString str = Util::getString(handles[i]);
        ret.append(str);
    }
    return ret;
}

