#include "Multiplayer.h"
#include "TankGame.h"

namespace cm
{
	//void GameUpdate::Reconstruct(int32 index, GameUpdate* last, GameUpdate* closest)
	//{
	//	Assert(hostTick != 0, "Host tick cannot be reconstructed !!");
	//	Assert(index <= 7, "Can't reconstruct a lead greater than 7");
	//	peerTick = hostTick;

	//	real32 t = 1.0f / (real32)(index + 1);
	//	player2TankPos = Lerp(last->player2TankPos, closest->player2TankPos, t);
	//	player2TankOri = Slerp(last->player2TankOri, closest->player2TankOri, t);
	//	player2TurretPos = Lerp(last->player2TurretPos, closest->player2TurretPos, t);
	//	player2TurretOri = Slerp(last->player2TurretOri, closest->player2TurretOri, t);
	//}

	//GameUpdate* MultiplayerState::GetLatestValidGameUpdate()
	//{
	//	GameUpdate* temp = GameMemory::PushPermanentStruct<GameUpdate>();
	//	for (int32 i = 0; i < (int32)gameUpdates.count; i++)
	//	{
	//		bool swapped = false;
	//		for (int32 j = 0; j < (int32)gameUpdates.count - i - 1; j++)
	//		{
	//			GameUpdate* e1 = &gameUpdates[j];
	//			GameUpdate* e2 = &gameUpdates[j + 1];

	//			int32 e1Tick = Max(e1->hostTick, e1->peerTick);
	//			int32 e2Tick = Max(e2->hostTick, e2->peerTick);

	//			if (e1Tick > e2Tick)
	//			{
	//				*temp = *e1;
	//				*e1 = *e2;
	//				*e2 = *temp;
	//				swapped = true;
	//			}
	//		}

	//		if (!swapped)
	//			break;
	//	}

	//	if (gameUpdates.count > 0)
	//	{
	//		GameUpdate* current = &gameUpdates[0];
	//		if (current->IsComplete())
	//		{
	//			return current;
	//		}
	//		else
	//		{
	//			current->ttl++;

	//			if (current->ttl >= MultiplayerState::TICKS_BEFORE_CONSIDERED_DROPED)
	//			{
	//				int32 numComplete = 0;
	//				int32 closestIndex = 0;
	//				for (int32 i = 1; i < (int32)gameUpdates.count; i++)
	//				{
	//					GameUpdate* next = &gameUpdates[i];
	//					if (next->IsComplete())
	//					{
	//						if (closestIndex == 0)
	//							closestIndex = i;
	//						numComplete++;
	//					}
	//				}

	//				if (closestIndex > 0)
	//				{
	//					Debug::LogInfo("Reconstructing packet!!");
	//					current->Reconstruct(closestIndex, &lastGameUpdate, &gameUpdates[closestIndex]);
	//				}
	//			}
	//		}
	//	}

	//	return nullptr;
	//}

	//void MultiplayerState::FillUpdateFromSnap(GameUpdate* update, SnapGameTick* snap, bool p1)
	//{
	//	if (p1)
	//	{
	//		update->player1TankPos = snap->tankPosition;
	//		update->player1TurretPos = snap->turretPosition;
	//		update->player1TankOri = snap->tankOrientation;
	//		update->player1TurretOri = snap->turretOrientation;
	//		update->player1SpawnBullet = snap->playerSpawnBullet;
	//	}
	//	else
	//	{
	//		update->player2TankPos = snap->tankPosition;
	//		update->player2TurretPos = snap->turretPosition;
	//		update->player2TankOri = snap->tankOrientation;
	//		update->player2TurretOri = snap->turretOrientation;
	//		update->player2SpawnBullet = snap->playerSpawnBullet;
	//	}
	//}

	//int32 MultiplayerState::GetNumberOfHostTicks()
	//{
	//	int32 count = 0;
	//	for (uint32 i = 0; i < gameUpdates.count; i++)
	//	{
	//		if (gameUpdates[i].hostTick != 0)
	//		{
	//			count++;
	//		}
	//	}

	//	return count;
	//}

	//GameUpdate* MultiplayerState::GetGameUpdate(int32 tickIndex)
	//{
	//	for (uint32 i = 0; i < gameUpdates.count; i++)
	//	{
	//		GameUpdate* gameUpdate = &gameUpdates[i];
	//		if (gameUpdate->hostTick == tickIndex || gameUpdate->peerTick == tickIndex)
	//		{
	//			return gameUpdate;
	//		}
	//	}

	//	// @TODO: This might be too big for the stack !!
	//	GameUpdate update = {};
	//	return gameUpdates.Add(update);
	//}

	//GameUpdate* MultiplayerState::GetNextGameplayUpdate(Room* room, real32 dt)
	//{
	//	pingTimer += dt;

	//	GetInput();

	//	if (!startedNetworkStuff)
	//	{
	//		if (IsKeyJustDown(input, f9) || DebugState::host)
	//		{
	//			myAddress = Platform::NetworkStart(54000);
	//			startedNetworkStuff = true;
	//			room->isPlayer1 = true;
	//			room->player1Tank = room->hostTank;
	//			room->player2Tank = room->peerTank;
	//		}

	//		if (IsKeyJustDown(input, f10) || DebugState::peer)
	//		{
	//			myAddress = Platform::NetworkStart(54001);
	//			startedNetworkStuff = true;
	//			room->player1Tank = room->peerTank;
	//			room->player2Tank = room->hostTank;
	//		}
	//	}

	//	if (!connectionValid && startedNetworkStuff)
	//	{
	//		SnapShot snapShot = {};
	//		snapShot.type = SnapShotType::HANDSHAKE_CONNECTION;
	//		int32 port = myAddress.port == 54000 ? 54001 : 54000;
	//		Platform::NetworkSend((void*)&snapShot, sizeof(SnapShot), "192.168.0.107", port);
	//	}


	//	uint8 buffer[Platform::MAX_NETWORK_PACKET_SIZE] = {};
	//	PlatformAddress address = {};
	//	while (Platform::NetworkReceive(buffer, sizeof(buffer), &address) > 0)
	//	{
	//		SnapShot* snap = (SnapShot*)buffer;
	//		if (snap->type == SnapShotType::HANDSHAKE_CONNECTION && !connectionValid)
	//		{
	//			Debug::LogInfo(CString("Connected to").Add((int32)address.port));
	//			peerAddress = address;
	//			Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
	//			connectionValid = true;
	//		}
	//		if (snap->type == SnapShotType::PING)
	//		{
	//			if (!snap->snapPing.ack)
	//			{
	//				snap->snapPing.ack = true;
	//				Platform::NetworkSend(snap, sizeof(SnapShot), peerAddress);
	//			}
	//			else
	//			{
	//				Debug::LogInfo(CString("Ping: ").Add(pingTimer * 1000.0f));
	//			}
	//		}
	//		else if (snap->type == SnapShotType::TICK)
	//		{
	//			//Debug::LogInfo(CString("RecTick ").Add(snap->snapTick.tickNumber));
	//			unproccessedPeerTicks.Add(snap->snapTick);
	//		}
	//	}

	//	if (connectionValid)
	//	{
	//		//Debug::LogInfo(CString("Count").Add(gameUpdates.count));

	//		timeSinceLastSend += dt;
	//		if ((timeSinceLastSend >= 1.0f / (real32)TICKS_PER_SECOND))
	//		{
	//			bool flooding = false;

	//			if (GetNumberOfHostTicks() >= TICKS_MAX_LEAD)
	//			{
	//				flooding = true;
	//				if (timeSinceLastSend > TIMEOUT_TIME_SECONDS)
	//				{
	//					//LOG("Connection timing out !!");
	//				}
	//				else
	//				{
	//					//LOG("flooding");
	//				}
	//			}

	//			if (!flooding)
	//			{
	//				currentTick++;

	//				Transform t1 = room->hostVisualTank.GetWorldTransform();
	//				Transform t2 = room->hostVisualTurret.GetWorldTransform();

	//				uint8 temp = 0;
	//				if (!lastSentTicks.IsEmpty())
	//					temp = (lastSentTicks[lastSentTicks.count - 1].playerSpawnBullet << 1);

	//				SnapGameTick snap = {};
	//				snap.tickNumber = currentTick;
	//				snap.playerSpawnBullet = room->spawnBullet;
	//				snap.tankPosition = t1.position;
	//				snap.tankOrientation = t1.orientation;
	//				snap.turretPosition = t2.position;
	//				snap.turretOrientation = t2.orientation;

	//				unproccessedHostTicks.Add(snap);

	//				timeSinceLastSend = 0.0f;
	//				room->spawnBullet = false;
	//			}
	//		}

	//		while (unproccessedHostTicks.count > 0)
	//		{
	//			SnapGameTick snap = unproccessedHostTicks[0];
	//			GameUpdate* update = GetGameUpdate(snap.tickNumber);
	//			update->hostTick = snap.tickNumber;
	//			FillUpdateFromSnap(update, &snap, room->isPlayer1);

	//			SnapShot snapShot = {};
	//			snapShot.type = SnapShotType::TICK;
	//			snapShot.snapTick = snap;

	//			Platform::NetworkSend(&snapShot, sizeof(SnapShot), peerAddress);

	//			if (lastSentTicks.IsFull())
	//			{
	//				lastSentTicks.Remove((uint32)0);
	//			}

	//			lastSentTicks.Add(snap);
	//			unproccessedHostTicks.Remove((uint32)0);
	//		}

	//		while (unproccessedPeerTicks.count > 0)
	//		{
	//			SnapGameTick snap = unproccessedPeerTicks[0];

	//			if (snap.tickNumber >= processTick)
	//			{
	//				GameUpdate* update = GetGameUpdate(snap.tickNumber);
	//				update->peerTick = snap.tickNumber;
	//				FillUpdateFromSnap(update, &snap, !room->isPlayer1);
	//			}
	//			else
	//			{
	//				// @NOTE: We've recived a packet we thought had been lost
	//				Debug::LogInfo("We've recived a packet we thought had been lost");
	//			}
	//			unproccessedPeerTicks.Remove((uint32)0);
	//		}
	//	}

	//	GameUpdate* gameUpdate = GetLatestValidGameUpdate();

	//	if (gameUpdate)
	//	{
	//		Assert(gameUpdate->hostTick == gameUpdate->peerTick, "Ticks are not the same !!");
	//		Assert(gameUpdate->hostTick == processTick, "Process tick is not the same!!");
	//		//Debug::LogFile(CString("h=").Add(gameUpdate->hostTick).Add(" p=").Add(gameUpdate->peerTick));
	//		//Debug::LogFile(CString("p1pos").Add(ToString(gameUpdate->player1TankPos)).Add("p2pos").Add(ToString(gameUpdate->player2TankPos)));
	//		//LOG(gameUpdate->hostTick << ":" << gameUpdate->peerTick);

	//		GameUpdate* next = GameMemory::PushTransientStruct<GameUpdate>();
	//		*next = *gameUpdate;
	//		lastGameUpdate = *gameUpdate;
	//		gameUpdates.Remove(gameUpdate);
	//		processTick++;

	//		return next;
	//	}

	//	return nullptr;
	//}
	static void DEBUGLogCommands(FixedArray<GameCommand, 256>* commands, uint32 tick)
	{
		for (uint32 i = 0; i < commands->count; i++)
		{
			Debug::LogFile(CString("").Add((uint32)commands->Get(i)->type).Add(" "));
		}
	}

	FixedArray<GameCommand, 256>* MultiplayerState::AddCommandsToCurrent(GameCommand* commands, uint32 count, bool peer)
	{
		if (playerNumber == PlayerNumber::ONE)
		{
			if (peer)
			{
				for (uint32 i = 0; i < count; i++)
				{
					currentCommands.player2Commands.Add(commands[i]);
				}

				return &currentCommands.player2Commands;
			}
			else
			{
				for (uint32 i = 0; i < count; i++)
				{
					currentCommands.player1Commands.Add(commands[i]);
				}

				return &currentCommands.player1Commands;
			}
		}
		else if (playerNumber == PlayerNumber::TWO)
		{
			if (peer)
			{
				for (uint32 i = 0; i < count; i++)
				{
					currentCommands.player1Commands.Add(commands[i]);
				}

				return &currentCommands.player1Commands;
			}
			else
			{
				for (uint32 i = 0; i < count; i++)
				{
					currentCommands.player2Commands.Add(commands[i]);
				}

				return &currentCommands.player2Commands;
			}
		}

		return nullptr;
	}

	ManagedArray<GameCommand> MultiplayerState::DeserializeCommandsFromInputStream(MemoryStream* inputMemoryStream, uint32 tickNumber)
	{
		ManagedArray<GameCommand> commands = GameMemory::PushTransientArray<GameCommand>(256);
		while (inputMemoryStream->GetNextType<GameCommandType>() != GameCommandType::INVALID)
		{
			GameCommand command = {};
			command.type = *inputMemoryStream->GetNext<GameCommandType>();
			switch (command.type)
			{
			case GameCommandType::SPAWN_BULLET:
			{
				//LOG("RBULLET" << tickNumber);
				command.spawnBullet.Deserialize(inputMemoryStream);
			}break;
			case GameCommandType::DESTROY_ENTITY:
			{
				//LOG("RDESTROY" << tickNumber);
				command.destroyEntity.Deserialize(inputMemoryStream);
			}break;
			default: Assert(0, "UNKNOWN COMMAND");
			}

			commands.Add(command);
		}

		inputMemoryStream->GetNext<GameCommandType>();

		return commands;
	}

	void MultiplayerState::SerializeCommandsIntoOutputStream(FixedArray<GameCommand, 256>* commands, uint32 tickNumber)
	{
		SnapShotCommand snapCommand = {};
		snapCommand.type = SnapShotType::BEGIN_COMMANDS;
		snapCommand.tickNumber = tickNumber;

		outputMemoryStream.Add(snapCommand);
		for (uint32 i = 0; i < commands->count; i++)
		{
			GameCommand* command = commands->Get(i);
			switch (command->type)
			{
			case GameCommandType::SPAWN_BULLET:
			{
				//LOG("SBULLET" << tickNumber);
				command->spawnBullet.Serialize(&outputMemoryStream);
			} break;
			case GameCommandType::DESTROY_ENTITY:
			{
				//LOG("SDESTROY" << tickNumber);
				command->destroyEntity.Serialize(&outputMemoryStream);
			}break;
			default: Assert(0, "UNKNOWN COMMAND");
			}
		}

		outputMemoryStream.Add(GameCommandType::INVALID);
	}

	GameCommands* MultiplayerState::GetNextGameCommands(Room* room, real32 dt)
	{
		GetInput();

		if (!startedNetworkStuff)
		{
			if (IsKeyJustDown(input, f9) || DebugState::host)
			{
				myAddress = Platform::NetworkStart(54000);
				startedNetworkStuff = true;
				playerNumber = PlayerNumber::ONE;
			}

			if (IsKeyJustDown(input, f10) || DebugState::peer)
			{
				myAddress = Platform::NetworkStart(54001);
				startedNetworkStuff = true;
				playerNumber = PlayerNumber::TWO;
			}
		}

		if (!connectionValid && startedNetworkStuff)
		{
			SnapShotHandShake snap = {};
			snap.type = SnapShotType::HANDSHAKE_CONNECTION;
			int32 port = myAddress.port == 54000 ? 54001 : 54000;
			Platform::NetworkSend((void*)&snap, sizeof(SnapShotHandShake), "192.168.0.107", port);
		}

		PlatformAddress address = {};
		MemoryStream inputMemoryStream = {};
		inputMemoryStream.buffer.count = Platform::NetworkReceive(inputMemoryStream.buffer.data, inputMemoryStream.buffer.GetCapcity(), &address);

		while (inputMemoryStream.buffer.count != UINT32_MAX)
		{
			bool32 leave = false;
			inputMemoryStream.BeginBufferLoop();
			while (inputMemoryStream.BufferLoopIncomplete() && !leave)
			{
				SnapShotType type = inputMemoryStream.GetNextType<SnapShotType>();
				switch (type)
				{
				case SnapShotType::HANDSHAKE_CONNECTION:
				{
					SnapShotHandShake* snap = inputMemoryStream.GetNext<SnapShotHandShake>();
					if (!connectionValid)
					{
						peerAddress = address;
						Platform::NetworkSend(snap, sizeof(SnapShotHandShake), peerAddress);
						connectionValid = true;
						Debug::LogInfo(CString("Connected to").Add((int32)address.port));
					}

				}break;
				case SnapShotType::TRANSFORM:
				{
					SnapShotTransform* snap = inputMemoryStream.GetNext<SnapShotTransform>();
					EntityId id = snap->entityId;
					Entity* entity = id.Get();
					if (entity)
					{
						//CString name = entity->GetName(); LOG(name.GetCStr());
						NetworkComponent* comp = &room->networkComponents[id.index];
						comp->lerpPosition = snap->position;
						comp->lerpOrientation = snap->orientation;
					}
				}break;
				case SnapShotType::RESEND:
				{
					SnapShotResend* snap = inputMemoryStream.GetNext<SnapShotResend>();
					if (snap->tickNumber == lastCommands.hostTick)
					{
						outputMemoryStream.buffer.Clear();

						if (playerNumber == PlayerNumber::ONE)
						{
							SerializeCommandsIntoOutputStream(&lastCommands.player1Commands, snap->tickNumber);
						}
						else if (playerNumber == PlayerNumber::TWO)
						{
							SerializeCommandsIntoOutputStream(&lastCommands.player2Commands, snap->tickNumber);
						}

						Platform::NetworkSend(outputMemoryStream.buffer.data, outputMemoryStream.buffer.count, peerAddress);
						outputMemoryStream.buffer.Clear();

						//LOG("Resending: " << snap->tickNumber);
					}
				}break;
				case SnapShotType::BEGIN_COMMANDS:
				{
					SnapShotCommand* snap = inputMemoryStream.GetNext<SnapShotCommand>();

					//LOG("REC: " << snap->tickNumber);

					if (snap->tickNumber == tickCounter && !currentCommands.ReceivedPeerTick())
					{
						currentCommands.peerTick = snap->tickNumber;
						ManagedArray<GameCommand> commands = DeserializeCommandsFromInputStream(&inputMemoryStream, snap->tickNumber);
						AddCommandsToCurrent(commands.data, commands.count, true);
					}
					else if (snap->tickNumber > tickCounter)
					{
						SnapShotResend resend = {};
						resend.type = SnapShotType::RESEND;
						resend.tickNumber = tickCounter;
						Platform::NetworkSend((void*)&resend, sizeof(SnapShotResend), peerAddress);

						//LOG("Requesting resend: " << "REC: " << snap->tickNumber << " C: " << tickCounter);
					}
				}break;

				default:leave = true; //LOG("UNKNOWN SNAP SHOT!!");
				}
			}

			inputMemoryStream.buffer.Clear();
			ZeroArray(inputMemoryStream.buffer.data);
			inputMemoryStream.buffer.count = Platform::NetworkReceive(inputMemoryStream.buffer.data, inputMemoryStream.buffer.GetCapcity(), &address);
		}

		if (connectionValid)
		{

			timeSinceLastSend += dt;
			if ((timeSinceLastSend >= 1.0f / (real32)TICKS_PER_SECOND))
			{
				//static uint32 t = 0;
				//if (RandomUInt(0, 2000) % 5 == 0 && playerNumber == PlayerNumber::ONE && room->commands.count == 0)
				//{
				//	GameCommand cmd = {};
				//	cmd.type = GameCommandType::SPAWN_BULLET;
				//	room->commands.Add(cmd);
				//}

				if (gatherCommands)
				{
					AddCommandsToCurrent(room->commands.data, room->commands.count, false);
					gatherCommands = false;
					room->commands.Clear();
				}

				if (!currentCommands.ReceivedHostTick() || !currentCommands.ReceivedPeerTick())
				{
					currentCommands.hostTick = tickCounter;

					FixedArray<GameCommand, 256>* commands = playerNumber == PlayerNumber::ONE ?
						&currentCommands.player1Commands : &currentCommands.player2Commands;

					SerializeCommandsIntoOutputStream(commands, tickCounter);

					//LOG("SENDING: " << tickCounter);
					//LOG("COMMANDS = " << commands->count);
				}

				for (uint32 i = 0; i < room->transformComponents.GetCapcity(); i++)
				{
					TransformComponent* comp = &room->transformComponents[i];
					NetworkComponent* net = &room->networkComponents[i];
					if (net->playerOwner == playerNumber && room->entities[i].IsValid())
					{
						SnapShotTransform snap = {};
						snap.type = SnapShotType::TRANSFORM;
						snap.entityId = room->entities[i].GetId();
						snap.position = comp->transform.position;
						snap.orientation = comp->transform.orientation;

						if (!outputMemoryStream.Add<SnapShotTransform>(snap))
						{
							LOG("FULL");
							break;
						}
					}
				}

				Platform::NetworkSend(outputMemoryStream.buffer.data, outputMemoryStream.buffer.count, peerAddress);
				outputMemoryStream.buffer.Clear();
				timeSinceLastSend = 0.0f;
			}
		}


		if (currentCommands.ReceivedHostTick() && currentCommands.ReceivedPeerTick())
		{
			Assert(currentCommands.hostTick == currentCommands.peerTick, "Not same");
			//LOG("Tick processing" << currentCommands.hostTick);
			//Debug::LogFile(CString("Tick: ").Add(currentCommands.hostTick));
			//Debug::LogFile("Player1");
			//DEBUGLogCommands(&currentCommands.player1Commands, currentCommands.hostTick);
			//Debug::LogFile("Player2");
			//DEBUGLogCommands(&currentCommands.player2Commands, currentCommands.hostTick);

			tickCounter++;
			gatherCommands = true;
			GameCommands* newCommands = GameMemory::PushTransientStruct<GameCommands>();
			*newCommands = currentCommands;
			lastCommands = currentCommands;
			ZeroStruct(&currentCommands);
			return newCommands;
		}

		return nullptr;
	}
}