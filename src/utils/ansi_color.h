#ifndef __ANSI_COLOR_H__
#define __ANSI_COLOR_H__


#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ANSI_CODE_RESET (0)

#define ANSI_CODE_BRIGHT    (1)
#define ANSI_CODE_FAINT     (2)
#define ANSI_CODE_UNDERLINE (4)

#define ANSI_CODE_FOREGROUND_COLOR_BLACK   (30)
#define ANSI_CODE_FOREGROUND_COLOR_RED     (31)
#define ANSI_CODE_FOREGROUND_COLOR_GREEN   (32)
#define ANSI_CODE_FOREGROUND_COLOR_YELLOW  (33)
#define ANSI_CODE_FOREGROUND_COLOR_BLUE    (34)
#define ANSI_CODE_FOREGROUND_COLOR_MAGENTA (35)
#define ANSI_CODE_FOREGROUND_COLOR_CYAN    (36)
#define ANSI_CODE_FOREGROUND_COLOR_WHITE   (37)

#define ANSI_CODE_BACKGROUND_COLOR_BLACK   (40)
#define ANSI_CODE_BACKGROUND_COLOR_RED     (41)
#define ANSI_CODE_BACKGROUND_COLOR_GREEN   (42)
#define ANSI_CODE_BACKGROUND_COLOR_YELLOW  (43)
#define ANSI_CODE_BACKGROUND_COLOR_BLUE    (44)
#define ANSI_CODE_BACKGROUND_COLOR_MAGENTA (45)
#define ANSI_CODE_BACKGROUND_COLOR_CYAN    (46)
#define ANSI_CODE_BACKGROUND_COLOR_WHITE   (47)

/**
 * @brief the escape character (ASCII 27)
 * - octal notation: '\033'
 * - Hexadecimal Notation: '\x1b'
 * - Special Escape Sequence (GCC extension): '\e'
 */
#define CHAR_ESCAPE '\033'


static inline const char* ansi_escape_sequence_impl(char* buf, size_t bufsz, ...)
{
    int offset = 0, pc = 0, wc;
    //
    if (!buf || bufsz <= 0)
    {
        return buf;
    }
    memset(buf, 0, bufsz);
    //
    wc = snprintf(buf + offset, bufsz - offset, "%c[", CHAR_ESCAPE);
    if (wc < 0)
    {
        return buf;
    }
    offset += wc;
    //
    va_list ap;
    va_start(ap, bufsz);
    int val = -1;
    do
    {
        val = va_arg(ap, int);
        if (!(val >= 0 && val <= UINT8_MAX))
        {
            break;
        }
        {
            //
            wc = snprintf(buf + offset, bufsz - offset, pc ? ";%d" : "%d", val);
            if (wc < 0)
            {
                break;
            }
            offset += wc;
        }
        ++pc;
    } while (1);
    va_end(ap);

    // reset
    if (pc == 0)
    {
        //
        wc = snprintf(buf + offset, bufsz - offset, "%d", ANSI_CODE_RESET);
        if (wc < 0)
        {
            return buf;
        }
        offset += wc;
    }

    {
        //
        wc = snprintf(buf + offset, bufsz - offset, "%c", 'm');
        if (wc < 0)
        {
            return buf;
        }
        offset += wc;
    }

    return buf;
}
#define ansi_escape_sequence(...) ansi_escape_sequence_impl((char*)__builtin_alloca(64), 64, ##__VA_ARGS__, -1)

#define ANSI_RESET ansi_escape_sequence(ANSI_CODE_RESET)

#define ANSI_BLACK   ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_BLACK)
#define ANSI_RED     ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_RED)
#define ANSI_GREEN   ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_GREEN)
#define ANSI_YELLOW  ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_YELLOW)
#define ANSI_BLUE    ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_BLUE)
#define ANSI_MAGENTA ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_MAGENTA)
#define ANSI_CYAN    ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_CYAN)
#define ANSI_WHITE   ansi_escape_sequence(ANSI_CODE_FOREGROUND_COLOR_WHITE)

#define ANSI_FAINT_BLACK   ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_BLACK)
#define ANSI_FAINT_RED     ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_RED)
#define ANSI_FAINT_GREEN   ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_GREEN)
#define ANSI_FAINT_YELLOW  ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_YELLOW)
#define ANSI_FAINT_BLUE    ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_BLUE)
#define ANSI_FAINT_MAGENTA ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_MAGENTA)
#define ANSI_FAINT_CYAN    ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_CYAN)
#define ANSI_FAINT_WHITE   ansi_escape_sequence(ANSI_CODE_FAINT, ANSI_CODE_FOREGROUND_COLOR_WHITE)

#define ANSI_BRIGHT_BLACK   ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_BLACK)
#define ANSI_BRIGHT_RED     ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_RED)
#define ANSI_BRIGHT_GREEN   ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_GREEN)
#define ANSI_BRIGHT_YELLOW  ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_YELLOW)
#define ANSI_BRIGHT_BLUE    ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_BLUE)
#define ANSI_BRIGHT_MAGENTA ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_MAGENTA)
#define ANSI_BRIGHT_CYAN    ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_CYAN)
#define ANSI_BRIGHT_WHITE   ansi_escape_sequence(ANSI_CODE_BRIGHT, ANSI_CODE_FOREGROUND_COLOR_WHITE)

#define ANSI_UNDERLINE_BLACK   ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_BLACK)
#define ANSI_UNDERLINE_RED     ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_RED)
#define ANSI_UNDERLINE_GREEN   ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_GREEN)
#define ANSI_UNDERLINE_YELLOW  ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_YELLOW)
#define ANSI_UNDERLINE_BLUE    ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_BLUE)
#define ANSI_UNDERLINE_MAGENTA ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_MAGENTA)
#define ANSI_UNDERLINE_CYAN    ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_CYAN)
#define ANSI_UNDERLINE_WHITE   ansi_escape_sequence(ANSI_CODE_UNDERLINE, ANSI_CODE_FOREGROUND_COLOR_WHITE)

#define ANSI_BACKGROUND_BLACK   ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_BLACK)
#define ANSI_BACKGROUND_RED     ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_RED)
#define ANSI_BACKGROUND_GREEN   ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_GREEN)
#define ANSI_BACKGROUND_YELLOW  ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_YELLOW)
#define ANSI_BACKGROUND_BLUE    ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_BLUE)
#define ANSI_BACKGROUND_MAGENTA ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_MAGENTA)
#define ANSI_BACKGROUND_CYAN    ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_CYAN)
#define ANSI_BACKGROUND_WHITE   ansi_escape_sequence(ANSI_CODE_BACKGROUND_COLOR_WHITE)



#endif  // __ANSI_COLOR_H__
