#include "sndcore.h"

BOOL (CSoundMixer::*CSoundMixer_InitSonglist)(int, char**) =
	SetFP(static_cast<BOOL (CSoundMixer::*)(int, char**)>		(&CSoundMixer::InitSonglist),	0x9E209B);

BOOL CSoundMixer::InitSonglist(int nSongs, char** pSongFileArray) { return (this->*CSoundMixer_InitSonglist)(nSongs, pSongFileArray); }

_n int CSound::SetFireForget(BOOL)               { _bgmain(0x9DD498) }
_n int CSound::SetChannel(int, CArea*)           { _bgmain(0x9DF0A2) }
_n BOOL CSound::Play(BOOL)                       { _bgmain(0x9DE123) }
_n BOOL CSound::Stop()                           { _bgmain(0x9DF4CC) }
_n BOOL CSound::PlayAtCoord(int, int, int, BOOL) { _bgmain(0x9DE433) }
_n BOOL CSound::IsSoundPlaying(bool)             { _bgmain(0x9DE091) }
_n BOOL CSoundMixer::IsChannelUsed(int channel)  { _bgmain(0x9E0DDA) }
_n BOOL CSound::SetVolume(int)                   { _bgmain(0x9DF37A) }
_n BOOL CSound::ResetVolume()                    { _bgmain(0x9DEAB9) }
_n void CSoundMixer::UpdateSoundList()           { _bgmain(0x9E17D5) }
_n void CSoundMixer::AddToLoopingList(CSound*)   { _bgmain(0x9DFF92) }
