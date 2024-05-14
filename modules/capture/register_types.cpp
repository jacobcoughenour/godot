#include "register_types.h"

#include "capture_feed.h"
#include "capture_server.h"
#include "capture_texture.h"
#include "window_info.h"

static CaptureServer *capture_server = nullptr;

void initialize_capture_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<CaptureFeed>();
	ClassDB::register_class<CaptureTexture>();
	ClassDB::register_class<WindowInfo>();
	ClassDB::register_class<CaptureServer>();

	capture_server = memnew(CaptureServer);
	Engine::get_singleton()->add_singleton(Engine::Singleton("CaptureServer", capture_server));
}

void uninitialize_capture_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	memdelete(capture_server);
}
