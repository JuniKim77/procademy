#pragma once

namespace procademy
{
	class CNetPacket;

	class CNetSmartPacket
	{
	public:
		CNetSmartPacket() {}
		CNetSmartPacket(CNetPacket* packet);
		CNetSmartPacket(const CNetSmartPacket& packet);
		~CNetSmartPacket();
		void operator= (const CNetSmartPacket& packet);
		void operator= (CNetPacket* packet);
		CNetPacket& operator* () { return *mpMemory; }
		CNetPacket* operator->() { return mpMemory; }
	private:
		CNetPacket* mpMemory = nullptr;
		long*		mpRefCount = nullptr;
	};
}