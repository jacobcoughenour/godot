#ifndef CUBE_TABLES_H
#define CUBE_TABLES_H

#include <core/math/vector2.h>
#include <core/math/vector3.h>
#include <modules/planet/math/vector3i.h>

namespace Cube {

// 	   5-------4
//    /|      /|
//   / |     / |
//  6-------7  |
//  |  1----|--0
//  | /     | /    y
//  |/      |/     * x
//  2-------3     z

enum Corner {
	CORNER_DOWN_FRONT_RIGHT = 0,
	CORNER_DOWN_FRONT_LEFT,
	CORNER_DOWN_BACK_LEFT,
	CORNER_DOWN_BACK_RIGHT,
	CORNER_UP_FRONT_RIGHT,
	CORNER_UP_FRONT_LEFT,
	CORNER_UP_BACK_LEFT,
	CORNER_UP_BACK_RIGHT,

	CORNER_COUNT
};

extern const Vector3 corner_positions[CORNER_COUNT];

//      o---------o
//     /|        /|
//    / | U  F  / |
//   /  |      /  |
//  o---------o R |
//  | L o-----|---o
//  |  / B    |  /
//  | /    D  | /    y
//  |/        |/     * x
//  o---------o     z

enum Directions {
	UP = 0,
	DOWN,
	FORWARD,
	RIGHT,
	BACK,
	LEFT,

	DIRECTION_COUNT
};

extern const Vector3 directions[DIRECTION_COUNT];

extern const Vector3i directions_i[DIRECTION_COUNT];

enum Side {
	SIDE_UP = 0,
	SIDE_DOWN,
	SIDE_FORWARD,
	SIDE_RIGHT,
	SIDE_BACK,
	SIDE_LEFT,

	SIDE_COUNT
};

enum SideAxis {
	SIDE_AXIS_X = 0,
	SIDE_AXIS_Y,
	SIDE_AXIS_Z,

	SIDE_AXIS_COUNT
};

extern const Directions relative_side_directions[SIDE_COUNT][DIRECTION_COUNT];

extern const Vector3 side_axes[SIDE_COUNT][SIDE_AXIS_COUNT];

extern const int side_triangles[SIDE_COUNT];

extern const int side_indices[SIDE_COUNT][4];

extern const Vector2 side_uvs[4];




} // namespace Cube

#endif //CUBE_TABLES_H
