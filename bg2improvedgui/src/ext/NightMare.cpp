#include "NightMare.h"
#include "InfGameCore.h"
#include "detoursext.h"
#include "EffectOpcode.h"
#include "ObjectStats.h"

static bool NightModeActive;


BOOL static __stdcall
CheckMorale (CCreatureObject& Cre) {
    ////////////////
    // BG2EE
    if (NightModeActive &&
        pGameOptionsEx->bNightmareBonusNoMoraleBreak &&
        Cre.BaseStats.bNightmareModeStats &&
        Cre.o.EnemyAlly >= EA_GOODCUTOFF) { // Neutral/Enemy
        return TRUE;                        // never break morale
    } else {
        return FALSE;
    }
    ////////////////
}



// IWD2 Heart of Fury
// HP +300% (+200% summons)
// Cre.animation.pAnimation->nMovementRateCurrent = (Cre.animation.pAnimation->nMovementRateDefault * 1.3) + 1;
// all 6 stats +10
// levels +12
// ...challenge rating +10
// ...Gold +75

// IWD1 Heart of Fury
// XP = 200% + 1000
// THAC0 -= 5
// HP +300% (+200% summons)
// attacks per round += 1
// saves += 1
// Cre.animation.pAnimation->nMovementRateCurrent = (Cre.animation.pAnimation->nMovementRateDefault * 1.3) + 1;
// levels +12
// ...Gold +75

void static
NightmareUpgrade(CCreatureObject& Cre) {
    if (Cre.BaseStats.dwFlags & 0x400000)  // Skip Party & Co.
        return;

        #ifdef _DEBUG
            IECString name = GetTlkString(Cre.BaseStats.shortNameStrRef);
            console.writef("Upgrade %s \n", (LPCTSTR)name);
        #endif

    unsigned char EnemyAlly = Cre.o.EnemyAlly;

    Cre.BaseStats.bNightmareModeStats = true;

    int base_XP = Cre.BaseStats.XPForKill;
    if (base_XP > 0 && pGameOptionsEx->bNightmareBonusXP) {
        // nHP = (2 * HP) + 1000
        Cre.BaseStats.XPForKill = (base_XP * pGameOptionsEx->bNightmareBonusXP_Multiplier) + pGameOptionsEx->bNightmareBonusXP_Bonus;
    }

    Cre.BaseStats.THAC0          -= pGameOptionsEx->bNightmareBonusTHAC0;       // 5
    Cre.BaseStats.ArmorClass     -= pGameOptionsEx->bNightmareBonusArmorClass;  // 11
    Cre.BaseStats.ArmorClassBase -= pGameOptionsEx->bNightmareBonusArmorClass;  // 11

    // Attacks Per Round increased in DETOUR_CDerivedStats::DETOUR_Reload() or CDerivedStats_Reload_Nightmare()
    // Rest spawn amount increased in GetSpawnAmount()
    // Avoid break morale in CheckMorale()

    if (pGameOptionsEx->bNightmareBonusMovementRate)
        Cre.animation.pAnimation->nMovementRateCurrent = (uchar) ((Cre.animation.pAnimation->nMovementRateDefault * 1.3) + 1);

    SubWithLimits(Cre.BaseStats.saveDeath,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    SubWithLimits(Cre.BaseStats.saveWands,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    SubWithLimits(Cre.BaseStats.savePoly,   pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5 
    SubWithLimits(Cre.BaseStats.saveBreath, pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5 
    SubWithLimits(Cre.BaseStats.saveSpell,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5

    if (EnemyAlly > EA_CHARMED) {
        Cre.BaseStats.currentHP = (Cre.BaseStats.currentHP * pGameOptionsEx->bNightmareBonusHP_Multiplier) +
                                    pGameOptionsEx->bNightmareBonusHP_Bonus; // Good/Neutral/Enemy: 300% + 80
        Cre.BaseStats.maxHP     = (Cre.BaseStats.maxHP     * pGameOptionsEx->bNightmareBonusHP_Multiplier)  +
                                    pGameOptionsEx->bNightmareBonusHP_Bonus;
    } else {
        Cre.BaseStats.currentHP = (Cre.BaseStats.currentHP * pGameOptionsEx->bNightmareBonusHPSummon_Multiplier) +
                                    pGameOptionsEx->bNightmareBonusHPSummon_Bonus; // Summons: 200% + 20
        Cre.BaseStats.maxHP     = (Cre.BaseStats.maxHP     * pGameOptionsEx->bNightmareBonusHPSummon_Multiplier) +
                                    pGameOptionsEx->bNightmareBonusHPSummon_Bonus;
    }

    if (Cre.BaseStats.gold > 0)
        Cre.BaseStats.gold += pGameOptionsEx->bNightmareBonusGold;

    if (Cre.BaseStats.levelPrimary)
        AddWithLimits(Cre.BaseStats.levelPrimary,   pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12
    if (Cre.BaseStats.levelSecondary)
        AddWithLimits(Cre.BaseStats.levelSecondary, pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12
    if (Cre.BaseStats.levelTertiary)
        AddWithLimits(Cre.BaseStats.levelTertiary,  pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12
}


void static
NightmareDowngrade(CCreatureObject& Cre) {
    if (Cre.BaseStats.dwFlags & 0x400000)  // Skip Party & Co.
        return;

        #ifdef _DEBUG
            IECString name = GetTlkString(Cre.BaseStats.shortNameStrRef);
            console.writef("Downgrade %s \n", (LPCTSTR)name);
        #endif

    unsigned char EnemyAlly = Cre.o.EnemyAlly;

    Cre.BaseStats.bNightmareModeStats = false;

    int base_XP = Cre.BaseStats.XPForKill;
    if (base_XP > pGameOptionsEx->bNightmareBonusXP_Bonus && pGameOptionsEx->bNightmareBonusXP) {
        // nHP = (HP-1000)/2
        Cre.BaseStats.XPForKill = (base_XP - pGameOptionsEx->bNightmareBonusXP_Bonus) / pGameOptionsEx->bNightmareBonusXP_Multiplier;
    }

    Cre.BaseStats.THAC0          += pGameOptionsEx->bNightmareBonusTHAC0;       // 5
    Cre.BaseStats.ArmorClass     += pGameOptionsEx->bNightmareBonusArmorClass;  // 11
    Cre.BaseStats.ArmorClassBase += pGameOptionsEx->bNightmareBonusArmorClass;  // 11

    if (pGameOptionsEx->bNightmareBonusMovementRate)
        Cre.animation.pAnimation->nMovementRateCurrent = Cre.animation.pAnimation->nMovementRateDefault;

    AddWithLimits(Cre.BaseStats.saveDeath,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    AddWithLimits(Cre.BaseStats.saveWands,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    AddWithLimits(Cre.BaseStats.savePoly,   pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    AddWithLimits(Cre.BaseStats.saveBreath, pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5
    AddWithLimits(Cre.BaseStats.saveSpell,  pGameOptionsEx->bNightmareBonusSavingThrows, 0, 20); // 5

    if (EnemyAlly > EA_CHARMED) {
        AssignWithLimits(Cre.BaseStats.currentHP,
            (Cre.BaseStats.currentHP - pGameOptionsEx->bNightmareBonusHP_Bonus)/
             pGameOptionsEx->bNightmareBonusHP_Multiplier, 1, 32767);  // Good/Neutral/Enemy
        AssignWithLimits(Cre.BaseStats.maxHP    ,
            (Cre.BaseStats.maxHP     - pGameOptionsEx->bNightmareBonusHP_Bonus)/
             pGameOptionsEx->bNightmareBonusHP_Multiplier, 1, 32767);
    } else {
        AssignWithLimits(Cre.BaseStats.currentHP,
            (Cre.BaseStats.currentHP - pGameOptionsEx->bNightmareBonusHPSummon_Bonus)/
             pGameOptionsEx->bNightmareBonusHPSummon_Multiplier, 1, 32767);  // Summons
        AssignWithLimits(Cre.BaseStats.maxHP    ,
            (Cre.BaseStats.maxHP     - pGameOptionsEx->bNightmareBonusHPSummon_Bonus)/
            pGameOptionsEx->bNightmareBonusHPSummon_Multiplier, 1, 32767);
    }

    if (Cre.BaseStats.gold > (uint)pGameOptionsEx->bNightmareBonusGold)
        Cre.BaseStats.gold -= pGameOptionsEx->bNightmareBonusGold;

    if (Cre.BaseStats.levelPrimary)
        SubWithLimits(Cre.BaseStats.levelPrimary,   pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12
    if (Cre.BaseStats.levelSecondary)
        SubWithLimits(Cre.BaseStats.levelSecondary, pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12
    if (Cre.BaseStats.levelTertiary)
        SubWithLimits(Cre.BaseStats.levelTertiary,  pGameOptionsEx->bNightmareBonusLevels, 1, 50); // 12

}


// add creature to game
void static __stdcall
CCreatureObject_Unmarshal(CCreatureObject& Cre, DWORD* Stack[]) {
    bool NeutralOrEnemy;

    ///////////////////////////////////
    // BG2EE
    if (NightModeActive) {
        unsigned char EnemyAlly = Cre.o.EnemyAlly;

        if (EnemyAlly == EA_PC) {               // Party
            Cre.BaseStats.dwFlags |= 0x400000;  // "Ignore Nightmare Stats" Flag
        }

        if (!(Cre.BaseStats.dwFlags & 0x400000)) {
            if (pGameOptionsEx->bNightmarePartySummon == FALSE) {
                if (Stack[1] == (DWORD*) 0x882458) {               // called from CCreatureObject::CCreatureObject()
                    DWORD* StackPrev = Stack[0];
                    if (pGameOptionsEx->bEngineSpellTurningFix) {
                        StackPrev = (DWORD*) StackPrev[0];    // spinup one level due detour to tobex
                    }

                    if (StackPrev[1] == 0x5193AD ||  // called from CEffectSummon::Apply()
                        StackPrev[1] == 0x5227EA) {  // CEffectSummonMonsters::Apply()
                        DWORD* ebp = (DWORD*) StackPrev[0];
                        CCreatureObject* SourceCre = (CCreatureObject *) ebp[2]; // SourceCre = [ebp+8]
                        if (SourceCre->o.EnemyAlly <= EA_GOODCUTOFF) {
                            // Summon from Party, skip new stats
                            Cre.BaseStats.dwFlags |= 0x400000;
                        }
                    }
                }
            }
        }

        if (!(Cre.BaseStats.dwFlags & 0x400000)) {
            // check PDIALOG.2da for joinable char
            CRuleTable* pRuleTable = NULL;
            pRuleTable = &g_pChitin->pGame->PDIALOG;
            int nCol = 0;
            for (int nRow = 0; nRow < pRuleTable->nRows; nRow++) {
                IECString& rowHeader = *(pRuleTable->pRowHeaderArray + nRow);
                if (!rowHeader.CompareNoCase(Cre.szScriptName)) {
                    Cre.BaseStats.dwFlags |= 0x400000;  // mark joinable
                }
            }
        }

        if (Cre.BaseStats.dwFlags & 0x400000) { // Skip Party & Co.
            return;
        }

        if (EnemyAlly >  EA_CHARMED ||      // Good/Neutral/Enemy
            EnemyAlly == EA_CONTROLLED) {   // Controlled ?
           NeutralOrEnemy = true;
        } else {
           NeutralOrEnemy = false;
        }

        if (Cre.BaseStats.currentHP > 0) {  // Live on Map ?
            if (Cre.BaseStats.bNightmareModeStats) {
              if (NeutralOrEnemy == false) {
                  NightmareDowngrade(Cre);  // Reset Party & Co. after savegame loaded
                                            // need if savegame has upgraded summon, but later
                                            // pGameOptionsEx->bNightmarePartySummon was disabled
              }
            } else {
                NightmareUpgrade(Cre);
            }
        }
    }
    ///////////////////////////////////
}


bool static __stdcall
GetStatusDifficultySlider() {
    if (NightModeActive)
        return false;
    else
        return true;
}


void
NightMare_LoadGame(CSavedGameHeader& savegame) {
    WORD  weather = savegame.wWeatherFlags;
    DWORD dwFlags = savegame.dwFlags;
    if (weather & 0x8000 ||  // old format
        dwFlags & 0x200) {   // new format
        NightModeActive = true;
        g_pChitin->pGame->m_GameOptions.m_nDifficultyLevel = DIFFLEV_HARDEST;
        CInfGame_SetDifficultyMultiplier(g_pChitin->pGame);
    } else {
        NightModeActive = false;
    }
}


void
NightMare_SaveGame(CSavedGameHeader& savegame) {
    if (NightModeActive)
        savegame.dwFlags |= 0x200;
}


CUIButtonStartNightmare::CUIButtonStartNightmare(
                                    CPanel& panel,
                                    ChuFileControlInfoBase& controlInfo)
                                    : CUIButton(panel, controlInfo, 1, TRUE)
{
    IECString ButtonText = GetTlkString(6568); // ~Legacy of Bhaal~
    CUIButton::SetText(ButtonText);
}

CUIButtonStartNightmare::~CUIButtonStartNightmare() { }

_n void CUIButtonStartNightmare::SetButtonStatus(bool status) { _bgmain(0x587C9D) }


void CUIButtonStartNightmare::OnLClicked(POINT pt) {
    NightModeActive = true;
    g_pChitin->pGame->m_GameOptions.m_nDifficultyLevel = DIFFLEV_HARDEST;
    CInfGame_SetDifficultyMultiplier(g_pChitin->pGame);

    // temporary enable EffLearnSpellMod if disabled
    // need to handle EFFECTLEARNSPELL_SUCCESS_ALWAYS for BGT's A6GenSpl.dlg
    // EffLearnSpellMod will be active until BG2 will be restarted
    if (!pGameOptionsEx->bEffLearnSpellMod ||
        *(DWORD *) 0x52C250 == 0x6AEC8B55) {    // CEffectLearnSpell::ApplyEffect == orig bytes, not detoured
        pGameOptionsEx->bEffLearnSpellMod = TRUE;
        DetourMemberFunction(Tramp_CEffectLearnSpell_ApplyEffect, DETOUR_CEffectLearnSpell::DETOUR_ApplyEffect);
    }

    // simulate Accept Button OnLClicked()
    CUIButton& AcceptButton = (CUIButton&) pPanel->GetUIControl(8);
    AcceptButton.OnLClicked(pt);
}


void static __stdcall
EnableOrDisableButton50(bool status) {
    CUIButtonStartNightmare& NightmareButton = (CUIButtonStartNightmare&) g_pChitin->pScreenCreateChar->GetManager().GetPanel(0).GetUIControl(50);
    NightmareButton.SetButtonStatus(status);
}


int __stdcall
GetSpawnAmount(CArea& area) {
    if (NightModeActive)
        return (area.wMaxNumSpawns + pGameOptionsEx->bNightmareBonusSpawns);
    else
        return (area.wMaxNumSpawns);
}


void __declspec(naked)
CheckMorale_asm() {
__asm
{
    push    edx
    push    ecx

    push    ecx             ;Cre
    call    CheckMorale
    test    eax,eax
    jnz     exitL11

    pop     ecx
    pop     edx
    push    08CF90Eh   ; CGameSprite::CheckMorale
    ret

exitL11:    ; eax = 1
    pop     ecx
    pop     edx
    ret
}
}


void __declspec(naked)
CCreatureObject_Unmarshal_asm() {
__asm
{
    push    edx
    push    ecx

    push    ebp            ; stack frame
    push    [ebp-1E8h]     ; Cre
    call    CCreatureObject_Unmarshal

    pop     ecx
    pop     edx

    mov     edx, [ebp-28h]  ; Stolen bytes
    add     edx, 5Ch
    ret
}
}


void __declspec(naked)
CDerivedStats_Reload_Nightmare_asm() {
__asm
{
    push    edx
    push    ecx

    push    [ebp+8]     ; CreFileData
    push    [ebp-30h]   ; cds
    call    CDerivedStats_Reload_Nightmare

    pop     ecx
    pop     edx

    mov     eax, [ebp+8]  ; Stolen bytes
    movzx   cx, byte ptr [eax+4Ch]
    ret
}
}


//void __declspec(naked)
//CInfGame_Marshal_asm() {
//__asm
//{
//    push    edx
//    push    ecx
//
//    push    eax             ; Weather, word
//    call    WeatherSaveGame
//
//    pop     ecx
//    pop     edx
//
//    mov     edx, [ebp-84Ch] ; Stolen bytes
//    ret
//}
//}


//void __declspec(naked)
//CInfGame_Unmarshal_asm() {
//__asm
//{
//    push    edx
//    push    ecx
//
//    push    [ebp+8]         ; Savegame
//    call    LoadGame
//
//    pop     ecx
//    pop     edx
//
//    mov     [ebp-3C0h], ecx ; Stolen bytes
//    ret
//}
//}


void __declspec(naked)
GameplayOptions_Init_asm() {
__asm
{
    push    edx
    push    ecx

    call    GetStatusDifficultySlider

    pop     ecx
    pop     edx

    mov     byte ptr [ecx+1F4h], al ; Stolen bytes, enable/disable slider
    ret
}
}


// Pressing "Quit Game" in active game session
void __declspec(naked)
DestroyGame_asm() {
__asm
{
    mov     byte ptr [NightModeActive], 0

    mov     [ebp-98h], ecx ; Stolen bytes
    ret
}
}


void __declspec(naked)
SetButtonStatus_asm() {
__asm
{
    ; dl  - enable/disable "Accept" Button
    ; ecx - CUIButton "Accept" Button

    push    ecx
    push    edx

    push    edx
    call    EnableOrDisableButton50

    pop     edx
    pop     ecx

    push    587C9Dh ; Stolen bytes
    ret
}
}


void __declspec(naked)
CGameArea_CheckRestEncounter_asm() {
__asm
{
    push    ecx
    push    edx

    push    edx             ; Area
    call    GetSpawnAmount

    ; eax = SpawnAmount
    pop     edx
    pop     ecx

    ret
}
}
