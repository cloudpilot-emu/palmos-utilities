#include "bad_stack.h"

void CrashExceptionWithBadStack() { asm volatile("
    movel #0xffffff00, %sp
    movew %sp@, %a0
"); }
