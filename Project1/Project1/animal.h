#include <iostream>

class animal
{
public:
	virtual void bark() const;
	void walk() const;
	virtual void run() const;
	virtual ~animal()
	{
		std::cout << "Animal ¼Ò¸êÀÚ" << std::endl;
	}
private:
	int mAge = 0;
};