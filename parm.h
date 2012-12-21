

class Parm {
    public:
        Parm();
        
        MIntArray getIntValues();
        MFloatArray getFloatValues();
        MStringArray getStringValues();

    public:
        HAPI_ParmInfo myInfo;
}
