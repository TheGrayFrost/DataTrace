// CPP program to demonstrate working of STL stack 
#include <iostream> 
#include <vector> 
using namespace std; 

vector <char> s;

int main()
{
    int num = 0;
    //initStack();
    char ch[] = "vishesh";
    while (ch[num] != '\0')
        s.push_back(ch[num++]);
    while (!s.empty())
    {
    	cout << s.back();
        s.pop_back();
    }
    cout << "\n";
    return 0;
}