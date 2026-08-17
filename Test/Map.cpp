#include <iostream>

import ACTL;

struct Class {
    static inline unsigned count = 0;

    int value = 0;

    Class() {
        count++;
    }

    Class(int value) : value(value) {
        count++;
    }

    Class(const Class& other) : value(other.value) {
        count++;
    }

    Class(Class&& other) : value(other.value) {
        count++;
    }

    ~Class() {
        count--;
    }

    Class& operator =(const Class& other) {
        value = other.value;

        return *this;
    }

    Class& operator =(Class&& other) {
        value = other.value;

        return *this;
    }
};

bool EmplaceAndEraseTest() {
    ACTL::Map<Class, ACTL::String<ACTL::Symbol>> map = 32;

    if (Class::count)
        return false;

    map.EmplaceOrSet("BlaBla"_ustr);

    if (Class::count != 1)
        return false;

    map.EmplaceOrSet("Rapira"_ustr);

    if (Class::count != 2)
        return false;

    map.EmplaceOrSet("Джарона"_ustr);

    if (Class::count != 3)
        return false;

    map.Erase("Rapira"_ustr);

    if (Class::count != 2)
        return false;

    map.EmplaceOrSet("Trolo"_ustr);

    if (Class::count != 3)
        return false;

    map.Erase("BlaBla"_ustr);

    if (Class::count != 2)
        return false;

    map.Clear();

    if (Class::count)
        return false;

    return true;
}

bool RehashTest() {
    ACTL::Map<Class, ACTL::String<ACTL::Symbol>> map = 16;

    map.EmplaceOrSet("L"_ustr);

    map.EmplaceOrSet("D"_ustr);

    map.EmplaceOrSet("A"_ustr);

    map.EmplaceOrSet("b"_ustr);

    map.Expand();

    if (Class::count != 4)
        return false;

    map.EmplaceOrSet("H"_ustr);

    map.EmplaceOrSet("J"_ustr);

    if (Class::count != 6)
        return false;

    return true;
}

int main() {
    if (EmplaceAndEraseTest())
        std::cout << "Emplace And Erase test: PASSED!" << std::endl;
    else
        std::cout << "Emplace And Erase test: FAILED!" << std::endl;

    if (RehashTest())
        std::cout << "Rehash test: PASSED!" << std::endl;
    else
        std::cout << "Rehash test: FAILED!" << std::endl;

    return 0;
}