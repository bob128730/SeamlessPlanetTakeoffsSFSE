#pragma once

#include "RE/B/BSTEvent.h"

namespace BobbyRE
{
	namespace BSResource::Archive2
	{
		class StreamOpenedEvent;
	}

	class Sky;
	class TESActorBase;
	class TESObjectCELL;

	struct PositionPlayerEvent;

	using namespace RE::RTTI;
	using namespace RE::VTABLE;

	class __declspec(novtable) TES :
		public RE::BSTEventSink<BSResource::Archive2::StreamOpenedEvent>,  // 000
		public RE::BSTEventSink<PositionPlayerEvent>                       // 008
	{
	public:

		virtual ~TES();  // 00

		// override (BSTEventSink<BSResource::Archive2::StreamOpenedEvent>)
		RE::BSEventNotifyControl ProcessEvent(const BSResource::Archive2::StreamOpenedEvent& a_event, RE::BSTEventSource<BSResource::Archive2::StreamOpenedEvent>* a_eventSource) override;  // 01 - { return BSEventNotifyControl::kContinue; }

		// override (BSTEventSink<PositionPlayerEvent>)
		RE::BSEventNotifyControl ProcessEvent(const PositionPlayerEvent& a_event, RE::BSTEventSource<PositionPlayerEvent>* a_eventSource) override;  // 01

		[[nodiscard]] static TES* GetSingleton()
		{
			static REL::Relocation<TES**> singleton{ RE::ID::TES::Singleton };
			return *singleton;
		}

		std::uint16_t GetDeadCount(TESActorBase* a_actorBase)
		{
			using func_t = decltype(&TES::GetDeadCount);
			static REL::Relocation<func_t> func{ RE::ID::TES::GetDeadCount };
			return func(this, a_actorBase);
		}

		// members
		std::byte pad010[8];
		void *m_GridCellArray;
		std::byte pad048[136];
		void* field_E0;
		TESObjectCELL* interiorCell;
		std::byte pad0F0[80];
		Sky* sky;
		std::byte pad140[64];
		RE::TESWorldSpace* worldSpace;
		void* field_190;
	};
	static_assert(offsetof(TES, sky) == 0x140);
	static_assert(offsetof(TES, interiorCell) == 0xE8);
}
