#include "InteractiveJournal.h"
#include "effopcode.h"

#define GAME g_pChitin->pGame

void __stdcall
JournalEvent (WORD Mode, unsigned long Quest_string_num, CStrRef& TitleRef)
{
    bool    found_existing = false;

    if (Mode == 1) { // 1 - Open Quests only
        int posEndLine;
        CStrRef  QuestStrRef;

        g_pChitin->m_TlkTbl.GetTlkString(Quest_string_num, QuestStrRef);

        posEndLine = QuestStrRef.text.FindOneOf("\n\r");
        if (posEndLine < 1024) {     // stupid overflow protection 
            unsigned long hash = crc32(*((const char **) &QuestStrRef.text), posEndLine - 1);
            unsigned long hash_1 = UPDCRC32(0x01, hash);    // open quests
            unsigned long hash_2 = UPDCRC32(0x02, hash);    // done quests
            POSITION        pos, current_pos;

            // clear array
            gQuestIndex = 0;

            for (int nChapter=1; nChapter <= 10; nChapter ++) {
                CPtrList& ChapterList  = *( (CPtrList *) GAME->m_CGameJournal.u0.GetAt(nChapter) );

                pos = ChapterList.GetHeadPosition();
                while (pos != NULL) {
                    current_pos = pos;
                    CJournalEntry* pJournalEntry = (CJournalEntry *) ChapterList.GetNext(pos);

                    if (!(pJournalEntry->m_wType & 0x01) &&    // open/close quests only
                        !(pJournalEntry->m_wType & 0x02)) {
                            continue;
                    }

                    if (pJournalEntry->m_strText == Quest_string_num ) { // exclude self
                            continue;
                    }

                    CStrRef  TempQuestStrRef;

                    g_pChitin->m_TlkTbl.GetTlkString(pJournalEntry->m_strText, TempQuestStrRef);

                    posEndLine = QuestStrRef.text.FindOneOf("\n\r");
                    if (posEndLine >= 1024)     // stupid overflow protection 
                        continue;

                    AddQuest(current_pos, pJournalEntry, pJournalEntry->m_wType, (const char **) &TempQuestStrRef.text, posEndLine);
                }
            }

            for (int i = 0; i < gQuestIndex ;i++) {
                if (gQuestBuf[i].hash == hash_1 ||
                    gQuestBuf[i].hash == hash_2) {
                    found_existing = true;
                }
            }
            
            // clear array
            gQuestIndex = 0;
        } else {
            // error mode, don't print "new quest"
            found_existing = true;
        }
    }

    STRREF Newref;
    switch (Mode)
    {
    case 2:
        Newref = 6564;        // ~Quest has been completed~ [FOLSTART]
	    break;
    case 1:
        if (found_existing) {
            Newref = 6565;    // ~Quest journal has been updated~ [FOLSTART]
        } else {
            Newref = 6567;    // ~New quest has been added to journal~ [FOLSTART]
        }
        break;
    case 4:
        Newref = 6566;        // ~Story journal has been updated~ [FOLSTART]
        break;
    default :
        Newref = 11359;       // orig text
        break;
    }

    g_pChitin->m_TlkTbl.GetTlkString(Newref, TitleRef);

    if (TitleRef.sound.wav.pResWav) {
        if ( !TitleRef.sound.bLoop ) {
            TitleRef.sound.SetFireForget(TRUE);
        }

        TitleRef.sound.SetChannel(14, GAME->m_pLoadedAreas[GAME->m_VisibleAreaIdx]);
        TitleRef.sound.Play(FALSE);
    }

}

STRREF __stdcall
GetItemEvent (CItem& Item) {
    STRREF id;

    Item.itm.pRes->Demand();
    {
        ItmFileHeader* pFile = Item.itm.pRes->pFile;
        if (Item.dwFlags & CREITEM_IDENTIFIED)
            id= pFile->m_strrefIdentifiedName;
        else
            id= pFile->m_strrefGenericName;
    }
    Item.itm.pRes->Release();
    return id;
}


STRREF __stdcall
ItemEvent_TakeItemReplace (CGameAIBase& GameSprite) {
    STRREF id = 0;
    CItem   Item;

    Item.LoadResource(ResRef(GameSprite.currentAction.sName2), TRUE);
    if (Item.itm.bLoaded && Item.itm.name.IsValid()) {
        Item.itm.pRes->Demand();
        {
            id= Item.itm.pRes->pFile->m_strrefIdentifiedName;
        }
        Item.itm.pRes->Release();
    }
    return id;
}


STRREF __stdcall
RevealMapEvent (CGameAIBase& GameSprite) {
    unsigned int Index = 0;

    ResRef resref = ResRef(GameSprite.currentAction.sName1);
    BOOL found = GAME->m_CWorldMap.GetAreaIndex(0, &resref, &Index);

    if (found){ 
        CWorldMapArea& Area = g_pChitin->pGame->m_CWorldMap.m_ppAreas[0][Index];
        return Area.m_strLabel;
    } else {
        return 0;
    }

}


STRREF __stdcall
DropItemEvent2 (CMessageHandler& Msg) {
    STRREF id = 0;
    CItem   Item;

    Item.LoadResource( (ResRef& )Msg.u106, TRUE);
    if (Item.itm.bLoaded && Item.itm.name.IsValid()) {
        Item.itm.pRes->Demand();
        {
            id= Item.itm.pRes->pFile->m_strrefIdentifiedName;
        }
        Item.itm.pRes->Release();
    }
    return id;
}


void __stdcall
ItemEvent_LostInArea (CItem& Item, CGameObject& Obj) {
    if (Obj.nObjType == 0x61)  {// CGameAIBase::PlaceItem() was called from Area script
        GAME->FeedBack(4, GetItemEvent(Item));
    }
}


void __stdcall
AddToThacRollMessage (CCreatureObject& Cre, int TargetAC, int SrcTHAC0, int ThacRoll, BOOL Hit, int HitRoll, IECString& DstString, IECString& SrcString ) {
    IECString AC;
    IECString prefix;
    bool crit;

    CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;
    if ( HitRoll >= (20 - cds.criticalHitBonus) )
        crit = true;
    else
        crit = false;

    if (HitRoll != 1 && crit == false) {
        if (Hit)
            if ((SrcTHAC0 - TargetAC) == ThacRoll)
                prefix = " >= ";
            else
                prefix = " > ";
        else
            prefix = " < ";

        if (TargetAC >= 0) {
                AC.Format("%d - %d = %d",   SrcTHAC0, TargetAC, SrcTHAC0 - TargetAC); // "20 - 8 = 12"
        } else {
                AC.Format("%d - (%d) = %d", SrcTHAC0, TargetAC, SrcTHAC0 - TargetAC); // "20 -(-8) = 28"
        }

        DstString += prefix;    // "Attack Roll 10 - 4 = 6" " > "
        DstString += AC;        // "Attack Roll 10 - 4 = 6" " > " "20 - 8 = 12"
        DstString += SrcString; // "Attack Roll 10 - 4 = 6" " < " "20 - 8 = 12" " : Miss"
    } else
    if (HitRoll == 1) { // crit miss
        //IECString sText = GetTlkString(14643);  // ~Attack Roll ~
        //DstString = sText + "1";                // ~Attack Roll 1~
        //sText = GetTlkString(16463);            // ~Critical Miss~
        //DstString += " : ";
        //DstString += sText;                     // ~Attack Roll 1 : Critical Miss"~

    } else
    if (crit) {         // crit hit
        IECString sText = GetTlkString(14643);  // ~Attack Roll ~
        AC.Format("%d", HitRoll);
        DstString = sText;                      
        DstString += AC;                        // ~Attack Roll 20~
        sText = GetTlkString(16462);            // ~Critical Hit~
        DstString += " : ";
        DstString += sText;                     // ~Attack Roll 20 : Critical Hit"~
    }
}


void __stdcall
AMOUNT_damage (CEffectDamage& eff, IECString& DestString, unsigned int RefTextID, CCreatureObject& CreTarget) {
    IECString    String;
    int          DiffMultiply;
    unsigned int nDamageType     = eff.effect.nParam2 & 0xFFFF0000;
    int          nDamageBehavior = eff.effect.nParam2 & 0xFFFF;
    float        multiply;

    // nParam6 roll amount
    // nParam1 final amount
    // nParam3 global percentage
    // nParam5 resist percentage
    int     RollDamage  = eff.effect.nParam6;
    int     FinalDamage = eff.effect.nParam1;

    //Enum    creTargetid = eff.eSource;
    //CCreatureObject* CreSrc = NULL;
    //char nResult;
    //do {
    //    nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(id, THREAD_ASYNCH, &CreSrc, INFINITE);
    //} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);
    //if (nResult == OBJECT_SUCCESS) {
    //    ;
    //    g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(id, THREAD_ASYNCH, INFINITE);
    //}

    if (FinalDamage != RollDamage &&
        RefTextID != 5199 &&                             // "<DAMAGEE> was immune to my damage."
        (GAME->m_GameOptions.m_nEffectTextLevel & 1) ) { // "To Hit Rolls" GUI setting
        if (pGameOptionsEx->bEngineExpandedStats &&
            nDamageBehavior == EFFECTDAMAGE_BEHAVIOR_NORMAL &&
            eff.effect.nParam3 != 0) {
                IECString Str;
                Str.Format("%d * %d%%", RollDamage, eff.effect.nParam3);
                String += Str;
        }

        if (eff.effect.nParam5 != 0) {  // resist 
                IECString Str;
                Str.Format("%d - %d%%", RollDamage, eff.effect.nParam5);
                String += Str;
        }

        //if (GAME->GetPartyMemberSlot(creTarget.e) != -1) {
        if (g_pChitin->cNetwork.bSessionOpen) {
            DiffMultiply = GAME->m_GameOptions.m_nMPDifficultyMultiplier;
        } else {
            DiffMultiply = GAME->m_GameOptions.m_nDifficultyMultiplier;
        }

        if (DiffMultiply != 0 &&
            CreTarget.o.EnemyAlly <= EA_CONTROLLEDFORCEADD) {   // only on party&co. multiply
            IECString Str;
            switch (DiffMultiply) {
            case -50:
                Str = "* 0.25";
                break;
            case -25:
                Str = "* 0.75";
                break;
            case 50:
                Str = "* 1.5";
                break;
            case 100:
                Str = "* 2";
                break;
            default:
                if (DiffMultiply > 0) {
                  multiply = 1 + ((float)DiffMultiply / 100);
                  Str.Format("%.2f", multiply);     // "x.yy"
                }
                Str = "* " + Str;
                break;
            }

            if (String.GetLength() == 0) {
                String.Format("%d", RollDamage);
                DestString += " = "  + String + " " +Str;
            } else {
                DestString += " = (" + String + ") " +Str;
            }
        }
        else {
            if (String.GetLength() != 0) {
                DestString += " = " + String;
            }
        }
    }
}



#define APPEND_DMGTYPE(x) \
    g_pChitin->m_TlkTbl.GetTlkString(x, TlkString); \
    DestString += " ";                              \
    DestString += TlkString.text;                   \
    break;


void __stdcall
TYPE_damage (CEffectDamage& eff, IECString& DestString) {
    CStrRef      TlkString;

    unsigned int nDamageType     = eff.effect.nParam2 & 0xFFFF0000;
    int          nDamageBehavior = eff.effect.nParam2 & 0xFFFF;

    switch (nDamageType) {
        case DAMAGETYPE_ACID:           APPEND_DMGTYPE(9517)
        case DAMAGETYPE_COLD:           APPEND_DMGTYPE(9515)
        case DAMAGETYPE_CRUSHING:       APPEND_DMGTYPE(9512)
        case DAMAGETYPE_STUNNING:       APPEND_DMGTYPE(9521)
        case DAMAGETYPE_PIERCING:       APPEND_DMGTYPE(9510)
        case DAMAGETYPE_SLASHING:       APPEND_DMGTYPE(9511)
        case DAMAGETYPE_ELECTRICITY:    APPEND_DMGTYPE(9514)
        case DAMAGETYPE_FIRE:           APPEND_DMGTYPE(9509)
        case DAMAGETYPE_POISON:         APPEND_DMGTYPE(9518)
        case DAMAGETYPE_MAGIC:          APPEND_DMGTYPE(9516)
        case DAMAGETYPE_MISSILE:        APPEND_DMGTYPE(9513)
        case DAMAGETYPE_MAGICFIRE:      APPEND_DMGTYPE(9519)
        case DAMAGETYPE_MAGICCOLD:      APPEND_DMGTYPE(9520)
        default:
            break;
    }
}


void __declspec(naked)
JournalEvent_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    lea     eax, [ebp-278h]
    push    eax                  ; Title CStrRef
    push    [ebp+0Ch]            ; Quest #string_num
    push    [ebp+10h]            ; Mode
    call    JournalEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    ret
}
}

void __declspec(naked)
GainItemEvent_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-12Ch]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-0D8h], ecx
    ret
}
}

void __declspec(naked)
GainItemEvent2_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-14h]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-044h], ecx
    ret
}
}

void __declspec(naked)
GainItemEvent3_asm() {
__asm
{
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-0Ch]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    pop     ecx     ; saved eax
    mov     ecx, [ecx+42BAh]     ; Stolen bytes
    mov     [ebp-3Ch], ecx
    ret
}
}

void __declspec(naked)
GainItemEvent4_asm() {
__asm
{
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-12Ch]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    pop     ecx     ; saved eax
    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-0F4h], ecx
    ret
}
}


void __declspec(naked)
LostItemEvent_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp+8]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-098h], ecx
    ret
}
}

void __declspec(naked)
LostItemEvent2_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-118h]           ; CGameAIBase
    call    ItemEvent_TakeItemReplace

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-0F0h], ecx
    ret
}
}

void __declspec(naked)
LostItemEvent3_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-0BCh]         ; CGameObject
    push    [ebp+8]            ; CItem
    call    ItemEvent_LostInArea

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     eax, [ebp-0BCh]     ; Stolen bytes
    ret
}
}


void __declspec(naked)
RevealMapEvent_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-34h]           ; CGameAIBase
    call    RevealMapEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [ecx+42BAh]     ; Stolen bytes
    mov     [ebp-2Ch], ecx
    ret
}
}

void __declspec(naked)
DropItemEvent1_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-34h]            ; CItem
    call    GetItemEvent

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [ecx+42BAh]     ; Stolen bytes
    mov     [ebp-8Ch], ecx
    ret
}
}

void __declspec(naked)
DropItemEvent2_asm() {
__asm
{
    ;push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-118h]            ; CMessage
    call    DropItemEvent2

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    ;pop     eax

    mov     ecx, [edx+42BAh]     ; Stolen bytes
    mov     [ebp-104h], ecx
    ret
}
}

void __declspec(naked)
ThacRollMessage_asm() {
__asm
{
    #define _CreCds      -0F0h
    #define _TargetCds   -0F4h
    #define _String1     -0E0h
    #define _WeaponMode  -30h
    #define _ThacRoll    -28h
    #define _Hit         -10h
    #define _TotalThac   -14h
    #define _Cre         -294h
    #define _ThacMinusAC -34h
    #define _CGameSprite_FeedBack 8FAE5Ah

    cmp     word ptr [ebp+_ThacRoll], 1     // Roll = 1 ?
    jz      ThacRollMessage_asm_Skip        // don't show hit roll detail because ~Critical Miss~ printed before

    mov     edx, [ebp+_CreCds]
    movsx   eax, word ptr [edx+10h]
    mov     ecx, [ebp+_TargetCds]
    movsx   edx, word ptr [ecx+6]
    push    edx             ; Target AC
    push    eax             ; Source THAC0

    lea     edx, [ebp+_String1]
    push    edx             ; String
    movsx   eax, word ptr [ebp+_WeaponMode]
    push    eax             ; WeaponMode
    push    0FFFFFFFFh      ; RefTextID
    movsx   ecx, [ebp+_ThacRoll]
    push    ecx             ; ThacRoll
    mov     edx, [ebp+_Hit]
    push    edx             ; Hit
    movsx   eax, [ebp+_TotalThac]
    push    eax             ; TotalThac
    push    4               ; Opcode
    mov     ecx, [ebp+_Cre]
    mov     eax, _CGameSprite_FeedBack 
    call    eax

    pop     eax             ; align stack
    pop     eax

    ret

ThacRollMessage_asm_Skip:
    ret
}
}

void __declspec(naked)
ThacRollMessage2_asm() {
__asm
{
    push    [esp+4]    ; Src  IECString
    push    ecx        ; Dest IECString
    push    [ebp+14h]  ; Hit Roll
    push    [ebp+10h]  ; Hit/Missed
    push    [ebp+0Ch]  ; ThacRoll
    push    [ebp+24h]  ; SrcTHAC0 (stack outside)
    push    [ebp+28h]  ; TargetAC (stack outside)
    push    [ebp-0E34h]; Cre
    call    AddToThacRollMessage

    ret
}
}


void __declspec(naked)
TYPE_damage_asm() {
__asm
{
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    lea     eax, [ebp-0ACh]
    push    eax             ; IECString 
    push    [ebp-190h]      ; CEffectDamage
    call    TYPE_damage

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    lea     ecx, [ebp-0C8h]      ; Stolen bytes
    ret
}
}

void __declspec(naked)
AMOUNT_damage_asm() {
__asm
{
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp+8]         ; Cre
    push    [ebp-14h]       ; RefTextID
    lea     eax, [ebp-090h]
    push    eax             ; IECString 
    push    [ebp-190h]      ; CEffectDamage
    call    AMOUNT_damage

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     dl, [ecx+41Ch]      ; Stolen bytes
    ret
}
}
