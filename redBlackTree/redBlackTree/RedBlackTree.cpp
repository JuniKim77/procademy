#include "RedBlackTree.h"
#include <iostream>

RedBlackTree::RedBlackTree()
{
	Nil = new Node(-1);
	Nil->color = NODE_COLOR::BLACK;
	mRoot = Nil;
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

				return;
			}

			node = node->right;
		}
	}
}

bool RedBlackTree::DeleteNode(int data)
{
	Node* pNode = SearchHelper(data);

	if (pNode == nullptr)
		return false;

	// �����Ͱ� �߰ߵ� ���
	// �� ��带 �� ���� �ִ� ���
	if (pNode->left != nullptr && pNode->right != nullptr)
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

		if (rightMin->right != nullptr)
		{
			rightMin->right->parent = parent;
		}

		delete rightMin;
	}
	// �ϳ��� �ڽĸ� ���� ���
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
	// �ƹ� �ڽ� ���� ���
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
