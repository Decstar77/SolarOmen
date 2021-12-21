#pragma once
#include "core/SolarCore.h"
#include "EntityId.h"
#include <bitset>

namespace cm
{

	struct CompressedQuatf
	{
		union
		{
			uint32 data;
			struct
			{
				unsigned int a : 10;
				unsigned int b : 10;
				unsigned int c : 10;
				unsigned int index : 2;
			};
		};
	};

	// @NOTE: Can't use bifields because of padding, or so I think, I haven't found away online to do so.
	struct CompressedVec3f
	{
		uint64 data;

		inline int32 GetX() const {
			int32 result = ((int32)((data >> 0) & 0b011111111111111111111));
			result *= (int32)((data >> 60) & 0b1) == 1 ? -1 : 1;

			return result;
		};

		inline int32 GetY() const {
			int32 result = ((int32)((data >> 20) & 0b011111111111111111111));
			result *= (int32)((data >> 61) & 0b1) == 1 ? -1 : 1;

			return result;
		};

		inline int32 GetZ() const {
			int32 result = ((int32)((data >> 40) & 0b011111111111111111111));
			result *= (int32)((data >> 62) & 0b1) == 1 ? -1 : 1;

			return result;
		};

		inline void SetX(int32 bits) {
			data = (((uint64)(Abs(bits) & 0b011111111111111111111)) << 0) | data;
			data = ((uint64)(bits < 0 ? 1 : 0) << 60) | data;
		}

		inline void SetY(int32 bits) {
			data = (((uint64)(Abs(bits) & 0b011111111111111111111)) << 20) | data;
			data = ((uint64)(bits < 0 ? 1 : 0) << 61) | data;
		}

		inline void SetZ(int32 bits) {
			data = (((uint64)(Abs(bits) & 0b011111111111111111111)) << 40) | data;
			data = ((uint64)(bits < 0 ? 1 : 0) << 62) | data;
		}
	};

	static_assert(sizeof(EntityId) == sizeof(uint64));
	static_assert(sizeof(CompressedQuatf) == sizeof(uint32));
	static_assert(sizeof(CompressedVec3f) == sizeof(uint64));

#define MAX_WORLD_SIZE 255
#define MAX_SIZE_20_BIT 1048575
#define COMPRESSED_VEC_MULTIPIER ((real32)((MAX_SIZE_20_BIT / MAX_WORLD_SIZE) - 2))
#define MAX_SIZE_10_BIT 510.0f
#define CompressFloatTo10Bits(f) (((int32)(f < 0.0f ? 1 : 0) << 9) | (int32)Abs(f))
#define Decompress10BitsToFloat(f) (real32)((((int32)f & 0b0111111111) | 0) * ((f >> 9) == 1 ? -1 : 1))

	inline CompressedQuatf CompressQuatf(const Quatf& q)
	{
		real32 sign = 1;
		int32 maxIndex = 1;
		real32 maxValue = Abs(q[0]);

		for (int32 i = 1; i < 4; i++)
		{
			real32 el = q[i];
			real32 ab = Abs(el);
			if (ab > maxValue)
			{
				sign = el < 0 ? -1.0f : 1.0f;
				maxIndex = i;
				maxValue = ab;
			}
		}

		real32 x = q.x * sign * MAX_SIZE_10_BIT;
		real32 y = q.y * sign * MAX_SIZE_10_BIT;
		real32 z = q.z * sign * MAX_SIZE_10_BIT;
		real32 w = q.w * sign * MAX_SIZE_10_BIT;

		CompressedQuatf result = {};

		switch (maxIndex)
		{
		case 0:
			result.a |= CompressFloatTo10Bits(y);
			result.b |= CompressFloatTo10Bits(z);
			result.c |= CompressFloatTo10Bits(w);
			result.index = 0;
			break;
		case 1:
			result.a |= CompressFloatTo10Bits(x);
			result.b |= CompressFloatTo10Bits(z);
			result.c |= CompressFloatTo10Bits(w);
			result.index = 1;
			break;
		case 2:
			result.a |= CompressFloatTo10Bits(x);
			result.b |= CompressFloatTo10Bits(y);
			result.c |= CompressFloatTo10Bits(w);
			result.index = 2;
			break;
		case 3:
			result.a |= CompressFloatTo10Bits(x);
			result.b |= CompressFloatTo10Bits(y);
			result.c |= CompressFloatTo10Bits(z);
			result.index = 3;
			break;
		}

		return result;
	}

	inline Quatf DecompressQuatf(const CompressedQuatf& compressed)
	{
		int32 maxIndex = (int32)compressed.index;
		real32 a = Decompress10BitsToFloat(compressed.a) / MAX_SIZE_10_BIT;
		real32 b = Decompress10BitsToFloat(compressed.b) / MAX_SIZE_10_BIT;
		real32 c = Decompress10BitsToFloat(compressed.c) / MAX_SIZE_10_BIT;
		real32 d = Sqrt(1.0f - (a * a + b * b + c * c));

		switch (maxIndex)
		{
		case 0: return Quatf(d, a, b, c);
		case 1: return Quatf(a, d, b, c);
		case 2: return Quatf(a, b, d, c);
		case 3: return Quatf(a, b, c, d);
		default: Assert(0, "Compression error");
		}

		return Quatf();
	}

	inline Vec3f DecompressVec3f(const CompressedVec3f& compressed)
	{
		int32 x = compressed.GetX();
		int32 y = compressed.GetY();
		int32 z = compressed.GetZ();

		Vec3f result = {};
		result.x = x / COMPRESSED_VEC_MULTIPIER;
		result.y = y / COMPRESSED_VEC_MULTIPIER;
		result.z = z / COMPRESSED_VEC_MULTIPIER;

		return result;
	}

	inline CompressedVec3f CompressVec3f(const Vec3f& v)
	{
		int32 x = (int32)(v.x * COMPRESSED_VEC_MULTIPIER);
		int32 y = (int32)(v.y * COMPRESSED_VEC_MULTIPIER);
		int32 z = (int32)(v.z * COMPRESSED_VEC_MULTIPIER);

		CompressedVec3f result = {};
		result.SetX(x);
		result.SetY(y);
		result.SetZ(z);

		return result;
	}

	struct MemoryStream
	{
		uint32 bufferCursor;
		FixedArray<uint8, Platform::MAX_NETWORK_PACKET_SIZE> buffer;

		void BeginBufferLoop() { bufferCursor = 0; }
		bool BufferLoopIncomplete() {
			return bufferCursor < buffer.count;
		}

		template<typename T>
		inline T GetNextType()
		{
			static_assert(std::is_enum<T>::value, "Not enum!!");
			return (T)buffer.data[bufferCursor];
		}

		template<typename T>
		inline T* GetNext()
		{
			uint32 index = bufferCursor;
			if (index < buffer.count)
			{
				bufferCursor += sizeof(T);
				return (T*)&buffer.data[index];
			}

			return nullptr;
		}

		template<typename T>
		bool Add(const T& t)
		{
			int32 bytes = sizeof(T);
			if (bytes + buffer.count < buffer.GetCapcity())
			{
				int32 index = buffer.count;
				buffer.count += bytes;
				memcpy(&buffer.data[index], (void*)(&t), bytes);

				return true;
			}

			return false;
		}
	};


	enum class SnapShotType : uint8
	{
		INVALID = 0,
		HANDSHAKE_CONNECTION,
		PING,
		RESEND,
		TRANSFORM,
		BEGIN_COMMANDS,
		END_COMMANDS,
	};

	struct SnapShotHeader
	{
		SnapShotType type;
	};

	struct SnapShotHandShake : public SnapShotHeader
	{

	};

	struct SnapShotResend : public SnapShotHeader
	{
		uint32 tickNumber;
	};

	struct SnapShotTransform : public SnapShotHeader
	{
		EntityId entityId;
		Vec3f position;
		Quatf orientation;

		void Deserialize(MemoryStream* stream)
		{
			entityId = *stream->GetNext<EntityId>();
			position = DecompressVec3f(*stream->GetNext<CompressedVec3f>());
			orientation = DecompressQuatf(*stream->GetNext<CompressedQuatf>());
		}

		bool Serialize(MemoryStream* stream)
		{
			if (stream->buffer.count + 21 < stream->buffer.GetCapcity())
			{
				stream->Add(SnapShotType::TRANSFORM);
				stream->Add(entityId);
				stream->Add(CompressVec3f(position));
				stream->Add(CompressQuatf(orientation));

				return true;
			}

			return false;
		}
	};

	struct SnapShotCommand : public SnapShotHeader
	{
		uint32 tickNumber;
	};

	struct SnapShotPing
	{
		bool ack;
	};

	struct SnapShot
	{
		SnapShotType type;
		uint32 sq;
		uint32 ack;
		union
		{
			SnapShotHandShake handShake;
			SnapShotTransform snapTransform;
			SnapShotPing snapPing;
		};
	};

	static_assert(sizeof(SnapShotTransform) < 256);


	enum class GameCommandType : uint8
	{
		INVALID,
		SPAWN_BULLET,
		DESTROY_ENTITY,
	};

	struct SpawnBulletCommand
	{
		Vec3f pos;
		Quatf ori;

		void Deserialize(MemoryStream* stream)
		{
			pos = DecompressVec3f(*stream->GetNext<CompressedVec3f>());
			ori = DecompressQuatf(*stream->GetNext<CompressedQuatf>());
		}

		void Serialize(MemoryStream* stream)
		{
			stream->Add(GameCommandType::SPAWN_BULLET);
			stream->Add(CompressVec3f(pos));
			stream->Add(CompressQuatf(ori));
		}
	};

	struct DestoryEntityCommand
	{
		EntityId id;

		void Deserialize(MemoryStream* stream)
		{
			id = *stream->GetNext<EntityId>();
		}

		void Serialize(MemoryStream* stream)
		{
			stream->Add(GameCommandType::DESTROY_ENTITY);
			stream->Add(id);
		}
	};

	struct GameCommand
	{
		GameCommandType type;
		union
		{
			SpawnBulletCommand spawnBullet;
			DestoryEntityCommand destroyEntity;
		};
	};

	struct GameCommands
	{
		uint32 hostTick;
		uint32 peerTick;

		inline bool ReceivedHostTick() { return hostTick != 0; }
		inline bool ReceivedPeerTick() { return peerTick != 0; }

		FixedArray<GameCommand, 256> player1Commands;
		FixedArray<GameCommand, 256> player2Commands;
	};

	struct MultiplayerState
	{
		static constexpr int32 TICKS_PER_SECOND = 30;

		bool startedNetworkStuff;
		bool connectionValid;
		PlatformAddress myAddress;
		PlatformAddress peerAddress;

		uint32 tickCounter;
		bool32 gatherCommands;
		real32 timeSinceLastSend;

		PlayerNumber playerNumber;
		GameCommands lastCommands;
		GameCommands currentCommands;

		MemoryStream outputMemoryStream;

		GameCommands* GetNextGameCommands(class Room* room, real32 dt);
		inline bool IsValid() const { return startedNetworkStuff && connectionValid; }

	private:
		FixedArray<GameCommand, 256>* AddCommandsToCurrent(GameCommand* commands, uint32 count, bool peer);
		ManagedArray<GameCommand> DeserializeCommandsFromInputStream(MemoryStream* inputMemoryStream, uint32 tickNumber);
		void SerializeCommandsIntoOutputStream(FixedArray<GameCommand, 256>* commands, uint32 tickNumber);

	};
}