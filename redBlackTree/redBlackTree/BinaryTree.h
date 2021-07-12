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
	int GetDepthInsertNode(int data);
	void printTree();
	bool DeleteNode(int data);
	int GetDepthDeleteNode(int data);
	bool SearchData(int data);
	int GetDepthSearchData(int data);
	

private:
	BinaryTree::Node* SearchHelper(int data);
	BinaryTree::Node* SearchMax(Node* root);
	BinaryTree::Node* SearchMin(Node* root);
	void printHelper(Node* root, int depth);
	void deleteHelper(Node* root);

private:
	BinaryTree::Node* mRoot;
};