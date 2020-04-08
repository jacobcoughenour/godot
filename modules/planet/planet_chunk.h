#ifndef GODOT_PLANET_CHUNK_H
#define GODOT_PLANET_CHUNK_H

#include <modules/planet/math/vector3i.h>
#include "cube_tables.h"
#include "planet_data.h"
#include <scene/3d/mesh_instance.h>
#include <core/os/os.h>

class PlanetChunk : public MeshInstance {
	GDCLASS(PlanetChunk, MeshInstance)

	Ref<Material> material;

	ChunkData *chunkData;

public:
	PlanetChunk();
	PlanetChunk(ChunkData &chunkData);
	~PlanetChunk();

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _ready();
	void _process();
};

#endif //GODOT_PLANET_CHUNK_H
