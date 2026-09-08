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
#include <concepts>
#include <type_traits>
#include <ostream>

export module ACTL:String;

import :Defines;
import :Array;

namespace ACTL {
    // Returns true if UTF-8 sequence is correct.
    export template <typename Char> requires(sizeof(Char) == 1)
    constexpr bool CheckUtf8(const Char* sequence, u8 length) noexcept {
        if (!sequence || !length)
            return false;

        if (*sequence >= Char(0x80) && *sequence <= Char(0xC1))
            return false;

        if (*sequence >= Char(0xF5) && *sequence <= Char(0xFF))
            return false;

        switch (length) {
        case 1:
            if (*sequence > Char(0x7F))
                return false;
            break;
        case 2:
            if (*sequence < Char(0xC2) || *sequence > Char(0xDF))
                return false;

            if ((sequence[1] & Char(0xC0)) != Char(0x80))
                return false;

            break;
        case 3:
            if (*sequence < Char(0xE0) || *sequence > Char(0xEF))
                return false;

            if ((sequence[1] & Char(0xC0)) != Char(0x80) || (sequence[2] & Char(0xC0)) != Char(0x80))
                return false;

            if (sequence[0] == Char(0xE0) && sequence[1] < Char(0xA0))
                return false;

            if (sequence[0] == Char(0xED) && sequence[1] >= Char(0xA0))
                return false;
            
            break;
        default:
            if (*sequence < Char(0xF0) && *sequence > Char(0xF4))
                return false;

            if ((sequence[1] & Char(0xC0)) != Char(0x80) || (sequence[2] & Char(0xC0)) != Char(0x80) || (sequence[3] & Char(0xC0)) != Char(0x80)) 
                return false;

            if (sequence[0] == Char(0xF0) && sequence[1] < Char(0x90))
                return false;

            if (sequence[0] == Char(0xF4) && sequence[1] >= Char(0x90))
                return false;

            break;
        }

        return true;
    }

    // Returns true if UTF-16 sequence is correct.
    export template <typename Char> requires(sizeof(Char) == 2)
    constexpr bool CheckUtf16(const Char* sequence, u8 length) noexcept {
        if (!sequence || !length)
            return false;

        auto IsLow = [](Char c) -> bool {
            return (c >= Char(0xDC00) && c <= Char(0xDFFF));
        };

        auto IsHigh = [](Char c) -> bool {
            return (c >= Char(0xD800) && c <= Char(0xDBFF));
        };

        auto IsValidSingle = [](Char c) -> bool {
            return (c < Char(0xD800) || c > Char(0xDFFF));
        };

        if (length == 1) {
            if (!IsValidSingle(sequence[0]))
                return false;
        }
        else {
            if (IsLow(sequence[0]))
                return false;

            if (!IsHigh(sequence[0]))
                return false;

            if (!IsLow(sequence[1]))
                return false;
        }

        return true;
    }

    // Returns true if UTF-32 sequence is correct.
    export template <typename Char> requires(sizeof(Char) == 4 && std::is_integral_v<Char>)
    constexpr bool CheckUtf32(Char symbol) noexcept {
        if (symbol < 0 || symbol > Char(0x10FFFF))
            return false;

        if (symbol >= Char(0xD800) && symbol <= Char(0xDFFF))
            return false;

        return true;
    }

    // UTF-32 always valid codepoint implementation.
    export class Symbol {
        u32 value;

        template <typename Char>
        static constexpr Symbol Convert(Char* sequence, u32 length) noexcept {
            Symbol result = {};

            if constexpr (sizeof(Char) == 1) {
                switch (length) {
                case 1:
                    result.value = u32(sequence[0]);
                    break;
                case 2:
                    result.value = (u32(sequence[0] & Char(0x1F)) << 6) | (sequence[1] & Char(0x3F));
                    break;
                case 3:
                    result.value = (u32(sequence[0] & Char(0x0F)) << 12) | 
                        (u32(sequence[1] & Char(0x3F)) << 6) | 
                        (sequence[2] & Char(0x3F));
                    break;
                default:
                    result.value = (u32(sequence[0] & Char(0x07)) << 18) | 
                        (u32(sequence[1] & Char(0x3F)) << 12) | 
                        (u32(sequence[2] & Char(0x3F)) << 6) | 
                        (sequence[3] & Char(0x3F));
                    break;
                }
            }
            else if constexpr (sizeof(Char) == 2) {
                if (length == 1) {
                    result.value = u32(sequence[0]);
                }
                else {
                    result.value = 0x10000 + ((u32(sequence[0] & Char(0x3FF)) << 10) | u32(sequence[1] & Char(0x3FF)));
                }
            }
            else if constexpr (std::is_same_v<Char, Symbol>) {
                result = *sequence;
            }
            else {
                result.value = u32(*sequence);
            }

            return result;
        }

        constexpr Symbol() noexcept {};

    public:
        friend class String;

        constexpr Symbol(const Symbol&) noexcept = default;

        constexpr Symbol(Symbol&&) noexcept = default;

        constexpr ~Symbol() noexcept = default;

        constexpr Symbol& operator =(const Symbol& other) noexcept = default;

        constexpr Symbol& operator =(Symbol&& other) noexcept = default;

        // Constructs from single symbol.
        // Throws exception if symbol is invalid.
        template <typename Char> requires(std::is_integral_v<Char>)
        constexpr Symbol(Char symbol) {
            if constexpr (sizeof(Char) == 1) {
                if (!CheckUtf8(&symbol, 1))
                    throw "Invalid UTF-8 symbol!";
            }
            else if constexpr (sizeof(Char) == 2) {
                if (!CheckUtf16(&symbol, 1))
                    throw "Invalid UTF-16 symbol!";
            }
            else {
                if (!CheckUtf32(symbol))
                    throw "Invalid UTF-32 symbol!";
            }

            *this = Convert(&symbol, 1);
        }

        // Constructs from unit sequence.
        // Throws exception if sequence is invalid.
        template <typename Char>
        constexpr Symbol(Char* sequence, u32 length = 0) noexcept(std::is_same_v<Char, Symbol>) {
            if (sequence && !length) {
                while (sequence[length] && length < 4 / sizeof(Char))
                    length++;
            }

            else if constexpr (sizeof(Char) == 1) {
                if (!CheckUtf8(sequence, length))
                    throw "Invalid UTF-8 symbol!";
            }
            else if constexpr (sizeof(Char) == 2) {
                if (!CheckUtf16(sequence, length))
                    throw "Invalid UTF-16 symbol!";
            }
            else if constexpr (!std::is_same_v<Char, Symbol>) {
                if (!sequence || !length || !CheckUtf32(*sequence))
                    throw "Invalid UTF-32 symbol!";
            }

            *this = Convert(sequence, length);
        }

        constexpr bool operator <=>(const Symbol&) const noexcept = default;

        // Returns byte count of symbol as UTF-8 sequence.
        constexpr u8 GetUtf8Length() const noexcept {
            if (value <= 0x007F)
                return 1;
            else if (value <= 0x07FF)
                return 2;
            else if (value <= 0xFFFF)
                return 3;
            else
                return 4;
        }

        // Returns unit count of symbol as UTF-16 sequence.
        constexpr u8 GetUtf16Length() const noexcept {
            if (value <= 0xFFFF)
                return 1;
            else
                return 2;
        }

        // Writes symbol to buffer as UTF-8 sequence.
        template <typename Char> requires(sizeof(Char) == 1)
        constexpr u8 WriteUtf8(Char* dest) const noexcept {
            if (!dest)
                return 0;

            u8 length = GetUtf8Length();

            switch (length) {
            case 1:
                *dest = Char(value);
                break;
            case 2:
                dest[0] = Char(0xC0 | (value >> 6));
                dest[1] = Char(0x80 | (value & 0x3F));
                break;
            case 3:
                dest[0] = Char(0xE0 | (value >> 12));
                dest[1] = Char(0x80 | ((value >> 6) & 0x3F));
                dest[2] = Char(0x80 | (value & 0x3F));
                break;
            default:
                dest[0] = Char(0xF0 | (value >> 18));
                dest[1] = Char(0x80 | ((value >> 12) & 0x3F));
                dest[2] = Char(0x80 | ((value >> 6) & 0x3F));
                dest[3] = Char(0x80 | (value & 0x3F));
                break;
            }

            return length;
        }

        // Writes symbol to buffer as UTF-16 sequence.
        template <typename Char> requires(sizeof(Char) == 2)
        constexpr u8 WriteUtf16(Char* dest) const noexcept {
            if (!dest)
                return 0;

            u8 length = GetUtf16Length();

            if (length == 1) {
                *dest = Char(value);
            }
            else {
                auto cp = value - 0x10000;

                dest[0] = Char(0xD800 | (cp >> 10));
                dest[1] = Char(0xDC00 | (cp & 0x03FF));
            }

            return length;
        }

        template <typename Char> requires(sizeof(Char) == 4)
        constexpr Char getUtf32() const noexcept {
            if constexpr (std::is_same_v<Char, Symbol>)
                return *this;
            else
                return Char(value);
        }
    };

    export std::ostream& operator <<(std::ostream& ostream, const Symbol& symbol) noexcept {
        char buffer[4];

        auto count = symbol.WriteUtf8(buffer);

        return ostream.write(buffer, count);
    }

    export std::wostream& operator <<(std::wostream& ostream, const Symbol& symbol) noexcept {
    #ifdef _WIN32
        wchar_t buffer[2];

        auto count = symbol.WriteUtf16(buffer);

        return ostream.write(buffer, count);
    #else
        return ostream << symbol.getUtf32<wchar_t>();
    #endif
    }

    export template <typename Type>
    struct Convert {};

    export template <std::integral Type>
    struct Convert<Type> {
        Type value;

        u8 base = 10;
    };

    export template <std::floating_point Type>
    struct Convert<Type> {
        Type value;

        u8 precision = 3;
    };

    export template <typename Number>
    Convert(Number) -> Convert<Number>;

    // UTF-8 valid null-terminated string implementation.
    export class String {
        Array<char> array = {};

        u32 symbolLength = 0;

        static constexpr u8 GetSymLength(auto unit) noexcept {
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
        static constexpr char null = '\0';

        // Reference to a static UTF-8 valid C-string.
        class View {
            const char* data = &null;

            u32 byteLength = 0;

            u32 symbolLength = 0;

        public:
            friend String;

            // Sets reference to C-string.
            consteval View(const char* cstring) {
                if (!cstring)
                    return;

                while (cstring[byteLength]) {
                    auto l = GetSymLength(cstring[byteLength]);

                    if (!CheckUtf8(cstring + byteLength, l))
                        throw "Invalid UTF-8 string!";

                    symbolLength++;

                    byteLength += l;
                }

                data = cstring;
            }
            
            constexpr View() noexcept {};

            constexpr View(const View&) noexcept = default;

            constexpr View(View&&) noexcept = default;

            constexpr ~View() noexcept = default;

            constexpr View& operator =(const View&) noexcept = default;

            constexpr View& operator =(View&&) noexcept = default;

            constexpr const char* begin() const noexcept {
                return data;
            }

            constexpr const char* end() const noexcept {
                return data + byteLength;
            }

            // Returns count of UTF-8 units without null-terminator.
            constexpr u32 getByteLength() const noexcept {
                return byteLength;
            }

            // Returns count of symbols.
            constexpr u32 getSymbolLength() const noexcept {
                return symbolLength;
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

            constexpr Symbol operator [](u32 index) const {
                if (index >= symbolLength)
                    throw "Invalid symbol index!";

                index = GetByteIndex(index, begin());

                auto s = begin() + index;

                return Symbol::Convert(s, GetSymLength(*s));
            }

            constexpr bool operator ==(const View& other) const noexcept {
                if (data == other.data)
                    return true;

                if (byteLength != other.byteLength)
                    return false;

                for (u32 i = 0; i < byteLength; i++)
                    if (data[i] != other.data[i])
                        return false;

                return true;
            }

            constexpr bool operator !=(const View& other) const noexcept {
                return !operator==(other);
            }

            constexpr bool operator <=(const View& other) const noexcept {
                if (data == other.data)
                    return true;

                if (byteLength > other.byteLength)
                    return false;

                for (u32 i = 0; i < byteLength; i++)
                    if (data[i] > other.data[i])
                        return false;

                return true;
            }

            constexpr bool operator >(const View& other) const noexcept {
                return !operator<=(other);
            }

            constexpr bool operator >=(const View& other) const noexcept {
                if (data == other.data)
                    return true;

                if (byteLength < other.byteLength)
                    return false;

                for (u32 i = 0; i < other.byteLength; i++)
                    if (data[i] < other.data[i])
                        return false;

                return true;
            }

            constexpr bool operator <(const View& other) const noexcept {
                return !operator>=(other);
            }

            // Returns unit count of UTF-16 representation.
            constexpr u32 GetUtf16Length() const noexcept {
                u32 result = 0;

                for (u32 i = 0; i < getByteLength();) {
                    auto l = GetSymLength(begin()[i]);

                    auto s = Symbol::Convert(begin() + i, l);

                    result += s.GetUtf16Length();

                    i += l;
                }

                return result;
            }

            // Writes UTF-16 units to buffer.
            template <typename Char> requires(sizeof(Char) == 2)
            constexpr void WriteUtf16(Char* dest, u32 limit = 0) const noexcept {
                if (!dest)
                    return;

                u32 di = 0;

                for (u32 i = 0; i < getByteLength();) {
                    auto l = GetSymLength(begin()[i]);

                    auto s = Symbol::Convert(begin() + i, l);

                    auto dl = s.WriteUtf16(&dest[di]);

                    di += dl;

                    i += l;

                    if (di == limit)
                        return;
                }
            }

            // Writes UTF-32 units to buffer.
            template <typename Char> requires(sizeof(Char) == 4)
            constexpr void WriteUtf32(Char* dest, u32 limit = 0) const noexcept {
                if (!dest)
                    return;

                u32 di = 0;

                for (u32 i = 0; i < getByteLength();) {
                    auto l = GetSymLength(begin()[i]);

                    auto s = Symbol::Convert(begin() + i, l);

                    dest[di] = s.getUtf32<Char>();

                    di += 1;

                    i += l;

                    if (di == limit)
                        return;
                }
            }
        };

        // Null-state initializer. Can be constinit.
        constexpr String() noexcept {};

        // Converts symbols from C-string.
        // Throws exception if C-string has invalid sequence of UTF units.
        template <typename Char>
        constexpr String(const Char* cstring, u32 limit = 1024) noexcept(std::is_same_v<Char, Symbol>) {
            if (!cstring || !limit)
                return;

            u32 length = 0;

            while (limit && cstring[length]) {
                length++;

                limit--;
            }

            array.EmplaceStrict(length, null);

            for (u32 i = 0; i < length;) {
                auto u = cstring + i;

                auto l = GetSymLength(*u);

                Symbol s(u, l);

                s.WriteUtf8(array.begin() + i);

                i += l;

                symbolLength++;
            }
        }

        // Copies data from view.
        constexpr String(const View& other) noexcept {
            array.EmplaceStrict(other.byteLength, null);

            std::copy(other.begin(), other.end(), array.begin());

            symbolLength = other.symbolLength;
        }

        // Copies data from other.
        constexpr String(const String& other) noexcept {
            *this = other;
        }

        // Transfers data from other.
        constexpr String(String&& other) noexcept {
            *this = Move(other);
        }

        constexpr ~String() noexcept {};

        // Copies data from other.
        constexpr String& operator =(const String& other) noexcept {
            array = other.array;

            symbolLength = other.symbolLength;

            return *this;
        }

        // Transfers data from other.
        constexpr String& operator =(String&& other) noexcept {
            Clear();

            Swap(array, other.array);

            Swap(symbolLength, other.symbolLength);

            return *this;
        }

        // Concatenates this string with other.
        constexpr String& operator +=(const String& other) noexcept {
            Insert(symbolLength, other);

            return *this;
        }

        // Concatenates this string with other.
        constexpr String operator +(const String& other) const noexcept {
            String result = *this;

            result += other;

            return result;
        }

        // Emplaces symbol strictly at index.
        constexpr void EmplaceStrict(u32 index, Symbol symbol) noexcept {
            auto l = symbol.GetUtf8Length();

            if (index >= symbolLength) {
                array.EraseBack();

                auto count = index - symbolLength + 1;

                auto l = symbol.GetUtf8Length();

                array.EmplaceStrict(array.getLength() + count * l - 1);

                for (u32 i = 1; i <= count; i++)
                    symbol.WriteUtf8(array.end() - i * l);

                array.EmplaceBack(null);

                symbolLength = index + 1;
            }
            else {
                index = GetByteIndex(index, array.begin());

                array.Insert(index, symbol.GetUtf8Length());

                symbol.WriteUtf8(array.begin() + index);

                symbolLength += 1;
            }
        }

        // Erases symbol at index.
        // Does nothing if index is out of bounds.
        constexpr void EraseStrict(u32 index) noexcept {
            if (index < symbolLength) {
                index = GetByteIndex(index, array.begin());

                array.EraseStrict(index, GetSymLength(array.begin()[index]));

                symbolLength -= 1;
            }
        }

        constexpr void EmplaceBack(Symbol symbol) noexcept {
            EmplaceStrict(symbolLength, symbol);
        }

        constexpr void EraseBack() noexcept {
            EraseStrict(symbolLength - 1);
        }

        constexpr void Insert(u32 index, u32 count, Symbol symbol) noexcept {
            if (!count)
                return;

            auto l = symbol.GetUtf8Length();

            array.EraseBack();

            if (index < symbolLength)
                index = GetByteIndex(index, begin());
            else
                index = array.getLength();

            array.Insert(index, count * l);

            array.EmplaceBack(null);

            for (u32 i = 0; i < count; i++)
                symbol.WriteUtf8(array.begin() + i * l + index);

            symbolLength += count;
        }

        constexpr void Insert(u32 index, const String& other) noexcept {
            array.EraseBack();

            if (index < symbolLength)
                index = GetByteIndex(index, begin());
            else
                index = array.getLength();

            array.Insert(index, other.getByteLength());

            std::copy(other.begin(), other.end(), array.begin() + index);

            array.EmplaceBack(null);

            symbolLength += other.getSymbolLength();
        }

        // Erases symbol at index.
        // Does nothing if index is out of bounds.
        // It is safe to set incorrect count.
        constexpr void EraseStrict(u32 index, u32 count) noexcept {
            while (count) {
                count -= 1;

                EraseStrict(index);
            }
        }

        constexpr void Clear() noexcept {
            array.Clear();

            array.EmplaceBack(null);

            symbolLength = 0;
        }

        // Sets symbol value at index.
        // Emplaces new symbols if index is out of bounds.
        constexpr void Set(u32 index, Symbol symbol) noexcept {
            if (index >= symbolLength) {
                EmplaceStrict(index, symbol);

                return;
            }

            index = GetByteIndex(index, begin());

            auto dl = GetSymLength(begin()[index]);

            auto sl = symbol.GetUtf8Length();

            if (dl > sl)
                array.EraseStrict(index, dl - sl);
            else if (dl < sl)
                array.Insert(index, sl - dl);

            symbol.WriteUtf8(array.begin() + index);
        }

        constexpr Symbol operator [](u32 index) const {
            if (index >= symbolLength)
                throw "Invalid symbol index!";

            index = GetByteIndex(index, begin());

            auto s = begin() + index;

            return Symbol::Convert(s, GetSymLength(*s));
        }

        // Copies range of symbols into new string and returns it.
        // Returns an empty string if index >= length or count == 0.
        constexpr String Get(u32 index, u32 count) const noexcept {
            if (!count || index >= symbolLength)
                return String();

            if (index + count > symbolLength)
                count = symbolLength - index;

            auto b = begin() + GetByteIndex(index, begin());

            auto e = b + GetByteIndex(count, b);

            String result = {};

            result.symbolLength = count;

            result.array.EmplaceBackMany(e - b + 1, null);

            std::copy(b, e, result.array.begin());

            return result;
        }

        // Returns UTF-8 unit count excluding null-terminator.
        constexpr u32 getByteLength() const noexcept {
            return array ? array.getLength() - 1 : 0;
        }

        // Returns symbol count.
        constexpr u32 getSymbolLength() const noexcept {
            return symbolLength;
        }

        // Pointer to begining of null-terminated string.
        constexpr const char* begin() const noexcept {
            return array ? array.begin() : &null;
        }

        // Pointer to null-terminator of null-terminated string.
        constexpr const char* end() const noexcept {
            return array ? array.end() - 1 : &null;
        }

        constexpr bool notEmpty() const noexcept {
            return symbolLength;
        }

        constexpr bool isEmpty() const noexcept {
            return !symbolLength;
        }

        constexpr operator bool() const noexcept {
            return notEmpty();
        }

        // Returns length of string as UTF-16 unit count.
        constexpr u32 GetUtf16Length() const noexcept {
            u32 result = 0;

            for (u32 i = 0; i < getByteLength();) {
                auto l = GetSymLength(array.begin()[i]);

                auto s = Symbol::Convert(array.begin() + i, l);

                result += s.GetUtf16Length();

                i += l;
            }

            return result;
        }

        // Writes UTF-16 string into buffer.
        template <typename Char> requires(sizeof(Char) == 2)
        constexpr void WriteUtf16(Char* dest, u32 limit = 0) const noexcept {
            if (!dest)
                return;

            u32 di = 0;

            for (u32 i = 0; i < getByteLength();) {
                auto l = GetSymLength(array.begin()[i]);

                auto s = Symbol::Convert(array.begin() + i, l);

                auto dl = s.WriteUtf16(&dest[di]);

                di += dl;

                i += l;

                if (di == limit)
                    return;
            }
        }

        // Writes UTF-32 string into buffer.
        template <typename Char> requires(sizeof(Char) == 4)
        constexpr void WriteUtf32(Char* dest, u32 limit = 0) const noexcept {
            if (!dest)
                return;

            u32 di = 0;

            for (u32 i = 0; i < getByteLength();) {
                auto l = GetSymLength(array.begin()[i]);

                auto s = Symbol::Convert(array.begin() + i, l);

                dest[di] = s.getUtf32<Char>();

                di += 1;

                i += l;

                if (di == limit)
                    return;
            }
        }

        constexpr bool operator ==(const String& other) const noexcept {
            if (getByteLength() != other.getByteLength())
                return false;

            for (u32 i = 0; i < getByteLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return false;

            return true;
        }

        constexpr bool operator !=(const String& other) const noexcept {
            return !operator==(other);
        }

        constexpr bool operator <=(const String& other) const noexcept {
            if (getByteLength() > other.getByteLength())
                return false;

            for (u32 i = 0; i < getByteLength(); i++)
                if (begin()[i] > other.begin()[i])
                    return false;

            return true;
        }

        constexpr bool operator >(const String& other) const noexcept {
            return !operator<=(other);
        }

        constexpr bool operator >=(const String& other) const noexcept {
            if (getByteLength() < other.getByteLength())
                return false;

            for (u32 i = 0; i < other.getByteLength(); i++)
                if (begin()[i] < other.begin()[i])
                    return false;

            return true;
        }

        constexpr bool operator <(const String& other) const noexcept {
            return !operator>=(other);
        }

        template <typename Func> requires(std::is_invocable_v<Func, Symbol>)
        constexpr void Iterate(Func&& func) const noexcept {
            for (u32 i = 0; i < getByteLength();) {
                auto l = GetSymLength(begin()[i]);

                func(Symbol::Convert(begin() + i, l));

                i += l;
            }
        }

        constexpr String& operator <<(Symbol symbol) noexcept {
            EmplaceBack(symbol);

            return *this;
        }

        constexpr String& operator <<(const View& other) noexcept {
            return *this += String(other);
        }

        constexpr String& operator <<(const String& other) noexcept {
            return *this += other;
        }

        constexpr String& operator <<(char symbol) {
            return *this << Symbol(symbol);
        }

        constexpr String& operator <<(wchar_t symbol) {
            return *this << Symbol(symbol);
        }

        template <typename Char>
        constexpr String& operator <<(const Char* cstring) noexcept(std::is_same_v<Char, Symbol>) {
            return *this << String(cstring);
        }

        template <std::integral Number> 
        constexpr String& operator <<(Convert<Number> convert) noexcept {
            array.EraseBack();

            constexpr char syms[] = "0123456789ABCDEF";

            if constexpr (std::is_signed_v<Number>) {
                if (convert.value < 0) {
                    convert.value = -convert.value;

                    array.EmplaceBack('-');

                    symbolLength += 1;
                }
            }

            if (convert.base < 2)
                convert.base = 2;
            else if (convert.base > 16)
                convert.base = 16;

            auto index = array.getLength();

            do {
                auto m = convert.value % convert.base;

                array.EmplaceStrict(index, syms[m]);

                symbolLength += 1;

                convert.value /= 10;
            } while(convert.value);

            array.EmplaceBack(null);

            return *this;
        }

        template <std::integral Number>
        constexpr String& operator <<(Number number) noexcept {
            return *this << Convert<Number>(number, 10);
        }

        template <std::floating_point Number>
        constexpr String& operator <<(Convert<Number> convert) noexcept {
            auto wp = i64(convert.value);

            *this << wp;

            convert.value -= Number(wp);

            array.EraseBack();

            array.EmplaceBack('.');

            symbolLength += 1;

            auto index = array.getLength();

            while (convert.precision) {
                symbolLength += 1;

                convert.value *= Number(10);

                wp = static_cast<i64>(convert.value);

                array.EmplaceBack(wp + '0');

                convert.value -= Number(wp);

                convert.precision--;
            }

            array.EmplaceBack(null);

            return *this;
        }

        template <std::floating_point Number>
        constexpr String& operator <<(Number number) noexcept {
            return *this << Convert<Number>(number, 3);
        }
    };

    export template <std::integral Number>
    constexpr Number& operator <<(Number& number, const String& string) noexcept {
        number = 0;

        if (!string)
            return number;

        bool n = false;

        if constexpr (std::is_signed_v<Number>)
            n = (string.begin()[0] == '-');

        for (u32 i = n; i < string.getByteLength(); i++) {
            auto b = string.begin()[i];

            if (b < '0' || b > '9')
                break;

            number *= 10;

            number += b - '0';
        }

        if constexpr (std::is_signed_v<Number>) {
            if (n)
                number = -number;
        }

        return number;
    }

    export std::ostream& operator <<(std::ostream& ostream, const String& string) noexcept {
        return ostream.write(string.begin(), string.getByteLength());
    }

    export std::wostream& operator <<(std::wostream& ostream, const String& string) noexcept {
        string.Iterate([&](Symbol symbol) {
            ostream << symbol;
        });

        return ostream;
    }

    consteval bool SymbolConvertTest() {
        Symbol symbol = '0';

        if (U'0' != symbol.getUtf32<char32_t>())
            return false;

        symbol = "я";

        if (U'я' != symbol.getUtf32<char32_t>())
            return false;

        symbol = L'Ч';

        if (U'Ч' != symbol.getUtf32<char32_t>())
            return false;

        char buffer[4] {};

        char str[4] = "Ч";

        symbol.WriteUtf8(buffer);

        if (!std::equal(buffer, buffer + 4, str, str + 4))
            return false;

        return true;
    }

    static_assert(SymbolConvertTest(), "Symbol convert test is FAILED!");

    consteval bool StringConvertTest() {
        String string = "apple";

        if (string.getSymbolLength() != 5)
            return false;

        string = "яблоко";

        if (string.getSymbolLength() != 6)
            return false;

        char16_t buffer[13] = {};

        char16_t s[12] = u"яблоко";

        string.WriteUtf16(buffer);

        if (!std::equal(buffer, buffer + 12, s, s + 12))
            return false;

        return true;
    }

    static_assert(StringConvertTest(), "String convert test is FAILED!");

    template <u32 length>
    consteval bool Compare(const String& left, const char (&right)[length]) noexcept {
        if (*left.end() != 0)
            return false;

        if (left.getByteLength() != length - 1)
            return false;

        return std::equal(left.begin(), left.end(), right, right + length - 1);
    }

    consteval bool StringEmplaceTest() {
        String string = "apple";

        string.EmplaceStrict(0, 'a');

        if (!Compare(string, "aapple"))
            return false;

        string.EmplaceStrict(0, "Ї");

        if (!Compare(string, "Їaapple"))
            return false;

        if (string.getSymbolLength() != 7)
            return false;

        string.EmplaceStrict(3, "э");

        if (!Compare(string, "Їaaэpple"))
            return false;

        if (string.getSymbolLength() != 8)
            return false;

        string.EmplaceBack("ю");

        if (!Compare(string, "Їaaэppleю"))
            return false;

        if (string.getSymbolLength() != 9)
            return false;

        string.EmplaceStrict(10, '0');

        if (!Compare(string, "Їaaэppleю00"))
            return false;

        if (string.getSymbolLength() != 11)
            return false;

        string.EmplaceStrict(13, "ц");

        if (!Compare(string, "Їaaэppleю00ццц"))
            return false;

        if (string.getSymbolLength() != 14)
            return false;

        return true;
    }

    static_assert(StringEmplaceTest(), "String Emplace test is FAILED!");

    consteval bool StringInsertTest() {
        String string = "ooo";

        string.Insert(0, 2, "Ж");

        if (!Compare(string, "ЖЖooo"))
            return false;

        if (string.getSymbolLength() != 5)
            return false;

        string.Insert(2, 2, "Ж");

        if (!Compare(string, "ЖЖЖЖooo"))
            return false;

        if (string.getSymbolLength() != 7)
            return false;

        string.Insert(10, 3, "ї");

        if (!Compare(string, "ЖЖЖЖoooїїї"))
            return false;

        if (string.getSymbolLength() != 10)
            return false;

        string.Insert(4, "щщщ");

        if (!Compare(string, "ЖЖЖЖщщщoooїїї"))
            return false;

        if (string.getSymbolLength() != 13)
            return false;

        string.Insert(30, "щщщ");

        if (!Compare(string, "ЖЖЖЖщщщoooїїїщщщ"))
            return false;

        if (string.getSymbolLength() != 16)
            return false;

        return true;
    }

    static_assert(StringInsertTest(), "String Insertion test is FAILED!");

    consteval bool StringEraseTest() {
        String string = "я был дома и ел";

        string.EraseBack();

        if (!Compare(string, "я был дома и е"))
            return false;

        if (string.getSymbolLength() != 14)
            return false;

        string.EraseStrict(0);

        if (!Compare(string, " был дома и е"))
            return false;

        if (string.getSymbolLength() != 13)
            return false;

        string.EraseStrict(0, 3);

        if (!Compare(string, "л дома и е"))
            return false;

        if (string.getSymbolLength() != 10)
            return false;

        string.EraseStrict(2, 2);

        if (!Compare(string, "л ма и е"))
            return false;

        if (string.getSymbolLength() != 8)
            return false;

        string.EraseStrict(10, 2);

        if (!Compare(string, "л ма и е"))
            return false;

        if (string.getSymbolLength() != 8)
            return false;

        string.EraseStrict(6, 3);

        if (!Compare(string, "л ма и"))
            return false;

        if (string.getSymbolLength() != 6)
            return false;

        string.EraseBack();

        if (!Compare(string, "л ма "))
            return false;

        if (string.getSymbolLength() != 5)
            return false;

        return true;
    }

    static_assert(StringEraseTest(), "String Erase test is FAILED!");

    consteval bool StringSetAndGetTest() {
        String string = "maxwell";

        if (string[0] != 'm')
            return false;

        if (string[3] != 'w')
            return false;

        string = "Иванович";

        if (string[0] != "И")
            return false;

        if (string[4] != "о")
            return false;

        string.Set(4, 'w');

        if (string[4] != "w")
            return false;

        if (string[6] != "и")
            return false;

        string.Set(1, "湖");

        if (string[6] != "и")
            return false;

        string = "Маksим";

        string.Set(5, 'w');

        if (string[5] != "w")
            return false;

        string.Set(8, "ї");

        if (!Compare(string, "Маksиwїїї"))
            return false;

        string = "Иvан Иваныч";

        if (!Compare(string.Get(0, 5), "Иvан "))
            return false;

        if (!Compare(string.Get(5, 1), "И"))
            return false;

        if (!Compare(string.Get(5, 6), "Иваныч"))
            return false;

        if (!Compare(string.Get(5, 7), "Иваныч"))
            return false;

        if (!Compare(string.Get(23, 6), ""))
            return false;

        return true;
    }

    static_assert(StringSetAndGetTest(), "String Set and Get test is FAILED!");

    consteval bool StringFromNumberTest() {
        String string = {};

        string << u32(8011);

        if (!Compare(string, "8011"))
            return false;

        string = {};

        string << i32(-902377);

        if (!Compare(string, "-902377"))
            return false;

        string = {};

        string << i32(0);

        if (!Compare(string, "0"))
            return false;

        string = {};

        string << u16(256);

        if (!Compare(string, "256"))
            return false;

        string = {};

        string << f32(0.0);

        if (!Compare(string, "0.000"))
            return false;

        string = {};

        string << f32(10.0);

        if (!Compare(string, "10.000"))
            return false;

        string = {};

        string << 10.101;

        if (!Compare(string, "10.101"))
            return false;

        return true;
    }

    static_assert(StringFromNumberTest(), "String from number test is FAILED!");

    consteval bool StringToNumberTest() {
        String string = "0";

        i32 number;

        number << string;

        if (number)
            return false;

        string = "-90";

        number << string;

        if (number != -90)
            return false;

        string = "6730";

        number << string;

        if (number != 6730)
            return false;

        string = "78h99";

        number << string;

        if (number != 78)
            return false;

        return true;
    }

    static_assert(StringToNumberTest(), "String to number test is FAILED!");
}

export consteval ACTL::Symbol operator ""_sym(char symbol) {
    return ACTL::Symbol(symbol);
}

export consteval ACTL::Symbol operator ""_sym(const char* sequence, size_t) {
    return ACTL::Symbol(sequence);
}

export consteval ACTL::String::View operator ""_str(const char* sequence, size_t) {
    return ACTL::String::View(sequence);
}

namespace ACTL {
    export template <typename Type>
    constexpr String& operator <<(String& string, const Array<Type>& array) noexcept {
        string << "["_str << array.getLength() << "/"_str << array.getCapacity() << "]{ "_str;

        for (auto& i : array)
            string << i << ", "_str;

        return string << "}"_str;
    }
}