
#ifndef GODOT_PLANET_DATA_H
#define GODOT_PLANET_DATA_H

#include <core/resource.h>
#include <modules/opensimplex/open_simplex_noise.h>
#include "math/vector3i.h"
#include "planet_generator.h"
#include "planet_chunk_data.h"

class PlanetData : public Resource {

	GDCLASS(PlanetData, Resource)

	Ref<PlanetGenerator> generator;

public:
	PlanetData();
	~PlanetData();

	Map<Vector3i, ChunkData*> sides[SIDE_COUNT];

	void set_generator(const Ref<PlanetGenerator> &p_generator);
	Ref<PlanetGenerator> get_generator() const;

	ChunkData* get_chunk(ChunkPosition &chunk_position);

	BlockType get_block(ChunkPosition &chunk_position, Vector3i &block_position);
	BlockType get_block(ChunkData *chunkData, Vector3i &block_position);

protected:
	static void _bind_methods();
};

#endif //GODOT_PLANET_DATA_H
