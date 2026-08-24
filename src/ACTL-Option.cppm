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

    template <u32 index, typename Type, typename... Other> requires(!std::is_reference_v<Type>)
    union OptionLeaf {
        using Next = OptionLeaf<index + 1, Other...>;

        Type data;

        Next next;

        template <typename Target>
        static consteval u8 getIndex() noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return index;
            else
                return Next::template getIndex<Target>();
        }

        constexpr OptionLeaf() noexcept : next() {};

        template <typename Target> requires(std::is_same_v<std::remove_cvref_t<Target>, Type>)
        constexpr OptionLeaf(Target&& value) noexcept : data(ACTL::forward(value)) {};

        template <typename Target>
        constexpr OptionLeaf(Target&& value) noexcept : next(ACTL::forward(value)) {};

        constexpr ~OptionLeaf() noexcept {};

        constexpr void Destruct(u8 targetIndex) noexcept {
            if (index == targetIndex)
                data.~Type();
            else
                next.Destruct(targetIndex);
        }

        template <typename Target>
        constexpr Target& get() noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return data;
            else
                return next.template get<Target>();
        }

        template <typename Target>
        constexpr const Target& get() const noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return data;
            else
                return next.template get<Target>();
        }

        template <typename Return>
        constexpr Return Visit(u8 targetIndex, auto&& func, auto&&... other) noexcept {
            if (targetIndex == index)
                return func(data);
            else
                return next.template Visit<Return>(targetIndex, ACTL::forward(other)...);
        }

        template <typename Return>
        constexpr Return Visit(u8 targetIndex, auto&& func, auto&&... other) const noexcept {
            if (targetIndex == index)
                return func(data);
            else
                return next.template Visit<Return>(targetIndex, ACTL::forward(other)...);
        }
    };

    template <u32 index, typename Type> requires(!std::is_reference_v<Type>)
    union OptionLeaf<index, Type> {
        Type data;

        Void dummy;

        template <typename Target>
        static consteval u8 getIndex() noexcept {
            if constexpr (std::is_same_v<Type, Target>)
                return index;
            else
                return u8max;
        }

        constexpr OptionLeaf() noexcept : dummy() {};

        template <typename Target> requires(std::is_same_v<std::remove_cvref_t<Target>, Type>)
        constexpr OptionLeaf(Target&& value) noexcept : data(ACTL::forward(value)) {};

        template <typename Target>
        constexpr OptionLeaf(Target&&) {
            static_assert(false, "Invalid type!");
        }

        constexpr ~OptionLeaf() noexcept {};

        constexpr void Destruct(u8 targetIndex) noexcept {
            data.~Type();
        }

        template <typename Target>
        constexpr Target& get() noexcept {
            return data;
        }

        template <typename Target>
        constexpr const Target& get() const noexcept {
            return data;
        }

        template <typename Return>
        constexpr Return Visit(u8 targetIndex, auto&& func) noexcept {
            return func(data);
        }

        template <typename Return>
        constexpr Return Visit(u8 targetIndex, auto&& func) const noexcept {
            return func(data);
        }
    };

    export template <typename... Types>
    class Option {
        using Leaf = OptionLeaf<0, Types...>;

        Leaf leaf;

        u8 index;

        template <typename Type>
        static consteval u8 getIndex() noexcept {
            return Leaf::template getIndex<std::remove_cvref_t<Type>>();
        }

        template <typename Type>
        static consteval bool has() noexcept {
            return getIndex<std::remove_cvref_t<Type>>() != u8max;
        }

        constexpr void Destruct() noexcept {
            leaf.Destruct(index);
        }

    public:
        constexpr Option() noexcept : leaf(Void()), index(getIndex<Void>()) {};

        template <typename Type> requires(has<Type>())
        constexpr Option(Type&& value) noexcept : leaf(ACTL::forward(value)), index(getIndex<Type>()) {};

        constexpr Option(const Option& other) noexcept : leaf(), index(other.index) {
            other.Visit([&](const Types& value) {
                ACTL::Construct(&leaf, value);
            }...);
        }

        constexpr Option(Option&& other) noexcept : leaf(), index(other.index) {
            other.Visit([&](Types& value) {
                ACTL::Construct(&leaf, ACTL::move(value));
            }...);
        }

        constexpr ~Option() noexcept {
            Destruct();
        }

        constexpr Option& operator =(const Option& other) noexcept {
            other.Visit([&](const Types& value) {
                if (index == other.index)
                    leaf.template get<Types>() = value;
                else
                    Set<Types>(value);
            }...);

            return *this;
        }

        constexpr Option& operator =(Option&& other) noexcept {
            other.Visit([&](Types& value) {
                if (index == other.index)
                    leaf.template get<Types>() = ACTL::move(value);
                else
                    Set<Types>(ACTL::move(value));
            }...);

            return *this;
        }

        template <typename Type, typename... Args> requires(has<Type>())
        constexpr Type& Set(Args&&... args) noexcept {
            Destruct();

            ACTL::Construct(&leaf, Type(ACTL::forward<Args>(args)...));

            index = getIndex<Type>();

            return leaf.template get<Type>();
        }

        constexpr void Clear() noexcept {
            Set<Void>();
        }

        template <typename Type> requires(has<Type>())
        constexpr bool is() const noexcept {
            return getIndex<Type>() == index;
        }

        template <typename Type> requires(has<Type>())
        constexpr Type& GetOrExcept() {
            if (is<Type>())
                return leaf.template get<Type>();

            throw "This type is not initialized!";
        }

        template <typename Type> requires(has<Type>())
        constexpr const Type& GetOrExcept() const {
            if (is<Type>())
                return leaf.template get<Type>();

            throw "This type is not initialized!";
        }

        template <typename Type, typename... Args> requires(has<Type>())
        constexpr Type GetOrNew(Args&&... args) const noexcept {
            if (is<Type>())
                return leaf.template get<Type>();

            return Type(ACTL::forward<Args>(args)...);
        }

        template <typename Type, typename... Args> requires(has<Type>())
        constexpr Type& GetOrSet(Args&&... args) noexcept {
            if (is<Type>())
                return leaf.template get<Type>();

            return Set<Type>(ACTL::forward<Args>(args)...);
        }

        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) noexcept {
            return leaf.template Visit<Return>(index, ACTL::forward<Funcs>(funcs)...);
        }

        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) const noexcept {
            return leaf.template Visit<Return>(index, ACTL::forward<Funcs>(funcs)...);
        }
    };

    export template <typename Char, typename... Types>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Option<Types...>& option) noexcept {
        option.Visit([&](const Types& value) {
            ostream << value;
        }...);

        return ostream;
    }

    export template <typename Type>
    class Option<Type> {
    public:
        Option<Void, Type> data = {};

        constexpr Option() noexcept {};

        template <typename... Args>
        constexpr Option(Args&&... args) noexcept {
            Set(ACTL::forward<Args>(args)...);
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
        constexpr Type& Set(Args&&... args) noexcept {
            return data.template Set<Type>(ACTL::forward<Args>(args)...);
        }

        constexpr void Clear() noexcept {
            return data.Clear();
        }

        constexpr bool isCreated() const noexcept {
            return data.template is<Type>();
        }

        constexpr operator bool() const noexcept {
            return isCreated();
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

        template <typename Return = void>
        constexpr Return Visit(auto&& ifVoid, auto&& ifData) noexcept {
            return data.template Visit<Return>(
                ACTL::forward(ifVoid),
                ACTL::forward(ifData)
            );
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifVoid, auto&& ifData) const noexcept {
            return data.template Visit<Return>(
                ACTL::forward(ifVoid),
                ACTL::forward(ifData)
            );
        }

        constexpr Type& operator *() noexcept {
            return GetOrExcept();
        }

        constexpr const Type& operator *() const noexcept {
            return GetOrExcept();
        }

        constexpr Type* operator ->() noexcept {
            return &GetOrExcept();
        }

        constexpr const Type* operator ->() const noexcept {
            return &GetOrExcept();
        }

        constexpr operator Type& () noexcept {
            return GetOrExcept();
        }

        constexpr operator const Type& () const noexcept {
            return GetOrExcept();
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Option<Type>& option) noexcept {
        option.Visit(
            [&](Void) {
                ostream << Void();
            },
            [&](const Type& value) {
                ostream << value;
            }
        );

        return ostream;
    }

    export template <typename SuccessType, typename FailType = Void>
    class Result {
        Option<SuccessType, FailType> option;

        constexpr Result(FailType&& fail) noexcept : option(forward(fail)) {};

    public:
        template <typename... Args>
        static constexpr Result Fail(Args&&... args) noexcept {
            return Result(FailType(forward<Args>(args)...));
        }

        constexpr Result(SuccessType&& success) noexcept : option(forward(success)) {};

        constexpr Result(const Result& other) noexcept : option(other.option) {};

        constexpr Result(Result&& other) noexcept : option(move(other.option)) {};

        constexpr ~Result() noexcept {};

        constexpr Result& operator =(const Result& other) noexcept {
            option = other.option;

            return *this;
        }

        constexpr Result& operator =(Result&& other) noexcept {
            option = move(other.option);

            return *this;
        }

        template <typename... Args>
        constexpr SuccessType& Set(Args&&... args) noexcept {
            return option.template Set<SuccessType>(forward<Args>(args)...);
        }

        template <typename... Args>
        constexpr FailType& SetFail(Args&&... args) noexcept {
            return option.template Set<FailType>(forward<Args>(args)...);
        }

        constexpr bool isSuccess() const noexcept {
            return option.template is<SuccessType>();
        }

        constexpr operator bool() const noexcept {
            return isSuccess();
        }

        constexpr SuccessType& Get() {
            return option.template GetOrExcept<SuccessType>();
        }

        constexpr const SuccessType& Get() const {
            return option.template GetOrExcept<SuccessType>();
        }

        constexpr FailType& GetFail() {
            return option.template GetOrExcept<FailType>();
        }

        constexpr const FailType& GetFail() const {
            return option.template GetOrExcept<FailType>();
        }

        constexpr SuccessType& operator *() {
            return Get();
        }

        constexpr const SuccessType& operator *() const {
            return Get();
        }

        constexpr SuccessType* operator ->() {
            return &Get();
        }

        constexpr const SuccessType* operator ->() const {
            return &Get();
        }

        constexpr operator SuccessType& () {
            return Get();
        }

        constexpr operator const SuccessType& () const {
            return Get();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifSuccess, auto&& ifFail) noexcept {
            return option.template Visit<Return>(
                forward(ifSuccess),
                forward(ifFail)
            );
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& ifSuccess, auto&& ifFail) const noexcept {
            return option.template Visit<Return>(
                forward(ifSuccess),
                forward(ifFail)
            );
        }
    };

    export template <typename Char, typename Success, typename Fail>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Result<Success, Fail>& result) noexcept {
        result.Visit(
            [&](const Success& success) {
                ostream << success;
            },
            [&](const Fail& fail) {
                ostream << fail;
            }
        );

        return ostream;
    }
}