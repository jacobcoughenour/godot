#ifndef GODOT_PLANET_CHUNK_H
#define GODOT_PLANET_CHUNK_H

#include "cube_tables.h"
#include "planet_data.h"
#include <core/os/os.h>
#include <modules/planet/math/vector3i.h>
#include <scene/3d/mesh_instance.h>

class PlanetChunk : public MeshInstance {
	GDCLASS(PlanetChunk, MeshInstance)

	Ref<Material> material;
	Ref<Material> transparent_material;

	Ref<PlanetData> planetData;
	ChunkData *chunkData;

public:
	PlanetChunk();
	PlanetChunk(ChunkData &chunkData);
	~PlanetChunk();

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_transparent_material(const Ref<Material> &p_material);
	Ref<Material> get_transparent_material() const;

	void set_planet_data(const Ref<PlanetData> &p_planet_data);
	Ref<PlanetData> get_planet_data() const;

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _ready();
	void _process();
};

#endif //GODOT_PLANET_CHUNK_H
