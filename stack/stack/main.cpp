#include <stdio.h>
#include <string>
#include <Windows.h>
#include <time.h>

#define ARR_SIZE (10)

using namespace std;

unsigned char arr[ARR_SIZE][ARR_SIZE + 1] =
{
   {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' },
   {'0', '0', ' ', '0', '0', '0', ' ', '0', '0', ' ', '\0' },
   {'0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '0', ' ', '\0' },
   {' ', '0', '0', '0', ' ', '0', '0', '0', ' ', ' ', '\0' },
   {'0', '0', '0', '0', '0', '0', '0', '0', '0', ' ', '\0' },
   {'0', '0', '0', '0', '0', '0', '0', '0', '0', ' ', '\0' },
   {' ', '0', '0', '0', '0', '0', '0', '0', ' ', ' ', '\0' },
   {' ', ' ', '0', '0', '0', '0', '0', ' ', ' ', ' ', '\0' },
   {'0', ' ', ' ', '0', '0', '0', ' ', ' ', '0', ' ', '\0' },
   {'0', '0', ' ', ' ', '0', ' ', ' ', '0', '0', ' ', '\0' }
};
char g_ch;
bool visit[ARR_SIZE][ARR_SIZE];
const int dx[4] = { 0, 1, 0, -1 };
const int dy[4] = { -1, 0, 1, 0 };
int g_funcCount;

void paint(int x, int y);
void print_array();

struct Coordi
{
    int x;
    int y;

    Coordi()
        : x(0)
        , y(0)
    {}

    Coordi(int _x, int _y)
        : x(_x)
        , y(_y)
    {}
};

class MyStack
{
public:
    MyStack(int capacity)
        : mTop(-1)
        , mCapacity(capacity)
        , arr(nullptr)
    {
        arr = new Coordi[capacity];
    }
    ~MyStack()
    {
        delete[] arr;
    }

    bool push(Coordi data)
    {
        if (isFull())
            return false;

        arr[++mTop] = data;

        return true;
    }

    int getSize() const
    {
        return mTop + 1;
    }

    bool pop(Coordi* data)
    {
        if (isEmpty())
            return false;
        
        data->x = arr[mTop].x;
        data->y = arr[mTop].y;

        mTop--;

        return true;
    }

    bool top(Coordi* data) const
    {
        if (isEmpty())
            return false;

        data->x = arr[mTop].x;
        data->y = arr[mTop].y;

        return true;
    }

    bool isEmpty() const
    {
        if (mTop == -1)
            return true;
        else
            return false;
    }

    bool isFull() const
    {
        if (mTop + 1 == mCapacity)
            return true;
        else
            return false;
    }
private:
    int mTop;
    int mCapacity;
    Coordi* arr;
};

void paintStack(Coordi begin, MyStack& st)
{
    g_funcCount++;
    st.push(begin);
    g_funcCount++;

    while (st.isEmpty() == false)
    {
        g_funcCount++;
        Coordi cur;
        st.pop(&cur);
        g_funcCount++;

        if (cur.x < 0 || cur.y < 0 || cur.x >= 10 || cur.y >= 10 || visit[cur.y][cur.x]) {
            continue;
        }

        visit[cur.y][cur.x] = true;

        for (int i = 0; i < 4; ++i)
        {
            int n_x = cur.x + dx[i];
            int n_y = cur.y + dy[i];

            if (arr[n_y][n_x] == arr[cur.y][cur.x]) {
                st.push(Coordi(n_x, n_y));
                g_funcCount++;
            }
        }

        arr[cur.y][cur.x] = g_ch;
    }
}

Coordi g_stack[1000];
int g_top = -1;

void paintStack2(Coordi begin)
{
    g_funcCount++;
    g_stack[++g_top] = begin;

    while (g_top > -1)
    {
        Coordi cur = g_stack[g_top--];

        if (cur.x < 0 || cur.y < 0 || cur.x >= 10 || cur.y >= 10 || visit[cur.y][cur.x]) {
            continue;
        }

        visit[cur.y][cur.x] = true;

        for (int i = 0; i < 4; ++i)
        {
            int n_x = cur.x + dx[i];
            int n_y = cur.y + dy[i];

            if (arr[n_y][n_x] == arr[cur.y][cur.x]) {
                g_stack[++g_top] = Coordi(n_x, n_y);
            }
        }

        arr[cur.y][cur.x] = g_ch;
    }
}

#define C

int main() {
    int i;
    i = 6;

    printf("%d", i);
    print_array();

    while (1) {
        unsigned int x;
        unsigned int y;

        memset(visit, 0, sizeof(visit));
        printf("Enter position x y(0 ~ 9) :");
        scanf_s("%d %d", &x, &y);
        if (x < 0 || y < 0 || x >= 10 || y >= 10) {
            printf("Wrong Input\n");
            continue;
        }
        printf("Enter character to paint :");
        scanf_s(" %c", &g_ch, 1);
        char c = getc(stdin);

        system("cls");

#ifdef A
        LARGE_INTEGER beginTime;
        LARGE_INTEGER endTime;
        g_funcCount = 0;

        QueryPerformanceCounter(&beginTime);
        paint(x, y);
        QueryPerformanceCounter(&endTime);

        printf("Duration: %lld\n", endTime.QuadPart - beginTime.QuadPart);
        printf("Function Call Count: %d", g_funcCount);
#elif defined B
        MyStack st(10000);

        LARGE_INTEGER beginTime;
        LARGE_INTEGER endTime;
        g_funcCount = 0;

        QueryPerformanceCounter(&beginTime);
        paintStack(Coordi(x, y), st);
        QueryPerformanceCounter(&endTime);

        printf("Duration: %lld\n", endTime.QuadPart - beginTime.QuadPart);
        printf("Function Call Count: %d", g_funcCount);
#elif defined C
        LARGE_INTEGER beginTime;
        LARGE_INTEGER endTime;
        g_funcCount = 0;

        QueryPerformanceCounter(&beginTime);
        paintStack2(Coordi(x, y));
        QueryPerformanceCounter(&endTime);

        printf("Duration: %lld\n", endTime.QuadPart - beginTime.QuadPart);
        printf("Function Call Count: %d", g_funcCount);
#endif
        print_array();
    }
    return 0;
}

void paint(int x, int y)
{
    g_funcCount++;
    if (x < 0 || y < 0 || x >= 10 || y >= 10 || visit[y][x]) {
        return;
    }

    visit[y][x] = true;

    for (int i = 0; i < 4; ++i)
    {
        int n_x = x + dx[i];
        int n_y = y + dy[i];

        if (arr[n_y][n_x] == arr[y][x]) {
            paint(n_x, n_y);
        }
    }

    arr[y][x] = g_ch;
}

void print_array()
{
    for (int i = 0; i < ARR_SIZE; ++i)
    {
        printf("%s\n", arr[i]);
    }
}