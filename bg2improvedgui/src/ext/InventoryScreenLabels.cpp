#include "InventoryScreenLabels.h"
#include "ChitinCore.h"
#include "HandOffSlot.h"

#define CONTROLID_ThacMain           73
#define CONTROLID_ThacOffHand        74
#define CONTROLID_DamageMain         75
#define CONTROLID_DamageOffHand      76
#define CONTROLID_AproxDamageMain    77
#define CONTROLID_AproxDamageOffHand 78
#define CONTROLID_TotalRoll          39

#define DarkColorMask 0x666666
#define SCREEN_INV g_pChitin->pScreenInventory

static bool gDarkPortrait[6];
static bool gYellowBorderPlayer[6];

ItmFileAbility*
CItem_GetFirstLauncherAbility_v2(CItem& itm, int* const pNumAbility) {
    int nAbilities = itm.nNumAbilities;
    for (int i = 0; i < nAbilities; i++) {
        ItmFileAbility* ability = itm.GetAbility(i);
        if (ability->attackType == ITEMABILITYATTACKTYPE_LAUNCHER) {
            *pNumAbility = i;
            return ability;
        }
    }

    return NULL;
}

int 
GetMinDamage(int DiceCount, int LuckBase, int DiceSize) {
    int Dmg = DiceCount + LuckBase;
    Dmg = min(Dmg, DiceCount * DiceSize);   // DiceCount <= MinDmg <= DiceCount*DiceSize
    Dmg = max(Dmg, DiceCount);
    return Dmg;
}

int 
GetMaxDamage(int DiceCount, int LuckBase, int DiceSize) {
    int Dmg = (DiceCount * DiceSize) + LuckBase;
    Dmg = min(Dmg, DiceCount * DiceSize);    // DiceCount <= MaxDmg <= DiceCount*DiceSize
    Dmg = max(Dmg, DiceCount);
    return Dmg;
}


void __stdcall
InventoryScreen_LabelText(
            CCreatureObject& Cre,
            CPanel& Panel,
            CScreenInventory& Screen,
            int Mode,
            int* LowValueMain,
            int* HighValueMain,
            int* LowValueOff,
            int* HighValueOff)
{
    bool LeftHand;
    CDerivedStats& cds = Cre.cdsCurrent;

    int BonusFromItemsAndEffects,
        HandBonusMain,
        HandBonusOffHand,
        ThacMain,
        ThacOffHand,
        MinRollDamageMain,
        MaxRollDamageMain,
        MinDamageMain,
        MaxDamageMain,
        MinRollDamageOffHand,
        MaxRollDamageOffHand,
        MinDamageOffHand,
        MaxDamageOffHand,
        ThacModifier,
        DamageBonus,
        Berserkbonus,
        StrengthbonusMain,
        StrengthbonusOffHand,
        RollDamageBonusMain,
        RollDamageBonusOffHand,
        LuckBase;
    unsigned char   DiceSize, DiceCount, nSlotSelected; 
    bool            fist;
    unsigned int    AbilityFlagsMain,
                    AbilityFlagsOffHand,
                    AbilityFlagsAmmo,
                    AbilityFlagsLauncher;
    unsigned short  nAbilityIdx;
    float           AproxDamageMain, AproxDamageOffHand;
    CItem*          pItemLauncher;



    if ( Cre.Inventory.items[SLOT_SHIELD] == NULL ||
         isOffHandDisabled(Cre) ) {
        LeftHand = false;
    } else {
        if (Cre.Inventory.items[SLOT_SHIELD]->GetType() != ITEMTYPE_SHIELD ) { // some mods put crap in this slot, so offhand values will be showed
            LeftHand = true;
        } else {
            LeftHand = false;
        }
    }


    ThacModifier  = 0;
    ThacModifier += (cds.stateFlags & STATE_BERSERK)              ? 2 : 0;
    //ThacModifier += (cds.stateFlags & STATE_IMPROVEDINVISIBILITY) ? 4 : 0;
    ThacModifier += cds.hitBonus;
    
    ThacMain    = cds.THAC0 - cds.toHitBonusRight - ThacModifier;
    ThacOffHand = cds.THAC0 - cds.toHitBonusLeft  - ThacModifier;

    if (cds.luck < 0) { // fatigue mode
        ThacMain    -= max(cds.luck, -20);  // limit to -20
        ThacOffHand -= max(cds.luck, -20);
    }

    if (Mode == 0) {
        UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_ThacMain, "%d", ThacMain);
        if (LeftHand) {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_ThacOffHand, "%d", ThacOffHand);
        } else {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_ThacOffHand, "");
        }
    }

    LuckBase = cds.luck + cds.damageLuck;    // 0x90CA70

    MinRollDamageMain       = 0;
    MinRollDamageOffHand    = 0;
    MaxRollDamageMain       = 0;
    MaxRollDamageOffHand    = 0;
    RollDamageBonusMain     = 0;
    RollDamageBonusOffHand  = 0;
    AbilityFlagsMain        = 0;
    AbilityFlagsOffHand     = 0;
    AbilityFlagsAmmo        = 0;
    AbilityFlagsLauncher    = 0;

    nSlotSelected = Cre.Inventory.nSlotSelected;
    CItem* const pItem = Cre.Inventory.items[nSlotSelected];
    fist = (nSlotSelected == SLOT_FIST) ?  true : false;
    nAbilityIdx = Cre.Inventory.nAbilitySelected;


    // Main Hand single weapon (melee or ranged)
    if ( pItem &&
            ((nSlotSelected >= SLOT_WEAPON0 && nSlotSelected <= SLOT_WEAPON3) ||
              nSlotSelected == SLOT_FIST ||
              nSlotSelected == SLOT_MISC19) ) // - magic weapon
    {
        pItem->Demand();

        ItmFileAbility* ability = pItem->GetAbility(nAbilityIdx);
        if (ability) {
            if (ability->attackType == ITEMABILITYATTACKTYPE_MELEE ||
                ability->attackType == ITEMABILITYATTACKTYPE_RANGED) {
                // 1d8 + 1
                DiceCount        = ability->nDice;
                DiceSize         = ability->sizeDice;
                DamageBonus      = ability->damBonus;
                AbilityFlagsMain = ability->flags;

                MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                RollDamageBonusMain += DamageBonus;

                for (int i = 0; i <= (ability->nEffects - 1); i++ ) {
                    CEffect* Eff = pItem->GetAbilityEffect(nAbilityIdx, i, &Cre);
                    if (Eff == NULL) {
                        continue;
                    }
                                                
                    ITEM_EFFECT* ItemEffect = Eff->GetItemEffect();
                    if (ItemEffect == NULL) {
                        continue;
                    }

                    if (ItemEffect->opcode == CEFFECT_OPCODE_DAMAGE &&
                        ItemEffect->target == 2 &&                            // present target, not AOE
                        (ItemEffect->param2 & 0xFFFF) == 0 &&                 // mode - normal
                        (ItemEffect->highProb - ItemEffect->lowProb) == 100 && // only 100% accepted
                        ItemEffect->saveType == 0 )                           // no save
                    {
                        DiceCount    = ItemEffect->numDiceOrMaxLevel;
                        DiceSize     = ItemEffect->sizeDiceOrMinLevel;
                        DamageBonus  = ItemEffect->param1;

                        /*if (StaticDamage) {
                            MinDamageMain += StaticDamage;
                            MaxDamageMain += StaticDamage;}
                        else {
                            MinDamageMain += (DiceCount * 1       );
                            MaxDamageMain += (DiceCount * DiceSize);
                        }*/
                        MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                        MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                        RollDamageBonusMain += DamageBonus;
                    }

                    IEFree(ItemEffect);
                }

            }
        }

        pItem->Release();
    }


    // Main Hand Ammo
    if ( pItem &&
        (nSlotSelected >= SLOT_AMMO0 && nSlotSelected <= SLOT_AMMO3) )
    {
        pItem->Demand();

        ItmFileAbility* ability = pItem->GetAbility(nAbilityIdx);
        if (ability) {
            if (ability->attackType == ITEMABILITYATTACKTYPE_RANGED) {
                // 1d8 + 1
                DiceCount        = ability->nDice;
                DiceSize         = ability->sizeDice;
                DamageBonus      = ability->damBonus;
                AbilityFlagsAmmo = ability->flags;

                MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                RollDamageBonusMain += DamageBonus;

                for (int i = 0; i <= (ability->nEffects - 1); i++ ) {
                    CEffect* Eff = pItem->GetAbilityEffect(nAbilityIdx, i, &Cre);
                    if (Eff == NULL) {
                        continue;
                    }
                                                
                    ITEM_EFFECT* ItemEffect = Eff->GetItemEffect();
                    if (ItemEffect == NULL) {
                        continue;
                    }

                    if (ItemEffect->opcode == CEFFECT_OPCODE_DAMAGE &&
                        ItemEffect->target == 2 &&                            // present target, not AOE
                        (ItemEffect->param2 & 0xFFFF) == 0 &&                 // mode - normal
                        (ItemEffect->highProb - ItemEffect->lowProb) == 100 && // only 100% accepted
                        ItemEffect->saveType == 0 )                           // no save
                    {
                        DiceCount    = ItemEffect->numDiceOrMaxLevel;
                        DiceSize     = ItemEffect->sizeDiceOrMinLevel;
                        DamageBonus  = ItemEffect->param1;

                        MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                        MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                        RollDamageBonusMain += DamageBonus;
                    }

                    IEFree(ItemEffect);
                }
            }
        }

        
        // Main Hand Launcher
        short nSlot;
        pItemLauncher = Cre.GetFirstEquippedLauncherOfAbility(ability, nSlot);

        if (pItemLauncher)
        {
            pItemLauncher->Demand();

            int nAbilityIdxLauncher = 0;
            ItmFileAbility* ability_l = CItem_GetFirstLauncherAbility_v2(*pItemLauncher, &nAbilityIdxLauncher);

            if (ability_l) {
                if (ability_l->attackType == ITEMABILITYATTACKTYPE_LAUNCHER) {
                    // 1d8 + 1
                    DiceCount            = ability_l->nDice;
                    DiceSize             = ability_l->sizeDice;
                    DamageBonus          = ability_l->damBonus;
                    AbilityFlagsLauncher = ability_l->flags;

                    MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                    MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                    RollDamageBonusMain += DamageBonus;

                    for (int i = 0; i <= (ability_l->nEffects - 1); i++ ) {
                        CEffect* Eff = pItemLauncher->GetAbilityEffect(nAbilityIdxLauncher, i, &Cre);
                        if (Eff == NULL) {
                            continue;
                        }
                                                
                        ITEM_EFFECT* ItemEffect = Eff->GetItemEffect();
                        if (ItemEffect == NULL) {
                            continue;
                        }

                        if (ItemEffect->opcode == CEFFECT_OPCODE_DAMAGE &&
                            ItemEffect->target == 2 &&                            // present target, not AOE
                            (ItemEffect->param2 & 0xFFFF) == 0 &&                 // mode - normal
                            (ItemEffect->highProb - ItemEffect->lowProb) == 100 && // only 100% accepted
                            ItemEffect->saveType == 0 )                           // no save
                        {
                            DiceCount    = ItemEffect->numDiceOrMaxLevel;
                            DiceSize     = ItemEffect->sizeDiceOrMinLevel;
                            DamageBonus  = ItemEffect->param1;

                            MinRollDamageMain   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                            MaxRollDamageMain   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                            RollDamageBonusMain += DamageBonus;
                        }

                        IEFree(ItemEffect);
                    }
                }
            }

            pItemLauncher->Release();
        }

        pItem->Release();
    }


    // Off Hand single weapon (melee only)
    if (LeftHand)
    {
        CItem* const pItemLeftHand = Cre.Inventory.items[SLOT_SHIELD];
        nAbilityIdx = 0;    // offhand can be only melee

        pItemLeftHand->Demand();

        ItmFileAbility* ability = pItemLeftHand->GetAbility(nAbilityIdx);
        if (ability) {
            if (ability->attackType == ITEMABILITYATTACKTYPE_MELEE) {
                // 1d8 + 1
                DiceCount           = ability->nDice;
                DiceSize            = ability->sizeDice;
                DamageBonus         = ability->damBonus;
                AbilityFlagsOffHand = ability->flags;

                MinRollDamageOffHand   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                MaxRollDamageOffHand   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                RollDamageBonusOffHand  += DamageBonus;

                for (int i = 0; i <= (ability->nEffects - 1); i++ ) {
                    CEffect* Eff = pItemLeftHand->GetAbilityEffect(nAbilityIdx, i, &Cre);
                    if (Eff == NULL) {
                        continue;
                    }
                                                
                    ITEM_EFFECT* ItemEffect = Eff->GetItemEffect();
                    if (ItemEffect == NULL) {
                        continue;
                    }

                    if (ItemEffect->opcode == CEFFECT_OPCODE_DAMAGE &&
                        ItemEffect->target == 2 &&                            // present target, not AOE
                        (ItemEffect->param2 & 0xFFFF) == 0 &&                 // mode - normal
                        (ItemEffect->highProb - ItemEffect->lowProb) == 100 && // only 100% probability
                        ItemEffect->saveType == 0 )                           // no save
                    {
                        DiceCount    = ItemEffect->numDiceOrMaxLevel;
                        DiceSize     = ItemEffect->sizeDiceOrMinLevel;
                        DamageBonus  = ItemEffect->param1;

                        MinRollDamageOffHand   += GetMinDamage(DiceCount, LuckBase, DiceSize);
                        MaxRollDamageOffHand   += GetMaxDamage(DiceCount, LuckBase, DiceSize);
                        RollDamageBonusOffHand  += DamageBonus;
                    }

                    IEFree(ItemEffect);
                }

            }
        }

        pItemLeftHand->Release();
    }

    Berserkbonus = (cds.stateFlags & STATE_BERSERK)  ? 2 : 0;

    StrengthbonusMain    = 0;
    StrengthbonusOffHand = 0;

    if ((AbilityFlagsMain     & ITEMABILITYFLAG_STRENGTH_BONUS) ||
        (AbilityFlagsOffHand  & ITEMABILITYFLAG_STRENGTH_BONUS) ||
        (AbilityFlagsAmmo     & ITEMABILITYFLAG_STRENGTH_BONUS) ||
        (AbilityFlagsLauncher & ITEMABILITYFLAG_STRENGTH_BONUS)) {
        int nSTRMOD, nSTRMODEX;
        short nCol = 1;
        short nRow = cds.strength;
        LPCTSTR *pString;
        CInfGame*  const pGame = g_pChitin->pGame;

        if (nCol < pGame->STRMOD.nCols &&
            nRow < pGame->STRMOD.nRows &&
            nCol >= 0 &&
            nRow >= 0) {
            pString = (LPCTSTR *) & pGame->STRMOD.pDataArray[(nRow  * g_pChitin->pGame->STRMOD.nCols) + nCol];
        } else {
            pString = (LPCTSTR *) & pGame->STRMOD.defaultVal;
        }
        sscanf_s(*pString, "%d", &nSTRMOD);

        nRow = cds.strengthEx;
        if (nCol < pGame->STRMODEX.nCols &&
            nRow < pGame->STRMODEX.nRows &&
            nCol >= 0 &&
            nRow >= 0) {
           pString = (LPCTSTR *) & pGame->STRMODEX.pDataArray[(nRow  * g_pChitin->pGame->STRMODEX.nCols) + nCol];
        } else {
           pString = (LPCTSTR *) & pGame->STRMODEX.defaultVal;
        }
        sscanf_s(*pString, "%d", &nSTRMODEX);
        

        // Strength bonus is not stackable
        if (AbilityFlagsMain     & ITEMABILITYFLAG_STRENGTH_BONUS)
            StrengthbonusMain    = nSTRMOD + nSTRMODEX;

        if (AbilityFlagsOffHand  & ITEMABILITYFLAG_STRENGTH_BONUS)
            StrengthbonusOffHand = nSTRMOD + nSTRMODEX;

        if (AbilityFlagsAmmo     & ITEMABILITYFLAG_STRENGTH_BONUS)
            StrengthbonusMain    = nSTRMOD + nSTRMODEX;

        if (AbilityFlagsLauncher & ITEMABILITYFLAG_STRENGTH_BONUS)
            StrengthbonusMain    = nSTRMOD + nSTRMODEX;
    }

    BonusFromItemsAndEffects = cds.damageBonus;
    HandBonusMain            = cds.damageBonusRight;
    HandBonusOffHand         = cds.damageBonusLeft;

    // original black magic:
    // Roll + DamageMod + Handbonus(Proficiency + Melee + Effects + TwoHandProficiency) +
    // Attack of Opportunity=(0 or 4) + Berserkbonus + Strengthbonus + Racebonus + Specialbonus

    MinDamageMain    = (MinRollDamageMain + RollDamageBonusMain) +
                        BonusFromItemsAndEffects +
                        HandBonusMain +
                        Berserkbonus +
                        StrengthbonusMain;

    MaxDamageMain    = (MaxRollDamageMain + RollDamageBonusMain) +
                        BonusFromItemsAndEffects +
                        HandBonusMain +
                        Berserkbonus +
                        StrengthbonusMain;

    MinDamageOffHand = (MinRollDamageOffHand + RollDamageBonusOffHand) +
                        BonusFromItemsAndEffects +
                        HandBonusOffHand +
                        Berserkbonus +
                        StrengthbonusOffHand;

    MaxDamageOffHand = (MaxRollDamageOffHand + RollDamageBonusOffHand) +
                        BonusFromItemsAndEffects +
                        HandBonusOffHand +
                        Berserkbonus +
                        StrengthbonusOffHand;

    if (Mode == 0) {
        UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_DamageMain, "%d-%d", MinDamageMain, MaxDamageMain);
        if (LeftHand) {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_DamageOffHand, "%d-%d", MinDamageOffHand, MaxDamageOffHand);
        } else {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_DamageOffHand, "");
        }
    } else {
        *LowValueMain =  MinDamageMain;
        *HighValueMain = MaxDamageMain;
        *LowValueOff =   MinDamageOffHand;
        *HighValueOff =  MaxDamageOffHand;
    }

    AproxDamageMain    = ((float)(MinRollDamageMain + MaxRollDamageMain))/2 +
                         RollDamageBonusMain +
                         BonusFromItemsAndEffects +
                         HandBonusMain +
                         Berserkbonus +
                         StrengthbonusMain;

    AproxDamageOffHand = ((float)(MinRollDamageOffHand + MaxRollDamageOffHand))/2 +
                         RollDamageBonusOffHand +
                         BonusFromItemsAndEffects +
                         HandBonusOffHand +
                         Berserkbonus +
                         StrengthbonusOffHand;

    if (Mode == 0) {
        UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_AproxDamageMain, "%.1f", AproxDamageMain);
        if (LeftHand) {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_AproxDamageOffHand, "%.1f", AproxDamageOffHand);
        } else {
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_AproxDamageOffHand, "");
        }
    }
}

#define BODY        0
#define INVENTORY   1
#define CONTAINER   2
#define BAG         3

void __stdcall
InventoryScreen_Drag(CItem** const ppPtr, CItem* const pDstItem, int Mode)
{
    bool   grab = false;
    bool ungrab = false;

    if (ppPtr == & g_pChitin->pScreenInventory->pItemGrabbed) {
        switch (Mode) {
        case INVENTORY:
        case BAG:
            if (pDstItem) {
                if (g_pChitin->pScreenInventory->pItemGrabbed == NULL) {
                    grab   = true;          // grab on, pItemGrabbed=0
                    ungrab = false;
                    break;
                }

                if (pDstItem->GetType() == ITEMTYPE_CONTAINER ||
                    pDstItem->itm.name == g_pChitin->pScreenInventory->pItemGrabbed->itm.name ) {
                    ungrab = true;          // grab off, drop on container/stack
                    grab   = false;
                    break;
                } else {
                    grab   = true;          // grab on, pItemGrabbed=x, exchange with other item
                    ungrab = false;
                    break;
                }
            } else {
                ungrab = true;              // grab off, drop on empty inventory slot
                grab   = false;
                break;
            }
            break; //end

        case BODY:
            if (g_pChitin->pScreenInventory->pItemGrabbed) {
                if (pDstItem == g_pChitin->pScreenInventory->pItemGrabbed) {
                    grab   = true;          // grab on, dst_item=grab_item
                    ungrab = false;
                    break;
                } else {
                    ungrab = true;          // grab off, dst_item<>grab_item, error 
                    grab   = false;
                    break;
                }
            } else {
                ungrab = true;              // grab off
                grab   = false;
                break;
            }
            break;  //end

        case CONTAINER:
            if (pDstItem) {
                grab   = true;          // grab on, dst_item=x
                ungrab = false;
                break;
            } else {
                ungrab = true;          // grab off
                grab   = false;
                break;
            }
            break;  //end
        } // switch

    } else {   // not drag action
        return;
    }

    if (grab) {
        bool             allowed;
        unsigned int     ItemTextIDRef;
        CUIButton*       pPortaitButton;
        CInfGame*  const pGame = g_pChitin->pGame;
        CPanel&          Panel  = g_pChitin->pScreenInventory->manager.GetPanel(1); // Portraits

        #ifdef _DEBUG
            //console.writef("Item %s grabbed \n", pDstItem->itm.name.GetResRefNulled());
        #endif

        for (short PortraitNum = 0; PortraitNum < pGame->numInParty; PortraitNum++) {
            allowed = to_bool(pGame->CheckItemUsable(PortraitNum, pDstItem, &ItemTextIDRef, 1));

            gDarkPortrait[PortraitNum] = !allowed;

            if (pGameOptionsEx->bUI_YellowBorderOnPortraitIfTooFar &&
                SCREEN_INV->nActivePlayerIdx != PortraitNum) {
                // IsCharacterInRange() uses ScreenInventory.nPlayerIdxOfGrabbedItem
                // it is not available when grab item, cheat it...
                int save = SCREEN_INV->nPlayerIdxOfGrabbedItem;
                SCREEN_INV->nPlayerIdxOfGrabbedItem = SCREEN_INV->nActivePlayerIdx;
                bool InRange = BOOL_to_bool(SCREEN_INV->IsCharacterInRange(PortraitNum));
                SCREEN_INV->nPlayerIdxOfGrabbedItem = save;

                // IsCharacterInRange() already has check for Dead Player
                //bool Dead = BOOL_to_bool(SCREEN_INV->IsCharacterDead(PortraitNum)); 

                if (!InRange)
                    gYellowBorderPlayer[PortraitNum] = true;
            }

            pPortaitButton = &(CUIButton&)Panel.GetUIControl(PortraitNum);
            pPortaitButton->SetRedraw();

            #ifdef _DEBUG
                //console.writef("Player%d allowed %d , Redraw() \n", PortraitNum, allowed);
            #endif
         }
    }

    if (ungrab) {
        for (unsigned i = 0; i<6; i++)
        {
            gDarkPortrait[i] = false;
            gYellowBorderPlayer[i] = false;
        }

        CInfGame* const pGame   = g_pChitin->pGame;
        CPanel&         Panel   = g_pChitin->pScreenInventory->manager.GetPanel(1); // Portraits
        CUIButton*      pPortaitButton;

        for (short PortraitNum = 0; PortraitNum < pGame->numInParty; PortraitNum++) {
            pPortaitButton = &(CUIButton&)Panel.GetUIControl(PortraitNum);
            pPortaitButton->SetRedraw();

            #ifdef _DEBUG
                //console.writef("Item dropped, Redraw() \n");
            #endif
        }
    }

}


void __stdcall
InventoryScreen_RenderPortrait(CCreatureObject& Cre, RECT* const pPictureRect, RECT* const pPlaceholderRect, ABGR* pBorderColor)
{
    Enum      const CreObjID = Cre.id;
    CInfGame* const pGame    = g_pChitin->pGame;
    short           PortraitNum;

    for (PortraitNum = 0; PortraitNum < pGame->numInParty; PortraitNum++) {
        if (pGame->ePlayersPartyOrder[PortraitNum] == CreObjID)
            break;
    }

    if (gDarkPortrait[PortraitNum]) {
        CVidBitmap* const pCreVidBitmap = &Cre.smallPortrait;
        RECT       rect;

        pCreVidBitmap->SetTintColor(DarkColorMask);
        IntersectRect(&rect, pPictureRect, pPlaceholderRect);
        pCreVidBitmap->RenderDirect(
                0,
                pPictureRect->left,
                pPictureRect->top,
                &rect,
                0x20000,
                0);

        #ifdef _DEBUG
            //console.writef("Portrait %d Masked \n", PortraitNum);
        #endif
    }

    if (pGameOptionsEx->bUI_YellowBorderOnPortraitIfTooFar &&
        gYellowBorderPlayer[PortraitNum]) {
        *pBorderColor = 0x00FFFF; // Yellow
    }

}


void __stdcall
RollScreen_UpdateTotalRoll(CCreatureObject& Cre)
{
    int total = Cre.BaseStats.strength     +
                Cre.BaseStats.dexterity    +
                Cre.BaseStats.constitution +
                Cre.BaseStats.intelligence +
                Cre.BaseStats.wisdom       +
                Cre.BaseStats.charisma;

    CScreenCharGen& Screen = * g_pChitin->pScreenCreateChar;
    CPanel& Panel = Screen.manager.GetPanel(4);
    UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_TotalRoll, "(  %d  )", total);
    //Panel.SetRedraw(0);
}


void __stdcall
InventoryScreen_SetTooltips(CPanel& Panel)
{
    Panel.GetUIControl(0x10000000 + CONTROLID_ThacMain).            SetTooltip(56911);
    Panel.GetUIControl(0x10000000 + CONTROLID_ThacOffHand).         SetTooltip(56910);
    Panel.GetUIControl(0x10000000 + CONTROLID_DamageMain).          SetTooltip(6560);
    Panel.GetUIControl(0x10000000 + CONTROLID_DamageOffHand).       SetTooltip(6561);
    Panel.GetUIControl(0x10000000 + CONTROLID_AproxDamageMain).     SetTooltip(6562);
    Panel.GetUIControl(0x10000000 + CONTROLID_AproxDamageOffHand).  SetTooltip(6563);
}


void HighlightLabel(CPanel *panel, int ControlId, ABGR color)
{
    if (panel) {
        CUILabel& Control = (CUILabel&) panel->GetUIControl(ControlId);
        if (&Control) {
            Control.SetForegroundColor(color);
        }
    }
}

#define HIGHLIGHT_STAT(x)                                           \
        if (Cre.cdsCurrent.##x > Cre.BaseStats.##x)                 \
            HighlightLabel(panel, ControlId, 0x00FF00); /* Green */ \
        else                                                        \
            HighlightLabel(panel, ControlId, 0x0000FF);  /* Red */  \
        break;                  

void __stdcall
HighlightLabelInject(CCreatureObject& Cre, CEngine *Screen, CPanel *panel, int ControlId, int bEnable) {
    if (!bEnable) {
        HighlightLabel(panel, ControlId, 0xFFFFFF);     /* White */
        return;
    }

    switch (ControlId) {
    case 0x1000002F:
        if (Cre.cdsCurrent.strength == Cre.BaseStats.strength) {
            HIGHLIGHT_STAT(strengthEx)
        } else {
            HIGHLIGHT_STAT(strength)
        }
    case 0x10000009:
        HIGHLIGHT_STAT(dexterity)
    case 0x1000000A:
        HIGHLIGHT_STAT(constitution)
    case 0x1000000B:
        HIGHLIGHT_STAT(intelligence)
    case 0x1000000C:
        HIGHLIGHT_STAT(wisdom)
    case 0x1000000D:
        HIGHLIGHT_STAT(charisma)
    default:
        break;
    }
}


void static __stdcall
CScreenChar_UpdateMainPanel_AddNewInfo(CScreenRecord& Screen, CUITextArea& pTextArea, CCreatureObject& Cre) {
    short intox =       Cre.GetDerivedStats().intoxication;
    short fatigue =     Cre.GetDerivedStats().fatigue;
    short luck =        Cre.GetDerivedStats().luck;
    short mentalSpeed   = Cre.GetDerivedStats().mentalSpeed;
    short physicalSpeed = Cre.GetDerivedStats().physicalSpeed;

    int   movement;
    if (Cre.animation.pAnimation)
        movement = Cre.animation.pAnimation->nMovementRateCurrent;
    else
        movement = 0;

    if (!pGameOptionsEx->bUI_ExtendedRecordScreenText_Threshold ||
        luck != 0) {
        IECString sluck = GetTlkString(14019);      // ~Luck~
        IECString sValue;
        sValue.Format("%d", luck);
        if (luck > 0)
            CEngine_UpdateText(Screen, pTextArea, "%s: +%s", (LPCTSTR)sluck, (LPCTSTR)sValue);
        else
            CEngine_UpdateText(Screen, pTextArea, "%s: %s", (LPCTSTR)sluck, (LPCTSTR)sValue);
    }

    if (!pGameOptionsEx->bUI_ExtendedRecordScreenText_Threshold ||
        intox >= 50) {   // INTOXMOD.2DA
        IECString sIntox = GetTlkString(14087);     // ~Intoxication~
        IECString sValue;
        sValue.Format("%d", intox);
        CEngine_UpdateText(Screen, pTextArea, "%s: %s", (LPCTSTR)sIntox, (LPCTSTR)sValue);
    }

    if (!pGameOptionsEx->bUI_ExtendedRecordScreenText_Threshold ||
        fatigue >= 7) {   // FATIGMOD.2DA
        IECString sfatigue = GetTlkString(14086);   // ~Fatigue~
        IECString sValue;
        sValue.Format("%d", fatigue);
        CEngine_UpdateText(Screen, pTextArea, "%s: %s", (LPCTSTR)sfatigue, (LPCTSTR)sValue);
    }

    if (!pGameOptionsEx->bUI_ExtendedRecordScreenText_Threshold ||
        mentalSpeed != 0) {
        IECString smentalSpeed = GetTlkString(3725); // ~Casting speed~
        IECString sValue;
        sValue.Format("%d", mentalSpeed);
        if (mentalSpeed > 0)
            CEngine_UpdateText(Screen, pTextArea, "%s: +%s", (LPCTSTR)smentalSpeed, (LPCTSTR)sValue);
        else
            CEngine_UpdateText(Screen, pTextArea, "%s: %s", (LPCTSTR)smentalSpeed, (LPCTSTR)sValue);
    }

    if (!pGameOptionsEx->bUI_ExtendedRecordScreenText_Threshold) {
        // WSPECIAL.2da
        short nRow;
        short ItemSpeed;
        short nCol = 2;             // SPEED
        int   bonus;
	    IECString sHit;
        CItem*  Item = Cre.Inventory.items[Cre.Inventory.nSlotSelected];
        short   launcherSlot;

        if (Item) {
            Item->Demand();

            ItmFileAbility* ItemAbility = Item->GetAbility(Cre.Inventory.nAbilitySelected);
            CItem* ItemLauncher = Cre.GetFirstEquippedLauncherOfAbility(ItemAbility, launcherSlot);

            if (ItemLauncher) {
                ItemLauncher->Demand();

                ItmFileAbility* ItemLauncherAbility = ItemLauncher->GetAbility(Cre.Inventory.nAbilitySelected);
                nRow = Cre.GetProficiencyInItem(*ItemLauncher);
                if (ItemLauncherAbility)
                    ItemSpeed = ItemLauncherAbility->speed;    // ignore ammo speed
                else
                    ItemSpeed = ItemAbility->speed;            // failed ability, use ammo speed

                ItemLauncher->Release();
            }
            else {
                nRow = Cre.GetProficiencyInItem(*Item);
                ItemSpeed = ItemAbility->speed;
            }

	        if (nCol  < g_pChitin->pGame->WSPECIAL.nCols &&
		        nRow  < g_pChitin->pGame->WSPECIAL.nRows &&
		        nCol  >= 0 &&
		        nRow  >= 0) {
		        sHit  = *((g_pChitin->pGame->WSPECIAL.pDataArray) + (g_pChitin->pGame->WSPECIAL.nCols * nRow  + nCol));
	        } else {
		        sHit  = g_pChitin->pGame->WSPECIAL.defaultVal;
	        }
	        sscanf_s((LPCTSTR)sHit,  "%d", &bonus); // Value is negative
            bonus = -bonus;

            IECString sphysicalSpeed = GetTlkString(6553); // ~Weapon speed~
            IECString sValue;
            sValue.Format("%d", ItemSpeed - physicalSpeed - bonus);
            if (ItemSpeed - physicalSpeed - bonus > 0)
                CEngine_UpdateText(Screen, pTextArea, "%s: %s", (LPCTSTR)sphysicalSpeed, (LPCTSTR)sValue);
            else
                CEngine_UpdateText(Screen, pTextArea, "%s: 0", (LPCTSTR)sphysicalSpeed);

            Item->Release();
        }
    }

    if (movement != 0) {
        IECString smovement = GetTlkString(14119); // ~Movement Rate~
        IECString sValue;

        if (Cre.nAIUpdateInterval == 0)
            movement *= 2;

        sValue.Format("%d", movement);
        CEngine_UpdateText(Screen, pTextArea, "%s : %s", (LPCTSTR)smovement, (LPCTSTR)sValue);
    }
}


void static __stdcall
InventoryScreen_CheckCheats(CCreatureObject& Cre, short wSlot) {
    // clear previous lstTargetIds if we try to swap already activated item
    if (Cre.currentAction.opcode == ACTION_USEITEM ||
        Cre.currentAction.opcode == ACTION_USEITEM_POINT ) {
        Cre.lstTargetIds.RemoveAll();

        if (Cre.currentAction.m_specificID == wSlot) {// Swap & Use same slot, cheat detected
            Cre.currentAction.opcode = ACTION_NO_ACTION;
            console.writef("Multi-target item swap exploit detected \n");
        }
    }
}


void static __stdcall
CScreenCharacter_UpdateMainPanel_AddDamageMain(CScreenRecord& Screen, CUITextArea& pTextArea, CCreatureObject& Cre) {
    IECString s1 = GetTlkString(6560);  // ~Main hand damage~
    IECString sValue1, sValue2;
    int LowValueMain, HighValueMain, LowValueOff, HighValueOff;
    InventoryScreen_LabelText(Cre, *((CPanel*) NULL), *((CScreenInventory*) NULL), 1, &LowValueMain, &HighValueMain, &LowValueOff, &HighValueOff);
    sValue1.Format("%d", LowValueMain);
    sValue2.Format("%d", HighValueMain);
    CEngine_UpdateText(Screen, pTextArea, "%s: %s-%s", (LPCTSTR)s1, (LPCTSTR)sValue1, (LPCTSTR)sValue2);
}

void static __stdcall
CScreenCharacter_UpdateMainPanel_AddDamageOff(CScreenRecord& Screen, CUITextArea& pTextArea, CCreatureObject& Cre) {
    IECString s1 = GetTlkString(6561);  // ~Off hand damage~
    IECString sValue1, sValue2;
    int LowValueMain, HighValueMain, LowValueOff, HighValueOff;
    InventoryScreen_LabelText(Cre, *((CPanel*) NULL), *((CScreenInventory*) NULL), 1, &LowValueMain, &HighValueMain, &LowValueOff, &HighValueOff);
    sValue1.Format("%d", LowValueOff);
    sValue2.Format("%d", HighValueOff);
    CEngine_UpdateText(Screen, pTextArea, "%s: %s-%s", (LPCTSTR)s1, (LPCTSTR)sValue1, (LPCTSTR)sValue2);
}


//void static __stdcall
//CScreenCharacter_UpdateStyleBonus_AddPenalty(CScreenRecord& Screen, CUITextArea& pTextArea) {
//    IECString s1 = GetTlkString(56911);  // ~Main Hand THAC0~
//    CEngine_UpdateText(Screen, pTextArea, "%s: -4", (LPCTSTR)s1);
//    IECString s2 = GetTlkString(56910);  // ~Off Hand THAC0~
//    CEngine_UpdateText(Screen, pTextArea, "%s: -8", (LPCTSTR)s2);
//}



void __declspec(naked)
HighlightLabel_asm()
{
__asm {
    ; esp+C bEnable
    ; esp+8 ControlId
    ; esp+4 Panel
    ; esp+0 return

    push    [esp+0ch]    ; bEnable
    push    [esp+0ch]    ; ControlId
    push    [esp+0ch]    ; Panel
    push    ecx          ; Screen
    push    [ebp-68h]    ; Cre 
    call    HighlightLabelInject


    ret     0ch
}
}


void __declspec(naked)
InventoryScreen_LabelText_asm()
{
__asm {

    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0                   ; dummy int
    push    0                   ; dummy int
    push    0                   ; dummy int
    push    0                   ; dummy int    
    push    0                   ; Inventory screen mode
    push    [ebp-0d0h]          ; CScreenInventory
    push    [ebp-10h]           ; Panel
    push    [ebp-4Ch]           ; Cre
    call    InventoryScreen_LabelText

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

;lexit:
    mov     eax, [ebp-48h]          ; stolen bytes
    movsx   ecx, word ptr [eax+6]
    ret
}
}





void __declspec(naked)
InventoryScreen_DragInventoryArea_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    INVENTORY   ; Inventory Area Mode 
    push    [ebp-14h]   ; Value(CItem)
    push    [ebp+0ch]   ; DstPtr
    call    InventoryScreen_Drag

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    cmp     dword ptr [ebp-14h], 0  ; stolen bytes
    jz      HackReturn1
    ret

HackReturn1:
    pop     eax             ; kill ret adr
    mov     eax, 06A052Dh
    jmp     eax
}
}


void __declspec(naked)
InventoryScreen_DragInventoryAreaOnBag_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    BAG         ; Inventory Area Mode 
    push    [ebp-14h]   ; Value(CItem)
    push    [ebp+0ch]   ; DstPtr
    call    InventoryScreen_Drag

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     ecx, [ebp+0ch]  ; stolen bytes
    mov     edx, [ecx]
    ret
}
}


void __declspec(naked)
InventoryScreen_DragContainerArea_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    CONTAINER   ; Container Area Mode 
    push    [ebp-18h]   ; Value(CItem)
    push    [ebp+10h]   ; DstPtr
    call    InventoryScreen_Drag

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     edx, [ebp+10h]  ; stolen bytes
    mov     eax, [ebp-18h]
    ret
}
}


void __declspec(naked)
InventoryScreen_DragBodyArea_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    BODY        ; Body Area Mode 
    push    [ebp-40h]   ; Value(CItem)
    push    [ebp+10h]   ; DstPtr
    call    InventoryScreen_Drag

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     eax, [ebp-48h]  ; stolen bytes
    mov     ecx, [eax+30h]
    ret
}
}


void __declspec(naked)
InventoryScreen_CheckCheats_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp+0Ch]   ; wSlot
    push    [ebp-48h]   ; Cre
    call    InventoryScreen_CheckCheats

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     [ebp-290h], ecx  ; stolen bytes
    ret
}
}


void __declspec(naked)
InventoryScreen_PortraitRedraw_asm()
{
__asm {
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    lea     eax, [ebp-8]    ; BorderColor
    push    eax
    push    [ebp+1ch]       ; Placeholder
    lea     eax, [ebp-44h]  ; PictureRect
    push    eax
    push    [ebp-26ch]      ; Cre
    call    InventoryScreen_RenderPortrait

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    mov     eax, [ebp-28h]  ; stolen bytes
    sub     eax, 1

    ret
}
}


void __declspec(naked)
CScreenCreateChar_ResetAbilities_asm()
{
__asm {
    push    ecx
    push    edx

    push    [ebp+8]     // Cre
    call    RollScreen_UpdateTotalRoll

    pop     edx
    pop     ecx

    add     esp, 4       // drop ret adress
    mov     esp, ebp    // stolen bytes
    pop     ebp
    ret     4
}
}


void __declspec(naked)
RollScreen_RecallButtonClick_asm()
{
__asm {
    push    ecx
    push    edx

    push    [ebp-10h]       ; Cre
    call    RollScreen_UpdateTotalRoll

    pop     edx
    pop     ecx

    mov     eax, [ebp-0ch]  ; stolen bytes
    mov     [ebp-20h], eax
    ret
}
}


void __declspec(naked)
InventoryScreen_SetTooltips_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-4h]  ; Panel
    call    InventoryScreen_SetTooltips

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    mov     eax, [ebp-8h]   ; stolen bytes
    mov     edx, [eax]
    ret
}
}


void __declspec(naked)
DragToBag_asm()
{
__asm {
    mov     ecx, [ebp+0Ch]  ; stolen bytes
    mov     edx, [ecx]      ; edx= Item
    and     byte ptr [edx+20h], 11111101b    ; clear "unstealable" flags
    ret
}
}


void __declspec(naked)
CStore__GetNumItems_asm()
{
__asm {                         ; ecx= Store
    mov     eax, [ecx+09Ch+0Ch] ; CStore::m_items.m_nCount
    ret
}
}


void __declspec(naked)
CScreenChar_UpdateMainPanel_AddNewInfo_asm()
{
__asm {
    push    ecx
    push    edx

    push    [ebp-68h]   ; Cre
    push    [ebp-74h]   ; TextArea
    push    [ebp-378h]  ; screen
    call    CScreenChar_UpdateMainPanel_AddNewInfo

    pop     edx
    pop     ecx

    push    06D957Dh    // return to orig code
    ret
}
}


void __declspec(naked)
CScreenCharacter_UpdateMainPanel_AddDamageOneHand_asm()
{
__asm {
    push    ecx
    push    edx

    push    [ebp-68h]   // Cre
    push    [ebp-74h]   // TextArea
    push    [ebp-378h]  // screen
    call    CScreenCharacter_UpdateMainPanel_AddDamageMain

    pop     edx
    pop     ecx

    lea     ecx, [ebp-0E0h]  // return to orig code
    ret
}
}


void __declspec(naked)
CScreenCharacter_UpdateMainPanel_AddDamageTwoHand_asm()
{
__asm {
    push    ecx
    push    edx

    push    [ebp-68h]   // Cre
    push    [ebp-74h]   // TextArea
    push    [ebp-378h]  // screen
    call    CScreenCharacter_UpdateMainPanel_AddDamageMain

    push    [ebp-68h]   // Cre
    push    [ebp-74h]   // TextArea
    push    [ebp-378h]  // screen
    call    CScreenCharacter_UpdateMainPanel_AddDamageOff

    pop     edx
    pop     ecx

    lea     ecx, [ebp-0DCh]  // return to orig code
    ret
}
}


//void __declspec(naked)
//CScreenCharacter_UpdateStyleBonus_AddPenalty_asm()
//{
//__asm {
//    push    ecx
//    push    edx
//
//    push    [ebp+8]     // TextArea
//    push    [ebp-40h]   // screen
//    call    CScreenCharacter_UpdateStyleBonus_AddPenalty
//
//    pop     edx
//    pop     ecx
//
//    add     esp, 4
//    push    06D9BD1h    // skip error
//    ret
//}
//}