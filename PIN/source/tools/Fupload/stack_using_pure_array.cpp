#include<iostream>
#define MAX 100
 
using namespace std;

char STACK[MAX];
int TOP = -1;

int main(){
    int num = 0;
    char ch[] = "vishesh";
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