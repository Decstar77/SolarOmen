#pragma once
#include "core/SolarCore.h"
#include "EntityId.h"
#include <bitset>

namespace cm
{
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
	};

	struct SnapShotCommand : public SnapShotHeader
	{
		uint32 tickNumber;
	};

	struct SnapShotSpawnBullet : public SnapShotHeader
	{
		Vec3f position;
		Quatf orientation;
	};

	struct SnapShotDestroyEntity : public SnapShotHeader
	{
		EntityId entityId;
	};

	struct SnapShotActivatePressurePad : public SnapShotHeader
	{
		EntityId entityId;
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

	enum class GameCommandType : uint8
	{
		INVALID,
		SPAWN_BULLET,
		DESTROY_ENTITY,
	};

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

	static_assert(sizeof(CompressedQuatf) == sizeof(uint32));

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

	struct SpawnBulletCommand
	{
		Vec3f pos;
		Quatf ori;

		void Deserialize(MemoryStream* stream)
		{
			pos = *stream->GetNext<Vec3f>();
			ori = *stream->GetNext<Quatf>();
		}

		void Serialize(MemoryStream* stream)
		{
			stream->Add(GameCommandType::SPAWN_BULLET);
			stream->Add(pos);
			stream->Add(ori);
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