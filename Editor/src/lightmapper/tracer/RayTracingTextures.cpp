#include "RayTracingTextures.h"


namespace sol
{
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
}