#include "MyHeap.h"
#include <iostream>

using namespace std;

int main()
{
	MyHeap Tree(128);

	Tree.InsertData(10);
	Tree.InsertData(11);
	Tree.InsertData(12);
	Tree.InsertData(5);
	Tree.InsertData(6);
	Tree.InsertData(3);
	Tree.InsertData(9);
	Tree.InsertData(100);
	Tree.InsertData(20);
	Tree.InsertData(1);
	Tree.InsertData(2);
	Tree.InsertData(4);
	Tree.InsertData(7);
	Tree.InsertData(8);
	Tree.InsertData(111);
	Tree.InsertData(112);
	Tree.InsertData(101);
	Tree.InsertData(24);
	Tree.InsertData(23);
	Tree.InsertData(22);
	Tree.InsertData(25);
	Tree.InsertData(26);

	Tree.printHeap();
	int size = Tree.GetSize();

	for (int i = 0; i < size; ++i)
	{
		int data = Tree.GetMax();

		cout << "Delete " << data << endl;

		Tree.printHeap();
	}
	

	return 0;
}