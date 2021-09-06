#ifndef  __PACKET__
#define  __PACKET__

class CPacket
{
public:
	/*---------------------------------------------------------------
	Packet Enum.
	----------------------------------------------------------------*/
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 3000		// ��Ŷ�� �⺻ ���� ������.
	};

	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CPacket();
	CPacket(int iBufferSize);

	virtual	~CPacket();

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
	int	GetCapacity(void) { return mCapacity; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� ������ ���.
	//
	// Parameters: ����.
	// Return: (int)������� ����Ÿ ������.
	//////////////////////////////////////////////////////////////////////////
	int		GetSize(void) { return mSize; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������ ���.
	//
	// Parameters: ����.
	// Return: (char *)���� ������.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void) { return mBuffer; }
	char* GetFrontPtr(void) { return mFront; }

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
	int GetFreeSize() const;

	// ������ �����ε� �ֱ�
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

	// ������ �����ε� ����
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
	// ����Ÿ ���.
	//
	// Parameters: (char *)Dest ������. (int)Size.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iLength);
	int		GetData(wchar_t* chpDest, int iLength);

	//////////////////////////////////////////////////////////////////////////
	// ����Ÿ ����.
	//
	// Parameters: (char *)Src ������. (int)SrcSize.
	// Return: (int)������ ������.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(const char* chpSrc, int iLength);
	int		PutData(const wchar_t* chpSrc, int iLength);

	CPacket& operator << (const char* s);
	CPacket& operator << (const wchar_t* s);

protected:
	/// <summary>
	/// ���� ũ�⸦ ��Ŷ�� �⺻ ���� ������ ��ŭ ����.
	/// </summary>
	void resize();
	void writeBuffer(const char* src, int size);

private:
	char* mBuffer;
	int mCapacity;
	int mSize;
	char* mFront;
	char* mRear;
};
#endif