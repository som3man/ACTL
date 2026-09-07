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

#include <memory>
#include <ostream>
#include <type_traits>

export module ACTL:Heap;

import :Defines;
import :String;
import :Option;

namespace ACTL {
    // Unique heap memory owner.
    export template <typename Type> requires(!std::is_reference_v<Type> && !std::is_same_v<Void, Type>)
    class Heap {
        Type* data = nullptr;

    public:
        // Null state initializer. Can be constinit.
        constexpr Heap(Void) noexcept {};

        // Allocates memory.
        template <typename... Args>
        constexpr Heap(Args&&... args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) {
            Allocate(Forward<Args>(args)...);
        }

        // Copies state and data of other.
        constexpr Heap(const Heap& other) noexcept {
            operator=(other);
        }

        // Transfers memory ownership from other.
        constexpr Heap(Heap&& other) noexcept {
            operator=(Move(other));
        }

        // Frees memory.
        constexpr ~Heap() noexcept(std::is_nothrow_destructible_v<Type>) {
            Free();
        }

        // Copies state and data of other.
        constexpr Heap& operator =(const Heap& other) noexcept(std::is_nothrow_destructible_v<Type> && std::is_nothrow_constructible_v<Type, const Heap&>) {
            other.Visit(
                [&](const Type& value) {
                    if (data)
                        *data = value;
                    else
                        Allocate(value);
                },
                [&]() {
                    Free();
                }
            );

            return *this;
        }

        // Transfers memory ownership from other.
        constexpr Heap& operator =(Heap&& other) noexcept(std::is_nothrow_destructible_v<Type>) {
            Free();

            data = other.data;

            other.data = nullptr;

            return *this;
        }

        // Allocates memory.
        template <typename... Args>
        constexpr Type& Allocate(Args&&... args) noexcept(std::is_nothrow_destructible_v<Type> && std::is_nothrow_constructible_v<Type, Args...>) {
            if (data)
                data->~Type();
            else {
                std::allocator<Type> allocator = {};

                data = allocator.allocate(1);
            }

            std::construct_at(data, Forward<Args>(args)...);

            return *data;
        }

        // Allocates memory of derived type.
        // Type MUST have virtual destructor.
        template <typename Child, typename... Args> requires(std::is_base_of_v<Type, Child> && std::has_virtual_destructor_v<Type>)
        constexpr Child& AllocateChild(Args&&... args) noexcept(std::is_nothrow_destructible_v<Type> && std::is_nothrow_constructible_v<Child, Args...>) {
            Free();

            std::allocator<Child> allocator = {};

            auto p = allocator.allocate(1);

            std::construct_at(p, Forward<Args>(args)...);

            data = static_cast<Type*>(p);

            return *p;
        }

        // Frees memory.
        constexpr void Free() noexcept(std::is_nothrow_destructible_v<Type>) {
            if (data) {
                data->~Type();

                std::allocator<Type> allocator = {};

                allocator.deallocate(data, 1);

                data = nullptr;
            }
        }

        constexpr bool notVoid() const noexcept {
            return data;
        }

        constexpr bool isVoid() const noexcept {
            return !notVoid();
        }

        constexpr operator bool() const noexcept {
            return notVoid();
        }

        template <typename Return = void> 
        constexpr Return Visit(auto&& ifData, auto&& ifVoid) noexcept {
            if (data)
                return ifData(*(static_cast<const Type*>(data)));
            else
                return ifVoid();
        }

        template <typename Return = void> 
        constexpr Return Visit(auto&& ifData, auto&& ifVoid) const noexcept {
            if (data)
                return ifData(*data);
            else
                return ifVoid();
        }

        // Throws exception if heap is void.
        constexpr Type& Get() {
            if (data)
                return *data;

            throw "Heap is void!";
        }

        // Throws exception if heap is void.
        constexpr const Type& Get() const {
            if (data)
                return *data;

            throw "Heap is void!";
        }

        // Returns new instance if heap is void.
        template <typename... Args>
        constexpr Type GetOr(Args&&... args) const noexcept(std::is_nothrow_constructible_v<Type, const Type&> && std::is_nothrow_constructible_v<Type, Args...>) {
            if (data)
                return *data;

            return Type(Forward<Args>(args)...);
        }

        // Throws exception if heap is void.
        constexpr Type& operator *() {
            return Get();
        }

        // Throws exception if heap is void.
        constexpr const Type& operator *() const {
            return Get();
        }

        // Throws exception if heap is void.
        constexpr Type* operator ->() {
            return &Get();
        }

        // Throws exception if heap is void.
        constexpr const Type* operator ->() const {
            return &Get();
        }

        // Throws exception if heap is void.
        constexpr operator Type& () {
            return Get();
        }

        // Throws exception if heap is void.
        constexpr operator const Type& () const {
            return Get();
        }

        // Compares pointers to data.
        constexpr bool operator ==(const Heap& other) const noexcept {
            return data == other.data;
        }

        // Compares pointers to data.
        constexpr auto operator <=>(const Heap& other) const noexcept {
            return data <=> other.data;
        }
    };

    export template <typename Type>
    Heap(Type) -> Heap<std::remove_cvref_t<Type>>;

    export template <typename Type>
    Heap(Heap<Type>&&) -> Heap<Type>;

    export template <typename Type>
    std::ostream& operator <<(std::ostream& ostream, const Heap<Type>& heap) noexcept {
        heap.Visit(
            [&](const Type& value) {
                ostream << value;
            },
            [&]() {
                ostream << Void();
            }
        );

        return ostream;
    }

    export template <typename Type>
    std::wostream& operator <<(std::wostream& ostream, const Heap<Type>& heap) noexcept {
        heap.Visit(
            [&](const Type& value) {
                ostream << value;
            },
            [&]() {
                ostream << Void();
            }
        );

        return ostream;
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Heap<Type>& heap) noexcept {
        heap.Visit(
            [&](const Type& value) {
                string << value;
            },
            [&]() {
                string << Void();
            }
        );

        return string;
    }

    // Shared heap memory reference.
    export template <typename Type> requires(!std::is_reference_v<Type> && !std::is_same_v<Void, Type>)
    class Shared {
        Type* data = nullptr;

        u32* counter = nullptr;

    public:
        // Null state initializer. Can be constinit.
        constexpr Shared(Void) noexcept {};

        // Copies referencing state.
        constexpr Shared(const Shared& other) noexcept {
            operator=(other);
        }

        // Transfers referencing state.
        constexpr Shared(Shared&& other) noexcept {
            operator=(Move(other));
        }

        // Drops referencing. Referencing data is deleted if it has no references.
        constexpr ~Shared() noexcept {
            Drop();
        }

        // Copies referencing state.
        constexpr Shared& operator =(const Shared& other) noexcept(std::is_nothrow_destructible_v<Type>) {
            Drop();

            data = other.data;

            counter = other.counter;

            if (counter)
                *counter += 1;

            return *this;
        }

        // Transfers referencing state.
        constexpr Shared& operator =(Shared&& other) noexcept(std::is_nothrow_destructible_v<Type>) {
            Drop();

            Swap(data, other.data);

            Swap(counter, other.counter);

            return *this;
        }

        // Allocates new data. Shared will reference to it.
        template <typename... Args>
        constexpr Type& Allocate(Args&&... args) noexcept(std::is_nothrow_destructible_v<Type> && std::is_nothrow_constructible_v<Type, Args...>) {
            Drop();

            std::allocator<Type> dalloc = {};

            data = dalloc.allocate(1);

            std::construct_at(data, Forward<Args>(args)...);

            std::allocator<u32> calloc = {};

            counter = calloc.allocate(1);

            std::construct_at(counter, 1);

            return *data;
        }

        // Allocates new derived data. Shared will reference to it.
        // Type MUST have virtual destructor.
        template <typename Child, typename... Args> requires(std::is_base_of_v<Type, Child> && std::has_virtual_destructor_v<Type>)
        constexpr Child& AllocateChild(Args&&... args) noexcept(std::is_nothrow_destructible_v<Type> && std::is_nothrow_constructible_v<Child, Args...>) {
            Drop();

            std::allocator<Child> dalloc = {};

            auto p = dalloc.allocate(1);

            std::construct_at(p, Forward<Args>(args)...);

            data = static_cast<Type*>(p);

            std::allocator<u32> calloc = {};

            counter = calloc.allocate(1);

            *counter = 1;

            return *p;
        }

        // Drops referencing. Referencing data is deleted if it has no references.
        constexpr void Drop() noexcept(std::is_nothrow_destructible_v<Type>) {
            if (!data)
                return;

            *counter -= 1;

            if (*counter == 0) {
                data->~Type();

                std::allocator<Type> dalloc = {};

                dalloc.deallocate(data, 1);

                std::allocator<u32> call = {};

                call.deallocate(counter, 1);
            }

            counter = nullptr;

            data = nullptr;
        }

        constexpr bool notVoid() const noexcept {
            return data;
        }

        constexpr bool isVoid() const noexcept {
            return !notVoid();
        }

        constexpr operator bool() const noexcept {
            return notVoid();
        }

        // Throws exception if Shared has no reference.
        constexpr Type& Get() const {
            if (data)
                return *data;

            throw "Shared is void!";
        }

        // Returns new instance if Shared has no reference.
        template <typename... Args>
        constexpr Type GetOr(Args&&... args) const noexcept {
            if (data)
                return *data;

            return Type(Forward<Args>(args)...);
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifData, auto&& ifVoid) const noexcept {
            if (data)
                return ifData(*data);
            else
                return ifVoid();
        }

        // Returns count of references to current data referenced by Shared including itself.
        // Returns 0 if Shared doesn`t reference any data.
        constexpr u32 getRefCount() const noexcept {
            return counter ? *counter : 0;
        }

        constexpr Type& operator *() const {
            return Get();
        }

        constexpr Type* operator ->() const {
            return &Get();
        }

        constexpr operator Type& () const {
            return Get();
        }

        // Compares referenced data pointers.
        constexpr bool operator ==(const Shared& other) const noexcept {
            return data == other.data;
        }

        // Compares referenced data pointers.
        constexpr auto operator <=>(const Shared& other) const noexcept {
            return data <=> other.data;
        }
    };

    export template <typename Type>
    Shared(Shared<Type>&&) -> Shared<Type>;

    export template <typename Type>
    Shared(Type) -> Shared<std::remove_cvref_t<Type>>;

    export template <typename Type>
    std::ostream& operator <<(std::ostream& ostream, const Shared<Type>& shared) noexcept {
        shared.Visit(
            [&](const Type& value) {
                ostream << value;
            },
            [&]() {
                ostream << Void();
            }
        );

        return ostream;
    }

    export template <typename Type>
    std::wostream& operator <<(std::wostream& ostream, const Shared<Type>& shared) noexcept {
        shared.Visit(
            [&](const Type& value) {
                ostream << value;
            },
            [&]() {
                ostream << Void();
            }
        );

        return ostream;
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Shared<Type>& shared) noexcept {
        shared.Visit(
            [&](const Type& value) {
                string << value;
            },
            [&]() {
                string << Void();
            }
        );

        return string;
    }

    consteval bool HeapTest() {
        Heap<int> heap = Void();

        heap.Allocate(167);

        if (heap.isVoid())
            return false;

        if (heap.Get() != 167)
            return false;

        Heap heap2 = 90;

        heap2 = heap;

        if (heap2.Get() != 167)
            return false;

        return true;
    }

    static_assert(HeapTest(), "Heap test is FAILED!");

    consteval bool SharedTest() {
        Shared<int> shared = Void();

        if (shared || shared.getRefCount())
            return false;

        shared.Allocate(10);

        if (!shared || shared.getRefCount() != 1)
            return false;

        if (shared.Get() != 10)
            return false;

        Shared shared2 = shared;

        if (!shared2 || shared.getRefCount() != 2)
            return false;

        shared.Allocate(23);

        if (shared.getRefCount() != 1 || shared2.getRefCount() != 1)
            return false;

        return true;
    }

    static_assert(SharedTest(), "Shared test is FAILED!");
}