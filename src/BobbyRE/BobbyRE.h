#pragma once

namespace BobbyRE 
{
	struct DBLookupContext;

	namespace TESObjectREFR
	{
		using func_GetRotationMatrix_t = RE::NiMatrix3* (RE::TESObjectREFR*, RE::NiMatrix3*);
		inline REL::Relocation<func_GetRotationMatrix_t>GetRotationMatrix{ REL::ID(63286) };

		using func_PlaceInStarSystem_t = int(RE::TESObjectREFR*, RE::TESObjectCELL*, bool, double);
		inline REL::Relocation<func_PlaceInStarSystem_t>PlaceInStarSystem{ REL::ID(102641) };
	}

	namespace PlayerCharacter
	{
		using func_UpdateDiscoveredStatus_t = void* (RE::Actor*, RE::BGSLocation*);
		inline REL::Relocation<func_UpdateDiscoveredStatus_t>UpdatePlanetDiscoveryStatus{ REL::ID(102968) };
	}

	namespace TESObjectCELL
	{
		using func_RemoveReference_t = void(RE::TESObjectCELL*, RE::TESObjectREFR*, bool);
		inline REL::Relocation<func_RemoveReference_t>RemoveReference{ REL::ID(62697) };

		using func_AddReference_t = double(RE::TESObjectCELL*, RE::TESObjectREFR*, bool, bool);
		inline REL::Relocation<func_AddReference_t>AddReference{ REL::ID(63034) };

		using func_GetIss_t = RE::TESImageSpace::ImageSpaceSettings*(RE::TESObjectCELL*);
		inline REL::Relocation<func_GetIss_t>GetIss{ REL::ID(62677) };
	}

	namespace TES
	{
		using func_SetInteriorCell_t = void* (RE::TES*, RE::TESObjectCELL*, RE::NiPoint3*, bool);
		inline REL::Relocation<func_SetInteriorCell_t>SetInteriorCell{ REL::ID(46037) };
	}

	namespace PlayerCamera
	{
		using func_SetShipFreeLook_t = void(RE::PlayerCamera*, bool);
		inline REL::Relocation<func_SetShipFreeLook_t>SetShipFreeLook{ REL::ID(113437) };
	}

	namespace Sky
	{
		using func_SetActiveWeather = void(RE::Sky*, RE::BGSWeatherSettingsForm*, bool, bool);
		inline REL::Relocation<func_SetActiveWeather>SetActiveWeather{ REL::ID(69299) };
	}

	namespace Spaceship 
	{

		struct FarTravelEvent : RE::Spaceship::FarTravelEvent
		{
			RE::NiPointer<RE::TESObjectREFR> source;
			RE::BGSLocation *fromLocation;
			RE::BGSLocation *fromLocation2;
			uint32_t unk5;
		};
	}

	struct PositionPlayerEvent 
	{
		uint32_t unk;
		bool unk4;
		bool unk5;
	};

	struct MovePlayerRelevantSpaceComponentsOnViewChange 
	{
		RE::NiAVObject** node;
		const char* fullName;
		uint32_t sourceID;
		uint32_t destPlanetID;
		int param;
	};

	struct DBInfo 
	{
		void *qword0;
		void *qword8;
		uint32_t dword10;
		uint32_t field_14;
		volatile signed __int64 refcount;
		double x;
		double y;
		double z;
		void *BSService__Jobsite;
		void *qword40;
		void *qword48;
		uint32_t FormID;

		bool traverseComponentTree(DBLookupContext* ctx, uint64_t* a2)
		{
			using func_traverseComponentTree_t = bool(DBInfo *, DBLookupContext*, uint64_t*);
			REL::Relocation<func_traverseComponentTree_t>func{ REL::ID(126584) };

			return func(this, ctx, a2);
		}

	};

	struct DBLookupContext
	{
		uintptr_t pages[4];
		uint32_t indices[4];
		uint32_t depth;
		uint32_t pad;
		DBInfo* cachedDBInfo;
	};

	struct NiPoint4_double
	{
		double x;
		double y;
		double z;
		double w;
	};

	namespace BGSPlanet
	{
		struct Manager
		{
			void* vftbl;
			uint64_t gap8;
			void* BSPauseRequester_vftbl;
			void* char18;
			uint32_t byte20;
			RE::BSReadWriteLock m_RWLock;
			byte gap2C[20];
			uint64_t qword40;
			uint64_t qword48;
			uint64_t qword50;
			uint64_t qword58;
			uint64_t qword60;
			uint64_t dword68;
			uint32_t gap6C;
			uint32_t dword70;
			uint32_t gap74;
			void* qword78;
			uint64_t qword80;
			RE::BSFixedString char88;
			uint64_t qword90;
			uint64_t qword98;
			uint64_t qwordA0;
			uint64_t qwordA8;
			uint64_t qwordB0;
			uint64_t qwordB8;
			uint64_t qwordC0;
			uint32_t byteC8;
			uint64_t qwordCC;
			uint32_t dwordD4;
			uint32_t byteD8;
			uint64_t qwordDC;
			uint32_t dwordE4;
			uint32_t byteE8;
			uint64_t qwordEC;
			void* m_BiomeMapResource;
			uint32_t gapF4;
			void* char110;
			__int64 int64118;
			LPSTR pchar120;
			__int64 int64128;
			void* pvoid130;
			__int64 int64138;
			void* pvoid140;
			void* pvoid148;
			void* pvoid150;
			void* NavMeshGen;
			RE::BSTEventDetail::SinkBase* pbsteventdetail__sinkbase160;
			void* pvoid168;
			RE::BSTArray<void*> m_PlanetModules;
			uint64_t qword180;
			uint64_t dword188;
			uint64_t qword190;
			uint64_t qword198;
			uint64_t qword1A0;
			double tileLongitude;
			double tileLatitude;
			uint64_t qword1B8;
			uint32_t dword190;
			uint32_t byte1C0;
			uint64_t qword1C4;
			uint32_t dword1CC;
			uint32_t byte1D0;
			uint64_t qword1D4;
			uint32_t dword1DC;
			uint32_t byte1E0;
			uint64_t qword1E4;
			uint64_t qword1EC;
			uint64_t dword1F4;
			uint64_t qword1F8;
			uint64_t gap200;
			uint32_t dword208;

			[[nodiscard]] static BGSPlanet::Manager* GetSingleton()
			{
				static REL::Relocation<BGSPlanet::Manager**> singleton{ REL::ID(937609) };
				return *singleton;
			}
		};

	}

	namespace BSService
	{
		struct JobSite
		{
			byte gap[2224];
			void* ComponentDB;

			[[nodiscard]] static BSService::JobSite* GetSingleton()
			{
				using func_t = decltype(&BobbyRE::BSService::JobSite::GetSingleton);
				static REL::Relocation<func_t> func{ REL::ID{ 126578 } };
				return func();
			}
		};

	}

	struct atmosphereRenderSettings
	{
		std::byte pad[0x80];
		float surfaceRadius;
		float seaRadius;
		float atmosphereTopRadius;
		float seaOffset;
		float atmosphereHeight;
		std::byte pad2[0xC];
		float unk;
		uint32_t limitRays;
		std::byte pad3[20];
		float unused;
	};

	struct PlanetComponent
	{
		RE::NiMatrix3 rotation;
		double X;
		double Y;
		double Z;
		double W;
		byte pad[120];
		float radius;
		float atmosphereHeight;
	};

	using func_updateStarDiscoveryStatus_t = void* (void*, uint32_t, bool);
	inline REL::Relocation<func_updateStarDiscoveryStatus_t>updateStarDiscoveryStatus{ REL::ID(102650) };

	using func_unkfunc_t = void* (RE::TESObjectREFR**, RE::BGSLocation*, RE::BGSLocation*);
	inline REL::Relocation<func_unkfunc_t>unkfunc{ REL::ID(64046) };

	using func_unkfunc2_t = void* ();
	inline REL::Relocation<func_unkfunc2_t>unkfunc2{ REL::ID(62173) };

	using func_unkfunc3_t = void* (void*, RE::TESObjectREFR*, RE::BGSLocation*, RE::BGSLocation*, bool);
	inline REL::Relocation<func_unkfunc3_t>unkfunc3{ REL::ID(72938) };

	using func_unkfunc4_t = void* (void*, uint32_t, void*);
	inline REL::Relocation<func_unkfunc4_t>unkfunc4{ REL::ID(72962) };

	using func_GetPlayerSystemPos_t = bool(double *, float *);
	inline REL::Relocation<func_GetPlayerSystemPos_t>GetPlayerSystemPos{ REL::ID(52189) };

	using func_IssLerp_t = void(RE::TESImageSpace::ImageSpaceSettings *, RE::TESImageSpace::ImageSpaceSettings*, RE::TESImageSpace::ImageSpaceSettings*, float);
	inline REL::Relocation<func_IssLerp_t>IssLerp{ REL::ID(127271) };
}

namespace hooks
{
	using func_updateSpaceLocation_t = bool(uint32_t, BobbyRE::NiPoint4_double*, RE::NiMatrix3*);
	inline func_updateSpaceLocation_t* original_updateSpaceLocation;

	using func_setSpacePlanetOrbit_t = void* (va_list, BobbyRE::DBInfo**, void*, uint32_t, double*, RE::NiMatrix3*, BobbyRE::NiPoint4_double*, uint32_t, bool);
	inline func_setSpacePlanetOrbit_t* original_setSpacePlanetOrbit;

	using func_PCUpdate_t = void(RE::PlayerCharacter*, float);
	inline func_PCUpdate_t* original_PCUpdate;

	using func_unkFunc_t = bool(int*);
	inline func_unkFunc_t* original_unkFunc;

	using func_unkFunc2_t = void* (void*, RE::TESImageSpace::ImageSpaceSettings*);
	inline func_unkFunc2_t* original_unkFunc2;

	using func_calcWeatherPct_t = float(RE::Sky*);
	inline func_calcWeatherPct_t* original_calcWeatherPct;

	using func_forEachRef_t = void* (void*, void*, bool);
	inline func_forEachRef_t* original_forEachRef;

	using func_endFrame_t = void(uintptr_t, void*);
	inline func_endFrame_t* original_endFrame;

	using func_convertClouds_t = void* (void* ctx, std::byte* out, void* src);
	inline func_convertClouds_t* original_convertClouds;

	using func_updateRelative_t = void* (void *, uintptr_t);
	inline func_updateRelative_t* original_updateRelative;

	using func_minOrbitKm_t = double(uint32_t);
	inline func_minOrbitKm_t *original_minOrbitKm;

	using func_updateClouds_t = void* (byte* cloudMgr, RE::Sky* sky, RE::NiNode* follow, double a4);
	inline func_updateClouds_t* original_updateClouds;

	using func_cloudCardRenderExecute_t = std::uint64_t(byte*, void*, void*);
	inline func_cloudCardRenderExecute_t* original_cloudCardRenderExecute;

}