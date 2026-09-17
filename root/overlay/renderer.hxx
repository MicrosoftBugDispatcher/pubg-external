#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <memory>

class c_renderer
{
public:
	ID3D11Device* device{ };
	ID3D11DeviceContext* device_ctx{ };
	IDXGISwapChain* swap_chain{ };
	ID3D11RenderTargetView* render_target{ };
	HWND hwnd{ };

	auto setup( HWND window ) -> bool;
	auto render( ) -> bool;
	auto shutdown( ) -> void;

private:
	auto init_imgui( ) -> bool;
	auto draw_frame( ) -> void;
	auto draw_esp( ) -> void;
};
