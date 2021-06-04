#pragma once
#include <wtypes.h>

class BinaryTree
{
private:
	struct Node
	{
		int data;

		Node* parent;
		Node* left;
		Node* right;

		Node(int data)
			: data(data)
			, parent(nullptr)
			, left(nullptr)
			, right(nullptr)
		{
		}
	};

public:
	BinaryTree();
	~BinaryTree();
	void InsertNode(int data);
	void printTree();
	void printTreeWin(HWND hWnd);
	bool DeleteNode(int data);
	

private:
	Node* SearchHelper(int data);
	Node* SearchMax(Node* root);
	Node* SearchMin(Node* root);
	void printHelper(Node* root, int depth);
	void printWinHelper(HDC hdc, Node* root, int beginX, int endX, int depth);
	void drawNode(HDC hdc, Node* node, int x, int y, int leftX, int rightX);
	void deleteHelper(Node* root);

private:
	BinaryTree::Node* mRoot;
	HBRUSH mBrush;
};