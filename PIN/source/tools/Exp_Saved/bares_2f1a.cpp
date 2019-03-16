#include <iostream>

using namespace std;

int a[3];

int func(int * b)
{
	return b[1];
}

int main()
{
	for (int i = 0; i < 3; ++i)
		a[i] = i;
	a[0] = func(a);
	return 0;
}