#pragma once

#include "../prelude.hh"

template<typename T>
struct MemberPtrTraitsHelper {};
 
template<typename T, typename P>
struct MemberPtrTraitsHelper<T P::*> {
    using Type = T;
    using Parent = P;
};

template<class T>
struct MemberPtrTraits : MemberPtrTraitsHelper<std::remove_cv_t<T>> {};
