#include "RedBlackTree.h"
#include <iostream>

RedBlackTree::RedBlackTree()
{
	Nil = new Node(-1);
	Nil->color = NODE_COLOR::BLACK;
	mRoot = Nil;
	mRedBrush = CreateSolidBrush(RGB(150, 0, 0));
	mBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
}

RedBlackTree::~RedBlackTree()
{
	DestroyHelper(mRoot);

	if (Nil != nullptr)
		delete Nil;
}

void RedBlackTree::InsertNode(int data)
{
	if (mRoot == Nil)
	{
		mRoot = new Node(data);
		mRoot->left = Nil;
		mRoot->right = Nil;
		mRoot->color = NODE_COLOR::BLACK;

		return;
	}

	Node* node = mRoot;

	while (1)
	{
		if (node->data > data)
		{
			if (node->left == Nil)
			{
				Node* newNode = new Node(data);
				newNode->left = Nil;
				newNode->right = Nil;
				newNode->parent = node;
				node->left = newNode;

				// Balancing
				InsertRebalance(newNode);

				return;
			}

			node = node->left;
		}
		else
		{
			if (node->right == Nil)
			{
				Node* newNode = new Node(data);
				newNode->left = Nil;
				newNode->right = Nil;
				newNode->parent = node;
				node->right = newNode;

				// Balancing
				InsertRebalance(newNode);

				return;
			}

			node = node->right;
		}
	}
}

bool RedBlackTree::DeleteNode(int data)
{
	Node* pNode = SearchHelper(data);

	if (pNode == Nil)
		return false;

	// �����Ͱ� �߰ߵ� ���
	// �� ��带 �� ���� �ִ� ���
	if (pNode->left != Nil && pNode->right != Nil)
	{
		Node* rightMin = SearchMin(pNode->right);

		pNode->data = rightMin->data;
		Node* parent = rightMin->parent;

		// �� ��� ����
		if (parent->left == rightMin)
		{
			parent->left = rightMin->right;
		}
		else
		{
			parent->right = rightMin->right;
		}

		rightMin->right->parent = parent;

		if (rightMin->color == NODE_COLOR::BLACK)
		{
			DeleteRebalance(rightMin->right);
		}
		
		delete rightMin;
	}
	// �ϳ��� �ڽĸ� ���� ���
	else if (pNode->left != Nil)
	{
		if (pNode == mRoot)
		{
			mRoot = pNode->left;
			mRoot->color = NODE_COLOR::BLACK;
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

		if (pNode->color == NODE_COLOR::BLACK)
		{
			DeleteRebalance(pNode->left);
		}

		delete pNode;
	}
	else if (pNode->right != Nil)
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

		if (pNode->color == NODE_COLOR::BLACK)
		{
			DeleteRebalance(pNode->right);
		}

		delete pNode;
	}
	// �ƹ� �ڽ� ���� ���
	else
	{
		if (pNode == mRoot)
		{
			mRoot = Nil;
			delete pNode;

			return true;
		}

		Node* parent = pNode->parent;

		if (parent->right == pNode)
		{
			parent->right = Nil;
		}
		else
		{
			parent->left = Nil;
		}

		Nil->parent = parent;

		if (pNode->color == NODE_COLOR::BLACK)
		{
			DeleteRebalance(Nil);
		}

		delete pNode;
	}

	return true;
}

void RedBlackTree::printTreeWin(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER);
	SetTextColor(hdc, RGB(200, 200, 200));

	int x = GetSystemMetrics(SM_CXSCREEN);

	printWinHelper(hdc, mRoot, 0, x, 0);

	ReleaseDC(hWnd, hdc);
}

void RedBlackTree::printWinHelper(HDC hdc, Node* root, int beginX, int endX, int depth)
{
	if (root == Nil)
		return;

	int x = (beginX + endX) / 2;
	int y = 100 * depth;

	drawNode(hdc, root, x, y, (beginX + x) / 2, (x + endX) / 2);

	printWinHelper(hdc, root->left, beginX, x, depth + 1);
	printWinHelper(hdc, root->right, x, endX, depth + 1);
}

void RedBlackTree::drawNode(HDC hdc, Node* node, int x, int y, int leftX, int rightX)
{
	// �� �׸���
	if (node->left != Nil)
	{
		MoveToEx(hdc, x + 25, y + 17, NULL);
		LineTo(hdc, leftX + 25, y + 117);
	}

	if (node->right != Nil)
	{
		MoveToEx(hdc, x + 25, y + 17, NULL);
		LineTo(hdc, rightX + 25, y + 117);
	}

	if (node->color == NODE_COLOR::BLACK)
	{
		SelectObject(hdc, mBlackBrush);
	}
	else
	{
		SelectObject(hdc, mRedBrush);
	}

	Ellipse(hdc, x, y, x + 50, y + 50);
	WCHAR text[20];
	wsprintf(text, L"%d", node->data);

	TextOut(hdc, x + 25, y + 17, text, wcslen(text));
}

RedBlackTree::Node* RedBlackTree::SearchHelper(int data)
{
	Node* pNode = mRoot;

	while (pNode != Nil)
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

	return Nil;
}

RedBlackTree::Node* RedBlackTree::SearchMax(Node* root)
{
	Node* pNode = root;

	while (pNode != Nil)
	{
		if (pNode->right == Nil)
		{
			return pNode;
		}

		pNode = pNode->right;
	}

	return Nil;
}

RedBlackTree::Node* RedBlackTree::SearchMin(Node* root)
{
	Node* pNode = root;

	while (pNode != Nil)
	{
		if (pNode->left == Nil)
		{
			return pNode;
		}

		pNode = pNode->left;
	}

	return Nil;
}

void RedBlackTree::DestroyHelper(Node* root)
{
	if (root == Nil)
		return;

	DestroyHelper(root->left);
	DestroyHelper(root->right);
#ifdef _DEBUG
	std::cout << "Delete Node " << root->data << std::endl;
#endif
	delete root;
}

void RedBlackTree::RotateRight(Node* root)
{
	if (root->left == Nil)
		return;

	Node* parent = root->parent;
	Node* left = root->left;

	// 1. �θ� ���� Left ����
	if (root == mRoot)
	{
		mRoot = left;
	}
	else
	{
		if (parent->left == root)
		{
			parent->left = left;
		}
		else
		{
			parent->right = left;
		}

		left->parent = parent;
	}

	// 2. LR�� root�� ����
	root->left = left->right;
	left->right->parent = root;

	// 3. Left�� Parent�� �°�
	left->right = root;
	root->parent = left;
}

void RedBlackTree::RotateLeft(Node* root)
{
	if (root->right == Nil)
		return;

	Node* parent = root->parent;
	Node* right = root->right;

	// 1. �θ� ���� Right ����
	if (root == mRoot)
	{
		mRoot = right;
	}
	else
	{
		if (parent->left == root)
		{
			parent->left = right;
		}
		else
		{
			parent->right = right;
		}

		right->parent = parent;
	}

	// 2. RL�� root�� ����
	root->right = right->left;
	right->left->parent = root;

	// 3. Right�� Parent�� �°�
	right->left = root;
	root->parent = right;
}

void RedBlackTree::InsertRebalance(Node* originRoot)
{
	Node* root = originRoot;

	while (1)
	{
		if (root == mRoot)
		{
			root->color = NODE_COLOR::BLACK;
			return;
		}

		if (root->parent->color == NODE_COLOR::BLACK)
			return;

		Node* parent = root->parent;
		Node* grand = parent->parent;

		if (grand->left == parent)
		{
			if (grand->right->color == NODE_COLOR::BLACK)
			{
				InsertRebalanceUncleBlackLeft(root);

				return;
			}
			else
			{
				root = InsertRebalanceUncleRed(root);
			}
		}
		else
		{
			if (grand->left->color == NODE_COLOR::BLACK)
			{
				InsertRebalanceUncleBlackRight(root);

				return;
			}
			else
			{
				root = InsertRebalanceUncleRed(root);
			}
		}
	}
}

void RedBlackTree::InsertRebalanceUncleBlackLeft(Node* root)
{
	Node* parent = root->parent;
	Node* grand = parent->parent;
	
	if (parent->right == root)
	{
		RotateLeft(parent);
		root = parent;
		parent = root->parent;
	}

	grand->color = NODE_COLOR::RED;
	parent->color = NODE_COLOR::BLACK;
	RotateRight(grand);

	if (grand == mRoot)
	{
		mRoot = parent;
	}
}

void RedBlackTree::InsertRebalanceUncleBlackRight(Node* root)
{
	Node* parent = root->parent;
	Node* grand = parent->parent;

	if (parent->left == root)
	{
		RotateRight(parent);
		root = parent;
		parent = root->parent;
	}

	grand->color = NODE_COLOR::RED;
	parent->color = NODE_COLOR::BLACK;
	RotateLeft(grand);

	if (grand == mRoot)
	{
		mRoot = parent;
	}
}

RedBlackTree::Node* RedBlackTree::InsertRebalanceUncleRed(Node* root)
{
	Node* parent = root->parent;
	Node* grand = parent->parent;
	Node* uncle = grand->left == parent ? grand->right : grand->left;

	grand->color = NODE_COLOR::RED;
	parent->color = NODE_COLOR::BLACK;
	uncle->color = NODE_COLOR::BLACK;

	return grand;
}

void RedBlackTree::DeleteRebalance(Node* originRoot)
{
	if (originRoot->color == NODE_COLOR::RED)
	{
		originRoot->color = NODE_COLOR::BLACK;
		return;
	}

	Node* root = originRoot;

	while (1)
	{
		Node* parent = root->parent;

		// �������� ���
		if (parent->left == root)
		{
			Node* sibling = parent->right;

			if (sibling->color == NODE_COLOR::RED)
			{
				DeleteRebalanceSibligRedLeft(sibling);

				// ���� �� ����
			}
			else
			{

			}
		}
		// ���������� ���
		else
		{
			Node* sibling = parent->left;

			if (sibling->color == NODE_COLOR::RED)
			{
				DeleteRebalanceSibligRedLeft(sibling);

				// ���� �� ����
			}
			else
			{

			}
		}
	}
}

void RedBlackTree::DeleteRebalanceSibligRedLeft(Node* root)
{
	Node* parent = root->parent;
	Node* sibling = parent->left;

	parent->color = NODE_COLOR::RED;
	root->color = NODE_COLOR::BLACK;

	RotateLeft(parent);
}

void RedBlackTree::DeleteRebalanceSibligRedRight(Node* root)
{
}
