#include<iostream>
#define MAX 100
 
using namespace std;

int main()
{
    char STACK[MAX];
    int TOP = -1;

    int num = 0;
    char ch[] = "ab";
    while (ch[num] != '\0')
    {
        TOP++;
        STACK[TOP] = ch[num];
        num++;
    }
    while (TOP != -1)
    {
        cout << STACK[TOP];
        TOP--;
    }
    cout << "\n";
    return 0;
}