#include <iostream>
#include "BinaryTree.h"

using namespace std;

int main()
{
	BinaryTree Tree;

	Tree.InsertNode(10);
	Tree.InsertNode(11);
	Tree.InsertNode(12);
	Tree.InsertNode(5);
	Tree.InsertNode(6);
	Tree.InsertNode(3);
	Tree.InsertNode(9);
	Tree.InsertNode(100);
	Tree.InsertNode(20);
	Tree.InsertNode(1);
	Tree.InsertNode(2);
	Tree.InsertNode(4);
	Tree.InsertNode(7);
	Tree.InsertNode(8);
	Tree.InsertNode(111);
	Tree.InsertNode(112);
	Tree.InsertNode(101);
	Tree.InsertNode(24);
	Tree.InsertNode(23);
	Tree.InsertNode(22);
	Tree.InsertNode(25);
	Tree.InsertNode(26);

	Tree.printTree();
	cout << "===================================================" << endl;

	if (Tree.DeleteNode(100))
	{
		Tree.printTree();
	}

	cout << "===================================================" << endl;

	if (Tree.DeleteNode(10))
	{
		Tree.printTree();
	}

	cout << "===================================================" << endl;

	if (Tree.DeleteNode(24))
	{
		Tree.printTree();
	}
}