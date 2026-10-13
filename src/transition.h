#pragma once
#include "plugin.h"

inline float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

void fadeWeather(float t, takeoffState* state)
{
	if (t > 1)
		return;

	t = powf(t, 3.0f);
	state->weatherPctOverride = lerp(0.0, 1.0, t);
}

void dropClouds(float t, takeoffState* state)
{
	state->cloudDrop = lerp(0, 2500, t);
}
void fadeAtmospherics(float t, takeoffState* state, BobbyRE::atmosphereRenderSettings *atmosphereSettings)
{
	if (t > 1)
		return;
	float targetAtmo = atmosphereSettings->surfaceRadius + 100.0;
	float targetStarVisibility = 1.0;
	state->atmosphereForm->settings.stars.disableSimulatedVisibility = 1;

	//atmosphereSettings->atmosphereTopRadius = lerp(state->originalAtmosphereTopRadius, targetAtmo, 1 - pow(1 - t, 3));
	state->atmosphereForm->settings.stars.staticVisibility = lerp(state->originalStarVisibility, targetStarVisibility, t);
}

void fadeStarGlow(float t, takeoffState* state, BobbyRE::atmosphereRenderSettings *spaceAtmosphereSettings)
{
	float target = state->originalStarGlow;

	spaceAtmosphereSettings->surfaceRadius = lerp(0, target, t);
}

void fadeBlur(float t, bool enable, takeoffState *state)
{
		return;
	if (t > 1)
	if (enable)
		state->blurValue = lerp(0.0, 0.7, t);
	else
		state->blurValue = lerp(0.7, 0.0, t);
}