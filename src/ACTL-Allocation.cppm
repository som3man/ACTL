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

#include <utility>
#include <memory>

export module ACTL:Allocation;

import :Defines;

export namespace ACTL {
    template <typename Type>
    [[nodiscard]] constexpr Type* Allocate(size count = 1) noexcept {
        std::allocator<Type> allocator = {};

        return allocator.allocate(count);
    }

    template <typename Type>
    constexpr void Free(Type* memory, size count = 1) noexcept {
        std::allocator<Type> allocator = {};

        allocator.deallocate(memory, count);
    }

    template <typename Type, typename... Args>
    constexpr void Construct(Type* memory, Args&&... args) noexcept {
        std::construct_at(memory, std::forward<Args>(args)...);
    }
}