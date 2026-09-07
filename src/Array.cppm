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

#include <initializer_list>
#include <memory>
#include <type_traits>
#include <ostream>

export module ACTL:Array;

import :Defines;

namespace ACTL {
    // A result of a search in array.
    export struct SearchResult {
        // Is index of found element.
        // Or index of new element placement.
        u32 index;

        // Is true if element is found.
        bool success;
    };

    // Resizable heap buffer.
    export template <typename Type> requires(!std::is_reference_v<Type>)
    class Array {
        template <typename... Args>
        static constexpr bool noThrowConstructor = std::is_nothrow_constructible_v<Type, Args...>;

        static constexpr bool noThrowDestructor = std::is_nothrow_destructible_v<Type>;

        static constexpr bool trivialDestructor = std::is_trivially_destructible_v<Type>;

        Type* data = nullptr;

        u32 length = 0;

        u32 capacity = 0;

        constexpr void ShiftRight(u32 index, u32 amount) noexcept(noThrowDestructor && noThrowConstructor<Type&&>) {
            for (auto i = length; i > index; i--) {
                auto& s = data[i - 1];

                std::construct_at(data + i + amount - 1, Move(s));

                s.~Type();
            }
        }

        constexpr void ShiftLeft(u32 index, u32 amount) noexcept(noThrowDestructor && noThrowConstructor<Type&&>) {
            for (auto i = index; i < length; i++) {
                auto& s = data[i];

                std::construct_at(data + i - amount, Move(s));

                s.~Type();
            }
        }

    public:
        // Can be constinit.
        constexpr Array() noexcept {};

        // Preallocates memory.
        constexpr Array(u32 capacity) noexcept {
            Allocate(capacity);
        }

        // Copies data.
        constexpr Array(const std::initializer_list<Type>& list) noexcept(noThrowConstructor<const Type&>) {
            Allocate(list.size());

            length = list.size();

            for (u32 i = 0; i < list.size(); i++)
                std::construct_at(data + i, list.begin()[i]);
        }

        // Constructs elements of given count.
        template <typename... Args>
        constexpr Array(u32 count, Args&&... args) noexcept(noThrowConstructor<Args...>) {
            Allocate(count);

            length = count;

            for (u32 i = 0; i < count; i++)
                std::construct_at(data + i, Forward<Args>(args)...);
        }

        // Copies every element of other.
        constexpr Array(const Array& other) noexcept(noThrowConstructor<const Type&>) {
            operator=(other);
        }

        // Sets other`s memory and leaves other in null state.
        constexpr Array(Array&& other) noexcept {
            operator=(Move(other));
        }

        // Destructs and deallocates all data.
        constexpr ~Array() noexcept(noThrowDestructor) {
            Free();
        }

        // Copies every element of other.
        // May expand memory up to other`s.
        constexpr Array& operator =(const Array& other) noexcept(noThrowDestructor && noThrowConstructor<const Type&>) {
            if (this == &other)
                return *this;

            Clear();

            if (capacity < other.length)
                Allocate(other.capacity);

            length = other.length;

            for (u32 i = 0; i < length; i++)
                std::construct_at(data + i, other.begin()[i]);

            return *this;
        }

        // Sets other`s memory and leaves other in null state.
        constexpr Array& operator =(Array&& other) noexcept(noThrowDestructor) {
            if (this == &other)
                return *this;

            Free();

            Swap(data, other.data);

            Swap(length, other.length);

            Swap(capacity, other.capacity);

            return *this;
        }

        // Allocates new memory.
        // Frees previous memory.
        constexpr void Allocate(u32 capacity) noexcept(noThrowDestructor) {
            Free();

            this->capacity = capacity;

            std::allocator<Type> allocator = {};

            data = allocator.allocate(capacity);
        }

        // Doubles capacity of array or allocates 8 elements if array is empty.
        constexpr void Expand() noexcept(noThrowDestructor && noThrowConstructor<Type&&>) {
            if (capacity)
                Expand(capacity);
            else
                Allocate(8);
        }

        // Increases capacity of array.
        constexpr void Expand(u32 count) noexcept(noThrowDestructor && noThrowConstructor<Type&&>) {
            if (!count)
                return;

            if (!data)
                return Allocate(count);

            auto newCap = capacity + count;

            std::allocator<Type> allocator = {};

            auto newData = allocator.allocate(newCap);

            auto l = length;

            for (u32 i = 0; i < length; i++)
                std::construct_at(newData + i, Move(data[i]));

            Free();

            data = newData;

            capacity = newCap;

            length = l;
        }

        // Shrinks capacity up to length.
        constexpr void ShrinkToFit() noexcept(noThrowDestructor && noThrowConstructor<Type&&>) {
            if (length == capacity)
                return;

            std::allocator<Type> allocator = {};

            auto newData = allocator.allocate(length);

            auto l = length;

            for (u32 i = 0; i < length; i++)
                std::construct_at(newData + i, Move(data[i]));

            Free();

            data = newData;

            capacity = l;

            length = l;
        }

        // Destructs and deallocates memory.
        constexpr void Free() noexcept(noThrowDestructor) {
            Clear();

            if (data) {
                std::allocator<Type> allocator = {};

                allocator.deallocate(data, capacity);

                data = nullptr;

                capacity = 0;
            }
        }

        // Constructs element in array saving its order.
        // If index < length, emplaces element before element with this index.
        // If index == length, emplaces element after the last one.
        // If index > length, emplaces element at index and constructs elements between emplaced and back.
        template <typename... Args>
        constexpr Type& EmplaceStrict(u32 index, Args&&... args) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<Args...>) {
            if (index < length) {
                if (isFull())
                    Expand();

                ShiftRight(index, 1);

                std::construct_at(data + index, Forward<Args>(args)...);

                length += 1;
            }
            else {
                if (index >= capacity * 2)
                    Expand(index + 1 - capacity);
                else if (index >= capacity)
                    Expand();

                for (auto i = data + length; i <= data + index; i++)
                    std::construct_at(i, Forward<Args>(args)...);

                length = index + 1;
            }

            return data[index];
        }

        // Destructs element in array saving its order.
        // Does nothing if index >= length.
        constexpr void EraseStrict(u32 index) noexcept(noThrowConstructor<Type&&> && noThrowDestructor) {
            if (index >= length)
                return;
            else if (index == length - 1)
                data[length - 1].~Type();
            else {
                data[index].~Type();

                ShiftLeft(index + 1, 1);
            }

            length--;
        }

        // Emplaces element at the back.
        template <typename... Args>
        constexpr Type& EmplaceBack(Args&&... args) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<Args...>) {
            return EmplaceStrict(length, Forward<Args>(args)...);
        }

        // Emplaces many elements at the back.
        template <typename... Args>
        constexpr void EmplaceBackMany(u32 count, Args&&... args) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<Args...>) {
            if (count)
                EmplaceStrict(length + count - 1, Forward<Args>(args)...);
        }

        // Erases element at the back.
        // Does nothing if array is empty.
        constexpr void EraseBack() noexcept(noThrowDestructor) {
            EraseStrict(length - 1);
        }

        template <typename... Args>
        constexpr Type& EmplaceSorted(Args&&... args) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<Args...>) {
            Type value(Forward<Args>(args)...);

            auto sr = Find(value);

            return EmplaceStrict(sr.index, Move(value));
        }

        constexpr void EraseSorted(const Type& value) noexcept(noThrowConstructor<Type&&> && noThrowDestructor) {
            auto sr = Find(value);

            if (sr.success)
                EraseStrict(sr.index);
        }

        // Inserts new elements.
        // If index >= length, emplaces them at the back.
        template <typename... Args>
        constexpr void Insert(u32 index, u32 count, Args&&... args) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<Args...>) {
            if (!count)
                return;

            if (length + count >= capacity * 2)
                Expand(length + count - capacity);
            else if (length + count >= capacity)
                Expand();

            if (index >= length) {
                for (u32 i = 0; i < count; i++)
                    std::construct_at(data + length + i, Forward<Args>(args)...);
            }
            else {
                ShiftRight(index, count);

                for (u32 i = 0; i < count; i++)
                    std::construct_at(data + index + i, Forward<Args>(args)...);
            }

            length += count;
        }

        // Inserts new elements from other array.
        // If index >= length, emplaces them at the back.
        constexpr void Insert(u32 index, const Array<Type>& other) noexcept(noThrowConstructor<Type&&> && noThrowDestructor && noThrowConstructor<const Type&>) {
            if (!other)
                return;

            if (length + other.length >= capacity * 2)
                Expand(length + other.length - capacity);
            else if (length + other.length >= capacity)
                Expand();

            if (index >= length) {
                for (u32 i = 0; i < other.length; i++)
                    std::construct_at(data + length + i, other.begin()[i]);
            }
            else {
                ShiftRight(index, other.length);

                for (u32 i = 0; i < other.length; i++)
                    std::construct_at(data + index + i, other.begin()[i]);
            }

            length += other.length;
        }

        // Inserts new elements from other array.
        // If index >= length, emplaces them at the back.
        constexpr void Insert(u32 index, Array<Type>&& other) noexcept(noThrowConstructor<Type&&> && noThrowDestructor) {
            if (!other)
                return;

            if (length + other.length >= capacity * 2)
                Expand(length + other.length - capacity);
            else if (length + other.length >= capacity)
                Expand();

            if (index >= length) {
                for (u32 i = 0; i < other.length; i++)
                    std::construct_at(data + length + i, Move(other.begin()[i]));
            }
            else {
                ShiftRight(index, other.length);

                for (u32 i = 0; i < other.length; i++)
                    std::construct_at(data + index + i, Move(other.begin()[i]));
            }

            length += other.length;
        }

        // Destructs elements in array saving its order.
        // Does nothing if index >= length.
        // It is safe to set incorrect count of elements.
        constexpr void EraseStrict(u32 index, u32 count) noexcept(noThrowConstructor<Type&&> && noThrowDestructor) {
            if (!count || index >= length)
                return;

            if (index + count >= length) {
                for (u32 i = index; i < length; i++)
                    data[i].~Type();

                length = index;
            }
            else {
                for (u32 i = 0; i < count; i++)
                    data[index + i].~Type();

                ShiftLeft(index + count, count);

                length -= count;
            }
        }

        // Destructs all elements.
        constexpr void Clear() noexcept(noThrowDestructor) {
            if constexpr (trivialDestructor) {
                length = 0;

                return;
            }
            
            while (length) {
                length--;

                data[length].~Type();
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

        constexpr Type* begin() noexcept {
            return data;
        }

        constexpr Type* end() noexcept {
            return data + length;
        }

        constexpr const Type* begin() const noexcept {
            return data;
        }

        constexpr const Type* end() const noexcept {
            return data + length;
        }

        // Throws exception if index >= length.
        constexpr Type& operator [](u32 index) {
            if (index < length)
                return data[index];

            throw "Invalid index!";
        }

        // Throws exception if index >= length.
        constexpr const Type& operator [](u32 index) const {
            if (index < length)
                return data[index];

            throw "Invalid index!";
        }

        // Searches for element in array and returns SearchResult.
        constexpr SearchResult Find(const Type& value) const noexcept {
            if (!length)
                return {
                    0,
                    false
                };

            u32 low = 0, high = length - 1;

            while (low <= high) {
                u32 middle = low + (high - low) / 2;

                if (data[middle] == value)
                    return {
                        middle,
                        true
                    };

                if (data[middle] < value)
                    low = middle + 1;
                else {
                    if (middle == 0)
                        break;

                    high = middle - 1;
                }
            }

            return {
                low,
                false
            };
        }

        template <typename Func> requires(std::is_invocable_v<Func, Type&>)
        constexpr void Iterate(Func&& func) noexcept(std::is_nothrow_invocable_v<Func, Type&>) {
            for (auto& value : *this)
                func(value);
        }

        template <typename Func> requires(std::is_invocable_v<Func, const Type&>)
        constexpr void Iterate(Func&& func) const noexcept(std::is_nothrow_invocable_v<Func, const Type&>) {
            for (const auto& value : *this)
                func(value);
        }

        template <typename Func> requires(std::is_invocable_v<Func, Type&>)
        constexpr void Iterate(bool reverse, Func&& func) noexcept(std::is_nothrow_invocable_v<Func, Type&>) {
            if (reverse) {
                for (auto i = getLength(); i > 0; i--)
                    func(data[i - 1]);
            }
            else {
                for (auto& value : *this)
                    func(value);
            }
        }

        template <typename Func> requires(std::is_invocable_v<Func, const Type&>)
        constexpr void Iterate(bool reverse, Func&& func) const noexcept(std::is_nothrow_invocable_v<Func, const Type&>) {
            if (reverse) {
                for (auto i = getLength(); i > 0; i--)
                    func(data[i - 1]);
            }
            else {
                for (const auto& value : *this)
                    func(value);
            }
        }
    };

    export template <typename Type>
    std::ostream& operator <<(std::ostream& ostream, const Array<Type>& array) noexcept {
        ostream << '[' << array.getLength() << '/' << array.getCapacity() << "]{ ";

        for (auto& i : array)
            ostream << i << ", ";

        return ostream << '}';
    }

    export template <typename Type>
    std::wostream& operator <<(std::wostream& ostream, const Array<Type>& array) noexcept {
        ostream << L'[' << array.getLength() << L'/' << array.getCapacity() << L"]{ ";

        for (auto& i : array)
            ostream << i << L", ";

        return ostream << L'}';
    }

    consteval bool AllocateAndFreeTest() {
        Array<int> array = {};

        array.Allocate(2);

        if (array.getCapacity() != 2)
            return false;

        array.Free();

        if (array.getCapacity())
            return false;

        array.Allocate(10);

        if (array.getCapacity() != 10)
            return false;

        return true;
    }

    static_assert(AllocateAndFreeTest(), "Allocate and Free test is FAILED!");

    class Class {
    public:
        i32 data;
        
        u32* counter;

        constexpr Class(i32 value, u32& counter) noexcept : data(value), counter(&counter) {
            counter += 1;
        }

        constexpr Class(const Class& other) noexcept : data(other.data), counter(other.counter) {
            (*counter) += 1;
        }

        constexpr Class(Class&& other) noexcept : data(other.data), counter(other.counter) {
            (*counter) += 1;
        }

        constexpr ~Class() noexcept {
            (*counter) -= 1;
        }

        constexpr Class& operator =(const Class& other) noexcept {
            data = other.data;

            return *this;
        }

        constexpr Class& operator =(Class&& other) noexcept {
            data = other.data;

            return *this;
        }
    };

    template <u32 length>
    consteval bool Compare(const Array<Class>& array, const i32 (&values)[length]) {
        if (array.getLength() != length)
            return false;

        for (u32 i = 0; i < length; i++)
            if (values[i] != array.begin()[i].data)
                return false;

        return true;
    }

    template <u32 length>
    consteval bool Compare(const Array<int>& array, const i32 (&values)[length]) {
        if (array.getLength() != length)
            return false;

        for (u32 i = 0; i < length; i++)
            if (values[i] != array.begin()[i])
                return false;

        return true;
    }

    consteval bool EmplaceAndEraseTest() {
        u32 counter = 0;

        Array<Class> array = {};

        array.Allocate(32);

        array.EmplaceBack(0, counter);

        if (!counter)
            return false;

        if (!Compare(array, { 0 }))
            return false;

        array.EmplaceStrict(0, 10, counter);

        if (counter != 2)
            return false;

        if (!Compare(array, { 10, 0 }))
            return false;

        array.EmplaceStrict(1, 10, counter);

        if (counter != 3)
            return false;

        if (!Compare(array, { 10, 10, 0 }))
            return false;

        array.EmplaceStrict(5, 5, counter);

        if (counter != 6)
            return false;

        if (!Compare(array, { 10, 10, 0, 5, 5, 5 }))
            return false;

        array.EraseBack();

        if (counter != 5)
            return false;

        if (!Compare(array, { 10, 10, 0, 5, 5 }))
            return false;

        array.EraseStrict(0);

        if (counter != 4)
            return false;

        if (!Compare(array, { 10, 0, 5, 5 }))
            return false;

        array.EraseStrict(2);

        if (counter != 3)
            return false;

        if (!Compare(array, { 10, 0, 5 }))
            return false;

        return true;
    }

    static_assert(EmplaceAndEraseTest(), "Emplace and Erase test is FAILED!");

    consteval bool ExpandAndShrinkTest() {
        u32 counter = 0;

        Array<Class> array = 2;

        array.EmplaceBack(10, counter);

        array.EmplaceBack(20, counter);

        array.Expand();

        if (array.getCapacity() != 4)
            return false;

        if (counter != 2)
            return false;

        if (!Compare(array, { 10, 20 }))
            return false;

        array.ShrinkToFit();

        if (array.getCapacity() != 2)
            return false;

        if (counter != 2)
            return false;

        if (!Compare(array, { 10, 20 }))
            return false;

        array.EmplaceStrict(4, 0, counter);

        if (array.getCapacity() != 5)
            return false;

        if (counter != 5)
            return false;

        if (!Compare(array, { 10, 20, 0, 0, 0 }))
            return false;

        return true;
    }

    static_assert(ExpandAndShrinkTest(), "Expand and Shrink test is FAILED!");

    consteval bool FindTest() {
        Array<int> array = {
            0, 1, 2, 3
        };

        SearchResult sr = {};

        sr = array.Find(4);

        if (sr.index != 4 || sr.success)
            return false;

        sr = array.Find(0);

        if (sr.index != 0 || !sr.success)
            return false;

        sr = array.Find(1);

        if (sr.index != 1 || !sr.success)
            return false;

        sr = array.Find(10);

        if (sr.index != 4 || sr.success)
            return false;

        sr = array.Find(-1);

        if (sr.index != 0 || sr.success)
            return false;

        array.EmplaceSorted(10);

        if (!Compare(array, { 0, 1, 2, 3, 10 }))
            return false;

        array.EmplaceSorted(2);

        if (!Compare(array, { 0, 1, 2, 2, 3, 10 }))
            return false;

        array.EmplaceSorted(-1);

        if (!Compare(array, { -1, 0, 1, 2, 2, 3, 10 }))
            return false;

        array.EraseSorted(50);

        if (!Compare(array, { -1, 0, 1, 2, 2, 3, 10 }))
            return false;

        array.EraseSorted(0);

        if (!Compare(array, { -1, 1, 2, 2, 3, 10 }))
            return false;

        array.EraseSorted(2);

        if (!Compare(array, { -1, 1, 2, 3, 10 }))
            return false;

        return true;
    }

    static_assert(FindTest(), "Find test is FAILED!");

    consteval bool InsertTest() {
        u32 counter = 0;

        Array<Class> array(3, 0, counter);

        array.Insert(0, { 2, 5, counter });

        if (counter != 5)
            return false;

        if (!Compare(array, { 5, 5, 0, 0, 0 }))
            return false;

        array.Insert(2, { 2, -1, counter });

        if (counter != 7)
            return false;

        if (!Compare(array, { 5, 5, -1, -1, 0, 0, 0 }))
            return false;

        array.Insert(8, { 3, 99, counter });

        if (counter != 10)
            return false;

        if (!Compare(array, { 5, 5, -1, -1, 0, 0, 0, 99, 99, 99 }))
            return false;

        array.EraseStrict(0, 2);

        if (counter != 8)
            return false;

        if (!Compare(array, { -1, -1, 0, 0, 0, 99, 99, 99 }))
            return false;

        array.EraseStrict(5, 4);

        if (counter != 5)
            return false;

        if (!Compare(array, { -1, -1, 0, 0, 0 }))
            return false;

        array.EraseStrict(2, 2);

        if (counter != 3)
            return false;

        if (!Compare(array, { -1, -1, 0 }))
            return false;

        return true;
    }

    static_assert(InsertTest(), "Insert test is FAILED!");
}