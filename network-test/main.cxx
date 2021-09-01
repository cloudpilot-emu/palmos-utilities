#include <PalmOS.h>

#include "debug.h"
#include "resids.h"

#define FIELD_BUFFER_SIZE 512

template <class T>
T min(T x, T y) {
    return x < y ? x : y;
}

template <class T>
T max(T x, T y) {
    return x > y ? x : y;
}

void UpdateScrollbar(FormType* form) {
    ScrollBarType* scrollBar =
        static_cast<ScrollBarType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, Scrollbar)));

    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    UInt16 scrollPos, textHeight, fieldHeight;
    FldGetScrollValues(field, &scrollPos, &textHeight, &fieldHeight);

    SclSetScrollBar(scrollBar, scrollPos, 0, max<Int16>(textHeight - fieldHeight, 0), fieldHeight);
}

void InitField(FormType* form) {
    MemHandle handle = MemHandleNew(FIELD_BUFFER_SIZE);
    char* fieldContent = static_cast<char*>(MemHandleLock(handle));

    MemSet(fieldContent, FIELD_BUFFER_SIZE, 0);
    MemHandleUnlock(handle);

    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    FldSetTextHandle(field, handle);
    FldSetMaxChars(field, FIELD_BUFFER_SIZE - 1);
}

void ScrollUp(FormType* form) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    FldScrollField(field, FldGetVisibleLines(field) / 2, winUp);

    UpdateScrollbar(form);
}

void ScrollDown(FormType* form) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    UInt16 scrollPos, textHeight, fieldHeight;
    FldGetScrollValues(field, &scrollPos, &textHeight, &fieldHeight);

    const UInt16 delta =
        min<UInt16>(fieldHeight / 2, max<Int16>(textHeight - scrollPos - fieldHeight, 0));

    FldScrollField(field, delta, winDown);

    UpdateScrollbar(form);
}

void HandleScrollBarRepeat(FormType* form, const EventType& event) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    const Int16 delta = event.data.sclRepeat.newValue - event.data.sclRepeat.value;

    FldScrollField(field, Abs(delta), delta > 0 ? winDown : winUp);
}

void Printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;

    va_start(args, fmt);

    UInt16 len = StrVPrintF(buffer, fmt, static_cast<_Palm_va_list>(args)) + 1;
    if (StrVPrintF(buffer, fmt, static_cast<_Palm_va_list>(args)) > 256) {
        LOG("VPrint: buffer overflowed, memory corruption ahead");

        return;
    }

    FormType* form = FrmGetActiveForm();
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    MemHandle handle = FldGetTextHandle(field);
    char* fieldBuffer = static_cast<char*>(MemHandleLock(handle));

    UInt16 cursor = 0;
    while (fieldBuffer[cursor] != 0 && cursor < FIELD_BUFFER_SIZE) cursor++;

    if (FIELD_BUFFER_SIZE - cursor >= len)
        MemMove(fieldBuffer + cursor, buffer, len);
    else {
        UInt16 clearTo = 0;
        while ((fieldBuffer[clearTo] != '\n' && fieldBuffer[clearTo] != 0) ||
               (FIELD_BUFFER_SIZE - cursor + clearTo) < len)
            clearTo++;

        if (fieldBuffer[clearTo] == 0)
            MemMove(fieldBuffer, buffer, len);
        else {
            cursor -= (clearTo + 1);

            MemMove(fieldBuffer, fieldBuffer + clearTo + 1, cursor);
            MemMove(fieldBuffer + cursor, buffer, len);
        }
    }

    MemHandleUnlock(handle);

    FldSetTextHandle(field, handle);

    UInt16 scrollPos, textHeight, fieldHeight;
    FldGetScrollValues(field, &scrollPos, &textHeight, &fieldHeight);

    if (textHeight > fieldHeight)
        FldScrollField(field, fieldHeight - textHeight, winDown);
    else
        FldDrawField(field);

    UpdateScrollbar(form);
}

Boolean MainFormHandler(EventType* event) {
    FormType* form = FrmGetActiveForm();

    switch (event->eType) {
        case frmOpenEvent:
            InitField(form);
            UpdateScrollbar(form);

            FrmDrawForm(form);

            return true;

        case keyDownEvent:
            if (event->data.keyDown.modifiers & commandKeyMask) {
                switch (event->data.keyDown.chr) {
                    case vchrPageDown:
                        ScrollDown(form);
                        return true;

                    case vchrPageUp:
                        ScrollUp(form);
                        return true;
                }
            }

            return false;

        case fldChangedEvent:
            UpdateScrollbar(form);
            return false;

        case sclRepeatEvent:
            HandleScrollBarRepeat(form, *event);
            return false;

        case ctlSelectEvent:
            if (event->data.ctlSelect.controlID == RunButton)
                Printf("Hello world %i\n", SysRandom(0));

            return false;

        default:
            return false;
    }
}

UInt32 PilotMain(UInt16 cmd, void* cmdPBP, UInt16 launchFlags) {
    if (cmd == sysAppLaunchCmdNormalLaunch) {
        EventType event;
        Err err;

        LOG("event loop running...");

        FrmGotoForm(MainForm);

        do {
            EvtGetEvent(&event, evtWaitForever);

            if (SysHandleEvent(&event)) continue;

            if (event.eType != nilEvent) LOG("incoming event %s", EventName(event.eType));

            if (MenuHandleEvent(NULL, &event, &err)) continue;

            if (event.eType == frmLoadEvent) {
                FormType* form = FrmInitForm(event.data.frmLoad.formID);
                FrmSetActiveForm(form);
                FrmSetEventHandler(form, MainFormHandler);

                continue;
            }

            if (FrmDispatchEvent(&event)) continue;
        } while (event.eType != appStopEvent);

        FrmCloseAllForms();
    }

    return 0;
}
