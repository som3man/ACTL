#include <iostream>

import ACTL;

bool Compare(const ACTL::String<char>& string, const char* cstring) {
    try {
        for (ACTL::u32 i = 0; i < string.getArray().getLength(); i++)
            if (string.getArray()[i] != cstring[i])
                return false;
    }
    catch (const char*) {
        return false;
    }

    return true;
}

using String = ACTL::String<char>;

bool InitTest() {
    String str1 = {};

    if (!Compare(str1, ""))
        return false;
    
    String str2 = "BlaBla"_str;

    if (!Compare(str2, "BlaBla"))
        return false;

    return true;
}

bool AssignTest() {
    String string = {};

    string = "BlaBla1234"_str;

    if (!Compare(string, "BlaBla1234"))
        return false;

    string = "Foo"_str;

    if (!Compare(string, "Foo"))
        return false;

    String string2 = "Hail Mary"_str;

    string = string2;

    if (!Compare(string, "Hail Mary"))
        return false;

    return true;
}

bool ConcatenationTest() {
    String string = "01234"_str;

    string += "56789"_str;

    if (!Compare(string, "0123456789"))
        return false;

    String string2 = string + " - List of numbers"_str;

    if (!Compare(string2, "0123456789 - List of numbers"))
        return false;

    return true;
}

bool ToStringTest() {
    String string = {};

    unsigned number = 101;

    string << number;

    if (!Compare(string, "101"))
        return false;

    string.Clear();

    string << -567;

    if (!Compare(string, "-567"))
        return false;

    return true;
}

bool FromStringTest() {
    String string = "12345"_str;

    ACTL::u64 num;

    try {
        string >> num;
    }
    catch (const char*) {
        return false;
    }

    if (num != 12345)
        return false;

    string = "688a"_str;

    try {
        string >> num;

        return false;
    }
    catch (const char*) {};

    return true;
}

bool EmplaceTest() {
    String string = "012345"_str;

    string.EmplaceBack('6');

    if (!Compare(string, "0123456"))
        return false;

    string.Emplace(0, '-');

    if (!Compare(string, "-0123456"))
        return false;

    string.Emplace(2, 'b');

    if (!Compare(string, "-0b123456"))
        return false;

    string.Insert(4, "iii"_str);

    if (!Compare(string, "-0b1iii23456"))
        return false;

    string.Clear();

    string.Set(5, '4');

    if (!Compare(string, "444444"))
        return false;

    return true;
}

bool EraseTest() {
    String string = "ababab"_str;

    string.EraseBack();

    if (!Compare(string, "ababa"))
        return false;

    string.Erase(0);

    if (!Compare(string, "baba"))
        return false;

    string.Erase(1);

    if (!Compare(string, "bba"))
        return false;

    return true;
}

int main() {
    if (InitTest())
        std::cout << "Init test: PASSED!" << std::endl;
    else 
        std::cout << "Init test: FAILED!" << std::endl;

    if (AssignTest())
        std::cout << "Assign test: PASSED!" << std::endl;
    else 
        std::cout << "Assign test: FAILED!" << std::endl;  

    if (ConcatenationTest())
        std::cout << "Concatenation test: PASSED!" << std::endl;
    else 
        std::cout << "Concatenation test: FAILED!" << std::endl;  
    
    if (ToStringTest())
        std::cout << "To String test: PASSED!" << std::endl;
    else 
        std::cout << "To String test: FAILED!" << std::endl;  

    if (FromStringTest())
        std::cout << "From String test: PASSED!" << std::endl;
    else 
        std::cout << "From String test: FAILED!" << std::endl;
    
    if (EmplaceTest())
        std::cout << "Emplace test: PASSED!" << std::endl;
    else 
        std::cout << "Emplace test: FAILED!" << std::endl;

    if (EraseTest())
        std::cout << "Erase test: PASSED!" << std::endl;
    else 
        std::cout << "Erase test: FAILED!" << std::endl;

    return 0;
}