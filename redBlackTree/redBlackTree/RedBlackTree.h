#pragma once
#include <wtypes.h>

class RedBlackTree
{
	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	/// <summary>
	/// ������ ����
	/// </summary>
	/// <param name="data"></param>
	void InsertNode(int data);
	/// <summary>
	/// ������ ����
	/// </summary>
	/// <param name="data"></param>
	/// <returns></returns>
	bool DeleteNode(int data);
	/// <summary>
	/// WinAPI ���
	/// </summary>
	/// <param name="hWnd"></param>
	void printTreeWin(HWND hWnd);
	bool SearchData(int data);
	bool CheckBalance();

private:
	void checkBalanceHelper(Node* root, int blackount);
	/// <summary>
	/// WinAPI ��� ����
	/// </summary>
	/// <param name="hdc"></param>
	/// <param name="root"></param>
	/// <param name="beginX">���� ����</param>
	/// <param name="endX">������ ����</param>
	/// <param name="depth">����</param>
	void printWinHelper(HDC hdc, Node* root, int beginX, int endX, int depth);
	/// <summary>
	/// ��� �׸���
	/// </summary>
	/// <param name="hdc"></param>
	/// <param name="node"></param>
	/// <param name="x">��ġ X</param>
	/// <param name="y">��ġ Y</param>
	/// <param name="leftX">���� ���� ������(��)</param>
	/// <param name="rightX">���� ���� ������(��)</param>
	void drawNode(HDC hdc, Node* node, int x, int y, int leftX, int rightX);
	/// <summary>
	/// �ش� ������ ã��
	/// </summary>
	/// <param name="data"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchHelper(int data);
	/// <summary>
	/// ���� Ʈ���� �ִ� �� ��� ã��
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchMax(Node* root);
	/// <summary>
	/// ���� Ʈ���� �ּ� �� ��� ã��
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchMin(Node* root);
	/// <summary>
	/// ���� ��ȸ ����
	/// </summary>
	/// <param name="root"></param>
	void DestroyHelper(Node* root);
	/// <summary>
	///  root ���� ��ȸ��
	/// </summary>
	/// <param name="root"></param>
	void RotateRight(Node* root);
	/// <summary>
	///  root ���� ��ȸ��
	/// </summary>
	/// <param name="root"></param>
	void RotateLeft(Node* root);
	/// <summary>
	/// �Է� �� ��������
	/// </summary>
	/// <param name="originRoot">NewNode</param>
	void InsertRebalance(Node* originRoot);
	/// <summary>
	/// �� ���(N)�� ���� ��, �θ� RED, ���� BLACK
	/// </summary>
	/// <param name="root">NewNode</param>
	void Left_InsertRebalanceUncleBlack(Node* root);
	/// <summary>
	/// �� ���(N)�� ������ ��, �θ� RED, ���� BLACK
	/// </summary>
	/// <param name="root">NewNode</param>
	void Right_InsertRebalanceUncleBlack(Node* root);
	/// <summary>
	/// �Է� ��, �θ� RED, ���̵� RED
	/// </summary>
	/// <param name="root">NewNode</param>
	/// <returns>grand root</returns>
	RedBlackTree::Node* InsertRebalanceUncleRed(Node* root);
	/// <summary>
	/// ���� �� ���뷱��
	/// </summary>
	/// <param name="originRoot">Rep</param>
	void DeleteRebalance(Node* originRoot);
	/// <summary>
	/// ���� ��, Rep ���� �� BLACK, Sibling RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingRed(Node* root);
	/// <summary>
	/// ���� ��, Rep ������ �� BLACK, Sibling RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Right_DeleteRebalanceSiblingRed(Node* root);
	/// <summary>
	/// ���� ��, Sibling BLACK, �� �ڽ� ��� BLACK
	/// </summary>
	/// <param name="root">Sibling</param>
	/// <returns></returns>
	RedBlackTree::Node* Both_DeleteRebalanceSiblingBlackBothBlack(Node* root);
	/// <summary>
	/// ���� ��, Rep ���� �� BLACK, Sibling BLACK ���� �ڽ� RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingBlackInsideRed(Node* root);
	/// <summary>
	/// ���� ��, Rep ������ �� BLACK, Sibling BLACK ���� �ڽ� RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Right_DeleteRebalanceSiblingBlackInsideRed(Node* root);
	/// <summary>
	/// ���� ��, Rep ���� �� BLACK, Sibling BLACK �۱��� �ڽ� RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingBlackOutsideRed(Node* root);
	/// <summary>
	/// ���� ��, Rep ������ �� BLACK, Sibling BLACK �۱��� �ڽ� RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Right_DeleteRebalanceSiblingBlackOutsideRed(Node* root);

private:
	RedBlackTree::Node* Nil;
	RedBlackTree::Node* mRoot;
	HBRUSH mRedBrush;
	HBRUSH mBlackBrush;
	int mNumBlack;
	bool mBalanced;
};