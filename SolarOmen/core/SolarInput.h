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

		bool isPS4Controller;
		real32 controllerLeftTrigger;
		real32 controllerRightTrigger;
		Vec2f controllerLeftThumbDrag;
		Vec2f controllerRightThumbDrag;

		bool mouse_locked;

		bool mb1;
		bool mb2;
		bool mb3;

		bool alt;
		bool shift;
		bool ctrl;

		union
		{
			bool buttons[100];
			struct
			{
				bool w;
				bool s;
				bool a;
				bool d;
				bool q;
				bool e;
				bool r;
				bool t;
				bool z;
				bool x;
				bool c;
				bool v;
				bool b;
				bool del;
				bool tlda;
				bool K1;
				bool K2;
				bool K3;
				bool K4;
				bool K5;
				bool K6;
				bool K7;
				bool K8;
				bool K9;
				bool K0;
				bool f1;
				bool f2;
				bool f3;
				bool f4;
				bool f5;
				bool f6;
				bool f7;
				bool f8;
				bool f9;
				bool f10;
				bool f11;
				bool f12;
				bool escape;
				bool space;

				bool controllerUp;
				bool controllerDown;
				bool controllerLeft;
				bool controllerRight;
				bool controllerStart;
				bool controllerBack;
				bool controllerLeftThumb;
				bool controllerRightThumb;
				bool controllerLeftShoulder;
				bool controllerRightShoulder;
				bool controllerA;
				bool controllerB;
				bool controllerX;
				bool controllerY;
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
