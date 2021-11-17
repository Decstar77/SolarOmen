#include "SolarCamera.h"


namespace cm
{
	Mat4f Camera::GetViewMatrix() const
	{
		Mat4f view = Inverse(transform.CalculateTransformMatrix());

		return view;
	}

	Mat4f Camera::GetProjectionMatrix() const
	{
		Mat4f projection = PerspectiveLH(DegToRad(yfov), aspect, near_, far_);

		return projection;
	}

	Ray Camera::ShootRayFromScreen(PlatformState* ws, const Vec2f& pixl_point, const Transform& worldTransform) const
	{
		real32 aspect = (real32)ws->client_width / (real32)ws->client_height;
		Mat4f proj = PerspectiveLH(DegToRad(yfov), aspect, near_, far_);
		Mat4f view = Inverse(worldTransform.CalculateTransformMatrix());

		Vec4f normal_coords = GetNormalisedDeviceCoordinates((real32)ws->client_width,
			(real32)ws->client_height, pixl_point.x, pixl_point.y);

		Vec4f view_coords = ToViewCoords(proj, normal_coords);

		// @NOTE: This 1 ensure we a have something pointing in to the screen
		view_coords = Vec4f(view_coords.x, view_coords.y, 1, 0);
		Vec3f world_coords = ToWorldCoords(view, view_coords);

		Ray ray = {};
		ray.origin = worldTransform.position;
		ray.direction = Normalize(Vec3f(world_coords.x, world_coords.y, world_coords.z));

		return ray;
	}

	Camera CameraComponent::ToPureCamera(const Transform& transform)
	{
		Camera result = {};
		result.transform = transform;

		result.pitch = this->pitch;
		result.yaw = this->yaw;
		result.far_ = this->far_;
		result.near_ = this->near_;
		result.yfov = this->yfov;
		result.aspect = this->aspect;

		return result;
	}

	Ray CameraComponent::ShootRayFromScreen(PlatformState* ws, const Vec2f& pixl_point, const Transform& worldTransform)
	{
		real32 aspect = (real32)ws->client_width / (real32)ws->client_height;
		Mat4f proj = PerspectiveLH(DegToRad(yfov), aspect, near_, far_);
		Mat4f view = Inverse(worldTransform.CalculateTransformMatrix());

		Vec4f normal_coords = GetNormalisedDeviceCoordinates((real32)ws->client_width,
			(real32)ws->client_height, pixl_point.x, pixl_point.y);

		Vec4f view_coords = ToViewCoords(proj, normal_coords);

		// @NOTE: This 1 ensure we a have something pointing in to the screen
		view_coords = Vec4f(view_coords.x, view_coords.y, 1, 0);
		Vec3f world_coords = ToWorldCoords(view, view_coords);

		Ray ray = {};
		ray.origin = worldTransform.position;
		ray.direction = Normalize(Vec3f(world_coords.x, world_coords.y, world_coords.z));

		return ray;
	}

}