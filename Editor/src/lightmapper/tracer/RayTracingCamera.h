#pragma once
#include "RayTracingRay.h"

namespace sol
{
	class RayTracingCamera
	{
	public:
		RayTracingCamera() : lens_radius(0), time0(0), time1(0) {};

		void Initialize(Vec3d lookfrom, Vec3d lookat, Vec3d   vup, real64 vfov, real64 aspect_ratio, real64 aperture, real64 focus_dist);
		RayTracingRay GetRay(double s, double t) const;

		inline void SetTime(real64 t1, real64 t2) { time0 = t1; time1 = t2; };

	private:
		Vec3d origin;
		Vec3d lower_left_corner;
		Vec3d horizontal;
		Vec3d vertical;
		Vec3d u, v, w;
		real64 lens_radius;
		real64 time0;
		real64 time1;
	};

}