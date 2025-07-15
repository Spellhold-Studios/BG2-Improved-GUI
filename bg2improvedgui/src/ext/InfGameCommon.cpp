#include "InfGameCommon.h"

#include "stdafx.h"
#include "chitin.h"

//CBlockVariables
CBlockVariables::CBlockVariables() {
    for (int i = 0; i < BLOCK_VAR_ARRAY_SIZE; i++) {
        m_Ints[i] = 0;
    }
}

void CBlockVariables::Empty() {
    for (int i = 0; i < BLOCK_VAR_ARRAY_SIZE; i++) {
        m_Ints[i] = 0;
        m_Strs[i].Empty();
    }
}

int CBlockVariables::GetInt(unsigned int index) {
    return (index >= 0 && index < BLOCK_VAR_ARRAY_SIZE) ? m_Ints[index] : 0;
}

IECString CBlockVariables::GetStr(unsigned int index) {
    return (index >= 0 && index < BLOCK_VAR_ARRAY_SIZE) ? m_Strs[index] : IECString();
}

void CBlockVariables::SetInt(unsigned int index, int value) {
    if (index >= 0 && index < BLOCK_VAR_ARRAY_SIZE) m_Ints[index] = value;
}

void CBlockVariables::SetStr(unsigned int index, IECString value) {
    if (index >= 0 && index < BLOCK_VAR_ARRAY_SIZE) m_Strs[index] = value;
}

//CRuleTablesEx
static MathPresso::mreal_t mpAnd(MathPresso::mreal_t x, MathPresso::mreal_t y) { return (int)x && (int)y; }
static MathPresso::mreal_t mpOr (MathPresso::mreal_t x, MathPresso::mreal_t y) { return (int)x || (int)y; }
static MathPresso::mreal_t mpBAnd(MathPresso::mreal_t x, MathPresso::mreal_t y) { return (MathPresso::mreal_t) ((int)x & (int)y); }
static MathPresso::mreal_t mpBOr (MathPresso::mreal_t x, MathPresso::mreal_t y) { return (MathPresso::mreal_t) ((int)x | (int)y); }

CRuleTablesEx* pRuleEx = NULL;
int g_nCDerivedStatsTemplateSize = 0;

CRuleTablesEx::CRuleTablesEx(CRuleTables& rule) {
    Init(rule);
}

extern void BGSNDSET_InitHash(CRuleTable& Table);
extern void BGSNDSET_DestroyHash();

CRuleTablesEx::~CRuleTablesEx() {
    if (m_StrModExToStepTable) {
        delete[] m_StrModExToStepTable;
        m_StrModExToStepTable = NULL;
    }
    if (m_StepToStrModExTable) {
        delete[] m_StepToStrModExTable;
        m_StepToStrModExTable = NULL;
    }

    std::map<Enum, CBlockVariables*>::iterator it;
    for (it = m_MapActionVars.begin(); it != m_MapActionVars.end(); it++) {
        delete it->second;
        it->second = NULL;
    }
    m_MapActionVars.clear();

    BGSNDSET_DestroyHash();
}


void CRuleTablesEx::Init(CRuleTables& rule) {

    if (pGameOptionsEx->bEffStrengthMod || pGameOptionsEx->bEffDexterityMod) {
        m_ClassSpellAbility.LoadTable(ResRef("CLSSPLAB"));
        if (!m_ClassSpellAbility.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): CLSSPLAB.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    m_StrModExToStepTable =  NULL;
    m_StepToStrModExTable =  NULL;
    m_StrModExStepNum = 0;

    if (pGameOptionsEx->bEffStrengthMod) {
        if (!rule.STRMODEX.m_2da.bLoaded) {
            rule.STRMODEX.LoadTable(ResRef("STRMODEX"));
        }
        if (rule.STRMODEX.m_2da.bLoaded) {
            m_StrModExToStepTable = new char[101];
            m_StepToStrModExTable = new char[101];
            IECString sToHit = "TO_HIT";
            IECString sDamage = "DAMAGE";
            IECString sBBLG = "BEND_BARS_LIFT_GATES";
            IECString sWeight = "WEIGHT_ALLOWANCE";
            IECString sIdx;
            IECString sOldToHit;
            IECString sOldDamage;
            IECString sOldBBLG;
            IECString sOldWeight;
            IECString sNewToHit; 
            IECString sNewDamage;
            IECString sNewBBLG;  
            IECString sNewWeight;
            for (char i = 0; i < 101; ++i) {
                sIdx.Format("%d", i);
                sNewToHit = rule.STRMODEX.GetValue(sToHit, sIdx);
                sNewDamage = rule.STRMODEX.GetValue(sDamage, sIdx);
                sNewBBLG = rule.STRMODEX.GetValue(sBBLG, sIdx);
                sNewWeight = rule.STRMODEX.GetValue(sWeight, sIdx);

                if (i > 0) {
                    if (sNewToHit.CompareNoCase(sOldToHit) || 
                        sNewDamage.CompareNoCase(sOldDamage) ||
                        sNewBBLG.CompareNoCase(sOldBBLG) ||
                        sNewWeight.CompareNoCase(sOldWeight)) {
                        m_StepToStrModExTable[m_StrModExStepNum++] = i;
                    }
                }

                m_StrModExToStepTable[i] = 18 + m_StrModExStepNum;

                sOldToHit = sNewToHit;
                sOldDamage = sNewDamage;
                sOldBBLG = sNewBBLG;
                sOldWeight = sNewWeight;
            }
        } else {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): STRMODEX.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }
 
    if (pGameOptionsEx->bEngineExternClassRaceRestrictions) {
        m_ClassRaceReq.LoadTable(ResRef("CLSRCREQ"));
        if (!m_ClassRaceReq.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): CLSRCREQ.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bEngineExternClassRaceRestrictions) {
        m_MageSchoolRaceReq.LoadTable(ResRef("MGSRCREQ"));
        if (!m_MageSchoolRaceReq.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): MGSRCREQ.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bSoundExternWalkSounds) {
        m_AnimWalkSound.LoadTable(ResRef("ANIWKSND"));
        if (!m_AnimWalkSound.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ANIWKSND.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bSoundExternWalkSounds) {
        m_AnimTerrainSound.LoadTable(ResRef("ANITNSND"));
        if (!m_AnimTerrainSound.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ANITNSND.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bUserExternMageSpellHiding) {
        m_HideSpell.LoadTable(ResRef("HIDESPL"));
        if (!m_HideSpell.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): HIDESPL.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bUserExternRaceSelectionText) {
        m_RaceText.LoadTable(ResRef("RACETEXT"));
        if (!m_RaceText.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): RACETEXT.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bItemsExternCreExcl) {
        m_ItemCreExclude.LoadTable(ResRef("ITEM_USE"));
        if (!m_ItemCreExclude.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ITEM_USE.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bEngineExpandedStats) {
        Identifiers idStats("STATS");
        if (!idStats.m_ids.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): Something went wrong with loading STATS.IDS. Expanded Stats is disabled.\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
            m_nStats = 201;
        } else {
            int nHighestStat = 400; //incorporates new hard-coded stats 202-400
            POSITION pos = idStats.entries.GetHeadPosition();
            while (pos != NULL) {
                IdsEntry* pIde = (IdsEntry*)(idStats.entries.GetNext(pos));
                nHighestStat = max(nHighestStat, pIde->nOpcode);
            }
            m_nStats = nHighestStat;
        }
        g_nCDerivedStatsTemplateSize = sizeof(CDerivedStatsTemplate) + (m_nStats - 200 - 1) * 4;
    }

    m_nEncumbranceLowThreshold = -1;
    m_nEncumbranceHighThreshold = -1;

    if (pGameOptionsEx->bEngineExternEncumbrance) {
        m_Encumbrance.LoadTable(ResRef("ENCUMBER"));
        if (!m_Encumbrance.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ENCUMBER.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        } else {
            int s;
            IECString sLowThreshold = m_Encumbrance.GetValue(IECString("THRESHOLD"), IECString("LOW_ENCUMBRANCE"));
            IECString sHighThreshold = m_Encumbrance.GetValue(IECString("THRESHOLD"), IECString("HIGH_ENCUMBRANCE"));

            sscanf_s((LPCTSTR)sLowThreshold, "%d", &s);
            m_nEncumbranceLowThreshold = s;

            sscanf_s((LPCTSTR)sHighThreshold, "%d", &s);
            m_nEncumbranceHighThreshold = s;
        }
    }

    if (m_nEncumbranceLowThreshold < 0) m_nEncumbranceLowThreshold = 100;
    if (m_nEncumbranceHighThreshold < 0) m_nEncumbranceHighThreshold = 120;

    if (pGameOptionsEx->bEngineExternHPTables) {
        m_HPClass.LoadTable(ResRef("HPCLASS"));
        if (!m_HPClass.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): HPCLASS.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        m_HPBarbarian.LoadTable(ResRef("HPBARB"));
        if (!m_HPBarbarian.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): HPBARB.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bEngineExternStealSlots) {
        m_StealSlots.LoadTable(ResRef("SLTSTEAL"));
        if (!m_StealSlots.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): SLTSTEAL.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bEngineShamanClass) {
        Shaman_SKILLSHM.LoadTable(ResRef("SKILLSHM")); // Shaman skill points
        if (!Shaman_SKILLSHM.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): SKILLSHM.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        Shaman_SPLSHMKN.LoadTable(ResRef("SPLSHMKN")); // Shaman spell slots per lev
        if (!Shaman_SPLSHMKN.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): SPLSHMKN.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        Shaman_MXSPLSHM.LoadTable(ResRef("MXSPLSHM")); // Shaman max memorized spells per lev
        if (!Shaman_MXSPLSHM.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): MXSPLSHM.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        Shaman_CLABSH01.LoadTable(ResRef("CLABSH01")); // Shaman class abilities
        if (!Shaman_CLABSH01.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): CLABSH01.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bSound_Normalize) {
        Normalize_EXCLNORM.LoadTable(ResRef("EXCLNORM")); // Exclude sounds from normalize
    }

    if (pGameOptionsEx->bUI_SpellIconRightClick) {
        SpellInfo_ABILDESC.LoadTable(ResRef("ABILDESC")); // Ability Description
        if (!SpellInfo_ABILDESC.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ABILDESC.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }
    
    if (pGameOptionsEx->bSound_BGTSelectionSoundSet) {
        SoundSet_BGSNDSET.LoadTable(ResRef("BGSNDSET")); // Soundset Types
        if (!SoundSet_BGSNDSET.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): BGSNDSET.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        } else {
             BGSNDSET_InitHash(SoundSet_BGSNDSET);
        }
    }

    if (pGameOptionsEx->bSound_ANISNDEX) {
        SoundSet_ANISNDEX.LoadTable(ResRef("ANISNDEX")); // ANISNDEX.2DA
        if (!SoundSet_ANISNDEX.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): ANISNDEX.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    if (pGameOptionsEx->bAnimation_EXTANIM) {
        Animation_EXTANIM.LoadTable(ResRef("EXTANIM")); // EXTANIM.2DA
        if (!Animation_EXTANIM.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): EXTANIM.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        Animation_EXTANI60.LoadTable(ResRef("EXTANI60")); // EXTANI60.2DA
        if (!Animation_EXTANI60.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): EXTANI60.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }

        Animation_EXTANI64.LoadTable(ResRef("EXTANI64")); // EXTANI64.2DA
        if (!Animation_EXTANI64.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): EXTANI64.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    
    if (pGameOptionsEx->bUI_BlockAreaTransitionCombat) {
        Area_UNBLAREA.LoadTable(ResRef("UNBLAREA")); // UNBLAREA.2DA
        if (!Area_UNBLAREA.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): UNBLAREA.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    m_mpContext.addFunction("and", mpAnd, MathPresso::MFUNC_F_ARG2);
    m_mpContext.addFunction("or", mpOr, MathPresso::MFUNC_F_ARG2);
    m_mpContext.addFunction("band", mpBAnd, MathPresso::MFUNC_F_ARG2);
    m_mpContext.addFunction("bor", mpBOr, MathPresso::MFUNC_F_ARG2);

    if (pGameOptionsEx->bEngineExternDifficulty) {
        m_DiffMod.LoadTable(ResRef("DIFFMOD"));
        if (!m_DiffMod.m_2da.bLoaded) {
            LPCTSTR lpsz = "CRuleTablesEx::Init(): DIFFMOD.2DA not found\r\n";
            console.write(lpsz);
            L.timestamp();
            L.append(lpsz);
        }
    }

    return;
}