#ifndef  __PACKET__
#define  __PACKET__

//#define NEW_DELETE_VER
//#define MEMORY_POOL_VER
#define TLS_MEMORY_POOL_VER

#ifdef MEMORY_POOL_VER
#include "TC_LFObjectPool.h"
#elif defined(TLS_MEMORY_POOL_VER)
#include "ObjectPool_TLS.h"
#endif // MEMORY_POOL_VER
#include <wtypes.h>

namespace procademy
{
	class CPacket
	{
		friend class CLanServerNoLock;
	public:
		/*---------------------------------------------------------------
		Packet Enum.
		----------------------------------------------------------------*/
		enum en_PACKET
		{
			eBUFFER_DEFAULT = 3000		// 패킷의 기본 버퍼 사이즈.
		};

		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CPacket();
		CPacket(int iBufferSize);

		virtual ~CPacket();

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
		int	GetCapacity(void) { return mCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 사이즈 얻기.
		//
		// Parameters: 없음.
		// Return: (int)사용중인 데이타 사이즈.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mSize; }

		//////////////////////////////////////////////////////////////////////////
		// 버퍼 포인터 얻기.
		//
		// Parameters: 없음.
		// Return: (char *)버퍼 포인터.
		//////////////////////////////////////////////////////////////////////////
		char* GetBufferPtr(void) { return mBuffer; }
		char* GetFrontPtr(void) { return mFront; }

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
		int GetFreeSize() const;

		// 연산자 오버로딩 넣기
		CPacket& operator << (unsigned char byValue);
		CPacket& operator << (char chValue);

		CPacket& operator << (short shValue);
		CPacket& operator << (unsigned short wValue);

		CPacket& operator << (int iValue);
		CPacket& operator << (unsigned int iValue);
		CPacket& operator << (long lValue);
		CPacket& operator << (unsigned long lValue);
		CPacket& operator << (float fValue);

		CPacket& operator << (__int64 iValue);
		CPacket& operator << (double dValue);

		// 연산자 오버로딩 빼기
		CPacket& operator >> (unsigned char& byValue);
		CPacket& operator >> (char& chValue);

		CPacket& operator >> (short& shValue);
		CPacket& operator >> (unsigned short& wValue);

		CPacket& operator >> (int& iValue);
		CPacket& operator >> (unsigned int& iValue);
		CPacket& operator >> (long& dwValue);
		CPacket& operator >> (unsigned long& dwValue);
		CPacket& operator >> (float& fValue);

		CPacket& operator >> (__int64& iValue);
		CPacket& operator >> (double& dValue);

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

		CPacket& operator << (const char* s);
		CPacket& operator << (const wchar_t* s);

		static CPacket* AllocAddRef();
		void			AddRef();
		void			SubRef();
		void			ResetCount();

	protected:
		/// <summary>
		/// 버퍼 크기를 패킷의 기본 버퍼 사이즈 만큼 증가.
		/// </summary>
		void		resize();
		void		writeBuffer(const char* src, int size);

	private:
		struct st_RefCount
		{
			union
			{
				LONG counter = 0;
				struct
				{
					SHORT count;
					SHORT isFreed;
				} refStaus;
			};
		};

		st_RefCount	mRefCount;
		char* mBuffer;
		int			mCapacity;
		int			mSize;
		char* mFront;
		char* mRear;
#ifdef MEMORY_POOL_VER
		alignas(64) static TC_LFObjectPool<CPacket> sPacketPool;
#elif defined(TLS_MEMORY_POOL_VER)
		alignas(64) static ObjectPool_TLS<CPacket> sPacketPool;
#endif // MEMORY_POOL_VER
	};
#endif
}
