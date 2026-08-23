#include <iostream>

import ACTL;

class Class {
public:
    int value = 0;

    static constinit inline ACTL::u32 count = 0;

    Class() noexcept {
        count++;
    }

    Class(const Class& other) noexcept {
        count++;

        operator=(other);
    }

    Class(Class&& other) noexcept {
        count++;

        operator=(ACTL::move(other));
    }

    Class(int value) noexcept {
        this->value = value;

        count++;
    }

    Class& operator =(const Class& other) noexcept {
        value = other.value;

        return *this;
    }

    Class& operator =(Class&& other) noexcept {
        value = other.value;

        other.value = 0;

        return *this;
    }

    ~Class() noexcept {
        count--;
    }

    bool operator !=(const Class& other) const noexcept {
        return value != other.value;
    }
};

std::ostream& operator <<(std::ostream& ostream, const Class& c) noexcept {
    return ostream << c.value;
}

template <typename Type, ACTL::u32 capacity>
bool Compare(const ACTL::Array<Type>& array, const Type (&other)[capacity]) noexcept {
    if (array.getLength() != capacity)
        return false;

    for (ACTL::u32 i = 0; i < capacity; i++)
        if (array.begin()[i] != other[i])
            return false;

    return true;
}

template <typename Type, ACTL::u32 capacity, ACTL::u32 capacity2>
bool CompareStatic(const ACTL::StaticArray<Type, capacity>& array, const Type (&other)[capacity2]) noexcept {
    if (array.getLength() != capacity2)
        return false;

    for (ACTL::u32 i = 0; i < capacity2; i++)
        if (array.begin()[i] != other[i])
            return false;

    return true;
}

bool AllocationTest() noexcept {
    ACTL::Array<Class> array = {};

    if (array.getCapacity())
        return false;

    array.Allocate(23);

    if (array.getCapacity() != 23)
        return false;

    array.Free();

    if (array.getCapacity())
        return false;

    return true;
}

bool InitializerListTest() noexcept {
    ACTL::Array<int> array = {};

    array = {
        0, 1, 2, 3, 4, 5
    };

    if (array.getCapacity() != 6)
        return false;

    return Compare(array, { 0, 1, 2, 3, 4, 5 });
}

bool ExpandTest() noexcept {
    ACTL::Array<int> array = {
        0, 1, 2, 3, 4, 5
    };

    array.Expand(20);

    if (array.getCapacity() != 26)
        return false;

    return Compare(array, { 0, 1, 2, 3, 4, 5 });
}

bool EmplaceAndEraseBackTest() noexcept {
    ACTL::Array<Class> array = {};

    if (Class::count)
        return false;

    array.EmplaceBack(5);

    if (array.getCapacity() != 8)
        return false;

    if (Class::count != 1)
        return false;

    array.EmplaceBack(10);

    if (Class::count != 2)
        return false;

    array.EmplaceBack(-9);

    if (Class::count != 3)
        return false;

    array.EraseBack();

    array.EraseBack();

    array.EraseBack();

    if (Class::count)
        return false;

    return true;
}

bool EmplaceAndEraseStrictTest() noexcept {
    ACTL::Array<Class> array = {};

    array.EmplaceStrict(0, 99);

    if (!Compare(array, { Class(99) }))
        return false;

    if (Class::count != 1)
        return false;

    array = {
        0, 1, 2, 3, 4
    };

    if (Class::count != 5)
        return false;

    array.EmplaceStrict(0, 99);

    if (!Compare(array, {
        Class(99), Class(0), Class(1), Class(2), Class(3), Class(4)
    })) return false;

    if (Class::count != 6)
        return false;

    array.EmplaceStrict(3, 100);

    array.EmplaceStrict(ACTL::u32max, -5);

    if (Class::count != 8)
        return false;

    if (!Compare(array, {
        Class(99), Class(0), Class(1), Class(100), Class(2), Class(3), Class(4), Class(-5)
    })) return false;

    array.EraseStrict(0);

    array.EraseStrict(2);

    if (Class::count != 6)
        return false;

    if (!Compare(array, {
        Class(0), Class(1), Class(2), Class(3), Class(4), Class(-5)
    })) return false;

    return true;
}

bool EmplaceAndEraseManyTest() {
    ACTL::Array<Class> array = {};

    array.EmplaceBackMany(4, 68);

    if (!Compare(array, {
        Class(68), Class(68), Class(68), Class(68)
    })) return false;

    array.EmplaceBackMany(5, 7);

    if (!Compare(array, {
        Class(68), Class(68), Class(68), Class(68),
        Class(7), Class(7), Class(7), Class(7), Class(7),
    })) return false;

    array.EraseBackMany(6);

    if (!Compare(array, {
        Class(68), Class(68), Class(68)
    })) return false;

    return true;
}

bool GettersTest() noexcept {
    ACTL::Array<Class> array = {
        0, 1, 2, 3
    };

    try {
        auto& v = array.GetOrExcept(1);
    }
    catch (const char*) {
        return false;
    }

    try {
        auto& v = array.GetOrExcept(10);

        return false;
    }
    catch (const char*) {
        
    }

    auto v = array.GetOrNew(2, -1);

    if (v.value == -1)
        return false;

    v = array.GetOrNew(100, -1);

    if (v.value != -1)
        return false;

    v = array.GetOrEmplace(4, 4);

    if (!Compare(array, {
        Class(0), Class(1), Class(2), Class(3), Class(4)
    })) return false;

    // 5 and +1 because of "v" variable
    if (Class::count != 6)
        return false;

    v = array.GetOrEmplace(6, 10);

    if (!Compare(array, {
        Class(0), Class(1), Class(2), Class(3), Class(4), Class(), Class(10)
    })) return false;

    if (Class::count != 8)
        return false;

    v = array.GetOrEmplace(10, 10);

    if (!Compare(array, {
        Class(0), Class(1), Class(2), Class(3), Class(4), Class(), Class(10), Class(), Class(), Class(), Class(10)
    })) return false;

    if (array.getCapacity() != 16)
        return false;

    if (Class::count != 12)
        return false;

    return true;
}

bool ConcatentaionTest() {
    ACTL::Array<Class> array = {};

    array += ACTL::Array<Class>({
        Class(0), Class(1), Class(2)
    });

    if (!Compare(array, {
        Class(0), Class(1), Class(2)
    })) return false;

    array += ACTL::Array<Class>({
        Class(55), Class(55), Class(56)
    });

    if (!Compare(array, {
        Class(0), Class(1), Class(2), Class(55), Class(55), Class(56)
    })) return false;

    return true;
}

bool InsertionTest() {
    ACTL::Array<Class> array = {
        0, 1, 2
    };

    array.Insert(0, { 4, 4 });

    if (!Compare(array, {
        Class(4), Class(4), Class(0), Class(1), Class(2)
    })) return false;

    array.Insert(2, { 5, 5, 5 });

    if (!Compare(array, {
        Class(4), Class(4), Class(5), Class(5), Class(5), Class(0), Class(1), Class(2)
    })) return false;

    return true;
}

bool StaticEmplaceAndEraseTest() {
    ACTL::StaticArray<Class, 4> array = {};

    array.EmplaceBack(0);

    if (!CompareStatic<Class>(array, {
        Class(0)
    })) return false;

    array.EmplaceBack(1);

    array.EmplaceBack(10);

    if (!CompareStatic<Class>(array, {
        Class(0), Class(1), Class(10)
    })) return false;

    array.EmplaceStrict(1, -1);

    if (!CompareStatic<Class>(array, {
        Class(0), Class(-1), Class(1), Class(10)
    })) return false;

    try {
        array.EmplaceBack(0);

        return false;
    }
    catch (const char*) {};

    auto& c = array.EmplaceBackOrGet(0);

    if (c.value != 10)
        return false;

    array.EraseBack();

    if (!CompareStatic<Class>(array, {
        Class(0), Class(-1), Class(1)
    })) return false;

    array.EraseStrict(0);

    if (!CompareStatic<Class>(array, {
        Class(-1), Class(1)
    })) return false;

    return true;
}

int main() {
    if (AllocationTest())
        std::cout << "Allocation test: PASSED!" << std::endl;
    else
        std::cout << "Allocation test: FAILED!" << std::endl;

    if (InitializerListTest())
        std::cout << "Initializer list test: PASSED!" << std::endl;
    else
        std::cout << "Initializer list test: FAILED!" << std::endl;

    if (ExpandTest())
        std::cout << "Expand test: PASSED!" << std::endl;
    else
        std::cout << "Expand test: FAILED!" << std::endl;

    if (EmplaceAndEraseBackTest())
        std::cout << "Emplace and Erase Back test: PASSED!" << std::endl;
    else
        std::cout << "Emplace and Erase Back test: FAILED!" << std::endl;

    if (EmplaceAndEraseStrictTest())
        std::cout << "Emplace and Erase Strict test: PASSED!" << std::endl;
    else
        std::cout << "Emplace and Erase Strict test: FAILED!" << std::endl;

    if (EmplaceAndEraseManyTest())
        std::cout << "Emplace and Erase Many test: PASSED!" << std::endl;
    else
        std::cout << "Emplace and Erase Many test: FAILED!" << std::endl;

    if (GettersTest())
        std::cout << "Getters test: PASSED!" << std::endl;
    else
        std::cout << "Getters test: FAILED!" << std::endl;

    if (ConcatentaionTest()) 
        std::cout << "Concatentaion test: PASSED!" << std::endl;
    else
        std::cout << "Concatentaion test: FAILED!" << std::endl;

    if (InsertionTest()) 
        std::cout << "Insertion test: PASSED!" << std::endl;
    else
        std::cout << "Insertion test: FAILED!" << std::endl;

    if (StaticEmplaceAndEraseTest())
        std::cout << "Static Emplace and Erase test: PASSED!" << std::endl;
    else
        std::cout << "Static Emplace and Erase test: FAILED!" << std::endl;

    return 0;
}