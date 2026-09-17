#include "visuals.hxx"
#include "../../dependencies/imgui/imgui.h"

namespace features::visuals
{
	auto get_players( ) -> std::vector<player_info_t>
	{
		std::vector<player_info_t> players;

		uint64_t local_pawn = engine::functions::get_acknowledged_pawn();
		if (!local_pawn) return players;

		int local_team = engine::functions::get_player_team_id(local_pawn);
		engine::math::vec3_t local_pos = engine::functions::get_player_position(local_pawn);

		auto actors = engine::functions::get_actors();
		for (auto actor : actors)
		{
			if (!actor) continue;

			uint64_t pawn = engine::functions::get_pawn_from_actor(actor);
			if (!pawn || pawn == local_pawn) continue;

			int team_id = engine::functions::get_player_team_id(pawn);
			engine::math::vec3_t position = engine::functions::get_player_position(pawn);
			float health = engine::functions::get_player_health(pawn);
			bool is_downed = engine::functions::is_player_downed(pawn);

			player_info_t info;
			info.actor = actor;
			info.pawn = pawn;
			info.position_x = position.x;
			info.position_y = position.y;
			info.position_z = position.z;
			info.health = health;
			info.team_id = team_id;
			info.is_downed = is_downed;
			info.name = "Player";

			players.push_back(info);
		}

		return players;
	}

	auto draw_esp( ImDrawList* draw_list , int screen_width , int screen_height ) -> void
	{
		if ( !config::visuals::enabled )
			return;

		auto players = get_players( );
		if ( players.empty( ) )
			return;

		uint64_t local_pawn = engine::functions::get_acknowledged_pawn( );
		if ( !local_pawn )
			return;

		engine::math::vec3_t camera_pos = engine::functions::get_camera_location( );
		int local_team = engine::functions::get_player_team_id( local_pawn );

		for ( const auto& player : players )
		{
			if ( player.team_id == local_team && !config::aimbot::target_teammates )
				continue;

			if ( player.is_downed && !config::aimbot::target_downed )
				continue;

			engine::math::vec3_t player_pos = { player.position_x , player.position_y , player.position_z };
			float dist = engine::math::distance( camera_pos , player_pos );

			if ( dist > config::aimbot::max_distance || dist < config::aimbot::min_distance )
				continue;

			engine::math::vec2_t screen_pos = engine::math::to_screen(
				player_pos ,
				camera_pos ,
				config::aimbot::field_of_view ,
				screen_width ,
				screen_height
			);

			if ( screen_pos.x < 0 || screen_pos.x > screen_width ||
				screen_pos.y < 0 || screen_pos.y > screen_height )
				continue;

		
			if ( config::visuals::box )
			{
				float box_height = 100.0f / dist * 500.0f;
				float box_width = box_height * 0.5f;
				ImVec2 box_min( screen_pos.x - box_width / 2 , screen_pos.y - box_height / 2 );
				ImVec2 box_max( screen_pos.x + box_width / 2 , screen_pos.y + box_height / 2 );

				ImU32 box_color = IM_COL32(
					static_cast<int>( config::visuals::box_color[ 0 ] * 255 ) ,
					static_cast<int>( config::visuals::box_color[ 1 ] * 255 ) ,
					static_cast<int>( config::visuals::box_color[ 2 ] * 255 ) ,
					static_cast<int>( config::visuals::box_color[ 3 ] * 255 )
				);

				draw_list->AddRect( box_min , box_max , box_color , 0.0f , 0 , 2.0f );
			}

			
			if ( config::visuals::health_bar )
			{
				float box_height = 100.0f / dist * 500.0f;
				float bar_width = 5.0f;
				float bar_height = box_height;
				float health_pct = player.health / 100.0f;

				ImVec2 bar_min( screen_pos.x - box_height * 0.3f , screen_pos.y - box_height / 2 );
				ImVec2 bar_max( bar_min.x + bar_width , bar_min.y + bar_height );

				draw_list->AddRectFilled( bar_min , bar_max , IM_COL32( 0 , 0 , 0 , 150 ) , 0.0f );

				ImVec2 health_min = bar_min;
				ImVec2 health_max( bar_min.x + bar_width , bar_min.y + bar_height * health_pct );

				ImU32 health_color = health_pct > 0.5f ? IM_COL32( 0 , 255 , 0 , 255 ) :
					health_pct > 0.25f ? IM_COL32( 255 , 255 , 0 , 255 ) : IM_COL32( 255 , 0 , 0 , 255 );

				draw_list->AddRectFilled( health_min , health_max , health_color , 0.0f );
			}

			
			if ( config::visuals::name )
			{
				float box_height = 100.0f / dist * 500.0f;
				ImVec2 text_pos( screen_pos.x , screen_pos.y - box_height / 2 - 15.0f );

				ImU32 name_color = IM_COL32(
					static_cast<int>( config::visuals::name_color[ 0 ] * 255 ) ,
					static_cast<int>( config::visuals::name_color[ 1 ] * 255 ) ,
					static_cast<int>( config::visuals::name_color[ 2 ] * 255 ) ,
					static_cast<int>( config::visuals::name_color[ 3 ] * 255 )
				);

				draw_list->AddText( text_pos , name_color , player.name.c_str( ) );
			}

			
			if ( config::visuals::distance )
			{
				float box_height = 100.0f / dist * 500.0f;
				char dist_str[ 32 ];
				sprintf_s( dist_str , "%.0fm" , dist );
				ImVec2 text_pos( screen_pos.x , screen_pos.y + box_height / 2 + 5.0f );

				ImU32 dist_color = IM_COL32(
					static_cast<int>( config::visuals::distance_color[ 0 ] * 255 ) ,
					static_cast<int>( config::visuals::distance_color[ 1 ] * 255 ) ,
					static_cast<int>( config::visuals::distance_color[ 2 ] * 255 ) ,
					static_cast<int>( config::visuals::distance_color[ 3 ] * 255 )
				);

				draw_list->AddText( text_pos , dist_color , dist_str );
			}
		}
	}
}
