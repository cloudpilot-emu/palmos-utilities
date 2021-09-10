#include <PalmOS.h>

#include "command.h"
#include "debug.h"
#include "field.h"
#include "resource.h"

void OnSelectCommand(FormType* form, const char* command) {
    ControlType* buttonCtl =
        static_cast<ControlType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, ConfigButton)));

    LOG("select ---- %s", command);

    if (StrCompare(command, CMD_TEST_SEND_PB) == 0)
        CtlShowControl(buttonCtl);
    else
        CtlHideControl(buttonCtl);
}

void DispatchCommand(FormType* form) {
    ControlType* triggerCtl =
        static_cast<ControlType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, CommandTrigger)));

    const char* command = CtlGetLabel(triggerCtl);
    if (StrCompare(command, CMD_TEST_SOCKET_OPTION_GET) == 0)
        TestSocketOptionGet();
    else if (StrCompare(command, CMD_TEST_DM_SEND) == 0)
        TestDmSend();
    else if (StrCompare(command, CMD_TEST_RECEIVE_PB) == 0)
        TestReceivePB();
    else if (StrCompare(command, CMD_TEST_SEND_PB) == 0)
        TestSendPB();
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
            switch (event->data.ctlSelect.controlID) {
                case RunButton:
                    DispatchCommand(form);
                    break;

                case ClearButton:
                    Clear(form);
                    break;
            }

            return false;

        case popSelectEvent:
            if (event->data.popSelect.controlID == CommandTrigger)
                OnSelectCommand(form, LstGetSelectionText(event->data.popSelect.listP,
                                                          event->data.popSelect.selection));

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
