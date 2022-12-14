#include "resref.h"

#include "utils.h"

//member function pointers
//ResRef* (ResRef::*ResRef_Construct_0)() =
//    SetFP(static_cast<ResRef* (ResRef::*)()>                (&ResRef::Construct),       0x999EC1);
//ResRef* (ResRef::*ResRef_Construct_1CString)(IECString&) =
//    SetFP(static_cast<ResRef* (ResRef::*)(IECString&)>      (&ResRef::Construct),       0x999ED9);
//ResRef* (ResRef::*ResRef_Construct_1LPTSTR)(LPTSTR) =
//    SetFP(static_cast<ResRef* (ResRef::*)(LPTSTR)>          (&ResRef::Construct),       0x999EFB);
//ResRef* (ResRef::*ResRef_Construct_1LPCTSTR)(LPCSTR) =
//    SetFP(static_cast<ResRef* (ResRef::*)(LPCSTR)>          (&ResRef::Construct),       0x999F1D);
//IECString (ResRef::*ResRef_ToString)() =
//    SetFP(static_cast<IECString (ResRef::*)()>              (&ResRef::ToString),        0x999F3F);
//void (ResRef::*ResRef_NullSpaces)() =
//    SetFP(static_cast<void (ResRef::*)()>                   (&ResRef::NullSpaces),      0x999F6C);  
//LPTSTR (ResRef::*ResRef_GetBuffer)() const =
//    SetFP(static_cast<LPTSTR (ResRef::*)() const>           (&ResRef::GetBuffer),       0x999FB3);
//IECString (ResRef::*ResRef_FormatToString)() =
//    SetFP(static_cast<IECString (ResRef::*)()>              (&ResRef::FormatToString),  0x999FC1);
//BOOL (ResRef::*ResRef_IsNotEmpty)() =
//    SetFP(static_cast<BOOL (ResRef::*)()>                   (&ResRef::IsNotEmpty),      0x99A040);
//void (ResRef::*ResRef_ToChar)(LPCTSTR) =
//    SetFP(static_cast<void (ResRef::*)(LPCTSTR)>            (&ResRef::ToChar),          0x99A05A);
//BOOL (ResRef::*ResRef_OpNeq_ResRef)(ResRef&) =
//    SetFP(static_cast<BOOL (ResRef::*)(ResRef&)>            (&ResRef::OpNeq),           0x99A07E);
//BOOL (ResRef::*ResRef_OpNeq_CString)(IECString&) =
//    SetFP(static_cast<BOOL (ResRef::*)(IECString&)>         (&ResRef::OpNeq),           0x99A0C9);
//BOOL (ResRef::*ResRef_OpNeq_LPTSTR)(LPTSTR) =
//    SetFP(static_cast<BOOL (ResRef::*)(LPTSTR)>             (&ResRef::OpNeq),           0x99A0EF);
//BOOL (ResRef::*ResRef_OpEq_CString)(IECString&) =
//    SetFP(static_cast<BOOL (ResRef::*)(IECString&)>         (&ResRef::OpEq),            0x99A1C3);
//BOOL (ResRef::*ResRef_OpEq_LPTSTR)(LPTSTR) =
//    SetFP(static_cast<BOOL (ResRef::*)(LPTSTR)>             (&ResRef::OpEq),            0x99A1E6);
//BOOL (ResRef::*ResRef_OpEq_LPCTSTR)(LPCTSTR) =
//    SetFP(static_cast<BOOL (ResRef::*)(LPCTSTR)>            (&ResRef::OpEq),            0x99A2BD);
//BOOL (ResRef::*ResRef_IsEmpty)() =
//    SetFP(static_cast<BOOL (ResRef::*)()>                   (&ResRef::IsEmpty),         0x99A3A0);
//ResRef (ResRef::*ResRef_OpAssign_ResRef)(ResRef&) =
//    SetFP(static_cast<ResRef (ResRef::*)(ResRef&)>          (&ResRef::OpAssign),        0x99A3BF);
//ResRef (ResRef::*ResRef_OpAssign_LPCTSTR)(LPCTSTR) =
//    SetFP(static_cast<ResRef (ResRef::*)(LPCTSTR)>          (&ResRef::OpAssign),        0x99A3EF);
//ResRef (ResRef::*ResRef_OpAssign_CString)(IECString&) =
//    SetFP(static_cast<ResRef (ResRef::*)(IECString&)>       (&ResRef::OpAssign),        0x99A4A5);
//ResRef (ResRef::*ResRef_OpAssign_LPTSTR)(LPTSTR) =
//    SetFP(static_cast<ResRef (ResRef::*)(LPTSTR)>           (&ResRef::OpAssign),        0x99A5A0);
//void (ResRef::*ResRef_ToUpper)() =
//    SetFP(static_cast<void (ResRef::*)()>                   (&ResRef::ToUpper),         0x99A634);
//ResRef (ResRef::*ResRef_CopyToUpper)(IECString&) =
//    SetFP(static_cast<ResRef (ResRef::*)(IECString&)>       (&ResRef::CopyToUpper),     0x99A6C5);
//void (ResRef::*ResRef_Copy)(ResRef&) =
//    SetFP(static_cast<void (ResRef::*)(ResRef&)>            (&ResRef::Copy),            0x99A7AC);

//implementations
ResRef::~ResRef()                                       {}
_n ResRef::ResRef()                                     { _bgmain(0x999EC1) }
_n ResRef::ResRef(const IECString& s)                   { _bgmain(0x999ED9) }
_n ResRef::ResRef(LPCTSTR sz)                           { _bgmain(0x999EFB) }
_n ResRef::ResRef(unsigned char* sz)                    { _bgmain(0x999F1D) }
_n void       ResRef::CopyToString(IECString& s) const  { _bgmain(0x999F3F) }
_n void       ResRef::Clean()                           { _bgmain(0x999F6C) }
_n LPTSTR     ResRef::GetResRef() const                 { _bgmain(0x999FB3) } // return NOT NULL-ENDED char *
_n IECString  ResRef::GetResRefStr() const              { _bgmain(0x999FC1) }
_n BOOL       ResRef::IsValid() const                   { _bgmain(0x99A040) }
_n void       ResRef::CopyToString(LPTSTR sz) const     { _bgmain(0x99A05A) } // set sz[8]=0 automaticaly

_n BOOL ResRef::operator!=(const ResRef& r) const       { _bgmain(0x99A07E) }
_n BOOL ResRef::operator!=(const IECString& s) const    { _bgmain(0x99A0C9) }
_n BOOL ResRef::operator!=(LPCTSTR sz) const            { _bgmain(0x99A0EF) }
_n BOOL ResRef::operator==(const IECString& s) const    { _bgmain(0x99A1C3) }
_n BOOL ResRef::operator==(LPCTSTR sz) const            { _bgmain(0x99A1E6) }
_n BOOL ResRef::EqualsSubstring(LPCTSTR sz) const       { _bgmain(0x99A2BD) }
_n BOOL ResRef::IsEmpty() const                         { _bgmain(0x99A3A0) }

_n ResRef ResRef::operator=(const ResRef& r)            { _bgmain(0x99A3BF) }
_n ResRef ResRef::operator=(unsigned char* sz)          { _bgmain(0x99A3EF) }
_n ResRef ResRef::operator=(const IECString& s)         { _bgmain(0x99A4A5) }
_n ResRef ResRef::operator=(LPCTSTR sz)                 { _bgmain(0x99A5A0) }
_n void   ResRef::MakeUpper()                           { _bgmain(0x99A634) }
_n ResRef ResRef::operator+=(const IECString& s)        { _bgmain(0x99A6C5) }
_n void   ResRef::CopyTo8Chars(LPTSTR sz)               { _bgmain(0x99A7AC) }

//ResRef::operator LPTSTR() const                         { return (LPTSTR)GetResRef(); }
BOOL ResRef::operator==(const ResRef& r) const          { return !(operator!=(r)); }



typedef unsigned __int64    QWORD;

void __forceinline
ResNameCopy(char* const dst, const char* const src) {
    *(QWORD *)dst = *(QWORD *)src;
}

char gResRefNulled1[9];                         // not multithreading safe !
char gResRefNulled2[9];                         // not multithreading safe !
char gResRefCharPtr[9];                         // not multithreading safe !

LPTSTR ResRef::GetResRefNulled() const {    // not multithreading safe !
    CopyToString(gResRefNulled1);           // copy & set NULL at end
    return gResRefNulled1;
}

LPTSTR ResRef::GetResRefNulled2() const {   // not multithreading safe !
    CopyToString(gResRefNulled2);           // copy & set NULL at end
    return gResRefNulled2;
}

IECString& __stdcall
IECString_From_ResRefCharPtr(IECString& Dst_CString, char* src) { // not multithreading safe !
    ResNameCopy(gResRefCharPtr, src);
    gResRefCharPtr[8] = '\0';
    Dst_CString = gResRefCharPtr;

    return Dst_CString;
}


void __declspec(naked)
GetResRefNulled_asm() {
__asm
{
    push    ecx
    push    edx

    // ecx - this
    call    ResRef::GetResRefNulled
    // result in eax

    pop     edx
    pop     ecx
    ret
}
}


void __declspec(naked)
GetCharPtrNulled_asm() {
__asm
{
    push    ecx
    push    edx

    push    [esp+0Ch]   // char*
    push    ecx         // CString
    call    IECString_From_ResRefCharPtr
    // result in eax

    pop     edx
    pop     ecx
    ret
}
}