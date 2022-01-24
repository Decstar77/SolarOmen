#include "LightMapping.h"
#include <algorithm>

namespace sol
{
	void ReferenceRayTracer::Initialize(uint32 samples)
	{
		*this = ReferenceRayTracer();

		imageWidth = (int32)Application::GetSurfaceWidth();
		imageHeight = (int32)Application::GetSurfaceHeight();
		aspectRatio = (real64)Application::GetSurfaceAspectRatio();

		TextureFormat format = TextureFormat::Value::R32G32B32A32_FLOAT;
		textureHandle = Renderer::CreateTexture(imageWidth, imageHeight, format, ResourceCPUFlags::Value::WRITE);

		pixels.clear();
		pixels.resize(imageWidth * imageHeight);

		pixelCaches.clear();
		pixelCaches.resize(imageWidth * imageHeight);

		pixelsProcessed = 0;

		complete = false;

		samplesPerPixel = samples;

		world.objects.Open();
		//world.MakeRandomSphereWorld(&camera, aspectRatio);
		//world.MakeTwoSphereWorld(&camera, aspectRatio);
		//world.MakeTwoPerlineSpheres(&camera, aspectRatio);
		//world.MakeTextureWorld(&camera, aspectRatio);
		//world.MakeSimpleLight(&camera, aspectRatio);
		//world.MakeBoxWorld(&camera, aspectRatio);
		world.MakeCornellBox(&camera, aspectRatio);

	}
	void ReferenceRayTracer::Shutdown()
	{
		Renderer::DestroyTexture(&textureHandle);
	}

	bool8 sol::RayTracingHittableList::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		HitRecord tempRec = {};
		bool8 hitAnything = false;
		real64 closest = tMax;

#if 1
		if (bvhTree.Raycast(r, tMin, closest, &tempRec))
		{
			hitAnything = true;
			closest = tempRec.t;
			*rec = tempRec;
		}

		return hitAnything;
#else
		for (const auto& object : objects)
		{
			if (object->Raycast(r, tMin, closest, &tempRec))
			{
				hitAnything = true;
				closest = tempRec.t;
				*rec = tempRec;
			}
		}

		return hitAnything;
#endif

	}

	bool8 sol::RayTracingHittableList::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		if (objects.empty()) return false;

		PreciseAABB tempBox = {};
		bool8 first_box = true;

		for (const auto& object : objects) {
			if (!object->GetBoundingBox(time0, time1, &tempBox)) { return false; }

			*box = first_box ? tempBox : SurroundingAABB(*box, tempBox);
			first_box = false;
		}

		return true;
	}

	void sol::RayTracingWorld::MakeRandomSphereWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0.70, 0.80, 1.00);
		auto ground_material = std::make_shared<Lambertian>(std::make_shared<CheckerTexture>(Vec3d(0.2, 0.3, 0.1), Vec3d(0.9, 0.9, 0.9)));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, ground_material));

		for (int a = -11; a < 11; a++) {
			for (int b = -11; b < 11; b++) {
				auto choose_mat = RandomReal64();
				Vec3d center(a + 0.9 * RandomReal64(), 0.2, b + 0.9 * RandomReal64());

				if (Mag(center - Vec3d(4, 0.2, 0)) > 0.9) {

					if (choose_mat < 0.8) {
						// diffuse
						auto albedo = RandomVec3() * RandomVec3();

						objects.Add(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<Lambertian>(albedo)));
					}
					else if (choose_mat < 0.95) {
						// metal
						auto albedo = RandomVec3(0.5, 1);
						auto fuzz = RandomReal64(0, 0.5);

						objects.Add(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<MetalMaterial>(albedo, fuzz)));
					}
					else {
						// glass
						objects.Add(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<DielectricMaterial>(1.5)));
					}
				}
			}
		}

		auto material1 = std::make_shared<DielectricMaterial>(1.5);
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 1, 0), 1.0, material1));

		auto material2 = std::make_shared<Lambertian>(Vec3d(0.4, 0.2, 0.1));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(-4, 1, 0), 1.0, material2));

		auto material3 = std::make_shared<MetalMaterial>(Vec3d(0.7, 0.6, 0.5), 0.0);
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(4, 1, 0), 1.0, material3));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void sol::RayTracingWorld::MakeTwoSphereWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0.70, 0.80, 1.00);
		auto checker = std::make_shared<CheckerTexture>(Vec3d(0.2, 0.3, 0.1), Vec3d(0.9, 0.9, 0.9));

		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, -10, 0), 10, std::make_shared<Lambertian>(checker)));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 10, 0), 10, std::make_shared<Lambertian>(checker)));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void RayTracingWorld::MakeTwoPerlineSpheres(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0.70, 0.80, 1.00);
		auto pertext = std::make_shared<NoiseTexture>();
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, std::make_shared<Lambertian>(pertext)));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 2, 0), 2, std::make_shared<Lambertian>(pertext)));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void RayTracingWorld::MakeTextureWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0.70, 0.80, 1.00);
		auto earth_texture = std::make_shared<ImageTexture>("MenuBackground");
		auto earth_surface = std::make_shared<Lambertian>(earth_texture);

		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 0, 0), 2, earth_surface));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void RayTracingWorld::MakeSimpleLight(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0);

		auto pertext = std::make_shared<NoiseTexture>();
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, std::make_shared<Lambertian>(pertext)));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 2, 0), 2, std::make_shared<Lambertian>(pertext)));

		auto difflight = std::make_shared<DiffuseLight>(Vec3d(4));
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, 6.5, 0), 2, difflight));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(26, 3, 6);
		Vec3d lookat(0, 2, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void RayTracingWorld::MakeBoxWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0.70, 0.80, 1.00);
		objects.Add(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, std::make_shared<Lambertian>(Vec3d(0.3f))));
		//objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, 2, 0), 1, std::make_shared<Lambertian>(Vec3d(0.2, 0.2, 7.0))));
		objects.Add(std::make_shared<RayTracingBox>(Vec3d(4, 1, 1), Vec3d(0, 2, 0), std::make_shared<Lambertian>(Vec3d(0.2, 0.2, 7.0))));

		objects.Seal();
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(0, 7, 13);
		Vec3d lookat(0, 2, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void RayTracingWorld::MakeCornellBox(RayTracingCamera* camera, real64 aspectRatio)
	{
		backgroundColour = Vec3d(0);

		auto red = std::make_shared<Lambertian>(Vec3d(.65, .05, .05));
		auto white = std::make_shared<Lambertian>(Vec3d(.73, .73, .73));
		auto green = std::make_shared<Lambertian>(Vec3d(.12, .45, .15));
		auto light = std::make_shared<DiffuseLight>(Vec3d(15, 15, 15));

		objects.Add(std::make_shared<RayTracingYZRect>(0, 555, 0, 555, 555, green));
		objects.Add(std::make_shared<RayTracingYZRect>(0, 555, 0, 555, 0, red));
		objects.Add(std::make_shared<RayTracingXZRect>(213, 343, 227, 332, 554, light));
		objects.Add(std::make_shared<RayTracingXZRect>(0, 555, 0, 555, 0, white));
		objects.Add(std::make_shared<RayTracingXZRect>(0, 555, 0, 555, 555, white));
		objects.Add(std::make_shared<RayTracingXYRect>(0, 555, 0, 555, 555, white));


		std::shared_ptr<RayTracingObject> box1 = std::make_shared<RayTracingBox>(Vec3d(0, 0, 0), Vec3d(165, 330, 165), white);
		box1 = std::make_shared<RayTracingRotateY>(box1, 15);
		box1 = std::make_shared<RayTracingTranslate>(box1, Vec3d(265, 0, 295));
		objects.Add(box1);

		std::shared_ptr<RayTracingObject> box2 = std::make_shared<RayTracingBox>(Vec3d(0, 0, 0), Vec3d(165, 165, 165), white);
		box2 = std::make_shared<RayTracingRotateY>(box2, -18);
		box2 = std::make_shared<RayTracingTranslate>(box2, Vec3d(130, 0, 65));
		objects.Add(box2);

		objects.Seal();

		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(278, 278, -800);
		Vec3d lookat(278, 278, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 40.0, aspectRatio, aperture, dist_to_focus);
	}

	// @NOTE: Thanks to ray tracing gems 2. 
	static bool8 RaycastAABB(const RayTracingRay& r, const PreciseAABB& aabb, HitRecord* record)
	{
		Vec3d tmin = (aabb.min - r.origin) / r.direction;
		Vec3d tmax = (aabb.max - r.origin) / r.direction;

		Vec3d sc = Min(tmin, tmax);
		Vec3d sf = Max(tmin, tmax);

		real64 t0 = Max(sc.x, sc.y, sc.z);
		real64 t1 = Min(sf.x, sf.y, sf.z);

		if (t0 <= t1 && t1 > 0.0)
		{
			if (record)
			{
				record->t = t0;
				record->p = r.TravelDown(t0);

				Vec3d a = Abs(record->p - aabb.min);
				Vec3d b = Abs(record->p - aabb.max);

				// @TODO: Make fast
				real64 min = a.x;
				record->normal = Vec3d(-1, 0, 0);
				if (a.y < min) { record->normal = Vec3d(0, -1, 0); min = a.y; }
				if (a.z < min) { record->normal = Vec3d(0, 0, -1); min = a.z; }
				if (b.x < min) { record->normal = Vec3d(1, 0, 0); min = b.x; }
				if (b.y < min) { record->normal = Vec3d(0, 1, 0); min = b.y; }
				if (b.z < min) { record->normal = Vec3d(0, 0, 1); min = b.z; }

				record->frontFace = true;
			}

			return true;
		}

		return false;
	}

	// @NOTE: Thanks to ray tracing gems 2. 
	static bool8 RaycastOBB(const RayTracingRay& r, const PreciseOBB& obb, HitRecord* record)
	{
		Mat4d m = Mat4d(ScaleCardinal(obb.basis, obb.extents), obb.origin);
		Mat4d invm = Inverse(m);
		Mat4d nm = Transpose(invm);

		RayTracingRay rPrime = {};
		rPrime.origin = Vec4ToVec3(Vec4d(r.origin, 1.0) * invm);
		rPrime.direction = Vec4ToVec3(Vec4d(r.direction, 0.0) * invm);

		PreciseAABB bPrime = {};
		bPrime.min = Vec3d(-0.5);
		bPrime.max = Vec3d(0.5);

		if (RaycastAABB(rPrime, bPrime, record))
		{
			if (record)
			{
				record->p = r.TravelDown(record->t);
				record->normal = Normalize(Vec4ToVec3(Vec4d(record->normal, 0.0) * nm));
			}

			return true;
		}

		return false;
	}

	static PreciseAABB CreateAABBContainingOBB(const PreciseOBB& obb)
	{
		Mat4d mat(obb.basis, obb.origin);
		Vec3d extents = obb.extents;

		Vec3d v0 = Vec4ToVec3(Vec4d(extents, 1) * mat);
		Vec3d v1 = Vec4ToVec3(Vec4d(extents * -1.0, 1) * mat);

		Vec3d v2 = Vec4ToVec3(Vec4d(-extents.x, extents.y, extents.z, 1) * mat);
		Vec3d v3 = Vec4ToVec3(Vec4d(extents.x, -extents.y, extents.z, 1) * mat);
		Vec3d v4 = Vec4ToVec3(Vec4d(extents.x, extents.y, -extents.z, 1) * mat);

		Vec3d v5 = Vec4ToVec3(Vec4d(-extents.x, -extents.y, extents.z, 1) * mat);
		Vec3d v6 = Vec4ToVec3(Vec4d(extents.x, -extents.y, -extents.z, 1) * mat);
		Vec3d v7 = Vec4ToVec3(Vec4d(-extents.x, extents.y, -extents.z, 1) * mat);

		PreciseAABB result = {};
		result.min = Min(Min(Min(Min(Min(Min(Min(v0, v1), v2), v3), v4), v5), v6), v7);
		result.max = Max(Max(Max(Max(Max(Max(Max(v0, v1), v2), v3), v4), v5), v6), v7);

		return result;
	}

	void RayTracingSphere::GetUV(const Vec3d& p, real64* u, real64* v) const
	{
		auto theta = acos(-p.y);
		auto phi = atan2(-p.z, p.x) + PI;

		*u = phi / (2.0 * PI);
		*v = theta / PI;
	}

	bool8 RayTracingSphere::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		Vec3d oc = r.origin - sphere.origin;
		auto a = MagSqrd(r.direction);
		auto half_b = Dot(oc, r.direction);
		auto c = MagSqrd(oc) - sphere.radius * sphere.radius;

		auto discriminant = half_b * half_b - a * c;
		if (discriminant < 0) return false;
		auto sqrtd = Sqrt(discriminant);

		auto root = (-half_b - sqrtd) / a;
		if (root < tMin || tMax < root)
		{
			root = (-half_b + sqrtd) / a;
			if (root < tMin || tMax < root) { return false; }
		}

		rec->t = root;
		rec->p = r.TravelDown(rec->t);
		rec->normal = (rec->p - sphere.origin) / sphere.radius;
		Vec3d outward_normal = (rec->p - sphere.origin) / sphere.radius;
		rec->SetFaceNormal(r, outward_normal);
		rec->material = material;
		GetUV(outward_normal, &rec->u, &rec->v);

		return true;
	}

	bool8 RayTracingXYRect::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		auto t = (k - r.origin.z) / r.direction.z;
		if (t < tMin || t > tMax) { return false; }

		auto x = r.origin.x + t * r.direction.x;
		auto y = r.origin.y + t * r.direction.y;

		if (x < x0 || x > x1 || y < y0 || y > y1) { return false; }

		rec->u = (x - x0) / (x1 - x0);
		rec->v = (y - y0) / (y1 - y0);
		rec->t = t;
		auto outward_normal = Vec3d(0, 0, 1);
		rec->SetFaceNormal(r, outward_normal);
		rec->material = material;
		rec->p = r.TravelDown(t);

		return true;
	}

	bool8 RayTracingXYRect::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		*box = PreciseAABB(Vec3d(x0, y0, k - 0.0001), Vec3d(x1, y1, k + 0.0001));
		return true;
	}

	bool8 RayTracingXZRect::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		auto t = (k - r.origin.y) / r.direction.y;
		if (t < tMin || t > tMax) { return false; }

		auto x = r.origin.x + t * r.direction.x;
		auto z = r.origin.z + t * r.direction.z;

		if (x < x0 || x > x1 || z < z0 || z > z1) { return false; }

		rec->u = (x - x0) / (x1 - x0);
		rec->v = (z - z0) / (z1 - z0);
		rec->t = t;
		auto outward_normal = Vec3d(0, 1, 0);
		rec->SetFaceNormal(r, outward_normal);
		rec->material = material;
		rec->p = r.TravelDown(t);

		return true;
	}

	bool8 RayTracingXZRect::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		*box = PreciseAABB(Vec3d(x0, k - 0.0001, z0), Vec3d(x1, k + 0.0001, z1));
		return true;
	}

	bool8 RayTracingYZRect::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		auto t = (k - r.origin.x) / r.direction.x;
		if (t < tMin || t > tMax) { return false; }
		auto y = r.origin.y + t * r.direction.y;
		auto z = r.origin.z + t * r.direction.z;

		if (y < y0 || y > y1 || z < z0 || z > z1) { return false; }

		rec->u = (y - y0) / (y1 - y0);
		rec->v = (z - z0) / (z1 - z0);
		rec->t = t;
		auto outward_normal = Vec3d(1, 0, 0);
		rec->SetFaceNormal(r, outward_normal);
		rec->material = material;
		rec->p = r.TravelDown(t);
		return true;
	}

	bool8 RayTracingYZRect::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		*box = PreciseAABB(Vec3d(k - 0.0001, y0, z0), Vec3d(k + 0.0001, y1, z1));
		return true;
	}

	RayTracingBox::RayTracingBox(Vec3d boxMin, Vec3d boxMax, std::shared_ptr<RayTracingMaterial> m) : boxMin(boxMin), boxMax(boxMax), material(m)
	{
		sides.Open();
		sides.Add(std::make_shared<RayTracingXYRect>(boxMin.x, boxMax.x, boxMin.y, boxMax.y, boxMax.z, m));
		sides.Add(std::make_shared<RayTracingXYRect>(boxMin.x, boxMax.x, boxMin.y, boxMax.y, boxMin.z, m));
		sides.Add(std::make_shared<RayTracingXZRect>(boxMin.x, boxMax.x, boxMin.z, boxMax.z, boxMax.y, m));
		sides.Add(std::make_shared<RayTracingXZRect>(boxMin.x, boxMax.x, boxMin.z, boxMax.z, boxMin.y, m));
		sides.Add(std::make_shared<RayTracingYZRect>(boxMin.y, boxMax.y, boxMin.z, boxMax.z, boxMax.x, m));
		sides.Add(std::make_shared<RayTracingYZRect>(boxMin.y, boxMax.y, boxMin.z, boxMax.z, boxMin.x, m));
		sides.Seal();
	}

	bool8 RayTracingBox::Raycast(const RayTracingRay& ray, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		return sides.Raycast(ray, tMin, tMax, rec);
	}

	bool8 RayTracingBox::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		*box = PreciseAABB(boxMin, boxMax);
		return true;
	}

	bool8 RayTracingTranslate::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		RayTracingRay moved_r = RayTracingRay::Create(r.origin - offset, r.direction);

		if (!ptr->Raycast(moved_r, tMin, tMax, rec)) { return false; }

		rec->p += offset;
		rec->SetFaceNormal(moved_r, rec->normal);

		return true;
	}

	bool8 RayTracingTranslate::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		if (!ptr->GetBoundingBox(time0, time1, box)) { return false; }

		*box = PreciseAABB(box->min + offset, box->max + offset);

		return true;
	}

	RayTracingRotateY::RayTracingRotateY(std::shared_ptr<RayTracingObject> p, real64 angle) : ptr(p)
	{
		auto radians = DegToRad(angle);
		sin_theta = sin(radians);
		cos_theta = cos(radians);
		hasbox = ptr->GetBoundingBox(0, 1, &bbox);

		Vec3d min(REAL_MAX);
		Vec3d max(-REAL_MAX);

		for (int32 i = 0; i < 2; i++)
		{
			for (int32 j = 0; j < 2; j++)
			{
				for (int k = 0; k < 2; k++) {
					auto x = i * bbox.max.x + (1 - i) * bbox.min.x;
					auto y = j * bbox.max.y + (1 - j) * bbox.min.y;
					auto z = k * bbox.max.z + (1 - k) * bbox.min.z;

					auto newx = cos_theta * x + sin_theta * z;
					auto newz = -sin_theta * x + cos_theta * z;

					Vec3d tester(newx, y, newz);

					for (int32 c = 0; c < 3; c++)
					{
						min[c] = fmin(min[c], tester[c]);
						max[c] = fmax(max[c], tester[c]);
					}
				}
			}
		}

		bbox = PreciseAABB(min, max);
	}

	bool8 RayTracingRotateY::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		auto origin = r.origin;
		auto direction = r.direction;

		origin[0] = cos_theta * r.origin[0] - sin_theta * r.origin[2];
		origin[2] = sin_theta * r.origin[0] + cos_theta * r.origin[2];

		direction[0] = cos_theta * r.direction[0] - sin_theta * r.direction[2];
		direction[2] = sin_theta * r.direction[0] + cos_theta * r.direction[2];

		RayTracingRay rotated_r = RayTracingRay::Create(origin, direction);

		if (!ptr->Raycast(rotated_r, tMin, tMax, rec)) { return false; }

		auto p = rec->p;
		auto normal = rec->normal;

		p[0] = cos_theta * rec->p[0] + sin_theta * rec->p[2];
		p[2] = -sin_theta * rec->p[0] + cos_theta * rec->p[2];

		normal[0] = cos_theta * rec->normal[0] + sin_theta * rec->normal[2];
		normal[2] = -sin_theta * rec->normal[0] + cos_theta * rec->normal[2];

		rec->p = p;
		rec->SetFaceNormal(rotated_r, normal);

		return true;
	}

	bool8 RayTracingSphere::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		box->min = sphere.origin - Vec3d(sphere.radius, sphere.radius, sphere.radius);
		box->max = sphere.origin + Vec3d(sphere.radius, sphere.radius, sphere.radius);

		return true;
	}

	bool8 RayTracingBVHNode::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitrecord) const
	{
		if (!RaycastAABB(r, nodeBox, nullptr)) { return false; }

		bool8 hitLeft = left->Raycast(r, tMin, tMax, hitrecord);
		bool8 hitRight = right->Raycast(r, tMin, hitLeft ? hitrecord->t : tMax, hitrecord);

		return hitLeft || hitRight;
	}

	bool8 RayTracingBVHNode::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		*box = nodeBox;
		return true;
	}

	inline static bool8 BoxCompare(const std::shared_ptr<RayTracingObject> a, const std::shared_ptr<RayTracingObject> b, int32 axis) {
		PreciseAABB boxA;
		PreciseAABB boxB;

		if (!a->GetBoundingBox(0, 0, &boxA) || !b->GetBoundingBox(0, 0, &boxB))
		{
			Assert(0, "No bounding box in bvh");
			return false;
		}

		return boxA.min[axis] < boxB.min[axis];
	}

	inline static bool8 BoxXCompare(const std::shared_ptr<RayTracingObject> a, const std::shared_ptr<RayTracingObject> b) {
		return BoxCompare(a, b, 0);
	}

	inline static bool8 BoxYCompare(const std::shared_ptr<RayTracingObject> a, const std::shared_ptr<RayTracingObject> b) {
		return BoxCompare(a, b, 1);
	}

	inline static bool8 BoxZCompare(const std::shared_ptr<RayTracingObject> a, const std::shared_ptr<RayTracingObject> b) {
		return BoxCompare(a, b, 2);
	}

	bool8 RayTracingBVHNode::Build(const std::vector<std::shared_ptr<RayTracingObject>>& srcObjects, uint64 start, uint64 end, real64 time0, real64 time1)
	{
		auto objects = srcObjects;

		int64 axis = RandomInt64(0, 2);
		auto comparator = (axis == 0) ? BoxXCompare : (axis == 1) ? BoxYCompare : BoxZCompare;

		uint64 objectSpan = end - start;

		if (objectSpan == 1)
		{
			left = objects[start];
			right = objects[start];
		}
		else if (objectSpan == 2)
		{
			if (comparator(objects[start], objects[start + 1]))
			{
				left = objects[start];
				right = objects[start + 1];
			}
			else
			{
				left = objects[start + 1];
				right = objects[start];
			}
		}
		else
		{
			std::sort(objects.begin() + start, objects.begin() + end, comparator);

			uint64 mid = start + objectSpan / 2;
			left = std::make_shared<RayTracingBVHNode>();
			((RayTracingBVHNode*)left.get())->Build(objects, start, mid, time0, time1);
			right = std::make_shared<RayTracingBVHNode>();
			((RayTracingBVHNode*)right.get())->Build(objects, mid, end, time0, time1);
		}

		PreciseAABB boxLeft = {};
		PreciseAABB boxRight = {};

		if (!left->GetBoundingBox(time0, time1, &boxLeft) || !right->GetBoundingBox(time0, time1, &boxRight))
		{
			Assert(0, "No bounding box in bvh");
			return false;
		}

		nodeBox = SurroundingAABB(boxLeft, boxRight);

		return true;
	}

	bool8 RayTracingWorld::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* rec) const
	{
		return objects.Raycast(r, tMin, tMax, rec);
	}

	bool8 sol::RayTracingWorld::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		return objects.GetBoundingBox(time0, time1, box);
	}

	Vec3d TraceRay(const RayTracingRay& r, const RayTracingWorld& world, int32 depth)
	{
		if (depth <= 0) { return Vec3d(0, 0, 0); }

		HitRecord record = {};
		if (!world.Raycast(r, 0.0001, 100000.0, &record)) { return world.backgroundColour; }

		Vec3d emitted = record.material->Emission(record.u, record.v, record.p);

		Vec3d attenuation;
		RayTracingRay scattered;
		if (!record.material->Scatter(r, record, &attenuation, &scattered)) {
			return emitted;
		}

		return emitted + attenuation * TraceRay(scattered, world, depth - 1);
	}

	void ReferenceRayTracer::Trace()
	{
		//Input* inp = Input::Get();
		//SOLTRACE(String("x:").Add(inp->mousePositionPixelCoords.x).Add(" y:").Add(inp->mousePositionPixelCoords.y).GetCStr());

		if (updateCount++ == 1) { updateCount = 0; Renderer::UpdateWholeTexture(textureHandle, pixels.data()); }

		if (!complete)
		{
			int32 pixelCount = (int32)pixels.size();
			SOLINFO(String("Sample: ").Add(pixelCaches.at(0).samples).GetCStr());

			//ProfilerClock PIXEL("TOTAL");
			for (int32 pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++)
			{
				//ProfilerClock PIXEL("PIXELS");
				int32 i = pixelIndex % imageWidth;
				int32 j = pixelIndex / imageWidth;

				if (i == 200 && j == 80)
					int a = 2;

				PixelCache* cache = &pixelCaches.at(pixelIndex);

				if (cache->samples < cache->totalSamples)
				{
					auto u = (real64(i) + RandomReal64()) / (real64)(imageWidth - 1);
					auto v = (real64(j) + RandomReal64()) / (real64)(imageHeight - 1);
					RayTracingRay ray = camera.GetRay(u, v);
					cache->colour += TraceRay(ray, world, cache->depth);
					cache->samples++;
				}
				else
				{
					complete = true;
				}

				Vec3d colour = cache->colour / (real64)cache->samples;
				int32 index = (-j + imageHeight - 1) * imageWidth + i;
				//int32 index = j * imageWidth + i;

				real32 r = Sqrt((real32)colour.x);
				real32 g = Sqrt((real32)colour.y);
				real32 b = Sqrt((real32)colour.z);

				pixels.at(index) = Vec4f(r, g, b, 1.0f);
			}
		}
	}
}