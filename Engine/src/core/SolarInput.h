#pragma once
#include "SolarMath.h"


namespace sol
{
#define IsKeyJustDown(input, key) (input->##key && !input->oldInput->##key)
#define IsKeyJustUp(input, key) (!input->##key && input->oldInput->##key)

	struct SOL_API Input
	{
		Input* oldInput;

		Vec2f mousePositionPixelCoords;
		Vec2f mouseNorm;
		Vec2f mouseDelta;

		bool8 isPS4Controller;
		real32 controllerLeftTrigger;
		real32 controllerRightTrigger;
		Vec2f controllerLeftThumbDrag;
		Vec2f controllerRightThumbDrag;

		bool8 mouse_locked;

		bool8 mb1;
		bool8 mb2;
		bool8 mb3;

		bool8 alt;
		bool8 shift;
		bool8 ctrl;

		union
		{
			bool8 buttons[100];
			struct
			{
				bool8 w;
				bool8 s;
				bool8 a;
				bool8 d;
				bool8 q;
				bool8 e;
				bool8 r;
				bool8 t;
				bool8 z;
				bool8 x;
				bool8 c;
				bool8 v;
				bool8 b;
				bool8 del;
				bool8 tlda;
				bool8 K1;
				bool8 K2;
				bool8 K3;
				bool8 K4;
				bool8 K5;
				bool8 K6;
				bool8 K7;
				bool8 K8;
				bool8 K9;
				bool8 K0;
				bool8 f1;
				bool8 f2;
				bool8 f3;
				bool8 f4;
				bool8 f5;
				bool8 f6;
				bool8 f7;
				bool8 f8;
				bool8 f9;
				bool8 f10;
				bool8 f11;
				bool8 f12;
				bool8 escape;
				bool8 space;

				bool8 controllerUp;
				bool8 controllerDown;
				bool8 controllerLeft;
				bool8 controllerRight;
				bool8 controllerStart;
				bool8 controllerBack;
				bool8 controllerLeftThumb;
				bool8 controllerRightThumb;
				bool8 controllerLeftShoulder;
				bool8 controllerRightShoulder;
				bool8 controllerA;
				bool8 controllerB;
				bool8 controllerX;
				bool8 controllerY;
			};
		};

		static bool8 Initailize();
		static void Shutdown();
		static void Flip();
		static Input* Get();
	};
}