#include "RayTracingCamera.h"

namespace sol
{
	static Vec3d RandomPointInUnitDisk()
	{
		while (true)
		{
			auto p = Vec3d(RandomReal64(-1, 1), RandomReal64(-1, 1), 0);
			if (MagSqrd(p) >= 1) continue;
			return p;
		}
	}

	void RayTracingCamera::Initialize(Vec3d lookfrom, Vec3d lookat, Vec3d vup, real64 vfov, real64 aspect_ratio, real64 aperture, real64 focus_dist)
	{
		real64 theta = DegToRad(vfov);
		real64 h = tan(theta / 2.0);
		real64 viewport_height = 2.0 * h;
		real64 viewport_width = aspect_ratio * viewport_height;

		w = Normalize(lookfrom - lookat);
		u = Normalize(Cross(vup, w));
		v = Cross(w, u);

		origin = lookfrom;
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - focus_dist * w;

		lens_radius = aperture / 2.0;

		time0 = 0.0;
		time1 = 0.0;
	}

	RayTracingRay RayTracingCamera::GetRay(real64 s, real64 t) const
	{
		Vec3d rd = lens_radius * RandomPointInUnitDisk();
		Vec3d offset = u * rd.x + v * rd.y;

		return RayTracingRay::Create(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
	}
}