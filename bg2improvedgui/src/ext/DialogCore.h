#ifndef DIALOGCORE_H
#define DIALOGCORE_H

#include "dlgcore.h"
#include "objcre.h"

void __stdcall CDlgResponse_QueueActions(CDlgResponse& cdResponse, CCreatureObject& cre);

void CGameSprite_PlayerDialog_InsertGreeting1_asm();
void CGameSprite_PlayerDialog_InsertGreeting2_asm();
void CGameSprite_PlayerDialog_ToggleGreeting1_asm();
void CGameSprite_PlayerDialog_ToggleGreeting2_asm();
void GameSprite_PlayerDialog_DisplaySayNothing_asm();
void CCreatureObject_PlaySound_CheckJoinable_asm();

#endif //DIALOGCORE_H