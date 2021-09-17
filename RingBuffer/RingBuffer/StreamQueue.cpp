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
// ���� ���� & ť�� �ʱ�ȭ.
//
// Parameters: (int)���ۿ뷮.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::Initial(int iBufferSize)
{
	m_iBufferSize = iBufferSize;
	m_chpBuffer = new char[iBufferSize];

	m_iReadPos = 0;
	m_iWritePos = 0;
}

/////////////////////////////////////////////////////////////////////////
// ���� ��ü�� �뷮 ����.
//
// Parameters: ����.
// Return: (int)���ۿ뷮.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetBufferSize(void)
{
	return m_iBufferSize;
}

/////////////////////////////////////////////////////////////////////////
// ���� ������� �뷮 ���.
//
// Parameters: ����.
// Return: (int)������� �뷮.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetUseSize(void)
{
	if (m_iWritePos >= m_iReadPos)
		return m_iWritePos - m_iReadPos;
	else
		return GetBufferSize() - m_iReadPos + m_iWritePos;
}

/////////////////////////////////////////////////////////////////////////
// ���� ���ۿ� ���� �뷮 ���.
//
// Parameters: ����.
// Return: (int)�����뷮.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::GetFreeSize(void)
{
	if (m_iReadPos > m_iWritePos)
		return m_iReadPos - m_iWritePos - eBUFFER_BLANK;
	else
		return GetBufferSize() - m_iWritePos + m_iReadPos - eBUFFER_BLANK;
}

/////////////////////////////////////////////////////////////////////////
// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
// (������ ���� ����)
//
// Parameters: ����.
// Return: (int)��밡�� �뷮.
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
// WritePos �� ����Ÿ ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��. 
// Return: (int)���� ũ��.
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
// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
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
// ReadPos ���� ����Ÿ �о��. ReadPos ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
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
// ReadPos ���� ����Ÿ �о��. ReadPos ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
/////////////////////////////////////////////////////////////////////////
int	StreamQueue::Peek(char* chpDest, int iIndex, int iSize)
{
	int iCnt;

	// PeekPos ��ġ�� Queue�� ������ ������ ��� ��
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
// ���ϴ� ���̸�ŭ �б���ġ ���� ����.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::RemoveData(int iSize)
{
	if (GetUseSize() < iSize)
		return;
	else
		m_iReadPos = (m_iReadPos + iSize) % GetBufferSize();
}

/////////////////////////////////////////////////////////////////////////
// Write �� ��ġ�� �̵��ϴ� �Լ�.
//
// Parameters: ����.
// Return: (int)Write �̵� ������
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
// ������ ��� ����Ÿ ����.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void StreamQueue::ClearBuffer(void)
{
	m_iWritePos = 0;
	m_iReadPos = 0;
}

/////////////////////////////////////////////////////////////////////////
// ������ ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetBufferPtr(void)
{
	return m_chpBuffer;
}

/////////////////////////////////////////////////////////////////////////
// ������ ReadPos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetReadBufferPtr(void)
{
	return &m_chpBuffer[m_iReadPos];
}

/////////////////////////////////////////////////////////////////////////
// ������ WritePos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char* StreamQueue::GetWriteBufferPtr(void)
{
	return &m_chpBuffer[m_iWritePos];
}