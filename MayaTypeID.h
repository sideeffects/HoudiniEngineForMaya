#ifndef __MayaTypeID_h__
#define __MayaTypeID_h__

// Inside Maya, the type IDs are used to identify nodes and dependency graph
// data. So when we create custom nodes and data in any Maya plugins, they must
// all be assigned unique IDs. For any plugins that will be used outside of
// Side Effects, we *must* assign globally unique IDs. However, for plugins
// that are only used internally inside Side Effects, we could assign the
// "internal IDs" (0x0 - 0x7ffff).

// Some important notes from the Maya SDK docs:

//     - In Maya, both intrinsic and user-defined Maya Objects are registered
//     and recognized by their type identifier or type id.

//     - It is very important to note that these ids are written into the Maya
//     binary file format. So, once an id is assigned to a node or data type it
//     can never be changed while any existing Maya file contains an instance
//     of that node or data type. If a change is made, such files will become
//     unreadable.

//     - For plug-ins that will forever be internal to your site use the
//     constructor that takes a single unsigned int parameter.  The numeric
//     range 0 - 0x7ffff (524288 ids) has been reserved for such plug-ins.

// For more information on these IDs, please refer to the documentation for
// MTypeId in the Maya SDK.

// Globally unique IDs assigned to Side Effects:
//     - 0x0011E240 - 0x0011E2BF : 128 IDs requested by Andrew Wong on 2013-07-17 13:55

// More IDs can be requested through the Autodesk Developer Network.

// Globally unique IDs being used by Side Effects
enum MayaTypeID
{
    MayaTypeID_HoudiniAssetNode = 0x0011E240,
    MayaTypeID_HoudiniFluidGridConvert = 0x0011E241,
    MayaTypeID_HoudiniCurveMeshInput = 0x0011E242
};

#endif
