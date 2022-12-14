#include "cstringex.h"

#include "utils.h"

//extern IECString& (IECString::*IECString_Construct_2TCHAR_int)(TCHAR, int) =
//    SetFP(static_cast<IECString& (IECString::*)(TCHAR, int)>                            (&IECString::Construct),                0xA4D075);
//extern IECString& (IECString::*IECString_Construct_2LPCSTR_int)(LPCSTR, int) =
//    SetFP(static_cast<IECString& (IECString::*)(LPCSTR, int)>                           (&IECString::Construct),                0xA4D0A9);
//extern IECString& (IECString::*IECString_Construct_2LPCWSTR_int)(LPCWSTR, int) =
//    SetFP(static_cast<IECString& (IECString::*)(LPCWSTR, int)>                          (&IECString::Construct),                0xA4D0DB);
//extern const IECString& (IECString::*IECString_OpAssign_TCHAR)(TCHAR) =
//    SetFP(static_cast<const IECString& (IECString::*)(TCHAR)>                           (&IECString::OpAssign),                 0xA4D124);
//extern IECString (AFXAPI *IECString_OpAdd_IECString_TCHAR)(const IECString&, TCHAR) =
//    SetFP(static_cast<IECString (AFXAPI *)(const IECString&, TCHAR)>                    (&IECString_OpAdd),                     0xA4D139);
//extern IECString (AFXAPI *IECString_OpAdd_TCHAR_IECString)(TCHAR, const IECString&) =
//    SetFP(static_cast<IECString (AFXAPI *)(TCHAR, const IECString&)>                    (&IECString_OpAdd),                     0xA4D19B);
//extern int (IECString::*IECString_Delete)(int, int) =
//    SetFP(static_cast<int (IECString::*)(int, int)>                                     (&IECString::Delete),                   0xA4D1FD);
//extern int (IECString::*IECString_Insert_int_TCHAR)(int, TCHAR) =
//    SetFP(static_cast<int (IECString::*)(int, TCHAR)>                                   (&IECString::Insert),                   0xA4D255);
//extern int (IECString::*IECString_Insert_int_LPCTSTR)(int, LPCTSTR) =
//    SetFP(static_cast<int (IECString::*)(int, LPCTSTR)>                                 (&IECString::Insert),                   0xA4D2D9);
//extern int (IECString::*IECString_Replace_TCHAR)(TCHAR, TCHAR) =
//    SetFP(static_cast<int (IECString::*)(TCHAR, TCHAR)>                                 (&IECString::Replace),                  0xA4D38E);
//extern int (IECString::*IECString_Replace_LPCTSTR)(LPCTSTR, LPCTSTR) =
//    SetFP(static_cast<int (IECString::*)(LPCTSTR, LPCTSTR)>                             (&IECString::Replace),                  0xA4D3CB);
//extern int (IECString::*IECString_Remove)(TCHAR) =
//    SetFP(static_cast<int (IECString::*)(TCHAR)>                                        (&IECString::Remove),                   0xA4D52E);
//extern IECString (IECString::*IECString_Mid_int_int)(int, int) const =
//    SetFP(static_cast<IECString (IECString::*)(int, int) const>                         (&IECString::Mid),                      0xA4D5A2);
//extern IECString (IECString::*IECString_Mid_int)(int) const =
//    SetFP(static_cast<IECString (IECString::*)(int) const>                              (&IECString::Mid),                      0xA4D57F);
//extern IECString (IECString::*IECString_Right)(int) const =
//    SetFP(static_cast<IECString (IECString::*)(int) const>                              (&IECString::Right),                    0xA4D638);
//extern IECString (IECString::*IECString_Left)(int) const =
//    SetFP(static_cast<IECString (IECString::*)(int) const>                              (&IECString::Left),                     0xA4D6B4);
//extern IECString (IECString::*IECString_SpanIncluding)(LPCTSTR) const =
//    SetFP(static_cast<IECString (IECString::*)(LPCTSTR) const>                          (&IECString::SpanIncluding),            0xA4D72C);
//extern IECString (IECString::*IECString_SpanExcluding)(LPCTSTR) const =
//    SetFP(static_cast<IECString (IECString::*)(LPCTSTR) const>                          (&IECString::SpanExcluding),            0xA4D757);
//extern int (IECString::*IECString_ReverseFind)(TCHAR) const =
//    SetFP(static_cast<int (IECString::*)(TCHAR) const>                                  (&IECString::ReverseFind),              0xA4D782);
//extern int (IECString::*IECString_Find_LPCTSTR)(LPCTSTR) const =
//    SetFP(static_cast<int (IECString::*)(LPCTSTR) const>                                (&IECString::Find),                     0xA4D7A4);
//extern int (IECString::*IECString_Find_LPCTSTR_int)(LPCTSTR, int) const =
//    SetFP(static_cast<int (IECString::*)(LPCTSTR, int) const>                           (&IECString::Find),                     0xA4D7B2);
//extern void (IECString::*IECString_FormatV)(LPCTSTR, va_list) =
//    SetFP(static_cast<void (IECString::*)(LPCTSTR, va_list)>                            (&IECString::FormatV),                  0xA4D7DD);

/*extern void (AFX_CDECL IECString::*IECString_Format_LPCTSTR)(LPCTSTR, ...) =
    SetFP(static_cast<void (AFX_CDECL IECString::*)(LPCTSTR, ...)>                      (&IECString::Format),                   0xA4DAE5);
extern void (AFX_CDECL IECString::*IECString_Format_UINT)(UINT, ...) =
    SetFP(static_cast<void (AFX_CDECL IECString::*)(UINT, ...)>                         (&IECString::Format),                   0xA4DAF8);
extern void (AFX_CDECL IECString::*IECString_FormatMessage_LPCTSTR)(LPCTSTR, ...) =
    SetFP(static_cast<void (AFX_CDECL IECString::*)(LPCTSTR, ...)>                      (&IECString::FormatMessage),            0xA4DB41);
extern void (AFX_CDECL IECString::*IECString_FormatMessage_UINT)(UINT, ...) =
    SetFP(static_cast<void (AFX_CDECL IECString::*)(UINT, ...)>                         (&IECString::FormatMessage),            0xA4DB8D);*/

//extern void (IECString::*IECString_TrimRight_LPCTSTR)(LPCTSTR) =
//    SetFP(static_cast<void (IECString::*)(LPCTSTR)>                                     (&IECString::TrimRight),                0xA4DC10);
//extern void (IECString::*IECString_TrimRight_TCHAR)(TCHAR) =
//    SetFP(static_cast<void (IECString::*)(TCHAR)>                                       (&IECString::TrimRight),                0xA4DC63);
//extern void (IECString::*IECString_TrimRight)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::TrimRight),                0xA4DCA5);
//extern void (IECString::*IECString_TrimLeft_LPCTSTR)(LPCTSTR) =
//    SetFP(static_cast<void (IECString::*)(LPCTSTR)>                                     (&IECString::TrimLeft),                 0xA4DCF1);
//extern void (IECString::*IECString_TrimLeft_TCHAR)(TCHAR) =
//    SetFP(static_cast<void (IECString::*)(TCHAR)>                                       (&IECString::TrimLeft),                 0xA4DD59);
//extern void (IECString::*IECString_TrimLeft)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::TrimLeft),                 0xA4DD9C);
    
//IECString& (IECString::*IECString_Construct_0)() =
//    SetFP(static_cast<IECString& (IECString::*)()>                                      (&IECString::Construct),                0xA5063C);
//IECString& (IECString::*IECString_Construct_1CString)(const IECString&) =
//    SetFP(static_cast<IECString& (IECString::*)(const IECString&)>                      (&IECString::Construct),                0xA50642);
//void (IECString::*IECString_Empty)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::Empty),                    0xA50858);
//void (IECString::*IECString_Deconstruct)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::Deconstruct),              0xA508CD);
//IECString& (IECString::*IECString_Construct_1LPCSTR)(LPCSTR) =
//    SetFP(static_cast<IECString& (IECString::*)(LPCSTR)>                                (&IECString::Construct),                0xA5093B);
//IECString& (IECString::*IECString_Construct_1LPCWSTR)(LPCWSTR) =
//    SetFP(static_cast<IECString& (IECString::*)(LPCWSTR)>                               (&IECString::Construct),                0xA5098D);
//const IECString& (IECString::*IECString_OpAssign_CString)(const IECString&) =
//    SetFP(static_cast<const IECString& (IECString::*)(const IECString&)>                (&IECString::OpAssign),                 0xA50A06);
//const IECString& (IECString::*IECString_OpAssign_LPCSTR)(LPCSTR) =
//    SetFP(static_cast<const IECString& (IECString::*)(LPCSTR)>                          (&IECString::OpAssign),                 0xA50A56);
//const IECString& (IECString::*IECString_OpAssign_LPCWSTR)(LPCWSTR) =
//    SetFP(static_cast<const IECString& (IECString::*)(LPCWSTR)>                         (&IECString::OpAssign),                 0xA50A7D);
//IECString (AFXAPI *IECString_OpAdd_CString_CString)(const IECString&, const IECString&) =
//    SetFP(static_cast<IECString (AFXAPI *)(const IECString&, const IECString&)>         (&IECString_OpAdd),                     0xA50AFC);
//IECString (AFXAPI *IECString_OpAdd_CString_LPCTSTR)(const IECString&, LPCTSTR) =
//    SetFP(static_cast<IECString (AFXAPI *)(const IECString&, LPCTSTR)>                  (&IECString_OpAdd),                     0xA50B62);
//IECString (AFXAPI *IECString_OpAdd_LPCTSTR_CString)(LPCTSTR, const IECString&) =
//    SetFP(static_cast<IECString (AFXAPI *)(LPCTSTR, const IECString&)>                  (&IECString_OpAdd),                     0xA50BD6);
//const IECString& (IECString::*IECString_OpAddEq_LPCTSTR)(LPCTSTR) =
//    SetFP(static_cast<const IECString& (IECString::*)(LPCTSTR)>                         (&IECString::OpAddEq),                  0xA50CA9);
//const IECString& (IECString::*IECString_OpAddEq_TCHAR)(TCHAR) =
//    SetFP(static_cast<const IECString& (IECString::*)(TCHAR)>                           (&IECString::OpAddEq),                  0xA50CD0);
//const IECString& (IECString::*IECString_OpAddEq_CString)(const IECString&) =
//    SetFP(static_cast<const IECString& (IECString::*)(const IECString&)>                (&IECString::OpAddEq),                  0xA50CE5);
//LPTSTR (IECString::*IECString_GetBuffer)(int) =
//    SetFP(static_cast<LPTSTR (IECString::*)(int)>                                       (&IECString::GetBuffer),                0xA50CFD);
//void (IECString::*IECString_ReleaseBuffer)(int) =
//    SetFP(static_cast<void (IECString::*)(int)>                                         (&IECString::ReleaseBuffer),            0xA50D4C);
//LPTSTR (IECString::*IECString_GetBufferSetLength)(int) =
//    SetFP(static_cast<LPTSTR (IECString::*)(int)>                                       (&IECString::GetBufferSetLength),       0xA50D74);
//void (IECString::*IECString_FreeExtra)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::FreeExtra),                0xA50D94);
//LPTSTR (IECString::*IECString_LockBuffer)() =
//    SetFP(static_cast<LPTSTR (IECString::*)()>                                          (&IECString::LockBuffer),               0xA50DC7);
//void (IECString::*IECString_UnlockBuffer)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::UnlockBuffer),             0xA50DD9);
//int (IECString::*IECString_Find_TCHAR)(TCHAR) const =
//    SetFP(static_cast<int (IECString::*)(TCHAR) const>                                  (&IECString::Find),                     0xA50DEE);
//int (IECString::*IECString_Find_TCHAR_int)(TCHAR, int) const =
//    SetFP(static_cast<int (IECString::*)(TCHAR, int) const>                             (&IECString::Find),                     0xA50DFC);
//int (IECString::*IECString_FindOneOf)(LPCTSTR) const =
//    SetFP(static_cast<int (IECString::*)(LPCTSTR) const>                                (&IECString::FindOneOf),                0xA50E29);
//void (IECString::*IECString_MakeUpper)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::MakeUpper),                0xA50E49);
//void (IECString::*IECString_MakeLower)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::MakeLower),                0xA50E5B);
//void (IECString::*IECString_MakeReverse)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::MakeReverse),              0xA50E6D);
//void (IECString::*IECString_SetAt)(int, TCHAR) =
//    SetFP(static_cast<void (IECString::*)(int, TCHAR)>                                  (&IECString::SetAt),                    0xA50E7F);
//void (IECString::*IECString_AnsiToOem)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::AnsiToOem),                0xA50E98);
//void (IECString::*IECString_OemToAnsi)() =
//    SetFP(static_cast<void (IECString::*)()>                                            (&IECString::OemToAnsi),                0xA50EAC);
//BOOL (IECString::*IECString_LoadStringA)(UINT) =
//    SetFP(static_cast<BOOL (IECString::*)(UINT)>                                        (&IECString::LoadStringA),              0xA52004);

_n IECString::IECString(TCHAR ch, int nRepeat)                              { _bgmain(0xA4D075) }
_n IECString::IECString(LPCSTR lpch, int nLength)                           { _bgmain(0xA4D0A9) }
_n IECString::IECString(LPCWSTR lpch, int nLength)                          { _bgmain(0xA4D0DB) }

_n const IECString& IECString::operator=(TCHAR ch)                          { _bgmain(0xA4D124) }
_n IECString AFXAPI operator+(const IECString& string, TCHAR ch)            { _bgmain(0xA4D139) }
_n IECString AFXAPI operator+(TCHAR ch, const IECString& string)            { _bgmain(0xA4D19B) }

_n int IECString::Delete(int nIndex, int nCount)                            { _bgmain(0xA4D1FD) }
_n int IECString::Insert(int nIndex, TCHAR ch)                              { _bgmain(0xA4D255) }
_n int IECString::Insert(int nIndex, LPCTSTR pstr)                          { _bgmain(0xA4D2D9) }
_n int IECString::Replace(TCHAR chOld, TCHAR chNew)                         { _bgmain(0xA4D38E) }
_n int IECString::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)                 { _bgmain(0xA4D3CB) }
_n int IECString::Remove(TCHAR chRemove)                                    { _bgmain(0xA4D52E) }
_n IECString IECString::Mid(int nFirst) const                               { _bgmain(0xA4D57F) }
_n IECString IECString::Mid(int nFirst, int nCount) const                   { _bgmain(0xA4D5A2) }
_n IECString IECString::Right(int nCount) const                             { _bgmain(0xA4D638) }
_n IECString IECString::Left(int nCount) const                              { _bgmain(0xA4D6B4) }
_n IECString IECString::SpanIncluding(LPCTSTR lpszCharSet) const            { _bgmain(0xA4D72C) }
_n IECString IECString::SpanExcluding(LPCTSTR lpszCharSet) const            { _bgmain(0xA4D757) }
_n int IECString::ReverseFind(TCHAR ch) const                               { _bgmain(0xA4D782) }
_n int IECString::Find(LPCTSTR lpszSub) const                               { _bgmain(0xA4D7A4) }
_n int IECString::Find(LPCTSTR lpszSub, int nStart) const                   { _bgmain(0xA4D7B2) }
_n void IECString::FormatV(LPCTSTR lpszFormat, va_list argList)             { _bgmain(0xA4D7DD) }
_n void AFX_CDECL IECString::Format(LPCTSTR lpszFormat, ...)                { _bgmain(0xA4DAE5) }

_n void IECString::TrimRight(LPCTSTR lpszTargets)                               { _bgmain(0xA4DC10) }
_n void IECString::TrimRight(TCHAR chTarget)                                    { _bgmain(0xA4DC63) }
_n void IECString::TrimRight()                                                  { _bgmain(0xA4DCA5) }
_n void IECString::TrimLeft(LPCTSTR lpszTargets)                                { _bgmain(0xA4DCF1) }
_n void IECString::TrimLeft(TCHAR chTarget)                                     { _bgmain(0xA4DD59) }
_n void IECString::TrimLeft()                                                   { _bgmain(0xA4DD9C) }

IECString::IECString()                                                          { m_pchData = reinterpret_cast<LPTSTR>(0xB72024); }
_n IECString::IECString(const IECString& stringSrc)                             { _bgmain(0xA50642) }
_n void IECString::Empty()                                                      { _bgmain(0xA50858) }
_n IECString::~IECString()                                                      { _bgmain(0xA508CD) }
_n IECString::IECString(LPCSTR lpsz)                                            { _bgmain(0xA5093B) }
_n IECString::IECString(LPCWSTR lpsz)                                           { _bgmain(0xA5098D) }

_n const IECString& IECString::operator=(const IECString& stringSrc)            { _bgmain(0xA50A06) }
_n const IECString& IECString::operator=(LPCSTR lpsz)                           { _bgmain(0xA50A56) }
_n const IECString& IECString::operator=(LPCWSTR lpsz)                          { _bgmain(0xA50A7D) }
_n IECString AFXAPI operator+(const IECString& string1,const IECString& string2){ _bgmain(0xA50AFC) }
_n IECString AFXAPI operator+(const IECString& string, LPCTSTR lpsz)            { _bgmain(0xA50B62) }
_n IECString AFXAPI operator+(LPCTSTR lpsz, const IECString& string)            { _bgmain(0xA50BD6) }
_n const IECString& IECString::operator+=(LPCTSTR lpsz)                         { _bgmain(0xA50CA9) }
_n const IECString& IECString::operator+=(TCHAR ch)                             { _bgmain(0xA50CD0) }
_n const IECString& IECString::operator+=(const IECString& string)              { _bgmain(0xA50CE5) }

_n LPTSTR IECString::GetBuffer(int nMinBufLength)                               { _bgmain(0xA50CFD) }
_n void   IECString::ReleaseBuffer(int nNewLength)                              { _bgmain(0xA50D4C) }
_n LPTSTR IECString::GetBufferSetLength(int nNewLength)                         { _bgmain(0xA50D74) }
_n void   IECString::FreeExtra()                                                { _bgmain(0xA50D94) }
_n LPTSTR IECString::LockBuffer()                                               { _bgmain(0xA50DC7) }
_n void   IECString::UnlockBuffer()                                             { _bgmain(0xA50DD9) }
_n int    IECString::Find(TCHAR ch) const                                       { _bgmain(0xA50DEE) }
_n int    IECString::Find(TCHAR ch, int nStart) const                           { _bgmain(0xA50DFC) }
_n int    IECString::FindOneOf(LPCTSTR lpszCharSet) const                       { _bgmain(0xA50E29) }
_n void   IECString::MakeUpper()                                                { _bgmain(0xA50E49) }
_n void   IECString::MakeLower()                                                { _bgmain(0xA50E5B) }
_n void   IECString::MakeReverse()                                              { _bgmain(0xA50E6D) }
_n void   IECString::SetAt(int nIndex, TCHAR ch)                                { _bgmain(0xA50E7F) }
_n void   IECString::AnsiToOem()                                                { _bgmain(0xA50E98) }
_n void   IECString::OemToAnsi()                                                { _bgmain(0xA50EAC) }
_n BOOL   IECString::LoadString(UINT nID)                                       { _bgmain(0xA52004) }

void* IECString::operator new(size_t size)                      { return ::operator new(size, 0); }
void IECString::operator delete(void* mem)                      { return ::operator delete(mem, 0); }
void IECString::operator delete[](void* mem)                    { return ::operator delete[](mem, 0); }
IECString::operator CString() const                             { return reinterpret_cast<const CString&>(*this); }
IECString::operator LPCTSTR() const                             { return m_pchData; }                 // 0x493EB0
IECString::operator LPTSTR()  const                             { return m_pchData; }
_n int   IECString::GetLength() const                           { __asm jmp CString::GetLength      }
_n BOOL  IECString::IsEmpty() const                             { __asm jmp CString::IsEmpty        }
_n TCHAR IECString::GetAt(int nIndex) const                     { __asm jmp CString::GetAt          }
_n TCHAR IECString::operator[](int nIndex) const                { __asm jmp CString::operator[]     }
_n int   IECString::Compare(LPCTSTR lpsz) const                 { __asm jmp CString::Compare        }
_n int   IECString::CompareNoCase(LPCTSTR lpsz) const           { __asm jmp CString::CompareNoCase	} // 0x493EC0
_n int   IECString::Collate(LPCTSTR lpsz) const                 { __asm jmp CString::Collate        }
_n int   IECString::CollateNoCase(LPCTSTR lpsz) const           { __asm jmp CString::CollateNoCase  }
_n int   IECString::GetAllocLength() const                      { __asm jmp CString::GetAllocLength }





// bgmain already has MFC 4.x implementation, commented

//re-implementations from STREX.CPP
//void AFX_CDECL IECString::Format(LPCTSTR lpszFormat, ...) {
//	va_list argList;
//	va_start(argList, lpszFormat);
//	FormatV(lpszFormat, argList);
//	va_end(argList);
//	return;
//}

//void AFX_CDECL IECString::Format(UINT nFormatID, ...) {
//    IECString strFormat;
//    VERIFY(strFormat.LoadString(nFormatID) != 0);
//
//    va_list argList;
//    va_start(argList, nFormatID);
//    FormatV(strFormat, argList);
//    va_end(argList);
//    return;
//}

//void AFX_CDECL IECString::FormatMessage(LPCTSTR lpszFormat, ...) {
//    // format message into temporary buffer lpszTemp
//    va_list argList;
//    va_start(argList, lpszFormat);
//    LPTSTR lpszTemp;
//
//    if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
//        lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 ||
//        lpszTemp == NULL)
//    {
//        AfxThrowMemoryException();
//    }
//
//    // assign lpszTemp into the resulting string and free the temporary
//    *this = lpszTemp;
//    LocalFree(lpszTemp);
//    va_end(argList);
//    return;
//}

//void AFX_CDECL IECString::FormatMessage(UINT nFormatID, ...) {
//    // get format string from string table
//    IECString strFormat;
//    VERIFY(strFormat.LoadString(nFormatID) != 0);
//
//    // format message into temporary buffer lpszTemp
//    va_list argList;
//    va_start(argList, nFormatID);
//    LPTSTR lpszTemp;
//    if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
//        strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 ||
//        lpszTemp == NULL)
//    {
//        AfxThrowMemoryException();
//    }
//
//    // assign lpszTemp into the resulting string and free lpszTemp
//    *this = lpszTemp;
//    LocalFree(lpszTemp);
//    va_end(argList);
//    return;
//}
