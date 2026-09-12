# pdkeyboard

![preview image of keyboard](./assets/etc/keyboard.png)

Playdate C library for on-screen keyboards.

## Features

- Supports Latin alphabet, numbers, symbols, and Japanese kana.
- Just drop `pdkeyboard.c` and `pdkeyboard.h` into your projects, or include this repo as a submodule. No external assets required.
- Can add arbitrary symbols next to space bar
- Can set custom per-key longpress symbols
- Supports external keyboards if they send `pdkb:<codepoint>` over serial.
- [MIT license](./LICENSE).
- Dapper hand-coded C.
