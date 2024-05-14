#ifndef WINDOW_INFO_H
#define WINDOW_INFO_H

#include "capture_feed.h"
#include "capture_texture.h"
#include "core/object/ref_counted.h"
#include <windows.h>

class WindowInfo : public RefCounted {
	GDCLASS(WindowInfo, RefCounted);

private:
	HWND handle;
	String title;
	String class_name;

protected:
	static void _bind_methods();

public:
	static Ref<WindowInfo> create(HWND p_handle, String const &p_title, String const &p_className);

	Ref<CaptureTexture> create_texture();

	HWND get_handle() const { return handle; }
	String get_title() const { return title; }
	String get_class_name() const { return class_name; }
};

#endif // WINDOW_INFO_H
