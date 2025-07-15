#ifndef MESSAGEEXT_H
#define MESSAGEEXT_H

#include "msgcore.h"

class CMessageRemoveAreaAirEffectSpecific : public CMessageRemoveAreaAirEffects { //Size 1Ch
public:
	virtual void Marshal(void* pData, int* dwSize); //v10
	virtual BOOL Unmarshal(void* pData, int* dwSize); //v14
	virtual void DoMessage(void); //v18

	ResRef rResource; //14h
};

struct CMessageRemoveAreaAirEffectSpecificM {
	ResRef rAreaName; //0h
	ResRef rResource; //8h
};


void CMessageHandler_AsynchronousUpdate_Log_asm();
void CCreativeObject_AddEffect_Log_asm();
void CGameAIBase_FireSpell_ProjectileAddEffect_Log_asm();
void __stdcall CMessageHandler_AsynchronousUpdate_Log(IECPtrList &list);
void CMessageHandler_AddMessage_Log_asm();
void CProjectileBAM_AIUpdate_Log_asm();
void CProjectileArea_AIUpdate_Log1_asm();
void CProjectileArea_AIUpdate_Log2_asm();
void CProjectileBAM_CProjectileBAM_PatchSpeed_asm();



#endif //MESSAGEEXT_H