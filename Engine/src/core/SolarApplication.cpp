#include "SolarApplication.h"
#include "SolarGameTypes.h"
#include "platform/SolarPlatform.h"

namespace sol
{
	bool8 Application::Initialize(Game* game)
	{
		if (Platform::Intialize(game->appConfig.startPosX, game->appConfig.startPosY,
			game->appConfig.startWidth, game->appConfig.startHeight))
		{

			Platform::Shutdown();
		}
		else
		{

		}
	}

	bool8 Application::Run(Game* game)
	{
		game->Update(game, 0);
		return 1;
	}

}