#pragma once
#include "core/SolarCore.h"

namespace cm
{
	struct GameUpdate
	{
		int32 hostTick;
		int32 peerTick;
		int32 ttl;

		Vec3f player1TankPos;
		Quatf player1TankOri;
		Vec3f player1TurretPos;
		Quatf player1TurretOri;

		Vec3f player2TankPos;
		Quatf player2TankOri;
		Vec3f player2TurretPos;
		Quatf player2TurretOri;

		uint8 player1SpawnBullet;
		uint8 player2SpawnBullet;

		void Reconstruct(int32 index, GameUpdate* last, GameUpdate* closest);
		inline bool IsComplete() { return hostTick == peerTick; }
	};

	enum class SnapShotType : uint8
	{
		INVALID = 0,
		HANDSHAKE_CONNECTION,
		PING,
		TRANSFORM,
		BULLET_SHOT,
		TICK,
	};

	struct SnapShotTransform
	{
		SnapShotType type;
		Vec3f tankPosition;
		Quatf tankOrientation;

		Vec3f turretPosition;
		Quatf turretOrientation;
	};

	struct SnapShotBulletShot
	{
		uint32 bulletId;
		bool ack;
		Vec3f position;
		Quatf orientation;
	};

	struct SnapGameTick
	{
		int32 tickNumber;

		uint8 playerSpawnBullet;

		Vec3f tankPosition;
		Quatf tankOrientation;
		Vec3f turretPosition;
		Quatf turretOrientation;
	};

	struct SnapShotPing
	{
		bool ack;
	};

	struct Command
	{
		uint32 type;
		union
		{

		};
	};

	struct SnapShot
	{
		SnapShotType type;
		union
		{
			SnapShotTransform snapTransform;
			SnapShotBulletShot snapBullet;
			SnapGameTick snapTick;
			SnapShotPing snapPing;
		};
	};

	static_assert(sizeof(SnapShotTransform) < 256);

	struct MultiplayerState
	{
		static constexpr int32 TICKS_PER_SECOND = 60;
		static constexpr int32 TICKS_BEFORE_CONSIDERED_DROPED = 5;
		static constexpr int32 TICKS_MAX_LEAD = 7;
		static constexpr int32 TIMEOUT_TIME_SECONDS = 2;

		bool startedNetworkStuff;
		bool connectionValid;
		PlatformAddress myAddress;
		PlatformAddress peerAddress;

		int32 currentTick;
		int32 processTick;

		GameUpdate lastGameUpdate;
		FixedArray<SnapGameTick, TICKS_MAX_LEAD> lastSentTicks;
		FixedArray<SnapGameTick, 64> unproccessedHostTicks;
		FixedArray<SnapGameTick, 64> unproccessedPeerTicks;
		FixedArray<GameUpdate, 64> gameUpdates;

		real32 timeSinceLastTick;
		real32 timeSinceLastSend;

		real32 pingTimer;

		void FillUpdateFromSnap(GameUpdate* gameUpdate, SnapGameTick* snap, bool p1);
		int32 GetNumberOfHostTicks();
		GameUpdate* GetLatestValidGameUpdate();
		GameUpdate* GetGameUpdate(int32 tickIndex);
		GameUpdate* GetNextGameplayUpdate(class Room* room, real32 dt);
	};
}