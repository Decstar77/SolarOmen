#include "AssetIdGenerator.h"

#include <random>
namespace cm
{
	inline static std::random_device randomDevice;
	inline static std::mt19937_64 randomEngine(randomDevice());
	inline static std::uniform_int_distribution<uint64> randomDistribution;

	AssetId GenerateAssetId()
	{
		AssetId id = {};
		id.number = randomDistribution(randomEngine);

		return id;
	}
}