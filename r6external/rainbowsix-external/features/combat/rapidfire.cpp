#include "combat.hpp"
#include "../../core/cheat_context.hpp"
#include "../../offsets.hpp"
#include "../../game/game_util.h"

enum fire_type {
	full_auto = 0,
	burst2 = 1,
	burst3 = 2,
	single = 3
};

void combat::rapid_fire(bool enabled) { // write_memory<int>(chain + 0x011A, 1); // wiggles gun
	if (!enabled)
		return;
	
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(game::get_local_entity().get_obj() + 0x90);
	chain = mem->read<uintptr_t>(chain + 0xC8);

	if (chain != 0)
		mem->write<int>(chain + 0x108, fire_type(full_auto)); // rapid fire
}