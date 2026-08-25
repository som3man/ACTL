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

#include <concepts>
#include <type_traits>
#include <ostream>

export module ACTL:Heap;

import :Defines;
import :Allocation;
import :Utility;
import :Option;

export namespace ACTL {
    // Unique pointer implementation.
    template <typename Type> requires(!std::is_reference_v<Type>)
    class Heap {
        Type* data = nullptr;

    public:
        // Can be constinit.
        constexpr Heap() noexcept {};

        template <typename... Args>
        constexpr Heap(Args&&... args) noexcept {
            Allocate(forward<Args>(args)...);
        }

        constexpr Heap(const Heap& other) noexcept {
            operator=(other);
        }

        constexpr Heap(Heap&& other) noexcept {
            operator=(move(other));
        }

        constexpr ~Heap() noexcept {
            Free();
        }

        // Assigns value to data if data is allocated.
        // Otherwise allocates new data with value as an argument.
        constexpr Heap& operator =(Type&& value) noexcept {
            if (isAllocated())
                *data = forward(value);
            else
                Allocate(forward(value));

            return *this;
        }

        constexpr Heap& operator =(const Heap& other) noexcept {
            other.Visit(
                [&](const Type& value) {
                    *this = value;
                },
                [&]() {
                    Free();
                }
            );

            return *this;
        }

        constexpr Heap& operator =(Heap&& other) noexcept {
            data = other.data;

            other.data = nullptr;

            return *this;
        }

        template <typename... Args>
        constexpr Type& Allocate(Args&&... args) noexcept {
            if (data)
                data->~Type();
            else
                data = ACTL::Allocate<Type>();

            ACTL::Construct(data, forward<Args>(args)...);

            return *data;
        }

        template <typename Child, typename... Args> requires(std::has_virtual_destructor_v<Type> && std::derived_from<Child, Type>)
        constexpr Child& Allocate(Args&&... args) noexcept {
            Free();

            Child* p = ACTL::Allocate<Child>();

            ACTL::Construct(p, forward<Args>(args)...);

            data = p;

            return *p;
        }

        constexpr void Free() noexcept {
            if (data) {
                data->~Type();

                ACTL::Free(data);

                data = nullptr;
            }
        }

        constexpr bool isAllocated() const noexcept {
            return data;
        }

        constexpr operator bool() const noexcept {
            return isAllocated();
        }

        // Throws exception if data is not allocated.
        constexpr Type& GetOrExcept() {
            if (data)
                return *data;

            throw "Heap is not allocated!";
        }

        // Throws exception if data is not allocated.
        constexpr const Type& GetOrExcept() const {
            if (data)
                return *data;

            throw "Heap is not allocated!";
        }

        // Returns new instance if data is not allocated.
        template <typename... Args>
        constexpr Type GetOrNew(Args&&... args) const noexcept {
            if (data)
                return *data;

            return Type(forward<Args>(args)...);
        }

        // Allocates data if it is not allocated.
        template <typename... Args>
        constexpr Type& GetOrAlloc(Args&&... args) noexcept {
            if (data)
                return *data;

            return Allocate(forward<Args>(args)...);
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifAlloc, auto&& ifVoid) noexcept {
            if (data)
                return ifAlloc(*data);
            else
                return ifVoid();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifAlloc, auto&& ifVoid) const noexcept {
            if (data)
                return ifAlloc(*data);
            else
                return ifVoid();
        }

        constexpr Type& operator *() {
            return GetOrExcept();
        }

        constexpr const Type& operator *() const {
            return GetOrExcept();
        }

        constexpr Type* operator ->() {
            return &GetOrExcept();
        }
        
        constexpr const Type* operator ->() const {
            return &GetOrExcept();
        }

        constexpr operator Type& () {
            return GetOrExcept();
        }

        constexpr operator const Type& () const {
            return GetOrExcept();
        }
    };

    template <typename Type>
    Heap(Type&&) -> Heap<std::remove_cvref_t<Type>>;

    template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Heap<Type>& heap) noexcept {
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

    // Shared pointer implementation.
    template <typename Type> requires(!std::is_reference_v<Type>)
    class Shared {
        Type* data = nullptr;

        u32* refCount = nullptr;

    public:
        // Can be constinit.
        constexpr Shared() noexcept {};

        template <typename... Args>
        constexpr Shared(Args&&... args) noexcept {
            Allocate(forward<Args>(args)...);
        }

        constexpr Shared(const Shared& other) noexcept {
            operator=(other);
        }

        constexpr Shared(Shared&& other) noexcept {
            operator=(move(other));
        }

        constexpr ~Shared() noexcept {
            Drop();
        }

        // Assigns value to data if data is allocated.
        // Otherwise allocates new data with value as an argument.
        constexpr Shared& operator =(Type&& value) noexcept {
            if (data)
                *data = forward(value);
            else
                Allocate(forward(value));

            return *this;
        }

        // Assigns pointer to other`s data.
        constexpr Shared& operator =(const Shared& other) noexcept {
            Drop();

            data = other.data;

            if (data) {
                refCount = other.refCount;

                (*refCount)++;
            }

            return *this;
        }

        constexpr Shared& operator =(Shared&& other) noexcept {
            Drop();

            swap(data, other.data);

            swap(refCount, other.refCount);

            return *this;
        }

        template <typename... Args>
        constexpr Type& Allocate(Args&&... args) noexcept {
            Drop();

            data = ACTL::Allocate<Type>();

            ACTL::Construct(data, forward<Args>(args)...);

            refCount = ACTL::Allocate<u32>();

            *refCount = 1;

            return *data;
        }

        template <typename Child, typename... Args> requires(std::has_virtual_destructor_v<Type> && std::derived_from<Child, Type>)
        constexpr Child& AllocateChild(Args&&... args) noexcept {
            Drop();

            Child* p = ACTL::Allocate<Type>();

            ACTL::Construct(p, forward<Args>(args)...);

            data = p;

            refCount = ACTL::Allocate<u32>();

            *refCount = 1;

            return *p;
        }

        constexpr void Drop() noexcept {
            if (!data)
                return;

            auto& c = *refCount;

            c--;

            if (c) {
                data = nullptr;

                return;
            }

            ACTL::Free(refCount);

            data->~Type();

            ACTL::Free(data);

            data = nullptr;
        }

        constexpr bool isAllocated() const noexcept {
            return data;
        }

        constexpr operator bool() const noexcept {
            return isAllocated();
        }

        // Throws exception if Shared has no data reference.
        constexpr Type& GetOrExcept() {
            if (isAllocated())
                return *data;

            throw "Shared is not allocated!";
        }

        // Throws exception if Shared has no data reference.
        constexpr const Type& GetOrExcept() const {
            if (isAllocated())
                return *data;

            throw "Shared is not allocated!";
        }

        // Returns new instance of data if Shared has no reference to it.
        template <typename... Args>
        constexpr Type GetOrNew(Args&&... args) const noexcept {
            if (isAllocated())
                return *data;

            return Type(forward<Args>(args)...);
        }

        // Allocates data if Shared has no reference to it.
        template <typename... Args>
        constexpr Type& GetOrAlloc(Args&&... args) noexcept {
            if (isAllocated())
                return *data;

            return Allocate(forward<Args>(args)...);
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifAlloc, auto&& ifVoid) noexcept {
            if (isAllocated())
                return ifAlloc(*data);
            else
                return ifVoid();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifAlloc, auto&& ifVoid) const noexcept {
            if (isAllocated())
                return ifAlloc(*data);
            else
                return ifVoid();
        }

        constexpr Type& operator *() {
            return GetOrExcept();
        }

        constexpr const Type& operator *() const {
            return GetOrExcept();
        }

        constexpr Type* operator ->() {
            return &GetOrExcept();
        }

        constexpr const Type* operator ->() const {
            return &GetOrExcept();
        }

        constexpr operator Type& () {
            return GetOrExcept();
        }

        constexpr operator const Type& () const {
            return GetOrExcept();
        }
    };

    template <typename Type>
    Shared(Type&&) -> Shared<std::remove_cvref_t<Type>>;

    template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Shared<Type>& shared) noexcept {
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
}