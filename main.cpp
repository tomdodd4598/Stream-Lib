#include "stream.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
	dodd::stream(std::vector<char>{'1', '2', '3', '4', '5', '6', '7', '8', '9'})
		.map([](auto x) { return std::string{ x }; })
		.map([](auto x) { return std::stoi(x); })
		.filter([](auto x) { return (x & 1) == 1; })
		.map([](auto x) { return x * x; })
		.for_each([](auto x) { std::cout << x << '\n';  });

	auto a = std::vector<char>{ '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	auto b = dodd::stream(a)
		.map([](auto x) { return static_cast<int>(x) - 0x30; })
		.filter([](auto x) { return x % 3 == 0; })
		.map([](auto x) { return x / 1.5f; })
		.to_vector<float>();

	for (auto e : b) {
		std::cout << e << '\n';
	}
}
