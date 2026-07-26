#pragma once

#include <cstdint>
#include <span>

namespace prompts {

struct codepoint_t {

    const uint32_t value;

    bool is_control() const {
        return value > 0x8000'0000;
    }

    uint32_t utf32() const {
        return value & 0x001F'FFFF;
    }

    uint32_t utf8(char* dst, size_t length) const {
        if (is_control() || dst == nullptr || length < 4)
            return 0;
        const auto v = utf32();
        if (v <= 0x7F) {
            dst[0] = v;
            return 1;
        }
        else if (v <= 0x7FF) {
            dst[0] = 0xC0 | (v >> 6);
            dst[1] = 0x80 | (v & 0x3f);
            return 2;
        }
        else if (v <= 0xFFFF) {
            dst[0] = 0xE0 | (v >> 12);
            dst[1] = 0x80 | ((v >> 6) & 0x3F);
            dst[2] = 0x80 | (v & 0x3F);
            return 3;
        }
        else if (v <= 0x1F'FFFF) {
            dst[0] = 0xF0 | (v >> 18);
            dst[1] = 0x80 | ((v >> 12) & 0x3F);
            dst[2] = 0x80 | ((v >> 6) & 0x3F);
            dst[3] = 0x80 | (v & 0x3F);
            return 4;
        }

        return 0;
    }

    static constexpr codepoint_t from_control(uint32_t value) {
        return { 0x8000'0000 + value };
    }

    static constexpr codepoint_t from_utf32(uint32_t value) {
        return { value & 0x001F'FFFF };
    }

};

constexpr codepoint_t kUnknown   = { 0x80FF'FFFF };
constexpr codepoint_t kUp   = { 0x8000'0041 };
constexpr codepoint_t kDown = { 0x8000'0042 };
constexpr codepoint_t kForward = { 0x8000'0043 };
constexpr codepoint_t kBackward = { 0x8000'0044 };
constexpr codepoint_t kBackspace = { 0x8000'0008 };

}



namespace prompts::strings {

inline std::uint16_t line_width(const std::string_view& sv) {
    auto s = sv.find('\n');

    if (s == std::string_view::npos) {
        s = sv.size();
    }

    if (s >= std::numeric_limits<std::uint16_t>::max()) {
        return std::numeric_limits<std::uint16_t>::max() - 1;
    }

    return s;
}

}