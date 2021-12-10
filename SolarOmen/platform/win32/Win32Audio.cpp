#include "core/SolarAudio.h"
#include "vendor/irrklang/include/irrKlang.h"
using namespace irrklang;

namespace cm
{
	static ISoundEngine* engine = nullptr;

	namespace Audio
	{
		bool32 Initialize()
		{
			engine = createIrrKlangDevice();

			if (!engine)
			{
				Assert(false, "Could not start audio engine");
				return false;
			}

			return true;
		}

		void Shutdown()
		{
			engine->drop();
		}
	}

	void PlaySound(const CString& path, bool32 loop)
	{
		engine->play2D(path.GetCStr(), loop);
	}
}
