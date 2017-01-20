#ifndef __OutputMaterial_h__
#define __OutputMaterial_h__

class MTime;
class MDataHandle;
class MStatus;

class OutputMaterial
{
public:
    OutputMaterial();

    MStatus compute(
            const MTime &time,
            MDataHandle &materialHandle
            );
};

#endif
