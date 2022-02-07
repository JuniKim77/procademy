namespace procademy
{
	template <typename DATA>
	class CSafeStack
	{
	private:
		enum {
			DEFAULT_SIZE = 500
		};

	public:
		CSafeStack();
		~CSafeStack();
		CSafeStack(int iBufferSize);

		int GetCapacity() { return mCapacity; }
		int GetUseSize();
		bool IsEmpty() { return mTop == -1; }
		bool IsFool();
		void Pop();
		bool Push(DATA data);
		DATA Top();

	private:
		int mTop = -1;
		DATA* mBuffer;
		int mCapacity;
	};


	template<typename DATA>
	inline CSafeStack<DATA>::CSafeStack()
		: CSafeStack(DEFAULT_SIZE)
	{
	}

	template<typename DATA>
	inline CSafeStack<DATA>::~CSafeStack()
	{
		delete[] mBuffer;
	}

	template<typename DATA>
	inline CSafeStack<DATA>::CSafeStack(int iBufferSize)
		: mTop(-1)
		, mCapacity(iBufferSize)
		, mBuffer(nullptr)
	{
		mBuffer = new DATA[(long long)iBufferSize];
	}

	template<typename DATA>
	inline int CSafeStack<DATA>::GetUseSize()
	{
		return mTop + 1;
	}

	template<typename DATA>
	inline bool CSafeStack<DATA>::IsFool()
	{
		return mTop + 1 == mCapacity;
	}

	template<typename DATA>
	inline void CSafeStack<DATA>::Pop()
	{
		if (IsEmpty())
		{
			return;
		}

		mTop--;
	}

	template<typename DATA>
	inline bool CSafeStack<DATA>::Push(DATA data)
	{
		if (IsFool())
		{
			return false;
		}

		mBuffer[++mTop] = data;

		return true;
	}

	template<typename DATA>
	inline DATA CSafeStack<DATA>::Top()
	{
		return mBuffer[mTop];
	}

}