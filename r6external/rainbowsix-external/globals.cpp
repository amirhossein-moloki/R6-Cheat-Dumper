#include "globals.hpp"

namespace globals {
	util::memory memory;
	uint32_t game_pid = 0;
	uintptr_t module_base = 0;

	bool w2s_good = false;
    entity targeted_entity = 0;

	uintptr_t game_manager = 0;
	uintptr_t game_profile = 0;
	uintptr_t network_manager = 0;
	uintptr_t input_manager = 0;
	uintptr_t round_manager = 0;
	uintptr_t glow_manager = 0;
	uintptr_t fov_manager = 0;
	uintptr_t freeze_manager = 0;

	uintptr_t entity_list = 0;
	uintptr_t render = 0;
	uintptr_t game_render = 0;
	uintptr_t engine_link = 0;
	uintptr_t engine = 0;
	uintptr_t camera = 0;
	uintptr_t vtable = 0;

	int window_horizontal_size = 0;
	int window_vertical_size = 0;

    vec3_t camera_view_right;
	vec3_t camera_view_up;
	vec3_t camera_view_forward;
	vec3_t camera_view_translation;
	float camera_view_fovx = 0;
	float camera_view_fovy = 0;
} // namespace globals
