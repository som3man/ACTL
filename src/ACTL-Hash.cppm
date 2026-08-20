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
#include <concepts>
#include <algorithm>

export module ACTL:Hash;

import :Defines;

export namespace ACTL {
    /*
    Fragments of this code are based on rapidhash algorithm.
    Original code: Copyright (c) 2025 Nicolas De Carli (https://github.com/Nicoshev/rapidhash)
    Modification: Copyright (c) 2026 som3man

    This code is distributed under the MIT license.

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
    [[nodiscard]] constexpr u64 RapidHashNano(const void* key, size keySize) noexcept {
    #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
        # define _likely_(x)  __builtin_expect(x,1)
        # define _unlikely_(x)  __builtin_expect(x,0)
    #else
        # define _likely_(x) (x)
        # define _unlikely_(x) (x)
    #endif

        constexpr u64 secret[8] =  {
            0x2d358dccaa6c78a5ull,
            0x8bb84b93962eacc9ull,
            0x4b33a62ed433d4a3ull,
            0x4d5a2da51de1aa47ull,
            0xa0761d6478bd642full,
            0xe7037ed1a0b428dbull,
            0x90ed1765281c388cull,
            0xaaaaaaaaaaaaaaaaull
        };

        auto Mum = [&](u64* a, u64* b) -> void {
        #if defined(_MSC_VER) && (defined(_WIN64) || defined(_M_HYBRID_CHPE_ARM64))
            #if defined(_M_X64)
                *A=_umul128(*A,*B,B);
            #else
                u64 c = __umulh(*A, *B);

                *A = *A * *B;

                *B = c;
            #endif
        #else
            u64 ha=*a>>32, hb=*b>>32, la=(u32)*a, lb=(u32)*b;

            u64 rh=ha*hb, rm0=ha*lb, rm1=hb*la, rl=la*lb, t=rl+(rm0<<32), c=t<rl;

            u64 lo=t+(rm1<<32); 

            c+=lo<t; 

            u64 hi=rh+(rm0>>32)+(rm1>>32)+c;

            *a=lo;  *b=hi;
        #endif
        };

        auto Mix = [&](u64 a, u64 b) -> u64 {
            Mum(&a, &b);

            return a ^ b;
        };

    #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
        auto Read64 = [&](const u8* p) -> u64 {
            u64 v; std::copy(p, p + sizeof(u64), std::bit_cast<u8*>(&v)); return __builtin_bswap64(v);
        };

        auto Read32 = [&](const u8* p) -> u64 {
            u32 v; std::copy(p, p + sizeof(u32), std::bit_cast<u8*>(&v)); return __builtin_bswap32(v);
        };
    #elif defined(_MSC_VER)
        auto Read64 = [&](const u8* p) -> u64 {
            u64 v; std::copy(p, p + sizeof(u64), std::bit_cast<u8*>(&v)); return  _byteswap_uint64(v);
        };

        auto Read32 = [&](const u8* p) -> u64 {
            u32 v; std::copy(p, p + sizeof(u32), std::bit_cast<u8*>(&v));return _byteswap_ulong(v);
        };
    #else
        auto Read64 = [&](const u8* p) -> u64 {
            u64 v; std::copy(p, p + sizeof(u64), std::bit_cast<u8*>(&v));;

            return (((v >> 56) & 0xff)| ((v >> 40) & 0xff00)| 
                ((v >> 24) & 0xff0000)| ((v >>  8) & 0xff000000)| 
                ((v <<  8) & 0xff00000000)| ((v << 24) & 0xff0000000000)| 
                ((v << 40) & 0xff000000000000)| ((v << 56) & 0xff00000000000000));
        };

        auto Read32 = [&](const u8* p) -> u64 {
            u32 v; std::copy(p, p + sizeof(u32), std::bit_cast<u8*>(&v));;

            return (((v >> 24) & 0xff)| ((v >>  8) & 0xff00)| ((v <<  8) & 0xff0000)| ((v << 24) & 0xff000000));
        };
    #endif

        u64 seed = 0;

        const u8 *p=std::bit_cast<const u8*>(key);

        seed ^= Mix(seed ^ secret[2], secret[1]);

        u64 a=0, b=0;

        size i = keySize;

        if (_likely_(keySize <= 16)) {
            if (keySize >= 4) {
                seed ^= keySize;

                if (keySize >= 8) {
                    const u8* plast = p + keySize - 8;

                    a = Read64(p);

                    b = Read64(plast);
                } 
                else {
                    const u8* plast = p + keySize - 4;

                    a = Read32(p);

                    b = Read32(plast);
                }
            } 
            else if (keySize > 0) {
                a = (((u64)p[0])<<45)|p[keySize-1];

                b = p[keySize>>1];
            } 
            else
                a = b = 0;
        } 
        else {
            if (i > 48) {
                u64 see1 = seed, see2 = seed;

                do {
                    seed = Mix(Read64(p) ^ secret[0], Read64(p + 8) ^ seed);

                    see1 = Mix(Read64(p + 16) ^ secret[1], Read64(p + 24) ^ see1);

                    see2 = Mix(Read64(p + 32) ^ secret[2], Read64(p + 40) ^ see2);

                    p += 48;

                    i -= 48;
                } while(i > 48);

                seed ^= see1;

                seed ^= see2;
            }

            if (i > 16) {
                seed = Mix(Read64(p) ^ secret[2], Read64(p + 8) ^ seed);

                if (i > 32) {
                    seed = Mix(Read64(p + 16) ^ secret[2], Read64(p + 24) ^ seed);
                }
            }

            a=Read64(p+i-16) ^ i;  b=Read64(p+i-8);
        }

        a ^= secret[1];

        b ^= seed;

        Mum(&a, &b);

        return Mix(a ^ secret[7], b ^ secret[1] ^ i);

    #undef _likely_
    #undef _unlikely_
    }

    template <class Type>
    concept Hashable = requires (const Type object) {
        { object.GetHash() } -> std::same_as<u64>;
    };

    template <Hashable Type>
    [[nodiscard]] constexpr u64 GetHash(const Type& object) noexcept {
        return object.GetHash();
    }

    template <typename Type>
    [[nodiscard]] constexpr u64 GetHash(const Type &object) noexcept {
        return RapidHashNano(&object, sizeof(Type));
    }
}