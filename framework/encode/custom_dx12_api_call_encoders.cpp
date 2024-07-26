/*
** Copyright (c) 2021 LunarG, Inc.
** Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include "generated/generated_dx12_api_call_encoders.h"
#include "encode/custom_dx12_api_call_encoders.h"

#include "encode/custom_dx12_struct_encoders.h"
#include "encode/d3d12_capture_manager.h"
#include "encode/parameter_encoder.h"
#include "encode/struct_pointer_encoder.h"
#include "format/api_call_id.h"
#include "util/defines.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(encode)

static uint32_t GetSubresourceDataArraySize(const D3D11_TEXTURE1D_DESC* desc)
{
    GFXRECON_ASSERT(desc != nullptr);
    return desc->MipLevels * desc->ArraySize;
}

static uint32_t GetSubresourceDataArraySize(const D3D11_TEXTURE2D_DESC* desc)
{
    GFXRECON_ASSERT(desc != nullptr);
    return desc->MipLevels * desc->ArraySize;
}

static uint32_t GetSubresourceDataArraySize(const D3D11_TEXTURE3D_DESC* desc)
{
    GFXRECON_ASSERT(desc != nullptr);
    return desc->MipLevels;
}

static void EncodeD3D11SubresourceDataPtr(ParameterEncoder*             encoder,
                                          const D3D11_BUFFER_DESC*      desc,
                                          const D3D11_SUBRESOURCE_DATA* value)
{
    GFXRECON_ASSERT(desc != nullptr);

    encoder->EncodeStructPtrPreamble(value);
    if (value != nullptr)
    {
        auto initial_data_size = 0;
        if (desc != nullptr)
        {
            initial_data_size = desc->ByteWidth;
        }

        encoder->EncodeVoidArray(value->pSysMem, initial_data_size);
        encoder->EncodeUInt32Value(value->SysMemPitch);
        encoder->EncodeUInt32Value(value->SysMemSlicePitch);
    }
}

template <typename ResourceDesc>
static void
EncodeD3D11SubresourceDataPtr(ParameterEncoder* encoder, const ResourceDesc* desc, const D3D11_SUBRESOURCE_DATA* value)
{
    auto array_size = GetSubresourceDataArraySize(desc);
    encoder->EncodeStructArrayPreamble(value, array_size);
    if (value != nullptr)
    {
        for (auto i = 0u; i < array_size; ++i)
        {
            // TODO: Size calculation utilities
            auto initial_data_size = 0;
            encoder->EncodeVoidArray(pInitialData->pSysMem, initial_data_size);
            encoder->EncodeUInt32Value(pInitialData->SysMemPitch);
            encoder->EncodeUInt32Value(pInitialData->SysMemSlicePitch);
        }
    }
}

void Encode_ID3D12Device_CheckFeatureSupport(ID3D12Device_Wrapper* wrapper,
                                             HRESULT               result,
                                             D3D12_FEATURE         Feature,
                                             void*                 pFeatureSupportData,
                                             UINT                  FeatureSupportDataSize)
{
    auto encoder = D3D12CaptureManager::Get()->BeginMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D12Device_CheckFeatureSupport, wrapper->GetCaptureId());
    if (encoder)
    {
        encoder->EncodeEnumValue(Feature);
        EncodeD3D12FeatureStruct(encoder, pFeatureSupportData, Feature);
        encoder->EncodeUInt32Value(FeatureSupportDataSize);
        encoder->EncodeInt32Value(result);
        D3D12CaptureManager::Get()->EndMethodCallCapture();
    }
}

void Encode_IDXGIFactory5_CheckFeatureSupport(IDXGIFactory5_Wrapper* wrapper,
                                              HRESULT                result,
                                              DXGI_FEATURE           Feature,
                                              void*                  pFeatureSupportData,
                                              UINT                   FeatureSupportDataSize)
{
    auto encoder = D3D12CaptureManager::Get()->BeginMethodCallCapture(
        format::ApiCallId::ApiCall_IDXGIFactory5_CheckFeatureSupport, wrapper->GetCaptureId());
    if (encoder)
    {
        encoder->EncodeEnumValue(Feature);
        EncodeDXGIFeatureStruct(encoder, pFeatureSupportData, Feature);
        encoder->EncodeUInt32Value(FeatureSupportDataSize);
        encoder->EncodeInt32Value(result);
        D3D12CaptureManager::Get()->EndMethodCallCapture();
    }
}

void Encode_ID3D12Resource_WriteToSubresource(ID3D12Resource_Wrapper* wrapper,
                                              HRESULT                 return_value,
                                              UINT                    DstSubresource,
                                              const D3D12_BOX*        pDstBox,
                                              const void*             pSrcData,
                                              UINT                    SrcRowPitch,
                                              UINT                    SrcDepthPitch)
{
    auto encoder = D3D12CaptureManager::Get()->BeginMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D12Resource_WriteToSubresource, wrapper->GetCaptureId());
    if (encoder)
    {
        bool omit_output_data = false;
        if (return_value != S_OK)
        {
            omit_output_data = true;
        }
        encoder->EncodeUInt32Value(DstSubresource);
        EncodeStructPtr(encoder, pDstBox);
        encoder->EncodeVoidArray(
            pSrcData,
            static_cast<size_t>(gfxrecon::graphics::dx12::GetSubresourceWriteDataSize(
                wrapper->GetWrappedObjectAs<ID3D12Resource>(), DstSubresource, pDstBox, SrcRowPitch, SrcDepthPitch)));
        encoder->EncodeUInt32Value(SrcRowPitch);
        encoder->EncodeUInt32Value(SrcDepthPitch);
        encoder->EncodeInt32Value(return_value);
        D3D12CaptureManager::Get()->EndMethodCallCapture();
    }
}

void Encode_ID3D11DeviceContext_UpdateSubresource(ID3D11DeviceContext_Wrapper* wrapper,
                                                  ID3D11Resource*              pDstResource,
                                                  UINT                         DstSubresource,
                                                  const D3D11_BOX*             pDstBox,
                                                  const void*                  pSrcData,
                                                  UINT                         SrcRowPitch,
                                                  UINT                         SrcDepthPitch)
{
    auto encoder = D3D12CaptureManager::Get()->BeginMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11DeviceContext_UpdateSubresource, wrapper->GetCaptureId());
    if (encoder)
    {
        encoder->EncodeObjectValue(pDstResource);
        encoder->EncodeUInt32Value(DstSubresource);
        EncodeStructPtr(encoder, pDstBox);
        // TODO: Compute data size.
        encoder->EncodeVoidPtr(pSrcData);
        encoder->EncodeUInt32Value(SrcRowPitch);
        encoder->EncodeUInt32Value(SrcDepthPitch);
        D3D12CaptureManager::Get()->EndMethodCallCapture();
    }
}

void Encode_ID3D11Device_CreateBuffer(ID3D11Device_Wrapper*         wrapper,
                                      HRESULT                       return_value,
                                      const D3D11_BUFFER_DESC*      pDesc,
                                      const D3D11_SUBRESOURCE_DATA* pInitialData,
                                      ID3D11Buffer**                ppBuffer)
{
    auto encoder = D3D12CaptureManager::Get()->BeginTrackedMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11Device_CreateBuffer, wrapper->GetCaptureId());
    if (encoder)
    {
        bool omit_output_data = false;
        if (return_value != S_OK)
        {
            omit_output_data = true;
        }
        EncodeStructPtr(encoder, pDesc);
        EncodeD3D11SubresourceDataPtr(encoder, pDesc, pInitialData);
        encoder->EncodeObjectPtr(ppBuffer, omit_output_data);
        encoder->EncodeInt32Value(return_value);
        D3D12CaptureManager::Get()->EndCreateMethodCallCapture(
            return_value, IID_ID3D11Buffer, reinterpret_cast<void**>(ppBuffer), wrapper);
    }
}

void Encode_ID3D11Device_CreateTexture1D(ID3D11Device_Wrapper*         wrapper,
                                         HRESULT                       return_value,
                                         const D3D11_TEXTURE1D_DESC*   pDesc,
                                         const D3D11_SUBRESOURCE_DATA* pInitialData,
                                         ID3D11Texture1D**             ppTexture1D)
{
    auto encoder = D3D12CaptureManager::Get()->BeginTrackedMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11Device_CreateTexture1D, wrapper->GetCaptureId());
    if (encoder)
    {
        bool omit_output_data = false;
        if (return_value != S_OK)
        {
            omit_output_data = true;
        }
        EncodeStructPtr(encoder, pDesc);
        EncodeStructPtr(encoder, pDesc, pInitialData);
        encoder->EncodeObjectPtr(ppTexture1D, omit_output_data);
        encoder->EncodeInt32Value(return_value);
        D3D12CaptureManager::Get()->EndCreateMethodCallCapture(
            return_value, IID_ID3D11Texture1D, reinterpret_cast<void**>(ppTexture1D), wrapper);
    }
}

void Encode_ID3D11Device_CreateTexture2D(ID3D11Device_Wrapper*         wrapper,
                                         HRESULT                       return_value,
                                         const D3D11_TEXTURE2D_DESC*   pDesc,
                                         const D3D11_SUBRESOURCE_DATA* pInitialData,
                                         ID3D11Texture2D**             ppTexture2D)
{
    auto encoder = D3D12CaptureManager::Get()->BeginTrackedMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11Device_CreateTexture2D, wrapper->GetCaptureId());
    if (encoder)
    {
        bool omit_output_data = false;
        if (return_value != S_OK)
        {
            omit_output_data = true;
        }
        EncodeStructPtr(encoder, pDesc);
        EncodeStructPtr(encoder, pDesc, pInitialData);
        encoder->EncodeObjectPtr(ppTexture2D, omit_output_data);
        encoder->EncodeInt32Value(return_value);
        D3D12CaptureManager::Get()->EndCreateMethodCallCapture(
            return_value, IID_ID3D11Texture2D, reinterpret_cast<void**>(ppTexture2D), wrapper);
    }
}

void Encode_ID3D11Device_CreateTexture3D(ID3D11Device_Wrapper*         wrapper,
                                         HRESULT                       return_value,
                                         const D3D11_TEXTURE3D_DESC*   pDesc,
                                         const D3D11_SUBRESOURCE_DATA* pInitialData,
                                         ID3D11Texture3D**             ppTexture3D)
{
    auto encoder = D3D12CaptureManager::Get()->BeginTrackedMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11Device_CreateTexture3D, wrapper->GetCaptureId());
    if (encoder)
    {
        bool omit_output_data = false;
        if (return_value != S_OK)
        {
            omit_output_data = true;
        }
        EncodeStructPtr(encoder, pDesc);
        EncodeStructPtr(encoder, pDesc, pInitialData);
        encoder->EncodeObjectPtr(ppTexture3D, omit_output_data);
        encoder->EncodeInt32Value(return_value);
        D3D12CaptureManager::Get()->EndCreateMethodCallCapture(
            return_value, IID_ID3D11Texture3D, reinterpret_cast<void**>(ppTexture3D), wrapper);
    }
}

void Encode_ID3D11DeviceContext1_UpdateSubresource1(ID3D11DeviceContext1_Wrapper* wrapper,
                                                    ID3D11Resource*               pDstResource,
                                                    UINT                          DstSubresource,
                                                    const D3D11_BOX*              pDstBox,
                                                    const void*                   pSrcData,
                                                    UINT                          SrcRowPitch,
                                                    UINT                          SrcDepthPitch,
                                                    UINT                          CopyFlags)
{
    auto encoder = D3D12CaptureManager::Get()->BeginMethodCallCapture(
        format::ApiCallId::ApiCall_ID3D11DeviceContext1_UpdateSubresource1, wrapper->GetCaptureId());
    if (encoder)
    {
        encoder->EncodeObjectValue(pDstResource);
        encoder->EncodeUInt32Value(DstSubresource);
        EncodeStructPtr(encoder, pDstBox);
        // TODO: Compute data size.
        encoder->EncodeVoidPtr(pSrcData);
        encoder->EncodeUInt32Value(SrcRowPitch);
        encoder->EncodeUInt32Value(SrcDepthPitch);
        encoder->EncodeUInt32Value(CopyFlags);
        D3D12CaptureManager::Get()->EndMethodCallCapture();
    }
}

GFXRECON_END_NAMESPACE(encode)
GFXRECON_END_NAMESPACE(gfxrecon)
