#include "BinaryTree.h"
#include <iostream>
#include <windowsx.h>

using namespace std;

BinaryTree::BinaryTree()
	: mRoot(nullptr)
{
	mBrush = CreateSolidBrush(RGB(0, 150, 0));
}

BinaryTree::~BinaryTree()
{
	deleteHelper(mRoot);
	DeleteObject(mBrush);
}

void BinaryTree::InsertNode(int data)
{
	if (mRoot == nullptr)
	{
		mRoot = new Node(data);

		return;
	}

	Node* node = mRoot;

	while (1)
	{
		if (node->data > data)
		{
			if (node->left == nullptr)
			{
				Node* newNode = new Node(data);
				node->left = newNode;
				newNode->parent = node;

				return;
			}

			node = node->left;
		}
		else
		{
			if (node->right == nullptr)
			{
				Node* newNode = new Node(data);
				node->right = newNode;
				newNode->parent = node;

				return;
			}

			node = node->right;
		}
	}
}

void BinaryTree::printTree()
{
	printHelper(mRoot, 0);
}

void BinaryTree::printTreeWin(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	SelectObject(hdc, mBrush);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER);

	int x = GetSystemMetrics(SM_CXSCREEN);

	printWinHelper(hdc, mRoot, 0, x, 0);

	ReleaseDC(hWnd, hdc);
}

bool BinaryTree::DeleteNode(int data)
{
	Node* pNode = SearchHelper(data);

	if (pNode == nullptr)
		return false;

	// 데이터가 발견된 경우
	// 양 노드를 다 갖고 있는 경우
	if (pNode->left != nullptr && pNode->right != nullptr)
	{
		Node* rightMin = SearchMin(pNode->right);

		pNode->data = rightMin->data;
		Node* parent = rightMin->parent;

		// 빈 노드 삭제
		if (parent->left == rightMin)
		{
			parent->left = rightMin->right;
		}
		else
		{
			parent->right = rightMin->right;
		}

		if (rightMin->right != nullptr)
		{
			rightMin->right->parent = parent;
		}

		delete rightMin;
	}
	// 하나의 자식만 갖은 경우
	else if (pNode->left != nullptr)
	{
		if (pNode == mRoot)
		{
			mRoot = pNode->left;
			delete pNode;

			return true;
		}

		Node* parent = pNode->parent;

		if (parent->right == pNode)
		{
			parent->right = pNode->left;
		}
		else
		{
			parent->left = pNode->left;
		}

		pNode->left->parent = parent;

		delete pNode;
	}
	else if (pNode->right != nullptr)
	{
		if (pNode == mRoot)
		{
			mRoot = pNode->right;
			delete pNode;

			return true;
		}

		Node* parent = pNode->parent;

		if (parent->right == pNode)
		{
			parent->right = pNode->right;
		}
		else
		{
			parent->left = pNode->right;
		}

		pNode->right->parent = parent;

		delete pNode;
	}
	// 아무 자식 없는 경우
	else
	{
		if (pNode == mRoot)
		{
			mRoot = nullptr;
			delete pNode;

			return true;
		}

		if (pNode->parent->right == pNode)
		{
			pNode->parent->right = nullptr;
		}
		else
		{
			pNode->parent->left = nullptr;
		}

		delete pNode;
	}

	return true;
}

BinaryTree::Node* BinaryTree::SearchHelper(int data)
{
	Node* pNode = mRoot;

	while (pNode != nullptr)
	{
		if (pNode->data == data)
		{
			return pNode;
		}
		else if (pNode->data > data)
		{
			pNode = pNode->left;
		}
		else
		{
			pNode = pNode->right;
		}
	}

	return nullptr;
}

BinaryTree::Node* BinaryTree::SearchMax(Node* root)
{
	Node* pNode = root;

	while (pNode != nullptr)
	{
		if (pNode->right == nullptr)
		{
			return pNode;
		}

		pNode = pNode->right;
	}

	return nullptr;
}

BinaryTree::Node* BinaryTree::SearchMin(Node* root)
{
	Node* pNode = root;

	while (pNode != nullptr)
	{
		if (pNode->left == nullptr)
		{
			return pNode;
		}

		pNode = pNode->left;
	}

	return nullptr;
}

void BinaryTree::printHelper(Node* root, int depth)
{
	if (root == nullptr)
		return;

	printHelper(root->left, depth + 1);
	
	int count = 0;
	while (count++ < depth)
	{
		cout << "\t";
	}
	cout << root->data << "(" << depth << ")" << endl;

	printHelper(root->right, depth + 1);
}

void BinaryTree::printWinHelper(HDC hdc, Node* root, int beginX, int endX, int depth)
{
	if (root == nullptr)
		return;

	int x = (beginX + endX) / 2;
	int y = 100 * depth;

	drawNode(hdc, root, x, y, (beginX + x) / 2, (x + endX) / 2);

	printWinHelper(hdc, root->left, beginX, x, depth + 1);
	printWinHelper(hdc, root->right, x, endX, depth + 1);
}

void BinaryTree::drawNode(HDC hdc, Node* node, int x, int y, int leftX, int rightX)
{
	// 선 그리기
	if (node->left != nullptr)
	{
		MoveToEx(hdc, x + 25, y + 17, NULL);
		LineTo(hdc, leftX + 25, y + 117);
	}

	if (node->right != nullptr)
	{
		MoveToEx(hdc, x + 25, y + 17, NULL);
		LineTo(hdc, rightX + 25, y + 117);
	}

	Ellipse(hdc, x, y, x + 50, y + 50);
	WCHAR text[20];
	wsprintf(text, L"%d", node->data);

	TextOut(hdc, x + 25, y + 17, text, wcslen(text));
}

void BinaryTree::deleteHelper(Node* root)
{
	if (root == nullptr)
		return;

	deleteHelper(root->left);
	deleteHelper(root->right);
	cout << "Delete Node " << root->data << endl;
	delete root;
}
