
#include "planet_node.h"
#include <core/core_string_names.h>
#include <modules/planet/math/math.h>
#include <scene/gui/viewport_container.h>

PlanetNode::PlanetNode() :
		material(0) {
	visible_chunks = new Set<ChunkPosition>();
	chunks_to_show_queue = new List<ChunkPosition>();
}

PlanetNode::~PlanetNode() {
}

void PlanetNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &PlanetNode::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &PlanetNode::get_camera);
	ClassDB::bind_method(D_METHOD("set_sky_viewport", "sky_viewport"), &PlanetNode::set_sky_viewport);
	ClassDB::bind_method(D_METHOD("get_sky_viewport"), &PlanetNode::get_sky_viewport);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &PlanetNode::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &PlanetNode::get_data);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &PlanetNode::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PlanetNode::get_material);
	ClassDB::bind_method(D_METHOD("set_transparent_material", "transparent_material"), &PlanetNode::set_transparent_material);
	ClassDB::bind_method(D_METHOD("get_transparent_material"), &PlanetNode::get_transparent_material);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera", PROPERTY_HINT_RESOURCE_TYPE, "NodePath"), "set_camera", "get_camera");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "sky_viewport", PROPERTY_HINT_RESOURCE_TYPE, "NodePath"), "set_sky_viewport", "get_sky_viewport");
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

void PlanetNode::set_sky_viewport(const NodePath &p_sky_viewport) {
	if (sky_viewport == p_sky_viewport)
		return;
	sky_viewport = p_sky_viewport;
	_mark_dirty();
}

NodePath PlanetNode::get_sky_viewport() const {
	return sky_viewport;
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

	if (sky_viewport.is_empty()) {
		print_error("no sky viewport");
		return;
	}

	SphereMesh *sphere = memnew(SphereMesh());
	sphere->set_material(transparent_material);
	sphere->set_radius(512 / CHUNK_SIZE);
	sphere->set_height(1024 / CHUNK_SIZE);

	MeshInstance *mesh = memnew(MeshInstance());
	mesh->set_mesh(sphere);

	get_node(sky_viewport)->add_child(mesh);
}


void PlanetNode::_process() {
	float delta = get_process_delta_time();

	time_since_last_update += delta;
	time_since_last_lazy_load += delta;

	if (time_since_last_update >= 2.0) {
		update_chunks_to_show();
		time_since_last_update = 0;
	}

	if (time_since_last_lazy_load >= 2.0) {
		lazy_load_chunks();
		time_since_last_lazy_load = 0;
	}

	if (_dirty && _mounted) {
		if (_loaded) {
			unload_chunks();
		}
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

	chunks_to_show_queue->clear();
	visible_chunks->clear();

	_loaded = false;
}

void PlanetNode::lazy_load_chunks() {
	float start = get_process_delta_time();
	int count = 0;

	while (!chunks_to_show_queue->empty()) {

		auto chunk_pos = chunks_to_show_queue->front()->get();
		show_chunk(chunk_pos);
		count++;

		// pop from queue
		chunks_to_show_queue->pop_front();

		// timeout
		if (get_process_delta_time() - start >= 0.1f)
			break;
	}
}

static auto radius_sample_map = new List<Vector3*>();

void PlanetNode::update_chunks_to_show() {

	if (camera.is_empty()) {
		print_error("no camera");
		return;
	}

	if (camera_node == NULL)
		camera_node = (Camera*) get_node(camera);

	if (camera_node == NULL)
		return;

	Vector3 camera_pos = camera_node->get_translation() - get_translation();

	if (camera_pos.distance_squared_to(last_camera_position) < CHUNK_SIZE)
		return;

	last_camera_position = camera_pos;


	// clear the queue
	chunks_to_show_queue->clear();

	if (radius_sample_map->empty()) {

//		OS::get_singleton()->print("empty\n");

		int distance = render_distance * CHUNK_SIZE;
		float render_distance_squared = distance * distance;
		int chunk_step = CHUNK_SIZE / 2;

		for (int x = -distance; x < distance; x += chunk_step) {
			for (int y = -distance; y < distance; y += chunk_step) {
				for (int z = -distance; z < distance; z += chunk_step) {
					auto *p = new Vector3(x, y, z);
					if (p->length_squared() <= render_distance_squared)
						radius_sample_map->push_back(p);
				}
			}
		}

		radius_sample_map->sort();

//		for(auto *E = radius_sample_map->front(); E; E = E->next()) {
//			Vector3 *p = E->get();
//			OS::get_singleton()->print("%f,%f,%f\n", p->x, p->y, p->z);
//		}
	}

	auto chunks_to_show_set = Set<ChunkPosition>();

	for(auto *E = radius_sample_map->front(); E; E = E->next()) {
		Vector3 pos = camera_pos + *E->get();
		ChunkPosition *chunk_pos = PlanetData::get_chunk_position_at(pos);

		if (!chunks_to_show_set.has(*chunk_pos)) {
			chunks_to_show_set.insert(*chunk_pos);
			chunks_to_show_queue->push_back(*chunk_pos);
		}
	}
}

void PlanetNode::show_chunk(ChunkPosition &chunk_pos) {

	visible_chunks->insert(chunk_pos);

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

	visible_chunks->erase(chunk_pos);

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

