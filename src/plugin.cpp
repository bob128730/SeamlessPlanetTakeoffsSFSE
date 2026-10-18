#include "config.h"
#include "plugin.h"
#include "transition.h"
#include "atmosphereHeight.h"

config g_settings;
RE::BGSLocation* prevLocation;
RE::BGSLocation* newLocation;
RE::NiMatrix3 takeoffRot;
RE::NiMatrix3 landedRot;
BobbyRE::atmosphereRenderSettings* atmosphereSettings;
BobbyRE::atmosphereRenderSettings* spaceAtmosphereSettings;

RE::TESImageSpace::ImageSpaceSettings* g_spaceImageSpaceSettings;
RE::TESImageSpace::ImageSpaceSettings* g_surfaceImageSpaceSettings;
RE::TESImageSpace::ImageSpaceSettings* g_originalSurfaceImageSpaceSettings;

BobbyRE::NiPoint4_double cachedPos;
BobbyRE::NiPoint4_double g_targetCellPos;
int g_index;

bool g_frameDraw = true;

void updateDiscoveryInfo(RE::TESObjectREFR* ship) 
{
	BobbyRE::PlayerCharacter::UpdatePlanetDiscoveryStatus(ship->GetSpaceshipPilot(), newLocation);

	REL::Relocation<uintptr_t*>unkGlobal{ REL::ID(938414) };

	BobbyRE::DBInfo* currentStarInfo = *(BobbyRE::DBInfo**)(*unkGlobal.get() + 0x58);

	BobbyRE::updateStarDiscoveryStatus(nullptr, currentStarInfo->FormID, 1);
}

void manualLoadSystem(RE::TESObjectREFR* ship)
{
	BobbyRE::TESObjectREFR::GetRotationMatrix(ship, &takeoffRot);

	RE::TESObjectCELL* parentCell = ship->parentCell;

	BobbyRE::TESObjectCELL::RemoveReference(parentCell, ship, false);
	prevLocation = ship->GetCurrentLocation();

	RE::TESObjectCELL* GalaxyCell = (RE::TESObjectCELL*)RE::TESObjectCELL::LookupByID(0x18343);

	ship->SetParentCell(GalaxyCell);
	BobbyRE::TESObjectREFR::PlaceInStarSystem(ship, parentCell, 0, 1);
	newLocation = ship->GetCurrentLocation();

	RE::NiPoint3 a3{ 0,0,0 };

	BobbyRE::TES::SetInteriorCell(RE::TES::GetSingleton(), RE::PlayerCharacter::GetSingleton()->parentCell, &a3, 1);
	ship->SetParentCell(0);
	
	BobbyRE::TESObjectCELL::AddReference(GalaxyCell, ship, 0, 0);

	if (prevLocation != newLocation)
	{
		_InterlockedExchangeAdd64((volatile long long*)&ship->refCount, 0x80000200001);
		BobbyRE::unkfunc(&ship, prevLocation, newLocation);
		ship->DecRefCount();
		void* v206 = BobbyRE::unkfunc2();
		int v456[8];
		void* v207 = BobbyRE::unkfunc3(v456, ship, prevLocation, newLocation, 0);

		BobbyRE::unkfunc4(v206, 4, v207);
	}
}

template <typename componentType> componentType *getComponent(BobbyRE::DBLookupContext* ctx)
{
	uint32_t leaf = ctx->depth - 1;
	uintptr_t page = ctx->pages[leaf];

	uint32_t dataOffset = *(uint32_t*)(page + 4);
	uint16_t recordOffset =
		*(uint16_t*)(page +
			dataOffset +
			2 * (ctx->indices[leaf] + 1));

	uintptr_t record =
		page +
		dataOffset +
		recordOffset;

	componentType *component = (componentType *)(record + 16);
	return component;
}

BobbyRE::PlanetComponent* getPlanetComponent(uint32_t formID, BobbyRE::DBInfo *DBInfo)
{
	uint16_t pluginIdx = *REL::Relocation<uint16_t *>(REL::ID(937669)).get();
	uint64_t componentID = (formID | (uint64_t)pluginIdx << 32) << 16;

	uintptr_t moduleState = *REL::Relocation<uintptr_t*>(REL::ID(938414)).get();


	BobbyRE::DBLookupContext ctx{ {0}, {0}, 0, 0, 0 };
	BobbyRE::DBInfo* context = *(BobbyRE::DBInfo**)(moduleState + 0x58);
	uint64_t out[4] = { 0 };

	context->traverseComponentTree(&ctx, &componentID);
	BobbyRE::PlanetComponent* result = getComponent<BobbyRE::PlanetComponent>(&ctx);

	return result;

}

RE::BGSAtmosphere* getPlanetAtmosphere(void* a1)
{
	using func_getAtmosphere_t = RE::BGSAtmosphere* (void*);
	REL::Relocation<func_getAtmosphere_t>getAtmosphere{ REL::ID(51351) };

	return getAtmosphere(a1);
}

void extendTakeoffAnim_pos(RE::TESObjectREFR* ship, float t)
{
	float targetPosFwd = g_takeoffState.endAnimPos.z + 15000;//atmosphereSettings->surfaceRadius * 0.046429;//210000;
	float e = powf(t, 3);
	RE::NiPoint3 pos = g_takeoffState.endAnimPos
		+ g_takeoffState.endFwd * (targetPosFwd * e);

	ship->data.location = pos;
	using func_update3DPos_t = void(RE::TESObjectREFR*, bool);
	REL::Relocation<func_update3DPos_t>update3DPos{ REL::ID(63181) };

	update3DPos(ship, true);
}

void extendTakeoffAnim_rot(RE::TESObjectREFR* ship, float t)
{
	float targetAngleY = g_takeoffState.endAnimAngle.y + 0.05;
	float targetAngleZ = g_takeoffState.endAnimAngle.z - 0.05;

	ship->data.angle.x = g_takeoffState.endAnimAngle.x;
	ship->data.angle.y = lerp(g_takeoffState.endAnimAngle.y, targetAngleY, t);
	ship->data.angle.z = lerp(g_takeoffState.endAnimAngle.z, targetAngleZ, t);

}

void togglePlanetDraw(bool enable)
{

	uintptr_t updateCelestialBodies = REL::Relocation<uintptr_t>(REL::ID(69330)).address();
	void* address = (void*)(updateCelestialBodies + 0x8bd);

	int8_t jmp[2] = { 0xEB, 0x04 };
	int8_t jz[2] = { 0x0F, 0x84 };

	if (enable)
	{
		REL::WriteSafeData(address, jmp);
		atmosphereSettings->limitRays = 1;
	}
	else
	{
		REL::WriteSafeData(address, jz);
		atmosphereSettings->limitRays = 0;
	}	
}
BobbyRE::atmosphereRenderSettings* getAtmosphereSettings(int index)
{
	uintptr_t storageTable = *REL::Relocation<uintptr_t*>(REL::ID(837415)).get();

	// most beautiful code ever written
	uint64_t base = *(uint64_t*)(storageTable + 272);
	uint32_t idx = *(uint32_t*)(*(uint64_t*)(base + 64LL) + 4 * index);
	return (BobbyRE::atmosphereRenderSettings*)(*(uint64_t*)(base + 968LL) + 192LL * (uint64_t)idx);
}

//SLOP :(
BobbyRE::atmosphereRenderSettings* getCurrentAtmosphereSettings()
{
	auto owner = *REL::Relocation<uintptr_t*>(REL::ID(936470)).get();   
	if (!owner) return nullptr;
	const uint32_t h = *(uint32_t*)(owner + 0x60);
	if (h == 0xFFFFFF) return nullptr;

	uintptr_t storage = *REL::Relocation<uintptr_t*>(REL::ID(837415)).get(); 
	if ((h & 0xFFFFFF) >= 0x40 || (h >> 24) == 0 ||
		(h >> 24) != *(uint8_t*)(*(uintptr_t*)(storage + 16) + (h & 0xFFFFFF)))
		return nullptr;
	return getAtmosphereSettings(h & 0xFFFFFF);
}
//END OF SLOP :)

void* getSpaceLut()
{
	RE::TextureUtils::TextureId id;
	BobbyRE::GetResourceID(&id, "Textures\\Effects\\LUTs\\LGT_LUT_Space_curve.dds");

	void* lut = nullptr;
	BobbyRE::GetTextureDB(&id, &lut);
	return lut;
};

BobbyRE::NiPoint4_double computeTargetSpacePos(float offset)
{
	uint32_t planetID = BobbyRE::BGSPlanet::Manager::GetSingleton()->planetID;
	BobbyRE::PlanetComponent* c = getPlanetComponent(planetID, nullptr);
	double p[4]; float up[3];
	BobbyRE::GetPlayerSystemPos(p, up);
	const double rCutD = double(c->radius) + g_takeoffState.maxZ + g_takeoffState.fakeHeight - 50000.0;
	const double r = rCutD + offset;

	BobbyRE::NiPoint4_double result;
	result.x = c->X + up[0] * r;
	result.y = c->Y + up[1] * r;
	result.z = c->Z + up[2] * r;
	result.w = c->W;
	return result;
}
class TakeOffEventSink : public RE::BSTEventSink<RE::Spaceship::TakeOffEvent> 
{
	using func_playerFastTravel_t = bool(RE::PlayerCharacter*, void*, RE::NiPoint3*, RE::TESObjectCELL*, bool, bool, bool, bool, uint16_t);

	RE::BSEventNotifyControl ProcessEvent(const RE::Spaceship::TakeOffEvent& event, RE::BSTEventSource<RE::Spaceship::TakeOffEvent>* a_source)
	{
		RE::TESObjectREFR* ship = event.ship.get();

		if (RE::PlayerCharacter::GetSingleton()->GetSpaceship() == ship)
		{
			if (g_takeoffState.state == NOT_STARTED) //npc ships send extra event?
			{
				REX::INFO("Player takeoff event");
				REX::INFO("State: {}", event.state);
				if (event.state == 0)
				{
					atmosphereSettings = getCurrentAtmosphereSettings();//getAtmosphereSettings(0); 
					RE::Sky* sky = RE::TES::GetSingleton()->sky;

					g_takeoffState.atmosphereForm = getPlanetAtmosphere(BobbyRE::BGSPlanet::Manager::GetSingleton()->char110);

					g_takeoffState.originalStarVisibility = g_takeoffState.atmosphereForm->settings.stars.staticVisibility;
					g_takeoffState.originalDisableSimulatedVisibility = g_takeoffState.atmosphereForm->settings.stars.disableSimulatedVisibility;
					g_takeoffState.originalAtmosphereTopRadius = atmosphereSettings->atmosphereTopRadius;
					g_takeoffState.weatherPctOverride = 0.0;
					g_takeoffState.origialCamZ = RE::PlayerCamera::GetSingleton()->cameraRoot->world.translate.z;;

					g_takeoffState.state = TAKEOFF_ANIM_STARTED;
					
					BobbyRE::PlayerCamera::SetShipFreeLook(RE::PlayerCamera::GetSingleton(), false);

					if (sky->currentWeather != (RE::BGSWeatherSettingsForm*)RE::TESForm::LookupByID(0x3D2491)) //WeatherNone
					{
						BobbyRE::Sky::SetActiveWeather(sky, (RE::BGSWeatherSettingsForm *)RE::TESForm::LookupByID(0x2B07E), 0, 0); //clear weather
					}

					BobbyRE::TESObjectREFR::GetRotationMatrix(ship, &landedRot);

					RE::TESObjectCELL* GalaxyCell = (RE::TESObjectCELL*)RE::TESObjectCELL::LookupByID(0x18343);
					g_spaceImageSpaceSettings = BobbyRE::TESObjectCELL::GetIss(GalaxyCell);

					g_takeoffState.spaceLUT = getSpaceLut();
					togglePlanetDraw(true);
				}
			}
		}
		return RE::BSEventNotifyControl::kContinue;
	}
};

namespace hooks
{
	uint32_t g_planetFormID;
	BobbyRE::DBInfo** g_DBInfo;

	bool markedCell;

	void hook_endFrame(uintptr_t execList, void* waitHandler)
	{
		if (!g_frameDraw)
			*(uint32_t*)(execList + 0x210) = 0;

		original_endFrame(execList, waitHandler);
	}

	bool hook_updateRelative(void* ctx, uintptr_t entry)
	{
		bool r = original_updateRelative(ctx, entry);

		if ((g_takeoffState.state == TAKEOFF_ANIM_STARTED || g_takeoffState.state == TAKEOFF_LOAD_STARTED)
			&& RE::TES::GetSingleton()->sky->planetID == *(uint32_t*)(entry + 0x50))
		{
			double atmoRadius = atmosphereSettings->surfaceRadius; 
			*(double*)(entry + 0x48) = atmoRadius;
			*(double*)(entry + 0x40) = atmoRadius + RE::PlayerCamera::GetSingleton()->cameraRoot->world.translate.z - 100 + g_takeoffState.fakeHeight;
		}
		return r;
	}

	void* hook_convertClouds(void* ctx, std::byte* out, void* src)
	{
		void* result = original_convertClouds(ctx, out, src);

		if (g_takeoffState.state == TAKEOFF_ANIM_STARTED)
		{
			const float drop = g_takeoffState.cloudDrop;
			float camZ = RE::PlayerCamera::GetSingleton()->cameraRoot->world.translate.z;
			float dZ = camZ - g_takeoffState.origialCamZ;

			for (int i = 0; i < 4; ++i) {
				std::byte* p = out + 0x200 + 112 * i;
				*(float*)(p + 0x5c) = lerp(0.5, 0, g_takeoffState.fadeTimer / 5); // alpha

				// whack things happen if i drop the height on its own
				float* elevation = (float*)(p + 0x28);
				*elevation = max((*elevation - dZ - drop), 1.0);
			}
			for (int i = 0; i < 4; ++i) {
				std::byte* l = out + 128 * i;
				*(float*)(l + 0x28) -= drop;
			}
		}
		return result;
	}

	thread_local byte* t_pass;
	uint64_t hook_cloudCardRenderExecute(byte* pass, void* ctx, void* a3)
	{
		t_pass = pass;
		uint64_t result = original_cloudCardRenderExecute(pass, ctx, a3);
		*(float*)(pass + 440) += g_takeoffState.cloudDrop;
		t_pass = nullptr;
		return result;
	}
	
	void* hook_cloudAllocConstants(uint64_t* set, uint16_t slot, void* alloc, void* a4, int32_t size)
	{
		if (t_pass && g_takeoffState.state == TAKEOFF_ANIM_STARTED && g_takeoffState.cloudDrop != 0)
		{
			*(float *)(t_pass + 440) += g_takeoffState.cloudDrop;
		}
		return atmosphereHeight::AllocConstants(set, slot, alloc, a4, size);
	}

	BobbyRE::PlanetComponent* planetComponent;
	float rCut;
	bool g_encounterMoved;
	double        g_encounterDelta[3];
	RE::NiMatrix3 g_encounterRot;

	//-------------------------------------------------------------------------------------------------
	// SLOP :(
	void* hook_encounterSpawnPos(BobbyRE::RefSystemTransform* t, RE::TESObjectREFR* ref)
	{
		void* result = original_getRefSystemTransform(t, ref);
		g_encounterMoved = false;

		if (g_takeoffState.state != NOT_STARTED && t->valid &&
			ref == RE::PlayerCharacter::GetSingleton()->GetSpaceship())
		{
			const double dx = g_targetCellPos.x - t->pos.x;
			const double dy = g_targetCellPos.y - t->pos.y;
			const double dz = g_targetCellPos.z - t->pos.z;
			const double d = sqrt(dx * dx + dy * dy + dz * dz);
			const double s = d > 0.0 ? 60000.0 / d : 0.0;

			g_encounterDelta[0] = dx * s;
			g_encounterDelta[1] = dy * s;
			g_encounterDelta[2] = dz * s;
			t->pos.x += g_encounterDelta[0];
			t->pos.y += g_encounterDelta[1];
			t->pos.z += g_encounterDelta[2];
			g_encounterRot = t->rot;
			g_encounterMoved = true;
		}
		return result;
	}

	void* hook_encounterParentOffset(RE::TESObjectREFR* ref, RE::TESObjectCELL** cell, float* localPos, RE::NiMatrix3* localRot, bool useGivenCell)
	{
		void* result = original_getRefSpaceCellLocal(ref, cell, localPos, localRot, useGivenCell);
		if (g_encounterMoved)
		{
			for (int i = 0; i < 3; ++i)
				localPos[i] += (float)(g_encounterRot[i][0] * g_encounterDelta[0] +
					g_encounterRot[i][1] * g_encounterDelta[1] +
					g_encounterRot[i][2] * g_encounterDelta[2]);
			g_encounterMoved = false;
		}
		return result;
	}
	// END OF SLOP :)
	//-------------------------------------------------------------------------------------------------
	bool hook_updateSpaceLocation(uint32_t objectFormID, BobbyRE::NiPoint4_double* pos, RE::NiMatrix3* rot)
	{
		if (objectFormID != RE::PlayerCharacter::GetSingleton()->GetSpaceship()->formID)
		{
			if (g_takeoffState.state != NOT_STARTED && !markedCell)
			{
				REX::INFO("Moving space cell location");

				*pos = computeTargetSpacePos(0);
			}
			else
			{
				markedCell = false;
			}
		}
		return original_updateSpaceLocation(objectFormID, pos, rot);
	}

	void* hook_setSpacePlanetOrbit(va_list a1, BobbyRE::DBInfo** DBInfo, void* a3, uint32_t planetFormID, double* planetLocation, RE::NiMatrix3* rot, BobbyRE::NiPoint4_double* pos, uint32_t a8, bool a9)
	{
		g_planetFormID = planetFormID;
		g_DBInfo = DBInfo;
		void *result = original_setSpacePlanetOrbit(a1, DBInfo, a3, planetFormID, planetLocation, rot, pos, a8, a9);

		if (g_takeoffState.state == TAKEOFF_LOAD_STARTED)
		{	
			BobbyRE::BGSPlanet::Manager* planetManager = BobbyRE::BGSPlanet::Manager::GetSingleton();

			float lat = planetManager->tileLatitude;
			float lon = planetManager->tileLongitude;

			RE::NiPoint3 localUp(cos(lat) * cos(lon), cos(lat) * sin(lon), sin(lat));
			RE::NiPoint3 localNorth(-sin(lat) * cos(lon), -sin(lat) * sin(lon), cos(lat));
			RE::NiPoint3 localEast(-sin(lon), cos(lon), 0);

			planetComponent = getPlanetComponent(planetFormID, *DBInfo);
			RE::NiMatrix3 planetRot = planetComponent->rotation;

			RE::NiPoint3 worldUp = planetRot.Transpose() * localUp;
			RE::NiPoint3 worldNorth = planetRot.Transpose() * localNorth;
			RE::NiPoint3 worldEast = planetRot.Transpose() * localEast;

			REX::INFO("planetLocation: {}, {}, {}", planetLocation[0], planetLocation[1], planetLocation[2]);
			*pos = computeTargetSpacePos(0);

			cachedPos = *pos;

			REX::INFO("final pos: {}, {}, {}", pos->x, pos->y, pos->z);

			RE::NiMatrix3 surfaceMat(
				{ worldEast.x,  worldEast.y,  worldEast.z,  0 },
				{ worldNorth.x, worldNorth.y, worldNorth.z, 0 },
				{ worldUp.x,    worldUp.y,    worldUp.z,    0 }
			);

			RE::NiMatrix3 flatRot(
				{ landedRot[0][0], landedRot[0][1], 0, 0 },
				{ landedRot[1][0], landedRot[1][1], 0, 0 },
				{ 0, 0, 1, 0 }
			);

			RE::NiMatrix3 delta = takeoffRot * flatRot.Transpose();

			RE::NiPoint3 x = landedRot * RE::NiPoint3(1, 0, 0);
			RE::NiPoint3 y = landedRot * RE::NiPoint3(0, 1, 0);
			RE::NiPoint3 z = landedRot * RE::NiPoint3(0, 0, 1);

			*rot = delta * flatRot * surfaceMat;
		}
		return result;
	}

	void hook_PCUpdate(RE::PlayerCharacter* player, float dt)
	{
		switch (g_takeoffState.state)
		{
		case NOT_STARTED:
		{
			break;
		}
		case TAKEOFF_ANIM_STARTED:
		{
			if (g_takeoffState.prefadeTimer < 9.0)
			{
				g_takeoffState.prefadeTimer += dt;
				if (g_takeoffState.prefadeTimer > 9.0)
				{
					REX::INFO("fade start");
				}
			}
			else
			{
				fadeWeather(g_takeoffState.fadeTimer / (5 + g_settings.TakeoffExtensionLength * 0.9f), &g_takeoffState);
				fadeAtmospherics(g_takeoffState.fadeTimer / (5 + g_settings.TakeoffExtensionLength), &g_takeoffState, atmosphereSettings);

				g_takeoffState.issT = g_takeoffState.fadeTimer / (5 + g_settings.TakeoffExtensionLength);

				g_takeoffState.fadeTimer += dt;

				//vanilla anim complete at around 4.9 seconds
				if (g_takeoffState.fadeTimer > 5)
				{
					RE::TESObjectREFR* ship = player->GetSpaceship();
					if (g_takeoffState.endAnimAngle == RE::NiPoint3{0.0, 0.0, 0.0})
					{
						g_takeoffState.endAnimAngle = ship->data.angle;
						g_takeoffState.endAnimPos   = ship->data.location;

						RE::NiMatrix3 rot;

						BobbyRE::TESObjectREFR::GetRotationMatrix(ship, &rot);
						g_takeoffState.endFwd = rot.Transpose() * RE::NiPoint3(0, 1, 0);
					}

					if (g_settings.TakeoffExtensionLength > 1.0)
					{
						extendTakeoffAnim_rot(ship, (g_takeoffState.fadeTimer - 5) / g_settings.TakeoffExtensionLength);
						byte* shaker = *REL::Relocation<byte**>(REL::ID(938713));
						byte* perlin = *reinterpret_cast<byte**>(shaker + 0x70);

						if (((g_takeoffState.fadeTimer - 5) / g_settings.TakeoffExtensionLength) < 0.25)
						{
							float t = (g_takeoffState.fadeTimer - 5) / (g_settings.TakeoffExtensionLength / 4);
							extendTakeoffAnim_pos(ship, t);

							if (g_settings.EnableCameraShake)
							{
								*(float*)(shaker + 0x7C) = lerp(0, 50, t);
								*(double*)(perlin + 0x10) = lerp(0, 16, t);
							}
						}
						else
						{
							float t = ((g_takeoffState.fadeTimer - 5) - g_settings.TakeoffExtensionLength * 0.25) / (g_settings.TakeoffExtensionLength * 0.75);
							g_takeoffState.fakeHeight = lerp(0, atmosphereSettings->surfaceRadius * 0.06, t);
							g_takeoffState.cloudDrop = g_takeoffState.fakeHeight;
							atmosphereHeight::setStarHorizon(atmosphereSettings, g_takeoffState.fakeHeight);

							if (g_settings.EnableCameraShake)
							{
								*(float*)(shaker + 0x7C) = lerp(50, 0, t);
								*(double*)(perlin + 0x10) = lerp(16, 0, t);
							}
						}
					}
				}

				if (g_takeoffState.fadeTimer > (5 + g_settings.TakeoffExtensionLength - 0.5) && g_settings.EnableLoadBlur)
				{
					fadeBlur((g_takeoffState.fadeTimer - (5 + g_settings.TakeoffExtensionLength - 0.5)) 
								/ 0.5
								, true, &g_takeoffState);
				}

				if (g_takeoffState.fadeTimer > 5 + g_settings.TakeoffExtensionLength)
				{
					g_takeoffState.state = TAKEOFF_LOAD_STARTED;
					REX::INFO("fade complete");
				}
			}
			break;
		}
		case TAKEOFF_LOAD_STARTED:
		{
			RE::TESObjectREFR* ship = player->GetSpaceship();

			g_frameDraw = false;

			g_takeoffState.maxZ = ship->data.location.z;
			g_targetCellPos = computeTargetSpacePos(10000);

			manualLoadSystem(ship);
			RE::TES::GetSingleton()->sky->mode = 0;
			g_takeoffState.state = TAKEOFF_LOAD_COMPLETE;
			togglePlanetDraw(false);
			g_takeoffState.fakeHeight = 0;

			break;
		}
		case TAKEOFF_LOAD_COMPLETE:
		{
			if (g_takeoffState.arrivalTimer < 0.05)
			{
				g_takeoffState.arrivalTimer += dt;
				if (g_takeoffState.arrivalTimer > 0.05)
					g_frameDraw = true;
			}
			else if (g_takeoffState.arrivalTimer < 0.3)
			{
				g_takeoffState.arrivalTimer += dt;
				if (g_takeoffState.arrivalTimer > 0.3)
				{
					spaceAtmosphereSettings = getAtmosphereSettings(g_index);

					g_takeoffState.originalStarGlow = spaceAtmosphereSettings->surfaceRadius;
					spaceAtmosphereSettings->surfaceRadius = 0;
					RE::TES::GetSingleton()->sky->mode = 1;
					REX::INFO("Star glow fade start");
				}
			}
			else if (g_takeoffState.arrivalTimer < 2.5)
			{
				g_takeoffState.arrivalTimer += dt;
				fadeStarGlow(g_takeoffState.arrivalTimer / 2.5, &g_takeoffState, spaceAtmosphereSettings);
				if(g_settings.EnableLoadBlur)
					fadeBlur((g_takeoffState.arrivalTimer - 0.3) / 2.2, false, &g_takeoffState);
			}
			else
			{
				REX::INFO("star glow fade complete");
				g_takeoffState.state = NOT_STARTED;
				g_takeoffState.prefadeTimer = 0.0;
				g_takeoffState.fadeTimer = 0.0;
				g_takeoffState.arrivalTimer = 0.0;

				g_takeoffState.endAnimAngle = RE::NiPoint3{ 0.0, 0.0, 0.0 };
				g_takeoffState.endAnimPos   = RE::NiPoint3{ 0.0, 0.0, 0.0 };
				g_takeoffState.cloudDrop = 0;
				g_takeoffState.issT = 0;

				g_takeoffState.atmosphereForm->settings.stars.staticVisibility = g_takeoffState.originalStarVisibility;
				g_takeoffState.atmosphereForm->settings.stars.disableSimulatedVisibility = g_takeoffState.originalDisableSimulatedVisibility;

				if (g_surfaceImageSpaceSettings)
				{
					free((void *)g_surfaceImageSpaceSettings);
					g_surfaceImageSpaceSettings = 0;
				}
			}
			break;
		}
		}
		original_PCUpdate(player, dt);
	}

	void *hook_forEachRef(void* a1, void* a2, bool a3)
	{
		if (g_takeoffState.state != NOT_STARTED)
		{
			markedCell = true;
		}
		return original_forEachRef(a1, a2, a3);
	}

	bool hook_unkFunc(int* a1)
	{
		g_index = *a1 & 0xFFFFFF;
		return original_unkFunc(a1);
	}

	void *hook_unkFunc2(void* a1, RE::TESImageSpace::ImageSpaceSettings *settings)
	{
		if (g_takeoffState.state == TAKEOFF_ANIM_STARTED || g_takeoffState.state == TAKEOFF_LOAD_STARTED)
		{
			if (!g_surfaceImageSpaceSettings)
			{
				g_surfaceImageSpaceSettings = (RE::TESImageSpace::ImageSpaceSettings*)malloc(sizeof(RE::TESImageSpace::ImageSpaceSettings));
				if (g_surfaceImageSpaceSettings)
				{
					*g_surfaceImageSpaceSettings = *settings;
					//*g_originalSurfaceImageSpaceSettings = *g_surfaceImageSpaceSettings;
				}
			}
			else
			{
				BobbyRE::IssLerp(settings, settings, g_spaceImageSpaceSettings, g_takeoffState.issT);

				float a = g_surfaceImageSpaceSettings->SunAndSky.CelestialBodyIlluminanceScaleOverride.value;
				settings->SunAndSky.CelestialBodyIlluminanceScaleOverride.value = lerp(a, a * 0.2, g_takeoffState.issT);

				byte* mgr = *REL::Relocation<byte**>(REL::ID(883711)).get();  // imageSpaceManager
				float* weights = (float*)(mgr + 0x410);
				void** slots = (void**)(mgr + 0x420);

				if (g_takeoffState.fadeTimer > 5)
				{
					float t = min(((g_takeoffState.fadeTimer - 5) / g_settings.TakeoffExtensionLength), 1);

					for (int i = 0; i < 3; ++i)
						weights[i] *= (1.0f - t);

					void* old = slots[3];
					if (old != g_takeoffState.spaceLUT) {
						_InterlockedIncrement((volatile long*)((byte*)g_takeoffState.spaceLUT + 0x14));
						slots[3] = g_takeoffState.spaceLUT;
						if (old)
							(*(void(**)(void*))(*(uintptr_t*)old + 8))(old);
					}
					weights[3] = t;
				}
			}
		}
		if (g_takeoffState.state != NOT_STARTED)
			settings->Blur.BlurRadiusValue.value = g_takeoffState.blurValue;

		return original_unkFunc2(a1, settings);
	}

	float hook_calcWeatherPct(RE::Sky* sky)
	{
		if (g_takeoffState.state == TAKEOFF_ANIM_STARTED || g_takeoffState.state == TAKEOFF_LOAD_STARTED)
			return g_takeoffState.weatherPctOverride;
		return original_calcWeatherPct(sky);
	}

	double hook_minOrbitKm(uint32_t planetID)
	{
		//if (g_takeoffState.state != NOT_STARTED)
		//{
			if (planetID != 0 && planetID == g_planetFormID)
			{
				return 0.0;
			}
		//}
		return original_minOrbitKm(planetID);
	}

	bool hook_setFlightInputs(float* in, void* a2)
	{
		if (g_takeoffState.state == TAKEOFF_ANIM_STARTED && g_takeoffState.fadeTimer > 4.9)
		{
			in[4] = 2;
		}
		return original_setFlightInputs(in, a2);
	}

	void hook_fade(bool, bool, float, bool, float, void* cb, bool, std::uint32_t)
	{
		if (!cb)
			return;
		uintptr_t *vtbl = *(uintptr_t **)(cb);
		reinterpret_cast<void (*)(void*)>(vtbl[1])(cb);
		reinterpret_cast<void (*)(void*, bool)>(vtbl[0])(cb, true);
	}

	void install() {
		uintptr_t addr1 = REL::Relocation<uintptr_t>(REL::ID(97772)).address();
		uintptr_t addr2 = REL::Relocation<uintptr_t>(REL::ID(57659)).address();
		uintptr_t addr3 = REL::Relocation<uintptr_t>(REL::ID(57653)).address();
		uintptr_t addr4 = REL::Relocation<uintptr_t>(REL::ID(57659)).address();
		uintptr_t addr5 = REL::Relocation<uintptr_t>(REL::ID(99411)).address();
		uintptr_t addr6 = REL::Relocation<uintptr_t>(REL::ID(99415)).address();
		uintptr_t addr7 = REL::Relocation<uintptr_t>(REL::ID(57653)).address();
		uintptr_t addr8 = REL::Relocation<uintptr_t>(REL::ID(69308)).address();
		uintptr_t addr9 = REL::Relocation<uintptr_t>(REL::ID(144186)).address();
		uintptr_t addr10 = REL::Relocation<uintptr_t>(REL::ID(99417)).address();
		uintptr_t addr11 = REL::Relocation<uintptr_t>(REL::ID(56756)).address();
		uintptr_t addr12 = REL::Relocation<uintptr_t>(REL::ID(69330)).address();

		uintptr_t playerShipUpdateCall = addr1 + 0x68;
		uintptr_t updateSpaceLocationCall = addr2 + 0x101f;
		uintptr_t updateSpaceLocationCall2;

		uintptr_t setSpacePlanetOrbitCall;
		if (SFSE::GetSFSEVersion() < 131408)
		{
			setSpacePlanetOrbitCall = addr3 + 0x89e;
			updateSpaceLocationCall2 = addr7 + 0xaaa;
		}
		else
		{
			setSpacePlanetOrbitCall = addr3 + 0x8f3;
			updateSpaceLocationCall2 = addr7 + 0xb0a;
		}

		uintptr_t forEachRefCall = addr4 + 0x2aa;
		uintptr_t PCUpdateCall = addr5 + 0xe2;
		uintptr_t unkFuncCall = addr9 + 0x2e;
		uintptr_t unkFunc2Call = addr6 + 0x350;
		uintptr_t calcWeatherPctCall = addr8 + 0x242;

		uintptr_t endFrameCall = addr10 + 0x2DE;

		uintptr_t convertCloudsCall = addr11 + 0x1E9;
		uintptr_t convertCloudsCall2 = addr11 + 0x262;

		uintptr_t updateRelative = addr12 + 0xB63;

		REL::Trampoline& tramp = REL::GetTrampoline();

		original_updateSpaceLocation = (func_updateSpaceLocation_t*)tramp.write_call<5>(updateSpaceLocationCall, hook_updateSpaceLocation);
		tramp.write_call<5>(updateSpaceLocationCall2, hook_updateSpaceLocation);
		original_calcWeatherPct = (func_calcWeatherPct_t*)tramp.write_call<5>(calcWeatherPctCall, hook_calcWeatherPct);
		original_setSpacePlanetOrbit = (func_setSpacePlanetOrbit_t*)tramp.write_call<5>(setSpacePlanetOrbitCall, hook_setSpacePlanetOrbit);
		original_PCUpdate = (func_PCUpdate_t*)tramp.write_call<5>(PCUpdateCall, hook_PCUpdate);
		original_forEachRef = (func_forEachRef_t*)tramp.write_call<5>(forEachRefCall, hook_forEachRef);
		original_unkFunc = (func_unkFunc_t*)tramp.write_call<5>(unkFuncCall, hook_unkFunc);
		original_unkFunc2 = (func_unkFunc2_t*)tramp.write_call<5>(unkFunc2Call, hook_unkFunc2);
		original_endFrame = (func_endFrame_t*)tramp.write_call<5>(endFrameCall, hook_endFrame);

		uintptr_t spawnSpaceCellAtRef = REL::Relocation<uintptr_t>(REL::ID(57645)).address();
		original_getRefSystemTransform = (func_getRefSystemTransform_t*)tramp.write_call<5>(spawnSpaceCellAtRef + 0x58, hook_encounterSpawnPos);

		original_getRefSpaceCellLocal = (func_getRefSpaceCellLocal_t*)tramp.write_call<5>(spawnSpaceCellAtRef + 0x17A, hook_encounterParentOffset);
		
		original_convertClouds = (func_convertClouds_t*)tramp.write_call<5>(convertCloudsCall, hook_convertClouds);
		tramp.write_call<5>(convertCloudsCall2, hook_convertClouds);

		original_updateRelative = (func_updateRelative_t*)tramp.write_call<5>(updateRelative, hook_updateRelative);
		
		func_minOrbitKm_t **slot = REL::Relocation<func_minOrbitKm_t**>{ REL::ID(1141031) }.get();
		original_minOrbitKm = *slot ? *slot
			: REL::Relocation<func_minOrbitKm_t*>{ REL::ID(1015279) }.get();
		*slot = &hook_minOrbitKm;

		auto* vt = REL::Relocation<std::uintptr_t*>(REL::ID(496810)).get();   // CloudCardsRenderPass vftable
		original_cloudCardRenderExecute = (func_cloudCardRenderExecute_t*)(vt[7]);                      // 0x142B0CB90
		REL::WriteSafeData((std::uintptr_t)(vt + 7), &hook_cloudCardRenderExecute);

		(atmosphereHeight::func_allocConst_t*)tramp.write_call<5>(REL::ID(145432).address() + 0x5A7, hook_cloudAllocConstants);
		tramp.write_call<5>(REL::ID(93823).address() + 0x78, hook_fade);

		original_setFlightInputs = (func_setFlightInputs_t *)tramp.write_call<5>(REL::ID(119859).address() + 0x572, hook_setFlightInputs);

		atmosphereHeight::hooks::install(&tramp);
	}

}

void OnMessage(SFSE::MessagingInterface::Message* message) 
{
	if (message->type == SFSE::MessagingInterface::kPostDataLoad) 
	{
		REX::INFO("Init");
		g_settings.load();
		g_takeoffState = {};

		TakeOffEventSink* TakeOffSink = new TakeOffEventSink();
		auto source = RE::Spaceship::TakeOffEvent::GetEventSource();
		source->RegisterSink(TakeOffSink);

		//stop normal loading
		uintptr_t initiateTakeOffCompleted = REL::Relocation<uintptr_t>(REL::ID(119911)).address();
		void* addCellToLoaderCall = (void *)(initiateTakeOffCompleted + 0x354);

		int8_t nopcall[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		REL::WriteSafeData(addCellToLoaderCall, nopcall);

		//prevent the unload function from showing the load screen
		uintptr_t unloadCurrentLocation = REL::Relocation<uintptr_t>( REL::ID(46037)).address();
		byte* shouldStartLoadScreen = (byte*)(unloadCurrentLocation + 0x9b4);

		byte jmp = 0xEB;

		REL::WriteSafeData(shouldStartLoadScreen, jmp);

		if (g_settings.DisableTakeOffCam) 
		{
			uintptr_t initiateTakeOffSequence = REL::Relocation<uintptr_t>(REL::ID(119908)).address();
			byte* shouldStartTakeOffCam = (byte*)(initiateTakeOffSequence + 0x1c9);

			REL::WriteSafeData(shouldStartTakeOffCam, jmp);

			uintptr_t startTakeoff = REL::Relocation<uintptr_t>(REL::ID(119907)).address();
			byte* inputLayerCall = (byte*)(startTakeoff + 0x2A3);
			byte* hudModePushCall = (byte*)(startTakeoff + 0x28E);
		
			REL::WriteSafeData(inputLayerCall, nopcall);
			REL::WriteSafeData(hudModePushCall, nopcall);

			REL::WriteSafeData(REL::ID(93824).address() + 0x1AF, nopcall);
		}

		hooks::install();
	}
}