#include <PalmOS.h>

#include "debug.h"
#include "resource.h"

#define FIELD_BUFFER_SIZE 512

template <class T>
T min(T x, T y) {
    return x < y ? x : y;
}

template <class T>
T max(T x, T y) {
    return x > y ? x : y;
}

static const char* REMOTE_IP = "127.0.0.1";
static const UInt16 REMOTE_PORT = 6666;

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

    StrCopy(fieldContent, "Initialized.\n\n");
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
        LOG("Printf: buffer overflowed, memory corruption ahead");

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

void Clear(FormType* form) {
    FieldType* field =
        static_cast<FieldType*>(FrmGetObjectPtr(form, FrmGetObjectIndex(form, OutputField)));

    MemHandle handle = FldGetTextHandle(field);
    char* fieldBuffer = static_cast<char*>(MemHandleLock(handle));

    fieldBuffer[0] = 0;

    MemHandleUnlock(handle);

    FldSetTextHandle(field, handle);
    FldDrawField(field);
    UpdateScrollbar(form);
}

void TestSocketOptionGet() { Printf("Testing NetLibSocketOptionGet...\n\n"); }

void TestReceivePB() { Printf("Testing NetLibReceivePB...\n\n"); }

void TestSendPB() {
    NetSocketRef socket = -1;
    Err err;
    static const char* words[] = {"one ", "two ", "three "};
    const UInt8 wordCount = 5;

    Printf("Testing NetLibSendPB...\n\n");

    Printf("opening netlib...\n");

    UInt16 refNum;
    if (SysLibFind("Net.lib", &refNum) != 0) {
        Printf("Net.lib not found\n");
        return;
    }

    UInt16 ifErr;
    if (NetLibOpen(refNum, &ifErr) != 0) {
        Printf("failed to open netlib\n");
        return;
    }

    Printf("connecting to %s:%u\n", REMOTE_IP, REMOTE_PORT);

    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP,
                              -1, &err);
    if (err != 0) {
        Printf("failed to open socket\n");
        goto cleanup;
    }

    NetSocketAddrINType addr;
    addr.family = netSocketAddrINET;
    addr.addr = NetLibAddrAToIN(refNum, REMOTE_IP);
    addr.port = REMOTE_PORT;
    if (NetLibSocketConnect(refNum, socket, reinterpret_cast<NetSocketAddrType*>(&addr),
                            sizeof(addr), -1, &err) != 0) {
        Printf("failed to connect\n");
        goto cleanup;
    }

    NetIOVecType chunks[wordCount + 1];
    for (int i = 0; i < wordCount; i++) {
        UInt8 word = SysRandom(0) % 3;

        chunks[i].bufP = (UInt8*)(words[word]);
        chunks[i].bufLen = StrLen(words[word]) + 1;
    }

    chunks[wordCount].bufP = (UInt8*)"\n";
    chunks[wordCount].bufLen = 1;

    NetIOParamType params;
    params.addrP = 0;
    params.addrLen = 0;
    params.iov = chunks;
    params.iovLen = wordCount + 1;
    params.accessRights = 0;
    params.accessRightsLen = 0;

    Int16 bytesSent;
    bytesSent = NetLibSendPB(refNum, socket, &params, 0, -1, &err);
    if (bytesSent > 0)
        Printf("successfully sent %i bytes\n", bytesSent);
    else
        Printf("failed to send\n");

cleanup:
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);
    NetLibClose(refNum, true);

    Printf("\n");
}

void TestDmSend() { Printf("Testing NetLibDmSend...\n\n"); }

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
