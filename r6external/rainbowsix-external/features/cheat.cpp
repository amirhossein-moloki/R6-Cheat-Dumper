#include "cheat.hpp"

#include <iostream>

#include "../config.hpp"
#include "../core/cheat_context.hpp"
#include "../game/game_util.h"
#include "combat/combat.hpp"
#include "exploits/exploits.hpp"
#include "movement/movement.hpp"
#include "visuals/visuals.hpp"

void cheat::run() {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	if (mem->has_write_access()) {
		//combat::aimbot();
		combat::norecoil(1, 0.25f, 0.25f, 0.15f);
		combat::rapid_fire(true);

		//movement::noclip(0.f);
		//movement::speed(int(105 * options::speed_multiplier));
		//movement::speed(155);

		//exploits::unlock_all(true);
		exploits::remove_animations(cheat_config::no_animations ? 0 : 16777216);

		//visuals::player_fov(options::player_fov);
		//visuals::gunmodel_fov(options::gunmodel_fov + 0.5f);

		/*static bool thirdperson = false;
		if (GetAsyncKeyState(0x58) & 1) { // very fun feature
			thirdperson = !thirdperson;
			if (thirdperson)
				visuals::thirdperson(0);
			else
				visuals::thirdperson(1028443341);
		}*/
		//visuals::gravity(true);
		//visuals::noflash(options::noflash_enabled ? 0 : 1);
		if (game::in_match())
			visuals::glow(cheat_config::glow_red, cheat_config::glow_green, cheat_config::glow_blue, cheat_config::glow_alpha, cheat_config::glow_opacity, cheat_config::glow_distance);
		else
			visuals::glow(0.4078431427f, 0.2980392277f, 0.1960784346f, 1.f, 7.f, 0.f);
		visuals::spotted(true);
	}
}

void cheat::restore() {
    auto ctx = core::CheatContext::get_instance();
    auto mem = ctx->get_memory_service();

	if (mem && mem->has_write_access()) {
		combat::norecoil(1, 0.75f, 0.75f, 1.f);
		combat::rapid_fire(false); // idk why i do this considering its absolutely pointless

		//movement::noclip(0.0001788139343f);
		//movement::speed(105); // no idea what default speed is

		//exploits::freezelobby(0); // hope to not fuck up shit but fucks up more shit by setting it to 0 when not needed
		exploits::remove_animations(16777216);

		//visuals::player_fov(1.570796371f);
		//visuals::gunmodel_fov(0.8726646304f);

		//visuals::gravity(false);
		//visuals::noflash(1);
		visuals::glow(0.4078431427f, 0.2980392277f, 0.1960784346f, 1.f, 7.f, 0.f);
		visuals::spotted(false);
	}
}
