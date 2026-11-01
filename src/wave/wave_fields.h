#pragma once

#include "plugin_interface.h"
#include "Engine_classes.hpp"
#include "Chimera_classes.hpp"
#include "Chimera_structs.hpp"
#include "plugin_map.h"

#include <iterator>

// Every member of a game class the wave reader reads, taken from the loader's
// map for the running game build (see plugin_map.h). Resolved once in
// OnPluginLoadHooks; read on the game thread with In(object), which is null
// when the object is null or the member did not resolve.

#define RUPTURETIMER_FIELD(Class, Member) SR_FIELD(decltype(SDK::Class::Member), Class, Member, 0)

namespace RuptureTimer::Fields
{
	inline auto WaveTimerActor = RUPTURETIMER_FIELD(ACrGameStateBase, WaveTimerActor);

	inline auto NextTime  = RUPTURETIMER_FIELD(ACrWaveTimerActor, NextTime);
	inline auto NextPhase = RUPTURETIMER_FIELD(ACrWaveTimerActor, NextPhase);
	inline auto Pause     = RUPTURETIMER_FIELD(ACrWaveTimerActor, bPause);

	inline auto PreWaveSubstage  = RUPTURETIMER_FIELD(UCrEnviroWaveSubsystem, CurrentPreWaveSubstage);
	inline auto FadeoutSubstage  = RUPTURETIMER_FIELD(UCrEnviroWaveSubsystem, CurrentFadeoutSubstage);
	inline auto GrowbackSubstage = RUPTURETIMER_FIELD(UCrEnviroWaveSubsystem, CurrentGrowbackSubstage);

	// GetCurrentStageSettings returns the struct by value and wave_model.cpp
	// reads its floats, so the whole struct is checked against the running build.
	inline auto WaveSettingsSize = SR_SIZE_CHECK(FCrEnviroWaveSettings);

	// UCrEnviroWaveTimerSubsystem's members sit inside the SDK's Pad_30[0x60], so
	// they have no names there: native rows. The built offsets are CL-127004's
	// and CL-128786 moved them (+12 each, TimerActor +8); the rows below are this
	// plugin's own until the loader's map carries both. WaitingDuration (0x78) is
	// not a member any more on CL-128786, so it is not requested: the configured
	// calm length is read from NextWaveTimer, which the game assigns from it and
	// nothing else changes.
	inline auto TimerNextWaveTimer = PluginField<float>{ { "UCrEnviroWaveTimerSubsystem::NextWaveTimer",
		"UCrEnviroWaveTimerSubsystem", "NextWaveTimer", PLUGIN_FIELD_NATIVE, sizeof(float), 0x7C } };
	inline auto TimerStopWaves = PluginField<bool>{ { "UCrEnviroWaveTimerSubsystem::bStopWaves",
		"UCrEnviroWaveTimerSubsystem", "bStopWaves", PLUGIN_FIELD_NATIVE, sizeof(bool), 0x83 } };
	inline auto TimerActor = PluginField<SDK::ACrWaveTimerActor*>{ { "UCrEnviroWaveTimerSubsystem::TimerActor",
		"UCrEnviroWaveTimerSubsystem", "TimerActor", PLUGIN_FIELD_NATIVE, sizeof(void*), 0x90 } };

	inline const PluginFieldCandidate kNextWaveTimerRows[] = {
		{ 127004, 0x7C, sizeof(float), 0, nullptr },
		{ 128786, 0x88, sizeof(float), 0, nullptr },
	};
	inline const PluginFieldCandidate kStopWavesRows[] = {
		{ 127004, 0x83, sizeof(bool), 0, nullptr },
		{ 128786, 0x8F, sizeof(bool), 0, nullptr },
	};
	inline const PluginFieldCandidate kTimerActorRows[] = {
		{ 127004, 0x90, sizeof(void*), 0, nullptr },
		{ 128786, 0x98, sizeof(void*), 0, nullptr },
	};

	// Asks the loader for every row above. False when any did not resolve; the
	// loader has then recorded the failure and refuses the plugin.
	inline bool Resolve(IPluginSelf* self, IPluginHookScanner* scanner)
	{
		bool all = PluginMap::ResolveFields(scanner, self,
			WaveTimerActor, NextTime, NextPhase, Pause,
			PreWaveSubstage, FadeoutSubstage, GrowbackSubstage, WaveSettingsSize);
		all = PluginMap::ResolveField(scanner, self, TimerNextWaveTimer, kNextWaveTimerRows, static_cast<uint32_t>(std::size(kNextWaveTimerRows))) && all;
		all = PluginMap::ResolveField(scanner, self, TimerStopWaves, kStopWavesRows, static_cast<uint32_t>(std::size(kStopWavesRows))) && all;
		all = PluginMap::ResolveField(scanner, self, TimerActor, kTimerActorRows, static_cast<uint32_t>(std::size(kTimerActorRows))) && all;
		return all;
	}

	// The member's value, or a fallback when the member did not resolve.
	template <class T, class Object>
	T ValueOr(const PluginField<T>& field, const Object* object, T fallback = T{})
	{
		const T* at = field.In(static_cast<const void*>(object));
		return at ? *at : fallback;
	}
}
