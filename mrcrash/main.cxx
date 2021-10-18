#include <PalmOS.h>

#include "debug.h"
#include "resource.h"

void Crash() { asm volatile("
    movel #0xffffff00, %sp
    movew %sp@, %a0
"); }

Boolean MainFormHandler(EventType* event) {
        FormType* form = FrmGetActiveForm();

        switch (event->eType) {
            case frmOpenEvent:
                FrmDrawForm(form);

                return true;
            case ctlSelectEvent:
                switch (event->data.ctlSelect.controlID) {
                    case CrashButton:
                        Crash();
                        break;
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

            LOG("started");

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
