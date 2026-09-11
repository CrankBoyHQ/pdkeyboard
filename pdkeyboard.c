#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pd_api.h"
#include "pdkeyboard.h"

static PlaydateAPI* pd;
static LCDFont* font;

const static uint8_t lcd50[16] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

enum PDKBPageType {
    PDKBP_None = 0,
    PDKBP_Latin,
    PDKBP_Numeric,
    PDKBP_Symbols,
    PDKBP_JA_Hiragana,
    PDKBP_JA_Katakana,
    PDKBP_COUNT
};

#define MAXROWS 4
#define MAXROWLEN 40

#define KEY_FILLER 1
#define KEY_SWITCH 2
#define KEY_EXTEND 3
#define KEY_CONFIRM 4
#define KEY_BACKSPACE 8

typedef struct PDKeyboardPage {
    enum PDKBPageType type;

    size_t rows_c;
    uint32_t rows[MAXROWS][MAXROWLEN];

} PDKeyboardPage;

typedef struct PDKeyboard {
    bool is_open;
    int result;
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

    PDStringEncoding encoding;

} PDKeyboard;



static size_t _codepoint_next(const char* s, size_t offset, PDStringEncoding encoding);
static size_t _codepoint_prev(const char* s, size_t offset, PDStringEncoding encoding);
static void _pdkb_draw_key(uint32_t key, int kx, int ky, int kw, int kh, bool selected, bool down);

size_t _codepoint_next(const char* s, size_t offset, PDStringEncoding encoding)
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

size_t _codepoint_prev(const char* s, size_t offset, PDStringEncoding encoding)
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
size_t _encode(uint32_t codepoint, PDStringEncoding encoding, char* o)
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

static size_t _pdkb_count_row_keys(PDKeyboardPage* page, size_t row)
{
    size_t count = 0;
    for (size_t i = 0; i < MAXROWLEN; ++i)
    {
        if (page->rows[row][i]) ++count;
    }

    return count;
}

static uint32_t _pdkb_count_row_get_key(PDKeyboardPage* page, size_t row, size_t idx)
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

static int _pdkb_get_key_width(PDKeyboardPage* page, size_t row)
{
    return LCD_COLUMNS / (int)_pdkb_count_row_keys(page, row);
}

static int _pdkb_get_key_height(PDKeyboardPage* page)
{
    if (page->rows_c == 0) return 1;
    unsigned size = (120 - 2) / page->rows_c;
    if (size > 60) return 60;
    return (int)size;
}

static int _pdb_get_key_offy(PDKeyboardPage* page)
{
    return 2 + (118 % _pdkb_get_key_height(page)) / 2;
}

static int _pdkb_get_key_offx(PDKeyboardPage* page, size_t row)
{
    return (LCD_COLUMNS % _pdkb_get_key_width(page, row)) / 2;
}

static void _pdkb_draw_key(uint32_t key, int kx, int ky, int kw, int kh, bool selected, bool down)
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
    
    // TODO: special images.
    if (key < 0x20) return;
    
    char buff[5];
    memset(buff, 0, sizeof(buff));
    _encode(key, kUTF8Encoding, buff);
    int textwidth = pd->graphics->getTextWidth(font, buff, 1, kUTF8Encoding, 0);
    pd->graphics->drawText(buff, 1, kUTF8Encoding, kx + kw/2 - textwidth/2, ky + kh/2-7);
    
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

static void _pdkb_draw(PDKeyboard* kb)
{
    int y = pdkb_get_visible_y(kb);

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
        int kx = _pdkb_get_key_offx(page, i);
        int kw = _pdkb_get_key_width(page, i);
        size_t k = _pdkb_count_row_keys(page, i);

        for (size_t j = 0; j < k;)
        {
            uint32_t key = _pdkb_count_row_get_key(page, i, j);

            size_t jw = 1;
            while (_pdkb_count_row_get_key(page, i, j + jw) == KEY_EXTEND) ++jw;
            bool sel = i == kb->sel_row && j == kb->sel_idx;
            _pdkb_draw_key(key, kx, ky, (int)(kw * jw), row_h, sel, sel && kb->sel_down);
            j += jw;
            kx += kw * jw;
        }

        ky += row_h;

        if (ky >= LCD_ROWS) return;
    }
}

static void _pdkb_page_advance(PDKeyboard* kb)
{
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

void pdkb_update(PDKeyboard* kb, float dt)
{
    _pdkb_update_anim(kb, dt);

    if (kb->open_p <= 0) return;

    _pdkb_draw(kb);
    
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
    
    int dx = !!(pushed & kButtonRight) - !!(pushed & kButtonLeft);
    int dy = !!(pushed & kButtonDown) - !!(pushed & kButtonUp);

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
}

PDKeyboard* pdkb_new(PlaydateAPI* _pd, PDKeyboardFlags f)
{
    pd = _pd;

    PDKeyboard* kb = _pd->system->realloc(NULL, sizeof(PDKeyboard));

    if (!kb) return NULL;

    memset(kb, 0, sizeof(PDKeyboard));

    if (f & PDKBF_SYMBOLS)
    {
        kb->pages[PDKBP_Symbols].type = PDKBP_Symbols;
        kb->current_page = PDKBP_Symbols;
    }

    if (f & PDKBF_NUMERIC)
    {
        kb->pages[PDKBP_Numeric].type = PDKBP_Numeric;
        kb->current_page = PDKBP_Numeric;
    }

    if (f & PDKBF_JA_KATAKANA)
    {
        kb->pages[PDKBP_JA_Katakana].type = PDKBP_JA_Katakana;
        kb->current_page = PDKBP_JA_Katakana;
    }

    if (f & PDKBF_JA_HIRAGANA)
    {
        kb->pages[PDKBP_JA_Hiragana].type = PDKBP_JA_Hiragana;
        kb->current_page = PDKBP_JA_Hiragana;
    }

    if ((f & PDKBF_LATIN_UPPERCASE) || (f & PDKBF_LATIN_LOWERCASE))
    {
        kb->pages[PDKBP_Latin].type = PDKBP_Latin;
        kb->current_page = PDKBP_Latin;

        kb->pages[PDKBP_Latin].rows_c = 3;
        kb->pages[PDKBP_Latin].rows[0][0] = 'A';
        kb->pages[PDKBP_Latin].rows[0][1] = 'B';
        kb->pages[PDKBP_Latin].rows[0][2] = 'C';
        kb->pages[PDKBP_Latin].rows[0][3] = 'D';
        kb->pages[PDKBP_Latin].rows[0][4] = 'E';
        kb->pages[PDKBP_Latin].rows[0][5] = 'F';
        kb->pages[PDKBP_Latin].rows[0][6] = 'G';
        kb->pages[PDKBP_Latin].rows[0][7] = 'H';
        kb->pages[PDKBP_Latin].rows[0][8] = 'I';
        kb->pages[PDKBP_Latin].rows[0][9] = KEY_BACKSPACE;
        kb->pages[PDKBP_Latin].rows[1][0] = 'J';
        kb->pages[PDKBP_Latin].rows[1][1] = 'K';
        kb->pages[PDKBP_Latin].rows[1][2] = 'L';
        kb->pages[PDKBP_Latin].rows[1][3] = 'M';
        kb->pages[PDKBP_Latin].rows[1][4] = 'N';
        kb->pages[PDKBP_Latin].rows[1][5] = 'O';
        kb->pages[PDKBP_Latin].rows[1][6] = 'P';
        kb->pages[PDKBP_Latin].rows[1][7] = 'Q';
        kb->pages[PDKBP_Latin].rows[1][8] = 'R';
        kb->pages[PDKBP_Latin].rows[1][9] = 'S';
        kb->pages[PDKBP_Latin].rows[2][0] = KEY_SWITCH;
        kb->pages[PDKBP_Latin].rows[2][1] = 'T';
        kb->pages[PDKBP_Latin].rows[2][2] = 'U';
        kb->pages[PDKBP_Latin].rows[2][3] = 'V';
        kb->pages[PDKBP_Latin].rows[2][4] = 'W';
        kb->pages[PDKBP_Latin].rows[2][5] = 'X';
        kb->pages[PDKBP_Latin].rows[2][6] = 'Y';
        kb->pages[PDKBP_Latin].rows[2][7] = 'Z';
        kb->pages[PDKBP_Latin].rows[2][8] = KEY_CONFIRM;
        kb->pages[PDKBP_Latin].rows[2][9] = KEY_EXTEND;
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
    float p = 1.0 - (1.0 - kb->open_p) * (1.0 - kb->open_p);
    return (int)(120 + 120 * (1.0f - p));
}

unsigned pdkb_get_hovered(const PDKeyboard* kb)
{
    PDKeyboardPage* page = &kb->pages[kb->current_page];
    if (page->type == PDKBP_None) return 0;
    uint32_t key = _pdkb_count_row_get_key(page, kb->sel_row, kb->sel_idx);
    if (key < 0x20) return 0;
    return (unsigned)key;
}
