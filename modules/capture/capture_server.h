#ifndef CAPTURE_SERVER_H
#define CAPTURE_SERVER_H

#include "capture_feed.h"
#include "core/object/object.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#include "window_info.h"

// #include "F:\Windows Kits\10\Include\10.0.19041.0\um\d3d11.h"
#include <dwmapi.h>
// #include "F:\Windows Kits\10\Include\10.0.19041.0\shared\dxgi.h"
#include <d3d11.h>
#include <dxgi.h>

#include "winrt_capture.h"

class CaptureServer : public Object {
	GDCLASS(CaptureServer, Object);

	static CaptureServer *singleton;

protected:
	Vector<Ref<CaptureFeed>> feeds;

	static void _bind_methods();

	HMODULE d3d11Module = nullptr;
	ID3D11Device *d3dDevice = nullptr;
	IDXGIDevice *dxgiDevice = nullptr;
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice direct3DDevice = nullptr;

public:
	static CaptureServer *get_singleton();

	CaptureServer();
	~CaptureServer();

	int create_feed_for_window(Ref<WindowInfo> const &p_window);

	TypedArray<WindowInfo> get_windows();

	Ref<CaptureFeed> CaptureServer::get_feed_by_id(int feed_id);
};

#endif // CAPTURE_SERVER_H
