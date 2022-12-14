#ifndef SCRIPTCOMMON_H
#define SCRIPTCOMMON_H

#include "scrcore.h"
#include "InfGameCommon.h"

IECString ParseBlockVariables(IECString s, int nType, CBlockVariables& vars);
void ParseStatement(unsigned int nIndex, int nType, IECString s, CGameAIBase& sprite, CBlockVariables& vars);

#endif //SCRIPTCOMMON_H