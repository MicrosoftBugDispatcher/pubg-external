#include "hijack.hxx"
#include "renderer.hxx"
#include "../additional/logger/logger.cuh"

auto c_discord::hook( HWND game ) -> bool
{
	if ( !game || !::IsWindow( game ) )
	{
		logger->print( "discord -> wrong window" );
		return false;
	}

	const int screen_w = ::GetSystemMetrics( SM_CXSCREEN );
	const int screen_h = ::GetSystemMetrics( SM_CYSCREEN );

	HWND hwnd = ::FindWindowA("Chrome_WidgetWin_1", "Discord Overlay");
	if ( !hwnd )
	{
		logger->print( "discord -> overlay not found" );
		return false;
	}

	m_hwnd = hwnd;

	::MoveWindow( m_hwnd , 0 , 0 , screen_w , screen_h , TRUE );
	::SetWindowPos( m_hwnd , HWND_TOPMOST , 0 , 0 , screen_w , screen_h ,
		SWP_NOACTIVATE | SWP_SHOWWINDOW );
	::ShowWindow( m_hwnd , SW_SHOWNOACTIVATE );
	::UpdateWindow( m_hwnd );

	window_handle = m_hwnd;
	game_hwnd = game;

	renderer = std::make_shared<c_renderer>();
	if ( !renderer->setup( m_hwnd ) )
	{
		logger->print( "discord -> renderer setup failed" );
		return false;
	}

	logger->print( "discord -> hooked" );
	renderer->render( );
	return true;
}
