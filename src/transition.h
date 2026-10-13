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

	atmosphereSettings->atmosphereTopRadius = lerp(state->originalAtmosphereTopRadius, targetAtmo, 1 - pow(1 - t, 3));
	state->atmosphereForm->settings.stars.staticVisibility = lerp(state->originalStarVisibility, targetStarVisibility, t);
}

void fadeImageSpaceSettings(float t, RE::TESImageSpace::ImageSpaceSettings *surfaceSettings, RE::TESImageSpace::ImageSpaceSettings* originalSurfaceSettings)
{
	if (t > 1)
		return;
	if (surfaceSettings)
	{
		surfaceSettings->IndirectLighting.IndirectDiffuseMultiplier.value =
			lerp(originalSurfaceSettings->IndirectLighting.IndirectDiffuseMultiplier.value, 0, t);

		surfaceSettings->IndirectLighting.IndirectSpecularMultiplier.value =
			lerp(originalSurfaceSettings->IndirectLighting.IndirectSpecularMultiplier.value, 0, t);

		surfaceSettings->SunAndSky.SkyLightingMultiplier.value =
			lerp(originalSurfaceSettings->SunAndSky.SkyLightingMultiplier.value, 0, t);

		//		g_surfaceImageSpaceSettings->SunAndSky.SpaceGlowBackgroundScaleOverride.value =
		//			lerp(g_originalSurfaceImageSpaceSettings->SunAndSky.SpaceGlowBackgroundScaleOverride.value, 25.0f, t);

		surfaceSettings->SunAndSky.StarfieldStarBrightnessScaleOverride.value =
			lerp(originalSurfaceSettings->SunAndSky.StarfieldStarBrightnessScaleOverride.value, 10.0f, t);

		surfaceSettings->SunAndSky.StarfieldBackgroundScaleOverride.value =
			lerp(originalSurfaceSettings->SunAndSky.StarfieldBackgroundScaleOverride.value, 5.0f, t);
	}
}

void fadeStarGlow(float t, takeoffState* state, BobbyRE::atmosphereRenderSettings *spaceAtmosphereSettings)
{
	float target = state->originalStarGlow;

	spaceAtmosphereSettings->surfaceRadius = lerp(0, target, t);
}

void fadeBlur(float t, bool enable, takeoffState *state)
{
	if (t > 1)
		return;
	if (enable)
		state->blurValue = lerp(0.0, 0.7, t);
	else
		state->blurValue = lerp(0.7, 0.0, t);
}