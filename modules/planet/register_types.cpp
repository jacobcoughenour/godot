
#include "register_types.h"

#include "core/class_db.h"
#include "planet_generator.h"
#include "planet_chunk.h"
#include "planet_data.h"
#include "planet_node.h"

void register_planet_types() {
	ClassDB::register_class<PlanetGenerator>();
	ClassDB::register_class<PlanetChunk>();
	ClassDB::register_class<PlanetData>();
	ClassDB::register_class<PlanetNode>();
}

void unregister_planet_types() {
}
