#include "planet_chunk_data.h"

const unsigned char block_textures[BLOCK_TYPE_COUNT][Cube::DIRECTION_COUNT] = {
		{ 0 },
		{ 1, 1, 1, 1, 1, 1 },
		{ 2, 2, 2, 2, 2, 2 },
		{ 0, 2, 3, 3, 3, 3 },
		{ 205, 205, 205, 205, 205, 205 }
};
