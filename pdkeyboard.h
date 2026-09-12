/*

Copyright (c) 2026 Sodium Hydroxide

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef _PD_KEYBOARD_H_
#define _PD_KEYBOARD_H_

typedef struct PlaydateAPI PlaydateAPI;
typedef struct PDKeyboard PDKeyboard;

#define PDKB_HEIGHT 120

enum PDKeyboardState {
    PDKBS_CLOSED,
    PDKBS_OPEN,
    PDKBS_ANIMATION_OPENING,
    PDKBS_ANIMATION_CLOSING,
};
typedef enum PDKeyboardState PDKeyboardState;

enum PDKeyboardFlags {
    /* user can confirm input (closes keyboard) */
    PDKBF_CONFIRM = 0x1000,

    /* user can cancel out (closes keyboard) */
    PDKBF_CANCEL = 0x2000,
    
    /* Start in uppercase */
    PDKBF_START_UPPERCASE = 0x4001,

    /* Audio feedback */
    PDKBF_SOUND = 0x8000,

    /* Allowed Characters */
    PDKBF_LATIN_UPPERCASE = 1, /* allow entering uppercase latin alphabet (A-Z) */
    PDKBF_LATIN_LOWERCASE = 2, /* allow entering lowercase latin alphabet (a-z) */
    PDKBF_SPACE = 4, /* allow spaces */
    PDKBF_LATIN_PUNCTUATION = 8, /* allow common punctuation  !?,.-_:;/'" */

    PDKBF_NUMERIC = 0x10, /* allow numbers 0-9 */
    PDKBF_SYMBOLS = 0x28, /* allow various special symbols @#$%^&*{}[]+~<> (including punctuation) */
    
    /* The following are ignored if the encoding scheme is set to ASCII. */
    PDKBF_JA_HIRAGANA = 0x10000,
    PDKBF_JA_KATAKANA = 0x20000,
    PDKBF_JA_KANA = PDKBF_JA_HIRAGANA | PDKBF_JA_KATAKANA,
    PDKBF_JA_ALL = PDKBF_JA_KANA,

    PDKBF_DEFAULT
        = PDKBF_CONFIRM | PDKBF_CANCEL
        | PDKBF_SOUND
        | PDKBF_START_UPPERCASE | PDKBF_LATIN_LOWERCASE
        | PDKBF_SPACE | PDKBF_NUMERIC
        | PDKBF_SYMBOLS
        | PDKBF_JA_ALL
};
typedef enum PDKeyboardFlags PDKeyboardFlags;

/* You only need to create a keyboard once.
 * 
 * Afterward, you can use pdkb_open() and pdkb_close() to show/hide it.
 * 
 * The same keyboard can be used on different text entry fields,
 * unless the configuration changes (different flags, etc.).
 * 
 * While the keyboard exists, invoke pdkb_update(kb) once per frame,
 * even if the keyboard has been closed.
 *
 * special_*keys: optional NULL-terminated list of extra keys to add to the right/left of the space bar.
 * (Only for latin keyboard.)
*/
PDKeyboard* pdkb_new(PlaydateAPI* pd, PDKeyboardFlags, const uint32_t* special_leftkeys, const uint32_t* special_rightkeys);

/* Delete keyboard. Cleans up resources. */
void pdkb_free(PDKeyboard* kb);

/* Invoke this once per frame while the keyboard exists (regardless of if it's open or closed).
 * Updates and draws the keyboard.
 * This should be called after any other draw operations.
*/
void pdkb_update(PDKeyboard* kb, float dt);

/* returns 1 if user confirmed input,
 * returns -1 if user cancelled input,
 * returns 0 if still in use or N/A
 */
int pdkb_get_result(const PDKeyboard* kb);

/*
 * Set the character encoding to be used in the output buffer.
 *
 * If set to "ascii", some keyboard flags (like japanese characters) will be ignored.
 * 
 * recognized values:
 * - "ascii"
 * - "utf8" (default)
 * - "le16"
 * 
 * returns 0 on failure.
 */
int pdkb_set_encoding(PDKeyboard* kb, const char* encoding);

/* Replaces the keyboard's current string buffer.
 * pdkb_set_content(NULL) should be used to clear the keyboard.
 */
void pdkb_set_content(PDKeyboard* kb, const char* content);

/* Gets the current string output of the keyboard. */
const char* pdkb_get_content(const PDKeyboard* kb);

/* Returns length of content in bytes */
unsigned pdkb_get_content_length_bytes(const PDKeyboard* kb);

/* Returns length of content in codepoints (e.g. unicode characters) */
unsigned pdkb_get_content_length_codepoints(const PDKeyboard* kb);

/* Sets the maximum size of output buffer in bytes.
 * 
 * Overrides pdkb_set_max_codepoints.
 */
void pdkb_set_max_bytes(PDKeyboard* kb, unsigned);

/* Sets the maximum size of output buffer in codepoints (e.g. unicode characters).
 * 
 * Overrides pdkb_set_max_bytes.
 */
void pdkb_set_max_codepoints(PDKeyboard* kb, unsigned);

/* Sets the length (in seconds) of the opening/closing animation for the keyboard. */
void pdkb_set_openclose_duration(PDKeyboard* kb, float s);

/* Reveal keyboard. User input (collected in pdkb_update) will
 * then drive the keyboard.
 * */
void pdkb_open(PDKeyboard* kb);

/* Closes keyboard. Note that this may not be instantaneous. */
void pdkb_close(PDKeyboard* kb);

/* Can be used to determine if the keyboard is open, closed,
 * or transitioning between.
 */
PDKeyboardState pdkb_get_state(const PDKeyboard* kb);

/* Returns a value in the range [0, 1] indicating how open (visible) the keyboard is.
 * This transitions linearly between 0 and 1 during opening/closing animations,
 * and sits at 0/1 while closed/open.
*/
float pdkb_get_open_p(const PDKeyboard* kb);

/* 
 * Gets the y value of the top of the keyboard.
 * When closed, this is LCD_ROWS.
 * When open, this is PDKB_HEIGHT.
 */
int pdkb_get_visible_y(const PDKeyboard* kb);

/* Returns the codepoint (e.g. unicode character) that the user's
 * cursor is hovering over, or 0 if not applicable.
 * */
unsigned pdkb_get_hovered(const PDKeyboard* kb);

/* call this from your serial message callback to
 * enable external device typing.
 * (See playdate->system->setSerialMessageCallback).
 * Returns 1 if the message was handled.
 *
 * (handles serial messages matching "pdkb:<codepoint in decimal>")
 */
int pdkb_on_serial(PDKeyboard* kb, const char* c);

#endif /* _PD_KEYBOARD_H_ */