#ifndef FILEITM_H
#define FILEITM_H

#include "stdafx.h"
#include "resref.h"

typedef struct _ItmFileHeader {
    DWORD   m_nFileType;            //  0
    DWORD   m_nFileVersion;         //  4
    STRREF  m_strrefGenericName;    //  8
    STRREF  m_strrefIdentifiedName; //  c
    ResRef  m_rUsedUpItem;          // 10
    DWORD   m_dwItemFlags;          // 18
    WORD    m_wItemType;            // 1C
    DWORD   m_dwNotUsableBy;        // 1E
    BYTE    m_cAnimationType[2];    // 22 WORD animationType;
    WORD    m_cMinLevelRequired;    // 24
    WORD    m_cMinSTRRequired;      // 26
    BYTE    m_cMinSTRBonusRequired; // 28
    BYTE    m_cNotUsableBy2a;       // 29 Uppermost byte of a DWORD
    BYTE    m_cMinINTRequired;      // 2a
    BYTE    m_cNotUsableBy2b;       // 2b
    BYTE    m_cMinDEXRequired;      // 2c
    BYTE    m_cNotUsableBy2c;       // 2d
    BYTE    m_cMinWISRequired;      // 2e
    BYTE    m_cNotUsableBy2d;       // 2f
    BYTE    m_cMinCONRequired;      // 30
    BYTE    m_cProficiencyType;     // 31
    WORD    m_wMinCHRRequired;      // 32

    DWORD   m_dwBaseValue;                  // 34 Price
    WORD    m_wMaxStackable;                // 38
    ResRef  m_rItemIcon;                    // 3a
    WORD    m_wLoreValue;                   // 42
    ResRef  m_rGroundIcon;                  // 44
    DWORD   m_dwBaseWeight;                 // 4c
    STRREF  m_strrefGenericDescription;     // 50
    STRREF  m_strrefIdentifiedDescription;  // 54
    ResRef  m_rDescriptionPicture;          // 58
    DWORD   m_dwAttributes;                 // 60

    DWORD   m_dwAbilityOffset;              // 64
    WORD    m_wAbilityCount;                // 68

    DWORD   m_dwEffectsOffset;              // 6a
    WORD    m_wEquippedStartingEffect;      // 6e
    WORD    m_wEquippedEffectCount;         // 70
} ItmFileHeader;

#endif //FILEITM_H