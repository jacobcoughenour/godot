#include "capture_server.h"
#include "window_info.h"

CaptureServer *CaptureServer::singleton = nullptr;

void CaptureServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_windows"),
			&CaptureServer::get_windows);
	ClassDB::bind_method(D_METHOD("create_feed_for_window"), &CaptureServer::create_feed_for_window);
}

CaptureServer *CaptureServer::get_singleton() { return singleton; }

CaptureServer::CaptureServer() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	// load in the direct3d dll
	d3d11Module = LoadLibrary("d3d11.dll");
	if (d3d11Module == NULL) {
		ERR_PRINT("Failed to load d3d11.dll");
		return;
	} else {
		print_line("Loaded d3d11.dll");
	}

	// initialize the d3d11 device
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d11Module, "D3D11CreateDevice");
	if (D3D11CreateDevice == NULL) {
		ERR_PRINT("Failed to get D3D11CreateDevice");
		return;
	}
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1,
			D3D11_SDK_VERSION, &d3dDevice, nullptr, nullptr);

	HRESULT hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	if (FAILED(hr)) {
		ERR_PRINT("Failed to get DXGI interface");
		return;
	} else {
		print_line("Got DXGI interface");
	}

	CreateDirect3D11DeviceFromDXGIDeviceFunc CreateDirect3D11DeviceFromDXGIDevice = (CreateDirect3D11DeviceFromDXGIDeviceFunc)GetProcAddress(d3d11Module, "CreateDirect3D11DeviceFromDXGIDevice");

	winrt::com_ptr<::IInspectable> inspectable;
	HRESULT hr2 = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice, inspectable.put());
	direct3DDevice = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

	if (FAILED(hr2)) {
		ERR_PRINT("Failed to create Direct3D device");
		return;
	} else {
		print_line("Created Direct3D device");
	}
}

CaptureServer::~CaptureServer() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
	dxgiDevice = nullptr;
	d3dDevice = nullptr;
	direct3DDevice = nullptr;
	FreeLibrary(d3d11Module);
}

int CaptureServer::create_feed_for_window(Ref<WindowInfo> const &p_window) {
	// get the capture item for the window handle
	auto windowHandle = p_window->get_handle();

	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };

	interop_factory->CreateForWindow(windowHandle, winrt::guid_of<winrt::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void **>(winrt::put_abi(item)));

	auto size = item.Size();

	auto framePool = winrt::Windows::Graphics::Capture::
			Direct3D11CaptureFramePool::Create(
					direct3DDevice,
					winrt::Windows::Graphics::DirectX::
							DirectXPixelFormat::B8G8R8A8UIntNormalized,
					2,
					size);

	print_line("Created frame pool");
	print_line("Frame pool size: " + itos(size.Width) + "x" + itos(size.Height));

	auto session = framePool.CreateCaptureSession(item);

	return -1;
}

bool IsAltTabWindow(Ref<WindowInfo> const &window) {
	HWND hwnd = window->get_handle();
	HWND shellWindow = GetShellWindow();

	if (hwnd == shellWindow)
		return false;

	if (window->get_title().length() == 0)
		return false;

	if (!IsWindowVisible(hwnd))
		return false;

	if (GetAncestor(hwnd, GA_ROOT) != hwnd)
		return false;

	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	if (!((style & WS_DISABLED) != WS_DISABLED))
		return false;

	DWORD cloaked = FALSE;
	HRESULT hrTemp = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
	if (SUCCEEDED(hrTemp) && cloaked == DWM_CLOAKED_SHELL)
		return false;

	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	Vector<Ref<WindowInfo>> *windows = reinterpret_cast<Vector<Ref<WindowInfo>> *>(lParam);

	char class_name[80];
	char title[80];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));

	Ref<WindowInfo> window = WindowInfo::create(hwnd, String(title), String(class_name));

	if (!IsAltTabWindow(window)) {
		// skip windows that do not appear in the alt tab list.
		return true;
	}

	windows->push_back(window);

	return true;
}

TypedArray<WindowInfo> CaptureServer::get_windows() {
	Vector<Ref<WindowInfo>> windows;
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

	TypedArray<WindowInfo> windows_array;
	windows_array.resize(windows.size());
	for (int i = 0; i < windows.size(); i++) {
		windows_array[i] = windows[i];
	}

	return windows_array;
}

Ref<CaptureFeed> CaptureServer::get_feed_by_id(int feed_id) {
	return Ref<CaptureFeed>();
}
