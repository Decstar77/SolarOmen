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

			engine->setSoundVolume(0.1f);

			return true;
		}

		void Shutdown()
		{
			engine->drop();
		}
	}

	CString LoadAudio(const CString& path)
	{
		CString name = Util::StripFilePathAndExtentions(path);
		PlatformFile file = Platform::LoadEntireFile(path, false);

		ISoundSource* sound = engine->addSoundSourceFromMemory(file.data, (uint32)file.sizeBytes, name.GetCStr());

		return CString();
	}

	void PlaySound(const CString& name, bool32 loop)
	{
		engine->play2D(name.GetCStr(), loop);
		//engine->play2D(path.GetCStr(), loop);
	}
}
