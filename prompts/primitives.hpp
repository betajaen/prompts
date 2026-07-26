#pragma once

#include <string_view>
#include <prompts/terminal.hpp>
#include <prompts/codepoint.hpp>
#include <cstdint>

namespace prompts {

struct pen {
    static constexpr uint32_t kRGB = 0x8000'0000;
    static constexpr uint32_t kPal4  = 0x4000'0000;
    static constexpr uint32_t kBlackWhite  = 0x2000'0000;
    static constexpr uint32_t kNone = 0x1000'0000;
    static constexpr uint32_t kPal8  = 0x0000'0000;

    const uint32_t value;

    static constexpr pen none() {
        return { kNone };
    }

    static constexpr pen rgb(uint8_t r, uint8_t g, uint8_t b) {
        return { kRGB | (g << 16 | b << 8 | r) };
    }

    static constexpr pen pal8(uint8_t col) {
        return { kPal8 | (col) };
    }

    static constexpr pen pal4(uint8_t col) {
        return { kPal4 | (col & 0xF) };
    }

    static constexpr pen bw(uint8_t col) {
        return { kBlackWhite | (col & 0x1) };
    }

    void write(terminal_writer& tw) const {
        if ((value & kNone) != 0) {
            tw.write("\x1b[39m");
        }
        else if ((value & kBlackWhite) != 0) {
            const uint32_t v = value & 0xFF;
            if (value == 0x00) {
                tw.write("\x1b[30m");
            }
            else {
                tw.write("\x1b[37m");
            }
        }
        else if ((value & kPal4) != 0) {
            const uint32_t v = value & 0x0F;
            if (v < 8) {
                tw.write("\x1b[", (30 + v), 'm');
            }
            else {
                tw.write("\x1b[", (90 - 8 + v), 'm');
            }
        }
        else if ((value & kRGB) != 0) {
            const uint32_t r = value & 0xFF;
            const uint32_t g = (value >> 8) & 0xFF;
            const uint32_t b = (value >> 16) & 0xFF;
            tw.write("\x1b[38;2;", r, ';', g, ';', b, 'm');
        }
        else if ((value & kPal8) == 0) {
            const uint32_t v = value & 0xFF;
            tw.write("\x1b[38;5;", v, 'm');
        }
    }
};

struct brush {
    static constexpr uint32_t kRGB = 0x8000'0000;
    static constexpr uint32_t kPal4  = 0x4000'0000;
    static constexpr uint32_t kBlackWhite  = 0x2000'0000;
    static constexpr uint32_t kNone = 0x1000'0000;
    static constexpr uint32_t kPal8  = 0x0000'0000;

    const uint32_t value;

    static constexpr pen none() {
        return { kNone };
    }

    static constexpr brush rgb(uint8_t r, uint8_t g, uint8_t b) {
        return { kRGB | (g << 16 | b << 8 | r) };
    }

    static constexpr brush pal8(uint8_t col) {
        return { kPal8 | (col) };
    }

    static constexpr brush pal4(uint8_t col) {
        return { kPal4 | (col & 0xF) };
    }

    static constexpr brush bw(uint8_t col) {
        return { kBlackWhite | (col & 0x1) };
    }

    void write(terminal_writer& tw) const {
        if ((value & kNone) != 0) {
            tw.write("\x1b[49m");
        }
        else if ((value & kBlackWhite) != 0) {
            const uint32_t v = value & 0xFF;
            if (value == 0x00) {
                tw.write("\x1b[40m");
            }
            else {
                tw.write("\x1b[47m");
            }
        }
        else if ((value & kPal4) != 0) {
            const uint32_t v = value & 0x0F;
            if (v < 8) {
                tw.write("\x1b[", (40 + v), 'm');
            }
            else {
                tw.write("\x1b[", (100 - 8 + v), 'm');
            }
        }
        else if ((value & kRGB) != 0) {
            const uint32_t r = value & 0xFF;
            const uint32_t g = (value >> 8) & 0xFF;
            const uint32_t b = (value >> 16) & 0xFF;
            tw.write("\x1b[48;2;", r, ';', g, ';', b, 'm');
        }
        else if ((value & kPal8) == 0) {
            const uint32_t v = value & 0xFF;
            tw.write("\x1b[48;5;", v, 'm');
        }
    }
};

}



inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::pen& pen) {
    pen.write(tw);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::brush& brush) {
    brush.write(tw);
    return tw;
}

namespace prompts::primitives {

inline void label(terminal_writer& tw, const std::string_view& label) {
    tw.write(label).write("\r\n").emit();
}

inline void big(terminal_writer& tw, const std::string_view& label) {
    tw.write("\x1b#3", label, "\r\n\x1b#4", label, "\r\n").emit();
}

inline void wide(terminal_writer& tw, const std::string_view& label) {
    tw.write("\x1b#5", label, "\r\n").emit();
}

inline void block(terminal_writer& tw, std::uint16_t cols, std::uint16_t rows, const brush& b = brush::pal4(1), const pen& p = pen::pal4(15), char fill = ' ', std::uint16_t offset = 0) {
    rows = std::max<std::uint16_t>(1, rows);
    cols = std::max<std::uint16_t>(1, cols);

    tw.write("\r\x1b[0m");
    for (std::uint16_t r = 0; r < rows; r++) {
        if (offset) {
            tw.cursor_forward(offset);
        }
        tw << p << b;
        tw.repeat(fill, cols);
        tw.write("\x1b[0m\r\n");
    }

    tw.emit();
}

inline void block_with_text(terminal_writer& tw, const std::string_view& text, std::uint16_t cols, std::uint16_t rows, const brush& b = brush::pal4(1), const pen& p = pen::pal4(15), char fill = ' ', std::uint16_t offset = 0) {
    rows = std::max<std::uint16_t>(1, rows);
    cols = std::max<std::uint16_t>(1, cols);
    std::uint16_t txwd = std::min(cols, strings::line_width(text));
    //if (txwd == 0)
    //    return;
    std::cout << txwd << std::endl;
    std::uint16_t left_w = (cols - txwd) / 2;
    std::uint16_t right_w = (cols - txwd - left_w);
    std::uint16_t mid_h = 0;
    if (rows > 1) {
        mid_h = rows / 2;
    }
    tw.write("\r\x1b[0m");
    for (std::uint16_t r = 0; r < rows; r++) {
        if (offset) {
            tw.cursor_forward(offset);
        }
        tw << p << b;
        if (r == mid_h) {
            tw.repeat(fill, left_w);
            tw.write(text.substr(0, txwd));
            tw.repeat(fill, right_w);
        }
        else {
            tw.repeat(fill, cols);
        }

        tw.write("\x1b[0m\r\n");
    }

    tw.emit();
}

}
