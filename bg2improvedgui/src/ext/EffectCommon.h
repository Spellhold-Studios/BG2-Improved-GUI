#ifndef EFFECTCOMMON_H
#define EFFECTCOMMON_H

#include "effcore.h"
#include "objcre.h"

CEffect* CEffectList_FindPrevious(CEffectList& list, CEffect& eff, POSITION posCurrent, CCreatureObject& cre);
void  CGameEffectSummon_ApplyVisualEffect(ResRef& Res, POINT* pt, CCreatureObject& Cre, CArea *pArea);


#endif //EFFECTCOMMON_H