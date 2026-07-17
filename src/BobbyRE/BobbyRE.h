#pragma once

namespace BobbyRE 
{
	namespace Spaceship 
	{
		struct GravJumpEvent : RE::Spaceship::GravJumpEvent 
		{
			RE::NiPointer<RE::TESObjectREFR> source; //source ship
			int aeState; // State { Initiated = 0, AnimStarted = 1, Completed = 2, Failed = 3 }
			RE::NiPointer<RE::BGSLocation> Location; 
		};

		struct TakeOffEvent : RE::Spaceship::TakeOffEvent 
		{
			RE::NiPointer<RE::TESObjectREFR> source;
			int aeState; 
		};

		struct FarTravelEvent : RE::Spaceship::FarTravelEvent
		{
			RE::NiPointer<RE::TESObjectREFR> source;
			RE::BGSLocation *fromLocation;
			RE::BGSLocation *fromLocation2;
			uint32_t unk5;
		};

		RE::Actor* GetPilot(RE::TESObjectREFR* ship) 
		{
			using func_t = decltype(&GetPilot);
			static REL::Relocation<func_t> func{ REL::ID(119876) };
			return func(ship);
		}
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

	class ImageSpaceSettings
	{
		struct SunAndSkySettings
		{
			RE::BSBlendable::FloatValue DirectionalLightIlluminanceOverride;
			RE::BSBlendable::FloatValue IndirectLightingSkyScaleOverride;
			RE::BSBlendable::FloatValue IndirectLightingSkyTargetEv100;
			RE::BSBlendable::FloatValue IndirectLightingSkyTargetStrength;
			RE::BSBlendable::FloatValue SkyLightingMultiplier;
			RE::BSBlendable::FloatValue CelestialBodyIlluminanceScaleOverride;
			RE::BSBlendable::FloatValue CelestialBodyIndirectIlluminanceScaleOverride;
			RE::BSBlendable::FloatValue SunDiskIlluminanceScaleOverride;
			RE::BSBlendable::FloatValue SunDiskIndirectIlluminanceScaleOverride;
			RE::BSBlendable::FloatValue StarfieldBackgroundScaleOverride;
			RE::BSBlendable::FloatValue StarfieldStarBrightnessScaleOverride;
			RE::BSBlendable::FloatValue SpaceGlowBackgroundScaleOverride;
			RE::BSBlendable::FloatValue CloudDirectLightingContribution;
			RE::BSBlendable::FloatValue CloudIndirectLightingContribution;
		};


		struct ExposureSettings
		{
			struct LuminanceHistogramSettings
			{
				RE::BSBlendable::FloatValue IgnoredDarksPercentile;
				RE::BSBlendable::FloatValue IgnoredBrightsPercentile;
			};

			struct AutoExposureSettings
			{
				bool                        EnableCompensationCurve;
				RE::BSFloatCurve            ExposureCompensationCurve;
				RE::BSBlendable::FloatValue ExposureMin;
				RE::BSBlendable::FloatValue ExposureMax;
				RE::BSBlendable::FloatValue AdaptSpeedDown;
				RE::BSBlendable::FloatValue AdaptSpeedUp;
				LuminanceHistogramSettings  LuminanceHistogram;
				std::byte pad[16];
			};

			struct CameraExposureSettings
			{
				RE::BSBlendable::FloatValue Aperture;
				RE::BSBlendable::FloatValue Time;
				RE::BSBlendable::FloatValue ISO;
			};

			std::uint32_t               CameraExposureMode;
			AutoExposureSettings        AutoExposure;
			RE::BSBlendable::FloatValue FixedValue;
			CameraExposureSettings      CameraExposure;
		};

		struct BloomSettings
		{
			RE::BSBlendable::FloatValue BloomThresholdOffset;
			RE::BSBlendable::FloatValue BloomScale;
			RE::BSBlendable::FloatValue BloomRangeScale;
		};

		struct CinematicSettings
		{
			RE::BSBlendable::FloatValue Saturation;
			RE::BSBlendable::FloatValue Brightness;
			RE::BSBlendable::FloatValue Contrast;
			RE::BSBlendable::ColorValue TintColourValue;
			RE::BSBlendable::ColorValue FadeColourValue;
			RE::BSFixedString*          ColorGradingTexture;
			float                       ColourGradingAmount;
		};

		struct DepthOfFieldSettings
		{
			uint32_t                    Mode;
			bool                        NoSky;
			RE::BSBlendable::FloatValue StrengthValue;
			RE::BSBlendable::FloatValue FarStartValue;
			RE::BSBlendable::FloatValue FarPlaneValue;
			RE::BSBlendable::FloatValue FarOpacityValue;
			RE::BSBlendable::FloatValue NearStartValue;
			RE::BSBlendable::FloatValue NearPlaneValue;
			RE::BSBlendable::FloatValue NearOpacityValue;
			bool                        UseTargetForDepthOfField;
		};

		struct BlurSettings {
			RE::BSBlendable::FloatValue BlurRadiusValue;
			RE::BSBlendable::FloatValue DoubleVisionStrengthValue;
			RE::BSBlendable::FloatValue MotionBlurStrengthValue;
		};

		struct RadialBlurSettings
		{
			RE::BSBlendable::FloatValue StrengthValue;
			RE::BSBlendable::FloatValue RampUpValue;
			RE::BSBlendable::FloatValue RampDownValue;
			RE::BSBlendable::FloatValue StartValue;
			RE::BSBlendable::FloatValue DownStartValue;
			RE::BSBlendable::FloatValue CenterXValue;
			RE::BSBlendable::FloatValue CenterYValue;
			bool                        UseTargetForRadialBlur;
		};

		struct AmbientOcclusionSettings
		{
			RE::BSBlendable::FloatValue IntensityValue;
		};
		
		struct FogSettings
		{
			RE::BSBlendable::FloatValue FogScaleValue;
		};

		struct VolumetricLightingSettings
		{
			RE::BSBlendable::FloatValue FogMapContribution;
			RE::BSBlendable::FloatValue VolumetricLightingDirectionalLightScale;
			RE::BSBlendable::FloatValue VolumetricLightingDirectionalAnisoScale;
			RE::BSBlendable::FloatValue VolumetricIndirectLightContribution;
		};

		struct IndirectLightingSettings
		{
			RE::BSBlendable::FloatValue IndirectDiffuseMultiplier;
			RE::BSBlendable::FloatValue IndirectSpecularMultiplier;
			RE::BSBlendable::FloatValue WaterIndirectSpecularMultiplier;
			bool                        UseAsInteriorCriteria;
			bool                        BlockClothWindIfInterior;
		};

		struct ToneMappingSettings
		{
			uint32_t                    CurveType;
			RE::BSBlendable::FloatValue WhitePointValue;
			RE::BSBlendable::FloatValue ToneMapE;
			RE::BSBlendable::FloatValue HableToeStrength;
			RE::BSBlendable::FloatValue HableToeLength;
			RE::BSBlendable::FloatValue HableShoulderStrength;
			RE::BSBlendable::FloatValue HableShoulderLength;
			RE::BSBlendable::FloatValue HableShoulderAngle;
		};

		//0x150 in TESImageSpace
		float                      OverallBlendAmount; // 00
		bool                       UseOutdoorExposure; // 04
		ExposureSettings           Exposure;           // 08
		SunAndSkySettings          SunAndSky;          //
		BloomSettings              Bloom;
		CinematicSettings          Cinematic;
		std::byte                  pad[8];
		DepthOfFieldSettings       DepthOfField;
		BlurSettings               Blur;
		RadialBlurSettings         RadialBlur;
		AmbientOcclusionSettings   AmbientOcclusion;
		FogSettings                Fog;
		VolumetricLightingSettings VolumetricLighting;
		IndirectLightingSettings   IndirectLighting;
		bool                       OverrideToneMapping;
		ToneMappingSettings        ToneMapping;

	};

	struct atmosphereRenderSettings
	{
		byte pad[0x80];
		float surfaceRadius;
		float seaRadius;
		float atmosphereTopRadius;
	};
	struct PlanetComponent
	{
		RE::NiMatrix3 rotation;
		double X;
		double Y;
		double Z;
	};

}