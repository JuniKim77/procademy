#pragma once

class BinaryTree
{
private:
	struct Node
	{
		int data;
		int depth;

		Node* left;
		Node* right;

		Node(int data, int depth)
			: data(data)
			, depth(depth)
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
	void printHelper(Node* root);
	void deleteHelper(Node* root);

private:
	Node* mRoot;
};