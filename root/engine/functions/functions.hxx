#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "../../driver/athena/hypervisor/hvre.hpp"
#include "../offsets/offsets.hxx"
#include "../math/math.hxx"
#include "../xenuine.hxx"
#include "../../additional/logger/logger.cuh"

namespace engine::functions
{
	struct player_t
	{
		uint64_t actor = 0;
		uint64_t pawn = 0;
		math::vec3_t position = {0, 0, 0};
		float health = 0;
		int team_id = 0;
		bool is_downed = false;
		std::string name = "";
	};

	// Helper functions for reading encrypted pointers
	inline auto read_ptr(uint64_t addr) -> uint64_t
	{
		if (!hvre::available || !addr) return 0;
		uint64_t value = 0;
		hvre::read(&value, addr, sizeof(uint64_t));
		return value;
	}

	inline auto read_enc_ptr(uint64_t addr) -> uint64_t
	{
		uint64_t enc = read_ptr(addr);
		if (!enc) return 0;
		return xenuine::decrypt_pointer(enc);
	}

	inline auto get_world() -> uint64_t
	{
		if (!hvre::available) return 0;
		uint64_t addr = hvre::target_base + offsets::UWorld;
		return read_enc_ptr(addr);
	}

	inline auto get_local_player() -> uint64_t
	{
		uint64_t world = get_world();
		if (!world) return 0;

		uint64_t game_instance = read_enc_ptr(world + offsets::GameInstance);
		if (!game_instance) return 0;

		uint64_t local_players_array_data = read_ptr(game_instance + offsets::ALocalPlayer);
		if (!local_players_array_data) return 0;

		return read_enc_ptr(local_players_array_data);
	}

	inline auto get_player_controller() -> uint64_t
	{
		uint64_t local_player = get_local_player();
		if (!local_player) return 0;

		return read_enc_ptr(local_player + offsets::PlayerController);
	}

	inline auto get_acknowledged_pawn() -> uint64_t
	{
		uint64_t player_controller = get_player_controller();
		if (!player_controller) return 0;

		return read_enc_ptr(player_controller + offsets::AcknowledgedPawn);
	}

	inline auto get_pawn_from_actor(uint64_t actor) -> uint64_t
	{
		if (!hvre::available || !actor) return 0;
		return read_enc_ptr(actor + 0x330);
	}

	inline auto get_camera_location() -> math::vec3_t
	{
		uint64_t player_controller = get_player_controller();
		if (!player_controller) return {0, 0, 0};

		uint64_t camera_manager = read_enc_ptr(player_controller + offsets::PlayerCameraManager);
		if (!camera_manager) return {0, 0, 0};

		math::vec3_t location;
		if (hvre::available)
			hvre::read(&location, camera_manager + offsets::CameraCacheLocation, sizeof(math::vec3_t));
		return location;
	}

	inline auto get_player_position(uint64_t pawn) -> math::vec3_t
	{
		if (!hvre::available || !pawn) return {0, 0, 0};

		uint64_t root_component = read_enc_ptr(pawn + offsets::RootComponent);
		if (!root_component) return {0, 0, 0};

		math::vec3_t position;
		hvre::read(&position, root_component + offsets::ComponentLocation, sizeof(math::vec3_t));
		return position;
	}

	inline auto get_player_health(uint64_t pawn) -> float
	{
		if (!hvre::available || !pawn) return 0;
		float health = 0;
		hvre::read(&health, pawn + offsets::GroggyHealth, sizeof(float));
		return health;
	}

	inline auto get_player_team_id(uint64_t pawn) -> int
	{
		if (!hvre::available || !pawn) return 0;
		int team_id = 0;
		hvre::read(&team_id, pawn + offsets::LastTeamNumber, sizeof(int));
		return team_id;
	}

	inline auto is_player_downed(uint64_t pawn) -> bool
	{
		if (!hvre::available || !pawn) return false;
		float groggy_health = 0;
		hvre::read(&groggy_health, pawn + offsets::GroggyHealth, sizeof(float));
		return groggy_health > 0 && groggy_health < 100;
	}

	inline auto get_actors() -> std::vector<uint64_t>
	{
		std::vector<uint64_t> actors;
		uint64_t world = get_world();
		if (!world) return actors;

		uint64_t level = read_enc_ptr(world + offsets::CurrentLevel);
		if (!level) return actors;

		uint64_t actors_array = read_ptr(level + offsets::AActors);
		if (!actors_array) return actors;

		uint32_t actor_count = 0;
		if (hvre::available)
			hvre::read(&actor_count, level + offsets::AActors + 0x8, sizeof(uint32_t));

		for (uint32_t i = 0; i < actor_count && i < 1000; i++)
		{
			uint64_t actor = read_enc_ptr(actors_array + (i * 0x8));
			if (actor)
				actors.push_back(actor);
		}

		return actors;
	}

	inline auto get_game_instance() -> uint64_t
	{
		uint64_t world = get_world();
		if (!world) return 0;
		return read_enc_ptr(world + offsets::GameInstance);
	}

	// Initialize Xenuine decryptor
	inline auto initialize_xenuine() -> bool
	{
		if (!hvre::available) return false;

		uint64_t xenuine_fn = 0;
		hvre::read(&xenuine_fn, hvre::target_base + offsets::XenuineDecrypt, sizeof(uint64_t));
		if (!xenuine_fn) return false;

		uint8_t buf[80];
		hvre::read(buf, xenuine_fn, sizeof(buf));

		return xenuine::setup(xenuine_fn, buf);
	}
}
