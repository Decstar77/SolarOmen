#pragma once
#include "core/SolarCore.h"

namespace cm
{
#define GetInput() Input *input = Input::Get()

	class Input
	{
	public:
		Input* old_input;

		Vec2f mousePositionPixelCoords;
		Vec2f mouse_norm;

		Vec2f mouseDelta;

		bool32 mouse_locked;

		bool32 mb1;
		bool32 mb2;
		bool32 mb3;

		bool32 alt;
		bool32 shift;
		bool32 ctrl;

		union
		{
			bool32 buttons[100];
			struct
			{
				bool32 w;
				bool32 s;
				bool32 a;
				bool32 d;
				bool32 q;
				bool32 e;
				bool32 r;
				bool32 t;
				bool32 z;
				bool32 x;
				bool32 c;
				bool32 v;
				bool32 b;
				bool32 del;
				bool32 tlda;
				bool32 K1;
				bool32 K2;
				bool32 K3;
				bool32 K4;
				bool32 K5;
				bool32 K6;
				bool32 K7;
				bool32 K8;
				bool32 K9;
				bool32 K0;
				bool32 f1;
				bool32 f2;
				bool32 f3;
				bool32 f4;
				bool32 f5;
				bool32 f6;
				bool32 f7;
				bool32 f8;
				bool32 f9;
				bool32 f10;
				bool32 f11;
				bool32 f12;
				bool32 escape;
				bool32 space;
			};
		};

		inline static Input* Get()
		{
			return current;
		}

		inline static void Initialize(Input* newInput)
		{
			current = newInput;
		}

	private:
		inline static Input* current = nullptr;
	};

#define IsKeyJustDown(input, key) (input->##key && !input->old_input->##key)
#define IsKeyJustUp(input, key) (!input->##key && input->old_input->##key)

	inline bool32 IsAnyKeyJustDown(Input* input)
	{
		bool32 result = (
			IsKeyJustDown(input, mb1) ||
			IsKeyJustDown(input, mb2) ||
			IsKeyJustDown(input, mb3) ||
			IsKeyJustDown(input, alt) ||
			IsKeyJustDown(input, shift) ||
			IsKeyJustDown(input, ctrl));

		for (int32 i = 0; i < ArrayCount(input->buttons) && !result; i++)
		{
			result = result || IsKeyJustDown(input, buttons[i]);
		}

		return result;
	}

	inline bool32 IsAnyKeyJustUp(Input* input)
	{
		bool32 result = (
			IsKeyJustUp(input, mb1) ||
			IsKeyJustUp(input, mb2) ||
			IsKeyJustUp(input, mb3) ||
			IsKeyJustUp(input, alt) ||
			IsKeyJustUp(input, shift) ||
			IsKeyJustUp(input, ctrl));

		for (int32 i = 0; i < ArrayCount(input->buttons) && !result; i++)
		{
			result = result || IsKeyJustUp(input, buttons[i]);
		}

		return result;
	}
}
