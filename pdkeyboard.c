#include "pdkeyboard.h"

static PlaydateApi* pd;

enum PDKBPageType {
    PDKBP_None = 0,
    PDKBP_Latin,
    PDKBP_Numeric,
    PDKBP_Symbols,
    PDKBP_JA_Hiragana,
    PDKBP_JA_Katakana,
    PDKBP_COUNT
};

typedef struct PDKeyboardPage {
    PDKBPageType type;
} PDKeyboardPage;

typedef struct PDKeyboard {
    bool is_open;
    int result;
    char* content;
    size_t content_capacity;
    size_t content_max_bytes; // 0: unrestrained
    size_t content_max_codepoints; // 0: unrestrained
    PDKeyboardPage pages[PDKBP_COUNT];
    size_t current_page;
    float open_p;
    float animation_duration;
    
    PDStringEncoding encoding;
    
} PDKeyboard;

static void _pdkb_ensure_capacity(PDKeyboard* kb, size_t mincapacity)
{
    
}

PDKeyboard* pdkb_new(PlaydateAPI* _pd, PDKeyboardFlags f)
{
    pd = _pd;
    
    PDKeyboard* keybkboard = pd->realloc(NULL, sizeof(PDKeyboard));
    
    if (!kb) return NULL;
    
    memset(kb, 0, sizeof(PDKeyboard));
    
    if (f & (PDKBF_SYMBOLS))
    {
        kb->pages[PDKBP_Symbols].type = PDKBP_Symbols;
        current_page = PDKBP_Symbols;
    }
    
    if (f & (PDKBF_NUMERIC))
    {
        kb->pages[PDKBP_Numeric].type = PDKBP_Numeric;
        current_page = PDKBP_Numeric;
    }
    
    if (f & (PDKBF_JA_KATAKANA))
    {
        kb->pages[PDKBP_JA_Katakana].type = PDKBP_JA_Katakana;
        current_page = PDKBP_JA_Katakana;
    }
    
    if (f & (PDKBF_JA_HIRAGANA))
    {
        kb->pages[PDKBP_JA_Hiragana].type = PDKBP_JA_Hiragana;
        current_page = PDKBP_JA_Hiragana;
    }
    
    if (f & (PDKBF_LATIN_UPPERCASE | PDKBF_LATIN_LOWERCASE))
    {
        kb->pages[PDKBP_Latin].type = PDKBP_Latin;
        current_page = PDKBP_Latin;
    }
    
    if (current_page == PDKBP_None)
    {
        pd->realloc(kb, 0);
        return NULL;
    }
    
    kb->encoding = kEncodingUTF8;
    
    return kb;
}

