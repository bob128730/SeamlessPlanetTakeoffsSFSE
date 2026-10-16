#include "atmosphereHeight.h"
#include "plugin.h"

namespace atmosphereHeight
{
	void setStarHorizon(BobbyRE::atmosphereRenderSettings* a, float fakeHeight)
	{
		const float rc = a->surfaceRadius + fakeHeight;
		const float rg = a->seaRadius;
		const float k = rg / rc;
		a->unused = //repurpoused in modified shader
			(rc > rg) ? std::sqrt(1.0f - k * k) : 0.0f;
	}

	uint64_t* cloneViewSet(uint64_t* src, void* alloc, float add)
	{
		byte* layout = (byte *)(src[0]);
		byte* entry = (byte *)(src[1]) + 24ull * *(uint32_t *)(layout + 24);
		byte* oldCb = *(byte **)(entry + 8);
		
		const auto size = *(int32_t *)(entry + 20);
		if ((int8_t)(*entry) != 14 || !oldCb || size != 53008)
			return nullptr;

		uint64_t* dst = AllocBindingSet(nullptr, alloc, 1);
		if (dst[0] != src[0])
			return nullptr;
		memcpy((void *)(dst[1]), (void *)(src[1]),
			24ull * *(uint32_t *)(layout + 16));

		dst[2] = src[2];
		dst[3] = src[3];

		std::byte* cb = AllocConstants(dst, 0, alloc, nullptr, size);
		memcpy(cb, oldCb, size);
		*(float *)(cb + 8) += add; 
		return dst;
	}


	namespace hooks
	{
		std::uint64_t hook_dispatch(void* cmd, uint64_t* pkt, uint32_t x, uint32_t y, uint32_t z)
		{
			if (originalSet && pkt[3] == originalSet)
				pkt[3] = cloneSet;
			
			return original_dispatch(cmd, pkt, x, y, z);
		}

		std::uint64_t hook_setupExec(byte* pass, void* ctx)
		{
			uint64_t result = original_setupExec(pass, ctx);

			byte* cam = *(byte**)(pass + 240);
			if (*(uint8_t*)(cam + 384))
			{
				*(float*)(cam + 328) += g_takeoffState.fakeHeight;
				cloneSet = 0; originalSet = 0;

				if (g_takeoffState.fakeHeight != 0)
				{
					byte* c = (byte*)ctx;
					uint32_t i = *(uint32_t*)(c + 320);

					byte* view = *(byte**)(c + 264 + 8 * i);

					void* alloc = *(void**)(view + 40);
					uint64_t* set = *(uint64_t**)(
									(*(byte**)(view + 48)
									+ 8));

					if (uint64_t* dst = cloneViewSet(set, alloc, g_takeoffState.fakeHeight))
					{
						originalSet = (uint64_t)(set);
						cloneSet = (uint64_t)(dst);
					}
				}
			}
			return result;
		}

		void install(REL::Trampoline* tramp)
		{
			original_dispatch = (func_dispatch_t*)tramp->write_call<5>(REL::ID(144202).address() + 0x7A7, hook_dispatch);
			tramp->write_call<5>(REL::ID(144203).address() + 0x696, hook_dispatch);
			tramp->write_call<5>(REL::ID(144204).address() + 0x89A, hook_dispatch);
			tramp->write_call<5>(REL::ID(144205).address() + 0x98F, hook_dispatch);

			auto** vtbl = REL::Relocation<std::uintptr_t**>(REL::ID(496000)).get();
			original_setupExec = (func_setupExec_t*)((uintptr_t *)(vtbl)[7]);
			REL::WriteSafeData((uintptr_t)(vtbl) + 7 * 8, &hook_setupExec);
		}
	}
}