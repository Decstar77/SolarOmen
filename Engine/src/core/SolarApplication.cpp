#include "SolarApplication.h"
#include "SolarInput.h"
#include "SolarGameTypes.h"
#include "platform/SolarPlatform.h"
#include "renderer/RendererFrontEnd.h"
#include "core/SolarClock.h"
#include "core/SolarLogging.h"

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
				if (Renderer::Initialize())
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
			Clock clock = { };
			clock.Start();

			if (game->Update(game, 0))
			{
				Renderer::Render(nullptr);
				game->Render(game, 0);
			}
			else
			{
				Platform::Quit();
			}

			clock.Update();
			//SOLTRACE(String("Dt: ").Add((real32)clock.elapsed).GetCStr())
		}

		return 1;
	}

	void Application::Shutdown()
	{
		Renderer::Shutdown();
		Platform::Shutdown();
		Input::Shutdown();
	}
}