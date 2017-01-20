#include "OutputMaterial.h"

#include <maya/MStatus.h>

OutputMaterial::OutputMaterial()
{
}

MStatus
OutputMaterial::compute(
        const MTime &time,
        MDataHandle &materialHandle
        )
{
    return MStatus::kSuccess;
}
