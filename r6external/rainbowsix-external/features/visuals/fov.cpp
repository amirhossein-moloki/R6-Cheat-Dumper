#include "visuals.hpp"
#include "../../core/cheat_context.hpp"

void visuals::player_fov(float fov) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.fov_manager + 0x28);
	chain = mem->read<uintptr_t>(chain + 0x0);

	if (chain != 0)
		mem->write<float>(chain + 0x38, fov);
}

void visuals::gunmodel_fov(float fov) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->addresses.fov_manager + 0x28);
	chain = mem->read<uintptr_t>(chain + 0x0);

	if (chain != 0)
		mem->write<float>(chain + 0x3C, fov);
}
