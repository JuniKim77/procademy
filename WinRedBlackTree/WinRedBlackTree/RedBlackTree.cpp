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

bool RedBlackTree::InsertNode(int data)
{
	if (mRoot == Nil)
	{
		mRoot = new Node(data);
		mRoot->left = Nil;
		mRoot->right = Nil;
		mRoot->color = NODE_COLOR::BLACK;

		return true;
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

				return true;
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

				return true;
			}

			node = node->right;
		}
	}

	return false;
}

bool RedBlackTree::DeleteNode(int data)
{
	Node* pNode = SearchHelper(data);

	if (pNode == Nil)
		return false;

	// 데이터가 발견된 경우
	// 양 노드를 다 갖고 있는 경우
	if (pNode->left != Nil && pNode->right != Nil)
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

		rightMin->right->parent = parent;

		if (rightMin->color == NODE_COLOR::BLACK)
		{
			DeleteRebalance(rightMin->right);
		}
		
		delete rightMin;
	}
	// 하나의 자식만 갖은 경우
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
	// 아무 자식 없는 경우
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

void RedBlackTree::clear()
{
	DestroyHelper(mRoot);

	mRoot = Nil;
}

void RedBlackTree::printAround(Node* root)
{
	Node* parent = root->parent;
	Node* sibling = parent->left == root ? parent->right : parent->left;
	Node* NL = root->left;
	Node* NR = root->right;
	Node* grand = parent->parent;

	wprintf_s(L"root: ");
	printNode(root);
	wprintf_s(L"parent: ");
	printNode(parent);
	wprintf_s(L"sibling: ");
	printNode(sibling);
	wprintf_s(L"NL: ");
	printNode(NL);
	wprintf_s(L"NR: ");
	printNode(NR);
	wprintf_s(L"grand: ");
	printNode(grand);

	wprintf_s(L"\n============================\n\n");
}

void RedBlackTree::printNode(Node* root)
{
	if (root == nullptr)
	{
		wprintf_s(L"NULL Node\n");
	}
	else if (root == Nil)
	{
		wprintf_s(L"Nil Node, Color: %s\n", root->color == NODE_COLOR::BLACK ? L"BLACK" : L"RED");
	}
	else
	{
		wprintf_s(L"Normal Node, Data: %d, Color: %s\n", root->data, root->color == NODE_COLOR::BLACK ? L"BLACK" : L"RED");
	}
}

void RedBlackTree::deleteNodePrint(Node* del)
{
	wprintf_s(L"Delete ");
	printNode(del);
	wprintf_s(L"\nParent: ");
	printNode(del->parent);
}

bool RedBlackTree::CheckBalance()
{
	mBalanced = true;
	mNumBlack = -1;

	checkBalanceHelper(mRoot, 0);

	return mBalanced;
}

void RedBlackTree::checkBalanceHelper(Node* root, int blackCount)
{
	if (!mBalanced)
		return;

	if (root == Nil)
	{
		if (mNumBlack != -1)
		{
			if (mNumBlack != blackCount)
			{
				mBalanced = false;
			}
		}
		else
		{
			mNumBlack = blackCount;
		}

		return;
	}

	if (root->color == NODE_COLOR::BLACK)
	{
		checkBalanceHelper(root->left, blackCount + 1);
		checkBalanceHelper(root->right, blackCount + 1);
	}
	else
	{
		checkBalanceHelper(root->left, blackCount);
		checkBalanceHelper(root->right, blackCount);
	}

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
	// 선 그리기
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

	// 1. 부모 노드와 Left 연결
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

	// 2. LR과 root와 연결
	root->left = left->right;
	left->right->parent = root;

	// 3. Left를 Parent로 승격
	left->right = root;
	root->parent = left;
}

void RedBlackTree::RotateLeft(Node* root)
{
	if (root->right == Nil)
		return;

	Node* parent = root->parent;
	Node* right = root->right;

	// 1. 부모 노드와 Right 연결
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

	// 2. RL과 root와 연결
	root->right = right->left;
	right->left->parent = root;

	// 3. Right를 Parent로 승격
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
				Left_InsertRebalanceUncleBlack(root);

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
				Right_InsertRebalanceUncleBlack(root);

				return;
			}
			else
			{
				root = InsertRebalanceUncleRed(root);
			}
		}
	}
}

void RedBlackTree::Left_InsertRebalanceUncleBlack(Node* root)
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

void RedBlackTree::Right_InsertRebalanceUncleBlack(Node* root)
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
	Node* root = originRoot;

	while (1)
	{
		if (root->color == NODE_COLOR::RED || mRoot == root)
		{
			root->color = NODE_COLOR::BLACK;
			return;
		}

		Node* parent = root->parent;

		// 왼쪽편인 경우
		if (parent->left == root)
		{
			Node* sibling = parent->right;

			if (sibling->color == NODE_COLOR::RED)
			{
				Left_DeleteRebalanceSiblingRed(sibling);
			}
			else
			{
				if (sibling->left->color == NODE_COLOR::BLACK &&
					sibling->right->color == NODE_COLOR::BLACK)
				{
					root = Both_DeleteRebalanceSiblingBlackBothBlack(sibling);
				}
				else
				{
					if (sibling->left->color == NODE_COLOR::RED)
					{
						Left_DeleteRebalanceSiblingBlackInsideRed(sibling);
					}

					sibling = parent->right;

					Left_DeleteRebalanceSiblingBlackOutsideRed(sibling);

					return;
				}
			}
		}
		// 오른쪽편인 경우
		else
		{
			Node* sibling = parent->left;

			if (sibling->color == NODE_COLOR::RED)
			{
				Right_DeleteRebalanceSiblingRed(sibling);
			}
			else
			{
				if (sibling->left->color == NODE_COLOR::BLACK &&
					sibling->right->color == NODE_COLOR::BLACK)
				{
					root = Both_DeleteRebalanceSiblingBlackBothBlack(sibling);
				}
				else
				{
					if (sibling->right->color == NODE_COLOR::RED)
					{
						Right_DeleteRebalanceSiblingBlackInsideRed(sibling);
					}

					sibling = parent->left;

					Right_DeleteRebalanceSiblingBlackOutsideRed(sibling);

					return;
				}
			}
		}
	}
}

void RedBlackTree::Left_DeleteRebalanceSiblingRed(Node* root)
{
	Node* parent = root->parent;

	parent->color = NODE_COLOR::RED;
	root->color = NODE_COLOR::BLACK;

	RotateLeft(parent);
}

void RedBlackTree::Right_DeleteRebalanceSiblingRed(Node* root)
{
	Node* parent = root->parent;

	parent->color = NODE_COLOR::RED;
	root->color = NODE_COLOR::BLACK;

	RotateRight(parent);
}

RedBlackTree::Node* RedBlackTree::Both_DeleteRebalanceSiblingBlackBothBlack(Node* root)
{
	Node* parent = root->parent;
	root->color = NODE_COLOR::RED;

	return parent;
}

void RedBlackTree::Left_DeleteRebalanceSiblingBlackInsideRed(Node* root)
{
	root->color = NODE_COLOR::RED;
	root->left->color = NODE_COLOR::BLACK;

	RotateRight(root);
}

void RedBlackTree::Right_DeleteRebalanceSiblingBlackInsideRed(Node* root)
{
	root->color = NODE_COLOR::RED;
	root->right->color = NODE_COLOR::BLACK;

	RotateLeft(root);
}

void RedBlackTree::Left_DeleteRebalanceSiblingBlackOutsideRed(Node* root)
{
	Node* parent = root->parent;

	root->color = parent->color;
	parent->color = NODE_COLOR::BLACK;
	root->right->color = NODE_COLOR::BLACK;

	RotateLeft(parent);
}

void RedBlackTree::Right_DeleteRebalanceSiblingBlackOutsideRed(Node* root)
{
	Node* parent = root->parent;

	root->color = parent->color;
	parent->color = NODE_COLOR::BLACK;
	root->left->color = NODE_COLOR::BLACK;

	RotateRight(parent);
}
