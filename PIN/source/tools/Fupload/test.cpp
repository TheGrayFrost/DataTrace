#include <iostream>
#include <vector>
#include <set>
#include <queue>

using namespace std;

vector <int> Q;
set <int> V;
queue <int> U;
priority_queue <float> R;


string A[5];
int main()
{
	Q.push_back(5);
    if (Q.empty())
        Q.push_back(7);
    Q.pop_back();
    V.insert(5);
    U.push(3);
    U.pop();
    R.push(3);
    R.pop();
	return 0;	
}