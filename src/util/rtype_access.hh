#pragma once

#include "../prelude.hh"

template<typename Type, size_t I>
struct RTypeAccess {
    using It = typename RTypeAccess<typename Type::PrevType, I-1>::It;
};

template<typename Type>
struct RTypeAccess<Type, 0> {
    using It = Type;
};
