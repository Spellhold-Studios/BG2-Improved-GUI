#include "patch.h"

#include <algorithm>
#include <vector>

#include "stdafx.h"
#include "options.h"
#include "scrcore.h"
#include "console.h"
#include "log.h"
#include "ChitinCore.h"
#include "DialogCore.h"
#include "EffectOpcode.h"
#include "EngineChargen.h"
#include "InfGameCommon.h"
#include "InfGameCore.h"
#include "ItemCommon.h"
#include "ScriptAction.h"
#include "ScriptParser.h"
#include "ScriptTrigger.h"
#include "StoreCore.h"
#include "UserCommon.h"
#include "ObjectCreature.h"
#include "ContainerPanel.h"
#include "LearnableScrolls.h"
#include "InventoryScreenLabels.h"
#include "InteractiveJournal.h"
#include "GreyBackground.h"
#include "HandOffSlot.h"
#include "LogActiveSpells.h"
#include "EventsText.h"
#include "AnimationCore.h"
#include "UserPriestBook.h"
#include "EffectCore.h"
#include "RenderPortrait.h"
#include "NightMare.h"
#include "AreaCommon.h"
#include "Shaman.h"
#include "objtrig.h"
#include "EngineCommon.h"
#include "VideoCommon.h"
#include "InfButtonArray.h"
#include "EngineMageBook.h"
#include "main.h"
#include "SpellMenu.h"
#include "EngineWorld.h"
#include "SoundCore.h"
#include "ObjectCore.h"
#include "ItemCore.h"

CRuleTable** ClassAbilityTable;
HMODULE DSOAL_DLL;


uchar DamageTypeACModText[] = " +DamageTypeACMod:%d";
extern CRITICAL_SECTION gCriticalSectionSetAnimationSequence;
extern bool             gX2Render;

typedef unsigned int    uint;
typedef unsigned char   uchar;

#define COMMIT_vDataList \
        vPatchList.push_back(Patch(vDataList)); \
        vDataList.clear();

#define pack_bytes(x) x, sizeof(x)

#define CallInject(a,b,c)   CallInject_vDataList(     a, (void *)b, c, sizeof(c), &vDataList) // opcode, opcode
#define RelativeInject(a,b) RelativeInject_vDataList( a, (void *)b, &vDataList)               // call bbb pc-relative
#define PointerInject(a,b)  PointerInject_vDataList(  a, (void *)b, &vDataList)               // [bbb]
#define JumpInject(a,b)     JumpInject_vDataList(     a, (void *)b, &vDataList)               // jmp far bbb

void
RelativeInject_vDataList (
    DWORD Addr,
    void *InjectProc,
    std::vector<Data> *vDataList
    )
{
    void* ptr = InjectProc;
    DWORD address = (DWORD)ptr - 5  - (Addr);
    uchar *offset = (uchar*) &address;
    vDataList->push_back( Data(Addr + 1, 4, offset) );
}

void
PointerInject_vDataList (
    DWORD Addr,
    void *InjectProc,
    std::vector<Data> *vDataList
    )
{
    DWORD address = (DWORD) InjectProc;
    uchar *offset = (uchar*) &address;
    vDataList->push_back( Data(Addr , 4, offset) );
}

void
JumpInject_vDataList (
    DWORD Addr,
    void *InjectProc,
    std::vector<Data> *vDataList
    )
{
    uchar byte_jump[] = {0xE9};
    void* ptr = InjectProc;
    DWORD address = (DWORD)ptr - 5  - (Addr);
    uchar *offset = (uchar*) &address;
    vDataList->push_back( Data(Addr, 1, byte_jump) );
    vDataList->push_back( Data(Addr + 1, 4, offset) );
}

void
CallInject_vDataList (
    DWORD               Addr,
    void                *InjectProc,
    uchar               *patchdata,
    SIZE_T              patchsize,
    std::vector<Data>   *vDataList
    )
{
    vDataList->push_back( Data(Addr, patchsize, patchdata) );
    RelativeInject_vDataList(Addr, InjectProc, vDataList);
}

void MakeWord_x2_(ushort *addr, std::vector<Data> *vDataList) {
    ushort buf = (*addr) * 2; // x2
    vDataList->push_back( Data((DWORD)addr, 2, (uchar *) &buf) );
}

#define MakeWord_x2(x) MakeWord_x2_( (ushort *)(x), &vDataList )

//Data implementation
Data::Data() : dwAddress(0), nSize(0), szBytes(NULL), szSrc(NULL) {}

Data::Data(const Data& d) : dwAddress(d.dwAddress), nSize(d.nSize) {
    if (d.nSize > 0) {
        szBytes = new uchar[d.nSize];
        memcpy(szBytes, d.szBytes, d.nSize);
        if (d.szSrc) {
            szSrc = new uchar[d.nSize];
            memcpy(szSrc, d.szSrc, d.nSize);
        } else szSrc = NULL;
    } else {
        szBytes = NULL;
        szSrc = NULL;
    }
}

Data::Data(DWORD address, SIZE_T size, uchar bytes[]) : dwAddress(address), nSize(size) {
    if (bytes) {
        szBytes = new uchar[size];
        memcpy(szBytes, bytes, size);
    } else {
        szBytes = NULL;
        nSize = 0;
    }
    szSrc = NULL;
}

Data::Data(DWORD address, SIZE_T size, uchar bytes[], uchar src[]) : dwAddress(address), nSize(size) {
    if (bytes) {
        szBytes = new uchar[size];
        memcpy(szBytes, bytes, size);
    } else {
        szBytes = NULL;
        nSize = 0;
    }

    if (src) {
        szSrc = new uchar[size];
        memcpy(szSrc, src, size);
    } else szSrc = NULL;
}

Data::~Data() {
    if (szBytes) {
        delete[] szBytes;
        szBytes = NULL;
    }
    if (szSrc) {
        delete[] szSrc;
        szSrc = NULL;
    }
}

Data& Data::operator=(const Data& d) {
    dwAddress = d.dwAddress;
    nSize = d.nSize;

    if (szBytes) {
        delete[] szBytes;
        szBytes = NULL;
    }
    if (szSrc) {
        delete[] szSrc;
        szSrc = NULL;
    }
    if (nSize > 0) {
        szBytes = new uchar[nSize];
        memcpy(szBytes, d.szBytes, nSize);
        if (d.szSrc) {
            szSrc = new uchar[d.nSize];
            memcpy(szSrc, d.szSrc, nSize);
        }
    }

    return *this;
}

DWORD Data::GetAddress() { return dwAddress; }
SIZE_T Data::GetSize() { return nSize; }
uchar* Data::GetBytes() { return szBytes; }
uchar* Data::GetSrc() { return szSrc; }

//Patch implementation
Patch::Patch() : szName(NULL) {}

Patch::Patch(const Patch& p) {
    vData = p.vData;
    if (p.szName) {
        size_t size = strlen(p.szName);
        if (size > 0) {
            szName = new char[size + 1];
            strcpy_s(szName, size + 1, p.szName);
        }
    } else szName = NULL;
}

Patch::Patch(std::vector<Data> vData) : vData(vData), szName(NULL) {}

Patch::Patch(std::vector<Data> vData, const char* sz) : vData(vData) {
    if (sz) {
        size_t size = strlen(sz);
        if (size > 0) {
            szName = new char[size + 1];
            strcpy_s(szName, size + 1, sz);
        } else szName = NULL;
    } else szName = NULL;
}

Patch::~Patch() {
    vData.clear();

    if (szName) {
        delete[] szName;
        szName = NULL;
    }
}

Patch& Patch::operator=(const Patch& p) {
    vData.clear();
    vData = p.vData;

    if (szName) {
        delete[] szName;
        szName = NULL;
    }
    if (p.szName) {
        size_t size = strlen(p.szName);
        if (size > 0) {
            szName = new char[size + 1];
            strcpy_s(szName, size + 1, p.szName);
        }
    }

    return *this;
}

char* Patch::GetName() { return szName; }
std::vector<Data> Patch::GetData() { return vData; }

//function implementation
bool CheckPatch(Data& d) {
    bool b = true;

    if ( d.GetSrc() != NULL ) {
        void* address = reinterpret_cast<void*>(d.GetAddress());
        SIZE_T size = d.GetSize();
        SIZE_T sizeTemp;
        DWORD oldProtect;
        VirtualProtect(address, size, PAGE_READONLY, &oldProtect);

        uchar* szSrc = d.GetSrc();
        uchar* szBuf = new uchar[size +  1];
        szBuf[size] = 0;
        if (ReadProcessMemory(GetCurrentProcess(), address, szBuf, size, &sizeTemp)) {
            if (memcmp(szSrc, szBuf, size)) {
                LPCTSTR lpsz = "ApplyPatch(): source memory differs from source data at address 0x%x\r\n";
                console.writef(CONSOLEFORECOLOR_WARNING, lpsz, address);
                L.timestamp();
                L.appendf(lpsz, address);

                b = false;
            }
        } else {
            LPCTSTR lpsz = "ApplyPatch(): ReadProcessMemory failed, address: 0x%x, error code: %d\r\n";
            DWORD dwError = GetLastError();
            console.writef(CONSOLEFORECOLOR_WARNING, lpsz, address, dwError);
            L.timestamp();
            L.appendf(lpsz, address, dwError);

            b = false;
        }
        delete[] szBuf;
        szBuf = NULL;

        VirtualProtect(address, size, oldProtect, &oldProtect);
    }

    return b;
}

void ApplyPatch(Data& d) {
    void* address = reinterpret_cast<void*>(d.GetAddress());
    SIZE_T size = d.GetSize();
    SIZE_T sizeTemp;
    DWORD oldProtect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);

    if ( !WriteProcessMemory(GetCurrentProcess(), address, d.GetBytes(), size, &sizeTemp) ) {
        LPCTSTR lpsz = "ApplyPatch(): WriteProcessMemory failed, address: 0x%x, error code: %d\r\n";
        DWORD dwError = GetLastError();
        console.writef(CONSOLEFORECOLOR_WARNING, lpsz, address, dwError);
        L.timestamp();
        L.appendf(lpsz, address, dwError);
    }
    VirtualProtect(address, size, oldProtect, &oldProtect);
    return;
}

void InitUserPatches(std::vector<Patch>* pvPatchList, std::vector<Data>* pvDataList) {
    WIN32_FIND_DATA w32fd = {0};
    LPCSTR szRegexp = "./TobEx_ini/patch/*.patch";
    DWORD nErrorCode = ERROR_SUCCESS;
    HANDLE hFind;

    hFind = FindFirstFile(szRegexp, &w32fd);
    if (hFind == INVALID_HANDLE_VALUE)
        nErrorCode = GetLastError();

    if (nErrorCode == ERROR_FILE_NOT_FOUND ||
        nErrorCode == ERROR_PATH_NOT_FOUND) {
        //do nothing
    } else if (nErrorCode != ERROR_SUCCESS) {
        LPCTSTR lpsz = "InitUserPatches(): FindFirstFile() failed (error code %d)\r\n";
        console.writef(lpsz, nErrorCode);
        L.timestamp();
        L.appendf(lpsz, nErrorCode);
    } else {
        do {
            LPCSTR szDir = "./TobEx_ini/patch/";
            char* szFile = new char[MAX_PATH];
            int i = 0;
            while (i < MAX_PATH) szFile[i++] = 0;
            strcpy_s(szFile, MAX_PATH, szDir);
            strcat_s(szFile, MAX_PATH, w32fd.cFileName);

            OFSTRUCT opf;

            HANDLE hFile = (HANDLE)OpenFile(szFile, &opf, OF_READ);
            nErrorCode = GetLastError();
            if (nErrorCode) {
                LPCTSTR lpsz = "InitUserPatches(): unable to open %s. Error code %d.\r\n";
                console.writef(lpsz, w32fd.cFileName, nErrorCode);
                L.timestamp();
                L.appendf(lpsz, w32fd.cFileName, nErrorCode);
            } else {
                DWORD dwBytesRead;
                if (w32fd.nFileSizeLow > 0) {
                    LPSTR szBuf = new char[w32fd.nFileSizeLow + 1];
                    szBuf[w32fd.nFileSizeLow] = 0;
                    if (ReadFile(hFile, (LPVOID)szBuf, w32fd.nFileSizeLow, &dwBytesRead, NULL)) {
                        IECString sBuf(szBuf);

                        int nLineNumber = -1;
                        IECString sName;
                        bool bInComment = false;
                        bool bInName = false;
                        bool bEnabled = false;
                        bool bHasAddress = false;
                        DWORD dwAddress = 0;
                        DWORD dwPatchSize = 0;
                        uchar* ptrSource = NULL;
                        uchar* ptrTarget = NULL;

                        int nIndex = 0;
                        while (!sBuf.IsEmpty()) {
                            IECString sLine;
                            if (sBuf.Find('\n') == -1) {
                                //last line
                                sLine = sBuf;
                                sBuf.Empty();
                            } else {
                                sLine = sBuf.Left(sBuf.Find('\n'));
                                sBuf = sBuf.Right(sBuf.GetLength() - sBuf.Find('\n') - 1);
                            }

                            sLine.TrimLeft();
                            sLine.TrimRight();
                            nLineNumber++;

                            //remove comments
                            if (sLine.Find("*/") != -1 && bInComment) {
                                sLine = sLine.Right(sLine.GetLength() - sLine.Find("*/") - 2);
                                bInComment = false;
                            }
                            if (bInComment) continue;
                            if (sLine.Left(2).Compare("//") == 0) continue;
                            while (sLine.Find("/*") != -1 &&
                                sLine.Find("*/", sLine.Find("/*") + 2) != -1) {
                                sLine = sLine.Left(sLine.Find("/*")) + sLine.Right(sLine.GetLength() - sLine.Find("*/") - 2);
                            }
                            if (sLine.Find("//") != -1) sLine = sLine.Left(sLine.Find("//"));
                            if (sLine.Find("/*") != -1) {
                                sLine = sLine.Left(sLine.Find("/*"));
                                bInComment = true;
                            }
                    
                            if (sLine.IsEmpty()) continue;
                    
                            //name
                            if (sLine.GetAt(0) == '[') {
                                if (bInName) {
                                    //reset
                                    if (!pvDataList->empty()) {
                                        pvPatchList->push_back( Patch(*pvDataList, (LPCTSTR)sName) );
                                        pvDataList->clear();
                                    }
                                    bInName = false;
                                    bEnabled = false;
                                    bHasAddress = false;
                                }

                                sLine = sLine.Right(sLine.GetLength() - 1);
                                if (sLine.Find(']') == -1) {
                                    LPCTSTR lpsz = "%s,%d: missing ']' in patch name\r\n";
                                    console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                    L.timestamp();
                                    L.appendf(lpsz, w32fd.cFileName, nLineNumber);

                                    sName = sLine;
                                } else {
                                    sName = sLine.Left(sLine.Find(']'));
                                }
                                sName.TrimLeft();
                                sName.TrimRight();
                                if (!sName.IsEmpty()) {
                                    bInName = true;
                                } else {
                                    LPCTSTR lpsz = "%s,%d: empty name in [], will ignore\r\n";
                                    console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                    L.timestamp();
                                    L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                }
                                continue;
                            }

                            //field
                            if (sLine.Find('=', 1) != -1) {
                                if (!bInName) {
                                    LPCTSTR lpsz = "%s,%d: found field (expected name)\r\n";
                                    console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                    L.timestamp();
                                    L.appendf(lpsz, w32fd.cFileName, nLineNumber);

                                    continue;
                                }

                                int nIndexEquals = sLine.Find('=');
                                IECString sLeft = sLine.Left(nIndexEquals);
                                sLeft.TrimRight();

                                IECString sRight = sLine.Right(sLine.GetLength() - nIndexEquals - 1);
                                sRight.TrimLeft();

                                if (sLeft.CompareNoCase("Enabled") == 0) {
                                    //bEnabled = to_bool(atoi(sRight.GetBuffer(0)));
                                    bEnabled = to_bool(atoi((LPCTSTR)sRight));
                                    continue;
                                }

                                if (sLeft.CompareNoCase("Address") == 0) {
                                    if (bHasAddress) {
                                        LPCTSTR lpsz = "%s,%d: address already specified, will overwrite\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                    }

                                    if (sRight.GetAt(0) == 'v' ||
                                        sRight.GetAt(0) == 'V') {
                                        //virtual address
                                        dwAddress = 0;
                                    }
                                    else if (sRight.GetAt(0) == 'b' ||
                                        sRight.GetAt(0) == 'B') {
                                        //base address
                                        dwAddress = 0x400000;
                                    } else {
                                        LPCTSTR lpsz = "%s,%d: no address specifier\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                        continue;
                                    }
                                    sRight = sRight.Right(sRight.GetLength() - 1);

                                    DWORD dwOffset;
                                    //sscanf_s(sRight.GetBuffer(0), "%x", &dwOffset);
                                    sscanf_s((LPCTSTR)sRight, "%x", &dwOffset);
                                    dwAddress += dwOffset;
                                    bHasAddress = true;

                                    continue;
                                } //address

                                if (sLeft.CompareNoCase("Source") == 0) {
                                    if (!bHasAddress) {
                                        LPCTSTR lpsz = "%s,%d: found source (expected address)\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                        continue;
                                    }
                                    if (!bEnabled) continue;
                                    if (ptrSource) {
                                        LPCTSTR lpsz = "%s,%d: source already specified, will overwrite\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);

                                        delete ptrSource;
                                        ptrSource = NULL;
                                    }

                                    while (sRight.Find(',') != -1) sRight.Delete(sRight.Find(','));

                                    dwPatchSize = sRight.GetLength() / 2;
                                    if (dwPatchSize > 0) {
                                        ptrSource = new uchar[dwPatchSize + 1];
                                        ptrSource[dwPatchSize] = 0;
                                        unsigned int i = 0;
                                        while (i < dwPatchSize) {
                                            int n;
                                            //sscanf_s(sRight.Left(2).GetBuffer(0), "%x", &n);
                                            sscanf_s((LPCTSTR)sRight.Left(2), "%x", &n);
                                            ptrSource[i] = n;
                                            sRight = sRight.Right(sRight.GetLength() - 2);
                                            i++;
                                        }
                                    } else {
                                        LPCTSTR lpsz = "%s,%d: source patch size is zero\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                    }
                                    continue;
                                } //source

                                if (sLeft.CompareNoCase("Target") == 0) {
                                    if (!bHasAddress) {
                                        LPCTSTR lpsz = "%s,%d: found target (expected address)\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                        continue;
                                    }
                                    if (!bEnabled) {
                                        bHasAddress = false;
                                        if (ptrSource) {
                                            delete[] ptrSource;
                                            ptrSource = NULL;
                                        }
                                        if (ptrTarget) {
                                            delete[] ptrTarget;
                                            ptrTarget = NULL;
                                        }
                                        continue;
                                    }
                                    /*if (ptrSource == NULL) {
                                        LPCTSTR lpsz = "%s,%d: found target (expected source)\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                        continue;
                                    }*/
                                    if (ptrSource &&
                                        dwPatchSize != sRight.GetLength() / 2) {
                                        LPCTSTR lpsz = "%s,%d: target and source patch sizes do not match, will skip\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);

                                        bHasAddress = false;
                                        dwPatchSize = 0;
                                        delete[] ptrSource;
                                        ptrSource = NULL;
                                        continue;
                                    }
                                    if (ptrTarget) {
                                        LPCTSTR lpsz = "%s,%d: target already specified, will overwrite\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);

                                        delete[] ptrTarget;
                                        ptrTarget = NULL;
                                    }

                                    while (sRight.Find(',') != -1) sRight.Delete(sRight.Find(','));

                                    dwPatchSize = sRight.GetLength() / 2;
                                    if (dwPatchSize > 0) {
                                        ptrTarget = new uchar[dwPatchSize + 1];
                                        ptrTarget[dwPatchSize] = 0;
                                        unsigned int i = 0;
                                        while (i < dwPatchSize) {
                                            int n;
                                            //sscanf_s(sRight.Left(2).GetBuffer(0), "%x", &n);
                                            sscanf_s((LPCTSTR)sRight.Left(2), "%x", &n);
                                            ptrTarget[i] = n;
                                            sRight = sRight.Right(sRight.GetLength() - 2);
                                            i++;
                                        }

                                        pvDataList->push_back( Data(dwAddress, dwPatchSize, ptrTarget, ptrSource) );

                                        bHasAddress = false;
                                        dwPatchSize = 0;
                                        if (ptrSource) {
                                            delete[] ptrSource;
                                            ptrSource = NULL;
                                        }
                                        if (ptrTarget) {
                                            delete[] ptrTarget;
                                            ptrTarget = NULL;
                                        }

                                    } else {
                                        LPCTSTR lpsz = "%s,%d: target size is zero\r\n";
                                        console.writef(lpsz, w32fd.cFileName, nLineNumber);
                                        L.timestamp();
                                        L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                                    }
                                    continue;
                                } //target
                            } //field

                            LPCTSTR lpsz = "%s,%d: unknown instruction\r\n";
                            console.writef(lpsz, w32fd.cFileName, nLineNumber);
                            L.timestamp();
                            L.appendf(lpsz, w32fd.cFileName, nLineNumber);
                        } //while

                        //clean up
                        if (!pvDataList->empty()) {
                            pvPatchList->push_back( Patch(*pvDataList, (LPCTSTR)sName) );
                            pvDataList->clear();
                        }

                        if (bInComment) {
                            LPCTSTR lpsz = "%s: reached EOF (expected end of comment)\r\n";
                            console.writef(lpsz, w32fd.cFileName);
                            L.timestamp();
                            L.appendf(lpsz, w32fd.cFileName);
                        }
                        if (bHasAddress) {
                            LPCTSTR lpsz = "%s: reached EOF (expected source/target)\r\n";
                            console.writef(lpsz, w32fd.cFileName);
                            L.timestamp();
                            L.appendf(lpsz, w32fd.cFileName);
                        }
                        if (ptrSource) {
                            LPCTSTR lpsz = "%s: reached EOF (expected target)\r\n";
                            console.writef(lpsz, w32fd.cFileName);
                            L.timestamp();
                            L.appendf(lpsz, w32fd.cFileName);

                            delete[] ptrSource;
                            ptrSource = NULL;
                        }
                        if (ptrTarget) {
                            LPCTSTR lpsz = "%s: reached EOF (before target cleared)\r\n";
                            console.writef(lpsz, w32fd.cFileName);
                            L.timestamp();
                            L.appendf(lpsz, w32fd.cFileName);

                            delete[] ptrTarget;
                            ptrTarget = NULL;
                        }

                    } else {
                        LPCTSTR lpsz = "InitUserPatches(): could not read from %s (error code %d)\r\n";
                        console.writef(lpsz, w32fd.cFileName, GetLastError());
                        L.timestamp();
                        L.appendf(lpsz, w32fd.cFileName, GetLastError());
                    }

                    delete[] szBuf;
                    szBuf = NULL;
                } //dwFileSize > 0
                CloseHandle(hFile);
            }

            delete[] szFile;
        } while (FindNextFile(hFind, &w32fd));

        if (!FindClose(hFind)) {
            LPCTSTR lpsz = "InitUserPatches(): FindClose() failed (error code %d)\r\n";
            console.writef(lpsz, nErrorCode);
            L.timestamp();
            L.appendf(lpsz, nErrorCode);
        }
    }

    return;
}

void InitPatches() {
    std::vector<Patch> vPatchList;
    std::vector<Patch>::iterator vPatchItr;
    std::vector<Data> vDataList;

    // zeroing new allocated objects
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        JumpInject(0xA50608, Malloc_asm); // malloc() + memset()

        //RelativeInject(0x435D64, CAlloc_asm); // 1) CInfGame
        //RelativeInject(0x680AE4, CAlloc_asm); // 2) CArea

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionAddKitFix) {
        //AddKit()
        //edx: i, ecx: pCCreatureObject
        //mov eax,edx
        //shr eax,10
        //and edx,0FFFF
        //mov word ptr ds:[ecx+632],ax
        //mov word ptr ds:[ecx+634],dx
        //nop
        uchar bytes[] = {0x8B, 0xC2,
                        0xC1, 0xE8, 0x10,
                        0x81, 0xE2, 0xFF, 0xFF, 0x00, 0x00,
                        0x66, 0x89, 0x81, 0x32, 0x06, 0x00, 0x00,
                        0x66, 0x89, 0x91, 0x34, 0x06, 0x00, 0x00,
                        0x90};
        vDataList.push_back( Data(0x8E460C, 26, bytes) );

        //AddSuperKit()
        //ecx: i, eax: pCCreatureObject
        //mov edx,ecx
        //shr edx,10
        //and ecx,0FFFF
        //mov word ptr ds:[eax+632],dx
        //mov word ptr ds:[eax+634],cx
        //nop
        uchar bytes2[] = {0x8B, 0xD1,
                        0xC1, 0xEA, 0x10,
                        0x81, 0xE1, 0xFF, 0xFF, 0x00, 0x00,
                        0x66, 0x89, 0x90, 0x32, 0x06, 0x00, 0x00,
                        0x66, 0x89, 0x88, 0x34, 0x06, 0x00, 0x00,
                        0x90};
        vDataList.push_back( Data(0x8E49F8, 26, bytes2) );

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionAttackOnceFix ||
        pGameOptionsEx->bActionAttacksGenuine) {
        //mov edx,dword ptr ss:[ebp-BC]
        //push edx
        //mov ecx,dword ptr ss:[ebp-614]
        //push ecx
        //call ...
        uchar bytes[] = {0x8B, 0x95, 0x44, 0xFF, 0xFF, 0xFF,
                        0x52,
                        0x8B, 0x8D, 0xEC, 0xF9, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x909528, 15, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_AttackOnce_DoHalfAttack;
        DWORD address = (DWORD)ptr - 5  - 0x909536;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x909537, 4, bytes2) );

        if (pGameOptionsEx->bActionAttacksGenuine) {
            //test eax,eax
            //jnz 90979C - do animation
            //jmp 9099BF - skip animation
            //nop
            uchar bytes3[] = {0x85, 0xC0,
                            0x0F, 0x85, 0x59, 0x02, 0x00, 0x00,
                            0xE9, 0x77, 0x04, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90};
            vDataList.push_back( Data(0x90953B, 18, bytes3) );

            //LIGHT_GREY pixel: disable melee attack animation if no statistical attack
            //mov al,byte ptr ds:[ecx+36D4]
            //test al,al
            //je 00906578
            //mov edx,dword ptr ss:[ebp-B8]
            //mov al,byte ptr ds:[edx]
            //cmp al,2
            uchar bytes5[] = {0x8A, 0x81, 0xD4, 0x36, 0x00, 0x00,
                            0x84, 0xC0,
                            0x0F, 0x84, 0xFC, 0x01, 0x00, 0x00,
                            0x8B, 0x95, 0x48, 0xFF, 0xFF, 0xFF,
                            0x8A, 0x02,
                            0x3C, 0x02};
            vDataList.push_back( Data(0x90636E, 24, bytes5) );

            //RED pixel: disable melee attack animation if no statistical attack
            //mov dl,byte ptr ss:[ebp-AC]
            //cmp dl,0
            //je 9073E1
            //mov eax,dword ptr ss:[ebp-614]
            //mov cl,byte ptr ds:[eax+36D4]
            //test cl,cl
            //je 9073E1
            //push 0E
            //call A50608
            //add esp,4
            //mov dword ptr ss:[ebp-270],eax
            //cmp dword ptr ss:[ebp-270],0
            //je short 9073A5
            //mov ecx,dword ptr ss:[ebp-614]
            //mov edx,dword ptr ds:[ecx+30]
            //mov dword ptr ss:[ebp-4BC],edx
            //mov dword ptr ss:[ebp-4B8],edx
            //mov dl,0
            uchar bytes6[] = {0x8A, 0x95, 0x54, 0xFF, 0xFF, 0xFF,
                            0x80, 0xFA, 0x00,
                            0x0F, 0x84, 0xD9, 0x00, 0x00, 0x00,
                            0x8B, 0x85, 0xEC, 0xF9, 0xFF, 0xFF,
                            0x8A, 0x88, 0xD4, 0x36, 0x00, 0x00,
                            0x84, 0xC9,
                            0x0F, 0x84, 0xC5, 0x00, 0x00, 0x00,
                            0x6A, 0x0E,
                            0xE8, 0xE5, 0x92, 0x14, 0x00,
                            0x83, 0xC4, 0x04,
                            0x89, 0x85, 0x90, 0xFD, 0xFF, 0xFF,
                            0x83, 0xBD, 0x90, 0xFD, 0xFF, 0xFF, 0x00,
                            0x74, 0x70,
                            0x8B, 0x8D, 0xEC, 0xF9, 0xFF, 0xFF,
                            0x8B, 0x51, 0x30,
                            0x89, 0x95, 0x44, 0xFB, 0xFF, 0xFF,
                            0x89, 0x95, 0x48, 0xFB, 0xFF, 0xFF,
                            0xB2, 0x00};
            vDataList.push_back( Data(0x9072F9, 83, bytes6) );
        } else {
            //jmp 90979C - do animation
            //nop
            uchar bytes3[] = {0xE9, 0x5C, 0x02, 0x00, 0x00,
                            0x90};
            vDataList.push_back( Data(0x90953B, 6, bytes3) );
        }

        if (pGameOptionsEx->bActionAttackOnceFix) {
            //disable use of CCreatureObject.36dah in TryHit() - unused anyway
            //normally would halve the to hit roll
            //je -> jmp
            uchar bytes4[] = {0xEB};
            vDataList.push_back( Data(0x90BC3D, 1, bytes4) );
        }

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionChangeAnimFix) {
        //mov ecx,dword ptr ss:[ebp-98]
        //push ecx
        //mov ecx,dword ptr ss:[ebp-178]
        //push ecx
        //call
        uchar bytes[] = {0x8B, 0x8D, 0x68, 0xFF, 0xFF, 0xFF,
                        0x51,
                        0x8B, 0x8D, 0x88, 0xFE, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x951C10, 15, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_ActionChangeAnimation_CopyState;
        DWORD address = (DWORD)ptr - 5  - 0x951C1E;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x951C1F, 4, bytes2) );

        //jmp 951C34
        //nop
        uchar bytes3[] = {0xEB, 0x0F,
                        0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x951C23, 5, bytes3) );

        COMMIT_vDataList
    }
    
    if (pGameOptionsEx->bActionEquipRangedFix) {
        //mov al,byte ptr ds:[edx]
        //cmp al,2 (TYPE_RANGED)
        //jnz short 0094FAAE
        //mov ecx,dword ptr ss:[ebp-8]
        //call CItem::GetFlags()
        //and eax,2 (ITEMFLAG_TWOHANDED)
        //test eax,eax
        //je short 0094FA6A
        //mov ecx,dword ptr ss:[ebp-C]
        //nop
        //mov dl,byte ptr ds:[ecx+10]
        //cmp dl,3 (PROJTYPE_BULLET)
        uchar bytes[] = {0x8A, 0x02,
                        0x3C, 0x02,
                        0x75, 0x65,
                        0x8B, 0x4D, 0xF8,
                        0xE8, 0x40, 0xBB, 0xC5, 0xFF,
                        0x83, 0xE0, 0x02,
                        0x85, 0xC0,
                        0x74, 0x12,
                        0x8B, 0x4D, 0xF4,
                        0x90,
                        0x8A, 0x51, 0x10,
                        0x80, 0xFA, 0x03};
        vDataList.push_back( Data(0x94FA43, 31, bytes) );

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionExpandedActions) {
        //mov eax,dword ptr ss:[ebp+8]
        //push eax
        //mov ecx,dword ptr ss:[ebp-1BC]
        //push ecx
        //call ...
        uchar bytes[] = {0x8B, 0x45, 0x08,
                        0x50,
                        0x8B, 0x8D, 0x44, 0xFE, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x4E885C, 12, bytes) );

        //CALL address
        void* ptr = (void*)CDlgResponse_QueueActions;
        DWORD address = (DWORD)ptr - 5  - 0x4E8867;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x4E8868, 4, bytes2) );

        //jmp 4E8D57
        //nop
        uchar bytes3[] = {0xE9, 0xE6, 0x04, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x4E886C, 7, bytes3) );

        /////////////////////////////////////////////
        // new ACTION_CLEAR_BLOCK_VARIABLES Action fixes
        // mov   ecx, 166h      ; skip modal change
        uchar bytes4[] = {0xB9, 0x66, 0x01, 0x00, 0x00, 0x90, 0x90};
        vDataList.push_back( Data(0x8F9A15, sizeof(bytes4), bytes4) );

        // cmp   ax, 166h
        // jz    skip           ; skip animation sequence change
        // movsx ecx, ax
        uchar bytes5[] = {0x66, 0x3D, 0x66, 0x01, 0x0F, 0x84, 0xBA, 0x03, 0x00, 0x00, 0x0F, 0xBF, 0xC8, 0x90};
        vDataList.push_back( Data(0x8FA293, sizeof(bytes5), bytes5) );
        /////////////////////////////////////////////

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionJoinPartyFix) {
        //push ecx
        //call
        uchar bytes[] = {0x51,
                        0xE8};
        vDataList.push_back( Data(0x9331C0, 2, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_JoinParty_UpdateClassAbilities;
        DWORD address = (DWORD)ptr - 5  - 0x9331C1;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9331C2, 4, bytes2) );

        uchar bytes3[] = {0xEB, 0x11,
                        0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x9331C6, 7, bytes3) );

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bActionTakePartyItemFix) {
        //change for loop starting point
        //19 -> 15
        uchar bytes[] = {0x0F};
        vDataList.push_back( Data(0x4A68EB, 1, bytes) );

        //change starting array offset to misc3
        uchar bytes2[] = {0xA2};
        vDataList.push_back( Data(0x4A6910, 1, bytes2) );
        vDataList.push_back( Data(0x4A6930, 1, bytes2) );

        COMMIT_vDataList
    }

    if (pGameOptionsEx->bArenasEnable) {
        uchar bytes[] = {0x75};
        vDataList.push_back( Data(0x7958D7, 1, bytes) );
        vDataList.push_back( Data(0x796C19, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bDebugCriticalMsgBoxFix) {
        //NOP, CALL
        uchar bytes[] = {0x90, 0xE8};
        vDataList.push_back( Data(0x9A5F63, 2, bytes) );
        vDataList.push_back( Data(0x9A5F9E, 2, bytes) );
        vDataList.push_back( Data(0x9A5FE5, 2, bytes) );

        //CALL address
        void* ptr = (void*)CBaldurChitin_MessageBox;
        DWORD address = (DWORD)ptr - 5  - 0x9A5F64;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9A5F65, 4, bytes2) );

        address = (DWORD)ptr - 5  - 0x9A5F9F;
        bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9A5FA0, 4, bytes2) );

        address = (DWORD)ptr - 5  - 0x9A5FE6;
        bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9A5FE7, 4, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bDebugLogFailures) {
        //in CUICheckButtonRecMageSpell::SetSpell()
        uchar bytes[] = {0x8B};
        vDataList.push_back( Data(0x6FA657, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bDebugLogMissingRes) {
        //in KeyTable::FindKey()
        uchar bytes[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        //ignore missed .BAH restype
        uchar bytes2[] = {0x8B, 0x45, 0x0C, 0x3D, 0x4C, 0x04, 0x00, 0x00, 0x0F, 0x84, 0xBC, 0x01, 0x00, 0x00};

        vDataList.push_back( Data(0x99B11F, sizeof(bytes2), bytes2) );
        vDataList.push_back( Data(0x99B12D, 6, bytes) );

        COMMIT_vDataList;
    }
    
    //Enable logging of system, AREA-TRANSITION and AREA-INVENTORY messages
    if (pGameOptionsEx->bDebugLogMore) {
        uchar bytes[] = {0x01};
        vDataList.push_back( Data(0xAB3BDC, 1, bytes) );
        vDataList.push_back( Data(0xAB3BE0, 1, bytes) );
        vDataList.push_back( Data(0xAB3BE4, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bDebugRestoreCombatInfoText) {

        //1. In TryHit()
        //LEA EDX, int PTR SS:[EBP-20]
        //PUSH EDX
        uchar bytes[] = {
            0x8D, 0x55, 0xE0, \
            0x52
        };
        vDataList.push_back( Data(0x90B928, 4, bytes) );

        //MOV EDX, int PTR SS:[EBP-294]
        //PUSH EDX
        uchar bytes2[] = {
            0x8B, 0x95, 0x6C, 0xFD, 0xFF, 0xFF, \
            0x52
        };
        vDataList.push_back( Data(0x90B92C, 7, bytes2) );

        //CALL
        uchar bytes3[] = {0xE8};
        vDataList.push_back( Data(0x90B933, 1, bytes3) );

        //CALL address
        void* ptr = (void*)CCreatureObject_PrintExtraCombatInfoText;
        DWORD address = (DWORD)ptr - 5  - 0x90B933;
        uchar* bytes4 = (uchar*)&address;
        vDataList.push_back( Data(0x90B934, 4, bytes4) );

        uchar bytes5[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x90B938, 8, bytes5) );

        //2. In DoDamage
        /*lea ecx,dword ptr ss:[ebp-24]
        push ecx
        mov ecx,dword ptr ss:[ebp-198]
        push ecx
        call*/
        uchar bytes6[] = {
            0x8D, 0x4D, 0xDC, \
            0x51, \
            0x8B, 0x8D, 0x68, 0xFE, 0xFF, 0xFF, \
            0x51, \
            0xE8
        };
        vDataList.push_back( Data(0x90DE6A, 12, bytes6) );

        //CALL address
        ptr = (void*)CCreatureObject_PrintExtraCombatInfoText;
        address = (DWORD)ptr - 5  - 0x90DE75;
        uchar* bytes7 = (uchar*)&address;
        vDataList.push_back( Data(0x90DE76, 4, bytes7) );

        uchar bytes8[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x90DE7A, 10, bytes8) );

        COMMIT_vDataList;
    }

    //CEffect::Unmarshal() copies the unused fields over
    //important say for Use EFF File saving child effects properly
    {
        uchar bytes[] = {0x1A};
        vDataList.push_back( Data(0x4F3E35, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffAwakenFix) {
        uchar bytes[] = {0x27};
        vDataList.push_back( Data(0x5035F4, 1, bytes) );
        vDataList.push_back( Data(0x50362C, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffCastingLevelModFix) {
        //spelltype PRIEST
        uchar bytes[26] = {0x8B, 0x4D, 0xE8, 0x0F, 0xBE, 0x91, 0xD0, 0x00, 0x00, 0x00, 0x03, 0xC2, 0x83, 0xF8, 0x01, 0x7E, 0x37, 0x89, 0x45, 0xB8, 0xEB, 0x39, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x951511, 26, bytes) );
        
        //spelltype WIZARD
        uchar bytes2[25] = {0x8B, 0x4D, 0xE0, 0x0F, 0xBE, 0x91, 0xCE, 0x00, 0x00, 0x00, 0x03, 0xC2, 0x83, 0xF8, 0x01, 0x7E, 0x37, 0x89, 0x45, 0xB4, 0xEB, 0x39, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x9515B4, 25, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffDamageBypassMirrorImageConfig) {
        /*mov ecx,dword ptr ss:[ebp-38]
        mov ecx,dword ptr ds:[ecx+3C]
        and ecx,1000000 CEFFECT_BYPASS_MIRRORIMAGES
        test ecx,ecx
        jnz BGMain.0050803E
        movsx ecx,word ptr ds:[eax+1404]
        push ecx
        add edx,1
        push edx
        nop*/
        uchar bytes[] = {0x8B, 0x4D, 0xC8,
                        0x8B, 0x49, 0x3C,
                        0x90, 0x90, 0x90,
                        0x81, 0xE1, 0x00, 0x00, 0x00, 0x01,
                        0x85, 0xC9,
                        0x0F, 0x85, 0x44, 0x01, 0x00, 0x00,
                        0x0F, 0xBF, 0x88, 0x04, 0x14, 0x00, 0x00,
                        0x51,
                        0x83, 0xC2, 0x01,
                        0x52,
                        0x90
        };
        vDataList.push_back( Data(0x507EE3, 36, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffContainerUseEffFileFix) {
        //JE-> JMP (skip deconstructor)
        uchar bytes[] = {0xEB};
        vDataList.push_back( Data(0x4DB20C, 1, bytes) ); //container
        vDataList.push_back( Data(0x4EB3B0, 1, bytes) ); //door

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffCureDrunkFix) {
        uchar bytes[] = {0x5E};
        vDataList.push_back( Data(0x52EB0E, 1, bytes) );
        vDataList.push_back( Data(0x52EB49, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffStoneskinDisableColour) {
        uchar bytes[] = {0x00};
        vDataList.push_back( Data(0x5399D4, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffDropInvDisintegrate) {
        uchar bytes[] = {0x00};
        vDataList.push_back( Data(0x50B30A, 1, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffDropInvFrozenDeath) {
        uchar bytes[] = {0x00};
        vDataList.push_back( Data(0x50AEE1, 1, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffDropInvStoneDeath) {
        uchar bytes[] = {0x00};
        vDataList.push_back( Data(0x50AB5F, 1, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffForbidItemTypeFix) {
        uchar bytes[] = {0x90, 0x90};
        vDataList.push_back( Data(0x530756, 2, bytes) );

        COMMIT_vDataList;
    }

    //Fix for setting CDerivedStats for permanent until death Improved Invisibility
    if (pGameOptionsEx->bEffInvisibleFix) {
        uchar bytes2[] = {0x0A, 0x0B};
        vDataList.push_back( Data(0x5126F4, 2, bytes2) );
        vDataList.push_back( Data(0x512703, 2, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffMirrorImageUseCastLevel) {
        //fix brackets in formula when caster level > effect level
        uchar bytes[] = {0x8B, 0x55, 0xD4, 0x8B, 0x4A, 0x48, 0x89, 0x48, 0x48, 0x83, 0xC0, 0x70, 0x83, 0xC2, 0x70, 0x8B, 0x4A, 0x54, 0x89, 0x48, 0x54, 0x8B, 0x4A, 0x5C, 0x89, 0x48, 0x5C, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x521184, 30, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEffPolymorphFix) {
        //je less -> jmp more, skip early return
        uchar bytes[] = {0xEB};
        vDataList.push_back( Data(0x5288CB, 1, bytes) );

        //mov eax,dword ptr ss:[ebp-34]
        //push eax
        //mov ecx,dword ptr ss:[ebp+8]
        //push ecx
        //mov edx,dword ptr ss:[ebp-1BC]
        //push edx
        //call
        uchar bytes2[] = {0x8B, 0x45, 0xCC,
                        0x50,
                        0x8B, 0x4D, 0x08,
                        0x51, 0x8B, 0x95, 0x44, 0xFE, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x5296CB, 16, bytes2) );

        //CALL address
        void* ptr = (void*)CEffectPolymorph_ApplyEffect_ReinitAnimation;
        DWORD address = (DWORD)ptr - 5  - 0x5296DA;
        uchar* bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x5296DB, 4, bytes3) );

        //jmp 5297D7
        //nop
        uchar bytes4[] = {0xE9, 0xF3, 0x00, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x5296DF, 7, bytes4) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEnginePriestKnownSpellsExtend) {
        uchar bytes[] = {0x62};
        vDataList.push_back( Data(0x63358D, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineAllowEquipArmorCombat) {
        //jle -> jmp
        uchar bytes[] = {0xEB};
        vDataList.push_back( Data(0x69BEDF, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineAllowZeroStartXP) {
        uchar bytes[] = {0x8C};
        vDataList.push_back( Data(0x729C45, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineAllowDualClassAll) {
        //nop a jnz
        uchar bytes[] = {0x90, 0x90};
        vDataList.push_back( Data(0x6ED934, 2, bytes) );

        COMMIT_vDataList;
    }


    if (pGameOptionsEx->bEngineExpandedStats) {
        //je short BGMain.008A7311
        //add ecx, 0B0A
        //jmp short BGMain.008A7317
        //add ecx, 13C2
        //mov dword ptr ss:[ebp-DC], ecx
        //push 0C9
        //mov ecx, dword ptr ss:[ebp-DC]
        //call CDerivedStats::GetStat(index)
        //cmp eax, 0
        //nop
        uchar bytes[] = {0x74, 0x08,
                        0x81, 0xC1, 0x0A, 0x0B, 0x00, 0x00,
                        0xEB, 0x06,
                        0x81, 0xC1, 0xC2, 0x13, 0x00, 0x00,
                        0x89, 0x8D, 0x24, 0xFF, 0xFF, 0xFF,
                        0x68, 0xC9, 0x00, 0x00, 0x00,
                        0x8B, 0x8D, 0x24, 0xFF, 0xFF, 0xFF,
                        0xE8, 0x35, 0xBE, 0xBC, 0xFF,
                        0x83, 0xF8, 0x00,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x8A7307, 52, bytes) );

        //For percentage damage type bonus
        //mov edx,dword ptr ss:[ebp+24]
        //push edx
        //lea ecx,dword ptr ss:[ebp-44]
        //push ecx
        //mov edx,dword ptr ss:[ebp-50]
        //push edx
        //mov ecx,dword ptr ss:[ebp-198]
        //push ecx
        //call
        uchar bytes2[] = {0x8B, 0x55, 0x24,
                        0x52,
                        0x8D, 0x4D, 0xBC,
                        0x51,
                        0x8B, 0x55, 0xB0,
                        0x52,
                        0x8B, 0x8D, 0x68, 0xFE, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x90CC9A, 20, bytes2) );

        //CALL address
        void* ptr = (void*)CCreatureObject_ApplyDamage_CalculateDamageBonus;
        DWORD address = (DWORD)ptr - 5  - 0x90CCAD;
        uchar* bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x90CCAE, 4, bytes3) );

        //jmp 90CD25
        //nop
        uchar bytes4[] = {0xEB, 0x71,
                        0x90};
        vDataList.push_back( Data(0x90CCB2, 3, bytes4) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bEngineAssBHPenaltyKit) {
        //1. CScreenRecord::LevelUpPanelOnLoad()
        uchar bytes[] = {0x25, 0x00, 0x00, 0x08, 0x00,
                        0x85, 0xC0,
                        0x90, 0x90, 0x90, 0x90,
                        0x74, 0x25,
                        0x8A, 0x8D, 0xC0, 0xF6, 0xFF, 0xFF,
                        0x8A, 0x95, 0xBC, 0xF6, 0xFF, 0xFF,
                        0x81, 0xE1, 0xFF, 0x00, 0x00, 0x00,
                        0x2A, 0xCA};
        vDataList.push_back( Data(0x6DF0B4, 33, bytes) );

        uchar bytes2[] = {0x25, 0x00, 0x00, 0x04, 0x00,
                        0x85, 0xC0,
                        0x90, 0x90, 0x90, 0x90,
                        0x74, 0x25,
                        0x8A, 0x8D, 0xC0, 0xF6, 0xFF, 0xFF,
                        0x8A, 0x95, 0xBC, 0xF6, 0xFF, 0xFF,
                        0x81, 0xE1, 0xFF, 0x00, 0x00, 0x00,
                        0x2A, 0xCA};
        vDataList.push_back( Data(0x6DF0EE, 33, bytes2) );

        //2. CScreenCharGen::SkillsBG1PanelOnLoad()
        uchar bytes3[] = {0x25, 0x00, 0x00, 0x08, 0x00,
                        0x85, 0xC0,
                        0x90, 0x90, 0x90,
                        0x74, 0x26,
                        0x6A, 0x04};
        vDataList.push_back( Data(0x71CDD0, 14, bytes3) );

        uchar bytes4[] = {0x25, 0x00, 0x00, 0x04, 0x00,
                        0x85, 0xC0,
                        0x90, 0x90, 0x90,
                        0x74, 0x26,
                        0x6A, 0x04};
        vDataList.push_back( Data(0x71CE0A, 14, bytes4) );

        //3. CEffectLevelDrain::ApplyEffect()
        uchar bytes5[] = {0x25, 0x00, 0x00, 0x08, 0x00,
                        0x85, 0xC0,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                        0x74, 0x3F,
                        0x8B, 0x95, 0x4C, 0xEE, 0xFF, 0xFF,
                        0x2B, 0x95, 0x48, 0xEE, 0xFF, 0xFF,
                        0x6B, 0xD2, 0x14,
                        0x39, 0x95, 0x6C, 0xEE, 0xFF, 0xFF,
                        0x7D, 0x0E,
                        0x8B, 0x85, 0x6C, 0xEE, 0xFF, 0xFF,
                        0x89, 0x85, 0x2C, 0xEE, 0xFF, 0xFF,
                        0xEB, 0x1A,
                        0x8B, 0x8D, 0x4C, 0xEE, 0xFF, 0xFF,
                        0x2B, 0x8D, 0x48, 0xEE, 0xFF, 0xFF,
                        0x6B, 0xC9, 0x14,
                        0x89, 0x8D, 0x2C, 0xEE, 0xFF, 0xFF,
                        0xE9, 0x9F, 0x00, 0x00, 0x00,
                        0x8B, 0x4D, 0x08,
                        0xE8, 0x3F, 0x7E, 0x3A, 0x00,
                        0x25, 0x00, 0x00, 0x04, 0x00,
                        0x85, 0xC0,
                        0x74, 0x48};
        vDataList.push_back( Data(0x5387D4, 97, bytes5) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bEngineClericRangerHLAFix) {
        //CLASS_RANGER -> CLASS_CLERIC
        uchar bytes[] = {0x66};
        vDataList.push_back( Data(0x62D0DB, 1, bytes) );
        vDataList.push_back( Data(0x62D179, 1, bytes) );
        vDataList.push_back( Data(0x62D18F, 1, bytes) );

        //CLASS_CLERIC -> CLASS_RANGER
        uchar bytes2[] = {0x6F};
        vDataList.push_back( Data(0x62D12D, 1, bytes2) );
        vDataList.push_back( Data(0x62D1A5, 1, bytes2) );
        vDataList.push_back( Data(0x62D1BA, 1, bytes2) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->nEngineCustomSoAStartXP != -1) {
        uchar* bytes = (uchar*)&pGameOptionsEx->nEngineCustomSoAStartXP;
        vDataList.push_back( Data(0xAB7258, 4, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->nEngineCustomToBStartXP != -1) {
        uchar* bytes = (uchar*)&pGameOptionsEx->nEngineCustomToBStartXP;
        vDataList.push_back( Data(0xAB7264, 4, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineDisableInvPauseSP) {
        uchar bytes[] = {0xEB};
        //void CScreenInventory::Init()
        vDataList.push_back( Data(0x7402A8, 1, bytes) );

        //void CScreenInventory::DeInit()
        vDataList.push_back( Data(0x7403D0, 1, bytes) );

        uchar bytes2[] = {0x90, 0x90};
        vDataList.push_back( Data(0x74912B, 2, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineDisableXPBoost) {
        //disables experience boost
        uchar bytes[] = {0x90, 0x90, 0x90,
                        0x90, 0x90,
                        0x90, 0x90, 0x90, 0x90, 0x90,
                        0x90, 0x90};
        vDataList.push_back( Data(0x6A97D3, 12, bytes) );

        //corrects XP reporting
        uchar bytes2[] = {0x90, 0x90, 0x90,
                        0x90, 0x90,
                        0x90, 0x90, 0x90, 0x90, 0x90,
                        0x90, 0x90};
        vDataList.push_back( Data(0x6A995B, 12, bytes2) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bEngineCharmSilenceRemoval) {
        uchar bytes[] = {0xEB};
        vDataList.push_back( Data(0x8A11D4, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineAutoPauseAllSP) {
        uchar bytes[] = {0xEB};
        vDataList.push_back( Data(0x8D1D35, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineExpandedStats) {
        //WEIGHTALLOWANCEMOD
        //1. CScreenRecord::RefreshMainPanel()

        //push edx
        //call
        uchar bytes[] = {0x52,
                        0xE8};
        vDataList.push_back( Data(0x6E8160, 2, bytes) );

        //CALL address
        void* ptr = (void*)CRuleTables_GetWeightAllowance;
        DWORD address = (DWORD)ptr - 5  - 0x6E8161;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x6E8162, 4, bytes2) );

        //jmp 6E830B
        //nop
        uchar bytes3[] = {0xE9, 0xA0, 0x01, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x6E8166, 10, bytes3) );

        //push eax
        //call
        uchar bytes4[] = {0x50,
                        0xE8};
        vDataList.push_back( Data(0x7428F0, 2, bytes4) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x7428F1;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x7428F2, 4, bytes5) );

        //mov dword ptr ss:[ebp-34],eax
        //jmp 742A6F
        //nop
        uchar bytes6[] = {0x89, 0x45, 0xCC,
                        0xE9, 0x71, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x7428F6, 10, bytes6) );

        //2. In CScreenStore::?()

        //push eax
        //call
        uchar bytes7[] = {0x50,
                        0xE8};
        vDataList.push_back( Data(0x79DA33, 2, bytes7) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x79DA34;
        uchar* bytes8 = (uchar*)&address;
        vDataList.push_back( Data(0x79DA35, 4, bytes8) );

        //mov dword ptr ss:[ebp-50],eax
        //jmp 79DBEB
        //nop
        uchar bytes9[] = {0x89, 0x45, 0xB0,
                        0xE9, 0xAA, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x79DA39, 10, bytes9) );

        //push eax
        //call
        uchar bytes10[] = {0x50,
                        0xE8};
        vDataList.push_back( Data(0x7A0068, 2, bytes10) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x7A0069;
        uchar* bytes11 = (uchar*)&address;
        vDataList.push_back( Data(0x7A006A, 4, bytes11) );

        //mov dword ptr ss:[ebp-48],eax
        //jmp 7A0220
        //nop
        uchar bytes12[] = {0x89, 0x45, 0xB8,
                        0xE9, 0xAA, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x7A006E, 10, bytes12) );

        //3. In CScreenWorld::?()

        //push ecx
        //call
        uchar bytes13[] = {0x51,
                        0xE8};
        vDataList.push_back( Data(0x7D95D6, 2, bytes13) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x7D95D7;
        uchar* bytes14 = (uchar*)&address;
        vDataList.push_back( Data(0x7D95D8, 4, bytes14) );

        //mov dword ptr ss:[ebp-30],eax
        //jmp 7D9784
        //nop
        uchar bytes15[] = {0x89, 0x45, 0xD0,
                        0xE9, 0xA0, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x7D95DC, 10, bytes15) );

        //push eax
        //call
        uchar bytes16[] = {0x50,
                        0xE8};
        vDataList.push_back( Data(0x7E62AF, 2, bytes16) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x7E62B0;
        uchar* bytes17 = (uchar*)&address;
        vDataList.push_back( Data(0x7E62B1, 4, bytes17) );

        //mov dword ptr ss:[ebp-48],eax
        //jmp 7E6464
        //nop
        uchar bytes18[] = {0x89, 0x45, 0xB8,
                        0xE9, 0xA7, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x7E62B5, 10, bytes18) );

        //4. CCreatureObject::Refresh()

        //add eax,0B0A
        //push eax
        //call
        uchar bytes19[] = {0x05, 0x0A, 0x0B, 0x00, 0x00,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x8EC906, 7, bytes19) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x8EC90C;
        uchar* bytes20 = (uchar*)&address;
        vDataList.push_back( Data(0x8EC90D, 4, bytes20) );

        //mov dword ptr ss:[ebp-40],eax
        //mov edx,eax <-to allow compatibility with externalise encumbrance hack
        //jmp 8ECAE6
        //nop
        uchar bytes21[] = {0x89, 0x45, 0xC0,
                        0x8B, 0xD0,
                        0xE9, 0xCB, 0x01, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x8EC911, 14, bytes21) );

        //add eax,0B0A
        //push eax
        //call
        uchar bytes22[] = {0x05, 0x0A, 0x0B, 0x00, 0x00,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x8EFBD9, 7, bytes22) );

        //CALL address
        ptr = (void*)CRuleTables_GetWeightAllowance;
        address = (DWORD)ptr - 5  - 0x8EFBDF;
        uchar* bytes23 = (uchar*)&address;
        vDataList.push_back( Data(0x8EFBE0, 4, bytes23) );

        //mov dword ptr ss:[ebp-118],eax
        //mov edx,eax <-to allow compatibility with externalise encumbrance hack
        //jmp 8EFDC8
        //nop
        uchar bytes24[] = {0x89, 0x85, 0xE8, 0xFE, 0xFF, 0xFF,
                        0x8B, 0xD0,
                        0xE9, 0xD7, 0x01, 0x00, 0x00,
                        0x90};
        vDataList.push_back( Data(0x8EFBE4, 14, bytes24) );


        //CMessageOnUpdateCharacterSlotReply::DoMessage() correction of CDerivedStatsTemplate size
        //mov edx,dword ptr ds:[]
        uchar bytes25[] = {0x8B, 0x15};
        vDataList.push_back( Data(0x44523A, 2, bytes25) );

        //[g_nCDerivedStatsTemplateSize]
        address = (DWORD)&g_nCDerivedStatsTemplateSize;
        uchar* bytes26 = (uchar*)&address;
        vDataList.push_back( Data(0x44523C, 4, bytes26) );

        //add eax,edx
        //nop
        //mov ecx,dword ptr ss:[ebp+C]
        //add ecx,eax
        //mov dx,word ptr ds:[ecx]
        //mov word ptr ss:[ebp-10C],dx
        uchar bytes27[] = {0x03, 0xC2,
                         0x90, 0x90, 0x90, 0x90,
                         0x8B, 0x4D, 0x0C,
                         0x03, 0xC8,
                         0x66, 0x8B, 0x11,
                         0x66, 0x89, 0x95, 0xF4, 0xFE, 0xFF, 0xFF};
        vDataList.push_back( Data(0x445240, 21, bytes27) );

        COMMIT_vDataList;
    }

    
    if (pGameOptionsEx->bEngineExternClassRaceRestrictions) {
        //ensures current mage kit is allowed before allowing Enter shortcut key to go forward in uchargen

        //push ecx
        //call
        uchar bytes[] = {0x51,
                        0xE8};
        vDataList.push_back( Data(0x72B3DC, 2, bytes) );

        //CALL address
        void* ptr = (void*)CScreenCharGen_MageSchoolPanelCanContinue;
        DWORD address = (DWORD)ptr - 5  - 0x72B3DD;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x72B3DE, 4, bytes2) );

        //mov dword ptr ss:[ebp-18],eax
        //jmp 72B4F2
        //nop
        uchar bytes3[] = {0x89, 0x45, 0xE8,
                        0xE9, 0x08, 0x01, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x72B3E2, 15, bytes3) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineExternDifficulty) {
        
        //1. CInfGame::LoadBaldurIni()
        //push edx
        //call
        uchar bytes[] = {0x52,
                        0xE8};
        vDataList.push_back( Data(0x68877B, 2, bytes) );

        //CALL address
        void* ptr = (void*)CInfGame_SetDifficultyMultiplier;
        DWORD address = (DWORD)ptr - 5  - 0x68877C;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x68877D, 4, bytes2) );

        //jmp 6887E6
        uchar bytes3[] = {0xEB, 0x63,
                        0x90};
        vDataList.push_back( Data(0x688781, 3, bytes3) );

        //2. CUIScrollBarOptionsSlider::OnMouseDragKnob()
        //push edx
        //call
        uchar bytes4[] = {0x8D, 0x45, 0xDC,
                        0x50,
                        0x8B, 0x55, 0xE8,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x7826D2, 9, bytes4) );

        //CALL address
        ptr = (void*)CInfGame_SetDifficultyMultiplierFeedback;
        address = (DWORD)ptr - 5  - 0x7826DA;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x7826DB, 4, bytes5) );

        //jmp 6887E6
        uchar bytes6[] = {0xEB, 0x7F,
                        0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x7826DF, 5, bytes6) );

        //3. CCreatureObject::ApplyRulesToStats()
        uchar bytes7[] = {0x8B, 0x95, 0x48, 0xF6, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x8CDD1D, 8, bytes7) );

        //CALL address
        ptr = (void*)CCreatureObject_SetDifficultyLuckModifier;
        address = (DWORD)ptr - 5  - 0x8CDD24;
        uchar* bytes8 = (uchar*)&address;
        vDataList.push_back( Data(0x8CDD25, 4, bytes8) );

        //jmp short 8CDD89
        //nop
        uchar bytes9[] = {0xE9, 0xA4, 0x00, 0x00, 0x00,
                        0x90};
        vDataList.push_back( Data(0x8CDD29, 6, bytes9) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bEngineExternEncumbrance) {
        //1. some animation thing depending on encumbrance

        //push edx
        //mov ecx,dword ptr ss:[ebp-44]
        //push ecx
        //call
        uchar bytes[] = {0x52,
                        0x8B, 0x4D, 0xBC,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x8ECAED, 6, bytes) );

        //CALL address
        void* ptr = (void*)CRuleTables_IsHighEncumbrance;
        DWORD address = (DWORD)ptr - 5  - 0x8ECAF2;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x8ECAF3, 4, bytes2) );

        //test eax,eax
        //mov edx,dword ptr ss:[ebp-40] <-- nTotalWeightAllowance for next condition
        //nop
        //je short 8ECB6B
        uchar bytes3[] = {0x85, 0xC0,
                        0x8B, 0x55, 0xC0,
                        0x90, 0x90, 0x90,
                        0x74, 0x6A};
        vDataList.push_back( Data(0x8ECAF7, 10, bytes3) );

        //push edx
        //push ecx
        //call
        uchar bytes4[] = {0x52,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x8ECB6E, 3, bytes4) );

        //CALL address
        ptr = (void*)CRuleTables_IsLowEncumbrance;
        address = (DWORD)ptr - 5  - 0x8ECB70;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x8ECB71, 4, bytes5) );

        //test eax,eax
        //nop
        //je 8ECC41
        //mov edx,dword ptr ss:[ebp-880]
        //add edx,2C4A
        //mov dword ptr ss:[ebp-3CC],edx
        //mov eax,dword ptr ss:[ebp-3CC]
        //cmp dword ptr ds:[eax],0
        //jnz short BGMain.008ECBB5
        //push 0
        //push B04B38
        //push B04B10
        //push 830
        //call AssertionFailedQuit
        //add esp,10
        //mov ecx,dword ptr ss:[ebp-3CC]
        //mov ecx,dword ptr ds:[ecx]
        //mov edx,dword ptr ds:[ecx]
        uchar bytes6[] = {0x85, 0xC0,
                        0x90, 0x90,
                        0x0F, 0x84, 0xC2, 0x00, 0x00, 0x00,
                        0x8B, 0x95, 0x80, 0xF7, 0xFF, 0xFF,
                        0x81, 0xC2, 0x4A, 0x2C, 0x00, 0x00,
                        0x89, 0x95, 0x34, 0xFC, 0xFF, 0xFF,
                        0x8B, 0x85, 0x34, 0xFC, 0xFF, 0xFF,
                        0x83, 0x38, 0x00,
                        0x75, 0x19,
                        0x6A, 0x00,
                        0x68, 0x38, 0x4B, 0xB0, 0x00,
                        0x68, 0x10, 0x4B, 0xB0, 0x00,
                        0x68, 0x30, 0x08, 0x00, 0x00,
                        0xE8, 0xBA, 0x24, 0x0B, 0x00,
                        0x83, 0xC4, 0x10,
                        0x8B, 0x8D, 0x34, 0xFC, 0xFF, 0xFF,
                        0x8B, 0x09,
                        0x8B, 0x11};
        vDataList.push_back( Data(0x8ECB75, 74, bytes6) );

        //2. settings of encumbrance in CDerivedStats

        //push edx
        //mov ecx,dword ptr ss:[ebp-11c]
        //push ecx
        //call
        uchar bytes7[] = {0x52,
                        0x8B, 0x8D, 0xE4, 0xFE, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x8EFDCF, 9, bytes7) );

        //CALL address
        ptr = (void*)CRuleTables_IsHighEncumbrance;
        address = (DWORD)ptr - 5  - 0x8EFDD7;
        uchar* bytes8 = (uchar*)&address;
        vDataList.push_back( Data(0x8EFDD8, 4, bytes8) );

        //test eax,eax
        //nop
        //je 8EFE7D
        uchar bytes9[] = {0x85, 0xC0,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                        0x0F, 0x84, 0x90, 0x00, 0x00, 0x00};
        vDataList.push_back( Data(0x8EFDDC, 17, bytes9) );

        //mov edx,dword ptr ss:[ebp-118]
        //push edx
        //mov ecx,dword ptr ss:[ebp-11c]
        //push ecx
        //call
        uchar bytes10[] = {0x8B, 0x95, 0xE8, 0xFE, 0xFF, 0xFF,
                        0x52,
                        0x8B, 0x8D, 0xE4, 0xFE, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x8EFE7D, 15, bytes10) );

        //CALL address
        ptr = (void*)CRuleTables_IsLowEncumbrance;
        address = (DWORD)ptr - 5  - 0x8EFE8B;
        uchar* bytes11 = (uchar*)&address;
        vDataList.push_back( Data(0x8EFE8C, 4, bytes11) );

        //test eax,eax
        //je 8EFF7D
        //nop
        //mov eax,dword ptr ds:[B773CC]
        //mov ecx,dword ptr ss:[eax+42BA]
        //mov eax,dword ptr ds:[ecx+4302]
        uchar bytes12[] = {0x85, 0xC0,
                        0x0F, 0x84, 0xE5, 0x00, 0x00, 0x00,
                        0x90, 0x90,
                        0xA1, 0xCC, 0x73, 0xB7, 0x00,
                        0x36, 0x8B, 0x88, 0xBA, 0x42, 0x00, 0x00,
                        0x8B, 0x81, 0x02, 0x43, 0x00, 0x00};
        vDataList.push_back( Data(0x8EFE90, 28, bytes12) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineExternMageSpellsCap) {
        //1. CScreenRecord::MageBookPanelOnLoad() - terminate loop at 200 instead of when invalid spell
        //mov eax,dword ptr ss:[ebp-9C]
        //cmp eax,0C7
        //jl short 6E14BC
        //nop
        uchar bytes1[] = {0x8B, 0x85, 0x64, 0xFF, 0xFF, 0xFF,
                        0x3D, 0xC8, 0x00, 0x00, 0x00,
                        0x7C, 0x0C,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x6E14A3, 20, bytes1) );

        //2. CScreenCharGen::MageBookPanelOnLoad() - terminate loop at 200 instead of when invalid spell
        //mov eax,dword ptr ss:[ebp-98]
        //cmp eax,0C8
        //jl short 71B74B
        //nop
        uchar bytes2[] = {0x8B, 0x85, 0x68, 0xFF, 0xFF, 0xFF,
                        0x3D, 0xC8, 0x00, 0x00, 0x00,
                        0x7C, 0x0C,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x71B732, 20, bytes2) );

        //3. CScreenCharGen::AutoPickSpells() - loop end point
        //movzx eax,byte ptr ss:[ebp-2C]
        //cmp eax,0C7
        //nop
        uchar bytes3[] = {0x0F, 0xB6, 0x45, 0xD4,
                        0x3D, 0xC7, 0x00, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x72F369, 11, bytes3) );

        //4. CScreenCharGen::HasSpecialistSpells() - loop end point
        //movzx ecx,byte ptr ss:[ebp-1C]
        //cmp eax,0C7
        //nop
        uchar bytes4[] = {0x0F, 0xB6, 0x4D, 0xE4,
                        0x81, 0xF9, 0xC7, 0x00, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x72FB61, 12, bytes4) );

        COMMIT_vDataList;
    }
        
    if (pGameOptionsEx->bEngineProficiencyRestrictions) {
        //changing PUSH arguments
        uchar bytes[] = {0x8B, 0x45, 0x08, 0x50, 0x90};
        vDataList.push_back( Data(0x6DC54F, 5, bytes) );
    
        //CALL address
        void* ptr = (void*)CRuleTables_GetMaxProfs;
        DWORD address = (DWORD)ptr - 5  - 0x6DC560;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x6DC561, 4, bytes2) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineRestSpawnsAdvanceTime) {
        //mov ecx,dword ptr ds:[<TICKS_EIGHT_HOURS>]
        //imul ecx,dword ptr ss:[ebp-CC]
        //add ecx,dword ptr ss:[ebp-E0]
        //push ecx
        //mov ecx,dword ptr ss:[ebp-3F8]
        //add ecx,1DD0
        //call CWorldTimer::AdvanceByTime
        //mov edx,dword ptr ds:[<TICKS_EIGHT_HOURS>]
        //imul edx,dword ptr ss:[ebp-CC]
        //add edx,dword ptr ss:[ebp-E0]
        //push edx
        //push 7
        //mov ecx,dword ptr ss:[ebp-3F8]
        //call CInfGame::PrintEvaluatedMessage
        //cmp dword ptr ss:[ebp-CC],0
        //jle BGMain.006ABC61
        uchar bytes[] = {0x8B, 0x0D, 0xCC, 0x0F, 0xB8, 0x00,
                        0x0F, 0xAF, 0x8D, 0x34, 0xFF, 0xFF, 0xFF,
                        0x03, 0x8D, 0x20, 0xFF, 0xFF, 0xFF,
                        0x51,
                        0x8B, 0x8D, 0x08, 0xFC, 0xFF, 0xFF,
                        0x81, 0xC1, 0xD0, 0x1D, 0x00, 0x00,
                        0xE8, 0x28, 0xDE, 0xF9, 0xFF,
                        0x8B, 0x15, 0xCC, 0x0F, 0xB8, 0x00,
                        0x0F, 0xAF, 0x95, 0x34, 0xFF, 0xFF, 0xFF,
                        0x03, 0x95, 0x20, 0xFF, 0xFF, 0xFF,
                        0x52,
                        0x6A, 0x07,
                        0x8B, 0x8D, 0x08, 0xFC, 0xFF, 0xFF,
                        0xE8, 0x3F, 0x32, 0x00, 0x00,
                        0x83, 0xBD, 0x34, 0xFF, 0xFF, 0xFF, 0x00,
                        0x0F, 0x8E, 0xBA, 0x09, 0x00, 0x00};
        vDataList.push_back( Data(0x6AB254, 83, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineSummonLimitFix) {
        uchar bytes[] = {0x74,0x05,
                        0x83,0xF8,0x09,
                        0x75,0x29,
                        0x8B,0x45,0xF0,
                        0x8B,0x48,0x12,
                        0x83,0xF9,0x00,
                        0x74,0x1E,
                        0xEB,0x13,
                        0x90,0x90};
        vDataList.push_back( Data(0x6B93DB, 22, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineTargetDeadFix) {
        //NOTE: this is an incomplete fix since it only applies to objects existing in LIST_FRONT

        //1. for all before posSource

        //mov edx,dword ptr ss:[ebp-5c]
        //push edx
        //call
        uchar bytes[] = {0x8B, 0x55, 0xA4,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x4BA3DD, 5, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_IsDeadInFrontVerticalList;
        DWORD address = (DWORD)ptr - 5  - 0x4BA3E1;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x4BA3E2, 4, bytes2) );

        //test eax,eax
        //je short 4BA3FA
        //mov dword ptr ss:[ebp-3c],0
        //jmp 4BA4E3
        //nop
        //mov ecx,dword ptr ss:[ebp-5c]
        //call CCreatureObject::GetDerivedStats()
        //nop
        //mov dword ptr ss:[ebp-ac],eax
        uchar bytes3[] = {0x85, 0xC0,
                        0x74, 0x10,
                        0xC7, 0x45, 0xC4, 0x00, 0x00, 0x00, 0x00,
                        0xE9, 0xED, 0x00, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90,
                        0x8B, 0x4D, 0xA4,
                        0xE8, 0x2E, 0xB2, 0xFD, 0xFF,
                        0x90,
                        0x89, 0x85, 0x54, 0xFF, 0xFF, 0xFF};
        vDataList.push_back( Data(0x4BA3E6, 35, bytes3) );

        //2. for all after posSource

        //mov edx,dword ptr ss:[ebp-5c]
        //push edx
        //call
        uchar bytes4[] = {0x8B, 0x55, 0xA4,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x4BA862, 5, bytes4) );

        //CALL address
        ptr = (void*)CCreatureObject_IsDeadInFrontVerticalList;
        address = (DWORD)ptr - 5  - 0x4BA866;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x4BA867, 4, bytes5) );

        //test eax,eax
        //je short 4BA87F
        //mov dword ptr ss:[ebp-3c],0
        //jmp 4BA968
        //nop
        //mov ecx,dword ptr ss:[ebp-5c]
        //call CCreatureObject::GetDerivedStats()
        //nop
        //mov dword ptr ss:[ebp-ac],eax
        uchar bytes6[] = {0x85, 0xC0,
                        0x74, 0x10,
                        0xC7, 0x45, 0xC4, 0x00, 0x00, 0x00, 0x00,
                        0xE9, 0xED, 0x00, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90,
                        0x8B, 0x4D, 0xA4,
                        0xE8, 0xA9, 0xAD, 0xFD, 0xFF,
                        0x90,
                        0x89, 0x85, 0x28, 0xFF, 0xFF, 0xFF};
        vDataList.push_back( Data(0x4BA86B, 35, bytes6) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bEngineWeapSpecNumAttacksMod) {
        //1. jmp -> monk class check section
        uchar bytes[] = {0xE9, 0x25, 0x01, 0x00, 0x00, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x8CE8AF, 8, bytes) );

        //3. CDerivedStats::GetMeanLevel()
        uchar bytes2[] = {0xE8, 0x7F, 0x57, 0xBA, 0xFF};
        vDataList.push_back( Data(0x8CE91E, 5, bytes2) );

        //2. if not monk, jmp to fighter section
        uchar bytes3[] = {0x0F, 0x84, 0x0C, 0xFF, 0xFF, 0xFF};
        vDataList.push_back( Data(0x8CE9F1, 6, bytes3) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bItemsAbilityItemAnim) {
        //1. CCreatureObject::UseItem()
        //mov edx,dword ptr ss:[ebp-388]
        //push edx
        //call
        uchar bytes[] = {0x8B, 0x95, 0x78, 0xFC, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x9213F3, 8, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_UseItem_OverrideAnimation;
        DWORD address = (DWORD)ptr - 5  - 0x9213FA;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9213FB, 4, bytes2) );

        //jmp 9215AA
        //nop
        uchar bytes3[] = {0xE9, 0xA6, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x9213FF, 7, bytes3) );

        //2. CCreatureObject::UseItemPoint()
        //mov ecx,dword ptr ss:[ebp-2FC]
        //push ecx
        //call
        uchar bytes4[] = {0x8B, 0x8D, 0x04, 0xFD, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x922F8F, 8, bytes4) );

        //CALL address
        ptr = (void*)CCreatureObject_UseItem_OverrideAnimation;
        address = (DWORD)ptr - 5  - 0x922F96;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x922F97, 4, bytes5) );

        //jmp 923148
        //nop
        uchar bytes6[] = {0xE9, 0xA8, 0x01, 0x00, 0x00,
                        0x90, 0x90};
        vDataList.push_back( Data(0x922F9B, 7, bytes6) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bItemsBackstabRestrictionsConfig ||
        pGameOptionsEx->bEffBackstabEveryHitConfig) {
        /*mov eax,dword ptr ss:[ebp+24]
        push eax
        mov eax,dword ptr ss:[ebp+20]
        push eax
        mov eax,dword ptr ss:[ebp+1C]
        push eax
        mov eax,dword ptr ss:[ebp-38]
        push eax
        mov eax,dword ptr ss:[ebp-50]
        push eax
        mov eax,dword ptr ss:[ebp-1c]
        push eax
        mov eax,dword ptr ss:[ebp-198]
        push eax
        CALL*/
        uchar bytes[] = {
            0x8B, 0x45, 0x24, \
            0x50, \
            0x8B, 0x45, 0x20, \
            0x50, \
            0x8B, 0x45, 0x1c, \
            0x50, \
            0x8B, 0x45, 0xC8, \
            0x50, \
            0x8B, 0x45, 0xB0, \
            0x50, \
            0x8B, 0x45, 0xE4, \
            0x50, \
            0x8B, 0x85, 0x68, 0xFE, 0xFF, 0xFF, \
            0x50, \
            0xE8
        };
        vDataList.push_back( Data(0x90D1B2, 32, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_ApplyDamage_TryBackstab;
        DWORD address = (DWORD)ptr - 5  - 0x90D1D1;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x90D1D2, 4, bytes2) );

        //TEST EAX, EAX
        //JE 90D655
        //JMP 90D48D
        //NOP
        uchar bytes3[] = {
            0x85, 0xC0, \
            0x0F, 0x84, 0x77, 0x04, 0x00, 0x00, \
            0xE9, 0xAA, 0x02, 0x00, 0x00, \
            0x90
        };
        vDataList.push_back( Data(0x90D1D6, 14, bytes3) );

    }
  
    if (pGameOptionsEx->bItemsCriticalHitAversionConfig) {

        //PUSH [EBP+Cre]
        //PUSH [EBP+CreTarget]
        uchar bytes1[] = {0xFF, 0xB5, 0x6C, 0xFD, 0xFF, 0xFF, 0xFF, 0x75, 0x08};
        vDataList.push_back( Data(0x90B9A3, 9, bytes1) );

        //CALL
        uchar bytes2[] = {0xE8};
        vDataList.push_back( Data(0x90B9AC, 1, bytes2) );

        //CALL address
        void* ptr = (void*)CCreatureObject_ShouldAvertCriticalHit;
        DWORD address = (DWORD)ptr - 5  - 0x90B9AC;
        uchar* bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x90B9AD, 4, bytes3) );

        //TEST EAX, EAX
        //NOP
        uchar bytes4[] = {0x85, 0xC0, 0x90, 0x90 };
        vDataList.push_back( Data(0x90B9B1, 4, bytes4) );

        COMMIT_vDataList;
    } else {
        uchar bytes6[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };
        CallInject(0x90BA0C,   CCreatureObject_ShouldAvertCriticalHit_SpellOnly_asm, bytes6);
        CallInject(0x53B20A,   CEffectSpellOnCondition_Apply_AddCriticalHit_asm, bytes6);

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bItemsTargetInvisConfig) {
        //jz 91F812 (pAbility != NULL)
        //mov ecx,dword ptr ss:[ebp+8]
        //push ecx
        //mov edx,dword ptr ss:[ebp-388]
        //push edx
        //call
        uchar bytes[] = {0x0F, 0x84, 0x4D, 0xFF, 0xFF, 0xFF,
                        0x8B, 0x4D, 0x08,
                        0x51,
                        0x8B, 0x95, 0x78, 0xFC, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x91F8BF, 18, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_UseItem_CannotTargetInvisible;
        DWORD address = (DWORD)ptr - 5  - 0x91F8D0;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x91F8D1, 4, bytes2) );

        uchar bytes3[] = {0x85, 0xC0,
                        0x0F, 0x85, 0x35, 0xFF, 0xFF, 0xFF};
        vDataList.push_back( Data(0x91F8D5, 8, bytes3) );

        COMMIT_vDataList;
    }

    //Expand RND*.2DA
    if (pGameOptionsEx->bItemsRandomTreasureExtend) {
        uchar bytes[29] = {0x8B, 0x45, 0xF0, 0x36, 0x0F, 0xBE, 0x48, 0x06, 0x83, 0xE9, 0x30, 0x6B, 0xC9, 0x0A, 0x36, 0x02, 0x48, 0x07, 0x83, 0xE9, 0x31, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    
        vDataList.push_back( Data(0x5A8607, 29, bytes) ); //RNDEQU
        vDataList.push_back( Data(0x5A8712, 29, bytes) ); //RNDTRE
        vDataList.push_back( Data(0x5A8829, 29, bytes) ); //RNDMAG
        vDataList.push_back( Data(0x5A8940, 29, bytes) ); //RNDWEP
        vDataList.push_back( Data(0x5A8A40, 29, bytes) ); //RNDSCR

        vDataList.push_back( Data(0x5A8E56, 29, bytes) ); //RNDEQU
        vDataList.push_back( Data(0x5A8F39, 29, bytes) ); //RNDTRE
        vDataList.push_back( Data(0x5A904F, 29, bytes) ); //RNDMAG
        vDataList.push_back( Data(0x5A9165, 29, bytes) ); //RNDSCR

        vDataList.push_back( Data(0x5A963A, 29, bytes) ); //RNDEQU
        vDataList.push_back( Data(0x5A971D, 29, bytes) ); //RNDTRE
        vDataList.push_back( Data(0x5A9833, 29, bytes) ); //RNDMAG
        vDataList.push_back( Data(0x5A9949, 29, bytes) ); //RNDWEP
        vDataList.push_back( Data(0x5A9A48, 29, bytes) ); //RNDSCR

        uchar bytes2[3] = { 0x90, 0x90, 0x90 };
        vDataList.push_back( Data(0x5A8656, 3, bytes2) ); //RNDEQU
        vDataList.push_back( Data(0x5A876D, 3, bytes2) ); //RNDTRE
        vDataList.push_back( Data(0x5A8884, 3, bytes2) ); //RNDMAG
        vDataList.push_back( Data(0x5A8984, 3, bytes2) ); //RNDTRE
        vDataList.push_back( Data(0x5A8A84, 3, bytes2) ); //RNDSCR

        vDataList.push_back( Data(0x5A8E9F, 3, bytes2) ); //RNDEQU
        vDataList.push_back( Data(0x5A8F94, 3, bytes2) ); //RNDTRE
        vDataList.push_back( Data(0x5A90AA, 3, bytes2) ); //RNDMAG
        vDataList.push_back( Data(0x5A91A9, 3, bytes2) ); //RNDSCR

        vDataList.push_back( Data(0x5A9683, 3, bytes2) ); //RNDEQU
        vDataList.push_back( Data(0x5A9778, 3, bytes2) ); //RNDTRE
        vDataList.push_back( Data(0x5A988E, 3, bytes2) ); //RNDMAG
        vDataList.push_back( Data(0x5A998D, 3, bytes2) ); //RNDTRE
        vDataList.push_back( Data(0x5A9A8C, 3, bytes2) ); //RNDSCR

        uchar bytes3[] = {0x8D};
        vDataList.push_back( Data(0x5A8625, 1, bytes3) ); //RNDEQU

        bytes3[0] = 0x8D;
        vDataList.push_back( Data(0x5A8730, 1, bytes3) ); //RNDTRE
        vDataList.push_back( Data(0x5A8847, 1, bytes3) ); //RNDMAG
        vDataList.push_back( Data(0x5A895E, 1, bytes3) ); //RNDWEP
        vDataList.push_back( Data(0x5A8A5E, 1, bytes3) ); //RNDWEP

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bItemsExternCreExcl) {
        //modify je address
        uchar bytes[] = {0xE3, 0x02};
        vDataList.push_back( Data(0x69CB6E, 2, bytes) );

        //mov eax,dword ptr ss:[ebp+14]
        //push eax
        //mov ecx,dword ptr ss:[ebp+10]
        //mov edx,dword ptr ds:[ecx]
        //push edx
        //movsx eax,word ptr ss:[ebp+C]
        //push eax
        //mov ecx,dword ptr ss:[ebp-48]
        //push ecx
        //call ...
        uchar bytes3[] = {0x8B, 0x45, 0x14,
                        0x50,
                        0x8B, 0x4D, 0x10,
                        0x8B, 0x11,
                        0x52,
                        0x0F, 0xBF, 0x45, 0x0C,
                        0x50,
                        0x8B, 0x4D, 0xB8,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x69CB72, 20, bytes3) );

        //CALL address
        void* ptr4 = (void*)CRuleTables_DoesEquipSlotPassCreExclude;
        DWORD address4 = (DWORD)ptr4 - 5  - 0x69CB85;
        uchar* bytes4 = (uchar*)&address4;
        vDataList.push_back( Data(0x69CB86, 4, bytes4) );

        //test eax,eax
        //jnz BGMain.0069CE55
        //mov dword ptr ss:[ebp-54],0
        //jmp BGMain.0069CE55
        uchar bytes5[] = {0x85, 0xC0,
                        0x0F, 0x85, 0xC3, 0x02, 0x00, 0x00,
                        0xC7, 0x45, 0xAC, 0x00, 0x00, 0x00, 0x00,
                        0xE9, 0xB7, 0x02, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x69CB8A, 24, bytes5) );

        //mov ecx,dword ptr ss:[ebp+14]
        //push ecx
        //mov eax,dword ptr ss:[ebp+c]
        //mov ecx,dword ptr ds:[eax]
        //push ecx
        //movsx ecx,word ptr ss:[ebp+10]
        //push ecx
        //mov edx,dword ptr ss:[ebp-1C]
        //push edx
        //call ...
        uchar bytes6[] = {0x8B, 0x4D, 0x14,
                        0x51,
                        0x8B, 0x45, 0x0C,
                        0x8B, 0x08,
                        0x51,
                        0x0F, 0xBF, 0x4D, 0x10,
                        0x51,
                        0x8B, 0x55, 0xE4,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x69FF65, 20, bytes6) );

        //CALL address
        void* ptr7 = (void*)CRuleTables_DoesInvSlotPassCreExclude;
        DWORD address7 = (DWORD)ptr7 - 5  - 0x69FF78;
        uchar* bytes7 = (uchar*)&address7;
        vDataList.push_back( Data(0x69FF79, 4, bytes7) );

        //test eax,eax
        //jnz BGMain.006A006D
        //mov dword ptr ss:[ebp-20],0
        //jmp BGMain.006A006D
        uchar bytes8[] = {0x85, 0xC0,
                        0x0F, 0x85, 0xE8, 0x00, 0x00, 0x00,
                        0xC7, 0x45, 0xE0, 0x00, 0x00, 0x00, 0x00,
                        0xE9, 0xDC, 0x00, 0x00, 0x00};
        vDataList.push_back( Data(0x69FF7D, 20, bytes8) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bItemsNonAmmoLauncherDamageFix) {
        //mov ecx,dword ptr ss:[ebp+c]
        //push ecx
        //nop
        uchar bytes[] = {0x8B, 0x4D, 0x0C,
                        0x51,
                        0x90};
        vDataList.push_back( Data(0x90C946, 5, bytes) );

        //CALL address
        void* ptr = (void*)CItem_GetFirstLauncherAbility;
        DWORD address = (DWORD)ptr - 5  - 0x90C94B;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x90C94C, 4, bytes2) );

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bItemsUseAnimPercentThrowingWeapons) {
        //1. prevent crashes if item ability attack %s do not add to 100%

        uchar bytes[] = {0x03};
        vDataList.push_back( Data(0x8AE50A, 1, bytes) );
        vDataList.push_back( Data(0x8AE6E3, 1, bytes) );
        /*
        //2. if attack %s do not add to 100% default to SLASH
        uchar bytes2[] = {0x0B};
        vDataList.push_back( Data(0x8AE70C, 1, bytes2) );
        */
        //3. these changes significantly reduce the amount of CMessageSetAnimationSequence objects sent during a round because throwing weapons are not matching SEQ_SHOOT

        //push ecx
        //call
        uchar bytes3[] = {0x51,
                        0xE8};
        vDataList.push_back( Data(0x90636E, 2, bytes3) );

        //CALL address
        void* ptr = (void*)CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly;
        DWORD address = (DWORD)ptr - 5  - 0x90636F;
        uchar* bytes4 = (uchar*)&address;
        vDataList.push_back( Data(0x906370, 4, bytes4) );

        //test eax, eax
        //jnz 90648F
        //mov edx, dword ptr ss:[ebp-B8]
        //mov al, byte ptr ds:[edx]
        //cmp al, 2
        uchar bytes5[] = {0x85, 0xC0,
                        0x0F, 0x85, 0x13, 0x01, 0x00, 0x00,
                        0x8B, 0x95, 0x48, 0xFF, 0xFF, 0xFF,
                        0x8A, 0x02,
                        0x3C, 0x02};
        vDataList.push_back( Data(0x906374, 18, bytes5) );

        //mov eax,dword ptr ss:[ebp-614]
        //push eax
        //call
        uchar bytes6[] = {0x8B, 0x85, 0xEC, 0xF9, 0xFF, 0xFF,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x90979C, 8, bytes6) );

        //CALL address
        ptr = (void*)CCreatureObject_HasThrowingWeaponEquippedHumanoidOnly;
        address = (DWORD)ptr - 5  - 0x9097A3;
        uchar* bytes7 = (uchar*)&address;
        vDataList.push_back( Data(0x9097A4, 4, bytes7) );

        //test eax,eax
        //jnz 9098C0
        //mov ecx,dword ptr ss:[ebp-B8]
        //mov dl,byte ptr ds:[ecx]
        //cmp dl,2
        //jnz 9098C0
        //mov al,byte ptr ss:[ebp-AC]
        //mov cl,8
        //cmp al,cl
        //nop
        uchar bytes8[] = {0x85, 0xC0,
                        0x0F, 0x85, 0x10, 0x01, 0x00, 0x00,
                        0x8B, 0x8D, 0x48, 0xFF, 0xFF, 0xFF,
                        0x8A, 0x11,
                        0x80, 0xFA, 0x02,
                        0x0F, 0x85, 0xFF, 0x00, 0x00, 0x00,
                        0x8A, 0x85, 0x54, 0xFF, 0xFF, 0xFF,
                        0xB1, 0x08,
                        0x3A, 0xC1,
                        0x90, 0x90};
        vDataList.push_back( Data(0x9097A8, 37, bytes8) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bMusicSonglistExtend) {
        //treat song entries as int instead of signed byte
        //section that stops the music
        uchar bytes[] = {0x89};
        vDataList.push_back( Data(0x4CC6F7, 1, bytes) );

        uchar bytes2[] = {0xA8, 0x8B, 0x4D, 0xA8, 0x89, 0x4D, 0xF4, 0x8B, 0x55, 0xFC, 0x90, 0x8B, 0x45, 0xF4, 0x90};
        vDataList.push_back( Data(0x4CC708, 15, bytes2) );

        uchar bytes3[] = {0x8B, 0x45, 0xF4, 0x90};
        vDataList.push_back( Data(0x4CC735, 4, bytes3) );
    
        //CArea::ChangeSong()
        bytes[0] = 0x89;
        vDataList.push_back( Data(0x4D4892, 1, bytes) );

        bytes3[0] = 0x8B; bytes3[1] = 0x45; bytes3[2] = 0xF8; bytes3[3] = 0x90;
        vDataList.push_back( Data(0x4D489D, 4, bytes3) );
    
        COMMIT_vDataList;
    }

    //Correct random selection of animation soundsets
    if (pGameOptionsEx->bSoundAnimSoundFix) {
        //decrements the rand number so intervening sounds can be played correctly

        //mov ecx,dword ptr ss:[ebp-8]
        //dec ecx
        //mov dword ptr ss:[ebp-8],ecx
        //nop
        uchar bytes[] = {0x8B, 0x4D, 0xF8,
                        0x49,
                        0x89, 0x4D, 0xF8,
                        0x90, 0x90};
        vDataList.push_back( Data(0x642E0B, 9, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bSoundDlgGreetingSubtitles) {
        //bPrintToConsole in CCreatureObject::PlaySound()
        uchar bytes[] = {0x1};
        vDataList.push_back( Data(0x8D1EF0, 1, bytes) );
        vDataList.push_back( Data(0x8D1F02, 1, bytes) );
        //bPrintToConsole in CMessagePlaySoundset
        vDataList.push_back( Data(0x8D1F7E, 1, bytes) );
        vDataList.push_back( Data(0x8D2019, 1, bytes) );
    
        COMMIT_vDataList;
    }

    //Correct 'periodic fidget' existance sound
    if (pGameOptionsEx->bSoundExistenceFix) {
        //correct func parameter to determine number of available EXISTANCE sounds
        uchar bytes[1] = {0x46};
        vDataList.push_back( Data(0x8A3B4A, 1, bytes) );

        //correct pointer to EXISTANCE soundset in m_BaseStats
        bytes[0] = 0xAA;
        vDataList.push_back( Data(0x8A3BB6, 1, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bSpellsCastingFix) {

        //1. CUICheckButtonChargenGender::OnLClicked() - set the sex also on picking gender

        //mov edx,dword ptr ss:[ebp-30]
        //add edx,22F
        //mov byte ptr ss:[edx],al
        //nop
        uchar bytes[] = {0x8B, 0x55, 0xD0,
                        0x81, 0xC2, 0x2F, 0x02, 0x00, 0x00,
                        0x36, 0x88, 0x02,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x733569, 18, bytes) );

        //mov ecx,dword ptr ss:[ebp-40]
        uchar bytes2[] = {0x8B, 0x4D, 0xC0};
        vDataList.push_back( Data(0x73357F, 3, bytes2) );

        //2. CCreatureObject::DoSpellCasting() - correct casting speed check for animation
        
        //push ecx
        //nop
        //mov eax,dword ptr ss:[ebp-1F4]
        //push eax
        //call
        uchar bytes3[] = {0x52,
                        0x90, 0x90, 0x90, 0x90,
                        0x8B, 0x95, 0x0C, 0xFE, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x940281, 13, bytes3) );

        //CALL address
        void* ptr = (void*)CCreatureObject_DoSpellCasting_GetCastingSpeed;
        DWORD address = (DWORD)ptr - 5  - 0x94028D;
        uchar* bytes4 = (uchar*)&address;
        vDataList.push_back( Data(0x94028E, 4, bytes4) );

        //3. CCreatureObject::DoSpellCasting() - correct casting speed check for playing of a sound

        //push ecx
        //mov edx,dword ptr ss:[ebp-1F4]
        //push edx
        //call
        uchar bytes5[] = {0x51,
                        0x8B, 0x95, 0x0C, 0xFE, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x93FFF5, 9, bytes5) );

        //CALL address
        ptr = (void*)CCreatureObject_DoSpellCasting_GetCastingSpeed;
        address = (DWORD)ptr - 5  - 0x93FFFD;
        uchar* bytes6 = (uchar*)&address;
        vDataList.push_back( Data(0x93FFFE, 4, bytes6) );

        //test eax,eax
        //jg short 94000D
        //mov dword ptr ss:[ebp-60],0
        //cmp dword ptr ss:[ebp-60],0
        //je 94027E
        //mov ecx,dword ptr ss:[ebp-1F4]
        //mov eax,dword ptr ds:[ecx+12]
        //push eax
        //push 4
        uchar bytes7[] = {0x85, 0xC0,
                        0x7F, 0x07,
                        0xC7, 0x45, 0xA0, 0x00, 0x00, 0x00, 0x00,
                        0x83, 0x7D, 0xA0, 0x00,
                        0x0F, 0x84, 0x67, 0x02, 0x00, 0x00,
                        0x8B, 0x8D, 0x0C, 0xFE, 0xFF, 0xFF,
                        0x8B, 0x41, 0x12,
                        0x50,
                        0x6A, 0x04};
        vDataList.push_back( Data(0x940002, 33, bytes7) );

        if (pGameOptionsEx->bSpellsUnvoicedConfig == FALSE) {
            //4. identical to #2 of bSpellsUnvoicedConfig

            //push ecx
            //mov edx,dword ptr ss:[ebp+8]
            //push edx
            //mov eax,dword ptr ss:[ebp-1F4]
            //push eax
            //call
            uchar bytes8[] = {0x51,
                            0x8B, 0x55, 0x08,
                            0x52,
                            0x8B, 0x85, 0x0C, 0xFE, 0xFF, 0xFF,
                            0x50,
                            0xE8};
            vDataList.push_back( Data(0x93FCC4, 13, bytes8) );

            //CALL address
            void* ptr1 = (void*)CCreatureObject_DoSpellCasting_GetGenderLetter;
            DWORD address1 = (DWORD)ptr1 - 5  - 0x93FCD0;
            uchar* bytes9 = (uchar*)&address1;
            vDataList.push_back( Data(0x93FCD1, 4, bytes9) );

            //push eax
            //jmp 93FD64
            //nop
            uchar bytes10[] = {0x50,
                            0xE9, 0x89, 0x00, 0x00, 0x00,
                            0x90};
            vDataList.push_back( Data(0x93FCD5, 7, bytes10) );
        }

        //5. enable casting sound for ZERO/fire/fountain/swirl animation
        // TODO: remove zero anim
        if (0) {
            uchar bytes1[] = {0x01}; // enable sound   
            uchar bytes9[] = {0x39}; // *09.wav
            vDataList.push_back( Data(0x93FFA9+3, 1, bytes1) );
            vDataList.push_back( Data(0xB595E1,   1, bytes9) );
        }

        COMMIT_vDataList;
    }
    
    if (pGameOptionsEx->bSpellsUnvoicedConfig) {
        //1. CCreatureObject::Spell() - check if casting allowed

        //mov edx,dword ptr ss:[ebp-764]
        //push edx
        //call
        uchar bytes[] = {0x8B, 0x95, 0x9C, 0xF8, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x9123DE, 8, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_Spell_IsOverrideSilence;
        DWORD address = (DWORD)ptr - 5  - 0x9123E5;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x9123E6, 4, bytes2) );

        //test eax,eax
        //je 912571
        //jmp 9125A6
        uchar bytes3[] = {0x85, 0xC0,
                        0x0F, 0x84, 0x7F, 0x01, 0x00, 0x00,
                        0xE9, 0xAF, 0x01, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x9123EA, 18, bytes3) );

        //2. CCreatureObject::DoSpellCasting() - set correct sound

        //push ecx
        //mov edx,dword ptr ss:[ebp+8]
        //push edx
        //mov eax,dword ptr ss:[ebp-1F4]
        //push eax
        //call
        uchar bytes4[] = {0x51,
                        0x8B, 0x55, 0x08,
                        0x52,
                        0x8B, 0x85, 0x0C, 0xFE, 0xFF, 0xFF,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x93FCC4, 13, bytes4) );

        //CALL address
        ptr = (void*)CCreatureObject_DoSpellCasting_GetGenderLetter;
        address = (DWORD)ptr - 5  - 0x93FCD0;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x93FCD1, 4, bytes5) );

        //push eax
        //jmp 93FD64
        //nop
        uchar bytes6[] = {0x50,
                        0xE9, 0x89, 0x00, 0x00, 0x00,
                        0x90};
        vDataList.push_back( Data(0x93FCD5, 7, bytes6) );

        //3. CCreatureObject::SpellPoint() - check if casting allowed

        //mov edx,dword ptr ss:[ebp-5D0]
        //call CCreatureObject::GetDerivedStats()
        //mov dword ptr ss:[ebp-418],eax
        //mov ecx,dword ptr ds:[eax]
        //and ecx,1000
        //test ecx,ecx
        //je short 916DEE
        //mov edx,dword ptr ss:[ebp-5D0]
        //push edx
        //call
        uchar bytes7[] = {0x8B, 0x8D, 0x30, 0xFA, 0xFF, 0xFF,
                        0xE8, 0xB3, 0xE8, 0xB7, 0xFF,
                        0x89, 0x85, 0xE8, 0xFB, 0xFF, 0xFF,
                        0x8B, 0x08,
                        0x81, 0xE1, 0x00, 0x10, 0x00, 0x00,
                        0x85, 0xC9,
                        0x74, 0x5F,
                        0x8B, 0x95, 0x30, 0xFA, 0xFF, 0xFF,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x916D72, 37, bytes7) );

        //CALL address
        ptr = (void*)CCreatureObject_Spell_IsOverrideSilence;
        address = (DWORD)ptr - 5  - 0x916D96;
        uchar* bytes8 = (uchar*)&address;
        vDataList.push_back( Data(0x916D97, 4, bytes8) );

        //test eax,eax
        //je short 916DB9
        //jmp short 916DEE
        uchar bytes9[] = {0x85, 0xC0,
                        0x74, 0x1A,
                        0xEB, 0x4D};
        vDataList.push_back( Data(0x916D9B, 6, bytes9) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bSpellsTargetInvisConfig) {
        //mov edx,dword ptr ss:[ebp+8]
        //mov ecx,dword ptr ss:[ebp-764]
        //push edx
        //push ecx
        //call ...
        uchar bytes[] = {0x8B, 0x8D, 0x9C, 0xF8, 0xFF, 0xFF,
                        0x52,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x911EA7, 9, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_Spell_IsOverrideInvisible;
        DWORD address = (DWORD)ptr - 5  - 0x911EAF;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x911EB0, 4, bytes2) );

        //test eax,eax
        //je 911FA7
        //jmp 91222F
        uchar bytes3[] = {0x85, 0xC0,
                        0x0F, 0x84, 0xEB, 0x00, 0x00, 0x00,
                        0xE9, 0x6E, 0x03, 0x00, 0x00};
        vDataList.push_back( Data(0x911EB4, 13, bytes3) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bSpellsExternBardSong) {
        //mov ecx,dword ptr ss:[ebp-920]
        //call
        uchar bytes[] = {0x8B, 0x8D, 0xE0, 0xF6, 0xFF, 0xFF,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x8F1C77, 8, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_UpdateModalState_DoBardSongNormal;
        DWORD address = (DWORD)ptr - 5  - 0x8F1C7E;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x8F1C7F, 4, bytes2) );

        //jmp 8F26AC
        //nop
        uchar bytes3[] = {0xE9, 0x24, 0x0A, 0x00, 0x00,
                        0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x8F1C83, 10, bytes3) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bStoreItemRechargeMod) {
        //1. CStore::AddItem(): fix so a non-zero charge item will not stack onto an identical but zero charge item in the bag
        uchar bytes[] = {0x00}; //1 -> 0
        vDataList.push_back( Data(0x645E34, 1, bytes) );

        //2. CStore::AddItem()
        //mov ecx,dword ptr ss:[ebp-20]
        //push ecx
        //mov ecx,dword ptr ss:[ebp+8]
        //push ecx
        //mov eax,dword ptr ss:[ebp-6C]
        //push eax
        //call ...
        uchar bytes2[] = {0x8B, 0x4D, 0xE0,
                        0x51,
                        0x8B, 0x4D, 0x08,
                        0x51,
                        0x8B, 0x45, 0x94,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x645FA6, 13, bytes2) );

        //CALL address
        void* ptr = (void*)CStore_AddItem_SetUsages;
        DWORD address = (DWORD)ptr - 5  - 0x645FB2;
        uchar* bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x645FB3, 4, bytes3) );

        //jmp
        uchar bytes4[] = {0xEB, 0x3D,
                        0x90};
        vDataList.push_back( Data(0x645FB7, 3, bytes4) );

        //3. CStore::GetBuyPrice(): fix so host item price takes into account if item recharges
        //push ecx
        //lea eax,dword ptr ss:[ebp-20]
        //push eax
        //lea edx,dword ptr ss:[ebp-C]
        //push edx
        //mov eax,dword ptr ss:[ebp-28]
        //push eax
        //call ...
        uchar bytes5[] = {0x51,
                        0x8D, 0x45, 0xE0,
                        0x50,
                        0x8D, 0x55, 0xF4,
                        0x52,
                        0x8B, 0x45, 0xD8,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x645671, 14, bytes5) );

        //CALL address
        ptr = (void*)CStore_GetBuyPrice_GetChargePercent;
        address = (DWORD)ptr - 5  - 0x64567E;
        uchar* bytes6 = (uchar*)&address;
        vDataList.push_back( Data(0x64567F, 4, bytes6) );

        //jmp
        uchar bytes7[] = {0xEB, 0x21,
                        0x90, 0x90};
        vDataList.push_back( Data(0x645683, 4, bytes7) );

        //4. CStore::GetSellPrice(): fix so customer item price takes into account if item recharges
        //push ecx
        //lea edx,dword ptr ss:[ebp-38]
        //push edx
        //lea ecx,dword ptr ss:[ebp-14]
        //push ecx
        //mov edx,dword ptr ss:[ebp-58]
        //push edx
        //call ...
        uchar bytes8[] = {0x51,
                        0x8D, 0x55, 0xC8,
                        0x52,
                        0x8D, 0x4D, 0xEC,
                        0x51,
                        0x8B, 0x55, 0xA8,
                        0x52,
                        0xE8};
        vDataList.push_back( Data(0x6454D2, 14, bytes8) );

        //CALL address
        ptr = (void*)CStore_GetSellPrice_GetChargePercent;
        address = (DWORD)ptr - 5  - 0x6454DF;
        uchar* bytes9 = (uchar*)&address;
        vDataList.push_back( Data(0x6454E0, 4, bytes9) );

        //jmp
        uchar bytes10[] = {0xEB, 0x21,
                        0x90, 0x90};
        vDataList.push_back( Data(0x6454E4, 4, bytes10) );

        //5. CStore::UnmarshalItem(): fix so non-stackable items with usages of 0 do not unmarshal with usages of 1
        //mov ecx,dword ptr ss:[ebp+C]
        //push edx
        //push eax
        //push ecx
        //call
        uchar bytes11[] = {0x8B, 0x4D, 0x0C,
                        0x52,
                        0x50,
                        0x51,
                        0xE8};
        vDataList.push_back( Data(0x645AE6, 7, bytes11) );

        //CALL address
        ptr = (void*)CStore_UnmarshalItem_SetUsages;
        address = (DWORD)ptr - 5  - 0x645AEC;
        uchar* bytes12 = (uchar*)&address;
        vDataList.push_back( Data(0x645AED, 4, bytes12) );

        //jmp
        //nop
        uchar bytes13[] = {0xEB, 0x32,
                        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x645AF1, 11, bytes13) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bTriggerExpandedTriggers) {
        //lea edx,dword ptr ss:[ebp-68]
        //push edx
        //lea edx,dword ptr ss:[ebp-2C]
        //push edx
        //lea edx,dword ptr ss:[ebp-28]
        //push edx
        //lea edx,dword ptr ss:[ebp-60]
        //push edx
        //lea edx,dword ptr ss:[ebp-24]
        //push edx
        //lea edx,dword ptr ss:[ebp+8]
        //push edx
        //movsx edx,dword ptr ss:[ebp-74]
        //push edx
        //nop
        //call ...
        uchar bytes[] = {0x8D, 0x55, 0x98,
                        0x52,
                        0x8D, 0x55, 0xD4,
                        0x52,
                        0x8D, 0x55, 0xD8,
                        0x52,
                        0x8D, 0x55, 0xA0,
                        0x52,
                        0x8D, 0x55, 0xDC,
                        0x52,
                        0x8D, 0x55, 0x08,
                        0x52,
                        0x0F, 0xBF, 0x55, 0x8C,
                        0x52,
                        0x90, 0x90,
                        0xE8};
        vDataList.push_back( Data(0x41DF3B, 32, bytes) );

        //CALL address
        void* ptr = (void*)ScriptParser_CreateTrigger_ParseStatementIdx;
        DWORD address = (DWORD)ptr - 5  - 0x41DF5A;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x41DF5B, 4, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bTriggerKitFix) {
        //push eax
        //lea ecx, dword ptr ss:[ebp-5c]
        //call Trigger::GetI()
        //push eax
        //call ...
        uchar bytes[] = {0x50,
                        0x8D, 0x4D, 0xA4,
                        0xE8, 0xB6, 0xC0, 0x00, 0x00,
                        0x50,
                        0xE8};
        vDataList.push_back( Data(0x488331, 11, bytes) );

        //CALL address
        void* ptr = (void*)CCreatureObject_EvaluateTrigger_Kit;
        DWORD address = (DWORD)ptr - 5  - 0x48833B;
        uchar* bytes2 = (uchar*)&address;
        vDataList.push_back( Data(0x48833C, 4, bytes2) );
        
        //mov ecx, eax
        uchar bytes3[] = {0x8B, 0xC8,
                        0x90, 0x90, 0x90, 0x90, 0x90,
                        0x90, 0x90,
                        0x90, 0x90};
        vDataList.push_back( Data(0x488340, 11, bytes3) );

        COMMIT_vDataList;
    }

    //Quintuple number of kits allowable
    //Externalises KitArray outside CInfGame object
    ClassAbilityTable = new ClAbTable();
    if (pGameOptionsEx->bUserKitsExtend) {
        //for count
        uchar bytes[4] = {0x00, 0x05, 0x00, 0x00};
        vDataList.push_back( Data(0x6278AF, 4, bytes) );
        vDataList.push_back( Data(0x627A11, 4, bytes) );

        uchar bytes2[2] = {0x90, 0xB8};
        vDataList.push_back( Data(0x62796A, 2, bytes2) ); //EAX
        bytes2[1] = 0xBA;
        vDataList.push_back( Data(0x627993, 2, bytes2) ); //EDX
        vDataList.push_back( Data(0x627A3B, 2, bytes2) ); //EDX
        bytes2[1] = 0xB9;
        vDataList.push_back( Data(0x627A4E, 2, bytes2) ); //ECX

        //ptr to new ClassAbilityTable array
        uchar* bytes3 = reinterpret_cast<uchar*>(&ClassAbilityTable);
        vDataList.push_back( Data(0x62796C, 4, bytes3) );
        vDataList.push_back( Data(0x627995, 4, bytes3) );
        vDataList.push_back( Data(0x627A3D, 4, bytes3) );
        vDataList.push_back( Data(0x627A50, 4, bytes3) );

        uchar bytes4[7] = {0x89, 0x4C, 0x90, 0x04, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x627973, 7, bytes4) );

        uchar bytes5[7] = {0x8B, 0x4C, 0x8A, 0x04, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x627999, 7, bytes5) );

        uchar bytes6[8] = {0x83, 0x7C, 0x8A, 0x04, 0x00, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x627A41, 8, bytes6) );

        uchar bytes7[7] = {0x8B, 0x54, 0x81, 0x04, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x627A54, 7, bytes7) );

        //GetClassAbilityTable()
        uchar bytes8[1] = {0xBA};
        vDataList.push_back( Data(0x63A8DF, 1, bytes8) );

        //ptr to new ClassAbilityTable array
        uchar* bytes9 = reinterpret_cast<uchar*>(&ClassAbilityTable);
        vDataList.push_back( Data(0x63A8E0, 4, bytes9) );

        uchar bytes10[5] = {0x8B, 0x04, 0x8A, 0x90, 0x90};
        vDataList.push_back( Data(0x63A8E4, 5, bytes10) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUserContingencySelectSpell) {
        uchar bytes[] = {0x76}; //choice size (+8h)
        vDataList.push_back( Data(0x971B2E, 1, bytes) );
        vDataList.push_back( Data(0x971D3A, 1, bytes) ); //strangely, contingency info uses this too

        uchar bytes2[] = {0x70}; //selected size (+8h)
        vDataList.push_back( Data(0x971B97, 1, bytes2) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUserExternMageSpellHiding) {
        //Converts all const ResRef to NULL
        uchar bytes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        vDataList.push_back( Data(0xB363D8, 8, bytes) ); //SPWI402
        //vDataList.push_back( Data(0xB36440, 8, bytes) ); //SPWI402, see below
        vDataList.push_back( Data(0xB33E4C, 8, bytes) ); //SPWI802
        vDataList.push_back( Data(0xB33E54, 8, bytes) ); //SPWI402
        //vDataList.push_back( Data(0xB33E5C, 8, bytes) ); //SPWI124
        vDataList.push_back( Data(0xB2BE40, 8, bytes) ); //SPWI802
        vDataList.push_back( Data(0xB2BE48, 8, bytes) ); //SPWI920
        vDataList.push_back( Data(0xB2BE50, 8, bytes) ); //SPWI921
        vDataList.push_back( Data(0xB2BE58, 8, bytes) ); //SPWI922
        vDataList.push_back( Data(0xB2BE60, 8, bytes) ); //SPWI923
        vDataList.push_back( Data(0xB2BE68, 8, bytes) ); //SPWI924
        vDataList.push_back( Data(0xB2BE70, 8, bytes) ); //SPWI925
        vDataList.push_back( Data(0xB2BE78, 8, bytes) ); //SPWI926
        vDataList.push_back( Data(0xB2BE80, 8, bytes) ); //SPWI402
        //vDataList.push_back( Data(0xB2BE88, 8, bytes) ); //SPWI124

        //Allow NEXT/DONE button even if not all spell slots filled
        //One specialist mage spell still must be chosen
        uchar bytes2[9] = {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x72B2AE, 9, bytes2) );

        //Terminate SPLAUTOP iterations if the defaultVal is obtained
        //bytes[0] = 0x30;
        vDataList.push_back( Data(0xB36440, 8, bytes) ); //"SPWI402" -> "0"

        //JMP for termination
        uchar bytes3[4] = {0xAE, 0x00, 0x00, 0x00};
        vDataList.push_back( Data(0x72F7B6, 4, bytes3) );

        //Fix up the text box displaying 'Known Mage Spells'
        //MOV ECX, int PTR SS:[EBP+8]
        //PUSH ECX
        //NOP NOP NOP
        uchar bytes4[7] = {0x8B, 0x4D, 0x08, 0x51, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x719006, 7, bytes4) ); //Single player
        
        //MOV ECX, int PTR SS:[EBP-5C]
        bytes[2] = 0xA4;
        vDataList.push_back( Data(0x7759BE, 7, bytes4) ); //Multiplayer

        //CALL address (single player)
        void* ptr = (void*)CRuleTables_HasKnownMageSpells;
        DWORD address = (DWORD)ptr - 5  - 0x71900D;
        uchar* bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x71900E, 4, bytes5) );

        //CALL address (multiplayer)
        ptr = (void*)CRuleTables_HasKnownMageSpells;
        address = (DWORD)ptr - 5  - 0x7759C5;
        bytes5 = (uchar*)&address;
        vDataList.push_back( Data(0x7759C6, 4, bytes5) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUserExternRaceSelectionText) {
    
        //1. during loading of a game
        //2. CharGen buttons (loaded during game startup)
        //3. <RACE> token (loaded on dialogue)
        //4. Record Screen Label (loaded on record screen) - note: indices here are one above the others
        //5. IsGabber() action on CCreatureObject (loaded on action)
    
        //1, 3, 5. SWITCH 
        uchar bytes[1] = {0x9E};
        vDataList.push_back( Data(0x6A7BF7, 1, bytes) );
        vDataList.push_back( Data(0x7E419B, 1, bytes) );
        vDataList.push_back( Data(0x4B27CF, 1, bytes) );
    
        //1, 3, 4, 5. SWITCH 
        bytes[0] = 0x9F;
        vDataList.push_back( Data(0x6D8FA9, 1, bytes) );
    
        //4. DEC EDX
        bytes[0] = 0x4A;
        vDataList.push_back( Data(0x6D8FB2, 1, bytes) );
    
        //1-4. PUSH EDX, CALL
        uchar bytes2[] = {0x52, 0xE8};
        vDataList.push_back( Data(0x6A7C03, 2, bytes2) );
        vDataList.push_back( Data(0x73438C, 2, bytes2) );
        vDataList.push_back( Data(0x7E41A7, 2, bytes2) );
        vDataList.push_back( Data(0x6D8FB3, 2, bytes2) );
    
        //5. PUSH EAX, CALL
        bytes2[0] = 0x50;
        vDataList.push_back( Data(0x4B27DB, 2, bytes2) );
    
        //1-5. CALL address
        void* ptr = (void*)CInfGame_GetRaceText;
        DWORD address = (DWORD)ptr - 5  - 0x6A7C04;
        uchar* bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x6A7C05, 4, bytes3) );
        
        address = (DWORD)ptr - 5  - 0x73438D;
        bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x73438E, 4, bytes3) );
    
        address = (DWORD)ptr - 5  - 0x7E41A8;
        bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x7E41A9, 4, bytes3) );
    
        address = (DWORD)ptr - 5  - 0x6D8FB4;
        bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x6D8FB5, 4, bytes3) );
    
        address = (DWORD)ptr - 5  - 0x4B27DC;
        bytes3 = (uchar*)&address;
        vDataList.push_back( Data(0x4B27DD, 4, bytes3) );
    
        //1, 3. MOV LOCAL, EAX; JMP SHORT
        uchar bytes4[9] = {0x89, 0x85, 0x78, 0xFF, 0xFF, 0xFF, 0xEB, 0x6B, 0x90};
        vDataList.push_back( Data(0x6A7C09, 9, bytes4) );
        vDataList.push_back( Data(0x7E41AD, 9, bytes4) );
    
        //2. MOV LOCAL, EAX; JMP SHORT
        uchar bytes5[8] = {0x89, 0x45, 0x80, 0xEB, 0x64, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x734392, 8, bytes5) );
    
        //4. MOV LOCAL, EAX; JMP SHORT
        uchar bytes6[8] = {0x89, 0x45, 0xF0, 0x90, 0xEB, 0x7F, 0x90, 0x90};
        vDataList.push_back( Data(0x6D8FB9, 8, bytes6) );
    
        //5. MOV LOCAL, EAX; JMP SHORT
        uchar bytes7[9] = {0x89, 0x45, 0x80, 0xEB, 0x53, 0x90, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x4B27E1, 8, bytes7) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUserRecordMageSpellScroll) {
        //allow for loops for buttons to include new button
        uchar bytes[] = {0x18};
        vDataList.push_back( Data(0x6E0E1E, 1, bytes) );
        vDataList.push_back( Data(0x6E1229, 1, bytes) );
        vDataList.push_back( Data(0x6E4CCD, 1, bytes) );
        vDataList.push_back( Data(0x6E5722, 1, bytes) );
        vDataList.push_back( Data(0x6E9B08, 1, bytes) );

        //squelch AssertionFailedQuit() in case GUIMG.CHU is overwritten
        uchar bytes2[] = {0xEB, 0xD1};
        vDataList.push_back( Data(0x6E0E3F, 2, bytes2) );
        vDataList.push_back( Data(0x6E124A, 2, bytes2) );

        uchar bytes3[] = {0xEB, 0xD6};
        vDataList.push_back( Data(0x6E4CE9, 2, bytes3) );
        vDataList.push_back( Data(0x6E573E, 2, bytes3) );

        uchar bytes4[] = {0xEB, 0xDA};
        vDataList.push_back( Data(0x6E9B20, 2, bytes4) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUserFourInventorySlots) {
        //2->4 (All playable classes except Fighter and those listed for 3 slots), 3->4 (Paladin, Ranger, Monk)
        uchar bytes[1] = {0x04};
        vDataList.push_back( Data(0x639C6F, 1, bytes) );
        vDataList.push_back( Data(0x639C78, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bVideoEnableMorePaperdolls) {
        //change vfunc to that of CAnimation1000::GetAnimGroupPaperdollName
        uchar bytes[] = {0x55, 0x77, 0x83, 0x00}; //address 0x837755
        vDataList.push_back( Data(0xAB6B40, 4, bytes) ); //CAnimation1200
        vDataList.push_back( Data(0xAB6DEC, 4, bytes) ); //CAnimation2000
        vDataList.push_back( Data(0xAB6ED0, 4, bytes) ); //CAnimation3000

        //CAnimation7000 not having a default sPrefix
        //note the paperdoll BAM must be <prefix>0INV.BAM (note the 0)
        uchar bytes2[] = {0xEB, 0x42, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x81AE67, 5, bytes2) );

        uchar bytes3[] = {0xEB, 0x23, 0x90, 0x90, 0x90};
        vDataList.push_back( Data(0x81AE86, 5, bytes3) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bVideoIWDAnimAttack3Fix) {
        //ASCII "2" -> "3" in "A2E"
        uchar bytes[] = {0x33};
        vDataList.push_back( Data(0xB4F875, 1, bytes) );
    
        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bVideoSpellTurningAnimFix) {
        //mov edx,dword ptr ss:[ebp-2B4]
        //mov eax,dword ptr ds:[edx+6E]
        //and eax,4
        //test eax,eax
        //je 654D36
        //mov ecx,dword ptr ss:[ebp-34]
        //mov eax,dword ptr ds:[ecx+6450]
        //cmp eax,-1
        //jnz short 654CCA
        //nop
        uchar bytes[] = {0x8B, 0x95, 0x4C, 0xFD, 0xFF, 0xFF,
                        0x8B, 0x42, 0x6E,
                        0x83, 0xE0, 0x04,
                        0x85, 0xC0,
                        0x0F, 0x84, 0xA1, 0x00, 0x00, 0x00,
                        0x8B, 0x4D, 0xCC,
                        0x8B, 0x81, 0x50, 0x64, 0x00, 0x00,
                        0x83, 0xF8, 0xFF,
                        0x75, 0x27,
                        0x90};
        vDataList.push_back( Data(0x654C81, 35, bytes) );

        COMMIT_vDataList;
    }
    
    //Remove crash on alpha rendering of RLE-encoded BAMs
    if (pGameOptionsEx->bVideoVvcAlphaCrashFix) {
        //PUSH 0 to skip AssertionFailedQuit()
        uchar bytes[] = {0x00};
        vDataList.push_back( Data(0xA0F1EF, 1, bytes) );

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bVideoVvcShadowAnimFix) {
        //correct use of ResRef
        uchar bytes[] = {0x4E};
        vDataList.push_back( Data(0x64CE62, 1, bytes) );

        uchar bytes2[] = {0x4A};
        vDataList.push_back( Data(0x64CE66, 1, bytes2) );

        COMMIT_vDataList;
    }






















    //////////////////////////////////////////////////////////////////////////////////////////////
    // ContainerPanel
    if (pGameOptionsEx->bUI_LootPanel) {
        ContainerPanel_Init();  

        // Container panel new opcodes/offsets tables
        vDataList.push_back( Data(0x964B55 + 2,  4, (uchar *) &g_pButtonCreateOpcodes) );

        // Gamefield screen max Y-Coordinate when container panel opened
        vDataList.push_back( Data(0x7D90C1 + 2,  4, (uchar *) &g_pPointerYCoor) );


        // 0x93F919:
        // call ContainerPanel_FillListContainersAround_asm()   ; e8 xx xx xx xx (5bytes)
        // nop                                                  ; 90             (1bytes)
        // push edx                                             ; 52             (1bytes)
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 
                           0x52 };
        CallInject (0x93F919, ContainerPanel_CreateArrayOfContainersAround_asm, bytes1);


        // 0x7E6794: for button_redraw_action=2, Loot Buttons
        // call ContainerPanel_ButtonRedraw_asm                 ; e8 xx xx xx xx (5bytes)
        // jmp  near 0x7E66FF                                   ; E9 yy yy yy yy (5bytes)
        // nop                                                  ; 90             (1bytes)
        uchar bytes3[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0xE9, 0x61, 0xff, 0xff, 0xff,
                           0x90 };
        CallInject (0x7E6794, ContainerPanel_ButtonRedraw_asm, bytes3);


        // 0x7E619C: for button_click_action=02, Loot Buttons
        // and  ax, 7FFF                                        ; 66 25 FF 7F           (4bytes)
        // mov  [edx+0Ah], ax                                   ; 66 89 42 0A           (4bytes)
        // jmp  short 0x7E61B1                                  ; eb 0f                 (2bytes)
        // ...danger relocs, leave orig bytes...                ;                      (11bytes)
        // 0x7E61B1:                            
        // mov  dword ptr [ebp-60h], 0                          ; C7 45 A0 00 00 00 00  (7bytes)
        // jmp  near 0x7E5A95                                   ; E9 D8 F8 FF FF        (5bytes)
        // nop                                                  ; 90                    (1bytes)
        uchar bytes5[] = { 0x66, 0x25, 0xFF, 0x7F,
                           0x66, 0x89, 0x42, 0x0A,
                           0xEB, 0x0B,
                           0x00, 0x68, 0x7C, 0xAA, 0xB4, 0x00, 0x68, 0x84, 0xAA, 0xB4, 0x00,
                           0xc7, 0x45, 0xa0, 0x00, 0x00, 0x00, 0x00,
                           0xE9 ,0xD8, 0xF8, 0xFF, 0xFF,
                           0x90  };
        vDataList.push_back( Data(0x7E619C, sizeof(bytes5), bytes5) );


        // 0x7E5DC9: CreateMesageType2
        // call ContainerPanel_CreateMesageType2_asm            ; e8 xx xx xx xx (5bytes)
        // nop                                                  ; 90 90          (2bytes)
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject (0x7E5DC9, ContainerPanel_CreateMesageType2_asm, bytes6);


        // 0x69B056: CreateMesageType1
        // call ContainerPanel_CreateMesageType1_asm            ; e8 xx xx xx xx (5bytes)
        // nop nop                                              ; 90 90          (2bytes)
        uchar bytes8[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject (0x69B056, ContainerPanel_CreateMesageType1_asm, bytes8);
        
        
        // 0x7D983B: StopContainer
        // call ContainerPanel_StopContainer_asm                ; e8 xx xx xx xx (5bytes)
        // nop                                                  ; 90             (1bytes)
        uchar bytes12[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };
        CallInject (0x7D983B, ContainerPanel_StopContainer_asm, bytes12);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Green Icon on Learnable Scroll
    if (pGameOptionsEx->bUI_ColorizeLearnableScrolls) {

        // 0x7E6BB7: xxxContainer_DrawItem
        //  call    ItemRedraw_Container_asm                    ; e8 xx xx xx xx    (5bytes)
        //  cmp     al, 1                                       ; 3c 01             (2bytes)
        //  jz      near NoColorIcon (0x7E6D0B)                 ; 0F 84 47 01 00 00 (6bytes)
        //  push    ecx                                         ; 51                (1bytes)
        // 7E6BC5:
        //  ; red/green icon
        //  mov     ecx, [edx+6]                                ; 8b 4a             (2bytes)
        //  ...
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x3C, 0x01,
                           0x0F, 0x84, 0x47, 0x01, 0x00, 0x00,
                           0x51,
                           0x8B, 0x4A };
        CallInject (0x7E6BB7, ItemRedraw_Container_asm, bytes1);

        // push 0xC0 => nops
        uchar bytes2[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                         };
        vDataList.push_back( Data(0x7E6CCF, sizeof(bytes2), bytes2) );


        //0x7E6BF4: push    offset "STORTINT"
        vDataList.push_back( Data(0x7E6BF4 + 1,  4, (uchar *) &g_pPTRSTR_STORINT) );


        // 0x74B311: xxxInventory_DrawItem
        //  call    ItemRedraw_Inventory_asm                    ; e8 xx xx xx xx    (5bytes)
        //  cmp     al, 1                                       ; 3c 01             (2bytes)
        //  jz      near NoColorIcon (0x74B465)                 ; 0F 84 47 01 00 00 (6bytes)
        //  push    edx                                         ; 52                (1bytes)
        // 0x74B31F:
        //  ; red/green icon
        //  mov     edx, [ecx+6]
        //  ...
        uchar bytes3[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x3C, 0x01,
                           0x0F, 0x84, 0x47, 0x01, 0x00, 0x00,
                           0x52 };
        CallInject (0x74B311, ItemRedraw_Inventory_asm, bytes3);
        

        //0x74B34E: push    offset "STORTINT"
        vDataList.push_back( Data(0x74B34E + 1,  4, (uchar *) &g_pPTRSTR_STORINT) );


        // push 0xC0 => nops
        uchar bytes4[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                         };
        vDataList.push_back( Data(0x74B429, sizeof(bytes4), bytes4) );
        

        // [eax+640h]  [eax+63b]:
        //  0           1   red
        //  1           0   no
        //  1           1   green

        // 0x7AFE1B: xxxStore_DrawLeftStoreItem
        //  call    ItemRedraw_StoreLeft_asm                    ; e8 xx xx xx xx        (5bytes)
        //  mov     eax, [ebp-348h]                             ; 8B 85 B8 FC FF FF     (6bytes)
        //  cmp     byte ptr [eax+63bh], 0                      ; 80 B8 3B 06 00 00 00  (7bytes)
        //  jz      NoColorIcon                                 ; 0F 84 47 01 00 00     (6bytes)
        //  push    edx                                         ; 52                    (1bytes)
        // 0x7B0E08:
        //  mov     edx, [ecx+6]
        //  ...
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x8B, 0x85, 0xB8, 0xFC, 0xFF, 0xFF,
                           0x80, 0xB8, 0x3B, 0x06, 0x00, 0x00, 0x00,
                           0x0F, 0x84, 0x47, 0x01, 0x00, 0x00,
                           0x52 };
        CallInject (0x7AFE1B, ItemRedraw_StoreLeft_asm, bytes5);


        //0x7AFE63: push    offset "STORTINT"
        vDataList.push_back( Data(0x7AFE63 + 1,  4, (uchar *) &g_pPTRSTR_STORINT) );
        
        
        // push 0xC0 => nops
        uchar bytes6[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                         };
        vDataList.push_back( Data(0x7AFF3E, sizeof(bytes6), bytes6) );


        // 0x79E252 xxxStore_ConfigStoreItem
        // call     ItemRedraw_StoreLeftConfig_asm              ; e8 xx xx xx xx        (5bytes)
        // nop                                                  ; 90                    (1bytes)
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x79E252, ItemRedraw_StoreLeftConfig_asm, bytes7);


        // 0x7B0DEF: xxxStore_DrawInventoryItem
        //  call    ItemRedraw_StoreInventory_asm               ; e8 xx xx xx xx        (5bytes)
        //  mov     eax, [ebp-348h]                             ; 8B 85 B8 FC FF FF     (6bytes)
        //  cmp     byte ptr [eax+63bh], 0                      ; 80 B8 3B 06 00 00 00  (7bytes)
        //  jz      NoColorIcon                                 ; 0F 84 47 01 00 00     (6bytes)
        //  push    edx                                         ; 52                    (1bytes)
        // 0x7B0E08:
        //  mov     edx, [ecx+6]
        //  ...
        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x8B, 0x85, 0xB8, 0xFC, 0xFF, 0xFF,
                           0x80, 0xB8, 0x3B, 0x06, 0x00, 0x00, 0x00,
                           0x0F, 0x84, 0x47, 0x01, 0x00, 0x00,
                           0x52 };
         CallInject (0x7B0DEF, ItemRedraw_StoreInventory_asm, bytes9);


        //0x7B0E37: push    offset "STORTINT"
        vDataList.push_back( Data(0x7B0E37 + 1,  4, (uchar *) &g_pPTRSTR_STORINT) );


        // push 0xC0 => nops
        uchar bytes10[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                          };
        vDataList.push_back( Data(0x7B0F12, sizeof(bytes10), bytes10) );


        // 0x79E970 xxxStore_ConfigStoreItem
        // call     ItemRedraw_StoreLeftConfig_asm              ; e8 xx xx xx xx        (5bytes)
        // nop                                                  ; 90                    (1bytes)
        uchar bytes11[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };
        CallInject (0x79E970, ItemRedraw_StoreInventoryConfig_asm, bytes11);


        // 0x79F4A8 xxxStore_ConfigStoreItem
        // call     ItemRedraw_StoreDetectionConfig_asm          ; e8 xx xx xx xx        (5bytes)
        // nops                                                  ; 90                    (3bytes)
        uchar bytes12[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90};
        CallInject (0x79F4A8, ItemRedraw_StoreIdentifyConfig_asm, bytes12);

    
        // 0x7A04F2 xxxStore_ConfigStoreItem
        // call     ItemRedraw_StoreStealConfig_asm             ; e8 xx xx xx xx        (5bytes)
        // nops                                                 ; 90                    (1bytes)
        uchar bytes13[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90};
        CallInject (0x7A04F2, ItemRedraw_StoreStealConfig_asm, bytes13);


        // 0x7A06AF xxxStore_ConfigStoreItem
        // call     ItemRedraw_StoreStealInventoryConfig_asm    ; e8 xx xx xx xx        (5bytes)
        // nops                                                 ; 90                    (1bytes)
        uchar bytes14[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90};
        CallInject (0x7A06AF, ItemRedraw_StoreStealInventoryConfig_asm, bytes14);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Hide Write Scroll Button
    if (pGameOptionsEx->bUI_HideWriteScrollButton) {

        // 0x7458BF Enable "Write" Button on info screen
        // call     ItemRedraw_EnableWriteButton_asm           ; e8 xx xx xx xx        (5bytes)
        // nops                                                ; 90 90 90 90           (4bytes) 
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90 };
        CallInject (0x7458BF, ItemRedraw_EnableWriteButton_asm, bytes1);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // extended combat text fixes
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        // +Hand bonus:%d for main hand
        uchar bytes1[] = { 0xE6 };
        vDataList.push_back( Data(0x90CE81, sizeof(bytes1), bytes1) );

        // +Left:%d
        uchar bytes2[] = " +Left:%d ";
        vDataList.push_back( Data(0xB58990, sizeof(bytes2), bytes2) );

        // +Right:%d
        uchar bytes3[] = " +Right:%d ";
        vDataList.push_back( Data(0xB589A4, sizeof(bytes3), bytes3) );

        // +TwoWeapons
        uchar bytes4[]  = " +TwoWeapons -6";
        vDataList.push_back( Data(0xB5795C, sizeof(bytes4), bytes4) );

        uchar bytes5[]  = " +TwoWeapons -2";
        vDataList.push_back( Data(0xB5796C, sizeof(bytes5), bytes5) );

        uchar bytes6[]  = " +TwoWeapons -4";
        vDataList.push_back( Data(0xB5797C, sizeof(bytes6), bytes6) );

        uchar bytes7[]  = " +TwoWeapons -0";
        vDataList.push_back( Data(0xB5798C, sizeof(bytes7), bytes7) );

        uchar bytes8[]  = " +TwoWeapons -2";
        vDataList.push_back( Data(0xB5799C, sizeof(bytes8), bytes8) );

        uchar bytes9[]  = " +TwoWeapons -0";
        vDataList.push_back( Data(0xB579AC, sizeof(bytes9), bytes9) );

        uchar bytes10[] = " +TwoWeapons -8";
        vDataList.push_back( Data(0xB579BC, sizeof(bytes10), bytes10) );

        uchar bytes11[] = " +TwoWeapons -4";
        vDataList.push_back( Data(0xB579CC, sizeof(bytes11), bytes11) );

        uchar bytes12[] = {'+'};
        vDataList.push_back( Data(0xB58AF5, sizeof(bytes12), bytes12) );

        // "+SpecialAC:%d " -> "+DamageTypeACMod:%d"
        uchar* ptr = DamageTypeACModText;
        vDataList.push_back( Data(0x90A41F +1, 4, (uchar*) &ptr) );
        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // clear "unstealable" bit when putting item to Bag to allow stacking,
    // engine itself clear bit when put items from Bag to Inventory
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        CallInject (0x69F9DD, DragToBag_asm, bytes1);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // replace bugged CStore::GetNumItems() when enumerating inside bag/store
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        // CInfGame::FindItemInStore
        // Apply only if tobex's original Optimise Bag Search Code is disabled
        if (pGameOptionsEx->bTriggerOptimiseBagSearch == FALSE) {
            RelativeInject (0x68F80E, CStore__GetNumItems_asm);
            RelativeInject (0x68F569, CStore__GetNumItems_asm);
        }

        // CInfGame::TakeItemFromStore
        // CInfGame::GetItemFromStore
        // CInfGame::DrainItemInStore
        RelativeInject (0x690EBD, CStore__GetNumItems_asm);
        RelativeInject (0x69167A, CStore__GetNumItems_asm);
        RelativeInject (0x6904E5, CStore__GetNumItems_asm);
        RelativeInject (0x690B4C, CStore__GetNumItems_asm);
        RelativeInject (0x68FAD2, CStore__GetNumItems_asm);
        RelativeInject (0x690179, CStore__GetNumItems_asm);

        // CInfGame::AddItemToStore  ?
        // 0x68EC16
        // 0x68F1D2 

        // CScreenStore::ButtonConfig ?
        // 0x79DCA8

        // ???
        // 0x7A457F
        // 0x7A588D

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // Infinity Animation patches:
    //  Remove hardcoded check for mage animation X2XX, usefull with BG1 Mage
    //  BG1+BG2 Mage Armor Sound
    //  0x10XX Ready Sound
    //  0xA0XX Ready Sound
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        // call InventoryScreen_CheckMageAnimation_asm 
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject (0x5AAF96, CItem_TranslateAnimationType_CheckMageAnimation_asm, bytes6);
        CallInject (0x5AAFC0, CItem_TranslateAnimationType_CheckMageAnimation_asm, bytes6);

        // GetSndArmor
        // 5xxx         ARM_0*      Not MAGE(x2xx), Not Robe(?)
        // 6xxx         ARM_0*      Not MAGE(x2xx), Not Robe(?) (IA add L_MAGEs at 6430-6435)
        CallInject (0x87029B, CAnimation6400_SetArmorSound_asm, bytes6);
        CallInject (0x851CBD, CAnimation5000_SetArmorSound_asm, bytes6);

        // GetSndReady
        // 10xx         WAL_77*     WYVERN_BIG
        // 7F03, 7F2D   WAL_PS      IMP, WYVERN_FAMILIAR
        // A0xx         WAL_77*     WYVERN
        CallInject (0x837875, CAnimation1000_SetReadySound_asm, bytes6);
        CallInject (0x82B168, CAnimationA000_SetReadySound_asm, bytes6);

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bBG1AnimationOffhandWeapon) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x86705D, CAnimation6400_EquipWeapon_SkipOffHand_asm, bytes6);
        CallInject(0x86FE5F, CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm, bytes7);
        CallInject(0x87007B, CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm, bytes7);
        CallInject(0x872F7F, CAnimation6400_SetColorRange_CheckOffHandWeapon_asm, bytes7); 

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // BG1 Type animation dual-class enable
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject(0x6F08B4, ScreenCharacter_MakeDualClass_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Green Highlight Stats on Inventory Screen
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        RelativeInject(0x6E6045, HighlightLabel_asm);
        RelativeInject(0x6E609A, HighlightLabel_asm);
        RelativeInject(0x6E6234, HighlightLabel_asm);
        RelativeInject(0x6E6262, HighlightLabel_asm);
        RelativeInject(0x6E6290, HighlightLabel_asm);
        RelativeInject(0x6E62BE, HighlightLabel_asm);
        RelativeInject(0x6E62EC, HighlightLabel_asm);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // fix creating/assign CString from not null-ended ResRef::GetResRef()
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        RelativeInject(0x4896CE, GetResRefNulled_asm);
        RelativeInject(0x4896F1, GetResRefNulled_asm);
        RelativeInject(0x48976D, GetResRefNulled_asm);
        RelativeInject(0x489790, GetResRefNulled_asm);
        //RelativeInject(0x489942, GetResRefNulled_asm);  see AreaCheckObject() fix
        //RelativeInject(0x489969, GetResRefNulled_asm);
        RelativeInject(0x491AD0, GetResRefNulled_asm);
        RelativeInject(0x4AC0A2, GetResRefNulled_asm);
        RelativeInject(0x4C2F4A, GetResRefNulled_asm);
        RelativeInject(0x4C3026, GetResRefNulled_asm);
        RelativeInject(0x4C4833, GetResRefNulled_asm);
        RelativeInject(0x4C5EFA, GetResRefNulled_asm);
        RelativeInject(0x4C86F4, GetResRefNulled_asm);
        RelativeInject(0x4D2D0D, GetResRefNulled_asm);
        RelativeInject(0x4D78FA, GetResRefNulled_asm);
        RelativeInject(0x4D7A3C, GetResRefNulled_asm);
        RelativeInject(0x4D7ACC, GetResRefNulled_asm);
        RelativeInject(0x4D7D80, GetResRefNulled_asm);
        RelativeInject(0x4D80AB, GetResRefNulled_asm);
        RelativeInject(0x4D9118, GetResRefNulled_asm);
        RelativeInject(0x4D91C5, GetResRefNulled_asm);
        RelativeInject(0x53CCFE, GetResRefNulled_asm);
        RelativeInject(0x576431, GetResRefNulled_asm);
        RelativeInject(0x6B8F6A, GetResRefNulled_asm);
        RelativeInject(0x7B9F03, GetResRefNulled_asm);
        RelativeInject(0x7B9F41, GetResRefNulled_asm);
        RelativeInject(0x7B9F7F, GetResRefNulled_asm);
        RelativeInject(0x7BC9B6, GetResRefNulled_asm);
        RelativeInject(0x7BC9F7, GetResRefNulled_asm);
        RelativeInject(0x7BCA38, GetResRefNulled_asm);
        RelativeInject(0x7C0D8A, GetResRefNulled_asm);
        RelativeInject(0x7C1380, GetResRefNulled_asm);
        RelativeInject(0x7C1475, GetResRefNulled_asm);
        RelativeInject(0x88BAAE, GetResRefNulled_asm);
        RelativeInject(0x8D6237, GetResRefNulled_asm);
        RelativeInject(0x91BA19, GetResRefNulled_asm);

        // _mbscmp()
        RelativeInject(0x4D884C, GetResRefNulled_asm);
        RelativeInject(0x4D8C29, GetResRefNulled_asm);

        // CString::operator=(char *)
        RelativeInject(0x8CB6E6, GetCharPtrNulled_asm);
        RelativeInject(0x8CB569, GetCharPtrNulled_asm);
        RelativeInject(0x8CB3D4, GetCharPtrNulled_asm);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    // Cre.m_BaseStats.XXX => unsigned char fatigue,intoxication,morale
    // uchar math -> signed char math

    // morale		 0-20
    // fatigue		 0-100
    // intoxication	 0-100

    if (pGameOptionsEx->bEffFatigueModFix ||
        pGameOptionsEx->bEffIntoxicationModFix ||
        pGameOptionsEx->bEffMoraleBreakModFix) {
        // 33 C0 8A 82 XX XX XX XX -> 90 0F BE 82 XX XX XX XX
        // xor  eax,eax            -> nop
        // mov  al, byte [edx+XX]  -> movsx eax, byte [edx+XX]

        // 33 C9 8A 88 XX XX XX XX -> 90 0F BE 88 XX XX XX XX
        // xor  ecx, ecx           -> nop
        // mov  cl, byte [eax+XX]  -> movsx ecx, byte [eax+XX]

        // 33 D2 8A 91 XX XX XX XX -> 90 0F BE 91 XX XX XX XX
        // xor  edx, edx           -> nop
        // mov  dl, byte [ecx+XX]  -> movsx edx, byte [ecx+XX]

        // 33 D2 8A 90 XX XX XX XX -> 90 0F BE 90 XX XX XX XX
        // xor  edx, edx           -> nop
        // mov  dl, byte [eax+XX]  -> movsx edx, byte [eax+XX]

        uchar bytes_movsx[] = { 0x90, 0x0F, 0xBE };

        //detoured CEffectFatigueMod, CEffectMoraleBreakMod, CEffectIntoxicationMod already have fix

        //BaseStats.morale
        vDataList.push_back( Data(0x481AB9, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x481BCA, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x481CB7, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.reputation
        vDataList.push_back( Data(0x482402, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x482526, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x48264A, sizeof(bytes_movsx), bytes_movsx) );

        /* CEffectResetMorale, not necessary
        vDataList.push_back( Data(0x512D0E, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512D28, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512D62, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512D7C, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512DA7, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512DCD, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x512DE7, sizeof(bytes_movsx), bytes_movsx) );
        */

        //BaseStats.intoxication
        vDataList.push_back( Data(0x7A8E6F, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.intoxication
        vDataList.push_back( Data(0x895BCD, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.morale
        vDataList.push_back( Data(0x895CEA, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x895CFD, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x895D45, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x895D58, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.morale
        vDataList.push_back( Data(0x898227, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8994EF, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.morale
        vDataList.push_back( Data(0x8CF91A, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CF92A, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CF94D, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CF95C, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CFA6C, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CFA92, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8CFAEF, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.fatigue
        vDataList.push_back( Data(0x8CFCC5, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8D01AE, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8D0294, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.morale
        vDataList.push_back( Data(0x8E090E, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0921, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0953, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0965, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E09FE, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0A11, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0A43, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0A55, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E0AC4, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0AD7, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0B09, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0B1B, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E0CF1, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0D04, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0D36, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0D48, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E0EFE, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0F11, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0F43, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E0F55, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E112A, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E113D, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E116F, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E1181, sizeof(bytes_movsx), bytes_movsx) );

        vDataList.push_back( Data(0x8E11EF, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E1202, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E1234, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8E1246, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.intoxication
        vDataList.push_back( Data(0x8EF27F, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8EF2BB, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8EF2D3, sizeof(bytes_movsx), bytes_movsx) );

        //BaseStats.morale
        vDataList.push_back( Data(0x8EF552, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8EF565, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8EF597, sizeof(bytes_movsx), bytes_movsx) );
        vDataList.push_back( Data(0x8EF5A9, sizeof(bytes_movsx), bytes_movsx) );

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // THAC0 & Damage Info
    if (pGameOptionsEx->bUI_WeaponInfoOnInventoryScreen) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject (0x7427EE, InventoryScreen_LabelText_asm, bytes7);
        CallInject (0x74080F, InventoryScreen_SetTooltips_asm, bytes5);

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bUI_WeaponInfoOnRecordScreen) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject (0x6E6B7C, CScreenCharacter_UpdateMainPanel_AddDamageOneHand_asm, bytes6);
        CallInject (0x6E6B0C, CScreenCharacter_UpdateMainPanel_AddDamageTwoHand_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Change portrait when dragging item
    if (pGameOptionsEx->bUI_DarkPortraitsIfItemNotAllowed) {

        // 0x6A020D 
        // call     InventoryScreen_DragInventoryArea_asm           ; e8 xx xx xx xx        (5bytes)
        // nops                                                     ; 90                    (5bytes) 
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90 };
        CallInject (0x6A020D, InventoryScreen_DragInventoryArea_asm, bytes1);

        // 0x69FA02 
        // call     InventoryScreen_DragInventoryAreaOnBag_asm      ; e8 xx xx xx xx        (5bytes)
        uchar bytes2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        CallInject (0x69FA02, InventoryScreen_DragInventoryAreaOnBag_asm, bytes2);


        // 0x69B3B2 
        // call     InventoryScreen_DragContainerArea_asm           ; e8 xx xx xx xx        (5bytes)
        // nop                                                      ; 90                    (1bytes) 
        uchar bytes3[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x69B3B2, InventoryScreen_DragContainerArea_asm, bytes3);


        // 0x69DC4D
        // call     InventoryScreen_DragBodyArea_asm                ; e8 xx xx xx xx        (5bytes)
        // nop                                                      ; 90                    (1bytes) 
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x69DC4D, InventoryScreen_DragBodyArea_asm, bytes5);
 

        // 0x8AA6A1
        // call     InventoryScreen_PortraitRedraw_asm              ; e8 xx xx xx xx        (5bytes)
        // nop                                                      ; 90                    (1bytes) 
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x8AA6A1, InventoryScreen_PortraitRedraw_asm, bytes7);

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Show Total Roll when creating new char
    if (pGameOptionsEx->bUI_TotalRollInfo) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x71CB16, CScreenCreateChar_ResetAbilities_asm, bytes6);
        CallInject (0x731D4F, RollScreen_RecallButtonClick_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Interactive Journal
    if (pGameOptionsEx->bUI_FoldJournal) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        RelativeInject (0x56FEBA, InteractiveJournal_QuestEntryAdd_asm);
        RelativeInject (0x5711A7, InteractiveJournal_ClearText_asm);
        CallInject (0x759EB0,   InteractiveJournal_CheckClickedNode_asm, bytes5);
        CallInject (0x7581F3,   InteractiveJournal_SortingInfoLabel_asm, bytes7);
        RelativeInject (0x7583D9, InteractiveJournal_FillText_asm);

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Grey background on Pause
    if (pGameOptionsEx->bUI_GreyBackgroundOnPause) {

        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,         
                           0x90 };
        CallInject (0x6C870A, GreyBackground_asm, bytes1);
        
        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Unlimit Hand Off Slot
    if (pGameOptionsEx->bUI_UnlimitedHandOffSlot) {

        // remove original checks
        uchar bytes_JMP[] = { 0xEB };

        // Two_Handed_Weapon_Equipped 
        vDataList.push_back( Data(0x69C2AC,  sizeof(bytes_JMP), bytes_JMP) );
        //vDataList.push_back( Data(0x69EE7D,  sizeof(bytes_JMP), bytes_JMP) );

        // Off_in_use
        vDataList.push_back( Data(0x69C758,  sizeof(bytes_JMP), bytes_JMP) );
        //vDataList.push_back( Data(0x69F317,  sizeof(bytes_JMP), bytes_JMP) );

        // OffWeapon_after_Ranged
        vDataList.push_back( Data(0x69C33D,  sizeof(bytes_JMP), bytes_JMP) );

        // Ranged_after_OffWeapon
        vDataList.push_back( Data(0x69C699,  sizeof(bytes_JMP), bytes_JMP) );
        //vDataList.push_back( Data(0x69F25B,  sizeof(bytes_JMP), bytes_JMP) );

        // OffWeapon_after_Launcher
        vDataList.push_back( Data(0x69C361,  sizeof(bytes_JMP), bytes_JMP) );

        // Launcher_after_OffWeapon 
        vDataList.push_back( Data(0x69C5D1,  sizeof(bytes_JMP), bytes_JMP) );

        
        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x6E6A2D, ScreenRecordThacInfo_asm, bytes10);

        uchar bytes12[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x905512, ApplyDamage_asm, bytes12);

        uchar bytes14[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x909B87, CCreatureObject__Hit_asm, bytes14);

        uchar bytes16[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x90C8E1, CombatInfo_asm, bytes16);

        uchar bytes18[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x951187, WeaponStyleBonuses_asm, bytes18);

        uchar bytes20[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        CallInject (0x951260, WeaponStyleBonuses2_asm, bytes20);
        
        uchar bytes22[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x8CDE43, ApplyLevelProgressTableSub_asm, bytes22);
        
        uchar bytes24[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        CallInject (0x8CE01E, ApplyLevelProgressTableSub2_asm, bytes24);


        uchar bytes26[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        CallInject (0x8C2B6F, Sub_8C29E6_asm, bytes26);

        uchar bytes28[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        CallInject (0x8C515A, Sub_8C4E09_asm, bytes28);

        uchar bytes30[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                          };
        CallInject (0x94F562, EquipRanged_asm, bytes30);

        uchar bytes32[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        CallInject (0x94F57D, EquipRanged2_asm, bytes32);

        // Cre.u6753 init
        uchar bytes34[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        CallInject (0x882436, CCreatureObject_CCreatureObject_InitU6753_asm, bytes34);
        
        // 0x8987AC dumplog, need ?
        // 0x899B6D dumplog, need ?

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // LogActiveBuffs
    if (pGameOptionsEx->bUI_LogActiveBuffs) {

        LogActiveBuffs_Init();
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,         
                         };

        CallInject (0x4CEDD1, CreatureRMClick_asm, bytes1);
        
        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // Enhanced Text Output
    if (pGameOptionsEx->bUI_ExtendedEventText) {

        uchar bytes_call[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,         
                             };

        //journal has updated
        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, // call  JournalEvent_asm
                           0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90 };

        uchar bytes2[] = { 0x66, 0x8B, 0x42, 0x0C, 0x90              // mov     ax, [edx+0Ch] ; journalmode
                         };

        uchar bytes3[] = { 0x50, 0x8B, 0x02, 0x8B, 0x89, 0xBA,       // push    eax
                           0x42, 0x00, 0x00, 0x50, 0x90, 0x90, 0x90, // mov     eax, [edx]
                           0x66, 0x8B, 0x05                          // mov     ecx, [ecx+42BAh];
                         };                                          // push    eax
                                                                     // mov     ax, ds:Mode_JournalUpdate

        uchar bytes4[] = { 0x50, 0xE8, 0x6D, 0xDF, 0x13,             // push    eax
                           0x00, 0x58, 0x90, 0x90                    // call    TextDispatch
                         };                                          // pop     eax

        uchar byt2_2[] = { 0x66, 0x8B, 0x42, 0x0C, 0x90, 0x90,       // mov     ax, [edx+0Ch] ; journalmode
                           0x90, 0x90 };

        uchar byt3_2[] = { 0x50, 0x8B, 0x02, 0x8B, 0x89, 0xBA,       // push    eax
                           0x42, 0x00, 0x00, 0x90, 0x90, 0x90,       // mov     eax, [edx]
                           0x90, 0x90, 0x90, 0x90, 0x90, 0x90,       // mov     ecx, [ecx+42BAh];
                           0x50, 0x66, 0x8B, 0x05                    // push    eax
                         };                                          // mov     ax, ds:Mode_JournalUpdate

        uchar byt4_2[] = { 0x50, 0xE8, 0xC8, 0xD6, 0x13, 0x00, 0x58, // push    eax
                           0x90, 0x90, 0x90, 0x90, 0x90              // call    TextDispatch
                         };                                          // pop     eax

        vDataList.push_back( Data(0x570547, sizeof(bytes2), bytes2) );
        vDataList.push_back( Data(0x570552, sizeof(bytes3), bytes3) );
        vDataList.push_back( Data(0x570566, sizeof(bytes4), bytes4) );
        // untested
        //vDataList.push_back( Data(0x570DE3, sizeof(byt2_2), byt2_2) );
        //vDataList.push_back( Data(0x570DF1, sizeof(byt3_2), byt3_2) );
        //vDataList.push_back( Data(0x570E0B, sizeof(byt4_2), byt4_2) );
        CallInject (0x6AF9E5, JournalEvent_asm, bytes1);


        // gain an item
        uchar bytes5[] = { 0xA8, 0x44, 0x00, 0x00 };                 // 44A8 ~The Party Has Gained An Item~
        uchar bytes6[] = { 0x8B, 0x45, 0x0C, 0x90, 0x90, 0x90,       // mov  eax, [ebp+InputID]
                           0x89, 0x42, 0x10, 0x90, 0x90, 0x90, 0x90  // mov  [edx+10h], eax
                         };
        uchar bytes7[] = { 0xC7, 0x41, 0x18, 0xFF, 0xC8, 0x96, 0x00  // mov  [ecx+18h], 96C8FFh ; colors
                         };
        uchar bytes8[] = { 0x14 };
        vDataList.push_back( Data(0x6AEE14, sizeof(bytes5), bytes5) );
        vDataList.push_back( Data(0x6AEE18, sizeof(bytes6), bytes6) );
        vDataList.push_back( Data(0x6AEE2B, sizeof(bytes7), bytes7) );
        vDataList.push_back( Data(0x6AEE3D, sizeof(bytes8), bytes8) );

        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x50, 0x90, // call GainEvent_asm
                           0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90  // push eax
                         };
        uchar byt9_2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x50, 0x90, // call GainEvent3_asm
                           0x90, 0x90, 0x90, 0x90                    // push eax
                         };
        vDataList.push_back( Data(0x4A72D2, sizeof(bytes9), bytes9) );
        CallInject (0x4A72D2, GainItemEvent_asm,  bytes_call);

        vDataList.push_back( Data(0x94211D, sizeof(byt9_2), byt9_2) );
        CallInject (0x94211D, GainItemEvent2_asm, bytes_call);

        vDataList.push_back( Data(0x94248F, sizeof(byt9_2), byt9_2) );
        CallInject (0x94248F, GainItemEvent3_asm, bytes_call);

        vDataList.push_back( Data(0x4A7448, sizeof(bytes9), bytes9) );
        CallInject (0x4A7448, GainItemEvent4_asm, bytes_call);


        // lost an item
        uchar bytes10[] = { 0xA9, 0x44, 0x00, 0x00 };                    // 44A9 ~The Party Has Lost An Item~
        uchar bytes11[] = { 0x8B, 0x45, 0x0C, 0x90, 0x90, 0x90,          // mov  eax, [ebp+InputID]
                            0x89, 0x42, 0x10, 0x90, 0x90, 0x90, 0x90     // mov  [edx+10h], eax
                          };
        uchar bytes12[] = { 0xC7, 0x41, 0x18, 0xFF, 0xC8, 0x96, 0x00     // mov  [ecx+18h], 96C8FFh ; colors
                          };
        uchar bytes13[] = { 0x14 };
        uchar byt13_1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };

        vDataList.push_back( Data(0x6AF08C, sizeof(bytes10), bytes10) );
        vDataList.push_back( Data(0x6AF090, sizeof(bytes11), bytes11) );
        vDataList.push_back( Data(0x6AF0A3, sizeof(bytes12), bytes12) );
        vDataList.push_back( Data(0x6AF0B5, sizeof(bytes13), bytes13) );

        uchar bytes14[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x50, 0x90, // call LostEvent_asm
                            0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90  // push eax
                          };

        CallInject (0x4A68A7, LostItemEvent3_asm, byt13_1);

        vDataList.push_back( Data(0x4A68C7, sizeof(bytes14), bytes14) );
        CallInject (0x4A68C7, LostItemEvent_asm,  bytes_call);

        vDataList.push_back( Data(0x4A7421, sizeof(bytes14), bytes14) );
        CallInject (0x4A7421, LostItemEvent2_asm,  bytes_call);


        //area_revealed
        uchar bytes15[] = { 0x60, 0x2C, 0x00, 0x00 };                    // 2C60 ~Your world map has been updated.~
        uchar bytes16[] = { 0x8B, 0x45, 0x0C, 0x90, 0x90, 0x90,          // mov  eax, [ebp+InputID]
                            0x89, 0x41, 0x10, 0x90, 0x90, 0x90, 0x90     // mov  [ecx+10h], eax
                          };
        uchar bytes17[] = { 0xC7, 0x41, 0x18, 0xFF, 0xC8, 0x96, 0x00,    // // mov  [ecx+18h], 96C8FFh ; colors
                          };
        uchar bytes18[] = { 0x14 };
        vDataList.push_back( Data(0x6AFE05, sizeof(bytes15), bytes15) );
        vDataList.push_back( Data(0x6AFE0F, sizeof(bytes16), bytes16) );
        vDataList.push_back( Data(0x6AFE1C, sizeof(bytes17), bytes17) );
        vDataList.push_back( Data(0x6AFE2E, sizeof(bytes18), bytes18) );

        vDataList.push_back( Data(0x4AA483, sizeof(byt9_2), byt9_2) );
        CallInject (0x4AA483, RevealMapEvent_asm,  bytes_call);
        

        // drop_ground_item
        uchar bytes19[] = { 0xCF, 0x2A, 0x00, 0x00 };                 // 2ACF ~Inventory Full: Item Dropped on Ground~
        uchar bytes20[] = { 0x8B, 0x45, 0x0C, 0x90, 0x90, 0x90,       // mov  eax, [ebp+InputID]
                            0x89, 0x42, 0x10, 0x90, 0x90, 0x90, 0x90  // mov  [edx+10h], eax
                          };
        uchar bytes21[] = { 0xC7, 0x41, 0x18, 0xFF, 0xC8, 0x96, 0x00  // mov  [ecx+18h], 96C8FFh ; colors
                          };
        uchar bytes22[] = { 0x14 };
        vDataList.push_back( Data(0x6AEF50, sizeof(bytes19), bytes19) );
        vDataList.push_back( Data(0x6AEF54, sizeof(bytes20), bytes20) );
        vDataList.push_back( Data(0x6AEF67, sizeof(bytes21), bytes21) );
        vDataList.push_back( Data(0x6AEF79, sizeof(bytes22), bytes22) );

        vDataList.push_back( Data(0x5BC7ED, sizeof(bytes9), bytes9) );
        CallInject (0x5BC7ED, DropItemEvent1_asm,  bytes_call);

        uchar bytes23[] = { 0xEB, 0x1B };   // remove first message
        vDataList.push_back( Data(0x5BC759, sizeof(bytes23), bytes23) );

        vDataList.push_back( Data(0x5E652F, sizeof(bytes9), bytes9) );
        CallInject (0x5E652F, DropItemEvent2_asm,  bytes_call);

        uchar bytes24[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90,
                            0x90, 0x90 };

        // Spell Failed: Action Change
        if (pGameOptionsEx->bActionExpandedActions == FALSE) {  // if need separate inject
            CallInject(0x47848B, CGameAIBase_ClearActions_CheckInterrupt_asm, bytes24);
        }

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Combat Extended Text Type
    if (pGameOptionsEx->bUI_CombatExtendedTextType) {

        uchar bytes2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject (0x508900, TYPE_damage_asm,   bytes2);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Combat Extended Text Full
    if (pGameOptionsEx->bUI_CombatExtendedTextFull) {

        uchar bytes1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0xEB, 0x22 };
        uchar bytes2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject  (0x90BC5D, ThacRollMessage_asm, bytes1);
        RelativeInject(0x8FD2DD, ThacRollMessage2_asm);
        CallInject  (0x508B82, AMOUNT_damage_asm, bytes2);
        // See also DETOUR_CEffect::DETOUR_CheckSave()

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Don't purge unavailable spells slots when drained
    if (pGameOptionsEx->bUI_KeepDrainedSpellSlots) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes1[] = { 0xEB };

        CallInject(0x5382C0, KeepPriestSlotsWhenDrained_asm, bytes6);
        CallInject(0x5383F8, KeepMageSlotsWhenDrained_asm,   bytes6);
        CallInject(0x8D5F76, RemoveSpellsPriest_asm,     bytes6);

        // disable "used spell" priority
        vDataList.push_back( Data(0x8D5E6F, sizeof(bytes1), bytes1) );

        CallInject(0x8D5D49, RemoveSpellsMage_asm,       bytes6);
        // disable "used spell" priority
        vDataList.push_back( Data(0x8D5C30, sizeof(bytes1), bytes1) );

        CallInject(0x53A47D, EffectRestorationApply_asm, bytes5);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Fix non-school bonuses
    if (pGameOptionsEx->bEffSavingThrowFix) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x5AB300, NonSchoolBonuses_asm, bytes6);
        CallInject(0x643453, NonSchoolBonuses_asm, bytes6);
        // 0x502166  - DETOUR_CEffect::DETOUR_CheckSave already has fix

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // int16->int32 Wild Mage Kit Fix
    // Fixes school to Generalist
    if (pGameOptionsEx->bActionAddKitFix ||
        pGameOptionsEx->bTriggerKitFix) {

        uchar bytes71[] = { 0x8B, 0x91, 0x32, 0x06, 0x00, 0x00,   // mov     edx, [ecx+632h]
                            0x90 };
        uchar bytes72[] = { 0x8B, 0x88, 0x32, 0x06, 0x00, 0x00,   // mov     ecx, [eax+632h]
                            0x90 };
        uchar bytes73[] = { 0x8B, 0x82, 0x32, 0x06, 0x00, 0x00,   // mov     eax, [edx+632h]
                            0x90 };

        vDataList.push_back( Data(0x5AB2EB, sizeof(bytes71), bytes71) );
        vDataList.push_back( Data(0x64343E, sizeof(bytes71), bytes71) );
        vDataList.push_back( Data(0x52C47F, sizeof(bytes72), bytes72) );
        vDataList.push_back( Data(0x50214E, sizeof(bytes73), bytes73) );

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Show HP/Action on Portrait
    if (pGameOptionsEx->bUI_ShowHitPointsOnPortrait ||
        pGameOptionsEx->bUI_ShowActionOnPortrait) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x8AACFB, RenderPortrait_asm, bytes6);

        if (!(pGameOptionsEx->bUI_ShowHitPointsOnPortrait_Always &&
              pGameOptionsEx->bUI_ShowActionOnPortrait_Always)) {
            CallInject(0x9A45EB, TabPressed_asm, bytes6);           // not always
        }

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Disarm trap at safe distance
    if (pGameOptionsEx->bDisarmTrapDistanceFix) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        CallInject(0x9447C4, DisarmAction_asm, bytes5);

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Legacy of Bhaal difficulty mode
    if (pGameOptionsEx->bUI_NightmareMode) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes8[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90 };
        if (pGameOptionsEx->bEngineExpandedStats == FALSE) // need separate inject
            CallInject  (0x46DC22, CDerivedStats_Reload_Nightmare_asm, bytes8);

        CallInject  (0x8BB44E, CCreatureObject_Unmarshal_asm, bytes6);
        //CallInject  (0x684C68, CInfGame_Unmarshal_asm, bytes6);
        //CallInject  (0x683EC3, CInfGame_Marshal_asm, bytes6);
        CallInject  (0x67CEBF, DestroyGame_asm, bytes6);
        CallInject  (0x780167, GameplayOptions_Init_asm, bytes7);

        RelativeInject(0x8E92BD, CheckMorale_asm);
        RelativeInject(0x717D7A, SetButtonStatus_asm);
        CallInject  (0x4C3657, CGameArea_CheckRestEncounter_asm, bytes7);

        COMMIT_vDataList;
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Continuous battle song
    if (pGameOptionsEx->bContinuousBattleMusic) {

        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90 };

        CallInject(0x4CC65D, CGameArea_OnActivation_SetSongTypeDay_asm, bytes9);
        CallInject(0x4CC6CC, CGameArea_OnActivation_SetSongTypeNight_asm, bytes9);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Start New Game With BG1 Animation
    if (pGameOptionsEx->bUI_StartNewGameWithBG1Animation) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject(0x723D5A, ScreenCreateChar_CompleteCharacterClass_asm, bytes7);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Shaman Class
    if (pGameOptionsEx->bEngineShamanClass) {

        uchar bytes5[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                          };
        uchar bytes6[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };
        uchar bytes7[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        uchar bytes8[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90 };
        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar buttonid[] = { 18 };

        RelativeInject(0x62B998, CRuleTables_GetClassString_asm);
        CallInject(0x635DC3,   CRuleTables_GetClassStringLower_asm, bytes5);
        CallInject(0x636AD1,   CRuleTables_GetClassStringMixed_asm, bytes10);
        CallInject(0x734085,   ButtonCharGenClassSelection_GetClassFromButtonIndex_asm, bytes5);
        CallInject(0x733DBF,   ButtonCharGenClassSelection_OnLButtonClick_SetKitDescriptionSTRREF_asm, bytes5);
        CallInject(0x733F4A,   ButtonCharGenClassSelection_OnLButtonClick_OpenSubKits_asm, bytes5);
        CallInject(0x723D08,   CScreenCreateChar_CompleteCharacterClass_SetAnimIDClassSuffix_asm, bytes5);
        RelativeInject(0x6324C6, CRuleTables_GetTHAC0_asm);
        vDataList.push_back(   Data(0x720F60+3, sizeof(buttonid), buttonid) );  // additional class button
        CallInject(0x71CAF4,   CScreenCreateChar_ResetAbilities_RollStrengthEx_asm, bytes5);
        CallInject(0x72B53A,   CScreenCreateChar_IsDoneButtonClickable_EnableMultiClassPanel_asm, bytes5);
        CallInject(0x729C1B,   ButtonCharGenMenu_OnLButtonClick_PrepareSkillPoints_asm, bytes5);
        CallInject(0x62FC6B,   CRuleTables_GetProficiencySlots_SelectClassLevels_asm, bytes5);
        CallInject(0x62C417,   CRuleTables_GetProficiencyClassIndex_asm, bytes5);
        CallInject(0x62F51E,   CRuleTables_GetNumLevelUpAbilities_asm, bytes5);
        CallInject(0x6325F0,   CRuleTables_GetMaxMemorizedSpellsPriest_asm, bytes6);
        CallInject(0x63ACAD,   CRuleTables_GetClassAbilityTable_asm, bytes5);
        CallInject(0x8D2DBD,   CGameSprite_AddNewSpecialAbilities_asm, bytes6);
        CallInject(0x8D38C0,   CGameSprite_RemoveNewSpecialAbilities_asm, bytes6);
        RelativeInject(0x63328A, CRuleTables_GetNextLevel_asm);

        //CScreenCharacter
        CallInject(0x6DF57C,   CScreenCharacter_ResetLevelUpPanel_SetSkillPoints_asm, bytes5);
        RelativeInject(0x6DDE79, CScreenCharacter_ResetLevelUpPanel_ShowNewPriestSpells_asm);
        CallInject(0x6DC23D,   CScreenCharacter_ResetLevelUpPanel_SetOldSorcererLevel_asm, bytes5);
        CallInject(0x6E3687,   CScreenCharacter_UpdateLevelUpPanel_SelectSkills_asm, bytes5);
        CallInject(0x6E2AE3,   CScreenCharacter_UpdateLevelUpPanel_SetFirstSkillType_asm, bytes6);
        CallInject(0x6E7622,   CScreenCharacter_UpdateMainPanel_ShowProficiencies_asm, bytes6);
        CallInject(0x6D8E2D,   CScreenCharacter_UpdateExperience_asm, bytes5);
        CallInject(0x774E75,   CScreenMultiPlayer_UpdateExperience_asm, bytes5);

        // CScreenGenChar::ResetChooseMagePanel
        CallInject(0x71B4B4,   CScreenGenChar_ResetChooseMagePanel_SwitchToShaman_asm, bytes6);
        RelativeInject(0x71B71A, GetXXXSpell_asm);
        CallInject(0x71B84A,   CScreenGenChar_ResetChooseMagePanel_CheckAllowedSpell_asm, bytes6);
        CallInject(0x71B5EB,   CScreenGenChar_ResetChooseMagePanel_PushBOOKNAME_asm, bytes5);

        // CScreenGenChar
        CallInject(0x718FA8,   CScreenGenChar_UpdateMainPanel_ShowProficienciesList_asm, bytes5);
        CallInject(0x717C84,   CScreenGenChar_UpdateMainPanel_SummonPopup_asm, bytes5);
        CallInject(0x71945E,   CScreenGenChar_UpdateMainPanel_ShowPriestSpellList_asm, bytes5);
        CallInject(0x7251BD,   CScreenCreateChar_CompleteCharacterWrapup_PriestSpells_asm, bytes5);
        CallInject(0x72521C,   CScreenCreateChar_CompleteCharacterWrapup_MageSpells_asm, bytes5);

        // CharGenChooseMageSelection::ButtonClick
        RelativeInject(0x732E54, ButtonCharGenChooseMageSelection_AddKnownSpell_asm);
        RelativeInject(0x732D5D, ButtonCharGenChooseMageSelection_RemoveKnownSpell_asm);

        CallInject(0x631C5A,   CRuleTables_GetHitPoints_asm, bytes8);
        CallInject(0x6312CB,   CRuleTables_RollHitPoints_asm, bytes6);
        CallInject(0x8DA336,   CGameSprite_GetSkillValue_asm, bytes7); 
        CallInject(0x630647,   CRuleTables_FindSavingThrow_asm, bytes6);
        RelativeInject(0x630BF8, CRuleTables_GetSavingThrow_asm);
        CallInject(0x63342B,   CRuleTables_GetNextLevelXP_asm, bytes5);
        CallInject(0x415DB5,   Object_IsUsableSubClass_asm, bytes6);
        CallInject(0x62BB58,   CRuleTables_GetClassHelp_asm, bytes6);
        CallInject(0x631603,   CRuleTables_GetHPCONBonusTotal_asm, bytes5);
        RelativeInject(0x8CE643, CGameSprite_CheckCombatStatsWeapon_SetTHACPenalty_asm);

        CallInject(0x69E03E,   CheckItemNotUsableByClass_asm, bytes7);
        CallInject(0x69E0B2,   CheckItemUsableByClass_asm, bytes7);

        // CScreenWizSpell
        CallInject(0x7B84E7,   CScreenWizSpell_UpdateMainPanel_SetSorcererViewMode_asm, bytes5);
        RelativeInject(0x7B894D, CScreenWizSpell_UpdateMainPanel_GetKnownSpellMage_asm);
        CallInject(0x7BCCF5,   IsArcaneClass_asm, bytes7);
        CallInject(0x8DAEAB,   CGameSprite_SorcererSpellCount_asm, bytes7);
        CallInject(0x7B8787,   CScreenWizSpell_UpdateMainPanel_GetCDSMaxMemSpells_asm, bytes8);

        CallInject(0x8DAAD4,   CGameSprite_SorcererSpellDecrement1_asm, bytes7);
        CallInject(0x8DAC08,   CGameSprite_SorcererSpellDecrement2_asm, bytes8);

        CallInject(0x931FC2,   CGameSprite_Rest_AddShaman_asm, bytes6);

        uchar caster_type[] =  { 0x2 };
        vDataList.push_back(   Data(0x913ED6, sizeof(caster_type), caster_type) ); 
        CallInject(0x913EC6,   CGameSprite_Spell_AddShaman_asm, bytes5);
        CallInject(0x91437E,   CGameSprite_Spell_AddShaman_asm, bytes5);

        vDataList.push_back(   Data(0x918428, sizeof(caster_type), caster_type) );
        CallInject(0x918418,   CGameSprite_SpellPoint_AddShaman_asm, bytes5);
        CallInject(0x9188D1,   CGameSprite_SpellPoint_AddShaman_asm, bytes5);

        CallInject(0x8D5E0C,   CGameSprite_RemoveSpellsPriest_AddShaman_asm, bytes5); // need test !

        vDataList.push_back(   Data(0x94C2D1, sizeof(caster_type), caster_type) );
        CallInject(0x94C2C1,   CGameSprite_RemoveSpell_AddShaman_asm, bytes5);
        CallInject(0x94C5FC,   CGameSprite_RemoveSpell_AddShaman_asm, bytes5);

        vDataList.push_back(   Data(0x8D63BE, sizeof(caster_type), caster_type) );
        CallInject(0x8D689F,   CGameSprite_SetMemorizedFlag_AddShaman_asm, bytes5);
        CallInject(0x8D63AE,   CGameSprite_SetMemorizedFlag_AddShaman_asm, bytes5);

        CallInject(0x540BFB,   CEffectRememorizeSpell_ApplyEffect_AddShaman_asm, bytes5);
        CallInject(0x8DACF9,   CGameSprite_SorcererSpellRememorize1_asm, bytes7);
        CallInject(0x8DAE2D,   CGameSprite_SorcererSpellRememorize2_asm, bytes8);

        CallInject(0x4758B9,   CDerivedStats_GetPriestLevelCast_AddShaman_asm, bytes5);
        CallInject(0x8CB9E9,   CGameSprite_GetMemorizedSpellPriest_SkipSpell_asm, bytes7);

        CallInject(0x68E5A7,   CInfGame_SelectToolbar_AddShaman_asm, bytes6);
        CallInject(0x662122,   CInfButtonArray_SetState_AddShaman_asm, bytes6);

        CallInject(0x8FB108,   CGameSprite_FeedBack_FindTraps_asm, bytes7);
        CallInject(0x8FB15C,   CGameSprite_FeedBack_StopFindTraps_asm, bytes7);

        CallInject(0x6EA5C6,   CScreenCharacter_OnDoneButtonClick_FakeSorcerer_asm, bytes5);
        RelativeInject(0x6EA722, CScreenCharacter_OnDoneButtonClick_GetXXXLevel_asm);
        RelativeInject(0x6EAEB5, CScreenCharacter_OnDoneButtonClick_GetXXXLevel_asm);
        CallInject(0x6EA767,   CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm, bytes6);
        CallInject(0x6EA857,   CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm, bytes6);
        CallInject(0x6EAEFA,   CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm, bytes6);
        CallInject(0x6EAFEA,   CScreenCharacter_OnDoneButtonClick_SPLSRCKN_asm, bytes6);

        CallInject(0x6E13AC,   CScreenCharacter_ResetChooseMagePanel_FakeSorcerer_asm, bytes5);
        CallInject(0x6E1335,   CScreenCharacter_ResetChooseMagePanel_BookName_asm, bytes6);
        CallInject(0x6E16B5,   CScreenCharacter_ResetChooseMagePanel_CheckAllowedSpell_asm, bytes6);

        RelativeInject(0x6E148B, GetXXXSpell_asm);
        RelativeInject(0x6E14F7, GetKnownSpellIndexXXX_asm);

        RelativeInject(0x6FAA0A, ButtonCharacterChooseMageSelection_OnLButtonClick_RemoveKnownSpell_asm);
        RelativeInject(0x6FAB31, ButtonCharacterChooseMageSelection_OnLButtonClick_AddKnownSpell_asm);

        CallInject(0x66579D,   CInfButtonArray_UpdateButtons_ToolTipFindTraps_asm, bytes5);
        CallInject(0x6650C7,   CInfButtonArray_UpdateButtons_ToolTipBattleSong_asm, bytes5);

        CallInject(0x8D4918,   CGameSprite_GetSecretDoorDetection_asm, bytes7);

        uchar last_class[] =   { CLASS_SHAMAN };
        vDataList.push_back(   Data(0x6384D3, sizeof(last_class), last_class) );    // CRuleTables::GetXPCap

        CallInject(0x63888E,   CRuleTables_GetBaseLore_asm, bytes7); 
        CallInject(0x639C43,   CRuleTables_GetNumQuickWeaponSlots_asm, bytes5);

        CallInject(0x47490E,   CDerivedStats_GetWizardLevel_asm, bytes6);
        RelativeInject(0x6EC90C, CScreenCharacter_OnCancelButtonClick_GetXXXLevel_asm);
        CallInject(0x6EC934,   CScreenCharacter_OnCancelButtonClick_SPLSRCKN1_asm, bytes6);
        CallInject(0x6EC9F5,   CScreenCharacter_OnCancelButtonClick_SPLSRCKN2_asm, bytes5);

        RelativeInject(0x6EF3B7, CScreenCharacter_RemoveAbilities_RemoveAllSpellsPriest_asm);

        CallInject(0x538A25,   CGameEffectLevelDrain_ApplyEffect_DrainSkillPoints_asm, bytes10);
        CallInject(0x53452B,   CGameEffectLevelDrain_OnAddSpecific_asm, bytes6);

        RelativeInject(0x52BBF8, CGameEffectDisableSpellType_ApplyEffect_asm);
        CallInject(0x52BC40,   CGameEffectDisableSpellType_ApplyEffect_TextFeedBack_asm, bytes6);
        CallInject(0x52BCA1,   CGameEffectDisableSpellType_ApplyEffect_RemoveQuickSpells_asm, bytes6);

        CallInject(0x6B75C1,   GetMultiplayerClassName_asm, bytes5);
        CallInject(0x72677B,   CScreenCreateChar_GetCharacterVersion_asm, bytes5);
        CallInject(0x639110,   CRuleTables_GetRaiseDeadCost_asm, bytes5);

        CallInject(0x77598A,   CScreenMultiPlayer_ResetViewCharacterPanel_ShowMageSpells_asm, bytes5);
        CallInject(0x775BCF,   CScreenMultiPlayer_ResetViewCharacterPanel_ShowPriestSpells_asm, bytes5);

        CallInject(0x8FB258,   CGameSprite_FeedBack_StartSongText_asm, bytes7);
        CallInject(0x8FB2AC,   CGameSprite_FeedBack_StopSongText_asm, bytes7);
        
        // override virtual OnRemove()
        DWORD address = (DWORD) CEffectDisableButton_OnRemove_Shaman_asm;
        uchar *offset = (uchar*) &address;
        vDataList.push_back( Data(0xAA8034, 4, offset) );

        CallInject(0x8933E5,   CGameSprite_SetPath_CancelDance_asm, bytes6);

        CallInject(0x8E3E6A,   CGameSprite_ExecuteAction_MakeUnselectable_asm, bytes6);
        CallInject(0x8E1792,   CGameSprite_ProcessPendingTriggers_asm, bytes7);


        CallInject(0x5221C8,   CGameEffectRandomSummon_Apply_CheckLimits_asm, bytes6);

        // patch CAnimation7****::MethodXXX to allow color location 255 (Character color)
        uchar bytes_anim7xxx[]  = { 0x77, 0x16, 0x90, 0x90, 0x90, 0x90
                                  };
        
        vDataList.push_back( Data(0x81051A, sizeof(bytes_anim7xxx), bytes_anim7xxx) );
        vDataList.push_back( Data(0x81B82B, sizeof(bytes_anim7xxx), bytes_anim7xxx) );

        CallInject(0x671BB1,   CInfButtonArray_PreRenderButton_asm, bytes5);

        // AddNewSpellsPriest       - keep orig to avoid adding all spells to book
        // CanCastPriestSpells      - keep orig to make priest screen unavailable
        // FilterSpecialAbilities   - NO NEED
        // AddClassAbilities        - NO NEED
        // GetAverageLevel          - NO NEED

        // 6EA6F9 6EA946 6EAE8C 9
        // limit max level in mage screen
        // search all 9 by CRE.offsets + cds.offsets

        // need to test:
        // SetMemorizedFlag


        /*

        class abilities per level (shaman unique spells)

        shaman HLAs

        start hp 16 != 8+2

        simulate DRUID and DRUID_ALL in scripts

        load savegame, fill memorized spells

        limit max level in mage screen

        check summons without IA

        add shaman script to auto detect illusion

        add 3 shaman items to areas/dialogs/scripts (BG1 missed)

        improved dance by Argent77

        */

        #ifdef _DEBUG
            //CallInject(0x4158A8,   Log_GetClass_asm, bytes6);
        #endif

        COMMIT_vDataList;
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Greeting before Dialog
    if (pGameOptionsEx->bUI_GreetingBeforeDialog) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        uchar bytes_jump[] =   { 0xEB };
        vDataList.push_back( Data(0x7D6EE7, sizeof(bytes_jump), bytes_jump) );

        CallInject(0x93AA1A, CGameSprite_PlayerDialog_InsertGreeting1_asm, bytes6);
        CallInject(0x939E39, CGameSprite_PlayerDialog_ToggleGreeting1_asm, bytes7);

        CallInject(0x938D71, CGameSprite_PlayerDialog_InsertGreeting2_asm, bytes6);
        CallInject(0x93833D, CGameSprite_PlayerDialog_ToggleGreeting2_asm, bytes5);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Highlight Active Zones
    if (pGameOptionsEx->bUI_HighlightActiveZones) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x581CFE, CGameTrigger_Render_Highlight_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Remove cursor rendering on primary surface after fliping
    // CVidInf::Flip() already did it properly on back buffer before fliping
    if (pGameOptionsEx->bVideo_FlickeringCursorFix) {
        uchar bytes_6nop[]= { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

        vDataList.push_back(   Data(0x6C1EA8, sizeof(bytes_6nop), bytes_6nop) );    // CInfTileSet::Render
        vDataList.push_back(   Data(0x6CF070, sizeof(bytes_6nop), bytes_6nop) );    // CInfTileSet::Render3D
        vDataList.push_back(   Data(0x9D108E, sizeof(bytes_6nop), bytes_6nop) );    // CVidMosaic::Render
        vDataList.push_back(   Data(0xA009C1, sizeof(bytes_6nop), bytes_6nop) );    // CVidMosaic::Render3D

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Put Cpu to Idle
    if (pGameOptionsEx->bEngine_CpuIdle) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90 };

        CreateWinMainSyncEvent();
        CallInject(0x9A622E, EngineNoActive_asm, bytes5);
        CallInject(0x9A8D81, CChitin_WinMain_AddWaitForEvent_asm, bytes7);
        CallInject(0x9A8DBE, CChitin_WinMain_AddDisplayDoneEvent_asm, bytes10);
        CallInject(0x4365D6, CBaldurChitin_MainAIThread_TriggerDisplaySync_asm, bytes10);
        CallInject(0xA2E72B, TimeGetTime_WaitCycle_asm, bytes5);    // Movie player timing

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Double Rate Rendering
    if (pGameOptionsEx->bUI_DoubleRenderRate) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90 };

        CreateWinMainSyncEvent();
        uchar bytes_timerperiod[] = { 0xF4, 0x01 };   //  1000/2
        vDataList.push_back(   Data(0x9A4E11, sizeof(bytes_timerperiod), bytes_timerperiod) );
        vDataList.push_back(   Data(0x436552, sizeof(bytes_timerperiod), bytes_timerperiod) );

        CallInject(0x9A8D81, CChitin_WinMain_AddWaitForEvent_asm, bytes7);
        CallInject(0x9A8DBE, CChitin_WinMain_AddDisplayDoneEvent_asm, bytes10);
        CallInject(0x4365D6, CBaldurChitin_MainAIThread_TriggerDisplaySync_asm, bytes10);
        CallInject(0x43658B, CBaldurChitin_MainAIThread_DoubleRateFrame_asm, bytes7);

        if (pGameOptionsEx->bUI_DoubleRenderRate_MouseOnlyMode) {
            CallInject(0x9A64F7, CChitin_SynchronousUpdate_CheckScreen_asm, bytes6);
        }

        //CallInject(0x9A51E6, CBaldurChitin_InitXXX_SetKeyboardSpeed_asm, bytes5);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Fast Progress Screen, also change "sleep cycles" -> "wait display"
    if (pGameOptionsEx->bUI_FastProgressBarScreen) {

        uchar bytes_jmp[] =   { 0xEB };
        uchar bytes_2nops[] = { 0x90, 0x90 };
        uchar bytes_2zero[] = { 0x00, 0x00 };
        uchar bytes_1zero[] = { 0x00 };

        uchar bytes_10nop[]= { 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes22[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0xEB };
        uchar bytes23[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0xEB };

        CreateWinMainSyncEvent();
        CallInject(0x9A8D81, CChitin_WinMain_AddWaitForEvent_asm, bytes7);
        CallInject(0x9A8DBE, CChitin_WinMain_AddDisplayDoneEvent_asm, bytes10);
        CallInject(0x4365D6, CBaldurChitin_MainAIThread_TriggerDisplaySync_asm, bytes10);

        // SaveGame
        CallInject(0x68BC8E, WaitDisplay_asm, bytes23);

        vDataList.push_back( Data(0x68CC54, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68CC64, RenderOneFrame_asm, bytes23);

        vDataList.push_back( Data(0x68D0F0, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68D100, RenderOneFrame_asm, bytes23);

        vDataList.push_back( Data(0x68CF1A, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68CF28, SleepEx_Emu_asm, bytes6);

        // LoadGame
        CallInject(0x68A068, WaitDisplay_asm, bytes22);

        vDataList.push_back( Data(0x68ACDB, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68ACEA, RenderOneFrame_asm, bytes23);

        vDataList.push_back( Data(0x68A115, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68A123, SleepEx_Emu_asm, bytes6);

        // ProgressBar + CacheStatus
        vDataList.push_back( Data(0x4675A7+1, sizeof(bytes_2zero), bytes_2zero) );
        vDataList.push_back( Data(0x6875DB+1, sizeof(bytes_1zero), bytes_1zero) );
        vDataList.push_back( Data(0x46A41D+1, sizeof(bytes_1zero), bytes_1zero) );

        vDataList.push_back( Data(0x4CC0F9, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x4CC138, SleepEx_Emu_asm, bytes6);

        vDataList.push_back( Data(0x687585, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6875C4, SleepEx_Emu_asm, bytes6);

        // LoadArea
        vDataList.push_back( Data(0x680CE6, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x680CF6, RenderOneFrame_asm, bytes23);

        CallInject(0x6813C1, WaitDisplay_asm, bytes22);

        vDataList.push_back( Data(0x6832FB, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68330B, RenderOneFrame_asm, bytes22);
        
        vDataList.push_back( Data(0x6808FA, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x680908, SleepEx_Emu_asm, bytes6);

        // 683245 first sleep, then trigger display, leave original

        // DestroyGame
        CallInject(0x67CEED, WaitDisplay_asm, bytes22);

        vDataList.push_back( Data(0x67DAC1, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x67DAD0, RenderOneFrame_asm, bytes23);

        // LeaveArea
        vDataList.push_back( Data(0x93028B, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x93029B, RenderOneFrame_asm, bytes22);

        // LeaveAreaName
        CallInject(0x92E322, WaitDisplay_asm, bytes23);
        CallInject(0x92DA96, WaitDisplay_asm, bytes22);

        // LeaveAreaLUA
        vDataList.push_back( Data(0x92B5F9, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x92B608, RenderOneFrame_asm, bytes23);

        // CScreenWorldMap::EnterArea
        CallInject(0x7F26DF, WaitDisplay_asm, bytes23);

        vDataList.push_back( Data(0x7F4E68, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F4E78, RenderOneFrame_asm, bytes22);

        vDataList.push_back( Data(0x7F65C9, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F65D9, RenderOneFrame_asm, bytes23);

        vDataList.push_back( Data(0x7F2944, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F2952, SleepEx_Emu_asm, bytes6);

        vDataList.push_back( Data(0x7F653A, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F6548, SleepEx_Emu_asm, bytes6);
        
        vDataList.push_back( Data(0x7F5E15, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F5E23, SleepEx_Emu_asm, bytes6);
        
        vDataList.push_back( Data(0x7F62C7, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F62D5, SleepEx_Emu_asm, bytes6);

        vDataList.push_back( Data(0x7F29CB, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7F29D9, SleepEx_Emu_asm, bytes6);

        // CInfTileSet::~CInfTileSet
        vDataList.push_back( Data(0x6C15DF+1, sizeof(bytes_1zero), bytes_1zero) );

        // LoadFile
        vDataList.push_back( Data(0x99D3C5, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x99D3DC, SleepEx_Emu_asm, bytes6);

        // CBaldurMessage::DemandResourceFromServer
        vDataList.push_back( Data(0x43D369, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x43D377, SleepEx_Emu_asm, bytes6);

        // CBaldurMessage::UpdateDemandCharacters
        vDataList.push_back( Data(0x4439F3, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x443A01, SleepEx_Emu_asm, bytes6);

        vDataList.push_back( Data(0x443C7C, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x443C8A, SleepEx_Emu_asm, bytes6);

        // CInfGame::Unmarshal
        vDataList.push_back( Data(0x685CE2, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x685CF0, SleepEx_Emu_asm, bytes6);

        vDataList.push_back( Data(0x685468, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x685476, SleepEx_Emu_asm, bytes6);
        
        // NewGame
        vDataList.push_back( Data(0x68B58C, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x68B59C, RenderOneFrame_asm, bytes22);

        // CScreenWorld::AsynchronousUpdate
        vDataList.push_back( Data(0x7D3B15, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7D3B23, SleepEx_Emu_asm, bytes6);

        // unknow 6B1B86
        vDataList.push_back( Data(0x6B1B86, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B1B94, SleepEx_Emu_asm, bytes6);

        // unknow 6B36F3
        vDataList.push_back( Data(0x6B36F3, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B3701, SleepEx_Emu_asm, bytes6);

        // unknow 70B902
        vDataList.push_back( Data(0x70B902, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x70B912, RenderOneFrame_asm, bytes23);

        // unknow 70BC0B
        vDataList.push_back( Data(0x70BC0B, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x70BC1B, RenderOneFrame_asm, bytes23);

        // unknow 710A13
        vDataList.push_back( Data(0x710A13, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x710A23, RenderOneFrame_asm, bytes23);

        // unknow 778875
        vDataList.push_back( Data(0x778875, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x778883, SleepEx_Emu_asm, bytes6);

        // unknow 43FC82
        vDataList.push_back( Data(0x43FC82, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x43FC90, SleepEx_Emu_asm, bytes6);

        // unknow 44E782
        vDataList.push_back( Data(0x44E782, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x44E7A1, SleepEx_Emu_asm, bytes6);

        // unknow 6B5398
        vDataList.push_back( Data(0x6B5398, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B53A8, RenderOneFrame_asm, bytes22);

        // unknow 6B6328
        vDataList.push_back( Data(0x6B6328, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B6336, SleepEx_Emu_asm, bytes6);

        // unknow 6B67DA
        vDataList.push_back( Data(0x6B67DA, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B67E8, SleepEx_Emu_asm, bytes6);

        // unknow 6B6A30
        vDataList.push_back( Data(0x6B6A30, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B6A3E, SleepEx_Emu_asm, bytes6);

        // unknow 6D45FA
        vDataList.push_back( Data(0x6D45FA, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6D4606, SleepEx_Emu_asm, bytes6);

        // unknow 6B6AC0
        vDataList.push_back( Data(0x6B6AC0, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B6AD0, RenderOneFrame_asm, bytes22);

        // unknow 6D5420
        vDataList.push_back( Data(0x6D5420, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6D5430, RenderOneFrame_asm, bytes22);

        // 708103 CD-ROM screen

        // unknow 76EE01
        vDataList.push_back( Data(0x76EE01, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x76EE0F, SleepEx_Emu_asm, bytes6);

        // unknow 772BC2
        vDataList.push_back( Data(0x772BC2, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x772BD0, SleepEx_Emu_asm, bytes6);

        // unknow 772C43
        vDataList.push_back( Data(0x772C43, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x772C51, SleepEx_Emu_asm, bytes6);

        // unknow 772D93
        vDataList.push_back( Data(0x772D93, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x772DA1, SleepEx_Emu_asm, bytes6);

        // unknow 772E14
        vDataList.push_back( Data(0x772E14, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x772E22, SleepEx_Emu_asm, bytes6);

        // unknow 773227
        vDataList.push_back( Data(0x773227, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x773235, SleepEx_Emu_asm, bytes6);

        // unknow 77394A
        vDataList.push_back( Data(0x77394A, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x77395B, SleepEx_Emu_asm, bytes6);
        CallInject(0x773967, WaitDisplay_asm, bytes22);

        // unknow 4464D0
        vDataList.push_back( Data(0x4464D0, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x4464DE, SleepEx_Emu_asm, bytes6);

        // unknow 44A9B6
        vDataList.push_back( Data(0x44A9B6, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x44A9C4, SleepEx_Emu_asm, bytes6);

        // unknow 5AF7C8
        vDataList.push_back( Data(0x5AF7C8, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x5AF7D6, SleepEx_Emu_asm, bytes6);

        // unknow 5C71EE
        vDataList.push_back( Data(0x5C71EE, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x5C71FC, SleepEx_Emu_asm, bytes6);

        // unknow 6B1C0D
        vDataList.push_back( Data(0x6B1C0D, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B1C1B, SleepEx_Emu_asm, bytes6);

        // unknow 6B28DD
        vDataList.push_back( Data(0x6B28DD, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B28EC, RenderOneFrame_asm, bytes23);

        // unknow 6B2D61
        vDataList.push_back( Data(0x6B2D61, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B2D6F, SleepEx_Emu_asm, bytes6);

        // unknow 6B2EAA
        vDataList.push_back( Data(0x6B2EAA, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B2EB8, SleepEx_Emu_asm, bytes6);

        // unknow 6B1C0D
        vDataList.push_back( Data(0x6B1C0D, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B1C1B, SleepEx_Emu_asm, bytes6);

        // unknow 6B2F37
        vDataList.push_back( Data(0x6B2F37, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B2F46, RenderOneFrame_asm, bytes23);

        // unknow 6B377A
        vDataList.push_back( Data(0x6B377A, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6B3788, SleepEx_Emu_asm, bytes6);

        // unknow 6D43FB
        vDataList.push_back( Data(0x6D43FB, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6D4409, SleepEx_Emu_asm, bytes6);

        // unknow 6D5063
        vDataList.push_back( Data(0x6D5063, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x6D5071, SleepEx_Emu_asm, bytes6);

        // unknow 70BB66
        vDataList.push_back( Data(0x70BB66, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x70BB75, RenderOneFrame_asm, bytes23);

        // unknow 710C0E
        vDataList.push_back( Data(0x710C0E, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x710C1D, RenderOneFrame_asm, bytes23);

        // unknow 72776D
        vDataList.push_back( Data(0x72776D, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x72778C, SleepEx_Emu_asm, bytes6);

        // unknow 76EC68
        vDataList.push_back( Data(0x76EC68, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x76EC77, RenderOneFrame_asm, bytes23);

        // unknow 772E6E
        vDataList.push_back( Data(0x772E6E, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x772E7D, RenderOneFrame_asm, bytes23);

        // unknow 773587
        vDataList.push_back( Data(0x773587, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x773596, RenderOneFrame_asm, bytes23);

        // unknow 7737A3
        vDataList.push_back( Data(0x7737A3, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x7737B1, SleepEx_Emu_asm, bytes6);

        // unknow 778448
        vDataList.push_back( Data(0x778448, sizeof(bytes_10nop), bytes_10nop) );
        CallInject(0x778456, SleepEx_Emu_asm, bytes6);

        // Skip FlushFileBuffers()
        vDataList.push_back( Data(0xA51873, 1, bytes_jmp) );

        // replace zlib to updated(asm optimized) version
        RelativeInject(0x99F033, z_uncompress);
        RelativeInject(0x99EFAF, z_compress2);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // "Over target (unattached)" visual effect fixes
    // 1) Stop animation if owner is already died
    // 2) Move animation together with a moving target/owner
    if (pGameOptionsEx->bEff_OverTargetVisualEffectFixes) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject(0x654D64, CVisualEffect_AIUpdate_CheckForDeath_asm, bytes7);
        CallInject(0x533D29, CVisualEffect_Apply_AddSelfMarker_asm, bytes6);
        CallInject(0x654C51, CVisualEffect_AIUpdate_ApplyTracking_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Prefer OpenGL Video Modes without DM_DISPLAYFIXEDOUTPUT 
    if (pGameOptionsEx->bVideo_SkipFixedInterpolationModes) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x9F1859, FilterVideoMode_asm, bytes6);

        if (pGameOptionsEx->bVideo_SkipFixedInterpolationModes == 2)
            CallInject(0x9F1885, OverrideVideoMode_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // OpenGL VSync On
    if (pGameOptionsEx->bVideo_OpenGL_VSync) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };

        CallInject(0x9EDBD8, Set_OpenGL_VSync_asm, bytes5);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Restore "min level" field in .ITM,
    // take only 2 bytes when converting .itm's AnimCode to String
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        RelativeInject(0x5AB014, CItem_TranslateAnimationType_FixAnimCode_asm);
        RelativeInject(0x5AB035, CItem_TranslateAnimationType_FixAnimCode_asm);
        RelativeInject(0x5AB056, CItem_TranslateAnimationType_FixAnimCode_asm);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Load Resolution from .ini/WidescreenMod
    bool   gDblResPossible;
    bool   widescreen;

    ushort widescreenNOPs   = *( (ushort *) (0x432793));  // widescreen 3.07 detect
    if (widescreenNOPs == 0x9090)
        widescreen=true;
    else
        widescreen=false;

    const ushort XResWidescreen = *(( WORD *) (0x43278F+2));
    const ushort YResWidescreen = *(( WORD *) (0x432B5E+7));

    int XResIni = GetPrivateProfileIntA("Program Options", "Resolution", 800, ".\\Baldur.ini");
    if (widescreen == false) {
        if (XResIni >= 1280)
            gDblResPossible = true;
        else
            gDblResPossible = false;
    } else {
        if (XResWidescreen >= 640 && YResWidescreen >= 480 && // widescreenmod must be reinstalled with logical size
            pGameOptionsEx->bVideo_StretchGUI_WidescreenMode) // user confirmed special widescreenmod mode
            gDblResPossible = true;
        else
            gDblResPossible = false;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Enable any resolution in window mode
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes_jmp[] =   { 0xEB };

        if (widescreen == false)
            vDataList.push_back( Data(0x4327A9, sizeof(bytes_jmp), bytes_jmp) );
        else
            vDataList.push_back( Data(0x4327AF, sizeof(bytes_jmp), bytes_jmp) );
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Blt8To32()/Blt8To16() fixes
    // 1. Transparent color handling for BAM without RLE
    // 2. BAM Frame size above 256 (support only VidCell without RLE)
    if (gDblResPossible) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes24[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90 };
        uchar bytes26[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes_FF[] = { 0xFF,0xFF };

        // Blt8To32
        CallInject(0xA0E723, CVidCell_Blt8To32_FixFrameSize_asm, bytes26);
        CallInject(0xA0E9A7, CVidCell_Blt8To32_FixPaletteFetch_asm, bytes5);
        vDataList.push_back( Data(0xA0E8DA+2, sizeof(bytes_FF), bytes_FF) );
        vDataList.push_back( Data(0xA0E98C+2, sizeof(bytes_FF), bytes_FF) );
        vDataList.push_back( Data(0xA0E99A+2, sizeof(bytes_FF), bytes_FF) );

        // Blt8To16
        CallInject(0x9CDBF0, CVidCell_Blt8To16_FixFrameSize_asm, bytes26);
        vDataList.push_back( Data(0x9CDD3D+2, sizeof(bytes_FF), bytes_FF) );
        JumpInject(0x9CDE3B, CVidCell_Blt8To16_FixPaletteNonCompressed_asm);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Enlarge Tooltip +
    // Enlarge DirectDraw buffers for doubleres Cursor
    if (pGameOptionsEx->bUserLargerTooltipScroll ||
        pGameOptionsEx->bVideo_StretchGUI        ||
        pGameOptionsEx->bVideo_StretchCursor) {

        uchar width_128x2[] = {0,    0x1};   // 128(orig max)      * 2(doubleres)
        uchar width_224[] =   {224,  0x0};   // 224(tobex new max)
        uchar width_224x2[] = {0xC0, 0x1};   // 224(tobex new max) * 2(doubleres)
        uchar height_64x2[] = {128,  0x0};   // 64(orig max)       * 2(doubleres)
        uchar buffer256_x2[]= {0x00, 0x02};  // 256x256(orig max)  * 2(doubleres)

        // mode:
        // 0 No changes
        // 1 StretchCursor
        // 2 LargerTooltipScroll
        // 3 Both
        int mode;
        if (gDblResPossible)
            mode = (pGameOptionsEx->bUserLargerTooltipScroll * 2) + pGameOptionsEx->bVideo_StretchCursor;
        else
            mode = (pGameOptionsEx->bUserLargerTooltipScroll * 2); // only modes 0/2 allowed

        //1. CInfToolTip.wScrollWidth height, 128
        switch (mode) {
            case 1:
                vDataList.push_back( Data(0x6739D9, sizeof(width_128x2), width_128x2) ); break;
            case 2:
                vDataList.push_back( Data(0x6739D9, sizeof(width_224),   width_224)   ); break;
            case 3:
                vDataList.push_back( Data(0x6739D9, sizeof(width_224x2), width_224x2) ); break;
        }
        
        //2. DDSURFACEDESC.dwWidth + DDSURFACEDESC.dwHeight, 128 + 64
        switch (mode) {
            case 1:
                vDataList.push_back( Data(0x9B73BD+3, sizeof(width_128x2), width_128x2) );
                vDataList.push_back( Data(0x9B73C4+3, sizeof(height_64x2), height_64x2) ); break;
            case 2:
                vDataList.push_back( Data(0x9B73BD+3, sizeof(width_224),   width_224)   ); break;
            case 3:
                vDataList.push_back( Data(0x9B73BD+3, sizeof(width_224x2), width_224x2) );
                vDataList.push_back( Data(0x9B73C4+3, sizeof(height_64x2), height_64x2) ); break;
        }

        //3. Rectangle when blitting between two cursor buffers in Fullscreen 2D
        switch (mode) {
            case 1:
                vDataList.push_back( Data(0x9B85D4+3, sizeof(width_128x2), width_128x2) );
                vDataList.push_back( Data(0x9B85DB+3, sizeof(height_64x2), height_64x2) ); break;
            case 2:
                vDataList.push_back( Data(0x9B85D4+3, sizeof(width_224),   width_224)   ); break;
            case 3:
                vDataList.push_back( Data(0x9B85D4+3, sizeof(width_224x2), width_224x2) );
                vDataList.push_back( Data(0x9B85DB+3, sizeof(height_64x2), height_64x2) ); break;
        }

        // Work buffers, DDSURFACEDESC.dwWidth + DDSURFACEDESC.dwHeight, 256 + 256
        if ((pGameOptionsEx->bVideo_StretchGUI || pGameOptionsEx->bVideo_StretchCursor) &&
             gDblResPossible) {
            vDataList.push_back( Data(0x9B76EC+3, sizeof(buffer256_x2), buffer256_x2) );
            vDataList.push_back( Data(0x9B76F3+3, sizeof(buffer256_x2), buffer256_x2) );
            vDataList.push_back( Data(0x9B7892+3, sizeof(buffer256_x2), buffer256_x2) );
            vDataList.push_back( Data(0x9B7899+3, sizeof(buffer256_x2), buffer256_x2) ); 
        }

        //vDataList.push_back( Data(0x9B79E3+3, sizeof(height_64x2), height_64x2) );
        //vDataList.push_back( Data(0x9B79EA+3, sizeof(height_64x2), height_64x2) );
        //vDataList.push_back( Data(0x9B7C43+3, sizeof(height_64x2), height_64x2) );
        //vDataList.push_back( Data(0x9B7C4A+3, sizeof(height_64x2), height_64x2) );
    
        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Cursor x2 Stretch
    if (pGameOptionsEx->bVideo_StretchCursor &&
        gDblResPossible) {

        uchar bytes7[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90 };
        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes_1[] = { 0x01 };
        uchar bytes5[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                          };
        uchar bytes6[]  = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90 };

        CallInject(0x9BD6AC, CVidInf_RenderPointerImage_DoubleCursor_asm, bytes7);
        CallInject(0x67295E, CInfCursor_Initialize_SetupCursorVidCell_asm,  bytes10);
        CallInject(0x6728D8, CInfCursor_Initialize_SetupCursorVidCell_asm,  bytes10);
        CallInject(0x672B02, CInfCursor_Initialize_SetupTooltipVidCell_asm, bytes10);
        CallInject(0x6739A1, CInfToolTip_Initialize_SetupTooltipFont_asm, bytes5);
        CallInject(0x9B5EAB, CVidMode_SetPointer_SetupCursorVidCell_asm, bytes10);

        // realign coordinates on Item cursor with numbers
        uchar bytes_2[] = { 2 };
        uchar bytes_10[] = { 5*2 };
        uchar bytes_14[] = { 7*2 };
        uchar bytes_18[] = { 9*2 };
        vDataList.push_back( Data(0x9BD6FF+2, sizeof(bytes_14), bytes_14) );
        vDataList.push_back( Data(0x9BD710+2, sizeof(bytes_14), bytes_14) );
        vDataList.push_back( Data(0x9BD740+2, sizeof(bytes_14), bytes_14) );
        vDataList.push_back( Data(0x9BD751+2, sizeof(bytes_14), bytes_14) );
            
        //vDataList.push_back( Data(0x9BD702+2, sizeof(bytes_18), bytes_18) );
        //vDataList.push_back( Data(0x9BD743+2, sizeof(bytes_18), bytes_18) );

        // hack for very small icons
        vDataList.push_back( Data(0x9BD71B+6, sizeof(bytes_2), bytes_2) );
        vDataList.push_back( Data(0x9BD75C+6, sizeof(bytes_2), bytes_2) );

        // new width of char
        vDataList.push_back( Data(0x9BD872+2, sizeof(bytes_10), bytes_10) );

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Shift Subtitles two lines down
    if (pGameOptionsEx->bVideo_ShiftSubtitlesDown) {

        uchar bytes6[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };

        CallInject(0x46321E, MVE_DrawString_ShiftLine_asm, bytes6);

        uchar MinSpace[]     = { 50 + 27 };  // 50(orig) + 27(two shifted lines)
        vDataList.push_back( Data(0x463021+2, sizeof(MinSpace), MinSpace) );
        vDataList.push_back( Data(0x46302C+2, sizeof(MinSpace), MinSpace) );

        COMMIT_vDataList;
    }


    extern void *gTextureBuffer;

    //////////////////////////////////////////////////////////////////////////////////////////////
    // GUI x2 Stretch
    if (pGameOptionsEx->bVideo_StretchGUI &&
        gDblResPossible) {

        uchar bytes10[]    = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes10nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes7[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90, 0x90 };
        uchar bytes6[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };
        uchar bytes33[]    = { 0xE8, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90 };
        uchar bytes41[]    = { 0xE8, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90 };
        uchar bytes43[]    = { 0xE8, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90 };

        if (widescreen == false) {
            //CallInject(0x4327DB, CBaldurChitin_SetupResolution1_asm, bytes10);
            //CallInject(0x4327EB, CBaldurChitin_SetupResolution2_asm, bytes10);
        } else {
            // force DoubleRes for Mode1024
            CallInject(0x4327E1, CBaldurChitin_SetupResolution1_asm, bytes10);
            CallInject(0x4327F1, CBaldurChitin_SetupResolution2_asm, bytes10);

            CallInject(0x432B53, SetLogicalWidth_asm, bytes7);
            CallInject(0x432B67, SetLogicalHeight_asm, bytes7);

            // Recalc GameWorld window coordinates
            MakeWord_x2(0x432B5E+7);
            MakeWord_x2(0x43278F+2);
            MakeWord_x2(0x432B72+6);
            MakeWord_x2(0x432B86+6);
            MakeWord_x2(0x432B90+6);
            MakeWord_x2(0x432BC8+6);
            MakeWord_x2(0x432BDC+6);
            MakeWord_x2(0x432BE6+6);
            MakeWord_x2(0x432C1F+6);
            MakeWord_x2(0x432C33+6);
            MakeWord_x2(0x432C3D+6);
            MakeWord_x2(0x432C76+6);
            MakeWord_x2(0x432C8A+6);
            MakeWord_x2(0x432C94+6);
            MakeWord_x2(0x432CCC+6);
            MakeWord_x2(0x432CE0+6);
            MakeWord_x2(0x432CEA+6);
            MakeWord_x2(0x432D23+6);
            MakeWord_x2(0x432D37+6);
            MakeWord_x2(0x432D41+6);
            
            // savegame(?) thumbnail unknow scaling, copied from mode1280
            uchar bytes384[]     = { 0x80, 0x01 };  // 384
            vDataList.push_back( Data(0x432D7A+6, sizeof(bytes384), bytes384) );
            uchar bytes246[]     = { 246, 0x00 };   // 246
            vDataList.push_back( Data(0x432D84+6, sizeof(bytes246), bytes246) );
            uchar bytes896[]     = { 0x80, 0x03 };  // 896
            vDataList.push_back( Data(0x432D8E+6, sizeof(bytes896), bytes896) );
            uchar bytes630[]     = { 0x76, 0x02 };  // 630
            vDataList.push_back( Data(0x432D98+6, sizeof(bytes630), bytes630) );

        }

        // disable override CVidMosaic.bDoubleResolution
        vDataList.push_back( Data(0x6C0E69, sizeof(bytes10nop), bytes10nop) );

        // New MVE ShowFrame32/ShowFrame16/ShowFrame3D
        uchar bytes_Blt8To32[] =  { 0xE8, 0x00, 0x00, 0x00, 0x00,
                                    0xE9, 0x85, 0x00, 0x00, 0x00 };
        uchar bytes_Blt8To16[] =  { 0xE8, 0x00, 0x00, 0x00, 0x00,
                                    0xE9, 0x7F, 0x00, 0x00, 0x00 };
        uchar bytes_BltPackedTo32[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                                        0xE9, 0xF1, 0x01, 0x00, 0x00 };
        uchar bytes_BltPackedTo16[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                                        0xE9, 0x84, 0x00, 0x00, 0x00 };
        CallInject(0x461059, CProjector_ShowFrame32_ScaleX2_asm, bytes_Blt8To32);
        CallInject(0x460B1E, CProjector_ShowFrame16_ScaleX2_asm, bytes_Blt8To16);

        CallInject(0x463F42, CProjector_ShowFrame3d_RecalcOffsetsX2_asm, bytes6);
        CallInject(0x463A36, CProjector_ShowFrame3d_RecalcOffsetsX2_asm, bytes6);   // packed
        CallInject(0x4641B1, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x4641CA, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x464253, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x46426C, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x4642D0, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x46430E, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x4643D0, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x46440E, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);

        CallInject(0x463CC0, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);   // packed
        CallInject(0x463CD9, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);   // ..
        CallInject(0x463D62, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x463D7B, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x463DDF, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x463E1D, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);
        CallInject(0x463EDF, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);   // ..
        CallInject(0x463F1D, CProjector_ShowFrame3d_ScaleX2_asm, bytes6);   // packed

        CallInject(0x460DCE, CProjector_ShowFrame32Packed_ScaleX2_asm, bytes_BltPackedTo32);
        CallInject(0x460A00, CProjector_ShowFrame16Packed_ScaleX2_asm, bytes_BltPackedTo16);

        // Normalize Subtitles
        CallInject(0x461104, CProjector_ShowFrame32_ShowSubtitles_asm, bytes33);
        CallInject(0x460BFA, CProjector_ShowFrame16_ShowSubtitles_asm, bytes33);
        CallInject(0x46456F, CProjector_ShowFrame3d_ShowSubtitles_asm, bytes41);
        CallInject(0x4644E0, CProjector_ShowFrame3dPacked_ShowSubtitles_asm, bytes43);
        
        // MVE_MovieMessage::DrawString
        uchar DrawString1[]     = { 0x66, 0x8B, 0x45, 0x14, 0x90, 0x90 };
        vDataList.push_back( Data(0x46305D, sizeof(DrawString1), DrawString1) );
        uchar DrawString2[]     = { 0x66, 0x8B, 0x4D, 0x10, 0x90, 0x90, 0x90 };
        vDataList.push_back( Data(0x46306B, sizeof(DrawString2), DrawString2) );
        uchar DrawString3[]     = { 0x66, 0x8B, 0x55, 0x14, 0x90, 0x90, 0x90 };
        vDataList.push_back( Data(0x4630F9, sizeof(DrawString3), DrawString3) );
        uchar DrawString4[]     = { 0x66, 0x8B, 0x45, 0x10, 0x90, 0x90 };
        vDataList.push_back( Data(0x463108, sizeof(DrawString4), DrawString4) );
        uchar DrawString5[]     = { 0x66, 0x8B, 0x55, 0x10, 0x90, 0x90, 0x90 };
        vDataList.push_back( Data(0x463197, sizeof(DrawString5), DrawString5) );

        if (pGameOptionsEx->bVideo_ShiftSubtitlesDown) {
            uchar MinSpace[]     = { 100 + 27 }; // 50(orig)*2 + 27(two shifted lines), 127 max
            vDataList.push_back( Data(0x463021+2, sizeof(MinSpace), MinSpace) );
            vDataList.push_back( Data(0x46302C+2, sizeof(MinSpace), MinSpace) );
        } else {
            uchar MinSpace[]     = { 100 };      // 50(orig)*2
            vDataList.push_back( Data(0x463021+2, sizeof(MinSpace), MinSpace) );
            vDataList.push_back( Data(0x46302C+2, sizeof(MinSpace), MinSpace) );
        }

        // OpenGL Texture Size x2
        gTextureBuffer = malloc(512*512*4); // 256x256 x2 = 512x512, free() in Console::~Console
        vDataList.push_back( Data(0x400000 + 0x63B5D, 4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x63C76, 4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x6405B, 4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x640F9, 4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x6412B, 4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x2CF336,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5B8B85,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5B8C95,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5B8CC2,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5B9DBA,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5B9DEA,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA0DD,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA11C,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA42C,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA46D,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA4AC,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA4ED,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA5ED,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA636,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA8C7,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BA90E,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BABBA,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BAD58,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5BAD99,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5D9F32,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5D9F72,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5F1E04,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FE838,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FE86A,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FF489,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FF4A2,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FF520,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FF8E1,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x5FF9F3,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x6000F2,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x600BDD,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x600C23,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x600C55,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x60208A,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x602142,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x602289,4, (uchar *)&gTextureBuffer) );
        vDataList.push_back( Data(0x400000 + 0x6022BB,4, (uchar *)&gTextureBuffer) );

        uchar NewTextureSize[] = { 0x00, 0x02 }; // 512
        vDataList.push_back( Data(0x9FF17C+2, sizeof(NewTextureSize), NewTextureSize) );
        vDataList.push_back( Data(0x9FF193+2, sizeof(NewTextureSize), NewTextureSize) );
        vDataList.push_back( Data(0xAB9104,   sizeof(NewTextureSize), NewTextureSize) );
        vDataList.push_back( Data(0xAB9108,   sizeof(NewTextureSize), NewTextureSize) );
        vDataList.push_back( Data(0x9B89F9+2, sizeof(NewTextureSize), NewTextureSize) );
        vDataList.push_back( Data(0x9B8A02+3, sizeof(NewTextureSize), NewTextureSize) );


        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Mouse Wheel on some ScrollBars
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes6[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };
        uchar bytes_6nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90 };
        uchar bytes5[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00
                             };

        //Store
        vDataList.push_back( Data(0x79D8C8, sizeof(bytes_6nop), bytes_6nop) );
        vDataList.push_back( Data(0x79D64B, sizeof(bytes_6nop), bytes_6nop) );
        vDataList.push_back( Data(0x79F19F, sizeof(bytes_6nop), bytes_6nop) );
        vDataList.push_back( Data(0x79FF10, sizeof(bytes_6nop), bytes_6nop) );
        vDataList.push_back( Data(0x7A0D6C, sizeof(bytes_6nop), bytes_6nop) );
        vDataList.push_back( Data(0x79F85C, sizeof(bytes_6nop), bytes_6nop) );

        //Custom Portraits Panel
        vDataList.push_back( Data(0x6DA37E, sizeof(bytes_6nop), bytes_6nop) );

        //LevelUp
        vDataList.push_back( Data(0x6E2448, sizeof(bytes_6nop), bytes_6nop) );

        CallInject(0x9A355D, CChitin_AsynchronousUpdate_SwitchScrollBar_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Improved Store&Bag Interface
    if (pGameOptionsEx->bUI_ImprovedStoreInterface) {

        uchar bytes6[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };
        uchar bytes7[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90, 0x90 };
        uchar bytes_6nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90 };
        uchar bytes5[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00
                             };
        uchar bytes16[]    = { 0xE8, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes31[]    = { 0xE8, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                               0x90 };

        RelativeInject(0x7B03BA, ScreenStoreRightPanelButtonClick_CheckShiftKey_asm);
        RelativeInject(0x7AF498, ScreenStoreLeftPanelButtonClick_CheckShiftKey_asm);

        CallInject(0x79E175, ScreenStoreLeftPanel_FetchItem_asm, bytes31);
        CallInject(0x7ADF06, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7ADE68, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7ADFDC, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7AE089, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7AE16E, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7ADDFC, ScreenStore_GetLeftListCountEDX_asm, bytes6);

        CallInject(0x7A5559, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7A552E, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7A54C6, ScreenStore_GetLeftListCountECX_asm, bytes6);
        CallInject(0x7A54F1, ScreenStore_GetLeftListCountECX_asm, bytes6);

        CallInject(0x7A665A, ScreenStore_GetLeftListCountECX_asm, bytes6);
        CallInject(0x7A6685, ScreenStore_GetLeftListCountECX_asm, bytes6);
        CallInject(0x7A66BC, ScreenStore_GetLeftListCountEAX_asm, bytes6);
        CallInject(0x7A66E7, ScreenStore_GetLeftListCountEAX_asm, bytes6);

        CallInject(0x7A4779, CScreenStore_OnBuyItemButtonClick_IncrementItemIndex_asm, bytes6);

        CallInject(0x7AF46F, ScreenStoreLeftPanelButtonClick_asm, bytes16);
        CallInject(0x7AF622, ScreenStoreLeftPanelDblButtonClick_asm, bytes6);
        CallInject(0x7AF79A, ScreenStoreLeftPanelRButtonClick_asm, bytes7);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Replace "Turn Undead" button to "Pick Pockets" for Cleric/Thief multiclass
    if (pGameOptionsEx->bUI_PickPocketsButtonForClericThief) {

        uchar bytes6[]     = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };

        uchar bytes1[]     = { 0x0C };  // Pick Pockets ID
        uchar bytes2[]     = { 0x0D };  // Turn Undead  ID
        vDataList.push_back( Data(0x663603+6, sizeof(bytes1), bytes1) );
        vDataList.push_back( Data(0x66272B+6, sizeof(bytes1), bytes2) );

        CallInject(0x66D301, ClericThief_TurnUndeadInnate_asm, bytes6);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Hide Innate Button if all innates spent
    if(pGameOptionsEx->bUI_DisableInnateToolBar) {
        RelativeInject(0x666632, CInfButtonArray_UpdateButtons_CheckInnateList_asm);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Sequencer Spell Panel
    if (pGameOptionsEx->bUI_SequenceWindowButton) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };

        CallInject(0x7B8B95, CScreenWizSpell_UpdateMainPanel_CheckSequenceList_asm, bytes6);
        CallInject(0x7BBA53, GetContingencyPtrEDX_asm, bytes6);
        CallInject(0x7BE1D9, GetContingencyPtrEAX_asm, bytes5);
        CallInject(0x7BC740, GetContingencyPtrCre_EDX_asm, bytes6);
        CallInject(0x7BBE74, CScreenWizSpell_UpdateContingPanel_GetStringRef_asm, bytes6);
        CallInject(0x7BC8B3, RemoveContingency_asm, bytes6);

        uchar bytes_1[] = { 0x01 };  // compareSourceType = 1 when call CGameEffectList::Remove()
        vDataList.push_back( Data(0x7BC8BC+1, sizeof(bytes_1), bytes_1) );
        vDataList.push_back( Data(0x7BC8E6+1, sizeof(bytes_1), bytes_1) );

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Innate/Spell Panel RClick 
    if (pGameOptionsEx->bUI_SpellIconRightClick) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes8[] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90};

        PointerInject(0xAB7730, CUIControlButtonAction_OnRButtonClick_asm); // CUIControlButtonAction::OnRButtonClick
        CallInject(0x787CBA, CScreenPriestSpell_OnDoneButtonClick_asm, bytes8);
        CallInject(0x787DA3, CScreenPriestSpell_EscapeKeyDown_asm, bytes8);
        RelativeInject(0x78A2E4, CScreenPriestSpell_IconAssign_asm);
        RelativeInject(0x787E83, CScreenPriestSpell_UpdateInfoPanel_GetGenericName_asm);
        RelativeInject(0x787F05, CScreenPriestSpell_UpdateInfoPanel_GetDescription_asm);
        CallInject(0x78A2B9, CScreenPriestSpell_LoadIconSpell_asm, bytes6);
        
        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Protection From Spell Opcode fix for Contingency Lists
    if (pGameOptionsEx->bEffProtectionFromSpellContingencyFix) {

        uchar bytes13[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90};
        CallInject(0x4AEAD4,  CGameAIBase_FireSpell_InjectProjectileUpdate_asm, bytes13);

        COMMIT_vDataList;
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Allow run game in inactive window
    if (pGameOptionsEx->bEngine_RunInBackground) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x9A6E15,  CChitin_OnAltTab_asm, bytes7);
        CallInject(0x9A378F,  CChitin_AsynchronousUpdate_CheckWindowEdge_asm, bytes6);
        CallInject(0x9A3811,  CChitin_AsynchronousUpdate_CheckWindowEdge_asm, bytes6);
        CallInject(0x9A3603,  CChitin_AsynchronousUpdate_CheckCursorPT_asm, bytes6);
        DWORD address = (DWORD) GetAsyncKeyState_asm;
        uchar *offset = (uchar*) &address;
        vDataList.push_back( Data(0xAA5674, 4, offset) );   // GetAsyncKeyState

        COMMIT_vDataList;
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Hide static Portrait icons
    if (pGameOptionsEx->bUI_HideStaticPortraitIcons) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x8AAB72, RenderIcons_asm, bytes6);
        CallInject(0x8AAD01, RenderIconsClear_asm, bytes7);

        COMMIT_vDataList;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////
    // Keyboard Shortcuts:
    // Numpad * Show HP on Portrait 
    // Numpad / Hide static Portrait icons
    // Numpad - Toggle Spell Menu
    // Numpad + Toggle GreyBackground On Pause
    if (pGameOptionsEx->bUI_ShortCuts) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject(0x7CD6D2, CScreenWorld_NoBindedKeyHandle_asm, bytes6);
        RelativeInject(0x6D8053, CScreenCharacter_NoBindedKeyHandle_asm);
        RelativeInject(0x740A73, CScreenInventory_NoBindedKeyHandle_asm);
        RelativeInject(0x761E60, CScreenMap_NoBindedKeyHandle_asm);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // External crashdump library
    if (pGameOptionsEx->bDebug_CrashdumpLibrary) {
        PointerInject(0xABC224, RecordExceptionInfoMain_asm);           // main()
        PointerInject(0xABC1EC, RecordExceptionInfoThreadStart_asm);    // _threadstart()
        PointerInject(0xABC38C, RecordExceptionInfoThreadStartEx_asm);  // _threadstartex()

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Color party foot circle
    if (pGameOptionsEx->bUI_PartyColorCircles) {
        RelativeInject(0x95CDFB, CMarker_RenderSprite_asm);

        COMMIT_vDataList;
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // Assume SPACE key as ENTER in dialog
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x7C71E3, CScreenWorld_HandleKeyboard_CheckDialogState_asm, bytes7);

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Stop casting animation/sound if spell was interrupted or caster died
    if (pGameOptionsEx->bEff_StopInterruptedCastingAnimation) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject(0x615984, CProjectileCastingGlow_AIUpdate_CheckInterrupt_asm, bytes5);
        CallInject(0x516186, CEffectSparkle_Apply_AddCreId_asm, bytes6);
        CallInject(0x5FA466, CSparkle_AIUpdate_CheckInterrupt_asm, bytes6);
        CallInject(0x8AD664, CGameSprite_SetSequence_StopCastingSound_asm, bytes6);
        CallInject(0x93FC61, CGameSprite_ApplyCastingEffect_Trigger_asm, bytes7);
        CallInject(0x94114F, CGameSprite_ApplyCastingEffectPost_Trigger_asm, bytes7);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Spell Select menu (Bubb EE's like)
    if (pGameOptionsEx->bUI_SpellSelectMenu) {
        pGameOptionsEx->bEngineCharacterBehindPolygonFix = TRUE; // require vert list sorting

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes6_nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                              0x90 };

        CallInject(0x662AF6, CInfButtonArray_SetState_OpenSpellToolbar_asm, bytes10);
        CallInject(0x662CFD, CInfButtonArray_SetState_OpenMixedSpellToolbar_asm, bytes10);
        //RelativeInject(0x66211A, CInfButtonArray_SetState_ClearPrevState_asm);
        //CallInject(0x6619E3, CInfButtonArray_ResetState_Log_asm, bytes6);
        PointerInject(0xAB57DC, CScreenWorld_OnLButtonUp_CheckClick_asm);
        RelativeInject(0x694680, CInfButtonArray_ResetState_CloseMenu_asm);
        RelativeInject(0x7CC3EA, CInfButtonArray_ResetState_CloseMenu_asm);
        CallInject(0x4D0AFD, CGameArea_Render_PostDraw_asm, bytes6);
        CallInject(0x4D09A0, CGameArea_Render_InjectCheck_asm, bytes6);
        CallInject(0x66E6CB, CInfButtonArray_OnLButtonPressed_SwitchBook_asm, bytes6);
        CallInject(0x4BE819, CGameArea_AIUpdate_FakeVisibleArea_asm, bytes6);

        // fake visible for RMouse click
        vDataList.push_back( Data(0x4CED2B, sizeof(bytes6_nop), bytes6_nop) );
        vDataList.push_back( Data(0x4CED67, sizeof(bytes6_nop), bytes6_nop) );

        // fake visible for LMouse click
        vDataList.push_back( Data(0x4CDA59, sizeof(bytes6_nop), bytes6_nop) );

        CallInject(0x4C665A, CGameArea_Marshal_Skip1_asm, bytes5);

        // TODO:
        // CInfGame::DestroyGame -> RemoveFromArea ?
        // scrollable if too many
        // wild mages Reckless Dweomer selection

        COMMIT_vDataList;
    }

    
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Character Behind Polygon Fix
    // must be after "Spell Select menu"
    if (pGameOptionsEx->bEngineCharacterBehindPolygonFix) {
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x4BF788, Area_GetFirstNode_asm, bytes6);
        CallInject(0x4BF7B0, Area_GetPrevNode_asm, bytes6);
        //CallInject(0x4BF7BC, Area_DumpEnum_asm, bytes6);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Fullscreen World Map
    if (pGameOptionsEx->bUI_FullScreenWorldMap) {

        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        uchar bytes5[] = { 0x05 };
        RelativeInject(0x7ECE11, CScreenWorldMap_EngineGameInit_asm);
        RelativeInject(0x7ECDBB, CScreenWorldMap_EngineGameInit_asm);
        vDataList.push_back( Data(0x7ECE16+3, sizeof(bytes5), bytes5) ); // disable STONE* panels

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Show HitPoints on NPC
    if (pGameOptionsEx->bUI_ShowNPCFloatHP) {

        uchar bytes_B0[] = { 0xB0, 0x00, 0x00, 0x00};  // enlarge stack AC->B0 (+4 bytes)
        vDataList.push_back( Data(0x957AB7+2, sizeof(bytes_B0), bytes_B0) );

        RelativeInject(0x957C37, CInfGame_GetCharacterPortrait_FakePartyMember_asm);

        if (pGameOptionsEx->bUI_ShowNPCFloatHP_Party == FALSE) {
            uchar bytes_5nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90};
            vDataList.push_back( Data(0x7CBF86, sizeof(bytes_5nop), bytes_5nop) );  // disable party float hp
        }

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // NPC tooltip names on Alt key
    if (pGameOptionsEx->bUI_ShowNPCFloatName) {

        if (pGameOptionsEx->bUI_ShowNPCFloatHP == FALSE) {         // apply helper
            uchar bytes_B0[] = { 0xB0, 0x00, 0x00, 0x00};  // enlarge stack AC->B0 (+4 bytes)
            vDataList.push_back( Data(0x957AB7+2, sizeof(bytes_B0), bytes_B0) );

            RelativeInject(0x957C37, CInfGame_GetCharacterPortrait_FakePartyMember_asm);
        }

        // push    1
        // push    1
        // push    0A4h, LEFT_ALT 
        uchar bytes_altkey[] = { 0x6A, 0x01, 0x6A, 0x01, 0x68, 0xA4, 0x00, 0x00, 0x00 };
        vDataList.push_back( Data(0x7C49DC, sizeof(bytes_altkey), bytes_altkey) );

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject(0x957D08, CGameSprite_FloatingHP_SetText_asm, bytes6);
        //CallInject(0x9A46BC, CChitin_AsynchronousUpdate_CheckAltKey_asm, bytes7);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Inject Load/Save Game:
    // - Pause state                0x100
    // - Legacy of Bhaal active     0x200
    // - Greyscale background       0x400
    if (
        pGameOptionsEx->bEngine_AddPauseToSaveGame  ||
        pGameOptionsEx->bUI_NightmareMode           ||
        pGameOptionsEx->bUI_GreyBackgroundOnPause   ||
        pGameOptionsEx->bEngine_LimitXP
        ) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x683ED5, CInfGame_Marshal_SaveGameInject_asm, bytes6);
        CallInject(0x684CDD, CInfGame_UnMarshal_LoadGameInject_asm, bytes6);

        COMMIT_vDataList;
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // "Short Overflow" fix if Cre.BaseStats.currentHP > 327 for Mislead effect
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes_mul[] = { 0x02};    // *100 -> *2
        uchar bytes_div[] = { 0x01};    // /50  -> /1
        vDataList.push_back( Data(0x53CEA5+3, 1, bytes_mul) );
        vDataList.push_back( Data(0x53CECD+1, 1, bytes_div) );

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////    
    // Auto detect & switch Priest/Mage book on Portrait Click
    if (pGameOptionsEx->bUI_AutoSwitchBookScreen) {
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x7868D3, CScreenPriestSpell_OnPortraitLClick_SwitchBook_asm, bytes6);
        CallInject(0x7B7BD6, CScreenWizSpell_OnPortraitLClick_SwitchBook_asm, bytes6);

        COMMIT_vDataList;
    }

    
    ////////////////////////////////////////////////////////////////////////////    
    // Add info at Record Screen:
    // - Intoxication
    // - Fatigue
    // - Luck
    // - Casting Speed
    // - Movement rate (disabled)
    // - Thac penalty when two-weapon specializtion is 0
    if (pGameOptionsEx->bUI_ExtendedRecordScreenText) {
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes_6nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

        RelativeInject(0x6E88E1, CScreenChar_UpdateMainPanel_AddNewInfo_asm);

        vDataList.push_back( Data(0x6E8DA1, sizeof(bytes_6nops), bytes_6nops) );   // enable with WeaponStyle=0
        vDataList.push_back( Data(0x6E8DAE, sizeof(bytes_6nops), bytes_6nops) );   // enable with Level=0
        vDataList.push_back( Data(0x95120D, sizeof(bytes_6nops), bytes_6nops) );   // enable fist mode

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////    
    // Normalize Volume
    if (pGameOptionsEx->bSound_Normalize) {
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x9889BC, CResWave_CopyWaveData_Normalize_asm, bytes5);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////    
    // Savegame's Creature name broken first char fix
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90 };
        CallInject(0x8B8A72, CGameSprite_Marshal_FirstCharFix_asm, bytes9);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Mixed Soundset for BGT
    if (pGameOptionsEx->bSound_BGTSelectionSoundSet) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90 };
        CallInject(0x8A25E6, CCreatureObject_PlaySound_COMMON_asm, bytes7);
        //CallInject(0x8A29E2, CCreatureObject_PlaySound_LimitActionSoundToBG1_asm, bytes7);
        CallInject(0x8A14BA, CCreatureObject_PlaySound_MORALE_asm, bytes7);
        CallInject(0x8A15A3, CCreatureObject_PlaySound_HAPPY_asm, bytes7);
        CallInject(0x8A1664, CCreatureObject_PlaySound_UNHAPPYANNOYED_asm, bytes7);
        CallInject(0x8A1725, CCreatureObject_PlaySound_UNHAPPYSERIOUS_asm, bytes7);
        CallInject(0x8A17E5, CCreatureObject_PlaySound_UNHAPPYBREAKINGPOINT_asm, bytes7);
        CallInject(0x8A18CE, CCreatureObject_PlaySound_LEADER_asm, bytes7);
        CallInject(0x8A198F, CCreatureObject_PlaySound_TIRED_asm, bytes7);
        CallInject(0x8A1A50, CCreatureObject_PlaySound_BORED_asm, bytes7);
        CallInject(0x8A1EAC, CCreatureObject_PlaySound_DAMAGE_asm, bytes7);
        CallInject(0x8A1F90, CCreatureObject_PlaySound_DYING_asm, bytes7);
        CallInject(0x8A207A, CCreatureObject_PlaySound_HURT_asm, bytes7);
        CallInject(0x8A2164, CCreatureObject_PlaySound_AREA_FOREST_asm, bytes7);
        CallInject(0x8A2225, CCreatureObject_PlaySound_AREA_CITY_asm, bytes7);
        CallInject(0x8A22E5, CCreatureObject_PlaySound_AREA_DUNGEON_asm, bytes7);
        CallInject(0x8A23A6, CCreatureObject_PlaySound_AREA_DAY_asm, bytes7);
        CallInject(0x8A2467, CCreatureObject_PlaySound_AREA_NIGHT_asm, bytes7);
        CallInject(0x8A29E2, CCreatureObject_PlaySound_ACTION_asm, bytes7);
        CallInject(0x8A4C3C, CGameSprite_VerbalConstant_asm, bytes9);

        COMMIT_vDataList;
    }
    

    ////////////////////////////////////////////////////////////////////////////
    // Disable Weapon in second hand for BG1 part
    // works only on InventoryScreen for Party, NPC can use second hand anyway
    if (pGameOptionsEx->bUI_DisableOffHandWeaponBG1part) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject (0x69C023, CInfGame_SwapItemPersonal_CheckShieldSlot_asm, bytes7);

        COMMIT_vDataList;        
    }


    ////////////////////////////////////////////////////////////////////////////
    // Trigger AreaCheckObject() crash fix
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        RelativeInject (0x489962, CGameAIBase_EvaluateStatusTrigger_AreaCheckObject_asm);

        COMMIT_vDataList;        
    }


    ////////////////////////////////////////////////////////////////////////////
    // Externalize ANISNDEX.2DA
    if (pGameOptionsEx->bSound_ANISNDEX ) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject (0x7FA8D2, CAnimationXXX_OverrideAniSndReference_asm, bytes7);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Priority CRE Sound Over 2DA Sound entries
    // Externalize Animation parameters (EXTANIM.2DA) :
    // walk speed
    // walk snd freq
    // footsize/personal space
    // blood color
    // death chunk
    // 4-char animation *.BAM + sound .2DA prefix
    // 4-char paperdoll *INV.BAM prefix
    // 0x5000 and 0x6400 have additional EXTANI*.2DA

    if (pGameOptionsEx->bAnimation_EXTANIM) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        //CallInject (0x, CAnimation0000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x835A76, CAnimation1000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x82DCBD, CAnimation1200_InjectPrefixOverride_asm, bytes6);
        CallInject (0x874935, CAnimation1300_InjectPrefixOverride_asm, bytes6);
        CallInject (0x83D5CA, CAnimation2000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x8412A4, CAnimation3000_InjectPrefixOverride_asm, bytes6);
        //CallInject (0x, CAnimation4000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x845205, CAnimation5000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x859AE8, CAnimation6400_InjectPrefixOverride_asm, bytes6);
        CallInject (0x819C08, CAnimation7000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x80D462, CAnimation7300_InjectPrefixOverride_asm, bytes6);
        CallInject (0x839F69, CAnimation8000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x828341, CAnimation9000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x82A4C8, CAnimationA000_InjectPrefixOverride_asm, bytes6);
        //CallInject (0x, CAnimationB000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x805441, CAnimationC000_InjectPrefixOverride_asm, bytes6);
        //CallInject (0x, CAnimationD000_InjectPrefixOverride_asm, bytes6);
        CallInject (0x81E1E6, CAnimationE000_InjectPrefixOverride_asm, bytes6);

        CallInject(0x6A09FF, CInfGame_GetAnimationBam_InjectPaperDollPrefix_asm, bytes6);
        CallInject(0x84710A, CAnimation5000_EquipArmor_SkipPrefixChange_asm, bytes6);
        //RelativeInject(0x74B64A, CUIButtonInventoryAppearance_GetAnimationVidCell_asm);

        COMMIT_vDataList;
    }

    if (pGameOptionsEx->bAnimation_EXTANIM ||
        pGameOptionsEx->bSound_PriorityCREOver2DA) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject (0x7FA8DE, CAnimation_SetAnimationType_PostHooks_asm, bytes6);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Restore Creature ATTACK[2-4] sounds
    if (pGameOptionsEx->bSound_RestoreCreatureAttackSounds) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes26[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90, 0x90,
                            0x90, 0x90, 0x90, 0x90, 0x90,
                            0x90 };
        uchar bytes_22nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                                 0x90, 0x90, 0x90, 0x90, 0x90,
                                 0x90, 0x90, 0x90, 0x90, 0x90,
                                 0x90, 0x90, 0x90, 0x90, 0x90,
                                 0x90, 0x90 };

        InitializeCriticalSection(&gCriticalSectionSetAnimationSequence);

        vDataList.push_back( Data(0x9098D7, sizeof(bytes_22nops), bytes_22nops) );
        CallInject(0x906228, CGameSpriteSwing_ForceProcessMessage_asm, bytes6);
        CallInject(0x9099A5, CGameSpriteSwing_SwapMessage_asm, bytes26);
        CallInject(0x941CC6, CGameSprite_DecodeSwingSound_Attack4_asm, bytes7);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Remove doubling sound in dialogs
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes_jmp[] = { 0xEB };
        vDataList.push_back( Data(0x4E7717, sizeof(bytes_jmp), bytes_jmp) );

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Fix SEQ_* -> SEQ_READY sound override
    if (pGameOptionsEx->bSound_SequenceOverrideFix) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes_6nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                                0x90 };
       
        //0x1000
        PointerInject(0x839BC2, 0x8399D7);  // SEQ_SHOOT          -> skip sequence override
        
        //0x2000
        CallInject(0x840A9A, CAnimation2000_SetAnimationSequence_SetShootSeq_asm, bytes6);

        //0x3000
        PointerInject(0x843604, 0x8432D5);  // SEQ_DAMAGE         -> skip sequence override
        PointerInject(0x84360C, 0x8432D5);  // SEQ_HEAD_TURN      -> skip sequence override
        PointerInject(0x84361C, 0x8432D5);  // SEQ_WALK           -> skip sequence override
        // SEQ_SHOOT - see "Replace Ankheg SEQ_SHOOT animation to SEQ_ATTACK_JAB" fix

        //0x9000
        PointerInject(0x82A20B, 0x82A076);  // SEQ_SHOOT          -> skip sequence override

        //0xA000
        PointerInject(0x82C88A, 0x82C71E);  // SEQ_SHOOT          -> skip sequence override

        //0xE000
        vDataList.push_back( Data(0x828019, sizeof(bytes_6nops), bytes_6nops) );    // SEQ_SHOOT
        vDataList.push_back( Data(0x827D63, sizeof(bytes_6nops), bytes_6nops) );    // SEQ_CONJURE
        vDataList.push_back( Data(0x827C9E, sizeof(bytes_6nops), bytes_6nops) );    // SEQ_CAST

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Replace Ankheg SEQ_SHOOT animation to patched SEQ_ATTACK_JAB
    if (pGameOptionsEx->bVideo_AnkhegRangeWeaponAnimFix) {

        uchar bytes_6nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90,
                                0x90 };
        PointerInject(0x843614, 0x8431B4);  // SEQ_SHOOT      -> SEQ_ATTACK_JAB
        vDataList.push_back( Data(0x843207, sizeof(bytes_6nops), bytes_6nops) );

        PointerInject(0x843624, 0x843156);  // SEQ_ATTACK_JAB -> SEQ_ATTACK_SLASH/SEQ_ATTACK_BACKSLASH
        
        COMMIT_vDataList;
    }

    
    ////////////////////////////////////////////////////////////////////////////
    // Remove hardcoded BG1 item sounds
    if (pGameOptionsEx->bSound_DisableHardcodedBG1ItemSounds) {
        uchar bytes_jmp[] = { 0xE9, 0x54, 0x02, 0x00, 0x00 }; // JMP 0x94198B
        vDataList.push_back( Data(0x941732, sizeof(bytes_jmp), bytes_jmp) );

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Disable Weapon Swing Sound Exclusive Mode
    // Enable attack sound for Darts and Touch weapons
    if (pGameOptionsEx->bSound_DisableWeaponSwingSoundExclusive) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x941B8C, CGameSprite_DecodeSwingSound_DisableExclusive_asm, bytes7);

        PointerInject(0x941F55, 0x941B6E);    // Darts          -> default
        PointerInject(0x941F65, 0x941B6E);    // Touch Weapon   -> default

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Self-Patched Effect Removing Fix
    if (pGameOptionsEx->bEff_SelfPatchedEffectRemovingFix) {

        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject(0x502A92, CGameEffect_Compare_BeforeRemove_asm, bytes7);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // CSound:Stop                  logging
    // CSoundMixer::ClearChannel    logging
    #ifdef _DEBUG
        if (pGameOptionsEx->bSound_NormalizePrintResname) {

            uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                             };
            uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                               0x90 };
            CallInject(0x9DDE38, CSound_Stop_Logging_asm, bytes6);
            CallInject(0x9DF4E6, CSound_Stop_Logging2_asm, bytes6);
            CallInject(0x9E0F12, CSoundMixer_ClearChannel_Logging_asm, bytes6);

            COMMIT_vDataList;
        }
    #endif


    ////////////////////////////////////////////////////////////////////////////
    // Disable Save/Load progress screen
    if (0) {
        uchar bzero[] =  { 0x00 };

        vDataList.push_back( Data(0x7CDC1C+1, sizeof(bzero), bzero) );
        vDataList.push_back( Data(0x7CDD11+1, sizeof(bzero), bzero) );
        vDataList.push_back( Data(0x7CDCB1+1, sizeof(bzero), bzero) );

        COMMIT_vDataList;        
    }


    ////////////////////////////////////////////////////////////////////////////
    // Enable spell hotkeys only for memorized spells, removes exploit
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };
        CallInject(0x7CE8E9, CScreenWorld_KeyHandle_CheckSpell_asm, bytes10);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Effect Opcodes: GenderChange Fix
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes2[] = { 0x98, 0x36}; // Cre.oDerived -> Cre.oBase
        vDataList.push_back( Data(0x51A3DD+2, sizeof(bytes2), bytes2) );

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Remove multi-target items/spell swap exploit on pause
    // see DETOUR_CCreatureObject::DETOUR_SetCurrentAction() for spell anti-cheat
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x69DC53, InventoryScreen_CheckCheats_asm, bytes6);

        COMMIT_vDataList;
    }


    
    ////////////////////////////////////////////////////////////////////////////
    // Disable 2DA sounds when playing SELECT_ACTION from .CRE
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject (0x8A28B1, CCreatureObject_PlaySound_ACTION2_asm, bytes6);

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Bow/Crossbow/Sling race bonus in Ranged(without ammo) mode fixes
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         };
        CallInject  (0x8CF199, CGameSprite_CheckCombatStatsWeapon_AddElfRacialThacBonus_asm, bytes5);
        CallInject  (0x8CF1F1, CGameSprite_CheckCombatStatsWeapon_AddHalflingRacialThacBonus_asm, bytes5);
        RelativeInject(0x8CE3EB, CGameSprite_CheckCombatStatsWeapon_SwapWeaponText_asm);

        COMMIT_vDataList;
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    // CAnimation1200::CAnimation1200() forget to set wAnimId
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        //  mov     edx, [ebp+animid]
        //  mov     eax, [ebp+CAnimation]
        //  mov     [eax+4], dx ; wAnimId
        uchar bytes_1200[] = { 0x8B, 0x55, 0x08, 0x8B, 0x85, 0x48, 0xFC,
                               0xFF, 0xFF, 0x66, 0x89, 0x50, 0x04
                             };
        vDataList.push_back( Data(0x82C8DF, sizeof(bytes_1200), bytes_1200) );

        COMMIT_vDataList;
    }

   
    ////////////////////////////////////////////////////////////////////////////
    // Block Area Transition in Combat
    if (pGameOptionsEx->bUI_BlockAreaTransitionCombat) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        CallInject  (0x92BA26, CGameSprite_LeaveArea_CheckBattleSong_asm, bytes6);
        CallInject  (0x92C894, CGameSprite_LeaveAreaName_CheckBattleSong_asm, bytes6);
        

        COMMIT_vDataList;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Morale Break icon
    if (pGameOptionsEx->bUI_MoraleBreakIcon) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject  (0x956101, CGameSprite_DoMoraleFailure_UpdateIcons_asm, bytes6);
        CallInject  (0x956384, CGameSprite_DoMoraleFailure_AddBerserk_asm, bytes7);
        CallInject  (0x9562BB, CGameSprite_DoMoraleFailure_AddRunAway_asm, bytes7);
        CallInject  (0x9561D0, CGameSprite_DoMoraleFailure_AddPanic_asm, bytes7);
        CallInject  (0x95656C, CGameSprite_EndMoraleFailure_RemoveIcon_asm, bytes6);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Robe Armor Sound
    if (pGameOptionsEx->bSound_RobeArmor) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         }; 

        CallInject (0x8702A1, CAnimation6400_SetArmorSoundRobe_asm, bytes5);
        CallInject (0x851CC3, CAnimation5000_SetArmorSoundRobe_asm, bytes5);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Simulacrum CreateInventoryItem(opcode=122) forget to set undroppable item flag
    if (!pGameOptionsEx->bDisableHiddenPatches) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject (0x52181C, CEffectCreateInventoryItem_Apply_asm, bytes6);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Disable resetting sequence to SEQ_READY in ClearActions() if:
    //  1) Going to walk
    //  2) Already have SEQ_HEAD_TURN
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes8[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90 };

        RelativeInject (0x40D1EC, CAIGroup_ClearActions_asm);
        CallInject   (0x8FA613, CCreativeObject_SetCurrentAction_SetREADY_asm, bytes8);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Increase SEQ_READY timer after weapon/shield was changed
    if (pGameOptionsEx->bAnimation_SEQ_READY_AfterWeaponChange) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        CallInject (0x5AA6BE, CItem_Equip_ResetIdleTimer_Shield_asm, bytes6);   //  shield
        CallInject (0x5AA831, CItem_Equip_ResetIdleTimer_OffHand_asm, bytes6);  //  off  hand weapon
        CallInject (0x5AA74E, CItem_Equip_ResetIdleTimer_MainHand_asm, bytes6); //  main hand weapon

        COMMIT_vDataList;
    }


    #ifdef _DEBUG
    {
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90};
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                            0x90, 0x90, 0x90, 0x90, 0x90 };

        //CallInject (0x8AD66A, CGameSprite_SetSequence_Log_asm, bytes7);

        //CallInject (0x9E011B, CSoundMixer_CleanUp_Log_asm, bytes7);
        //CallInject (0x9E068E, CSoundMixer_Initialize_Log_asm, bytes7);
        //CallInject (0x4CC589, CGameArea_OnActivation_Log_asm, bytes6);
        //CallInject (0x467E47, CCacheStatus_Update_Log_asm, bytes6);
        //CallInject (0x9E0417, CCacheStatus_Update_Log_asm, bytes6);
        //CallInject (0x9DF3BB, CSound_SetVolume_Log2_asm, bytes7);
        //CallInject (0x9DEB29, CSound_ResetVolume_Log2_asm, bytes6);

        //COMMIT_vDataList;
    }
    #endif


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Apply SPEED bonus from WSPECIAL.2DA
    if (pGameOptionsEx->bEngine_WSPECIAl_SPEED) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         }; 

        CallInject (0x905A7B, CGameSprite_Swing_FixWeaponSpeed_asm, bytes5);

        COMMIT_vDataList;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // keep MISLEAD invisibility when attacking
    //if (pGameOptionsEx->bEngine_InvisibilityRenderFix) {

    //    uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
    //                     }; 

    //    CallInject (0x8A72A3, CGameSprite_Render_CheckInvisibiltyEffects_asm, bytes5);
    //    CallInject (0x8A8C3F, CGameSprite_RenderMarkers_CheckInvisibiltyEffects_asm, bytes5);

    //    //90658B - in attack remove illusion !!!

    //    COMMIT_vDataList;
    //}


    if (pGameOptionsEx->bEngine_PartyBumpableInMoving) {

        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        
        // set bBumpable=1 for action opcode:
        // MoveToObject(22)
        // MoveToPoint(23)
        // ProtectObject(87)
        // Attack(3)
        uchar bytes_patch[] = { 0x83, 0xF9, 0x54, 0x74, 0x1C,
                                0x83, 0xF9, 0x16, 0x74, 0x17,
                                0x83, 0xF9, 0x17, 0x74, 0x12,
                                0x81, 0xF9, 0x57, 0x00, 0x00, 0x00, 0x74, 0x0A,
                                0x83, 0xF9, 0x03, 0x74, 0x05, 0x90
                              };
        vDataList.push_back( Data(0x89485E, sizeof(bytes_patch), bytes_patch) );

        // increase walk tries 4 > 32
        uchar bytes_WALK_RETRY[] = { 32 };
        vDataList.push_back( Data(0xAB72F6, sizeof(bytes_WALK_RETRY), bytes_WALK_RETRY) );

        // set bBumpable=1 for party/ally when moving
        CallInject(0x8947DC, CGameSprite_CheckBumpable_asm, bytes7);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Enemy Can Bump Invisible Party Member
    if (pGameOptionsEx->bEngine_EnemyCanBumpParty) {

        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00
                         }; 
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        uchar bytes_jump[] = { 0xEB };
        vDataList.push_back( Data(0x641861, sizeof(bytes_jump), bytes_jump) );
        vDataList.push_back( Data(0x89360E, sizeof(bytes_jump), bytes_jump) );

        CallInject(0x893C15, CGameSprite_ClearBumpPath_CheckParty_asm, bytes6);

        if (pGameOptionsEx->bEngine_EnemyCanBumpPartyUnmoveable)
            CallInject(0x894772, CGameSprite_CheckBumpable2_asm, bytes6);

        COMMIT_vDataList;
    }


    #ifdef _DEBUG
    // allow loading damaged savegame
    if (1) {
        uchar bytes1[] = { 0xEB } ;

        vDataList.push_back( Data(0x068714C, sizeof(bytes1), bytes1) );

        COMMIT_vDataList;
    }
    #endif


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  BG1 Casting Sound
    if (pGameOptionsEx->bSound_BG1CastingSound      ||
        pGameOptionsEx->bSound_BG2ClearCastingSound ||
        pGameOptionsEx->bSound_NWNCastingSound) {
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        PointerInject(0x93FC55, gCastVoiceFilePrefix);
        CallInject(0x93FC2B, CGameSprite_ApplyCastingEffect_PatchFilePrefix_asm, bytes7);
        CallInject(0x93FEC3, CGameSprite_ApplyCastingEffect_PatchAnimId_asm, bytes6);
        CallInject(0x941184, CGameSprite_ApplyCastingEffectPost_PatchFilePrefix_asm, bytes5);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    // GetDiskFreeSpaceA 2Tb bug
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        PointerInject(0xAA5374, FakeGetDiskFreeSpaceA_asm);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  x2 Render
    if (!pGameOptionsEx->bVideo_StretchGUI          &&
        !pGameOptionsEx->bVideo_StretchCursor       &&
        (widescreen == true && pGameOptionsEx->bVideo_StretchGUI_WidescreenMode) &&
         pGameOptionsEx->bVideo_StretchAll) {
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes10[]= { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90, 0x90 };

        gX2Render = true;

        // Screen resolution
        CallInject(0x9B6766, GetWindowHeightDX_asm, bytes7);
        CallInject(0x9B6770, GetWindowWidthAX_asm,  bytes6);

        CallInject(0x9EDAB3, GetWindowHeightAX_asm, bytes6);
        CallInject(0x9EDABC, GetWindowWidthCX_asm,  bytes7);

        CallInject(0x9EDE6D, GetWindowHeightCX_asm, bytes7);
        CallInject(0x9EDE77, GetWindowWidthDX_asm,  bytes7);

        CallInject(0x9F1834, GetWindowHeightCX_asm, bytes7);
        CallInject(0x9F1827, GetWindowWidthAX_asm,  bytes6);

        // Mouse
        CallInject(0x9A3625, LimitMouseXY_asm, bytes5);

        // glLineWidth(2)
        uchar bytesWidth[]   = { 0x00, 0x40 }; // 2.0 float
        vDataList.push_back( Data(0x9EE97B+3, sizeof(bytesWidth), bytesWidth) );

        CallInject(0x9EE838, CVidMode_DrawLine3d_TuneX1_asm, bytes7);
        CallInject(0x9EE851, CVidMode_DrawLine3d_TuneY1_asm, bytes7);

        uchar bytesStack[] = { 0x60 }; // new Stack
        vDataList.push_back( Data(0x9EE7ED+2, sizeof(bytesStack), bytesStack) );
        CallInject(0x9EEA53, CVidMode_DrawLine3d_TuneX0_asm, bytes6);
        CallInject(0x9EEA40, CVidMode_DrawLine3d_TuneY0_asm, bytes6);
        CallInject(0x9EE9EF, CVidMode_DrawLine3d_TuneX0_asm, bytes6);
        CallInject(0x9EE9DC, CVidMode_DrawLine3d_TuneY0_asm, bytes6);

        // glPointSize(2)
        vDataList.push_back( Data(0x9EF586+3, sizeof(bytesWidth), bytesWidth) );
        vDataList.push_back( Data(0x9EE697+3, sizeof(bytesWidth), bytesWidth) );
        vDataList.push_back( Data(0x9EF844+3, sizeof(bytesWidth), bytesWidth) );

        // Polygons
        //CallInject(0x9EFB62, SetLineWidth_asm, bytes10); not working

        // GameWorld Scissor Area
        CallInject(0x9EE41E, SetScissorOpenGL_asm, bytes6);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  0x7313 Animation Palette Fix
    //  MFIEG1B->MFISG1B, MFIEG2B->MFISG2B
    if (pGameOptionsEx->bVideo_7313_PaletteFix) {
        uchar byteS[]   = { 'S' }; // 'S'
        vDataList.push_back( Data(0xB4DD34+3, sizeof(byteS), byteS) );
        vDataList.push_back( Data(0xB4DD3C+3, sizeof(byteS), byteS) );
        vDataList.push_back( Data(0xB4DD44+3, sizeof(byteS), byteS) );

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Fix for ambient sound muted after QuickLoad
    if (!pGameOptionsEx->bDisableHiddenPatches) {
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        CallInject (0x68AE55, CInfGame_LoadGame_ReActivateArea_asm, bytes7);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Restore XP Limits for BGT
    if (pGameOptionsEx->bEngine_LimitXP) {
        // Savedata[5Ch]:
        // 1 BG1 TotSC
        // 3 XNewArea, used in NewGame Screen
        // 4 BG2 of ToB
        // 5 ToB of ToB
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        uchar byte1[]   = { 1 }; // 1 BG1 TotSC
        uchar byte4[]   = { 4 }; // 4 BG2 of ToB
        vDataList.push_back( Data(0x68648C+3, sizeof(byte4), byte4) );
        vDataList.push_back( Data(0x684069+3, sizeof(byte1), byte1) );
        vDataList.push_back( Data(0x686625+3, sizeof(byte1), byte1) );
        CallInject (0x63855C, CRuleTables_GetXPCap_asm, bytes6);

        COMMIT_vDataList;
    } else {    // undone patched savegames if bEngine_LimitXP disabled
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };

        CallInject (0x68643A, OldSaveGame_Undone_asm, bytes7);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  EAX/DirectSound3D emulation through DSOAL
    if (pGameOptionsEx->bSound_DSOAL) {
        void* ptr;
        DSOAL_DLL = LoadLibrary("dsoal-dsound.dll");

        if (DSOAL_DLL == NULL) {
            console.writef("dsoal-dsound.dll not found \n");
        } else {
            ptr = GetProcAddress(DSOAL_DLL, "DirectSoundCreate");
            if (ptr == NULL) {
                console.writef("DirectSoundCreate error in dsoal-dsound.dll \n");
            } else {
                PointerInject(0xAA5044, ptr);   // __imp_DirectSoundCreate
                COMMIT_vDataList;
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Freeze/Unfreeze sounds when game pause/unpaused
    if (pGameOptionsEx->bSound_FreezeOnPause) {
        uchar bytes5[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                         };
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };
        uchar bytes7[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90 };
        uchar bytes9[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90, 0x90, 0x90, 0x90 };

        uchar bytes[]    = {0x30}; // 16->30 new CVoice size
        vDataList.push_back( Data(0x9DFE8A+1, sizeof(bytes), bytes) );

        uchar DSBCAPS_High[]    = {0x01, 0x00}; // set DSBCAPS_GETCURRENTPOSITION2
        vDataList.push_back( Data(0x9DD6D5+5, sizeof(DSBCAPS_High), DSBCAPS_High) );
        vDataList.push_back( Data(0x9DD8A0+5, sizeof(DSBCAPS_High), DSBCAPS_High) );

        //CallInject (0x7D43B7,  CScreenWorld_Unpause_asm, bytes6);
        //CallInject (0x7D4443,  CScreenWorld_Pause_asm, bytes6);
        RelativeInject(0x7D43BD, CTimerWorld_HardUnPause_asm);  // CScreenWorld::TogglePauseGame
        RelativeInject(0x4CC592, CGameArea_OnActivation_asm);   // CGameArea::OnActivation
        CallInject (0x7CF925,  TimeStopEnded_asm, bytes6);
        //RelativeInject(0x7D7C4D, CTimerWorld_StartTime_asm);    // CScreenWorld::EndDialog

        RelativeInject(0x7D4449, CTimerWorld_HardPause_asm);    // CScreenWorld::TogglePauseGame
        RelativeInject(0x7C61DB, CTimerWorld_StopTime_asm);     // CScreenWorld::EngineDeActivated
        CallInject (0x5E3801,  CMessage_TimeStop_asm, bytes6);
        //RelativeInject(0x7D5F85, CTimerWorld_StopTime_asm);     // CScreenWorld::StartDialog

        uchar reorder[]    = {0x8B, 0x45, 0xFC, 0x50}; // mov     eax, [ebp-4]; push eax
        uchar reorder2[]   = {0x8B, 0x45, 0xFC, 0x50,  // mov     eax, [ebp-4]; push eax
                              0x90, 0x90, 0x90, 0x90}; // nops
        vDataList.push_back( Data(0x9E1864+6, sizeof(reorder),  reorder)  );
        vDataList.push_back( Data(0x9E1C26+5, sizeof(reorder2), reorder2) );
        CallInject (0x9E1864,  CSoundMixer_UpdateSoundList_RemoveFromPlayingNow_asm, bytes6);
        CallInject (0x9E1C26,  CSoundMixer_UpdateSoundListPriority_RemoveFromPlayingNow_asm, bytes5);

        // area mismatch 
        //vDataList.push_back( Data(0x9E194A+6, sizeof(reorder),  reorder)  );
        //vDataList.push_back( Data(0x9E1CFA+5, sizeof(reorder2), reorder2) );
        //CallInject (0x9E194A,  CSoundMixer_UpdateSoundList_MismatchAreaRemoveFromPlayingNow_asm, bytes6);
        //CallInject (0x9E1CFA,  CSoundMixer_UpdateSoundListPriority_MismatchAreaRemoveFromPlayingNow_asm, bytes5);

        RelativeInject(0x9E1B63, CSound_PlayWaiting_asm);
        RelativeInject(0x9DEB40, CSound_PlayWaiting_asm);
        RelativeInject(0x9DF3DE, CSound_PlayWaiting_asm);

        RelativeInject(0x4D1572, SoundPlay1CheckPause_asm); // SetDay
        RelativeInject(0x4D191C, SoundPlay1CheckPause_asm); // SetNight
        RelativeInject(0x4D263C, SoundPlay1CheckPause_asm); // SetDusk
        RelativeInject(0x4D2A76, SoundPlay1CheckPause_asm); // SetDusk
        RelativeInject(0x4D1D8B, SoundPlay1CheckPause_asm); // SetDawn
        RelativeInject(0x4D21C5, SoundPlay1CheckPause_asm); // SetDawn
        RelativeInject(0x6CE348, SoundPlay1CheckPause_asm); // AIUpdate(Lightning)
        RelativeInject(0x65975E, SoundPlay1CheckPause_asm); // SetRainSound
        RelativeInject(0x65954A, SoundPlay1CheckPause_asm); // SetRainSound
        RelativeInject(0x6596E7, SoundPlay1CheckPause_asm); // SetRainSound
        RelativeInject(0x659BA1, SoundPlay1CheckPause_asm); // SetWind
        RelativeInject(0x659E49, SoundPlay1CheckPause_asm); // SetWind
        RelativeInject(0x65A049, SoundPlay1CheckPause_asm); // SetWind

        CallInject (0x9DF571,  CVoice_CVoice_asm, bytes7);
        CallInject (0x9DFF5E,  CSoundMixer_TransferBuffer_asm, bytes6);

        COMMIT_vDataList;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////
    //  Set 44100Hz Sound Mixer Frequency
    //  Compatible with 1pp High Quality Music Mod
    if (pGameOptionsEx->bSound_44KhzMixer) {
        uchar bytes6[] = { 0xE8, 0x00, 0x00, 0x00, 0x00,
                           0x90 };

        uchar byte44[]    = {0x44, 0xAC}; // 44100
        uchar byteNOP2[]  = {0x90, 0x90}; // nop nop
        uchar byteNOP6[]  = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}; // nop nop
        uchar byteShift[]  = {0x8B, 0x55, 0xF4, 0x69, 0xC0, 0xF4, 0x01, 0x00, 0x00};
        uchar byteNoCheckRange[] = {0xEB, 0x14}; // jmp 9DDFBC

        vDataList.push_back( Data(0x9DD5FC, sizeof(byteNOP2), byteNOP2) ); // reinit frequency for reused csound
        vDataList.push_back( Data(0x9E094D+6, sizeof(byte44), byte44) ); // primary buffer + 3D 0x11
        vDataList.push_back( Data(0x9F95E4+3, sizeof(byte44), byte44) ); // static  buffer + 3D 0x12
        vDataList.push_back( Data(0x9DD60B, sizeof(byteNOP2), byteNOP2) ); // freq/500
        vDataList.push_back( Data(0x5AE340, sizeof(byteNOP2), byteNOP2) ); // freq/500
        vDataList.push_back( Data(0x9DD829, sizeof(byteNOP6), byteNOP6) ); // freq*500
        vDataList.push_back( Data(0x9DDB06, sizeof(byteNOP6), byteNOP6) ); // freq*500

        vDataList.push_back( Data(0x9DDF93, sizeof(byteShift), byteShift) ); // shift*500
        vDataList.push_back( Data(0x9DDFBF, sizeof(byteNOP6), byteNOP6) );   // freq*500
        vDataList.push_back( Data(0x9DDFA6, sizeof(byteNoCheckRange), byteNoCheckRange) );

        // Music patch disabled, 1pp HQ Music Mod has wrong .ACM headers,
        // they have 44100 body with 22050 header, there is no way to determinate proper format
        //vDataList.push_back( Data(0x9E0A4E+1, sizeof(byte44), byte44) ); // music primary buffer 0x01

        COMMIT_vDataList;
    }







    // show spell cooldown time
    // show modal state cooldown time
    // show spell protection remain levels


// End of patches
//////////////////////////////////////////////////////////////////////////////////////////////

    InitUserPatches(&vPatchList, &vDataList);

    for (vPatchItr = vPatchList.begin(); vPatchItr != vPatchList.end(); vPatchItr++) {
        vDataList = vPatchItr->GetData();
        bool bApply = true;
        for (unsigned int i = 0; i < vDataList.size(); i++) {
            bApply = CheckPatch(vDataList[i]);
            if (bApply) {
                if (vPatchItr->GetName()) {
                    LPCTSTR lpsz = "InitPatches(): patch '%s' applied\r\n";
                    console.writef(CONSOLEFORECOLOR_NORMAL, lpsz, vPatchItr->GetName());
                    L.timestamp();
                    L.appendf(lpsz, vPatchItr->GetName());
                }
            } else {
                LPCTSTR lpsz = "InitPatches(): patch '%s' not applied\r\n";
                console.writef(CONSOLEFORECOLOR_WARNING, lpsz, vPatchItr->GetName());
                L.timestamp();
                L.appendf(lpsz, vPatchItr->GetName());
                break;
            }
        }
        if (bApply) for_each(vDataList.begin(), vDataList.end(), ApplyPatch);
        vDataList.clear();
    }

    return;
}
