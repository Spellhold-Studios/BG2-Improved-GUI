#include "MessageExt.h"

#include "chitin.h"
#include "AreaCommon.h"

void CMessageRemoveAreaAirEffectSpecific::Marshal(void* pData, int* dwSize) {
    if (pData == NULL || *dwSize == 0) {
        //normally an assert continue
        LPCTSTR lpsz = "CMessageRemoveAreaAirEffectSpecific::Marshal(): pData or *dwSize is NULL\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
    }
    *dwSize = 16;
    pData = IENew CMessageRemoveAreaAirEffectSpecificM();
    if (pData == NULL) {
        //normally an assert continue
        *dwSize = 0;
        LPCTSTR lpsz = "CMessageRemoveAreaAirEffectSpecific::Marshal(): IENew failed\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        return;
    }

    int cnt = 0;
    ((CMessageRemoveAreaAirEffectSpecificM*)pData)->rAreaName = this->rAreaName.GetResRefNulled();
    cnt += 8;
    ((CMessageRemoveAreaAirEffectSpecificM*)pData)->rResource = this->rResource.GetResRefNulled();
    cnt += 8;

    //if (cnt != *dwSize) {
    //    //normally an assert continue
    //    LPCTSTR lpsz = "CMessageRemoveAreaAirEffectSpecific::Marshal(): cnt != *dwSize\r\n";
    //    L.timestamp();
    //    L.append(lpsz);
    //    console.write(lpsz);
    //}

    return;
}

BOOL CMessageRemoveAreaAirEffectSpecific::Unmarshal(void* pData, int* dwSize) {
    if (pData == NULL) {
        //normally an assert continue
        LPCTSTR lpsz = "CMessageRemoveAreaAirEffectSpecific::Unmarshal(): pData is NULL\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
        return TRUE;
    }
    int cnt = 0;
    this->rAreaName = ((CMessageRemoveAreaAirEffectSpecific*)pData)->rAreaName.GetResRefNulled();
    cnt += 8;
    this->rResource = ((CMessageRemoveAreaAirEffectSpecific*)pData)->rResource.GetResRefNulled();
    cnt += 8;

    if (cnt != *dwSize) {
        //normally an assert continue
        LPCTSTR lpsz = "CMessageRemoveAreaAirEffectSpecific::Unmarshal(): cnt != *dwSize\r\n";
        L.timestamp();
        L.append(lpsz);
        console.write(lpsz);
    }

    return TRUE;
}

void CMessageRemoveAreaAirEffectSpecific::DoMessage(void) {

    IECString s = this->rAreaName.GetResRefStr();
    CArea& area = g_pChitin->pGame->GetLoadedArea(s);
    if (&area != NULL) {
        CArea_RemoveAreaAirEffectSpecific(area, this->rResource);
    }

    return;
}


void __stdcall
CMessageHandler_AsynchronousUpdate_Log(IECPtrList &list) {
    CMessage*    pEff = NULL;
    //int a;

    if (list.GetCount() != 0 &&
        list.GetCount() != 1) {
        console.write_debug("Asyn msgs=%d \n", list.GetCount());
        POSITION pos = list.GetHeadPosition();
        while (pos) {
            pEff = (CMessage *) list.GetNext(pos);
            if (*(int*)pEff == 0xAA6A40) {
                CMessageApplyEffect *msg2 = (CMessageApplyEffect *) pEff;
                console.write_debug(" node AddEffect ");

                if (GetVT(msg2->pCEffect) == 0xAA7F98) console.write_debug("CastSpell \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa8740) console.write_debug("Regeneration \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa8060) console.write_debug("DisplayPortraitIcon \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa82e0) console.write_debug("MovementBonus \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa9618) console.write_debug("SetColor \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa94b0) console.write_debug("EffectHeal \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa9730) console.write_debug("AC mod \n"); else
                if (GetVT(msg2->pCEffect) == 0xaaa038) console.write_debug("ActivateSequencer \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa7c78) console.write_debug("RemoveSpell \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa80b0) console.write_debug("CastingGlow \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa8f88) console.write_debug("SetColorPulse \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa9230) console.write_debug("SaveVsDeath \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa8c90) console.write_debug("Web \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa8588) console.write_debug("Paralyze \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa80d8) console.write_debug("String \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa7610) console.write_debug("PlayVisualEffect \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa7778) console.write_debug("ProtectionFromSpell \n"); else
                if (GetVT(msg2->pCEffect) == 0xaa7c28) console.write_debug("PlaySound \n"); else
                    console.write_debug("%X \n", GetVT(msg2->pCEffect));
            }
            else
            if (GetVT(pEff) == 0xAA6CE0) console.write_debug(" node SetVariable \n"); else
            if (GetVT(pEff) == 0xaa5c84) ; else //console.write_debug(" msg SetTrigger \n"); else
            if (GetVT(pEff) == 0xaa6cfc) console.write_debug(" node aa6cfc \n"); else
            if (GetVT(pEff) == 0xaa743c) ; else //console.write_debug(" msg FaceTalker \n"); else
            if (GetVT(pEff) == 0xaa6da4) console.write_debug(" node SetDirection \n"); else
            if (GetVT(pEff) == 0xaa6d6c) console.write_debug(" node SetAnimationSequence \n"); else
            if (GetVT(pEff) == 0xaa9774) console.write_debug(" node DisplayText \n"); else
            if (GetVT(pEff) == 0xaab8c0) console.write_debug(" node FireProjectile \n"); else
            if (GetVT(pEff) == 0xaa9790) console.write_debug(" node VisualEffect \n"); else
                console.write_debug("node %X \n", GetVT(pEff));
        }
    }
}


void __stdcall
CCreativeObject_AddEffect_Log(CEffect *eff, int mode) {
    //int  a;
    char *str;

    if (mode == 1) 
        str="Cre addeff";
    else
    if (mode == 2) 
        str="Proj addeff";
    else
    if (mode == 3) 
        str="Msg addmsg";

    if (GetVT(eff) == 0xAA7F98) console.write_debug("%s CastSpell \n", str); else
    if (GetVT(eff) == 0xaa8740) console.write_debug("%s Regeneration \n", str); else
    if (GetVT(eff) == 0xaa8060) console.write_debug("%s DisplayPortraitIcon \n", str); else
    if (GetVT(eff) == 0xaa82e0) console.write_debug("%s MovementBonus \n", str); else
    if (GetVT(eff) == 0xaa9618) console.write_debug("%s SetColor \n", str); else
    if (GetVT(eff) == 0xaa94b0) console.write_debug("%s EffectHeal \n", str); else
    if (GetVT(eff) == 0xaa9730) console.write_debug("%s AC mod \n", str); else
    if (GetVT(eff) == 0xaaa038) console.write_debug("%s ActivateSequencer \n", str); else
    if (GetVT(eff) == 0xaa7c78) console.write_debug("%s RemoveSpell \n", str); else
    if (GetVT(eff) == 0xaa80b0) console.write_debug("%s CastingGlow \n", str); else
    if (GetVT(eff) == 0xaa8f88) console.write_debug("%s SetColorPulse \n", str); else
    if (GetVT(eff) == 0xaa9230) console.write_debug("%s SaveVsDeath \n", str); else
    if (GetVT(eff) == 0xaa8c90) console.write_debug("%s Web \n", str); else
    if (GetVT(eff) == 0xaa8588) console.write_debug("%s Paralyze \n", str); else
    if (GetVT(eff) == 0xaa80d8) console.write_debug("%s String \n", str); else
    if (GetVT(eff) == 0xaa7610) console.write_debug("%s PlayVisualEffect \n", str); else
    if (GetVT(eff) == 0xaa7778) console.write_debug("%s ProtectionFromSpell \n", str); else
    if (GetVT(eff) == 0xaa7c28) console.write_debug("%s PlaySound \n", str); else
    // msg
    if (GetVT(eff) == 0xAA5C84) ; else //console.write_debug("%s SetTrigger \n", str); else
    if (GetVT(eff) == 0xAAB758) console.write_debug("%s ResetAnimColors \n", str); else
    if (GetVT(eff) == 0xAAB914) ; else //console.write_debug("%s SpriteUpdate \n", str); else
    if (GetVT(eff) == 0xAA743C) ; else //console.write_debug("%s FaceTalker \n", str); else
    if (GetVT(eff) == 0xAA6A40) {
        CMessageApplyEffect *msg2 = (CMessageApplyEffect *) eff;
        console.write_debug("%s ApplyEffect ", str);
        if (GetVT(msg2->pCEffect) == 0xAA7F98) console.write_debug("CastSpell \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa8740) console.write_debug("Regeneration \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa8060) console.write_debug("DisplayPortraitIcon \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa82e0) console.write_debug("MovementBonus \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa9618) console.write_debug("SetColor \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa94b0) console.write_debug("EffectHeal \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa9730) console.write_debug("AC mod \n"); else
        if (GetVT(msg2->pCEffect) == 0xaaa038) console.write_debug("ActivateSequencer \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa7c78) console.write_debug("RemoveSpell \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa80b0) console.write_debug("CastingGlow \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa8f88) console.write_debug("SetColorPulse \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa9230) console.write_debug("SaveVsDeath \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa8c90) {
            console.write_debug("Web \n");
        } else
        if (GetVT(msg2->pCEffect) == 0xaa8588) console.write_debug("Paralyze \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa80d8) console.write_debug("String \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa7610) console.write_debug("PlayVisualEffect \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa7778) console.write_debug("ProtectionFromSpell \n"); else
        if (GetVT(msg2->pCEffect) == 0xaa7c28) console.write_debug("PlaySound \n"); else
            console.write_debug("%X \n", GetVT(msg2->pCEffect));
    }
    else
    if (GetVT(eff) == 0xAA63CC) console.write_debug("%s DisplayDialogue \n", str); else
    if (GetVT(eff) == 0xAAB8C0) console.write_debug("%s CreateProjectile \n", str); else
    if (GetVT(eff) == 0xAAB838) console.write_debug("%s AddVVCInternal \n", str); else
    if (GetVT(eff) == 0xAA9774) console.write_debug("%s DisplayText \n", str); else
    if (GetVT(eff) == 0xAA6E68) console.write_debug("%s PlaySound \n", str); else
    if (GetVT(eff) == 0xAA6DA4) console.write_debug("%s OrientToPoint \n", str); else
    if (GetVT(eff) == 0xAA6D6C) console.write_debug("%s SetAnimationSequence \n", str); else
    if (GetVT(eff) == 0xAA5C68) console.write_debug("%s StopActions \n", str); else
    if (GetVT(eff) == 0xAA6AE8) ; else //console.write_debug("%s UpdateScript \n", str); else
    if (GetVT(eff) == 0xAA9790) console.write_debug("%s VisualEffect \n", str); else
    if (GetVT(eff) == 0xAA7134) console.write_debug("%s StartCombatMusic \n", str); else
        console.write_debug("%s %X \n", str, GetVT(eff));
}


void __stdcall
CProjectileBAM_AIUpdate_Log(CProjectile &pro) {
    CEffect *eff;
    char *str="- ";

    console.write_debug("proj x=%d y=%d id =%X speed=%d ", pro.currentLoc.x, pro.currentLoc.y, pro.id, pro.nSpeed);
    if (GetVT(&pro) == 0xAABF70) console.write_debug("ProjectileArea"); else
    if (GetVT(&pro) == 0xAABEDC) console.write_debug("ProjectileBAM"); else
    if (GetVT(&pro) == 0xAAC2F0) console.write_debug("ProjectileInvisible"); else
        console.write_debug("%s", GetVT(&pro));

    if ( ((pro.posDest.y - pro.currentLoc.y) * (pro.posDest.y - pro.currentLoc.y) * 16 / 9
          + (pro.posDest.x - pro.currentLoc.x) * (pro.posDest.x - pro.currentLoc.x))
                 <= (pro.nSpeed + 1) * (pro.nSpeed + 1)
       ) {
        console.write_debug(" Arrival \n");
        if (pro.effects.GetCount() != 0 &&
            pro.effects.GetCount() != 1) {
            console.write_debug("effects=%d \n", pro.effects.GetCount());
            POSITION pos = pro.effects.GetHeadPosition();
            while (pos) {
                eff = (CEffect *) pro.effects.GetNext(pos);
                if (GetVT(eff) == 0xAA7F98) console.write_debug("%s CastSpell \n", str); else
                if (GetVT(eff) == 0xaa8740) console.write_debug("%s Regeneration \n", str); else
                if (GetVT(eff) == 0xaa8060) console.write_debug("%s DisplayPortraitIcon \n", str); else
                if (GetVT(eff) == 0xaa82e0) console.write_debug("%s MovementBonus \n", str); else
                if (GetVT(eff) == 0xaa9618) console.write_debug("%s SetColor \n", str); else
                if (GetVT(eff) == 0xaa94b0) console.write_debug("%s EffectHeal \n", str); else
                if (GetVT(eff) == 0xaa9730) console.write_debug("%s AC mod \n", str); else
                if (GetVT(eff) == 0xaaa038) console.write_debug("%s ActivateSequencer \n", str); else
                if (GetVT(eff) == 0xaa7c78) console.write_debug("%s RemoveSpell \n", str); else
                if (GetVT(eff) == 0xaa80b0) console.write_debug("%s CastingGlow \n", str); else
                if (GetVT(eff) == 0xaa8f88) console.write_debug("%s SetColorPulse \n", str); else
                if (GetVT(eff) == 0xaa9230) console.write_debug("%s SaveVsDeath \n", str); else
                if (GetVT(eff) == 0xaa8c90) console.write_debug("%s Web \n", str); else
                if (GetVT(eff) == 0xaa8588) console.write_debug("%s Paralyze \n", str); else
                if (GetVT(eff) == 0xaa80d8) console.write_debug("%s String \n", str); else
                if (GetVT(eff) == 0xaa7610) console.write_debug("%s PlayVisualEffect \n", str); else
                if (GetVT(eff) == 0xaa7778) console.write_debug("%s ProtectionFromSpell \n", str); else
                if (GetVT(eff) == 0xaa7c28) console.write_debug("%s PlaySound \n", str); else
                    console.write_debug("%s %X \n", str, GetVT(eff));
            }
        }
    }

    console.write_debug("\n");
}


void __stdcall
CProjectileArea_AIUpdate_Log1(CProjectile &pro) {
    console.write_debug("AIUpdate ");
    if (GetVT(&pro) == 0xAABF70) console.write_debug("ProjectileArea %X", pro.id); else
    if (GetVT(&pro) == 0xAABEDC) console.write_debug("ProjectileBAM %X", pro.id); else
    if (GetVT(&pro) == 0xAAC2F0) console.write_debug("ProjectileInvisible %X", pro.id); else
        console.write_debug("%s", GetVT(&pro));

    console.write_debug("\n");
}

void __stdcall
CProjectileArea_AIUpdate_Log2(CProjectile &pro) {
    console.write_debug("AreaEffect ");
    if (GetVT(&pro) == 0xAABF70) console.write_debug("ProjectileArea %X", pro.id); else
    if (GetVT(&pro) == 0xAABEDC) console.write_debug("ProjectileBAM %X", pro.id); else
    if (GetVT(&pro) == 0xAAC2F0) console.write_debug("ProjectileInvisible %X", pro.id); else
        console.write_debug("%s", GetVT(&pro));

    console.write_debug("\n");
}

void  __declspec(naked)
CMessageHandler_AsynchronousUpdate_Log_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-0C38h] // IECPtrList
    call    CMessageHandler_AsynchronousUpdate_Log

    pop     edx
    pop     ecx
    pop     eax

    mov     dword ptr [ebp-1Ch], 0    // stolen bytes
    ret
}
}


void  __declspec(naked)
CCreativeObject_AddEffect_Log_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    1
    push    [ebp+8] // CEffect
    call    CCreativeObject_AddEffect_Log

    pop     edx
    pop     ecx
    pop     eax

    add     edx, 6288h    // stolen bytes
    ret
}
}

void  __declspec(naked)
CGameAIBase_FireSpell_ProjectileAddEffect_Log_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    2
    push    [ebp-78h] // CEffect
    call    CCreativeObject_AddEffect_Log //CGameAIBase_FireSpell_ProjectileAddEffect

    pop     edx
    pop     ecx
    pop     eax

    mov     ecx, [ebp-0B0h] // stolen bytes
    ret
}
}


void  __declspec(naked)
CMessageHandler_AddMessage_Log_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    3
    push    eax         // CEffect
    call    CCreativeObject_AddEffect_Log //CGameAIBase_FireSpell_ProjectileAddEffect

    pop     edx
    pop     ecx
    pop     eax

    mov     edx, [eax]  // stolen bytes
    mov     ecx, [ebp+8]
    ret
}
}

void  __declspec(naked)
CProjectileBAM_AIUpdate_Log_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-110h]  // CProjectile
    call    CProjectileBAM_AIUpdate_Log

    pop     edx
    pop     ecx
    pop     eax

    mov     eax, [ebp-110h]  // stolen bytes
    ret
}
}


void  __declspec(naked)
CProjectileArea_AIUpdate_Log1_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-4Ch]  // CProjectile
    call    CProjectileArea_AIUpdate_Log1

    pop     edx
    pop     ecx
    pop     eax

    cmp     edx, 0D6h  // stolen bytes
    ret
}
}


void  __declspec(naked)
CProjectileArea_AIUpdate_Log2_asm() {
__asm {
    push    eax
    push    ecx
    push    edx

    push    [ebp-4Ch]  // CProjectile
    call    CProjectileArea_AIUpdate_Log2

    pop     edx
    pop     ecx
    pop     eax

    mov     cx, [eax+2C6h]  // stolen bytes
    ret
}
}