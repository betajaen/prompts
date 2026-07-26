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

    static constexpr brush none() {
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
    rows = std::max<std::int16_t>(1, rows);
    cols = std::max<std::int16_t>(1, cols);
    std::int16_t txwd = std::min<std::int16_t>(cols, strings::line_width(text));
    std::int16_t left_w = (cols - txwd) / 2;
    std::int16_t right_w = (cols - txwd - left_w);
    std::int16_t mid_h = 0;
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




template<typename T>
struct list_info {
    T hilighted = {};
    T selected = {};
    pen selected_pen = { 170 };
    pen not_selected_pen = { 240 };
    pen item_pen = { 15 };
    brush hilighted_brush = { 190 };
    brush hilighted_pen = { 0 };
    std::string_view not_selected_glyph = "\u2022 ";
    std::string_view selected_glyph = "\u2713 ";
};

template<typename I, typename S, typename T = I::value_type>
inline void list(terminal_writer& tw, I begin, S end, const list_info<T>& info = {}) {

    while (begin != end) {

        const bool is_highlighted = (info.hilighted == *begin);
        const bool is_selected = (info.selected == *begin);

        if (is_highlighted) {
            tw << info.hilighted_brush << info.hilighted_pen;
        }
        else {
            if (is_selected) {
                tw << info.selected_pen;
            }
            else {
                tw << info.not_selected_pen;
            }
        }

        if (is_selected) {
            tw << info.selected_glyph;
        }
        else {
            tw << info.not_selected_glyph;
        }

        tw << info.item_pen << *begin;

        if (is_highlighted) {
            const std::int16_t len = 60 - strings::line_width(*begin);
            if (len > 1) {
                tw.repeat(' ', len);
                tw << brush::none();
            }
        }

        tw << pen::none() << "\r\n";
        begin++;
    }
    tw.emit();
}


template<typename T>
inline void list(terminal_writer& tw, auto begin, auto end, const T& selected) {
    list_info<T> info = {};
    info.selected = selected;
    list(tw, begin, end, info);
}

inline void seperator(terminal_writer& tw, const std::string_view& ch = "\u23AF", std::uint16_t width = 60, const pen& pen = { 240 }) {
    tw << "\r\n";
    tw << pen;
    tw.repeat(ch, width);
    tw << pen::none();
    tw << "\n\r\n";
    tw.emit();
}

inline void hero(terminal_writer& tw, const std::string_view& text, const brush& on = { 120 }, const brush& off = brush::none(), std::uint16_t padding = 0) {

    /// !!! BEGIN
    /// See https://github.com/azmr/blit-fonts for details.
    ///
    /// ISC License
    ///
    /// Copyright (c) 2018 Andrew Reece
    ///
    /// Permission to use, copy, modify, and/or distribute this software for any
    /// purpose with or without fee is hereby granted, provided that the above
    /// copyright notice and this permission notice appear in all copies.
    ///
    /// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    /// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    /// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    /// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    /// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    /// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    /// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    static constexpr std::uint32_t kFont[] = {
        0x00000000,0x08021084,0x0000294a,0x15f52bea,0x08fa38be,0x33a22e60,0x2e94d8a6,0x00001084,
        0x10421088,0x04421082,0x00a23880,0x00471000,0x04420000,0x00070000,0x0c600000,0x02222200,
        0x1d3ad72e,0x3e4214c4,0x3e22222e,0x1d18320f,0x210fc888,0x1d183c3f,0x1d17844c,0x0222221f,
        0x1d18ba2e,0x210f463e,0x0c6018c0,0x04401000,0x10411100,0x00e03800,0x04441040,0x0802322e,
        0x3c1ef62e,0x231fc544,0x1f18be2f,0x3c10862e,0x1f18c62f,0x3e10bc3f,0x0210bc3f,0x1d1c843e,
        0x2318fe31,0x3e42109f,0x0c94211f,0x23149d31,0x3e108421,0x231ad6bb,0x239cd671,0x1d18c62e,
        0x0217c62f,0x30eac62e,0x2297c62f,0x1d141a2e,0x0842109f,0x1d18c631,0x08454631,0x375ad631,
        0x22a21151,0x08421151,0x3e22221f,0x1842108c,0x20820820,0x0c421086,0x00004544,0xbe000000,
        0x00000082,0x1c97b000,0x0e949c21,0x1c10b800,0x1c94b908,0x3c1fc5c0,0x42211c4c,0x4e87252e,
        0x12949c21,0x0c210040,0x8c421004,0x12519521,0x0c210842,0x235aac00,0x12949c00,0x0c949800,
        0x4213a526,0x7087252e,0x02149800,0x0e837000,0x0c213c42,0x0e94a400,0x0464a400,0x155ac400,
        0x36426c00,0x4e872529,0x1e223c00,0x1843188c,0x08421084,0x0c463086,0x0006d800,
    };
    /// !!! END

    tw.write("\r\n");
    for (int y=0; y < 6;y++) {

        tw.repeat(' ', padding);

        for (char ch : text) {
            if (ch < 32 || ch > 127)
                continue;

            uint32_t bm = kFont[ch - 32];

            bm >>= 5 * y;


            for (int x = 0;x < 5;x++) {
                int set = bm & 1;

                if (set) {
                    tw << on;
                }
                else {
                    tw << off;
                }

                tw << "  ";
                bm >>= 1;
            }

            tw.write("\x1b[0m  ");
        }
        tw.write("\r\n");
    }
    tw.write("\r\n");
    tw.emit();
}

inline void border(terminal_writer& tw,
    std::uint16_t cols,
    std::uint16_t rows,
    std::uint16_t offset = 0,
    const pen& fg = { 120 },
    const brush& bg = brush::none(),
    const std::array<std::string_view, 9>& pattern = {
        "┏", "━", "┓",
        "┃", " ", "┃",
        "┗", "━", "┛"
    })
{
    rows = std::max<std::uint16_t>(3, rows);
    cols = std::max<std::uint16_t>(3, cols);

    tw.write("\r\x1b[0m");
    for (std::uint16_t r = 0; r < rows; r++) {
        if (offset) {
            tw.cursor_forward(offset);
        }
        tw << fg << bg;
        int off = 0;

        if (rows == 1) {
            off = 3;
        }
        else if (rows == 2) {
            off = 6 * r;
        }
        else if (rows > 2) {
            if (r == 0) {
                off = 0;
            }
            else if (r == rows - 1) {
                off = 6;
            }
            else {
                off = 3;
            }
        }

        tw.write(pattern[off+0]);
        tw.repeat(pattern[off+1], cols - 2);
        tw.write(pattern[off+2]);

        tw.write("\x1b[0m\r\n");
    }

    tw.emit();
}


inline void border_with_text(terminal_writer& tw,
    std::string_view text,
    std::int16_t cols,
    std::int16_t rows,
    std::int16_t offset = 0,
    const pen& border_fg = { 120 },
    const pen& text_fg = { 15 },
    const brush& bg = brush::none(),
    const std::array<std::string_view, 9>& pattern = {
        "┏", "━", "┓",
        "┃", " ", "┃",
        "┗", "━", "┛"
    })
{
    rows = std::max<std::int16_t>(3, rows);
    cols = std::max<std::int16_t>(3, cols);
    std::int16_t txwd = std::min<std::int16_t>(cols - 2, strings::line_width(text));
    std::int16_t left_w = ((cols - 1 - txwd) / 2);
    std::int16_t right_w = (cols - 2 - txwd - left_w);
    std::uint16_t mid_h = 0;
    if (rows > 1) {
        mid_h = rows / 2;
    }

    std::string_view render_text = text.substr(0, txwd);


    tw.write("\r\x1b[0m");
    for (std::uint16_t r = 0; r < rows; r++) {
        if (offset) {
            tw.cursor_forward(offset);
        }
        tw << border_fg << bg;
        int off = 0;

        if (rows == 1) {
            off = 3;
        }
        else if (rows == 2) {
            off = 6 * r;
        }
        else if (rows > 2) {
            if (r == 0) {
                off = 0;
            }
            else if (r == rows - 1) {
                off = 6;
            }
            else {
                off = 3;
            }
        }


        tw.write(pattern[off+0]);

        if (r == mid_h) {
            tw.repeat(pattern[off+1], left_w);
            tw << text_fg << render_text << border_fg;
            tw.repeat(pattern[off+1], right_w);
        }
        else {
            tw.repeat(pattern[off+1], cols - 2);
        }

        tw.write(pattern[off+2]);

        tw.write("\x1b[0m\r\n");
    }

    tw.emit();
}



}
