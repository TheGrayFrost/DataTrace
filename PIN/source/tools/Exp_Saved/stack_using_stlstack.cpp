// CPP program to demonstrate working of STL stack 
#include <iostream> 
#include <stack> 
using namespace std; 

stack <char> s;

int main()
{
    int num = 0;
    //initStack();
    char ch[] = "vishesh";
    while (ch[num] != '\0')
        s.push(ch[num++]);
    while (!s.empty())
    {
    	//cout << s.top();
        s.pop();
    }
    //cout << "\n";
    return 0;
}