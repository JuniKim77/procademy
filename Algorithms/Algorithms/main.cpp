#include <iostream>
#include <string>
#include <stack>

using namespace std;

struct LinkedListNode
{
	int data;
	LinkedListNode* next;

	LinkedListNode(int num)
		: data(num)
		, next(nullptr)
	{

	}
};

class LinkedList
{
public:
	LinkedList();
	~LinkedList();
	void AddData(int data);
	void RemoveData(int data);
	LinkedListNode* GetNodeAt(int idx) const;
	void PrintAll() const;
	int GetSize() const;

private:
	LinkedListNode* mHead;
	LinkedListNode* mLast;
	int mSize;
};

LinkedList::LinkedList()
	: mHead(nullptr)
	, mLast(nullptr)
	, mSize(0)
{
}

LinkedList::~LinkedList()
{
	while (mHead != nullptr)
	{
		LinkedListNode* temp = mHead->next;
		delete mHead;
		mHead = temp;
	}
}

void LinkedList::AddData(int data)
{
	if (mLast == nullptr)
	{
		mLast = new LinkedListNode(data);
		mHead = mLast;
		mSize++;
		return;
	}

	mLast->next = new LinkedListNode(data);
	mLast = mLast->next;
	mSize++;
}

void LinkedList::RemoveData(int data)
{
	LinkedListNode* nxtNode;
	
	while (mHead != nullptr && mHead->data == data)
	{
		 nxtNode = mHead->next;
		 delete mHead;
		 mHead = nxtNode;
		 mSize--;
	}

	if (mHead == nullptr) return;

	nxtNode = mHead;
	
	while (nxtNode->next != nullptr)
	{
		if (nxtNode->next->data == data)
		{
			LinkedListNode* temp = nxtNode->next;
			nxtNode->next = temp->next;
			delete temp;
			mSize--;
			continue;
		}

		nxtNode = nxtNode->next;
	}
}

LinkedListNode* LinkedList::GetNodeAt(int idx) const
{
	if (idx < 0 || idx >= mSize)
		return nullptr;

	LinkedListNode* ret = mHead;
	while (idx-- > 0)
	{
		ret = ret->next;
	}
	return ret;
}

void LinkedList::PrintAll() const
{
	LinkedListNode* cur = mHead;

	while (cur != nullptr)
	{
		cout << cur->data << endl;
		cur = cur->next;
	}
}

int LinkedList::GetSize() const
{
	return mSize;
}

struct DoubleNode {
	int data;
	DoubleNode* prev;
	DoubleNode* next;

	DoubleNode(int num)
		: data(num)
		, prev(nullptr)
		, next(nullptr)
	{

	}

	~DoubleNode()
	{
		cout << "Double Node is deleted." << endl;
	}
};

class CycledLinkedList
{
public:
	CycledLinkedList();
	~CycledLinkedList();
	void addData(int data);
	void removeData(int data);
	int getCount() const;
	void printAll() const;

private:
	DoubleNode* pHead;
};

CycledLinkedList::CycledLinkedList()
	: pHead(nullptr)
{
}

CycledLinkedList::~CycledLinkedList()
{
	DoubleNode* cur = pHead->next;

	while (cur != pHead)
	{
		DoubleNode* temp = cur;
		cur = cur->next;
		delete temp;
	}

	delete cur;
}

void CycledLinkedList::addData(int data)
{
	if (pHead == nullptr)
	{
		pHead = new DoubleNode(data);
		pHead->next = pHead;
		pHead->prev = pHead;
		return;
	}

	DoubleNode* node = new DoubleNode(data);
	node->prev = pHead->prev;
	node->next = pHead;
	pHead->prev->next = node;
	pHead->prev = node;
	pHead = node;
}

void CycledLinkedList::removeData(int data)
{
	if (pHead == nullptr) return;

	DoubleNode* cur = pHead->next;

	while (cur != pHead)
	{
		if (cur->data == data)
		{
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;

			delete cur;
		}
		cur = cur->next;
	}

	if (cur->data == data)
	{
		if (cur->next == pHead) {
			delete cur;
			pHead = nullptr;
			return;
		}
		cur->prev->next = cur->next;
		cur->next->prev = cur->prev;

		delete cur;
	}
}

int CycledLinkedList::getCount() const
{
	if (pHead == nullptr) return 0;

	int count = 0;

	DoubleNode* cur = pHead;

	while (cur->next != pHead)
	{
		cur = cur->next;
		count++;
	}

	return count + 1;
}

void CycledLinkedList::printAll() const
{
	DoubleNode* cur = pHead;

	cout << "====================" << endl;

	while (cur->next != pHead)
	{
		cout << cur->data << endl;
		cur = cur->next;
	}

	cout << cur->data << endl;
}

struct TreeNode
{
	int data;
	TreeNode* left;
	TreeNode* right;

	TreeNode(int _data)
		: data(_data)
		, left(nullptr)
		, right(nullptr)
	{	}
};

class MyBinaryTree
{
public:
	MyBinaryTree()
		: mHeight(0)
		, mRoot(nullptr)
	{}

	~MyBinaryTree()
	{

	}

	bool addData(int data)
	{
		TreeNode* node = new TreeNode(data);

		if (node == nullptr)
			return false;

		if (mRoot == nullptr) {
			mRoot = node;
		}
		else {
			addHelper(node, mRoot);
		}

		return true;
	}

	void addHelper(TreeNode* node, TreeNode* root)
	{
		if (node->data <= root->data) {
			if (root->left == nullptr)
				root->left = node;
			else
				addHelper(node, root->left);
		}
		else {
			if (root->right == nullptr)
				root->right = node;
			else
				addHelper(node, root->right);
		}
	}

	bool getData(int data, TreeNode* result)
	{
		if (mRoot == nullptr)
		{
			return false;
		}

		bool ret = getDataHelper(data, result, mRoot);

		return ret;
	}

	bool getDataHelper(int data, TreeNode* result, TreeNode* root)
	{
		if (root->data == data)
		{
			result->data = data;
		}
	}



	bool deleteData(int data)
	{

	}

private:
	int mHeight;
	TreeNode* mRoot;

};



int main()
{
	// LinkedList
	{
		LinkedList liList;

		liList.AddData(1);
		liList.AddData(1);
		liList.AddData(1);
		liList.AddData(2);
		liList.AddData(3);
		liList.AddData(4);
		liList.AddData(4);
		liList.AddData(4);
		liList.AddData(5);
		liList.AddData(6);
		liList.AddData(7);
		liList.AddData(7);
		liList.AddData(1);
		liList.AddData(1);
		liList.AddData(2);
		liList.AddData(3);

		liList.PrintAll();
		cout << "=====================" << endl;

		liList.RemoveData(1);

		liList.PrintAll();

		cout << "=====================" << endl;

		cout << liList.GetNodeAt(0)->data << endl;
		cout << liList.GetNodeAt(1)->data << endl;
		cout << liList.GetSize() << endl;
	}
	
	// CycledLinkedList
	{
		CycledLinkedList lst;

		lst.addData(1);
		lst.addData(1);
		lst.addData(1);
		lst.addData(1);
		lst.addData(1);
		lst.addData(1);
		lst.addData(1);
		lst.addData(2);
		lst.addData(3);
		lst.addData(4);

		lst.printAll();

		lst.removeData(1);

		lst.printAll();

		cout << "CycledList size: " << lst.getCount() << endl;
	}
	return 0;
}