#include "movement.hpp"
#include "../../core/cheat_context.hpp"
#include "../../offsets.hpp"

void movement::noclip(float value) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.network_manager + 0xF8);
	chain = mem->read<uintptr_t>(chain + 0x8);

	if (chain != 0) { // todo read this shit normally so i can unset it
		mem->write<float>(chain + 0x7F0, -1.0f);
		mem->write<float>(chain + 0x7F4, -1.0f);
		mem->write<float>(chain + 0x7F8, -1.0f);
		mem->write<float>(chain + 0x7FC, -1.0f);
	}
}


void movement::nocollision(float value) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.network_manager + 0xF8);
	chain = mem->read<uintptr_t>(chain + 0x8);

	if (chain != 0)
		mem->write<float>(chain + 540, value);
}
