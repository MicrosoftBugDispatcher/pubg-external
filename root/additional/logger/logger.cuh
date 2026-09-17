#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <memory>
#include <windows.h>

class logger_c
{
public:
	auto setup( const char* title ) -> void
	{
		SetConsoleTitleA( title );

		HANDLE std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
		DWORD mode = 0;
		if ( GetConsoleMode( std_handle , &mode ) )
		{
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode( std_handle , mode );
		}

		CONSOLE_FONT_INFOEX cfi{ };
		cfi.cbSize = sizeof( cfi );
		cfi.nFont = 0;
		cfi.dwFontSize.X = 8;
		cfi.dwFontSize.Y = 15;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s( cfi.FaceName , L"Raster Fonts" );
		SetCurrentConsoleFontEx( std_handle , FALSE , &cfi );
	}

	template<typename... args_t>
	auto print( const char* format , args_t... args ) -> void
	{
		auto now = std::chrono::system_clock::now( );
		std::time_t time = std::chrono::system_clock::to_time_t( now );
		tm local_tm{ };
		localtime_s( &local_tm , &time );

		std::printf( "\x1b[38;2;130;90;180m[%02d/%02d/%04d %02d:%02d:%02d]\x1b[0m " ,
			local_tm.tm_mon + 1 ,
			local_tm.tm_mday ,
			local_tm.tm_year + 1900 ,
			local_tm.tm_hour ,
			local_tm.tm_min ,
			local_tm.tm_sec );

		std::printf( "\x1b[38;2;200;130;255m>\x1b[0m " );
		std::printf( "\x1b[38;2;230;215;255m" );
		std::printf( format , args... );
		std::printf( "\x1b[0m\n" );
	}
};

inline auto logger = std::make_unique<logger_c>( );
