#include "SolarAudio.h"
#include "vendor/irrklang/include/irrKlang.h"

using namespace irrklang;

namespace cm
{
	static ISoundEngine* engine = nullptr;
	void PlaySomeAudio()
	{
		std::string path = "../Assets/Raw/Audio/GamingMusicCollection/SciFi/04_Genom.WAV";

		engine = createIrrKlangDevice();

		if (!engine)
		{
			Assert(false, "Could not start audio engine");
			return;
		}

		// play some sound stream, looped
		//engine->play2D(path.c_str(), true);
	}

	void PlayWalkSound()
	{
		std::string path = "../Assets/Raw/Audio/Footsteps_Casual_Concrete_02.wav";
		if (engine)
		{
			engine->play2D(path.c_str(), false);
		}
	}

	void Shutdown()
	{
		//engine->drop(); // delete engine
		//return 0;
	}

}
