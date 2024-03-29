#ifndef  __CLAN_PACKET__
#define  __CLAN_PACKET__

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
			eBUFFER_DEFAULT = 400		// 패킷의 기본 버퍼 사이즈.
		};

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Return:
		//////////////////////////////////////////////////////////////////////////

		virtual ~CLanPacket();

		//////////////////////////////////////////////////////////////////////////
		// 패킷  파괴.
		//
		// Parameters: 없음.
		// Return: 없음.
		//////////////////////////////////////////////////////////////////////////
		void	Release(void);

		//////////////////////////////////////////////////////////////////////////
		// 패킷 청소.
		//
		// Parameters: 없음.
		// Return: 없음.
		//////////////////////////////////////////////////////////////////////////
		void	Clear(void);

		//////////////////////////////////////////////////////////////////////////
		// 버퍼 사이즈 얻기.
		//
		// Parameters: 없음.
		// Return: (int)패킷 버퍼 사이즈 얻기.
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacity(void) { return mCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 사이즈 얻기.
		//
		// Parameters: 없음.
		// Return: (int)사용중인 데이타 사이즈.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return (int)(mRear - mZero); }
		int		GetUseSize() { return (int)(mRear - mFront); }

		//////////////////////////////////////////////////////////////////////////
		// 버퍼 포인터 얻기.
		//
		// Parameters: 없음.
		// Return: (char *)버퍼 포인터.
		//////////////////////////////////////////////////////////////////////////
		char* GetBufferPtr(void) { return mBuffer; }
		char* GetFrontPtr(void) { return mFront; }
		char* GetZeroPtr(void) { return mZero; }

		//////////////////////////////////////////////////////////////////////////
		// 버퍼 Pos 이동. (음수이동은 안됨)
		// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
		//
		// Parameters: (int) 이동 사이즈.
		// Return: (int) 이동된 사이즈.
		//////////////////////////////////////////////////////////////////////////
		int		MoveFront(int iSize);
		int		MoveRear(int iSize);
		/// <summary>
		/// 현재 버퍼의 사용 가능 사이즈 반환
		/// </summary>
		/// <returns>사용 가능 사이즈</returns>
		int		GetFreeSize() const { return (int)(mEnd - mRear); }

		// 연산자 오버로딩 넣기
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

		// 연산자 오버로딩 빼기
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
		// 데이타 얻기.
		//
		// Parameters: (char *)Dest 포인터. (int)Size.
		// Return: (int)복사한 사이즈.
		//////////////////////////////////////////////////////////////////////////
		int			GetData(char* chpDest, int iLength);
		int			GetData(wchar_t* chpDest, int iLength);

		//////////////////////////////////////////////////////////////////////////
		// 데이타 삽입.
		//
		// Parameters: (char *)Src 포인터. (int)SrcSize.
		// Return: (int)복사한 사이즈.
		//////////////////////////////////////////////////////////////////////////
		int			PutData(const char* chpSrc, int iLength);
		int			PutData(const wchar_t* chpSrc, int iLength);

		CLanPacket& operator << (const char* s);
		CLanPacket& operator << (const wchar_t* s);

		static CLanPacket*	AllocAddRef();
		void				AddRef();
		void				SubRef();
		void				SetHeader();

		static int			GetPoolCapacity();
		static DWORD		GetPoolSize();

	protected:
		/// <summary>
		/// 버퍼 크기를 패킷의 기본 버퍼 사이즈 만큼 증가.
		/// </summary>
		void		resize();
		void		writeBuffer(const char* src, int size);

	private:
		CLanPacket();
		CLanPacket(int iBufferSize);

	private:
		SHORT		mRefCount = 0;
		char*		mBuffer;
		char*		mEnd;
		int			mCapacity;
		char*		mFront;
		char*		mRear;
		char*		mZero;
		bool		mbSetHeader = false;

	public:
#ifdef MEMORY_POOL_VER
		static TC_LFObjectPool<CLanPacket> sPacketPool;
#elif defined(TLS_MEMORY_POOL_VER)
		static ObjectPool_TLS<CLanPacket> sPacketPool;
#endif // MEMORY_POOL_VER
	};
#endif
}
