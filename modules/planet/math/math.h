
#ifndef GODOT_MATH_H
#define GODOT_MATH_H

#include <core/math/vector3.h>

inline void map_to_sphere(Vector3 &position) {
	float x2 = position.x * position.x;
	float y2 = position.y * position.y;
	float z2 = position.z * position.z;

	position.x = position.x * sqrt(1.0f - (y2 * 0.5f) - (z2 * 0.5f) + ((y2 * z2) / 3.0f));
	position.y = position.y * sqrt(1.0f - (z2 * 0.5f) - (x2 * 0.5f) + ((z2 * x2) / 3.0f));
	position.z = position.z * sqrt(1.0f - (x2 * 0.5f) - (y2 * 0.5f) + ((x2 * y2) / 3.0f));
}

inline unsigned int max_face_divisions(unsigned int radius) {
	if (radius <= 0)
		return 1;

	// https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c#994709
	int targetlevel = 1;
	while (radius >>= 1) ++targetlevel;

	return targetlevel;
}

inline unsigned int max_face_voxel_count(unsigned int radius) {
	if (radius <= 1)
		return 2;

	return pow(2, max_face_divisions(radius) - 1) * 2;
}

#endif //GODOT_MATH_H
