#include <PalmOS.h>

#include "resource.h"

struct __attribute__((packed)) DbEntry {
    LocalID localID;
    UInt16 cardNo;
};

Err WriteProcStub(const void* dataP, UInt32* sizeP, void* userDataP) { return errNone; }

void Export() {
    void* dummyPtr = NULL;
    const UInt16 numCards = MemNumCards();
    UInt32 numDatabases = 0;

    for (UInt16 cardNo = 0; cardNo < numCards; cardNo++) numDatabases += DmNumDatabases(cardNo);

    DbEntry* entries = reinterpret_cast<DbEntry*>(MemPtrNew(numDatabases * sizeof(DbEntry)));
    dummyPtr = MemPtrNew(44);

    UInt32 entryIdx = 0;
    for (UInt16 cardNo = 0; cardNo < numCards; cardNo++) {
        const UInt32 databasesPerCard = DmNumDatabases(cardNo);

        for (UInt32 dbIdx = 0; dbIdx < databasesPerCard; dbIdx++) {
            entries[entryIdx].localID = DmGetDatabase(cardNo, dbIdx);
            entries[entryIdx].cardNo = cardNo;

            if (!entries[entryIdx].localID) {
                ErrAlertCustom(0, "Failed to get DB", NULL, NULL);
                goto cleanup;
            }

            entryIdx++;
        }
    }

    MemPtrFree(dummyPtr);
    dummyPtr = NULL;

    for (UInt32 entryIdx = 0; entryIdx < numDatabases; entryIdx++) {
        dummyPtr = MemPtrNew(3);

        if (ExgDBWrite(WriteProcStub, NULL, "", entries[entryIdx].localID,
                       entries[entryIdx].cardNo) != errNone) {
            ErrAlertCustom(0, "Failed to write DB", NULL, NULL);
            goto cleanup;
        }

        MemPtrFree(dummyPtr);
        dummyPtr = NULL;
    }

    char message[128];
    StrPrintF(message, "Exported %u databases", (UInt16)numDatabases);
    ErrAlertCustom(0, message, NULL, NULL);

cleanup:
    MemPtrFree(entries);
    if (dummyPtr) MemPtrFree(dummyPtr);
}

Boolean MainFormHandler(EventType* event) {
    FormType* form = FrmGetActiveForm();

    switch (event->eType) {
        case frmOpenEvent:
            FrmDrawForm(form);

            return true;
        case ctlSelectEvent:
            switch (event->data.ctlSelect.controlID) {
                case ExportButton:
                    Export();
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

        FrmGotoForm(MainForm);

        do {
            EvtGetEvent(&event, evtWaitForever);

            if (SysHandleEvent(&event)) continue;

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
