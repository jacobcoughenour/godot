#include "capture_feed.h"

#include "servers/rendering_server.h"

void CaptureFeed::_bind_methods() {
}

Ref<CaptureFeed> CaptureFeed::create(HWND p_handle, Type p_type) {
	Ref<CaptureFeed> feed;
	feed.instantiate();
	feed->handle = p_handle;
	feed->type = p_type;
	return feed;
}

HWND CaptureFeed::get_handle() const {
	return handle;
}

CaptureFeed::Type CaptureFeed::get_type() const {
	return type;
}

RID CaptureFeed::get_texture() const {
	return texture;
}

CaptureFeed::CaptureFeed() {
	// initialize our feed
	handle = nullptr;
	type = WINDOW;
	texture = RenderingServer::get_singleton()->texture_2d_placeholder_create();
}

CaptureFeed::~CaptureFeed() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer::get_singleton()->free(texture);
}
