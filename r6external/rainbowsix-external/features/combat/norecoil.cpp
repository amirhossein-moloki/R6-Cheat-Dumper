#include "combat.hpp"
#include "../../core/cheat_context.hpp"
#include "../../offsets.hpp"

void combat::norecoil(byte recoil, float horizontal_recoil, float vertical_recoil, float spread) {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	uintptr_t chain = mem->read<uintptr_t>(ctx->get_module_base() + offsets::fov_manager); // perfect no recoil but makes it weird when aiming
	chain = mem->read<uintptr_t>(chain + 0x110);
	chain = mem->read<uintptr_t>(chain + 0x0);
	if (chain != 0)
		mem->write<byte>(chain + 0xE2D, recoil);

	chain = mem->read<uintptr_t>(ctx->addresses.game_manager + 0xC8); // no recoil that fucks up a bit vertically but feels normal when aiming
	chain = mem->read<uintptr_t>(chain + 0x0);
	chain = mem->read<uintptr_t>(chain + 0x90);
	chain = mem->read<uintptr_t>(chain + 0xC8);
	chain = mem->read<uintptr_t>(chain + 0x278);
	
	if (chain != 0) {
		mem->write<byte>(chain + 0x168, 0);
		mem->write<float>(chain + 0x14C, horizontal_recoil);
		mem->write<float>(chain + 0x15C, vertical_recoil);
		mem->write<float>(chain + 0x58, spread);
	}
}
