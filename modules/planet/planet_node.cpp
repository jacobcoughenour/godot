
#include <core/core_string_names.h>
#include <modules/planet/math/math.h>
#include "planet_node.h"

PlanetNode::PlanetNode() :
		material(0) {
}

PlanetNode::~PlanetNode() {
}

void PlanetNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_data", "data"), &PlanetNode::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &PlanetNode::get_data);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &PlanetNode::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PlanetNode::get_material);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "PlanetData"), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_material", "get_material");

}

void PlanetNode::set_data(const Ref<PlanetData> &p_data) {
	if (data.is_valid()) {
		data->disconnect(CoreStringNames::get_singleton()->changed, this, "data_changed");
	}
	data = p_data;
	if (data.is_valid()) {
		data->connect(CoreStringNames::get_singleton()->changed, this, "data_changed");
	}
}

Ref<PlanetData> PlanetNode::get_data() const {
	return data;
}


void PlanetNode::set_material(const Ref<Material> &p_material) {
	if (material == p_material)
		return;
	material = p_material;
	// _make_dirty();
}

Ref<Material> PlanetNode::get_material() const {
	return material;
}

void PlanetNode::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_ENTER_TREE:
			set_process(true);
			break;

		case NOTIFICATION_READY:
			_ready();
			break;

		case NOTIFICATION_PROCESS:
			// if (!Engine::get_singleton()->is_editor_hint()) {
			_process();
			// }
			break;

		case NOTIFICATION_EXIT_TREE:
			break;

		case NOTIFICATION_ENTER_WORLD:
			break;

		case NOTIFICATION_EXIT_WORLD:
			break;

		case NOTIFICATION_VISIBILITY_CHANGED:
			break;

		default:
			break;
	}
}

void PlanetNode::_ready() {

	if (data.is_null()) {
		print_error("no planet data");
		return;
	}

	int layer_count = 6;
	int side_chunk_count;

	int total_chunk_count = 0;

	for (int side = 0; side < SIDE_COUNT; side++) {
		for (int layer = 0; layer < layer_count; layer++) {

			side_chunk_count = layer == 0 ? 1 : (int) max_face_voxel_count(layer);

			for (int x = 0; x < side_chunk_count; x++) {
				for (int z = 0; z < side_chunk_count; z++) {

					ChunkPosition pos = ChunkPosition();
					pos.side = static_cast<PlanetSide>(side);
					pos.relative_position = Vector3i(x, layer, z);

					show_chunk(pos);

					total_chunk_count++;
				}
			}
		}
	}

	OS::get_singleton()->print("chunk count: %d\n", total_chunk_count);

}

void PlanetNode::unload_chunks() {

	for (int side = 0; side < SIDE_COUNT; side++) {
		for (auto *E = chunk_renderers[side].front(); E; E = E -> next()) {
			PlanetChunk *chunk = E->get();
			remove_child(chunk);
			free(chunk);
		}
		chunk_renderers[side].clear();
	}

}

void PlanetNode::show_chunk(ChunkPosition &chunk_pos) {

	PlanetChunk *chunk;

	// find existing chunk renderer
	auto chunk_renderer = chunk_renderers[chunk_pos.side].find(chunk_pos.relative_position);

	// no existing chunk renderer for this chunk
	if (chunk_renderer == NULL) {

		// get chunk data
		// (this also handles chunk generation)
		ChunkData *chunk_data = data->get_chunk(chunk_pos);

		// create a new chunk renderer
		// todo: recycle existing chunk renderer from unloaded chunk
		chunk = memnew(PlanetChunk(*chunk_data));
		chunk->set_material(material);

		// add chunk renderer to planet node
		add_child(chunk);

		// register it to the pool of chunk renderers
		chunk_renderers[chunk_pos.side].insert(chunk_pos.relative_position, chunk);

	}
	// found existing chunk renderer
	else {

		// get existing chunk renderer
		chunk = chunk_renderer->get();

		// ensure chunk renderer is visible
		chunk->set_visible(true);
	}

}



void PlanetNode::_process() {
	// print_line("_process");
}
