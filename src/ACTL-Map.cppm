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

export module ACTL:Map;

import :Defines;
import :Hash;
import :Allocation;
import :Utility;

export namespace ACTL {
    // Hashtable implementation.
    template <typename Type, typename Key>
    class Map {
        struct Bucket {
            union {
                Key key;
            };

            union {
                Type data;
            };

            Bucket* child;

            bool valid;

            constexpr Bucket() noexcept {};

            constexpr ~Bucket() noexcept {};

            template <typename... Args>
            constexpr void Create(Key&& key, Args&&... args) noexcept {
                valid = true;

                std::construct_at(&this->key, ACTL::forward(key));

                std::construct_at(&data, ACTL::forward<Args>(args)...);
            }

            constexpr void Destroy() noexcept {
                if (child) {
                    key = ACTL::move(child->key);

                    data = ACTL::move(child->data);

                    child->key.~Key();

                    child->data.~Type();

                    auto n = child->child;

                    delete child;

                    child = n;

                    return;
                }

                valid = false;

                data.~Type();

                key.~Key();
            }

            constexpr void DestroyWithFamily() noexcept {
                valid = false;

                data.~Type();

                key.~Key();

                while (child) {
                    auto next = child->child;

                    child->data.~Type();

                    child->key.~Key();

                    delete child;

                    child = next;
                }
            }

            struct SearchResult {
                Bucket* bucket;

                bool success;
            };

            constexpr SearchResult Find(const Key& key) noexcept {
                if (this->key == key)
                    return {
                        this,
                        true
                    };

                auto c = child;

                if (!c)
                    return {
                        this,
                        false
                    };

                while (true) {
                    if (c->key == key)
                        return {
                            c,
                            true
                        };

                    if (!c->child)
                        return {
                            c,
                            false
                        };

                    c = c->child;
                }
            }
        };

        Bucket* memory = nullptr;

        u32 memoryCapacity = 0;

        u32 bucketCount = 0;

        constexpr u32 GetIndex(u64 hash) const noexcept {
            return static_cast<u32>(hash % memoryCapacity);
        }

        template <typename... Args>
        constexpr Type& Emplace(auto&& func, Key&& key, Args&&... args) noexcept {
            if (0.75f * memoryCapacity < bucketCount)
                Expand();

            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto bucket = memory + index;

            if (!bucket->valid) {
                bucketCount++;

                bucket->Create(ACTL::forward(key), ACTL::forward<Args>(args)...);

                return bucket->data;
            }

            auto search = bucket->Find(key);

            bucket = search.bucket;

            if (search.success) {
                return func(bucket, ACTL::forward<Args>(args)...);
            }

            bucket->child = new Bucket();

            bucket = bucket->child;

            bucket->Create(ACTL::forward(key), ACTL::forward<Args>(args)...);

            bucketCount++;

            return bucket->data;
        }

    public:
        Map(const Map&) = delete;

        Map& operator =(const Map&) = delete;

        constexpr Map() noexcept {};

        constexpr Map(u32 capacity) noexcept {
            operator=(capacity);
        }

        constexpr Map(Map&& other) noexcept {
            operator=(ACTL::move(other));
        }

        constexpr ~Map() noexcept {
            Free();
        }

        constexpr Map& operator =(u32 capacity) noexcept {
            Allocate(capacity);

            return *this;
        }

        constexpr Map& operator =(Map&& other) noexcept {
            Free();

            ACTL::swap(memory, other.memory);

            ACTL::swap(memoryCapacity, other.memoryCapacity);

            ACTL::swap(bucketCount, other.bucketCount);

            return *this;
        }

        constexpr void Allocate(u32 count) noexcept {
            Free();

            memory = ACTL::Allocate<Bucket>(count);

            memoryCapacity = count;

            for (u32 i = 0; i < memoryCapacity; i++)
                memory[i].valid = false;
        }

        // Increases Map capacity and rehashes all data.
        constexpr void Expand(u32 count) noexcept {
            if (!count)
                return;

            auto oldMemoryCapacity = memoryCapacity;

            memoryCapacity += count;

            auto oldMemory = memory;

            memory = ACTL::Allocate<Bucket>(memoryCapacity);

            for (u32 i = 0; i < memoryCapacity; i++)
                memory[i].valid = false;

            for (u32 i = 0; i < oldMemoryCapacity; i++) {
                auto& bucket = oldMemory[i];

                if (!bucket.valid)
                    continue;

                EmplaceOrSet(ACTL::move(bucket.key), ACTL::move(bucket.data));

                bucket.valid = false;

                bucket.data.~Type();

                bucket.key.~Key();

                auto child = bucket.child;

                while (child) {
                    EmplaceOrSet(ACTL::move(child->key), ACTL::move(child->data));

                    child->data.~Type();

                    child->key.~Key();

                    auto d = child;

                    child = child->child;

                    delete d;
                }
            }

            ACTL::Free(oldMemory, oldMemoryCapacity);
        }

        constexpr void Expand() noexcept {
            if (memoryCapacity)
                Expand(memoryCapacity);
            else
                Allocate(32);
        }

        // Clears Map and frees its memory.
        constexpr void Free() noexcept {
            if (!memory)
                return;

            Clear();

            ACTL::Free(memory, memoryCapacity);

            memory = nullptr;

            memoryCapacity = 0;
        }

        // Emplaces new data into map. Reinitializes existing data if key is occupied.
        template <typename... Args>
        constexpr Type& EmplaceOrSet(Key&& key, Args&&... args) noexcept {
            return Emplace(
                [&](Bucket* bucket, Args&&... args) -> Type& {
                    bucket->data.~Type();

                    std::construct_at(&bucket->data, ACTL::forward<Args>(args)...);

                    return bucket->data;
                },
                ACTL::forward(key),
                ACTL::forward<Args>(args)...
            );
        }

        // Emplaces new data into map. Returns existing data if key is occupied.
        template <typename... Args>
        constexpr Type& EmplaceOrGet(Key&& key, Args&&... args) noexcept {
            return Emplace(
                [&](Bucket* bucket, Args&&... args) -> Type& {
                    return bucket->data;
                },
                ACTL::forward(key),
                ACTL::forward<Args>(args)...
            );
        }

        // Erases data from map. Does nothing if key is not occupied.
        constexpr void Erase(const Key& key) noexcept {
            if (isEmpty())
                return;

            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto& bucket = memory[index];

            if (!bucket.valid)
                return;

            auto sr = bucket.Find(key);

            if (sr.success) {
                sr.bucket->Destroy();

                bucketCount--;
            }
        }

        // Destructs all data in map.
        constexpr void Clear() noexcept {
            if (!bucketCount)
                return;

            for (u32 i = 0; i < memoryCapacity; i++)
                if (memory[i].valid)
                    memory[i].DestroyWithFamily();

            bucketCount = 0;
        }

        // Returns true if map has data with given key.
        constexpr bool has(const Key& key) const noexcept {
            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto& bucket = memory[index];

            if (!bucket.valid)
                return false;

            return bucket.Find(key).success;
        }

        // Returns data of given key. Throws exception if map has no data with given key.
        constexpr Type& GetOrExcept(const Key& key) {
            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto& bucket = memory[index];

            constexpr auto exception = "Failed to find matching key!";

            if (!bucket.valid)
                throw exception;

            auto sr = bucket.Find(key);

            if (!sr.success)
                throw exception;

            return sr.bucket->data;
        }

        // Returns data of given key. Throws exception if map has no data with given key.
        constexpr const Type& GetOrExcept(const Key& key) const {
            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto& bucket = memory[index];

            constexpr auto exception = "Failed to find matching key!";

            if (!bucket.valid)
                throw exception;

            auto sr = bucket.Find(key);

            if (!sr.success)
                throw exception;

            return sr.bucket->data;
        }

        // Returns data of given key. Returns new instance of data if map has no data with given key.
        template <typename... Args>
        constexpr Type GetOrNew(const Key& key, Args&&... args) const noexcept {
            auto hash = GetHash(key);

            auto index = GetIndex(hash);

            auto& bucket = memory[index];

            constexpr auto exception = "Failed to find matching key!";

            if (!bucket.valid)
                return Type(ACTL::forward<Args>(args)...);

            auto sr = bucket.Find(key);

            if (!sr.success)
                return Type(ACTL::forward<Args>(args)...);

            return sr.bucket->data;
        }

        // Returns data of given key. Emplaces new data if map has no data with given key.
        template <typename... Args>
        constexpr Type& GetOrEmplace(Key&& key, Args&&... args) noexcept {
            return EmplaceOrGet(ACTL::forward(key), ACTL::forward<Args>(args)...);
        }

        // Returns data of given key. Throws exception if map has no data with given key.
        constexpr Type& operator [](const Key& key) {
            return GetOrExcept(key);
        }

        // Returns data of given key. Throws exception if map has no data with given key.
        constexpr const Type& operator [](const Key& key) const {
            return GetOrExcept(key);
        }

        constexpr bool isAllocated() const noexcept {
            return memory;
        }

        constexpr bool notEmpty() const noexcept {
            return bucketCount;
        }

        constexpr bool isEmpty() const noexcept {
            return !notEmpty();
        }

        constexpr operator bool() const noexcept {
            return notEmpty();
        }

        constexpr void Iterate(auto&& func) noexcept {
            for (u32 i = 0; i < memoryCapacity; i++) {
                auto bucket = memory + i;

                if (!bucket->valid)
                    continue;

                while (bucket) {
                    func(ACTL::forward(bucket->data));

                    bucket = bucket->child;
                }
            }
        }

        constexpr void Iterate(auto&& func) const noexcept {
            for (u32 i = 0; i < memoryCapacity; i++) {
                const Bucket* bucket = memory + i;

                if (!bucket->valid)
                    continue;

                while (bucket) {
                    func(ACTL::forward(bucket->data));

                    bucket = bucket->child;
                }
            }
        }
    };
}