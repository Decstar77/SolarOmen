#include "SolarOmen.h"
#include "Debug.h"
#include <stack>
#include <queue>

namespace cm
{
	static Entity* CreatePointLight(GameState* gs, Vec3f colour = Vec3f(1))
	{
		Entity* entity = gs->CreateEntity();
		entity->lightComp.active = true;
		entity->lightComp.type = LightType::Value::POINT;
		entity->lightComp.colour = colour;
		entity->lightComp.intensity = 1;

		entity->boundingBoxLocal = CreateAABBFromCenterRadius(Vec3f(0), Vec3f(0.1f));

		return entity;
	}

	static Entity* CreateDirectionalLight(GameState* gs, Vec3f colour = Vec3f(1))
	{
		Entity* entity = gs->CreateEntity();
		entity->lightComp.active = true;
		entity->lightComp.type = LightType::Value::DIRECTIONAL;
		entity->lightComp.colour = colour;
		entity->lightComp.intensity = 10;

		entity->boundingBoxLocal = CreateAABBFromCenterRadius(Vec3f(0), Vec3f(0.1f));

		return entity;
	}

	void BakeCollisionGrid(GameState* gs)
	{
		//// @NOTE: Using create to clear
		//CreateCollisionGrid(gs);

		//// @TODO: Entity loop
		//for (int32 i = 0; i < ArrayCount(gs->entites); i++)
		//{
		//	Entity* entity = &gs->entites[i];
		//	OBB collider = GetEntityBoxCollider(entity);

		//	if (IsValidEntity(gs, entity) && entity->active
		//		&& entity->type == EntityType::ENVIRONMENT)
		//	{
		//		CollisionGrid* grid = &gs->collision_grid;
		//		for (int32 z = 0; z < COLLISION_GRID_DEPTH; z++)
		//		{
		//			for (int32 x = 0; x < COLLISION_GRID_WIDTH; x++)
		//			{
		//				CollisionCell* cell = &grid->cells[z][x];
		//				AABB cellbox = GetCollisionCellBox(grid, cell);

		//				if (CheckIntersectionAABBOBB(cellbox, collider))
		//				{
		//					cell->filled = 1;
		//				}
		//			}
		//		}
		//	}
		//}
	}

	static void UpdatePlayer(GameState* gs, AssetState* as, Input* input)
	{
#if 0
		Entity* bot = &gs->player;
		real32 dt = input->dt;

		Camera* camera = &gs->camera;
		Basisf moveBasis = camera->transform.GetBasis();

		Vec3f upwards = Vec3f(0.0f, 1.0f, 0.0f);
		Vec3f sidewards = moveBasis.right; // @NOTE: Assumes we don't ever tilt the camera
		Vec3f forwards = moveBasis.forward;
		forwards.y = 0.0f;
		forwards = Normalize(forwards);

		real32 moveSpeed = 5.0f;
		Vec3f deltaMove = Vec3f(0);
		if (input->w)
		{
			deltaMove += forwards * dt * moveSpeed;
		}
		if (input->s)
		{
			deltaMove -= forwards * dt * moveSpeed;
		}
		if (input->a)
		{
			deltaMove -= sidewards * dt * moveSpeed;
		}
		if (input->d)
		{
			deltaMove += sidewards * dt * moveSpeed;
		}

		deltaMove = Normalize(deltaMove) * moveSpeed * dt;

		bot->transform.position += deltaMove;
		Vec3f desired_cam_pos = bot->transform.position + gs->camera_offset_player;
		// @TODO: Smooth step the camera!!
		camera->transform.position = desired_cam_pos;

#else
		Entity* playerEntity = gs->player;
		PlayerPart* playerPart = &playerEntity->playerPart;
		Basisf basis = QuatToBasis(playerEntity->transform.orientation);

		basis.forward.y = 0.0f;
		basis.right.y = 0.0f;
		basis.upward = Vec3f(0, 1, 0);

		basis.forward = Normalize(basis.forward);
		basis.right = Normalize(basis.right);

		playerPart->acceleration = Vec3f(0);
		playerPart->isSprinting = input->shift;

		real32 sprintMultiplier = playerPart->isSprinting ? 1.5f : 1.0f;

		real32 walkSpeed = 64.0f;
		real32 airSpeed = 16.0f;

		real32 groundResistance = 14.0f;
		real32 airResistance = 2.0f;

		real32 lookSpeed = 0.1f;

		playerPart->grounded = true;

		real32 speed = playerPart->grounded ? walkSpeed * sprintMultiplier : airSpeed;
		real32 resistance = playerPart->grounded ? groundResistance : airResistance;
		if (input->w)
		{
			playerPart->acceleration += (basis.forward * speed);
		}
		if (input->s)
		{
			playerPart->acceleration += (-1.0f * basis.forward * speed);
		}
		if (input->a)
		{
			playerPart->acceleration += (-1.0f * basis.right * speed);
		}
		if (input->d)
		{
			playerPart->acceleration += (basis.right * speed);
		}

		playerPart->acceleration = ClampMag(playerPart->acceleration, speed);

		playerPart->acceleration.x += -resistance * playerPart->velocity.x;
		playerPart->acceleration.z += -resistance * playerPart->velocity.z;

		Vec3f gravity = Vec3f(0.f, -15.8f, 0.f);
		//playerPart->acceleration += gravity;

		//LOG(playerPart->grounded);
		//playerPart->acceleration.y -= 15.8;


		//if (IsKeyJustDown(input, space) && playerPart->grounded)
		//{
		//	//LOG("Jump");
		//	playerPart->velocity.y += 5.2f;
		//	playerPart->grounded = false;
		//}

		Vec3f currentPos = playerEntity->transform.position;

		playerPart->velocity = playerPart->acceleration * gs->dt + playerPart->velocity;
		playerEntity->transform.position += playerPart->velocity * gs->dt;

		//LOG(playerPart->velocity.y);

#if 1
		playerPart->collider = CreateSphere(playerEntity->transform.position, 0.75f);
		playerPart->groundRay = CreateRay(playerEntity->transform.position, Vec3f(0, -1, 0));

		//LOG(playerEntity->transform.position.y);
		//LOG(playerPart->groundRay.origin.y);

		Vec3f upwardVector = Vec3f(0, 1, 0);

		Clock clock;
		clock.Start();

		//for (int32 itterationCount = 0; itterationCount < 10; itterationCount++)
		{
			for (int32 entityIndex = 0; entityIndex < ArrayCount(gs->entites); entityIndex++)
			{
				Entity* entity = &gs->entites[entityIndex];

				if (entity->IsValid() && entity->active
					&& entity->type == EntityType::ENVIRONMENT)
				{
					//MeshData* meshData = LookUpMeshData(as, entity->render.mesh);

					//for (int32 boxIndex = 0; boxIndex < meshData->shadowBoxCount; boxIndex++)
					//{
					//	OBB otherCollider = TransformOBB(CreateOBB(meshData->shadowBoxes[boxIndex]),
					//		entity->transform.CalculateTransformMatrix());

					//	Manifold info = {};
					//	if (CheckManifoldSphereOBB(playerPart->collider, otherCollider, &info))
					//	{
					//		Vec3f sep = info.normal * info.seperationDistance;

					//		if (Dot(info.normal, upwardVector) > 0.9f)
					//		{
					//			sep = Vec3f(0, 1, 0) * info.seperationDistance;
					//			playerPart->velocity.y = 0;
					//			playerPart->grounded = true;
					//		}

					//		playerEntity->transform.position += sep;
					//		playerPart->collider = CreateSphere(playerEntity->transform.position, 0.75f);
					//	}

					//	//real32 rayDist;
					//	//if (RaycastOBB(playerPart->groundRay, otherCollider, &rayDist))
					//	//{
					//	//	if (rayDist < playerPart->groundCheckDist && frameDelta == 0)
					//	//	{
					//	//		real32 r = playerPart->collider.data.w;
					//	//		real32 moveDelta = rayDist - r;

					//	//		//playerEntity->transform.position += playerPart->groundRay.direction * moveDelta;
					//	//		//playerPart->velocity.y = 0;
					//	//		//playerPart->grounded = true;
					//	//		//playerPart->collider = CreateSphere(playerEntity->transform.position, 0.75f);
					//	//		//playerPart->groundRay = CreateRay(playerEntity->transform.position, Vec3f(0, -1, 0));
					//	//	}
					//	//}
					//}

				}
			}
		}

		clock.End();


		if (playerPart->soundStepInterval > .0f) { playerPart->soundStepInterval -= gs->dt * sprintMultiplier; }

		if (DistanceSqrd(currentPos, playerEntity->transform.position) > 0.004f && playerPart->soundStepInterval <= .0f && playerPart->grounded)
		{
			playerPart->soundStepInterval = 0.016f * 30;
			//PlayWalkSound();
		}

#else
		AABB playerCollider = playerEntity->object_space_bounding_box;
		playerCollider.min += playerEntity->transform.position;
		playerCollider.max += playerEntity->transform.position;
		Vec3f ground = Vec3f(0, 1, 0);
		for (int32 entityIndex = 0; entityIndex < ArrayCount(gs->entites); entityIndex++)
		{
			Entity* entity = &gs->entites[entityIndex];
			if (IsValidEntity(gs, entity) && entity->active
				&& entity->type == EntityType::ENVIRONMENT)
			{
				AABB otherCollider = GetEntityBoundingBox(entity);
				// @NOTE: Miscosky sum
				//otherCollider = CreateAABBFromCenterRadius(GetAABBCenter(otherCollider),
				//	GetAABBRadius(otherCollider) + Vec3f(0.24f));
				//DEBUGDrawAABB(rs, otherCollider);
				Manifold info = {};
				if (CheckManifoldAABB(playerCollider, otherCollider, &info))
				{
					Vec3f sep = info.normal * info.seperationDistance;
					playerEntity->transform.position += sep;
					if (Dot(info.normal, ground) > 0.9f)
					{
						playerPart->velocity.y = 0;
						playerPart->grounded = true;
					}
				}
			}
		}
#endif

		Vec2f delta = input->mouseDelta;
		delta.y = -delta.y;

		real32 nyaw = playerPart->yaw - delta.x * lookSpeed;
		real32 npitch = playerPart->pitch + delta.y * lookSpeed;

		npitch = Clamp(npitch, -89.0f, 89.0f);
		playerPart->yaw = Lerp(playerPart->yaw, nyaw, 0.67f);
		playerPart->pitch = Lerp(playerPart->pitch, npitch, 0.67f);

		Vec3f direction;
		direction.x = Cos(DegToRad(playerPart->yaw)) * Cos(DegToRad(playerPart->pitch));
		direction.y = Sin(DegToRad(playerPart->pitch));
		direction.z = Sin(DegToRad(playerPart->yaw)) * Cos(DegToRad(playerPart->pitch));

		Entity* camera = gs->playerCamera;
		Mat4f result = LookAtLH(playerEntity->transform.position, playerEntity->transform.position + direction, Vec3f(0, 1, 0));

		Vec3f cameraOffset = Vec3f(0, 0.75, 0);
		camera->transform.orientation = Mat4ToQuat(result);
		camera->transform.position = playerEntity->transform.position + cameraOffset;
		playerEntity->transform.orientation = camera->transform.orientation;

		input->mouse_locked = true;
#endif
	}

	static void UpdateGame(GameState* gs, Input* input)
	{
		for (int32 entityIndex = 0; entityIndex < ArrayCount(gs->entites); entityIndex++)
		{
			Entity* entity = &gs->entites[entityIndex];

			if (entity->IsValid() && entity->active
				&& entity->type == EntityType::PARTICLE_EMITTER)
			{
				ParticleEmitterPart* emitter = &entity->particlePart;
				emitter->timeSeconds += gs->dt;
				if (emitter->timeSeconds > emitter->spawnRateSeconds)
				{
					//CreateParticle(entity);
					emitter->timeSeconds = 0;
				}

				for (int32 particleIndex = 0; particleIndex < MAX_PARTICLES_PER_EMITTER; particleIndex++)
				{
					Particle* particle = &emitter->particles[particleIndex];
					if (particle->IsAlive())
					{
						particle->velocity += emitter->acceleration * gs->dt;
						particle->transform.position += particle->velocity * gs->dt;

						particle->transform.orientation = (particle->transform.orientation * EulerToQuat(particle->angularAccelertation));

						// @NOTE: Lerps are backwards because life decreases with time
						particle->transform.scale = Lerp(emitter->endingSize, emitter->startingSize, particle->currentLife);
						particle->colour = Lerp(emitter->endingColour, emitter->startingColour, particle->currentLife);

						//particle->transform.position.y += 1 * input->dt;
						//particle->transform.scale = particle->transform.scale * 0.99f;
						particle->currentLife -= gs->dt;
					}
					//LOG(particle->transform.position.y);
				}
			}
		}
	}

	void DEBUGStepPhysics(GameState* gs, TransientState* ts, Input* input)
	{
		ts->physicsSimulator.Begin(0.016f);

		gs->BeginEntityLoop();
		while (Entity* entity = gs->GetNextEntity())
		{

			if (entity->IsPhsyicsEnabled())
			{
				ts->physicsSimulator.AddRigidBody(entity->transform, &entity->rigidBody);
			}
		}

		UpdatePhsyics(gs, ts, input);

		gs->BeginEntityLoop();
		while (Entity* entity = gs->GetNextEntity())
		{
			if (entity->IsPhsyicsEnabled())
			{
				ts->physicsSimulator.UpdateEntity(&entity->transform, &entity->rigidBody);
			}
		}
	}

	static void LoadTrack(GameState* gs, RacingTrack* track)
	{
		Entity* trackEntity = gs->CreateEntity();
		trackEntity->SetName("Track");
		for (uint32 i = 0; i < track->lanes[0].spline.waypoints.count; i++)
		{
			Entity* waypoint = gs->CreateEntity();
			waypoint->transform.position = track->lanes[0].spline.waypoints[i].position;
			waypoint->SetName(CString("Waypoint ").Add(i));
			waypoint->SetParent(trackEntity->GetId());

			RacingWaypoint* rc = waypoint->GetRacingWaypointComponent();
			rc->active = true;
			rc->laneIndex = 0;
			rc->waypointIndex = i;
		}
	}

	void InitializeGameState(GameState* gs, AssetState* as, PlatformState* ws)
	{
		GameState::Initialize(gs);

		gs->CreateEntityFreeList();

		gs->camera.far_ = 100.0f;
		//gs->camera.far_ = 10.0f;
		gs->camera.near_ = 0.3f;
		gs->camera.yfov = 45.0f;
		gs->camera.aspect = (real32)ws->client_width / (real32)ws->client_height;
		gs->camera.transform = Transform();
		gs->camera.transform.position.x = 7;
		gs->camera.transform.position.y = 14;
		gs->camera.transform.position.z = -7;
		gs->camera.transform.LookAtLH(0);

#if 0
		{
			Entity* e = CreateEntity(gs);
			e->transform.scale = Vec3f(100);
			e->renderComp.active = true;
			e->renderComp.modelId = ModelId::Value::BOOMBOX;
			e->renderComp.material.albedoTex = TextureId::BOOMBOX_BASECOLOR;
			e->renderComp.material.occRoughMetTex = TextureId::BOOMBOX_OCCLUSIONROUGHNESSMETALLIC;
			e->renderComp.material.normalTex = TextureId::BOOMBOX_NORMAL;
			e->renderComp.material.emissiveTex = TextureId::BOOMBOX_EMISSIVE;
			e->name = e->renderComp.modelId.ToString();
		}
#endif
		{
			for (int32 modelIndex = 0; modelIndex < as->meshCount; modelIndex++)
			{
				int32 index = gs->meshColliderCount;
				gs->meshColliderCount++;

				MeshCollider* collider = &gs->meshColliders[index];

				MeshData* meshData = &as->meshesData[modelIndex];
				int32 triCount = (int32)Ceil(meshData->positionCount / 3.0f);

				Vec3f min = Vec3f(REAL_MAX);
				Vec3f max = Vec3f(REAL_MIN);

				collider->triangles = GameMemory::PushPermanentArray<Triangle>(triCount);
				for (int32 triangleIndex = 0; triangleIndex < meshData->positionCount; triangleIndex += 3)
				{
					index = collider->triangles.count;
					Triangle* tri = &collider->triangles[index];
					tri->v0 = meshData->positions[triangleIndex];
					tri->v1 = meshData->positions[triangleIndex + 1];
					tri->v2 = meshData->positions[triangleIndex + 2];

					min = Min(min, tri->v0);
					min = Min(min, tri->v1);
					min = Min(min, tri->v2);

					max = Max(max, tri->v0);
					max = Max(max, tri->v1);
					max = Max(max, tri->v2);

					collider->triangles.count++;
				}

				collider->boundingBox = CreateAABBFromMinMax(min, max);
			}
		}

		if (0)
		{
			Entity* player = gs->CreateEntity();
			player->SetName("Player");
			player->boundingBoxLocal = CreateAABBFromCenterRadius(0, Vec3f(0.5f, 1.4f, 0.5f));
			player->transform = Transform();
			player->transform.position.y = 0.8f;
			player->transform.position.z = -3;
			player->playerPart.groundCheckDist = 1.0f;

			gs->player = player;

			Entity* camera = gs->CreateEntity();
			camera->SetName("Player camera");
			camera->cameraComp.active = true;
			camera->cameraComp.far_ = 100.0f;
			camera->cameraComp.near_ = 0.3f;
			camera->cameraComp.yfov = 45.0f;
			camera->cameraComp.aspect = (real32)ws->client_width / (real32)ws->client_height;

			gs->playerCamera = camera;
			gs->camera_offset_player = gs->camera.transform.position - gs->player->transform.position;

			Entity* gun = gs->CreateEntity();
			gun->SetName("Player gun");
			gun->renderComp.active = true;
			gun->renderComp.modelId = ModelId::Value::SM_WEP_RIFLE_BASE_01;
			gun->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			gun->renderComp.SetFlag(RenderFlag::NO_CAST_SHADOW);
			gun->transform.GlobalRotateY(DegToRad(180.0f));
			gun->transform.position.x = 0.35f;
			gun->transform.position.y = -0.263f;
			gun->transform.position.z = 0.63489f;

			gun->SetParent(camera->GetId());

			gs->gun = gun;
		}

		{
			Entity* car = gs->CreateEntity();
			car->renderComp.active = true;
			car->renderComp.modelId = ModelId::Value::SM_VEH_CLASSIC_01;
			car->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			car->SetName(car->renderComp.modelId.ToString());
			car->carComp.active = true;
			car->carComp.speed = 5.5f;
			car->carComp.laneIndex = 0;

			gs->testEntity1 = car;

			Entity* camera = gs->CreateEntity();
			camera->SetName("Player camera");
			camera->cameraComp.active = true;
			camera->cameraComp.far_ = 100.0f;
			camera->cameraComp.near_ = 0.3f;
			camera->cameraComp.yfov = 45.0f;
			camera->cameraComp.aspect = (real32)ws->client_width / (real32)ws->client_height;
			camera->transform.position = Vec3f(0, 5, -9);
			camera->transform.LookAtLH(0);
			camera->transform.LocalRotateX(DegToRad(-10.0f));
			camera->SetParent(car->GetId());

			gs->playerCamera = camera;

			Entity* track = gs->CreateEntity();
			track->renderComp.active = true;
			track->renderComp.modelId = ModelId::Value::CM_TRACK_OVAL;
			track->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			track->SetName(track->renderComp.modelId.ToString());

		}

		{
			Entity* e1 = gs->CreateEntity();
			e1->renderComp.active = true;
			e1->renderComp.modelId = ModelId::Value::SM_WEP_RIFLE_BASE_01;
			e1->SetCollider(ModelId::Value::CUBE);
			e1->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			e1->SetName(e1->renderComp.modelId.ToString());
			e1->transform.position.x = 3;
			//e1->transform.GlobalRotateY(DegToRad(45.0f));

			Entity* e2 = gs->CreateEntity();
			e2->renderComp.active = true;
			e2->renderComp.modelId = ModelId::Value::CUBE;
			e2->SetCollider(ModelId::Value::CUBE);
			e2->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			e2->SetName(e2->renderComp.modelId.ToString());

			e2->transform.position.x = 3;
			e2->SetParent(e1->GetId());

			/*Entity* e3 = gs->CreateEntity();
			e3->renderComp.active = true;
			e3->renderComp.modelId = ModelId::Value::PLANE;
			e3->SetCollider(ModelId::Value::CUBE);
			e3->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			e3->name = e3->renderComp.modelId.ToString();
			e3->transform.scale = Vec3f(10.0f);*/
		}

		if (0)
		{
			Entity* e = gs->CreateEntity();
			e->renderComp.active = true;
			e->renderComp.modelId = ModelId::Value::TEST_1;
			e->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			e->SetName(e->renderComp.modelId.ToString());
		}

		if (0)
		{
			Entity* e = gs->CreateEntity();
			e->renderComp.active = true;
			e->renderComp.modelId = ModelId::Value::FLOORTILE_EMPTY;
			e->renderComp.material.albedoTex = TextureId::Value::POLYGONSCIFI_01_C;
			e->transform.position.y = -0.05f;
			e->transform.scale = Vec3f(1, 1, 1);
			e->SetName(e->renderComp.modelId.ToString());
		}


		{
			Entity* e = CreateDirectionalLight(gs);
			e->transform.position = Vec3f(1.5, 1.5, -1.5);
			e->transform.LookAtLH(Vec3f(0));
			e->SetName("Directional light");
		}

		PlaySomeAudio();

		BakeCollisionGrid(gs);

		CreateWorldSector(gs, Vec3f(100, 100, 100));

		//for (uint32 i = 0; i < gs->currentTrack.lanes.GetCapcity(); i++)
		//{
		//	for (real32 theta = 0; theta <= 2.0f * PI - 0.26f; theta += 0.26f)
		//	{
		//		real32 x = Cos(theta) * (5.0f * (i + 1)) / 2.0f;
		//		real32 y = Sin(theta) * (5.0f * (i + 1)) / 2.0f;
		//		real32 z = 0;// RandomBillateral<real32>();
		//		gs->currentTrack.lanes[i].spline.AddWaypoint({ Vec3f(x,0,y), Normalize(Vec3f(0, 1, 0)) });
		//	}
		//
		//	gs->currentTrack.lanes[i].spline.ComputeDistances();
		//	gs->currentTrack.lanes.count++;
		//}

		LoadTrack(gs, &as->racingTracks[0]);
	}

	WORK_CALLBACK(JobPhysics)
	{
		PhysicsSimulator* ps = (PhysicsSimulator*)data;
		ps->Update();
	}

	static void CarFollowSpline(CatmullRomSpline* spline, Entity* car, real32 dt)
	{
		real32 follow = car->carComp.follow;
		real32 speed = car->carComp.speed;

		Vec3f grad = spline->GetSplineGradient(follow);
		Vec3f currentPos = spline->GetSplinePoint(follow);
		Vec3f currentNormal = spline->GetSplineNormal(follow);
		car->carComp.follow = spline->WrapValue(follow + (speed * dt) / Mag(grad));

		car->transform.position = Lerp(car->transform.position, currentPos, 0.25f);

		Quatf ori = Mat4ToQuat(LookAtLH(car->transform.position, currentPos + grad, currentNormal));

		car->transform.orientation = Slerp(car->transform.orientation, ori, 0.25f);
		//car->transform.LookAtLH(currentPos + grad, currentNormal);

	}

	static void CarController(RacingTrack* track, Entity* car)
	{
		Assert(car->carComp.active, "No car componente");
		Input* input = Input::Get();

		if (IsKeyJustDown(input, a))
		{
			car->carComp.laneIndex = track->GoLeft(car->carComp.laneIndex);
		}
		if (IsKeyJustDown(input, d))
		{
			car->carComp.laneIndex = track->GetRight(car->carComp.laneIndex);
		}
	}

	void UpdateGame(GameState* gs, AssetState* as, PlatformState* ws, Input* input)
	{
		//HermiteCurve curve = {};
		//curve.p0 = Vec3f(0, 1, 1);
		//curve.v0 = Vec3f(3, 0, 0);
		//curve.p1 = Vec3f(1, 0, 0);
		//curve.v1 = Vec3f(5, 0, 0);
		// 
		//BezierCurve curve = {};
		//curve.p0 = Vec3f(0, 1, 0);
		//curve.p1 = Vec3f(1, 1, 0);
		//curve.p2 = Vec3f(0, 0, 0);
		//curve.p3 = Vec3f(1, 0, 0);

		//DEBUGDrawPoint(curve.p0);
		//DEBUGDrawPoint(curve.p1);
		//DEBUGDrawPoint(curve.p2);
		//DEBUGDrawPoint(curve.p3);

		//Vec3f last = curve.Evaulate(0);
		//for (real32 t = 0.01f; t <= 1.0f; t += 0.01f)
		//{
		//	Vec3f next = curve.Evaulate(t);

		//	DEBUGDrawLine(last, next);
		//	last = next;
		//}

		{

		}
		gs->BuildTrackFromEntities();
		gs->DEBUGDrawCurrentTrack();

		//if (IsKeyJustDown(input, t))
		{


			//CarController(&gs->currentTrack, gs->testEntity1);
			CatmullRomSpline* spline = &gs->currentTrack.lanes[gs->testEntity1->carComp.laneIndex].spline;
			CarFollowSpline(spline, gs->testEntity1, gs->dt);
		}


		//ts->physicsSimulator.dt = input->dt;
		//ts->physicsSimulator.Update();
		//Platform::AddWorkEntry(Platform::WorkEntry(JobPhysics, &ts->physicsSimulator));
		//Platform::
		//UpdatePlayer(gs, as, input);

		//Capsule cap = CreateCapsuleFromBaseTip(Vec3f(0, 0, 0), Vec3f(0, 1, 0), 0.25f);
		//DEBUGDrawCapsule(cap);

		//gs->BeginEntityLoop();
		//while (Entity* entity = gs->GetNextEntity())
		//{
		//	if (entity->collisionComp.active)
		//	{
		//		MeshCollider* collider = &gs->meshColliders[entity->collisionComp.meshIndex];
		//		for (uint32 i = 0; i < collider->triangles.count; i++)
		//		{
		//			DEBUGDrawTriangleWithNormal(collider->triangles[i]);
		//		}
		//	}
		//}


		//UpdatePlayerAction(gs, as, input, ws);
		//UpdateGame(gs, input);
	}

	void ConstructRenderGroup(GameState* gs, EntityRenderGroup* renderGroup)
	{
		// @NOTE: Construct the point lights
		int32 maxPointLightCount = ArrayCount(renderGroup->pointLights);
		int32 maxOpaqueEntityCount = ArrayCount(renderGroup->opaqueRenderEntries);

		gs->BeginEntityLoop();
		while (Entity* entity = gs->GetNextEntity())
		{
			if (entity->renderComp.active)
			{
				int32 index = renderGroup->opaqueEntityCount;
				OpaqueRenderEntry* entry = &renderGroup->opaqueRenderEntries[index];

				// @TODO: Is the zero neccesary ?!
				ZeroStruct(entry);
				entry->transform = entity->GetWorldTransform();
				entry->renderComp = entity->renderComp;
				renderGroup->opaqueEntityCount++;
			}

			if (entity->lightComp.active)
			{
				if (entity->lightComp.type == LightType::Value::DIRECTIONAL)
				{
					renderGroup->mainDirectionalLight = *entity;
				}
			}
		}

		//renderGroup->pointLightCount = 1;
		//renderGroup->pointLights[0].transform.position = Vec3f(5, 5, -5);

		renderGroup->mainCamera = gs->playerCamera->cameraComp.ToPureCamera(gs->playerCamera->GetWorldTransform());
		renderGroup->playerCamera = renderGroup->mainCamera;

		//renderGroup->mainCamera = gs->playerCamera->cameraComp.ToPureCamera(gs->playerCamera->transform);
		//renderGroup->playerCamera = renderGroup->mainCamera;
		//renderGroup->player = *gs->player;
	}


	int32 RacingTrack::GoLeft(int32 currentLane)
	{
		if (currentLane - 1 < 0)
		{
			return currentLane;
		}
		currentLane--;
		return currentLane;
	}

	int32 RacingTrack::GetRight(int32 currentLane)
	{
		if (currentLane + 1 >= (int32)lanes.count)
		{
			return currentLane;
		}

		currentLane++;
		return currentLane;
	}

	void GameState::BuildTrackFromEntities()
	{
		RacingTrack* track = &currentTrack;
		track->lanes.count = 1;
		track->lanes[0].spline.waypoints.count = 0;

		for (int32 i = 0; i < ENTITY_STORAGE_COUNT; i++)
		{
			Entity* entity = &entites[i];
			if (entity->IsValid())
			{
				RacingWaypoint* waypoint = &racingWaypointComponents[i];
				if (waypoint->active)
				{
					track->lanes[0].spline.AddWaypoint({ entity->transform.position, Vec3f(0, 1, 0) });
				}
			}
		}
	}

	void GameState::DEBUGDrawCurrentTrack()
	{
		for (uint32 i = 0; i < currentTrack.lanes.count; i++)
		{
			CatmullRomSpline* spline = &currentTrack.lanes[i].spline;
			Vec3f last = spline->GetSplinePoint(0);
			for (real32 t = 0.1f; t < spline->waypoints.count; t += 0.25f)
			{
				Vec3f next = spline->GetSplinePoint(t);
				DEBUGDrawLine(last, next);
				last = next;
			}

			for (uint32 i = 0; i < spline->waypoints.count; i++)
			{
				DEBUGDrawSphere(CreateSphere(spline->waypoints[i].position, 0.1f));
			}

			for (real32 t = 0.0f; t < spline->waypoints.count; t += 0.25f)
			{
				Vec3f p = spline->GetSplinePoint(t);
				Vec3f n = spline->GetSplineNormal(t);
				DEBUGDrawLine(p, p + n);
			}
		}
	}

}

