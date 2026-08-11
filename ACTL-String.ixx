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

#include <algorithm>
#include <iostream>

export module ACTL:String;

import :Defines;
import :Hash;
import :Array;

export namespace ACTL {
    template <typename Char>
    class String {
        Array<Char> data = {};

    public:
        static constexpr auto null = Char(0);

        class View {
            const Char* data;

            u32 length;

        public:
            constexpr View() noexcept {
                data = &null;

                length = 0;
            }

            consteval View(const Char* cstring) noexcept {
                operator=(cstring);
            }

            constexpr View(const View& other) noexcept = default;

            constexpr View(View&& other) noexcept = default;

            constexpr ~View() noexcept = default;

            consteval View& operator =(const Char* cstring) noexcept {
                data = cstring;

                length = 0;

                while (*cstring) {
                    length++;

                    cstring++;
                }

                return *this;
            }

            constexpr View& operator =(const View& other) noexcept = default;

            constexpr View& operator =(View&& other) noexcept = default;

            [[nodiscard]] constexpr u32 getLength() const noexcept {
                return length;
            }

            [[nodiscard]] constexpr const Char* begin() const noexcept {
                return data;
            }

            [[nodiscard]] constexpr const Char* end() const noexcept {
                return data + length;
            }

            [[nodiscard]] constexpr Char operator [](u32 index) const noexcept {
                return index < length ? data[index] : null;
            }

            [[nodiscard]] constexpr bool notEmpty() const noexcept {
                return length;
            }

            [[nodiscard]] constexpr bool isEmpty() const noexcept {
                return !notEmpty();
            }

            [[nodiscard]] constexpr operator bool() const noexcept {
                return notEmpty();
            }

            constexpr void Iterate(u32 start, u32 finish, auto&& func) const noexcept {
                if (start > length)
                    start = length;

                if (finish > length)
                    finish = length;

                if (start < finish) {
                    for (auto i = data + start; i < data + finish; i++)
                        func(*i);
                }
                else {
                    for (auto i = data + start - 1; i >= data + finish; i--)
                        func(*i);
                }
            }
        };

        [[nodiscard]] static constexpr String FromCstring(const Char* cstring, u32 maxLength = 1024) noexcept {
            String result = {};

            if (!cstring)
                return result;

            u32 length = 0;

            while (cstring[length]) {
                length++;

                if (length == maxLength)
                    break;
            }

            result.data.EmplaceBackMany(length, null);

            std::copy(cstring, cstring + length, result.data.begin());

            return result;
        }

        constexpr String(const View& view) noexcept {
            if (!view) {
                data.EmplaceBack(null);

                return;
            }

            data.EmplaceBackMany(view.getLength() + 1, 0);

            std::copy(view.begin(), view.end(), data.begin());
        }

        constexpr String() noexcept {
            data.EmplaceBack(null);
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

            return *this;
        }

        constexpr String& operator =(String&& other) noexcept {
            Clear();

            ACTL::swap(data, other.data);

            return *this;
        }

        constexpr String& operator +=(const String& other) noexcept {
            data.EraseBack();

            data += other.data;

            return *this;
        }

        constexpr String operator +(const String& other) const noexcept {
            String result = *this;

            result += other;

            return result;
        }

        constexpr void Clear() noexcept {
            data.Clear();

            data.EmplaceBack(null);
        }

        constexpr void Insert(u32 index, const String& other) noexcept {
            if (index >= getLength()) {
                *this += other;

                return;
            }

            auto oldLength = getLength();

            for (u32 i = 0; i < other.getLength(); i++)
                data.EmplaceStrict(index + i, other.data.begin()[i]);
        }

        constexpr void EmplaceBack(Char symbol) noexcept {
            data.back() = symbol;

            data.EmplaceBack(null);
        }

        constexpr void Emplace(u32 index, Char symbol) noexcept {
            if (index < getLength()) {
                data.EmplaceStrict(index, symbol);

                return;
            }

            EmplaceBack(symbol);
        }

        constexpr void Set(u32 index, Char symbol) noexcept {
            if (index < getLength()) {
                data.begin()[index] = symbol;

                return;
            }
            else if (index == getLength()) {
                return EmplaceBack(symbol);
            }
            
            auto count = index - getLength() + 1;

            data.EraseBack();

            data.EmplaceBackMany(count, symbol);

            data.EmplaceBack(null);
        }

        constexpr void EraseBack() noexcept {
            data.EraseBack();

            data.back() = null;
        }

        constexpr void Erase(u32 index) noexcept {
            if (index >= getLength())
                return EraseBack();

            data.EraseStrict(index);
        }

        [[nodiscard]] constexpr Char operator [](u32 index) const noexcept {
            if (index < getLength())
                return data.begin()[index];

            return null;
        }

        [[nodiscard]] constexpr u32 getLength() const noexcept {
            return data.getLength() - 1;
        }

        [[nodiscard]] constexpr bool isEmpty() const noexcept {
            return !notEmpty();
        }

        [[nodiscard]] constexpr bool notEmpty() const noexcept {
            return getLength();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return notEmpty();
        }

        [[nodiscard]] constexpr bool operator ==(const String& other) const noexcept {
            if (getLength() != other.getLength())
                return false;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return false;

            return true;
        }

        [[nodiscard]] constexpr bool operator !=(const String& other) const noexcept {
            return !operator==(other);
        }

        [[nodiscard]] constexpr bool operator >=(const String& other) const noexcept {
            if (getLength() < other.getLength())
                return false;
            else if (getLength() > other.getLength())
                return true;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] < other.begin()[i])
                    return false;

            return true;
        }

        [[nodiscard]] constexpr bool operator <=(const String& other) const noexcept {
            if (getLength() > other.getLength())
                return false;
            else if (getLength() < other.getLength())
                return true;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] > other.begin()[i])
                    return false;

            return true;
        }

        [[nodiscard]] constexpr bool operator >(const String& other) const noexcept {
            return !operator<=(other);
        }

        [[nodiscard]] constexpr bool operator <(const String& other) const noexcept {
            return !operator>=(other);
        }

        [[nodiscard]] constexpr const auto& getArray() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Char* begin() noexcept {
            return data.begin();
        }

        [[nodiscard]] constexpr Char* end() noexcept {
            return data.begin() + getLength();
        }

        [[nodiscard]] constexpr const Char* begin() const noexcept {
            return data.begin();
        }

        [[nodiscard]] constexpr const Char* end() const noexcept {
            return data.begin() + getLength();
        }

        constexpr void Iterate(u32 start, u32 finish, auto&& func) noexcept {
            if (finish > getLength())
                finish = getLength();

            if (start > getLength())
                start = getLength();

            if (start < finish) {
                for (auto i = begin() + start; i < begin() + finish; i++)
                    func(*i);
            }
            else {
                for (auto i = begin() + start - 1; i >= begin() + finish; i--)
                    func(*i);
            }
        }

        constexpr void Iterate(u32 start, u32 finish, auto&& func) const noexcept {
            if (finish > getLength())
                finish = getLength();

            if (start > getLength())
                start = getLength();

            if (start < finish) {
                for (auto i = begin() + start; i < begin() + finish; i++)
                    func(*i);
            }
            else {
                for (auto i = begin() + start - 1; i >= begin() + finish; i--)
                    func(*i);
            }
        }
    };

    template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const typename String<Char>::View& view) noexcept {
        return ostream.write(view.begin(), view.getLength());
    }

    template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const String<Char>& other) noexcept {
        return ostream.write(other.begin(), other.getLength());
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, const String<Char>& other) noexcept {
        return string += other;
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, Char symbol) noexcept {
        string.Set(string.getLength(), symbol);

        return string;
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, u64 number) noexcept {
        if (!number)
            return string << Char('0');

        auto index = string.getLength();

        string.Set(string.getLength(), Char('0') + (number % 10));

        number /= 10;

        while (number) {
            string.Emplace(index, Char('0') + (number % 10));

            number /= 10;
        }

        return string;
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, u32 number) noexcept {
        return string << u64(number);
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, i64 number) noexcept {
        if (number < 0) {
            string << Char('-');

            number = -number;
        }

        return string << u64(number);
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, i32 number) noexcept {
        return string << i64(number);
    }

    template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, bool value) noexcept {
        if (value)
            return string << Char('t') << Char('r') << Char('u') << Char('e');

        return string << Char('f') << Char('a') << Char('l') << Char('s') << Char('e');
    }

    template <typename Char>
    constexpr u64& operator >>(const String<Char>& string, u64& number) {
        number = 0;

        for (auto s : string) {
            if (s == Char('.'))
                break;

            if (s == Char('-'))
                throw "Minus sign in unsigned number!";

            if (s < Char('0') || s > Char('9'))
                throw "Invalid symbol!";

            number *= 10;

            number += s - Char('0');
        }

        return number;
    }

    template <typename Char>
    constexpr u32& operator >>(const String<Char>& string, u32& number) {
        u64 temp;

        string >> temp;

        number = u32(temp);

        return number;
    }

    template <typename Char>
    constexpr i64& operator >>(const String<Char>& string, i64& number) {
        number = 0;

        bool neg = false;

        for (auto s : string) {
            if (s == Char('.'))
                break;

            if (s == Char('-')) {
                neg = true;

                continue;
            }

            if (s < Char('0') || s > Char('9'))
                throw "Invalid symbol!";

            number *= 10;

            number += s - Char('0');
        }

        if (neg)
            number = -number;

        return number;
    }

    template <typename Char>
    constexpr i32& operator >>(const String<Char>& string, i32& number) {
        i64 temp;

        string >> temp;

        number = i32(temp);

        return number;
    }

    template <typename Char, typename Type, size length>
    constexpr String<Char>& operator <<(String<Char>& string, const Type (&array)[length]) noexcept {
        string << Char('[') << length << Char(']') << Char('{') << Char(' ');

        for (auto& i : array)
            string << i << Char(',') << Char(' ');

        return string << Char('}');
    }

    template <typename Char, typename Type>
    constexpr String<Char>& operator <<(String<Char>& string, const Array<Type>& array) noexcept {
        string << Char('[') << array.getLength() << Char('/') << array.getCapacity() << Char(']') << Char('{') << Char(' ');

        for (auto& i : array)
            string << i << Char(',') << Char(' ');

        return string << Char('}');
    }
}

export consteval ACTL::String<char>::View operator ""_str(const char* cstring, ACTL::size length) noexcept {
    return ACTL::String<char>::View(cstring);
}

export consteval ACTL::String<wchar_t>::View operator ""_str(const wchar_t* cstring, ACTL::size length) noexcept {
    return ACTL::String<wchar_t>::View(cstring);
}