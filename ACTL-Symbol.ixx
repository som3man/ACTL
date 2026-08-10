/*
MIT License

Copyright (c) 2026 som3man

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

module;

#include <iostream>
#include <ostream>
#include <type_traits>

export module ACTL:Symbol;

import :String;
import :Array;

export namespace ACTL {
    class Symbol {
        char32_t value = 0;

    public:
        constexpr Symbol() noexcept = default;

        constexpr Symbol(const Symbol&) noexcept = default;

        constexpr Symbol(Symbol&&) noexcept = default;

        constexpr Symbol(auto other) {
            if constexpr (sizeof(other) == 1) {
                if (other & 0b10000000)
                    throw "Invalid utf-8 symbol!";
            }
            else if constexpr (sizeof(other) == 2) {
                if (other > 0xD7FF && other < 0xE000)
                    throw "Invalid utf-16 symbol!";
            }
            else if constexpr (sizeof(other) == 4) {
                if (other < 0 || other > 0x10FFFF)
                    throw "Invalid utf-32 symbol!";

                if (other >= 0xD800 && other <= 0xDFFF)
                    throw "Invalid utf-32 symbol!";
            }
            else {
                static_assert(false, "Invalid symbol type!");
            }

            value = char32_t(other);
        }

        template <typename Char>
        constexpr Symbol(const Char* other, byte length = 0) {
            constexpr auto size = sizeof(Char);

            static_assert(size <= 4, "Invalid symbol type!");

            value = 0;

            if (!other)
                return;

            if (!length) {
                while (other[length]) {
                    length++;

                    if (length == 4 / size)
                        break;
                }
            }

            if (!length)
                return;

            if constexpr (size == 1) {
                constexpr const char* exception = "Invalid utf-8 sequence!";

                if (*other >= char(0x80) && *other <= char(0xC1))
                    throw exception;

                if (*other >= char(0xF5) && *other <= char(0xFF))
                    throw exception;

                switch (length) {
                case 1:
                    if (*other > 0x7F)
                        throw exception;

                    value = char32_t(*other);

                    break;
                case 2:
                    if (*other < char(0xC2) || *other > char(0xDF))
                        throw exception;

                    if ((other[1] & 0xC0) != 0x80)
                        throw exception;

                    value = (char32_t(other[0] & 0x1F) << 6) | (other[1] & 0x3F);

                    break;
                case 3:
                    if (*other < char(0xE0) || *other > char(0xEF))
                        throw exception;

                    if ((other[1] & 0xC0) != 0x80 || (other[2] & 0xC0) != 0x80)
                        throw exception;

                    if (other[0] == char(0xE0) && other[1] < char(0xA0))
                        throw exception;

                    if (other[0] == char(0xED) && other[1] >= char(0xA0))
                        throw exception;

                    value = (char32_t(other[0] & 0x0F) << 12) | 
                        (char32_t(other[1] & 0x3F) << 6) | 
                        (other[2] & 0x3F);

                    break;
                default:
                    if (*other < char(0xF0) && *other > char(0xF4))
                        throw exception;

                    if ((other[1] & 0xC0) != 0x80 || (other[2] & 0xC0) != 0x80 || (other[3] & 0xC0) != 0x80) 
                        throw exception;

                    if (other[0] == char(0xF0) && other[1] < char(0x90))
                        throw exception;

                    if (other[0] == char(0xF4) && other[1] >= char(0x90))
                        throw exception;

                    value = (char32_t(other[0] & 0x07) << 18) | 
                        (char32_t(other[1] & 0x3F) << 12) | 
                        (char32_t(other[2] & 0x3F) << 6) | 
                        (other[3] & 0x3F);

                    break;
                }
            }
            else if constexpr (size == 2) {
                auto IsLow = [](wchar_t c) -> bool {
                    return (c >= 0xDC00 && c <= 0xDFFF);
                };

                auto IsHigh = [](wchar_t c) -> bool {
                    return (c >= 0xD800 && c <= 0xDBFF);
                };

                auto IsValidSingle = [](wchar_t c) -> bool {
                    return (c < 0xD800 || c > 0xDFFF);
                };

                constexpr const char* exception = "Invalid utf-16 sequence!";

                if (length == 1) {
                    if (!IsValidSingle(other[0]))
                        throw exception;
                
                    value = char32_t(other[0]);
                }
                else {
                    if (IsLow(other[0]))
                        throw exception;

                    if (!IsHigh(other[0]))
                        throw exception;

                    if (!IsLow(other[1]))
                        throw exception;

                    value = 0x10000 + ((char32_t(other[0] & 0x3FF) << 10) | char32_t(other[1] & 0x3FF));
                }
            }
            else {
                if (*other < 0 || *other > 0x10FFFF)
                    throw "Invalid utf-32 symbol!";

                if (*other >= 0xD800 && *other <= 0xDFFF)
                    throw "Invalid utf-32 symbol!";

                value = char32_t(*other);
            }
        }

        constexpr ~Symbol() noexcept = default;

        constexpr Symbol& operator =(const Symbol&) noexcept = default;

        constexpr Symbol& operator =(Symbol&&) noexcept = default;

        constexpr operator bool() const noexcept {
            return value;
        }

        constexpr operator char32_t() const noexcept {
            return value;
        }

        template <typename Char>
        constexpr String<Char> ToString() const noexcept {
            String<Char> result = {};

            if (!*this)
                return result;

            if constexpr (sizeof(Char) == 1) {
                if (value <= 0x007F) {
                    result.Set(0, Char(value));
                }
                else if (value <= 0x07FF) {
                    result.Set(0, Char(0xC0 | (value >> 6)));
                    result.Set(1, Char(0x80 | (value & 0x3F)));
                }
                else if (value <= 0xFFFF) {
                    result.Set(0, Char(0xE0 | (value >> 12)));
                    result.Set(1, Char(0x80 | ((value >> 6) & 0x3F)));
                    result.Set(2, Char(0x80 | (value & 0x3F)));
                }
                else {
                    result.Set(0, Char(0xF0 | (value >> 18)));
                    result.Set(1, Char(0x80 | ((value >> 12) & 0x3F)));
                    result.Set(2, Char(0x80 | ((value >> 6) & 0x3F)));
                    result.Set(3, Char(0x80 | (value & 0x3F)));
                }
            }
            else if constexpr (sizeof(Char) == 2) {
                if (value <= 0xFFFF) {
                    result.Set(0, Char(value));
                }
                else {
                    auto cp = value - 0x10000;

                    result.Set(0, Char(0xD800 | (cp >> 10)));
                    result.Set(1, Char(0xDC00 | (cp & 0x03FF)));
                }
            }
            else if constexpr (sizeof(Char) == 4) {
                result.Set(0, Char(value));
            }
            else {
                static_assert(false, "Invalid symbol type!");
            }

            return result;
        }

        constexpr bool operator ==(const Symbol& other) const noexcept {
            return value == other.value;
        }

        constexpr bool operator !=(const Symbol& other) const noexcept {
            return value != other.value;
        }

        constexpr bool operator <=(const Symbol& other) const noexcept {
            return value <= other.value;
        }

        constexpr bool operator >=(const Symbol& other) const noexcept {
            return value >= other.value;
        }

        constexpr bool operator <(const Symbol& other) const noexcept {
            return value < other.value;
        }

        constexpr bool operator >(const Symbol& other) const noexcept {
            return value > other.value;
        }
    };

    template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Symbol& symbol) noexcept {
        return ostream << symbol.ToString<Char>();
    }

    template <>
    class String<Symbol> {
        String<char> data = {};

        u32 symLength = 0;

        static constexpr byte GetSymLength(auto unit) noexcept {
            if constexpr (sizeof(unit) == 1) {
                if ((unit & 0x80) == 0)
                    return 1;
                else if ((unit & 0xE0) == 0xC0)
                    return 2;
                else if ((unit & 0xF0) == 0xE0)
                    return 3;
                else
                    return 4;
            }
            else if constexpr (sizeof(unit) == 2) {
                if (unit >= 0xD800 && unit <= 0xDBFF)
                    return 2;
                else
                    return 1;
            }
            else {
                return 1;
            }
        }

        static constexpr u32 GetByteIndex(u32 symIndex, const char* data) noexcept {
            u32 byteIndex = 0;

            while (symIndex) {
                symIndex--;

                byteIndex += GetSymLength(data[byteIndex]);
            }

            return byteIndex;
        }

    public:
        static constexpr auto null = String<char>::null;

        class View {
            const char* data = &null;

            u32 byteLength = 0;

            u32 symLength = 0;

        public:
            consteval View(const char* cstring) {
                while (cstring[byteLength])
                    byteLength++;

                for (u32 i = 0; i < byteLength;) {
                    auto length = GetSymLength(cstring[i]);

                    Symbol check(cstring + i, length);

                    symLength++;

                    i += length;
                }

                data = cstring;
            }

            constexpr View() noexcept = default;

            constexpr View(const View&) noexcept = default;

            constexpr View(View&&) noexcept = default;

            constexpr ~View() noexcept = default;

            constexpr View& operator =(const View&) noexcept = default;

            constexpr View& operator =(View&&) noexcept = default;

            constexpr u32 getByteLength() const noexcept {
                return byteLength;
            }

            constexpr u32 getLength() const noexcept {
                return symLength;
            }

            constexpr bool notEmpty() const noexcept {
                return byteLength;
            }

            constexpr bool isEmpty() const noexcept {
                return !notEmpty();
            }

            constexpr operator bool() const noexcept {
                return notEmpty();
            }

            constexpr const char* begin() const noexcept {
                return data;
            }

            constexpr const char* end() const noexcept {
                return begin() + byteLength;
            }

            constexpr Symbol operator [](u32 index) const noexcept {
                if (index >= symLength)
                    return Symbol();

                u32 byteIndex = GetByteIndex(index, data);

                return Symbol(data + byteIndex, GetSymLength(data[byteIndex]));
            }

            consteval operator typename String<char>::View() noexcept {
                return String<char>::View(begin());
            }

            template <typename Char>
            constexpr String<Char> ConvertTo() const noexcept {
                if constexpr (std::is_same_v<char, Char>)
                    return String<char>::FromCstring(data, byteLength);

                if constexpr (std::is_same_v<Symbol, Char>)
                    return String<Symbol>(*this);

                String<Char> result = {};

                for (u32 i = 0; i < byteLength;) {
                    auto syml = GetSymLength(data[i]);

                    Symbol symbol(data + i, syml);

                    result += symbol.ToString<Char>();

                    i += syml;
                }

                return result;
            }
        };

        constexpr String() noexcept {};

        constexpr String(const View& other) noexcept {
            symLength = other.getLength();

            if (!symLength)
                return;

            data.Set(other.getByteLength() - 1, null);

            std::copy(other.begin(), other.end(), data.begin());
        }

        template <typename Char>
        constexpr String(const String<Char>& other) {
            for (u32 i = 0; i < other.getLength();) {
                auto length = GetSymLength(other.begin()[i]);

                Symbol sym(other.begin() + i, length);

                data += sym.ToString<char>();

                symLength++;

                i += length;
            }
        }

        constexpr String(const String& other) noexcept {
            operator=(other);
        }

        constexpr String(String&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~String() noexcept {};

        constexpr String& operator =(const String& other) noexcept {
            data = other.data;

            symLength = other.symLength;

            return *this;
        }

        constexpr String& operator =(String&& other) noexcept {
            data = ACTL::move(other.data);

            symLength = other.symLength;

            other.symLength = 0;

            return *this;
        }

        constexpr String& operator +=(const String& other) noexcept {
            data += other.data;

            symLength += other.symLength;

            return *this;
        }

        constexpr String operator +(const String& other) const noexcept {
            String result = *this;

            result += other;

            return result;
        }

        constexpr void Clear() noexcept {
            data.Clear();

            symLength = 0;
        }

        constexpr void EmplaceBack(Symbol symbol) noexcept {
            data += symbol.ToString<char>();

            symLength++;
        }

        constexpr void Emplace(u32 index, Symbol symbol) noexcept {
            if (index >= symLength)
                return EmplaceBack(symbol);

            u32 byteIndex = GetByteIndex(index, data.begin());

            data.Insert(byteIndex, symbol.ToString<char>());

            symLength++;
        }

        constexpr void Set(u32 index, Symbol symbol) noexcept {
            if (index <= symLength)
                return Emplace(index, symbol);

            u32 count = index - symLength + 1;

            auto str = symbol.ToString<char>();

            symLength += count;

            while (count) {
                count--;

                data += str;
            }
        }

        constexpr void Insert(u32 index, const String& other) noexcept {
            symLength += other.symLength;

            if (index >= symLength) {
                data += other.data;

                return;
            }

            u32 byteIndex = GetByteIndex(index, data.begin());

            data.Insert(byteIndex, other.data);
        }

        constexpr void EraseBack() noexcept {
            Erase(symLength - 1);
        }

        constexpr void Erase(u32 index) noexcept {
            if (isEmpty())
                return;

            if (index >= symLength)
                index = symLength - 1;

            index = GetByteIndex(index, data.begin());

            auto l = GetSymLength(data.begin()[index]);

            while (l) {
                data.Erase(index);

                l--;
            }

            symLength--;
        }

        constexpr u32 getByteLength() const noexcept {
            return data.getLength();
        }

        constexpr u32 getLength() const noexcept {
            return symLength;
        }

        constexpr bool isEmpty() const noexcept {
            return data.isEmpty();
        }

        constexpr bool notEmpty() const noexcept {
            return data.notEmpty();
        }

        constexpr operator bool() const noexcept {
            return data;
        }

        constexpr const char* begin() const noexcept {
            return data.begin();
        }

        constexpr const char* end() const noexcept {
            return data.end();
        }

        constexpr const String<char>& getData() const noexcept {
            return data;
        }

        constexpr Symbol operator [](u32 index) const noexcept {
            if (index >= symLength)
                return Symbol();

            index = GetByteIndex(index, data.begin());

            return Symbol(data.begin() + index, GetSymLength(data.begin()[index]));
        }

        template <typename Char>
        constexpr String<Char> ConvertTo() const noexcept {
            if constexpr (std::is_same_v<char, Char>)
                return data;

            if constexpr (std::is_same_v<Char, Symbol>)
                return *this;

            String<Char> result = {};

            for (u32 i = 0; i < data.getLength();) {
                auto syml = GetSymLength(data.begin()[i]);

                Symbol symbol(data.begin() + i, syml);

                result += symbol.ToString<Char>();

                i += syml;
            }

            return result;
        }

        constexpr bool operator ==(const String& other) const noexcept {
            return data == other.data;
        }

        constexpr bool operator !=(const String& other) const noexcept {
            return data != other.data;
        }

        constexpr bool operator <=(const String& other) const noexcept {
            return data <= other.data;
        }

        constexpr bool operator >=(const String& other) const noexcept {
            return data >= other.data;
        }

        constexpr bool operator <(const String& other) const noexcept {
            return data < other.data;
        }

        constexpr bool operator >(const String& other) const noexcept {
            return data > other.data;
        }

        constexpr void Iterate(auto&& func) const noexcept {
            for (u32 i = 0; i < getByteLength();) {
                auto byte = begin() + i;

                auto symLength = GetSymLength(*byte);

                func(Symbol(byte, symLength));

                i += symLength;
            }
        }
    };

    constexpr String<Symbol>& operator <<(String<Symbol>& string, Symbol symbol) noexcept {
        string.EmplaceBack(symbol);

        return string;
    }

    constexpr String<Symbol>& operator <<(String<Symbol>& string, u64 number) noexcept {
        String<char> s = {};

        s << number;

        return string += s;
    }

    constexpr String<Symbol>& operator <<(String<Symbol>& string, i64 number) noexcept {
        String<char> s = {};

        s << number;

        return string += s;
    }

    constexpr String<Symbol>& operator <<(String<Symbol>& string, u32 number) noexcept {
        String<char> s = {};

        s << number;

        return string += s;
    }

    constexpr String<Symbol>& operator <<(String<Symbol>& string, i32 number) noexcept {
        String<char> s = {};

        s << number;

        return string += s;
    }

    constexpr u64& operator >>(String<Symbol>& string, u64& number) {
        return string.getData() >> number;
    }

    constexpr i64& operator >>(String<Symbol>& string, i64& number) {
        return string.getData() >> number;
    }

    constexpr u32& operator >>(String<Symbol>& string, u32& number) {
        return string.getData() >> number;
    }

    constexpr i32& operator >>(String<Symbol>& string, i32& number) {
        return string.getData() >> number;
    }

    std::ostream& operator <<(std::ostream& ostream, const typename String<Symbol>::View& view) noexcept {
        return ostream.write(view.begin(), view.getByteLength());
    }

    std::ostream& operator <<(std::ostream& ostream, const String<Symbol>& string) noexcept {
        return ostream.write(string.begin(), string.getByteLength());
    }
}

export consteval ACTL::Symbol operator ""_sym (const char* cstring, ACTL::size length) {
    return ACTL::Symbol(cstring, length);
}

export consteval ACTL::String<ACTL::Symbol>::View operator ""_ustr (const char* cstring, ACTL::size length) {
    return ACTL::String<ACTL::Symbol>::View(cstring);
}