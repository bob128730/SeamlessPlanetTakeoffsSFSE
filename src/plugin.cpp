#include "BobbyRE/BobbyRE.h"
#include "config.h"
#include "plugin.h"
#include "transition.h"

config settings;
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
int g_index;

bool g_frameDraw = true;

takeoffState g_takeoffState;

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

	BobbyRE::PlayerCamera::SetShipFreeLook(RE::PlayerCamera::GetSingleton(), false);
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

void extendTakeoffAnim(RE::TESObjectREFR* ship, float t)
{
	float targetAngleY = g_takeoffState.endAnimAngle.y + 0.05;
	float targetAngleZ = g_takeoffState.endAnimAngle.z - 0.05;
	float targetPosZ   = g_takeoffState.endAnimPos.z + settings.TakeoffExtensionLength * 1000;
	float targetPosFwd = g_takeoffState.endAnimPos.z + settings.TakeoffExtensionLength * 3000;


	ship->data.angle.y = lerp(g_takeoffState.endAnimAngle.y, targetAngleY, t);
	ship->data.angle.z = lerp(g_takeoffState.endAnimAngle.z, targetAngleZ, t);

	float e = t * t * (3.0f - 2.0f * t);
	RE::NiPoint3 pos = g_takeoffState.endAnimPos
		+ g_takeoffState.endFwd * (targetPosFwd * e) + RE::NiPoint3(0, 0, targetPosZ * e);

	ship->data.location = pos;
	using func_update3DPos_t = void(RE::TESObjectREFR *, bool);
	REL::Relocation<func_update3DPos_t>update3DPos{ REL::ID(63181) };

	update3DPos(ship, true);

}

BobbyRE::atmosphereRenderSettings* getAtmosphereSettings(int index)
{
	uintptr_t storageTable = *REL::Relocation<uintptr_t*>(REL::ID(837415)).get();

	// most beautiful code ever written
	uint64_t base = *(uint64_t*)(storageTable + 272);
	uint32_t idx = *(uint32_t*)(*(uint64_t*)(base + 64LL) + 4 * index);
	return (BobbyRE::atmosphereRenderSettings*)(*(uint64_t*)(base + 968LL) + 192LL * (uint64_t)idx);
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
					atmosphereSettings = getAtmosphereSettings(0);
					RE::Sky* sky = RE::TES::GetSingleton()->sky;

					g_takeoffState.atmosphereForm = getPlanetAtmosphere(BobbyRE::BGSPlanet::Manager::GetSingleton()->char110);

					g_takeoffState.originalStarVisibility = g_takeoffState.atmosphereForm->settings.stars.staticVisibility;
					g_takeoffState.originalDisableSimulatedVisibility = g_takeoffState.atmosphereForm->settings.stars.disableSimulatedVisibility;
					g_takeoffState.originalAtmosphereTopRadius = atmosphereSettings->atmosphereTopRadius;
					g_takeoffState.weatherPctOverride = 0.0;

					g_takeoffState.state = TAKEOFF_ANIM_STARTED;
					
					if (sky->currentWeather != (RE::BGSWeatherSettingsForm*)RE::TESForm::LookupByID(0x3D2491)) //WeatherNone
					{
						BobbyRE::Sky::SetActiveWeather(sky, (RE::BGSWeatherSettingsForm *)RE::TESForm::LookupByID(0x2B07E), 0, 0); //clear weather
					}

					BobbyRE::TESObjectREFR::GetRotationMatrix(ship, &landedRot);
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

	void* hook_convertClouds(void* ctx, std::byte* out, void* src)
	{
		void* r = original_convertClouds(ctx, out, src);

		if (g_takeoffState.state == TAKEOFF_ANIM_STARTED)
		{
			const float drop = g_takeoffState.cloudDrop;

			for (int i = 0; i < 4; ++i) {
				std::byte* p = out + 0x200 + 112 * i;
				*(float*)(p + 0x28) -= drop;
			}
			for (int i = 0; i < 4; ++i) {
				std::byte* l = out + 128 * i;
				*(float*)(l + 0x28) -= drop;
			}
		}
		return r;
	}

	bool hook_updateSpaceLocation(uint32_t objectFormID, BobbyRE::NiPoint4_double* pos, RE::NiMatrix3* rot)
	{
		if (g_takeoffState.state != NOT_STARTED && !markedCell)
		{
			REX::INFO("Moving space cell location");

			double newPos[4];
			float newRot[3];
			BobbyRE::GetPlayerSystemPos(newPos, newRot);

			pos->x = newPos[0];
			pos->y = newPos[1];
			pos->z = newPos[2];
		}
		else
		{
			markedCell = false;
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

			BobbyRE::PlanetComponent* planetComponent = getPlanetComponent(planetFormID, *DBInfo);
			RE::NiMatrix3 planetRot = planetComponent->rotation;

			RE::NiPoint3 worldUp = planetRot.Transpose() * localUp;
			RE::NiPoint3 worldNorth = planetRot.Transpose() * localNorth;
			RE::NiPoint3 worldEast = planetRot.Transpose() * localEast;

			REX::INFO("planetLocation: {}, {}, {}", planetLocation[0], planetLocation[1], planetLocation[2]);
			double newPos[4];
			float newRot[3];
			BobbyRE::GetPlayerSystemPos(newPos, newRot);

			pos->x = newPos[0];
			pos->y = newPos[1];
			pos->z = newPos[2];

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
				fadeWeather(g_takeoffState.fadeTimer / (5 + settings.TakeoffExtensionLength * 0.9f), &g_takeoffState);
				//fadeAtmospherics(g_takeoffState.fadeTimer / (5 + settings.TakeoffExtensionLength));
				fadeImageSpaceSettings(g_takeoffState.fadeTimer / (5 + settings.TakeoffExtensionLength * 0.17), 
					g_surfaceImageSpaceSettings, g_originalSurfaceImageSpaceSettings);

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
						g_takeoffState.endFwd = rot * RE::NiPoint3(0, 1, 0);
					}

					if(settings.TakeoffExtensionLength > 1.0)
						extendTakeoffAnim(ship, (g_takeoffState.fadeTimer - 5) / settings.TakeoffExtensionLength);
				}

				if (g_takeoffState.fadeTimer > (5 + settings.TakeoffExtensionLength - 0.5) && settings.EnableLoadBlur)
				{
					fadeBlur((g_takeoffState.fadeTimer - (5 + settings.TakeoffExtensionLength - 0.5)) 
								/ 0.5
								, true, &g_takeoffState);
				}

				if (g_takeoffState.fadeTimer > 5 + settings.TakeoffExtensionLength)
				{
					g_takeoffState.state = TAKEOFF_LOAD_STARTED;
					REX::INFO("fade complete");
				}
			}
			break;
		}
		case TAKEOFF_LOAD_STARTED:
		{
			g_frameDraw = false;
			manualLoadSystem(player->GetSpaceship());
			RE::TES::GetSingleton()->sky->mode = 0;
			g_takeoffState.state = TAKEOFF_LOAD_COMPLETE;
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
				if(settings.EnableLoadBlur)
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
					*g_originalSurfaceImageSpaceSettings = *g_surfaceImageSpaceSettings;
				}
			}
			else
			{
				//causes crash sometimes?
				//*settings = *g_surfaceImageSpaceSettings;

				settings->IndirectLighting.IndirectDiffuseMultiplier.value = g_surfaceImageSpaceSettings->IndirectLighting.IndirectDiffuseMultiplier.value;
				settings->IndirectLighting.IndirectSpecularMultiplier.value = g_surfaceImageSpaceSettings->IndirectLighting.IndirectSpecularMultiplier.value;
				settings->SunAndSky.SkyLightingMultiplier.value = g_surfaceImageSpaceSettings->SunAndSky.SkyLightingMultiplier.value;
				settings->SunAndSky.StarfieldStarBrightnessScaleOverride.value = g_surfaceImageSpaceSettings->SunAndSky.StarfieldStarBrightnessScaleOverride.value;
				settings->SunAndSky.StarfieldBackgroundScaleOverride.value = g_surfaceImageSpaceSettings->SunAndSky.StarfieldBackgroundScaleOverride.value;
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

		original_convertClouds = (func_convertClouds_t*)tramp.write_call<5>(convertCloudsCall, hook_convertClouds);
		tramp.write_call<5>(convertCloudsCall2, hook_convertClouds);

	}

}

void OnMessage(SFSE::MessagingInterface::Message* message) 
{
	if (message->type == SFSE::MessagingInterface::kPostDataLoad) 
	{
		REX::INFO("Init");
		settings.load();
		g_takeoffState = {};

		g_originalSurfaceImageSpaceSettings = (RE::TESImageSpace::ImageSpaceSettings*)malloc(sizeof(RE::TESImageSpace::ImageSpaceSettings));

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

		if (settings.DisableTakeOffCam) 
		{
			uintptr_t initiateTakeOffSequence = REL::Relocation<uintptr_t>(REL::ID(119908)).address();
			byte* shouldStartTakeOffCam = (byte*)(initiateTakeOffSequence + 0x1c9);

			REL::WriteSafeData(shouldStartTakeOffCam, jmp);
		}

		hooks::install();
	}
}