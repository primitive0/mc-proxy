#pragma once

#include "../prelude.hh"

inline optional<u8> parse_u8(string_view str) {
    i32 value{};
    try {
        value = std::stoi(string(str));
    } catch (const std::invalid_argument& e) {
        return nullopt;
    } catch (const std::out_of_range& e) {
        return nullopt;
    }
    if (value > 255 || value < 0) {
        return nullopt;
    }
    return static_cast<u8>(value);
}

struct IpAddrV4 {
    using DataType = array<u8, 4>;

    DataType data;

    constexpr IpAddrV4(const DataType& v) : data(v) {}

    static const IpAddrV4 UNSPECIFIED;

    u32 as_u32() const {
        return std::bit_cast<u32>(this->data);
    }

    static optional<IpAddrV4> parse(string_view str) {
        IpAddrV4 addr = IpAddrV4::UNSPECIFIED;
        size_t cursor = 0;

        for (size_t i = 0; i < addr.data.size() - 1; i++) {
            size_t index = str.find('.', cursor);
            if (index == string_view::npos) {
                return nullopt;
            }
            auto octet = IpAddrV4::parse_octet(str, index - 1, cursor);
            if (!octet) {
                return nullopt;
            }
            cursor++;
            addr.data[i] = *octet;
        }

        auto octet = IpAddrV4::parse_octet(str, str.size() - 1, cursor);
        if (!octet) {
            return nullopt;
        }
        cursor++;
        addr.data[3] = *octet;

        return addr;
    }

private:
    static optional<u8> parse_octet(string_view str, size_t index, size_t& cursor) {
        string_view part = str.substr(cursor, index - cursor + 1);
        auto octet = parse_u8(part);
        if (!octet) {
            return nullopt;
        }
        cursor += part.size();
        return *octet;
    }
};

inline const IpAddrV4 IpAddrV4::UNSPECIFIED({u8(0), u8(0), u8(0), u8(0)});
