#include "visuals.hpp"
#include "../../core/cheat_context.hpp"

void visuals::gravity(bool enable) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.network_manager); // network manager
	chain = mem->read<uintptr_t>(chain + 0xF8); // network manager connection
	chain = mem->read<uintptr_t>(chain + 0x8); // network manager connection game

	if (chain != 0)
		mem->write<float>(chain + 0x760, enable ? 1.5f : 0.f);
}
