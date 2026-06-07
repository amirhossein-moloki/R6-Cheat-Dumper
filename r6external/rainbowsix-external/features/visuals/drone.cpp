#include "visuals.hpp"
#include "../../core/cheat_context.hpp"

bool visuals::state = false;
int visuals::real_team = 0;

void visuals::drone(bool enabled) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	if (enabled) {
		uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.game_manager + 0x250);
		chain = mem->read<uintptr_t>(chain + 0xBB8);
		if (chain != 0) {
			mem->write<float>(chain + 0x48, 1.f); // red
			mem->write<float>(chain + 0x48 + 0x4, 0.f); // blue
			mem->write<float>(chain + 0x48 + 0x8, 0.f); // green
			mem->write<float>(chain + 0x48 + 0xC, 1.f); // alpha
		}
	}
}
