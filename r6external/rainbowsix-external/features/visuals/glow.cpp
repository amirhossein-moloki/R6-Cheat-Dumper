#include "visuals.hpp"
#include "../../core/cheat_context.hpp"

void visuals::glow(float red, float green, float blue, float alpha, float opacity, float distance) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.glow_manager + 0xB8);
	
	if (chain != 0) {
		mem->write<float>(chain + 0xD0, red);
		mem->write<float>(chain + 0xD4, green);
		mem->write<float>(chain + 0xD8, blue);
		mem->write<float>(chain + 0x118, alpha);
		mem->write<float>(chain + 0x11C, opacity);
		mem->write<float>(chain + 0x110, distance); // other
		mem->write<float>(chain + 0x110 + 0x4, 1.f); // local
	}
}
