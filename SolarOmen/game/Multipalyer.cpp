#include "Multiplayer.h"
#include "TankGame.h"

namespace cm
{
	void GameUpdate::Reconstruct(int32 index, GameUpdate* last, GameUpdate* closest)
	{
		Assert(hostTick != 0, "Host tick cannot be reconstructed !!");
		Assert(index <= 7, "Can't reconstruct a lead greater than 7");
		peerTick = hostTick;

		real32 t = 1.0f / (real32)(index + 1);
		player2TankPos = Lerp(last->player2TankPos, closest->player2TankPos, t);
		player2TankOri = Slerp(last->player2TankOri, closest->player2TankOri, t);
		player2TurretPos = Lerp(last->player2TurretPos, closest->player2TurretPos, t);
		player2TurretOri = Slerp(last->player2TurretOri, closest->player2TurretOri, t);
	}

	GameUpdate* MultiplayerState::GetLatestValidGameUpdate()
	{
		GameUpdate* temp = GameMemory::PushPermanentStruct<GameUpdate>();
		for (int32 i = 0; i < (int32)gameUpdates.count; i++)
		{
			bool swapped = false;
			for (int32 j = 0; j < (int32)gameUpdates.count - i - 1; j++)
			{
				GameUpdate* e1 = &gameUpdates[j];
				GameUpdate* e2 = &gameUpdates[j + 1];

				int32 e1Tick = Max(e1->hostTick, e1->peerTick);
				int32 e2Tick = Max(e2->hostTick, e2->peerTick);

				if (e1Tick > e2Tick)
				{
					*temp = *e1;
					*e1 = *e2;
					*e2 = *temp;
					swapped = true;
				}
			}

			if (!swapped)
				break;
		}

		if (gameUpdates.count > 0)
		{
			GameUpdate* current = &gameUpdates[0];
			if (current->IsComplete())
			{
				return current;
			}
			else
			{
				current->ttl++;

				if (current->ttl >= MultiplayerState::TICKS_BEFORE_CONSIDERED_DROPED)
				{
					int32 numComplete = 0;
					int32 closestIndex = 0;
					for (int32 i = 1; i < (int32)gameUpdates.count; i++)
					{
						GameUpdate* next = &gameUpdates[i];
						if (next->IsComplete())
						{
							if (closestIndex == 0)
								closestIndex = i;
							numComplete++;
						}
					}

					if (closestIndex > 0)
					{
						Debug::LogInfo("Reconstructing packet!!");
						current->Reconstruct(closestIndex, &lastGameUpdate, &gameUpdates[closestIndex]);
					}
				}
			}
		}

		return nullptr;
	}

	void MultiplayerState::FillUpdateFromSnap(GameUpdate* update, SnapGameTick* snap, bool p1)
	{
		if (p1)
		{
			update->player1TankPos = snap->tankPosition;
			update->player1TurretPos = snap->turretPosition;
			update->player1TankOri = snap->tankOrientation;
			update->player1TurretOri = snap->turretOrientation;
			update->player1SpawnBullet = snap->playerSpawnBullet;
		}
		else
		{
			update->player2TankPos = snap->tankPosition;
			update->player2TurretPos = snap->turretPosition;
			update->player2TankOri = snap->tankOrientation;
			update->player2TurretOri = snap->turretOrientation;
			update->player2SpawnBullet = snap->playerSpawnBullet;
		}
	}

	int32 MultiplayerState::GetNumberOfHostTicks()
	{
		int32 count = 0;
		for (uint32 i = 0; i < gameUpdates.count; i++)
		{
			if (gameUpdates[i].hostTick != 0)
			{
				count++;
			}
		}

		return count;
	}

	GameUpdate* MultiplayerState::GetGameUpdate(int32 tickIndex)
	{
		for (uint32 i = 0; i < gameUpdates.count; i++)
		{
			GameUpdate* gameUpdate = &gameUpdates[i];
			if (gameUpdate->hostTick == tickIndex || gameUpdate->peerTick == tickIndex)
			{
				return gameUpdate;
			}
		}

		// @TODO: This might be too big for the stack !!
		GameUpdate update = {};
		return gameUpdates.Add(update);
	}

	GameUpdate* MultiplayerState::GetNextGameplayUpdate(Room* room, real32 dt)
	{
		pingTimer += dt;

		GetInput();

		if (!startedNetworkStuff)
		{
			if (IsKeyJustDown(input, f9) || DebugState::host)
			{
				myAddress = Platform::NetworkStart(54000);
				startedNetworkStuff = true;
				room->isPlayer1 = true;
				room->player1Tank = room->hostTank;
				room->player2Tank = room->peerTank;
			}

			if (IsKeyJustDown(input, f10) || DebugState::peer)
			{
				myAddress = Platform::NetworkStart(54001);
				startedNetworkStuff = true;
				room->player1Tank = room->peerTank;
				room->player2Tank = room->hostTank;
			}
		}

		if (!connectionValid && startedNetworkStuff)
		{
			SnapShot snapShot = {};
			snapShot.type = SnapShotType::HANDSHAKE_CONNECTION;
			int32 port = myAddress.port == 54000 ? 54001 : 54000;
			Platform::NetworkSend((void*)&snapShot, sizeof(SnapShot), "192.168.0.107", port);
		}


		uint8 buffer[Platform::MAX_NETWORK_PACKET_SIZE] = {};
		PlatformAddress address = {};
		while (Platform::NetworkReceive(buffer, sizeof(buffer), &address) > 0)
		{
			SnapShot* snap = (SnapShot*)buffer;
			if (snap->type == SnapShotType::HANDSHAKE_CONNECTION && !connectionValid)
			{
				Debug::LogInfo(CString("Connected to").Add((int32)address.port));
				peerAddress = address;
				Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
				connectionValid = true;
			}
			if (snap->type == SnapShotType::PING)
			{
				if (!snap->snapPing.ack)
				{
					snap->snapPing.ack = true;
					Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
				}
				else
				{
					Debug::LogInfo(CString("Ping: ").Add(pingTimer * 1000.0f));
				}
			}
			else if (snap->type == SnapShotType::TICK)
			{
				//Debug::LogInfo(CString("RecTick ").Add(snap->snapTick.tickNumber));
				unproccessedPeerTicks.Add(snap->snapTick);
			}
		}

		if (connectionValid)
		{
			//Debug::LogInfo(CString("Count").Add(gameUpdates.count));

			timeSinceLastSend += dt;
			if ((timeSinceLastSend >= 1.0f / (real32)TICKS_PER_SECOND))
			{
				bool flooding = false;

				if (GetNumberOfHostTicks() >= TICKS_MAX_LEAD)
				{
					flooding = true;
					if (timeSinceLastSend > TIMEOUT_TIME_SECONDS)
					{
						//LOG("Connection timing out !!");
					}
					else
					{
						//LOG("flooding");
					}
				}

				if (!flooding)
				{
					currentTick++;

					Transform t1 = room->hostVisualTank.GetWorldTransform();
					Transform t2 = room->hostVisualTurret.GetWorldTransform();

					uint8 temp = 0;
					if (!lastSentTicks.IsEmpty())
						temp = (lastSentTicks[lastSentTicks.count - 1].playerSpawnBullet << 1);

					SnapGameTick snap = {};
					snap.tickNumber = currentTick;
					snap.playerSpawnBullet = room->spawnBullet;
					snap.tankPosition = t1.position;
					snap.tankOrientation = t1.orientation;
					snap.turretPosition = t2.position;
					snap.turretOrientation = t2.orientation;

					unproccessedHostTicks.Add(snap);

					timeSinceLastSend = 0.0f;
					room->spawnBullet = false;
				}
			}

			while (unproccessedHostTicks.count > 0)
			{
				SnapGameTick snap = unproccessedHostTicks[0];
				GameUpdate* update = GetGameUpdate(snap.tickNumber);
				update->hostTick = snap.tickNumber;
				FillUpdateFromSnap(update, &snap, room->isPlayer1);

				SnapShot snapShot = {};
				snapShot.type = SnapShotType::TICK;
				snapShot.snapTick = snap;

				Platform::NetworkSend(&snapShot, sizeof(SnapShot), peerAddress);

				if (lastSentTicks.IsFull())
				{
					lastSentTicks.Remove((uint32)0);
				}

				lastSentTicks.Add(snap);
				unproccessedHostTicks.Remove((uint32)0);
			}

			while (unproccessedPeerTicks.count > 0)
			{
				SnapGameTick snap = unproccessedPeerTicks[0];

				if (snap.tickNumber >= processTick)
				{
					GameUpdate* update = GetGameUpdate(snap.tickNumber);
					update->peerTick = snap.tickNumber;
					FillUpdateFromSnap(update, &snap, !room->isPlayer1);
				}
				else
				{
					// @NOTE: We've recived a packet we thought had been lost
					Debug::LogInfo("We've recived a packet we thought had been lost");
				}
				unproccessedPeerTicks.Remove((uint32)0);
			}
		}

		GameUpdate* gameUpdate = GetLatestValidGameUpdate();

		if (gameUpdate)
		{
			Assert(gameUpdate->hostTick == gameUpdate->peerTick, "Ticks are not the same !!");
			Assert(gameUpdate->hostTick == processTick, "Process tick is not the same!!");
			//Debug::LogFile(CString("h=").Add(gameUpdate->hostTick).Add(" p=").Add(gameUpdate->peerTick));
			//Debug::LogFile(CString("p1pos").Add(ToString(gameUpdate->player1TankPos)).Add("p2pos").Add(ToString(gameUpdate->player2TankPos)));
			//LOG(gameUpdate->hostTick << ":" << gameUpdate->peerTick);

			GameUpdate* next = GameMemory::PushTransientStruct<GameUpdate>();
			*next = *gameUpdate;
			lastGameUpdate = *gameUpdate;
			gameUpdates.Remove(gameUpdate);
			processTick++;

			return next;
		}

		return nullptr;
	}
}