#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../../engine/math/math.hxx"
#include "../../engine/functions/functions.hxx"
#include "../../driver/driver.hxx"
#include "../../additional/logger/logger.cuh"
#include "../../additional/config/config.hxx"

struct ImDrawList;

namespace features::visuals
{
	struct player_info_t
	{
		uint64_t actor = 0;
		uint64_t pawn = 0;
		float position_x = 0;
		float position_y = 0;
		float position_z = 0;
		float health = 0;
		int team_id = 0;
		bool is_downed = false;
		std::string name = "";
	};

	auto get_players( ) -> std::vector<player_info_t>;
	auto draw_esp( ImDrawList* draw_list , int screen_width , int screen_height ) -> void;
}
