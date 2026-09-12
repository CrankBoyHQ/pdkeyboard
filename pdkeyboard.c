/*

Copyright (c) 2026 Sodium Hydroxide

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pd_api.h"
#include "pdkeyboard.h"

static PlaydateAPI* pd;
static LCDFont* font;
static bool is_jp;

const static uint8_t lcd50[16] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static unsigned char backspace_data[] = {
    0xFF, 0xFF, 0xFC, 0x00, 0xF8, 0x00, 0xF3, 0xFC,
    0xE6, 0xFE, 0xCE, 0x72, 0x9F, 0x26, 0x3F, 0x8E,
    0x3F, 0x1E, 0x9E, 0x4E, 0xCC, 0xE6, 0xE7, 0xF6,
    0xF3, 0xFC, 0xF8, 0x00, 0xFC, 0x00, 0xFF, 0xFF
};
static unsigned backspace_width = 16;
static unsigned backspace_height = 16;
static LCDBitmap* backspace_img;

static unsigned char confirm_data[] = {
    0xFF, 0xFF, 0xFF, 0xF9, 0xFF, 0xF1, 0xFF, 0xF3,
    0xFF, 0xE7, 0xFF, 0xC7, 0xFF, 0xCF, 0xFF, 0x9F,
    0xFF, 0x9F, 0xFF, 0x3F, 0xFF, 0x3F, 0x9E, 0x7F,
    0x86, 0x7F, 0xE0, 0x7F, 0xF8, 0xFF, 0xFC, 0xFF
};
static unsigned confirm_width = 16;
static unsigned confirm_height = 16;
static unsigned char confirm_jp_data[] = {
    0xF8, 0x1F, 0xE0, 0x07, 0xC7, 0xE3, 0x9F, 0xF9,
    0x9E, 0x79, 0x3F, 0x3C, 0x3F, 0x9C, 0x30, 0x0C,
    0x30, 0x0C, 0x3F, 0x9C, 0x3F, 0x3C, 0x9E, 0x79,
    0x9F, 0xF9, 0xC7, 0xE3, 0xE0, 0x07, 0xF8, 0x1F
};
static unsigned confirm_jp_width = 16;
static unsigned confirm_jp_height = 16;
static LCDBitmap* confirm_img;

static unsigned char switch_data[] = {
    0xFF, 0xFF, 0xE7, 0xFF, 0xF3, 0xFF, 0x00, 0x7F,
    0x9C, 0xFF, 0x9C, 0xC3, 0xCD, 0x99, 0xE3, 0x99,
    0xF3, 0x99, 0xC9, 0x00, 0x9D, 0x3C, 0xFF, 0x3C,
    0xFF, 0x3C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned switch_width = 16;
static unsigned switch_height = 16;
static LCDBitmap* switch_img;

static unsigned char dakuten_data[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFE, 0x1F, 0xFF, 0x9F, 0xF8, 0xFF, 0xFC, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned dakuten_width = 16;
static unsigned dakuten_height = 16;
static LCDBitmap* dakuten_img;

static unsigned char handakuten_data[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x3F,
    0xFC, 0x9F, 0xFD, 0xDF, 0xFC, 0x9F, 0xFE, 0x3F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned handakuten_width = 16;
static unsigned handakuten_height = 16;
static LCDBitmap* handakuten_img;

static unsigned char shrink_data[] = {
    0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F,
    0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F,
    0xFE, 0x7F, 0xDE, 0x7B, 0xCE, 0x73, 0xE6, 0x67,
    0xF2, 0x4F, 0xF8, 0x1F, 0xFC, 0x3F, 0xFE, 0x7F
};
static unsigned shrink_width = 16;
static unsigned shrink_height = 16;
static LCDBitmap* shrink_img;

static unsigned char upper_data[] = {
    0xFE, 0x7F, 0xFC, 0x3F, 0xF8, 0x1F, 0xF2, 0x4F,
    0xE6, 0x67, 0xCE, 0x73, 0xDE, 0x7B, 0xFE, 0x7F,
    0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F,
    0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F
};
static unsigned upper_width = 16;
static unsigned upper_height = 16;
static LCDBitmap* upper_img;

static unsigned char upper_latch_data[] = {
    0xFE, 0x7F, 0xFC, 0x3F, 0xF9, 0x9F, 0xF3, 0xCF,
    0xE7, 0xE7, 0xCF, 0xF3, 0x99, 0x99, 0xB1, 0x8D,
    0xA1, 0x85, 0x89, 0x91, 0x99, 0x99, 0xF9, 0x9F,
    0xF9, 0x9F, 0xF9, 0x9F, 0xF9, 0x9F, 0xF8, 0x1F
};
static unsigned upper_latch_width = 16;
static unsigned upper_latch_height = 16;
static LCDBitmap* upper_latch_img;

static LCDBitmap* _pdkb_load_image(const uint8_t* data, int width, int height)
{
    LCDBitmap* bm = pd->graphics->newBitmap(width, height, kColorBlack);
    if (!bm) return NULL;

    int rowbytes = 0;
    uint8_t* mask = NULL;
    uint8_t* pixels = NULL;
    pd->graphics->getBitmapData(bm, NULL, NULL, &rowbytes, &mask, &pixels);

    for (int y = 0; y < height; ++y)
    {
        int src_off = y * ((width + 7) / 8);
        int dst_off = y * rowbytes;
        int copy = rowbytes;
        if (copy > (width + 7) / 8)
            copy = (width + 7) / 8;
        memcpy(pixels + dst_off, data + src_off, copy);
    }

    return bm;
}

enum PDKBPageType {
    PDKBP_None = 0,
    PDKBP_Latin,
    PDKBP_Numeric,
    PDKBP_Symbols,
    PDKBP_JA_Hiragana,
    PDKBP_JA_Katakana,
    PDKBP_COUNT
};

#define MAXROWS 6
#define MAXROWLEN 40

#define KEY_FILLER 1
#define KEY_SWITCH 2
#define KEY_EXTEND 3
#define KEY_CONFIRM 4
#define KEY_DAKUTEN 5
#define KEY_HANDAKUTEN 6
#define KEY_UPPER 7
#define KEY_BACKSPACE 8
#define KEY_SHRINK 9

#define LONGPRESS_TIME (0.35f)
#define DAS_ONSET_TIME (0.2f)
#define DAS_INTERVAL (0.05f)

typedef struct PDKeyboardPage {
    enum PDKBPageType type;

    size_t rows_c;
    uint32_t rows[MAXROWS][MAXROWLEN];
    uint32_t* rows_longpress[MAXROWS][MAXROWLEN];

} PDKeyboardPage;

typedef struct PDKeyboard {
    bool is_open;
    int result;
    PDKeyboardFlags flags;
    char* content;
    size_t content_capacity;
    size_t content_max_bytes; // 0: unrestrained
    size_t content_max_codepoints; // 0: unrestrained
    size_t current_byte;
    size_t current_codepoint;
    PDKeyboardPage pages[PDKBP_COUNT];
    size_t current_page;
    float open_p;
    float animation_duration;
    
    size_t sel_row;
    size_t sel_idx;
    bool sel_down;
    float longpress_t;
    float das_t;
    // negative: unselected
    int longpress_menu_idx;
    int altstate;

    PDStringEncoding encoding;

    bool is_jp_region;
    int shake;
    
    const LCDFont* user_font;
} PDKeyboard;

static size_t _codepoint_next(const char* s, size_t offset, PDStringEncoding encoding);
static size_t _codepoint_prev(const char* s, size_t offset, PDStringEncoding encoding);
static uint32_t _pdkb_apply_dakuten(uint32_t c, bool is_handakuten);
static void _pdkb_draw_key(PDKeyboard* kb, uint32_t key, int kx, int ky, int kw, int kh, bool selected, bool down);
static void _pdkb_type(PDKeyboard* kb, uint32_t key);
static LCDFont* _pdkb_get_font(void);
static LCDFont* _pdkb_font_for(PDKeyboard* kb, uint32_t c);

static size_t _len(uint32_t* l)
{
    if (!l) return 0;
    
    size_t n = 0;
    while (true)
    {
        if (!l[n]) return n;
        ++n;
    }
}

uint32_t* _append(uint32_t* l, uint32_t v)
{
    size_t len = _len(l);
    
    uint32_t* l2 = pd->system->realloc(NULL, (len+2)*sizeof(uint32_t));
    if (!l2) return l;
    
    for (size_t i = 0; i < len; ++i)
    {
        l2[i] = l[i];
    }
    l2[len] = v;
    l2[len+1] = 0;
    
    if (l) pd->system->realloc(l, 0);
    
    return l2;
}

static void _pdkb_lp_append(PDKeyboardPage* page, size_t row, size_t idx, uint32_t key)
{
    page->rows_longpress[row][idx] = _append(page->rows_longpress[row][idx], key);
}

static void _pdkb_lp_free(PDKeyboardPage* page)
{
    for (size_t i = 0; i < MAXROWS; ++i)
    {
        for (size_t j = 0; j < MAXROWLEN; ++j)
        {
            if (page->rows_longpress[i][j])
            {
                pd->system->realloc(page->rows_longpress[i][j], 0);
                page->rows_longpress[i][j] = NULL;
            }
        }
    }
}

static uint32_t _pdkb_apply_sutegana(uint32_t c);
static uint32_t _pdkb_apply_altstate(uint32_t key)
{
    uint32_t alt = _pdkb_apply_sutegana(key);
    if (alt != key) return alt;
    
    if (key < 0x80) return (uint32_t)toupper((int)key);

    return key;
}

static size_t _codepoint_next(const char* s, size_t offset, PDStringEncoding encoding)
{
    size_t len = strlen(s);

    switch (encoding) {
    case kASCIIEncoding:
        if (offset >= len) return offset;
        return offset + 1;

    case kUTF8Encoding:
        if (offset >= len) return offset;
        switch ((unsigned char)s[offset] & 0xF0) {
        case 0x00: case 0x10: case 0x20: case 0x30:
        case 0x40: case 0x50: case 0x60: case 0x70:
            return offset + 1;
        case 0xC0: case 0xD0:
            return offset + 2;
        case 0xE0:
            return offset + 3;
        default:
            return offset + 4;
        }

    case k16BitLEEncoding:
        if (offset >= len) return offset;
        {
            unsigned short cp = (unsigned short)((unsigned char)s[offset])
                    | ((unsigned short)((unsigned char)s[offset + 1]) << 8);
            if (cp >= 0xD800 && cp < 0xDC00 && offset + 3 < len)
                return offset + 4;
            return offset + 2;
        }

    default:
        return offset;
    }
}

static size_t _codepoint_prev(const char* s, size_t offset, PDStringEncoding encoding)
{
    if (offset == 0) return 0;

    switch (encoding) {
    case kASCIIEncoding:
        return offset - 1;

    case kUTF8Encoding:
        {
            size_t pos = offset - 1;
            while (pos > 0 && ((unsigned char)s[pos] & 0xC0) == 0x80)
                pos--;
            return pos;
        }

    case k16BitLEEncoding:
        if (offset <= 2) return 0;
        {
            size_t pos = offset - 2;
            unsigned short unit = (unsigned short)((unsigned char)s[pos])
                    | ((unsigned short)((unsigned char)s[pos + 1]) << 8);
            if (unit >= 0xDC00 && unit < 0xE000)
                pos -= 2;
            return pos;
        }

    default:
        return offset;
    }
}

// returns bytes of encoding output
static size_t _encode(uint32_t codepoint, PDStringEncoding encoding, char* o)
{
    switch (encoding)
    {
    case kASCIIEncoding:
        if (codepoint >= 0x80) return 0;
        if (o)
        {
            *o = (char)(codepoint & 0x7F);
        }
        return 1;

    case kUTF8Encoding:
        if (codepoint < 0x80) {
            if (o) o[0] = (char)codepoint;
            return 1;
        } else if (codepoint < 0x800) {
            if (o) {
                o[0] = (char)(0xC0 | (codepoint >> 6));
                o[1] = (char)(0x80 | (codepoint & 0x3F));
            }
            return 2;
        } else if (codepoint < 0x10000) {
            if (o) {
                o[0] = (char)(0xE0 | (codepoint >> 12));
                o[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                o[2] = (char)(0x80 | (codepoint & 0x3F));
            }
            return 3;
        } else {
            if (o) {
                o[0] = (char)(0xF0 | (codepoint >> 18));
                o[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
                o[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                o[3] = (char)(0x80 | (codepoint & 0x3F));
            }
            return 4;
        }

    case k16BitLEEncoding:
        if (codepoint < 0x10000) {
            if (o) {
                o[0] = (char)(codepoint & 0xFF);
                o[1] = (char)((codepoint >> 8) & 0xFF);
            }
            return 2;
        } else {
            uint32_t off = codepoint - 0x10000;
            uint16_t high = (uint16_t)(0xD800 + (off >> 10));
            uint16_t low  = (uint16_t)(0xDC00 + (off & 0x3FF));
            if (o) {
                o[0] = (char)(high & 0xFF);
                o[1] = (char)((high >> 8) & 0xFF);
                o[2] = (char)(low & 0xFF);
                o[3] = (char)((low >> 8) & 0xFF);
            }
            return 4;
        }

    default:
        return 0;
    }
}

// returns 0 on failure
static uint32_t _decode(const char* s, PDStringEncoding encoding, size_t* size)
{
    if (size) *size = 0;

    switch (encoding)
    {
    case kASCIIEncoding:
        if (size) *size = 1;
        return (uint32_t)(unsigned char)s[0];

    case kUTF8Encoding:
        {
            unsigned char c = (unsigned char)s[0];
            if (c < 0x80)
            {
                if (size) *size = 1;
                return c;
            }
            if (c < 0xE0)
            {
                if (size) *size = 2;
                return ((uint32_t)(c & 0x1F) << 6) | ((uint32_t)(unsigned char)s[1] & 0x3F);
            }
            if (c < 0xF0)
            {
                if (size) *size = 3;
                return ((uint32_t)(c & 0x0F) << 12)
                    | ((uint32_t)(unsigned char)s[1] & 0x3F) << 6
                    | ((uint32_t)(unsigned char)s[2] & 0x3F);
            }
            if (size) *size = 4;
            return ((uint32_t)(c & 0x07) << 18)
                | ((uint32_t)(unsigned char)s[1] & 0x3F) << 12
                | ((uint32_t)(unsigned char)s[2] & 0x3F) << 6
                | ((uint32_t)(unsigned char)s[3] & 0x3F);
        }

    case k16BitLEEncoding:
        {
            uint16_t hi = (uint16_t)(unsigned char)s[0] | ((uint16_t)(unsigned char)s[1] << 8);
            if (hi >= 0xD800 && hi < 0xDC00)
            {
                uint16_t lo = (uint16_t)(unsigned char)s[2] | ((uint16_t)(unsigned char)s[3] << 8);
                if (size) *size = 4;
                return 0x10000 + ((uint32_t)(hi - 0xD800) << 10) + (lo - 0xDC00);
            }
            if (size) *size = 2;
            return hi;
        }

    default:
        return 0;
    }
}

static void _pdkb_ensure_capacity(PDKeyboard* kb, size_t mincapacity)
{
    if (mincapacity > kb->content_capacity)
    {
        size_t capacity = 1;
        while (capacity < mincapacity)
            capacity <<= 1;

        char* new_content = pd->system->realloc(kb->content, capacity);
        if (!new_content)
            return;

        kb->content = new_content;
        kb->content_capacity = capacity;
    }
}

static void _pdkb_update_anim(PDKeyboard* kb, float dt)
{
    float step;
    if (kb->animation_duration <= 0.0f)
    {
        step = 1.0f;
    }
    else
    {
        step = dt / kb->animation_duration;
    }

    if (kb->is_open) {
        kb->open_p += step;
        if (kb->open_p > 1.0f) kb->open_p = 1.0f;
    } else {
        kb->open_p -= step;
        if (kb->open_p < 0.0f) kb->open_p = 0.0f;
    }
}

static size_t _pdkb_count_row_keys(const PDKeyboardPage* page, size_t row)
{
    size_t count = 0;
    for (size_t i = 0; i < MAXROWLEN; ++i)
    {
        if (page->rows[row][i]) ++count;
    }

    return count;
}

static uint32_t _pdkb_count_row_get_key(const PDKeyboardPage* page, size_t row, size_t idx)
{
    for (size_t i = 0; i < MAXROWLEN; ++i)
    {
        if (page->rows[row][i])
        {
            if (idx-- == 0) return page->rows[row][i];
        }
    }

    return 0;
}

static int _pdkb_get_key_width(const PDKeyboardPage* page, size_t row)
{
    size_t keys = _pdkb_count_row_keys(page, row);
    if (keys == 0) return LCD_COLUMNS;
    return LCD_COLUMNS / (int)keys;
}

static int _pdkb_get_key_height(const PDKeyboardPage* page)
{
    if (page->rows_c == 0) return 1;
    unsigned size = (120 - 2) / page->rows_c;
    if (size > 60) return 60;
    return (int)size;
}

static int _pdb_get_key_offy(const PDKeyboardPage* page)
{
    return 2 + (118 % _pdkb_get_key_height(page)) / 2;
}

static int _pdkb_get_key_offx(const PDKeyboardPage* page, size_t row)
{
    return (LCD_COLUMNS % _pdkb_get_key_width(page, row)) / 2;
}

static uint32_t _pdkb_apply_sutegana(uint32_t c)
{
    switch (c)
    {
    case U'あ': return U'ぁ'; case U'い': return U'ぃ'; case U'う': return U'ぅ';
    case U'え': return U'ぇ'; case U'お': return U'ぉ';
    case U'か': return U'ゕ'; case U'け': return U'ゖ';
    case U'つ': return U'っ';
    case U'や': return U'ゃ'; case U'ゆ': return U'ゅ'; case U'よ': return U'ょ';

    case U'ア': return U'ァ'; case U'イ': return U'ィ'; case U'ウ': return U'ゥ';
    case U'エ': return U'ェ'; case U'オ': return U'ォ';
    case U'カ': return U'ヵ'; case U'ケ': return U'ヶ';
    case U'ツ': return U'ッ';
    case U'ヤ': return U'ャ'; case U'ユ': return U'ュ'; case U'ヨ': return U'ョ';
    default: return c;
    }
}

static uint32_t _pdkb_apply_dakuten(uint32_t c, bool is_handakuten)
{
    if (is_handakuten)
    {
        switch (c)
        {
        case U'は': return U'ぱ';
        case U'ひ': return U'ぴ';
        case U'ふ': return U'ぷ';
        case U'へ': return U'ぺ';
        case U'ほ': return U'ぽ';
        case U'ハ': return U'パ';
        case U'ヒ': return U'ピ';
        case U'フ': return U'プ';
        case U'ヘ': return U'ペ';
        case U'ホ': return U'ポ';
        default: return c;
        }
    }
    else
    {
        switch (c)
        {
        case U'か': return U'が'; case U'き': return U'ぎ'; case U'く': return U'ぐ';
        case U'け': return U'げ'; case U'こ': return U'ご';
        case U'さ': return U'ざ'; case U'し': return U'じ'; case U'す': return U'ず';
        case U'せ': return U'ぜ'; case U'そ': return U'ぞ';
        case U'た': return U'だ'; case U'ち': return U'ぢ'; case U'つ': return U'づ';
        case U'て': return U'で'; case U'と': return U'ど';
        case U'は': return U'ば'; case U'ひ': return U'び'; case U'ふ': return U'ぶ';
        case U'へ': return U'べ'; case U'ほ': return U'ぼ';
        case U'カ': return U'ガ'; case U'キ': return U'ギ'; case U'ク': return U'グ';
        case U'ケ': return U'ゲ'; case U'コ': return U'ゴ';
        case U'サ': return U'ザ'; case U'シ': return U'ジ'; case U'ス': return U'ズ';
        case U'セ': return U'ゼ'; case U'ソ': return U'ゾ';
        case U'タ': return U'ダ'; case U'チ': return U'ヂ'; case U'ツ': return U'ヅ';
        case U'テ': return U'デ'; case U'ト': return U'ド';
        case U'ハ': return U'バ'; case U'ヒ': return U'ビ'; case U'フ': return U'ブ';
        case U'ヘ': return U'ベ'; case U'ホ': return U'ボ';
        case U'ウ': return U'ヴ'; case U'ワ': return U'ヷ'; case U'ヲ': return U'ヺ';
        case U'ヰ': return U'ヸ'; case U'ヱ': return U'ヹ';
        case U'ゝ': return U'ゞ';
        case U'ヽ': return U'ヾ';
        default: return c;
        }
    }
}

static uint32_t _pdkb_swap_kana(uint32_t c)
{
    if (c >= U'ぁ' && c <= U'ゖ') return c + 0x60;
    if (c >= U'ァ' && c <= U'ヶ') return c - 0x60;
    
    return c;
}

static LCDBitmap* _pdkb_key_image(PDKeyboard* kb, uint32_t key)
{
    LCDBitmap* img = NULL;

    #define CACHEIMG(OUT, VAR, NAME) \
        if (!VAR##_img) VAR##_img = _pdkb_load_image(NAME##_data, NAME##_width, NAME##_height); \
        OUT = VAR##_img;

    switch(key)
    {
    case KEY_BACKSPACE:
        CACHEIMG(img, backspace, backspace);
        break;
    case KEY_CONFIRM:
        if (kb->is_jp_region)
        {
            CACHEIMG(img, confirm, confirm_jp);
        }
        else
        {
            CACHEIMG(img, confirm, confirm);
        }
        break;
    case KEY_SWITCH:
        CACHEIMG(img, switch, switch)
        break;
    case KEY_DAKUTEN:
        CACHEIMG(img, dakuten, dakuten);
        break;
    case KEY_HANDAKUTEN:
        CACHEIMG(img, handakuten, handakuten);
        break;
    case KEY_SHRINK:
        CACHEIMG(img, shrink, shrink);
        break;
    case KEY_UPPER:
        if (kb->altstate == 2)
        {
            CACHEIMG(img, upper_latch, upper_latch);
        }
        else
        {
            CACHEIMG(img, upper, upper);
        }
        break;
    default:
        break;
    }

    #undef CACHEIMG

    return img;
}

static void _pdkb_draw_key(PDKeyboard* kb, uint32_t key, int kx, int ky, int kw, int kh, bool selected, bool down)
{
    int r = 12;
    int margin = selected ? 2 : 1;
    if (down) margin += 1;
    kx += margin;
    ky += margin;
    kw -= 2 * margin;
    kh -= 2 * margin;

    LCDColor fill = kColorWhite;
    pd->graphics->fillRoundRect(kx, ky, kw, kh, r, fill);
    pd->graphics->drawRoundRect(kx, ky, kw, kh, r, selected ? 2 : 1, kColorBlack);

    if (key < 0x20)
    {
        LCDBitmap* img = _pdkb_key_image(kb, key);

        if (img)
        {
            int w, h;
            pd->graphics->getBitmapData(img, &w, &h, NULL, NULL, NULL);
            pd->graphics->drawBitmap(img, kx + kw/2 - w/2, ky + kh/2 - h/2, kBitmapUnflipped);
        }
    }
    else
    {
        // standard unicode
        char buff[5];
        memset(buff, 0, sizeof(buff));
        
        uint32_t orgkey = key;
        
        if (kb->altstate)
        {
            key = _pdkb_apply_altstate(key);
        }

        _encode(key, kUTF8Encoding, buff);

        LCDFont* f = _pdkb_font_for(kb, key);
        pd->graphics->setFont(f);
        int textwidth = pd->graphics->getTextWidth(f, buff, 1, kUTF8Encoding, 0);
        pd->graphics->drawText(buff, 1, kUTF8Encoding, kx + kw/2 - textwidth/2, ky + kh/2-7);
    }
    
    if (selected)
    {
        pd->graphics->fillRoundRect(kx, ky, kw, kh, r, kColorXOR);
    }
}

static LCDFont* _pdkb_get_font(void)
{
    if (font) return font;

    font = pd->graphics->loadFont("/System/Fonts/Asheville-Sans-14-Bold", NULL);
    if (!font)
    {
        pd->system->error("Failed to load system font");
    }
    return font;
}

static LCDFont* _pdkb_font_for(PDKeyboard* kb, uint32_t c)
{
    if (kb->user_font)
    {
        LCDFont* uf = (LCDFont*)kb->user_font;
        LCDFontPage* fp = pd->graphics->getFontPage(uf, c);
        if (fp && pd->graphics->getPageGlyph(fp, c, NULL, NULL))
        {
            return uf;
        }
    }

    return _pdkb_get_font();
}

static uint32_t _pdkb_get_real_longpress_key(PDKeyboard* kb, uint32_t lp, uint32_t basekey)
{
    if (lp == KEY_UPPER)
    {
        if (!kb->altstate)
            lp = toupper(basekey);
        else
            lp = basekey;
    }
    
    if (lp == KEY_SHRINK)
    {
        if (!kb->altstate)
            lp = _pdkb_apply_sutegana(basekey);
        else
            lp = basekey;
    }
    
    if (lp == KEY_DAKUTEN)
    {
        lp = _pdkb_apply_dakuten(basekey, false);
    }
    
    if (lp == KEY_HANDAKUTEN)
    {
        lp = _pdkb_apply_dakuten(basekey, true);
    }
    
    if (lp < 0x20) return basekey;
    
    return lp;
}

// can k be produced by some key on some page (directly or via a modifier)?
static bool _pdkb_key_reachable(PDKeyboard* kb, uint32_t k)
{
    // the case a page does not store is only reachable through KEY_UPPER,
    // which is only placed when both latin cases are enabled
    const bool can_change_case =
        (kb->flags & (PDKBF_LATIN_UPPERCASE | PDKBF_LATIN_LOWERCASE))
            == (PDKBF_LATIN_UPPERCASE | PDKBF_LATIN_LOWERCASE);

    for (size_t p = 0; p < PDKBP_COUNT; ++p)
    {
        const PDKeyboardPage* page = &kb->pages[p];
        if (page->type == PDKBP_None) continue;

        for (size_t i = 0; i < MAXROWS; ++i)
        {
            for (size_t j = 0; j < MAXROWLEN; ++j)
            {
                uint32_t key = page->rows[i][j];
                if (!key) continue;

                if (key == k) return true;

                // case variations
                if (can_change_case && key < 0x80)
                {
                    if ((uint32_t)toupper((int)key) == k) return true;
                    if ((uint32_t)tolower((int)key) == k) return true;
                }

                // kana modifiers
                if (_pdkb_apply_sutegana(key) == k) return true;
                if (_pdkb_apply_dakuten(key, false) == k) return true;
                if (_pdkb_apply_dakuten(key, true) == k) return true;

                // longpress entries
                const uint32_t* lp = page->rows_longpress[i][j];
                for (size_t n = 0; lp && lp[n]; ++n)
                {
                    if (lp[n] == k) return true;
                }
            }
        }
    }

    return false;
}

int pdkb_on_serial(PDKeyboard* kb, const char* c)
{
    if (!c || c[0] != 'p' || c[1] != 'd' || c[2] != 'k' || c[3] != 'b' || c[4] != ':')
    {
        return 0;
    }
    
    // "pdkb:<codepoint in decimal>"
    const char* num = c + 5;
    char* end = NULL;
    unsigned long k = strtoul(num, &end, 10);

    if (end == num || k == 0 || k > 0x10FFFF)
    {
        return 0;
    }

    if (kb)
    {
        if (_pdkb_key_reachable(kb, (uint32_t)k))
        {
            _pdkb_type(kb, (uint32_t)k);
        }
        else
        {
            kb->shake = 10;
        }
    }

    return 1;
}

static void _pdkb_draw(PDKeyboard* kb)
{
    int y = pdkb_get_visible_y(kb);
    int shake_offset = 0;
    if (kb->shake > 0)
    {
        shake_offset = (kb->shake % 2 == 0) ? -2 : 2;
    }

    pd->graphics->fillRect(0, y, 400, 240 - y, (uintptr_t)(const void*)lcd50);
    pd->graphics->drawLine(0, y, 400, y, 1, kColorBlack);
    pd->graphics->drawLine(0, y + 1, 400, y + 1, 1, kColorWhite);

    PDKeyboardPage* page = &kb->pages[kb->current_page];
    if (page->type == PDKBP_None) return;

    int ky = _pdb_get_key_offy(page) + pdkb_get_visible_y(kb);
    int row_h = _pdkb_get_key_height(page);

    pd->graphics->setFont(_pdkb_get_font());

    for (size_t i = 0; i < page->rows_c; ++i)
    {
        int kx = _pdkb_get_key_offx(page, i) + shake_offset;
        int kw = _pdkb_get_key_width(page, i);
        size_t k = _pdkb_count_row_keys(page, i);

        for (size_t j = 0; j < k;)
        {
            uint32_t key = _pdkb_count_row_get_key(page, i, j);

            size_t jw = 1;
            while (_pdkb_count_row_get_key(page, i, j + jw) == KEY_EXTEND) ++jw;
            bool sel = i == kb->sel_row && j == kb->sel_idx;
            if (key != KEY_FILLER)
            {
                _pdkb_draw_key(kb, key, kx, ky, (int)(kw * jw), row_h, sel, sel && kb->sel_down && (kb->longpress_t >= 0 || kb->longpress_menu_idx >= 0));
            }
            j += jw;
            kx += kw * jw;
        }

        ky += row_h;

        if (ky >= LCD_ROWS) break;
    }

    if (kb->longpress_menu_idx >= 0)
    {
        int r = 10;
        int kw = _pdkb_get_key_width(page, kb->sel_row);
        int ky = pdkb_get_visible_y(kb) + _pdb_get_key_offy(page) + row_h*(kb->sel_row);
        int kx = _pdkb_get_key_offx(page, kb->sel_row) + shake_offset + kw*(kb->sel_idx + 0.5f);

        uint32_t* lp = kb->pages[kb->current_page].rows_longpress[kb->sel_row][kb->sel_idx];

        size_t count = _len(lp);
        int lw = 18;
        if (kb->current_page == PDKBP_JA_Katakana || kb->current_page == PDKBP_JA_Hiragana)
        {
            // kana need a bit more width :s ugly hack sorry
            lw += 5;
        }
        int lh = 24;
        int lm = 0;
        int tt = 6;

        int lx = kx - count*lw/2;
        int ly = ky - lh - lm;
        
        if (lx < 0) lx = 0;
        if (lx + lw*count >= LCD_COLUMNS) lx = LCD_COLUMNS - lw*count;

        int t = 3;
        pd->graphics->fillTriangle(kx - tt - t, ly + lh, kx + tt + t, ly + lh, kx, ly + lh + tt + t, kColorBlack);
        pd->graphics->fillRoundRect(lx, ly, lw * count, lh, r, kColorWhite);
        pd->graphics->drawRoundRect(lx, ly, lw * count, lh, r, 2, kColorBlack);
        pd->graphics->fillTriangle(kx - tt - 4, ly + lh - 4, kx + tt + 4, ly + lh - 4, kx, ly + lh + tt, kColorWhite);
        
        for (size_t i = 0; i < count; ++i)
        {
            uint32_t key = _pdkb_get_real_longpress_key(kb, lp[i], kb->pages[kb->current_page].rows[kb->sel_row][kb->sel_idx]);
            int ix = lx + (int)i * lw;
            bool sel = ((int)i == kb->longpress_menu_idx);

            if (sel)
            {
                pd->graphics->fillRoundRect(ix, ly, lw, lh, r, kColorBlack);
            }

            char buff[5];
            memset(buff, 0, sizeof(buff));
            _encode(key, kUTF8Encoding, buff);

            LCDFont* f = _pdkb_font_for(kb, key);
            pd->graphics->setFont(f);
            int textwidth = pd->graphics->getTextWidth(f, buff, 1, kUTF8Encoding, 0);
            pd->graphics->setDrawMode(sel ? kDrawModeInverted : kDrawModeCopy);
            pd->graphics->drawText(buff, 1, kUTF8Encoding, ix + lw/2 - textwidth/2, ly + lh/2 - 9);
            pd->graphics->setDrawMode(kDrawModeCopy);
        }
    }
}

static void _pdkb_page_advance(PDKeyboard* kb)
{
    kb->altstate = 0;
    kb->longpress_t = -1;
    kb->longpress_menu_idx = -1;
    
    size_t start = kb->current_page;
    for (int i = 0; i < PDKBP_COUNT; ++i)
    {
        size_t p = (start + 1 + i) % PDKBP_COUNT;
        if (kb->pages[p].type != PDKBP_None)
        {
            kb->current_page = p;
            kb->sel_row = 0;
            kb->sel_idx = 0;
            return;
        }
    }
}

static void _pdkb_type(PDKeyboard* kb, uint32_t key)
{
    if (key >= 0x20)
    {
        char encoded[5];
        uint32_t orgkey = key;
        if (kb->altstate)
        {
            key = _pdkb_apply_altstate(key);
        }
        size_t elen = _encode(key, kb->encoding, encoded);
        if (elen > 0)
        {
            if (kb->content_max_codepoints > 0 &&
                kb->current_codepoint >= kb->content_max_codepoints
            )
            {
                kb->shake = 10;
            }
            else if (kb->content_max_bytes > 0 && kb->current_byte + elen > kb->content_max_bytes)
            {
                kb->shake = 10;
            }
            else
            {
                _pdkb_ensure_capacity(kb, kb->current_byte + elen + 1);
                memcpy(kb->content + kb->current_byte, encoded, elen);
                kb->current_byte += elen;
                kb->current_codepoint++;
                kb->content[kb->current_byte] = 0;

                if (key != orgkey && kb->altstate == 1)
                {
                    kb->altstate = 0;
                }
            }
        }
    }
    else if (key == KEY_BACKSPACE)
    {
        if (kb->current_byte > 0)
        {
            size_t prev_byte = _codepoint_prev(kb->content, kb->current_byte, kb->encoding);
            kb->current_byte = prev_byte;
            kb->content[kb->current_byte] = '\0';
            kb->current_codepoint--;
        }
    }
    else if (key == KEY_UPPER)
    {
        kb->altstate = (kb->altstate + 1) % 3;
    }
    else if (key == KEY_SHRINK)
    {
        kb->altstate = !kb->altstate;
    }
    else if (key == KEY_DAKUTEN || key == KEY_HANDAKUTEN)
    {
        if (kb->current_byte == 0 || kb->current_codepoint == 0)
        {
            kb->shake = 8;
        }
        else
        {
            size_t prev_byte = _codepoint_prev(kb->content, kb->current_byte, kb->encoding);

            uint32_t last_cp = _decode(kb->content + prev_byte, kb->encoding, NULL);
            
            uint32_t modified;
            modified = _pdkb_apply_dakuten(last_cp, key == KEY_HANDAKUTEN);

            if (modified && modified != last_cp)
            {
                char encoded[5];
                size_t elen = _encode(modified, kb->encoding, encoded);
                if (elen > 0)
                {
                    _pdkb_ensure_capacity(kb, prev_byte + elen + 1);
                    memcpy(kb->content + prev_byte, encoded, elen);
                    kb->current_byte = prev_byte + elen;
                    kb->content[kb->current_byte] = 0;
                }
            }
            else
            {
                kb->shake = 8;
            }
        }
    }
    else if (key == KEY_CONFIRM)
    {
        kb->result = 1;
        pdkb_close(kb);
    }
    else if (key == KEY_SWITCH)
    {
        _pdkb_page_advance(kb);
        
        // select the switch key
        for (size_t i = 0; i < MAXROWS; ++i)
        {
            for (size_t j = 0; j < MAXROWLEN; ++j)
            {
                if (kb->pages[kb->current_page].rows[i][j] == KEY_SWITCH)
                {
                    kb->sel_row = i;
                    kb->sel_idx = j;
                }
            }
        }
    }
}

void pdkb_update(PDKeyboard* kb, float dt)
{
    _pdkb_update_anim(kb, dt);

    if (kb->shake > 0) --kb->shake;

    if (kb->open_p <= 0) return;

    _pdkb_draw(kb);
    
    if (!kb->is_open || kb->open_p < 0.5f) return;
    
    PDKeyboardPage* page = &kb->pages[kb->current_page];
    
    if (page->type == PDKBP_None)
    {
        _pdkb_page_advance(kb);
        return;
    }
    
    PDButtons down;
    PDButtons pushed;
    PDButtons released;
    pd->system->getButtonState(&down, &pushed, &released);
    
    kb->sel_down = (down & kButtonA);
    
    bool release_longpress = false;
    
    if (!kb->sel_down)
    {
        if (kb->longpress_t < 0)
        {
            release_longpress = true;
        }
        kb->longpress_t = 0;
    }
    else if (kb->longpress_menu_idx < 0 && kb->longpress_t >= 0)
    {
        kb->longpress_t += dt;
        if (kb->longpress_t >= LONGPRESS_TIME)
        {
            if (kb->pages[kb->current_page].rows_longpress[kb->sel_row][kb->sel_idx])
            {
                kb->longpress_menu_idx = 0;
            }
        }
    }
    
    int dx = !!(pushed & kButtonRight) - !!(pushed & kButtonLeft);
    int dy = !!(pushed & kButtonDown) - !!(pushed & kButtonUp);
    
    if (kb->longpress_menu_idx >= 0)
    {        
        if (released & kButtonA)
        {
            uint32_t key = kb->pages[kb->current_page].rows_longpress[kb->sel_row][kb->sel_idx][kb->longpress_menu_idx];
            key = _pdkb_get_real_longpress_key(kb, key, kb->pages[kb->current_page].rows[kb->sel_row][kb->sel_idx]);
            _pdkb_type(kb, key);
            kb->longpress_menu_idx = -1;
            kb->longpress_t = -1;
            return;
        }
        
        if (dy != 0)
        {
            kb->longpress_menu_idx = -1;
            kb->longpress_t = -1;
            return;
        }
        kb->longpress_menu_idx += dx;
        size_t count = _len(kb->pages[kb->current_page].rows_longpress[kb->sel_row][kb->sel_idx]);
        if (kb->longpress_menu_idx >= count)
        {
            kb->longpress_menu_idx = -1;
            kb->longpress_t = -1;
            return;
        }
    }
    else
    {
        if (dx != 0 || dy != 0)
        {
            // move vertically first
            if (dy != 0)
            {
                int new_row = (int)kb->sel_row + dy;
                if (new_row < 0) new_row = (int)page->rows_c - 1;
                if (new_row >= (int)page->rows_c) new_row = 0;

                size_t count = _pdkb_count_row_keys(page, (size_t)new_row);
                if (count > 0)
                {
                    // compute center x of the currently selected key
                    int cur_kw = _pdkb_get_key_width(page, kb->sel_row);
                    int cur_offx = _pdkb_get_key_offx(page, kb->sel_row);
                    size_t cur_jw = 1;
                    while (_pdkb_count_row_get_key(page, kb->sel_row, kb->sel_idx + cur_jw) == KEY_EXTEND) ++cur_jw;
                    int cur_center = cur_offx + (int)kb->sel_idx * cur_kw + cur_kw * (int)cur_jw / 2;

                    // find key in new row whose center x is closest
                    int new_kw = _pdkb_get_key_width(page, (size_t)new_row);
                    int new_offx = _pdkb_get_key_offx(page, (size_t)new_row);
                    size_t best_idx = 0;
                    int best_dist = 0x7fffffff;
                    for (size_t j = 0; j < count;)
                    {
                        uint32_t k = _pdkb_count_row_get_key(page, (size_t)new_row, j);
                        if (k != 0 && k != KEY_FILLER && k != KEY_EXTEND)
                        {
                            size_t jw = 1;
                            while (_pdkb_count_row_get_key(page, (size_t)new_row, j + jw) == KEY_EXTEND) ++jw;
                            int new_center = new_offx + (int)j * new_kw + new_kw * (int)jw / 2;
                            int dist = cur_center - new_center;
                            if (dist < 0) dist = -dist;
                            if (dist < best_dist)
                            {
                                best_dist = dist;
                                best_idx = j;
                            }
                        }
                        // skip to next key
                        size_t skip_jw = 1;
                        while (_pdkb_count_row_get_key(page, (size_t)new_row, j + skip_jw) == KEY_EXTEND) ++skip_jw;
                        j += skip_jw;
                    }

                    kb->sel_row = (size_t)new_row;
                    kb->sel_idx = best_idx;
                }
            }

            // move horizontally
            if (dx != 0)
            {
                size_t count = _pdkb_count_row_keys(page, kb->sel_row);
                if (count > 0)
                {
                    // collect valid indices
                    size_t valid[40];
                    size_t vc = 0;
                    for (size_t j = 0; j < count; ++j)
                    {
                        uint32_t k = _pdkb_count_row_get_key(page, kb->sel_row, j);
                        if (k != 0 && k != KEY_FILLER && k != KEY_EXTEND)
                            valid[vc++] = j;
                    }

                    if (vc > 0)
                    {
                        // find current position in valid list
                        size_t pos = 0;
                        for (size_t v = 0; v < vc; ++v)
                        {
                            if (valid[v] == kb->sel_idx) { pos = v; break; }
                        }

                        if (dx > 0)
                        {
                            pos++;
                            if (pos >= vc) pos = 0;
                        }
                        else
                        {
                            if (pos == 0) pos = vc - 1;
                            else pos--;
                        }

                        kb->sel_idx = valid[pos];
                    }
                }
            }
        }
        
        if ((released & kButtonB) && (kb->flags & PDKBF_CANCEL))
        {
            kb->result = -1;
            kb->is_open = false;
            return;
        }
        
        // typing
        if (released & kButtonA && kb->open_p == 1 && (!release_longpress))
        {
            uint32_t key = _pdkb_count_row_get_key(page, kb->sel_row, kb->sel_idx);

            _pdkb_type(kb, key);
        }
    }
}

PDKeyboard* pdkb_new(PlaydateAPI* _pd, PDKeyboardFlags f, const uint32_t* leftkeys, const uint32_t* rightkeys)
{
    pd = _pd;

    PDKeyboard* kb = _pd->system->realloc(NULL, sizeof(PDKeyboard));

    if (!kb) return NULL;

    memset(kb, 0, sizeof(PDKeyboard));
    kb->flags = f;
    kb->is_jp_region = pd->system->getLanguage() == kPDLanguageJapanese;
    kb->longpress_menu_idx = -1;
    
    int pagec = !!(f&(PDKBF_LATIN_LOWERCASE | PDKBF_LATIN_UPPERCASE))
        +  ((f & PDKBF_SYMBOLS) == PDKBF_SYMBOLS) + !!(f & PDKBF_NUMERIC)
        + !!(f & PDKBF_JA_HIRAGANA) + !!(f & PDKBF_JA_KATAKANA);
        
    int spacekey = ' ';
    if (!(kb->flags & PDKBF_SPACE))
    {
        spacekey = KEY_FILLER;
    }

    const bool has_upper = !!(f & PDKBF_LATIN_UPPERCASE);
    const bool has_lower = !!(f & PDKBF_LATIN_LOWERCASE);
    const bool has_upper_and_lower = (f & (PDKBF_LATIN_UPPERCASE | PDKBF_LATIN_LOWERCASE))
        == (PDKBF_LATIN_UPPERCASE | PDKBF_LATIN_LOWERCASE);

    if ((f & PDKBF_SYMBOLS) == PDKBF_SYMBOLS)
    {
        kb->pages[PDKBP_Symbols].type = PDKBP_Symbols;
        kb->current_page = PDKBP_Symbols;

        kb->pages[PDKBP_Symbols].rows_c = 4;
        
        int r = 0;

        kb->pages[PDKBP_Symbols].rows[r][0] = '+';
        kb->pages[PDKBP_Symbols].rows[r][1] = '-';
        kb->pages[PDKBP_Symbols].rows[r][2] = '=';
        kb->pages[PDKBP_Symbols].rows[r][3] = '@';
        kb->pages[PDKBP_Symbols].rows[r][4] = '#';
        kb->pages[PDKBP_Symbols].rows[r][5] = '$';
        kb->pages[PDKBP_Symbols].rows[r][6] = '%';
        kb->pages[PDKBP_Symbols].rows[r][7] = '^';
        kb->pages[PDKBP_Symbols].rows[r][8] = '&';
        kb->pages[PDKBP_Symbols].rows[r][9] = KEY_BACKSPACE;
        kb->pages[PDKBP_Symbols].rows[r][10] = KEY_EXTEND;

        r++;
        
        kb->pages[PDKBP_Symbols].rows[r][0] = '!';
        kb->pages[PDKBP_Symbols].rows[r][1] = '?';
        kb->pages[PDKBP_Symbols].rows[r][2] = '.';
        kb->pages[PDKBP_Symbols].rows[r][3] = ',';
        kb->pages[PDKBP_Symbols].rows[r][4] = ':';
        kb->pages[PDKBP_Symbols].rows[r][5] = ';';
        kb->pages[PDKBP_Symbols].rows[r][6] = '_';
        kb->pages[PDKBP_Symbols].rows[r][7] = '\'';
        kb->pages[PDKBP_Symbols].rows[r][8] = '"';
        kb->pages[PDKBP_Symbols].rows[r][9] = '*';
        kb->pages[PDKBP_Symbols].rows[r][10] = '~';
        
        r++;

        kb->pages[PDKBP_Symbols].rows[r][0] = '(';
        kb->pages[PDKBP_Symbols].rows[r][1] = ')';
        kb->pages[PDKBP_Symbols].rows[r][2] = '[';
        kb->pages[PDKBP_Symbols].rows[r][3] = ']';
        kb->pages[PDKBP_Symbols].rows[r][4] = '/';
        kb->pages[PDKBP_Symbols].rows[r][5] = '|';
        kb->pages[PDKBP_Symbols].rows[r][6] = '\\';
        kb->pages[PDKBP_Symbols].rows[r][7] = '<';
        kb->pages[PDKBP_Symbols].rows[r][8] = '>';
        kb->pages[PDKBP_Symbols].rows[r][9] = '{';
        kb->pages[PDKBP_Symbols].rows[r][10] = '}';

        size_t ki = 0;
        if (pagec >= 2)
        {
            kb->pages[PDKBP_Symbols].rows[3][ki++] = KEY_SWITCH;
        }

        for (const uint32_t* kv = leftkeys; kv && *kv; ++kv)
        {
            kb->pages[PDKBP_Symbols].rows[3][ki++] = *kv;
        }

        kb->pages[PDKBP_Symbols].rows[3][ki++] = spacekey;
        kb->pages[PDKBP_Symbols].rows[3][ki++] = KEY_EXTEND;
        kb->pages[PDKBP_Symbols].rows[3][ki++] = KEY_EXTEND;
        kb->pages[PDKBP_Symbols].rows[3][ki++] = KEY_EXTEND;
        kb->pages[PDKBP_Symbols].rows[3][ki++] = KEY_EXTEND;

        for (const uint32_t* kv = rightkeys; kv && *kv; ++kv)
        {
            kb->pages[PDKBP_Symbols].rows[3][ki++] = *kv;
        }

        if (kb->flags & PDKBF_CONFIRM)
        {
            kb->pages[PDKBP_Symbols].rows[3][6] = KEY_CONFIRM;
        }
    }

    if (f & PDKBF_NUMERIC)
    {
        kb->pages[PDKBP_Numeric].type = PDKBP_Numeric;
        kb->current_page = PDKBP_Numeric;

        kb->pages[PDKBP_Numeric].rows_c = 4;

        kb->pages[PDKBP_Numeric].rows[0][0] = '9';
        kb->pages[PDKBP_Numeric].rows[0][1] = '8';
        kb->pages[PDKBP_Numeric].rows[0][2] = '7';

        kb->pages[PDKBP_Numeric].rows[1][0] = '6';
        kb->pages[PDKBP_Numeric].rows[1][1] = '5';
        kb->pages[PDKBP_Numeric].rows[1][2] = '4';

        kb->pages[PDKBP_Numeric].rows[2][0] = '3';
        kb->pages[PDKBP_Numeric].rows[2][1] = '2';
        kb->pages[PDKBP_Numeric].rows[2][2] = '1';
        
        const bool syms = !!(kb->flags & PDKBF_LATIN_PUNCTUATION);

        kb->pages[PDKBP_Numeric].rows[3][0] = KEY_FILLER;
        kb->pages[PDKBP_Numeric].rows[3][1] = (syms) ? '-' : KEY_EXTEND;
        kb->pages[PDKBP_Numeric].rows[3][2] = '0';
        kb->pages[PDKBP_Numeric].rows[3][3] = KEY_EXTEND;
        kb->pages[PDKBP_Numeric].rows[3][4] = (syms) ? '.' : KEY_FILLER;
        
        if (pagec >= 2)
        {
            kb->pages[PDKBP_Numeric].rows[3][0] = KEY_SWITCH;
        }

        if (kb->flags & PDKBF_CONFIRM)
        {
            kb->pages[PDKBP_Numeric].rows[3][5] = KEY_CONFIRM;
            if (kb->pages[PDKBP_Numeric].rows[3][4] == KEY_FILLER)
            {
                kb->pages[PDKBP_Numeric].rows[3][4] = KEY_CONFIRM;
                kb->pages[PDKBP_Numeric].rows[3][5] = KEY_EXTEND;
            }
        }
        else
        {
            kb->pages[PDKBP_Numeric].rows[3][5] = KEY_FILLER;
        }
    }

    if (f & PDKBF_JA_KATAKANA)
    {
        kb->pages[PDKBP_JA_Katakana].type = PDKBP_JA_Katakana;
        kb->current_page = PDKBP_JA_Katakana;

        kb->pages[PDKBP_JA_Katakana].rows_c = 5;

        kb->pages[PDKBP_JA_Katakana].rows[0][0] = ' ';
        kb->pages[PDKBP_JA_Katakana].rows[1][0] = U'ー';
        kb->pages[PDKBP_JA_Katakana].rows[2][0] = U'・';
        kb->pages[PDKBP_JA_Katakana].rows[3][0] = KEY_SHRINK;
        kb->pages[PDKBP_JA_Katakana].rows[4][0] = (pagec >= 2) ? KEY_SWITCH : KEY_FILLER;

        kb->pages[PDKBP_JA_Katakana].rows[0][1] = U'ア';
        kb->pages[PDKBP_JA_Katakana].rows[1][1] = U'イ';
        kb->pages[PDKBP_JA_Katakana].rows[2][1] = U'ウ';
        kb->pages[PDKBP_JA_Katakana].rows[3][1] = U'エ';
        kb->pages[PDKBP_JA_Katakana].rows[4][1] = U'オ';

        kb->pages[PDKBP_JA_Katakana].rows[0][2] = U'カ';
        kb->pages[PDKBP_JA_Katakana].rows[1][2] = U'キ';
        kb->pages[PDKBP_JA_Katakana].rows[2][2] = U'ク';
        kb->pages[PDKBP_JA_Katakana].rows[3][2] = U'ケ';
        kb->pages[PDKBP_JA_Katakana].rows[4][2] = U'コ';

        kb->pages[PDKBP_JA_Katakana].rows[0][3] = U'サ';
        kb->pages[PDKBP_JA_Katakana].rows[1][3] = U'シ';
        kb->pages[PDKBP_JA_Katakana].rows[2][3] = U'ス';
        kb->pages[PDKBP_JA_Katakana].rows[3][3] = U'セ';
        kb->pages[PDKBP_JA_Katakana].rows[4][3] = U'ソ';

        kb->pages[PDKBP_JA_Katakana].rows[0][4] = U'タ';
        kb->pages[PDKBP_JA_Katakana].rows[1][4] = U'チ';
        kb->pages[PDKBP_JA_Katakana].rows[2][4] = U'ツ';
        kb->pages[PDKBP_JA_Katakana].rows[3][4] = U'テ';
        kb->pages[PDKBP_JA_Katakana].rows[4][4] = U'ト';

        kb->pages[PDKBP_JA_Katakana].rows[0][5] = U'ナ';
        kb->pages[PDKBP_JA_Katakana].rows[1][5] = U'ニ';
        kb->pages[PDKBP_JA_Katakana].rows[2][5] = U'ヌ';
        kb->pages[PDKBP_JA_Katakana].rows[3][5] = U'ネ';
        kb->pages[PDKBP_JA_Katakana].rows[4][5] = U'ノ';

        kb->pages[PDKBP_JA_Katakana].rows[0][6] = U'ハ';
        kb->pages[PDKBP_JA_Katakana].rows[1][6] = U'ヒ';
        kb->pages[PDKBP_JA_Katakana].rows[2][6] = U'フ';
        kb->pages[PDKBP_JA_Katakana].rows[3][6] = U'ヘ';
        kb->pages[PDKBP_JA_Katakana].rows[4][6] = U'ホ';

        kb->pages[PDKBP_JA_Katakana].rows[0][7] = U'マ';
        kb->pages[PDKBP_JA_Katakana].rows[1][7] = U'ミ';
        kb->pages[PDKBP_JA_Katakana].rows[2][7] = U'ム';
        kb->pages[PDKBP_JA_Katakana].rows[3][7] = U'メ';
        kb->pages[PDKBP_JA_Katakana].rows[4][7] = U'モ';

        kb->pages[PDKBP_JA_Katakana].rows[0][8] = U'ラ';
        kb->pages[PDKBP_JA_Katakana].rows[1][8] = U'リ';
        kb->pages[PDKBP_JA_Katakana].rows[2][8] = U'ル';
        kb->pages[PDKBP_JA_Katakana].rows[3][8] = U'レ';
        kb->pages[PDKBP_JA_Katakana].rows[4][8] = U'ロ';

        kb->pages[PDKBP_JA_Katakana].rows[0][9] = U'ヤ';
        kb->pages[PDKBP_JA_Katakana].rows[1][9] = U'ユ';
        kb->pages[PDKBP_JA_Katakana].rows[2][9] = U'ヨ';
        kb->pages[PDKBP_JA_Katakana].rows[3][9] = U'ワ';
        kb->pages[PDKBP_JA_Katakana].rows[4][9] = U'ヲ';

        kb->pages[PDKBP_JA_Katakana].rows[0][10] = KEY_BACKSPACE;
        kb->pages[PDKBP_JA_Katakana].rows[1][10] = KEY_DAKUTEN;
        kb->pages[PDKBP_JA_Katakana].rows[2][10] = KEY_HANDAKUTEN;
        kb->pages[PDKBP_JA_Katakana].rows[3][10] = U'ん';
        kb->pages[PDKBP_JA_Katakana].rows[4][10] = KEY_CONFIRM;
    }

    if (f & PDKBF_JA_HIRAGANA)
    {
        kb->pages[PDKBP_JA_Hiragana].type = PDKBP_JA_Hiragana;
        kb->current_page = PDKBP_JA_Hiragana;

        kb->pages[PDKBP_JA_Hiragana].rows_c = 5;

        kb->pages[PDKBP_JA_Hiragana].rows[0][0] = ' ';
        kb->pages[PDKBP_JA_Hiragana].rows[1][0] = U'、';
        kb->pages[PDKBP_JA_Hiragana].rows[2][0] = U'。';
        kb->pages[PDKBP_JA_Hiragana].rows[3][0] = KEY_SHRINK;
        kb->pages[PDKBP_JA_Hiragana].rows[4][0] = (pagec >= 2) ? KEY_SWITCH : KEY_FILLER;

        kb->pages[PDKBP_JA_Hiragana].rows[0][1] = U'あ';
        kb->pages[PDKBP_JA_Hiragana].rows[1][1] = U'い';
        kb->pages[PDKBP_JA_Hiragana].rows[2][1] = U'う';
        kb->pages[PDKBP_JA_Hiragana].rows[3][1] = U'え';
        kb->pages[PDKBP_JA_Hiragana].rows[4][1] = U'お';

        kb->pages[PDKBP_JA_Hiragana].rows[0][2] = U'か';
        kb->pages[PDKBP_JA_Hiragana].rows[1][2] = U'き';
        kb->pages[PDKBP_JA_Hiragana].rows[2][2] = U'く';
        kb->pages[PDKBP_JA_Hiragana].rows[3][2] = U'け';
        kb->pages[PDKBP_JA_Hiragana].rows[4][2] = U'こ';

        kb->pages[PDKBP_JA_Hiragana].rows[0][3] = U'さ';
        kb->pages[PDKBP_JA_Hiragana].rows[1][3] = U'し';
        kb->pages[PDKBP_JA_Hiragana].rows[2][3] = U'す';
        kb->pages[PDKBP_JA_Hiragana].rows[3][3] = U'せ';
        kb->pages[PDKBP_JA_Hiragana].rows[4][3] = U'そ';

        kb->pages[PDKBP_JA_Hiragana].rows[0][4] = U'た';
        kb->pages[PDKBP_JA_Hiragana].rows[1][4] = U'ち';
        kb->pages[PDKBP_JA_Hiragana].rows[2][4] = U'つ';
        kb->pages[PDKBP_JA_Hiragana].rows[3][4] = U'て';
        kb->pages[PDKBP_JA_Hiragana].rows[4][4] = U'と';

        kb->pages[PDKBP_JA_Hiragana].rows[0][5] = U'な';
        kb->pages[PDKBP_JA_Hiragana].rows[1][5] = U'に';
        kb->pages[PDKBP_JA_Hiragana].rows[2][5] = U'ぬ';
        kb->pages[PDKBP_JA_Hiragana].rows[3][5] = U'ね';
        kb->pages[PDKBP_JA_Hiragana].rows[4][5] = U'の';

        kb->pages[PDKBP_JA_Hiragana].rows[0][6] = U'は';
        kb->pages[PDKBP_JA_Hiragana].rows[1][6] = U'ひ';
        kb->pages[PDKBP_JA_Hiragana].rows[2][6] = U'ふ';
        kb->pages[PDKBP_JA_Hiragana].rows[3][6] = U'へ';
        kb->pages[PDKBP_JA_Hiragana].rows[4][6] = U'ほ';

        kb->pages[PDKBP_JA_Hiragana].rows[0][7] = U'ま';
        kb->pages[PDKBP_JA_Hiragana].rows[1][7] = U'み';
        kb->pages[PDKBP_JA_Hiragana].rows[2][7] = U'む';
        kb->pages[PDKBP_JA_Hiragana].rows[3][7] = U'め';
        kb->pages[PDKBP_JA_Hiragana].rows[4][7] = U'も';

        kb->pages[PDKBP_JA_Hiragana].rows[0][8] = U'ら';
        kb->pages[PDKBP_JA_Hiragana].rows[1][8] = U'り';
        kb->pages[PDKBP_JA_Hiragana].rows[2][8] = U'る';
        kb->pages[PDKBP_JA_Hiragana].rows[3][8] = U'れ';
        kb->pages[PDKBP_JA_Hiragana].rows[4][8] = U'ろ';

        kb->pages[PDKBP_JA_Hiragana].rows[0][9] = U'や';
        kb->pages[PDKBP_JA_Hiragana].rows[1][9] = U'ゆ';
        kb->pages[PDKBP_JA_Hiragana].rows[2][9] = U'よ';
        kb->pages[PDKBP_JA_Hiragana].rows[3][9] = U'わ';
        kb->pages[PDKBP_JA_Hiragana].rows[4][9] = U'を';

        kb->pages[PDKBP_JA_Hiragana].rows[0][10] = KEY_BACKSPACE;
        kb->pages[PDKBP_JA_Hiragana].rows[1][10] = KEY_DAKUTEN;
        kb->pages[PDKBP_JA_Hiragana].rows[2][10] = KEY_HANDAKUTEN;
        kb->pages[PDKBP_JA_Hiragana].rows[3][10] = U'ン';
        kb->pages[PDKBP_JA_Hiragana].rows[4][10] = KEY_CONFIRM;
    }

    if ((f & PDKBF_LATIN_UPPERCASE) || (f & PDKBF_LATIN_LOWERCASE))
    {
        kb->pages[PDKBP_Latin].type = PDKBP_Latin;
        kb->current_page = PDKBP_Latin;
        
        bool punct = !!(kb->flags & PDKBF_LATIN_PUNCTUATION);
        
        kb->pages[PDKBP_Latin].rows_c = 4;
        kb->pages[PDKBP_Latin].rows[0][0] = 'a';
        kb->pages[PDKBP_Latin].rows[0][1] = 'b';
        kb->pages[PDKBP_Latin].rows[0][2] = 'c';
        kb->pages[PDKBP_Latin].rows[0][3] = 'd';
        kb->pages[PDKBP_Latin].rows[0][4] = 'e';
        kb->pages[PDKBP_Latin].rows[0][5] = 'f';
        kb->pages[PDKBP_Latin].rows[0][6] = 'g';
        kb->pages[PDKBP_Latin].rows[0][7] = 'h';
        kb->pages[PDKBP_Latin].rows[0][8] = KEY_BACKSPACE;
        if (punct) kb->pages[PDKBP_Latin].rows[0][9] = KEY_EXTEND;
        kb->pages[PDKBP_Latin].rows[1][0] = 'i';
        kb->pages[PDKBP_Latin].rows[1][1] = 'j';
        kb->pages[PDKBP_Latin].rows[1][2] = 'k';
        kb->pages[PDKBP_Latin].rows[1][3] = 'l';
        kb->pages[PDKBP_Latin].rows[1][4] = 'm';
        kb->pages[PDKBP_Latin].rows[1][5] = 'n';
        kb->pages[PDKBP_Latin].rows[1][6] = 'o';
        kb->pages[PDKBP_Latin].rows[1][7] = 'p';
        kb->pages[PDKBP_Latin].rows[1][8] = 'q';
        if (punct) kb->pages[PDKBP_Latin].rows[1][9] = '\'';
        kb->pages[PDKBP_Latin].rows[2][0] = 'r';
        kb->pages[PDKBP_Latin].rows[2][1] = 's';
        kb->pages[PDKBP_Latin].rows[2][2] = 't';
        kb->pages[PDKBP_Latin].rows[2][3] = 'u';
        kb->pages[PDKBP_Latin].rows[2][4] = 'v';
        kb->pages[PDKBP_Latin].rows[2][5] = 'w';
        kb->pages[PDKBP_Latin].rows[2][6] = 'x';
        kb->pages[PDKBP_Latin].rows[2][7] = 'y';
        kb->pages[PDKBP_Latin].rows[2][8] = 'z';
        if (punct) kb->pages[PDKBP_Latin].rows[2][9] = '.';
        
        if (!(kb->flags & PDKBF_LATIN_LOWERCASE))
        {
            for (size_t i = 0; i < MAXROWS; ++i)
            {
                for (size_t j = 0; j < MAXROWLEN; ++j)
                {
                    kb->pages[PDKBP_Latin].rows[i][j] = 
                        toupper(kb->pages[PDKBP_Latin].rows[i][j]);
                }
            }
        }
        else if ((kb->flags & PDKBF_START_UPPERCASE) == PDKBF_START_UPPERCASE)
        {
            kb->altstate = 1;
        }
        
        size_t ki = 0;
        if (pagec >= 2)
        {
            kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_SWITCH;
        }
        
        if (has_upper_and_lower)
        {
            kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_UPPER;
        }
        
        for (const uint32_t* kv = leftkeys; kv && *kv; ++kv)
        {
            kb->pages[PDKBP_Latin].rows[3][ki++] = *kv;
        }
        
        kb->pages[PDKBP_Latin].rows[3][ki++] = spacekey;
        kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
        kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
        kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
        if (!leftkeys)
            kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
        if (!rightkeys)
            kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
        
        for (const uint32_t* kv = rightkeys; kv && *kv; ++kv)
        {
            kb->pages[PDKBP_Latin].rows[3][ki++] = *kv;
        }
        
        if (kb->flags & PDKBF_CONFIRM)
        {
            kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_CONFIRM;
            if (has_upper_and_lower)
            {
                kb->pages[PDKBP_Latin].rows[3][ki++] = KEY_EXTEND;
            }
        }
    }
    
    // longpress keys
    for (size_t p = 0; p < PDKBP_COUNT; ++p)
    {
        PDKeyboardPage* page = &kb->pages[p];
        for (size_t i = 0; i < MAXROWS; ++i)
        {
            for (size_t j = 0; j < MAXROWLEN; ++j)
            {
                uint32_t key = page->rows[i][j];

                if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z'))
                {
                    if (has_upper_and_lower)
                        _pdkb_lp_append(page, i, j, KEY_UPPER);
                }
                
                uint32_t key_sg = _pdkb_apply_sutegana(key);
                if (key_sg != key) _pdkb_lp_append(page, i, j, key_sg);
                
                key_sg = _pdkb_apply_dakuten(key, false);
                if (key_sg != key) _pdkb_lp_append(page, i, j, key_sg);
                
                key_sg = _pdkb_apply_dakuten(key, true);
                if (key_sg != key) _pdkb_lp_append(page, i, j, key_sg);
                
                if ((page->type == PDKBP_JA_Hiragana && (f & PDKBF_JA_KATAKANA))
                    || (page->type == PDKBP_JA_Katakana && (f & PDKBF_JA_HIRAGANA))
                )
                {
                    uint32_t key2 = _pdkb_swap_kana(key);
                    _pdkb_lp_append(page, i, j, key2);
                    
                    key_sg = _pdkb_apply_sutegana(key2);
                    if (key_sg != key2) _pdkb_lp_append(page, i, j, key_sg);
                    
                    key_sg = _pdkb_apply_dakuten(key2, false);
                    if (key_sg != key2) _pdkb_lp_append(page, i, j, key_sg);
                    
                    key_sg = _pdkb_apply_dakuten(key2, true);
                    if (key_sg != key2) _pdkb_lp_append(page, i, j, key_sg);
                }
                    
                switch(key)
                {
                case ' ':
                    if (f & PDKBF_LATIN_PUNCTUATION)
                    {
                        _pdkb_lp_append(page, i, j, '_');
                        _pdkb_lp_append(page, i, j, '-');
                    }
                    break;
                case '~':
                    if (f & PDKBF_SYMBOLS)
                    {
                        _pdkb_lp_append(page, i, j, '`');
                    }
                    break;
                case '.':
                case ',':
                    if (f & PDKBF_LATIN_PUNCTUATION)
                    {
                        _pdkb_lp_append(page, i, j, (key == '.') ? ',' : '.');
                        _pdkb_lp_append(page, i, j, '?');
                        _pdkb_lp_append(page, i, j, '!');
                        _pdkb_lp_append(page, i, j, ':');
                        _pdkb_lp_append(page, i, j, ';');
                    }
                    break;
                case U'、':
                    _pdkb_lp_append(page, i, j, U'ゝ');
                    _pdkb_lp_append(page, i, j, U'ゞ');
                    break;
                case U'ー':
                    _pdkb_lp_append(page, i, j, U'ヽ');
                    _pdkb_lp_append(page, i, j, U'ヾ');
                    break;
                case U'・':
                    _pdkb_lp_append(page, i, j, U'゠');
                    break;
                case U'コ':
                    _pdkb_lp_append(page, i, j, U'ヿ');
                    break;
                case U'よ':
                    _pdkb_lp_append(page, i, j, U'ゟ');
                    break;
                case '\'':
                    if (f & PDKBF_LATIN_PUNCTUATION)
                    {
                        _pdkb_lp_append(page, i, j, '"');
                        _pdkb_lp_append(page, i, j, '_');
                        _pdkb_lp_append(page, i, j, '-');
                        _pdkb_lp_append(page, i, j, '/');
                        _pdkb_lp_append(page, i, j, '\\');
                        _pdkb_lp_append(page, i, j, '(');
                        _pdkb_lp_append(page, i, j, ')');
                        _pdkb_lp_append(page, i, j, '`');
                    }
                    break;
                case U'い':
                    _pdkb_lp_append(page, i, j, U'ゐ');
                    if (f & PDKBF_JA_KATAKANA)
                    {
                        _pdkb_lp_append(page, i, j, U'ヰ');
                        _pdkb_lp_append(page, i, j, U'ヸ');
                    }
                    break;
                case U'え':
                    _pdkb_lp_append(page, i, j, U'ゑ');
                    if (f & PDKBF_JA_KATAKANA)
                    {
                        _pdkb_lp_append(page, i, j, U'ヰ');
                        _pdkb_lp_append(page, i, j, U'ヸ');
                    }
                    break;
                case U'イ':
                    _pdkb_lp_append(page, i, j, U'ヰ');
                    _pdkb_lp_append(page, i, j, U'ヸ');
                    if (f & PDKBF_JA_HIRAGANA)
                    {
                        _pdkb_lp_append(page, i, j, U'ゐ');
                         	
                    }
                    break;
                case U'エ':
                    _pdkb_lp_append(page, i, j, U'ヱ');
                    _pdkb_lp_append(page, i, j, U'ヹ');
                    if (f & PDKBF_JA_HIRAGANA)
                    {
                        _pdkb_lp_append(page, i, j, U'ゑ');
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (kb->current_page == PDKBP_None)
    {
        pd->system->realloc(kb, 0);
        return NULL;
    }

    kb->encoding = kUTF8Encoding;
    kb->animation_duration = 0.25;

    return kb;
}

void pdkb_free(PDKeyboard* kb)
{
    if (!kb) return;

    for (size_t p = 0; p < PDKBP_COUNT; ++p)
    {
        _pdkb_lp_free(&kb->pages[p]);
    }

    pd->system->realloc(kb->content, 0);
    pd->system->realloc(kb, 0);
}

int pdkb_get_result(const PDKeyboard* kb)
{
    return kb->result;
}

int pdkb_set_encoding(PDKeyboard* kb, const char* encoding)
{
    if (strcmp(encoding, "ascii") == 0) {
        kb->encoding = kASCIIEncoding;
        return 1;
    } else if (strcmp(encoding, "utf8") == 0) {
        kb->encoding = kUTF8Encoding;
        return 1;
    } else if (strcmp(encoding, "le16") == 0) {
        kb->encoding = k16BitLEEncoding;
        return 1;
    }
    return 0;
}

void pdkb_set_content(PDKeyboard* kb, const char* content)
{
    pd->system->realloc(kb->content, 0);
    kb->content = NULL;
    kb->content_capacity = 0;
    kb->current_byte = 0;
    kb->current_codepoint = 0;

    if (content) {
        size_t len = strlen(content) + 1;
        _pdkb_ensure_capacity(kb, len);
        memcpy(kb->content, content, len);

        kb->current_byte = len - 1;
        size_t pos = kb->current_byte;
        while (pos > 0) {
            kb->current_codepoint++;
            pos = _codepoint_prev(content, pos, kb->encoding);
        }
    }
}

const char* pdkb_get_content(const PDKeyboard* kb)
{
    return kb->content;
}

unsigned pdkb_get_content_length_bytes(const PDKeyboard* kb)
{
    if (!kb->content) return 0;
    return (unsigned)strlen(kb->content);
}

unsigned pdkb_get_content_length_codepoints(const PDKeyboard* kb)
{
    if (!kb->content) return 0;
    unsigned count = 0;
    size_t pos = strlen(kb->content);
    while (pos > 0) {
        count++;
        pos = _codepoint_prev(kb->content, pos, kb->encoding);
    }
    return count;
}

void pdkb_set_font(PDKeyboard* kb, const LCDFont* f)
{
    if (!kb) return;
    kb->user_font = f;
}

void pdkb_set_max_bytes(PDKeyboard* kb, unsigned max_bytes)
{
    kb->content_max_bytes = max_bytes;
}

void pdkb_set_max_codepoints(PDKeyboard* kb, unsigned max_codepoints)
{
    kb->content_max_codepoints = max_codepoints;
}

void pdkb_set_openclose_duration(PDKeyboard* kb, float s)
{
    kb->animation_duration = s;
}

void pdkb_open(PDKeyboard* kb)
{
    kb->is_open = true;
    if (kb->animation_duration <= 0.0f)
    {
        kb->open_p = 1.0f;
    }
}

void pdkb_close(PDKeyboard* kb)
{
    kb->is_open = false;
    if (kb->animation_duration <= 0.0f)
    {
        kb->open_p = 0.0f;
    }
}

PDKeyboardState pdkb_get_state(const PDKeyboard* kb)
{
    if (kb->open_p == 0 && !kb->is_open) return PDKBS_CLOSED;
    if (kb->open_p == 1 && kb->is_open) return PDKBS_OPEN;
    if (kb->is_open) return PDKBS_ANIMATION_OPENING;
    return PDKBS_ANIMATION_CLOSING;
}

float pdkb_get_open_p(const PDKeyboard* kb)
{
    return kb->open_p;
}

int pdkb_get_visible_y(const PDKeyboard* kb)
{
    float p = 1.0f - (1.0f - kb->open_p) * (1.0f - kb->open_p);
    return (int)(120 + 120 * (1.0f - p));
}

unsigned pdkb_get_hovered(const PDKeyboard* kb)
{
    const PDKeyboardPage* page = &kb->pages[kb->current_page];
    if (page->type == PDKBP_None) return 0;
    uint32_t key = _pdkb_count_row_get_key(page, kb->sel_row, kb->sel_idx);
    if (key < 0x20) return 0;
    if (kb->altstate)
    {
        key = _pdkb_apply_altstate(key);
    }
    return (unsigned)key;
}

static uint32_t _pdkb_fold_case(uint32_t c)
{
    // toupper is only defined for unsigned char values
    if (c < 0x80) return (uint32_t)toupper((int)c);
    return c;
}

void pdkb_add_longpress_entry(PDKeyboard* kb, uint32_t basekey, uint32_t longpress_key)
{
    if (!kb || !basekey || !longpress_key) return;

    uint32_t folded = _pdkb_fold_case(basekey);

    for (size_t p = 0; p < PDKBP_COUNT; ++p)
    {
        PDKeyboardPage* page = &kb->pages[p];
        if (page->type == PDKBP_None) continue;

        for (size_t i = 0; i < MAXROWS; ++i)
        {
            for (size_t j = 0; j < MAXROWLEN; ++j)
            {
                uint32_t key = page->rows[i][j];
                if (!key || _pdkb_fold_case(key) != folded) continue;

                // don't add the same entry twice
                bool present = false;
                const uint32_t* lp = page->rows_longpress[i][j];
                for (size_t n = 0; lp && lp[n]; ++n)
                {
                    if (lp[n] == longpress_key) { present = true; break; }
                }

                if (!present)
                {
                    _pdkb_lp_append(page, i, j, longpress_key);
                }
            }
        }
    }
}