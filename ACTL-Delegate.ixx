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

#include <cstddef>
#include <type_traits>

export module ACTL:Delegate;

import :Defines;
import :Utility;
import :Allocation;

export namespace ACTL {
    template <typename Return, typename... Args>
    class Delegate;

    template <typename Return, typename... Args>
    class Delegate<Return(Args...)> {
        class Interface {
        public:
            constexpr virtual Return Call(Args&&... args) noexcept = 0;

            constexpr virtual ~Interface() noexcept {};
        };

        template <typename Type>
        class Callable final : public Interface {
            Type instance;

        public:
            constexpr Callable(Type&& value) noexcept : instance(ACTL::forward(value)) {};

            constexpr virtual ~Callable() noexcept {};

            constexpr virtual Return Call(Args&&... args) noexcept {
                instance(ACTL::forward<Args>(args)...);
            }
        };

        Interface* interface = nullptr;

    public:
        constexpr Delegate() noexcept {};

        template <typename Type>
        constexpr Delegate(Type&& value) noexcept {
            interface = new Callable<Type>(ACTL::forward(value));
        }

        constexpr Delegate(Delegate&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Delegate() noexcept {
            Clear();
        }

        template <typename Type>
        constexpr Delegate& operator =(Type&& value) noexcept {
            Clear();

            interface = new Callable<std::remove_reference_t<Type>>(ACTL::forward(value));

            return *this;
        }

        constexpr Delegate& operator =(Delegate&& other) noexcept {
            Clear();

            interface = other.interface;

            other.interface = nullptr;

            return *this;
        }

        constexpr void Clear() noexcept {
            if (interface) {
                delete interface;

                interface = nullptr;
            }
        }

        constexpr bool operator ==(std::nullptr_t) const noexcept {
            return interface == nullptr;
        }

        constexpr bool operator !=(std::nullptr_t) const noexcept {
            return interface != nullptr;
        }

        constexpr operator bool() const noexcept {
            return interface;
        }

        constexpr Return operator ()(Args&&... args) const {
            if (interface)
                return interface->Call(ACTL::forward<Args>(args)...);

            throw "Delegate is empty!";
        }
    };
}