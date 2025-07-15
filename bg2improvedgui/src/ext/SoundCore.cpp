#include "SoundCore.h"

#include "stdafx.h"
#include "sndcore.h"
#include "sndmus.h"
#include "console.h"
#include "chitin.h"
#include "InfGameCommon.h"
#include "hashmap.h"
#include "EngineCommon.h"


char gCastVoiceFilePrefix[5] = {0,0,0,0,0};

BOOL (CSoundMixer::*Tramp_CSoundMixer_InitSonglist)(int, char**) =
    SetFP(static_cast<BOOL (CSoundMixer::*)(int, char**)>       (&CSoundMixer::InitSonglist),   0x9E209B);

BOOL DETOUR_CSoundMixer::DETOUR_InitSonglist(int nSongs, char** pSongFileArray) {
    if (!bSosDriverLoaded) return FALSE;
    if (nNumSongs) return FALSE; //normally an assert
    LPTSTR szSongPath = sSongPath.GetBuffer(0);
#ifndef _DEBUG
    //CSingleLock csl = CSingleLock(&csSos, TRUE);
    CSingleLock csl(&csSos, TRUE);
#endif
    Sos_LoadSongPath(szSongPath, "acm");
    sSongPath.ReleaseBuffer(-1);

    SongResource* pArray = (SongResource*)malloc(nSongs * sizeof(SongResource));
    SongResource** ppArray = (SongResource**)malloc(nSongs * 4);

    for (int i = 0; i < nSongs; i++) {
        ppArray[i] = & pArray[i];
        sprintf_s(pArray[i].path, 256, "%s/%s", (LPCTSTR)sSongPath, pSongFileArray[i]);
    }

    Sos_InitSonglist(ppArray, nSongs);

#ifndef _DEBUG
    csl.Unlock();
#endif

    nNumSongs = nSongs;

    free(pArray);
    free(ppArray);

    return TRUE;
}

extern ResRef gSoundFileName;
extern bool   gTobexSoundCaller;
extern int    gSndChannel;
extern void Normalize(short* PCM, unsigned long Len, short Channels);

void static __stdcall
CResWave_CopyWaveData_Normalize(short* PCM, unsigned long Len, DWORD* Stack, ResWav* ResWave, short nCompressedChannels) {
    bool    enabled = false, wrong = false, exclude = false;
    DWORD*  StackPrev;
    short   Bits;
    short   Channels;
    ResRef  resname = ResWave->pKey->name;

    gSoundFileName = "";

    if (ResWave->bCompressed) {
        Channels = nCompressedChannels;
        Bits = * (short*)((int)ResWave->pData + 0x16);      // WAVC format, bits
        if (Bits != 16)
            wrong = true;
    }
    else {
        Channels = * (short*)((int)ResWave->pData + 0x16);  // WAV format, channels
        Bits =     * (short*)((int)ResWave->pData + 0x22);  // WAV format, bits
        if ( *(short*)((int)ResWave->pData + 0x14) != 0x01 || Bits != 16) // 0x14 codec_id=PCM 16 Signed
            wrong = true;
    }
    
    if (pRuleEx->Normalize_EXCLNORM.m_2da.bLoaded) {
        POINT  pos;
        IECString restring = resname.GetResRefStr();
        if (pRuleEx->Normalize_EXCLNORM.FindString(restring, &pos, FALSE))
            exclude = true;
    }

    if (wrong) {
        console.write_debug("wrong WAV format : %s \n", resname.GetResRefNulled());
        return;
    }

    if (exclude) {
        console.write_debug("exclude from normalize: %s \n", resname.GetResRefNulled());
        return;
    }

    //       CSound::Play(4 args)
    // Normal: CSound::Play->CSound::ExclusivePlay->CopyData->CopyWaveData
    // EAX:    CSound::Play--------------------->CopyDataEAX->CopyWaveData
    if (Stack[1] == 0x9DD5B3 ||                 // CopyData
        Stack[1] == 0x9FA151) {                 // CopyDataEAX
        StackPrev = (DWORD*) Stack[0];
        if (StackPrev[1] == 0x9DDC59 ||         // CSound::ExclusivePlay
            StackPrev[1] == 0x9DE332) {         // CSound::Play for EAX

            char* EBP = (char*) ((DWORD) StackPrev[0]);
            CSound* SoundObj;
            if (StackPrev[1] == 0x9DDC59) {
                SoundObj = (CSound*) *((DWORD*)(EBP - 0x10));   // CSound inside CSound::ExclusivePlay
                gSndChannel = SoundObj->nChannelIdx;
            }

            if (StackPrev[1] != 0x9DE332) {
                StackPrev = (DWORD*) StackPrev[0];
            }

            if (StackPrev[1] == 0x9DE416 ||      // CSound::Play
                StackPrev[1] == 0x9DE332) {      // CSound::Play for EAX   
                StackPrev = (DWORD*) StackPrev[0];

                ////////////////////////////////////////////////////////////////
                // CSound::Play
                if (StackPrev[1] == 0x8A4522 ||     // CGameSprite::PlaySound
                    StackPrev[1] == 0x8A418E ||
                    StackPrev[1] == 0x8A4979) {
                    enabled = true;

                    EBP = (char*) ((DWORD) StackPrev[0]);
                    uchar SoundSetIdx = *(EBP + 8);
                    if (SoundSetIdx == SOUNDSET_SELECT_COMMON)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::Play CRE.SEL_COMMON \t");
                    else
                    if (SoundSetIdx == SOUNDSET_SELECT_ACTION)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::Play CRE.SEL_ACTION \t");
                    else
                    if (SoundSetIdx == SOUNDSET_INITIAL_MEETING)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::Play CRE.INIT_MEET \t");
                    else
                    if (SoundSetIdx == SOUNDSET_DAMAGE)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::Play CRE.DAMAGE \t");                            
                        
                    else
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::Play SoundSet=%d\t", SoundSetIdx);

                } else
                if (StackPrev[1] == 0x92cd59 ||    // CGameSprite::LeaveAreaName
                    StackPrev[1] == 0x92C7AD ||
                    StackPrev[1] == 0x92D0E8 ||
                    StackPrev[1] == 0x92D20E) {
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::LeaveAreaName \t\t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x92B969 ||    // CGameSprite::LeaveArea
                    StackPrev[1] == 0x92BEEE ||
                    StackPrev[1] == 0x92C27D ||
                    StackPrev[1] == 0x92C38E) {
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::LeaveArea \t\t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x5c1d04) {   // CMessageDisplayTextRef::Apply   
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("MessageDisplayText \t\t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x6D1F51) {   // CScreenChapter::TimerAsynchronousUpdate
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("ScreenChapter::TimerAsyncUpdate "); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x4e7004 ||   // CGameDialogEntry::Display
                    StackPrev[1] == 0x4e7745 ||
                    StackPrev[1] == 0x4E78F9 ||
                    StackPrev[1] == 0x4E8229 ) {
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("DialogEntry::Display \t\t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x8A5069) {   // CGameSprite::VerbalConstant
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::VerbalConstant \t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x4A8D93 ||
                    StackPrev[1] == 0x4A9281) {   // CGameAIBase::DisplayStringWait
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("CGameAIBase::DisplayStringWait \t"); 
                        enabled = true;
                } else
                if (StackPrev[1] == 0x43b595)
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("PlayGUISound \t\t\t");
                else
                if (StackPrev[1] == 0x673ee1)
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("SetCursor \t\t\t"); 
                else
                if (StackPrev[1] == 0x74192d)
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("InventoryMsg \t\t\t"); 
                else
                if (StackPrev[1] == 0x57564D)
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("AreaGameSound \t\t\t"); 
                else
                if (StackPrev[1] == 0x9DEAB5) {
                    StackPrev = (DWORD*) StackPrev[0];
                    if (StackPrev[1] == 0x9DF3E3) {
                        StackPrev = (DWORD*) StackPrev[0];
                        if (StackPrev[1] == 0x9E1ABA)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SndMixer::UpdateSoundList1 loop\t");
                        else
                        if (StackPrev[1] == 0x4d1567)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDay loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d1911)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetNight loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d2a68 ||
                            StackPrev[1] == 0x4D262E)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDusk loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d1d7d ||
                            StackPrev[1] == 0x4d21b7)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDawn loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4b8eea ||
                            StackPrev[1] == 0x4b8f1b)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("ApplyWindToAmbients loop \t");

                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetVolume loop \t\t\t"); 
                    }
                    else
                    if (StackPrev[1] == 0x9DEB45) {
                        StackPrev = (DWORD*) StackPrev[0];
                        if (StackPrev[1] == 0x9E1ABA)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SndMixer::UpdateSoundList2 loop\t"); 
                        else
                        if (StackPrev[1] == 0x9e1e79)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("UpdateSoundPositions1 loop \t"); 

                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("ResetVolume1 loop \t\t");
                    }
                    else
                    if (StackPrev[1] == 0x9E1B68)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("SndMixer::UpdateSoundList3 loop\t");
                }
                else
                if (StackPrev[1] == 0x574F85)
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CGameSound::DoAIUpdate \t\t"); 

                else
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("unknow CSound::Play caller %X \t", StackPrev[1]);
                // CSound::Play
                ////////////////////////////////////////////////////////////////

            } else 
            if (StackPrev[1] == 0x9dea2c ||       // CSound::Play_AtCoordinates
                StackPrev[1] == 0x9DE628) {
                StackPrev = (DWORD*) StackPrev[0];
                ////////////////////////////////////////////////////////////////
                // CSound::Play_AtCoordinates
                if (StackPrev[1] == 0x9401DA ) {  // CGameSprite::ApplyCastingEffect
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::ApplyCastingEffect \t");
                    enabled = true;
                } else 
                if (StackPrev[1] == 0x643060 ) {     // CSequenceSoundList::PlaySound
                    enabled = true;
                    StackPrev = (DWORD*) StackPrev[0];
                    if (StackPrev[1] == 0x7f9e46 ) { // CGameAnimationType::PlaySound
                        StackPrev = (DWORD*) StackPrev[0];
                        if (gTobexSoundCaller)
                            gSoundFileName = resname;
                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("Animation::PlayXY \t\t");
                    } else
                    if (StackPrev[1] == 0x8a1ccf)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY 2DA.Batt_Cry \t");
                    else
                    if (StackPrev[1] == 0x8a268e)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY Selection \t");
                    else
                        if (gTobexSoundCaller)
                            gSoundFileName = resname;
                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("unknow SequenceSoundList::PlayXY caller %X \t", StackPrev[1]);
                } else
                if (StackPrev[1] == 0x8a475d) { // CCreatureObject::PlaySound
                    enabled = true;

                    //__asm {int 3}
                    EBP = (char*) ((DWORD) StackPrev[0]);
                    uchar SoundSetIdx = *(EBP + 8);
                    if (SoundSetIdx == SOUNDSET_ATTACK)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.ATTACK \t");
                    else
                    if (SoundSetIdx == SOUNDSET_DAMAGE)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.DAMAGE \t");
                    else
                    if (SoundSetIdx == SOUNDSET_BATTLE_CRY)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.BATT_CRY \t");
                    else
                    if (SoundSetIdx == SOUNDSET_DYING)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.DYING \t");
                    else
                    if (SoundSetIdx == SOUNDSET_SELECT_COMMON)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.SEL_COMMON \t");
                    else
                    if (SoundSetIdx == SOUNDSET_SELECT_ACTION)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.SEL_ACTION \t");
                    else
                    if (SoundSetIdx == SOUNDSET_EXISTANCE)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY CRE.EXISTANCE \t");
                    else

                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("Creature::PlayXY SoundSetLine=%d\t", SoundSetIdx);

                } else
                if (StackPrev[1] == 0x892ff1) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::AIUpdateWalk \t\t");
                } else
                if (StackPrev[1] == 0x892a02) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::AIUpdateWalk \t\t");
                } else 
                if (StackPrev[1] == 0x5079de) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("EffectDamage \t\t\t");
                } else
                if (StackPrev[1] == 0x941efe) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::Swing CRE.Attack\t");
                    enabled = true;
                } else
                if (StackPrev[1] == 0x895571) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::ArmorSound \t\t");
                } else
                if (StackPrev[1] == 0x8ADB81) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::ArmorSound \t\t");
                } else
                if (StackPrev[1] == 0x8ADF84) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::Death/Fall \t\t");
                    enabled = true;
                } else
                if (StackPrev[1] == 0x4FFF54) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CGameEffect::PlaySound \t\t");
                } else
                if (StackPrev[1] == 0x9412F8) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::ApplyCastingEffPost \t");
                } else
                if (StackPrev[1] == 0x64ec3e) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("VEFVidCell::PlaySound \t\t");
                } else
                if (StackPrev[1] == 0x4f08eb ||
                    StackPrev[1] == 0x4F1501) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CreakingDoor \t\t\t");
                } else
                if (StackPrev[1] == 0x60416c) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Projectile \t\t\t");
                } else
                if (StackPrev[1] == 0x54528e) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("EffectDeath \t\t\t");
                } else
                if (StackPrev[1] == 0x8AE35F ||
                    StackPrev[1] == 0x887A20) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::SetSequence READY \t");
                } else
                if (StackPrev[1] == 0x496A74) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Script PlaySound() \t\t");
                } else
                if (StackPrev[1] == 0x5CC48A) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("MessagePlaySound \t\t");
                } else
                if (StackPrev[1] == 0x56A067) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CGameFireball3d \t\t");
                } else
                if (StackPrev[1] == 0x8BD8B1) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CGameTemporal \t\t\t");
                } else
                if (StackPrev[1] == 0x9DEAA9) {
                    StackPrev = (DWORD*) StackPrev[0];
                    if (StackPrev[1] == 0x9DF3E3) {
                        StackPrev = (DWORD*) StackPrev[0];
                        if (StackPrev[1] == 0x9E1ABA)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SndMixer::UpdateSoundList4 loop\t");
                        else
                        if (StackPrev[1] == 0x4d1567)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDay loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d1911)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetNight loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d2a68 ||
                            StackPrev[1] == 0x4D262E)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDusk loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4d1d7d ||
                            StackPrev[1] == 0x4d21b7)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetDawn loop \t\t\t");
                        else
                        if (StackPrev[1] == 0x4b8eea ||
                            StackPrev[1] == 0x4b8f1b)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("ApplyWindToAmbients loop \t");

                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SetVolume loop \t\t\t"); 
                    }
                    else
                    if (StackPrev[1] == 0x9DEB45) {
                        StackPrev = (DWORD*) StackPrev[0];
                        if (StackPrev[1] == 0x9E1ABA)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("SndMixer::UpdateSoundList5 loop\t"); 
                        else
                        if (StackPrev[1] == 0x9e1e79)
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("UpdateSoundPositions2 loop \t"); 

                        else
                            if (pGameOptionsEx->bSound_NormalizePrintResname)
                                console.write_debug("ResetVolume2 loop \t\t");
                    }
                    else
                    if (StackPrev[1] == 0x9E1B68)
                        if (pGameOptionsEx->bSound_NormalizePrintResname)
                            console.write_debug("SndMixer::UpdateSoundList6 loop\t");
                } else
                if (StackPrev[1] == 0x91EF7A) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("JumpToPoint \t\t\t");
                } else
                if (StackPrev[1] == 0x574FB0 ||
                    StackPrev[1] == 0x57566F) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("CGameSound::DoAIUpdate \t\t");
                } else
                if (StackPrev[1] == 0x8B4D3A) {
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("Creature::UpdateSpriteEffect \t");
                }                

                else
                // default
                if (gTobexSoundCaller)
                    gSoundFileName = resname;
                else
                    if (pGameOptionsEx->bSound_NormalizePrintResname)
                        console.write_debug("unknow CSound::Play_At_XY caller %X \t", StackPrev[1]);
                // CSound::Play_AtCoordinates
                ////////////////////////////////////////////////////////////////
            }
        }
    }

    if (!pGameOptionsEx->bSound_NormalizeCreOnly) {  // Creatures only
        enabled = true;
    }

    #ifndef _DEBUG
        gSoundFileName.Clean(); // for Release disable post-poned print 
    #endif

    if (pGameOptionsEx->bSound_NormalizePrintResname) {
        if (gSoundFileName.IsEmpty()) {
            console.writef("sound: %s", resname.GetResRefNulled());
            console.write_debug("\t ch:%d", gSndChannel);
        }
    }

    if (enabled)
        Normalize(PCM, Len, Channels);

    if (pGameOptionsEx->bSound_NormalizePrintResname) {
        if (gSoundFileName.IsEmpty())
            console.writef("\n");
    }
}


hashmap* gHash_BGSNDSET = NULL;

#define SndSet_BG1TYPE 1
#define SndSet_BG2TYPE 2
#define SndSet_MixTYPE 3
#define SndSet_MaxTYPE 4


void
BGSNDSET_InitHash(CRuleTable& Table) {
    gHash_BGSNDSET = hashmap_create();
    for (int i = 0; i < Table.nRows; i++) {
        IECString sName  = Table.pRowHeaderArray[i];
        IECString* sValue = Table.pDataArray + (Table.nCols * i + 0);   // data column #0
        hashmap_set(gHash_BGSNDSET, (LPTSTR)sName, sName.GetLength(), (uintptr_t)sValue);
    }
}


void
BGSNDSET_DestroyHash() {
    if (gHash_BGSNDSET)
        hashmap_free(gHash_BGSNDSET);
}


/*
       List Policy:
        BG1 -> BG1
        BG2 -> BG2
        Mix -> Mix
    default -> BG2
*/

int
GetSoundSetType(CCreatureObject& Cre) {
    //bool PureBG1part = IsBG1Part();

    if (pRuleEx->SoundSet_BGSNDSET.m_2da.bLoaded) {
        IECString sName;

        if (Cre.rSaveName.GetResRefNulled()[0] == '*') { // first char
            sName.Format("%c%s", Cre.cFirstResSlot, Cre.rSaveName.GetResRefNulled());
            sName.Remove('*');
        }
        else
            sName.Format("%s", Cre.rSaveName.GetResRefNulled());
        
        //console.write_debug("Cre FileName %s \n", (LPTSTR)sName);

        //if (Cre.statistics.nJoinPartyTime != 0) { // not main char
            IECString* sValue;
            if (hashmap_get(gHash_BGSNDSET, (LPTSTR)sName, sName.GetLength(), (uintptr_t*)&sValue)) {
                if (strcmp((LPCTSTR)*sValue, "BG1") == 0)
                    return SndSet_BG1TYPE;
                else
                if (strcmp((LPCTSTR)*sValue, "BG2") == 0)
                    return SndSet_BG2TYPE;
                else
                if (strcmp((LPCTSTR)*sValue, "Mix") == 0)
                    return SndSet_MixTYPE;
                else
                if (strcmp((LPCTSTR)*sValue, "Max") == 0)
                    return SndSet_MaxTYPE;
                else
                    return SndSet_BG2TYPE;  // unexpected value, , set BG2

            } else              // not found in BGSNDSET.2DA, set BG2
                return SndSet_BG2TYPE;      

    } else {                    // no BGSNDSET.2DA, set BG2
        return SndSet_BG2TYPE;
    }
}


BOOL static __stdcall
CCreatureObject_PlaySound_COMMON(CCreatureObject& Cre, CStrRef** pCStrRef, int* pIndex, CStrRef& StrRefObj) {
    int SoundSetType = GetSoundSetType(Cre);
    
    //console.write_debug("COMMON Cre SoundSetType %d \n", SoundSetType);

    if (SoundSetType != SndSet_BG2TYPE) {
        // BG1/Mix/Max
        if (SoundSetType != SndSet_MaxTYPE &&               // bg1/mix common
            Cre.nSelectionCountCommonRareCounter <= 8) {    // rare frequence
                Cre.nSelectionCountCommonRareCounter++;
                return  FALSE;
        } else                                              // max common
        if (SoundSetType == SndSet_MaxTYPE &&
            Cre.nSelectionCountCommonRareCounter <= 11) {   // rare frequence
                ushort  availablesounds, available_ext_sounds;
                int     offset;

                availablesounds =      Cre.GetNumSounds(26, 6); // orig offsets
                available_ext_sounds = Cre.GetNumSounds(95, 5); // ext offsets
                if (availablesounds == 6 && // avoid gap in orig entries
                    available_ext_sounds > 0) {
                    int curslot = Cre.nSelectionCountCommon % (availablesounds + available_ext_sounds);

                    if (curslot <= 5)
                        offset = 26 + curslot;       // orig slots
                    else
                        offset = 95 + (curslot - 6); // new slots
            
                    STRREF ref = Cre.BaseStats.soundset[offset];
                    g_pChitin->m_TlkTbl.GetTlkString(ref, StrRefObj);
           
                    *pCStrRef = &StrRefObj;
                    *pIndex = offset; // npc slots ?
                    Cre.nSelectionCountCommonRareCounter++;

                    return TRUE;
                } else 
                    return FALSE;   // no sounds
        } else {                                        // bg1/mix/max rare 
            ushort  availablesounds;
            int     offset;

            if (SoundSetType == SndSet_BG1TYPE) {
                offset = 35;                                    // 35(0x130) bg1_select_rare
                availablesounds = Cre.GetNumSounds(offset, 4);  // 4 - max slots
            } else
            if  (SoundSetType == SndSet_MixTYPE ||
                 SoundSetType == SndSet_MaxTYPE) {
                offset = 75;                                    // 75(0x1d0) bg2ee_select_rare
                availablesounds = Cre.GetNumSounds(offset, 4);  // 4 - max slots
            }

            if (availablesounds > 0) {
                int curslot;
                if (SoundSetType == SndSet_MaxTYPE)
                    curslot = (Cre.nSelectionCountCommonRareCounter - 12) % availablesounds; // 12-15
                else
                    curslot = (Cre.nSelectionCountCommonRareCounter - 9)  % availablesounds; // 9-12

                STRREF ref = Cre.BaseStats.soundset[offset + curslot];
                g_pChitin->m_TlkTbl.GetTlkString(ref, StrRefObj);
            
                *pCStrRef = &StrRefObj;
                *pIndex = offset + curslot; // npc slots ?
                Cre.nSelectionCountCommonRareCounter++;

                if (SoundSetType != SndSet_MaxTYPE) {
                    if ( (Cre.nSelectionCountCommonRareCounter - 9) >= availablesounds ) {
                        Cre.nSelectionCountCommonRareCounter = 0;
                        Cre.nSelectionCountCommon -= availablesounds; // compensating rare clicks
                    }
                } else
                    if ( (Cre.nSelectionCountCommonRareCounter - 12) >= availablesounds ) {
                        Cre.nSelectionCountCommonRareCounter = 0;
                        Cre.nSelectionCountCommon -= availablesounds; // compensating rare clicks
                    }

                return TRUE;
            } else 
                return FALSE;   // no sounds
        }
    } else
        // pure BG2
        return FALSE;
}


BOOL static __stdcall
CCreatureObject_PlaySound_ACTION(CCreatureObject& Cre, CStrRef** pCStrRef, int* pIndex, CStrRef& StrRefObj) {
    int SoundSetType = GetSoundSetType(Cre);
    
    //console.write_debug("ACTION Cre SoundSetType %d \n", SoundSetType);

    if (SoundSetType == SndSet_MaxTYPE) {
        ushort  availablesounds, available_ext_sounds;
        int     offset;

        availablesounds =      Cre.GetNumSounds(32, 7); // orig offsets
        available_ext_sounds = Cre.GetNumSounds(39, 5); // ext offsets
        if (availablesounds == 7 &&
            available_ext_sounds > 0) {
            int curslot = Cre.nSelectionCountAction % (availablesounds + available_ext_sounds);

            if (curslot <= 6)
                offset = 32 + curslot;       // orig slots
            else
                offset = 39 + (curslot - 7); // new slots

            STRREF ref = Cre.BaseStats.soundset[offset];
            g_pChitin->m_TlkTbl.GetTlkString(ref, StrRefObj);
            
            *pCStrRef = &StrRefObj;
            *pIndex = offset; // npc slots ?

            return TRUE;
        } else
            return FALSE;   // no ext sounds
    } else
        // pure BG2 type
        return FALSE;
}


void static __stdcall
CCreatureObject_PlaySound_ACTION2(CCreatureObject& Cre) {
    ushort availablesounds = Cre.GetNumSounds(32, 7); // action offset
    if (availablesounds > 0) {
        Cre.u6529 = 0x55;
    }
}


int static __stdcall
CGameSprite_VerbalConstant(CCreatureObject& Cre, int Slot) {
    if (GetSoundSetType(Cre) == SndSet_MaxTYPE) {
        if (Slot == 2)  // HAPPY
            return (IERand(2) == 1) ? 80 : Slot; // HAPPY2
        if (Slot == 3)  // UNHAPPY_ANNOYED
            return (IERand(2) == 1) ? 81 : Slot; // UNHAPPY_ANNOYED2
        if (Slot == 4)  // SOUNDSET_UNHAPPY_SERIOUS
            return (IERand(2) == 1) ? 82 : Slot; // SOUNDSET_UNHAPPY_SERIOUS2
        if (Slot == 5)  // SOUNDSET_UNHAPPY_BREAKINGPOIINT
            return (IERand(2) == 1) ? 83 : Slot; // SOUNDSET_UNHAPPY_BREAKINGPOIINT2
        if (Slot == 21)  // SOUNDSET_AREA_FOREST
            return (IERand(2) == 1) ? 90 : Slot; // SOUNDSET_AREA_FOREST2
        if (Slot == 22)  // SOUNDSET_AREA_CITY
            return (IERand(2) == 1) ? 91 : Slot; // SOUNDSET_AREA_CITY2
        if (Slot == 23)  // SOUNDSET_AREA_DUNGEON
            return (IERand(2) == 1) ? 92 : Slot; // SOUNDSET_AREA_DUNGEON2
        if (Slot == 24)  // SOUNDSET_AREA_DAY
            return (IERand(2) == 1) ? 93 : Slot; // SOUNDSET_AREA_DAY2
        if (Slot == 25)  // SOUNDSET_AREA_NIGHT
            return (IERand(2) == 1) ? 94 : Slot; // SOUNDSET_AREA_NIGHT2

        return Slot;    // passthrough
    } else {
        return Slot;    // passthrough
    }

}


BOOL static __stdcall
CCreatureObject_PlaySound_MAXTYPE(
        int              ChannelMode,
        uchar            ChannelNum,
        int              Ext_offset,
        int              Orig_offset,
        CCreatureObject& Cre,
        CStrRef**        pCStrRef,
        int*             pIndex,
        CStrRef*         StrRefObj)
{
    int SoundSetType = GetSoundSetType(Cre);
    
    //console.write_debug("MAXTYPE Cre SoundSetType %d \n", SoundSetType);

    if (SoundSetType == SndSet_MaxTYPE) {
        ushort  availablesounds, available_ext_sounds;
        int     offset;

        availablesounds =      Cre.GetNumSounds(Orig_offset, 1);
        available_ext_sounds = Cre.GetNumSounds(Ext_offset, 1);
        if (availablesounds || available_ext_sounds) {
            if (available_ext_sounds)
                offset = (IERand(2) == 1) ? Ext_offset : Orig_offset; // 0-1 random
            else
                offset = Orig_offset;

            STRREF ref = Cre.BaseStats.soundset[offset];
            *pIndex = offset; // npc slots ?
            g_pChitin->m_TlkTbl.GetTlkString(ref, *StrRefObj);

            if (ChannelMode)
                if (ChannelNum == 13)
                  StrRefObj->sound.SetChannel(ChannelNum, Cre.pArea);
                else
                  StrRefObj->sound.SetChannel(ChannelNum, 0);
            else
                StrRefObj->sound.SetChannel(ChannelNum, Cre.pArea);

            *pCStrRef = StrRefObj;

            return TRUE;    // 0x8A14CC
        } else  { // no sounds
            //delete StrRefObj;
            return FALSE;   // 0x8A3DCA
        }
    } else {
        return 2;   // no MAXtype
    }
}


BOOL static __stdcall
CCreatureObject_PlaySound_LimitActionSoundToBG1(CCreatureObject& Cre, ushort availablesounds) {
    if (GetSoundSetType(Cre) == SndSet_BG1TYPE)
        return min(3, availablesounds);   // limit to 3 Action Sounds
    else
        return availablesounds;
}


DWORD gDirectSoundBuffer_GetStatus_Result;

void static __stdcall
CSound_Stop_Logging(CSound& snd) {
    //if (snd.m_pSoundBuffer)
    //    console.write_debug("CSound::Remove sound: %s\t ch:%d\n", snd.wav.soundName.GetResRefNulled(), snd.nChannelIdx);

    if (snd.wav.pResWav) {
        if (gDirectSoundBuffer_GetStatus_Result & 1) // DSBSTATUS_PLAYING 
            console.write_debug("CSound::Abort................\tsound: %s\t ch:%d\n", snd.wav.soundName.GetResRefNulled(), snd.nChannelIdx);
        else
            //console.write_debug("CSound::Stop empty........\tsound: %s\t ch:%d\n", snd.wav.soundName.GetResRefNulled(), snd.nChannelIdx);
            ;
    }
}


void static __stdcall
CSoundMixer_ClearChannel_Logging(int nChannel) {
    console.write_debug("SoundMixer::ClearChannel.....\t\t\t ch:%d\n", nChannel);
}


extern double gNormalizeAmplLastValue;

void static __stdcall
CGameSprite_DecodeSwingSound_DisableExclusive(
                            CCreatureObject& Cre, 
                            BOOL *bContinue,
                            IECString& sFilename,
                            int rand_char) {
    CSound Sound;

    ///////////////////
    // orig code
    switch (rand_char) {
    case 0:
        sFilename += "A";
        break;
    case 1:
        sFilename += "B";
        break;
    case 2:
        sFilename += "C";
        break;
    case 3:
        sFilename += "D";
        break;
    default:
        break;
    }
    ///////////////////

    if (!sFilename.IsEmpty()) {
        ResRef rFilename = sFilename;

        Res *ResObj = g_pChitin->m_ResHandler.GetResObject(rFilename, 4, 1);
        if (ResObj) {
            Sound.wav.pResWav = (ResWav*) ResObj;
            Sound.wav.bLoaded = TRUE;
            ResObj->AddToHandler();
            Sound.wav.soundName = rFilename;

        } else {
            Sound.wav.pResWav = NULL;
            Sound.wav.soundName.Clean();
        }

        Sound.SetChannel(3, Cre.pArea);
        Sound.nFrequencyShift = 5;
        Sound.nVolumeShift = 20;
        if (!Sound.bLoop)
            Sound.SetFireForget(1);

        gTobexSoundCaller = true;
        gNormalizeAmplLastValue = 0;
        gSoundFileName = "";
        Sound.PlayAtCoord(Cre.currentLoc.x, Cre.currentLoc.y, 0, 0);

        if (!gSoundFileName.IsEmpty()) {
            if (pGameOptionsEx->bSound_NormalizePrintResname) {
                console.write_debug("Creature::Swing Weapon \t\tsound: %s", gSoundFileName.GetResRefNulled());
                console.write_debug("\t ch:%d", gSndChannel);
                if (gNormalizeAmplLastValue)
                    console.write_debug("\t +%.3f dB \n", gNormalizeAmplLastValue);
                else
                    console.write_debug("\n");
            }
        }

        gSoundFileName = "";
        gNormalizeAmplLastValue = 0;
        gTobexSoundCaller = false;

        *bContinue=1;
    }
}


void static __stdcall
CGameSprite_ApplyCastingEffect_PatchFilePrefix(CCreatureObject& Cre, ResSplContainer& Spl, SplFileAbility& ability) {
    bool  highlev = false;
    uchar Gender  = Cre.o.Gender;

    if ( (Spl.GetSpellType() == SPELLTYPE_MAGE   && Spl.GetSpellLevel() >= 9) ||     // Wiz    Lev 9 + HLA
         (Spl.GetSpellType() == SPELLTYPE_PRIEST && Spl.GetSpellLevel() >= 7)        // Priest Lev 7 + HLA
       ) {
	    //if (pRuleEx->m_HideSpell.m_2da.bLoaded) {
	    //    if (pRuleEx->m_HideSpell.nCols && pRuleEx->m_HideSpell.nRows) {
		   //     for (int i = 0; i < pRuleEx->m_HideSpell.nRows; i++) {
			  //      IECString* sSpellHide = pRuleEx->m_HideSpell.pRowHeaderArray + i;
			  //      if (Spl.name == *sSpellHide) {
				        highlev = true;
				    //    break;
			     //   }
		      //  }
	       // }
        //}
    }

    // CHA_SM = CHA_ + Gender + MagePriest
    if (pGameOptionsEx->bSound_NWNCastingSound &&
        highlev && 
        (Gender == 1 || Gender == 2) &&                                 // Male or Female
        ((ability.castSpeed - Cre.GetDerivedStats().mentalSpeed) >= 3)  // Enough casting time
        ) {
            memcpy(gCastVoiceFilePrefix, "NWN_", 4);     // NWN Human Soundset
    }
    else {
        if (pGameOptionsEx->bSound_BG1CastingSound_Level) {
            short SpellLevel = Spl.GetSpellLevel();
            if (Spl.GetSpellType() == SPELLTYPE_INNATE &&
                pGameOptionsEx->bSound_BG2ClearCastingSound)
                memcpy(gCastVoiceFilePrefix, "BG2C", 4); // BG2 Clean Soundset for Innate
            else {
                if (SpellLevel < 6)
                    memcpy(gCastVoiceFilePrefix, "BG1_", 4);
                else
                    memcpy(gCastVoiceFilePrefix, "CHA_", 4);
            }
        } else {
            if (IsBG1Part())
                // BG2 Clean Soundset for Innate not available in pure BG1 experience
                memcpy(gCastVoiceFilePrefix, "BG1_", 4);
            else
                if (Spl.GetSpellType() == SPELLTYPE_INNATE &&
                    pGameOptionsEx->bSound_BG2ClearCastingSound)
                    memcpy(gCastVoiceFilePrefix, "BG2C", 4); // BG2 Clean Soundset for Innate
                else
                    memcpy(gCastVoiceFilePrefix, "CHA_", 4);
        }
    }
}


uint static __stdcall
CGameSprite_ApplyCastingEffectPost_PatchAnimId(CCreatureObject& Cre, ResSplContainer& Spl, IECString& sPrefix, uint AnimID) {
    if (strcmp(gCastVoiceFilePrefix, "NWN_") == 0) {
        int random = IERand(5); // 0-4

        if (AnimID == 0)    // Necromancy NWN_**06.wav
            return AnimID = 6;

        else
        switch (random) { // 01234567 -> 43572601, see 0x93FEDC
        case 0:
            AnimID = 4;
            break;
        case 1:
            AnimID = 3;
            break;
        case 2:
            AnimID = 5;
            break;
        case 3:
            AnimID = 7;
            break;
        case 4:
            AnimID = 2;
            break;
        default:
            AnimID = 4;
            break;
        }
    }

    return AnimID;
}

bool 
IsStandartCastSound(ResRef* filename) {
    filename->MakeUpper();

    if (!strncmp((char*)filename, "CAS_M01", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M02", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M03", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M04", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M05", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M06", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M07", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_M08", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P01", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P02", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P03", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P04", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P05", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P06", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P07", 8))
        return true;
    if (!strncmp((char*)filename, "CAS_P08", 8))
        return true;

    return false;
}


void static __stdcall
CGameSprite_ApplyCastingEffectPost_PatchFilePrefix(CCreatureObject& Cre, ResSplContainer& Spl, ResRef* filename) {
    // CAS_***
    if (pGameOptionsEx->bSound_BG1CastingSound_Level) {
        short SpellLevel = Spl.GetSpellLevel();
        if (SpellLevel < 6)
            if (IsStandartCastSound(filename))
                memcpy(filename, "BG1_", 4);
    } else {
        if (IsBG1Part())
            if (IsStandartCastSound(filename))
                memcpy(filename, "BG1_", 4);
    }
}


void
PlayToggleSound(bool On_Off) {
    if (g_pChitin->pEngineActive) {
        if (On_Off == true) 
            g_pChitin->pEngineActive->PlayGUISound(ResRef("KEYTOGON"));
        else
            g_pChitin->pEngineActive->PlayGUISound(ResRef("KEYTOGOF"));
    }
}


//void static __stdcall
//CSoundMixer_CleanUp_Log() {
//    console.write_debug("CSoundMixer_CleanUp \n");
//}
//
//void static __stdcall
//CSoundMixer_Initialize_Log() {
//    console.write_debug("CSoundMixer_Initialize \n");
//}
//
//void static __stdcall
//CGameArea_OnActivation_Log() {
//    console.write_debug("CGameArea_OnActivation \n");
//}
//
//void static __stdcall
//CCacheStatus_Update_Log() {
//    console.write_debug("CCacheStatus_Update \n");
//}
//
//void static __stdcall
//CSound_SetVolume_Log2() {
//    console.write_debug("CSound_SetVolume2 \n");
//}
//
//void static __stdcall
//CSound_ResetVolume_Log2() {
//    console.write_debug("CSound_ResetVolume2 \n");
//}


void static __stdcall
CInfGame_LoadGame_ReActivateArea(CInfGame& Game) {
    if (!g_pChitin->m_mixer.IsChannelUsed(1)) { // if ambient channel is empty
        //console.write_debug("ReActivate Ambient\n");
        Game.m_pLoadedAreas[Game.m_VisibleAreaIdx]->OnDeactivation();
        Game.m_pLoadedAreas[Game.m_VisibleAreaIdx]->OnActivation();
    } else
        console.write_debug("ReActivate Ambient failed, channel used \n");
}


void __declspec(naked)
CResWave_CopyWaveData_Normalize_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-10h]       // nCompressedChannels
    push    [ebp-28h]       // ResWave
    push    ebp             // ret adr
    mov     eax, [ebp-28h]
    mov     eax, [eax+58h]
    push    eax             // Length in Bytes
    push    [ebp+8]         // PCM Bufer
    call    CResWave_CopyWaveData_Normalize

    pop     edx
    pop     ecx

    mov     eax, 1      // stolen bytes
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_COMMON_asm() {
__asm
{
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    call    CCreatureObject_PlaySound_COMMON

    pop     edx
    pop     ecx

    test    eax, eax
    jz      CCreatureObject_PlaySound_COMMON_asm_BG2

    add     esp, 4
    push    08A26DFh    // skip orig code
    ret

CCreatureObject_PlaySound_COMMON_asm_BG2:
    movsx   eax, word ptr [ebp-90h]      // stolen bytes
    ret

}
}


void __declspec(naked)
CCreatureObject_PlaySound_MORALE_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    1               // orig offset
    push    79              // extended offset
    push    [ebp-98h]       // Channel
    push    1               // Variable channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_HAPPY_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    2               // orig offset
    push    80              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_UNHAPPYANNOYED_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    3               // orig offset
    push    81              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_UNHAPPYSERIOUS_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    4               // orig offset
    push    82              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_UNHAPPYBREAKINGPOINT_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    5               // orig offset
    push    83              // extended offset
    push    [ebp-98h]       // Channel
    push    1               // Variable channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_LEADER_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    6               // orig offset
    push    84              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_TIRED_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    7               // orig offset
    push    85              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_BORED_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    8               // orig offset
    push    86              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_DAMAGE_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    18              // orig offset
    push    87              // extended offset
    push    [ebp-98h]       // Channel
    push    1               // Variable channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_DYING_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    19              // orig offset
    push    88              // extended offset
    push    [ebp-98h]       // Channel
    push    1               // Variable channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_HURT_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    20              // orig offset
    push    89              // extended offset
    push    [ebp-98h]       // Channel
    push    1               // Variable channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_AREA_FOREST_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    21              // orig offset
    push    90              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_AREA_CITY_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    22              // orig offset
    push    91              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_AREA_DUNGEON_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    23              // orig offset
    push    92              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_AREA_DAY_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    24              // orig offset
    push    93              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_AREA_NIGHT_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    push    25              // orig offset
    push    94              // extended offset
    push    [ebp-98h]       // Channel
    push    0               // Area channel
    call    CCreatureObject_PlaySound_MAXTYPE

    pop     edx
    pop     ecx

    cmp     eax, 2
    jz      asm_NoType

    test    eax, eax
    jz      asm_NoSound

    pop     eax
    add     esp, 4
    push    08A3DCAh    // no sound
    ret

asm_NoSound:
    pop     eax
    add     esp, 4
    push    08A14CCh    // play sound
    ret

asm_NoType:
    pop     eax
    mov     [ebp-0A4h], ax
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_ACTION_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    // limit BG1 to 3 sounds
    push    eax             // ACTION sound count
    push    [ebp-218h]      // Cre
    call    CCreatureObject_PlaySound_LimitActionSoundToBG1
    // result in ax
    mov     [ebp-90h], ax   // stolen bytes

    // play Max slots
    lea     eax, [ebp-80h]  // StrRefObj
    push    eax             
    lea     eax, [ebp-8Ch]  // selection + base
    push    eax
    lea     eax, [ebp-94h]  // pCStrRef
    push    eax
    push    [ebp-218h]      // Cre
    call    CCreatureObject_PlaySound_ACTION

    test    eax, eax
    pop     edx
    pop     ecx
    pop     eax

    jz      CCreatureObject_PlaySound_ACTION_asm_BG2

    add     esp, 4
    push    08A2A5Eh    // skip orig code
    ret

CCreatureObject_PlaySound_ACTION_asm_BG2:
    ret
}
}


void __declspec(naked)
CCreatureObject_PlaySound_ACTION2_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-218h]      // Cre
    call    CCreatureObject_PlaySound_ACTION2

    pop     edx
    pop     ecx

    mov     eax, [ebp-218h] // stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_VerbalConstant_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp+8]     // Sound slot
    push    [ebp-1F4h]  // Cre
    call    CGameSprite_VerbalConstant
    // eax - return slot
    pop     edx
    pop     ecx

    mov     ecx, eax    // stolen bytes
    mov     edx, [ebp-1F4h]
    ret
}
}


//void __declspec(naked)
//CCreatureObject_PlaySound_LimitActionSoundToBG1_asm() {
//__asm
//{
//    push    ecx
//    push    edx
//
//    push    eax
//    push    [ebp-218h]      // Cre
//    call    CCreatureObject_PlaySound_LimitActionSoundToBG1
//    // result in ax
//
//    pop     edx
//    pop     ecx
//
//    mov     [ebp-90h], ax      // stolen bytes
//    ret
//}
//}


void __declspec(naked)
CSound_Stop_Logging_asm() {
__asm
{
    mov     eax, [ebp-8]      // stolen bytes
    mov     ecx, [eax+58h]
    ; ecx     // m_pSoundBuffer
    ; eax     // CSound

    push    ecx
    push    edx
    push    eax

    lea     eax, gDirectSoundBuffer_GetStatus_Result
    push    eax
    push    ecx     // IDirectSoundBuffer
    mov     ecx, [ecx]
    call    dword ptr [ecx+24h] //IDirectSoundBuffer::Status

    pop     eax
    push    eax

    push    eax // CSound
    call    CSound_Stop_Logging

    pop     eax
    pop     edx
    pop     ecx
    ret
}
}


void __declspec(naked)
CSound_Stop_Logging2_asm() {
__asm
{
    push    ecx
    push    edx
    push    eax

    push    [ebp-8] // CSound
    call    CSound_Stop_Logging

    pop     eax
    pop     edx
    pop     ecx

    and     edx, 0FFh   // Stolen bytes
    ret
}
}


void __declspec(naked)
CSoundMixer_ClearChannel_Logging_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp+8]     // nChannel
    call    CSoundMixer_ClearChannel_Logging

    pop     edx
    pop     ecx

    add     ecx, 0A4h      // stolen bytes
    ret
}
}


void __declspec(naked)
CGameSprite_DecodeSwingSound_DisableExclusive_asm()
{
__asm {
	push    ecx
	push    edx

    push    [ebp-144h]      // random letter 0-4
    lea     eax, [ebp-88h]  // SoundName
    push    eax
    lea     eax, [ebp-18h]
    push    eax             // bFound
    push    [ebp-13Ch]      // Cre
	call    CGameSprite_DecodeSwingSound_DisableExclusive

	pop     edx
	pop     ecx

    add     esp, 4
    push    0941C01h        // look for cre/2da.attack*
	ret     
}
}


void __declspec(naked)
CGameSprite_ApplyCastingEffect_PatchFilePrefix_asm()
{
__asm {
	push    ecx
	push    edx

    push    [ebp+0Ch]       // SplFileAbility
    push    [ebp+8]         // CSpell
    push    [ebp-1F4h]      // Cre
	call    CGameSprite_ApplyCastingEffect_PatchFilePrefix

	pop     edx
	pop     ecx

    mov     dword ptr [ebp-4Ch], 0  // stolen bytes
	ret     
}
}


void __declspec(naked)
CGameSprite_ApplyCastingEffectPost_PatchFilePrefix_asm()
{
__asm {
    push    eax
	push    ecx
	push    edx

    push    eax         // filename resref
    push    [ebp+8]     // CSpell
    push    [ebp-6Ch]   // Cre
	call    CGameSprite_ApplyCastingEffectPost_PatchFilePrefix

	pop     edx
	pop     ecx
    pop     eax

    mov     ecx, [eax]  // stolen bytes
    mov     edx, [eax+4]
	ret     
}
}


void __declspec(naked)
CGameSprite_ApplyCastingEffect_PatchAnimId_asm()
{
__asm {
	push    ecx
	push    edx

    push    eax         // AnimID
    lea     eax, [ebp-48h]
    push    eax         // CString Prefix
    push    [ebp+8]     // CSpell
    push    [ebp-1F4h]  // Cre
	call    CGameSprite_ApplyCastingEffectPost_PatchAnimId

	pop     edx
	pop     ecx

    mov     [ebp-208h], eax  // stolen bytes
	ret     
}
}


//void __declspec(naked)
//CSoundMixer_CleanUp_Log_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CSoundMixer_CleanUp_Log
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    mov     dword ptr [eax+5Ch], 0 // stolen bytes
//	ret     
//}
//}
//
//
//void __declspec(naked)
//CSoundMixer_Initialize_Log_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CSoundMixer_Initialize_Log
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    mov     dword ptr [ecx+5Ch], 1 // stolen bytes
//	ret     
//}
//}


//void __declspec(naked)
//CGameArea_OnActivation_Log_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CGameArea_OnActivation_Log
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    mov     [ecx+10Ch], edx // stolen bytes
//	ret     
//}
//}


//void __declspec(naked)
//CCacheStatus_Update_Log_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CCacheStatus_Update_Log
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    mov     [eax+10Ch], ecx // stolen bytes
//	ret     
//}
//}
//
//
//void __declspec(naked)
//CSound_SetVolume_Log2_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CSound_SetVolume_Log2
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    mov     eax, [ebp-0Ch] // stolen bytes
//    cmp     dword ptr [eax+38h], 0
//	ret     
//}
//}
//
//
//void __declspec(naked)
//CSound_ResetVolume_Log2_asm()
//{
//__asm {
//	push    ecx
//	push    edx
//    push    eax
//
//    call    CSound_ResetVolume_Log2
//   
//    pop     eax
//	pop     edx
//	pop     ecx
//
//    and     ecx, 0FFh // stolen bytes
//	ret     
//}
//}


void __declspec(naked)
CInfGame_LoadGame_ReActivateArea_asm()
{
__asm {
    push    ecx
    push    edx
    push    eax

    push    [ebp-150h]
    call    CInfGame_LoadGame_ReActivateArea
   
    pop     eax
    pop     edx
    pop     ecx

    mov     dword ptr [ebp-28h], 0 // stolen bytes
    ret     
}
}


void __declspec(naked)
CGameDialogEntry_Display_ClearChannel_asm()
{
__asm {
    push    ecx
    push    edx
    push    eax

    push    6                   // dialog channel
    mov     ecx, 0xB8CF64       // g_pSoundMixer
    mov     ecx, [ecx]
    mov     eax, 0x9E0EB8       // CSoundMixer::ClearChannel
    call    eax
   
    pop     eax
    pop     edx
    pop     ecx

    mov     edx, [ebp-288h] // stolen bytes
    ret     
}
}


void __declspec(naked)
CScreenWorld_EndDialog_ClearChannel_asm()
{
__asm {
    push    ecx
    push    edx
    push    eax

    push    6                   // dialog channel
    mov     ecx, 0xB8CF64       // g_pSoundMixer
    mov     ecx, [ecx]
    mov     eax, 0x9E0EB8       // CSoundMixer::ClearChannel
    call    eax
   
    pop     eax
    pop     edx
    pop     ecx

    mov     ecx, [ebp-204h] // stolen bytes
    ret     
}
}
