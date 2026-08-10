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

export module ACTL:Sort;

import :Defines;
import :Utility;

export namespace ACTL {
    struct SearchResult {
        // If found is true, it is the index of object.
        // If found is false, it is the index of the place where the object must be emplaced to keep array sorted.
        // If it is equal to u32max, the object must be emplaced to the back of array.
		u32 index;

        // Is true if object was actually found.
		bool found;
	};

    // Binary search implementation.
    template <typename Type, typename Value>
    constexpr SearchResult Find(const Type* array, size length, const Value& value) noexcept {
        if (!length)
            return {
                u32max,
                false
            };

        u32 low = 0, high = length - 1;

        while (low <= high) {
            u32 middle = low + (high - low) / 2;

            if (array[middle] == value)
                return {
                    middle,
                    true
                };

            if (array[middle] < value)
                low = middle + 1;
            else {
                if (middle == 0)
                    break;

                high = middle - 1;
            }
        }

        return {
            low == length ? u32max : low,
            false
        };
    }

    template <typename Type>
    constexpr void QuickSort(Type* array, size start, size end) noexcept {
        auto GetPartitionIndex = [&](size low, size high) -> u32 {
            Type& pivot = array[low + (high - low) / 2];

            size i = low - 1, j = high + 1;

            while (true) {
                do 
                    i++;
                while (array[i] <= pivot);

                do 
                    j--;
                while (array[j] > pivot);

                if (i >= j)
                    return j;

                ACTL::swap(array[i], array[j]);
            }
        };

        if (start < end) {
            size index = GetPartitionIndex(array, start, end - 1);

            QuickSort(array, start, index);

            QuickSort(array, index + 1, end);
        }
    }
}