#include "planet_chunk_data.h"

const unsigned char block_textures[BLOCK_TYPE_COUNT][Cube::DIRECTION_COUNT] = {
		{ 0 },
		{ 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 2 },
		{ 0, 2, 3, 3, 3, 3 },
		{ 18, 18, 18, 18, 18, 18 },
		{ 205, 205, 205, 205, 205, 205 }
};

// 	   2-------6
//    /|      /|
//   / |     / |
//  3-------7  |
//  |  0----|--4
//  | /     | /    y
//  |/      |/     * x
//  1-------5     z
//
//        0
//    4-_   _-2
//       -6-
//    3   |   1
//        5

const PlanetSide inverse_side_map[Cube::CORNER_COUNT][Cube::CORNER_COUNT - 1] = {
	{ SIDE_FORWARD, SIDE_DOWN, SIDE_DOWN,    SIDE_LEFT,  SIDE_FORWARD, SIDE_LEFT,  SIDE_DOWN },
	{ SIDE_BACK,    SIDE_DOWN, SIDE_BACK,    SIDE_LEFT,  SIDE_LEFT,    SIDE_DOWN,  SIDE_BACK },
	{ SIDE_FORWARD, SIDE_UP,   SIDE_FORWARD, SIDE_LEFT,  SIDE_LEFT,    SIDE_UP,    SIDE_UP },
	{ SIDE_BACK,    SIDE_UP,   SIDE_UP,      SIDE_LEFT,  SIDE_BACK,    SIDE_LEFT,  SIDE_LEFT },
	{ SIDE_FORWARD, SIDE_DOWN, SIDE_FORWARD, SIDE_RIGHT, SIDE_RIGHT,   SIDE_DOWN,  SIDE_RIGHT },
	{ SIDE_BACK,    SIDE_DOWN, SIDE_DOWN,    SIDE_RIGHT, SIDE_BACK,    SIDE_RIGHT, SIDE_DOWN },
	{ SIDE_FORWARD, SIDE_UP,   SIDE_UP,      SIDE_RIGHT, SIDE_FORWARD, SIDE_RIGHT, SIDE_FORWARD },
	{ SIDE_BACK,    SIDE_UP,   SIDE_BACK,    SIDE_RIGHT, SIDE_RIGHT,   SIDE_UP,    SIDE_UP }
};