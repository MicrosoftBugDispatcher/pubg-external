#include "aimbot.hxx"
#include <windows.h>

namespace features::aimbot
{
	auto get_target( ) -> aimbot_target_t
	{
		aimbot_target_t best_target;
		float closest_distance = config::aimbot::max_distance;

		uint64_t local_pawn = engine::functions::get_acknowledged_pawn();
		if (!local_pawn) return best_target;

		engine::math::vec3_t local_pos = engine::functions::get_player_position(local_pawn);
		int local_team = engine::functions::get_player_team_id(local_pawn);

		auto actors = engine::functions::get_actors();
		for (auto actor : actors)
		{
			if (!actor || actor == local_pawn) continue;

			// Try to read actor as potential player pawn
			int team_id = engine::functions::get_player_team_id(actor);
			if (team_id == local_team && !config::aimbot::target_teammates) continue;

			bool is_downed = engine::functions::is_player_downed(actor);
			if (is_downed && !config::aimbot::target_downed) continue;

			engine::math::vec3_t position = engine::functions::get_player_position(actor);
			if (position.x == 0 && position.y == 0 && position.z == 0) continue; // Invalid position

			float distance = engine::math::distance(local_pos, position);

			if (distance < config::aimbot::min_distance || distance > config::aimbot::max_distance) continue;

			if (distance < closest_distance)
			{
				closest_distance = distance;
				best_target.actor = actor;
				best_target.pawn = actor;
				best_target.position = position;
				best_target.distance = distance;
			}
		}

		return best_target;
	}

	auto hook( ) -> void
	{
		if (!config::aimbot::enabled) return;

		if (!(GetAsyncKeyState(config::aimbot::bind_key) & 0x8000)) return;

		aimbot_target_t target = get_target();
		if (!target.pawn) return;

		uint64_t local_pawn = engine::functions::get_acknowledged_pawn();
		if (!local_pawn) return;

		engine::math::vec3_t local_pos = engine::functions::get_player_position(local_pawn);
		engine::math::vec3_t target_pos = target.position;

		// Calculate aim angles
		engine::math::vec3_t aim_angles = engine::math::calc_angle(local_pos, target_pos);

		// Apply smoothing
		// (Placeholder - actual mouse movement would go here)
	}
}
