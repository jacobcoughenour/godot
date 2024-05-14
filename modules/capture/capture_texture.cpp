#include "capture_texture.h"
// #include "capture_server.h"

void CaptureTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_capture_feed_id", "feed_id"), &CaptureTexture::set_capture_feed_id);
	ClassDB::bind_method(D_METHOD("get_capture_feed_id"), &CaptureTexture::get_capture_feed_id);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "capture_feed_id"), "set_capture_feed_id", "get_capture_feed_id");
}

int CaptureTexture::get_width() const {
	return 0;
}

int CaptureTexture::get_height() const {
	return 0;
}

Ref<Image> CaptureTexture::get_image() const {
	return Ref<Image>();
}

RID CaptureTexture::get_rid() const {
	// Ref<CaptureFeed> feed = CaptureServer::get_singleton()->get_feed_by_id(_capture_feed_id);
	// if (feed.is_valid()) {
	// 	return feed->get_texture();
	// } else {
	if (_texture.is_null()) {
		_texture = RenderingServer::get_singleton()->texture_2d_placeholder_create();
	}
	return _texture;
	// }
}

void CaptureTexture::set_capture_feed_id(int p_new_id) {
	_capture_feed_id = p_new_id;
	notify_property_list_changed();
}

int CaptureTexture::get_capture_feed_id() const {
	return _capture_feed_id;
}

CaptureTexture::CaptureTexture() {}

CaptureTexture::~CaptureTexture() {
	if (_texture.is_valid()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RenderingServer::get_singleton()->free(_texture);
	}
}
