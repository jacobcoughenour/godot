#ifndef CAPTURE_TEXTURE_H
#define CAPTURE_TEXTURE_H

#include "scene/resources/texture.h"

class CaptureTexture : public Texture2D {
	GDCLASS(CaptureTexture, Texture2D);

private:
	mutable RID _texture;
	int _capture_feed_id = 0;

protected:
	static void _bind_methods();

public:
	int get_width() const override;
	int get_height() const override;
	virtual bool has_alpha() const override { return false; }

	virtual Ref<Image> get_image() const;

	RID get_rid() const override;

	void set_capture_feed_id(int p_new_id);
	int get_capture_feed_id() const;

	CaptureTexture();
	~CaptureTexture();
};

#endif // CAPTURE_TEXTURE_H
