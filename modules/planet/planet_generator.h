
#ifndef GODOT_PLANET_GENERATOR_H
#define GODOT_PLANET_GENERATOR_H

#include <core/resource.h>
#include <modules/opensimplex/open_simplex_noise.h>
#include <modules/planet/math/vector3i.h>
#include "planet_chunk_data.h"

class PlanetGenerator : public Resource {
	GDCLASS(PlanetGenerator, Resource)

public:
	PlanetGenerator();

	void set_noise(Ref<OpenSimplexNoise> noise);
	Ref<OpenSimplexNoise> get_noise() const;

	void generate_chunk(PlanetSide planet_side, Vector3i &chunk_pos, ChunkData &chunk_data, int lod);

protected:
	static void _bind_methods();

private:
	Ref<OpenSimplexNoise> _noise;

};

#endif //GODOT_PLANET_GENERATOR_H
