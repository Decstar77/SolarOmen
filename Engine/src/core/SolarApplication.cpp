#include "SolarApplication.h"
#include "SolarInput.h"
#include "game/SolarGameTypes.h"
#include "platform/SolarPlatform.h"
#include "renderer/SolarRenderer.h"
#include "core/SolarClock.h"
#include "core/SolarLogging.h"
#include "core/SolarEvent.h"

#if SOL_DEBUG_RENDERING
#include "core/SolarDebug.h"
#endif

namespace sol
{
	bool8 Application::Initialize(Game* game)
	{
#if SOL_DEBUG_RENDERING
		if (!Debug::Initialize()) { SOLFATAL("Debug failed to initialize"); return false; }
#endif

		if (EventSystemInitialize())
		{
			if (Input::Initailize())
			{
				if (Platform::Intialize(
					game->appConfig.startPosX, game->appConfig.startPosY,
					game->appConfig.startWidth, game->appConfig.startHeight))
				{
					if (Resources::Initialize())
					{
						if (Renderer::Initialize())
						{
							if (game->Initialize(game))
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
		RenderPacket* packet = GameMemory::PushPermanentStruct<RenderPacket>();
		while (Platform::PumpMessages())
		{
			Clock clock = { };
			clock.Start();

			GameMemory::ZeroStruct(packet);
			if (game->Update(game, packet, dt))
			{
				Renderer::Render(packet);
			}
			else
			{
				Platform::Quit();
			}

			GameMemory::ReleaseAllTransientMemory();
			clock.Update();
			dt = (real32)clock.elapsed;
			//SOLTRACE(String("Dt: ").Add((real32)clock.elapsed * 1000.0f).GetCStr());
		}

		return 1;
	}

	void Application::Shutdown()
	{
		Renderer::Shutdown();
		Platform::Shutdown();
		Input::Shutdown();
	}

	real32 Application::GetDeltaTime()
	{
		return dt;
	}

	uint32 Application::GetSurfaceWidth()
	{
		return Platform::GetSurfaceWidth();
	}

	uint32 Application::GetSurfaceHeight()
	{
		return Platform::GetSurfaceHeight();
	}

	real32 Application::GetSurfaceAspectRatio()
	{
		real32 w = (real32)Platform::GetSurfaceWidth();
		real32 h = (real32)Platform::GetSurfaceHeight();
		real32 aspect = w / h;

		return aspect;
	}
}