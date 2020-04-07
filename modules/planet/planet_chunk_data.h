
#ifndef GODOT_PLANET_CHUNK_DATA_H
#define GODOT_PLANET_CHUNK_DATA_H

#include "cube_tables.h"

enum PlanetSide {
	SIDE_UP = 0,
	SIDE_DOWN,
	SIDE_FORWARD,
	SIDE_RIGHT,
	SIDE_BACK,
	SIDE_LEFT,

	SIDE_COUNT
};

enum BlockType {
	BLOCK_AIR = 0,
	BLOCK_STONE,
	BLOCK_DIRT,
	BLOCK_GRASS,
	BLOCK_WATER,

	BLOCK_TYPE_COUNT
};

static const int ATLAS_SIZE = 16;
extern const unsigned char block_textures[BLOCK_TYPE_COUNT][Cube::DIRECTION_COUNT];

static const int CHUNK_SIZE = 16;
static const int CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

struct ChunkData {
	PlanetSide side;
	Vector3i position;
	BlockType data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

#endif //GODOT_PLANET_CHUNK_DATA_H
