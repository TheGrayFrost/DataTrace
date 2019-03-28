struct Sample{
	int a;
	float c;
};

struct Sample2{
	int x;
	int y;
	struct Sample z;
};

int main(){
	struct Sample a;
	struct Sample2 b;
	a.a = 4;
	a.c = 7.0;
	b.z = a;
	return 0;
}

