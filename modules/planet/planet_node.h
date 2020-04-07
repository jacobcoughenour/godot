#ifndef GODOT_PLANET_NODE_H
#define GODOT_PLANET_NODE_H

#include "planet_chunk.h"
#include "planet_data.h"
#include "planet_generator.h"
#include <core/engine.h>
#include <scene/3d/spatial.h>

class PlanetNode : public Spatial {
	GDCLASS(PlanetNode, Spatial)

	Ref<PlanetData> data;
	Ref<Material> material;

	Map<Vector3i, PlanetChunk*> chunk_renderers[SIDE_COUNT];

public:
	PlanetNode();
	~PlanetNode();

	void set_data(const Ref<PlanetData> &p_data);
	Ref<PlanetData> get_data() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

protected:
	static void _bind_methods();

	void _notification(int p_what);
	void _ready();
	void _process();

private:
	// bool _set(const StringName &p_name, const Variant &p_value);
	// bool _get(const StringName &p_name, Variant &r_ret) const;
	// void _get_property_list(List<PropertyInfo> *p_list) const;
	void unload_chunks();
	void show_chunk(PlanetSide planet_side, Vector3i &chunk_pos);

};

#endif //GODOT_PLANET_NODE_H
