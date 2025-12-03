#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <math.h>

#include "include/ACTL/ACTL.hpp"

class SomeClass {
public:
	int value;

	SomeClass() : value(35) {
		
	}

	SomeClass(int val) : value(val) {
		
	}

	SomeClass(const SomeClass& Other) : value(Other.value) {
		
	}

	~SomeClass() {
		
	}
};

std::ostream& operator <<(std::ostream& ostream, const SomeClass& Something) {
	return ostream << Something.value;
}

int main() {
	SomeClass Something;

	ACTL::Tuple<ACTL::String<char>, float, SomeClass> tuple("Someman", 0.0f, Something);

	std::cout << (ACTL::String<char>&)tuple << std::endl;

	return 0;
}
