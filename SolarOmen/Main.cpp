

#include "core/SolarPlatform.h"
#include "core/SolarRenderer.h"
#include "core/SolarGame.h"
#include "core/SolarEditor.h"

#include "game/TankGame.h"

using namespace cm;

int main(int argc, const char* argv[])
{
	if (Platform::AllocateMemory(Gigabytes(1), Gigabytes(2)))
	{
		PlatformState* platformState = GameMemory::PushPermanentStruct<PlatformState>();
		//if (Platform::Initialize(platformState, "Solar omen", 1900, 1000, true))
		if (Platform::Initialize(platformState, "Solar omen", 800, 600, true))
		{
			Platform::IntializeThreads();
			Platform::IntializeNetworking();
			Audio::Initialize();

#if DEBUG
			Debug::Initialize(argc, argv);
#endif
			// @NOTE: Load the assets
			if (Assets::Initialize())
			{
				// @NOTE: Create the renderer
				if (Renderer::Initialize())
				{
					if (Game::Initialize())
					{
#if EDITOR
						if (Editor::Initialize())
#endif
						{

							// @NOTE: Initialize input structs
							Input oldInput = {};
							Input newInput = {};
							newInput.old_input = &oldInput;
							Input::Initialize(&newInput);

							EntityRenderGroup* renderGroupA = GameMemory::PushPermanentStruct<EntityRenderGroup>();
							EntityRenderGroup* renderGroupB = GameMemory::PushPermanentStruct<EntityRenderGroup>();

							EntityRenderGroup* renderGroupGame = renderGroupA;
							EntityRenderGroup* renderGroupRender = renderGroupB;

							Clock clock = {};
							real32 dt = 0.016f;
							while (Platform::ProcessInput(platformState, &newInput))
							{
								clock.Start();

								if (newInput.escape)
								{
									Platform::PostQuitMessage();
								}


#if EDITOR							
								Game::ConstructRenderGroup(renderGroupA);
								Editor::UpdateEditor(renderGroupA, dt);
								Renderer::RenderGame(renderGroupA);
								Editor::RenderEditor();
#else
								Game::UpdateGame(dt);
								Game::ConstructRenderGroup(renderGroupA);
								Renderer::RenderGame(renderGroupA);
#endif

								Renderer::PresentFrame();

								oldInput = newInput;
								ZeroStruct(renderGroupA);
								GameMemory::ReleaseAllTransientMemory();

								clock.End();
								dt = clock.Get().delta_milliseconds / 1000.f;
							}
#if EDITOR
							Editor::Shutdown();
#endif
						}

						Game::Shutdown();
						}

					Renderer::Shutdown();
					}

				Assets::Shutdown();
				}

#if DEBUG
			Debug::Shutdown();
#endif

			Audio::Shutdown();
			Platform::ShutdownNetworking();
			Platform::ShutdownThreads();
			Platform::Shutdown();
			}
		}
	else
	{
		Platform::DisplayError("Could not allocate the game memory, minimun required 4GB RAM");
	}

	}