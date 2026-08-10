#include <iostream>

import ACTL;

struct Callable {
    int operator ()(float f) const noexcept {
        return int(f) + 1;
    }
};

int Function(float f) {
    return int(f) + 3;
}

int main() {
    ACTL::Delegate<int(float)> delegate = Function;

    if (delegate(1.0f) != 4) {
        std::cout << "Test FAILED!" << std::endl;

        return 0;
    }

    delegate = Callable();

    if (delegate(1.0f) != 2) {
        std::cout << "Test FAILED!" << std::endl;

        return 0;
    }

    std::cout << "Test PASSED!" << std::endl;

    return 0;
}