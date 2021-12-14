#ifndef  __PACKET__
#define  __PACKET__

//#define NEW_DELETE_VER
//#define MEMORY_POOL_VER
#define TLS_MEMORY_POOL_VER
#//define PROFILE

#ifdef MEMORY_POOL_VER
#include "TC_LFObjectPool.h"
#elif defined(TLS_MEMORY_POOL_VER)
#include "ObjectPool_TLS.h"
#endif // MEMORY_POOL_VER
#include <wtypes.h>

namespace procademy
{
	class CLanPacket
	{
		template<typename DATA>
		friend class TC_LFObjectPool;
		template<typename DATA>
		friend class ObjectPool_TLS;

	public:
		/*---------------------------------------------------------------
		Packet Enum.
		----------------------------------------------------------------*/
		enum en_PACKET
		{
			HEADER_MAX_SIZE = 5,
			eBUFFER_DEFAULT = 500		// ��Ŷ�� �⺻ ���� ������.
		};

		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Return:
		//////////////////////////////////////////////////////////////////////////

		virtual ~CLanPacket();

		//////////////////////////////////////////////////////////////////////////
		// ��Ŷ  �ı�.
		//
		// Parameters: ����.
		// Return: ����.
		//////////////////////////////////////////////////////////////////////////
		void	Release(void);

		//////////////////////////////////////////////////////////////////////////
		// ��Ŷ û��.
		//
		// Parameters: ����.
		// Return: ����.
		//////////////////////////////////////////////////////////////////////////
		void	Clear(void);

		//////////////////////////////////////////////////////////////////////////
		// ���� ������ ���.
		//
		// Parameters: ����.
		// Return: (int)��Ŷ ���� ������ ���.
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacity(void) { return mCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� ������ ���.
		//
		// Parameters: ����.
		// Return: (int)������� ����Ÿ ������.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mPacketSize + mHeaderSize; }
		int		GetUseSize() { return mPacketSize; }
		USHORT	GetPacketSize() { return mPacketSize; }

		//////////////////////////////////////////////////////////////////////////
		// ���� ������ ���.
		//
		// Parameters: ����.
		// Return: (char *)���� ������.
		//////////////////////////////////////////////////////////////////////////
		char* GetBufferPtr(void) { return mBuffer; }
		char* GetFrontPtr(void) { return mFront; }
		char* GetZeroPtr(void) { return mZero; }

		//////////////////////////////////////////////////////////////////////////
		// ���� Pos �̵�. (�����̵��� �ȵ�)
		// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
		//
		// Parameters: (int) �̵� ������.
		// Return: (int) �̵��� ������.
		//////////////////////////////////////////////////////////////////////////
		int		MoveFront(int iSize);
		int		MoveRear(int iSize);
		/// <summary>
		/// ���� ������ ��� ���� ������ ��ȯ
		/// </summary>
		/// <returns>��� ���� ������</returns>
		int		GetFreeSize() const { return mCapacity - mPacketSize - HEADER_MAX_SIZE; }

		// ������ �����ε� �ֱ�
		CLanPacket& operator << (unsigned char byValue);
		CLanPacket& operator << (char chValue);

		CLanPacket& operator << (short shValue);
		CLanPacket& operator << (unsigned short wValue);

		CLanPacket& operator << (int iValue);
		CLanPacket& operator << (unsigned int iValue);
		CLanPacket& operator << (long lValue);
		CLanPacket& operator << (unsigned long lValue);
		CLanPacket& operator << (float fValue);

		CLanPacket& operator << (__int64 iValue);
		CLanPacket& operator << (double dValue);

		// ������ �����ε� ����
		CLanPacket& operator >> (unsigned char& byValue);
		CLanPacket& operator >> (char& chValue);

		CLanPacket& operator >> (short& shValue);
		CLanPacket& operator >> (unsigned short& wValue);

		CLanPacket& operator >> (int& iValue);
		CLanPacket& operator >> (unsigned int& iValue);
		CLanPacket& operator >> (long& dwValue);
		CLanPacket& operator >> (unsigned long& dwValue);
		CLanPacket& operator >> (float& fValue);

		CLanPacket& operator >> (__int64& iValue);
		CLanPacket& operator >> (double& dValue);

		//////////////////////////////////////////////////////////////////////////
		// ����Ÿ ���.
		//
		// Parameters: (char *)Dest ������. (int)Size.
		// Return: (int)������ ������.
		//////////////////////////////////////////////////////////////////////////
		int			GetData(char* chpDest, int iLength);
		int			GetData(wchar_t* chpDest, int iLength);

		//////////////////////////////////////////////////////////////////////////
		// ����Ÿ ����.
		//
		// Parameters: (char *)Src ������. (int)SrcSize.
		// Return: (int)������ ������.
		//////////////////////////////////////////////////////////////////////////
		int			PutData(const char* chpSrc, int iLength);
		int			PutData(const wchar_t* chpSrc, int iLength);

		CLanPacket& operator << (const char* s);
		CLanPacket& operator << (const wchar_t* s);

		static CLanPacket* AllocAddRef();
		void				AddRef();
		void				SubRef();

		static int			GetPoolCapacity();
		static DWORD		GetPoolSize();
		static void			SetCode(BYTE code) { sCode = code; }
		static void			SetPacketKey(BYTE key) { sPacketKey = key; }


	protected:
		/// <summary>
		/// ���� ũ�⸦ ��Ŷ�� �⺻ ���� ������ ��ŭ ����.
		/// </summary>
		void		resize();
		void		writeBuffer(const char* src, int size);

	private:
		CLanPacket();
		CLanPacket(int iBufferSize);

	public:
#pragma pack(push, 1)
		struct st_Header
		{
			BYTE	code;
			USHORT	len;
			BYTE	randKey;
			BYTE	checkSum;
		};
#pragma pack(pop)
	private:

		SHORT		mRefCount = 0;
		char* mBuffer;
		int			mCapacity;
		int			mPacketSize;
		int			mHeaderSize;
		char* mFront;
		char* mRear;
		char* mZero;
	public:
		static BYTE	sCode;
		static BYTE	sPacketKey;

#ifdef MEMORY_POOL_VER
		alignas(64) static TC_LFObjectPool<CLanPacket> sPacketPool;
#elif defined(TLS_MEMORY_POOL_VER)
		alignas(64) static ObjectPool_TLS<CLanPacket> sPacketPool;
#endif // MEMORY_POOL_VER
	};
#endif
}