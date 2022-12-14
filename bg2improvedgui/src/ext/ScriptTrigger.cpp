#include "ScriptTrigger.h"

#include "console.h"

BOOL __stdcall CCreatureObject_EvaluateTrigger_Kit(int dwKit, CreFileData* pBaseStats) {
	int dwCreKit = pBaseStats->kitLow | (pBaseStats->kitHigh << 16);
	if (dwKit == 0) return !pBaseStats->kitLow; // ?
	return dwKit == dwCreKit;
}
