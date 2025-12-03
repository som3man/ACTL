#pragma once

#include "../MetaProgram/MetaProgram.hpp"
#include "../Allocation.hpp"

namespace ACTL {
    template <typename... Types>
    class Tuple {
    private:
        template <typename CurrentType, typename... OtherTypes>
        struct Item {
            Item<OtherTypes...> Next;

            CurrentType Data;

            Item() : Next(), Data() {};

            Item(const CurrentType& This, const OtherTypes&... Other) : Next(Other...), Data(This) {};

            Item(CurrentType&& This, OtherTypes&&... Other) : Next(ACTL::move(Other...)), Data(ACTL::move(This)) {};

            ~Item() {};

            void Assign(const CurrentType& This, const OtherTypes&... Other) {
                Data = This;

                Next.Assign(Other...);
            }

            template <typename Type>
            Type& getData() {
                if constexpr (IsSame<Type, CurrentType>())
                    return Data;
                else
                    return Next.template getData<Type>();
            }
        };

        template <typename CurrentType>
        struct Item<CurrentType> {
            CurrentType Data;

            Item() : Data() {};

            Item(const CurrentType& Value) : Data(Value) {};

            Item(CurrentType&& Value) : Data(ACTL::move(Value)) {};

            ~Item() {};

            void Assign(const CurrentType& Value) {
                Data = Value;
            }

            template <typename Type>
            Type& getData() {
                return Data;
            }
        };

        Item<Types...> First;
    
    public:
        static constexpr size fullSize = fullPackSize<Types...>;

        static constexpr size length = sizeof...(Types);

        Tuple(const Types&... Values) : First(Values...) {};

        ~Tuple() {};

        template <typename Type>
        operator Type& () {
            return First.template getData<Type>();
        }

        template <typename Type>
        Tuple& operator =(const Type& Value) {
            First.template getData<Type>() = Value;

            return *this;
        }
    };
}