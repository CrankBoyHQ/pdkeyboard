#include "pd_api.h"

PlaydateAPI* playdate;

int update(void* userdata)
{
    // return non-zero to indicate to update the display
    return 1;
}

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
    playdate = pd;
    pd->system->logToConsole("Event %d\n", event);
    
    if (event == kEventInit)
    {
        playdate->system->setUpdateCallback(update, NULL);
    }
    
    return 0;
}