#include "engine/engine.hxx"
#include "additional/logger/logger.cuh"
#include <iostream>

auto main( int argv[], char* argvc[] ) -> int
{
	engine::initializer->run( );
	std::cin.get( );
}
