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
				node->left = new Node(data);
				return;
			}

			node = node->left;
		}
		else
		{
			if (node->right == nullptr)
			{
				node->right = new Node(data);
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
	cout << "Delete Node " << root->data << endl;
	delete root;
}
