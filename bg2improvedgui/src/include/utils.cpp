#include "stdafx.h"

const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char* days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static _naked void* __cdecl _New(size_t)   { _bgmain(0xA50608) };
static _naked void  __cdecl _Delete(void*) { _bgmain(0xA50631) };
static _naked int   __cdecl _Rand()        { _bgmain(0xA39100) }; // (0 - 32767)


void* __cdecl operator new (size_t size, int i)     { return _New(size); }
void* __cdecl operator new[] (size_t size, int i)   { return _New(size); }
void __cdecl operator delete (void* mem, int i)     { return _Delete(mem); }
void __cdecl operator delete[] (void* mem, int i)   { return _Delete(mem); }

int  __cdecl IERand()           { return _Rand(); }
int  __cdecl IERand(int n)      { return (IERand() * n) >> 15; } // (0 - (n-1))
void __cdecl _IEFree(void* Ptr) { return _Delete(Ptr); }



/////////////////////////////////////////////////////////
// force to use bgmain.exe's new/delete/malloc/free
//
//void* __cdecl  _malloc_dbg(size_t size, int, const char *, int) { return _New(size); }
//
//void* __cdecl  malloc(size_t size) { return _New(size); }
//
//void __cdecl _free_dbg(void* p, int nBlockUse) { return _Delete(p); }
//
//void __cdecl  free(void* p) { return _Delete(p); }
/////////////////////////////////////////////////////////


void* ThisCall(void* func, void* pThis) {
    void* returnVal;
    __asm {
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1){
    void* returnVal;
    __asm {
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2){
    void* returnVal;
    __asm {
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3){
    void* returnVal;
    __asm {
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4){
    void* returnVal;
    __asm {
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5){
    void* returnVal;
    __asm {
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5, void* Arg6){
    void* returnVal;
    __asm {
        push Arg6
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5, void* Arg6, void* Arg7){
    void* returnVal;
    __asm {
        push Arg7
        push Arg6
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5, void* Arg6, void* Arg7, void* Arg8){
    void* returnVal;
    __asm {
        push Arg8
        push Arg7
        push Arg6
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5, void* Arg6, void* Arg7, void* Arg8, void* Arg9){
    void* returnVal;
    __asm {
        push Arg9
        push Arg8
        push Arg7
        push Arg6
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}

void* ThisCall(void* func, void* pThis, void* Arg1, void* Arg2, void* Arg3, void* Arg4, void* Arg5, void* Arg6, void* Arg7, void* Arg8, void* Arg9, void* Arg10){
    void* returnVal;
    __asm {
        push Arg10
        push Arg9
        push Arg8
        push Arg7
        push Arg6
        push Arg5
        push Arg4
        push Arg3
        push Arg2
        push Arg1
        mov ecx, pThis
        call func
        mov returnVal, eax
    }
    return returnVal;
}