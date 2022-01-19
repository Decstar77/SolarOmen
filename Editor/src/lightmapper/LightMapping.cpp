#include "LightMapping.h"

namespace sol
{
	void ReferenceRayTracer::Initialize(uint32 samples)
	{
		imageWidth = (int32)Application::GetSurfaceWidth();
		imageHeight = (int32)Application::GetSurfaceHeight();
		aspectRatio = (real64)Application::GetSurfaceAspectRatio();

		TextureFormat format = TextureFormat::Value::R32G32B32A32_FLOAT;
		textureHandle = Renderer::CreateTexture(imageWidth, imageHeight, format, ResourceCPUFlags::Value::WRITE);
		pixels.resize(imageWidth * imageHeight);

		pixelsProcessed = 0;

		complete = false;
		camera.Initialize(Vec3d(0, 0, 3), Vec3d(0, 0, -1), Vec3d(0, 1, 0), 45.0, aspectRatio);
		samplesPerPixel = samples;
		world.objects.push_back(PreciseSphere::Create(Vec3d(0, 0, -1), 0.5f));
		world.objects.push_back(PreciseSphere::Create(Vec3d(0, -100.5, -1), 100.0f));
	}

	void ReferenceRayTracer::Shutdown()
	{
		Renderer::DestroyTexture(&textureHandle);
	}

	bool8 RaycastSphere(const PreciseSphere& sphere, const PreciseRay& r, real64 tMin, real64 tMax, PreciseHitRecord* rec)
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

		return true;
	}

	bool8 RayTracerWorld::Trace(const PreciseRay& r, real64 tMin, real64 tMax, PreciseHitRecord* hitRecord) const
	{
		PreciseHitRecord tempRec = {};
		bool8 hitAnything = false;
		real64 closest = tMax;

		for (const auto& object : objects)
		{
			if (RaycastSphere(object, r, tMin, closest, &tempRec))
			{
				hitAnything = true;
				closest = tempRec.t;
				*hitRecord = tempRec;
			}
		}

		return hitAnything;
	}

	Vec3d TraceRay(const PreciseRay& r, const RayTracerWorld& world)
	{
		PreciseHitRecord record = {};
		if (world.Trace(r, 0.0, 100000.0, &record))
		{
			return 0.5 * (record.normal + Vec3d(1, 1, 1));
		}

		Vec3d unit_direction = Normalize(r.direction);
		real64 t = 0.5 * (unit_direction.y + 1.0);
		return (1.0 - t) * Vec3d(1.0, 1.0, 1.0) + t * Vec3d(0.5, 0.7, 1.0);
	}

	void ReferenceRayTracer::Trace()
	{
		if (updateCount++ == 30) { updateCount = 0; Renderer::UpdateWholeTexture(textureHandle, pixels.data()); }

		if (!complete)
		{
			int32 pixelCount = (int32)pixels.size();
			uint32 pixelsProcssedThisUpdate = 0;
			for (int32 pixelIndex = pixelsProcessed; pixelIndex < pixelCount; pixelIndex++, pixelsProcessed++)
			{
				int32 i = pixelIndex % imageWidth;
				int32 j = pixelIndex / imageWidth;

				Vec3d colour = Vec3d();

				int32 samples = 0;
				for (; samples < 1; samples++)
				{
					auto u = real64(i + RandomBillateral<real64>()) / (real64)(imageWidth - 1);
					auto v = real64(j + RandomBillateral<real64>()) / (real64)(imageHeight - 1);
					PreciseRay ray = camera.GetRay(u, v);
					colour += TraceRay(ray, world);
				}

				colour = colour / (real64)samples;

				int32 index = (-j + imageHeight - 1) * imageWidth + i;
				pixels.at(index) = Vec4f((real32)colour.x, (real32)colour.y, (real32)colour.z, 1.0f);

				pixelsProcssedThisUpdate++;
				if (pixelsProcssedThisUpdate == 5) { return; }
			}

			//for (int32 j = imageHeight - 1; j >= 0; j--)
			//{
			//	for (int32 i = 0; i < imageWidth; i++)
			//	{
			//		Vec3d colour = Vec3d();
			//		for (int32 s = 1; s <= 1; ++s)
			//		{
			//			auto u = real64(i + RandomBillateral<real64>()) / (real64)(imageWidth - 1);
			//			auto v = real64(j + RandomBillateral<real64>()) / (real64)(imageHeight - 1);
			//			PreciseRay ray = camera.GetRay(u, v);
			//			colour += TraceRay(ray, world);
			//		}

			//		colour = colour / (real64)1;

			//		int32 index = (-j + imageHeight) * imageWidth + i;
			//		pixels.at(index) = Vec4f((real32)colour.x, (real32)colour.y, (real32)colour.z, 1.0f);
			//	}
			//}

			complete = true;
		}
	}
}