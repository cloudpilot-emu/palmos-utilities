#include "field.h"

#include <stdarg.h>

#include "debug.h"
#include "resource.h"
#include "util.h"

#define FIELD_BUFFER_SIZE 512

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
        FldSetScrollPosition(field, 0xffff);
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
