#pragma once

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
	bool DeleteNode(int data);
	

private:
	Node* SearchHelper(int data);
	Node* SearchMax(Node* root);
	Node* SearchMin(Node* root);
	void printHelper(Node* root, int depth);
	void deleteHelper(Node* root);

private:
	BinaryTree::Node* mRoot;
};