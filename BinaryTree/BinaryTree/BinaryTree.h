#pragma once

class BinaryTree
{
private:
	struct Node
	{
		int data;

		Node* left;
		Node* right;

		Node(int data)
			: data(data)
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

private:
	void printHelper(Node* root, int depth);
	void deleteHelper(Node* root);

private:
	Node* mRoot;
};