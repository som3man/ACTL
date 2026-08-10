#include <iostream>

import ACTL;

template <typename Type>
struct Class {
    static inline unsigned count = 0;

    Type value = 0;

    Class() {
        count++;
    }

    Class(Type value) {
        this->value = value;

        count++;
    }

    Class(const Class& other) {
        count++;

        value = other.value;
    }

    Class(Class&& other) {
        count++;

        value = other.value;
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

template <typename Type>
std::ostream& operator <<(std::ostream& ostream, const Class<Type>& c) {
    return ostream << c.value;
}

bool SettersTest() {
    ACTL::Option<Class<int>, Class<float>, Class<bool>> option = Class<int>(10);

    if (!Class<int>::count || Class<float>::count || Class<bool>::count)
        return false;

    option = Class<bool>(true);

    if (Class<int>::count || Class<float>::count || !Class<bool>::count)
        return false;

    option = Class<float>(0.19f);

    if (Class<int>::count || !Class<float>::count || Class<bool>::count)
        return false;

    option.Set<Class<int>>();

    if (!Class<int>::count || Class<float>::count || Class<bool>::count)
        return false;

    return true;
}

bool GettersTest() {
    ACTL::Option<Class<int>, Class<float>, Class<bool>> option = Class<int>(10);

    try {
        option.GetOrExcept<Class<bool>>();

        return false;
    }
    catch (const char*) {}

    try {
        auto& c = option.GetOrExcept<Class<int>>();

        if (c.value != 10)
            return false;
    }
    catch (const char*) {
        return false;
    }

    option = Class<float>(2077.0f);

    try {
        auto& c = option.GetOrExcept<Class<float>>();

        if (c.value != 2077.0f)
            return false;
    }
    catch (const char*) {
        return false;
    }

    auto c = option.GetOrNew<Class<bool>>(true);

    try {
        option.GetOrExcept<Class<float>>();
    }
    catch (const char*) {
        return false;
    }

    return true;
}

bool HeapTest() {
    ACTL::Option<ACTL::Heap<Class<int>>, ACTL::Heap<Class<float>>, Class<bool>> option = ACTL::Heap<Class<int>>(20);

    if (!Class<int>::count || Class<float>::count || Class<bool>::count)
        return false;

    try {
        if (option.GetOrExcept<ACTL::Heap<Class<int>>>().value != 20)
            return false;
    }
    catch (const char*) {
        return false;
    }

    option = Class<bool>(false);

    if (Class<int>::count || Class<float>::count || !Class<bool>::count)
        return false;

    try {
        option.GetOrExcept<ACTL::Heap<Class<int>>>();

        return false;
    }
    catch (const char*) {}

    try {
        if (option.GetOrExcept<Class<bool>>().value == true)
            return false;
    }
    catch (const char*) {
        return false;
    }

    option.Set<ACTL::Heap<Class<float>>>(3.14f);

    if (Class<int>::count || !Class<float>::count || Class<bool>::count)
        return false;

    try {
        if (option.GetOrExcept<ACTL::Heap<Class<float>>>().value != 3.14f)
            return false;
    }
    catch (const char*) {
        return false;
    }

    try {
        option = ACTL::Heap<Class<float>>();

        return false;
    }
    catch (const char*) {};

    return true;
}

bool VoidTest() {
    ACTL::Option<ACTL::Void, ACTL::Heap<Class<int>>, Class<float>> option = {};

    if (Class<int>::count || Class<float>::count)
        return false;

    option = ACTL::Heap<Class<int>>(0);

    if (!Class<int>::count || Class<float>::count)
        return false;

    option.Set<ACTL::Void>();

    if (Class<int>::count || Class<float>::count)
        return false;

    try {
        option.GetOrExcept<ACTL::Heap<Class<int>>>();

        return false;
    }
    catch (const char*) {};

    return true;
}

bool AssignTest() {
    ACTL::Option<ACTL::Void, Class<int>, Class<float>> a = {}, b = {};

    a = Class<int>(127);

    a = b;

    if (Class<int>::count)
        return false;

    a = Class<float>(3.14f);

    b = a;

    if (Class<float>::count != 2)
        return false;

    return true;
}

int main() {
    if (SettersTest())
        std::cout << "Setters test: PASSED!" << std::endl;
    else
        std::cout << "Setters test: FAILED!" << std::endl;

    if (GettersTest())
        std::cout << "Getters test: PASSED!" << std::endl;
    else
        std::cout << "Getters test: FAILED!" << std::endl;

    if (HeapTest())
        std::cout << "Heap test: PASSED!" << std::endl;
    else
        std::cout << "Heap test: FAILED!" << std::endl;

    if (VoidTest())
        std::cout << "Void test: PASSED!" << std::endl;
    else
        std::cout << "Void test: FAILED!" << std::endl;

    if (AssignTest())
        std::cout << "Assign test: PASSED!" << std::endl;
    else
        std::cout << "Assign test: FAILED!" << std::endl;

    return 0;
}