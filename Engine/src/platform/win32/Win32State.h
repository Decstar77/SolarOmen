#pragma once
#include "SolarDefines.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shobjidl_core.h>
#include <Xinput.h>
#include <stdlib.h>

namespace sol
{
	struct Win32State
	{
		HWND window;
		HINSTANCE hinstance;
		bool8 running;
		bool8 active;
		bool8 rawInput;

		uint32 windowWidth;
		uint32 windowHeight;
		uint32 surfaceWidth;
		uint32 surfaceHeight;

		real64 clockFrequency;
		LARGE_INTEGER startTime;
	};

	struct Win32EventPumpMessageContext
	{
		HWND hwnd;
		UINT msg;
		WPARAM wparam;
		LPARAM lparam;
	};

}