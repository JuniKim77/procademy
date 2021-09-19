#include "StreamQueue.h"

StreamQueue::StreamQueue()
{
	Initial(eBUFFER_DEFAULT);
}

StreamQueue::StreamQueue(int iBufferSize)
{
	Initial(iBufferSize);
}

StreamQueue::~StreamQueue(void)
{
	delete[]m_chpBuffer;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼 생성 & 큐의 초기화.
//
// Parameters: (int)버퍼용량.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::Initial(int iBufferSize)
{
	m_iBufferSize = iBufferSize;
	m_chpBuffer = new char[iBufferSize];

	m_iReadPos = 0;
	m_iWritePos = 0;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼 전체의 용량 얻음.
//
// Parameters: 없음.
// Return: (int)버퍼용량.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetBufferSize(void)
{
	return m_iBufferSize;
}

/////////////////////////////////////////////////////////////////////////
// 현재 사용중인 용량 얻기.
//
// Parameters: 없음.
// Return: (int)사용중인 용량.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetUseSize(void)
{
	if (m_iWritePos >= m_iReadPos)
		return m_iWritePos - m_iReadPos;
	else
		return GetBufferSize() - m_iReadPos + m_iWritePos;
}

/////////////////////////////////////////////////////////////////////////
// 현재 버퍼에 남은 용량 얻기.
//
// Parameters: 없음.
// Return: (int)남은용량.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetFreeSize(void)
{
	if (m_iReadPos > m_iWritePos)
		return m_iReadPos - m_iWritePos - eBUFFER_BLANK;
	else
		return GetBufferSize() - m_iWritePos + m_iReadPos - eBUFFER_BLANK;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
// (끊기지 않은 길이)
//
// Parameters: 없음.
// Return: (int)사용가능 용량.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetNotBrokenGetSize(void)
{
	if (m_iWritePos < m_iReadPos)
		return GetBufferSize() - m_iReadPos;
	else
		return GetUseSize();
}

int	StreamQueue::GetNotBrokenPutSize(void)
{
	if (m_iWritePos < m_iReadPos)
		return GetFreeSize();
	else
	{
		if (m_iReadPos < eBUFFER_BLANK)
			return GetBufferSize() - m_iWritePos - (eBUFFER_BLANK - m_iReadPos);
		else
			return GetBufferSize() - m_iWritePos;
	}
}

/////////////////////////////////////////////////////////////////////////
// WritePos 에 데이타 넣음.
//
// Parameters: (char *)데이타 포인터. (int)크기. 
// Return: (int)넣은 크기.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::Put(char* chpData, int iSize)
{
	if (GetFreeSize() < iSize)
		iSize = GetFreeSize();

	if (GetNotBrokenPutSize() < iSize && m_iWritePos > m_iReadPos)
		iSize = GetNotBrokenPutSize() + m_iReadPos - eBUFFER_BLANK;

	for (int iCnt = 0; iCnt < iSize; iCnt++)
	{
		m_chpBuffer[m_iWritePos++] = chpData[iCnt];
		if (m_iWritePos == m_iBufferSize)	m_iWritePos = 0;
	}

	return iSize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos 에서 데이타 가져옴. ReadPos 이동.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::Get(char* chpDest, int iSize)
{
	if (GetUseSize() < iSize)
		iSize = GetUseSize();

	if (GetNotBrokenGetSize() < iSize && m_iWritePos < m_iReadPos)
		iSize = GetNotBrokenGetSize() + m_iWritePos;

	for (int iCnt = 0; iCnt < iSize; iCnt++)
	{
		chpDest[iCnt] = m_chpBuffer[m_iReadPos++];
		if (m_iReadPos == m_iBufferSize)	m_iReadPos = 0;
	}

	return iSize;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::Peek(char* chpDest, int iSize)
{
	int iCnt;

	if (GetUseSize() < iSize)
		iSize = GetUseSize();

	if (GetNotBrokenGetSize() < iSize && m_iWritePos < m_iReadPos)
		iSize = GetNotBrokenGetSize() + m_iWritePos;

	for (iCnt = 0; iCnt < iSize; iCnt++)
	{
		chpDest[iCnt] = m_chpBuffer[(m_iReadPos + iCnt) % m_iBufferSize];
	}

	return iCnt;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::Peek(char* chpDest, int iIndex, int iSize)
{
	int iCnt;

	// PeekPos 위치가 Queue의 데이터 범위를 벗어날 때
	if (GetUseSize() < iIndex)
		return 0;

	int iPeekPos = (m_iReadPos + iIndex) % m_iBufferSize;

	if (GetUseSize() < iSize)
		iSize = GetUseSize();

	if (m_iBufferSize - iPeekPos < iSize && m_iWritePos < iPeekPos)
		iSize = m_iBufferSize - iPeekPos + m_iWritePos;

	for (iCnt = 0; iCnt < iSize; iCnt++)
	{
		chpDest[iCnt] = m_chpBuffer[(iPeekPos + iCnt) % m_iBufferSize];
	}

	return iCnt;
}



/////////////////////////////////////////////////////////////////////////
// 원하는 길이만큼 읽기위치 에서 삭제.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::RemoveData(int iSize)
{
	if (GetUseSize() < iSize)
		return;
	else
		m_iReadPos = (m_iReadPos + iSize) % GetBufferSize();
}

/////////////////////////////////////////////////////////////////////////
// Write 의 위치를 이동하는 함수.
//
// Parameters: 없음.
// Return: (int)Write 이동 사이즈
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::MoveWritePos(int iSize)
{
	if (GetFreeSize() < iSize)
		return 0;
	else
		m_iWritePos = (m_iWritePos + iSize) % GetBufferSize();

	return iSize;
}


/////////////////////////////////////////////////////////////////////////
// 버퍼의 모든 데이타 삭제.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::ClearBuffer(void)
{
	m_iWritePos = 0;
	m_iReadPos = 0;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetBufferPtr(void)
{
	return m_chpBuffer;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 ReadPos 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetReadBufferPtr(void)
{
	return &m_chpBuffer[m_iReadPos];
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 WritePos 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetWriteBufferPtr(void)
{
	return &m_chpBuffer[m_iWritePos];
}