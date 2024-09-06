#include "stream.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
	dodd::stream(std::vector<char>{'1', '2', '3', '4', '5'})
		.map([](auto x) { return std::string{ x }; })
		.map([](auto x) { return std::stoi(x); })
		.map([](auto x) { return x * x; })
		.for_each([](auto x) { std::cout << x << '\n';  });
}
