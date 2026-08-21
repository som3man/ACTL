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

module ;

#include <initializer_list>
#include <iostream>
#include <math.h>
#include <assert.h>
#include <ostream>
#include <type_traits>
#include <algorithm>

export module ACTL:Array;

import :Defines;
import :Allocation;
import :Utility;
import :Sort;

#define nodiscard [[nodiscard]]

export namespace ACTL {
    // Dynamic capacity resizable array.
    template <typename Type>
    class Array {
        static constexpr bool defaultConstruct = std::is_default_constructible_v<Type>;

        static constexpr bool trivialCopy = std::is_trivially_copyable_v<Type>;

        static constexpr bool trivialMoveAssign = std::is_trivially_move_assignable_v<Type>;

        static constexpr bool trivialMoveConstruct = std::is_trivially_move_constructible_v<Type>;

        static constexpr bool trivialDestruct = std::is_trivially_destructible_v<Type>;

        Type* data = nullptr;

        u32 length = 0;

        u32 capacity = 0;

    public:
        template <u32 capacity>
        class Static;

        constexpr Array() noexcept {}
        
        template <u32 capacity2>
        constexpr Array(const Static<capacity2>& other) noexcept {
            operator=(other);
        }

        template <u32 capacity2>
        constexpr Array(Static<capacity2>&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr Array(const Array& other) noexcept {
            operator=(other);
        }

        constexpr Array(Array&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr Array(const std::initializer_list<Type>& other) noexcept {
            operator=(other);
        }

        constexpr Array(u32 capacity) noexcept {
            operator=(capacity);
        }

        constexpr ~Array() noexcept {
            Free();
        }

        template <u32 capacity2>
        constexpr Array& operator =(const Static<capacity2>& other) noexcept {
            if (getCapacity() < other.getLength())
                Allocate(other.getCapacity());
            else
                Clear();

            length = other.getLength();

            if constexpr (trivialCopy)
                std::copy(other.begin(), other.end(), data);
            else
                for (u32 i = 0; i < length; i++)
                    ACTL::Construct(data + i, other.begin()[i]);

            return *this;
        }

        template <u32 capacity2>
        constexpr Array& operator =(Static<capacity2>&& other) noexcept {
            if (getCapacity() < other.getLength())
                Allocate(other.getCapacity());
            else
                Clear();

            length = other.getLength();

            if constexpr (trivialMoveConstruct)
                std::copy(other.begin(), other.end(), data);
            else
                for (u32 i = 0; i < length; i++)
                    ACTL::Construct(data + i, ACTL::move(other.begin()[i]));

            other.Clear();

            return *this;
        }

        constexpr Array& operator =(const Array& other) noexcept {
            if (this == &other)
                return *this;

            if (getCapacity() < other.getLength())
                Allocate(other.getCapacity());
            else
                Clear();

            length = other.getLength();

            if constexpr (trivialCopy)
                std::copy(other.begin(), other.end(), data);
            else
                for (u32 i = 0; i < length; i++)
                    ACTL::Construct(data + i, other.begin()[i]);

            return *this;
        }

        constexpr Array& operator =(Array&& other) noexcept {
            Free();

            ACTL::swap(data, other.data);

            ACTL::swap(length, other.length);

            ACTL::swap(capacity, other.capacity);

            return *this;
        }

        constexpr Array& operator =(const std::initializer_list<Type>& list) noexcept {
            if (getCapacity() < list.size())
                Allocate(list.size());
            else
                Clear();

            length = list.size();

            if constexpr (trivialCopy)
                std::copy(list.begin(), list.end(), data);
            else
                for (u32 i = 0; i < length; i++)
                    ACTL::Construct(data + i, list.begin()[i]);

            return *this;
        }

        constexpr Array& operator =(u32 capacity) noexcept {
            Allocate(capacity);

            return *this;
        }

        constexpr Array& operator +=(const Array& other) noexcept {
            if (!isAllocated())
                Allocate(other.getLength());
            else if (getCapacity() < getLength() + other.getLength())
                Expand(other.getLength());

            if constexpr (trivialCopy)
                std::copy(other.begin(), other.end(), data + length);
            else 
                for (u32 i = 0; i < other.getLength(); i++)
                    ACTL::Construct(data + length + i, other.begin()[i]);

            length += other.getLength();

            return *this;
        }

        constexpr Array& operator +=(Array&& other) noexcept {
            if (!isAllocated())
                Allocate(other.getLength());
            else if (getCapacity() < getLength() + other.getLength())
                Expand(other.getLength());

            for (u32 i = 0; i < other.getLength(); i++)
                ACTL::Construct(data + length + i, ACTL::move(other.begin()[i]));

            length += other.getLength();

            return *this;
        }

        constexpr Array operator +(const Array& other) const noexcept {
            Array result = getLength() + other.getLength();

            result = *this;

            result += other;

            return result;
        }

        // Frees array and allocates new memory.
        constexpr void Allocate(u32 capacity) noexcept {
            Free();

            if (!capacity)
                return;

            data = ACTL::Allocate<Type>(capacity);

            this->capacity = capacity;
        }

        // Expands array by its capacity or allocates 8 elements if array is not allocated.
        constexpr void Expand() noexcept {
            if (isAllocated())
                return Expand(getCapacity());

            Allocate(8);
        }

        // Increases arrays capacity by given count of elements.
        // Moves all data to a new memory.
        constexpr void Expand(u32 count) noexcept {
            if (!count)
                return;

            if (!isAllocated())
                return Allocate(count);

            Type* newData = ACTL::Allocate<Type>(capacity + count);

            for (u32 i = 0; i < length; i++) {
                auto& old = data[i];

                ACTL::Construct(newData + i, ACTL::move(old));

                old.~Type();
            }
            
            ACTL::Free(data, capacity);

            capacity += count;

            data = newData;
        }

        constexpr void ShrinkToFit() noexcept {
            if (getCapacity() == getLength())
                return;

            Type* newData = ACTL::Allocate<Type>(getLength());

            capacity = getLength();

            for (u32 i = 0; i < length; i++) {
                auto& old = data[i];

                ACTL::Construct(data + i, ACTL::move(i));

                old.~Type();
            }

            ACTL::Free(data);

            data = newData;
        }

        // Clears array and frees its memory.
        constexpr void Free() noexcept {
            if (!isAllocated())
                return;

            Clear();

            ACTL::Free(data, capacity);

            capacity = 0;

            data = nullptr;
        }

        // Constructs a new element at the end of array and returns it.
        // If array is full, expands it.
        template <typename... Args>
        constexpr Type& EmplaceBack(Args&&... args) noexcept {
            if (isFull())
                Expand();

            ACTL::Construct<Type>(data + length, ACTL::forward<Args>(args)...);

            length += 1;

            return back();
        }

        // Destructs the back of array.
        // Or simply decrements length if destructor is trivial.
        // Does nothing if array is empty.
        constexpr void EraseBack() noexcept {
            if (isEmpty())
                return;

            length -= 1;

            if (!trivialDestruct)
                data[length].~Type();
        }

        // Constructs new elements at the end of array.
        // If count + length > capacity, expands array.
        // If count is zero, does nothing.
        template <typename... Args>
        constexpr void EmplaceBackMany(u32 count, Args&&... args) noexcept {
            if (!count)
                return;
            else if (getCapacity() * 2 < getLength() + count)
                Expand(getLength() + count);
            else if (getCapacity() < getLength() + count)
                Expand();

            for (Type* i = data + length; i < data + length + count; i++)
                ACTL::Construct(i, ACTL::forward<Args>(args)...);

            length += count;
        }

        // Destructs elements at the back of array.
        // Or simply subtracts length by count if destructor is trivial.
        // If count > length, sets count with length value.
        // If count is zero, does nothing.
        constexpr void EraseBackMany(u32 count) noexcept {
            if (count > length)
                count = length;

            if constexpr (trivialDestruct) {
                length -= count;

                return;
            }

            while (count) {
                length--;

                count--;

                data[length].~Type();
            }
        }

        // Emplaces element to the back and swaps it with element of given index.
        // If index >= length calls EmplaceBack.
        // If array is full, expands it.
        template <typename... Args>
        constexpr Type& EmplaceFast(u32 index, Args&&... args) noexcept {
            if (index >= getLength())
                return EmplaceBack(ACTL::forward<Args>(args)...);

            EmplaceBack(ACTL::forward<Args>(args)...);

            ACTL::swap(data[index], back());

            return data[index];
        }

        // Swaps element of the given index with element at the back and destructs it.
        // Calls EraseBack if index >= length.
        constexpr void EraseFast(u32 index) noexcept {
            if (index >= getLength())
                return EraseBack();

            ACTL::swap(data[index], back());

            EraseBack();
        }

        // Constructs element at the given index and saves order.
        // If index >= length, calls EmplaceBack.
        // If array is full, expands it.
        template <typename... Args>
        constexpr Type& EmplaceStrict(u32 index, Args&&... args) noexcept {
            if (index >= getLength())
                return EmplaceBack(ACTL::forward<Args>(args)...);

            if (isFull())
                Expand();

            ACTL::Construct<Type>(data + length, ACTL::move(data[length - 1]));

            for (u32 d = length - 1; d > index; d--)
                data[d] = ACTL::move(data[d - 1]);

            data[index].~Type();

            Construct<Type>(data + index, ACTL::forward<Args>(args)...);

            length += 1;
            
            return data[index];
        }

        // Erases element by index and saves order.
        // Calls EraseBack if index >= length;
        constexpr void EraseStrict(u32 index) noexcept {
            if (index >= getLength())
                return EraseBack();

            if (isEmpty())
                return;

            for (Type* d = data + index; d < end() - 1; d++)
                *d = ACTL::move(*(d + 1));

            EraseBack();
        }

        // Emplaces element and keeps array sorted.
        template <typename... Args>
        constexpr Type& EmplaceSorted(Args&&... args) noexcept {
            Type temp(ACTL::forward<Args>(args)...);

            auto result = Find(temp);

            return EmplaceStrict(result.index, ACTL::move(temp));
        }

        // Erases element with given value and keeps array sorted.
        // Does nothing if element is not found.
        constexpr void EraseSorted(const Type& value) noexcept {
            auto result = Find(value);

            if (result.found)
                EraseStrict(result.index);
        }

        // Destructs all elements.
        // Or just nulls length if destructor is trivial.
        constexpr void Clear() noexcept {
            if constexpr (trivialDestruct) {
                length = 0;

                return;
            }

            while (length) {
                length--;

                data[length].~Type();
            }
        }

        nodiscard constexpr bool isAllocated() const noexcept {
            return capacity;
        }

        nodiscard constexpr bool isFull() const noexcept {
            return length == capacity;
        }

        nodiscard constexpr bool isEmpty() const noexcept {
            return !notEmpty();
        }

        nodiscard constexpr bool notEmpty() const noexcept {
            return length;
        }

        nodiscard constexpr operator bool() const noexcept {
            return notEmpty();
        }

        nodiscard constexpr u32 getLength() const noexcept {
            return length;
        }

        nodiscard constexpr u32 getCapacity() const noexcept {
            return capacity;
        }

        nodiscard constexpr Type* begin() noexcept {
            return data;
        }

        nodiscard constexpr const Type* begin() const noexcept {
            return data;
        }

        nodiscard constexpr Type* end() noexcept {
            return begin() + getLength();
        }

        nodiscard constexpr const Type* end() const noexcept {
            return begin() + getLength();
        }

        nodiscard constexpr Type& front() {
            if (notEmpty())
                return *begin();

            throw "Array is empty!";
        }

        nodiscard constexpr const Type& front() const {
            if (notEmpty())
                return *begin();

            throw "Array is empty!";
        }

        nodiscard constexpr Type& back() {
            if (notEmpty())
                return end()[-1];

            throw "Array is empty!";
        }

        nodiscard constexpr const Type& back() const {
            if (notEmpty())
                return end()[-1];

            throw "Array is empty!";
        }

        nodiscard constexpr SearchResult Find(const Type& value) const noexcept {
            return ACTL::Find(data, getLength(), value);
        }

        constexpr void QuickSort() noexcept {
            ACTL::QuickSort(data, 0, getLength());
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr Type& operator [](u32 index) {
            return GetOrExcept(index);
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr const Type& operator [](u32 index) const {
            return GetOrExcept(index);
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr Type& GetOrExcept(u32 index) {
            if (index < getLength())
                return data[index];

            throw "Invalid array index!";
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr const Type& GetOrExcept(u32 index) const {
            if (index < getLength())
                return data[index];

            throw "Invalid array index!";
        }

        // Returns copy of element if index < length.
        // Returns new instance if index >= length.
        template <typename... Args>
        nodiscard constexpr Type GetOrNew(u32 index, Args&&... args) const noexcept {
            if (index < getLength())
                return data[index];

            return Type(ACTL::forward<Args>(args)...);
        }

        // Returns reference to a element.
        // Emplaces new element at index and returns it if index >= length.
        // May construct new elements in between with no args or with given args if default constructor is not accessible.
        // If index >= capacity, expands array.
        template <typename... Args>
        nodiscard constexpr Type& GetOrEmplace(u32 index, Args&&... args) noexcept {
            if (index < getLength())
                return data[index];
            else if (index == getLength())
                return EmplaceBack(ACTL::forward<Args>(args)...);

            u32 count = index - length;

            if constexpr (defaultConstruct) {
                EmplaceBackMany(count);

                return EmplaceBack(ACTL::forward<Args>(args)...);
            }
            else {
                EmplaceBackMany(count + 1, ACTL::forward<Args>(args)...);

                return back();
            }
        }

        constexpr void Iterate(auto&& func) noexcept {
            for (auto& i : *this)
                func(i);
        }

        constexpr void Iterate(auto&& func) const noexcept {
            for (auto& i : *this)
                func(i);
        }

        // Iterates array from start index to finish index.
        // Start and finish will be clamped to arrays length.
        // If start < finish, iterates from left to right. Finish is excluded from iteration.
        // If start >= finish, iterates from right to left. Start is excluded from iteration.
        constexpr void Iterate(u32 start, u32 finish, auto&& func) noexcept {
            if (start > getLength())
                start = getLength();

            if (finish > getLength())
                finish = getLength();

            if (start < finish) {
                for (auto i = begin() + start; i < begin() + finish; i++)
                    func(*i);
            }
            else {
                for (auto i = begin() + start - 1; i >= begin() + finish; i--)
                    func(*i);
            }
        }

        // Iterates array from start index to finish index.
        // Start and finish will be clamped to arrays length.
        // If start < finish, iterates from left to right. Finish is excluded from iteration.
        // If start >= finish, iterates from right to left. Start is excluded from iteration.
        constexpr void Iterate(u32 start, u32 finish, auto&& func) const noexcept {
            if (start > getLength())
                start = getLength();

            if (finish > getLength())
                finish = getLength();

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

    template <typename Type>
    using HeapArray = Array<Type>;

    template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Array<Type>& array) noexcept {
        ostream << Char('[') << array.getLength() << Char('/') << array.getCapacity() << Char(']') << Char('{') << Char(' ');

        for (auto& i : array)
            ostream << i << Char(',') << Char(' ');

        return ostream << Char('}');
    }

    template <typename Type> template <u32 capacity>
    class Array<Type>::Static {
        union {
            Type data[capacity];
        };

        u32 length = 0;

    public:
        static_assert(capacity > 1);

        constexpr Static() noexcept {};

        constexpr Static(const Static& other) noexcept {
            operator=(other);
        }

        constexpr Static(Static&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Static() noexcept {
            Clear();
        }

        constexpr Static& operator =(const Static& other) noexcept {
            Clear();

            length = other.length;

            if constexpr (trivialCopy) {
                std::copy(other.data, other.data + length, data);
            }
            else {
                for (unsigned i = 0; i < length; i++)
                    ACTL::Construct(data + i, other.data[i]);
            }

            return *this;
        }

        constexpr Static& operator =(Static&& other) noexcept {
            Clear();

            length = other.length;

            if constexpr (trivialMoveConstruct) {
                std::copy(other.data, other.data + length, data);
            }
            else {
                for (unsigned i = 0; i < length; i++)
                    ACTL::Construct(data + i, ACTL::move(other.data[i]));
            }

            other.Clear();

            return *this;
        }

        // Constructs a new element at the end of array and returns it.
        // Throws exception if array is full.
        template <typename... Args>
        constexpr Type& EmplaceBack(Args&&... args) {
            if (isFull())
                throw "Array is full!";

            ACTL::Construct(data + length, ACTL::forward<Args>(args)...);

            length++;

            return data[length - 1];
        }

        // Constructs a new element at the end of array and returns it.
        // Returns back if array is full.
        template <typename... Args>
        constexpr Type& EmplaceBackOrGet(Args&&... args) noexcept {
            if (isFull())
                return data[length - 1];

            ACTL::Construct(data + length, ACTL::forward<Args>(args)...);

            length++;

            return data[length - 1];
        }

        // Erases back of array.
        // Does nothing if array is empty.
        constexpr void EraseBack() noexcept {
            if (isEmpty())
                return;

            length--;

            if constexpr (!trivialDestruct)
                data[length].~Type();
        }

        // Emplaces many elements at the back.
        // Throws exception if array has not enough space.
        template <typename... Args>
        constexpr void EmplaceBackMany(u32 count, Args&&... args) {
            if (!count)
                return;

            if (length + count > capacity)
                throw "Array has not enough free space!";

            for (u32 i = 0; i < count; i++)
                ACTL::Construct(data + length + i, ACTL::forward<Args>(args)...);

            length += count;
        }

        // Emplaces many elements at the back.
        // If array has not enough space, emplaces elements with count equal to free space.
        template <typename... Args>
        constexpr void EmplaceBackManyOrLess(u32 count, Args&&... args) noexcept {
            if (!count)
                return;

            if (length + count > capacity)
                count = capacity - length;

            for (u32 i = 0; i < count; i++)
                ACTL::Construct(data + length + i, ACTL::forward<Args>(args)...);

            length += count;
        }

        // Erases many elements from the back of array.
        constexpr void EraseBackMany(u32 count) {
            if (count > length)
                count = length;

            if (!count)
                return;

            length -= count;

            if constexpr (!trivialDestruct) {
                for (u32 i = 0; i < count; i++)
                    data[length + i].~Type();
            }
        }

        // Emplaces element to the back and swaps it with element of given index.
        // If index >= length calls EmplaceBack.
        template <typename... Args>
        constexpr Type& EmplaceFast(u32 index, Args&&... args) {
            if (index >= length)
                return EmplaceBack(ACTL::forward<Args>(args)...);

            auto& n = EmplaceBack(ACTL::forward<Args>(args)...);

            auto& o = data[index];

            ACTL::swap(n, o);

            return o;
        }

        // Emplaces element to the back and swaps it with element of given index.
        // If index >= length calls EmplaceBack.
        // If array is full, returns existing element by index.
        template <typename... Args>
        constexpr Type& EmplaceFastOrGet(u32 index, Args&&... args) noexcept {
            if (index >= length)
                return EmplaceBackOrGet(ACTL::forward<Args>(args)...);

            if (isFull())
                return data[index];

            auto& n = EmplaceBack(ACTL::forward<Args>(args)...);

            auto& o = data[index];

            ACTL::swap(n, o);

            return o;
        }

        // Swaps element of the given index with element at the back and destructs it.
        // Calls EraseBack if index >= length.
        constexpr void EraseFast(u32 index) noexcept {
            if (index >= length)
                return EraseBack();

            auto& t = data[index];

            auto& e = data[length - 1];

            ACTL::swap(t, e);

            EraseBack();
        }

        // Constructs element at the given index and saves order.
        // If index >= length, calls EmplaceBack.
        // If array is full, throws exception.
        template <typename... Args>
        constexpr Type& EmplaceStrict(u32 index, Args&&... args) {
            if (index >= length)
                return EmplaceBack(ACTL::forward<Args>(args)...);

            if (isFull())
                throw "Array is full!";

            ACTL::Construct(data + length, ACTL::move(data[length - 1]));

            for (u32 d = length - 1; d > index; d--)
                data[d] = ACTL::move(data[d - 1]);

            data[index].~Type();

            ACTL::Construct(data + index, ACTL::forward<Args>(args)...);

            length++;

            return data[index];
        }

        // Constructs element at the given index and saves order.
        // If index >= length, calls EmplaceBackOrGet.
        // If array is full, returns element by index.
        template <typename... Args>
        constexpr Type& EmplaceStrictOrGet(u32 index, Args&&... args) noexcept {
            if (index >= length)
                return EmplaceBackOrGet(ACTL::forward<Args>(args)...);

            if (isFull())
                return data[index];

            ACTL::Construct(data + length, ACTL::move(data[length - 1]));

            for (u32 d = length - 1; d > index; d--)
                data[d] = ACTL::move(data[d - 1]);

            data[index].~Type();

            ACTL::Construct(data + index, ACTL::forward<Args>(args)...);

            length++;

            return data[index];
        }

        // Erases element by index and saves order.
        // Calls EraseBack if index >= length;
        constexpr void EraseStrict(u32 index) noexcept {
            if (index >= length)
                return EraseBack();

            if (isEmpty())
                return;

            for (Type* d = data + index; d < data + length - 1; d++)
                *d = ACTL::move(*(d + 1));

            EraseBack();
        }

        // Emplaces element and keeps array sorted.
        // Throws exception if array is full.
        template <typename... Args>
        constexpr Type& EmplaceSorted(Args&&... args) {
            Type value(ACTL::forward<Args>(args)...);

            auto sr = Find(value);

            return EmplaceStrict(sr.index, ACTL::move(value));
        }

        // Emplaces element and keeps array sorted.
        // Returns existing element if array is full.
        template <typename... Args>
        constexpr Type& EmplaceSortedOrGet(Args&&... args) {
            Type value(ACTL::forward<Args>(args)...);

            auto sr = Find(value);

            return EmplaceStrictOrGet(sr.index, ACTL::move(value));
        }

        // Erases element and keeps array sorted.
        // Does nothing if element was not found.
        constexpr void EraseSorted(const Type& value) noexcept {
            if (isEmpty())
                return;

            auto sr = Find(value);

            if (sr.found)
                EraseStrict(sr.index);
        }

        // Destructs all elements.
        constexpr void Clear() noexcept {
            if constexpr (trivialDestruct) {
                length = 0;
            }
            else {
                while (length) {
                    length--;

                    data[length].~Type();
                }
            }
        }

        constexpr u32 getLength() const noexcept {
            return length;
        }

        constexpr u32 getCapacity() const noexcept {
            return capacity;
        }

        constexpr bool isFull() const noexcept {
            return length == capacity;
        }

        constexpr bool notEmpty() const noexcept {
            return length;
        }

        constexpr bool isEmpty() const noexcept {
            return !notEmpty();
        }

        constexpr operator bool() const noexcept {
            return notEmpty();
        }

        nodiscard constexpr SearchResult Find(const Type& value) const noexcept {
            return ACTL::Find(data, length, value);
        }

        constexpr void QuickSort() noexcept {
            ACTL::QuickSort(data, 0, getLength());
        }

        nodiscard constexpr Type* begin() noexcept {
            return data;
        }

        nodiscard constexpr const Type* begin() const noexcept {
            return data;
        }

        nodiscard constexpr Type* end() noexcept {
            return begin() + getLength();
        }

        nodiscard constexpr const Type* end() const noexcept {
            return begin() + getLength();
        }

        nodiscard constexpr Type& front() {
            if (notEmpty())
                return *begin();

            throw "Array is empty!";
        }

        nodiscard constexpr const Type& front() const {
            if (notEmpty())
                return *begin();

            throw "Array is empty!";
        }

        nodiscard constexpr Type& back() {
            if (notEmpty())
                return end()[-1];

            throw "Array is empty!";
        }

        nodiscard constexpr const Type& back() const {
            if (notEmpty())
                return end()[-1];

            throw "Array is empty!";
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr Type& GetOrExcept(u32 index) {
            if (index < getLength())
                return data[index];

            throw "Invalid array index!";
        }

        // Returns reference to a element.
        // Throws exeception if index >= length.
        nodiscard constexpr const Type& GetOrExcept(u32 index) const {
            if (index < getLength())
                return data[index];

            throw "Invalid array index!";
        }

        // Returns copy of element if index < length.
        // Returns new instance if index >= length.
        template <typename... Args>
        nodiscard constexpr Type GetOrNew(u32 index, Args&&... args) const noexcept {
            if (index < getLength())
                return data[index];

            return Type(ACTL::forward<Args>(args)...);
        }

        constexpr void Iterate(auto&& func) noexcept {
            for (auto& i : *this)
                func(i);
        }

        constexpr void Iterate(auto&& func) const noexcept {
            for (auto& i : *this)
                func(i);
        }

        // Iterates array from start index to finish index.
        // Start and finish will be clamped to arrays length.
        // If start < finish, iterates from left to right. Finish is excluded from iteration.
        // If start >= finish, iterates from right to left. Start is excluded from iteration.
        constexpr void Iterate(u32 start, u32 finish, auto&& func) noexcept {
            if (start > getLength())
                start = getLength();

            if (finish > getLength())
                finish = getLength();

            if (start < finish) {
                for (auto i = begin() + start; i < begin() + finish; i++)
                    func(*i);
            }
            else {
                for (auto i = begin() + start - 1; i >= begin() + finish; i--)
                    func(*i);
            }
        }

        // Iterates array from start index to finish index.
        // Start and finish will be clamped to arrays length.
        // If start < finish, iterates from left to right. Finish is excluded from iteration.
        // If start >= finish, iterates from right to left. Start is excluded from iteration.
        constexpr void Iterate(u32 start, u32 finish, auto&& func) const noexcept {
            if (start > getLength())
                start = getLength();

            if (finish > getLength())
                finish = getLength();

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

    template <typename Type, u32 capacity>
    using StaticArray = typename Array<Type>::template Static<capacity>;

    template <typename Char, typename Type, u32 capacity>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const StaticArray<Type, capacity>& array) noexcept {
        ostream << Char('[') << array.getLength() << Char('/') << capacity << Char(']') << Char('{') << Char(' ');

        for (auto& i : array)
            ostream << i << Char(',') << Char(' ');

        return ostream << Char('}');
    }
}