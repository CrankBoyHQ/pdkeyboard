#include "pd_api.h"
#include "../pdkeyboard.h"

PlaydateAPI* pd;
PDKeyboard* kb;
LCDFont* font;

int update(void* userdata)
{
    PDButtons pushed;
    pd->system->getButtonState(NULL, &pushed, NULL);

    if (pushed & kButtonA)
    {
        pdkb_open(kb);
    }

    pd->graphics->clear(kColorWhite);
    pd->graphics->setFont(font);

    const char* text = pdkb_get_content(kb);
    if (text && text[0] != '\0')
    {
        pd->graphics->drawText(text, strlen(text), kUTF8Encoding, 10, 10);
    }

    pdkb_update(kb, 1.0f / 30.0f);

    return 1;
}

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
    pd = playdate;

    if (event == kEventInit)
    {
        kb = pdkb_new(pd, PDKBF_DEFAULT);
        font = pd->graphics->loadFont("/Disk/System/Fonts/Asheville-Sans-14-Bold", NULL);
        pd->system->setUpdateCallback(update, NULL);
    }

    return 0;
}
