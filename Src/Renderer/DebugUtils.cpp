﻿#include "Common/Precompile.h"

#ifdef _DEBUG

#include "Renderer/DebugUtils.h"

#include "Common/DirectxError.h"

namespace d14engine::renderer::debug_utils
{
    void enableD3d12DebugLayer()
    {
        ComPtr<ID3D12Debug> debugController;
        THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }

    void suppressWarnings(ID3D12Device* device)
    {
        ComPtr<ID3D12InfoQueue> infoQueue;
        THROW_IF_FAILED(device->QueryInterface<ID3D12InfoQueue>(&infoQueue));

        D3D12_MESSAGE_ID hide[] =
        {
            // After introducing the preceding D2D1 drawing layer, some D2D1
            // effects need to be processed before setting the render target.

            D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_DEPTHSTENCILVIEW_NOT_SET,

            // We allow UIKit::Panel to resize the associated rect to zero,
            // in which case the final scissor rect will be drawn as empty.

            D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,

            // It seems that ID2D1DeviceContext::FillGeometry are implemented
            // to draw different colors by clearing current render target
            // with different values.
            // 
            // The mismatching of the actual value used to clear the render
            // target and the preset value configed in the related resource
            // will lead to slow operation.

            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

            // DirectX 3D complains that "D3D12 resource is not sharable with
            // D3D11 layer" when trying to create a wrapped resource with the
            // D3D11On12 mechanism.
            // 
            // However, the function returns S_OK and everything works well,
            // so we guess this might be a bug of the D3D12 interop library.
            // By the way, we also find the similar problems in some forums.

            D3D12_MESSAGE_ID_REFLECTSHAREDPROPERTIES_INVALIDOBJECT,

            // This warning is produced when recreating a D2D1 resource that
            // was bound to any render target in the previous render passes,
            // even if all references have been cleared.
            // 
            // We guess that the clearing of the descriptor is delayed after
            // the associated resource destroyed.

            D3D12_MESSAGE_ID_DESCRIPTOR_HANDLE_WITH_INVALID_RESOURCE,

            // Creating D2D1 device from D3D11On12 device will trigger this warning,
            // and it says "Ignoring InitialState D3D12_RESOURCE_STATE_GENERIC_READ.
            // Buffers are effectively created in state D3D12_RESOURCE_STATE_COMMON"

            D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED
        };

        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;

        THROW_IF_FAILED(infoQueue->AddStorageFilterEntries(&filter));
    }

    ComPtr<IDXGIDebug> queryDxgiDebugInterface()
    {
        HMODULE hDll; // Dxgidebug.dll

        // CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG) may have already loaded it.
        hDll = GetModuleHandle(L"Dxgidebug.dll");
        if (hDll == nullptr)
        {
            // Load DLL manually otherwise (DXGI does not provide LIB for Dxgidebug).
            hDll = LoadLibrary(L"Dxgidebug.dll");
        }
        THROW_IF_NULL(hDll);

        using FuncPtr = HRESULT(*)(REFIID, void**);
        auto DXGIGetDebugInterface = (FuncPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");
        THROW_IF_NULL(DXGIGetDebugInterface);

        ComPtr<IDXGIDebug> dxgiDebugController;
        THROW_IF_FAILED(DXGIGetDebugInterface(IID_PPV_ARGS(&dxgiDebugController)));

        // Because dxgiDebugController may only be accessed in the DEBUG environment,
        // there is no need to free "Dxgidebug.dll" during the application's lifecycle.
        return dxgiDebugController;
    }
}

#endif
