#include "cube_tables.h"

namespace Cube {

const Vector3 corner_positions[CORNER_COUNT] = {
	Vector3(1, 0, 0),
	Vector3(0, 0, 0),
	Vector3(0, 0, 1),
	Vector3(1, 0, 1),
	Vector3(1, 1, 0),
	Vector3(0, 1, 0),
	Vector3(0, 1, 1),
	Vector3(1, 1, 1)
};

const Vector3 directions[DIRECTION_COUNT] = {
	Vector3(0, 1, 0),
	Vector3(0, -1, 0),
	Vector3(0, 0, -1),
	Vector3(1, 0, 0),
	Vector3(0, 0, 1),
	Vector3(-1, 0, 0)
};

const Vector3i directions_i[DIRECTION_COUNT] = {
		Vector3i(0, 1, 0),
		Vector3i(0, -1, 0),
		Vector3i(0, 0, -1),
		Vector3i(1, 0, 0),
		Vector3i(0, 0, 1),
		Vector3i(-1, 0, 0)
};

const Vector3 side_axes[SIDE_COUNT][SIDE_AXIS_COUNT] = {
	{ directions[RIGHT], directions[UP], directions[BACK] },
	{ directions[FORWARD], directions[DOWN], directions[LEFT] },
	{ directions[RIGHT], directions[FORWARD], directions[UP] },
	{ directions[BACK], directions[RIGHT], directions[UP] },
	{ directions[LEFT], directions[BACK], directions[UP] },
	{ directions[FORWARD], directions[LEFT], directions[UP] }
};

const int side_triangles[6] = {
	0, 1, 2, 0, 2, 3
};

const int side_indices[SIDE_COUNT][4] = {
	{ 6, 5, 4, 7 },
	{ 0, 1, 2, 3 },
	{ 4, 5, 1, 0 },
	{ 7, 4, 0, 3 },
	{ 6, 7, 3, 2 },
	{ 5, 6, 2, 1 }
};

const Vector2 side_uvs[4] = {
	Vector2(0, 0),
	Vector2(1, 0),
	Vector2(1, 1),
	Vector2(0, 1)
};

} // namespace Cube
