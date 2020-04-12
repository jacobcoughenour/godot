
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
	BLOCK_SAND,
	BLOCK_WATER,

	BLOCK_TYPE_COUNT
};

static const int ATLAS_SIZE = 16;
extern const unsigned char block_textures[BLOCK_TYPE_COUNT][Cube::DIRECTION_COUNT];
extern const PlanetSide inverse_side_map[Cube::CORNER_COUNT][Cube::CORNER_COUNT - 1];

static const int CHUNK_SIZE = 16;
static const int CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

struct ChunkPosition {
	PlanetSide side;
	Vector3i relative_position;
};

_FORCE_INLINE_ bool operator<(const ChunkPosition &a, const ChunkPosition &b) {
	if (a.side == b.side) {
		return a.relative_position < b.relative_position;
	} else {
		return a.side < b.side;
	}
}

struct ChunkData {
	ChunkPosition position;
	BlockType data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]{};
};

#endif //GODOT_PLANET_CHUNK_DATA_H
