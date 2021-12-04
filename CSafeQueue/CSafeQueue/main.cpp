#include "CSafeQueue.h"
#include <stdio.h>

procademy::CSafeQueue<int*> g_q;

int main()
{
	for (int i = 0; i < 10; ++i)
	{
		int* temp = new int;
		*temp = i;

		g_q.Enqueue(temp);
	}

	while (g_q.IsEmpty() == false)
	{
		int* temp = g_q.Dequeue();

		printf("%d\n", *temp);
	}

	return 0;
}