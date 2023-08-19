#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <string_view>
#include <optional>
#include <array>
#include <bit>
#include <variant>
#include <span>
#include <concepts>
#include <type_traits>
#include <thread>
#include <atomic>
#include <cstring>
#include <new>
#include <memory>
#include <cstdlib>

#include <tl/expected.hpp>

using std::vector;
using std::string;
using std::string_view;
using std::optional;
using std::array;
using std::monostate;
using std::span;

using std::nullopt;

using tl::expected;
using tl::unexpected;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

inline span<const u8> string_as_bytes(const string& str) {
    return span(reinterpret_cast<const u8*>(str.c_str()), str.size());
}

namespace obj {
    using std::move;
    using std::ref;
    using std::cref;
}

namespace mem {
    using std::memcpy;
    using std::unique_ptr;
    using std::make_unique;
}
