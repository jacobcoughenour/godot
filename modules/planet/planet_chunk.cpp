
#include <modules/planet/math/math.h>
#include "planet_chunk.h"

PlanetChunk::PlanetChunk() {

}

PlanetChunk::PlanetChunk(const PlanetSide planetSide, const Vector3i& chunkPosition, ChunkData &chunkData) : planetSide(planetSide), chunkPosition(chunkPosition) {
	this->chunkData = &chunkData;
}

PlanetChunk::~PlanetChunk() {
}

void PlanetChunk::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_material", "material"), &PlanetChunk::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PlanetChunk::get_material);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_material", "get_material");
}

void PlanetChunk::set_material(const Ref<Material> &p_material) {
	if (material == p_material)
		return;
	material = p_material;
	// _make_dirty();
}

Ref<Material> PlanetChunk::get_material() const {
	return material;
}

void PlanetChunk::_notification(int p_what) {

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

void PlanetChunk::_ready() {

	float atlas_step = 1.0f / (float)(ATLAS_SIZE);

	PoolVector3Array verts;
	PoolVector2Array uvs;
	PoolVector2Array uvs2;
	PoolVector3Array normals;
	PoolIntArray indices;

	Vector3i chunk_offset = CHUNK_SIZE * chunkPosition;

	int face_voxel_count = 0;
	int half_voxel_count = 0;
	float step;

	Vector3 x_dir;
	Vector3 y_dir = Cube::side_axes[planetSide][Cube::SIDE_AXIS_Y];
	Vector3 z_dir;

	if (chunk_offset.y > 0) {
		face_voxel_count = CHUNK_SIZE;
		half_voxel_count = (int)(max_face_voxel_count(chunk_offset.y)) / 2;
		step = 1.0f / (float)(half_voxel_count);

		x_dir = Cube::side_axes[planetSide][Cube::SIDE_AXIS_X] * step;
		z_dir = Cube::side_axes[planetSide][Cube::SIDE_AXIS_Z] * step;
	}

	for (int layer = 0; layer < CHUNK_SIZE; layer++) {

		if (chunk_offset.y == 0) {
			face_voxel_count = (int) max_face_voxel_count(layer);
			half_voxel_count = face_voxel_count / 2;
			step = 1.0f / (float)(half_voxel_count);

			x_dir = Cube::side_axes[planetSide][Cube::SIDE_AXIS_X] * step;
			z_dir = Cube::side_axes[planetSide][Cube::SIDE_AXIS_Z] * step;
		}

		for (int x = 0; x < face_voxel_count; x++) {
			for (int z = 0; z < face_voxel_count; z++) {

				BlockType block_type = chunkData->data[x][layer][z];

				if (block_type == BLOCK_AIR) {
					continue;
				}

				Vector3i block_pos = Vector3i(x, layer, z);

				// build the voxel

				Vector3 mapped_corners_positions[Cube::CORNER_COUNT];

				for (int corner = 0; corner < Cube::CORNER_COUNT; corner++) {

					// rotate the corners to match the face x and z directions
					Vector3 vert =
							x_dir * (chunk_offset.x + x - half_voxel_count + Cube::corner_positions[corner].x);
					vert += z_dir * (chunk_offset.z + z - half_voxel_count + Cube::corner_positions[corner].z);

					// since we are mapping a 2d (xz) plane to the sphere
					// we move the vec 1 unit in the y direction to align it
					// to the unit cube
					vert += y_dir;

					// map position to sphere
					map_to_sphere(vert);

					// now scale the vec to match the layer depth
					vert *= chunk_offset.y + layer + Cube::corner_positions[corner].y;

					mapped_corners_positions[corner] = vert;
				}

				for (int side = 0; side < Cube::SIDE_COUNT; side++) {

					// todo properly cull sides here

					if (chunk_offset.y > 0) {
						Vector3i neighbor_pos = block_pos + Cube::directions_i[side];

						if (neighbor_pos.is_contained_in(Vector3i(0), Vector3i(CHUNK_SIZE))) {

							// todo get neighbor blocks from other chunks
							BlockType neighbor_block_type = chunkData->data[neighbor_pos.x][neighbor_pos.y][neighbor_pos.z];
							// custom water culling
							if (block_type == BLOCK_WATER) {

								if (neighbor_block_type != BLOCK_AIR) {
									continue;
								}
							} else if (neighbor_block_type != BLOCK_AIR && neighbor_block_type != BLOCK_WATER) {
								continue;
							}
						}
					}

					int offset = verts.size();

					for (int side_triangle : Cube::side_triangles) {
						indices.append(offset + side_triangle);
					}

					// top-left of texture in the texture atlas
					int texture_id = block_textures[block_type][side];
					Vector2 uv2_offset = Vector2(texture_id % ATLAS_SIZE, texture_id / ATLAS_SIZE) * atlas_step;

					for (int side_corner = 0; side_corner < 4; side_corner++) {

						Vector3 corner = mapped_corners_positions[Cube::side_indices[side][side_corner]];

						verts.append(corner);
						uvs.append(Cube::side_uvs[side_corner]);

						// map block texture from atlas
						uvs2.append(uv2_offset + (Cube::side_uvs[side_corner] * atlas_step));

						if (side == Cube::SIDE_UP) {
							normals.append(corner.normalized());
						} else if (side == Cube::SIDE_DOWN) {
							normals.append(-corner.normalized());
						}
					}

					if (side != Cube::SIDE_UP && side != Cube::SIDE_DOWN) {
						Vector3 norm = mapped_corners_positions[Cube::side_indices[side][2]]
											   .cross(mapped_corners_positions[Cube::side_indices[side][0]]);
						normals.append(norm);
						normals.append(norm);
						normals.append(norm);
						normals.append(norm);
					}
				}
			}
		}
	}

	// empty chunk
	if (verts.empty())
		return;

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);

	arrays[Mesh::ARRAY_VERTEX] = verts;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	arrays[Mesh::ARRAY_TEX_UV2] = uvs2;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_INDEX] = indices;

	Ref<ArrayMesh> mesh;
	mesh.instance();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

	mesh->surface_set_material(0, material);

	set_mesh(*mesh);
}

void PlanetChunk::_process() {
	// print_line("_process");
}

