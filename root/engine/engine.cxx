#include "engine.hxx"
#include "../additional/config/config.hxx"
#include "../additional/logger/logger.cuh"
#include "../driver/athena/hypervisor/hvre.hpp"
#include "../features/aimbot/aimbot.hxx"
#include "../features/visuals/visuals.hxx"
#include "../overlay/hijack.hxx"

auto engine::c_initializer::threads( ) -> void
{
	std::thread( [] ( ) {
		while ( true )
		{
			features::aimbot::hook( );
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	} ).detach( );
}

auto engine::c_initializer::run( ) -> void
{
	logger->setup( "pubg-external" );

	logger->print( "initializing pubg-external..." );

	// Initialize hypervisor
	logger->print( "initializing hypervisor..." );
	if (!hvre::initialize())
	{
		logger->print( "hypervisor -> not found" );
		return;
	}
	logger->print( "hypervisor -> initialized" );

	logger->print( "waiting for TslGame.exe..." );
	while (true)
	{
		if (hvre::bind("TslGame.exe"))
		{
			logger->print( "attached to TslGame.exe" );
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	logger->print( "base address -> 0x%llx" , hvre::target_base );
	logger->print( "cr3 -> 0x%llx" , hvre::target_cr3 );

	logger->print( "initializing xen decryptor..." );
	if (!engine::functions::initialize_xenuine())
	{
		logger->print( "xen decryptor -> failed" );
		return;
	}
	logger->print( "xen decryptor -> initialized" );

	logger->print( "starting threads..." );
	initializer->threads();

	logger->print( "waiting for game window..." );
	HWND game_wnd = nullptr;
	while (true)
	{
		game_wnd = ::FindWindowA(nullptr, "PUBG: BATTLEGROUNDS");
		if (game_wnd && ::IsWindow(game_wnd))
		{
			logger->print( "game window found" );
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	auto discord = std::make_shared<c_discord>();
	if (!discord->hook(game_wnd))
	{
		logger->print( "discord -> hook failed" );
		return;
	}

	logger->print( "pubg-external initialization complete" );
}
