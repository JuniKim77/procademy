#include "BinaryTree.h"
#include <iostream>

using namespace std;

BinaryTree::BinaryTree()
	: mRoot(nullptr)
{
}

BinaryTree::~BinaryTree()
{
	deleteHelper(mRoot);
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

bool BinaryTree::DeleteNode(int data)
{
	BinaryTree::Node* pNode = SearchHelper(data);

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

bool BinaryTree::SearchData(int data)
{
	Node* node = SearchHelper(data);

	return node != nullptr && node->data == data;
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

void BinaryTree::deleteHelper(Node* root)
{
	if (root == nullptr)
		return;

	deleteHelper(root->left);
	deleteHelper(root->right);
	delete root;
}
