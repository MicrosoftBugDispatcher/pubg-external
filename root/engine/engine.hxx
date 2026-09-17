#pragma once
#include "offsets/offsets.hxx"
#include "math/math.hxx"
#include "functions/functions.hxx"
#include "xenuine.hxx"
#include "../driver/driver.hxx"
#include "../additional/logger/logger.cuh"
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

namespace engine
{
	class c_initializer {
	public:
		auto run( ) -> void;
		auto threads( ) -> void;
	};
	inline c_initializer* initializer = new c_initializer( );
}
