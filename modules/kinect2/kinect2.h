#ifndef KINECT2_H
#define KINECT2_H

// we have to rename the Vector4 to avoid conflicts with the Kinect SDK
#define Vector4 Godot_Vector4
#include "core/io/image.h"
#include "core/object/ref_counted.h"
#include "scene/resources/mesh.h"
#include "servers/rendering_server.h"
#undef Vector4
#define Vector4 Kinect_Vector4
#include <kinect.h>
#undef Vector4
#include <windows.h>

class Kinect2 : public RefCounted {
	GDCLASS(Kinect2, RefCounted);

private:
	IKinectSensor *kinectSensor;
	IColorFrameReader *colorFrameReader;
	IDepthFrameReader *depthFrameReader;
	ICoordinateMapper *coordinateMapper;

	bool acquire_latest_depth_frame_buffer(UINT16 *buffer, int width, int height);

protected:
	static void _bind_methods();

public:
	bool init_sensor();
	Ref<Image> acquire_latest_color_frame();
	Ref<Image> acquire_latest_depth_frame(uint32_t minDepth, uint32_t maxDepth, bool fullRange);
	Ref<Mesh> create_mesh_from_depth_frame(uint32_t minDepth, uint32_t maxDepth);

	Kinect2();
	~Kinect2();
};

#endif // KINECT2_H
