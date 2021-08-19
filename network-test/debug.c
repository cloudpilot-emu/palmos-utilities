#include "debug.h"

#ifdef DEBUG

void DebugLog(const char* fmt, ...) {
    UInt32 ftrValue;
    char buffer[256];
    va_list args;

    if (FtrGet('cldp', 0, &ftrValue) || ftrValue != 0x20150103) return;

    va_start(args, fmt);

    if (StrVPrintF(buffer, fmt, args) > 255)
        DbgMessage("DebugLog: buffer overflowed, memory corruption ahead");
    else
        DbgMessage(buffer);
}

const char* EventName(eventsEnum event) {
    static char buffer[16];

    #define CASE(evt) \
        case evt:     \
            return #evt;

    switch (event) {
        CASE(nilEvent)
        CASE(penDownEvent)
        CASE(penUpEvent)
        CASE(penMoveEvent)
        CASE(keyDownEvent)
        CASE(winEnterEvent)
        CASE(winExitEvent)
        CASE(ctlEnterEvent)
        CASE(ctlExitEvent)
        CASE(ctlSelectEvent)
        CASE(ctlRepeatEvent)
        CASE(lstEnterEvent)
        CASE(lstSelectEvent)
        CASE(lstExitEvent)
        CASE(popSelectEvent)
        CASE(fldEnterEvent)
        CASE(fldHeightChangedEvent)
        CASE(fldChangedEvent)
        CASE(tblEnterEvent)
        CASE(tblSelectEvent)
        CASE(daySelectEvent)
        CASE(menuEvent)
        CASE(appStopEvent)
        CASE(frmLoadEvent)
        CASE(frmOpenEvent)
        CASE(frmGotoEvent)
        CASE(frmUpdateEvent)
        CASE(frmSaveEvent)
        CASE(frmCloseEvent)
        CASE(frmTitleEnterEvent)
        CASE(frmTitleSelectEvent)
        CASE(tblExitEvent)
        CASE(sclEnterEvent)
        CASE(sclExitEvent)
        CASE(sclRepeatEvent)
        CASE(tsmConfirmEvent)
        CASE(tsmFepButtonEvent)
        CASE(tsmFepModeEvent)
        CASE(attnIndicatorEnterEvent)
        CASE(attnIndicatorSelectEvent)
        CASE(menuCmdBarOpenEvent)
        CASE(menuOpenEvent)
        CASE(menuCloseEvent)
        CASE(frmGadgetEnterEvent)
        CASE(frmGadgetMiscEvent)
        CASE(keyUpEvent)
        CASE(keyHoldEvent)
        CASE(frmObjectFocusTakeEvent)
        CASE(frmObjectFocusLostEvent)
        default:
            StrPrintF(buffer, "unknown: %u", event);
            return buffer;
    }
    #undef CASE
}
#endif
