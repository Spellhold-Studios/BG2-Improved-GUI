#include "AreaCommon.h"

#include <cassert>

#include "chitin.h"
#include "infgame.h"
#include "options.h"
#include "InfGameCommon.h"

#define GAMEOBJ     g_pChitin->pGame->m_GameObjectArrayHandler

void CArea_RemoveAreaAirEffectSpecific(CArea& area, ResRef& rResource) {
	if (0) IECString("CArea_RemoveAreaAirEffectSpecific");

	bool bUseResource = true;
	CRuleTable crtResource;
	crtResource.LoadTable(rResource);
	if (crtResource.m_2da.bLoaded == FALSE) {
		LPCTSTR lpsz = "CArea_RemoveAreaAirEffectSpecific(): %s.2da not found, using ClearAir.2da...\r\n";
		L.timestamp();
		L.appendf(lpsz, rResource.GetResRefNulled());
		console.writef(lpsz, rResource.GetResRefNulled());
		bUseResource = false;
	}

	POSITION pos = area.m_lVertSortFront.GetHeadPosition();
	while (pos != NULL) {
		Enum e = (Enum)area.m_lVertSortFront.GetNext(pos);
		CGameObject* pObj;
		if (g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pObj, -1) == OBJECT_SUCCESS) {
			if (pObj->GetObjectType() == CGAMEOBJECT_TYPE_PROJECTILE) {
				IECString sMissileId;
				sMissileId.Format("%d", ((CProjectile*)pObj)->nMissileId);

				POINT posString;
				bool bFound = bUseResource
					? crtResource.FindString(sMissileId, &posString, FALSE) 
					: g_pChitin->pGame->CLEARAIR.FindString(sMissileId, &posString, FALSE);
				if (bFound) {
					char nResult;
					do {
						nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectDeny(e, THREAD_ASYNCH, &pObj, INFINITE);
					} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

					if (nResult == OBJECT_SUCCESS) {
						pObj->RemoveFromArea();
						g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectDeny(e, THREAD_ASYNCH, INFINITE);
					}
				}
			} 
			else if (pObj->GetObjectType() == CGAMEOBJECT_TYPE_SMOKE) {
				bool bRemoveSmoke = false;
				AnimData& animData = ((CSmokeObject*)pObj)->m_animation;
				assert(animData.pAnimation != NULL);
				
				short wAnimId = animData.pAnimation->wAnimId;
				
				if (bUseResource) {
					char szSmokeId[7] = {0};
					sprintf_s(szSmokeId, "0x%.4X", wAnimId & 0xFF0F);
					IECString sSmokeId(szSmokeId);
					POINT posString;
					bRemoveSmoke = crtResource.FindString(sSmokeId, &posString, FALSE);
				}
				else if ((wAnimId & 0xFF0F) == 0x500) { //STINKCLOUD_*
					bRemoveSmoke = true;
				}

				if (bRemoveSmoke) {
					char nResult;
					do {
						nResult = g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectDeny(e, THREAD_ASYNCH, &pObj, INFINITE);
					} while (nResult == OBJECT_SHARING || nResult == OBJECT_DENYING);

					if (nResult == OBJECT_SUCCESS) {
						pObj->RemoveFromArea();
						g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectDeny(e, THREAD_ASYNCH, INFINITE);
					}
				}
			}
			g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
		}
	}
	return;
}


typedef struct _ListNode {
	struct _ListNode* pNext;
	struct _ListNode* pPrev;
	DWORD     EnumID;
} ListNode;

inline CGameObject*
GetObjectByID_Fast(DWORD encodedID) {
    short Index = (encodedID >> 16) & 0x7FFF;
    short ID    = encodedID & 0x7FFF;

    CGameObjectArrayHandler& ArrayHandler = g_pChitin->pGame->m_GameObjectArrayHandler;
    if (Index <= ArrayHandler.nMaxArrayIndex) { 
        if (ArrayHandler.pArray[Index].id == ID)
            return ArrayHandler.pArray[Index].pGameObject;
        else {
            // locked/unavailable/mismatch
            return NULL;
        }
        
    } else {
        // out of index
        return NULL;
    }
};


static ListNode* __stdcall
GetPrevNode(CArea& Area, ListNode* TryNode) {
    static int phase;
    ListNode* TryNodeOrig = TryNode;

    if (TryNode == (ListNode*) Area.m_lVertSortFront.GetTailPosition()) {
        phase = 1; // start new cycle
    }

    if (phase == 1) { // skip anything but gui icons
        while (TryNode) {
            CGameObject* Obj = GetObjectByID_Fast(TryNode->EnumID);
            if (Obj == NULL ||  // failed object , return as is
                Obj->nObjType == CGAMEOBJECT_TYPE_ICONGUI) {
                    return TryNode;
            }

            TryNode = TryNode->pPrev;
        }

        // no more icons, switch&restart to creature types
        TryNode = (ListNode*) Area.m_lVertSortFront.GetTailPosition();
        phase = 2;

    }

    if (phase == 2) { // skip anything but creature
        while (TryNode) {
            CGameObject* Obj = GetObjectByID_Fast(TryNode->EnumID);
            if (Obj == NULL ||  // failed object , return as is
                Obj->nObjType == CGAMEOBJECT_TYPE_CREATURE) {
                    return TryNode;
            }

            TryNode = TryNode->pPrev;
        }

        // no more creatures, switch&restart to remain types
        TryNode = (ListNode*) Area.m_lVertSortFront.GetTailPosition();
        phase = 3;

    }

    if (phase == 3) { // skip icons&creatures
        while (TryNode) {
            CGameObject* Obj = GetObjectByID_Fast(TryNode->EnumID);
            if (Obj == NULL ||
                (Obj->nObjType != CGAMEOBJECT_TYPE_CREATURE &&
                 Obj->nObjType != CGAMEOBJECT_TYPE_ICONGUI)) {
                    return TryNode;
            }

            TryNode = TryNode->pPrev;
        }

        // end of list
    }

    // end of list, return as is
    return TryNode;
}


//static void __stdcall
//Area_DumpEnum(DWORD e) {
//    static int max = 72;
//    CGameObject* pObj;
//
//    if (max > 0) {
//        g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(e, THREAD_ASYNCH, &pObj, INFINITE);
//        unsigned char objtype = pObj->nObjType;
//        g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(e, THREAD_ASYNCH, INFINITE);
//
//        if (objtype == CGAMEOBJECT_TYPE_CREATURE) {
//            console.writef("%0x Cre %d \n", e, objtype);
//        } else {
//            console.writef("%0x %d \n", e, objtype);
//        }
//        max--;
//    }
//}


BOOL static __stdcall
IsBattleSongPlaying(CArea& Area) {
    if (Area.nBattleSongCounter > 0 &&
        Area.wCurrentSongType == 3) // battle type
        return TRUE;
    else
        return FALSE;
}


BOOL static __stdcall
IsCombatActiveOnCreatureArea(CCreatureObject& Cre) {
    CArea& Area = * Cre.pArea;

    if (pRuleEx->Area_UNBLAREA.m_2da.bLoaded) {
        POINT  pos;
        IECString restring = Area.areaPrefix.GetResRefStr();
        if (pRuleEx->Area_UNBLAREA.FindString(restring, &pos, FALSE))
            return FALSE;
    }
    

    if (Area.nBattleSongCounter > 0 ||
        Area.nDamageCounter > 0) {
        IECString sText = GetTlkString(6559); // ~You can't leave or change area in combat~
        g_pChitin->pScreenWorld->PrintToConsole(IECString(), sText, 0, 0xCFF3F, -1, 0);

        return TRUE;
    }
    else {
        POSITION         pos;
        Enum             ObjID;
        CCreatureObject* pCre;

        pos = Area.m_lVertSortFront.GetHeadPosition();
        while (pos != NULL) {
            ObjID = (Enum) Area.m_lVertSortFront.GetNext(pos);
            if (GAMEOBJ.GetGameObjectShare(ObjID, THREAD_ASYNCH, &pCre, -1) != OBJECT_SUCCESS)
                {continue;}

            // rip from CCreativeObject::CanSaveGame()
            if (pCre->nObjType == CGAMEOBJECT_TYPE_CREATURE) { 
                if (pCre->o.EnemyAlly >= EA_EVILCUTOFF &&
                    pCre->canBeSeen > 0 &&
                    pCre->Animate() &&
                    (g_pChitin->pGame->GetPartyMemberSlot(pCre->id) == -1) ) {
                        GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
                        IECString sText = GetTlkString(6559); // ~You can't leave or change area in combat~
                        g_pChitin->pScreenWorld->PrintToConsole(IECString(), sText, 0, 0xCFF3F, -1, 0);

                        return TRUE;
                }
            }

             GAMEOBJ.FreeGameObjectShare(ObjID, THREAD_ASYNCH, INFINITE);
            }

        return FALSE;
    }
}


void static __stdcall
CMarker_RenderSprite(
        CMarker& Marker,
        CVideoMode& VidMode,
        uint ArgX,
        RECT* FootCircle,
        CCreatureObject& Cre ) {
    CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;
    if (Cre.statistics.bInParty                &&
        Cre.bMoraleBroken == 0                 &&   // not in panic
        Cre.BaseStats.morale > cds.moraleBreak &&   // not in panic
        (cds.stateFlags & STATE_PANIC) == 0    &&   // not in panic
        (cds.stateFlags & STATE_CHARMED) == 0) {    // not charmed
        //Marker.Render(VidMode, ArgX, (DWORD *)&Cre.pArea->m_cInfinity, &Cre.currentLoc, FootCircle->right, FootCircle->bottom);

        ABGR oldrgb = Marker.rgbCircle;
        Marker.rgbCircle = *(g_pColorRangeArray + Cre.BaseStats.colors.colorMajor);

        if (pGameOptionsEx->bUI_PartyColorCirclesExcludeNPCColors) {
            // exclude neutral/enemy/panic colors
            if (Marker.rgbCircle == 0xffff40 ||   // Neutral color
                Marker.rgbCircle == 0xfcdd85 )    // Blue1 clothes
                    Marker.rgbCircle = 0x00fa00;  // Party Green

            if (Marker.rgbCircle == 0x2000ff ||   // Enemy color
                Marker.rgbCircle == 0x6174f5 ||   // Red1 clothes
                Marker.rgbCircle == 0x3345d9 )    // Red2 clothes
                    Marker.rgbCircle = 0x00fa00;  // Party Green

            if (Marker.rgbCircle == 0x00FFFF)       // Yellow panic
                    Marker.rgbCircle = 0x00fa00;    // Party Green
        }

        Marker.Render(VidMode, ArgX, (DWORD *)&Cre.pArea->m_cInfinity, &Cre.currentLoc, FootCircle->right, FootCircle->bottom);
        // Bold
        //Marker.Render(VidMode, ArgX, (DWORD *)&Cre.pArea->m_cInfinity, &Cre.currentLoc, FootCircle->right + 2, FootCircle->bottom + 2);
        //Marker.Render(VidMode, ArgX, (DWORD *)&Cre.pArea->m_cInfinity, &Cre.currentLoc, FootCircle->right + 3, FootCircle->bottom + 3);

        Marker.rgbCircle = oldrgb;
    }
    else
        Marker.Render(VidMode, ArgX, (DWORD *)&Cre.pArea->m_cInfinity, &Cre.currentLoc, FootCircle->right, FootCircle->bottom);
}


 void __declspec(naked)
Area_GetFirstNode_asm() {
__asm
{
    push    ecx
    push    edx

    push    edx         ; TryNode
    push    [ebp-3CCh]  ; Area
    call    GetPrevNode

    pop     edx
    pop     ecx

    ; eax = NextNode
    ret
}
}


 void __declspec(naked)
Area_GetPrevNode_asm() {
__asm
{
    push    ecx
    push    edx

    ; eax CurrentNode
    mov     eax, [eax+4]; prevNode
    push    eax         ; TryNode
    push    [ebp-3CCh]  ; Area
    call    GetPrevNode

    pop     edx
    pop     ecx

    ; eax = NextNode
    mov     [ebp-54h], eax  ; Stolen Bytes
    ret
}
}


// void __declspec(naked)
//Area_DumpEnum_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//    push    eax
//
//    mov     eax, [edx+8]
//    push    eax             ; Enum
//    call    Area_DumpEnum
//
//    pop     eax
//    pop     edx
//    pop     ecx
//
//    mov     eax, [edx+8]
//    mov     [ebp-24h], eax  ; Stolen Bytes
//    ret
//}
//}


void __declspec(naked)
CGameArea_OnActivation_SetSongTypeDay_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-5Ch]   ; Area
    call    IsBattleSongPlaying

    pop     edx
    pop     ecx

    test    eax,eax
    pop     eax
    jnz     L_KeepBattleSong1

    mov     word ptr [edx+0A1Ch], 0  ; Stolen Bytes

L_KeepBattleSong1:
    ret
}
}


void __declspec(naked)
CGameArea_OnActivation_SetSongTypeNight_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-5Ch]   ; Area
    call    IsBattleSongPlaying

    pop     edx
    pop     ecx

    test    eax,eax
    pop     eax
    jnz     L_KeepBattleSong2

    mov     word ptr [eax+0A1Ch], 1  ; Stolen Bytes
    ret

L_KeepBattleSong2:
    ret
}
}



void __declspec(naked)
CMarker_RenderSprite_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp+10h]   // Cre
    push    [ebp-8]     // FootCircle
    push    [ebp+0Ch]   // ArgX
    push    [ebp+8]     // VidMode
    push    [ebp-1Ch]   // Marker
    call    CMarker_RenderSprite

    pop     edx
    pop     ecx
    ret     18h
}
}


void __declspec(naked)
CGameSprite_LeaveArea_CheckBattleSong_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-1E0h]  // Cre
    call    IsCombatActiveOnCreatureArea

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      CGameSprite_LeaveArea_CheckBattleSong_asm_Continue

    mov     cx, -2          // ACTION_ERROR
    mov     [ebp-130h], cx

    add     esp, 4
    push    092BB92h        // Abort
    ret

CGameSprite_LeaveArea_CheckBattleSong_asm_Continue:
    mov     word ptr [ebp-2Ch], 0   // Stolen Bytes
    ret
}
}


void __declspec(naked)
CGameSprite_LeaveAreaName_CheckBattleSong_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp-66Ch]  // Cre
    call    IsCombatActiveOnCreatureArea

    test    eax,eax
    pop     edx
    pop     ecx
    pop     eax
    jz      CGameSprite_LeaveAreaName_CheckBattleSong_asm_Continue

    // release shared trigger
    push    0FFFFFFFFh      ; WaitMode
    mov     eax, 0AAD1CBh
    mov     dl, [eax]
    push    edx             ; ThreadMode
    mov     eax, [ebp-58h]
    push    eax             ; ID
    mov     ecx, [ebp-20h]
    add     ecx, 37F6h      ; CGameObjectArrayHandler
    call    CGameObjectArrayHandler::FreeGameObjectShare
    mov     byte ptr [ebp-1Ch], al
    mov     cx, -2          // ACTION_ERROR
    mov     [ebp-23Ch], cx

    add     esp, 4
    push    092CAC9h        // Abort
    ret

CGameSprite_LeaveAreaName_CheckBattleSong_asm_Continue:
    mov     word ptr [ebp-3Ch], 0   // Stolen Bytes
    ret
}
}