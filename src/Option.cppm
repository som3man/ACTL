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
#include <type_traits>

export module ACTL:Option;

import :Defines;
import :String;

namespace ACTL {
    template <u32 index, typename Type, typename... Other>
    union OptionLeaf {
        Type data;

        using Next = OptionLeaf<index + 1, Other...>;

        Next next;

        template <typename Target>
        static consteval u8 GetIndex() noexcept {
            if constexpr (std::is_same_v<Target, Type>)
                return index;
            else
                return Next::template GetIndex<Target>();
        }

        template <typename Target> requires(std::is_same_v<Target, Type>)
        constexpr OptionLeaf(Target&& target) noexcept(std::is_nothrow_constructible_v<Target, Target&&>) : data(Forward(target)) {};

        template <typename Target>
        constexpr OptionLeaf(Target&& target) noexcept(std::is_nothrow_constructible_v<Target, Target&&>) : next(Forward(target)) {};

        constexpr OptionLeaf() noexcept : next() {};

        constexpr ~OptionLeaf() noexcept {};

        static consteval bool NoThrowDestructible() noexcept {
            if constexpr (std::is_nothrow_destructible_v<Type>)
                return Next::NoThrowDestructible();
            else
                return false;
        }

        constexpr void Destruct(u8 targetIndex) noexcept(NoThrowDestructible()) {
            if (targetIndex == index)
                data.~Type();
            else
                next.Destruct(targetIndex);
        }

        template <typename Return, typename Func, typename... Funcs>
        constexpr Return Visit(u8 targetIndex, Func&& func, Funcs&&... funcs) noexcept {
            if (targetIndex == index)
                return func(data);
            else
                return next.template Visit<Return>(targetIndex, Forward<Funcs>(funcs)...);
        }

        template <typename Return, typename Func, typename... Funcs>
        constexpr Return Visit(u8 targetIndex, Func&& func, Funcs&&... funcs) const noexcept {
            if (targetIndex == index)
                return func(data);
            else
                return next.template Visit<Return>(targetIndex, Forward<Funcs>(funcs)...);
        }

        template <typename Target>
        constexpr Target& Get() noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return data;
            else
                return next.template Get<Target>();
        }

        template <typename Target>
        constexpr const Target& Get() const noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return data;
            else
                return next.template Get<Target>();
        }
    };

    template <u32 index, typename Type>
    union OptionLeaf<index, Type> {
        Type data;

        struct {} none;

        template <typename Target>
        static consteval u8 GetIndex() noexcept {
            if constexpr (std::is_same_v<Target, Type>)
                return index;
            else
                return 255;
        }

        template <typename Target> requires(std::is_same_v<Target, Type>)
        constexpr OptionLeaf(Target&& target) noexcept(std::is_nothrow_constructible_v<Target, Target&&>) : data(Forward(target)) {};

        constexpr OptionLeaf() noexcept : none() {};

        constexpr ~OptionLeaf() noexcept {};

        static consteval bool NoThrowDestructible() noexcept {
            return std::is_nothrow_destructible_v<Type>;
        }

        constexpr void Destruct(u8 targetIndex) noexcept(NoThrowDestructible()) {
            if (targetIndex == index)
                data.~Type();
        }

        template <typename Return, typename Func>
        constexpr Return Visit(u8 targetIndex, Func&& func) noexcept {
            return func(data);
        }

        template <typename Return, typename Func>
        constexpr Return Visit(u8 targetIndex, Func&& func) const noexcept {
            return func(data);
        }

        template <typename Target>
        constexpr Target& Get() noexcept {
            return data;
        }

        template <typename Target>
        constexpr const Target& Get() const noexcept {
            return data;
        }
    };

    // Null state abstraction.
    export struct Void {};

    export std::ostream& operator <<(std::ostream& ostream, Void) noexcept {
        return ostream << "void";
    }

    export std::wostream& operator <<(std::wostream& ostream, Void) noexcept {
        return ostream << L"void";
    }

    export constexpr String& operator <<(String& string, Void) noexcept {
        return string << "void"_str;
    }

    // Type safe union implementation.
    export template <typename... Types>
    class Option {
        using Leaf = OptionLeaf<0, Types...>;

        Leaf leaf;

        u8 typeIndex;

        template <typename Target>
        static consteval u8 GetIndex() noexcept {
            return Leaf::template GetIndex<Target>();
        }

        static consteval bool NoThrowDestructible() noexcept {
            return Leaf::NoThrowDestructible();
        }

        constexpr void Destruct() noexcept {
            return leaf.Destruct(typeIndex);
        }

    public:
        // Returns true if Option has Target in its type pack.
        template <typename Target>
        static consteval bool Has() noexcept {
            return GetIndex<Target>() != 255;
        }

        template <typename Type> requires(Has<Type>())
        constexpr Option(Type&& value) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Type>, Type&&>) : leaf(Forward(value)) {
            typeIndex = GetIndex<std::remove_cvref_t<Type>>();
        }

        constexpr Option(const Option& other) noexcept : leaf(), typeIndex(255) {
            other.Visit([&](const Types& value) {
                Set<Types>(value);
            }...);
        }

        constexpr Option(Option&& other) noexcept : leaf(), typeIndex(255) {
            other.Visit([&](Types& value) {
                Set<Types>(Move(value));
            }...);
        }

        constexpr ~Option() noexcept(NoThrowDestructible()) {
            Destruct();
        }

        constexpr Option& operator =(const Option& other) noexcept {
            if (this == &other)
                return *this;

            other.Visit([&](const Types& value) {
                if (is<Types>())
                    leaf.template Get<Types>() = value;
                else
                    Set<Types>(value);
            }...);

            return *this;
        }

        constexpr Option& operator =(Option&& other) noexcept {
            if (this == &other)
                return *this;

            other.Visit([&](Types& value) {
                if (is<Types>())
                    leaf.template Get<Types>() = Move(value);
                else
                    Set<Types>(Move(value));
            }...);

            return *this;
        }

        template <typename Type, typename... Args> requires(Has<Type>())
        constexpr Type& Set(Args&&... args) noexcept(NoThrowDestructible() && std::is_nothrow_constructible_v<Type, Args...>) {
            Destruct();

            leaf.~Leaf();

            std::construct_at(&leaf, Type(Forward<Args>(args)...));

            typeIndex = GetIndex<Type>();

            return leaf.template Get<Type>();
        }

        template <typename Type> requires(Has<Type>())
        constexpr Type& Get() {
            if (is<Type>())
                return leaf.template Get<Type>();

            throw "Type is not initialized!";
        }

        template <typename Type> requires(Has<Type>())
        constexpr const Type& Get() const {
            if (is<Type>())
                return leaf.template Get<Type>();

            throw "Type is not initialized!";
        }

        template <typename Type, typename... Args> requires(Has<Type>() && std::is_constructible_v<Type, const Type&>)
        constexpr Type GetOr(Args&&... args) const noexcept(std::is_nothrow_constructible_v<Type, Args...> && std::is_nothrow_constructible_v<Type, const Type&>) {
            if (is<Type>())
                return leaf.template Get<Type>();

            return Type(Forward<Args>(args)...);
        }

        template <typename Type> requires(Has<Type>())
        constexpr bool is() const noexcept {
            return GetIndex<Type>() == typeIndex;
        }

        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) noexcept {
            return leaf.template Visit<Return>(typeIndex, Forward<Funcs>(funcs)...);
        }

        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) const noexcept {
            return leaf.template Visit<Return>(typeIndex, Forward<Funcs>(funcs)...);
        }
    };

    export template <typename... Types>
    std::ostream& operator <<(std::ostream& ostream, const Option<Types...>& option) noexcept {
        option.Visit([&](const Types& value) {
            ostream << value;
        }...);

        return ostream;
    }

    export template <typename... Types>
    std::wostream& operator <<(std::wostream& ostream, const Option<Types...>& option) noexcept {
        option.Visit([&](const Types& value) {
            ostream << value;
        }...);

        return ostream;
    }

    export template <typename... Types>
    constexpr String& operator <<(String& string, const Option<Types...>& option) noexcept {
        option.Visit([&](const Types& value) {
            string << value;
        }...);

        return string;
    }

    // Single type specialization of Option.
    // Acts as Option<Type, Void>.
    export template <typename Type>
    class Option<Type> {
        using Data = Option<Type, Void>;

        Data data;

    public:
        constexpr Option(Void) noexcept : data(Void()) {};

        template <typename... Args> requires(std::is_constructible_v<Type, Args...>)
        constexpr Option(Args&&... args) noexcept(std::is_nothrow_constructible_v<Type, Args...>) : data(Type(Forward(args)...)) {};

        constexpr Option(const Option& other) noexcept(std::is_nothrow_constructible_v<Data, const Data&>) : data(other.data) {};

        constexpr Option(Option&& other) noexcept(std::is_nothrow_constructible_v<Data, Data&&>) : data(Move(other.data)) {};

        constexpr ~Option() noexcept {};

        constexpr Option& operator =(const Option& other) noexcept(std::is_nothrow_assignable_v<Data, const Data&>) {
            data = other.data;

            return *this;
        }

        constexpr Option& operator =(Option&& other) noexcept(std::is_nothrow_assignable_v<Data, Data&&>) {
            data = Move(other.data);

            return *this;
        }

        constexpr bool notVoid() const noexcept {
            return data.template is<Type>();
        }

        constexpr bool isVoid() const noexcept {
            return data.template is<Void>();
        }

        constexpr operator bool() const noexcept {
            return notVoid();
        }

        constexpr Type& Get() {
            return data.template Get<Type>();
        }

        constexpr const Type& Get() const {
            return data.template Get<Type>();
        }

        template <typename... Args> requires(std::is_constructible_v<Type, const Type&>)
        constexpr Type GetOr(Args&&... args) const noexcept(std::is_nothrow_constructible_v<Type, Args...> && std::is_nothrow_constructible_v<Type, const Type&>) {
            return data.template GetOr<Type>(Forward<Args>(args)...);
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifData, auto&& ifVoid) noexcept {
            return data.template Visit<Return>(
                Forward(ifData),
                [&](const Void&) {
                    return ifVoid();
                }
            );
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifData, auto&& ifVoid) const noexcept {
            return data.template Visit<Return>(
                Forward(ifData),
                [&](const Void&) {
                    return ifVoid();
                }
            );
        }

        constexpr Type& operator *() {
            return Get();
        }

        constexpr const Type& operator *() const {
            return Get();
        }

        constexpr Type* operator ->() {
            return &Get();
        }

        constexpr const Type* operator ->() const {
            return &Get();
        }

        constexpr operator Type& () {
            return Get();
        }

        constexpr operator const Type& () const {
            return Get();
        }
    };

    export template <typename Type>
    std::ostream& operator <<(std::ostream& ostream, const Option<Type>& option) noexcept {
        option.Visit(
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
    std::wostream& operator <<(std::wostream& ostream, const Option<Type>& option) noexcept {
        option.Visit(
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
    constexpr String& operator <<(String& string, const Option<Type>& option) noexcept {
        option.Visit(
            [&](const Type& value) {
                string << value;
            },
            [&]() {
                string << Void();
            }
        );

        return string;
    }

    consteval bool SingleOptionTest() {
        u32 counter = 0;

        Option<Class> option = Void();

        option = Class(123, counter);

        if (counter != 1)
            return false;

        if (!option)
            return false;

        if (option->data != 123)
            return false;

        option = Void();

        if (counter != 0)
            return false;

        if (option)
            return false;

        return true;
    }

    static_assert(SingleOptionTest(), "Single Option test is FAILED!");

    template <u32>
    class Tclass {
    public:
        u32& counter;

        i32 value;

        constexpr Tclass(u32& counter, i32 value) : counter(counter), value(value) {
            counter++;
        }

        constexpr Tclass(const Tclass& other) : counter(other.counter), value(other.value) {
            counter++;
        }

        constexpr Tclass(Tclass&& other) : counter(other.counter), value(other.value) {
            counter++;
        }

        constexpr ~Tclass() {
            counter--;
        }

        constexpr Tclass& operator =(const Tclass& other) {
            value = other.value;

            return *this;
        }

        constexpr Tclass& operator =(Tclass&& other) {
            value = other.value;

            return *this;
        }
    };

    consteval bool OptionTest() {
        u32 counters[3] = {};

        Option<Tclass<0>, Tclass<1>, Tclass<2>> option = Tclass<0>(counters[0], 10);

        static_assert(sizeof(Tclass<0>) + 8 == sizeof(Option<Tclass<0>, Tclass<1>, Tclass<2>>));

        if (counters[0] != 1)
            return false;

        if (option.Get<Tclass<0>>().value != 10)
            return false;

        option = Tclass<2>(counters[2], 56);

        if (counters[0] != 0)
            return false;

        if (counters[2] != 1)
            return false;

        if (option.Get<Tclass<2>>().value != 56)
            return false;

        if (option.GetOr<Tclass<1>>(counters[1], -6).value != -6)
            return false;

        return true;
    }

    static_assert(OptionTest(), "Option test is FAILED!");
}