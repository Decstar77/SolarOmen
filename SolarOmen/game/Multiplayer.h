#pragma once
#include "core/SolarCore.h"
#include "EntityId.h"

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
		static constexpr int32 TICKS_PER_SECOND = 60;

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