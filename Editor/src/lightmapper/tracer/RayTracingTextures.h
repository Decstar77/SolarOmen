#pragma once
#include "RayTracingRay.h"

namespace sol
{
	class PerlinNoise
	{
	public:
		PerlinNoise();
		~PerlinNoise();

		real64 Sample(const Vec3d& p)const;
		real64 Sample01(const Vec3d& p)const;
		real64 Turb(const Vec3d& p, int depth = 7) const;

	private:
		inline static const int32 POINT_COUNT = 256;
		Vec3d* ranVec;
		int32* permx;
		int32* permy;
		int32* permz;
	private:
		real64 TrilinearInterp(Vec3d c[2][2][2], real64 u, real64 v, real64 w) const;
		int32* GeneratePerm();
		void Shuffle(int32* p, int32 n);
	};

	class RayTracingTexture
	{
	public:
		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const = 0;
	};

	class SolidColour : public RayTracingTexture
	{
	public:
		SolidColour() {};
		SolidColour(const Vec3d& colour) : colour(colour) {}
		SolidColour(real64 r, real64 g, real64 b) : colour(r, g, b) {};

		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override { return colour; };

	private:
		Vec3d colour;
	};

	class CheckerTexture : public RayTracingTexture
	{
	public:
		CheckerTexture() {};
		CheckerTexture(const Vec3d& colour1, const Vec3d& colour2)
			: even(std::make_shared<SolidColour>(colour1)), odd(std::make_shared<SolidColour>(colour2)) {}

		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override;
	public:
		std::shared_ptr<SolidColour> odd;
		std::shared_ptr<SolidColour> even;
	};

	class NoiseTexture : public RayTracingTexture
	{
	public:
		NoiseTexture() {};
		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override {
			return Vec3d(1, 1, 1) * 0.5 * (1 + Sin(4.0 * p.z + 10.0 * noise.Turb(p)));
		};

	private:
		PerlinNoise noise;
	};

	class ImageTexture : public RayTracingTexture
	{
	public:
		ImageTexture() :data(nullptr), width(0), height(0), rowBytes(0), bytesPerPixel(0) {}
		ImageTexture(const String& name);
		virtual Vec3d Value(real64 u, real64 v, const Vec3d& p) const override;
	private:
		uint8* data;
		int32 width;
		int32 height;
		int32 bytesPerPixel;
		int32 rowBytes;
	};
}