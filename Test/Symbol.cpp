#include <iostream>

import ACTL;

bool SymbolConvertTest() {
    ACTL::Symbol symbol = {};

    try {
        symbol = '\254';

        return false;
    }
    catch (const char*) {};

    try {
        symbol = 'a';
    }
    catch (const char*) {
        return false;
    }

    if (symbol.ToString<char>() != "a"_str)
        return false;

    if (symbol.ToString<wchar_t>() != L"a"_str)
        return false;

    try {
        symbol = "п";
    }
    catch (const char*) {
        return false;
    }

    if (symbol.ToString<char>() != "п"_str)
        return false;

    char32_t s = symbol;

    if (s != U'п')
        return false;

    try {
        symbol = L"Ъ";
    }
    catch (const char*) {
        return false;
    }

    if (symbol.ToString<char>() != "Ъ"_str)
        return false;

    s = symbol;

    if (s != U'Ъ')
        return false;

    return true;
}

bool StringConvertTest() {
    ACTL::String<ACTL::Symbol> string = {};

    try {
        string = ACTL::String<char>("80877"_str);
    }
    catch (const char*) {
        return false;
    }

    if (string.getLength() != 5)
        return false;

    if (string.ConvertTo<char>() != "80877"_str)
        return false;

    if (string.ConvertTo<wchar_t>() != L"80877"_str)
        return false;

    try {
        string = ACTL::String<char>("po\254po\253"_str);

        return false;
    }
    catch (const char*) {};

    try {
        string = ACTL::String<wchar_t>(L"Яблоко"_str);
    }
    catch (const char*) {
        return false;
    }

    if (string.getLength() != 6)
        return false;

    if (string.ConvertTo<char>() != "Яблоко"_str)
        return false;

    if (string.ConvertTo<wchar_t>() != L"Яблоко"_str)
        return false;

    return true;
}

bool StringGetterTest() {
    ACTL::String<ACTL::Symbol> string = "01234567"_ustr;

    if (string[0] != "0"_sym)
        return false;

    if (string[1] != "1"_sym)
        return false;

    if (string[2] != "2"_sym)
        return false;

    if (string[7] != "7"_sym)
        return false;

    if (string[10] != ""_sym)
        return false;

    string = "Яблоко_sss_ІВАН"_ustr;

    if (string[0] != "Я"_sym)
        return false;

    if (string[6] != "_"_sym)
        return false;

    if (string[8] != "s"_sym)
        return false;

    if (string[12] != "В"_sym)
        return false;

    return true;
}

bool StringEmplaceTest() {
    ACTL::String<ACTL::Symbol> string = "01234567"_ustr;

    string.EmplaceBack("8"_sym);

    if (string != "012345678"_ustr)
        return false;

    string.Emplace(4, "4"_sym);

    string.Emplace(0, "Б"_sym);

    if (string != "Б0123445678"_ustr)
        return false;

    string.Set(13, "п");

    if (string != "Б0123445678ппп"_ustr)
        return false;

    string.Insert(1, "000"_ustr);

    if (string != "Б0000123445678ппп"_ustr)
        return false;

    return true;
}

bool StringEraseTest() {
    ACTL::String<ACTL::Symbol> string = "世界和平"_ustr;

    string.EraseBack();

    if (string != "世界和"_ustr)
        return false;

    string.Erase(0);

    if (string != "界和"_ustr)
        return false;

    string = "Ультракилл"_ustr;

    string.Erase(0);

    if (string != "льтракилл"_ustr)
        return false;

    string.Erase(3);

    if (string != "льтакилл"_ustr)
        return false;

    string.Erase(5);

    if (string != "льтаклл"_ustr)
        return false;

    string.EraseBack();

    if (string != "льтакл"_ustr)
        return false;

    return true;
}

int main() {
    if (SymbolConvertTest())
        std::cout << "Symbol Convert test: PASSED!" << std::endl;
    else
        std::cout << "Symbol Convert test: FAILED!" << std::endl;

    if (StringConvertTest())
        std::cout << "String Convert test: PASSED!" << std::endl;
    else
        std::cout << "String Convert test: FAILED!" << std::endl;

    if (StringGetterTest())
        std::cout << "String Getter test: PASSED!" << std::endl;
    else
        std::cout << "String Getter test: FAILED!" << std::endl;

    if (StringEmplaceTest())
        std::cout << "String Emplace test: PASSED!" << std::endl;
    else
        std::cout << "String Emplace test: FAILED!" << std::endl;

    if (StringEraseTest())
        std::cout << "String Erase test: PASSED!" << std::endl;
    else
        std::cout << "String Erase test: FAILED!" << std::endl;

    return 0;
}