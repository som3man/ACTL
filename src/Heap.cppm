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

export module ACTL:Heap;

import :Defines;
import :Traits;
import :String;
import :Option;

namespace ACTL {
    // Unique heap memory owner.
    export template <typename Type> requires(!isReference<Type> && !areSame<Void, Type>)
    class Heap {
        Type* data = nullptr;

    public:
        template <typename Other> requires(!isReference<Other> && !areSame<Void, Other>)
        friend class Heap;

        template <typename Derived> requires(isBaseOf<Type, Derived> && hasVirtualDestructor<Type>)
        constexpr Heap<Derived> DownCast() noexcept {
            Heap<Derived> result = Void();

            result.data = dynamic_cast<Derived*>(data);

            if (result)
                data = nullptr;

            return result;
        }

        // Null state initializer. Can be constinit.
        constexpr Heap(Void) noexcept {};

        // Allocates memory.
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Heap(Args&&... args) noexcept(hasNoexceptConstructor<Type, Args...>) {
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

        // Transfers memory ownership from other.
        template <typename Derived> requires(isBaseOf<Type, Derived> && hasVirtualDestructor<Type>)
        constexpr Heap(Heap<Derived>&& other) noexcept {
            operator=(Move(other));
        }

        // Frees memory.
        constexpr ~Heap() noexcept(hasNoexceptDestructor<Type>) {
            Free();
        }

        // Copies state and data of other.
        constexpr Heap& operator =(const Heap& other) noexcept(hasNoexceptDestructor<Type> && hasNoexceptConstructor<Type, const Heap&>) {
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
        constexpr Heap& operator =(Heap&& other) noexcept(hasNoexceptDestructor<Type>) {
            Free();

            data = other.data;

            other.data = nullptr;

            return *this;
        }

        // Transfers memory ownership from other.
        template <typename Derived> requires(isBaseOf<Type, Derived> && hasVirtualDestructor<Type>)
        constexpr Heap& operator =(Heap<Derived>&& other) noexcept(hasNoexceptDestructor<Type>) {
            Free();

            data = static_cast<Type*>(other.data);

            other.data = nullptr;

            return *this;
        }

        // Allocates memory.
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type& Allocate(Args&&... args) noexcept(hasNoexceptDestructor<Type> && hasNoexceptConstructor<Type, Args...>) {
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
        template <typename Derived, typename... Args> requires(hasConstructor<Derived, Args...> && isBaseOf<Type, Derived> && hasVirtualDestructor<Type>)
        constexpr Derived& AllocateChild(Args&&... args) noexcept(hasNoexceptDestructor<Type> && hasNoexceptConstructor<Derived, Args...>) {
            Free();

            std::allocator<Derived> allocator = {};

            auto p = allocator.allocate(1);

            std::construct_at(p, Forward<Args>(args)...);

            data = static_cast<Type*>(p);

            return *p;
        }

        // Frees memory.
        constexpr void Free() noexcept(hasNoexceptDestructor<Type>) {
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

        template <typename Return = void, typename IfData, typename IfVoid> requires(isInvocable<Return, IfData, Type&> && isInvocable<Return, IfVoid>)
        constexpr Return Visit(IfData&& ifData, IfVoid&& ifVoid) noexcept(isNoexceptInvocable<Return, IfData, Type&> && isNoexceptInvocable<Return, IfVoid>) {
            if (data)
                return ifData(*data);
            else
                return ifVoid();
        }

        template <typename Return = void, typename IfData, typename IfVoid> requires(isInvocable<Return, IfData, const Type&> && isInvocable<Return, IfVoid>)
        constexpr Return Visit(IfData&& ifData, IfVoid&& ifVoid) const noexcept(isNoexceptInvocable<Return, IfData, const Type&> && isNoexceptInvocable<Return, IfVoid>) {
            if (data)
                return ifData(*(static_cast<const Type*>(data)));
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
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type GetOr(Args&&... args) const noexcept(hasNoexceptConstructor<Type, const Type&> && hasNoexceptConstructor<Type, Args...>) {
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
    export template <typename Type> requires(!isReference<Type> && !areSame<Void, Type>)
    class Shared {
        struct Cont {
            Type data;

            u32 counter;

            template <typename... Args>
            constexpr Cont(Args&&... args) noexcept(hasNoexceptConstructor<Type, Args...>) : data(Forward<Args>(args)...) {
                counter = 1;
            }

            constexpr ~Cont() noexcept(hasNoexceptDestructor<Type>) {};
        };
        
        Cont* cont = nullptr;

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
        constexpr Shared& operator =(const Shared& other) noexcept(hasNoexceptDestructor<Type>) {
            Drop();

            cont = other.cont;

            if (cont)
                cont->counter += 1;

            return *this;
        }

        // Transfers referencing state.
        constexpr Shared& operator =(Shared&& other) noexcept(hasNoexceptDestructor<Type>) {
            Drop();

            cont = other.cont;

            other.cont = nullptr;

            return *this;
        }

        // Allocates new data. Shared will reference to it.
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type& Allocate(Args&&... args) noexcept(hasNoexceptDestructor<Type> && hasNoexceptConstructor<Type, Args...>) {
            Drop();

            std::allocator<Cont> alloc = {};

            cont = alloc.allocate(1);

            std::construct_at(cont, Forward<Args>(args)...);

            return cont->data;
        }

        // Drops referencing. Referencing data is deleted if it has no references.
        constexpr void Drop() noexcept(hasNoexceptDestructor<Type>) {
            if (!cont)
                return;

            cont->counter -= 1;

            if (cont->counter)
                return;

            cont->~Cont();

            std::allocator<Cont> alloc = {};

            alloc.deallocate(cont, 1);

            cont = nullptr;
        }

        constexpr bool notVoid() const noexcept {
            return cont;
        }

        constexpr bool isVoid() const noexcept {
            return !notVoid();
        }

        constexpr operator bool() const noexcept {
            return notVoid();
        }

        // Throws exception if Shared has no reference.
        constexpr Type& Get() const {
            if (cont)
                return cont->data;

            throw "Shared is void!";
        }

        // Returns new instance if Shared has no reference.
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type GetOr(Args&&... args) const noexcept(hasNoexceptConstructor<Type, const Type&> && hasNoexceptConstructor<Type, Args...>) {
            if (cont)
                return cont->data;

            return Type(Forward<Args>(args)...);
        }

        template <typename Return = void, typename IfData, typename IfVoid> requires(isInvocable<Return, IfData, Type&> && isInvocable<Return, IfVoid>)
        constexpr Return Visit(IfData&& ifData, IfVoid&& ifVoid) const noexcept(hasNoexceptConstructor<Return, IfData, Type&> && hasNoexceptConstructor<Return, IfVoid>) {
            if (cont)
                return ifData(cont->data);
            else
                return ifVoid();
        }

        // Returns count of references to current data referenced by Shared including itself.
        // Returns 0 if Shared doesn`t reference any data.
        constexpr u32 getRefCount() const noexcept {
            return cont ? cont->counter : 0;
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
            return cont == other.cont;
        }

        // Compares referenced data pointers.
        constexpr auto operator <=>(const Shared& other) const noexcept {
            return cont <=> other.cont;
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