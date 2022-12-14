#include "UserCommon.h"

#include "stdafx.h"
#include "objcre.h"
#include "resref.h"

bool UserCommon_HasSpell(CCreatureObject* pCre, ResRef& rSpell, CKnownSpellList* cplKnownSpell) {
	POSITION pos = cplKnownSpell->GetHeadPosition();
	while (pos != NULL) {
		CreFileKnownSpell* pcks = (CreFileKnownSpell*)cplKnownSpell->GetNext(pos);
		if (!(pcks->name != rSpell)) return true;
	}
	return false;
}

bool UserCommon_HasKnownSpell(CCreatureObject* pCre, ResRef& rSpell, int nLevel) {
    if (pGameOptionsEx->bEngineShamanClass && pCre->o.Class == CLASS_SHAMAN)
        return UserCommon_HasSpell(pCre, rSpell, &pCre->KnownSpellsPriest[nLevel-1]);
    else
        return UserCommon_HasSpell(pCre, rSpell, &pCre->KnownSpellsWizard[nLevel-1]);

}