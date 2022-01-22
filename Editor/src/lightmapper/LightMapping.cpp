#include "LightMapping.h"
#include <algorithm>

namespace sol
{

	static Vec3d RandomPointInUnitSphere()
	{
		while (true)
		{
			Vec3d p = {};
			p.x = RandomReal64(-1.0, 1.0);
			p.y = RandomReal64(-1.0, 1.0);
			p.z = RandomReal64(-1.0, 1.0);

			if (MagSqrd(p) >= 1) { continue; }

			return p;
		}
	}

	static Vec3d RandomPointInUnitHemisphere(const Vec3d& normal)
	{
		Vec3d in_unit_sphere = RandomPointInUnitSphere();
		return (Dot(in_unit_sphere, normal) > 0.0) ? in_unit_sphere : -1.0 * in_unit_sphere;
	}

	static Vec3d RandomPointInUnitDisk()
	{
		while (true)
		{
			auto p = Vec3d(RandomReal64(-1, 1), RandomReal64(-1, 1), 0);
			if (MagSqrd(p) >= 1) continue;
			return p;
		}
	}
	static Vec3d RandomVec3()
	{
		return Vec3d(RandomReal64(), RandomReal64(), RandomReal64());
	}

	static Vec3d RandomVec3(real64 min, real64 max)
	{
		return Vec3d(RandomReal64(min, max), RandomReal64(min, max), RandomReal64(min, max));
	}

	PerlinNoise::PerlinNoise()
	{
		ranVec = new Vec3d[POINT_COUNT];
		for (int32 i = 0; i < POINT_COUNT; i++) { ranVec[i] = RandomVec3(-1.0, 1.0); }

		permx = GeneratePerm();
		permy = GeneratePerm();
		permz = GeneratePerm();
	}

	PerlinNoise::~PerlinNoise()
	{
		delete[] ranVec;
		delete[] permx;
		delete[] permy;
		delete[] permz;
	}

	real64 PerlinNoise::Sample(const Vec3d& p) const
	{
		real64 u = p.x - Floor(p.x);
		real64 v = p.y - Floor(p.y);
		real64 w = p.z - Floor(p.z);

		int32 i = (int32)(Floor(p.x));
		int32 j = (int32)(Floor(p.y));
		int32 k = (int32)(Floor(p.z));

		Vec3d c[2][2][2] = {};

		for (int32 di = 0; di < 2; di++)
		{
			for (int32 dj = 0; dj < 2; dj++)
			{
				for (int32 dk = 0; dk < 2; dk++)
				{
					c[di][dj][dk] = ranVec[permx[(i + di) & 255] ^ permy[(j + dj) & 255] ^ permz[(k + dk) & 255]];
				}
			}
		}

		return TrilinearInterp(c, u, v, w);
	}

	real64 sol::PerlinNoise::Sample01(const Vec3d& p) const
	{
		return Clamp(0.5 * (1.0 + Sample(p)), 0.0, 1.0);
	}

	real64 sol::PerlinNoise::Turb(const Vec3d& p, int depth) const
	{
		Vec3d temp = p;
		real64 accum = 0.0;
		real64 weight = 1.0;

		for (int32 i = 0; i < depth; i++)
		{
			accum += weight * Sample(temp);
			weight *= 0.5;
			temp = temp * 2.0;
		}

		return Abs(accum);
	}

	real64 sol::PerlinNoise::TrilinearInterp(Vec3d c[2][2][2], real64 u, real64 v, real64 w) const
	{
		real64 uu = u * u * (3 - 2 * u);
		real64 vv = v * v * (3 - 2 * v);
		real64 ww = w * w * (3 - 2 * w);
		real64 accum = 0.0;

		for (int32 i = 0; i < 2; i++)
		{
			for (int32 j = 0; j < 2; j++)
			{
				for (int32 k = 0; k < 2; k++)
				{
					Vec3d weight_v(u - i, v - j, w - k);
					accum += (i * uu + (1 - i) * (1 - uu))
						* (j * vv + (1 - j) * (1 - vv))
						* (k * ww + (1 - k) * (1 - ww))
						* Dot(c[i][j][k], weight_v);
				}
			}
		}

		return accum;
	}

	int32* PerlinNoise::GeneratePerm()
	{
		int32* p = new int32[POINT_COUNT];
		for (int32 i = 0; i < POINT_COUNT; i++) { p[i] = i; }
		Shuffle(p, POINT_COUNT);

		return p;
	}

	void PerlinNoise::Shuffle(int32* p, int32 n)
	{
		for (int32 i = n - 1; i > 0; i--) {
			int32 target = (int32)RandomInt64(0, i);
			int32 tmp = p[i];
			p[i] = p[target];
			p[target] = tmp;
		}
	}

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

		pixelsProcessed = 0;

		complete = false;

		samplesPerPixel = samples;

		world.objects.clear();
		//world.MakeRandomSphereWorld(&camera, aspectRatio);
		//world.MakeTwoSphereWorld(&camera, aspectRatio);
		//world.MakeTwoPerlineSpheres(&camera, aspectRatio);
		world.MakeTextureWorld(&camera, aspectRatio);
	}
	void ReferenceRayTracer::Shutdown()
	{
		Renderer::DestroyTexture(&textureHandle);
	}



	void sol::RayTracingWorld::MakeRandomSphereWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		auto ground_material = std::make_shared<Lambertian>(std::make_shared<CheckerTexture>(Vec3d(0.2, 0.3, 0.1), Vec3d(0.9, 0.9, 0.9)));
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, ground_material));

		for (int a = -11; a < 11; a++) {
			for (int b = -11; b < 11; b++) {
				auto choose_mat = RandomReal64();
				Vec3d center(a + 0.9 * RandomReal64(), 0.2, b + 0.9 * RandomReal64());

				if (Mag(center - Vec3d(4, 0.2, 0)) > 0.9) {

					if (choose_mat < 0.8) {
						// diffuse
						auto albedo = RandomVec3() * RandomVec3();

						objects.push_back(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<Lambertian>(albedo)));
					}
					else if (choose_mat < 0.95) {
						// metal
						auto albedo = RandomVec3(0.5, 1);
						auto fuzz = RandomReal64(0, 0.5);

						objects.push_back(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<MetalMaterial>(albedo, fuzz)));
					}
					else {
						// glass
						objects.push_back(std::make_shared<RayTracingSphere>(center, 0.2, std::make_shared<DielectricMaterial>(1.5)));
					}
				}
			}
		}

		auto material1 = std::make_shared<DielectricMaterial>(1.5);
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, 1, 0), 1.0, material1));

		auto material2 = std::make_shared<Lambertian>(Vec3d(0.4, 0.2, 0.1));
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(-4, 1, 0), 1.0, material2));

		auto material3 = std::make_shared<MetalMaterial>(Vec3d(0.7, 0.6, 0.5), 0.0);
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(4, 1, 0), 1.0, material3));

		bvhTree.Build(objects, 0, objects.size(), 0, 0);
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
		auto checker = std::make_shared<CheckerTexture>(Vec3d(0.2, 0.3, 0.1), Vec3d(0.9, 0.9, 0.9));

		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, -10, 0), 10, std::make_shared<Lambertian>(checker)));
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, 10, 0), 10, std::make_shared<Lambertian>(checker)));

		bvhTree.Build(objects, 0, objects.size(), 0, 0);
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
		auto pertext = std::make_shared<NoiseTexture>();
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, -1000, 0), 1000, std::make_shared<Lambertian>(pertext)));
		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, 2, 0), 2, std::make_shared<Lambertian>(pertext)));

		bvhTree.Build(objects, 0, objects.size(), 0, 0);
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
	}

	void sol::RayTracingWorld::MakeTextureWorld(RayTracingCamera* camera, real64 aspectRatio)
	{
		auto earth_texture = std::make_shared<ImageTexture>("MenuBackground");
		auto earth_surface = std::make_shared<Lambertian>(earth_texture);

		objects.push_back(std::make_shared<RayTracingSphere>(Vec3d(0, 0, 0), 2, earth_surface));

		bvhTree.Build(objects, 0, objects.size(), 0, 0);
		SOLINFO("Raytracing BVH built");

		Vec3d lookfrom(13, 2, 3);
		Vec3d lookat(0, 0, 0);
		Vec3d vup(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;

		camera->Initialize(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, dist_to_focus);
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

	static bool8 NearZero(const Vec3d& e)
	{
		const auto s = 1e-8;
		return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
	}

	Vec3d CheckerTexture::Value(real64 u, real64 v, const Vec3d& p) const
	{
		real64 sines = Sin(10 * p.x) * Sin(10 * p.y) * Sin(10 * p.z);
		return sines < 0 ? odd->Value(u, v, p) : even->Value(u, v, p);
	}

	ImageTexture::ImageTexture(const String& name)
	{
		TextureResource* res = Resources::GetTextureResource(name);
		width = res->width;
		height = res->height;
		data = (uint8*)res->pixels.data;

		Assert(res->format == TextureFormat::Value::R8G8B8A8_UNORM, "Texture format not supported for ray tracing");
		bytesPerPixel = res->format.GetPitchBytes();
		rowBytes = res->format.GetPitchBytes() * width;
	}

	Vec3d ImageTexture::Value(real64 u, real64 v, const Vec3d& p) const
	{
		if (!data) { return Vec3d(0, 1, 1); }

		u = Clamp(u, 0.0, 1.0);
		v = 1.0 - Clamp(v, 0.0, 1.0);

		int32 i = int32(u * width);
		int32 j = int32(v * height);

		if (i >= width) { i = width - 1; }
		if (j >= height) { j = height - 1; }

		const real64 color_scale = 1.0 / 255.0;

		uint8* pixel = &data[j * rowBytes + i * bytesPerPixel];
		return Vec3d(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
	}

	bool8 Lambertian::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		Vec3d scatterDir = rec.normal + Normalize(RandomPointInUnitSphere());

		if (NearZero(scatterDir)) { scatterDir = rec.normal; }


		*scattered = RayTracingRay::Create(rec.p, scatterDir);
		*attenuation = albedo->Value(rec.u, rec.v, rec.p);
		return true;
	}

	bool8 MetalMaterial::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		Vec3d reflected = Reflect(Normalize(r.direction), rec.normal);
		*scattered = RayTracingRay::Create(rec.p, reflected + fuzz * RandomPointInUnitSphere());
		*attenuation = albedo;
		return (Dot(scattered->direction, rec.normal) > 0);
	}

	bool8 sol::DielectricMaterial::Scatter(const RayTracingRay& r, const HitRecord& rec, Vec3d* attenuation, RayTracingRay* scattered) const
	{
		*attenuation = Vec3d(1.0, 1.0, 1.0);
		real64 refraction_ratio = rec.frontFace ? (1.0 / ir) : ir;

		Vec3d unit_direction = Normalize(r.direction);
		real64 cos_theta = Min(Dot(-1.0 * unit_direction, rec.normal), 1.0);
		real64 sin_theta = Sqrt(1.0 - cos_theta * cos_theta);

		bool8 cannot_refract = refraction_ratio * sin_theta > 1.0;
		Vec3d direction;

		if (cannot_refract || Reflectance(cos_theta, refraction_ratio) > RandomReal64())
			direction = Reflect(unit_direction, rec.normal);
		else
			direction = Refract(unit_direction, rec.normal, refraction_ratio);

		*scattered = RayTracingRay::Create(rec.p, direction);
		return true;
	}

	static bool8 RaycastAABB(const RayTracingRay& r, const PreciseAABB& aabb, real64 t_min, real64 t_max)
	{
		for (int32 a = 0; a < 3; a++) {
			auto invD = 1.0f / r.direction[a];
			auto t0 = (aabb.min[a] - r.origin[a]) * invD;
			auto t1 = (aabb.max[a] - r.origin[a]) * invD;

			if (invD < 0.0f) { Swap(&t0, &t1); }

			t_min = t0 > t_min ? t0 : t_min;
			t_max = t1 < t_max ? t1 : t_max;
			if (t_max <= t_min) { return false; }
		}

		return true;
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

	bool8 RayTracingSphere::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		box->min = sphere.origin - Vec3d(sphere.radius, sphere.radius, sphere.radius);
		box->max = sphere.origin + Vec3d(sphere.radius, sphere.radius, sphere.radius);

		return true;
	}

	bool8 RayTracingBVHNode::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitrecord) const
	{
		if (!RaycastAABB(r, nodeBox, tMin, tMax)) { return false; }

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

	bool8 RayTracingWorld::Raycast(const RayTracingRay& r, real64 tMin, real64 tMax, HitRecord* hitRecord) const
	{
		HitRecord tempRec = {};
		bool8 hitAnything = false;
		real64 closest = tMax;

#if 1
		if (bvhTree.Raycast(r, tMin, closest, &tempRec))
		{
			hitAnything = true;
			closest = tempRec.t;
			*hitRecord = tempRec;
		}

		return hitAnything;
#else
		for (const auto& object : objects)
		{
			if (object->Raycast(r, tMin, closest, &tempRec))
			{
				hitAnything = true;
				closest = tempRec.t;
				*hitRecord = tempRec;
			}
		}

		return hitAnything;
#endif
	}

	bool8 sol::RayTracingWorld::GetBoundingBox(real64 time0, real64 time1, PreciseAABB* box) const
	{
		if (objects.empty()) return false;

		bool8 first = true;
		for (const auto& object : objects)
		{
			PreciseAABB temp = {};
			if (object->GetBoundingBox(time0, time1, &temp))
			{
				*box = first ? temp : SurroundingAABB(*box, temp);
				first = false;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	Vec3d TraceRay(const RayTracingRay& r, const RayTracingWorld& world, int32 depth)
	{
		if (depth <= 0) { return Vec3d(0, 0, 0); }

		HitRecord record = {};
		if (world.Raycast(r, 0.0001, 100000.0, &record))
		{
			RayTracingRay scatterd = {};
			Vec3d attenuation = {};

			if (record.material->Scatter(r, record, &attenuation, &scatterd))
			{
				return attenuation * TraceRay(scatterd, world, depth - 1);
			}

			return Vec3d(0);
		}

		Vec3d unit_direction = Normalize(r.direction);
		real64 t = 0.5 * (unit_direction.y + 1.0);
		return (1.0 - t) * Vec3d(1.0, 1.0, 1.0) + t * Vec3d(0.5, 0.7, 1.0);
	}

	void ReferenceRayTracer::Trace()
	{
		if (updateCount++ == 1) { updateCount = 0; Renderer::UpdateWholeTexture(textureHandle, pixels.data()); }

		if (!complete)
		{
			int32 pixelCount = (int32)pixels.size();
			uint32 pixelsProcssedThisUpdate = 0;

			//ProfilerClock PIXEL("TOTAL");
			for (int32 pixelIndex = pixelsProcessed; pixelIndex < pixelCount; pixelIndex++, pixelsProcessed++)
			{
				//ProfilerClock PIXEL("PIXELS");
				int32 i = pixelIndex % imageWidth;
				int32 j = pixelIndex / imageWidth;

				Vec3d colour = Vec3d();

				int32 samples = 0;
				int32 depth = 2;

				for (; samples < 10; samples++)
				{
					auto u = (real64(i) + RandomReal64()) / (real64)(imageWidth - 1);
					auto v = (real64(j) + RandomReal64()) / (real64)(imageHeight - 1);
					RayTracingRay ray = camera.GetRay(u, v);
					colour += TraceRay(ray, world, depth);
				}


				colour = colour / (real64)samples;

				int32 index = (-j + imageHeight - 1) * imageWidth + i;

				real32 r = Sqrt((real32)colour.x);
				real32 g = Sqrt((real32)colour.y);
				real32 b = Sqrt((real32)colour.z);

				pixels.at(index) = Vec4f(r, g, b, 1.0f);

				pixelsProcssedThisUpdate++;
				if (pixelsProcssedThisUpdate == 5000000 / (samples * depth)) { return; }
			}

			complete = true;
		}
	}
}