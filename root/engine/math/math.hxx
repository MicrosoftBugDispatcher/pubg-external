#pragma once
#include <cstdint>
#include <cmath>

namespace engine::math
{
	struct vec2_t
	{
		float x, y;
	};

	struct vec3_t
	{
		float x, y, z;

		vec3_t operator-(const vec3_t& other) const
		{
			return {x - other.x, y - other.y, z - other.z};
		}

		vec3_t operator+(const vec3_t& other) const
		{
			return {x + other.x, y + other.y, z + other.z};
		}

		vec3_t operator*(float scalar) const
		{
			return {x * scalar, y * scalar, z * scalar};
		}
	};

	struct vec4_t
	{
		float x, y, z, w;
	};

	inline auto distance(const vec3_t& a, const vec3_t& b) -> float
	{
		return static_cast<float>(std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2)));
	}

	inline auto normalize(const vec3_t& v) -> vec3_t
	{
		float len = static_cast<float>(std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
		if (len == 0) return {0, 0, 0};
		return {v.x / len, v.y / len, v.z / len};
	}

	inline auto to_screen(const vec3_t& world_pos, const vec3_t& camera_pos, float fov, int screen_width, int screen_height) -> vec2_t
	{
		vec3_t delta = world_pos - camera_pos;
		float distance = static_cast<float>(std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z));

		if (distance <= 0) return {0, 0};

		// Simple perspective projection
		float scale = fov / distance;
		float screen_x = screen_width / 2.0f + delta.x * scale;
		float screen_y = screen_height / 2.0f - delta.y * scale;

		return {screen_x, screen_y};
	}

	inline auto calc_angle(const vec3_t& from, const vec3_t& to) -> vec3_t
	{
		vec3_t delta = to - from;
		float distance = static_cast<float>(std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z));

		if (distance == 0) return {0, 0, 0};

		vec3_t angles;
		angles.y = static_cast<float>(std::atan2(delta.y, delta.x)) * 180.0f / 3.14159265f;
		angles.x = static_cast<float>(std::atan2(delta.z, std::sqrt(delta.x * delta.x + delta.y * delta.y))) * 180.0f / 3.14159265f;
		angles.z = 0;

		return angles;
	}
}
