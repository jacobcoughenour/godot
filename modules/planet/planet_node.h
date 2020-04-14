#ifndef GODOT_PLANET_NODE_H
#define GODOT_PLANET_NODE_H

#include "planet_chunk.h"
#include "planet_data.h"
#include "planet_generator.h"
#include <core/engine.h>
#include <scene/3d/camera.h>
#include <scene/resources/primitive_meshes.h>

class PlanetNode : public Spatial {
	GDCLASS(PlanetNode, Spatial)

	NodePath camera;
	NodePath sky_viewport;
	Ref<PlanetData> data;
	Ref<Material> material;
	Ref<Material> transparent_material;

	int render_distance = 4;

public:
	PlanetNode();
	~PlanetNode();

	void set_camera(const NodePath &p_camera);
	NodePath get_camera() const;

	void set_sky_viewport(const NodePath &p_sky_viewport);
	NodePath get_sky_viewport() const;

	void set_data(const Ref<PlanetData> &p_data);
	Ref<PlanetData> get_data() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_transparent_material(const Ref<Material> &p_material);
	Ref<Material> get_transparent_material() const;

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _ready();
	void _process();

private:
	// bool _set(const StringName &p_name, const Variant &p_value);
	// bool _get(const StringName &p_name, Variant &r_ret) const;
	// void _get_property_list(List<PropertyInfo> *p_list) const;

	Map<Vector3i, PlanetChunk*> chunk_renderers[SIDE_COUNT];
	Set<ChunkPosition> *visible_chunks;
	List<ChunkPosition> *chunks_to_show_queue;

	bool _dirty = false;
	bool _mounted = false;
	bool _loaded = false;

	Camera *camera_node;
	Vector3 last_camera_position;
	float time_since_last_update = 0;
	float time_since_last_lazy_load = 0;

	void _mark_dirty();
	void unload_chunks();
	void update_chunks_to_show();
	void lazy_load_chunks();
	void show_chunk(ChunkPosition &chunk_pos);
	void hide_chunk(ChunkPosition &chunk_pos);

};

#endif //GODOT_PLANET_NODE_H
