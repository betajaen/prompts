#pragma once

#include <vector>
#include <string_view>
#include <charconv>
#include <unistd.h>
#include <termios.h>
#include <prompts/codepoint.hpp>

namespace prompts {

class terminal_writer {
private:
    std::vector<char> m_data;
    bool m_emit_on_linebreak;
    int m_output;
public:

    using sv = std::string_view;

    terminal_writer(int output_ = STDOUT_FILENO)
       : m_output(output_) {
        m_data.reserve(100);
    }


    ~terminal_writer() = default;

    terminal_writer(terminal_writer&) = delete;
    terminal_writer(const terminal_writer&) = delete;
    terminal_writer(terminal_writer&&) = delete;
    terminal_writer& operator=(terminal_writer&) = delete;
    terminal_writer& operator=(const terminal_writer&) = delete;
    terminal_writer& operator=(terminal_writer&&) = delete;

private:

    constexpr void _append_char(char ch) {
        m_data.push_back(ch);
    }


    constexpr void _append_text(const sv& sv) {
        for (size_t i=0;i < sv.size();i++) {
            m_data.push_back(sv[i]);
        }
    }


    constexpr void _append_text(const char* text) {
        while (*text) {
            m_data.push_back(*text++);
        }
    }


    template<typename T>
    constexpr void _append_convertable(const T& convertable) {
        std::array<char, std::numeric_limits<T>::digits10 + 1> temp;
        const auto rv = std::to_chars(temp.begin(), temp.end(), convertable);
        size_t length = rv.ptr - temp.begin();
        for (size_t i=0;i < length;i++) {
            m_data.push_back(temp[i]);
        }
    }

public:

    void emit(bool append_sgr_reset = true) {
        if (append_sgr_reset) {
            _append_text("\x1b[0m");
        }

        ::write(STDOUT_FILENO, m_data.data(), sizeof(char) * m_data.size());
        m_data.clear();
    }


    constexpr void write() {}


    constexpr terminal_writer& repeat(char ch, size_t count) {
        for (size_t i=0;i < count;i++) {
            m_data.push_back(ch);
        }
        return *this;
    }


    constexpr terminal_writer& write(char ch) {
        _append_char(ch);
        return *this;
    }


    constexpr terminal_writer& write(const sv& str_view) {
        _append_text(str_view);
        return *this;
    }


    constexpr terminal_writer& write(const char* cstr) {
        _append_text(sv(cstr));
        return *this;
    }


    constexpr terminal_writer& write(bool convertable) {
        _append_text(convertable ? sv("true") : sv("false"));
        return *this;
    }


    template<typename T>
    constexpr terminal_writer& write(const T& convertable) {
        _append_convertable(convertable);
        return *this;
    }


    template<typename TFirst, typename ...TRest>
    constexpr terminal_writer& write(TFirst first, TRest ... args) {
        write(first);
        write(args...);
        return *this;
    }

};



}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, char ch) {
    tw.write(ch);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const char* str) {
    tw.write(str);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const std::string_view& sv) {
    tw.write(sv);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const bool& sv) {
    tw.write(sv);
    return tw;
}


template<typename T>
inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const T& convertable) {
    tw.write(convertable);
    return tw;
}


inline prompts::terminal_writer& operator<<(prompts::terminal_writer& tw, const prompts::codepoint_t& cp) {
    if (cp.value < 0x7F) {
        tw.write((char) cp.value);
    }
    else {
        std::array<char, 4> x = { 0 };
        std::uint32_t l = cp.utf8(x.data(), x.size());
        if (l) {
            tw.write(std::string_view(x.data(), l));
        }
    }
    return tw;
}




namespace prompts {

class terminal_reader {
private:
    struct termios m_orig_termios;
    bool m_captured;
    int m_input, m_output;

    // !!! BEGIN
    // Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
    // See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
    static constexpr uint8_t utf8d[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
        0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
        0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
        0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
        1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
        1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
        1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
    };
    static constexpr int UTF8_ACCEPT = 0;
    static constexpr int UTF8_REJECT = 1;

    uint32_t inline decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
        uint32_t type = utf8d[byte];

        *codep = (*state != UTF8_ACCEPT) ?
          (byte & 0x3fu) | (*codep << 6) :
          (0xff >> type) & (byte);

        *state = utf8d[256 + *state*16 + type];
        return *state;
    }
    // !!! END

    uint8_t _read_byte() const {
        char ch;
        int nread;
        while ((nread = read(m_input, &ch,sizeof(ch))) == 0) {
            usleep(5000);
        }
        if (nread == -1) {
            return 0;
        }
        return ch;
    }

    uint32_t _read_utf8_as_utf32()  {
        std::uint32_t state = 0;
        std::uint32_t cp = 0;

        while (true) {
            uint32_t byte = _read_byte();
            decode(&state, &cp, byte);

            if (state == UTF8_ACCEPT) {
                return cp;
            }
            else if (state == UTF8_REJECT) {
                return 0;
            }
        }

        return 0;
    }

    codepoint_t _read_sequence() {
        uint32_t seq = _read_utf8_as_utf32();

        switch (seq) {
            default:
                return { seq };
            case 8:
                return kBackspace;
            case 27: {
                seq = _read_utf8_as_utf32();
                if (seq == '[') {
                    seq = _read_utf8_as_utf32();
                    if (seq == 'A') {
                        return kUp;
                    }
                    else if (seq == 'B') {
                        return kDown;
                    }
                    else if (seq == 'C') {
                        return kForward;
                    }
                    else if (seq == 'D') {
                        return kBackward;
                    }

                    return kUnknown;
                }
                else {
                    return { 27 };
                }
            }
            case 127:
                return kBackspace;
        }

    }

public:


    terminal_reader(int input = STDIN_FILENO, int output = STDOUT_FILENO, bool bind_tty = true)
        : m_input(input), m_output(output), m_captured(false), m_orig_termios{} {
        if (bind_tty) {
            bind();
        }
    }

    ~terminal_reader() {
        unbind();
    }

    void bind() {
        if (m_captured == false) {

            // !!! BEGIN
            // Copyright (c) 2016, Salvatore Sanfilippo <antirez at gmail dot com>
            // See https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html for details.
            struct termios raw;

            if (!isatty(m_input))  {
                errno = ENOTTY;
                return;
            }

            if (tcgetattr(m_input, &m_orig_termios) == -1)  {
                errno = ENOTTY;
                return;
            }

            raw = m_orig_termios;
            raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
            raw.c_oflag &= ~(OPOST);
            raw.c_cflag |= (CS8);
            raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 1;

            if (tcsetattr(m_input, TCSAFLUSH,&raw) < 0) {
                errno = ENOTTY;
                return;
            }
            // !!! END

            ::write(m_output, "\x1b[?2005h", 8);
            m_captured = true;
        }
    }

    void unbind() {
        if (m_captured && tcsetattr(m_input, TCSAFLUSH,&m_orig_termios) != -1) {
            ::write(m_output, "\x1b[?2005l", 8);
            m_captured = false;
        }
    }

    terminal_reader(terminal_reader&) = delete;
    terminal_reader(const terminal_reader&) = delete;
    terminal_reader(terminal_reader&&) = delete;

    terminal_reader& operator=(terminal_reader&) = delete;
    terminal_reader& operator=(const terminal_reader&) = delete;
    terminal_reader& operator=(terminal_reader&&) = delete;

    codepoint_t wait_codepoint() {
        return _read_sequence();
    }

};


}