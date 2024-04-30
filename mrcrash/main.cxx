#include <PalmOS.h>

#include "bad_stack.h"
#include "debug.h"
#include "resource.h"

void CrashProtectionFailure() {
    Err createErr = DmCreateDatabase(0, "Crash Test Dummy", 'CRSH', 'CRSH', false);
    if (createErr && createErr != dmErrAlreadyExists) return;

    DmOpenRef ref = DmOpenDatabaseByTypeCreator('CRSH', 'CRSH', dmModeReadWrite);
    if (!ref) return;

    UInt16 idx = 0;

    MemHandle handle = DmQueryRecord(ref, idx);
    if (!handle) DmNewRecord(ref, &idx, 1);

    handle = DmQueryRecord(ref, idx);
    MemPtr ptr = MemHandleLock(handle);

    LOG("Triggering protection fault now");
    *static_cast<UInt8*>(ptr) = 0;
}

void CrashUnmappedAddress() {
    UInt8* bad = (UInt8*)0xfefe0000;
    *bad = 0;
}

void Crash(FormType* form) {
    ControlType* triggerCtl =
        static_cast<ControlType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, ModeTrigger)));

    const char* mode = CtlGetLabel(triggerCtl);

    if (StrCompare(mode, MODE_BAD_STACK) == 0)
        CrashExceptionWithBadStack();
    else if (StrCompare(mode, MODE_PROTECTION) == 0)
        CrashProtectionFailure();
    else if (StrCompare(mode, MODE_UNMAPPED_ADDRESS) == 0)
        CrashUnmappedAddress();
}

Boolean MainFormHandler(EventType* event) {
    FormType* form = FrmGetActiveForm();

    switch (event->eType) {
        case frmOpenEvent:
            FrmDrawForm(form);

            return true;
        case ctlSelectEvent:
            switch (event->data.ctlSelect.controlID) {
                case CrashButton:
                    Crash(form);
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
