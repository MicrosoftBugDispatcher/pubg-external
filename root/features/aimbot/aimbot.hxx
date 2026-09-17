#pragma once
#include <cstdint>
#include "../../engine/math/math.hxx"
#include "../../engine/functions/functions.hxx"
#include "../../driver/driver.hxx"
#include "../../additional/logger/logger.cuh"
#include "../../additional/config/config.hxx"

namespace features::aimbot
{
	struct aimbot_target_t
	{
		uint64_t actor = 0;
		uint64_t pawn = 0;
		engine::math::vec3_t position = {0, 0, 0};
		float distance = 0;
	};

	auto get_target( ) -> aimbot_target_t;

	auto hook( ) -> void;
}
