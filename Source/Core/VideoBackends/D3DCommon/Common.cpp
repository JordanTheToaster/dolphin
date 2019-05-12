// Copyright 2019 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#include "Common/Assert.h"
#include "Common/DynamicLibrary.h"
#include "Common/MsgHandler.h"
#include "Common/StringUtil.h"
#include "VideoBackends/D3DCommon/Common.h"
#include "VideoCommon/TextureConfig.h"
#include "VideoCommon/VideoConfig.h"

namespace D3DCommon
{
pD3DCompile d3d_compile;

static Common::DynamicLibrary s_dxgi_library;
static Common::DynamicLibrary s_d3dcompiler_library;
static bool s_libraries_loaded = false;

static HRESULT (*create_dxgi_factory)(REFIID riid, _COM_Outptr_ void** ppFactory);
static HRESULT (*create_dxgi_factory2)(UINT Flags, REFIID riid, void** ppFactory);

bool LoadLibraries()
{
  if (s_libraries_loaded)
    return true;

  if (!s_dxgi_library.Open("dxgi.dll"))
  {
    PanicAlertT("Failed to load dxgi.dll");
    return false;
  }

  if (!s_d3dcompiler_library.Open(D3DCOMPILER_DLL_A))
  {
    PanicAlertT("Failed to load %s. If you are using Windows 7, try installing the "
                "KB4019990 update package.",
                D3DCOMPILER_DLL_A);
    s_dxgi_library.Close();
    return false;
  }

  // Required symbols.
  if (!s_d3dcompiler_library.GetSymbol("D3DCompile", &d3d_compile) ||
      !s_dxgi_library.GetSymbol("CreateDXGIFactory", &create_dxgi_factory))
  {
    PanicAlertT("Failed to find one or more D3D symbols");
    s_d3dcompiler_library.Close();
    s_dxgi_library.Close();
    return false;
  }

  // Optional symbols.
  s_dxgi_library.GetSymbol("CreateDXGIFactory2", &create_dxgi_factory2);
  s_libraries_loaded = true;
  return true;
}

void UnloadLibraries()
{
  create_dxgi_factory = nullptr;
  create_dxgi_factory2 = nullptr;
  d3d_compile = nullptr;
  s_d3dcompiler_library.Close();
  s_dxgi_library.Close();
  s_libraries_loaded = false;
}

IDXGIFactory* CreateDXGIFactory(bool debug_device)
{
  IDXGIFactory* factory;

  // Use Win8.1 version if available.
  if (create_dxgi_factory2 &&
      SUCCEEDED(create_dxgi_factory2(debug_device ? DXGI_CREATE_FACTORY_DEBUG : 0,
                                     IID_PPV_ARGS(&factory))))
  {
    return factory;
  }

  // Fallback to original version, without debug support.
  HRESULT hr = create_dxgi_factory(IID_PPV_ARGS(&factory));
  if (FAILED(hr))
  {
    PanicAlert("CreateDXGIFactory() failed with HRESULT %08X", hr);
    return nullptr;
  }

  return factory;
}

std::vector<std::string> GetAdapterNames()
{
  Microsoft::WRL::ComPtr<IDXGIFactory> factory;
  HRESULT hr = create_dxgi_factory(IID_PPV_ARGS(&factory));
  if (!SUCCEEDED(hr))
    return {};

  std::vector<std::string> adapters;
  IDXGIAdapter* adapter;
  while (factory->EnumAdapters(static_cast<UINT>(adapters.size()), &adapter) !=
         DXGI_ERROR_NOT_FOUND)
  {
    std::string name;
    DXGI_ADAPTER_DESC desc;
    if (SUCCEEDED(adapter->GetDesc(&desc)))
      name = UTF16ToUTF8(desc.Description);

    adapters.push_back(std::move(name));
  }

  return adapters;
}

DXGI_FORMAT GetDXGIFormatForAbstractFormat(AbstractTextureFormat format, bool typeless)
{
  switch (format)
  {
  case AbstractTextureFormat::DXT1:
    return DXGI_FORMAT_BC1_UNORM;
  case AbstractTextureFormat::DXT3:
    return DXGI_FORMAT_BC2_UNORM;
  case AbstractTextureFormat::DXT5:
    return DXGI_FORMAT_BC3_UNORM;
  case AbstractTextureFormat::BPTC:
    return DXGI_FORMAT_BC7_UNORM;
  case AbstractTextureFormat::RGBA8:
    return typeless ? DXGI_FORMAT_R8G8B8A8_TYPELESS : DXGI_FORMAT_R8G8B8A8_UNORM;
  case AbstractTextureFormat::BGRA8:
    return typeless ? DXGI_FORMAT_B8G8R8A8_TYPELESS : DXGI_FORMAT_B8G8R8A8_UNORM;
  case AbstractTextureFormat::R16:
    return typeless ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_R16_UNORM;
  case AbstractTextureFormat::R32F:
    return typeless ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R32_FLOAT;
  case AbstractTextureFormat::D16:
    return DXGI_FORMAT_R16_TYPELESS;
  case AbstractTextureFormat::D24_S8:
    return DXGI_FORMAT_R24G8_TYPELESS;
  case AbstractTextureFormat::D32F:
    return DXGI_FORMAT_R32_TYPELESS;
  case AbstractTextureFormat::D32F_S8:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  default:
    PanicAlert("Unhandled texture format.");
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  }
}
DXGI_FORMAT GetSRVFormatForAbstractFormat(AbstractTextureFormat format)
{
  switch (format)
  {
  case AbstractTextureFormat::DXT1:
    return DXGI_FORMAT_BC1_UNORM;
  case AbstractTextureFormat::DXT3:
    return DXGI_FORMAT_BC2_UNORM;
  case AbstractTextureFormat::DXT5:
    return DXGI_FORMAT_BC3_UNORM;
  case AbstractTextureFormat::BPTC:
    return DXGI_FORMAT_BC7_UNORM;
  case AbstractTextureFormat::RGBA8:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case AbstractTextureFormat::BGRA8:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case AbstractTextureFormat::R16:
    return DXGI_FORMAT_R16_UNORM;
  case AbstractTextureFormat::R32F:
    return DXGI_FORMAT_R32_FLOAT;
  case AbstractTextureFormat::D16:
    return DXGI_FORMAT_R16_UNORM;
  case AbstractTextureFormat::D24_S8:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  case AbstractTextureFormat::D32F:
    return DXGI_FORMAT_R32_FLOAT;
  case AbstractTextureFormat::D32F_S8:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  default:
    PanicAlert("Unhandled SRV format");
    return DXGI_FORMAT_UNKNOWN;
  }
}

DXGI_FORMAT GetRTVFormatForAbstractFormat(AbstractTextureFormat format, bool integer)
{
  switch (format)
  {
  case AbstractTextureFormat::RGBA8:
    return integer ? DXGI_FORMAT_R8G8B8A8_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
  case AbstractTextureFormat::BGRA8:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case AbstractTextureFormat::R16:
    return integer ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R16_UNORM;
  case AbstractTextureFormat::R32F:
    return DXGI_FORMAT_R32_FLOAT;
  default:
    PanicAlert("Unhandled RTV format");
    return DXGI_FORMAT_UNKNOWN;
  }
}
DXGI_FORMAT GetDSVFormatForAbstractFormat(AbstractTextureFormat format)
{
  switch (format)
  {
  case AbstractTextureFormat::D16:
    return DXGI_FORMAT_D16_UNORM;
  case AbstractTextureFormat::D24_S8:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case AbstractTextureFormat::D32F:
    return DXGI_FORMAT_D32_FLOAT;
  case AbstractTextureFormat::D32F_S8:
    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  default:
    PanicAlert("Unhandled DSV format");
    return DXGI_FORMAT_UNKNOWN;
  }
}

AbstractTextureFormat GetAbstractFormatForDXGIFormat(DXGI_FORMAT format)
{
  switch (format)
  {
  case DXGI_FORMAT_R8G8B8A8_UINT:
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    return AbstractTextureFormat::RGBA8;

  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    return AbstractTextureFormat::BGRA8;

  case DXGI_FORMAT_R16_UINT:
  case DXGI_FORMAT_R16_UNORM:
  case DXGI_FORMAT_R16_TYPELESS:
    return AbstractTextureFormat::R16;

  case DXGI_FORMAT_R32_FLOAT:
  case DXGI_FORMAT_R32_TYPELESS:
    return AbstractTextureFormat::R32F;

  case DXGI_FORMAT_D16_UNORM:
    return AbstractTextureFormat::D16;

  case DXGI_FORMAT_D24_UNORM_S8_UINT:
    return AbstractTextureFormat::D24_S8;

  case DXGI_FORMAT_D32_FLOAT:
    return AbstractTextureFormat::D32F;

  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    return AbstractTextureFormat::D32F_S8;

  case DXGI_FORMAT_BC1_UNORM:
    return AbstractTextureFormat::DXT1;
  case DXGI_FORMAT_BC2_UNORM:
    return AbstractTextureFormat::DXT3;
  case DXGI_FORMAT_BC3_UNORM:
    return AbstractTextureFormat::DXT5;
  case DXGI_FORMAT_BC7_UNORM:
    return AbstractTextureFormat::BPTC;

  default:
    return AbstractTextureFormat::Undefined;
  }
}

void SetDebugObjectName(IUnknown* resource, const char* format, ...)
{
  if (!g_ActiveConfig.bEnableValidationLayer)
    return;

  std::va_list ap;
  va_start(ap, format);
  std::string name = StringFromFormatV(format, ap);
  va_end(ap);

  Microsoft::WRL::ComPtr<ID3D11DeviceChild> child11;
  Microsoft::WRL::ComPtr<ID3D12DeviceChild> child12;
  if (SUCCEEDED(resource->QueryInterface(IID_PPV_ARGS(&child11))))
  {
    child11->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.length()),
                            name.c_str());
  }
  else if (SUCCEEDED(resource->QueryInterface(IID_PPV_ARGS(&child12))))
  {
    child12->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.length()),
                            name.c_str());
  }
}

std::string GetDebugObjectName(IUnknown* resource)
{
  if (!g_ActiveConfig.bEnableValidationLayer)
    return {};

  std::string name;
  UINT size = 0;

  Microsoft::WRL::ComPtr<ID3D11DeviceChild> child11;
  Microsoft::WRL::ComPtr<ID3D12DeviceChild> child12;
  if (SUCCEEDED(resource->QueryInterface(IID_PPV_ARGS(&child11))))
  {
    child11->GetPrivateData(WKPDID_D3DDebugObjectName, &size, nullptr);
    name.resize(size);
    child11->GetPrivateData(WKPDID_D3DDebugObjectName, &size, name.data());
  }
  else if (SUCCEEDED(resource->QueryInterface(IID_PPV_ARGS(&child12))))
  {
    child12->GetPrivateData(WKPDID_D3DDebugObjectName, &size, nullptr);
    name.resize(size);
    child12->GetPrivateData(WKPDID_D3DDebugObjectName, &size, name.data());
  }

  return name;
}
}  // namespace D3DCommon
