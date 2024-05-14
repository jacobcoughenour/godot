#ifndef WINRT_CAPTURE_H
#define WINRT_CAPTURE_H

#include <Unknwn.h>
#include <inspectable.h>
#define WIN32_LEAN_AND_MEAN
// #include <windows.graphics.capture.interop.h>
// #include <windows.graphics.directx.direct3d11.interop.h>
// #include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <winrt/windows.graphics.capture.h>
#include <winrt/windows.graphics.directx.direct3d11.h>

#undef INTERFACE
#define INTERFACE IGraphicsCaptureItemInterop
DECLARE_INTERFACE_IID_(IGraphicsCaptureItemInterop, IUnknown, "3628E81B-3CAC-4C60-B7F4-23CE0E0C3356") {
	IFACEMETHOD(CreateForWindow)
	(
			HWND window,
			REFIID riid,
			_COM_Outptr_ void **result) PURE;

	IFACEMETHOD(CreateForMonitor)
	(
			HMONITOR monitor,
			REFIID riid,
			_COM_Outptr_ void **result) PURE;
};

typedef HRESULT(WINAPI *CreateDirect3D11DeviceFromDXGIDeviceFunc)(IDXGIDevice *, IInspectable **);

extern "C" {
HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice *dxgiDevice,
		::IInspectable **graphicsDevice);

HRESULT __stdcall CreateDirect3D11SurfaceFromDXGISurface(::IDXGISurface *dgxiSurface,
		::IInspectable **graphicsSurface);
}

struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
		IDirect3DDxgiInterfaceAccess : ::IUnknown {
	virtual HRESULT __stdcall GetInterface(GUID const &id, void **object) = 0;
};

inline auto CreateDirect3DDevice(IDXGIDevice *dxgi_device) {
	winrt::com_ptr<::IInspectable> d3d_device;
	winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
	return d3d_device.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

inline auto CreateDirect3DSurface(IDXGISurface *dxgi_surface) {
	winrt::com_ptr<::IInspectable> d3d_surface;
	winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(dxgi_surface, d3d_surface.put()));
	return d3d_surface.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface>();
}

template <typename T>
winrt::com_ptr<T> GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const &object) {
	auto access = object.as<IDirect3DDxgiInterfaceAccess>();
	winrt::com_ptr<T> result;
	winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
	return result;
}

#endif // WINRT_CAPTURE_H
