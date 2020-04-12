
#include <core/os/os.h>
#include <core/core_string_names.h>
#include <modules/planet/math/math.h>
#include "planet_generator.h"
#include "cube_tables.h"

PlanetGenerator::PlanetGenerator() {
}

void PlanetGenerator::generate_chunk(ChunkData &chunk_data, int lod) {

	if (_noise.is_null()) {
		print_error("PlanetGenerator: noise is null");
		return;
	}

	_noise->set_seed(40);
	_noise->set_period(0.4);
	_noise->set_octaves(4);
	_noise->set_persistence(0.7);

	float offset = 8.0f;
	float difference = 64.0f;
	float sea_level = offset * difference;

	Vector3i chunk_offset = CHUNK_SIZE * chunk_data.position.relative_position;

	int face_voxel_count = 0;

	if (chunk_offset.y == 0) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			face_voxel_count = (int) max_face_voxel_count(y);
			for (int x = 0; x < face_voxel_count; x++) {
				for (int z = 0; z < face_voxel_count; z++) {
					chunk_data.data[x][y][z] = BLOCK_STONE;
				}
			}
		}

		return;
	}

	if (chunk_offset.y > (offset + 1) * difference) {
		return;
	}

	PlanetSide planet_side = chunk_data.position.side;

	int half_voxel_count = 0;
	float step;

	face_voxel_count = CHUNK_SIZE;
	half_voxel_count = (int)(max_face_voxel_count(chunk_offset.y)) / 2;
	step = 1.0f / (float)(half_voxel_count);

	Vector3 x_dir = Cube::side_axes[planet_side][Cube::SIDE_AXIS_X] * step;
	Vector3 y_dir = Cube::side_axes[planet_side][Cube::SIDE_AXIS_Y];
	Vector3 z_dir = Cube::side_axes[planet_side][Cube::SIDE_AXIS_Z] * step;

	for (int x = 0; x < face_voxel_count; x++) {
		for (int z = 0; z < face_voxel_count; z++) {

			// rotate the corners to match the face x and z directions
			Vector3 vert = x_dir * (float)(chunk_offset.x + x - half_voxel_count);
			vert += z_dir * (float)(chunk_offset.z + z - half_voxel_count);

			// since we are mapping a 2d (xz) plane to the sphere
			// we move the vec 1 unit in the y direction to align it
			// to the unit cube
			vert += y_dir;

			// map position to sphere
			map_to_sphere(vert);

			vert.normalize();

			// now scale the vec to match the layer depth
//				vert *= (float) y;

			float val = _noise->get_noise_3d(vert.x, vert.y, vert.z);

			val += offset;
			val *= difference;

			// skip empty columns
			if (chunk_offset.y > sea_level && chunk_offset.y - val > 0.0f) {
				continue;
			}

			bool prev_was_air = true;

			// starts at the top of the chunk + 1 to get the neighboring block
			for (int y = CHUNK_SIZE; y >= 0; y--) {

				int block_y = chunk_offset.y + y;
				float diff = block_y - val;
				float sea_diff = block_y - sea_level;

				// air/water
				BlockType blockType = sea_diff <= 0.0f ? BLOCK_WATER : BLOCK_AIR;

				// solid terrain
				if (diff <= 0.0f) {
					if (diff >= -3.0f) {
						blockType = sea_diff < 2.5f ? BLOCK_SAND :
										prev_was_air ? BLOCK_GRASS : BLOCK_DIRT;
					} else {
						blockType = BLOCK_STONE;
					}
				}

				prev_was_air = blockType == BLOCK_AIR;

				if (y < CHUNK_SIZE)
					chunk_data.data[x][y][z] = blockType;
			}
		}
	}
}

void PlanetGenerator::set_noise(Ref<OpenSimplexNoise> p_noise) {
	if (p_noise == _noise)
		return;
	if (_noise.is_valid()) {
		_noise->disconnect(CoreStringNames::get_singleton()->changed, this, "emit_changed");
	}
	_noise = p_noise;
	if (_noise.is_valid()) {
		_noise->connect(CoreStringNames::get_singleton()->changed, this, "emit_changed");
	}
	emit_changed();
}

Ref<OpenSimplexNoise> PlanetGenerator::get_noise() const {
	return _noise;
}

void PlanetGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &PlanetGenerator::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &PlanetGenerator::get_noise);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
}