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

#include <bit>
#include <cstddef>
#include <memory>
#include <ostream>
#include <type_traits>

export module ACTL:Option;

import :Defines;
import :Utility;
import :Allocation;
import :String;

namespace ACTL {
    // Type for null-state representation in Option.
    export struct Void {};

    export template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Void&) noexcept {
        return ostream << Char('v') << Char('o') << Char('i') << Char('d');
    }

    export template <typename Char>
    constexpr String<Char>& operator <<(String<Char>& string, const Void&) noexcept {
        return string << Char('v') << Char('o') << Char('i') << Char('d');
    }

    // Unique pointer implementation.
    export template <typename Type>
    class Heap {
        Type* pointer = nullptr;

    public:
        constexpr Heap() noexcept {};

        template <typename... Args>
        constexpr Heap(Args&&... args) noexcept {
            Allocate(ACTL::forward<Args>(args)...);
        }

        constexpr Heap(const Heap& other) noexcept {
            operator=(other);
        }

        constexpr Heap(Heap&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Heap() noexcept {
            Free();
        }

        // Assigns instances or allocates new.
        // Does nothing if both heaps have no instances.
        constexpr Heap& operator =(const Heap& other) noexcept {
            if (pointer && other.pointer)
                *pointer = *other.pointer;
            else if (pointer)
                Free();
            else if (other.pointer)
                Allocate(*other.pointer);

            return *this;
        }

        // Moves pointer to instance memory.
        constexpr Heap& operator =(Heap&& other) noexcept {
            Free();

            pointer = other.pointer;

            other.pointer = nullptr;

            return *this;
        }

        // Allocates and constructs instance.
        // Frees previous instance.
        template <typename... Args>
        Type& Allocate(Args&&... args) noexcept {
            Free();

            pointer = ACTL::Allocate<Type>();

            std::construct_at<Type>(pointer, ACTL::forward<Args>(args)...);

            return *pointer;
        }

        // Frees and destructs instance.
        // Does nothing if it has no instance.
        void Free() noexcept {
            if (pointer) {
                pointer->~Type();

                ACTL::Free(pointer);

                pointer = nullptr;
            }
        }

        constexpr bool isAllocated() const noexcept {
            return pointer;
        }

        constexpr Type& GetOrExcept() {
            if (pointer)
                return *pointer;

            throw "Heap is not allocated!";
        }

        constexpr const Type& GetOrExcept() const {
            if (pointer)
                return *pointer;

            throw "Heap is not allocated!";
        }

        constexpr Type* operator ->() {
            return &GetOrExcept();
        }

        constexpr const Type* operator ->() const {
            return &GetOrExcept();
        }

        constexpr Type& operator *() {
            return GetOrExcept();
        }

        constexpr const Type& operator *() const {
            return GetOrExcept();
        }

        constexpr operator Type& () {
            return GetOrExcept();
        }

        constexpr operator const Type& () const {
            return GetOrExcept();
        }

        constexpr bool operator ==(std::nullptr_t) const noexcept {
            return pointer == nullptr;
        }

        constexpr bool operator !=(std::nullptr_t) const noexcept {
            return pointer != nullptr;
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forVoid, auto&& forType) noexcept {
            if (pointer)
                return forType(*pointer);
            else
                return forVoid();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forVoid, auto&& forType) const noexcept {
            if (pointer)
                return forType(*pointer);
            else
                return forVoid();
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Heap<Type>& heap) noexcept {
        if (heap.isAllocated())
            return ostream << heap.GetOrExcept();
        else
            return ostream << Void();
    }

    export template <typename Char, typename Type>
    constexpr String<Char>& operator <<(String<Char>& string, const Heap<Type>& heap) noexcept {
        if (heap.isAllocated())
            return string << heap.GetOrExcept();
        else
            return string << Void();
    }

    template <typename Type>
    struct IsHeap : std::false_type {};

    template <typename... Args>
    struct IsHeap<Heap<Args...>> : std::true_type {};

    export template <typename Type>
    constexpr bool isHeap = IsHeap<Type>::value;

    // Type-safe union implementation.
    // Option can have null state if Void is stored type.
    export template <typename... Types>
    class Option {
        template <typename Type, typename... Other>
        static consteval size GetMaxSize(size prev) noexcept {
            size curr = sizeof(Type);

            size next = curr > prev ? curr : prev;

            if constexpr (sizeof...(Other))
                return GetMaxSize<Other...>(next);
            
            return next;
        }

        template <typename Type, typename... Other>
        static consteval size GetMaxAlign(size prev) noexcept {
            size curr = alignof(Type);

            size next = curr > prev ? curr : prev;

            if constexpr (sizeof...(Other))
                return GetMaxAlign<Other...>(next);
            
            return next;
        }

        alignas(GetMaxAlign<Types...>(1)) byte data[GetMaxSize<Types...>(1)] = {};

        template <typename Target, typename Type, typename... Other>
        static consteval byte GetIndex(byte index) noexcept {
            if constexpr (std::is_same_v<Target, Type>)
                return index;

            if constexpr (sizeof...(Other))
                return GetIndex<Target, Other...>(index + 1);

            return u8max;
        }

        byte typeIndex = GetIndex<Void, Types...>(0);

        template <typename Type>
        constexpr Type* Cast() noexcept {
            return std::bit_cast<Type*, byte*>(data);
        }

        template <typename Type>
        constexpr const Type* Cast() const noexcept {
            return std::bit_cast<const Type*, const byte*>(data);
        }

        template <byte index, typename Type, typename... Other>
        constexpr void Destruct() noexcept {
            if (typeIndex != index) {
                if constexpr (sizeof...(Other))
                    Destruct<index + 1, Other...>();

                return;
            }

            if constexpr (!std::is_trivially_destructible_v<Type>)
                Cast<Type>()->~Type();
        }

        template <byte index, typename Type, typename... Other>
        constexpr void Copy(const Option& other) noexcept {
            if (index == other.typeIndex) {
                if (typeIndex == index)
                    *Cast<Type>() = *other.Cast<Type>();
                else
                    Set<Type>(*other.Cast<Type>());

                return;
            }

            if constexpr (sizeof...(Other))
                Copy<index + 1, Other...>(other);
        }

        template <byte index, typename Type, typename... Other>
        constexpr void Move(Option&& other) noexcept {
            if (index == other.typeIndex) {
                if constexpr (isHeap<Type> && !hasVoid) {
                    if (typeIndex == index)
                        Cast<Type>()->GetOrExcept() = ACTL::move(other.Cast<Type>()->GetOrExcept());
                    else
                        Set<Type>(ACTL::move(other.Cast<Type>()->GetOrExcept()));
                }
                else {
                    if (typeIndex == index)
                        *Cast<Type>() = ACTL::move(*other.Cast<Type>());
                    else
                        Set<Type>(ACTL::move(*other.Cast<Type>()));
                }

                return;
            }

            if constexpr (sizeof...(Other))
                Move<index + 1, Other...>(ACTL::move(other));
        }

        template <byte index, typename Char, typename Type, typename... Other>
        void ToOstreamImpl(std::basic_ostream<Char>& ostream) const noexcept {
            if (typeIndex == index) {
                ostream << *Cast<Type>();

                return;
            }

            if constexpr (sizeof...(Other))
                ToOstreamImpl<index + 1, Char, Other...>(ostream);
        }

        template <byte index, typename Char, typename Type, typename... Other>
        void ToStringImpl(String<Char>& string) const noexcept {
            if (typeIndex == index) {
                string << *Cast<Type>();

                return;
            }

            if constexpr (sizeof...(Other))
                ToStringImpl<index + 1, Char, Other...>(string);
        }

        template <byte index, typename Return, typename Type, typename... Other>
        constexpr Return VisitImpl(auto&& func, auto&&... other) noexcept {
            if (index == typeIndex) {
                if constexpr (std::is_same_v<Void, Type>)
                    return func();
                else
                    return func(*Cast<Type>());
            }

            if constexpr (sizeof...(Other))
                return VisitImpl<index + 1, Return, Other...>(other...);
            else {
                if constexpr (std::is_same_v<Void, Type>)
                    return func();
                else
                    return func(*Cast<Type>());
            }
        }

        template <byte index, typename Return, typename Type, typename... Other>
        constexpr Return VisitImpl(auto&& func, auto&&... other) const noexcept {
            if (index == typeIndex) {
                if constexpr (std::is_same_v<Void, Type>)
                    return func();
                else
                    return func(*Cast<Type>());
            }

            if constexpr (sizeof...(Other))
                return VisitImpl<index + 1, Return, Other...>(other...);
            else {
                if constexpr (std::is_same_v<Void, Type>)
                    return func();
                else
                    return func(*Cast<Type>());
            }
        }

    public:
        static constexpr bool hasVoid = GetIndex<Void, Types...>(0) != u8max;

        // Requires Void as stored type.
        constexpr Option() noexcept {
            static_assert(hasVoid);
        }

        // Constructs forwarded type.
        // If forwarded Heap is unallocated, sets Void.
        // If Option has no Void, throws exception.
        template <typename Type>
        constexpr Option(Type&& value) {
            using NoRef = std::remove_reference_t<Type>;

            constexpr byte index = GetIndex<NoRef, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            if constexpr (isHeap<NoRef>) {
                if (!value.isAllocated()) {
                    if constexpr (hasVoid)
                        return;
                    else
                        throw "Unallocated heap cannot be assigned to option without void!";
                }
            }

            std::construct_at(Cast<NoRef>(), ACTL::forward(value));

            typeIndex = index;
        }

        constexpr Option(const Option& other) noexcept {
            operator=(other);
        }

        constexpr Option(Option&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Option() noexcept {
            Destruct<0, Types...>();
        }

        constexpr Option& operator =(const Option& other) noexcept {
            Copy<0, Types...>(other);

            return *this;
        }

        constexpr Option& operator =(Option&& other) noexcept {
            Move<0, Types...>(ACTL::move(other));

            return *this;
        }

        // Checks if Type is initialized in Option.
        template <typename Type>
        constexpr bool is() const noexcept {
            constexpr byte index = GetIndex<Type, Types...>(0);

            return typeIndex == index;
        }

        constexpr bool isVoid() const noexcept {
            return is<Void>();
        }

        constexpr bool notVoid() const noexcept {
            return !is<Void>();
        }

        constexpr operator bool() const noexcept {
            return notVoid();
        }

        template <typename Type, typename... Args>
        constexpr auto& Set(Args&&... args) noexcept {
            Destruct<0, Types...>();

            constexpr byte index = GetIndex<Type, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            typeIndex = index;

            if constexpr (std::is_same_v<Void, Type>) {
                return *Cast<Type>();
            }
            else if constexpr (isHeap<Type>) {
                std::construct_at<Type>(Cast<Type>());

                return Cast<Type>()->Allocate(ACTL::forward<Args>(args)...);
            }
            else {
                std::construct_at(Cast<Type>(), ACTL::forward<Args>(args)...);

                return *Cast<Type>();
            }
        }

        template <typename Type>
        constexpr auto& GetOrExcept() {
            static_assert(!std::is_same_v<Type, Void>, "Void cannot be got!");

            constexpr byte index = GetIndex<Type, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            if (index != typeIndex)
                throw "This type is not initialized!";

            if constexpr (isHeap<Type>)
                return Cast<Type>()->GetOrExcept();
            else
                return *Cast<Type>();
        }

        template <typename Type>
        constexpr const auto& GetOrExcept() const {
            static_assert(!std::is_same_v<Type, Void>, "Void cannot be got!");

            constexpr byte index = GetIndex<Type, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            if (index != typeIndex)
                throw "This type is not initialized!";

            if constexpr (isHeap<Type>)
                return Cast<Type>()->GetOrExcept();
            else
                return *Cast<Type>();
        }

        template <typename Type, typename... Args>
        constexpr const auto GetOrNew(Args&&... args) const noexcept {
            static_assert(!std::is_same_v<Type, Void>, "Void cannot be got!");

            constexpr byte index = GetIndex<Type, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            if (index != typeIndex)
                return Type(ACTL::forward<Args>(args)...);

            if constexpr (isHeap<Type>)
                return Cast<Type>()->GetOrExcept();
            else
                return *Cast<Type>();
        }

        template <typename Type, typename... Args>
        constexpr auto& GetOrSet(Args&&... args) noexcept {
            static_assert(!std::is_same_v<Type, Void>, "Void cannot be got!");

            constexpr byte index = GetIndex<Type, Types...>(0);

            static_assert(index != u8max, "Invalid type!");

            if (index != typeIndex)
                return Set<Type>(ACTL::forward<Args>(args)...);

            if constexpr (isHeap<Type>)
                return Cast<Type>()->GetOrExcept();
            else
                return *Cast<Type>();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&&... funcs) noexcept {
            static_assert(sizeof...(funcs) == sizeof...(Types));

            return VisitImpl<0, Return, Types...>(funcs...);
        }

        template <typename Return = void>
        constexpr Return Visit(auto&&... funcs) const noexcept {
            static_assert(sizeof...(funcs) == sizeof...(Types));

            return VisitImpl<0, Return, Types...>(funcs...);
        }

        template <typename Char>
        std::basic_ostream<Char>& ToOstream(std::basic_ostream<Char>& ostream) const noexcept {
            ToOstreamImpl<0, Char, Types...>(ostream);

            return ostream;
        }

        template <typename Char>
        constexpr String<Char>& ToString(String<Char>& string) const noexcept {
            ToStringImpl<0, Char, Types...>(string);

            return string;
        }
    };

    export template <typename Char, typename... Types>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Option<Types...>& option) noexcept {
        return option.ToOstream(ostream);
    }

    export template <typename Char, typename... Types>
    constexpr String<Char>& operator <<(String<Char>& string, const Option<Types...>& option) noexcept {
        return option.ToString(string);
    }

    // Single-type specialization of Option.
    // Works as a regular optional type.
    export template <typename Type>
    class Option<Type> {
    public:
        Option<Void, Type> data = {};

        constexpr Option() noexcept {};

        template <typename... Args>
        constexpr Option(Args&&... args) noexcept {
            Construct(ACTL::forward<Args>(args)...);
        }

        constexpr Option(const Option& other) noexcept {
            operator=(other);
        }

        constexpr Option(Option&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Option() noexcept {};

        constexpr Option& operator =(const Option& other) noexcept {
            data = other.data;

            return *this;
        }

        constexpr Option& operator =(Option&& other) noexcept {
            data = ACTL::move(other.data);

            return *this;
        }

        template <typename... Args>
        constexpr Type& Construct(Args&&... args) noexcept {
            return data.template Set<Type>(ACTL::forward<Args>(args)...);
        }

        constexpr void Destruct() noexcept {
            data.template Set<Void>();
        }

        constexpr bool isConstructed() const noexcept {
            return data.notVoid();
        }

        constexpr Type& GetOrExcept() {
            return data.template GetOrExcept<Type>();
        }

        constexpr const Type& GetOrExcept() const {
            return data.template GetOrExcept<Type>();
        }

        template <typename... Args>
        constexpr Type GetOrNew(Args&&... args) const noexcept {
            return data.template GetOrNew<Type>(ACTL::forward<Args>(args)...);
        }

        template <typename... Args>
        constexpr Type& GetOrSet(Args&&... args) noexcept {
            return data.template GetOrSet<Type>(ACTL::forward<Args>(args)...);
        }

        constexpr Type* operator ->() {
            return &GetOrExcept();
        }

        constexpr const Type* operator ->() const {
            return &GetOrExcept();
        }

        constexpr Type& operator *() {
            return GetOrExcept();
        }

        constexpr const Type& operator *() const {
            return GetOrExcept();
        }

        constexpr operator Type& () {
            return GetOrExcept();
        }

        constexpr operator const Type& () const {
            return GetOrExcept();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forVoid, auto&& forType) noexcept {
            return data.template Visit<Return>(ACTL::forward(forVoid), ACTL::forward(forType));
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forVoid, auto&& forType) const noexcept {
            return data.template Visit<Return>(ACTL::forward(forVoid), ACTL::forward(forType));
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Option<Type>& option) noexcept {
        return ostream << option.data;
    }

    export template <typename Char, typename Type>
    constexpr String<Char>& operator <<(String<Char>& string, const Option<Type>& option) noexcept {
        return string << option.data;
    }
}