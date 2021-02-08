#include <stdio.h>
#include <string>
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

void paint(int x, int y);
void print_array();

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
        fflush(stdin);

        paint(x, y);

        system("cls");

        print_array();
    }

    return 0;
}

void paint(int x, int y)
{
    visit[y][x] = true;

    for (int i = 0; i < 4; ++i)
    {
        int n_x = x + dx[i];
        int n_y = y + dy[i];

        if (n_x < 0 || n_y < 0 || n_x >= 10 || n_y >= 10 || visit[n_y][n_x]) {
            continue;
        }

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