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
		mRoot = new Node(data, 0);

		return;
	}

	int count = 0;
	Node* node = mRoot;

	while (1)
	{
		count++;
		if (node->data > data)
		{
			if (node->left == nullptr)
			{
				node->left = new Node(data, count);
				return;
			}

			node = node->left;
		}
		else
		{
			if (node->right == nullptr)
			{
				node->right = new Node(data, count);
				return;
			}

			node = node->right;
		}
	}
}

void BinaryTree::printTree()
{
	printHelper(mRoot);
}

void BinaryTree::printHelper(Node* root)
{
	if (root == nullptr)
		return;

	printHelper(root->left);
	
	int count = 0;
	while (count++ < root->depth)
	{
		cout << "\t";
	}
	cout << root->data << "(" << root->depth << ")" << endl;

	printHelper(root->right);
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
