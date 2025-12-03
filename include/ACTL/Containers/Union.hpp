#pragma once

#include "../MetaProgram/MetaProgram.hpp"
#include "../Allocation.hpp"

namespace ACTL {
    template <typename... Types>
    class Union {
    private:
        template <char typeIndex, typename CurrentType, typename... OtherTypes>
        static void Destruct(char index, byte* memory) {
            if (index == typeIndex) {
                CurrentType* Target = (CurrentType*)memory;

                Target->~CurrentType();

                return;
            }

            if constexpr (sizeof...(OtherTypes) != 0)
                Destruct<typeIndex + 1, OtherTypes...>(index, memory);
        }

        char currentTypeIndex = 0;

    public:
        static constexpr size maxSize = biggestPackSize<Types...>;

        static constexpr size length = sizeof...(Types);

        template <typename Type>
        Union(const Type& Value) {
            new(memory) Type(Value);

            currentTypeIndex = (char)packTypeIndex<Type, Types...>;
        }

        template <typename Type>
        Union(Type&& Value) {
            new(memory) Type(ACTL::move(Value));

            currentTypeIndex = (char)packTypeIndex<Type, Types...>;
        }

        ~Union() {
            Destruct<0, Types...>(currentTypeIndex, memory);
        }

        template <typename Type>
        Union& operator =(const Type& Value) {
            constexpr char targetTypeIndex = (char)packTypeIndex<Type, Types...>;

            if (targetTypeIndex != currentTypeIndex) {
                Destruct<0, Types...>(currentTypeIndex, memory);

                new(memory) Type(Value);

                currentTypeIndex = targetTypeIndex;
            }
            else {
                Type* Target = (Type*)memory;

                (*Target) = Value;
            }

            return *this;
        }

        template <typename Type>
        Union& operator =(Type&& Value) {
            constexpr char targetTypeIndex = (char)packTypeIndex<Type, Types...>;

            if (targetTypeIndex != currentTypeIndex) {
                Destruct<0, Types...>(currentTypeIndex, memory);

                new(memory) Type(ACTL::move(Value));

                currentTypeIndex = targetTypeIndex;
            }
            else {
                Type* Target = (Type*)memory;

                (*Target) = ACTL::move(Value);
            }

            return *this;
        }

        template <typename Type>
        operator Type& () {
            constexpr char targetTypeIndex = (char)packTypeIndex<Type, Types...>;

            assert(targetTypeIndex != currentTypeIndex && "This type is not initialized!");

            Type* Pointer = (Type*)memory;

            return *Pointer;
        }

    private:
        byte memory[maxSize] = {};
    };
}