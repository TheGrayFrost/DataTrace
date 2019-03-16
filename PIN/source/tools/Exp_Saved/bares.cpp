#include <iostream>

using namespace std;

int a[3];

int main()
{
	int b[3];
	for (int i = 0; i < 1; ++i)
	{
		a[i] = i;
		b[i] = 2*i;
		a[0] = i;
		if (i == 0)
		{
			int c[1000];
			int j = 5;
			c[j] = 9;
			cout << "here\n";
		}
		else
		{
			int j = 5;
		}
		//cout << c[5] << "\n";
	}
	return 0;
}