#include "renderer.hxx"
#include "../additional/logger/logger.cuh"
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"
#include "../dependencies/imgui/imgui_impl_dx11.h"
#include "../features/visuals/visuals.hxx"
#include "../additional/config/config.hxx"

auto c_renderer::setup( HWND window ) -> bool
{
	if ( !window || !::IsWindow( window ) )
	{
		logger->print( "renderer -> invalid window" );
		return false;
	}

	hwnd = window;

	RECT rc{ };
	::GetClientRect( hwnd , &rc );
	const UINT w = rc.right > 0 ? static_cast<UINT>( rc.right ) : static_cast<UINT>( ::GetSystemMetrics( SM_CXSCREEN ) );
	const UINT h = rc.bottom > 0 ? static_cast<UINT>( rc.bottom ) : static_cast<UINT>( ::GetSystemMetrics( SM_CYSCREEN ) );

	DXGI_SWAP_CHAIN_DESC desc{ };
	desc.BufferCount = 2;
	desc.BufferDesc.Width = w;
	desc.BufferDesc.Height = h;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = hwnd;
	desc.SampleDesc.Count = 1;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL feat_level;
	const D3D_FEATURE_LEVEL feat_arr[ 1 ] = { D3D_FEATURE_LEVEL_11_0 };
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
		feat_arr , 1 , D3D11_SDK_VERSION , &desc ,
		&swap_chain , &device , &feat_level , &device_ctx );

	if ( FAILED( hr ) || !swap_chain || !device || !device_ctx )
	{
		logger->print( "renderer -> d3d11 create failed" );
		return false;
	}

	ID3D11Texture2D* back_buf = nullptr;
	swap_chain->GetBuffer( 0 , IID_PPV_ARGS( &back_buf ) );
	if ( !back_buf )
	{
		logger->print( "renderer -> back buffer failed" );
		return false;
	}

	device->CreateRenderTargetView( back_buf , nullptr , &render_target );
	back_buf->Release( );
	if ( !render_target )
	{
		logger->print( "renderer -> render target failed" );
		return false;
	}

	if ( !init_imgui( ) )
	{
		logger->print( "renderer -> imgui init failed" );
		return false;
	}

	logger->print( "renderer -> initialized" );
	return true;
}

auto c_renderer::init_imgui( ) -> bool
{
	IMGUI_CHECKVERSION( );
	ImGui::CreateContext( );
	ImGuiIO& io = ImGui::GetIO( );
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	RECT rc{ };
	::GetClientRect( hwnd , &rc );
	io.DisplaySize = ImVec2( static_cast<float>( rc.right ) , static_cast<float>( rc.bottom ) );

	ImGuiStyle& style = ImGui::GetStyle( );
	style.AntiAliasedLines = false;
	style.AntiAliasedLinesUseTex = false;
	style.AntiAliasedFill = false;

	ImGui_ImplWin32_Init( hwnd );
	ImGui_ImplDX11_Init( device , device_ctx );

	io.Fonts->AddFontDefault( );

	return true;
}

auto c_renderer::render( ) -> bool
{
	MSG msg = { };
	while ( msg.message != WM_QUIT )
	{
		while ( ::PeekMessageA( &msg , nullptr , 0 , 0 , PM_REMOVE ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
			if ( msg.message == WM_QUIT )
				break;
		}
		if ( msg.message == WM_QUIT )
			break;

		ImGuiIO& io = ImGui::GetIO( );
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		::GetCursorPos( &p );
		io.MousePos.x = static_cast<float>( p.x );
		io.MousePos.y = static_cast<float>( p.y );
		io.MouseDown[ 0 ] = ( ::GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;

		draw_frame( );
		swap_chain->Present( 1 , 0 );
	}

	return true;
}

auto c_renderer::draw_frame( ) -> void
{
	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ImGui::NewFrame( );

	draw_esp( );

	ImGui::Render( );
	device_ctx->OMSetRenderTargets( 1 , &render_target , nullptr );
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
}

auto c_renderer::draw_esp( ) -> void
{
	ImGuiIO& io = ImGui::GetIO( );
	int screen_width = static_cast<int>( io.DisplaySize.x );
	int screen_height = static_cast<int>( io.DisplaySize.y );

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList( );
	features::visuals::draw_esp( draw_list , screen_width , screen_height );
}

auto c_renderer::shutdown( ) -> void
{
	if ( render_target )
	{
		render_target->Release( );
		render_target = nullptr;
	}
	if ( swap_chain )
	{
		swap_chain->Release( );
		swap_chain = nullptr;
	}
	if ( device_ctx )
	{
		device_ctx->Release( );
		device_ctx = nullptr;
	}
	if ( device )
	{
		device->Release( );
		device = nullptr;
	}

	ImGui_ImplDX11_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );
}
