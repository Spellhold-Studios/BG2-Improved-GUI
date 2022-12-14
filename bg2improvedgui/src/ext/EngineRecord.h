#ifndef ENGINERECORD_H
#define ENGINERECORD_H

#include "engrecord.h"

extern void (CScreenRecord::*Tramp_CScreenRecord_MageBookPanelOnLoad)(CCreatureObject&);
extern void (CScreenRecord::*Tramp_CScreenRecord_MageBookPanelOnUpdate)(CCreatureObject&);
extern void (CScreenRecord::*Tramp_CScreenRecord_UpdateCharacter)();
extern void (CScreenRecord::*Tramp_CScreenRecord_UpdateStyleBonus)(CUITextArea&, uint, int);

class DETOUR_CScreenRecord : public CScreenRecord {
public:
	void DETOUR_MageBookPanelOnLoad(CCreatureObject& cre);
	void DETOUR_MageBookPanelOnUpdate(CCreatureObject& cre);
	void DETOUR_UpdateCharacter();
    void DETOUR_UpdateStyleBonus(CUITextArea&, uint, int);
};

#endif //ENGINERECORD_H
