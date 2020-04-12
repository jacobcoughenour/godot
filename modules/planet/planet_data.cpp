
#include "planet_data.h"
#include <core/core_string_names.h>
#include <core/os/os.h>
#include <modules/planet/math/math.h>
#include <map>

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

ChunkData *PlanetData::get_chunk(ChunkPosition &chunk_position) {

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

const float identity_threshold = Vector3(1.0, 1.0, 1.0).normalized().x;
const float angle_offset = Math_PI / 4.0;
const float half_pi = Math_PI / 2.0;

ChunkPosition *PlanetData::get_chunk_position_at(Vector3 &position) {

	auto chunkPosition = new ChunkPosition();

	// figure out what planet side it's on

	Vector3 normal = position.normalized();
	Vector3 normal_abs = normal.abs();

	float max = MAX(MAX(normal_abs.x, normal_abs.y), normal_abs.z);

	// todo: could probably use some bit shifting to do the indexing
	int a = (normal.x > 0.0f ? 4 : 0)
			+ (normal.y > 0.0f ? 2 : 0)
			+ (normal.z > 0.0f ? 1 : 0);
	int b = (normal_abs.x == max ? 4 : 0)
			+ (normal_abs.y == max ? 2 : 0)
			+ (normal_abs.z == max ? 1 : 0) - 1;

	PlanetSide side = inverse_side_map[a][b];

	chunkPosition->side = side;

	// use angle_to to map the position to the face axes

	Vector3 x_dir = Cube::side_axes[side][Cube::SIDE_AXIS_X];
	Vector3 y_dir = Cube::side_axes[side][Cube::SIDE_AXIS_Y];
	Vector3 z_dir = Cube::side_axes[side][Cube::SIDE_AXIS_Z];

	Vector3 normal_x = ((y_dir + x_dir) * position).normalized();
	Vector3 normal_z = ((y_dir + z_dir) * position).normalized();

	float angle_x = acos(y_dir.dot(normal_x));
	if (angle_x > half_pi) {
		angle_x -= Math_PI;
	}
	if (z_dir.dot(y_dir.cross(normal_x)) > 0) {
		angle_x = -angle_x;
	}
	if (side == SIDE_FORWARD || side == SIDE_BACK) {
		angle_x = -angle_x;
	}

	float angle_z = acos(y_dir.dot(normal_z));
	if (angle_z > half_pi) {
		angle_z -= Math_PI;
	}
	if (x_dir.dot(y_dir.cross(normal_z)) < 0) {
		angle_z = -angle_z;
	}
	if (side == SIDE_FORWARD || side == SIDE_LEFT) {
		angle_z = -angle_z;
	}

	int dist = position.length() / CHUNK_SIZE;

	int chunk_count = dist < 1 ? 1 : max_face_voxel_count(dist);

	angle_x += angle_offset;
	angle_x /= half_pi;
	angle_x *= chunk_count;

	angle_z += angle_offset;
	angle_z /= half_pi;
	angle_z *= chunk_count;

//	OS::get_singleton()->print("side: %d angle_x: %f angle_z: %f\n", side, angle_x, angle_z);

//	chunkPosition->relative_position = Vector3i(0, dist, 0);
	chunkPosition->relative_position = Vector3i(MIN((int) angle_x, chunk_count-1), dist, MIN((int) angle_z, chunk_count-1));

//	OS::get_singleton()->print("x: %d y: %d z: %d \n", chunkPosition->relative_position.x, chunkPosition->relative_position.y, chunkPosition->relative_position.z);

	return chunkPosition;
}

const Vector3i CHUNK_ZERO = Vector3i(0);
const Vector3i CHUNK_MAX = Vector3i(CHUNK_SIZE);

Vector<ChunkPosition*> PlanetData::get_neighboring_chunk_positions(ChunkPosition &chunk_position, Cube::Directions direction) {

	auto *positions = new Vector<ChunkPosition*>();

	// check if we need to wrap to a different planet side

	// if core chunk, we definitely need to wrap
	if (chunk_position.relative_position == CHUNK_ZERO) {

		if (direction == Cube::UP) {

			// we know that the next layer is subdivided

			positions->resize(4);

			for (int i = 0; i < 4; i++) {
				auto pos = new ChunkPosition();
				pos->relative_position = Vector3i(i % 2, 1, i / 2);
				pos->side = chunk_position.side;
				positions->set(i, pos);
			}

			return *positions;
		}

		auto pos = new ChunkPosition();
		pos->relative_position = CHUNK_ZERO;
		pos->side = static_cast<PlanetSide>(Cube::relative_side_directions[chunk_position.side][direction]);
		positions->resize(1);
		positions->set(0, pos);

		return *positions;

	}

	Vector3i neighbor_pos = chunk_position.relative_position + Cube::directions_i[direction];

	int layer_chunk_count = (int)max_face_voxel_count(chunk_position.relative_position.y);

	// handle up and down

	if (direction == Cube::UP || direction == Cube::DOWN) {

		int neighbor_layer_chunk_count = (int)max_face_voxel_count(neighbor_pos.y);

		// both layers are aligned, so we don't have to do anything
		if (layer_chunk_count == neighbor_layer_chunk_count) {
			auto pos = new ChunkPosition();
			pos->relative_position = neighbor_pos;
			pos->side = chunk_position.side;
			positions->resize(1);
			positions->set(0, pos);

			return *positions;
		}

		// layers are not aligned

		if (direction == Cube::UP) {

			// since the layer above is subdivided +1 more than this one
			// we need to scale the position to align with that subdivision
			Vector3i offset = Vector3i(neighbor_pos.x * 2, neighbor_pos.y, neighbor_pos.z * 2);

			positions->resize(4);

			for (int i = 0; i < 4; i++) {
				auto pos = new ChunkPosition();
				pos->relative_position = offset + Vector3i(i % 2, 0, i / 2);
				pos->side = chunk_position.side;
				positions->set(i, pos);
			}

			return *positions;

		} else {
			auto pos = new ChunkPosition();

			// since the layer below is subdivided -1 less than this one
			// we need to scale the position to align with that subdivision
			pos->relative_position = Vector3i(neighbor_pos.x / 2, neighbor_pos.y, neighbor_pos.z / 2);
			pos->side = chunk_position.side;
			positions->resize(1);
			positions->set(0, pos);

			return *positions;
		}
	}

	// handle forward, back, left, and right

	auto pos = new ChunkPosition();

	pos->relative_position = Vector3i(
			// wrap the x and z to the layer chunk count
			wrap(neighbor_pos.x, layer_chunk_count),
			neighbor_pos.y,
			wrap(neighbor_pos.z, layer_chunk_count));

	// if the target chunk is on another planet side
	if (neighbor_pos.x < 0 || neighbor_pos.z < 0 || neighbor_pos.x >= layer_chunk_count || neighbor_pos.z >= layer_chunk_count) {
		// lookup the relative planet size and switch to it
		pos->side = static_cast<PlanetSide>(Cube::relative_side_directions[chunk_position.side][direction]);
	} else {
		// use the same planet side
		pos->side = chunk_position.side;
	}

	positions->resize(1);
	positions->set(0, pos);

	return *positions;
}

Vector<ChunkData*> PlanetData::get_neighboring_chunks(ChunkPosition &chunk_position, Cube::Directions direction) {
	auto chunks = new Vector<ChunkData*>();
	auto positions = get_neighboring_chunk_positions(chunk_position, direction);

	int size = positions.size();
	chunks->resize(size);

	for (int i = 0; i < size; i++) {
		auto position = positions.get(i);
		chunks->set(i, get_chunk(*position));
	}

	return *chunks;
}

BlockType PlanetData::get_neighboring_block(ChunkData *chunk_data, Vector3i &block_position, Cube::Directions direction) {

	Vector3i neighbor_pos = block_position + Cube::directions_i[direction];

	if (chunk_data->position.relative_position == CHUNK_ZERO) {

		return BLOCK_AIR;

//		int layer_block_count = (int)max_face_voxel_count(block_position.y);
//
//		if (direction == Cube::UP || direction == Cube::DOWN) {
//
//			if (neighbor_pos.y >= CHUNK_SIZE) {
//
//				// scale up to match the target layer
//				neighbor_pos.x *= 2;
//				neighbor_pos.z *= 2;
//
//				// ChunkData *chunks = get_neighboring_chunks(chunk_data->position, direction)[
//				// 		// pick the right chunk
//				// 		CHUNK_SIZE];
//			}
//
//			int neighbor_layer_block_count = (int)max_face_voxel_count(neighbor_pos.y);
//
//			if (layer_block_count == neighbor_layer_block_count) {
//
//				return get_block(chunk_data, neighbor_pos);
//			}
//		}
//
//		// handle forward, back, left, and right
//
//		// block is in another chunk
//		if (neighbor_pos.x < 0 || neighbor_pos.z < 0 || neighbor_pos.x >= layer_block_count || neighbor_pos.z >= layer_block_count) {
//
//			ChunkData *chunks = get_neighboring_chunks(chunk_data->position, direction);
//
//			neighbor_pos.x = wrap(neighbor_pos.x, layer_block_count);
//			neighbor_pos.z = wrap(neighbor_pos.z, layer_block_count);
//
//			return get_block(chunks, neighbor_pos);
//		}
//
//		return get_block(chunk_data, neighbor_pos);
	}

	// block is in the same chunk
	if (neighbor_pos.is_contained_in(CHUNK_ZERO, CHUNK_MAX)) {
		return get_block(chunk_data, neighbor_pos);
	}

	// block is in another chunk

	if (direction == Cube::UP || direction == Cube::DOWN) {

		neighbor_pos.y = wrap(neighbor_pos.y, CHUNK_SIZE);

		auto chunk_positions = get_neighboring_chunk_positions(chunk_data->position, direction);

		// only one chunk
		if (chunk_positions.size() == 1) {

			auto chunk_position = chunk_positions.get(0);

			// target layer is aligned
			if (direction == Cube::UP) {
				return get_block(*chunk_position, neighbor_pos);

			} else {
				int layer_chunk_count = (int) max_face_voxel_count(chunk_data->position.relative_position.y);
				int neighbor_layer_chunk_count = (int) max_face_voxel_count(chunk_position->relative_position.y);

				// target layer is aligned
				if (layer_chunk_count == neighbor_layer_chunk_count) {
					return get_block(*chunk_position, neighbor_pos);
				}

				// target layer is not aligned

				neighbor_pos.x += (chunk_position->relative_position.x % 2) * CHUNK_SIZE;
				neighbor_pos.z += (chunk_position->relative_position.z % 2) * CHUNK_SIZE;

				neighbor_pos.x = neighbor_pos.x / 2;
				neighbor_pos.z = neighbor_pos.z / 2;

				return get_block(*chunk_position, neighbor_pos);
			}
		}

		// multiple chunks

		// this should only happen when direction == up

		neighbor_pos.x *= 2;
		neighbor_pos.z *= 2;

		int index = (neighbor_pos.x / CHUNK_SIZE) + (neighbor_pos.z / CHUNK_SIZE) * 2;

		auto chunk_position = chunk_positions.get(index);

		neighbor_pos.x = wrap(neighbor_pos.x, CHUNK_SIZE);
		neighbor_pos.z = wrap(neighbor_pos.z, CHUNK_SIZE);

		BlockType solidBlock;

		for (int x = 0; x < 2; x++) {
			for (int z = 0; z < 2; z++) {

				Vector3i pos = Vector3i(neighbor_pos.x + x, neighbor_pos.y, neighbor_pos.z + z);
				BlockType blockType = get_block(*chunk_position, pos);

				if (blockType == BLOCK_AIR || blockType == BLOCK_WATER) {
					return blockType;
				}

				solidBlock = blockType;
			}
		}

		return solidBlock;
	}

	// handle forward, back, left, and right

	auto chunk_positions = get_neighboring_chunk_positions(chunk_data->position, direction);

	neighbor_pos.x = wrap(neighbor_pos.x, CHUNK_SIZE);
	neighbor_pos.z = wrap(neighbor_pos.z, CHUNK_SIZE);

	auto chunk_position = chunk_positions.get(0);
	return get_block(*chunk_position, neighbor_pos);
}

BlockType PlanetData::get_block(ChunkPosition &chunk_position, Vector3i &block_position) {
	ChunkData *chunk_data = get_chunk(chunk_position);
	return get_block(chunk_data, block_position);
}

BlockType PlanetData::get_block(ChunkData *chunk_data, Vector3i &block_position) {
	return chunk_data->data[block_position.x][block_position.y][block_position.z];
}
