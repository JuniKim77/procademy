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
	/// 데이터 삽입
	/// </summary>
	/// <param name="data"></param>
	void InsertNode(int data);
	/// <summary>
	/// 데이터 삭제
	/// </summary>
	/// <param name="data"></param>
	/// <returns></returns>
	bool DeleteNode(int data);
	/// <summary>
	/// WinAPI 뷰어
	/// </summary>
	/// <param name="hWnd"></param>
	void printTreeWin(HWND hWnd);
	bool SearchData(int data);
	bool CheckBalance();

private:
	void checkBalanceHelper(Node* root, int blackount);
	/// <summary>
	/// WinAPI 뷰어 헬퍼
	/// </summary>
	/// <param name="hdc"></param>
	/// <param name="root"></param>
	/// <param name="beginX">왼쪽 범위</param>
	/// <param name="endX">오른쪽 범위</param>
	/// <param name="depth">깊이</param>
	void printWinHelper(HDC hdc, Node* root, int beginX, int endX, int depth);
	/// <summary>
	/// 노드 그리기
	/// </summary>
	/// <param name="hdc"></param>
	/// <param name="node"></param>
	/// <param name="x">위치 X</param>
	/// <param name="y">위치 Y</param>
	/// <param name="leftX">다음 선의 목적지(좌)</param>
	/// <param name="rightX">다음 선의 목적지(우)</param>
	void drawNode(HDC hdc, Node* node, int x, int y, int leftX, int rightX);
	/// <summary>
	/// 해당 데이터 찾기
	/// </summary>
	/// <param name="data"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchHelper(int data);
	/// <summary>
	/// 서브 트리의 최대 값 노드 찾기
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchMax(Node* root);
	/// <summary>
	/// 서브 트리의 최소 값 노드 찾기
	/// </summary>
	/// <param name="root"></param>
	/// <returns></returns>
	RedBlackTree::Node* SearchMin(Node* root);
	/// <summary>
	/// 후위 순회 삭제
	/// </summary>
	/// <param name="root"></param>
	void DestroyHelper(Node* root);
	/// <summary>
	///  root 기준 우회전
	/// </summary>
	/// <param name="root"></param>
	void RotateRight(Node* root);
	/// <summary>
	///  root 기준 좌회전
	/// </summary>
	/// <param name="root"></param>
	void RotateLeft(Node* root);
	/// <summary>
	/// 입력 시 리벨런싱
	/// </summary>
	/// <param name="originRoot">NewNode</param>
	void InsertRebalance(Node* originRoot);
	/// <summary>
	/// 새 노드(N)가 왼쪽 편, 부모 RED, 삼촌 BLACK
	/// </summary>
	/// <param name="root">NewNode</param>
	void Left_InsertRebalanceUncleBlack(Node* root);
	/// <summary>
	/// 새 노드(N)가 오른쪽 편, 부모 RED, 삼촌 BLACK
	/// </summary>
	/// <param name="root">NewNode</param>
	void Right_InsertRebalanceUncleBlack(Node* root);
	/// <summary>
	/// 입력 시, 부모 RED, 삼촌도 RED
	/// </summary>
	/// <param name="root">NewNode</param>
	/// <returns>grand root</returns>
	RedBlackTree::Node* InsertRebalanceUncleRed(Node* root);
	/// <summary>
	/// 삭제 시 리밸런싱
	/// </summary>
	/// <param name="originRoot">Rep</param>
	void DeleteRebalance(Node* originRoot);
	/// <summary>
	/// 삭제 시, Rep 왼쪽 편 BLACK, Sibling RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingRed(Node* root);
	/// <summary>
	/// 삭제 시, Rep 오른쪽 편 BLACK, Sibling RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Right_DeleteRebalanceSiblingRed(Node* root);
	/// <summary>
	/// 삭제 시, Sibling BLACK, 두 자식 모두 BLACK
	/// </summary>
	/// <param name="root">Sibling</param>
	/// <returns></returns>
	RedBlackTree::Node* Both_DeleteRebalanceSiblingBlackBothBlack(Node* root);
	/// <summary>
	/// 삭제 시, Rep 왼쪽 편 BLACK, Sibling BLACK 안쪽 자식 RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingBlackInsideRed(Node* root);
	/// <summary>
	/// 삭제 시, Rep 오른쪽 편 BLACK, Sibling BLACK 안쪽 자식 RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Right_DeleteRebalanceSiblingBlackInsideRed(Node* root);
	/// <summary>
	/// 삭제 시, Rep 왼쪽 편 BLACK, Sibling BLACK 밖깥쪽 자식 RED
	/// </summary>
	/// <param name="root">Sibling</param>
	void Left_DeleteRebalanceSiblingBlackOutsideRed(Node* root);
	/// <summary>
	/// 삭제 시, Rep 오른쪽 편 BLACK, Sibling BLACK 밖깥쪽 자식 RED
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