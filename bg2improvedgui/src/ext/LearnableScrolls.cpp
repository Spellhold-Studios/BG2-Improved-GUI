#include "LearnableScrolls.h"

#define GAMEOBJ     g_pChitin->pGame->m_GameObjectArrayHandler
typedef unsigned __int64    QWORD;

static char         gSTR_STORINT[8];
const  char* const  g_pPTRSTR_STORINT = gSTR_STORINT;



void __forceinline
ResNameCopy(char* const dst, const char* const src) {
    *(QWORD *)dst = *(QWORD *)src;
}


bool __forceinline
ResNameCmp(const char* const dst, const char* const src) {
    if (*(QWORD *)dst == *(QWORD *)src)
        return true;
    else
        return false;
}


#define NOGREEN     1
#define GREEN       0 
#define REDICON     "STORTINT"
#define GREENICON   "STORTIN3"


char __stdcall
ItemRedraw_GetColor(unsigned short PortraitNum, CItem& Item, bool UpdateResourceIcon) {     // return NOGREEN/GREEN
    Enum                CreObjID;
    CCreatureObject*    pCre;
    int                 SpellLevel;
    POSITION            pos;
    short               ItemType;
    char                nReturn;

    // default is Red
    if (UpdateResourceIcon) {
        ResNameCopy(gSTR_STORINT, REDICON);
    }

    if ( &Item == NULL) {               // empty button
        return NOGREEN;
    }

    ItemType = Item.GetType();
    if ( ItemType != 11) {              // 11 - Scrolls, PlanescapeEE also check Type=0 ("Misc")
        return NOGREEN;
    }

    CreObjID = g_pChitin->pGame->ePlayersPartyOrder[PortraitNum];
    if (GAMEOBJ.GetGameObjectShare(CreObjID, THREAD_ASYNCH, &pCre, -1) != OBJECT_SUCCESS)
        return NOGREEN;                 // error accessing creature
    
    //Object& CurObject = Cre->GetCurrentObject();
    Object* const pCurObject = & pCre->oDerived;
    bool CreMage =  pCurObject->HasActiveSubclass(CLASS_MAGE, TRUE) ||
                    pCurObject->HasActiveSubclass(CLASS_BARD, TRUE);
    if (!CreMage) {
        GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
        return NOGREEN;
    }

    // TODO: Check all abilities ?
    CEffect* Eff = Item.GetAbilityEffect(1, 0, pCre);   // original bg2 scrolls: 1 - second ability, 0 - first effect
    if (Eff == NULL) {
        GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
        return NOGREEN;
    }

    ITEM_EFFECT* ItemEffect = Eff->GetItemEffect();
    if (ItemEffect == NULL) {
        GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
        return NOGREEN;
    }

    if (Eff->effect.nOpcode == CEFFECT_OPCODE_LEARN_SPELL) {
        for (SpellLevel = 0; SpellLevel < 9; SpellLevel++) {
            pos = pCre->KnownSpellsWizard[SpellLevel].GetHeadPosition();
            while (pos != NULL) {
                const CreFileKnownSpell* const pKSpell = (CreFileKnownSpell *) pCre->KnownSpellsWizard[SpellLevel].GetNext(pos);
                if (ResNameCmp((char *) &pKSpell->name, (char *) &ItemEffect->resource)) {
                    nReturn = NOGREEN;
                    goto _FINISH;
                }
            }
        }

        // know spell not found
        if (UpdateResourceIcon) {
            ResNameCopy(gSTR_STORINT, GREENICON);   //Green icon bam
        }
        nReturn = GREEN;

    } else { // other effect opcode
        nReturn = NOGREEN;
    }

_FINISH:
    IEFree(ItemEffect);
    GAMEOBJ.FreeGameObjectShare(CreObjID, THREAD_ASYNCH, INFINITE);
    return nReturn;
}


char __stdcall
ItemRedraw_IsSchoolAvailable(CCreatureObject& Cre, CEffect& eff) {
    ResSplContainer resSpell;
    resSpell.SetResRef(eff.effect.rResource, TRUE, TRUE);
    resSpell.Demand();
    if (!resSpell.pRes) {
        resSpell.Release();
        return 1;                   // ResSplContainer error
    }

    unsigned int result = resSpell.GetExclusionFlags() & Cre.GetKitUnusableFlag();
    resSpell.Release();
    return !result;
}


void __declspec(naked)
ItemRedraw_Container_asm() {
__asm {
    test    eax,eax
    jz      redicon

    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    1                           ; Update icon
    push    [ebp-28h]                   ; CItem
    push    [ebp-1Ch]                   ; PortraitNum
    call    ItemRedraw_GetColor

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    mov     ecx, 070h                   ; alpha intensity
lexit:
    mov     edx, [ebp-33Ch]             ; stolen bytes
                                        ; original was
                                        ; mov     eax, [ebp-33Ch]
                                        ; mov     ecx, [eax+6]
    ret

redicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TINT'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, 'TNIT'
    mov     dword ptr gSTR_STORINT[4], eax

    mov     ecx, 0C0h                   ; alpha intensity
    xor     eax,eax
    jmp     lexit
}
}


void __declspec(naked)
ItemRedraw_Inventory_asm() {
__asm {
    test    eax,eax
    jz      redicon

    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    1                           ; Update icon
    push    [ebp-24h]                   ; CItem

    ;CScreenInventory [ebp-40h]
    ;nActivePlayerIdx CScreenInventory+28h
    mov     ecx, [ebp-40h]
    push    [ecx+28h]                   ; PortraitNum

    call    ItemRedraw_GetColor

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    mov     edx, 070h                   ; alpha intensity
lexit:
    mov     ecx, [ebp-3D8h]             ; stolen bytes
    ret

redicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TINT'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, 'TNIT'
    mov     dword ptr gSTR_STORINT[4], eax

    mov     edx, 0C0h                   ; alpha intensity
    xor     eax,eax
    jmp     lexit
}
}


// [eax+640h]  [eax+63b]:
//  0           1   red
//  1           0   no
//  1           1   green
void __declspec(naked)
ItemRedraw_StoreLeft_asm() {
__asm {
    ; eax = CButton

    push    eax

    mov     eax, [ebp-348h]
    cmp     dword ptr [eax+640h], 0
    jz      redicon 

    cmp     byte ptr [eax+63bh], 0
    jz      redicon                         ; really no any icon, just reset gSTR_STORINT to default

;greenicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TIN3'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, '3NIT'
    mov     dword ptr gSTR_STORINT[4], eax
    
    mov     edx, 070h                   ; alpha intensity
lexit:
    pop     eax
    mov     ecx, [ebp-348h]             ; stolen bytes
    ret

redicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TINT'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, 'TNIT'
    mov     dword ptr gSTR_STORINT[4], eax

    mov     edx, 0C0h                   ; alpha intensity
    jmp     lexit
}
}


// [eax+640h]  [eax+63b]:
//  0           1   red
//  1           0   no
//  1           1   green
void __declspec(naked)
ItemRedraw_StoreInventory_asm() {
__asm {
    push    eax
    
    mov     eax, [ebp-348h]
    cmp     dword ptr [eax+640h], 0
    jz      redicon 

    cmp     byte ptr [eax+63bh], 0
    jz      redicon 

;greenicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TIN3'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, '3NIT'
    mov     dword ptr gSTR_STORINT[4], eax
    mov     edx, 070h                   ; alpha intensity

lexit:
    pop     eax
    mov     ecx, [ebp-348h]             ; stolen bytes
    ret

redicon:
    mov     eax, 'ROTS'                     ; 'STOR'+'TINT'
    mov     dword ptr gSTR_STORINT[0], eax
    mov     eax, 'TNIT'
    mov     dword ptr gSTR_STORINT[4], eax
    mov     edx, 0C0h                   ; alpha intensity
    jmp     lexit
}
}


// [eax+640h]  [eax+63b]:
//  0           1   red
//  1           0   no
//  1           1   green
void __declspec(naked)
ItemRedraw_StoreLeftConfig_asm() {
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    mov     eax, [ebp-3Ch]              ; CButton
    cmp     dword ptr [eax+640h], 0
    jz      redicon                     ; already red

    push    1                           ; Update icon
    push    [ebp-84h]                   ; CItem

    ;CScreenStore [ebp-238h]
    ;nActivePlayerIdx CScreenStore+28h
    mov     ecx, [ebp-238h]
    push    [ecx+28h]                   ; PortraitNum

    call    ItemRedraw_GetColor
    cmp     al, NOGREEN
    jz      nocolor

;greenicon:
    mov     eax, [ebp-3Ch]
    mov     byte ptr [eax+63bh], 1
    jmp     lexit

nocolor:
    mov     eax, [ebp-3Ch]
    mov     byte ptr [eax+63bh], 0
    jmp     lexit

redicon:
    mov     byte ptr [eax+63bh], 1

lexit:
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     ecx, [ebp-104h]             ; stolen bytes
    ret
}
}



// [eax+640h]  [eax+63b]:
//  0           1   red
//  1           0   no
//  1           1   green
void __declspec(naked)
ItemRedraw_StoreInventoryConfig_asm() {
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    
    mov     eax, [ebp-54h]              ; CButton
    cmp     dword ptr [eax+640h], 0
    jz      redicon                     ; already red

    push    1                           ; Update icon
    push    [ebp-84h]                   ; CItem

    ;CScreenStore [ebp-238h]
    ;nActivePlayerIdx CScreenStore+28h
    mov     ecx, [ebp-238h]
    push    [ecx+28h]                   ; PortraitNum

    call    ItemRedraw_GetColor
    cmp     al, NOGREEN
    jz      nocolor

;greenicon:
    mov     eax, [ebp-54h]
    mov     byte ptr [eax+63bh], 1
    jmp     lexit

nocolor:
    mov     eax, [ebp-54h]
    mov     byte ptr [eax+63bh], 0
    jmp     lexit

redicon:
    mov     byte ptr [eax+63bh], 1

lexit:
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     eax, [ebp-104h]             ; stolen bytes
    ret
}
}


// [eax+640h]  [eax+63b]:
//  1           0   no
void __declspec(naked)
ItemRedraw_StoreIdentifyConfig_asm() {
__asm {
    mov     eax, [ebp-30h]              ; CButton
    mov     byte ptr [eax+63bh], 0

    mov     eax, [ebp-68h]              ; stolen bytes
    add     eax, 1000000Ch
    ret
}
}


// [eax+640h]  [eax+63b]:
//  0           1   red
//  1           0   no
//  1           1   green
void __declspec(naked)
ItemRedraw_StoreStealConfig_asm() {
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    
    mov     eax, [ebp-34h]              ; CButton
    cmp     dword ptr [eax+640h], 0
    jz      redicon                     ; already red

    push    1                           ; Update icon
    push    [ebp-78h]                   ; CItem

    ;CScreenStore [ebp-110h]
    ;nActivePlayerIdx CScreenStore+28h
    mov     ecx, [ebp-110h]
    push    [ecx+28h]                   ; PortraitNum

    call    ItemRedraw_GetColor
    cmp     al, NOGREEN
    jz      nocolor

;greenicon:
    mov     eax, [ebp-34h]
    mov     byte ptr [eax+63bh], 1
    jmp     lexit

nocolor:
    mov     eax, [ebp-34h]
    mov     byte ptr [eax+63bh], 0
    jmp     lexit

redicon:
    mov     byte ptr [eax+63bh], 1

lexit:
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     edx, [ebp-88h]              ; stolen bytes
    ret
}
}


// [eax+640h]  [eax+63b]:
//  1           0   no
void __declspec(naked)
ItemRedraw_StoreStealInventoryConfig_asm() {
__asm {
    push    eax
    mov     eax, [ebp-4Ch]              ; CButton
    mov     byte ptr [eax+63bh], 0
    pop     eax

    mov     edx, [ebp-88h]              ; stolen bytes
    ret
}
}




void __declspec(naked)
ItemRedraw_EnableWriteButton_asm() {
__asm {
    pop     eax     ; will return other way
    
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0                           ; No update icon
    push    [ebp-20h]                   ; CItem
    ;CScreenInv [ebp-0BCh]
    ;nActivePlayerIdx CScreenInv+28h
    mov     ecx, [ebp-0BCh]
    push    [ecx+28h]                   ; PortraitNum
    call    ItemRedraw_GetColor

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    not     eax                         ; invert of ItemRedraw_GetColor
    and     al, 00000001b               

    test    al,al
    jz      lexit                       ; if already memorized

    push    [ebp-38h]                   ; eff
    push    [ebp-44h]                   ; Cre
    call    ItemRedraw_IsSchoolAvailable

lexit:
    mov     byte ptr [ebp-18h], al      ; stolen bytes
    mov     eax, 745B1Fh                ; ret adr
    jmp     eax
}
}

