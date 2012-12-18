#ifndef GEOMETRY_PART_H
#define GEOMETRY_PART_H

class GeometryPart
{
    public:
        GeometryPart();
        GeometryPart(int assetId, int objectId, int geoId, int partId,
                MString partName, HAPI_GeoInfo geoInfo, Asset* objectControl);
        virtual ~GeometryPart();


        virtual MStatus compute(MDataHandle& handle);

        virtual void setGeoInfo(HAPI_GeoInfo& info);
        virtual bool hasMesh();

    public:
        MString partName;

    protected:
        virtual void update();

    private:
        void updateMaterial(MDataHandle& handle);
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();

        MObject createMesh();

        // Utility
        virtual MFloatArray getAttributeFloatData(HAPI_AttributeOwner owner, MString name);

    private:
        Asset* objectControl;

        int assetId;
        int objectId;
        int geoId;
        int partId;

        HAPI_GeoInfo geoInfo;
        HAPI_PartInfo partInfo;
        HAPI_MaterialInfo materialInfo;

        MIntArray faceCounts;
        MIntArray vertexList;
        MFloatPointArray points;
        MVectorArray normals;
        MFloatArray us;
        MFloatArray vs;

        bool neverBuilt;
};

#endif
