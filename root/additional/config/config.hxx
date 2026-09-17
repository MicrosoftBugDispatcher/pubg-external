#pragma once
#include <string>

namespace additional
{
	namespace config
	{
		inline std::wstring binary = L"TslGame.exe";

		namespace aimbot
		{
			inline bool enabled = { true };
			inline int bind_key = VK_RBUTTON; // Right mouse button

			// FOV & Display
			inline bool show_fov = { false };
			inline float field_of_view = { 500.0f };

			// Smoothing
			inline float smooth_x = { 0.15f };
			inline float smooth_y = { 0.15f };
			inline float deadzone = { 2.0f };

			// Targeting
			inline int targeting_mode = { 0 }; // 0=Crosshair, 1=Distance
			inline int aim_bone = { 0 }; // Head

			// Distance
			inline float max_distance = { 5000.0f };
			inline float min_distance = { 0.0f };

			// Filters
			inline bool target_downed = { false };
			inline bool target_teammates = { false };
		}

		namespace visuals
		{
			inline bool enabled = { true };

			// Box
			inline bool box = { true };
			inline float box_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Skeleton
			inline bool skeleton = { false };
			inline float skeleton_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Player Name
			inline bool name = { true };
			inline float name_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Health
			inline bool health_bar = { true };

			// Distance
			inline bool distance = { true };
			inline float distance_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
	}
}

using namespace additional;