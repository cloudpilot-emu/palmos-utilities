#include "assembly.h"

void CrashExceptionWithBadStack() { asm volatile("
    movel #0xffffff00, %sp
    movew %sp@, %a0
"); }

void CrashDivideByZeroUnsigned() { asm volatile("
    movel %d0, %sp@-
    divuw #0, %d0
    movel %sp@+, %d0
"); }

void CrashDivideByZeroSigned() { asm volatile("
    movel %d0, %sp@-
    divsw #0, %d0
    movel %sp@+, %d0
"); }

void CrashInvalidIstruction() { asm volatile("
    dc.w 0x00c0
"); }

void CrashLine1010() { asm volatile("
    dc.w 0xa000 
"); }

void CrashLine1111() { asm volatile("
    dc.w 0xf000
"); }

void CrashTrap0() { asm volatile("
    trap #0
"); }

void CrashTrap8() { asm volatile("
    trap #8
"); }

void CrashChkFail() { asm volatile("
    movel %d0, %sp@-
    movel #2, %d0
    chk #0, %d0
    movel %sp@+, %d0
"); }

void CrashTrapv() { asm volatile("
    eori #2,%ccr
    trapv
"); }

void CrashReset() { asm volatile("
    reset
"); }