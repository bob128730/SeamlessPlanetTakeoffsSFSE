#pragma once

#include "BobbyRE/BobbyRE.h"

void OnMessage(SFSE::MessagingInterface::Message* message);

enum TAKEOFF_STATE
{
	NOT_STARTED,
	TAKEOFF_ANIM_STARTED,
	TAKEOFF_LOAD_STARTED,
	TAKEOFF_LOAD_COMPLETE
};

struct takeoffState
{
	TAKEOFF_STATE state;
	float prefadeTimer;
	float fadeTimer;
	float arrivalTimer;
	float originalStarGlow;
	float originalStarVisibility;
	float blurValue;
	float weatherPctOverride;
	bool originalDisableSimulatedVisibility;
	RE::BGSAtmosphere* atmosphereForm;
	float originalAtmosphereTopRadius;
	RE::NiPoint3 endAnimAngle;
	RE::NiPoint3 endAnimPos;
	RE::NiPoint3 endFwd;
	float maxZ;
	float cloudDrop;
	float issT;
	float fakeHeight;
	float origialCamZ;
	void* spaceLUT;
};

inline takeoffState g_takeoffState;