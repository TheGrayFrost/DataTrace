#include <iostream>
#include <vector>
#include <string>

std::vector <std::string> parse (std::string& diss)
{
	std::vector <std::string> ops;
	int l = diss.find(" ");
	ops.push_back(diss.substr(0, l));
	//std::cout << l << "\n";
	if (l == diss.length()-1)
		return ops;
	int r = diss.find(",");
	if (r == -1)
		ops.push_back(diss.substr(l+1, r));
	else
	{
		ops.push_back(diss.substr(l+1, r-l-1));
		ops.push_back(diss.substr(r+2, -1));
	}
	return ops;
}

int main()
{
	std::string l = "ok ";
	auto p = parse(l);
	//std::cout << l << " " << l.length() << "\n";
	for (auto u: p)
		std::cout << u << " " << u.length() << "\n";
	return 0;
}