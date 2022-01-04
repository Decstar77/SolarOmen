#pragma once
#include "core/SolarMemory.h"
#include "core/SolarApplication.h"
#include "core/SolarLogging.h"
#include "SolarGameTypes.h"

int main(int argc, const char* argv[])
{
	if (sol::GameMemory::Initialize(Gigabytes(1), Gigabytes(2)))
	{
		sol::Game game = {};
		if (sol::CreateGame(&game))
		{
			if (sol::Application::Initialize(&game))
			{
				if (sol::Application::Run(&game))
				{

				}
				else
				{
					SOLFATAL("Application did not run properly");
				}

				sol::Application::Shutdown();
			}
			else
			{
				SOLFATAL("Could not intialize application");
			}
		}
		else
		{
			SOLFATAL("Could not create game");
		}

		sol::GameMemory::Shutdown();
	}
	else
	{
		SOLFATAL("Could not allocate memory");
	}

	return 0;
}