#include "SolarApplication.h"
#include "SolarInput.h"
#include "SolarGameTypes.h"
#include "platform/SolarPlatform.h"

namespace sol
{
	bool8 Application::Initialize(Game* game)
	{
		if (Input::Initailize())
		{
			if (Platform::Intialize(
				game->appConfig.startPosX, game->appConfig.startPosY,
				game->appConfig.startWidth, game->appConfig.startHeight))
			{
				return true;
			}
			else
			{

			}
		}
		else
		{

		}

		return false;
	}

	bool8 Application::Run(Game* game)
	{
		while (Platform::PumpMessages())
		{
			if (game->Update(game, 0))
			{
				game->Render(game, 0);
			}
			else
			{
				Platform::Quit();
			}
		}

		return 1;
	}

	void Application::Shutdown()
	{
	}
}