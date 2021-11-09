#pragma once

#include "core/SolarCore.h"

namespace cm
{
	class NodeWindow
	{
	public:
		static void Initialize();
		static void Shutdown();

	public:
		void Show();
	};
}