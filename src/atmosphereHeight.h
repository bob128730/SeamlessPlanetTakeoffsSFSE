#pragma once

#include "BobbyRE/BobbyRE.h"
namespace atmosphereHeight
{
	inline uint64_t originalSet;
	inline uint64_t cloneSet;

	namespace hooks 
	{
		using func_dispatch_t = std::uint64_t(void*, uint64_t*, uint32_t, uint32_t, uint32_t);
		inline func_dispatch_t* original_dispatch;

		using func_setupExec_t = std::uint64_t(byte*, void*);
		inline func_setupExec_t* original_setupExec;

		void install(REL::Trampoline*);
	}

	using func_allocSet_t = std::uint64_t* (void*, void*, uint16_t);
	using func_allocConst_t = std::byte*(uint64_t*, uint16_t, void*, void*, int32_t);
	inline REL::Relocation<func_allocSet_t>   AllocBindingSet{ REL::ID(143075) };
	inline REL::Relocation<func_allocConst_t> AllocConstants{ REL::ID(143792) };

	void setStarHorizon(BobbyRE::atmosphereRenderSettings* a, float virtualCamZ);
}