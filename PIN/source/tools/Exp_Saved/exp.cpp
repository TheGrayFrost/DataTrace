#include "exp.h"
#include <cmath>

v u, r[3];
int c;

int main()
{
	c = 5;
	u.a = 3;
	u.b[5] = 9;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 10; ++j)
			r[i].b[j] = mult(i,j);
	c = sqrt(c);
	return 0;
}