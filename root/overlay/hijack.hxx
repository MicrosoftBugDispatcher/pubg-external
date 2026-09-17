#pragma once

#include <Windows.h>
#include <memory>

class c_renderer;

class c_discord
{
public:
	HWND m_hwnd{ };
	HWND window_handle{ };
	HWND game_hwnd{ };
	std::shared_ptr<c_renderer> renderer{ };

	auto hook( HWND game ) -> bool;
};
