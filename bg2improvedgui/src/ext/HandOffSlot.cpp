#include "HandOffSlot.h"
#include "Animation5000.h"
#include "ItemCore.h"
#include "EngineCommon.h"

void (ResContainer::*Tramp_ResBamContainer_SetRes)(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing) =
	SetFP(static_cast<void (ResContainer::*)(ResRef&, int, int)> (&ResContainer::ResBamContainer_SetRes), 0x46A4D0);

BOOL (ResContainer::*Tramp_FindKey)(ResRef& ResRef, int a1, int a2) =
	SetFP(static_cast<BOOL (ResContainer::*)(ResRef&, int, int)> (&ResContainer::FindKey), 0x99B009);


void ResContainer::ResBamContainer_SetRes(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing) \
    { return (this->*Tramp_ResBamContainer_SetRes)(NewResRef, bSetAutoRequest, bWarningIfMissing); }

BOOL ResContainer::FindKey(ResRef& NewResRef, int a1, int a2) \
    { return (this->*Tramp_FindKey)(NewResRef, a1, a2); }


typedef unsigned __int64    QWORD;

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


//void DETOUR_ResContainer::DETOUR_ResBamContainer_SetRes(ResRef& NewResRef, int bSetAutoRequest, int bWarningIfMissing) {
//    char buf[9];
//    bool skip;
//
//    ResNameCopy(buf, NewResRef.GetResRef());
//    buf[8]='\0';
//
//    skip=false;
//    if (ResNameCmp(buf, "STONWEAP")) skip=true;
//    if (ResNameCmp(buf, "FIST")) skip=true;
//    if (ResNameCmp(buf, "STONQUIV")) skip=true;
//    if (ResNameCmp(buf, "STONBOOT")) skip=true;
//    if (ResNameCmp(buf, "STONRING")) skip=true;
//    if (ResNameCmp(buf, "STONITEM")) skip=true;
//    if (ResNameCmp(buf, "STONBELT")) skip=true;
//    if (ResNameCmp(buf, "STONGLET")) skip=true;
//    if (ResNameCmp(buf, "STONHELM")) skip=true;
//    if (ResNameCmp(buf, "STONAMUL")) skip=true;
//    if (ResNameCmp(buf, "STONCLOK")) skip=true;
//    if (ResNameCmp(buf, "STONSHIL")) skip=true;
//    if (ResNameCmp(buf, "STONARM")) skip=true;
//    if (ResNameCmp(buf, "STONSPEL")) skip=true;
//    if (strlen(buf) == 0) skip=true;
//
//    (this->*Tramp_ResBamContainer_SetRes)(NewResRef, bSetAutoRequest, bWarningIfMissing);
//
//
//    if (skip == false) {
//        console.writef("ResLoad %s", buf);
//
//        if (this->buf1[1])
//            console.writef("\n");
//        else
//            console.writef(" err\n");
//    }
//}
//
//
//BOOL DETOUR_ResContainer::DETOUR_FindKey(ResRef& NewResRef, int a1, int a2) {
//    static int line;
//    DWORD* Eip2;
//
//    char buf[9];
//    bool skip;
//
//    ResNameCopy(buf, NewResRef.GetResRef());
//    buf[8]='\0';
//
//    skip=false;
//    if (ResNameCmp(buf, "STONWEAP")) skip=true;
//    if (ResNameCmp(buf, "FIST")) skip=true;
//    if (ResNameCmp(buf, "STONQUIV")) skip=true;
//    if (ResNameCmp(buf, "STONBOOT")) skip=true;
//    if (ResNameCmp(buf, "STONRING")) skip=true;
//    if (ResNameCmp(buf, "STONITEM")) skip=true;
//    if (ResNameCmp(buf, "STONBELT")) skip=true;
//    if (ResNameCmp(buf, "STONGLET")) skip=true;
//    if (ResNameCmp(buf, "STONHELM")) skip=true;
//    if (ResNameCmp(buf, "STONAMUL")) skip=true;
//    if (ResNameCmp(buf, "STONCLOK")) skip=true;
//    if (ResNameCmp(buf, "STONSHIL")) skip=true;
//    if (ResNameCmp(buf, "STONARM")) skip=true;
//    if (ResNameCmp(buf, "STONSPEL")) skip=true;
//    if (ResNameCmp(buf, "SPWI104")) skip=true;
//    if (strlen(buf) == 0) skip=true;
//
//    __asm mov Eip2, esp
//
//        Eip2 += 23;
//    if (skip == false) {
//        line++;
//        console.writef("%d FindKey %s 0x%X \n", line, buf, *Eip2);
//    }
//
//    return (this->*Tramp_FindKey)(NewResRef, a1, a2);
//}


bool inline
isOffHandDisabled(const CCreatureObject& Cre)
{
    //CAnimation5000* const pAnimation5  = (CAnimation5000 *) Cre.animation.pAnimation;
    //CAnimation6400* const pAnimation6  = (CAnimation6400 *) Cre.animation.pAnimation;
    //
    //if (IsAnim5000(pAnimation5)) {      // bg2 humans
    //    return ( pAnimation5->pvcShieldCurrent == NULL && Cre.u6753 == 0x01 ); }
    //else
    //if (IsAnim6400(pAnimation6)) {      // bg1 humans
    //    return ( pAnimation6->pvcShieldCurrent == NULL && Cre.u6753 == 0x01 ); }
    //else

    return (Cre.u6753 == OffHand_OFF);
}


bool inline
isOffHandLimited(const CCreatureObject& Cre)
{
    return (Cre.u6753 != OffHand_Full);
}


bool inline
IsCreatureAllowed(CCreatureObject& Cre)
{
    /*for (int i = 0; i < g_pChitin->pGame->numInParty; i++) {
        if (g_pChitin->pGame->ePlayersPartyOrder[i] == Cre.e) {
            return true;
        }
    }
    return false;*/

    // check all creatures
    // even on NPC shield slot will be disabled if two hand weapon equipped
    return true;
}


/*
        Off Hand states:
        Off      0 0 = 0
        Effects  0 1 = 1
        AnimOnly 1 0 = 2
        Full     1 1 = 3
*/

/*
1 one hand melee               + shield
2 two hand melee               + shield
3 one hand launcher/range      + shield
4 two hand launcher/range      + shield
5 ammo + one hand launcher     + shield
6 ammo + two hand launcher     + shield
13 one hand magic              + shield
14 two hand magic              + shield

7  one hand melee              + weapon
8  two hand melee              + weapon
9  one hand launcher/range     + weapon
10 two hand launcher/range     + weapon
11 ammo + one hand launcher    + weapon
12 ammo + two hand launcher    + weapon
15 one hand magic              + weapon
16 two hand magic              + weapon
*/

bool
IsOffHandAllowed(CCreatureObject& Cre, CItem* const pOffHandItem)
{
    if (!IsCreatureAllowed(Cre))
        {return true;}

    char   const nSlotSelected  = Cre.Inventory.nSlotSelected;
    CItem* const pItem          = Cre.Inventory.items[nSlotSelected];

    if (!pItem || !pOffHandItem) {
        // OK Anim
        // no items
        return true;
    }

    if ((nSlotSelected >= SLOT_WEAPON0 && nSlotSelected <= SLOT_WEAPON3) ||
        nSlotSelected == SLOT_FIST ||
        nSlotSelected == SLOT_MISC19) { // magic weapon
        if (pItem->GetFlags() & ITEMFLAG_TWO_HANDED) {
            // REMOVE Anim
            // 2  two hand melee           + shield
            // 4  two hand launcher/range  + shield
            // 14 two hand magic           + shield
            // 8  two hand melee           + weapon
            // 10 two hand launcher/range  + weapon
            // 16 two hand magic           + weapon
            return false;
        } 

        pItem->Demand();

        short nAbilityIdx = Cre.Inventory.nAbilitySelected;
        ItmFileAbility* ability = pItem->GetAbility(nAbilityIdx);

        if (ability) {
            if ( (ability->attackType == ITEMABILITYATTACKTYPE_RANGED || 
                  ability->attackType == ITEMABILITYATTACKTYPE_LAUNCHER)  &&
                 pOffHandItem->GetType() != ITEMTYPE_SHIELD ) {
                // REMOVE Anim
                // 9 one hand launcher/range main + weapon
                pItem->Release();
                return false;
            }

            // OK Anim
            // 1 one hand melee           + shield
            // 3 one hand launcher/range  + shield
            // 13 one hand magi           + shield
            // 7 one hand melee           + weapon
            // 15 one hand magic          + weapon
            pItem->Release();
            return true;
        } else {
            console.writef("ERROR: Weapon= %s without abilities \n", pItem->itm.name.GetResRefNulled());
            // OK Anim
            // error
            pItem->Release();
            return true;
        }
    }

    if (nSlotSelected >= SLOT_AMMO0 && nSlotSelected <= SLOT_AMMO3) {
        pItem->Demand();

        short nAbilityIdx = Cre.Inventory.nAbilitySelected;
        ItmFileAbility* ability = pItem->GetAbility(nAbilityIdx);

        short nSlot;
        CItem* pItemLauncher = NULL;
        if (&ability) {
            pItemLauncher = Cre.GetFirstEquippedLauncherOfAbility(ability, nSlot);
        } else {
            console.writef("ERROR: Ammo= %s without abilities \n", pItem->itm.name.GetResRefNulled());
            // Anim OK
            // error
            pItem->Release();
            return true;
        }

        if (pItemLauncher) {
            if (pItemLauncher->GetFlags() & ITEMFLAG_TWO_HANDED) {
                // REMOVE Anim
                // 6  ammo + two-hand launcher + shield
                // 12 ammo + two-hand launcher + weapon
                pItem->Release();
                return false;
            }

            if (pOffHandItem->GetType() != ITEMTYPE_SHIELD) {
                // REMOVE Anim
                // 11 ammo + one hand ranged + weapon off
                pItem->Release();
                return false;
            }

            // OK Anim
            // 5 ammo + one-hand launcher + shield
            pItem->Release();
            return true;
        } else {
            // error, active ammo without launcher
            // OK Anim
            pItem->Release();
            return true;
        }
    }
    
    // Anim OK
    // no active slot
    return true;
}


void
ReConfigOffHand(CCreatureObject& Cre, CItem* const pItem, int const nSlotSelected, BOOL bAnimationOnly, bool bSpecialCaller)
{
    if (!IsCreatureAllowed(Cre))
        { return; }

    CItem* const pOffHandItem  = Cre.Inventory.items[SLOT_SHIELD];
    bool Launcher        = false;
    bool TwoHandLauncher = false;
    bool REMOVE          = false;
    bool AnimOK          = false;

    DEBUG_hand(" reconf() prev=%d \n", Cre.u6753)

    if (!pOffHandItem || !pItem) {
        // Skip, one hand is free
        DEBUG_hand(" reconf return: no offhand, prev=%d \n", Cre.u6753)
        return;
    }

    pItem->Demand();

    if ((nSlotSelected >= SLOT_WEAPON0 && nSlotSelected <= SLOT_WEAPON3) ||
         nSlotSelected == SLOT_FIST ||
         nSlotSelected == SLOT_MISC19) { // magic weapon
        short nAbilityIdx = Cre.Inventory.nAbilitySelected;
        ItmFileAbility* ability = pItem->GetAbility(nAbilityIdx);

        if (&ability) {
            if ( (ability->attackType == ITEMABILITYATTACKTYPE_RANGED ||
                  ability->attackType == ITEMABILITYATTACKTYPE_LAUNCHER)  &&
                 !(pItem->GetFlags() & ITEMFLAG_TWO_HANDED) &&
                 pOffHandItem->GetType() == ITEMTYPE_SHIELD ) {
                // Anim OK
                // 3 one hand launcher/range + shield
                AnimOK = true;
                goto _finish;
            }

            if (ability->attackType == ITEMABILITYATTACKTYPE_MELEE &&
                !(pItem->GetFlags() & ITEMFLAG_TWO_HANDED)) {
                // Anim OK
                // 1 one hand melee + shield
                // 7 one hand melee + weapon
                AnimOK = true;
                goto _finish;
            }

            // REMOVE Anim
            // 2  two hand melee           + shield
            // 4  two hand launcher/range  + shield
            // 8  two hand melee           + weapon
            // 9  one hand launcher/range  + weapon
            // 10 two hand launcher/range  + weapon
            REMOVE = true;
            goto _finish;
        } else {
            console.writef("ERROR: Weapon= %s without abilities \n", pItem->itm.name.GetResRefNulled());
            // Anim OK
            // error
            AnimOK = true;
            goto _finish;
        }    
    }

    else {
    if (nSlotSelected >= SLOT_AMMO0 && nSlotSelected <= SLOT_AMMO3) {
        CItem* const pItemSelected = Cre.Inventory.items[nSlotSelected];

        if (pItem == pItemSelected) {
            // Stage 2. Item is ammo, skip checks

            //CItem* pItemLauncher = NULL;
            //short nAbilityIdx = Cre.Inventory.nAbilitySelected;
            //ItmFileAbility& ability = Item->GetAbility(nAbilityIdx);
            //short nSlot;

            //if (&ability) {
            //    ItemLauncher = & Cre.GetFirstEquippedLauncherOfAbility(ability, &nSlot);
            //} else {
            //    console.writef("ERROR: Ammo= %s without abilities", Item->itm.name.GetResRefNulled());
            //    // Anim OK
            //    // error
            //    AnimOK = true;
            //    goto _finish;
            //}

            AnimOK = true;
            goto _finish;
        }
        //else {
             // Stage 1. Item is launcher
             CItem* pItemLauncher = pItem;
        //}

        //if (pItemLauncher) {
            Launcher = true;
            if (pItemLauncher->GetFlags() & ITEMFLAG_TWO_HANDED) {
                // ammo + two hand launcher + xxx
                TwoHandLauncher = true;
            }
        //} else {
        //    // Anim OK
        //    // error, ammo without launcher
        //    AnimOK = true;
        //    goto _finish;
        //}

        if ( Launcher &&
            !TwoHandLauncher &&
            pOffHandItem->GetType() == ITEMTYPE_SHIELD ) {
                // Anim OK
                // 5 ammo + one hand launcher + shield
                AnimOK = true;
                goto _finish;
        }

        // REMOVE Anim
        // 6  ammo + two hand launcher + shield
        // 11 ammo + one hand launcher + weapon
        // 12 ammo + two hand launcher + weapon
        REMOVE = true;
        goto _finish;
    }
    }

_finish:
    pItem->Release();

    if (AnimOK && isOffHandLimited(Cre)) {  // off hand not fully active
        /*
                    Restore Off hand:
              Type     | PrevState | Result
            --------------------------------
             Anim	   | Off       | equip(anim), AnimOnly
             Anim      | Effects   | equip(anim), Full
             Anim      | AnimOnly  | skip, AnimOnly
             Anim	   | Full      | skip, Full

             Full	   | Off       | equip(full), Full
             Full      | Effects   | equip(anim), Full
             Full      | AnimOnly  | uneqip(anim)?, equip(full), Full
             Full	   | Full      | skip, Full
        */

        DEBUG_hand(" reconf: restore offhand, anim=%d prev=%d itm=%s ", bAnimationOnly, Cre.u6753, pItem->itm.name.GetResRefNulled());

        if (bAnimationOnly) {   // main hand equipping anim only
            // main hand anim equipping, restore off hand
            if (Cre.u6753 == OffHand_Effects) {  // Effects + Anim -> Full
                DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum);
                (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum++);
                Cre.u6753 = OffHand_Full;
            } else
            if (Cre.u6753 == OffHand_OFF) {
                DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum);
                (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum++);
                Cre.u6753 = OffHand_Anim;
            }
        } else {
            // main hand full equipping, restore off hand
            if (Cre.u6753 == OffHand_Effects) {  // Effects + Anim -> Full
                DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum);
                (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) call=%d \n", debug_callnum++);
                Cre.u6753 = OffHand_Full;
            } else
            if (Cre.u6753 == OffHand_OFF) {  // Off -> Full
                DEBUG_hand(" call Equip(SLOT_SHIELD, %d) call=%d \n", bSpecialCaller ? 0xA6 : 0xA5, debug_callnum);
                pOffHandItem->Equip(Cre, SLOT_SHIELD, bSpecialCaller ? 0xA6 : 0xA5); // full equip, bypass mode
                DEBUG_hand(" return Equip(SLOT_SHIELD, x) call=%d \n", debug_callnum++);
                Cre.u6753 = OffHand_Full;
            } else
            if (Cre.u6753 == OffHand_Anim) { // Anim only -> Full
                //(pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 0, 1);   // remove anim only
                DEBUG_hand(" call Equip(SLOT_SHIELD, %d) call=%d \n", bSpecialCaller ? 0xA6 : 0xA5, debug_callnum);
                pOffHandItem->Equip(Cre, SLOT_SHIELD, bSpecialCaller ? 0xA6 : 0xA5); // full equip, bypass mode
                DEBUG_hand(" return Equip(SLOT_SHIELD, x) call=%d \n", debug_callnum++);
                Cre.u6753 = OffHand_Full;
            }
        }

        DEBUG_hand(" reconf: new=%d \n", Cre.u6753);
    } else

    if (REMOVE && !isOffHandDisabled(Cre)) {        // off hand was enabled, need change
        /*        Equiping Main hand conflict:
                Main hand | PrevState | Result
            --------------------------------
                Anim	  | Off       | skip, Off
                Anim      | Effects   | skip, Effects
                Anim      | AnimOnly  | unequip(anim), Off
                Anim	  | Full      | unequip(anim), Effects

                Full	  | Off       | skip, Off
                Full      | Effects   | unequip(full), Off
                Full      | AnimOnly  | unequip(anim), Off
                Full	  | Full      | unequip(full), Off
        */

        DEBUG_hand(" reconf: remove offhand, anim=%d prev=%d itm=%s \n", bAnimationOnly, Cre.u6753, pItem->itm.name.GetResRefNulled());

        if (bAnimationOnly) {   // main hand equipping anim only 
            if (Cre.u6753 == OffHand_Anim) {    // Anim only -> Off
                DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                (pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 0, 1);   // remove anim only 
                DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                Cre.u6753 = OffHand_OFF;
            } else
            if (Cre.u6753 == OffHand_Full) {    // Full -> Effects
                DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                (pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 0, 1);   // remove anim only 
                DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                Cre.u6753 = OffHand_Effects;
            }
        } else {
            if (Cre.u6753 == OffHand_Anim) {    // Anim only -> Off
                DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                (pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 0, 1);   // remove anim only 
                DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, 0, 1) \n");
                Cre.u6753 = OffHand_OFF;
            } else {                            // Effects OR Full -> Off
                DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, 1, 0) \n");
                (pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 1, 0);   // full remove
                DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, 1, 0) \n");
                Cre.u6753 = OffHand_OFF;
            }
        }

        DEBUG_hand(" reconf: new=%d \n", Cre.u6753);
    } else {

        DEBUG_hand(" reconf: no changes, prev=%d \n", Cre.u6753);
    }

    DEBUG_hand(" reconf return new=%d \n", Cre.u6753)
}


void
DETOUR_CItem::DETOUR_UnEquip(CCreatureObject& Cre, int const nSlot, BOOL const bRecalculateEffects, BOOL const bAnimationOnly)
{
    DWORD Eip;
	GetEip(Eip);
    //  UnequipAll()         Eip
    //  unequip main 	   0x8CA7F9
    //  unequip offhand    0x8CA6A3
    //  unequip Launcher   0x8CA86D

    DEBUG_hand("CItem::UnEquip(recalc=%d anim=%d itm=%s) nSlot=%d IP=%X \n",
                        bRecalculateEffects,
                        bAnimationOnly,
                        this->itm.name.GetResRefNulled(),
                        nSlot,
                        Eip)

    CItem* const  pOffHandItem  = Cre.Inventory.items[SLOT_SHIELD];
    char         nSlotSelected  = Cre.Inventory.nSlotSelected;
    CItem* const pMainHandItem  = Cre.Inventory.items[nSlotSelected];
    short     nAbilitySelected  = Cre.Inventory.nAbilitySelected;

    switch (nSlot) {
    case SLOT_SHIELD:
        if (!IsCreatureAllowed(Cre)) {
            DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, %d) \n", bRecalculateEffects, bAnimationOnly);
            (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, bAnimationOnly);
            DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, x) \n");
            return;
        }

        if (!isOffHandDisabled(Cre)) {
         /*
            #define OffHand_OFF         0
            #define OffHand_Effects     1
            #define OffHand_Anim        2
            #define OffHand_Full        3
         
                        Unequip Off hand:
                  Type     | PrevState | Result
                --------------------------------
                 Anim	   | Off       | skip, Off                  *
                 Anim      | Effects   | error, Effects             *
                 Anim      | AnimOnly  | unequip(anim), Off         OffHand_Anim    ->  OffHand_OFF
                 Anim	   | Full      | unequip(anim), Effects     OffHand_Full    ->  OffHand_Effects

                 Full	   | Off       | skip, Off                  *
                 Full      | Effects   | unequip(full), Off         OffHand_Effects ->  OffHand_OFF
                 Full      | AnimOnly  | unequip(anim), Off         OffHand_Anim    ->  OffHand_OFF    
                 Full	   | Full      | unequip(full), Off         OffHand_Full    ->  OffHand_OFF
         */
            if (Cre.u6753 == OffHand_Effects && bAnimationOnly)
                console.writef("unequip Off hand error, Anim | Effects | error, Effects %s itm=%s \n", (LPTSTR)Cre.GetLongName(), this->itm.name.GetResRefNulled());

            DEBUG_hand("unequip offhand, prev=%d itm=%s \n", Cre.u6753, this->itm.name.GetResRefNulled());

            if (bAnimationOnly) {
                if (Cre.u6753 == OffHand_Anim) {    // Anim only -> Off
                    DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, 1) \n", bRecalculateEffects);
                    (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, 1);   // remove anim only 
                    DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, 1) \n");
                    Cre.u6753 = OffHand_OFF;
                } else
                if (Cre.u6753 == OffHand_Full) {    // Full -> Effects
                    DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, 1) \n", bRecalculateEffects);
                    (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, 1);   // remove anim only 
                    DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, 1) \n");
                    Cre.u6753 = OffHand_Effects;
                }
            } else {
                if (Cre.u6753 == OffHand_Anim) {    // Anim only -> Off
                    DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, 1) \n", bRecalculateEffects);
                    (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, 1);   // remove anim only 
                    DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, 1) \n");
                    Cre.u6753 = OffHand_OFF;
                } else {    // Effects, Full - > Off
                    DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, %d) \n", bRecalculateEffects, bAnimationOnly);
                    (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, bAnimationOnly); // anim or full remove
                    DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, x) \n");
                    Cre.u6753 = OffHand_OFF;
                }
            }

            DEBUG_hand("unEquip new=%d \n", Cre.u6753);
        } else {    // already disabled
            DEBUG_hand("unequip offhand, already disabled, prev=%d itm=%s new=0 \n", Cre.u6753, this->itm.name.GetResRefNulled());

            // remove anim from slot with already empty anim, just for safe
            DEBUG_hand(" call orig_UnEquip(SLOT_SHIELD, %d, 1) \n", bRecalculateEffects);
            (this->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, bRecalculateEffects, 1);   // remove anim only
            DEBUG_hand(" return orig_UnEquip(SLOT_SHIELD, x, 1) \n");
        }
        break;

    case SLOT_WEAPON0:
    case SLOT_WEAPON1:
    case SLOT_WEAPON2:
    case SLOT_WEAPON3:
    case SLOT_FIST:
    case SLOT_MISC19: // magic weapon
        DEBUG_hand(" call orig_UnEquip(nSlot=%d, %d, %d) \n", nSlot, bRecalculateEffects, bAnimationOnly);
        (this->*Tramp_CItem_UnEquip)(Cre, nSlot, bRecalculateEffects, bAnimationOnly);
        DEBUG_hand(" return orig_UnEquip(nSlot=%d, x, x) \n", nSlot);

        if (!IsCreatureAllowed(Cre))
            {return;}

        if (pOffHandItem && isOffHandLimited(Cre)) {
            /*
                    Unequip Main hand:
                      Type     | PrevState | Result
                    --------------------------------
                     Anim	   | Off       | equip(anim), AnimOnly              OffHand_Full    ->  OffHand_OFF
                     Anim      | Effects   | equip(anim), Full                  OffHand_Effects ->  OffHand_Full
                     Anim      | AnimOnly  | skip, AnimOnly                     *
                     Anim	   | Full      | skip, Full                         *

                     Full	   | Off       | equip(full), Full                  OffHand_OFF     ->  OffHand_Full
                     Full      | Effects   | equip(anim), Full                  OffHand_Effects ->  OffHand_Full
                     Full      | AnimOnly  | uneqip(anim)?, equip(full), Full   OffHand_Anim    ->  OffHand_Full
                     Full	   | Full      | skip, Full                         *
            */
            DEBUG_hand("unequip main, restore offhand, prev=%d itm=%s \n", Cre.u6753, this->itm.name.GetResRefNulled());

            if (bAnimationOnly) {
                if (Eip != 0x8CA7F9 &&    // don't restore offhand if called from CGameSprite::UnequipAll()
                    Eip != 0x8CA86D) {
                    if (Cre.u6753 == OffHand_Effects) {  // Effects + Anim -> Full
                        DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) \n");
                        (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                        DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) \n");
                        Cre.u6753 = OffHand_Full;
                    } else
                    if (Cre.u6753 == OffHand_OFF) {
                        DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) \n");
                        (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                        DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) \n");
                        Cre.u6753 = OffHand_Anim;
                    }
                } else {
                    DEBUG_hand("UnequipAll(), skip offhand  \n");
                }
            } else {
                if (Cre.u6753 == OffHand_Effects) {  // Effects + Anim -> Full
                    DEBUG_hand(" call orig_Equip(SLOT_SHIELD, 1) \n");
                    (pOffHandItem->*Tramp_CItem_Equip)(Cre, SLOT_SHIELD, 1);   // anim only
                    DEBUG_hand(" return orig_Equip(SLOT_SHIELD, 1) \n");
                    Cre.u6753 = OffHand_Full;
                } else
                if (Cre.u6753 == OffHand_Anim) {     // Anim only -> Full
                    //(pOffHandItem->*Tramp_CItem_UnEquip)(Cre, SLOT_SHIELD, 0, 1);   // remove anim only
                    DEBUG_hand(" call Equip(SLOT_SHIELD, 0xA5) \n");
                    pOffHandItem->Equip(Cre, SLOT_SHIELD, 0xA5);    // full equip, bypass mode
                    DEBUG_hand(" return Equip(SLOT_SHIELD, 0xA5) \n");
                    Cre.u6753 = OffHand_Full;
                } else
                if (Cre.u6753 == OffHand_OFF) {
                    DEBUG_hand(" call Equip(SLOT_SHIELD, 0xA5) \n");
                    pOffHandItem->Equip(Cre, SLOT_SHIELD, 0xA5);    // full equip, bypass mode
                    DEBUG_hand(" return Equip(SLOT_SHIELD, 0xA5) \n");
                    Cre.u6753 = OffHand_Full;
                }
            }

            DEBUG_hand("unEquip new=%d \n", Cre.u6753);
        } else {
            DEBUG_hand("unequip main, offhand already enabled/non-exist, prev=%d itm=%s eip=%X \n", Cre.u6753, this->itm.name.GetResRefNulled(), Eip);
        }
        break;

    // ammo slots don't change animation ?
    //case SLOT_AMMO0:
    //case SLOT_AMMO1:
    //case SLOT_AMMO2:
    //case SLOT_AMMO3:

    default:
        DEBUG_hand(" default call orig_UnEquip(nSlot=%d, %d, %d) \n",nSlot, bRecalculateEffects, bAnimationOnly);
        (this->*Tramp_CItem_UnEquip)(Cre, nSlot, bRecalculateEffects, bAnimationOnly);
        DEBUG_hand(" return default orig_UnEquip(SLOT_SHIELD, x, x) \n");
        break;
    }
}


short __stdcall
FakeShieldType(CCreatureObject& Cre, short ItemType)
{
    if (!IsCreatureAllowed(Cre))
        {return ItemType;}

    CItem* const pOffHandItem = Cre.Inventory.items[SLOT_SHIELD];

    if (pOffHandItem && isOffHandDisabled(Cre)) {
       // change item type to shield to fake checks
       ItemType = ITEMTYPE_SHIELD;
    }

    return ItemType;
}



void __declspec(naked)
ScreenRecordThacInfo_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax         ; ItemType
    push    [ebp-68h]   ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
ApplyDamage_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-614h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
CCreatureObject__Hit_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-294h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
CombatInfo_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-198h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}



void __declspec(naked)
WeaponStyleBonuses_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-18h]       ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
WeaponStyleBonuses2_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0               ; FakeItemType
    push    [ebp-18h]       ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    cmp     ax, 0x0C        ; is fakeshield ?
    jz      short localexit1
    ; Z=1 if fakeshield    => 113 PROFICIENCYSINGLEWEAPON

    mov     eax, [ebp-18h]
    cmp     dword ptr [eax+0A7Eh], 0  ; stolen bytes
    ; Z=0 if active shield => 112 PROFICIENCYSWORDANDSHIELD

localexit1:
    ret
}
}


void __declspec(naked)
ApplyLevelProgressTableSub_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-24h]       ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
ApplyLevelProgressTableSub2_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0               ; FakeItemType
    push    [ebp-24h]       ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    cmp     ax, 0x0C        ; is fakeshield ?
    jz      short localexit2
    ; Z=1 if fakeshield    => 113 PROFICIENCYSINGLEWEAPON

    mov     ecx, [ebp-24h]
    cmp     dword ptr [ecx+0A7Eh], 0  ; stolen bytes
    ; Z=0 if active shield => 112 PROFICIENCYSWORDANDSHIELD

localexit2:
    ret
}
}


void __declspec(naked)
Sub_8C29E6_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0               ; FakeItemType
    push    [ebp-2e8h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    cmp     ax, 0x0C        ; is fakeshield ?

    jz      short localexit3
    ; Z=1 if fakeshield => empty hand off

    mov     edx, [ebp-2e8h]
    cmp     dword ptr [edx+0A7Eh], 0  ; stolen bytes
    ; Z=1 => off hand slot is not empty

localexit3:
    ret
}
}


void __declspec(naked)
Sub_8C4E09_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0               ; FakeItemType
    push    [ebp-17Ch]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    cmp     ax, 0x0C        ; is fakeshield ?

    jz      short localexit4
    ; Z=1 if fakeshield => empty hand off

    mov     ecx, [ebp-17Ch]
    cmp     dword ptr [ecx+0A7Eh], 0  ; stolen bytes
    ; Z=1 => off hand slot is not empty

localexit4:
    ret
}
}


void __declspec(naked)
EquipRanged_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    eax             ; ItemType
    push    [ebp-0A8h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    and     eax, 0FFFFh  ; stolen bytes
    ret
}
}


void __declspec(naked)
EquipRanged2_asm()
{
__asm {
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    0               ; FakeItemType
    push    [ebp-0A8h]      ; CCreature
    call    FakeShieldType

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    cmp     ax, 0x0C        ; is fakeshield ?

    jz      short localexit5
    ; Z=1 if fakeshield => empty hand off

    mov     eax, [ebp-0A8h]
    cmp     dword ptr [eax+0A7Eh], 0  ; stolen bytes
    ; Z=1 => off hand slot is not empty

localexit5:
    ret
}
}


void __declspec(naked)
CCreatureObject_CCreatureObject_InitU6753_asm()
{
__asm {
    // edx - Cre
    mov     byte ptr [edx.u6753], 0 // off hand disabled

    mov     dword ptr [edx+6692h], 0FFFFFFFFh   // stolen bytes
    ret
}
}


void __declspec(naked)
CInfGame_SwapItemPersonal_CheckShieldSlot_asm()
{
__asm {
    push    ecx
    push    edx

    call    IsBG1Part
    test    al, al
    jz      CInfGame_SwapItemPersonal_CheckShieldSlot_asm_Skip

    mov     eax, [ebp+10h]  // *item
    mov     ecx, [eax]      // item
    test    ecx, ecx        // equiping
    jz      CInfGame_SwapItemPersonal_CheckShieldSlot_asm_Skip

    call    CItem::GetType
    cmp     eax, 0xC        // shield
    jz      CInfGame_SwapItemPersonal_CheckShieldSlot_asm_Skip

    pop     edx
    pop     ecx

    add     esp, 4
    push    069C212h        // not allowed
    ret

CInfGame_SwapItemPersonal_CheckShieldSlot_asm_Skip:
    pop     edx
    pop     ecx

    mov     dword ptr [ebp-6Ch], 1  // stolen bytes
    ret
}
}