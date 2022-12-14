#include "chitin.h"

void CheckMorale_asm();
void CCreatureObject_Unmarshal_asm();
void CDerivedStats_Reload_Nightmare_asm();
void CInfGame_Unmarshal_asm();
void CInfGame_Marshal_asm();
void GameplayOptions_Init_asm();
void DestroyGame_asm();
void SetButtonStatus_asm();
void DifficultyEvaluateStatusTrigger_asm();
void CGameArea_CheckRestEncounter_asm();


class CUIButtonStartNightmare : public CUIButton {
public:
    CUIButtonStartNightmare(CPanel& panel, ChuFileControlInfoBase& controlInfo);
    void SetButtonStatus(bool status);

    virtual ~CUIButtonStartNightmare(); //v0
    virtual void OnLClicked(POINT pt); //v5c
};

