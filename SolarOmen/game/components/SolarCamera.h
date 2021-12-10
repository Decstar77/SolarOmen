#pragma once
#include "core/SolarCore.h"
#include "SimpleColliders.h"
#include "core/SolarPlatform.h"

namespace cm
{
	struct Camera
	{
		Transform transform;

		// @NOTE: Temporary
		real32 pitch;
		real32 yaw;

		real32 far_;
		real32 near_;
		real32 yfov;
		real32 aspect;

		Mat4f GetViewMatrix() const;
		Mat4f GetProjectionMatrix() const;
		Ray ShootRayFromScreen() const;
	};

	struct CameraComponent
	{
		bool32 active;

		real32 pitch;
		real32 yaw;

		real32 far_;
		real32 near_;
		real32 yfov;
		real32 aspect;

		Camera ToPureCamera(const Transform& transform);
		Ray ShootRayFromScreen(PlatformState* ws, const Vec2f& pixl_point, const Transform& worldTransform);
	};

}