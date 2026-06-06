#include "movement.hpp"
#include "../../offsets.hpp"
#include "../../core/cheat_context.hpp"

void movement::speed(int speed) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.game_manager + 0xC8);
	chain = mem->read<uintptr_t>(chain + 0x0);
	chain = mem->read<uintptr_t>(chain + 0x30);
	chain = mem->read<uintptr_t>(chain + 0x30);
	chain = mem->read<uintptr_t>(chain + 0x38);
	
	if (chain != 0)
		mem->write<int>(chain + 0x58, speed);
}
