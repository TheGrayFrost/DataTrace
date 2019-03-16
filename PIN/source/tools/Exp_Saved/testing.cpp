#include <iostream> 

using namespace std;

int two (int a)
{
	return 2*a;
}

int oneplus (int& a)
{
	return 2*a;
}

int main() 
{	
	int x = 5;
	int y = x;
	int a[] = {2};
	int b[3];
	int j = 1;
	for (int i = 0; i < 1; ++i)
		b[a[i]] = i;
	// for (int i = 0; i < j; ++i)
	// 	b[a[i]] = i;
	// for (int i = 0; i < j; ++i)
	// 	b[a[i]] = two(i);
	// for (int i = 0; i < j; ++i)
	// 	b[a[i]] = oneplus(i);
	// for (int i = 0; i < j; ++i)
	// 	b[a[i]] = two(i) + oneplus(i);
	return 0; 
}
