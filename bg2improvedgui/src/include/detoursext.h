#ifndef DETOURSEXT_H
#define DETOURSEXT_H

#include "stdafx.h"
#include "detours.h"

#define DetourFunction(target, detour)                  \
    DetourTransactionBegin();                           \
    DetourAttach(&(PVOID&)(target), (PVOID)(detour));   \
    DetourTransactionCommit();


// VS2010 IDE highlight calling code as error, but compiler accept without errors
#define DetourMemberFunctionBad(target, detour)                     \
    DetourTransactionBegin();                                       \
    DetourAttach(&(PVOID&)(target), (PVOID)(&(PVOID&)(detour)));    \
    DetourTransactionCommit();

// compatible with C++ standarts/compilers, but require additional temporary member-function pointer
// see member.cpp in Detour samples
#define DetourMemberFunction_compatible(target, detour)     \
    DetourTransactionBegin();                               \
    DetourAttach(&(PVOID&)(target), * (PVOID*)&(detour));   \
    DetourTransactionCommit();

#define DetourMemberFunctionGood(target, classname, funcname, returntype, ...)                  \
    returntype (classname::* classname##_##funcname##)(__VA_ARGS__) = &classname::funcname ;    \
    DetourMemberFunction_compatible(target, classname##_##funcname##)

// most simple, no pointer conversions crap
// DetourAttach() is stdcall
#define DetourMemberFunction(target, detour)    \
    DetourTransactionBegin();                   \
    __asm {                                     \
        __asm push OFFSET detour                \
        __asm push OFFSET target                \
        __asm call DetourAttach                 \
    }                                           \
	DetourTransactionCommit();


void ThisToStd(BYTE* target);
void DetourMemberFunctionOld(PVOID* pTargetFunc, PVOID detourFunc, PVOID targetFunc);

#endif //DETOURSEXT_H