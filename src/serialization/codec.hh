#pragma once

#include "../prelude.hh"

#include "../util/member_ptr_traits.hh"

template<
    typename P,
    typename F,
    std::enable_if_t<std::is_same_v<typename MemberPtrTraits<typename F::MemberPtr>::Parent, P>, bool> = true>
struct FieldInfo {
    using Parent = P;
    using FieldCodec = F;
    using FieldType = typename FieldCodec::FieldType;
};

template<typename T, typename F = void, typename... Fs>
struct Codec : public Codec<T, Fs...> {

    using PrevType = Codec<T, Fs...>;
    using FieldInfoType = FieldInfo<T, F>;

    static constexpr size_t SIZE = PrevType::SIZE + 1;
};

template<typename T>
struct Codec<T, void> {
    static constexpr size_t SIZE = 0;
};
