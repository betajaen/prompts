#pragma once

#include <prompts/terminal.hpp>

namespace prompts {
struct sequence {
    const std::string_view data;
};
}


namespace prompts::seq {
constexpr sequence rnl = {"\r\n"};
constexpr sequence r = {"\r"};
constexpr sequence normal = {"\x1b[0m"};
constexpr sequence bold = {"\x1b[1m"};
constexpr sequence underline = {"\x1b[4m"};
constexpr sequence blinking = {"\x1b[5m"};
constexpr sequence inverse = {"\x1b[7m"};
constexpr sequence not_bold ("\x1b[22m");
constexpr sequence not_underline ("\x1b[24m");
constexpr sequence not_blinking("\x1b[25m");
constexpr sequence not_inverse("\x1b[27m");
constexpr sequence erase_this_line("\r\x1b[0K");
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::sequence& seq) {
    tw.write(seq.data);
    return tw;
}
