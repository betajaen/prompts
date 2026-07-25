#pragma once

#include <string_view>
#include <prompts/terminal.hpp>
#include <cstdint>

namespace prompts {

struct pen {
    static constexpr uint32_t kRGB = 0x8000'0000;
    static constexpr uint32_t kPal8  = 0x4000'0000;
    static constexpr uint32_t kPal4  = 0x2000'0000;
    static constexpr uint32_t kBlackWhite  = 0x1000'0000;
    static constexpr uint32_t kNone = 0x0800'0000;

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
        if ((value & kBlackWhite) != 0) {
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
        else if ((value & kPal8) != 0) {
            const uint32_t v = value & 0xFF;
            tw.write("\x1b[38;5;", v, 'm');
        }
        else if ((value & kRGB) != 0) {
            const uint32_t r = value & 0xFF;
            const uint32_t g = (value >> 8) & 0xFF;
            const uint32_t b = (value >> 16) & 0xFF;
            tw.write("\x1b[38;2;", r, ';', g, ';', b, 'm');
        }
    }
};

struct brush {
    static constexpr uint32_t kRGB = 0x8000'0000;
    static constexpr uint32_t kPal8  = 0x4000'0000;
    static constexpr uint32_t kPal4  = 0x2000'0000;
    static constexpr uint32_t kBlackWhite  = 0x1000'0000;
    static constexpr uint32_t kNone = 0x0800'0000;

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
        if ((value & kBlackWhite) != 0) {
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
        else if ((value & kPal8) != 0) {
            const uint32_t v = value & 0xFF;
            tw.write("\x1b[48;5;", v, 'm');
        }
        else if ((value & kRGB) != 0) {
            const uint32_t r = value & 0xFF;
            const uint32_t g = (value >> 8) & 0xFF;
            const uint32_t b = (value >> 16) & 0xFF;
            tw.write("\x1b[48;2;", r, ';', g, ';', b, 'm');
        }
    }
};

}



namespace prompts::primitives {

inline void label(terminal_writer& tw, const std::string_view& label) {
    tw.write(label).write("\r\n").emit();
}

}



inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::pen& pen) {
    pen.write(tw);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::brush& brush) {
    brush.write(tw);
    return tw;
}
