/*
 * Copyright (c) <2022> Side Effects Software Inc. *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 * COMMENTS:
 *      This file is generated. Do not modify directly.
 */

#include "HoudiniApi.h"
#include "Platform.h"

HoudiniApi::AddAttributeFuncPtr
HoudiniApi::AddAttribute = &HoudiniApi::AddAttributeEmptyStub;

HoudiniApi::AddGroupFuncPtr
HoudiniApi::AddGroup = &HoudiniApi::AddGroupEmptyStub;

HoudiniApi::AssetInfo_CreateFuncPtr
HoudiniApi::AssetInfo_Create = &HoudiniApi::AssetInfo_CreateEmptyStub;

HoudiniApi::AssetInfo_InitFuncPtr
HoudiniApi::AssetInfo_Init = &HoudiniApi::AssetInfo_InitEmptyStub;

HoudiniApi::AttributeInfo_CreateFuncPtr
HoudiniApi::AttributeInfo_Create = &HoudiniApi::AttributeInfo_CreateEmptyStub;

HoudiniApi::AttributeInfo_InitFuncPtr
HoudiniApi::AttributeInfo_Init = &HoudiniApi::AttributeInfo_InitEmptyStub;

HoudiniApi::BindCustomImplementationFuncPtr
HoudiniApi::BindCustomImplementation = &HoudiniApi::BindCustomImplementationEmptyStub;

HoudiniApi::CancelPDGCookFuncPtr
HoudiniApi::CancelPDGCook = &HoudiniApi::CancelPDGCookEmptyStub;

HoudiniApi::CheckForSpecificErrorsFuncPtr
HoudiniApi::CheckForSpecificErrors = &HoudiniApi::CheckForSpecificErrorsEmptyStub;

HoudiniApi::CleanupFuncPtr
HoudiniApi::Cleanup = &HoudiniApi::CleanupEmptyStub;

HoudiniApi::ClearConnectionErrorFuncPtr
HoudiniApi::ClearConnectionError = &HoudiniApi::ClearConnectionErrorEmptyStub;

HoudiniApi::CloseSessionFuncPtr
HoudiniApi::CloseSession = &HoudiniApi::CloseSessionEmptyStub;

HoudiniApi::CommitGeoFuncPtr
HoudiniApi::CommitGeo = &HoudiniApi::CommitGeoEmptyStub;

HoudiniApi::CommitWorkItemsFuncPtr
HoudiniApi::CommitWorkItems = &HoudiniApi::CommitWorkItemsEmptyStub;

HoudiniApi::CommitWorkitemsFuncPtr
HoudiniApi::CommitWorkitems = &HoudiniApi::CommitWorkitemsEmptyStub;

HoudiniApi::ComposeChildNodeListFuncPtr
HoudiniApi::ComposeChildNodeList = &HoudiniApi::ComposeChildNodeListEmptyStub;

HoudiniApi::ComposeNodeCookResultFuncPtr
HoudiniApi::ComposeNodeCookResult = &HoudiniApi::ComposeNodeCookResultEmptyStub;

HoudiniApi::ComposeObjectListFuncPtr
HoudiniApi::ComposeObjectList = &HoudiniApi::ComposeObjectListEmptyStub;

HoudiniApi::CompositorOptions_CreateFuncPtr
HoudiniApi::CompositorOptions_Create = &HoudiniApi::CompositorOptions_CreateEmptyStub;

HoudiniApi::CompositorOptions_InitFuncPtr
HoudiniApi::CompositorOptions_Init = &HoudiniApi::CompositorOptions_InitEmptyStub;

HoudiniApi::ConnectNodeInputFuncPtr
HoudiniApi::ConnectNodeInput = &HoudiniApi::ConnectNodeInputEmptyStub;

HoudiniApi::ConvertMatrixToEulerFuncPtr
HoudiniApi::ConvertMatrixToEuler = &HoudiniApi::ConvertMatrixToEulerEmptyStub;

HoudiniApi::ConvertMatrixToQuatFuncPtr
HoudiniApi::ConvertMatrixToQuat = &HoudiniApi::ConvertMatrixToQuatEmptyStub;

HoudiniApi::ConvertTransformFuncPtr
HoudiniApi::ConvertTransform = &HoudiniApi::ConvertTransformEmptyStub;

HoudiniApi::ConvertTransformEulerToMatrixFuncPtr
HoudiniApi::ConvertTransformEulerToMatrix = &HoudiniApi::ConvertTransformEulerToMatrixEmptyStub;

HoudiniApi::ConvertTransformQuatToMatrixFuncPtr
HoudiniApi::ConvertTransformQuatToMatrix = &HoudiniApi::ConvertTransformQuatToMatrixEmptyStub;

HoudiniApi::CookNodeFuncPtr
HoudiniApi::CookNode = &HoudiniApi::CookNodeEmptyStub;

HoudiniApi::CookOptions_AreEqualFuncPtr
HoudiniApi::CookOptions_AreEqual = &HoudiniApi::CookOptions_AreEqualEmptyStub;

HoudiniApi::CookOptions_CreateFuncPtr
HoudiniApi::CookOptions_Create = &HoudiniApi::CookOptions_CreateEmptyStub;

HoudiniApi::CookOptions_InitFuncPtr
HoudiniApi::CookOptions_Init = &HoudiniApi::CookOptions_InitEmptyStub;

HoudiniApi::CookPDGFuncPtr
HoudiniApi::CookPDG = &HoudiniApi::CookPDGEmptyStub;

HoudiniApi::CookPDGAllOutputsFuncPtr
HoudiniApi::CookPDGAllOutputs = &HoudiniApi::CookPDGAllOutputsEmptyStub;

HoudiniApi::CreateCustomSessionFuncPtr
HoudiniApi::CreateCustomSession = &HoudiniApi::CreateCustomSessionEmptyStub;

HoudiniApi::CreateHeightFieldInputFuncPtr
HoudiniApi::CreateHeightFieldInput = &HoudiniApi::CreateHeightFieldInputEmptyStub;

HoudiniApi::CreateHeightfieldInputVolumeNodeFuncPtr
HoudiniApi::CreateHeightfieldInputVolumeNode = &HoudiniApi::CreateHeightfieldInputVolumeNodeEmptyStub;

HoudiniApi::CreateInProcessSessionFuncPtr
HoudiniApi::CreateInProcessSession = &HoudiniApi::CreateInProcessSessionEmptyStub;

HoudiniApi::CreateInputCurveNodeFuncPtr
HoudiniApi::CreateInputCurveNode = &HoudiniApi::CreateInputCurveNodeEmptyStub;

HoudiniApi::CreateInputNodeFuncPtr
HoudiniApi::CreateInputNode = &HoudiniApi::CreateInputNodeEmptyStub;

HoudiniApi::CreateNodeFuncPtr
HoudiniApi::CreateNode = &HoudiniApi::CreateNodeEmptyStub;

HoudiniApi::CreateThriftNamedPipeSessionFuncPtr
HoudiniApi::CreateThriftNamedPipeSession = &HoudiniApi::CreateThriftNamedPipeSessionEmptyStub;

HoudiniApi::CreateThriftSocketSessionFuncPtr
HoudiniApi::CreateThriftSocketSession = &HoudiniApi::CreateThriftSocketSessionEmptyStub;

HoudiniApi::CreateWorkItemFuncPtr
HoudiniApi::CreateWorkItem = &HoudiniApi::CreateWorkItemEmptyStub;

HoudiniApi::CreateWorkitemFuncPtr
HoudiniApi::CreateWorkitem = &HoudiniApi::CreateWorkitemEmptyStub;

HoudiniApi::CurveInfo_CreateFuncPtr
HoudiniApi::CurveInfo_Create = &HoudiniApi::CurveInfo_CreateEmptyStub;

HoudiniApi::CurveInfo_InitFuncPtr
HoudiniApi::CurveInfo_Init = &HoudiniApi::CurveInfo_InitEmptyStub;

HoudiniApi::DeleteAttributeFuncPtr
HoudiniApi::DeleteAttribute = &HoudiniApi::DeleteAttributeEmptyStub;

HoudiniApi::DeleteGroupFuncPtr
HoudiniApi::DeleteGroup = &HoudiniApi::DeleteGroupEmptyStub;

HoudiniApi::DeleteNodeFuncPtr
HoudiniApi::DeleteNode = &HoudiniApi::DeleteNodeEmptyStub;

HoudiniApi::DirtyPDGNodeFuncPtr
HoudiniApi::DirtyPDGNode = &HoudiniApi::DirtyPDGNodeEmptyStub;

HoudiniApi::DisconnectNodeInputFuncPtr
HoudiniApi::DisconnectNodeInput = &HoudiniApi::DisconnectNodeInputEmptyStub;

HoudiniApi::DisconnectNodeOutputsAtFuncPtr
HoudiniApi::DisconnectNodeOutputsAt = &HoudiniApi::DisconnectNodeOutputsAtEmptyStub;

HoudiniApi::ExtractImageToFileFuncPtr
HoudiniApi::ExtractImageToFile = &HoudiniApi::ExtractImageToFileEmptyStub;

HoudiniApi::ExtractImageToMemoryFuncPtr
HoudiniApi::ExtractImageToMemory = &HoudiniApi::ExtractImageToMemoryEmptyStub;

HoudiniApi::GeoInfo_CreateFuncPtr
HoudiniApi::GeoInfo_Create = &HoudiniApi::GeoInfo_CreateEmptyStub;

HoudiniApi::GeoInfo_GetGroupCountByTypeFuncPtr
HoudiniApi::GeoInfo_GetGroupCountByType = &HoudiniApi::GeoInfo_GetGroupCountByTypeEmptyStub;

HoudiniApi::GeoInfo_InitFuncPtr
HoudiniApi::GeoInfo_Init = &HoudiniApi::GeoInfo_InitEmptyStub;

HoudiniApi::GetActiveCacheCountFuncPtr
HoudiniApi::GetActiveCacheCount = &HoudiniApi::GetActiveCacheCountEmptyStub;

HoudiniApi::GetActiveCacheNamesFuncPtr
HoudiniApi::GetActiveCacheNames = &HoudiniApi::GetActiveCacheNamesEmptyStub;

HoudiniApi::GetAssetDefinitionParmCountsFuncPtr
HoudiniApi::GetAssetDefinitionParmCounts = &HoudiniApi::GetAssetDefinitionParmCountsEmptyStub;

HoudiniApi::GetAssetDefinitionParmInfosFuncPtr
HoudiniApi::GetAssetDefinitionParmInfos = &HoudiniApi::GetAssetDefinitionParmInfosEmptyStub;

HoudiniApi::GetAssetDefinitionParmValuesFuncPtr
HoudiniApi::GetAssetDefinitionParmValues = &HoudiniApi::GetAssetDefinitionParmValuesEmptyStub;

HoudiniApi::GetAssetInfoFuncPtr
HoudiniApi::GetAssetInfo = &HoudiniApi::GetAssetInfoEmptyStub;

HoudiniApi::GetAttributeFloat64ArrayDataFuncPtr
HoudiniApi::GetAttributeFloat64ArrayData = &HoudiniApi::GetAttributeFloat64ArrayDataEmptyStub;

HoudiniApi::GetAttributeFloat64DataFuncPtr
HoudiniApi::GetAttributeFloat64Data = &HoudiniApi::GetAttributeFloat64DataEmptyStub;

HoudiniApi::GetAttributeFloatArrayDataFuncPtr
HoudiniApi::GetAttributeFloatArrayData = &HoudiniApi::GetAttributeFloatArrayDataEmptyStub;

HoudiniApi::GetAttributeFloatDataFuncPtr
HoudiniApi::GetAttributeFloatData = &HoudiniApi::GetAttributeFloatDataEmptyStub;

HoudiniApi::GetAttributeInfoFuncPtr
HoudiniApi::GetAttributeInfo = &HoudiniApi::GetAttributeInfoEmptyStub;

HoudiniApi::GetAttributeInt16ArrayDataFuncPtr
HoudiniApi::GetAttributeInt16ArrayData = &HoudiniApi::GetAttributeInt16ArrayDataEmptyStub;

HoudiniApi::GetAttributeInt16DataFuncPtr
HoudiniApi::GetAttributeInt16Data = &HoudiniApi::GetAttributeInt16DataEmptyStub;

HoudiniApi::GetAttributeInt64ArrayDataFuncPtr
HoudiniApi::GetAttributeInt64ArrayData = &HoudiniApi::GetAttributeInt64ArrayDataEmptyStub;

HoudiniApi::GetAttributeInt64DataFuncPtr
HoudiniApi::GetAttributeInt64Data = &HoudiniApi::GetAttributeInt64DataEmptyStub;

HoudiniApi::GetAttributeInt8ArrayDataFuncPtr
HoudiniApi::GetAttributeInt8ArrayData = &HoudiniApi::GetAttributeInt8ArrayDataEmptyStub;

HoudiniApi::GetAttributeInt8DataFuncPtr
HoudiniApi::GetAttributeInt8Data = &HoudiniApi::GetAttributeInt8DataEmptyStub;

HoudiniApi::GetAttributeIntArrayDataFuncPtr
HoudiniApi::GetAttributeIntArrayData = &HoudiniApi::GetAttributeIntArrayDataEmptyStub;

HoudiniApi::GetAttributeIntDataFuncPtr
HoudiniApi::GetAttributeIntData = &HoudiniApi::GetAttributeIntDataEmptyStub;

HoudiniApi::GetAttributeNamesFuncPtr
HoudiniApi::GetAttributeNames = &HoudiniApi::GetAttributeNamesEmptyStub;

HoudiniApi::GetAttributeStringArrayDataFuncPtr
HoudiniApi::GetAttributeStringArrayData = &HoudiniApi::GetAttributeStringArrayDataEmptyStub;

HoudiniApi::GetAttributeStringDataFuncPtr
HoudiniApi::GetAttributeStringData = &HoudiniApi::GetAttributeStringDataEmptyStub;

HoudiniApi::GetAttributeUInt8ArrayDataFuncPtr
HoudiniApi::GetAttributeUInt8ArrayData = &HoudiniApi::GetAttributeUInt8ArrayDataEmptyStub;

HoudiniApi::GetAttributeUInt8DataFuncPtr
HoudiniApi::GetAttributeUInt8Data = &HoudiniApi::GetAttributeUInt8DataEmptyStub;

HoudiniApi::GetAvailableAssetCountFuncPtr
HoudiniApi::GetAvailableAssetCount = &HoudiniApi::GetAvailableAssetCountEmptyStub;

HoudiniApi::GetAvailableAssetsFuncPtr
HoudiniApi::GetAvailableAssets = &HoudiniApi::GetAvailableAssetsEmptyStub;

HoudiniApi::GetBoxInfoFuncPtr
HoudiniApi::GetBoxInfo = &HoudiniApi::GetBoxInfoEmptyStub;

HoudiniApi::GetCachePropertyFuncPtr
HoudiniApi::GetCacheProperty = &HoudiniApi::GetCachePropertyEmptyStub;

HoudiniApi::GetComposedChildNodeListFuncPtr
HoudiniApi::GetComposedChildNodeList = &HoudiniApi::GetComposedChildNodeListEmptyStub;

HoudiniApi::GetComposedNodeCookResultFuncPtr
HoudiniApi::GetComposedNodeCookResult = &HoudiniApi::GetComposedNodeCookResultEmptyStub;

HoudiniApi::GetComposedObjectListFuncPtr
HoudiniApi::GetComposedObjectList = &HoudiniApi::GetComposedObjectListEmptyStub;

HoudiniApi::GetComposedObjectTransformsFuncPtr
HoudiniApi::GetComposedObjectTransforms = &HoudiniApi::GetComposedObjectTransformsEmptyStub;

HoudiniApi::GetCompositorOptionsFuncPtr
HoudiniApi::GetCompositorOptions = &HoudiniApi::GetCompositorOptionsEmptyStub;

HoudiniApi::GetConnectionErrorFuncPtr
HoudiniApi::GetConnectionError = &HoudiniApi::GetConnectionErrorEmptyStub;

HoudiniApi::GetConnectionErrorLengthFuncPtr
HoudiniApi::GetConnectionErrorLength = &HoudiniApi::GetConnectionErrorLengthEmptyStub;

HoudiniApi::GetCookingCurrentCountFuncPtr
HoudiniApi::GetCookingCurrentCount = &HoudiniApi::GetCookingCurrentCountEmptyStub;

HoudiniApi::GetCookingTotalCountFuncPtr
HoudiniApi::GetCookingTotalCount = &HoudiniApi::GetCookingTotalCountEmptyStub;

HoudiniApi::GetCurveCountsFuncPtr
HoudiniApi::GetCurveCounts = &HoudiniApi::GetCurveCountsEmptyStub;

HoudiniApi::GetCurveInfoFuncPtr
HoudiniApi::GetCurveInfo = &HoudiniApi::GetCurveInfoEmptyStub;

HoudiniApi::GetCurveKnotsFuncPtr
HoudiniApi::GetCurveKnots = &HoudiniApi::GetCurveKnotsEmptyStub;

HoudiniApi::GetCurveOrdersFuncPtr
HoudiniApi::GetCurveOrders = &HoudiniApi::GetCurveOrdersEmptyStub;

HoudiniApi::GetDisplayGeoInfoFuncPtr
HoudiniApi::GetDisplayGeoInfo = &HoudiniApi::GetDisplayGeoInfoEmptyStub;

HoudiniApi::GetEdgeCountOfEdgeGroupFuncPtr
HoudiniApi::GetEdgeCountOfEdgeGroup = &HoudiniApi::GetEdgeCountOfEdgeGroupEmptyStub;

HoudiniApi::GetEnvIntFuncPtr
HoudiniApi::GetEnvInt = &HoudiniApi::GetEnvIntEmptyStub;

HoudiniApi::GetFaceCountsFuncPtr
HoudiniApi::GetFaceCounts = &HoudiniApi::GetFaceCountsEmptyStub;

HoudiniApi::GetFirstVolumeTileFuncPtr
HoudiniApi::GetFirstVolumeTile = &HoudiniApi::GetFirstVolumeTileEmptyStub;

HoudiniApi::GetGeoInfoFuncPtr
HoudiniApi::GetGeoInfo = &HoudiniApi::GetGeoInfoEmptyStub;

HoudiniApi::GetGeoSizeFuncPtr
HoudiniApi::GetGeoSize = &HoudiniApi::GetGeoSizeEmptyStub;

HoudiniApi::GetGroupCountOnPackedInstancePartFuncPtr
HoudiniApi::GetGroupCountOnPackedInstancePart = &HoudiniApi::GetGroupCountOnPackedInstancePartEmptyStub;

HoudiniApi::GetGroupMembershipFuncPtr
HoudiniApi::GetGroupMembership = &HoudiniApi::GetGroupMembershipEmptyStub;

HoudiniApi::GetGroupMembershipOnPackedInstancePartFuncPtr
HoudiniApi::GetGroupMembershipOnPackedInstancePart = &HoudiniApi::GetGroupMembershipOnPackedInstancePartEmptyStub;

HoudiniApi::GetGroupNamesFuncPtr
HoudiniApi::GetGroupNames = &HoudiniApi::GetGroupNamesEmptyStub;

HoudiniApi::GetGroupNamesOnPackedInstancePartFuncPtr
HoudiniApi::GetGroupNamesOnPackedInstancePart = &HoudiniApi::GetGroupNamesOnPackedInstancePartEmptyStub;

HoudiniApi::GetHIPFileNodeCountFuncPtr
HoudiniApi::GetHIPFileNodeCount = &HoudiniApi::GetHIPFileNodeCountEmptyStub;

HoudiniApi::GetHIPFileNodeIdsFuncPtr
HoudiniApi::GetHIPFileNodeIds = &HoudiniApi::GetHIPFileNodeIdsEmptyStub;

HoudiniApi::GetHandleBindingInfoFuncPtr
HoudiniApi::GetHandleBindingInfo = &HoudiniApi::GetHandleBindingInfoEmptyStub;

HoudiniApi::GetHandleInfoFuncPtr
HoudiniApi::GetHandleInfo = &HoudiniApi::GetHandleInfoEmptyStub;

HoudiniApi::GetHeightFieldDataFuncPtr
HoudiniApi::GetHeightFieldData = &HoudiniApi::GetHeightFieldDataEmptyStub;

HoudiniApi::GetImageFilePathFuncPtr
HoudiniApi::GetImageFilePath = &HoudiniApi::GetImageFilePathEmptyStub;

HoudiniApi::GetImageInfoFuncPtr
HoudiniApi::GetImageInfo = &HoudiniApi::GetImageInfoEmptyStub;

HoudiniApi::GetImageMemoryBufferFuncPtr
HoudiniApi::GetImageMemoryBuffer = &HoudiniApi::GetImageMemoryBufferEmptyStub;

HoudiniApi::GetImagePlaneCountFuncPtr
HoudiniApi::GetImagePlaneCount = &HoudiniApi::GetImagePlaneCountEmptyStub;

HoudiniApi::GetImagePlanesFuncPtr
HoudiniApi::GetImagePlanes = &HoudiniApi::GetImagePlanesEmptyStub;

HoudiniApi::GetInputCurveInfoFuncPtr
HoudiniApi::GetInputCurveInfo = &HoudiniApi::GetInputCurveInfoEmptyStub;

HoudiniApi::GetInstanceTransformsOnPartFuncPtr
HoudiniApi::GetInstanceTransformsOnPart = &HoudiniApi::GetInstanceTransformsOnPartEmptyStub;

HoudiniApi::GetInstancedObjectIdsFuncPtr
HoudiniApi::GetInstancedObjectIds = &HoudiniApi::GetInstancedObjectIdsEmptyStub;

HoudiniApi::GetInstancedPartIdsFuncPtr
HoudiniApi::GetInstancedPartIds = &HoudiniApi::GetInstancedPartIdsEmptyStub;

HoudiniApi::GetInstancerPartTransformsFuncPtr
HoudiniApi::GetInstancerPartTransforms = &HoudiniApi::GetInstancerPartTransformsEmptyStub;

HoudiniApi::GetManagerNodeIdFuncPtr
HoudiniApi::GetManagerNodeId = &HoudiniApi::GetManagerNodeIdEmptyStub;

HoudiniApi::GetMaterialInfoFuncPtr
HoudiniApi::GetMaterialInfo = &HoudiniApi::GetMaterialInfoEmptyStub;

HoudiniApi::GetMaterialNodeIdsOnFacesFuncPtr
HoudiniApi::GetMaterialNodeIdsOnFaces = &HoudiniApi::GetMaterialNodeIdsOnFacesEmptyStub;

HoudiniApi::GetNextVolumeTileFuncPtr
HoudiniApi::GetNextVolumeTile = &HoudiniApi::GetNextVolumeTileEmptyStub;

HoudiniApi::GetNodeFromPathFuncPtr
HoudiniApi::GetNodeFromPath = &HoudiniApi::GetNodeFromPathEmptyStub;

HoudiniApi::GetNodeInfoFuncPtr
HoudiniApi::GetNodeInfo = &HoudiniApi::GetNodeInfoEmptyStub;

HoudiniApi::GetNodeInputNameFuncPtr
HoudiniApi::GetNodeInputName = &HoudiniApi::GetNodeInputNameEmptyStub;

HoudiniApi::GetNodeOutputNameFuncPtr
HoudiniApi::GetNodeOutputName = &HoudiniApi::GetNodeOutputNameEmptyStub;

HoudiniApi::GetNodePathFuncPtr
HoudiniApi::GetNodePath = &HoudiniApi::GetNodePathEmptyStub;

HoudiniApi::GetNumWorkItemsFuncPtr
HoudiniApi::GetNumWorkItems = &HoudiniApi::GetNumWorkItemsEmptyStub;

HoudiniApi::GetNumWorkitemsFuncPtr
HoudiniApi::GetNumWorkitems = &HoudiniApi::GetNumWorkitemsEmptyStub;

HoudiniApi::GetObjectInfoFuncPtr
HoudiniApi::GetObjectInfo = &HoudiniApi::GetObjectInfoEmptyStub;

HoudiniApi::GetObjectTransformFuncPtr
HoudiniApi::GetObjectTransform = &HoudiniApi::GetObjectTransformEmptyStub;

HoudiniApi::GetOutputGeoCountFuncPtr
HoudiniApi::GetOutputGeoCount = &HoudiniApi::GetOutputGeoCountEmptyStub;

HoudiniApi::GetOutputGeoInfosFuncPtr
HoudiniApi::GetOutputGeoInfos = &HoudiniApi::GetOutputGeoInfosEmptyStub;

HoudiniApi::GetOutputNodeIdFuncPtr
HoudiniApi::GetOutputNodeId = &HoudiniApi::GetOutputNodeIdEmptyStub;

HoudiniApi::GetPDGEventsFuncPtr
HoudiniApi::GetPDGEvents = &HoudiniApi::GetPDGEventsEmptyStub;

HoudiniApi::GetPDGGraphContextIdFuncPtr
HoudiniApi::GetPDGGraphContextId = &HoudiniApi::GetPDGGraphContextIdEmptyStub;

HoudiniApi::GetPDGGraphContextsFuncPtr
HoudiniApi::GetPDGGraphContexts = &HoudiniApi::GetPDGGraphContextsEmptyStub;

HoudiniApi::GetPDGGraphContextsCountFuncPtr
HoudiniApi::GetPDGGraphContextsCount = &HoudiniApi::GetPDGGraphContextsCountEmptyStub;

HoudiniApi::GetPDGStateFuncPtr
HoudiniApi::GetPDGState = &HoudiniApi::GetPDGStateEmptyStub;

HoudiniApi::GetParametersFuncPtr
HoudiniApi::GetParameters = &HoudiniApi::GetParametersEmptyStub;

HoudiniApi::GetParmChoiceListsFuncPtr
HoudiniApi::GetParmChoiceLists = &HoudiniApi::GetParmChoiceListsEmptyStub;

HoudiniApi::GetParmExpressionFuncPtr
HoudiniApi::GetParmExpression = &HoudiniApi::GetParmExpressionEmptyStub;

HoudiniApi::GetParmFileFuncPtr
HoudiniApi::GetParmFile = &HoudiniApi::GetParmFileEmptyStub;

HoudiniApi::GetParmFloatValueFuncPtr
HoudiniApi::GetParmFloatValue = &HoudiniApi::GetParmFloatValueEmptyStub;

HoudiniApi::GetParmFloatValuesFuncPtr
HoudiniApi::GetParmFloatValues = &HoudiniApi::GetParmFloatValuesEmptyStub;

HoudiniApi::GetParmIdFromNameFuncPtr
HoudiniApi::GetParmIdFromName = &HoudiniApi::GetParmIdFromNameEmptyStub;

HoudiniApi::GetParmInfoFuncPtr
HoudiniApi::GetParmInfo = &HoudiniApi::GetParmInfoEmptyStub;

HoudiniApi::GetParmInfoFromNameFuncPtr
HoudiniApi::GetParmInfoFromName = &HoudiniApi::GetParmInfoFromNameEmptyStub;

HoudiniApi::GetParmIntValueFuncPtr
HoudiniApi::GetParmIntValue = &HoudiniApi::GetParmIntValueEmptyStub;

HoudiniApi::GetParmIntValuesFuncPtr
HoudiniApi::GetParmIntValues = &HoudiniApi::GetParmIntValuesEmptyStub;

HoudiniApi::GetParmNodeValueFuncPtr
HoudiniApi::GetParmNodeValue = &HoudiniApi::GetParmNodeValueEmptyStub;

HoudiniApi::GetParmStringValueFuncPtr
HoudiniApi::GetParmStringValue = &HoudiniApi::GetParmStringValueEmptyStub;

HoudiniApi::GetParmStringValuesFuncPtr
HoudiniApi::GetParmStringValues = &HoudiniApi::GetParmStringValuesEmptyStub;

HoudiniApi::GetParmTagNameFuncPtr
HoudiniApi::GetParmTagName = &HoudiniApi::GetParmTagNameEmptyStub;

HoudiniApi::GetParmTagValueFuncPtr
HoudiniApi::GetParmTagValue = &HoudiniApi::GetParmTagValueEmptyStub;

HoudiniApi::GetParmWithTagFuncPtr
HoudiniApi::GetParmWithTag = &HoudiniApi::GetParmWithTagEmptyStub;

HoudiniApi::GetPartInfoFuncPtr
HoudiniApi::GetPartInfo = &HoudiniApi::GetPartInfoEmptyStub;

HoudiniApi::GetPresetFuncPtr
HoudiniApi::GetPreset = &HoudiniApi::GetPresetEmptyStub;

HoudiniApi::GetPresetBufLengthFuncPtr
HoudiniApi::GetPresetBufLength = &HoudiniApi::GetPresetBufLengthEmptyStub;

HoudiniApi::GetServerEnvIntFuncPtr
HoudiniApi::GetServerEnvInt = &HoudiniApi::GetServerEnvIntEmptyStub;

HoudiniApi::GetServerEnvStringFuncPtr
HoudiniApi::GetServerEnvString = &HoudiniApi::GetServerEnvStringEmptyStub;

HoudiniApi::GetServerEnvVarCountFuncPtr
HoudiniApi::GetServerEnvVarCount = &HoudiniApi::GetServerEnvVarCountEmptyStub;

HoudiniApi::GetServerEnvVarListFuncPtr
HoudiniApi::GetServerEnvVarList = &HoudiniApi::GetServerEnvVarListEmptyStub;

HoudiniApi::GetSessionEnvIntFuncPtr
HoudiniApi::GetSessionEnvInt = &HoudiniApi::GetSessionEnvIntEmptyStub;

HoudiniApi::GetSessionSyncInfoFuncPtr
HoudiniApi::GetSessionSyncInfo = &HoudiniApi::GetSessionSyncInfoEmptyStub;

HoudiniApi::GetSphereInfoFuncPtr
HoudiniApi::GetSphereInfo = &HoudiniApi::GetSphereInfoEmptyStub;

HoudiniApi::GetStatusFuncPtr
HoudiniApi::GetStatus = &HoudiniApi::GetStatusEmptyStub;

HoudiniApi::GetStatusStringFuncPtr
HoudiniApi::GetStatusString = &HoudiniApi::GetStatusStringEmptyStub;

HoudiniApi::GetStatusStringBufLengthFuncPtr
HoudiniApi::GetStatusStringBufLength = &HoudiniApi::GetStatusStringBufLengthEmptyStub;

HoudiniApi::GetStringFuncPtr
HoudiniApi::GetString = &HoudiniApi::GetStringEmptyStub;

HoudiniApi::GetStringBatchFuncPtr
HoudiniApi::GetStringBatch = &HoudiniApi::GetStringBatchEmptyStub;

HoudiniApi::GetStringBatchSizeFuncPtr
HoudiniApi::GetStringBatchSize = &HoudiniApi::GetStringBatchSizeEmptyStub;

HoudiniApi::GetStringBufLengthFuncPtr
HoudiniApi::GetStringBufLength = &HoudiniApi::GetStringBufLengthEmptyStub;

HoudiniApi::GetSupportedImageFileFormatCountFuncPtr
HoudiniApi::GetSupportedImageFileFormatCount = &HoudiniApi::GetSupportedImageFileFormatCountEmptyStub;

HoudiniApi::GetSupportedImageFileFormatsFuncPtr
HoudiniApi::GetSupportedImageFileFormats = &HoudiniApi::GetSupportedImageFileFormatsEmptyStub;

HoudiniApi::GetTimeFuncPtr
HoudiniApi::GetTime = &HoudiniApi::GetTimeEmptyStub;

HoudiniApi::GetTimelineOptionsFuncPtr
HoudiniApi::GetTimelineOptions = &HoudiniApi::GetTimelineOptionsEmptyStub;

HoudiniApi::GetTotalCookCountFuncPtr
HoudiniApi::GetTotalCookCount = &HoudiniApi::GetTotalCookCountEmptyStub;

HoudiniApi::GetUseHoudiniTimeFuncPtr
HoudiniApi::GetUseHoudiniTime = &HoudiniApi::GetUseHoudiniTimeEmptyStub;

HoudiniApi::GetVertexListFuncPtr
HoudiniApi::GetVertexList = &HoudiniApi::GetVertexListEmptyStub;

HoudiniApi::GetViewportFuncPtr
HoudiniApi::GetViewport = &HoudiniApi::GetViewportEmptyStub;

HoudiniApi::GetVolumeBoundsFuncPtr
HoudiniApi::GetVolumeBounds = &HoudiniApi::GetVolumeBoundsEmptyStub;

HoudiniApi::GetVolumeInfoFuncPtr
HoudiniApi::GetVolumeInfo = &HoudiniApi::GetVolumeInfoEmptyStub;

HoudiniApi::GetVolumeTileFloatDataFuncPtr
HoudiniApi::GetVolumeTileFloatData = &HoudiniApi::GetVolumeTileFloatDataEmptyStub;

HoudiniApi::GetVolumeTileIntDataFuncPtr
HoudiniApi::GetVolumeTileIntData = &HoudiniApi::GetVolumeTileIntDataEmptyStub;

HoudiniApi::GetVolumeVisualInfoFuncPtr
HoudiniApi::GetVolumeVisualInfo = &HoudiniApi::GetVolumeVisualInfoEmptyStub;

HoudiniApi::GetVolumeVoxelFloatDataFuncPtr
HoudiniApi::GetVolumeVoxelFloatData = &HoudiniApi::GetVolumeVoxelFloatDataEmptyStub;

HoudiniApi::GetVolumeVoxelIntDataFuncPtr
HoudiniApi::GetVolumeVoxelIntData = &HoudiniApi::GetVolumeVoxelIntDataEmptyStub;

HoudiniApi::GetWorkItemAttributeSizeFuncPtr
HoudiniApi::GetWorkItemAttributeSize = &HoudiniApi::GetWorkItemAttributeSizeEmptyStub;

HoudiniApi::GetWorkItemFloatAttributeFuncPtr
HoudiniApi::GetWorkItemFloatAttribute = &HoudiniApi::GetWorkItemFloatAttributeEmptyStub;

HoudiniApi::GetWorkItemInfoFuncPtr
HoudiniApi::GetWorkItemInfo = &HoudiniApi::GetWorkItemInfoEmptyStub;

HoudiniApi::GetWorkItemIntAttributeFuncPtr
HoudiniApi::GetWorkItemIntAttribute = &HoudiniApi::GetWorkItemIntAttributeEmptyStub;

HoudiniApi::GetWorkItemOutputFilesFuncPtr
HoudiniApi::GetWorkItemOutputFiles = &HoudiniApi::GetWorkItemOutputFilesEmptyStub;

HoudiniApi::GetWorkItemStringAttributeFuncPtr
HoudiniApi::GetWorkItemStringAttribute = &HoudiniApi::GetWorkItemStringAttributeEmptyStub;

HoudiniApi::GetWorkItemsFuncPtr
HoudiniApi::GetWorkItems = &HoudiniApi::GetWorkItemsEmptyStub;

HoudiniApi::GetWorkitemDataLengthFuncPtr
HoudiniApi::GetWorkitemDataLength = &HoudiniApi::GetWorkitemDataLengthEmptyStub;

HoudiniApi::GetWorkitemFloatDataFuncPtr
HoudiniApi::GetWorkitemFloatData = &HoudiniApi::GetWorkitemFloatDataEmptyStub;

HoudiniApi::GetWorkitemInfoFuncPtr
HoudiniApi::GetWorkitemInfo = &HoudiniApi::GetWorkitemInfoEmptyStub;

HoudiniApi::GetWorkitemIntDataFuncPtr
HoudiniApi::GetWorkitemIntData = &HoudiniApi::GetWorkitemIntDataEmptyStub;

HoudiniApi::GetWorkitemResultInfoFuncPtr
HoudiniApi::GetWorkitemResultInfo = &HoudiniApi::GetWorkitemResultInfoEmptyStub;

HoudiniApi::GetWorkitemStringDataFuncPtr
HoudiniApi::GetWorkitemStringData = &HoudiniApi::GetWorkitemStringDataEmptyStub;

HoudiniApi::GetWorkitemsFuncPtr
HoudiniApi::GetWorkitems = &HoudiniApi::GetWorkitemsEmptyStub;

HoudiniApi::HandleBindingInfo_CreateFuncPtr
HoudiniApi::HandleBindingInfo_Create = &HoudiniApi::HandleBindingInfo_CreateEmptyStub;

HoudiniApi::HandleBindingInfo_InitFuncPtr
HoudiniApi::HandleBindingInfo_Init = &HoudiniApi::HandleBindingInfo_InitEmptyStub;

HoudiniApi::HandleInfo_CreateFuncPtr
HoudiniApi::HandleInfo_Create = &HoudiniApi::HandleInfo_CreateEmptyStub;

HoudiniApi::HandleInfo_InitFuncPtr
HoudiniApi::HandleInfo_Init = &HoudiniApi::HandleInfo_InitEmptyStub;

HoudiniApi::ImageFileFormat_CreateFuncPtr
HoudiniApi::ImageFileFormat_Create = &HoudiniApi::ImageFileFormat_CreateEmptyStub;

HoudiniApi::ImageFileFormat_InitFuncPtr
HoudiniApi::ImageFileFormat_Init = &HoudiniApi::ImageFileFormat_InitEmptyStub;

HoudiniApi::ImageInfo_CreateFuncPtr
HoudiniApi::ImageInfo_Create = &HoudiniApi::ImageInfo_CreateEmptyStub;

HoudiniApi::ImageInfo_InitFuncPtr
HoudiniApi::ImageInfo_Init = &HoudiniApi::ImageInfo_InitEmptyStub;

HoudiniApi::InitializeFuncPtr
HoudiniApi::Initialize = &HoudiniApi::InitializeEmptyStub;

HoudiniApi::InputCurveInfo_CreateFuncPtr
HoudiniApi::InputCurveInfo_Create = &HoudiniApi::InputCurveInfo_CreateEmptyStub;

HoudiniApi::InputCurveInfo_InitFuncPtr
HoudiniApi::InputCurveInfo_Init = &HoudiniApi::InputCurveInfo_InitEmptyStub;

HoudiniApi::InsertMultiparmInstanceFuncPtr
HoudiniApi::InsertMultiparmInstance = &HoudiniApi::InsertMultiparmInstanceEmptyStub;

HoudiniApi::InterruptFuncPtr
HoudiniApi::Interrupt = &HoudiniApi::InterruptEmptyStub;

HoudiniApi::IsInitializedFuncPtr
HoudiniApi::IsInitialized = &HoudiniApi::IsInitializedEmptyStub;

HoudiniApi::IsNodeValidFuncPtr
HoudiniApi::IsNodeValid = &HoudiniApi::IsNodeValidEmptyStub;

HoudiniApi::IsSessionValidFuncPtr
HoudiniApi::IsSessionValid = &HoudiniApi::IsSessionValidEmptyStub;

HoudiniApi::Keyframe_CreateFuncPtr
HoudiniApi::Keyframe_Create = &HoudiniApi::Keyframe_CreateEmptyStub;

HoudiniApi::Keyframe_InitFuncPtr
HoudiniApi::Keyframe_Init = &HoudiniApi::Keyframe_InitEmptyStub;

HoudiniApi::LoadAssetLibraryFromFileFuncPtr
HoudiniApi::LoadAssetLibraryFromFile = &HoudiniApi::LoadAssetLibraryFromFileEmptyStub;

HoudiniApi::LoadAssetLibraryFromMemoryFuncPtr
HoudiniApi::LoadAssetLibraryFromMemory = &HoudiniApi::LoadAssetLibraryFromMemoryEmptyStub;

HoudiniApi::LoadGeoFromFileFuncPtr
HoudiniApi::LoadGeoFromFile = &HoudiniApi::LoadGeoFromFileEmptyStub;

HoudiniApi::LoadGeoFromMemoryFuncPtr
HoudiniApi::LoadGeoFromMemory = &HoudiniApi::LoadGeoFromMemoryEmptyStub;

HoudiniApi::LoadHIPFileFuncPtr
HoudiniApi::LoadHIPFile = &HoudiniApi::LoadHIPFileEmptyStub;

HoudiniApi::LoadNodeFromFileFuncPtr
HoudiniApi::LoadNodeFromFile = &HoudiniApi::LoadNodeFromFileEmptyStub;

HoudiniApi::MaterialInfo_CreateFuncPtr
HoudiniApi::MaterialInfo_Create = &HoudiniApi::MaterialInfo_CreateEmptyStub;

HoudiniApi::MaterialInfo_InitFuncPtr
HoudiniApi::MaterialInfo_Init = &HoudiniApi::MaterialInfo_InitEmptyStub;

HoudiniApi::MergeHIPFileFuncPtr
HoudiniApi::MergeHIPFile = &HoudiniApi::MergeHIPFileEmptyStub;

HoudiniApi::NodeInfo_CreateFuncPtr
HoudiniApi::NodeInfo_Create = &HoudiniApi::NodeInfo_CreateEmptyStub;

HoudiniApi::NodeInfo_InitFuncPtr
HoudiniApi::NodeInfo_Init = &HoudiniApi::NodeInfo_InitEmptyStub;

HoudiniApi::ObjectInfo_CreateFuncPtr
HoudiniApi::ObjectInfo_Create = &HoudiniApi::ObjectInfo_CreateEmptyStub;

HoudiniApi::ObjectInfo_InitFuncPtr
HoudiniApi::ObjectInfo_Init = &HoudiniApi::ObjectInfo_InitEmptyStub;

HoudiniApi::ParmChoiceInfo_CreateFuncPtr
HoudiniApi::ParmChoiceInfo_Create = &HoudiniApi::ParmChoiceInfo_CreateEmptyStub;

HoudiniApi::ParmChoiceInfo_InitFuncPtr
HoudiniApi::ParmChoiceInfo_Init = &HoudiniApi::ParmChoiceInfo_InitEmptyStub;

HoudiniApi::ParmHasExpressionFuncPtr
HoudiniApi::ParmHasExpression = &HoudiniApi::ParmHasExpressionEmptyStub;

HoudiniApi::ParmHasTagFuncPtr
HoudiniApi::ParmHasTag = &HoudiniApi::ParmHasTagEmptyStub;

HoudiniApi::ParmInfo_CreateFuncPtr
HoudiniApi::ParmInfo_Create = &HoudiniApi::ParmInfo_CreateEmptyStub;

HoudiniApi::ParmInfo_GetFloatValueCountFuncPtr
HoudiniApi::ParmInfo_GetFloatValueCount = &HoudiniApi::ParmInfo_GetFloatValueCountEmptyStub;

HoudiniApi::ParmInfo_GetIntValueCountFuncPtr
HoudiniApi::ParmInfo_GetIntValueCount = &HoudiniApi::ParmInfo_GetIntValueCountEmptyStub;

HoudiniApi::ParmInfo_GetStringValueCountFuncPtr
HoudiniApi::ParmInfo_GetStringValueCount = &HoudiniApi::ParmInfo_GetStringValueCountEmptyStub;

HoudiniApi::ParmInfo_InitFuncPtr
HoudiniApi::ParmInfo_Init = &HoudiniApi::ParmInfo_InitEmptyStub;

HoudiniApi::ParmInfo_IsFloatFuncPtr
HoudiniApi::ParmInfo_IsFloat = &HoudiniApi::ParmInfo_IsFloatEmptyStub;

HoudiniApi::ParmInfo_IsIntFuncPtr
HoudiniApi::ParmInfo_IsInt = &HoudiniApi::ParmInfo_IsIntEmptyStub;

HoudiniApi::ParmInfo_IsNodeFuncPtr
HoudiniApi::ParmInfo_IsNode = &HoudiniApi::ParmInfo_IsNodeEmptyStub;

HoudiniApi::ParmInfo_IsNonValueFuncPtr
HoudiniApi::ParmInfo_IsNonValue = &HoudiniApi::ParmInfo_IsNonValueEmptyStub;

HoudiniApi::ParmInfo_IsPathFuncPtr
HoudiniApi::ParmInfo_IsPath = &HoudiniApi::ParmInfo_IsPathEmptyStub;

HoudiniApi::ParmInfo_IsStringFuncPtr
HoudiniApi::ParmInfo_IsString = &HoudiniApi::ParmInfo_IsStringEmptyStub;

HoudiniApi::PartInfo_CreateFuncPtr
HoudiniApi::PartInfo_Create = &HoudiniApi::PartInfo_CreateEmptyStub;

HoudiniApi::PartInfo_GetAttributeCountByOwnerFuncPtr
HoudiniApi::PartInfo_GetAttributeCountByOwner = &HoudiniApi::PartInfo_GetAttributeCountByOwnerEmptyStub;

HoudiniApi::PartInfo_GetElementCountByAttributeOwnerFuncPtr
HoudiniApi::PartInfo_GetElementCountByAttributeOwner = &HoudiniApi::PartInfo_GetElementCountByAttributeOwnerEmptyStub;

HoudiniApi::PartInfo_GetElementCountByGroupTypeFuncPtr
HoudiniApi::PartInfo_GetElementCountByGroupType = &HoudiniApi::PartInfo_GetElementCountByGroupTypeEmptyStub;

HoudiniApi::PartInfo_InitFuncPtr
HoudiniApi::PartInfo_Init = &HoudiniApi::PartInfo_InitEmptyStub;

HoudiniApi::PausePDGCookFuncPtr
HoudiniApi::PausePDGCook = &HoudiniApi::PausePDGCookEmptyStub;

HoudiniApi::PythonThreadInterpreterLockFuncPtr
HoudiniApi::PythonThreadInterpreterLock = &HoudiniApi::PythonThreadInterpreterLockEmptyStub;

HoudiniApi::QueryNodeInputFuncPtr
HoudiniApi::QueryNodeInput = &HoudiniApi::QueryNodeInputEmptyStub;

HoudiniApi::QueryNodeOutputConnectedCountFuncPtr
HoudiniApi::QueryNodeOutputConnectedCount = &HoudiniApi::QueryNodeOutputConnectedCountEmptyStub;

HoudiniApi::QueryNodeOutputConnectedNodesFuncPtr
HoudiniApi::QueryNodeOutputConnectedNodes = &HoudiniApi::QueryNodeOutputConnectedNodesEmptyStub;

HoudiniApi::RemoveCustomStringFuncPtr
HoudiniApi::RemoveCustomString = &HoudiniApi::RemoveCustomStringEmptyStub;

HoudiniApi::RemoveMultiparmInstanceFuncPtr
HoudiniApi::RemoveMultiparmInstance = &HoudiniApi::RemoveMultiparmInstanceEmptyStub;

HoudiniApi::RemoveParmExpressionFuncPtr
HoudiniApi::RemoveParmExpression = &HoudiniApi::RemoveParmExpressionEmptyStub;

HoudiniApi::RenameNodeFuncPtr
HoudiniApi::RenameNode = &HoudiniApi::RenameNodeEmptyStub;

HoudiniApi::RenderCOPToImageFuncPtr
HoudiniApi::RenderCOPToImage = &HoudiniApi::RenderCOPToImageEmptyStub;

HoudiniApi::RenderTextureToImageFuncPtr
HoudiniApi::RenderTextureToImage = &HoudiniApi::RenderTextureToImageEmptyStub;

HoudiniApi::ResetSimulationFuncPtr
HoudiniApi::ResetSimulation = &HoudiniApi::ResetSimulationEmptyStub;

HoudiniApi::RevertGeoFuncPtr
HoudiniApi::RevertGeo = &HoudiniApi::RevertGeoEmptyStub;

HoudiniApi::RevertParmToDefaultFuncPtr
HoudiniApi::RevertParmToDefault = &HoudiniApi::RevertParmToDefaultEmptyStub;

HoudiniApi::RevertParmToDefaultsFuncPtr
HoudiniApi::RevertParmToDefaults = &HoudiniApi::RevertParmToDefaultsEmptyStub;

HoudiniApi::SaveGeoToFileFuncPtr
HoudiniApi::SaveGeoToFile = &HoudiniApi::SaveGeoToFileEmptyStub;

HoudiniApi::SaveGeoToMemoryFuncPtr
HoudiniApi::SaveGeoToMemory = &HoudiniApi::SaveGeoToMemoryEmptyStub;

HoudiniApi::SaveHIPFileFuncPtr
HoudiniApi::SaveHIPFile = &HoudiniApi::SaveHIPFileEmptyStub;

HoudiniApi::SaveNodeToFileFuncPtr
HoudiniApi::SaveNodeToFile = &HoudiniApi::SaveNodeToFileEmptyStub;

HoudiniApi::SessionSyncInfo_CreateFuncPtr
HoudiniApi::SessionSyncInfo_Create = &HoudiniApi::SessionSyncInfo_CreateEmptyStub;

HoudiniApi::SetAnimCurveFuncPtr
HoudiniApi::SetAnimCurve = &HoudiniApi::SetAnimCurveEmptyStub;

HoudiniApi::SetAttributeFloat64ArrayDataFuncPtr
HoudiniApi::SetAttributeFloat64ArrayData = &HoudiniApi::SetAttributeFloat64ArrayDataEmptyStub;

HoudiniApi::SetAttributeFloat64DataFuncPtr
HoudiniApi::SetAttributeFloat64Data = &HoudiniApi::SetAttributeFloat64DataEmptyStub;

HoudiniApi::SetAttributeFloatArrayDataFuncPtr
HoudiniApi::SetAttributeFloatArrayData = &HoudiniApi::SetAttributeFloatArrayDataEmptyStub;

HoudiniApi::SetAttributeFloatDataFuncPtr
HoudiniApi::SetAttributeFloatData = &HoudiniApi::SetAttributeFloatDataEmptyStub;

HoudiniApi::SetAttributeInt16ArrayDataFuncPtr
HoudiniApi::SetAttributeInt16ArrayData = &HoudiniApi::SetAttributeInt16ArrayDataEmptyStub;

HoudiniApi::SetAttributeInt16DataFuncPtr
HoudiniApi::SetAttributeInt16Data = &HoudiniApi::SetAttributeInt16DataEmptyStub;

HoudiniApi::SetAttributeInt64ArrayDataFuncPtr
HoudiniApi::SetAttributeInt64ArrayData = &HoudiniApi::SetAttributeInt64ArrayDataEmptyStub;

HoudiniApi::SetAttributeInt64DataFuncPtr
HoudiniApi::SetAttributeInt64Data = &HoudiniApi::SetAttributeInt64DataEmptyStub;

HoudiniApi::SetAttributeInt8ArrayDataFuncPtr
HoudiniApi::SetAttributeInt8ArrayData = &HoudiniApi::SetAttributeInt8ArrayDataEmptyStub;

HoudiniApi::SetAttributeInt8DataFuncPtr
HoudiniApi::SetAttributeInt8Data = &HoudiniApi::SetAttributeInt8DataEmptyStub;

HoudiniApi::SetAttributeIntArrayDataFuncPtr
HoudiniApi::SetAttributeIntArrayData = &HoudiniApi::SetAttributeIntArrayDataEmptyStub;

HoudiniApi::SetAttributeIntDataFuncPtr
HoudiniApi::SetAttributeIntData = &HoudiniApi::SetAttributeIntDataEmptyStub;

HoudiniApi::SetAttributeStringArrayDataFuncPtr
HoudiniApi::SetAttributeStringArrayData = &HoudiniApi::SetAttributeStringArrayDataEmptyStub;

HoudiniApi::SetAttributeStringDataFuncPtr
HoudiniApi::SetAttributeStringData = &HoudiniApi::SetAttributeStringDataEmptyStub;

HoudiniApi::SetAttributeUInt8ArrayDataFuncPtr
HoudiniApi::SetAttributeUInt8ArrayData = &HoudiniApi::SetAttributeUInt8ArrayDataEmptyStub;

HoudiniApi::SetAttributeUInt8DataFuncPtr
HoudiniApi::SetAttributeUInt8Data = &HoudiniApi::SetAttributeUInt8DataEmptyStub;

HoudiniApi::SetCachePropertyFuncPtr
HoudiniApi::SetCacheProperty = &HoudiniApi::SetCachePropertyEmptyStub;

HoudiniApi::SetCompositorOptionsFuncPtr
HoudiniApi::SetCompositorOptions = &HoudiniApi::SetCompositorOptionsEmptyStub;

HoudiniApi::SetCurveCountsFuncPtr
HoudiniApi::SetCurveCounts = &HoudiniApi::SetCurveCountsEmptyStub;

HoudiniApi::SetCurveInfoFuncPtr
HoudiniApi::SetCurveInfo = &HoudiniApi::SetCurveInfoEmptyStub;

HoudiniApi::SetCurveKnotsFuncPtr
HoudiniApi::SetCurveKnots = &HoudiniApi::SetCurveKnotsEmptyStub;

HoudiniApi::SetCurveOrdersFuncPtr
HoudiniApi::SetCurveOrders = &HoudiniApi::SetCurveOrdersEmptyStub;

HoudiniApi::SetCustomStringFuncPtr
HoudiniApi::SetCustomString = &HoudiniApi::SetCustomStringEmptyStub;

HoudiniApi::SetFaceCountsFuncPtr
HoudiniApi::SetFaceCounts = &HoudiniApi::SetFaceCountsEmptyStub;

HoudiniApi::SetGroupMembershipFuncPtr
HoudiniApi::SetGroupMembership = &HoudiniApi::SetGroupMembershipEmptyStub;

HoudiniApi::SetHeightFieldDataFuncPtr
HoudiniApi::SetHeightFieldData = &HoudiniApi::SetHeightFieldDataEmptyStub;

HoudiniApi::SetImageInfoFuncPtr
HoudiniApi::SetImageInfo = &HoudiniApi::SetImageInfoEmptyStub;

HoudiniApi::SetInputCurveInfoFuncPtr
HoudiniApi::SetInputCurveInfo = &HoudiniApi::SetInputCurveInfoEmptyStub;

HoudiniApi::SetInputCurvePositionsFuncPtr
HoudiniApi::SetInputCurvePositions = &HoudiniApi::SetInputCurvePositionsEmptyStub;

HoudiniApi::SetInputCurvePositionsRotationsScalesFuncPtr
HoudiniApi::SetInputCurvePositionsRotationsScales = &HoudiniApi::SetInputCurvePositionsRotationsScalesEmptyStub;

HoudiniApi::SetNodeDisplayFuncPtr
HoudiniApi::SetNodeDisplay = &HoudiniApi::SetNodeDisplayEmptyStub;

HoudiniApi::SetObjectTransformFuncPtr
HoudiniApi::SetObjectTransform = &HoudiniApi::SetObjectTransformEmptyStub;

HoudiniApi::SetParmExpressionFuncPtr
HoudiniApi::SetParmExpression = &HoudiniApi::SetParmExpressionEmptyStub;

HoudiniApi::SetParmFloatValueFuncPtr
HoudiniApi::SetParmFloatValue = &HoudiniApi::SetParmFloatValueEmptyStub;

HoudiniApi::SetParmFloatValuesFuncPtr
HoudiniApi::SetParmFloatValues = &HoudiniApi::SetParmFloatValuesEmptyStub;

HoudiniApi::SetParmIntValueFuncPtr
HoudiniApi::SetParmIntValue = &HoudiniApi::SetParmIntValueEmptyStub;

HoudiniApi::SetParmIntValuesFuncPtr
HoudiniApi::SetParmIntValues = &HoudiniApi::SetParmIntValuesEmptyStub;

HoudiniApi::SetParmNodeValueFuncPtr
HoudiniApi::SetParmNodeValue = &HoudiniApi::SetParmNodeValueEmptyStub;

HoudiniApi::SetParmStringValueFuncPtr
HoudiniApi::SetParmStringValue = &HoudiniApi::SetParmStringValueEmptyStub;

HoudiniApi::SetPartInfoFuncPtr
HoudiniApi::SetPartInfo = &HoudiniApi::SetPartInfoEmptyStub;

HoudiniApi::SetPresetFuncPtr
HoudiniApi::SetPreset = &HoudiniApi::SetPresetEmptyStub;

HoudiniApi::SetServerEnvIntFuncPtr
HoudiniApi::SetServerEnvInt = &HoudiniApi::SetServerEnvIntEmptyStub;

HoudiniApi::SetServerEnvStringFuncPtr
HoudiniApi::SetServerEnvString = &HoudiniApi::SetServerEnvStringEmptyStub;

HoudiniApi::SetSessionSyncFuncPtr
HoudiniApi::SetSessionSync = &HoudiniApi::SetSessionSyncEmptyStub;

HoudiniApi::SetSessionSyncInfoFuncPtr
HoudiniApi::SetSessionSyncInfo = &HoudiniApi::SetSessionSyncInfoEmptyStub;

HoudiniApi::SetTimeFuncPtr
HoudiniApi::SetTime = &HoudiniApi::SetTimeEmptyStub;

HoudiniApi::SetTimelineOptionsFuncPtr
HoudiniApi::SetTimelineOptions = &HoudiniApi::SetTimelineOptionsEmptyStub;

HoudiniApi::SetTransformAnimCurveFuncPtr
HoudiniApi::SetTransformAnimCurve = &HoudiniApi::SetTransformAnimCurveEmptyStub;

HoudiniApi::SetUseHoudiniTimeFuncPtr
HoudiniApi::SetUseHoudiniTime = &HoudiniApi::SetUseHoudiniTimeEmptyStub;

HoudiniApi::SetVertexListFuncPtr
HoudiniApi::SetVertexList = &HoudiniApi::SetVertexListEmptyStub;

HoudiniApi::SetViewportFuncPtr
HoudiniApi::SetViewport = &HoudiniApi::SetViewportEmptyStub;

HoudiniApi::SetVolumeInfoFuncPtr
HoudiniApi::SetVolumeInfo = &HoudiniApi::SetVolumeInfoEmptyStub;

HoudiniApi::SetVolumeTileFloatDataFuncPtr
HoudiniApi::SetVolumeTileFloatData = &HoudiniApi::SetVolumeTileFloatDataEmptyStub;

HoudiniApi::SetVolumeTileIntDataFuncPtr
HoudiniApi::SetVolumeTileIntData = &HoudiniApi::SetVolumeTileIntDataEmptyStub;

HoudiniApi::SetVolumeVoxelFloatDataFuncPtr
HoudiniApi::SetVolumeVoxelFloatData = &HoudiniApi::SetVolumeVoxelFloatDataEmptyStub;

HoudiniApi::SetVolumeVoxelIntDataFuncPtr
HoudiniApi::SetVolumeVoxelIntData = &HoudiniApi::SetVolumeVoxelIntDataEmptyStub;

HoudiniApi::SetWorkItemFloatAttributeFuncPtr
HoudiniApi::SetWorkItemFloatAttribute = &HoudiniApi::SetWorkItemFloatAttributeEmptyStub;

HoudiniApi::SetWorkItemIntAttributeFuncPtr
HoudiniApi::SetWorkItemIntAttribute = &HoudiniApi::SetWorkItemIntAttributeEmptyStub;

HoudiniApi::SetWorkItemStringAttributeFuncPtr
HoudiniApi::SetWorkItemStringAttribute = &HoudiniApi::SetWorkItemStringAttributeEmptyStub;

HoudiniApi::SetWorkitemFloatDataFuncPtr
HoudiniApi::SetWorkitemFloatData = &HoudiniApi::SetWorkitemFloatDataEmptyStub;

HoudiniApi::SetWorkitemIntDataFuncPtr
HoudiniApi::SetWorkitemIntData = &HoudiniApi::SetWorkitemIntDataEmptyStub;

HoudiniApi::SetWorkitemStringDataFuncPtr
HoudiniApi::SetWorkitemStringData = &HoudiniApi::SetWorkitemStringDataEmptyStub;

HoudiniApi::ShutdownFuncPtr
HoudiniApi::Shutdown = &HoudiniApi::ShutdownEmptyStub;

HoudiniApi::StartThriftNamedPipeServerFuncPtr
HoudiniApi::StartThriftNamedPipeServer = &HoudiniApi::StartThriftNamedPipeServerEmptyStub;

HoudiniApi::StartThriftSocketServerFuncPtr
HoudiniApi::StartThriftSocketServer = &HoudiniApi::StartThriftSocketServerEmptyStub;

HoudiniApi::ThriftServerOptions_CreateFuncPtr
HoudiniApi::ThriftServerOptions_Create = &HoudiniApi::ThriftServerOptions_CreateEmptyStub;

HoudiniApi::ThriftServerOptions_InitFuncPtr
HoudiniApi::ThriftServerOptions_Init = &HoudiniApi::ThriftServerOptions_InitEmptyStub;

HoudiniApi::TimelineOptions_CreateFuncPtr
HoudiniApi::TimelineOptions_Create = &HoudiniApi::TimelineOptions_CreateEmptyStub;

HoudiniApi::TimelineOptions_InitFuncPtr
HoudiniApi::TimelineOptions_Init = &HoudiniApi::TimelineOptions_InitEmptyStub;

HoudiniApi::TransformEuler_CreateFuncPtr
HoudiniApi::TransformEuler_Create = &HoudiniApi::TransformEuler_CreateEmptyStub;

HoudiniApi::TransformEuler_InitFuncPtr
HoudiniApi::TransformEuler_Init = &HoudiniApi::TransformEuler_InitEmptyStub;

HoudiniApi::Transform_CreateFuncPtr
HoudiniApi::Transform_Create = &HoudiniApi::Transform_CreateEmptyStub;

HoudiniApi::Transform_InitFuncPtr
HoudiniApi::Transform_Init = &HoudiniApi::Transform_InitEmptyStub;

HoudiniApi::Viewport_CreateFuncPtr
HoudiniApi::Viewport_Create = &HoudiniApi::Viewport_CreateEmptyStub;

HoudiniApi::VolumeInfo_CreateFuncPtr
HoudiniApi::VolumeInfo_Create = &HoudiniApi::VolumeInfo_CreateEmptyStub;

HoudiniApi::VolumeInfo_InitFuncPtr
HoudiniApi::VolumeInfo_Init = &HoudiniApi::VolumeInfo_InitEmptyStub;

HoudiniApi::VolumeTileInfo_CreateFuncPtr
HoudiniApi::VolumeTileInfo_Create = &HoudiniApi::VolumeTileInfo_CreateEmptyStub;

HoudiniApi::VolumeTileInfo_InitFuncPtr
HoudiniApi::VolumeTileInfo_Init = &HoudiniApi::VolumeTileInfo_InitEmptyStub;

void
HoudiniApi::InitializeHAPI(void* LibraryHandle)
{
    if(!LibraryHandle) return;

    HoudiniApi::AddAttribute = (AddAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AddAttribute");
    HoudiniApi::AddGroup = (AddGroupFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AddGroup");
    HoudiniApi::AssetInfo_Create = (AssetInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AssetInfo_Create");
    HoudiniApi::AssetInfo_Init = (AssetInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AssetInfo_Init");
    HoudiniApi::AttributeInfo_Create = (AttributeInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AttributeInfo_Create");
    HoudiniApi::AttributeInfo_Init = (AttributeInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_AttributeInfo_Init");
    HoudiniApi::BindCustomImplementation = (BindCustomImplementationFuncPtr) fetchSymbol(LibraryHandle, "HAPI_BindCustomImplementation");
    HoudiniApi::CancelPDGCook = (CancelPDGCookFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CancelPDGCook");
    HoudiniApi::CheckForSpecificErrors = (CheckForSpecificErrorsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CheckForSpecificErrors");
    HoudiniApi::Cleanup = (CleanupFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Cleanup");
    HoudiniApi::ClearConnectionError = (ClearConnectionErrorFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ClearConnectionError");
    HoudiniApi::CloseSession = (CloseSessionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CloseSession");
    HoudiniApi::CommitGeo = (CommitGeoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CommitGeo");
    HoudiniApi::CommitWorkItems = (CommitWorkItemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CommitWorkItems");
    HoudiniApi::CommitWorkitems = (CommitWorkitemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CommitWorkitems");
    HoudiniApi::ComposeChildNodeList = (ComposeChildNodeListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ComposeChildNodeList");
    HoudiniApi::ComposeNodeCookResult = (ComposeNodeCookResultFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ComposeNodeCookResult");
    HoudiniApi::ComposeObjectList = (ComposeObjectListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ComposeObjectList");
    HoudiniApi::CompositorOptions_Create = (CompositorOptions_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CompositorOptions_Create");
    HoudiniApi::CompositorOptions_Init = (CompositorOptions_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CompositorOptions_Init");
    HoudiniApi::ConnectNodeInput = (ConnectNodeInputFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConnectNodeInput");
    HoudiniApi::ConvertMatrixToEuler = (ConvertMatrixToEulerFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConvertMatrixToEuler");
    HoudiniApi::ConvertMatrixToQuat = (ConvertMatrixToQuatFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConvertMatrixToQuat");
    HoudiniApi::ConvertTransform = (ConvertTransformFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConvertTransform");
    HoudiniApi::ConvertTransformEulerToMatrix = (ConvertTransformEulerToMatrixFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConvertTransformEulerToMatrix");
    HoudiniApi::ConvertTransformQuatToMatrix = (ConvertTransformQuatToMatrixFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ConvertTransformQuatToMatrix");
    HoudiniApi::CookNode = (CookNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookNode");
    HoudiniApi::CookOptions_AreEqual = (CookOptions_AreEqualFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookOptions_AreEqual");
    HoudiniApi::CookOptions_Create = (CookOptions_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookOptions_Create");
    HoudiniApi::CookOptions_Init = (CookOptions_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookOptions_Init");
    HoudiniApi::CookPDG = (CookPDGFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookPDG");
    HoudiniApi::CookPDGAllOutputs = (CookPDGAllOutputsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CookPDGAllOutputs");
    HoudiniApi::CreateCustomSession = (CreateCustomSessionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateCustomSession");
    HoudiniApi::CreateHeightFieldInput = (CreateHeightFieldInputFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateHeightFieldInput");
    HoudiniApi::CreateHeightfieldInputVolumeNode = (CreateHeightfieldInputVolumeNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateHeightfieldInputVolumeNode");
    HoudiniApi::CreateInProcessSession = (CreateInProcessSessionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateInProcessSession");
    HoudiniApi::CreateInputCurveNode = (CreateInputCurveNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateInputCurveNode");
    HoudiniApi::CreateInputNode = (CreateInputNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateInputNode");
    HoudiniApi::CreateNode = (CreateNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateNode");
    HoudiniApi::CreateThriftNamedPipeSession = (CreateThriftNamedPipeSessionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateThriftNamedPipeSession");
    HoudiniApi::CreateThriftSocketSession = (CreateThriftSocketSessionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateThriftSocketSession");
    HoudiniApi::CreateWorkItem = (CreateWorkItemFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateWorkItem");
    HoudiniApi::CreateWorkitem = (CreateWorkitemFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CreateWorkitem");
    HoudiniApi::CurveInfo_Create = (CurveInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CurveInfo_Create");
    HoudiniApi::CurveInfo_Init = (CurveInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_CurveInfo_Init");
    HoudiniApi::DeleteAttribute = (DeleteAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DeleteAttribute");
    HoudiniApi::DeleteGroup = (DeleteGroupFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DeleteGroup");
    HoudiniApi::DeleteNode = (DeleteNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DeleteNode");
    HoudiniApi::DirtyPDGNode = (DirtyPDGNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DirtyPDGNode");
    HoudiniApi::DisconnectNodeInput = (DisconnectNodeInputFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DisconnectNodeInput");
    HoudiniApi::DisconnectNodeOutputsAt = (DisconnectNodeOutputsAtFuncPtr) fetchSymbol(LibraryHandle, "HAPI_DisconnectNodeOutputsAt");
    HoudiniApi::ExtractImageToFile = (ExtractImageToFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ExtractImageToFile");
    HoudiniApi::ExtractImageToMemory = (ExtractImageToMemoryFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ExtractImageToMemory");
    HoudiniApi::GeoInfo_Create = (GeoInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GeoInfo_Create");
    HoudiniApi::GeoInfo_GetGroupCountByType = (GeoInfo_GetGroupCountByTypeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GeoInfo_GetGroupCountByType");
    HoudiniApi::GeoInfo_Init = (GeoInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GeoInfo_Init");
    HoudiniApi::GetActiveCacheCount = (GetActiveCacheCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetActiveCacheCount");
    HoudiniApi::GetActiveCacheNames = (GetActiveCacheNamesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetActiveCacheNames");
    HoudiniApi::GetAssetDefinitionParmCounts = (GetAssetDefinitionParmCountsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAssetDefinitionParmCounts");
    HoudiniApi::GetAssetDefinitionParmInfos = (GetAssetDefinitionParmInfosFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAssetDefinitionParmInfos");
    HoudiniApi::GetAssetDefinitionParmValues = (GetAssetDefinitionParmValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAssetDefinitionParmValues");
    HoudiniApi::GetAssetInfo = (GetAssetInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAssetInfo");
    HoudiniApi::GetAttributeFloat64ArrayData = (GetAttributeFloat64ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeFloat64ArrayData");
    HoudiniApi::GetAttributeFloat64Data = (GetAttributeFloat64DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeFloat64Data");
    HoudiniApi::GetAttributeFloatArrayData = (GetAttributeFloatArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeFloatArrayData");
    HoudiniApi::GetAttributeFloatData = (GetAttributeFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeFloatData");
    HoudiniApi::GetAttributeInfo = (GetAttributeInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInfo");
    HoudiniApi::GetAttributeInt16ArrayData = (GetAttributeInt16ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt16ArrayData");
    HoudiniApi::GetAttributeInt16Data = (GetAttributeInt16DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt16Data");
    HoudiniApi::GetAttributeInt64ArrayData = (GetAttributeInt64ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt64ArrayData");
    HoudiniApi::GetAttributeInt64Data = (GetAttributeInt64DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt64Data");
    HoudiniApi::GetAttributeInt8ArrayData = (GetAttributeInt8ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt8ArrayData");
    HoudiniApi::GetAttributeInt8Data = (GetAttributeInt8DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeInt8Data");
    HoudiniApi::GetAttributeIntArrayData = (GetAttributeIntArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeIntArrayData");
    HoudiniApi::GetAttributeIntData = (GetAttributeIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeIntData");
    HoudiniApi::GetAttributeNames = (GetAttributeNamesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeNames");
    HoudiniApi::GetAttributeStringArrayData = (GetAttributeStringArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeStringArrayData");
    HoudiniApi::GetAttributeStringData = (GetAttributeStringDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeStringData");
    HoudiniApi::GetAttributeUInt8ArrayData = (GetAttributeUInt8ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeUInt8ArrayData");
    HoudiniApi::GetAttributeUInt8Data = (GetAttributeUInt8DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAttributeUInt8Data");
    HoudiniApi::GetAvailableAssetCount = (GetAvailableAssetCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAvailableAssetCount");
    HoudiniApi::GetAvailableAssets = (GetAvailableAssetsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetAvailableAssets");
    HoudiniApi::GetBoxInfo = (GetBoxInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetBoxInfo");
    HoudiniApi::GetCacheProperty = (GetCachePropertyFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCacheProperty");
    HoudiniApi::GetComposedChildNodeList = (GetComposedChildNodeListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetComposedChildNodeList");
    HoudiniApi::GetComposedNodeCookResult = (GetComposedNodeCookResultFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetComposedNodeCookResult");
    HoudiniApi::GetComposedObjectList = (GetComposedObjectListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetComposedObjectList");
    HoudiniApi::GetComposedObjectTransforms = (GetComposedObjectTransformsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetComposedObjectTransforms");
    HoudiniApi::GetCompositorOptions = (GetCompositorOptionsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCompositorOptions");
    HoudiniApi::GetConnectionError = (GetConnectionErrorFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetConnectionError");
    HoudiniApi::GetConnectionErrorLength = (GetConnectionErrorLengthFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetConnectionErrorLength");
    HoudiniApi::GetCookingCurrentCount = (GetCookingCurrentCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCookingCurrentCount");
    HoudiniApi::GetCookingTotalCount = (GetCookingTotalCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCookingTotalCount");
    HoudiniApi::GetCurveCounts = (GetCurveCountsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCurveCounts");
    HoudiniApi::GetCurveInfo = (GetCurveInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCurveInfo");
    HoudiniApi::GetCurveKnots = (GetCurveKnotsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCurveKnots");
    HoudiniApi::GetCurveOrders = (GetCurveOrdersFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetCurveOrders");
    HoudiniApi::GetDisplayGeoInfo = (GetDisplayGeoInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetDisplayGeoInfo");
    HoudiniApi::GetEdgeCountOfEdgeGroup = (GetEdgeCountOfEdgeGroupFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetEdgeCountOfEdgeGroup");
    HoudiniApi::GetEnvInt = (GetEnvIntFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetEnvInt");
    HoudiniApi::GetFaceCounts = (GetFaceCountsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetFaceCounts");
    HoudiniApi::GetFirstVolumeTile = (GetFirstVolumeTileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetFirstVolumeTile");
    HoudiniApi::GetGeoInfo = (GetGeoInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGeoInfo");
    HoudiniApi::GetGeoSize = (GetGeoSizeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGeoSize");
    HoudiniApi::GetGroupCountOnPackedInstancePart = (GetGroupCountOnPackedInstancePartFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGroupCountOnPackedInstancePart");
    HoudiniApi::GetGroupMembership = (GetGroupMembershipFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGroupMembership");
    HoudiniApi::GetGroupMembershipOnPackedInstancePart = (GetGroupMembershipOnPackedInstancePartFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGroupMembershipOnPackedInstancePart");
    HoudiniApi::GetGroupNames = (GetGroupNamesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGroupNames");
    HoudiniApi::GetGroupNamesOnPackedInstancePart = (GetGroupNamesOnPackedInstancePartFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetGroupNamesOnPackedInstancePart");
    HoudiniApi::GetHIPFileNodeCount = (GetHIPFileNodeCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetHIPFileNodeCount");
    HoudiniApi::GetHIPFileNodeIds = (GetHIPFileNodeIdsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetHIPFileNodeIds");
    HoudiniApi::GetHandleBindingInfo = (GetHandleBindingInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetHandleBindingInfo");
    HoudiniApi::GetHandleInfo = (GetHandleInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetHandleInfo");
    HoudiniApi::GetHeightFieldData = (GetHeightFieldDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetHeightFieldData");
    HoudiniApi::GetImageFilePath = (GetImageFilePathFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetImageFilePath");
    HoudiniApi::GetImageInfo = (GetImageInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetImageInfo");
    HoudiniApi::GetImageMemoryBuffer = (GetImageMemoryBufferFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetImageMemoryBuffer");
    HoudiniApi::GetImagePlaneCount = (GetImagePlaneCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetImagePlaneCount");
    HoudiniApi::GetImagePlanes = (GetImagePlanesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetImagePlanes");
    HoudiniApi::GetInputCurveInfo = (GetInputCurveInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetInputCurveInfo");
    HoudiniApi::GetInstanceTransformsOnPart = (GetInstanceTransformsOnPartFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetInstanceTransformsOnPart");
    HoudiniApi::GetInstancedObjectIds = (GetInstancedObjectIdsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetInstancedObjectIds");
    HoudiniApi::GetInstancedPartIds = (GetInstancedPartIdsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetInstancedPartIds");
    HoudiniApi::GetInstancerPartTransforms = (GetInstancerPartTransformsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetInstancerPartTransforms");
    HoudiniApi::GetManagerNodeId = (GetManagerNodeIdFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetManagerNodeId");
    HoudiniApi::GetMaterialInfo = (GetMaterialInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetMaterialInfo");
    HoudiniApi::GetMaterialNodeIdsOnFaces = (GetMaterialNodeIdsOnFacesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetMaterialNodeIdsOnFaces");
    HoudiniApi::GetNextVolumeTile = (GetNextVolumeTileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNextVolumeTile");
    HoudiniApi::GetNodeFromPath = (GetNodeFromPathFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNodeFromPath");
    HoudiniApi::GetNodeInfo = (GetNodeInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNodeInfo");
    HoudiniApi::GetNodeInputName = (GetNodeInputNameFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNodeInputName");
    HoudiniApi::GetNodeOutputName = (GetNodeOutputNameFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNodeOutputName");
    HoudiniApi::GetNodePath = (GetNodePathFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNodePath");
    HoudiniApi::GetNumWorkItems = (GetNumWorkItemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNumWorkItems");
    HoudiniApi::GetNumWorkitems = (GetNumWorkitemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetNumWorkitems");
    HoudiniApi::GetObjectInfo = (GetObjectInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetObjectInfo");
    HoudiniApi::GetObjectTransform = (GetObjectTransformFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetObjectTransform");
    HoudiniApi::GetOutputGeoCount = (GetOutputGeoCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetOutputGeoCount");
    HoudiniApi::GetOutputGeoInfos = (GetOutputGeoInfosFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetOutputGeoInfos");
    HoudiniApi::GetOutputNodeId = (GetOutputNodeIdFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetOutputNodeId");
    HoudiniApi::GetPDGEvents = (GetPDGEventsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPDGEvents");
    HoudiniApi::GetPDGGraphContextId = (GetPDGGraphContextIdFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPDGGraphContextId");
    HoudiniApi::GetPDGGraphContexts = (GetPDGGraphContextsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPDGGraphContexts");
    HoudiniApi::GetPDGGraphContextsCount = (GetPDGGraphContextsCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPDGGraphContextsCount");
    HoudiniApi::GetPDGState = (GetPDGStateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPDGState");
    HoudiniApi::GetParameters = (GetParametersFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParameters");
    HoudiniApi::GetParmChoiceLists = (GetParmChoiceListsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmChoiceLists");
    HoudiniApi::GetParmExpression = (GetParmExpressionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmExpression");
    HoudiniApi::GetParmFile = (GetParmFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmFile");
    HoudiniApi::GetParmFloatValue = (GetParmFloatValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmFloatValue");
    HoudiniApi::GetParmFloatValues = (GetParmFloatValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmFloatValues");
    HoudiniApi::GetParmIdFromName = (GetParmIdFromNameFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmIdFromName");
    HoudiniApi::GetParmInfo = (GetParmInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmInfo");
    HoudiniApi::GetParmInfoFromName = (GetParmInfoFromNameFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmInfoFromName");
    HoudiniApi::GetParmIntValue = (GetParmIntValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmIntValue");
    HoudiniApi::GetParmIntValues = (GetParmIntValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmIntValues");
    HoudiniApi::GetParmNodeValue = (GetParmNodeValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmNodeValue");
    HoudiniApi::GetParmStringValue = (GetParmStringValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmStringValue");
    HoudiniApi::GetParmStringValues = (GetParmStringValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmStringValues");
    HoudiniApi::GetParmTagName = (GetParmTagNameFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmTagName");
    HoudiniApi::GetParmTagValue = (GetParmTagValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmTagValue");
    HoudiniApi::GetParmWithTag = (GetParmWithTagFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetParmWithTag");
    HoudiniApi::GetPartInfo = (GetPartInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPartInfo");
    HoudiniApi::GetPreset = (GetPresetFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPreset");
    HoudiniApi::GetPresetBufLength = (GetPresetBufLengthFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetPresetBufLength");
    HoudiniApi::GetServerEnvInt = (GetServerEnvIntFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetServerEnvInt");
    HoudiniApi::GetServerEnvString = (GetServerEnvStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetServerEnvString");
    HoudiniApi::GetServerEnvVarCount = (GetServerEnvVarCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetServerEnvVarCount");
    HoudiniApi::GetServerEnvVarList = (GetServerEnvVarListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetServerEnvVarList");
    HoudiniApi::GetSessionEnvInt = (GetSessionEnvIntFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetSessionEnvInt");
    HoudiniApi::GetSessionSyncInfo = (GetSessionSyncInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetSessionSyncInfo");
    HoudiniApi::GetSphereInfo = (GetSphereInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetSphereInfo");
    HoudiniApi::GetStatus = (GetStatusFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStatus");
    HoudiniApi::GetStatusString = (GetStatusStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStatusString");
    HoudiniApi::GetStatusStringBufLength = (GetStatusStringBufLengthFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStatusStringBufLength");
    HoudiniApi::GetString = (GetStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetString");
    HoudiniApi::GetStringBatch = (GetStringBatchFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStringBatch");
    HoudiniApi::GetStringBatchSize = (GetStringBatchSizeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStringBatchSize");
    HoudiniApi::GetStringBufLength = (GetStringBufLengthFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetStringBufLength");
    HoudiniApi::GetSupportedImageFileFormatCount = (GetSupportedImageFileFormatCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetSupportedImageFileFormatCount");
    HoudiniApi::GetSupportedImageFileFormats = (GetSupportedImageFileFormatsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetSupportedImageFileFormats");
    HoudiniApi::GetTime = (GetTimeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetTime");
    HoudiniApi::GetTimelineOptions = (GetTimelineOptionsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetTimelineOptions");
    HoudiniApi::GetTotalCookCount = (GetTotalCookCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetTotalCookCount");
    HoudiniApi::GetUseHoudiniTime = (GetUseHoudiniTimeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetUseHoudiniTime");
    HoudiniApi::GetVertexList = (GetVertexListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVertexList");
    HoudiniApi::GetViewport = (GetViewportFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetViewport");
    HoudiniApi::GetVolumeBounds = (GetVolumeBoundsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeBounds");
    HoudiniApi::GetVolumeInfo = (GetVolumeInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeInfo");
    HoudiniApi::GetVolumeTileFloatData = (GetVolumeTileFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeTileFloatData");
    HoudiniApi::GetVolumeTileIntData = (GetVolumeTileIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeTileIntData");
    HoudiniApi::GetVolumeVisualInfo = (GetVolumeVisualInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeVisualInfo");
    HoudiniApi::GetVolumeVoxelFloatData = (GetVolumeVoxelFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeVoxelFloatData");
    HoudiniApi::GetVolumeVoxelIntData = (GetVolumeVoxelIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetVolumeVoxelIntData");
    HoudiniApi::GetWorkItemAttributeSize = (GetWorkItemAttributeSizeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemAttributeSize");
    HoudiniApi::GetWorkItemFloatAttribute = (GetWorkItemFloatAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemFloatAttribute");
    HoudiniApi::GetWorkItemInfo = (GetWorkItemInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemInfo");
    HoudiniApi::GetWorkItemIntAttribute = (GetWorkItemIntAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemIntAttribute");
    HoudiniApi::GetWorkItemOutputFiles = (GetWorkItemOutputFilesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemOutputFiles");
    HoudiniApi::GetWorkItemStringAttribute = (GetWorkItemStringAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItemStringAttribute");
    HoudiniApi::GetWorkItems = (GetWorkItemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkItems");
    HoudiniApi::GetWorkitemDataLength = (GetWorkitemDataLengthFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemDataLength");
    HoudiniApi::GetWorkitemFloatData = (GetWorkitemFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemFloatData");
    HoudiniApi::GetWorkitemInfo = (GetWorkitemInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemInfo");
    HoudiniApi::GetWorkitemIntData = (GetWorkitemIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemIntData");
    HoudiniApi::GetWorkitemResultInfo = (GetWorkitemResultInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemResultInfo");
    HoudiniApi::GetWorkitemStringData = (GetWorkitemStringDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitemStringData");
    HoudiniApi::GetWorkitems = (GetWorkitemsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_GetWorkitems");
    HoudiniApi::HandleBindingInfo_Create = (HandleBindingInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_HandleBindingInfo_Create");
    HoudiniApi::HandleBindingInfo_Init = (HandleBindingInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_HandleBindingInfo_Init");
    HoudiniApi::HandleInfo_Create = (HandleInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_HandleInfo_Create");
    HoudiniApi::HandleInfo_Init = (HandleInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_HandleInfo_Init");
    HoudiniApi::ImageFileFormat_Create = (ImageFileFormat_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ImageFileFormat_Create");
    HoudiniApi::ImageFileFormat_Init = (ImageFileFormat_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ImageFileFormat_Init");
    HoudiniApi::ImageInfo_Create = (ImageInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ImageInfo_Create");
    HoudiniApi::ImageInfo_Init = (ImageInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ImageInfo_Init");
    HoudiniApi::Initialize = (InitializeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Initialize");
    HoudiniApi::InputCurveInfo_Create = (InputCurveInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_InputCurveInfo_Create");
    HoudiniApi::InputCurveInfo_Init = (InputCurveInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_InputCurveInfo_Init");
    HoudiniApi::InsertMultiparmInstance = (InsertMultiparmInstanceFuncPtr) fetchSymbol(LibraryHandle, "HAPI_InsertMultiparmInstance");
    HoudiniApi::Interrupt = (InterruptFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Interrupt");
    HoudiniApi::IsInitialized = (IsInitializedFuncPtr) fetchSymbol(LibraryHandle, "HAPI_IsInitialized");
    HoudiniApi::IsNodeValid = (IsNodeValidFuncPtr) fetchSymbol(LibraryHandle, "HAPI_IsNodeValid");
    HoudiniApi::IsSessionValid = (IsSessionValidFuncPtr) fetchSymbol(LibraryHandle, "HAPI_IsSessionValid");
    HoudiniApi::Keyframe_Create = (Keyframe_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Keyframe_Create");
    HoudiniApi::Keyframe_Init = (Keyframe_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Keyframe_Init");
    HoudiniApi::LoadAssetLibraryFromFile = (LoadAssetLibraryFromFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadAssetLibraryFromFile");
    HoudiniApi::LoadAssetLibraryFromMemory = (LoadAssetLibraryFromMemoryFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadAssetLibraryFromMemory");
    HoudiniApi::LoadGeoFromFile = (LoadGeoFromFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadGeoFromFile");
    HoudiniApi::LoadGeoFromMemory = (LoadGeoFromMemoryFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadGeoFromMemory");
    HoudiniApi::LoadHIPFile = (LoadHIPFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadHIPFile");
    HoudiniApi::LoadNodeFromFile = (LoadNodeFromFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_LoadNodeFromFile");
    HoudiniApi::MaterialInfo_Create = (MaterialInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_MaterialInfo_Create");
    HoudiniApi::MaterialInfo_Init = (MaterialInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_MaterialInfo_Init");
    HoudiniApi::MergeHIPFile = (MergeHIPFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_MergeHIPFile");
    HoudiniApi::NodeInfo_Create = (NodeInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_NodeInfo_Create");
    HoudiniApi::NodeInfo_Init = (NodeInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_NodeInfo_Init");
    HoudiniApi::ObjectInfo_Create = (ObjectInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ObjectInfo_Create");
    HoudiniApi::ObjectInfo_Init = (ObjectInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ObjectInfo_Init");
    HoudiniApi::ParmChoiceInfo_Create = (ParmChoiceInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmChoiceInfo_Create");
    HoudiniApi::ParmChoiceInfo_Init = (ParmChoiceInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmChoiceInfo_Init");
    HoudiniApi::ParmHasExpression = (ParmHasExpressionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmHasExpression");
    HoudiniApi::ParmHasTag = (ParmHasTagFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmHasTag");
    HoudiniApi::ParmInfo_Create = (ParmInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_Create");
    HoudiniApi::ParmInfo_GetFloatValueCount = (ParmInfo_GetFloatValueCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_GetFloatValueCount");
    HoudiniApi::ParmInfo_GetIntValueCount = (ParmInfo_GetIntValueCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_GetIntValueCount");
    HoudiniApi::ParmInfo_GetStringValueCount = (ParmInfo_GetStringValueCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_GetStringValueCount");
    HoudiniApi::ParmInfo_Init = (ParmInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_Init");
    HoudiniApi::ParmInfo_IsFloat = (ParmInfo_IsFloatFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsFloat");
    HoudiniApi::ParmInfo_IsInt = (ParmInfo_IsIntFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsInt");
    HoudiniApi::ParmInfo_IsNode = (ParmInfo_IsNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsNode");
    HoudiniApi::ParmInfo_IsNonValue = (ParmInfo_IsNonValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsNonValue");
    HoudiniApi::ParmInfo_IsPath = (ParmInfo_IsPathFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsPath");
    HoudiniApi::ParmInfo_IsString = (ParmInfo_IsStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ParmInfo_IsString");
    HoudiniApi::PartInfo_Create = (PartInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PartInfo_Create");
    HoudiniApi::PartInfo_GetAttributeCountByOwner = (PartInfo_GetAttributeCountByOwnerFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PartInfo_GetAttributeCountByOwner");
    HoudiniApi::PartInfo_GetElementCountByAttributeOwner = (PartInfo_GetElementCountByAttributeOwnerFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PartInfo_GetElementCountByAttributeOwner");
    HoudiniApi::PartInfo_GetElementCountByGroupType = (PartInfo_GetElementCountByGroupTypeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PartInfo_GetElementCountByGroupType");
    HoudiniApi::PartInfo_Init = (PartInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PartInfo_Init");
    HoudiniApi::PausePDGCook = (PausePDGCookFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PausePDGCook");
    HoudiniApi::PythonThreadInterpreterLock = (PythonThreadInterpreterLockFuncPtr) fetchSymbol(LibraryHandle, "HAPI_PythonThreadInterpreterLock");
    HoudiniApi::QueryNodeInput = (QueryNodeInputFuncPtr) fetchSymbol(LibraryHandle, "HAPI_QueryNodeInput");
    HoudiniApi::QueryNodeOutputConnectedCount = (QueryNodeOutputConnectedCountFuncPtr) fetchSymbol(LibraryHandle, "HAPI_QueryNodeOutputConnectedCount");
    HoudiniApi::QueryNodeOutputConnectedNodes = (QueryNodeOutputConnectedNodesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_QueryNodeOutputConnectedNodes");
    HoudiniApi::RemoveCustomString = (RemoveCustomStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RemoveCustomString");
    HoudiniApi::RemoveMultiparmInstance = (RemoveMultiparmInstanceFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RemoveMultiparmInstance");
    HoudiniApi::RemoveParmExpression = (RemoveParmExpressionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RemoveParmExpression");
    HoudiniApi::RenameNode = (RenameNodeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RenameNode");
    HoudiniApi::RenderCOPToImage = (RenderCOPToImageFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RenderCOPToImage");
    HoudiniApi::RenderTextureToImage = (RenderTextureToImageFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RenderTextureToImage");
    HoudiniApi::ResetSimulation = (ResetSimulationFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ResetSimulation");
    HoudiniApi::RevertGeo = (RevertGeoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RevertGeo");
    HoudiniApi::RevertParmToDefault = (RevertParmToDefaultFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RevertParmToDefault");
    HoudiniApi::RevertParmToDefaults = (RevertParmToDefaultsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_RevertParmToDefaults");
    HoudiniApi::SaveGeoToFile = (SaveGeoToFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SaveGeoToFile");
    HoudiniApi::SaveGeoToMemory = (SaveGeoToMemoryFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SaveGeoToMemory");
    HoudiniApi::SaveHIPFile = (SaveHIPFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SaveHIPFile");
    HoudiniApi::SaveNodeToFile = (SaveNodeToFileFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SaveNodeToFile");
    HoudiniApi::SessionSyncInfo_Create = (SessionSyncInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SessionSyncInfo_Create");
    HoudiniApi::SetAnimCurve = (SetAnimCurveFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAnimCurve");
    HoudiniApi::SetAttributeFloat64ArrayData = (SetAttributeFloat64ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeFloat64ArrayData");
    HoudiniApi::SetAttributeFloat64Data = (SetAttributeFloat64DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeFloat64Data");
    HoudiniApi::SetAttributeFloatArrayData = (SetAttributeFloatArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeFloatArrayData");
    HoudiniApi::SetAttributeFloatData = (SetAttributeFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeFloatData");
    HoudiniApi::SetAttributeInt16ArrayData = (SetAttributeInt16ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt16ArrayData");
    HoudiniApi::SetAttributeInt16Data = (SetAttributeInt16DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt16Data");
    HoudiniApi::SetAttributeInt64ArrayData = (SetAttributeInt64ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt64ArrayData");
    HoudiniApi::SetAttributeInt64Data = (SetAttributeInt64DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt64Data");
    HoudiniApi::SetAttributeInt8ArrayData = (SetAttributeInt8ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt8ArrayData");
    HoudiniApi::SetAttributeInt8Data = (SetAttributeInt8DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeInt8Data");
    HoudiniApi::SetAttributeIntArrayData = (SetAttributeIntArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeIntArrayData");
    HoudiniApi::SetAttributeIntData = (SetAttributeIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeIntData");
    HoudiniApi::SetAttributeStringArrayData = (SetAttributeStringArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeStringArrayData");
    HoudiniApi::SetAttributeStringData = (SetAttributeStringDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeStringData");
    HoudiniApi::SetAttributeUInt8ArrayData = (SetAttributeUInt8ArrayDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeUInt8ArrayData");
    HoudiniApi::SetAttributeUInt8Data = (SetAttributeUInt8DataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetAttributeUInt8Data");
    HoudiniApi::SetCacheProperty = (SetCachePropertyFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCacheProperty");
    HoudiniApi::SetCompositorOptions = (SetCompositorOptionsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCompositorOptions");
    HoudiniApi::SetCurveCounts = (SetCurveCountsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCurveCounts");
    HoudiniApi::SetCurveInfo = (SetCurveInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCurveInfo");
    HoudiniApi::SetCurveKnots = (SetCurveKnotsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCurveKnots");
    HoudiniApi::SetCurveOrders = (SetCurveOrdersFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCurveOrders");
    HoudiniApi::SetCustomString = (SetCustomStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetCustomString");
    HoudiniApi::SetFaceCounts = (SetFaceCountsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetFaceCounts");
    HoudiniApi::SetGroupMembership = (SetGroupMembershipFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetGroupMembership");
    HoudiniApi::SetHeightFieldData = (SetHeightFieldDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetHeightFieldData");
    HoudiniApi::SetImageInfo = (SetImageInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetImageInfo");
    HoudiniApi::SetInputCurveInfo = (SetInputCurveInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetInputCurveInfo");
    HoudiniApi::SetInputCurvePositions = (SetInputCurvePositionsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetInputCurvePositions");
    HoudiniApi::SetInputCurvePositionsRotationsScales = (SetInputCurvePositionsRotationsScalesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetInputCurvePositionsRotationsScales");
    HoudiniApi::SetNodeDisplay = (SetNodeDisplayFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetNodeDisplay");
    HoudiniApi::SetObjectTransform = (SetObjectTransformFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetObjectTransform");
    HoudiniApi::SetParmExpression = (SetParmExpressionFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmExpression");
    HoudiniApi::SetParmFloatValue = (SetParmFloatValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmFloatValue");
    HoudiniApi::SetParmFloatValues = (SetParmFloatValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmFloatValues");
    HoudiniApi::SetParmIntValue = (SetParmIntValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmIntValue");
    HoudiniApi::SetParmIntValues = (SetParmIntValuesFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmIntValues");
    HoudiniApi::SetParmNodeValue = (SetParmNodeValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmNodeValue");
    HoudiniApi::SetParmStringValue = (SetParmStringValueFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetParmStringValue");
    HoudiniApi::SetPartInfo = (SetPartInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetPartInfo");
    HoudiniApi::SetPreset = (SetPresetFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetPreset");
    HoudiniApi::SetServerEnvInt = (SetServerEnvIntFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetServerEnvInt");
    HoudiniApi::SetServerEnvString = (SetServerEnvStringFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetServerEnvString");
    HoudiniApi::SetSessionSync = (SetSessionSyncFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetSessionSync");
    HoudiniApi::SetSessionSyncInfo = (SetSessionSyncInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetSessionSyncInfo");
    HoudiniApi::SetTime = (SetTimeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetTime");
    HoudiniApi::SetTimelineOptions = (SetTimelineOptionsFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetTimelineOptions");
    HoudiniApi::SetTransformAnimCurve = (SetTransformAnimCurveFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetTransformAnimCurve");
    HoudiniApi::SetUseHoudiniTime = (SetUseHoudiniTimeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetUseHoudiniTime");
    HoudiniApi::SetVertexList = (SetVertexListFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVertexList");
    HoudiniApi::SetViewport = (SetViewportFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetViewport");
    HoudiniApi::SetVolumeInfo = (SetVolumeInfoFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVolumeInfo");
    HoudiniApi::SetVolumeTileFloatData = (SetVolumeTileFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVolumeTileFloatData");
    HoudiniApi::SetVolumeTileIntData = (SetVolumeTileIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVolumeTileIntData");
    HoudiniApi::SetVolumeVoxelFloatData = (SetVolumeVoxelFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVolumeVoxelFloatData");
    HoudiniApi::SetVolumeVoxelIntData = (SetVolumeVoxelIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetVolumeVoxelIntData");
    HoudiniApi::SetWorkItemFloatAttribute = (SetWorkItemFloatAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkItemFloatAttribute");
    HoudiniApi::SetWorkItemIntAttribute = (SetWorkItemIntAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkItemIntAttribute");
    HoudiniApi::SetWorkItemStringAttribute = (SetWorkItemStringAttributeFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkItemStringAttribute");
    HoudiniApi::SetWorkitemFloatData = (SetWorkitemFloatDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkitemFloatData");
    HoudiniApi::SetWorkitemIntData = (SetWorkitemIntDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkitemIntData");
    HoudiniApi::SetWorkitemStringData = (SetWorkitemStringDataFuncPtr) fetchSymbol(LibraryHandle, "HAPI_SetWorkitemStringData");
    HoudiniApi::Shutdown = (ShutdownFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Shutdown");
    HoudiniApi::StartThriftNamedPipeServer = (StartThriftNamedPipeServerFuncPtr) fetchSymbol(LibraryHandle, "HAPI_StartThriftNamedPipeServer");
    HoudiniApi::StartThriftSocketServer = (StartThriftSocketServerFuncPtr) fetchSymbol(LibraryHandle, "HAPI_StartThriftSocketServer");
    HoudiniApi::ThriftServerOptions_Create = (ThriftServerOptions_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ThriftServerOptions_Create");
    HoudiniApi::ThriftServerOptions_Init = (ThriftServerOptions_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_ThriftServerOptions_Init");
    HoudiniApi::TimelineOptions_Create = (TimelineOptions_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_TimelineOptions_Create");
    HoudiniApi::TimelineOptions_Init = (TimelineOptions_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_TimelineOptions_Init");
    HoudiniApi::TransformEuler_Create = (TransformEuler_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_TransformEuler_Create");
    HoudiniApi::TransformEuler_Init = (TransformEuler_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_TransformEuler_Init");
    HoudiniApi::Transform_Create = (Transform_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Transform_Create");
    HoudiniApi::Transform_Init = (Transform_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Transform_Init");
    HoudiniApi::Viewport_Create = (Viewport_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_Viewport_Create");
    HoudiniApi::VolumeInfo_Create = (VolumeInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_VolumeInfo_Create");
    HoudiniApi::VolumeInfo_Init = (VolumeInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_VolumeInfo_Init");
    HoudiniApi::VolumeTileInfo_Create = (VolumeTileInfo_CreateFuncPtr) fetchSymbol(LibraryHandle, "HAPI_VolumeTileInfo_Create");
    HoudiniApi::VolumeTileInfo_Init = (VolumeTileInfo_InitFuncPtr) fetchSymbol(LibraryHandle, "HAPI_VolumeTileInfo_Init");
}

void
HoudiniApi::FinalizeHAPI()
{
    HoudiniApi::AddAttribute = &HoudiniApi::AddAttributeEmptyStub;
    HoudiniApi::AddGroup = &HoudiniApi::AddGroupEmptyStub;
    HoudiniApi::AssetInfo_Create = &HoudiniApi::AssetInfo_CreateEmptyStub;
    HoudiniApi::AssetInfo_Init = &HoudiniApi::AssetInfo_InitEmptyStub;
    HoudiniApi::AttributeInfo_Create = &HoudiniApi::AttributeInfo_CreateEmptyStub;
    HoudiniApi::AttributeInfo_Init = &HoudiniApi::AttributeInfo_InitEmptyStub;
    HoudiniApi::BindCustomImplementation = &HoudiniApi::BindCustomImplementationEmptyStub;
    HoudiniApi::CancelPDGCook = &HoudiniApi::CancelPDGCookEmptyStub;
    HoudiniApi::CheckForSpecificErrors = &HoudiniApi::CheckForSpecificErrorsEmptyStub;
    HoudiniApi::Cleanup = &HoudiniApi::CleanupEmptyStub;
    HoudiniApi::ClearConnectionError = &HoudiniApi::ClearConnectionErrorEmptyStub;
    HoudiniApi::CloseSession = &HoudiniApi::CloseSessionEmptyStub;
    HoudiniApi::CommitGeo = &HoudiniApi::CommitGeoEmptyStub;
    HoudiniApi::CommitWorkItems = &HoudiniApi::CommitWorkItemsEmptyStub;
    HoudiniApi::CommitWorkitems = &HoudiniApi::CommitWorkitemsEmptyStub;
    HoudiniApi::ComposeChildNodeList = &HoudiniApi::ComposeChildNodeListEmptyStub;
    HoudiniApi::ComposeNodeCookResult = &HoudiniApi::ComposeNodeCookResultEmptyStub;
    HoudiniApi::ComposeObjectList = &HoudiniApi::ComposeObjectListEmptyStub;
    HoudiniApi::CompositorOptions_Create = &HoudiniApi::CompositorOptions_CreateEmptyStub;
    HoudiniApi::CompositorOptions_Init = &HoudiniApi::CompositorOptions_InitEmptyStub;
    HoudiniApi::ConnectNodeInput = &HoudiniApi::ConnectNodeInputEmptyStub;
    HoudiniApi::ConvertMatrixToEuler = &HoudiniApi::ConvertMatrixToEulerEmptyStub;
    HoudiniApi::ConvertMatrixToQuat = &HoudiniApi::ConvertMatrixToQuatEmptyStub;
    HoudiniApi::ConvertTransform = &HoudiniApi::ConvertTransformEmptyStub;
    HoudiniApi::ConvertTransformEulerToMatrix = &HoudiniApi::ConvertTransformEulerToMatrixEmptyStub;
    HoudiniApi::ConvertTransformQuatToMatrix = &HoudiniApi::ConvertTransformQuatToMatrixEmptyStub;
    HoudiniApi::CookNode = &HoudiniApi::CookNodeEmptyStub;
    HoudiniApi::CookOptions_AreEqual = &HoudiniApi::CookOptions_AreEqualEmptyStub;
    HoudiniApi::CookOptions_Create = &HoudiniApi::CookOptions_CreateEmptyStub;
    HoudiniApi::CookOptions_Init = &HoudiniApi::CookOptions_InitEmptyStub;
    HoudiniApi::CookPDG = &HoudiniApi::CookPDGEmptyStub;
    HoudiniApi::CookPDGAllOutputs = &HoudiniApi::CookPDGAllOutputsEmptyStub;
    HoudiniApi::CreateCustomSession = &HoudiniApi::CreateCustomSessionEmptyStub;
    HoudiniApi::CreateHeightFieldInput = &HoudiniApi::CreateHeightFieldInputEmptyStub;
    HoudiniApi::CreateHeightfieldInputVolumeNode = &HoudiniApi::CreateHeightfieldInputVolumeNodeEmptyStub;
    HoudiniApi::CreateInProcessSession = &HoudiniApi::CreateInProcessSessionEmptyStub;
    HoudiniApi::CreateInputCurveNode = &HoudiniApi::CreateInputCurveNodeEmptyStub;
    HoudiniApi::CreateInputNode = &HoudiniApi::CreateInputNodeEmptyStub;
    HoudiniApi::CreateNode = &HoudiniApi::CreateNodeEmptyStub;
    HoudiniApi::CreateThriftNamedPipeSession = &HoudiniApi::CreateThriftNamedPipeSessionEmptyStub;
    HoudiniApi::CreateThriftSocketSession = &HoudiniApi::CreateThriftSocketSessionEmptyStub;
    HoudiniApi::CreateWorkItem = &HoudiniApi::CreateWorkItemEmptyStub;
    HoudiniApi::CreateWorkitem = &HoudiniApi::CreateWorkitemEmptyStub;
    HoudiniApi::CurveInfo_Create = &HoudiniApi::CurveInfo_CreateEmptyStub;
    HoudiniApi::CurveInfo_Init = &HoudiniApi::CurveInfo_InitEmptyStub;
    HoudiniApi::DeleteAttribute = &HoudiniApi::DeleteAttributeEmptyStub;
    HoudiniApi::DeleteGroup = &HoudiniApi::DeleteGroupEmptyStub;
    HoudiniApi::DeleteNode = &HoudiniApi::DeleteNodeEmptyStub;
    HoudiniApi::DirtyPDGNode = &HoudiniApi::DirtyPDGNodeEmptyStub;
    HoudiniApi::DisconnectNodeInput = &HoudiniApi::DisconnectNodeInputEmptyStub;
    HoudiniApi::DisconnectNodeOutputsAt = &HoudiniApi::DisconnectNodeOutputsAtEmptyStub;
    HoudiniApi::ExtractImageToFile = &HoudiniApi::ExtractImageToFileEmptyStub;
    HoudiniApi::ExtractImageToMemory = &HoudiniApi::ExtractImageToMemoryEmptyStub;
    HoudiniApi::GeoInfo_Create = &HoudiniApi::GeoInfo_CreateEmptyStub;
    HoudiniApi::GeoInfo_GetGroupCountByType = &HoudiniApi::GeoInfo_GetGroupCountByTypeEmptyStub;
    HoudiniApi::GeoInfo_Init = &HoudiniApi::GeoInfo_InitEmptyStub;
    HoudiniApi::GetActiveCacheCount = &HoudiniApi::GetActiveCacheCountEmptyStub;
    HoudiniApi::GetActiveCacheNames = &HoudiniApi::GetActiveCacheNamesEmptyStub;
    HoudiniApi::GetAssetDefinitionParmCounts = &HoudiniApi::GetAssetDefinitionParmCountsEmptyStub;
    HoudiniApi::GetAssetDefinitionParmInfos = &HoudiniApi::GetAssetDefinitionParmInfosEmptyStub;
    HoudiniApi::GetAssetDefinitionParmValues = &HoudiniApi::GetAssetDefinitionParmValuesEmptyStub;
    HoudiniApi::GetAssetInfo = &HoudiniApi::GetAssetInfoEmptyStub;
    HoudiniApi::GetAttributeFloat64ArrayData = &HoudiniApi::GetAttributeFloat64ArrayDataEmptyStub;
    HoudiniApi::GetAttributeFloat64Data = &HoudiniApi::GetAttributeFloat64DataEmptyStub;
    HoudiniApi::GetAttributeFloatArrayData = &HoudiniApi::GetAttributeFloatArrayDataEmptyStub;
    HoudiniApi::GetAttributeFloatData = &HoudiniApi::GetAttributeFloatDataEmptyStub;
    HoudiniApi::GetAttributeInfo = &HoudiniApi::GetAttributeInfoEmptyStub;
    HoudiniApi::GetAttributeInt16ArrayData = &HoudiniApi::GetAttributeInt16ArrayDataEmptyStub;
    HoudiniApi::GetAttributeInt16Data = &HoudiniApi::GetAttributeInt16DataEmptyStub;
    HoudiniApi::GetAttributeInt64ArrayData = &HoudiniApi::GetAttributeInt64ArrayDataEmptyStub;
    HoudiniApi::GetAttributeInt64Data = &HoudiniApi::GetAttributeInt64DataEmptyStub;
    HoudiniApi::GetAttributeInt8ArrayData = &HoudiniApi::GetAttributeInt8ArrayDataEmptyStub;
    HoudiniApi::GetAttributeInt8Data = &HoudiniApi::GetAttributeInt8DataEmptyStub;
    HoudiniApi::GetAttributeIntArrayData = &HoudiniApi::GetAttributeIntArrayDataEmptyStub;
    HoudiniApi::GetAttributeIntData = &HoudiniApi::GetAttributeIntDataEmptyStub;
    HoudiniApi::GetAttributeNames = &HoudiniApi::GetAttributeNamesEmptyStub;
    HoudiniApi::GetAttributeStringArrayData = &HoudiniApi::GetAttributeStringArrayDataEmptyStub;
    HoudiniApi::GetAttributeStringData = &HoudiniApi::GetAttributeStringDataEmptyStub;
    HoudiniApi::GetAttributeUInt8ArrayData = &HoudiniApi::GetAttributeUInt8ArrayDataEmptyStub;
    HoudiniApi::GetAttributeUInt8Data = &HoudiniApi::GetAttributeUInt8DataEmptyStub;
    HoudiniApi::GetAvailableAssetCount = &HoudiniApi::GetAvailableAssetCountEmptyStub;
    HoudiniApi::GetAvailableAssets = &HoudiniApi::GetAvailableAssetsEmptyStub;
    HoudiniApi::GetBoxInfo = &HoudiniApi::GetBoxInfoEmptyStub;
    HoudiniApi::GetCacheProperty = &HoudiniApi::GetCachePropertyEmptyStub;
    HoudiniApi::GetComposedChildNodeList = &HoudiniApi::GetComposedChildNodeListEmptyStub;
    HoudiniApi::GetComposedNodeCookResult = &HoudiniApi::GetComposedNodeCookResultEmptyStub;
    HoudiniApi::GetComposedObjectList = &HoudiniApi::GetComposedObjectListEmptyStub;
    HoudiniApi::GetComposedObjectTransforms = &HoudiniApi::GetComposedObjectTransformsEmptyStub;
    HoudiniApi::GetCompositorOptions = &HoudiniApi::GetCompositorOptionsEmptyStub;
    HoudiniApi::GetConnectionError = &HoudiniApi::GetConnectionErrorEmptyStub;
    HoudiniApi::GetConnectionErrorLength = &HoudiniApi::GetConnectionErrorLengthEmptyStub;
    HoudiniApi::GetCookingCurrentCount = &HoudiniApi::GetCookingCurrentCountEmptyStub;
    HoudiniApi::GetCookingTotalCount = &HoudiniApi::GetCookingTotalCountEmptyStub;
    HoudiniApi::GetCurveCounts = &HoudiniApi::GetCurveCountsEmptyStub;
    HoudiniApi::GetCurveInfo = &HoudiniApi::GetCurveInfoEmptyStub;
    HoudiniApi::GetCurveKnots = &HoudiniApi::GetCurveKnotsEmptyStub;
    HoudiniApi::GetCurveOrders = &HoudiniApi::GetCurveOrdersEmptyStub;
    HoudiniApi::GetDisplayGeoInfo = &HoudiniApi::GetDisplayGeoInfoEmptyStub;
    HoudiniApi::GetEdgeCountOfEdgeGroup = &HoudiniApi::GetEdgeCountOfEdgeGroupEmptyStub;
    HoudiniApi::GetEnvInt = &HoudiniApi::GetEnvIntEmptyStub;
    HoudiniApi::GetFaceCounts = &HoudiniApi::GetFaceCountsEmptyStub;
    HoudiniApi::GetFirstVolumeTile = &HoudiniApi::GetFirstVolumeTileEmptyStub;
    HoudiniApi::GetGeoInfo = &HoudiniApi::GetGeoInfoEmptyStub;
    HoudiniApi::GetGeoSize = &HoudiniApi::GetGeoSizeEmptyStub;
    HoudiniApi::GetGroupCountOnPackedInstancePart = &HoudiniApi::GetGroupCountOnPackedInstancePartEmptyStub;
    HoudiniApi::GetGroupMembership = &HoudiniApi::GetGroupMembershipEmptyStub;
    HoudiniApi::GetGroupMembershipOnPackedInstancePart = &HoudiniApi::GetGroupMembershipOnPackedInstancePartEmptyStub;
    HoudiniApi::GetGroupNames = &HoudiniApi::GetGroupNamesEmptyStub;
    HoudiniApi::GetGroupNamesOnPackedInstancePart = &HoudiniApi::GetGroupNamesOnPackedInstancePartEmptyStub;
    HoudiniApi::GetHIPFileNodeCount = &HoudiniApi::GetHIPFileNodeCountEmptyStub;
    HoudiniApi::GetHIPFileNodeIds = &HoudiniApi::GetHIPFileNodeIdsEmptyStub;
    HoudiniApi::GetHandleBindingInfo = &HoudiniApi::GetHandleBindingInfoEmptyStub;
    HoudiniApi::GetHandleInfo = &HoudiniApi::GetHandleInfoEmptyStub;
    HoudiniApi::GetHeightFieldData = &HoudiniApi::GetHeightFieldDataEmptyStub;
    HoudiniApi::GetImageFilePath = &HoudiniApi::GetImageFilePathEmptyStub;
    HoudiniApi::GetImageInfo = &HoudiniApi::GetImageInfoEmptyStub;
    HoudiniApi::GetImageMemoryBuffer = &HoudiniApi::GetImageMemoryBufferEmptyStub;
    HoudiniApi::GetImagePlaneCount = &HoudiniApi::GetImagePlaneCountEmptyStub;
    HoudiniApi::GetImagePlanes = &HoudiniApi::GetImagePlanesEmptyStub;
    HoudiniApi::GetInputCurveInfo = &HoudiniApi::GetInputCurveInfoEmptyStub;
    HoudiniApi::GetInstanceTransformsOnPart = &HoudiniApi::GetInstanceTransformsOnPartEmptyStub;
    HoudiniApi::GetInstancedObjectIds = &HoudiniApi::GetInstancedObjectIdsEmptyStub;
    HoudiniApi::GetInstancedPartIds = &HoudiniApi::GetInstancedPartIdsEmptyStub;
    HoudiniApi::GetInstancerPartTransforms = &HoudiniApi::GetInstancerPartTransformsEmptyStub;
    HoudiniApi::GetManagerNodeId = &HoudiniApi::GetManagerNodeIdEmptyStub;
    HoudiniApi::GetMaterialInfo = &HoudiniApi::GetMaterialInfoEmptyStub;
    HoudiniApi::GetMaterialNodeIdsOnFaces = &HoudiniApi::GetMaterialNodeIdsOnFacesEmptyStub;
    HoudiniApi::GetNextVolumeTile = &HoudiniApi::GetNextVolumeTileEmptyStub;
    HoudiniApi::GetNodeFromPath = &HoudiniApi::GetNodeFromPathEmptyStub;
    HoudiniApi::GetNodeInfo = &HoudiniApi::GetNodeInfoEmptyStub;
    HoudiniApi::GetNodeInputName = &HoudiniApi::GetNodeInputNameEmptyStub;
    HoudiniApi::GetNodeOutputName = &HoudiniApi::GetNodeOutputNameEmptyStub;
    HoudiniApi::GetNodePath = &HoudiniApi::GetNodePathEmptyStub;
    HoudiniApi::GetNumWorkItems = &HoudiniApi::GetNumWorkItemsEmptyStub;
    HoudiniApi::GetNumWorkitems = &HoudiniApi::GetNumWorkitemsEmptyStub;
    HoudiniApi::GetObjectInfo = &HoudiniApi::GetObjectInfoEmptyStub;
    HoudiniApi::GetObjectTransform = &HoudiniApi::GetObjectTransformEmptyStub;
    HoudiniApi::GetOutputGeoCount = &HoudiniApi::GetOutputGeoCountEmptyStub;
    HoudiniApi::GetOutputGeoInfos = &HoudiniApi::GetOutputGeoInfosEmptyStub;
    HoudiniApi::GetOutputNodeId = &HoudiniApi::GetOutputNodeIdEmptyStub;
    HoudiniApi::GetPDGEvents = &HoudiniApi::GetPDGEventsEmptyStub;
    HoudiniApi::GetPDGGraphContextId = &HoudiniApi::GetPDGGraphContextIdEmptyStub;
    HoudiniApi::GetPDGGraphContexts = &HoudiniApi::GetPDGGraphContextsEmptyStub;
    HoudiniApi::GetPDGGraphContextsCount = &HoudiniApi::GetPDGGraphContextsCountEmptyStub;
    HoudiniApi::GetPDGState = &HoudiniApi::GetPDGStateEmptyStub;
    HoudiniApi::GetParameters = &HoudiniApi::GetParametersEmptyStub;
    HoudiniApi::GetParmChoiceLists = &HoudiniApi::GetParmChoiceListsEmptyStub;
    HoudiniApi::GetParmExpression = &HoudiniApi::GetParmExpressionEmptyStub;
    HoudiniApi::GetParmFile = &HoudiniApi::GetParmFileEmptyStub;
    HoudiniApi::GetParmFloatValue = &HoudiniApi::GetParmFloatValueEmptyStub;
    HoudiniApi::GetParmFloatValues = &HoudiniApi::GetParmFloatValuesEmptyStub;
    HoudiniApi::GetParmIdFromName = &HoudiniApi::GetParmIdFromNameEmptyStub;
    HoudiniApi::GetParmInfo = &HoudiniApi::GetParmInfoEmptyStub;
    HoudiniApi::GetParmInfoFromName = &HoudiniApi::GetParmInfoFromNameEmptyStub;
    HoudiniApi::GetParmIntValue = &HoudiniApi::GetParmIntValueEmptyStub;
    HoudiniApi::GetParmIntValues = &HoudiniApi::GetParmIntValuesEmptyStub;
    HoudiniApi::GetParmNodeValue = &HoudiniApi::GetParmNodeValueEmptyStub;
    HoudiniApi::GetParmStringValue = &HoudiniApi::GetParmStringValueEmptyStub;
    HoudiniApi::GetParmStringValues = &HoudiniApi::GetParmStringValuesEmptyStub;
    HoudiniApi::GetParmTagName = &HoudiniApi::GetParmTagNameEmptyStub;
    HoudiniApi::GetParmTagValue = &HoudiniApi::GetParmTagValueEmptyStub;
    HoudiniApi::GetParmWithTag = &HoudiniApi::GetParmWithTagEmptyStub;
    HoudiniApi::GetPartInfo = &HoudiniApi::GetPartInfoEmptyStub;
    HoudiniApi::GetPreset = &HoudiniApi::GetPresetEmptyStub;
    HoudiniApi::GetPresetBufLength = &HoudiniApi::GetPresetBufLengthEmptyStub;
    HoudiniApi::GetServerEnvInt = &HoudiniApi::GetServerEnvIntEmptyStub;
    HoudiniApi::GetServerEnvString = &HoudiniApi::GetServerEnvStringEmptyStub;
    HoudiniApi::GetServerEnvVarCount = &HoudiniApi::GetServerEnvVarCountEmptyStub;
    HoudiniApi::GetServerEnvVarList = &HoudiniApi::GetServerEnvVarListEmptyStub;
    HoudiniApi::GetSessionEnvInt = &HoudiniApi::GetSessionEnvIntEmptyStub;
    HoudiniApi::GetSessionSyncInfo = &HoudiniApi::GetSessionSyncInfoEmptyStub;
    HoudiniApi::GetSphereInfo = &HoudiniApi::GetSphereInfoEmptyStub;
    HoudiniApi::GetStatus = &HoudiniApi::GetStatusEmptyStub;
    HoudiniApi::GetStatusString = &HoudiniApi::GetStatusStringEmptyStub;
    HoudiniApi::GetStatusStringBufLength = &HoudiniApi::GetStatusStringBufLengthEmptyStub;
    HoudiniApi::GetString = &HoudiniApi::GetStringEmptyStub;
    HoudiniApi::GetStringBatch = &HoudiniApi::GetStringBatchEmptyStub;
    HoudiniApi::GetStringBatchSize = &HoudiniApi::GetStringBatchSizeEmptyStub;
    HoudiniApi::GetStringBufLength = &HoudiniApi::GetStringBufLengthEmptyStub;
    HoudiniApi::GetSupportedImageFileFormatCount = &HoudiniApi::GetSupportedImageFileFormatCountEmptyStub;
    HoudiniApi::GetSupportedImageFileFormats = &HoudiniApi::GetSupportedImageFileFormatsEmptyStub;
    HoudiniApi::GetTime = &HoudiniApi::GetTimeEmptyStub;
    HoudiniApi::GetTimelineOptions = &HoudiniApi::GetTimelineOptionsEmptyStub;
    HoudiniApi::GetTotalCookCount = &HoudiniApi::GetTotalCookCountEmptyStub;
    HoudiniApi::GetUseHoudiniTime = &HoudiniApi::GetUseHoudiniTimeEmptyStub;
    HoudiniApi::GetVertexList = &HoudiniApi::GetVertexListEmptyStub;
    HoudiniApi::GetViewport = &HoudiniApi::GetViewportEmptyStub;
    HoudiniApi::GetVolumeBounds = &HoudiniApi::GetVolumeBoundsEmptyStub;
    HoudiniApi::GetVolumeInfo = &HoudiniApi::GetVolumeInfoEmptyStub;
    HoudiniApi::GetVolumeTileFloatData = &HoudiniApi::GetVolumeTileFloatDataEmptyStub;
    HoudiniApi::GetVolumeTileIntData = &HoudiniApi::GetVolumeTileIntDataEmptyStub;
    HoudiniApi::GetVolumeVisualInfo = &HoudiniApi::GetVolumeVisualInfoEmptyStub;
    HoudiniApi::GetVolumeVoxelFloatData = &HoudiniApi::GetVolumeVoxelFloatDataEmptyStub;
    HoudiniApi::GetVolumeVoxelIntData = &HoudiniApi::GetVolumeVoxelIntDataEmptyStub;
    HoudiniApi::GetWorkItemAttributeSize = &HoudiniApi::GetWorkItemAttributeSizeEmptyStub;
    HoudiniApi::GetWorkItemFloatAttribute = &HoudiniApi::GetWorkItemFloatAttributeEmptyStub;
    HoudiniApi::GetWorkItemInfo = &HoudiniApi::GetWorkItemInfoEmptyStub;
    HoudiniApi::GetWorkItemIntAttribute = &HoudiniApi::GetWorkItemIntAttributeEmptyStub;
    HoudiniApi::GetWorkItemOutputFiles = &HoudiniApi::GetWorkItemOutputFilesEmptyStub;
    HoudiniApi::GetWorkItemStringAttribute = &HoudiniApi::GetWorkItemStringAttributeEmptyStub;
    HoudiniApi::GetWorkItems = &HoudiniApi::GetWorkItemsEmptyStub;
    HoudiniApi::GetWorkitemDataLength = &HoudiniApi::GetWorkitemDataLengthEmptyStub;
    HoudiniApi::GetWorkitemFloatData = &HoudiniApi::GetWorkitemFloatDataEmptyStub;
    HoudiniApi::GetWorkitemInfo = &HoudiniApi::GetWorkitemInfoEmptyStub;
    HoudiniApi::GetWorkitemIntData = &HoudiniApi::GetWorkitemIntDataEmptyStub;
    HoudiniApi::GetWorkitemResultInfo = &HoudiniApi::GetWorkitemResultInfoEmptyStub;
    HoudiniApi::GetWorkitemStringData = &HoudiniApi::GetWorkitemStringDataEmptyStub;
    HoudiniApi::GetWorkitems = &HoudiniApi::GetWorkitemsEmptyStub;
    HoudiniApi::HandleBindingInfo_Create = &HoudiniApi::HandleBindingInfo_CreateEmptyStub;
    HoudiniApi::HandleBindingInfo_Init = &HoudiniApi::HandleBindingInfo_InitEmptyStub;
    HoudiniApi::HandleInfo_Create = &HoudiniApi::HandleInfo_CreateEmptyStub;
    HoudiniApi::HandleInfo_Init = &HoudiniApi::HandleInfo_InitEmptyStub;
    HoudiniApi::ImageFileFormat_Create = &HoudiniApi::ImageFileFormat_CreateEmptyStub;
    HoudiniApi::ImageFileFormat_Init = &HoudiniApi::ImageFileFormat_InitEmptyStub;
    HoudiniApi::ImageInfo_Create = &HoudiniApi::ImageInfo_CreateEmptyStub;
    HoudiniApi::ImageInfo_Init = &HoudiniApi::ImageInfo_InitEmptyStub;
    HoudiniApi::Initialize = &HoudiniApi::InitializeEmptyStub;
    HoudiniApi::InputCurveInfo_Create = &HoudiniApi::InputCurveInfo_CreateEmptyStub;
    HoudiniApi::InputCurveInfo_Init = &HoudiniApi::InputCurveInfo_InitEmptyStub;
    HoudiniApi::InsertMultiparmInstance = &HoudiniApi::InsertMultiparmInstanceEmptyStub;
    HoudiniApi::Interrupt = &HoudiniApi::InterruptEmptyStub;
    HoudiniApi::IsInitialized = &HoudiniApi::IsInitializedEmptyStub;
    HoudiniApi::IsNodeValid = &HoudiniApi::IsNodeValidEmptyStub;
    HoudiniApi::IsSessionValid = &HoudiniApi::IsSessionValidEmptyStub;
    HoudiniApi::Keyframe_Create = &HoudiniApi::Keyframe_CreateEmptyStub;
    HoudiniApi::Keyframe_Init = &HoudiniApi::Keyframe_InitEmptyStub;
    HoudiniApi::LoadAssetLibraryFromFile = &HoudiniApi::LoadAssetLibraryFromFileEmptyStub;
    HoudiniApi::LoadAssetLibraryFromMemory = &HoudiniApi::LoadAssetLibraryFromMemoryEmptyStub;
    HoudiniApi::LoadGeoFromFile = &HoudiniApi::LoadGeoFromFileEmptyStub;
    HoudiniApi::LoadGeoFromMemory = &HoudiniApi::LoadGeoFromMemoryEmptyStub;
    HoudiniApi::LoadHIPFile = &HoudiniApi::LoadHIPFileEmptyStub;
    HoudiniApi::LoadNodeFromFile = &HoudiniApi::LoadNodeFromFileEmptyStub;
    HoudiniApi::MaterialInfo_Create = &HoudiniApi::MaterialInfo_CreateEmptyStub;
    HoudiniApi::MaterialInfo_Init = &HoudiniApi::MaterialInfo_InitEmptyStub;
    HoudiniApi::MergeHIPFile = &HoudiniApi::MergeHIPFileEmptyStub;
    HoudiniApi::NodeInfo_Create = &HoudiniApi::NodeInfo_CreateEmptyStub;
    HoudiniApi::NodeInfo_Init = &HoudiniApi::NodeInfo_InitEmptyStub;
    HoudiniApi::ObjectInfo_Create = &HoudiniApi::ObjectInfo_CreateEmptyStub;
    HoudiniApi::ObjectInfo_Init = &HoudiniApi::ObjectInfo_InitEmptyStub;
    HoudiniApi::ParmChoiceInfo_Create = &HoudiniApi::ParmChoiceInfo_CreateEmptyStub;
    HoudiniApi::ParmChoiceInfo_Init = &HoudiniApi::ParmChoiceInfo_InitEmptyStub;
    HoudiniApi::ParmHasExpression = &HoudiniApi::ParmHasExpressionEmptyStub;
    HoudiniApi::ParmHasTag = &HoudiniApi::ParmHasTagEmptyStub;
    HoudiniApi::ParmInfo_Create = &HoudiniApi::ParmInfo_CreateEmptyStub;
    HoudiniApi::ParmInfo_GetFloatValueCount = &HoudiniApi::ParmInfo_GetFloatValueCountEmptyStub;
    HoudiniApi::ParmInfo_GetIntValueCount = &HoudiniApi::ParmInfo_GetIntValueCountEmptyStub;
    HoudiniApi::ParmInfo_GetStringValueCount = &HoudiniApi::ParmInfo_GetStringValueCountEmptyStub;
    HoudiniApi::ParmInfo_Init = &HoudiniApi::ParmInfo_InitEmptyStub;
    HoudiniApi::ParmInfo_IsFloat = &HoudiniApi::ParmInfo_IsFloatEmptyStub;
    HoudiniApi::ParmInfo_IsInt = &HoudiniApi::ParmInfo_IsIntEmptyStub;
    HoudiniApi::ParmInfo_IsNode = &HoudiniApi::ParmInfo_IsNodeEmptyStub;
    HoudiniApi::ParmInfo_IsNonValue = &HoudiniApi::ParmInfo_IsNonValueEmptyStub;
    HoudiniApi::ParmInfo_IsPath = &HoudiniApi::ParmInfo_IsPathEmptyStub;
    HoudiniApi::ParmInfo_IsString = &HoudiniApi::ParmInfo_IsStringEmptyStub;
    HoudiniApi::PartInfo_Create = &HoudiniApi::PartInfo_CreateEmptyStub;
    HoudiniApi::PartInfo_GetAttributeCountByOwner = &HoudiniApi::PartInfo_GetAttributeCountByOwnerEmptyStub;
    HoudiniApi::PartInfo_GetElementCountByAttributeOwner = &HoudiniApi::PartInfo_GetElementCountByAttributeOwnerEmptyStub;
    HoudiniApi::PartInfo_GetElementCountByGroupType = &HoudiniApi::PartInfo_GetElementCountByGroupTypeEmptyStub;
    HoudiniApi::PartInfo_Init = &HoudiniApi::PartInfo_InitEmptyStub;
    HoudiniApi::PausePDGCook = &HoudiniApi::PausePDGCookEmptyStub;
    HoudiniApi::PythonThreadInterpreterLock = &HoudiniApi::PythonThreadInterpreterLockEmptyStub;
    HoudiniApi::QueryNodeInput = &HoudiniApi::QueryNodeInputEmptyStub;
    HoudiniApi::QueryNodeOutputConnectedCount = &HoudiniApi::QueryNodeOutputConnectedCountEmptyStub;
    HoudiniApi::QueryNodeOutputConnectedNodes = &HoudiniApi::QueryNodeOutputConnectedNodesEmptyStub;
    HoudiniApi::RemoveCustomString = &HoudiniApi::RemoveCustomStringEmptyStub;
    HoudiniApi::RemoveMultiparmInstance = &HoudiniApi::RemoveMultiparmInstanceEmptyStub;
    HoudiniApi::RemoveParmExpression = &HoudiniApi::RemoveParmExpressionEmptyStub;
    HoudiniApi::RenameNode = &HoudiniApi::RenameNodeEmptyStub;
    HoudiniApi::RenderCOPToImage = &HoudiniApi::RenderCOPToImageEmptyStub;
    HoudiniApi::RenderTextureToImage = &HoudiniApi::RenderTextureToImageEmptyStub;
    HoudiniApi::ResetSimulation = &HoudiniApi::ResetSimulationEmptyStub;
    HoudiniApi::RevertGeo = &HoudiniApi::RevertGeoEmptyStub;
    HoudiniApi::RevertParmToDefault = &HoudiniApi::RevertParmToDefaultEmptyStub;
    HoudiniApi::RevertParmToDefaults = &HoudiniApi::RevertParmToDefaultsEmptyStub;
    HoudiniApi::SaveGeoToFile = &HoudiniApi::SaveGeoToFileEmptyStub;
    HoudiniApi::SaveGeoToMemory = &HoudiniApi::SaveGeoToMemoryEmptyStub;
    HoudiniApi::SaveHIPFile = &HoudiniApi::SaveHIPFileEmptyStub;
    HoudiniApi::SaveNodeToFile = &HoudiniApi::SaveNodeToFileEmptyStub;
    HoudiniApi::SessionSyncInfo_Create = &HoudiniApi::SessionSyncInfo_CreateEmptyStub;
    HoudiniApi::SetAnimCurve = &HoudiniApi::SetAnimCurveEmptyStub;
    HoudiniApi::SetAttributeFloat64ArrayData = &HoudiniApi::SetAttributeFloat64ArrayDataEmptyStub;
    HoudiniApi::SetAttributeFloat64Data = &HoudiniApi::SetAttributeFloat64DataEmptyStub;
    HoudiniApi::SetAttributeFloatArrayData = &HoudiniApi::SetAttributeFloatArrayDataEmptyStub;
    HoudiniApi::SetAttributeFloatData = &HoudiniApi::SetAttributeFloatDataEmptyStub;
    HoudiniApi::SetAttributeInt16ArrayData = &HoudiniApi::SetAttributeInt16ArrayDataEmptyStub;
    HoudiniApi::SetAttributeInt16Data = &HoudiniApi::SetAttributeInt16DataEmptyStub;
    HoudiniApi::SetAttributeInt64ArrayData = &HoudiniApi::SetAttributeInt64ArrayDataEmptyStub;
    HoudiniApi::SetAttributeInt64Data = &HoudiniApi::SetAttributeInt64DataEmptyStub;
    HoudiniApi::SetAttributeInt8ArrayData = &HoudiniApi::SetAttributeInt8ArrayDataEmptyStub;
    HoudiniApi::SetAttributeInt8Data = &HoudiniApi::SetAttributeInt8DataEmptyStub;
    HoudiniApi::SetAttributeIntArrayData = &HoudiniApi::SetAttributeIntArrayDataEmptyStub;
    HoudiniApi::SetAttributeIntData = &HoudiniApi::SetAttributeIntDataEmptyStub;
    HoudiniApi::SetAttributeStringArrayData = &HoudiniApi::SetAttributeStringArrayDataEmptyStub;
    HoudiniApi::SetAttributeStringData = &HoudiniApi::SetAttributeStringDataEmptyStub;
    HoudiniApi::SetAttributeUInt8ArrayData = &HoudiniApi::SetAttributeUInt8ArrayDataEmptyStub;
    HoudiniApi::SetAttributeUInt8Data = &HoudiniApi::SetAttributeUInt8DataEmptyStub;
    HoudiniApi::SetCacheProperty = &HoudiniApi::SetCachePropertyEmptyStub;
    HoudiniApi::SetCompositorOptions = &HoudiniApi::SetCompositorOptionsEmptyStub;
    HoudiniApi::SetCurveCounts = &HoudiniApi::SetCurveCountsEmptyStub;
    HoudiniApi::SetCurveInfo = &HoudiniApi::SetCurveInfoEmptyStub;
    HoudiniApi::SetCurveKnots = &HoudiniApi::SetCurveKnotsEmptyStub;
    HoudiniApi::SetCurveOrders = &HoudiniApi::SetCurveOrdersEmptyStub;
    HoudiniApi::SetCustomString = &HoudiniApi::SetCustomStringEmptyStub;
    HoudiniApi::SetFaceCounts = &HoudiniApi::SetFaceCountsEmptyStub;
    HoudiniApi::SetGroupMembership = &HoudiniApi::SetGroupMembershipEmptyStub;
    HoudiniApi::SetHeightFieldData = &HoudiniApi::SetHeightFieldDataEmptyStub;
    HoudiniApi::SetImageInfo = &HoudiniApi::SetImageInfoEmptyStub;
    HoudiniApi::SetInputCurveInfo = &HoudiniApi::SetInputCurveInfoEmptyStub;
    HoudiniApi::SetInputCurvePositions = &HoudiniApi::SetInputCurvePositionsEmptyStub;
    HoudiniApi::SetInputCurvePositionsRotationsScales = &HoudiniApi::SetInputCurvePositionsRotationsScalesEmptyStub;
    HoudiniApi::SetNodeDisplay = &HoudiniApi::SetNodeDisplayEmptyStub;
    HoudiniApi::SetObjectTransform = &HoudiniApi::SetObjectTransformEmptyStub;
    HoudiniApi::SetParmExpression = &HoudiniApi::SetParmExpressionEmptyStub;
    HoudiniApi::SetParmFloatValue = &HoudiniApi::SetParmFloatValueEmptyStub;
    HoudiniApi::SetParmFloatValues = &HoudiniApi::SetParmFloatValuesEmptyStub;
    HoudiniApi::SetParmIntValue = &HoudiniApi::SetParmIntValueEmptyStub;
    HoudiniApi::SetParmIntValues = &HoudiniApi::SetParmIntValuesEmptyStub;
    HoudiniApi::SetParmNodeValue = &HoudiniApi::SetParmNodeValueEmptyStub;
    HoudiniApi::SetParmStringValue = &HoudiniApi::SetParmStringValueEmptyStub;
    HoudiniApi::SetPartInfo = &HoudiniApi::SetPartInfoEmptyStub;
    HoudiniApi::SetPreset = &HoudiniApi::SetPresetEmptyStub;
    HoudiniApi::SetServerEnvInt = &HoudiniApi::SetServerEnvIntEmptyStub;
    HoudiniApi::SetServerEnvString = &HoudiniApi::SetServerEnvStringEmptyStub;
    HoudiniApi::SetSessionSync = &HoudiniApi::SetSessionSyncEmptyStub;
    HoudiniApi::SetSessionSyncInfo = &HoudiniApi::SetSessionSyncInfoEmptyStub;
    HoudiniApi::SetTime = &HoudiniApi::SetTimeEmptyStub;
    HoudiniApi::SetTimelineOptions = &HoudiniApi::SetTimelineOptionsEmptyStub;
    HoudiniApi::SetTransformAnimCurve = &HoudiniApi::SetTransformAnimCurveEmptyStub;
    HoudiniApi::SetUseHoudiniTime = &HoudiniApi::SetUseHoudiniTimeEmptyStub;
    HoudiniApi::SetVertexList = &HoudiniApi::SetVertexListEmptyStub;
    HoudiniApi::SetViewport = &HoudiniApi::SetViewportEmptyStub;
    HoudiniApi::SetVolumeInfo = &HoudiniApi::SetVolumeInfoEmptyStub;
    HoudiniApi::SetVolumeTileFloatData = &HoudiniApi::SetVolumeTileFloatDataEmptyStub;
    HoudiniApi::SetVolumeTileIntData = &HoudiniApi::SetVolumeTileIntDataEmptyStub;
    HoudiniApi::SetVolumeVoxelFloatData = &HoudiniApi::SetVolumeVoxelFloatDataEmptyStub;
    HoudiniApi::SetVolumeVoxelIntData = &HoudiniApi::SetVolumeVoxelIntDataEmptyStub;
    HoudiniApi::SetWorkItemFloatAttribute = &HoudiniApi::SetWorkItemFloatAttributeEmptyStub;
    HoudiniApi::SetWorkItemIntAttribute = &HoudiniApi::SetWorkItemIntAttributeEmptyStub;
    HoudiniApi::SetWorkItemStringAttribute = &HoudiniApi::SetWorkItemStringAttributeEmptyStub;
    HoudiniApi::SetWorkitemFloatData = &HoudiniApi::SetWorkitemFloatDataEmptyStub;
    HoudiniApi::SetWorkitemIntData = &HoudiniApi::SetWorkitemIntDataEmptyStub;
    HoudiniApi::SetWorkitemStringData = &HoudiniApi::SetWorkitemStringDataEmptyStub;
    HoudiniApi::Shutdown = &HoudiniApi::ShutdownEmptyStub;
    HoudiniApi::StartThriftNamedPipeServer = &HoudiniApi::StartThriftNamedPipeServerEmptyStub;
    HoudiniApi::StartThriftSocketServer = &HoudiniApi::StartThriftSocketServerEmptyStub;
    HoudiniApi::ThriftServerOptions_Create = &HoudiniApi::ThriftServerOptions_CreateEmptyStub;
    HoudiniApi::ThriftServerOptions_Init = &HoudiniApi::ThriftServerOptions_InitEmptyStub;
    HoudiniApi::TimelineOptions_Create = &HoudiniApi::TimelineOptions_CreateEmptyStub;
    HoudiniApi::TimelineOptions_Init = &HoudiniApi::TimelineOptions_InitEmptyStub;
    HoudiniApi::TransformEuler_Create = &HoudiniApi::TransformEuler_CreateEmptyStub;
    HoudiniApi::TransformEuler_Init = &HoudiniApi::TransformEuler_InitEmptyStub;
    HoudiniApi::Transform_Create = &HoudiniApi::Transform_CreateEmptyStub;
    HoudiniApi::Transform_Init = &HoudiniApi::Transform_InitEmptyStub;
    HoudiniApi::Viewport_Create = &HoudiniApi::Viewport_CreateEmptyStub;
    HoudiniApi::VolumeInfo_Create = &HoudiniApi::VolumeInfo_CreateEmptyStub;
    HoudiniApi::VolumeInfo_Init = &HoudiniApi::VolumeInfo_InitEmptyStub;
    HoudiniApi::VolumeTileInfo_Create = &HoudiniApi::VolumeTileInfo_CreateEmptyStub;
    HoudiniApi::VolumeTileInfo_Init = &HoudiniApi::VolumeTileInfo_InitEmptyStub;
}

bool
HoudiniApi::IsHAPIInitialized()
{
    return ( HoudiniApi::IsInitialized != &HoudiniApi::IsInitializedEmptyStub );
}

HAPI_Result
HoudiniApi::AddAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::AddGroupEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, const char * group_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_AssetInfo
HoudiniApi::AssetInfo_CreateEmptyStub()
{
    return HAPI_AssetInfo();
}

void
HoudiniApi::AssetInfo_InitEmptyStub(HAPI_AssetInfo * in)
{
    return;
}

HAPI_AttributeInfo
HoudiniApi::AttributeInfo_CreateEmptyStub()
{
    return HAPI_AttributeInfo();
}

void
HoudiniApi::AttributeInfo_InitEmptyStub(HAPI_AttributeInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::BindCustomImplementationEmptyStub(HAPI_SessionType session_type, const char * dll_path)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CancelPDGCookEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CheckForSpecificErrorsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ErrorCodeBits errors_to_look_for, HAPI_ErrorCodeBits * errors_found)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CleanupEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ClearConnectionErrorEmptyStub()
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CloseSessionEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CommitGeoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CommitWorkItemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CommitWorkitemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ComposeChildNodeListEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, HAPI_NodeTypeBits node_type_filter, HAPI_NodeFlagsBits node_flags_filter, HAPI_Bool recursive, int * count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ComposeNodeCookResultEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_StatusVerbosity verbosity, int * buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ComposeObjectListEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, const char * categories, int * object_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_CompositorOptions
HoudiniApi::CompositorOptions_CreateEmptyStub()
{
    return HAPI_CompositorOptions();
}

void
HoudiniApi::CompositorOptions_InitEmptyStub(HAPI_CompositorOptions * in)
{
    return;
}

HAPI_Result
HoudiniApi::ConnectNodeInputEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int input_index, HAPI_NodeId node_id_to_connect, int output_index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ConvertMatrixToEulerEmptyStub(const HAPI_Session * session, const float * matrix, HAPI_RSTOrder rst_order, HAPI_XYZOrder rot_order, HAPI_TransformEuler * transform_out)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ConvertMatrixToQuatEmptyStub(const HAPI_Session * session, const float * matrix, HAPI_RSTOrder rst_order, HAPI_Transform * transform_out)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ConvertTransformEmptyStub(const HAPI_Session * session, const HAPI_TransformEuler * transform_in, HAPI_RSTOrder rst_order, HAPI_XYZOrder rot_order, HAPI_TransformEuler * transform_out)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ConvertTransformEulerToMatrixEmptyStub(const HAPI_Session * session, const HAPI_TransformEuler * transform, float * matrix)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ConvertTransformQuatToMatrixEmptyStub(const HAPI_Session * session, const HAPI_Transform * transform, float * matrix)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CookNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const HAPI_CookOptions * cook_options)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Bool
HoudiniApi::CookOptions_AreEqualEmptyStub(const HAPI_CookOptions * left, const HAPI_CookOptions * right)
{
    return HAPI_Bool();
}

HAPI_CookOptions
HoudiniApi::CookOptions_CreateEmptyStub()
{
    return HAPI_CookOptions();
}

void
HoudiniApi::CookOptions_InitEmptyStub(HAPI_CookOptions * in)
{
    return;
}

HAPI_Result
HoudiniApi::CookPDGEmptyStub(const HAPI_Session * session, HAPI_NodeId cook_node_id, int generate_only, int blocking)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CookPDGAllOutputsEmptyStub(const HAPI_Session* session, HAPI_NodeId cook_node_id, int generate_only, int blocking)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateCustomSessionEmptyStub(HAPI_SessionType session_type, void * session_info, HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateHeightFieldInputEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, const char * name, int xsize, int ysize, float voxelsize, HAPI_HeightFieldSampling sampling, HAPI_NodeId * heightfield_node_id, HAPI_NodeId * height_node_id, HAPI_NodeId * mask_node_id, HAPI_NodeId * merge_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateHeightfieldInputVolumeNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, HAPI_NodeId * new_node_id, const char * name, int xsize, int ysize, float voxelsize)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateInProcessSessionEmptyStub(HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateInputCurveNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId * node_id, const char * name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateInputNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId * node_id, const char * name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, const char * operator_name, const char * node_label, HAPI_Bool cook_on_creation, HAPI_NodeId * new_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateThriftNamedPipeSessionEmptyStub(HAPI_Session * session, const char * pipe_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateThriftSocketSessionEmptyStub(HAPI_Session * session, const char * host_name, int port)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateWorkItemEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId * work_item_id, const char * name, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::CreateWorkitemEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId * workitem_id, const char * name, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_CurveInfo
HoudiniApi::CurveInfo_CreateEmptyStub()
{
    return HAPI_CurveInfo();
}

void
HoudiniApi::CurveInfo_InitEmptyStub(HAPI_CurveInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::DeleteAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::DeleteGroupEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, const char * group_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::DeleteNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::DirtyPDGNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_Bool clean_results)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::DisconnectNodeInputEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int input_index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::DisconnectNodeOutputsAtEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int output_index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ExtractImageToFileEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, const char * image_file_format_name, const char * image_planes, const char * destination_folder_path, const char * destination_file_name, int * destination_file_path)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ExtractImageToMemoryEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, const char * image_file_format_name, const char * image_planes, int * buffer_size)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_GeoInfo
HoudiniApi::GeoInfo_CreateEmptyStub()
{
    return HAPI_GeoInfo();
}

int
HoudiniApi::GeoInfo_GetGroupCountByTypeEmptyStub(HAPI_GeoInfo * in, HAPI_GroupType type)
{
    return -1;
}

void
HoudiniApi::GeoInfo_InitEmptyStub(HAPI_GeoInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::GetActiveCacheCountEmptyStub(const HAPI_Session * session, int * active_cache_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetActiveCacheNamesEmptyStub(const HAPI_Session * session, HAPI_StringHandle * cache_names_array, int active_cache_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAssetDefinitionParmCountsEmptyStub(const HAPI_Session * session, HAPI_AssetLibraryId library_id, const char * asset_name, int * parm_count, int * int_value_count, int * float_value_count, int * string_value_count, int * choice_value_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAssetDefinitionParmInfosEmptyStub(const HAPI_Session * session, HAPI_AssetLibraryId library_id, const char * asset_name, HAPI_ParmInfo * parm_infos_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAssetDefinitionParmValuesEmptyStub(const HAPI_Session * session, HAPI_AssetLibraryId library_id, const char * asset_name, int * int_values_array, int int_start, int int_length, float * float_values_array, int float_start, int float_length, HAPI_Bool string_evaluate, HAPI_StringHandle * string_values_array, int string_start, int string_length, HAPI_ParmChoiceInfo * choice_values_array, int choice_start, int choice_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAssetInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_AssetInfo * asset_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeFloat64ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, double * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeFloat64DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, double * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeFloatArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, float * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, float * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeOwner owner, HAPI_AttributeInfo * attr_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt16ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_Int16 * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt16DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, HAPI_Int16 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt64ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_Int64 * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt64DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, HAPI_Int64 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt8ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_Int8 * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeInt8DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, HAPI_Int8 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeIntArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, int * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeNamesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_AttributeOwner owner, HAPI_StringHandle * attribute_names_array, int count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeStringArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_StringHandle * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeStringDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_StringHandle * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeUInt8ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, HAPI_UInt8 * data_fixed_array, int data_fixed_length, int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAttributeUInt8DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, HAPI_AttributeInfo * attr_info, int stride, HAPI_UInt8 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAvailableAssetCountEmptyStub(const HAPI_Session * session, HAPI_AssetLibraryId library_id, int * asset_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetAvailableAssetsEmptyStub(const HAPI_Session * session, HAPI_AssetLibraryId library_id, HAPI_StringHandle * asset_names_array, int asset_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetBoxInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId geo_node_id, HAPI_PartId part_id, HAPI_BoxInfo * box_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCachePropertyEmptyStub(const HAPI_Session * session, const char * cache_name, HAPI_CacheProperty cache_property, int * property_value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetComposedChildNodeListEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, HAPI_NodeId * child_node_ids_array, int count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetComposedNodeCookResultEmptyStub(const HAPI_Session * session, char * string_value, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetComposedObjectListEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, HAPI_ObjectInfo * object_infos_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetComposedObjectTransformsEmptyStub(const HAPI_Session * session, HAPI_NodeId parent_node_id, HAPI_RSTOrder rst_order, HAPI_Transform * transform_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCompositorOptionsEmptyStub(const HAPI_Session * session, HAPI_CompositorOptions * compositor_options)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetConnectionErrorEmptyStub(char * string_value, int length, HAPI_Bool clear)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetConnectionErrorLengthEmptyStub(int * buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCookingCurrentCountEmptyStub(const HAPI_Session * session, int * count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCookingTotalCountEmptyStub(const HAPI_Session * session, int * count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCurveCountsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int * counts_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCurveInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_CurveInfo * info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCurveKnotsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, float * knots_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetCurveOrdersEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int * orders_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetDisplayGeoInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId object_node_id, HAPI_GeoInfo * geo_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetEdgeCountOfEdgeGroupEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * group_name, int * edge_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetEnvIntEmptyStub(HAPI_EnvIntType int_type, int * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetFaceCountsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int * face_counts_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetFirstVolumeTileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_VolumeTileInfo * tile)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGeoInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_GeoInfo * geo_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGeoSizeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * format, int * size)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGroupCountOnPackedInstancePartEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int * pointGroupCount, int * primitiveGroupCount)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGroupMembershipEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, const char * group_name, HAPI_Bool * membership_array_all_equal, int * membership_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGroupMembershipOnPackedInstancePartEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, const char * group_name, HAPI_Bool * membership_array_all_equal, int * membership_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGroupNamesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_GroupType group_type, HAPI_StringHandle * group_names_array, int group_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetGroupNamesOnPackedInstancePartEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, HAPI_StringHandle * group_names_array, int group_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetHIPFileNodeCountEmptyStub(const HAPI_Session *session, HAPI_HIPFileId id, int * count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetHIPFileNodeIdsEmptyStub(const HAPI_Session *session, HAPI_HIPFileId id, HAPI_NodeId * node_ids, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetHandleBindingInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int handle_index, HAPI_HandleBindingInfo * handle_binding_infos_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetHandleInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_HandleInfo * handle_infos_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetHeightFieldDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, float * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetImageFilePathEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, const char * image_file_format_name, const char * image_planes, const char * destination_folder_path, const char * destination_file_name, HAPI_ParmId texture_parm_id, int * destination_file_path)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetImageInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, HAPI_ImageInfo * image_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetImageMemoryBufferEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, char * buffer, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetImagePlaneCountEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, int * image_plane_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetImagePlanesEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, HAPI_StringHandle * image_planes_array, int image_plane_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetInputCurveInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_InputCurveInfo * info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetInstanceTransformsOnPartEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_RSTOrder rst_order, HAPI_Transform * transforms_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetInstancedObjectIdsEmptyStub(const HAPI_Session * session, HAPI_NodeId object_node_id, HAPI_NodeId * instanced_node_id_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetInstancedPartIdsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_PartId * instanced_parts_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetInstancerPartTransformsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_RSTOrder rst_order, HAPI_Transform * transforms_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetManagerNodeIdEmptyStub(const HAPI_Session * session, HAPI_NodeType node_type, HAPI_NodeId * node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetMaterialInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, HAPI_MaterialInfo * material_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetMaterialNodeIdsOnFacesEmptyStub(const HAPI_Session * session, HAPI_NodeId geometry_node_id, HAPI_PartId part_id, HAPI_Bool * are_all_the_same, HAPI_NodeId * material_ids_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNextVolumeTileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_VolumeTileInfo * tile)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNodeFromPathEmptyStub(const HAPI_Session * session, const HAPI_NodeId parent_node_id, const char * path, HAPI_NodeId * node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNodeInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_NodeInfo * node_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNodeInputNameEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int input_idx, HAPI_StringHandle * name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNodeOutputNameEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int output_idx, HAPI_StringHandle * name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNodePathEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_NodeId relative_to_node_id, HAPI_StringHandle * path)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNumWorkItemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int * num)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetNumWorkitemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int * num)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetObjectInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ObjectInfo * object_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetObjectTransformEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_NodeId relative_to_node_id, HAPI_RSTOrder rst_order, HAPI_Transform * transform)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetOutputGeoCountEmptyStub(const HAPI_Session* session, HAPI_NodeId node_id, int* count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetOutputGeoInfosEmptyStub(const HAPI_Session* session, HAPI_NodeId node_id, HAPI_GeoInfo* geo_infos_array, int count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetOutputNodeIdEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int output, HAPI_NodeId * output_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPDGEventsEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id, HAPI_PDG_EventInfo * event_array, int length, int * event_count, int * remaining_events)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPDGGraphContextIdEmptyStub(const HAPI_Session * session, HAPI_NodeId top_node_id, HAPI_PDG_GraphContextId * context_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPDGGraphContextsEmptyStub(const HAPI_Session * session, HAPI_StringHandle * context_names_array, HAPI_PDG_GraphContextId * context_id_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPDGGraphContextsCountEmptyStub(const HAPI_Session* session, int* num_contexts)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPDGStateEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id, int * pdg_state)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParametersEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmInfo * parm_infos_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmChoiceListsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmChoiceInfo * parm_choices_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmExpressionEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, HAPI_StringHandle * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmFileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, const char * destination_directory, const char * destination_file_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmFloatValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, float * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmFloatValuesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, float * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmIdFromNameEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, HAPI_ParmId * parm_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, HAPI_ParmInfo * parm_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmInfoFromNameEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, HAPI_ParmInfo * parm_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmIntValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, int * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmIntValuesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmNodeValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, HAPI_NodeId * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmStringValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, HAPI_Bool evaluate, HAPI_StringHandle * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmStringValuesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_Bool evaluate, HAPI_StringHandle * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmTagNameEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, int tag_index, HAPI_StringHandle * tag_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmTagValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, const char * tag_name, HAPI_StringHandle * tag_value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetParmWithTagEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * tag_name, HAPI_ParmId * parm_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPartInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_PartInfo * part_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPresetEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, char * buffer, int buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetPresetBufLengthEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PresetType preset_type, const char * preset_name, int * buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetServerEnvIntEmptyStub(const HAPI_Session * session, const char * variable_name, int * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetServerEnvStringEmptyStub(const HAPI_Session * session, const char * variable_name, HAPI_StringHandle * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetServerEnvVarCountEmptyStub(const HAPI_Session * session, int * env_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetServerEnvVarListEmptyStub(const HAPI_Session * session, HAPI_StringHandle * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetSessionEnvIntEmptyStub(const HAPI_Session * session, HAPI_SessionEnvIntType int_type, int * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetSessionSyncInfoEmptyStub(const HAPI_Session * session, HAPI_SessionSyncInfo * session_sync_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetSphereInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId geo_node_id, HAPI_PartId part_id, HAPI_SphereInfo * sphere_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStatusEmptyStub(const HAPI_Session * session, HAPI_StatusType status_type, int * status)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStatusStringEmptyStub(const HAPI_Session * session, HAPI_StatusType status_type, char * string_value, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStatusStringBufLengthEmptyStub(const HAPI_Session * session, HAPI_StatusType status_type, HAPI_StatusVerbosity verbosity, int * buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStringEmptyStub(const HAPI_Session * session, HAPI_StringHandle string_handle, char * string_value, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStringBatchEmptyStub(const HAPI_Session * session, char * char_buffer, int char_array_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStringBatchSizeEmptyStub(const HAPI_Session * session, const int * string_handle_array, int string_handle_count, int * string_buffer_size)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetStringBufLengthEmptyStub(const HAPI_Session * session, HAPI_StringHandle string_handle, int * buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetSupportedImageFileFormatCountEmptyStub(const HAPI_Session * session, int * file_format_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetSupportedImageFileFormatsEmptyStub(const HAPI_Session * session, HAPI_ImageFileFormat * formats_array, int file_format_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetTimeEmptyStub(const HAPI_Session * session, float * time)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetTimelineOptionsEmptyStub(const HAPI_Session * session, HAPI_TimelineOptions * timeline_options)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetTotalCookCountEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_NodeTypeBits node_type_filter, HAPI_NodeFlagsBits node_flags_filter, HAPI_Bool recursive, int * count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetUseHoudiniTimeEmptyStub(const HAPI_Session * session, HAPI_Bool * enabled)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVertexListEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int * vertex_list_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetViewportEmptyStub(const HAPI_Session * session, HAPI_Viewport * viewport)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeBoundsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, float * x_min, float * y_min, float * z_min, float * x_max, float * y_max, float * z_max, float * x_center, float * y_center, float * z_center)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_VolumeInfo * volume_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeTileFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, float fill_value, const HAPI_VolumeTileInfo * tile, float * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeTileIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int fill_value, const HAPI_VolumeTileInfo * tile, int * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeVisualInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_VolumeVisualInfo * visual_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeVoxelFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int x_index, int y_index, int z_index, float * values_array, int value_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetVolumeVoxelIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int x_index, int y_index, int z_index, int * values_array, int value_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemAttributeSizeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, int * length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemFloatAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, float * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemInfoEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id, HAPI_PDG_WorkItemId work_item_id, HAPI_PDG_WorkItemInfo * work_item_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemIntAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, int * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemOutputFilesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, HAPI_PDG_WorkItemOutputFile * resultinfo_array, int resultinfo_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemStringAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, HAPI_StringHandle * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkItemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int * work_item_ids_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemDataLengthEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, int * length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, float * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemInfoEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id, HAPI_PDG_WorkItemId workitem_id, HAPI_PDG_WorkItemInfo * workitem_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, int * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemResultInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, HAPI_PDG_WorkItemOutputFile * resultinfo_array, int resultinfo_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemStringDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, HAPI_StringHandle * data_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::GetWorkitemsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int * workitem_ids_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_HandleBindingInfo
HoudiniApi::HandleBindingInfo_CreateEmptyStub()
{
    return HAPI_HandleBindingInfo();
}

void
HoudiniApi::HandleBindingInfo_InitEmptyStub(HAPI_HandleBindingInfo * in)
{
    return;
}

HAPI_HandleInfo
HoudiniApi::HandleInfo_CreateEmptyStub()
{
    return HAPI_HandleInfo();
}

void
HoudiniApi::HandleInfo_InitEmptyStub(HAPI_HandleInfo * in)
{
    return;
}

HAPI_ImageFileFormat
HoudiniApi::ImageFileFormat_CreateEmptyStub()
{
    return HAPI_ImageFileFormat();
}

void
HoudiniApi::ImageFileFormat_InitEmptyStub(HAPI_ImageFileFormat *in)
{
    return;
}

HAPI_ImageInfo
HoudiniApi::ImageInfo_CreateEmptyStub()
{
    return HAPI_ImageInfo();
}

void
HoudiniApi::ImageInfo_InitEmptyStub(HAPI_ImageInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::InitializeEmptyStub(const HAPI_Session * session, const HAPI_CookOptions * cook_options, HAPI_Bool use_cooking_thread, int cooking_thread_stack_size, const char * houdini_environment_files, const char * otl_search_path, const char * dso_search_path, const char * image_dso_search_path, const char * audio_dso_search_path)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_InputCurveInfo
HoudiniApi::InputCurveInfo_CreateEmptyStub()
{
    return HAPI_InputCurveInfo();
}

void
HoudiniApi::InputCurveInfo_InitEmptyStub(HAPI_InputCurveInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::InsertMultiparmInstanceEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, int instance_position)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::InterruptEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::IsInitializedEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::IsNodeValidEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int unique_node_id, HAPI_Bool * answer)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::IsSessionValidEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Keyframe
HoudiniApi::Keyframe_CreateEmptyStub()
{
    return HAPI_Keyframe();
}

void
HoudiniApi::Keyframe_InitEmptyStub(HAPI_Keyframe * in)
{
    return;
}

HAPI_Result
HoudiniApi::LoadAssetLibraryFromFileEmptyStub(const HAPI_Session * session, const char * file_path, HAPI_Bool allow_overwrite, HAPI_AssetLibraryId * library_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::LoadAssetLibraryFromMemoryEmptyStub(const HAPI_Session * session, const char * library_buffer, int library_buffer_length, HAPI_Bool allow_overwrite, HAPI_AssetLibraryId * library_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::LoadGeoFromFileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * file_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::LoadGeoFromMemoryEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * format, const char * buffer, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::LoadHIPFileEmptyStub(const HAPI_Session * session, const char * file_name, HAPI_Bool cook_on_load)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::LoadNodeFromFileEmptyStub(const HAPI_Session * session, const char * file_name, HAPI_NodeId parent_node_id, const char * node_label, HAPI_Bool cook_on_load, HAPI_NodeId * new_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_MaterialInfo
HoudiniApi::MaterialInfo_CreateEmptyStub()
{
    return HAPI_MaterialInfo();
}

void
HoudiniApi::MaterialInfo_InitEmptyStub(HAPI_MaterialInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::MergeHIPFileEmptyStub(const HAPI_Session * session, const char * file_name, HAPI_Bool cook_on_load, HAPI_HIPFileId * file_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_NodeInfo
HoudiniApi::NodeInfo_CreateEmptyStub()
{
    return HAPI_NodeInfo();
}

void
HoudiniApi::NodeInfo_InitEmptyStub(HAPI_NodeInfo * in)
{
    return;
}

HAPI_ObjectInfo
HoudiniApi::ObjectInfo_CreateEmptyStub()
{
    return HAPI_ObjectInfo();
}

void
HoudiniApi::ObjectInfo_InitEmptyStub(HAPI_ObjectInfo * in)
{
    return;
}

HAPI_ParmChoiceInfo
HoudiniApi::ParmChoiceInfo_CreateEmptyStub()
{
    return HAPI_ParmChoiceInfo();
}

void
HoudiniApi::ParmChoiceInfo_InitEmptyStub(HAPI_ParmChoiceInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::ParmHasExpressionEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, HAPI_Bool * has_expression)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ParmHasTagEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, const char * tag_name, HAPI_Bool * has_tag)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_ParmInfo
HoudiniApi::ParmInfo_CreateEmptyStub()
{
    return HAPI_ParmInfo();
}

int
HoudiniApi::ParmInfo_GetFloatValueCountEmptyStub(const HAPI_ParmInfo * in)
{
    return -1;
}

int
HoudiniApi::ParmInfo_GetIntValueCountEmptyStub(const HAPI_ParmInfo * in)
{
    return -1;
}

int
HoudiniApi::ParmInfo_GetStringValueCountEmptyStub(const HAPI_ParmInfo* in)
{
    return -1;
}

void
HoudiniApi::ParmInfo_InitEmptyStub(HAPI_ParmInfo * in)
{
    return;
}

HAPI_Bool
HoudiniApi::ParmInfo_IsFloatEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_Bool
HoudiniApi::ParmInfo_IsIntEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_Bool
HoudiniApi::ParmInfo_IsNodeEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_Bool
HoudiniApi::ParmInfo_IsNonValueEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_Bool
HoudiniApi::ParmInfo_IsPathEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_Bool
HoudiniApi::ParmInfo_IsStringEmptyStub(const HAPI_ParmInfo * in)
{
    return HAPI_Bool();
}

HAPI_PartInfo
HoudiniApi::PartInfo_CreateEmptyStub()
{
    return HAPI_PartInfo();
}

int
HoudiniApi::PartInfo_GetAttributeCountByOwnerEmptyStub(HAPI_PartInfo * in, HAPI_AttributeOwner owner)
{
    return -1;
}

int
HoudiniApi::PartInfo_GetElementCountByAttributeOwnerEmptyStub(HAPI_PartInfo * in, HAPI_AttributeOwner owner)
{
    return -1;
}

int
HoudiniApi::PartInfo_GetElementCountByGroupTypeEmptyStub(HAPI_PartInfo * in, HAPI_GroupType type)
{
    return -1;
}

void
HoudiniApi::PartInfo_InitEmptyStub(HAPI_PartInfo * in)
{
    return;
}

HAPI_Result
HoudiniApi::PausePDGCookEmptyStub(const HAPI_Session * session, HAPI_PDG_GraphContextId graph_context_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::PythonThreadInterpreterLockEmptyStub(const HAPI_Session * session, HAPI_Bool locked)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::QueryNodeInputEmptyStub(const HAPI_Session * session, HAPI_NodeId node_to_query, int input_index, HAPI_NodeId * connected_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::QueryNodeOutputConnectedCountEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int output_idx, HAPI_Bool into_subnets, HAPI_Bool through_dots, int * connected_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::QueryNodeOutputConnectedNodesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int output_idx, HAPI_Bool into_subnets, HAPI_Bool through_dots, HAPI_NodeId * connected_node_ids_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RemoveCustomStringEmptyStub(const HAPI_Session * session, const HAPI_StringHandle string_handle)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RemoveMultiparmInstanceEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, int instance_position)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RemoveParmExpressionEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RenameNodeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * new_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RenderCOPToImageEmptyStub(const HAPI_Session * session, HAPI_NodeId cop_node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RenderTextureToImageEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, HAPI_ParmId parm_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ResetSimulationEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RevertGeoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RevertParmToDefaultEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::RevertParmToDefaultsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SaveGeoToFileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * file_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SaveGeoToMemoryEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, char * buffer, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SaveHIPFileEmptyStub(const HAPI_Session * session, const char * file_path, HAPI_Bool lock_nodes)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SaveNodeToFileEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * file_name)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_SessionSyncInfo
HoudiniApi::SessionSyncInfo_CreateEmptyStub()
{
    return HAPI_SessionSyncInfo();
}

HAPI_Result
HoudiniApi::SetAnimCurveEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_ParmId parm_id, int parm_index, const HAPI_Keyframe * curve_keyframes_array, int keyframe_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeFloat64ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const double * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeFloat64DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const double * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeFloatArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const float * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const float * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt16ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int16 * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt16DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int16 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt64ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int64 * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt64DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int64 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt8ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int8 * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeInt8DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_Int8 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeIntArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const int * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const int * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeStringArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const char ** data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeStringDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const char ** data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeUInt8ArrayDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_UInt8 * data_fixed_array, int data_fixed_length, const int * sizes_fixed_array, int start, int sizes_fixed_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetAttributeUInt8DataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const HAPI_AttributeInfo * attr_info, const HAPI_UInt8 * data_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCachePropertyEmptyStub(const HAPI_Session * session, const char * cache_name, HAPI_CacheProperty cache_property, int property_value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCompositorOptionsEmptyStub(const HAPI_Session * session, const HAPI_CompositorOptions * compositor_options)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCurveCountsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const int * counts_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCurveInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_CurveInfo * info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCurveKnotsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const float * knots_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCurveOrdersEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const int * orders_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetCustomStringEmptyStub(const HAPI_Session * session, const char * string_value, HAPI_StringHandle * handle_value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetFaceCountsEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const int * face_counts_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetGroupMembershipEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, HAPI_GroupType group_type, const char * group_name, const int * membership_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetHeightFieldDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const char * name, const float * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetImageInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId material_node_id, const HAPI_ImageInfo * image_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetInputCurveInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_InputCurveInfo * info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetInputCurvePositionsEmptyStub(const HAPI_Session* session, HAPI_NodeId node_id, HAPI_PartId part_id, const float* positions_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetInputCurvePositionsRotationsScalesEmptyStub(const HAPI_Session* session, HAPI_NodeId node_id, HAPI_PartId part_id, const float* positions_array, int positions_start, int positions_length, const float* rotations_array, int rotations_start, int rotations_length, const float * scales_array, int scales_start, int scales_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetNodeDisplayEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, int onOff)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetObjectTransformEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const HAPI_TransformEuler * trans)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmExpressionEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * value, HAPI_ParmId parm_id, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmFloatValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, float value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmFloatValuesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const float * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmIntValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, int index, int value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmIntValuesEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const int * values_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmNodeValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * parm_name, HAPI_NodeId value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetParmStringValueEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, const char * value, HAPI_ParmId parm_id, int index)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetPartInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_PartInfo * part_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetPresetEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PresetType preset_type, const char * preset_name, const char * buffer, int buffer_length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetServerEnvIntEmptyStub(const HAPI_Session * session, const char * variable_name, int value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetServerEnvStringEmptyStub(const HAPI_Session * session, const char * variable_name, const char * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetSessionSyncEmptyStub(const HAPI_Session * session, HAPI_Bool enable)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetSessionSyncInfoEmptyStub(const HAPI_Session * session, const HAPI_SessionSyncInfo * session_sync_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetTimeEmptyStub(const HAPI_Session * session, float time)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetTimelineOptionsEmptyStub(const HAPI_Session * session, const HAPI_TimelineOptions * timeline_options)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetTransformAnimCurveEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_TransformComponent trans_comp, const HAPI_Keyframe * curve_keyframes_array, int keyframe_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetUseHoudiniTimeEmptyStub(const HAPI_Session * session, HAPI_Bool enabled)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVertexListEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const int * vertex_list_array, int start, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetViewportEmptyStub(const HAPI_Session * session, const HAPI_Viewport * viewport)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVolumeInfoEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_VolumeInfo * volume_info)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVolumeTileFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_VolumeTileInfo * tile, const float * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVolumeTileIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, const HAPI_VolumeTileInfo * tile, const int * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVolumeVoxelFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int x_index, int y_index, int z_index, const float * values_array, int value_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetVolumeVoxelIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PartId part_id, int x_index, int y_index, int z_index, const int * values_array, int value_count)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkItemFloatAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, const float * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkItemIntAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, const int * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkItemStringAttributeEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId work_item_id, const char * attribute_name, int data_index, const char * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkitemFloatDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, const float * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkitemIntDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, const int * values_array, int length)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::SetWorkitemStringDataEmptyStub(const HAPI_Session * session, HAPI_NodeId node_id, HAPI_PDG_WorkItemId workitem_id, const char * data_name, int data_index, const char * value)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::ShutdownEmptyStub(const HAPI_Session * session)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::StartThriftNamedPipeServerEmptyStub(const HAPI_ThriftServerOptions * options, const char * pipe_name, HAPI_ProcessId * process_id, const char * log_file)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_Result
HoudiniApi::StartThriftSocketServerEmptyStub(const HAPI_ThriftServerOptions * options, int port, HAPI_ProcessId * process_id, const char * log_file)
{
    return HAPI_RESULT_FAILURE;
}

HAPI_ThriftServerOptions
HoudiniApi::ThriftServerOptions_CreateEmptyStub()
{
    return HAPI_ThriftServerOptions();
}

void
HoudiniApi::ThriftServerOptions_InitEmptyStub(HAPI_ThriftServerOptions * in)
{
    return;
}

HAPI_TimelineOptions
HoudiniApi::TimelineOptions_CreateEmptyStub()
{
    return HAPI_TimelineOptions();
}

void
HoudiniApi::TimelineOptions_InitEmptyStub(HAPI_TimelineOptions * in)
{
    return;
}

HAPI_TransformEuler
HoudiniApi::TransformEuler_CreateEmptyStub()
{
    return HAPI_TransformEuler();
}

void
HoudiniApi::TransformEuler_InitEmptyStub(HAPI_TransformEuler * in)
{
    return;
}

HAPI_Transform
HoudiniApi::Transform_CreateEmptyStub()
{
    return HAPI_Transform();
}

void
HoudiniApi::Transform_InitEmptyStub(HAPI_Transform * in)
{
    return;
}

HAPI_Viewport
HoudiniApi::Viewport_CreateEmptyStub()
{
    return HAPI_Viewport();
}

HAPI_VolumeInfo
HoudiniApi::VolumeInfo_CreateEmptyStub()
{
    return HAPI_VolumeInfo();
}

void
HoudiniApi::VolumeInfo_InitEmptyStub(HAPI_VolumeInfo * in)
{
    return;
}

HAPI_VolumeTileInfo
HoudiniApi::VolumeTileInfo_CreateEmptyStub()
{
    return HAPI_VolumeTileInfo();
}

void
HoudiniApi::VolumeTileInfo_InitEmptyStub(HAPI_VolumeTileInfo * in)
{
    return;
}


