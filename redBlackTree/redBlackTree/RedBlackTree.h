class RedBlackTree
{
	enum class NODE_COLOR
	{
		BLACK = 0,
		RED
	};

	struct Node
	{
		Node* parent;
		Node* left;
		Node* right;

		NODE_COLOR color;

		int data;

		Node()
			: color(NODE_COLOR::BLACK)
			, parent(nullptr)
			, left(nullptr)
			, right(nullptr)
			, data(0)
		{}

		Node(int data)
			: color(NODE_COLOR::RED)
			, data(data)
			, parent(nullptr)
			, left(nullptr)
			, right(nullptr)
		{
		}
	};
public:
	RedBlackTree();
	~RedBlackTree();
	void InsertNode(int data);
	bool DeleteNode(int data);

private:
	Node* SearchHelper(int data);
	Node* SearchMax(Node* root);
	Node* SearchMin(Node* root);
	void DestroyHelper(Node* root);
	void RotateRight(Node* root);
	void RotateLeft(Node* root);

private:
	RedBlackTree::Node Nil;
	RedBlackTree::Node* mRoot;
};