#ifndef UTILS_H
#define UTILS_H

#include "win32def.h"

#ifndef uchar
    typedef unsigned char  uchar;
    typedef unsigned short ushort;
    typedef unsigned long  ulong;
    typedef unsigned int   uint;
#endif

#define GetEip(Eip) __asm {                     \
    __asm mov eax, [ebp+4] /*return address*/   \
    __asm mov Eip, eax                          \
}

#define SetVT(obj, vt) (*(DWORD*)(obj) = (vt))
#define IENew new(0)
#define IENewA new[](0)

#ifdef _DEBUG 
//In _DEBUG, CSyncObject contains an extra CString, screwing up the sizes of game objects
class _CCriticalSection {   //Size 20h
    DWORD* vt;              //0h, AA61F8
    HANDLE m_hObject;       //4h, from CSyncObject
    CRITICAL_SECTION cs;    //8h
};
#endif

template <typename T> T SetFP(T pFunction, DWORD dwAddress) {
    if (sizeof(T) == 4) {
        *(DWORD*)&pFunction = dwAddress;
    }
    return pFunction;
}

void* __cdecl operator new (size_t, int);
void* __cdecl operator new[] (size_t, int);
void __cdecl operator delete (void*, int);
void __cdecl operator delete[] (void*, int);

void __cdecl _IEFree(void*);
#define IEFree(x) _IEFree((void*)(x))

int __cdecl IERand(void);
int __cdecl IERand(int n);

void* ThisCall(void*, void*);
void* ThisCall(void*, void*, void*);
void* ThisCall(void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
void* ThisCall(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

#define THISCALL(func, obj) ThisCall( (void*)(func), (void*)(obj) )
#define THISCALL_1(func, obj, a1) ThisCall( (void*)(func), (void*)(obj), (void*)(a1) )
#define THISCALL_2(func, obj, a1, a2) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2) )
#define THISCALL_3(func, obj, a1, a2, a3) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3) )
#define THISCALL_4(func, obj, a1, a2, a3, a4) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3), (void*)(a4) )
#define THISCALL_5(func, obj, a1, a2, a3, a4, a5) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3), (void*)(a4), (void*)(a5) )
#define THISCALL_6(func, obj, a1, a2, a3, a4, a5, a6) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3), (void*)(a4), (void*)(a5), (void*)(a6) )
#define THISCALL_7(func, obj, a1, a2, a3, a4, a5, a6, a7) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3), (void*)(a4), (void*)(a5), (void*)(a6), (void*)(a7) )
#define THISCALL_8(func, obj, a1, a2, a3, a4, a5, a6, a7, a8) ThisCall( (void*)(func), (void*)(obj), (void*)(a1), (void*)(a2), (void*)(a3), (void*)(a4), (void*)(a5), (void*)(a6), (void*)(a7), (void*)(a8) )

#define _naked  __declspec(naked)
#define _n      _naked

// tell compiler with enabled global optimization (i.e. register allocator)
// that eax/edx/ecx destroyed inside _bgmain(), almost bgmain.exe's functions change these regs
// other way is wrap __pragma(optimize("", off)) < _bgmain(...) > __pragma(optimize("", on))
#define _bgmain(adr)        \
    __asm {                 \
        __asm push adr      \
        __asm ret           \
                            \
        __asm xor eax, eax  \
        __asm xor edx, edx  \
        __asm xor ecx, ecx  \
        }

#define to_bool(x)  ( (x) ? true : false )
#define BOOL_to_bool to_bool


#define AssignWithLimits(what, value, minrange, maxrange)   \
        if ( (int)(value) > (maxrange) ) {                  \
            what = (maxrange);                              \
        } else                                              \
        if ( (int)(value) < (minrange) ) {                  \
            what = (minrange);                              \
        } else {                                            \
            what = (value);                                 \
        }

#define AddWithLimits(what, value, minrange, maxrange)      \
        if ( ((int)(what) + (int)(value)) > (maxrange) ) {  \
            what = (maxrange);                              \
        } else                                              \
        if ( ((int)(what) + (int)(value)) < (minrange) ) {  \
            what = (minrange);                              \
        } else {                                            \
            what += (value);                                \
        }

#define SubWithLimits(what, value, minrange, maxrange)      \
        if ( ((int)(what) - (int)(value)) > (maxrange) ) {  \
            what = (maxrange);                              \
        } else                                              \
        if ( ((int)(what) - (int)(value)) < (minrange) ) {  \
            what = (minrange);                              \
        } else {                                            \
            what -= (value);                                \
        }


//////////////////////////////////////////////////////////////////////
// universal member hook, makes this code:

//  class MyClass {
//  public:
//    void LoadGame(int,int);
//  };
//
//  class DETOUR_MyClass : public MyClass {
//  public:
//    void DETOUR_LoadGame(int,int);
//  };
//
//  void (MyClass::*Tramp_LoadGame)(int,int) =
//    SetFP(static_cast<void (MyClass::*)(int,int)>  (&MyClass::LoadGame),           0x689FCC);
//
//
//  void MyClass::LoadGame(int arg0, int arg1) \
//    { return (this->*Tramp_LoadGame)(arg0, arg1); }

#define H_MEMBERHOOK_1args(classname, methodname, arg1type)          \
    class classname {                                                \
    public:                                                          \
        int methodname(arg1type);                                    \
    };                                                               \
                                                                     \
    class DETOUR_##classname : public classname {                    \
    public:                                                          \
        int DETOUR_##methodname(arg1type);                           \
    };                                                               \
                                                                     \
    extern int (classname::*Tramp_##methodname)(arg1type);

#define H_MEMBERHOOK_2args(classname, methodname, arg1type, arg2ype) \
    class classname {                                                \
    public:                                                          \
        int methodname(arg1type, arg2ype);                           \
    };                                                               \
                                                                     \
    class DETOUR_##classname : public classname {                    \
    public:                                                          \
        int DETOUR_##methodname(arg1type, arg2ype);                  \
    };                                                               \
                                                                     \
    extern int (classname::*Tramp_##methodname)(arg1type, arg2ype);


#define CPP_MEMBERHOOK_2args(classname, methodname, addr, arg1type, arg2ype) \
    int (classname::*Tramp_##methodname)(arg1type, arg2ype) =                \
        SetFP(static_cast<int (classname::*)(arg1type, arg2ype)>  (&classname::##methodname), addr); \
                                                                             \
    int classname::##methodname(arg1type arg1, arg2ype arg2)                 \
        { return (this->*Tramp_##methodname)(arg1, arg2); }

#define CPP_MEMBERHOOK_1args(classname, methodname, addr, arg1type)          \
    int (classname::*Tramp_##methodname)(arg1type) =                         \
        SetFP(static_cast<int (classname::*)(arg1type)>  (&classname::##methodname), addr); \
                                                                             \
    int classname::##methodname(arg1type arg1)                               \
        { return (this->*Tramp_##methodname)(arg1); }


#define MEMBERHOOK_2args(classname, methodname, addr, arg1type, arg2ype) \
    class classname {                               \
    public:                                         \
        int methodname(arg1type, arg2ype);          \
    };                                              \
                                                    \
    class DETOUR_##classname : public classname {   \
    public:                                         \
        int DETOUR_##methodname(arg1type, arg2ype); \
    };                                              \
                                                    \
    int (classname::*Tramp_##methodname)(arg1type, arg2ype) =   \
        SetFP(static_cast<int (classname::*)(arg1type, arg2ype)>  (&classname::##methodname), addr); \
                                                                \
    int classname::##methodname(arg1type arg1, arg2ype arg2)    \
        { return (this->*Tramp_##methodname)(arg1, arg2); }     \



#define MEMBERHOOK_1args(classname, methodname, addr, arg1type) \
    class classname {                               \
    public:                                         \
        int methodname(arg1type);                   \
    };                                              \
                                                    \
    class DETOUR_##classname : public classname {   \
    public:                                         \
        int DETOUR_##methodname(arg1type);          \
    };                                              \
                                                    \
    int (classname::*Tramp_##methodname)(arg1type) =    \
        SetFP(static_cast<int (classname::*)(arg1type)>  (&classname::##methodname), addr); \
                                                        \
    int classname::##methodname(arg1type arg1)          \
        { return (this->*Tramp_##methodname)(arg1); }   \


#define MEMBERHOOK_0args(classname, methodname, addr) \
    class classname {                               \
    public:                                         \
        int methodname();                           \
    };                                              \
                                                    \
    class DETOUR_##classname : public classname {   \
    public:                                         \
        int DETOUR_##methodname();                  \
    };                                              \
                                                    \
    int (classname::*Tramp_##methodname)() =        \
        SetFP(static_cast<int (classname::*)()>  (&classname::##methodname), addr); \
                                                    \
    int classname::##methodname()                   \
        { return (this->*Tramp_##methodname)(); }   \

//////////////////////////////////////////////////////////////////////

#endif //UTILS_H
