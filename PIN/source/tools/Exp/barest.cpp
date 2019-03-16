#include <iostream>
#include <thread>

void foo(int x)
{
	int l = x;
}

int main()
{
	int l = 12;
	std::thread t(foo, 3);
	int r = 11;
	t.join();
	int * p = &r;
	return 0;
}