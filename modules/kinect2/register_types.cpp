#include "register_types.h"

#include "core/object/class_db.h"
#include "kinect2.h"

void initialize_kinect2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Kinect2>();
}

void uninitialize_kinect2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	// Nothing to do here in this example.
}
