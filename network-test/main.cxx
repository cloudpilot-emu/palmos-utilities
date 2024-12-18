#include <PalmOS.h>

#include "command.h"
#include "db.h"
#include "debug.h"
#include "field.h"
#include "resource.h"
#include "util.h"

void OnSelectCommand(FormType* form, const char* command) {
    ControlType* buttonCtl =
        static_cast<ControlType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, ConfigButton)));

    if (StrCompare(command, CMD_TEST_SEND_PB) == 0 || StrCompare(command, CMD_TEST_RECEIVE_PB) == 0)
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
    else if (StrCompare(command, CMD_TEST_RECEIVE_PB) == 0)
        TestReceivePB();
    else if (StrCompare(command, CMD_TEST_SEND_PB) == 0)
        TestSendPB();
    else if (StrCompare(command, CMD_NAMESERVER) == 0)
        ShowNameserver();
    else if (StrCompare(command, CMD_GETHOSTBYNAME) == 0)
        GetHostByName();
    else if (StrCompare(command, CMD_GETSERVBYNAME) == 0)
        GetServByName();
    else if (StrCompare(command, CMD_GETHOSTNAME) == 0)
        GetHostname();
}

bool DispatchDestinationModal(DB::Address& address) {
    FormType* destinationForm = FrmInitForm(DestinationForm);

    FieldType* ipField = static_cast<FieldType*>(FrmGetObjectPtr(
        destinationForm, FrmGetObjectIndex(destinationForm, DestinationAddressField)));

    MemHandle handle = MemHandleNew(FldGetMaxChars(ipField));
    MemPtr ptr = MemHandleLock(handle);
    StrCopy(static_cast<char*>(ptr), address.ip);
    MemHandleUnlock(handle);

    FldSetTextHandle(ipField, handle);

    FieldType* portField = static_cast<FieldType*>(
        FrmGetObjectPtr(destinationForm, FrmGetObjectIndex(destinationForm, DestinationPortField)));

    handle = MemHandleNew(FldGetMaxChars(portField));
    ptr = MemHandleLock(handle);
    StrPrintF(static_cast<char*>(ptr), "%u", address.port);
    MemHandleUnlock(handle);

    FldSetTextHandle(portField, handle);

    bool save = FrmDoDialog(destinationForm) == DestinationOKButton;
    if (save) {
        StrCopy(address.ip, FldGetTextPtr(ipField));
        address.port = min<UInt32>(StrAToI(FldGetTextPtr(portField)), 0xffff);
    }

    FrmDeleteForm(destinationForm);

    return save;
}

void DispachDestinationReceivePBModal() {
    DB::Address address;
    db.GetDestinationReceivePB(address);

    if (DispatchDestinationModal(address)) db.SetDestinationReceivePB(address);
}

void DispachDestinationSendPBModal() {
    DB::Address address;
    db.GetDestinationSendPB(address);

    if (DispatchDestinationModal(address)) db.SetDestinationSendPB(address);
}

void DispatchConfigModal(FormType* form) {
    ControlType* triggerCtl =
        static_cast<ControlType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, CommandTrigger)));

    const char* command = CtlGetLabel(triggerCtl);
    if (StrCompare(command, CMD_TEST_RECEIVE_PB) == 0)
        DispachDestinationReceivePBModal();
    else if (StrCompare(command, CMD_TEST_SEND_PB) == 0)
        DispachDestinationSendPBModal();
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

                case ConfigButton:
                    DispatchConfigModal(form);
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

        LOG("started");
        db.Initialize();

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
