#ifndef _FIELD_H_
#define _FIELD_H_

#include <PalmOS.h>

void InitField(FormType* form);

void UpdateScrollbar(FormType* form);

void ScrollUp(FormType* form);

void ScrollDown(FormType* form);

void HandleScrollBarRepeat(FormType* form, const EventType& event);

void Printf(const char* fmt, ...);

void Clear(FormType* form);

#endif _FIELD_H_
