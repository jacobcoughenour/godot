
#include "planet_node.h"
#include <core/core_string_names.h>
#include <modules/planet/math/math.h>
#include <scene/gui/viewport_container.h>

PlanetNode::PlanetNode() :
		material(0) {
}

PlanetNode::~PlanetNode() {
}

void PlanetNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &PlanetNode::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &PlanetNode::get_camera);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &PlanetNode::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &PlanetNode::get_data);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &PlanetNode::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PlanetNode::get_material);
	ClassDB::bind_method(D_METHOD("set_transparent_material", "transparent_material"), &PlanetNode::set_transparent_material);
	ClassDB::bind_method(D_METHOD("get_transparent_material"), &PlanetNode::get_transparent_material);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera", PROPERTY_HINT_RESOURCE_TYPE, "NodePath"), "set_camera", "get_camera");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "PlanetData"), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "transparent_material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_transparent_material", "get_transparent_material");
}

void PlanetNode::set_camera(const NodePath &p_camera) {
	if (camera == p_camera)
		return;
	camera = p_camera;
	_mark_dirty();
}

NodePath PlanetNode::get_camera() const {
	return camera;
}

void PlanetNode::set_data(const Ref<PlanetData> &p_data) {
	if (data.is_valid()) {
		data->disconnect(CoreStringNames::get_singleton()->changed, this, "data_changed");
	}
	data = p_data;
	if (data.is_valid()) {
		data->connect(CoreStringNames::get_singleton()->changed, this, "data_changed");
	}
	_mark_dirty();
}

Ref<PlanetData> PlanetNode::get_data() const {
	return data;
}

void PlanetNode::set_material(const Ref<Material> &p_material) {
	if (material == p_material)
		return;
	material = p_material;
	_mark_dirty();
}

Ref<Material> PlanetNode::get_material() const {
	return material;
}

void PlanetNode::set_transparent_material(const Ref<Material> &p_material) {
	if (transparent_material == p_material)
		return;
	transparent_material = p_material;
	_mark_dirty();
}

Ref<Material> PlanetNode::get_transparent_material() const {
	return transparent_material;
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

	_mounted = true;

	if (data.is_null()) {
		print_error("no planet data");
		return;
	}

	if (camera.is_empty()) {
		print_error("no camera");
		return;
	}

	camera_node = (Camera*) get_node(camera);

	SphereMesh *sphere = memnew(SphereMesh());
	sphere->set_material(transparent_material);
	sphere->set_radius(512);
	sphere->set_height(1024);

	MeshInstance *mesh = memnew(MeshInstance());
	mesh->set_mesh(sphere);

	add_child(mesh);
}


void PlanetNode::_process() {
	float delta = get_process_delta_time();

	time_since_last_update += delta;

	if (time_since_last_update >= 0.2) {
		show_chunks();

		time_since_last_update = 0;
	}

	if (_dirty && _mounted) {

		if (_loaded) {
			unload_chunks();
		}

		show_chunks();

		_loaded = true;
		_dirty = false;
	}

}

void PlanetNode::_mark_dirty() {
	this->_dirty = true;
}

void PlanetNode::unload_chunks() {

	for (int side = 0; side < SIDE_COUNT; side++) {
		for (auto *E = chunk_renderers[side].front(); E; E = E->next()) {
			PlanetChunk *chunk = E->get();
			remove_child(chunk);
			chunk->queue_delete();
		}
		chunk_renderers[side].clear();
	}

	visible_chunks.clear();

	_loaded = false;
}

void PlanetNode::show_chunks() {

	if (camera.is_empty()) {
		print_error("no camera");
		return;
	}

	if (camera_node == NULL) {
		camera_node = (Camera*) get_node(camera);
	}

	Vector3 camera_pos = camera_node->get_translation() - get_translation();

	if (camera_pos.distance_squared_to(last_camera_position) < CHUNK_SIZE)
		return;

	last_camera_position = camera_pos;

	int render_distance = 8 * CHUNK_SIZE;
	int render_distance_squared = render_distance * render_distance;
	int chunk_step = CHUNK_SIZE / 2;

	auto *chunks_to_show = new Set<ChunkPosition>();

	Vector3 min = camera_pos - Vector3(render_distance, render_distance, render_distance);
	Vector3 max = camera_pos + Vector3(render_distance, render_distance, render_distance);

	for (int x = min.x; x < max.x; x += chunk_step) {
		for (int y = min.y; y < max.y; y += chunk_step) {
			for (int z = min.z; z < max.z; z += chunk_step) {

				Vector3 *wpos = new Vector3(x, y, z);

				if (wpos->distance_squared_to(camera_pos) > render_distance_squared)
					continue;

				ChunkPosition *cpos = PlanetData::get_chunk_position_at(*wpos);
				chunks_to_show->insert(*cpos);
			}
		}
	}

//	for(auto *E = visible_chunks.front(); E; E = E->next()) {
//		ChunkPosition pos = E->get();
//
//		if (!chunks_to_show->has(pos)) {
//			hide_chunk(pos);
//		}
//	}

	for(auto *E = chunks_to_show->front(); E; E = E->next()) {
		ChunkPosition pos = E->get();

		if (!visible_chunks.has(pos)) {
			show_chunk(pos);
		}

	}

}

void PlanetNode::show_chunk(ChunkPosition &chunk_pos) {

	visible_chunks.insert(chunk_pos);

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
		chunk->set_planet_data(data);
		chunk->set_material(material);
		chunk->set_transparent_material(transparent_material);

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

void PlanetNode::hide_chunk(ChunkPosition &chunk_pos) {

	visible_chunks.erase(chunk_pos);

	PlanetChunk *chunk;

	// find existing chunk renderer
	auto chunk_renderer = chunk_renderers[chunk_pos.side].find(chunk_pos.relative_position);

	// found one
	if (chunk_renderer != NULL) {

		// get existing chunk renderer
		chunk = chunk_renderer->get();

		// hide it
		chunk->set_visible(false);
	}

}

