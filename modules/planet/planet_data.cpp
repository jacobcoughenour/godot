
#include <map>
#include <core/os/os.h>
#include <core/core_string_names.h>
#include <modules/planet/math/math.h>
#include "planet_data.h"

PlanetData::PlanetData() {
}

PlanetData::~PlanetData() {
}

void PlanetData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_generator", "generator"), &PlanetData::set_generator);
	ClassDB::bind_method(D_METHOD("get_generator"), &PlanetData::get_generator);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "generator", PROPERTY_HINT_RESOURCE_TYPE, "PlanetGenerator"), "set_generator", "get_generator");
}

void PlanetData::set_generator(const Ref<PlanetGenerator> &p_generator) {
	if (generator == p_generator)
		return;
	if (generator.is_valid()) {
		generator->disconnect(CoreStringNames::get_singleton()->changed, this, "emit_changed");
	}
	generator = p_generator;
	if (generator.is_valid()) {
		generator->connect(CoreStringNames::get_singleton()->changed, this, "emit_changed");
	}

	emit_changed();
}

Ref<PlanetGenerator> PlanetData::get_generator() const {
	return generator;
}

ChunkData* PlanetData::get_chunk(ChunkPosition &chunk_position) {

	auto *chunk_entry = sides[chunk_position.side].find(chunk_position.relative_position);

	if (chunk_entry == nullptr) {

		ChunkData *chunk_data = new ChunkData();
		chunk_data->position = chunk_position;

		generator->generate_chunk(*chunk_data, 0);

		sides[chunk_position.side].insert(chunk_position.relative_position, chunk_data);

		return chunk_data;
	}

	return chunk_entry->get();

}


const Vector3i CHUNK_ZERO = Vector3i(0);
const Vector3i CHUNK_MAX = Vector3i(CHUNK_SIZE);

//ChunkData* PlanetData::get_neighboring_chunk(ChunkPosition &chunk_position, Cube::Side side) {

//	Vector3i neighbor_pos = block_position + Cube::directions_i[side];

//}

//BlockType PlanetData::get_neighboring_block(ChunkPosition &chunk_position, Vector3i &block_position, Cube::Side side) {
//
//	Vector3i neighbor_pos = block_position + Cube::directions_i[side];
//
//	if (chunk_position.relative_position.y == 0) {
//
//		max_face_voxel_count(neighbor_pos.y);
//
//
//	}
//
//	if (neighbor_pos.is_contained_in(CHUNK_ZERO, CHUNK_MAX)) {
//
//
//
//	}
//
//
//}

BlockType PlanetData::get_block(ChunkPosition &chunk_position, Vector3i &block_position) {
	ChunkData* chunk_data = get_chunk(chunk_position);
	return get_block(chunk_data, block_position);
}

BlockType PlanetData::get_block(ChunkData *chunk_data, Vector3i &block_position) {
	return chunk_data->data[block_position.x][block_position.y][block_position.z];
}

