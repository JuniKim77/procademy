#include "animal.h"

class dog : public animal
{
public:
	void bark() const;
	virtual void run() const;
	void walk() const;
private:

};