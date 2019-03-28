int sample_func(int x, int y){
	int z = x * x + 2 * y;
	z = z - 5;
	return z;
}

int main(){
	int x = 4;
	int y = 5;
	int z = sample_func(x, y);
	int *ptr = &x;
	int *ptr2 = &y;
	return 0;
}
