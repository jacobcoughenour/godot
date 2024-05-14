#include "window_info.h"

void WindowInfo::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("get_handle"), &WindowInfo::get_handle);
	ClassDB::bind_method(D_METHOD("get_title"), &WindowInfo::get_title);
	ClassDB::bind_method(D_METHOD("get_class_name"), &WindowInfo::get_class_name);
}

Ref<WindowInfo> WindowInfo::create(HWND p_handle, String const &p_title, String const &p_class_name) {
	Ref<WindowInfo> window;
	window.instantiate();
	window->handle = p_handle;
	window->title = p_title;
	window->class_name = p_class_name;
	return window;
}

Ref<CaptureTexture> WindowInfo::create_texture() {
	// CaptureServer *capture_server = CaptureServer::get_singleton();
	// if (capture_server == nullptr) {
	// 	return Ref<CaptureTexture>();
	// }

	// int feed_id = capture_server->create_feed_for_window(this);
	// if (feed_id == -1) {
	// 	return Ref<CaptureTexture>();
	// }

	// Ref<CaptureFeed> feed = capture_server->get_feed_by_id(feed_id);
	// if (feed == nullptr) {
	// 	return Ref<CaptureTexture>();
	// }

	// Ref<CaptureTexture> texture = memnew(CaptureTexture);
	// texture->set_capture_feed_id(feed_id);

	return Ref<CaptureTexture>();

	// return texture;
}
