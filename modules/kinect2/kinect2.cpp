#include "kinect2.h"

#define PACK_DEPTH_R(d) (d & 0xFF)
#define PACK_DEPTH_G(d) ((d >> 8) & 0xFF)
#define UNPACK_DEPTH(r, g) ((r | (g << 8)))

bool Kinect2::init_sensor() {
	HRESULT hr = GetDefaultKinectSensor(&kinectSensor);

	if (kinectSensor) {
		IColorFrameSource *colorFrameSource = NULL;
		IDepthFrameSource *depthFrameSource = NULL;

		hr = kinectSensor->Open();

		if (SUCCEEDED(hr))
			hr = kinectSensor->get_ColorFrameSource(&colorFrameSource);
		if (SUCCEEDED(hr))
			hr = colorFrameSource->OpenReader(&colorFrameReader);

		if (colorFrameSource) {
			colorFrameSource->Release();
			colorFrameSource = NULL;
		}

		if (SUCCEEDED(hr))
			hr = kinectSensor->get_DepthFrameSource(&depthFrameSource);
		if (SUCCEEDED(hr))
			hr = depthFrameSource->OpenReader(&depthFrameReader);

		if (depthFrameSource) {
			depthFrameSource->Release();
			depthFrameSource = NULL;
		}

		kinectSensor->get_CoordinateMapper(&coordinateMapper);
	}

	if (FAILED(hr) || !kinectSensor) {
		ERR_PRINT("Failed to initialize kinect sensor");
		return false;
	}

	return true;
}

Ref<Image> Kinect2::acquire_latest_color_frame() {
	ERR_FAIL_COND_V(!colorFrameReader, Ref<Image>());

	IColorFrame *colorFrame = NULL;
	HRESULT hr = colorFrameReader->AcquireLatestFrame(&colorFrame);

	if (!SUCCEEDED(hr)) {
		if (colorFrame)
			colorFrame->Release();

		if (hr == E_PENDING)
			return Ref<Image>();

		ERR_FAIL_COND_V_MSG(true, Ref<Image>(), "Failed to acquire latest color frame " + hr);
	}

	IFrameDescription *frameDescription = NULL;
	int width = 1920;
	int height = 1080;
	ColorImageFormat imageFormat = ColorImageFormat_None;
	UINT bufferSize = 0;
	RGBQUAD *bufferRGB = new RGBQUAD[1920 * 1080];
	RGBQUAD *buffer = nullptr;

	if (SUCCEEDED(hr)) {
		hr = colorFrame->get_FrameDescription(&frameDescription);
	}
	if (SUCCEEDED(hr)) {
		hr = frameDescription->get_Width(&width);
	}
	if (SUCCEEDED(hr)) {
		hr = frameDescription->get_Height(&height);
	}
	if (SUCCEEDED(hr)) {
		hr = colorFrame->get_RawColorImageFormat(&imageFormat);
	}

	if (SUCCEEDED(hr)) {
		if (imageFormat == ColorImageFormat_Bgra) {
			// todo should use copy instead of accessing buffer because it is known to cause access violations.
			hr = colorFrame->AccessRawUnderlyingBuffer(&bufferSize, reinterpret_cast<BYTE **>(&buffer));
		} else if (bufferRGB) {
			buffer = bufferRGB;
			bufferSize = width * height * sizeof(RGBQUAD);
			hr = colorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE *>(buffer), ColorImageFormat_Bgra);
		} else {
			hr = E_FAIL;
		}
	}

	if (frameDescription)
		frameDescription->Release();
	if (colorFrame)
		colorFrame->Release();

	ERR_FAIL_COND_V_MSG(!SUCCEEDED(hr), Ref<Image>(), "colorFrame->AccessRawUnderlyingBuffer failed");

	Vector<uint8_t> data;

	int pixelCount = width * height;
	data.resize(pixelCount * 3);

	for (auto i = 0; i < pixelCount; i++) {
		data.write[i * 3 + 0] = buffer[i].rgbRed;
		data.write[i * 3 + 1] = buffer[i].rgbGreen;
		data.write[i * 3 + 2] = buffer[i].rgbBlue;
	}

	return Image::create_from_data(width, height, false, Image::FORMAT_RGB8, data);
}

bool Kinect2::acquire_latest_depth_frame_buffer(UINT16 *buffer, int width, int height) {
	ERR_FAIL_COND_V(!depthFrameReader, false);

	IDepthFrame *depthFrame = NULL;
	HRESULT hr = depthFrameReader->AcquireLatestFrame(&depthFrame);

	if (!SUCCEEDED(hr)) {
		if (depthFrame)
			depthFrame->Release();

		if (hr == E_PENDING)
			return false;

		ERR_FAIL_COND_V_MSG(true, false, "Failed to acquire latest depth frame " + hr);
	}

	// IFrameDescription *frameDescription = NULL;

	// if (SUCCEEDED(hr)) {
	// 	hr = depthFrame->get_FrameDescription(&frameDescription);
	// }
	// if (SUCCEEDED(hr)) {
	// 	hr = frameDescription->get_Width(&width);
	// }
	// if (SUCCEEDED(hr)) {
	// 	hr = frameDescription->get_Height(&height);
	// }

	if (SUCCEEDED(hr)) {
		// hr = depthFrame->AccessUnderlyingBuffer(&bufferSize, reinterpret_cast<UINT16 **>(&buffer));
		hr = depthFrame->CopyFrameDataToArray(width * height, buffer);

		// we're using CopyFrameDataToArray because using AccessUnderlyingBuffer
		// causes an access violation randomly.
		// I think it is caused by the buffer being written to or freed in
		// another thread and I think the only fix is to find a way to wait
		// for the buffer to be "ready" in some way.
		// copying shouldn't be too bad because the depth frame is only 512x424
	}

	// if (frameDescription)
	// 	frameDescription->Release();
	if (depthFrame)
		depthFrame->Release();

	ERR_FAIL_COND_V_MSG(!SUCCEEDED(hr), false, "depthFrame->AccessRawUnderlyingBuffer failed");

	return true;
}

Ref<Image> Kinect2::acquire_latest_depth_frame(uint32_t minDepth, uint32_t maxDepth, bool fullRange) {
	int width = 512;
	int height = 424;
	UINT16 *buffer = new UINT16[512 * 424];

	bool success = acquire_latest_depth_frame_buffer(buffer, width, height);

	ERR_FAIL_COND_V_MSG(!success, Ref<Image>(), "failed to acquire latest depth frame");

	Vector<uint8_t> data;

	int pixelCount = width * height;

	if (fullRange)
		data.resize(pixelCount * 2);
	else
		data.resize(pixelCount);

	for (auto i = 0; i < pixelCount; i++) {
		// the depth value is 16 bit so it goes from 0 to 65535.
		// we need to clamp it to the min and max depth values then normalize it to 0-255 for 8bit

		// get depth
		UINT16 depth = buffer[i];

		// clamp
		if (depth < minDepth)
			depth = minDepth;
		if (depth > maxDepth)
			depth = maxDepth;

		if (fullRange) {
			// store first half in red and second half in green
			data.write[i * 2 + 0] = PACK_DEPTH_R(depth);
			data.write[i * 2 + 1] = PACK_DEPTH_G(depth);
		} else {
			// normalize
			data.write[i] = (depth - minDepth) * 255 / (maxDepth - minDepth);
		}
	}

	return Image::create_from_data(width, height, false, fullRange ? Image::FORMAT_RG8 : Image::FORMAT_L8, data);
}

Ref<Mesh> Kinect2::create_mesh_from_depth_frame(uint32_t minDepth, uint32_t maxDepth) {
	int width = 512;
	int height = 424;
	UINT16 *buffer = new UINT16[512 * 424];

	bool success = acquire_latest_depth_frame_buffer(buffer, width, height);

	if (!success)
		return Ref<Mesh>();

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);

	Vector<Vector3> points;
	Vector<Vector3> normals;
	Vector<Vector2> uvs;
	Vector<int> indices;

	auto x_offset = width / 2;
	auto y_offset = height / 2;

	auto x_scale = 1.0f / width;
	auto y_scale = 1.0f / height;

	// auto data = depth->get_data();

	CameraSpacePoint *camera_points = new CameraSpacePoint[width * height];
	coordinateMapper->MapDepthFrameToCameraSpace(width * height, buffer, width * height, camera_points);
	ColorSpacePoint *color_points = new ColorSpacePoint[width * height];
	coordinateMapper->MapDepthFrameToColorSpace(width * height, buffer, width * height, color_points);

	points.resize(width * height);
	normals.resize(width * height);
	uvs.resize(width * height);

	// points.resize(4);
	// normals.resize(4);
	// uvs.resize(4);

	// points.set(0, Vector3(0, 0, 0));
	// points.set(1, Vector3(1, 0, 0));
	// points.set(2, Vector3(0, 1, 0));
	// points.set(3, Vector3(1, 1, 0));

	// normals.set(0, Vector3(0, 0, -1));
	// normals.set(1, Vector3(0, 0, -1));
	// normals.set(2, Vector3(0, 0, -1));
	// normals.set(3, Vector3(0, 0, -1));

	// uvs.set(0, Vector2(0, 0));
	// uvs.set(1, Vector2(1, 0));
	// uvs.set(2, Vector2(0, 1));
	// uvs.set(3, Vector2(1, 1));

	// indices.append(0);
	// indices.append(1);
	// indices.append(2);

	// indices.append(1);
	// indices.append(3);
	// indices.append(2);

	for (auto y = 0; y < height; y++) {
		for (auto x = 0; x < width; x++) {
			auto i = y * width + x;

			// uint16_t d = buffer[i];

			// CameraSpacePoint point;
			// coordinateMapper->MapDepthPointToCameraSpace(DepthSpacePoint{ (float)x, (float)y }, d, &point);

			// auto x_normalized = (x - x_offset) * x_scale;
			// auto y_normalized = (y - y_offset) * -y_scale;

			auto point = camera_points[i];

			if (isinf(point.X) || isinf(point.Y) || isinf(point.Z))
				points.set(i, Vector3(0, 0, 0));
			else
				points.set(i, Vector3(point.X, point.Y, point.Z));
			// points.set(i, Vector3(x_normalized, y_normalized, -depth * z_scale));
			// points.set(i, Vector3(camera_points[i].X, camera_points[i].Y, camera_points[i].Z));

			auto color = color_points[i];
			uvs.set(i, Vector2(color.X / 1920.0f, color.Y / 1080.0f));
		}
	}

	for (auto y = 0; y < height; y++) {
		for (auto x = 0; x < width; x++) {
			auto i = y * width + x;

			auto i1 = i;
			auto i2 = i + 1;
			auto i3 = i + width;
			auto i4 = i + width + 1;

			if (x == width - 1 || y == height - 1) {
				normals.set(i, Vector3(0, 0, -1));
				continue;
			}

			// generate normals
			auto normal = (points[i1] - points[i2])
								  .cross(points[i1] - points[i3])
								  .normalized();
			normals.set(i, normal);

			if (points[i1].is_zero_approx() || points[i2].is_zero_approx() || points[i3].is_zero_approx() || points[i4].is_zero_approx())
				continue;

			uint16_t depth1 = buffer[i1];
			uint16_t depth2 = buffer[i2];
			uint16_t depth3 = buffer[i3];
			uint16_t depth4 = buffer[i4];

			if (depth1 < minDepth || depth2 < minDepth || depth3 < minDepth || depth4 < minDepth || depth1 > maxDepth || depth2 > maxDepth || depth3 > maxDepth || depth4 > maxDepth)
				continue;

			indices.append(i1);
			indices.append(i3);
			indices.append(i2);

			indices.append(i2);
			indices.append(i3);
			indices.append(i4);
		}
	}

	arrays[Mesh::ARRAY_VERTEX] = points;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	arrays[Mesh::ARRAY_INDEX] = indices;

	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

	return mesh;
}

void Kinect2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_sensor"), &Kinect2::init_sensor);
	ClassDB::bind_method(D_METHOD("acquire_latest_color_frame"), &Kinect2::acquire_latest_color_frame);
	ClassDB::bind_method(D_METHOD("acquire_latest_depth_frame"), &Kinect2::acquire_latest_depth_frame);
	ClassDB::bind_method(D_METHOD("create_mesh_from_depth_frame"), &Kinect2::create_mesh_from_depth_frame);
}

Kinect2::Kinect2() :
		kinectSensor(NULL),
		colorFrameReader(NULL) {
}

Kinect2::~Kinect2() {
	if (colorFrameReader)
		colorFrameReader->Release();

	if (kinectSensor) {
		kinectSensor->Close();
		kinectSensor->Release();
	}
}
