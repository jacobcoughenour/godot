#ifndef CAPTURE_FEED_H
#define CAPTURE_FEED_H

#include "core/object/ref_counted.h"

#include <windows.h>

class CaptureFeed : public RefCounted {
	GDCLASS(CaptureFeed, RefCounted);

private:
	enum Type {
		WINDOW = 0,
		MONITOR = 1,
	};

	HWND handle;
	Type type;
	RID texture;

protected:
	static void _bind_methods();

public:
	static Ref<CaptureFeed> create(HWND p_handle, Type p_type);

	HWND get_handle() const;
	Type get_type() const;
	RID get_texture() const;

	CaptureFeed();
	~CaptureFeed();
};

#endif // CAPTURE_FEED_H
