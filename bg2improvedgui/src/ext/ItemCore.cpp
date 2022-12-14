#include "ItemCore.h"
#include "HandOffSlot.h"

#include "Animation5000.h"
#include "objcre.h"
#include "effopcode.h"
#include "ObjectStats.h"
#include "EngineCommon.h"

void (CItem::*Tramp_CItem_Equip)(CCreatureObject& cre, int nSlot, BOOL bAnimationOnly) =
	SetFP(static_cast<void (CItem::*)(CCreatureObject&, int, BOOL)>	(&CItem::Equip),			0x5AA430);
void (CItem::*Tramp_CItem_UnEquip)(CCreatureObject& cre, int nSlot, BOOL bRecalculateEffects, BOOL bAnimationOnly) =
	SetFP(static_cast<void (CItem::*)(CCreatureObject&, int, BOOL , BOOL)>	(&CItem::UnEquip),	0x5AAA10);
CEffect* (CItem::*Tramp_CItem_GetAbilityEffect)(int, int, CCreatureObject*) =
	SetFP(static_cast<CEffect* (CItem::*)(int, int, CCreatureObject*)>	(&CItem::GetAbilityEffect),	0x5AB168);

void DETOUR_CItem::DETOUR_Equip(CCreatureObject& cre, int nSlot, BOOL bAnimationOnly) { // bAnimationOnly used as int
	DWORD Eip;
	GetEip(Eip);
    bool    bSpriteEquipmentUnmarshalCall;

    // EquipAll() Eip:
    // equip main 	    0x8CAA55
    // equip offhand    0x8CA90F

    if (Eip != 0x8BFEA5 &&  // CGameSpriteEquipment::Unmarshal caller
	    Eip != 0x8C002D &&
	    Eip != 0x8C01B6 &&
	    Eip != 0x8C033F &&
	    Eip != 0x8C04C8 &&
	    Eip != 0x8C0651 &&
	    Eip != 0x8C07D4 &&
	    Eip != 0x8C095D &&
	    Eip != 0x8C0AE6 &&
	    Eip != 0x8C0C6F &&
	    Eip != 0x8C1B9E &&
	    Eip != 0x8C1BE0 &&
        bAnimationOnly != 0xA6) // A6 - Unmarshal marker
            bSpriteEquipmentUnmarshalCall = false;
    else {
            bSpriteEquipmentUnmarshalCall = true;

            // CGameSpriteEquipment::Unmarshal()->Equip(main)->
            // ->ReConfigOffHand()->Equip(offhand)
            if (bAnimationOnly == 0xA6)
                bAnimationOnly = 0xA5;
    }

    if (pGameOptionsEx->bUI_UnlimitedHandOffSlot && bAnimationOnly != 0xA5) {    // bypass marker
        switch (nSlot) {
        case SLOT_SHIELD:
            /*
                          Equip Allowed Off hand:
                      Type     | PrevState | Result
                    --------------------------------
                     Anim	   | Off       | equip(anim), AnimOnly
                     Anim      | Effects   | equip(anim), Full
                     Anim      | AnimOnly  | error, AnimOnly
                     Anim	   | Full      | error, Full

                     Full	   | Off       | equip(full), Full
                     Full      | Effects   | error, Full
                     Full      | AnimOnly  | uneqip(anim)?, equip(full), Full
                     Full	   | Full      | error, Full


                  Equiping Off hand after conflicted Main hand:
                     Type      | PrevState | Result
                    --------------------------------
                     Anim	   | Off       | skip, Off
                     Anim      | Effects   | error, Off
                     Anim      | AnimOnly  | error, Off
                     Anim	   | Full      | error, Off

                     Full	   | Off       | skip, Off
                     Full      | Effects   | error, Off
                     Full      | AnimOnly  | error, Off
                     Full	   | Full      | error, Off
            */

            //if (pGameOptionsEx->bUI_DisableOffHandWeaponBG1part) {
            //    if (IsBG1Part() && this->GetType() != ITEMTYPE_SHIELD) {   // in BG1 only shield allowed
            //        cre.u6753 = OffHand_OFF;
            //        return;                 // abort equip
            //    }
            //}

            if (IsOffHandAllowed(cre, this)) {
                if (cre.u6753 == OffHand_Full)
                    console.writef("equip offhand error, xxx | Full | error, Full %s %s %X \n", (LPTSTR)cre.GetLongName(), this->itm.name.GetResRefNulled(), Eip);
                if (cre.u6753 == OffHand_Effects && !bAnimationOnly)
                    console.writef("equip offhand error, Full | Effects | error, Full %s %s %X \n", (LPTSTR)cre.GetLongName(), this->itm.name.GetResRefNulled(), Eip);
                if (cre.u6753 == OffHand_Anim && bAnimationOnly)
                    console.writef("equip offhand error, Anim | AnimOnly | error, AnimOnly %s %s %X \n", (LPTSTR)cre.GetLongName(), this->itm.name.GetResRefNulled(), Eip);

                //console.write_debug("equip offhand, anim=%d prev=%d %s ", bAnimationOnly, cre.u6753, this->itm.name.GetResRefNulled());

                (this->*Tramp_CItem_Equip)(cre, SLOT_SHIELD, bAnimationOnly);
                if (bAnimationOnly) {
                    cre.u6753 |= OffHand_Anim; // Off -> Anim Only or Effects -> Full
                } else {
                    cre.u6753 = OffHand_Full; // full enabled
                }

                //console.write_debug("new=%d \n", cre.u6753);
            } else {
                if (cre.u6753 != OffHand_OFF)
                    console.writef("Equiping Off hand after conflicted Main hand error, xxx | !Full | error, Off %s %s \n", (LPTSTR)cre.GetLongName(), this->itm.name.GetResRefNulled());

                //console.write_debug("equip offhand, forbidden by main, anim=%d prev=%d %s new=0 \n", bAnimationOnly, cre.u6753, this->itm.name.GetResRefNulled());

                cre.u6753 = OffHand_OFF;
                return;                 // abort equip
            }
            break;

        case SLOT_WEAPON0:
        case SLOT_WEAPON1:
        case SLOT_WEAPON2:
        case SLOT_WEAPON3:
        case SLOT_FIST:
        case SLOT_MISC19:   // magic weapon
            //console.write_debug("equip main anim=%d, wait reconfig prev=%d %s \n", bAnimationOnly, cre.u6753, this->itm.name.GetResRefNulled());
            ReConfigOffHand(cre, this, nSlot, bAnimationOnly, bSpriteEquipmentUnmarshalCall);

            (this->*Tramp_CItem_Equip)(cre, nSlot, bAnimationOnly);
            break;

        // skip ammo
        //case SLOT_AMMO0:
        //case SLOT_AMMO1:
        //case SLOT_AMMO2:
        //case SLOT_AMMO3:

        default:
            (this->*Tramp_CItem_Equip)(cre, nSlot, bAnimationOnly);
            break;
        }
    }
    else {  // bypass Unlimited HandOff Slot/Marker
        if (bAnimationOnly != 0xA5)     // Normal Mode
            (this->*Tramp_CItem_Equip)(cre, nSlot, bAnimationOnly);
        else                            // Bypass mode
         (this->*Tramp_CItem_Equip)(cre, nSlot, 0); // full equip
    }

    if (pGameOptionsEx->bEffApplyEffItemFix ||
        pGameOptionsEx->bEffApplyEffItemtypeFix) {

        // CGameSpriteEquipment::Unmarshal calls should not apply these effects
	    if (!bSpriteEquipmentUnmarshalCall) {
		    if (itm.name.IsEmpty()) return;
		    if (itm.pRes == NULL) return;
		    if (bAnimationOnly) return;

		    cre.bInEquipItem = TRUE;
		
		    //effect 182
		    if (pGameOptionsEx->bEffApplyEffItemFix) {
			    POSITION pos = cre.cdsCurrent.ApplyEffOnEquipItem.GetHeadPosition();
			    while (pos != NULL) {
				    COnEquipItem* pOnEquipItem = (COnEquipItem*)cre.cdsCurrent.ApplyEffOnEquipItem.GetNext(pos);
				    if (pOnEquipItem->rItem == itm.name) {
					    if (pOnEquipItem->pEffect != NULL) {
						    cre.AddEffect(pOnEquipItem->pEffect->Duplicate(), true, FALSE, TRUE);
					    }
				    }
			    }
		    }

		    //effect 183
		    if (pGameOptionsEx->bEffApplyEffItemtypeFix) {
			    short nItemType = GetType();
			    POSITION pos = cre.cdsCurrent.ApplyEffOnEquipItemType.GetHeadPosition();
			    while (pos != NULL) {
				    COnEquipItemType* pOnEquipItemType = (COnEquipItemType*)cre.cdsCurrent.ApplyEffOnEquipItemType.GetNext(pos);
				    if (pOnEquipItemType->nItemType == (int)nItemType) {
					    if (pOnEquipItemType->pEffect != NULL) {
						    cre.AddEffect(pOnEquipItemType->pEffect->Duplicate(), true, FALSE, TRUE);
					    }
				    }
			    }
		    }

	    cre.bInEquipItem = FALSE;
	    }
    }

}


CEffect* DETOUR_CItem::DETOUR_GetAbilityEffect(int nAbilityIdx, int nEffectIdx, CCreatureObject* creSource) {
	CEffect* eff = (this->*Tramp_CItem_GetAbilityEffect)(nAbilityIdx, nEffectIdx, creSource);

	if (pGameOptionsEx->bEngineExpandedStats) {
		if (eff != NULL &&
			eff->effect.nOpcode == CEFFECT_OPCODE_DAMAGE &&
			&creSource != NULL &&
			creSource->GetObjectType() == CGAMEOBJECT_TYPE_CREATURE) {

			unsigned int nDamageType = eff->effect.nParam2 & 0xFFFF0000;
			int nDamageBehavior = eff->effect.nParam2 & 0xFFFF;

			if (nDamageBehavior == EFFECTDAMAGE_BEHAVIOR_NORMAL) {
				switch (nDamageType) {
				case DAMAGETYPE_ACID:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_ACIDDAMAGEBONUS);
					break;
				case DAMAGETYPE_COLD:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_COLDDAMAGEBONUS);
					break;
				case DAMAGETYPE_CRUSHING:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_CRUSHINGDAMAGEBONUS);
					break;
				case DAMAGETYPE_STUNNING:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_STUNNINGDAMAGEBONUS);
					break;
				case DAMAGETYPE_PIERCING:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_PIERCINGDAMAGEBONUS);
					break;
				case DAMAGETYPE_SLASHING:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_SLASHINGDAMAGEBONUS);
					break;
				case DAMAGETYPE_ELECTRICITY:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_ELECTRICITYDAMAGEBONUS);
					break;
				case DAMAGETYPE_FIRE:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_FIREDAMAGEBONUS);
					break;
				case DAMAGETYPE_POISON:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_POISONDAMAGEBONUS);
					break;
				case DAMAGETYPE_MAGIC:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_MAGICDAMAGEBONUS);
					break;
				case DAMAGETYPE_MISSILE:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_MISSILEDAMAGEBONUS);
					break;
				case DAMAGETYPE_MAGICFIRE:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_MAGICFIREDAMAGEBONUS);
					break;
				case DAMAGETYPE_MAGICCOLD:
					eff->effect.nParam3 = creSource->GetDerivedStats().GetStat(CDERIVEDSTATSEX_BASE + CDERIVEDSTATSEX_MAGICCOLDDAMAGEBONUS);
					break;
				default:
					break;
				}
			}
		}
	}

	return eff;
}


void static __stdcall
AddSeqREADYMessage(CCreatureObject& Cre) {
    CDerivedStats& cds = Cre.GetDerivedStats();
    if (!(cds.stateFlags & STATE_DEAD)) {
        CMessageSetAnimationSequence* pMsg = IENew CMessageSetAnimationSequence();
          pMsg->eSource = Cre.id;
          pMsg->eTarget = Cre.id;
          pMsg->nSeq = SEQ_READY;
          g_pChitin->messages.Send(*pMsg, FALSE);
    }
}


void __declspec(naked)
CItem_Equip_ResetIdleTimer_Shield_asm()
{
__asm {
	push    eax
	push    ecx
	push    edx

	mov     eax, [ebp+8]                // Cre
    cmp     DWORD ptr [eax+036c8h], 0   // skip broadcast if timer already cleared
    jz      CItem_Equip_ResetIdleTimer_Shield_asm_Skip      
    mov     DWORD ptr [eax+036c8h], 0   // noActionCount
    push    [ebp+8]                     // Cre
    call    AddSeqREADYMessage

CItem_Equip_ResetIdleTimer_Shield_asm_Skip:
    pop     edx
	pop     ecx
	pop     eax

	add     edx, 41Ah       // stolen bytes
	ret
}
}


void __declspec(naked)
CItem_Equip_ResetIdleTimer_OffHand_asm()
{
__asm {
	push    eax
	push    ecx
	push    edx

	mov     eax, [ebp+8]                // Cre
    cmp     DWORD ptr [eax+036c8h], 0   // skip broadcast if timer already cleared
    jz      CItem_Equip_ResetIdleTimer_OffHand_asm_Skip      
    mov     DWORD ptr [eax+036c8h], 0   // noActionCount
    push    [ebp+8]                     // Cre
    call    AddSeqREADYMessage

CItem_Equip_ResetIdleTimer_OffHand_asm_Skip:
	pop     edx
	pop     ecx
	pop     eax

	add     ecx, 41Ah       // stolen bytes
	ret
}
}


void __declspec(naked)
CItem_Equip_ResetIdleTimer_MainHand_asm()
{
__asm {
	push    eax
	push    ecx
	push    edx

	mov     eax, [ebp+8]                // Cre
    mov     DWORD ptr [eax+036c8h], 0   // noActionCount

	pop     edx
	pop     ecx
	pop     eax
	
	add     ecx, 41Ah       // stolen bytes
	ret
}
}