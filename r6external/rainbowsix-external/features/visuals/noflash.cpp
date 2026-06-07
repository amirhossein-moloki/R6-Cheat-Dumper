#include "visuals.hpp"
#include "../../core/cheat_context.hpp"

void visuals::noflash(byte value) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.game_profile + 0x78);
	chain = mem->read<uintptr_t>(chain + 0x0);
	chain = mem->read<uintptr_t>(chain + 0x28);
	chain = mem->read<uintptr_t>(chain + 0x30);
	chain = mem->read<uintptr_t>(chain + 0x30);
	chain = mem->read<uintptr_t>(chain + 0x28);
	
	if (chain != 0)
		mem->write<byte>(chain + 0x40, value);
}
