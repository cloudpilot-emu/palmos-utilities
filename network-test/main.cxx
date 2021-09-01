#include <PalmOS.h>

#include "debug.h"
#include "resids.h"

template <class T>
T min(T x, T y) {
    return x < y ? x : y;
}

template <class T>
T max(T x, T y) {
    return x > y ? x : y;
}

void InitField(FormType* form) {
    MemHandle handle = MemHandleNew(256);
    char* fieldContent = static_cast<char*>(MemHandleLock(handle));

    MemSet(fieldContent, 256, 0);

    for (char i = 0; i < 26; i++) {
        fieldContent[2 * i] = 'a' + i;
        fieldContent[2 * i + 1] = '\n';
    }

    MemHandleUnlock(handle);

    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    FldSetTextHandle(field, handle);
    FldSetMaxChars(field, 255);

    FrmDrawForm(form);
}

void ScrollUp(FormType* form) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    FldScrollField(field, FldGetVisibleLines(field) / 2, winUp);
}

void ScrollDown(FormType* form) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    UInt16 scrollPos, textHeight, fieldHeight;
    FldGetScrollValues(field, &scrollPos, &textHeight, &fieldHeight);

    UInt16 delta =
        min<UInt16>(fieldHeight / 2, max<Int16>(textHeight - scrollPos - fieldHeight, 0));

    FldScrollField(field, delta, winDown);
}

Boolean MainFormHandler(EventType* event) {
    FormType* form = FrmGetActiveForm();

    switch (event->eType) {
        case frmOpenEvent:
            InitField(form);

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
