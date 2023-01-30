#include "AnimationCore.h"

#include "chitin.h"
#include "objcre.h"
#include "infgame.h"
#include "console.h"
#include "log.h"
#include "InfGameCommon.h"
#include "Animation5000.h"

#define OR ||

void (CAnimation::*Tramp_CAnimation_PlayCurrentSequenceSound)(CCreatureObject&) =
	SetFP(static_cast<void (CAnimation::*)(CCreatureObject&)>	(&CAnimation::PlayCurrentSequenceSound),	0x7F9DF4);
BOOL (*Tramp_CAnimation_IsAnimation5000)(unsigned short) =
	reinterpret_cast<BOOL (*)(unsigned short)>					(                                           0x85F364);
LPCTSTR (CAnimation::*Tramp_CAnimation_GetWalkingSound)(short) =
	SetFP(static_cast<LPCTSTR (CAnimation::*)(short)>			(&CAnimation::GetWalkingSound),				0x87B7B0);


bool
CreHasAttackSound(CCreatureObject& Cre, short wCurrentSequence) {
    CStrRef ref;

    switch (wCurrentSequence) {
    case SEQ_ATTACK_SLASH:
        g_pChitin->m_TlkTbl.GetTlkString(Cre.BaseStats.soundset[14], ref);  // ATTACK1
        if (!ref.sound.wav.soundName.IsEmpty())
            return true;
        break;
    case SEQ_ATTACK_BACKSLASH:
        g_pChitin->m_TlkTbl.GetTlkString(Cre.BaseStats.soundset[15], ref);  // ATTACK2
        if (!ref.sound.wav.soundName.IsEmpty())
            return true;
        break;
    case SEQ_ATTACK_JAB:
        g_pChitin->m_TlkTbl.GetTlkString(Cre.BaseStats.soundset[16], ref);  // ATTACK3
        if (!ref.sound.wav.soundName.IsEmpty())
            return true;
        break;
    case SEQ_SHOOT:
        g_pChitin->m_TlkTbl.GetTlkString(Cre.BaseStats.soundset[17], ref);  // ATTACK4
        if (!ref.sound.wav.soundName.IsEmpty())
            return true;
        break;
    default:
        break;
    }

    return false;
}



static const char*
gGetSeqText[] = 
{   "SEQ_ATTACK",
    "SEQ_AWAKE",
    "SEQ_CAST",
    "SEQ_CONJURE",
    "SEQ_DAMAGE",
    "SEQ_DIE",
    "SEQ_HEADTRN",
    "SEQ_READY",
    "SEQ_SHOOT",
    "SEQ_TWITCH",
    "SEQ_WALK",
    "SEQ_ATTACK_SLASH",
    "SEQ_ATTACK_BACKSLASH",
    "SEQ_ATTACK_JAB",
    "SEQ_EMERGE",
    "SEQ_HIDE",
    "SEQ_SLEEP" };


static const char*
GetSeqText(short wSequence) {
    return gGetSeqText[wSequence];
}


ResRef gSoundFileName;
bool   gTobexSoundCaller;
double gNormalizeAmplLastValue;
int    gSndChannel;

void DETOUR_CAnimation::DETOUR_PlayCurrentSequenceSound(CCreatureObject& cre) {
    // vanilla just play
    // CSequenceSoundList::PlaySound(&anim->sequencesoundset[Cre->wAnimSequenceSimplified], wFrame, Cre);

	short wCycle, wFrame;
	this->GetCurrentSequenceAndFrame(wCycle, wFrame);
	short wCurrentSequence = cre.animation.wCurrentSequence;
    gTobexSoundCaller = true;
    gNormalizeAmplLastValue = 0;
    gSoundFileName = "";

    if (wCurrentSequence == SEQ_HEAD_TURN ||
        wCurrentSequence == SEQ_READY ||
        wCurrentSequence == SEQ_WALK )
        if (cre.u6529 == 0x55) {    // SELECT_ACTION sound playing
            uchar channel = cre.GetCharacterPortraitNumChannel();
            if (g_pChitin->m_mixer.IsChannelUsed(channel)) {
                gTobexSoundCaller = false;
	            return;             // skip new sounds until char channel be free
            } else {
                cre.u6529 = 0x00;   // SELECT_ACTION playing ended
            }
        }

    bool bHasAttack2DASound = false;
	switch (wCurrentSequence) {
		case SEQ_ATTACK_SLASH:
            if (sequencesoundset[SEQ_ATTACK_SLASH].GetCount() > 0)
                bHasAttack2DASound = true;
            break;
		case SEQ_ATTACK_BACKSLASH:
            if (sequencesoundset[SEQ_ATTACK_BACKSLASH].GetCount() > 0)
                bHasAttack2DASound = true;
			break;
		case SEQ_ATTACK_JAB:
            if (sequencesoundset[SEQ_ATTACK_JAB].GetCount() > 0)
                bHasAttack2DASound = true;
			break;
		default:
            break;
    }

    if (pGameOptionsEx->bSound_DisableGenericAttackSound == false ||    // generic ATTACK sound enabled
        bHasAttack2DASound == false) {                                  // no 2DA' specific sound
        if (cre.wAnimSequenceSimplified == SEQ_ATTACK ) {               // melee attack
            if (!CreHasAttackSound(cre, wCurrentSequence)) {            // no CRE' specific sound
                (this->*Tramp_CAnimation_PlayCurrentSequenceSound)(cre);// sequencesoundset[Cre->wAnimSequenceSimplified].PlaySound(wFrame, &cre)

                if (!gSoundFileName.IsEmpty()) {
                    switch (wCurrentSequence) {
		            case SEQ_ATTACK_SLASH:
                        if (bHasAttack2DASound == false)
                            console.write_debug("2DA.SEQ_ATTACK_SLASH->Generic \tsound: %s", gSoundFileName.GetResRefNulled());
                        else
                            console.write_debug("2DA.Generic attack \t\tsound: %s", gSoundFileName.GetResRefNulled());
                        break;
		            case SEQ_ATTACK_BACKSLASH:
                        if (bHasAttack2DASound == false)
                            console.write_debug("2DA.SEQ_ATTACK_BACKSLASH->Generic \tsound: %s", gSoundFileName.GetResRefNulled());
                        else
                            console.write_debug("2DA.Generic attack \t\tsound: %s", gSoundFileName.GetResRefNulled());
			            break;
		            case SEQ_ATTACK_JAB:
                        if (bHasAttack2DASound == false)
                            console.write_debug("2DA.SEQ_ATTACK_JAB->Generic \tsound: %s", gSoundFileName.GetResRefNulled());
                        else
                            console.write_debug("2DA.Generic attack \t\tsound: %s", gSoundFileName.GetResRefNulled());
			            break;
		            default:
                        console.write_debug("2DA.Generic attack \t\tsound: %s", gSoundFileName.GetResRefNulled());
                        break;
                    }

                    console.write_debug("\t ch:%d", gSndChannel);
                    if (gNormalizeAmplLastValue)
                        console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                    else
                        console.write_debug("\n");
                }
            }
        } else {    // not melee attack
            (this->*Tramp_CAnimation_PlayCurrentSequenceSound)(cre); // sequencesoundset[Cre->wAnimSequenceSimplified].PlaySound(wFrame, &cre)

            if (!gSoundFileName.IsEmpty()) {
                console.write_debug("2DA.%s\t\t\tsound: %s", GetSeqText(wCurrentSequence), gSoundFileName.GetResRefNulled());
                console.write_debug("\t ch:%d", gSndChannel);
                if (gNormalizeAmplLastValue)
                    console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                else
                    console.write_debug("\n");

            }
        }
    }

    gTobexSoundCaller = true;
    gNormalizeAmplLastValue = 0;
    gSoundFileName = "";

	switch (wCurrentSequence) {
		case SEQ_ATTACK_SLASH:
            sequencesoundset[SEQ_ATTACK_SLASH].PlaySound(wFrame, &cre);

            if (!gSoundFileName.IsEmpty() &&
                sequencesoundset[SEQ_ATTACK_SLASH].GetCount() > 0) {
                console.write_debug("2DA.SEQ_ATTACK_SLASH \t\tsound: %s", gSoundFileName.GetResRefNulled());
                console.write_debug("\t ch:%d", gSndChannel);
                if (gNormalizeAmplLastValue)
                    console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                else
                    console.write_debug("\n");
            }
            break;
		case SEQ_ATTACK_BACKSLASH:
			sequencesoundset[SEQ_ATTACK_BACKSLASH].PlaySound(wFrame, &cre);

            if (!gSoundFileName.IsEmpty() &&
                sequencesoundset[SEQ_ATTACK_BACKSLASH].GetCount() > 0) {
                console.write_debug("2DA.SEQ_ATTACK_BACKSLASH \tsound: %s", gSoundFileName.GetResRefNulled());
                console.write_debug("\t ch:%d", gSndChannel);
                if (gNormalizeAmplLastValue)
                    console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                else
                    console.write_debug("\n");
            }
			break;
		case SEQ_ATTACK_JAB:
            sequencesoundset[SEQ_ATTACK_JAB].PlaySound(wFrame, &cre);

            if (!gSoundFileName.IsEmpty() &&
                sequencesoundset[SEQ_ATTACK_JAB].GetCount() > 0) {
                console.write_debug("2DA.SEQ_ATTACK_JAB \t\tsound: %s", gSoundFileName.GetResRefNulled());
                console.write_debug("\t ch:%d", gSndChannel);
                if (gNormalizeAmplLastValue)
                    console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                else
                    console.write_debug("\n");
            }
			break;
		default:
            if (pGameOptionsEx->bSound_DisableGenericAttackSound &&
                bHasAttack2DASound == true) {
                (this->*Tramp_CAnimation_PlayCurrentSequenceSound)(cre); // sequencesoundset[Cre->wAnimSequenceSimplified].PlaySound(wFrame, &cre)

                if (!gSoundFileName.IsEmpty()) {
                    console.write_debug("2DA.%s \t\t\tsound: %s", GetSeqText(wCurrentSequence), gSoundFileName.GetResRefNulled());
                    console.write_debug("\t ch:%d", gSndChannel);
                    if (gNormalizeAmplLastValue)
                        console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                    else
                        console.write_debug("\n");
                }
            }
			break;
	}

    gSoundFileName = "";
    gNormalizeAmplLastValue = 0;
    gTobexSoundCaller = false;
	return;
}


CRITICAL_SECTION gCriticalSectionSetAnimationSequence;

void __stdcall
CGameSpriteSwing_ForceProcessMessage(CMessageSetAnimationSequence* TargetMsg)
{
    CMessage* pMsg;
    POSITION  pos, current_pos;

    EnterCriticalSection(&gCriticalSectionSetAnimationSequence);
    pos = g_pChitin->messages.GetTailPosition();
    while (pos != NULL) {
        current_pos = pos;
        pMsg = (CMessage *) g_pChitin->messages.GetPrev(pos);
        if ( (pMsg == TargetMsg))
        {
            g_pChitin->messages.RemoveAt(current_pos);
            pMsg->DoMessage();
            delete pMsg;
        }

    }
    LeaveCriticalSection(&gCriticalSectionSetAnimationSequence);
}


BOOL DETOUR_CAnimation::DETOUR_IsAnimation5000(unsigned short wAnimId) {
	if (isInfinityAnimationActive) {
        if (wAnimId >= 0x6600 && wAnimId <= 0x6aff) // IA BG2 pool
            return TRUE;

        if (wAnimId >= 0x6B00 && wAnimId <= 0x6fff) // IA BG1 pool
            return FALSE;

		switch (wAnimId & 0xFFF0) { // class
		case 0x6000: // CLERIC_*
		case 0x6010: // CLERIC_*
		case 0x6100: // FIGHTER_*
		case 0x6110: // FIGHTER_*
		case 0x6200: // MAGE_*
		case 0x6210: // MAGE_*
		case 0x6300: // THIEF_*
		case 0x6310: // THIEF_*
			switch (wAnimId & 0xF) { // race
				case 0: //HUMAN
				case 1: //ELF
				case 2: //DWARF
				case 3: //HALFLING
				case 4: //GNOME
				case 5: //HALF_ORC
					return TRUE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return TRUE;
					break;
			}
			break;

		case 0x6400: // 0x6400-0x6406 DRIZZT - DOOM_GUARD_LARGER, Animation6400
			return FALSE;
			break;

		case 0x6410: // L_CLERIC_*  , Animation6400
		case 0x6420: // L_FIGHTER_* , Animation6400
		case 0x6440: // L_THIEF_*   , Animation6400
			switch (wAnimId & 0xF) { // race
				case 0: //HUMAN
				case 1: //HUMAN
				case 2: //ELF
				case 3: //ELF
				case 4: //DWARF
				case 5: //DWARF
				case 6: //HALFLING
				case 7: //HALFLING
					return FALSE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return FALSE;
					break;
			}
			break;

		case 0x6430: //L_MAGE_*     , Animation6400
			switch (wAnimId & 0xF) {
				case 0: //HUMAN
				case 1: //HUMAN
				case 2: //ELF
				case 3: //ELF
				case 4: //DWARF
				case 5: //DWARF
					return FALSE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return FALSE;
					break;
			}
			break;

		case 0x6500: // MONK_*
		case 0x6510: // MONK_*
			switch (wAnimId & 0xF) {
				case 0: //HUMAN
					return TRUE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return TRUE;
					break;
			}
			break;

		default:
			console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
			return TRUE;
			break;
		}
	}
	else  // orig code (0x5xxx & 0x6000 animations)
	{
		switch (wAnimId & 0x0F00) { // class
		case 0x000: // CLERIC_*
		case 0x100: // FIGHTER_*
		case 0x200: // MAGE_*
		case 0x300: // THIEF_*
			switch (wAnimId & 0xF) { // race
				case 0: //HUMAN
				case 1: //ELF
				case 2: //DWARF
				case 3: //HALFLING
				case 4: //GNOME
				case 5: //HALF_ORC
					return TRUE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return TRUE;
					break;
			}
			break;

		case 0x400: // 0x6400-0x6406 DRIZZT - DOOM_GUARD_LARGER, Animation6400
			return FALSE;
			break;

		case 0x500: // MONK_*
			switch (wAnimId & 0xF) {
				case 0: //HUMAN
					return TRUE;
					break;
				default:
					console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
					return TRUE;
					break;
			}
			break;

		default:
			console.writef("Unknow Human AnimationID 0x%hX \n", wAnimId);
			return TRUE;
			break;
		}
	}

}

LPCTSTR DETOUR_CAnimation::DETOUR_GetWalkingSound(short wTerrainCode) {
	if (!pRuleEx->m_AnimWalkSound.m_2da.bLoaded ||
		!pRuleEx->m_AnimTerrainSound.m_2da.bLoaded)
		return (this->*Tramp_CAnimation_GetWalkingSound)(wTerrainCode);

	IECString sColSound = "WALK_SOUND";
	IECString sColRand = "WALK_NUM";

	char szAnimId[7];
	sprintf_s(szAnimId, 7, "0x%.4X", wAnimId);
	IECString sRowAnimId = szAnimId;

	char* szSound = IENew char[8];
	for (int i = 0; i < 8; i ++) {
		szSound[i] = '\0';
	}
	IECString sSound = pRuleEx->m_AnimWalkSound.GetValue(sColSound, sRowAnimId);
	IECString sRand = pRuleEx->m_AnimWalkSound.GetValue(sColRand, sRowAnimId);

	if (strcmp((LPCTSTR)sSound, "*")) {
		int nRand = 0;

		if (!strcmp((LPCTSTR)sSound, "TERRAIN") &&
			!strcmp((LPCTSTR)sRand, "*")) {
			int nCol = 0;
			int nRow = wTerrainCode;

			if (nCol < pRuleEx->m_AnimTerrainSound.nCols &&
				nRow < pRuleEx->m_AnimTerrainSound.nRows &&
				nCol >= 0 &&
				nRow >= 0) {
				sSound = *((pRuleEx->m_AnimTerrainSound.pDataArray) + (pRuleEx->m_AnimTerrainSound.nCols * nRow + nCol));
			} else {
				sSound = pRuleEx->m_AnimTerrainSound.defaultVal;
			}

			nCol = 1;
			if (nCol < pRuleEx->m_AnimTerrainSound.nCols &&
				nRow < pRuleEx->m_AnimTerrainSound.nRows &&
				nCol >= 0 &&
				nRow >= 0) {
				sRand = *((pRuleEx->m_AnimTerrainSound.pDataArray) + (pRuleEx->m_AnimTerrainSound.nCols * nRow + nCol));
			} else {
				sRand = pRuleEx->m_AnimTerrainSound.defaultVal;
			}
		}

		sscanf_s((LPCTSTR)sRand, "%d", &nRand);
		strncpy_s(szSound, 7, (LPCTSTR)sSound, 6);

		if (nRand > 0) {
			szSound[6] = IERand(nRand) + 'a';
			if (szSound[6] == nRand + 'a' - 1) szSound[6] = '\0';
		} else {
			szSound[6] = '\0';
		}
	}

	return szSound;
};


bool
EXTANIM_IsMageSpecified(CAnimation* Anim) {
    bool        bMageClass = false;

    if (*((DWORD*)Anim) == 0xAB6F90 &&  // CAnimation5000
        pRuleEx->Animation_EXTANI60.m_2da.bLoaded) {
        CAnimation5000* Animation5 = (CAnimation5000*) Anim;
        if (Animation5->u16df & 0x1)    // already found W=1
            bMageClass = true;
        else
        if (Animation5->u16df & 0x2)    // already found W=0
            bMageClass = false;
        else {
            POINT  pos, poscol;
            IECString sString;
            IECString* sValue;
            sString.Format("0x%X", Anim->wAnimId);

            if (pRuleEx->Animation_EXTANI60.FindString(sString, &pos, TRUE)) {
                poscol.y = pos.y;

                poscol.x = 6;                   // anim_class
                sValue = & pRuleEx->Animation_EXTANI60.GetValue(poscol);
                if (strcmp((LPCTSTR)*sValue, "W") == 0) {   // W - mage class
                    bMageClass = true;
                    Animation5->u16df |= 0x1;   // W marker for quick access without 2da search
                } else  // other class
                    Animation5->u16df |= 0x2;
            } else      // no entry
                Animation5->u16df |= 0x2;
        }
    } else

    if (*((DWORD*)Anim) == 0xAB7074 &&   // CAnimation6400
        pRuleEx->Animation_EXTANI64.m_2da.bLoaded) {
        CAnimation6400* Animation6 = (CAnimation6400*) Anim;
        if (Animation6->u36af & 0x1)    // already found W=1
            bMageClass = true;
        else
        if (Animation6->u36af & 0x2)    // already found W=0
            bMageClass = false;
        else {
            POINT  pos, poscol;
            IECString sString;
            IECString* sValue;
            sString.Format("0x%X", Anim->wAnimId);

            if (pRuleEx->Animation_EXTANI64.FindString(sString, &pos, TRUE)) {
                poscol.y = pos.y;

                poscol.x = 5;                   // anim_class
                sValue = & pRuleEx->Animation_EXTANI64.GetValue(poscol);
                if (strcmp((LPCTSTR)*sValue, "W") == 0) {   // W - mage class
                    bMageClass = true;
                    Animation6->u36af |= 0x1;   // W marker for quick access without 2da search
                } else  // other class
                    Animation6->u36af |= 0x2;
            } else      // no entry
                Animation6->u36af |= 0x2;
        }
    }

    return bMageClass;
}

unsigned long __stdcall
CItem_TranslateAnimationType_CheckMageAnimation(CCreatureObject& Cre, CItem& Item)
{
    CAnimation* Anim = Cre.animation.pAnimation;
    IECString*  Prefix50 = NULL;
    IECString*  Prefix64 = NULL;

    if (*((DWORD*)Anim) == 0xAB6F90) {   // CAnimation5000
        CAnimation5000* Animation5 = (CAnimation5000*) Anim;
	    Prefix50 = & Animation5->sPrefix;
    } else
    if (*((DWORD*)Anim) == 0xAB7074) {   // CAnimation6400
        CAnimation6400* Animation6 = (CAnimation6400*) Anim;
	    Prefix64 = & Animation6->sPrefix;
    }

	#ifdef _DEBUG
	    char ArmourPrefix[3];
	    ArmourPrefix[0] = Item.itm.pRes->pFile->m_cAnimationType[1]; // A
	    ArmourPrefix[1] = Item.itm.pRes->pFile->m_cAnimationType[0]; // 2
	    ArmourPrefix[2] = '\0';
	    //console.writef("%s equip armour, char animation %s, armour type %s \n", (LPCTSTR)Cre.GetLongName(), (LPCTSTR)MainPrefix, ArmourPrefix);
	#endif

	if ( isInfinityAnimationActive && (
            EXTANIM_IsMageSpecified(Anim)
            ||
            (Prefix50 &&
                (!Prefix50->Compare("\xA2\x41\xA2\x45") OR   // IA BG2  MOINESSE_MAGE_FEMALE_ELF
                 !Prefix50->Compare("\xA2\x41\xA2\x4B")))    // IA BG2  MOINESSE_MAGE_FEMALE_HUMAN
            ||
            (Prefix64 &&
		        (Anim->wAnimId >= 0x6430 && Anim->wAnimId <= 0x6435)))) { // IA BG1 Mages
            return 0x0200;                          // fake MAGE_* mask animation
	} else {
            return Cre.BaseStats.animId;            // return as is
	}
}


unsigned long __stdcall
CAnimationHuman_SetArmorSound(CAnimation* Anim) {
    unsigned short anim_id = Anim->wAnimId;
    if (isInfinityAnimationActive &&
        ((anim_id >= 0x6430 && anim_id <= 0x6435 ) || // IA BG1 Mages
         EXTANIM_IsMageSpecified(Anim)))
            return 0x0200;          // fake MAGE_* mask animation
    else
            return anim_id;    // return as is
}


BOOL __stdcall
CAnimationHuman_CheckRobe(CAnimation* Anim) {
    unsigned short anim_id = Anim->wAnimId;
    if (isInfinityAnimationActive) {
        if ((anim_id >= 0x6430 && anim_id <= 0x6435) || // IA BG1 Mages
            (anim_id >= 0x6200 && anim_id <= 0x62FF) || // BG2 Mages
            EXTANIM_IsMageSpecified(Anim))
            return TRUE;     // enable for mage animation
    else
            return FALSE;    // disable for non-mage animation
    } else  // no IA installed
        return (anim_id & 0x0200);    // MAGE_* mask
}


unsigned long __stdcall
CAnimation1000_SetReadySound(CAnimation1000& anim) {
    unsigned short anim_id = anim.wAnimId;
    if (isInfinityAnimationActive &&
        anim_id != 0x1000 &&    // WYVERN_BIG (BG2)
        anim_id != 0x1003 &&    // WYVERN_WHITE_BIG (BG2)
        anim_id != 0x100a)      // WYRMLING_ALBINO  (IA)
            return 0x1100;      // fake TANARRI, empty ready sound
    else
            return anim.wAnimId;
}


unsigned long __stdcall
CAnimationA000_SetReadySound(CAnimationA000& anim) {
    unsigned short anim_id = anim.wAnimId;
    if (isInfinityAnimationActive &&
        anim_id != 0xA000 &&    // WYVERN       (BG2)
        anim_id != 0xA002)      // WYVERN_WHITE (IA)
            return 0xA100;      // fake CARRION_CRAWLER, empty ready sound
    else
            return anim.wAnimId;
}


IECString& __stdcall
CItem_TranslateAnimationType_FixAnimCode(IECString& result, char* s) {
    char buf[3] = {0,0,0};

    buf[0] = s[0];
    buf[1] = s[1];
    result = buf;
    return result;
}


void static __stdcall
CAnimationXXX_OverrideSounds(CAnimation& Anim, ushort anim_id) {
    if (pRuleEx->SoundSet_ANISNDEX.m_2da.bLoaded) {
        POINT  pos;
        IECString sString;
        sString.Format("0x%X", anim_id);
        if (pRuleEx->SoundSet_ANISNDEX.FindString(sString, &pos, TRUE)) {
            POINT pos0 = {0, pos.y};    // File column
            IECString& sFilename = pRuleEx->SoundSet_ANISNDEX.GetValue(pos0);
            int Len = sFilename.GetLength();
            if (strcmp((LPCTSTR)sFilename, "****") != 0 &&
                Len > 0 &&
                Len < 9) {
                Anim.rAniSnd = ResRef(sFilename);
            }
        }
    }
}


void static
RemoveAniSndEntry(CAnimation& Anim, int index) {
    CAnimationSoundList& Entry = Anim.sequencesoundset[index];

    POSITION pos = Entry.GetHeadPosition();
	POSITION posOld;
    while ((posOld = pos) != NULL) {
		CSequenceSound* sound = (CSequenceSound*) Entry.GetNext(pos);
		    Entry.RemoveAt(posOld);
			IEFree(sound);
	}
}


#define SET_ANIMPREFIX                                                                                        \
    if (Mode == 0 && strcmp((LPCTSTR)anim->sPrefix, (LPCTSTR)*sValue) != 0) {                                 \
        console.write_debug("Animation & Sound prefix %s->%s\n", (LPCTSTR)anim->sPrefix, (LPCTSTR)*sValue);   \
        anim->sPrefix = *sValue;                                                                              \
    }

#define SET_PAPERDOLLPREFIX5000                                                                               \
    if (Mode == 0 && strcmp((LPCTSTR)anim->sPrefixPaperDoll, (LPCTSTR)*sValue) != 0) {                        \
        console.write_debug("Paperdoll prefix %s->%s\n", (LPCTSTR)anim->sPrefixPaperDoll, (LPCTSTR)*sValue);  \
        anim->sPrefixPaperDoll = *sValue;                                                                             \
    }

#define SET_INFRAVISION                                                              \
    if (Mode == 1) {                                                                 \
        anim->bDetectedByInfravision = to_bool(atoi((LPCTSTR)*sValue));              \
    }

#define SET_USEPALETTE                                                               \
    if (Mode == 2) {                                                                 \
        anim->bUseColorRange = atoi((LPCTSTR)*sValue);                               \
    }


void
SetAnimPrefix(DWORD* Ptr, IECString* sValue, int Mode) {
    //  Mode:
    //  0   sPrefix
    //  1   bDetectedByInfravision
    //  2   bUseColorRange

    //if (*Ptr == 0xAB622C) {  // CAnimation0000
    //    CAnimation0000* anim = (CAnimation0000*) Ptr;
    //    ;
    //} else
    if (*Ptr == 0xAB6C00) {  // CAnimation1000
        CAnimation1000* anim = (CAnimation1000*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB6B1C) {  // CAnimation1200
        CAnimation1200* anim = (CAnimation1200*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB7158) {  // CAnimation1300
        CAnimation1300* anim = (CAnimation1300*) Ptr;
        SET_ANIMPREFIX
        SET_INFRAVISION
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB6DC8) {  // CAnimation2000
        CAnimation2000* anim = (CAnimation2000*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB6EAC) {  // CAnimation3000
        CAnimation3000* anim = (CAnimation3000*) Ptr;
        SET_ANIMPREFIX
    } else
    //if (*Ptr == 0xAB63F4) {  // CAnimation4000
    //    CAnimation4000* anim = (CAnimation4000*) Ptr;
    //    SET_USEPALETTE
    //} else
    if (*Ptr == 0xAB6F90) {  // CAnimation5000
        CAnimation5000* anim = (CAnimation5000*) Ptr;
        SET_ANIMPREFIX
        SET_PAPERDOLLPREFIX5000
        SET_INFRAVISION
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB7074) {  // CAnimation6400
        CAnimation6400* anim = (CAnimation6400*) Ptr;
        SET_ANIMPREFIX
        SET_INFRAVISION
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB678C) {  // CAnimation7000
        CAnimation7000* anim = (CAnimation7000*) Ptr;
        SET_ANIMPREFIX
        SET_INFRAVISION
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB66A0) {  // CAnimation7300
        CAnimation7300* anim = (CAnimation7300*) Ptr;
        SET_ANIMPREFIX
        SET_INFRAVISION
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB6CE4) {  // CAnimation8000
        CAnimation8000* anim = (CAnimation8000*) Ptr;
        SET_ANIMPREFIX
    } else
    if (*Ptr == 0xAB6954) {  // CAnimation9000
        CAnimation9000* anim = (CAnimation9000*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    if (*Ptr == 0xAB6A38) {  // CAnimationA000
        CAnimationA000* anim = (CAnimationA000*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    //if (*Ptr == 0xAB64D8) {  // CAnimationB000
    //    CAnimationB000* anim = (CAnimationB000*) Ptr;
    //    SET_USEPALETTE
    //} else
    if (*Ptr == 0xAB65BC) {  // CAnimationC000
        CAnimationC000* anim = (CAnimationC000*) Ptr;
        SET_ANIMPREFIX
        SET_USEPALETTE
    } else
    //if (*Ptr == 0xAB6310) {  // CAnimationD000
    //    CAnimationD000* anim = (CAnimationD000*) Ptr;
    //    SET_USEPALETTE
    //} else
    if (*Ptr == 0xAB6870) {  // CAnimationE000
        CAnimationE000* anim = (CAnimationE000*) Ptr;
        SET_ANIMPREFIX
        SET_INFRAVISION
    }
}


void static __stdcall
CAnimationXXX_OverrideConfig(CAnimation& Anim) {
    if (pRuleEx->Animation_EXTANIM.m_2da.bLoaded) {
        POINT  pos, poscol;
        IECString sString;
        IECString* sValue;
        sString.Format("0x%X", Anim.wAnimId);
        if (pRuleEx->Animation_EXTANIM.FindString(sString, &pos, TRUE)) {
            poscol.y = pos.y;

            poscol.x = 0;                   // move_scale
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                Anim.nMovementRateDefault = atoi((LPCTSTR)*sValue);
                Anim.nMovementRateCurrent = Anim.nMovementRateDefault;
            }

            poscol.x = 1;                   // personal_space
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                Anim.nFootCircleSize = atoi((LPCTSTR)*sValue);
                Anim.rFootcircle.right = 0; // trigger for SetFootCircle()
                Anim.SetFootCircle();
            }

            poscol.x = 2;                   // color_blood
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                Anim.colorBlood = atoi((LPCTSTR)*sValue);
            }

            poscol.x = 3;                   // color_chunks
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                if (atoi((LPCTSTR)*sValue) == 1)
                    Anim.colorChunks = 0;
                else
                    Anim.colorChunks = 255;
            }

            poscol.x = 4;                   // sound_freq
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                Anim.nWalkSndFreq = atoi((LPCTSTR)*sValue);
            }

            poscol.x = 5;                   // sound_death
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                console.write_debug("FallingSound= %s\n", (LPCTSTR)*sValue);
                Anim.szFallingSound = (LPCTSTR)*sValue;
            }

            poscol.x = 6;                  // brightest
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                console.write_debug("bBrightest= %d\n", atoi((LPCTSTR)*sValue));
                Anim.bBrightest = to_bool(atoi((LPCTSTR)*sValue));
            }

            poscol.x = 7;                   // light_source
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                console.write_debug("bLightSource= %d\n", atoi((LPCTSTR)*sValue));
                Anim.bLightSource = to_bool(atoi((LPCTSTR)*sValue));
            }

            poscol.x = 8;                   // detected_by_infravision
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                SetAnimPrefix((DWORD*) &Anim, sValue, 1);
            }

            // poscol.x = 9;                // false_color

            // poscol.x = 10;               // resref

            // poscol.x = 11;               // resref_paperdoll

        }

    }
}


void static __stdcall
EXTANIM_InjectPrefix(CAnimation* Anim) {
    if (pRuleEx->Animation_EXTANIM.m_2da.bLoaded) {

        if ( *((DWORD*)Anim) == 0xAB6F90) {    // CAnimation5000
            CAnimation5000* Anim5000 = (CAnimation5000*) Anim;
            Anim5000->u16df = 0;
        }

        if ( *((DWORD*)Anim) == 0xAB7074) {    // CAnimation6400
            CAnimation6400* Anim6400 = (CAnimation6400*) Anim;
            Anim6400->u36af = 0;
        }

        {
            POINT  pos, poscol;
            IECString sString;
            IECString* sValue;
            sString.Format("0x%X", Anim->wAnimId);
            if (pRuleEx->Animation_EXTANIM.FindString(sString, &pos, TRUE)) {
                poscol.y = pos.y;

                poscol.x = 9;                   // false_color
                sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
                if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                    SetAnimPrefix((DWORD*) Anim, sValue, 2);
                }

                poscol.x = 10;                   // resref
                sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
                if (strcmp((LPCTSTR)*sValue, "*") != 0) {
                    SetAnimPrefix((DWORD*) Anim, sValue, 0);
                }
            }
        }

        if ( *((DWORD*)Anim) == 0xAB6F90 &&    // CAnimation5000
             pRuleEx->Animation_EXTANI60.m_2da.bLoaded) {
            CAnimation5000* Anim5000 = (CAnimation5000*) Anim;
            POINT  pos5, poscol5;
            IECString sString5;
            IECString* sValue5;
            sString5.Format("0x%X", Anim->wAnimId);

            if (pRuleEx->Animation_EXTANI60.FindString(sString5, &pos5, TRUE)) {
                poscol5.y = pos5.y;

                poscol5.x = 0;                   // armor_max_code
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->cArmorMaxCode = sValue5->GetAt(0);
                }

                poscol5.x = 1;                   // equip_helmet
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->bEquipHelmet = atoi((LPCTSTR)*sValue5);
                }

                poscol5.x = 2;                   // split_bams
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->bSplitBams = atoi((LPCTSTR)*sValue5);
                }

                poscol5.x = 3;                   // height_code
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->sHeightCode = *sValue5;
                }

                poscol5.x = 4;                   // height_code_helmet
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->sHeightCodeHelmet = *sValue5;
                }

                poscol5.x = 5;                   // height_code_shield
                sValue5 = & pRuleEx->Animation_EXTANI60.GetValue(poscol5);
                if (strcmp((LPCTSTR)*sValue5, "*") != 0) {
                    Anim5000->sHeightCodeShieldPaperDoll = *sValue5;
                }

                // poscol.x = 6;               // anim_class
            }
        }

        if ( *((DWORD*)Anim) == 0xAB7074 &&    // CAnimation6400
             pRuleEx->Animation_EXTANI64.m_2da.bLoaded) {
            CAnimation6400* Anim6400 = (CAnimation6400*) Anim;
            POINT  pos6, poscol6;
            IECString sString6;
            IECString* sValue6;
            sString6.Format("0x%X", Anim->wAnimId);

            if (pRuleEx->Animation_EXTANI64.FindString(sString6, &pos6, TRUE)) {
                poscol6.y = pos6.y;

                poscol6.x = 0;                   // armor_max_code
                sValue6 = & pRuleEx->Animation_EXTANI64.GetValue(poscol6);
                if (strcmp((LPCTSTR)*sValue6, "*") != 0) {
                    Anim6400->cArmorMaxCode = sValue6->GetAt(0);
                }

                poscol6.x = 1;                   // equip_helmet
                sValue6 = & pRuleEx->Animation_EXTANI64.GetValue(poscol6);
                if (strcmp((LPCTSTR)*sValue6, "*") != 0) {
                    Anim6400->bEquipHelmet = atoi((LPCTSTR)*sValue6);
                }

                poscol6.x = 2;                   // height_code
                sValue6 = & pRuleEx->Animation_EXTANI64.GetValue(poscol6);
                if (strcmp((LPCTSTR)*sValue6, "*") != 0) {
                    Anim6400->sHeightCode = *sValue6;
                }

                poscol6.x = 3;                   // height_code_helmet
                sValue6 = & pRuleEx->Animation_EXTANI64.GetValue(poscol6);
                if (strcmp((LPCTSTR)*sValue6, "*") != 0) {
                    Anim6400->sHeightCodeHelmet = *sValue6;
                }

                poscol6.x = 4;                   // shadow
                sValue6 = & pRuleEx->Animation_EXTANI64.GetValue(poscol6);
                if (strcmp((LPCTSTR)*sValue6, "*") != 0) {
                    Anim6400->sShadowPrefix = *sValue6;
                }

                // poscol.x = 5;               // anim_class
            }
        }

    }
}


void static __stdcall
CInfGame_GetAnimationBam_InjectPaperDollPrefix(AnimData& AnimData, IECString& sPrefix, uchar range)
{
    CAnimation* Anim = AnimData.pAnimation;

    if (pRuleEx->Animation_EXTANIM.m_2da.bLoaded) {
        POINT  pos, poscol;
        IECString sString;
        IECString NewPrefix;
        IECString* sValue;
        IECString* sValueResref;
        ushort AnimId = Anim->wAnimId;
        sString.Format("0x%X", AnimId);
        if (pRuleEx->Animation_EXTANIM.FindString(sString, &pos, TRUE)) {
            poscol.y = pos.y;

            poscol.x = 11;                   // resref_paperdoll
            sValue =       & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            poscol.x = 10;                   // resref
            sValueResref = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            
            if (range == 0) {   // BODY
                if (*((DWORD*)Anim) == 0xAB6F90) {   // if CAnimation5000
                    if (strcmp((LPCTSTR)*sValue, "*")       != 0 ||     // RESREF_PAPERDOLL is set
                        strcmp((LPCTSTR)*sValueResref, "*") != 0) {     // RESREF is set
                            CAnimation5000* anim = (CAnimation5000*) Anim;
                            if (strcmp((LPCTSTR)*sValue, "*") != 0)     // RESREF_PAPERDOLL is set
                                NewPrefix = *sValue + anim->cArmorCode;
                            else
                                NewPrefix = *sValueResref + anim->cArmorCode;

                            console.write_debug("Body   prefix %s\n", (LPCTSTR)NewPrefix);
                            sPrefix = NewPrefix;
                    }
                } else
                if (*((DWORD*)Anim) == 0xAB7074) {   // if CAnimation6400
                    if (strcmp((LPCTSTR)*sValue, "*")       != 0 ||     // RESREF_PAPERDOLL is set
                        strcmp((LPCTSTR)*sValueResref, "*") != 0) {     // RESREF is set
                            CAnimation6400* anim = (CAnimation6400*) Anim;
                            if (strcmp((LPCTSTR)*sValue, "*") != 0)     // RESREF_PAPERDOLL is set
                                NewPrefix = *sValue + anim->cArmorCode;
                            else
                                NewPrefix = *sValueResref + anim->cArmorCode;

                            console.write_debug("Body   prefix %s\n", (LPCTSTR)NewPrefix);
                            sPrefix = NewPrefix;
                    }
                } else
                if (strcmp((LPCTSTR)*sValue, "*") != 0) {   // not 5000/6400
                        NewPrefix = *sValue;
                        console.write_debug("Body   prefix %s\n", (LPCTSTR)NewPrefix);
                        sPrefix = NewPrefix;
                }
            } else

            if (range == 0x10) {    // WEAPON
                if (*((DWORD*)Anim) == 0xAB6F90) {   // CAnimation5000
                    CAnimation5000* anim = (CAnimation5000*) Anim;

                    if (anim->pvcWeaponCurrent) {
                        if (pRuleEx->Animation_EXTANI60.FindString(sString, &pos, TRUE)) {
                            poscol.y = pos.y;
                            poscol.x = 3;                   // height_code
                            sValue = & pRuleEx->Animation_EXTANI60.GetValue(poscol);
                            if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQS") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQM") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQN") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQL") != 0 ) {
                                NewPrefix = anim->sHeightCode + 'P' + anim->sWeaponPrefix;
                                console.write_debug("Weapon prefix %s\n", (LPCTSTR)NewPrefix);
                                sPrefix = NewPrefix;
                            }
                        }
                    }
                }

                if (*((DWORD*)Anim) == 0xAB7074) {   // CAnimation6400
                    CAnimation6400* anim = (CAnimation6400*) Anim;

                    if (anim->pvcWeaponCurrent) {
                        if (pRuleEx->Animation_EXTANI64.FindString(sString, &pos, TRUE)) {
                            poscol.y = pos.y;
                            poscol.x = 2;                   // height_code
                            sValue = & pRuleEx->Animation_EXTANI64.GetValue(poscol);
                            if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPS") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPM") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPL") != 0 ) {
                                NewPrefix = anim->sHeightCode + 'P' + anim->sWeaponPrefix;
                                console.write_debug("Weapon prefix %s\n", (LPCTSTR)NewPrefix);
                                sPrefix = NewPrefix;
                            }
                        }
                    }
                }
            } else

            if (range == 0x20) {    // SHIELD
                if (*((DWORD*)Anim) == 0xAB6F90) {   // CAnimation5000
                    CAnimation5000* anim = (CAnimation5000*) Anim;

                    if (anim->pvcShieldCurrent) {
                        if (anim->sHeightCodeShieldPaperDoll.IsEmpty()) {
                            if (pRuleEx->Animation_EXTANI60.FindString(sString, &pos, TRUE)) {
                                poscol.y = pos.y;
                                poscol.x = 3;                   // height_code
                                sValue = & pRuleEx->Animation_EXTANI60.GetValue(poscol);
                                if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQS") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQM") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQN") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQL") != 0 ) {
                                    NewPrefix = anim->sHeightCode + 'P' + anim->sShieldPrefix;
                                    console.write_debug("Shield prefix %s\n", (LPCTSTR)NewPrefix);
                                    sPrefix = NewPrefix;
                                }
                            }
                        } else {
                            if (pRuleEx->Animation_EXTANI60.FindString(sString, &pos, TRUE)) {
                                poscol.y = pos.y;
                                poscol.x = 5;                   // height_code_shield
                                sValue = & pRuleEx->Animation_EXTANI60.GetValue(poscol);
                                if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQS") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQM") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQN") != 0 &&
                                    strcmp((LPCTSTR)*sValue, "WQL") != 0 ) {
                                    NewPrefix = anim->sHeightCodeShieldPaperDoll + 'P' + anim->sShieldPrefix;
                                    console.write_debug("Shield prefix %s\n", (LPCTSTR)NewPrefix);
                                    sPrefix = NewPrefix;
                                }
                            }
                        }
                    }
                }

                if (*((DWORD*)Anim) == 0xAB7074) {   // CAnimation6400
                    CAnimation6400* anim = (CAnimation6400*) Anim;

                    if (anim->pvcShieldCurrent ||
                        !anim->sShieldPrefix.IsEmpty()) {
                        if (pRuleEx->Animation_EXTANI64.FindString(sString, &pos, TRUE)) {
                            poscol.y = pos.y;
                            poscol.x = 2;                   // height_code
                            sValue = & pRuleEx->Animation_EXTANI64.GetValue(poscol);
                            if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPS") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPM") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPL") != 0 ) {
                                NewPrefix = anim->sHeightCode + 'P' + anim->sShieldPrefix;
                                console.write_debug("Shield prefix %s\n", (LPCTSTR)NewPrefix);
                                sPrefix = NewPrefix;
                            }
                        }
                    }
                }
            } else

            if (range == 0x30) {    // HELMET
                if (*((DWORD*)Anim) == 0xAB6F90) {   // CAnimation5000
                    CAnimation5000* anim = (CAnimation5000*) Anim;

                    if (anim->bEquipHelmet &&
                        anim->pvcHelmetCurrent ) {
                        if (pRuleEx->Animation_EXTANI60.FindString(sString, &pos, TRUE)) {
                            poscol.y = pos.y;
                            poscol.x = 4;                   // height_code_helmet
                            sValue = & pRuleEx->Animation_EXTANI60.GetValue(poscol);
                            if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQS") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQM") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQN") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WQL") != 0 ) {
                                NewPrefix = anim->sHeightCodeHelmet + 'P' + anim->sHelmetPrefix;
                                console.write_debug("Helmet prefix %s\n", (LPCTSTR)NewPrefix);
                                sPrefix = NewPrefix;
                            }
                        }
                    }
                }

                if (*((DWORD*)Anim) == 0xAB7074) {   // CAnimation6400
                    CAnimation6400* anim = (CAnimation6400*) Anim;

                        if (anim->bEquipHelmet &&
                            anim->pvcHelmetCurrent ) {
                        if (pRuleEx->Animation_EXTANI64.FindString(sString, &pos, TRUE)) {
                            poscol.y = pos.y;
                            poscol.x = 3;                   // height_code_helmet
                            sValue = & pRuleEx->Animation_EXTANI64.GetValue(poscol);
                            if (strcmp((LPCTSTR)*sValue, "*")   != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPS") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPM") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPL") != 0 &&
                                strcmp((LPCTSTR)*sValue, "WPT") != 0 ) {
                                NewPrefix = anim->sHeightCodeHelmet + 'P' + anim->sHelmetPrefix;
                                console.write_debug("Helmet prefix %s\n", (LPCTSTR)NewPrefix);
                                sPrefix = NewPrefix;
                            }
                        }
                    }
                }
            }

        }
    }
}


BOOL static __stdcall
CAnimation5000_EquipArmor_isOverrided(CAnimation5000* Anim) {
    if (pRuleEx->Animation_EXTANIM.m_2da.bLoaded) {
        POINT  pos, poscol;
        IECString sString;
        IECString NewPrefix;
        IECString* sValue;
        ushort AnimId = Anim->wAnimId;
        sString.Format("0x%X", AnimId);
        if (pRuleEx->Animation_EXTANIM.FindString(sString, &pos, TRUE)) {
            poscol.y = pos.y;

            poscol.x = 10;                   // resref
            sValue = & pRuleEx->Animation_EXTANIM.GetValue(poscol);
            if (strcmp((LPCTSTR)*sValue, "*") != 0)
                return TRUE;
            else
                return FALSE;
        } else
            return FALSE;
    } else
        return FALSE;
}

CCreatureObject* gCreateAnimationCre = NULL;

void static __stdcall
CAnimationXXX_Override2DASndEntries(CAnimation& Anim, DWORD EIP, DWORD ParentEBP) {
    CCreatureObject* Cre = NULL;
    CStrRef ref;

    // non Creatures
    if (EIP == 0x8BD620) {   // CGameTemporal::CGameTemporal
        return;
    } else
    if (EIP == 0x8BCAEA) {   // CGameChunk::CGameChunk
        return;
    } else
    if (EIP == 0x8B5A5B) {   // CGameSprite::Shatter
        return;
    } else
    if (EIP == 0x8B5CFE) {   // CGameSprite::Shatter
        return;
    } else
    if (EIP == 0x8B5FA1) {   // CGameSprite::Shatter
        return;
    } else
    if (EIP == 0x8B6246) {   // CGameSprite::Shatter
        return;
    }

    // Creatures
    if (EIP == 0x8BA72B) {   // CGameSprite::Unmarshal
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x1E8);
    } else
    if (EIP == 0x8ED30D) {   // CGameSprite::ProcessEffectList
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x880);
    } else
    if (EIP == 0x5291CC) {   // CEffectPolymorph::Apply
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x52976D) {   // CEffectPolymorph::Apply
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x34);
    } else
    if (EIP == 0x6991B1) {   // CInfGame::StepAnimation
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x644);
    } else
    if (EIP == 0x699609) {   // CInfGame::StepAnimation
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x644);
    } else
    if (EIP == 0x723DDA) {   // CScreenCreateChar::CompleteCharacterClass
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x6F0B88) {   // CScreenCharacter::MakeDualClass
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x4458FB) {   // CBaldurMessage::OnUpdateCharacterSlotReply
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-0x104);
    } else
    if (EIP == 0x49C4E7) {   // CGameAIBase::CreateCreatureObject
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x952BAE) {   // CGameSprite::PolymorphCopy
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x952CA8) {   // CGameSprite::PolymorphCopy
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x517B73) {   // CGameEffectAnimationChange::ApplyEffect
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x517EBF) {   // CGameEffectAnimationChange::ApplyEffect
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x51A779) {   // CGameEffectSexChange::ApplyEffect
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP+8);
    } else
    if (EIP == 0x5BCEAF) {   // CMessageAnimationChange::Run
        Cre = (CCreatureObject*) *(DWORD *)(ParentEBP-8);
    } else {
        if (gCreateAnimationCre ==  NULL) {
            console.writef("Unknow CAnimation::CAnimation() %x\n", EIP);
            return;
        } else {
            Cre = gCreateAnimationCre;
        }
    }

    if (*(DWORD*)Cre != 0xAA98A8) { //  CCreatureObject vtbl
        console.writef("Broken CAnimation() Creature %x\n", EIP);
        return;
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[18], ref); // DAMAGE
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 4);             // DAMAGE
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[19], ref);  // DYING
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 5);             // DIE
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[14], ref);  // ATTACK1
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 11);            // ATTACK_SLASH
        RemoveAniSndEntry(Anim, 1);             // ATTACK
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[15], ref);  // ATTACK2
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 12);            // ATTACK_BACKSLASH
        RemoveAniSndEntry(Anim, 1);             // ATTACK
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[16], ref);  // ATTACK3
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 13);            // ATTACK_JAB
        RemoveAniSndEntry(Anim, 1);             // ATTACK
    }

    g_pChitin->m_TlkTbl.GetTlkString(Cre->BaseStats.soundset[17], ref);  // ATTACK4
    if (!ref.sound.wav.soundName.IsEmpty()) {
        RemoveAniSndEntry(Anim, 8);             // SHOOT
        RemoveAniSndEntry(Anim, 1);             // ATTACK
    }
}


BOOL static __stdcall
CAnimation6400_EquipWeapon_SkipOffHand(CAnimation6400& anim, IECString& WeaponString, ulong itemFlags, ColorRangeValues& colorRangeValues) {

    if (itemFlags & 0x400) {    // off-hand weapon
        anim.sShieldPrefix = WeaponString;

        // fill vpShieldColorRanges
        uchar *pcolorRangeValues =  (uchar *) &colorRangeValues;
        CVidBitmap& colorRangePalette = g_pChitin->pGame->m_colorRangePalette;
        for (ushort ColorIndex = 0; ColorIndex < 7; ColorIndex++ ) {
          anim.vpShieldColorRanges.SetRange(ColorIndex, pcolorRangeValues[ColorIndex], colorRangePalette);
        }

        //anim.EquipShield(WeaponString, colorRangeValues);

        return TRUE;    // skip equipping

    } else
        return FALSE;   // equip main hand
}


void static __stdcall
CGameSprite_SetSequence_Log(CCreatureObject& Cre, short seqID) {
    Enum id = Cre.oBase.eTarget;
    short result = g_pChitin->pGame->GetPartyMemberSlot(id);

    //if (result != -1) {   // Party
    //if (Cre.wAnimSequenceSimplified != seqID) {
        //console.write_debug("seq= %s \n", GetSeqText(seqID));
    //}
}


BOOL static __stdcall
CCreativeObject_SetCurrentAction_SetREADY(CCreatureObject& Cre, Action& action) {
    if (Cre.wAnimSequenceSimplified == SEQ_HEAD_TURN && 
        (action.opcode == 22  || // MoveToObject
         action.opcode == 23  || // MoveToPoint
         action.opcode == 90  || // MoveToPointNoRecticle
         action.opcode == 107 || // MoveToOffset
         action.opcode == 180 || // MoveToObjectFollow
         action.opcode == 207 || // MoveToPointNoInterrupt
         action.opcode == 208    // MoveToObjectNoInterrupt
        ))
        return FALSE;   // skip SetSequence(SEQ_READY)
    else
        return TRUE;
}


void __declspec(naked)
CItem_TranslateAnimationType_CheckMageAnimation_asm()
{
__asm {
	push    ecx
	push    edx

	push    [ebp-14h]   ; CItem
	push    ecx         ; CCreature
	call    CItem_TranslateAnimationType_CheckMageAnimation

	pop     edx
	pop     ecx
	
	mov     edx, eax  ; stolen bytes

	ret
}
}


void __declspec(naked)
CAnimation5000_SetArmorSound_asm()
{
__asm {
	push    ecx
	push    edx

	push    [ebp-14h]   ; Animation5000
	call    CAnimationHuman_SetArmorSound
    ; return eax - anim_id

	pop     edx
	pop     ecx
	ret
}
}


void __declspec(naked)
CAnimation6400_SetArmorSound_asm()
{
__asm {
	push    ecx
	push    edx

	push    [ebp-14h]   ; Animation6400
	call    CAnimationHuman_SetArmorSound
    ; return eax - anim_id

	pop     edx
	pop     ecx
	ret
}
}


void __declspec(naked)
CAnimation5000_SetArmorSoundRobe_asm()
{
__asm {
    push    eax
    push    ecx
	push    edx

	push    [ebp-14h]       ; Animation5000
	call    CAnimationHuman_CheckRobe
    // return eax - anim_id
    test    eax,eax
    jz      CAnimation5000_SetArmorSoundRobe_asm_exit

    mov     eax, [ebp-14h]  ; Animation5000
    mov     cl, [eax+6ECh]  ; armorcode
    cmp     cl, 0x31        ; no armor
    jz      CAnimation5000_SetArmorSoundRobe_asm_exit

    mov     byte ptr [ebp-0Ch], 0x31   ; force ARM_01*

    pop     edx
	pop     ecx
    pop     eax

    add     esp, 4
    push    0851CF4h    ; calculate filename
    ret

CAnimation5000_SetArmorSoundRobe_asm_exit:
    pop     edx
	pop     ecx
    pop     eax

    and     eax, 0F00h  ; stolen bytes
	ret
}
}


void __declspec(naked)
CAnimation6400_SetArmorSoundRobe_asm()
{
__asm {
    push    eax
    push    ecx
	push    edx

	push    [ebp-14h]       ; Animation6400
	call    CAnimationHuman_CheckRobe
    // return eax - anim_id
    test    eax,eax
    jz      CAnimation6400_SetArmorSoundRobe_asm_exit

    mov     eax, [ebp-14h]  ; Animation6400
    mov     cl, [eax+6e8h]  ; armorcode
    cmp     cl, 0x31        ; no armor
    jz      CAnimation6400_SetArmorSoundRobe_asm_exit

    mov     byte ptr [ebp-0Ch], 0x31   ; force ARM_01*

    pop     edx
	pop     ecx
    pop     eax

    add     esp, 4
    push    08702D2h    ; calculate filename
    ret

CAnimation6400_SetArmorSoundRobe_asm_exit:
    pop     edx
	pop     ecx
    pop     eax

    and     eax, 0F00h  ; stolen bytes
	ret
}
}


void __declspec(naked)
CAnimation1000_SetReadySound_asm()
{
__asm {
	push    edx

	push    [ebp-18h]   ; Animation1000
	call    CAnimation1000_SetReadySound
    ; return eax - anim_id

    mov     ecx, eax
    and     ecx, 0F00h

	pop     edx
	ret
}
}


void __declspec(naked)
CAnimationA000_SetReadySound_asm()
{
__asm {
	push    edx

	push    [ebp-18h]   ; Animation1000
	call    CAnimationA000_SetReadySound
    ; return eax - anim_id

    mov     ecx, eax
    and     ecx, 0F00h

	pop     edx
	ret
}
}


void __declspec(naked)
CItem_TranslateAnimationType_FixAnimCode_asm()
{
__asm {
	push    ecx
	push    edx

	push    [esp+0Ch]   // arg0, char *, not nulled
    push    ecx         // CString
	call    CItem_TranslateAnimationType_FixAnimCode

	pop     edx
	pop     ecx
	ret     4
}
}


void __declspec(naked)
CAnimationXXX_OverrideAniSndReference_asm()
{
__asm {
    push    eax
	push    ecx
	push    edx

	push    [ebp+8]     // animation_id
    push    [ebp-10h]   // CAnimation
	call    CAnimationXXX_OverrideSounds

	pop     edx
	pop     ecx
    pop     eax

    mov     [eax+4], cx     // stolen bytes
    mov     ecx, [ebp-10h]
	ret     
}
}


void __declspec(naked)
CAnimation_SetAnimationType_PostHooks_asm()
{
__asm {
	push    ecx
	push    edx

    mov     ecx, [pGameOptionsEx]
    cmp     dword ptr [ecx]CGameOptionsEx.bSound_PriorityCREOver2DA, 1
    jnz     CAnimation_SetAnimationType_PostHooks_asm_Skip1

    mov     eax,[ebp]   // Parent EBP
    push    eax
	push    [ebp+4]     // EIP
    push    [ebp-10h]   // CAnimation
	call    CAnimationXXX_Override2DASndEntries
    
CAnimation_SetAnimationType_PostHooks_asm_Skip1:
    mov     ecx, [pGameOptionsEx]
    cmp     dword ptr [ecx]CGameOptionsEx.bAnimation_EXTANIM, 1
    jnz     CAnimation_SetAnimationType_PostHooks_asm_Skip2

    push    [ebp-10h]   // CAnimation
    call    CAnimationXXX_OverrideConfig

CAnimation_SetAnimationType_PostHooks_asm_Skip2:
	pop     edx
	pop     ecx

    mov     eax, [ebp-10h]     // stolen bytes
    mov     ecx, [ebp-0Ch]
	ret     
}
}


void __declspec(naked)
CGameSpriteSwing_ForceProcessMessage_asm()
{
__asm {
    cmp     word ptr [ebp-0ACh], 0  // if Sequence == Attack
    jz      CGameSpriteSwing_ForceProcessMessage_asm_Skip

	push    ecx
	push    edx

    push    [ebp-0A4h]  // Message
	call    CGameSpriteSwing_ForceProcessMessage

	pop     edx
	pop     ecx

CGameSpriteSwing_ForceProcessMessage_asm_Skip:
    mov     eax, [ebp-0B4h]     // stolen bytes
	ret     
}
}


void __declspec(naked)
CGameSpriteSwing_SwapMessage_asm() 
{
__asm {
    // stolen bytes
    push    0
    mov     edx, [ebp-0A4h]
    push    edx
    mov     ecx, g_pChitin
    add     ecx, 6C48h
    call    CPtrListMessage::Send

    cmp     dword ptr [ebp-6ECh], 0  // message not null
    jz      CGameSpriteSwing_SwapMessage_asm_Skip

	push    ecx
	push    edx

    push    [ebp-0A4h]  // Message
	call    CGameSpriteSwing_ForceProcessMessage

    // stolen bytes
    mov     edx, [ebp-0B4h] // ItemLauncher
    push    edx
    mov     eax, [ebp-2Ch]  // ItemMelee
    push    eax
    mov     ecx, [ebp-614h] // Cre
    mov     eax, 094165Eh   // CGameSprite::DecodeSwing()
    call    eax

	pop     edx
	pop     ecx

CGameSpriteSwing_SwapMessage_asm_Skip:
	ret     
}
}


void __declspec(naked)
CGameSprite_DecodeSwingSound_Attack4_asm() 
{
__asm {
    push    eax
	push    ecx
	push    edx

    mov     eax, [ebp-148h]         // wCurrentSequence
	cmp     eax, 8                  // SEQ_SHOOT
    jnz     CGameSprite_DecodeSwingSound_Attack4_asm_Next
    mov     byte ptr [ebp-90h], 17  // ATTACK4
    jmp     CGameSprite_DecodeSwingSound_Attack4_asm_Continue

CGameSprite_DecodeSwingSound_Attack4_asm_Next:
    cmp     eax, 4                  // SEQ_DAMAGE
    jnz     CGameSprite_DecodeSwingSound_Attack4_asm_Default
    mov     byte ptr [ebp-90h], 18  // DAMAGE
    jmp     CGameSprite_DecodeSwingSound_Attack4_asm_Continue

CGameSprite_DecodeSwingSound_Attack4_asm_Default:
    mov     byte ptr [ebp-90h], 14  // ATTACK1

CGameSprite_DecodeSwingSound_Attack4_asm_Continue:
	pop     edx
	pop     ecx
    pop     eax

	ret     
}
}


void __declspec(naked)
CAnimation2000_SetAnimationSequence_SetShootSeq_asm()
{
__asm {
    mov     edx, [ebp-14h]
    mov     dword ptr [edx+0DF0h], 1

    add     esp, 4
    push    0840D2Dh    // 0x2000/0x2100 animation without shoot frames
    ret
}
}


#define EXTANIM_SETPREFIX(x)        \
    __asm   push    eax             \
	__asm   push    ecx             \
	__asm   push    edx             \
                                    \
    __asm   push    [ebp-(x)]       \
    __asm   call    EXTANIM_InjectPrefix    \
                                    \
    __asm   pop     edx             \
	__asm   pop     ecx             \
    __asm   pop     eax             \
                                    \
    __asm   mov     ecx, [ebp-(x)]  \
    __asm   ret


void __declspec(naked)
CAnimation1000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(1BCh)
}

void __declspec(naked)
CAnimation1200_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(3B8h)
}

void __declspec(naked)
CAnimation1300_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(0B0h)
}

void __declspec(naked)
CAnimation2000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(108h)
}

void __declspec(naked)
CAnimation3000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(13Ch)
}

//void __declspec(naked)
//CAnimation4000_InjectPrefixOverride_asm()
//{
//    EXTANIM_SETPREFIX(0h)
//}

void __declspec(naked)
CAnimation5000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(220h)
}

void __declspec(naked)
CAnimation6400_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(79Ch)
}

void __declspec(naked)
CAnimation7000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(3E8h)
}

void __declspec(naked)
CAnimation7300_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(78Ch)
}

void __declspec(naked)
CAnimation8000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(108h)
}

void __declspec(naked)
CAnimation9000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(0B8h)
}

void __declspec(naked)
CAnimationA000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(0C0h)
}

//void __declspec(naked)
//CAnimationB000_InjectPrefixOverride_asm()
//{
//    EXTANIM_SETPREFIX(0h)
//}

void __declspec(naked)
CAnimationC000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(0A0h)
}

//void __declspec(naked)
//CAnimationD000_InjectPrefixOverride_asm()
//{
//    EXTANIM_SETPREFIX(0h)
//}

void __declspec(naked)
CAnimationE000_InjectPrefixOverride_asm()
{
    EXTANIM_SETPREFIX(464h)
}


void __declspec(naked)
CInfGame_GetAnimationBam_InjectPaperDollPrefix_asm()
{
__asm {
    mov     [ebp-10h], al  // Stolen bytes
    lea     edx, [ebp-14h]

	push    ecx
	push    edx

    push    [ebp+10h]   // range
    push    edx         // IECString
    push    [ebp-20h]   // AnimData
	call    CInfGame_GetAnimationBam_InjectPaperDollPrefix

	pop     edx
	pop     ecx

	ret     
}
}


void __declspec(naked)
CAnimation5000_EquipArmor_SkipPrefixChange_asm()
{
__asm {
	push    ecx
	push    edx

    push    [ebp-1E0h]  // Anim
	call    CAnimation5000_EquipArmor_isOverrided
    test    eax,eax
	pop     edx
	pop     ecx
    jz      CAnimation5000_EquipArmor_SkipPrefixChange_asm_Continue

    add     esp, 4
    push    084715Dh        // SkipPrefixChange
    ret

CAnimation5000_EquipArmor_SkipPrefixChange_asm_Continue:
    mov     edx, [ebp-1E0h] // stolen bytes
	ret     
}
}


void __declspec(naked)
CAnimation6400_EquipWeapon_SkipOffHand_asm()
{
__asm {
	push    ecx
	push    edx
    push    eax

    push    [ebp+0Ch]       // colorRangeValues
    push    [ebp+10h]       // itemFlags
    push    [ebp+8]         // WeaponString
    push    [ebp-0BE4h]     // Anim
	call    CAnimation6400_EquipWeapon_SkipOffHand

    test    eax, eax
    pop     eax
	pop     edx
	pop     ecx
    jz      CAnimation6400_EquipWeapon_SkipOffHand_asm_continue
    
    add     esp, 4
    push    086FDC5h         // abort EquipWeapon()
    ret

CAnimation6400_EquipWeapon_SkipOffHand_asm_continue:
    mov     eax, [ebp-0BE4h] // stolen bytes
	ret     
}
}


void __declspec(naked)
CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm()
{
__asm {
	push    ecx
	push    edx
    push    eax

    mov     ecx, eax            // anim
    add     ecx, 19b2h          // sShieldPrefix
    call    IECString::IsEmpty  // return BOOL
    test    eax, eax
   
    pop     eax
	pop     edx
	pop     ecx
    jnz     CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm_Continue

    cmp     eax, 0A5A5A5A5h     // force Z=0
    ret

CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm_Continue:
    cmp     dword ptr [eax+19B6h], 0 // stolen bytes
	ret     
}
}


void __declspec(naked)
CAnimation6400_SetColorRange_CheckOffHandWeapon_asm()
{
__asm {
	push    ecx
	push    edx
    push    eax

    ; ecx                       // anim
    add     ecx, 19b2h          // sShieldPrefix
    call    IECString::IsEmpty  // return BOOL
    test    eax, eax
   
    pop     eax
	pop     edx
	pop     ecx
    jnz     CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm_Continue

    cmp     eax, 0A5A5A5A5h     // force Z=0
    ret

CAnimation6400_GetAnimationPalette_CheckOffHandWeapon_asm_Continue:
    cmp     dword ptr [ecx+19B6h], 0 // stolen bytes
	ret     
}
}


void __declspec(naked)
CGameSprite_SetSequence_Log_asm()
{
__asm {
	push    ecx
	push    edx
    push    eax

    push    [ebp+8]     // seq
    push    [ebp-150h]  // Cre
    call    CGameSprite_SetSequence_Log
   
    pop     eax
	pop     edx
	pop     ecx

    cmp     dword ptr [ecx+6412h], 0 // stolen bytes
	ret     
}
}


void __declspec(naked)
CCreativeObject_SetCurrentAction_SetREADY_asm()
{
__asm {
	push    ecx
	push    edx
    push    eax

    push    [ebp+8]     // Action
    push    [ebp-14Ch]  // Cre
    call    CCreativeObject_SetCurrentAction_SetREADY
    cmp     eax,eax

    pop     eax
	pop     edx
	pop     ecx
    jz      CCreativeObject_SetCurrentAction_SetREADY_Abort

    mov     ecx, 7 // stolen bytes
	ret

CCreativeObject_SetCurrentAction_SetREADY_Abort:
    add     esp, 4
    push   08FA627h // Skip SetSequence()
    ret
}
}