#include "LogActiveSpells.h"

#define MAXICONS 512         // max 64 new + 448 statdesc.2da

typedef struct _ArrayType {
    int TextRef;
    int Ticks;
    int Optional;
    int LoopMode;
} ArrayType;

// last used Array index (reserved part) = -17 !!!

static char     gIniFilter_Buf[MAXICONS * 5];
static bool     gFilter_Buf   [MAXICONS];


void inline
AddIcon(ArrayType* const Array, int const IconRef, int const TextRef, int const Ticks, int const LoopMode, int const Optional = 0)
{
    Array[IconRef + 64].TextRef      = TextRef;
    Array[IconRef + 64].Ticks        = Ticks;
    Array[IconRef + 64].LoopMode     = LoopMode;
    if (Optional != 0) {
        Array[IconRef + 64].Optional = Optional;
    }
}


int
GetTextRef(int IconRef)
{
    const CRuleTable* const pRuleTable = &g_pChitin->pGame->STATDESC;

    for (int nRow = 0; nRow < pRuleTable->nRows; nRow++) {
            IECString& rowHeader = pRuleTable->pRowHeaderArray[nRow];
            int RefID = atoi((LPCTSTR)rowHeader);
            if (RefID == IconRef) {
                return atoi((LPCTSTR)pRuleTable->pDataArray[nRow]);
            }
    }

    return 0;
}


void
ReturnMessage(CCreatureObject& Cre, IECString& sOwner, ABGR const colorOwner, ABGR const colorText)
{
    CStrRef TlkStringerror;

    g_pChitin->m_TlkTbl.GetTlkString(396, TlkStringerror); // 396 - Invisible
    g_pChitin->pScreenWorld->PrintToConsole(sOwner, TlkStringerror.text, colorOwner, colorText, -1, 0); 
}

void __stdcall
CreatureRMClick__LogActiveBuffs(CCreatureObject& Cre, unsigned int const ObjID)
{
    #ifdef _DEBUG
        IECString DebugText = "";
        if (Cre.nObjType == CGAMEOBJECT_TYPE_CREATURE ||
            Cre.nObjType == CGAMEOBJECT_TYPE_DOOR     ||
            Cre.nObjType == CGAMEOBJECT_TYPE_CONTAINER)
            Cre.DebugDump(DebugText, 0); // > baldur.log
    #endif

    if (Cre.nObjType != CGAMEOBJECT_TYPE_CREATURE)   // 0x31 = NPC
        return;

    ABGR const colorOwner = *(g_pColorRangeArray + Cre.BaseStats.colors.colorMajor);
    ABGR const colorTextItems   = g_ColorDefaultText;
    ABGR const colorTextSpells  = 0x78c094;   // pale yellow

    CEffectList& SpellEffectList = Cre.GetMainEffectsList();
    CEffectList& ItemsEffectList = Cre.GetEquippedEffectsList();
    IECString&   sOwner = Cre.GetLongName();
    bool        const ShowTicks             =  to_bool(pGameOptionsEx->bUI_LogActiveBuffsWithTime);
    bool        const ShowItems             =  to_bool(pGameOptionsEx->bUI_LogActiveBuffsWithItems);
    bool        const ShowTicksPartyOnly    =  to_bool(pGameOptionsEx->bUI_LogActiveBuffsWithTime_Party);
    bool              SkipShowTicks         = false;

    CDerivedStats& cds = (Cre.bUseCurrentState) ? Cre.cdsCurrent : Cre.cdsPrevious;

    if (Cre.o.EnemyAlly > EA_CONTROLLEDFORCEADD &&
        (cds.stateFlags & STATE_INVISIBLE ||
        Cre.canBeSeen <= 0)) {
        ReturnMessage(Cre, sOwner, colorOwner, colorTextSpells);
        return;
    }

    ArrayType   Array[MAXICONS];
    memset(Array, 0, sizeof(Array));

    POSITION pos_spells = SpellEffectList.GetHeadPosition();
    POSITION pos_items = NULL;
    if (ShowItems) {
        pos_items = ItemsEffectList.GetHeadPosition();
    }
    
    if (Cre.BaseStats.morale <= cds.moraleBreak ||
        Cre.bMoraleBroken) {
        AddIcon(Array, -15, 14134, -1, 1); // Morale Failure
    }

    while (pos_spells || pos_items) {
        CEffect*    pEff = NULL;
        int         ListMode = 0;
        if (pos_spells) {
            pEff = (CEffect *) SpellEffectList.GetNext(pos_spells);
            ListMode = 1;   // spell loop
        } else if (pos_items) {
            pEff = (CEffect *) ItemsEffectList.GetNext(pos_items);
            ListMode = 2;   // item loop
        }

        if (pEff == NULL) {
            continue;
        }

        EffFileData&    effect = pEff->effect;
        int             Mode = 0;
        int             IconRef;
        int OpCode    = effect.nOpcode;

        int ticks;
        if (effect.nDuration)
            ticks = (effect.nDuration - g_pChitin->pGame->m_WorldTimer.nGameTime) / 15; // x/TICKS_PER_SECOND
        else
            if (ShowItems)
                ticks = -1; // infinite
            else
                ticks = 0;  // skip effect

        int Optional  = 0;

        if (ticks <= 0 && ListMode == 1)                  // effect is out
            continue;    

        //if (effect.nParentResourceType == 1 ) {         // effect from spell
        //    Mode = 1;
        //}

        //if (effect.nParentResourceType == 0 ) {         // some effects without parent, force to spell type
        //    Mode = 1;
        //}

        //if (effect.nParentResourceType == 2 ) {         // effect from item
        //    Mode = 2;
        //}
        Mode = 1;

        if (ShowTicksPartyOnly) {
            CCreatureObject*   pCre;
            Enum SourceID = pEff->eSource;
            SkipShowTicks  = true;
            
            // -1 if:
            // unknow/self source creature
            // infinite persistent effect
            // timed effect from savegame
            if (SourceID != -1 && SourceID != 0)    // -1
                if (g_pChitin->pGame->m_GameObjectArrayHandler.GetGameObjectShare(SourceID, THREAD_ASYNCH, &pCre, -1) == OBJECT_SUCCESS) {
                    if (pCre->nObjType == CGAMEOBJECT_TYPE_CREATURE) { // + CGAMEOBJECT_TYPE_SPRITE ?
                        unsigned char EA = pCre->o.EnemyAlly;
                        if (EA <= 7) {   //  ALLY/FAMILIAR/CONTROLLED/CHARMED
                            SkipShowTicks = false;
                        }
                    }
                    g_pChitin->pGame->m_GameObjectArrayHandler.FreeGameObjectShare(SourceID, THREAD_ASYNCH, INFINITE);
                }
        }

        if (SkipShowTicks) {
            ticks = -2; // unknow duration
        }

        // 142 - Display Icon
        if (OpCode == 142) { // && Mode == 1
            IconRef = effect.nParam2;
            int TextRef = 0;

            // skip out-of-battle/illusion icons
            if (IconRef != 31  &&           // Non-detectable
                IconRef != 39  &&           // Fatigue
                IconRef != 40  &&           // Bard Song
                IconRef != 42  &&           // Regenerate
                IconRef != 54  &&           // PolymorphSelf, Human form
                IconRef != 56  &&           // Regeneration
                IconRef != 57  &&           // Perception
                IconRef != 58  &&           // Master Thievery
                IconRef != 66  &&           // Repulsing Undead
                IconRef != 75  &&           // Contingency Active
                IconRef != 77  &&           // Projected Image
                IconRef != 87  &&           // Regeneration
                IconRef != 88  &&           // In Dialog
                IconRef != 89  &&           // In Store
                IconRef != 92  &&           // Spell Sequencer Active
                IconRef != 96  &&           // Setting Trap
                IconRef != 108 &&           // True Seeing 
                IconRef != 109 &&           // Detecting Traps
                IconRef != 114 &&           // Infravision
                IconRef != 115 &&           // Friends
                IconRef != 120 &&           // Farsight
                IconRef != 133 &&           // Repulse Undead
                IconRef != 151 &&           // Magic Flute
                IconRef != 155 ) {          // Avoid Death
                    TextRef = GetTextRef(IconRef);
                }

                if (TextRef != 0) {
                    AddIcon(Array, IconRef, TextRef, ticks, ListMode);
                }

                continue; // to next pos
            }


        // effects without "display icon" opcode
        if (1) { //if (Mode == 1) {
            switch (OpCode) {
            // 0..9
            // "AC bonus", "Modify attacks per round", "Cure sleep", "Berserk", "Cure berserk",
            // "Charm creature", "Charisma bonus", "Set color", "Set color glow solid",
            // "Set color glow pulse",
            case 3:     // "Berserk"
                AddIcon(Array, 4, GetTextRef(4), ticks, ListMode);          // 4 Berserk
                break;
            case 5:     // "Charm creature"
                AddIcon(Array, 0, GetTextRef(0), ticks, ListMode);          // 0 Charm
                break;
            // 10..19
            // "Constitution bonus", "Cure poison", "Damage", "Kill target", "Defrost",
            // "Dexterity bonus", "Haste", "Current HP bonus", "Maximum HP bonus", "Intelligence bonus",
            case 16:    // "Haste"
                if (effect.nParam2 == 0 ||  effect.nParam2 == 2) {          // normal haste
                    AddIcon(Array, 38, GetTextRef(38), ticks, ListMode);    // 38 Haste
                }
                if (effect.nParam2 == 1) {  // improved haste
                    AddIcon(Array, 110, GetTextRef(110), ticks, ListMode);  // 110 Improved Haste
                }
                break;
            // 20..29
            // "Invisibility", "Lore bonus", "Luck bonus", "Reset morale", "Panic", "Poison",
            // "Remove curse", "Acid resistance bonus", "Cold resistance bonus",
            // "Electricity resistance bonus",
            case 20:    // "Invisibility"
                if (Cre.o.EnemyAlly > EA_CONTROLLEDFORCEADD) {
                    ReturnMessage(Cre, sOwner, colorOwner, colorTextSpells);
                    return;
                }
                else
                    if (effect.nParam2 == 0 ) 
                        AddIcon(Array, -16, 396, ticks, ListMode);      // Invisibility
                    else
                        AddIcon(Array, -17, 14783, ticks, ListMode);    // Improved Invisibility
                break;
            case 24:    // "Panic"
                AddIcon(Array, 36, GetTextRef(36), ticks, ListMode);          // 36 Panic
                break;
            case 25:    // "Poison"
                AddIcon(Array, 6, GetTextRef(6), ticks, ListMode);            // 6 Poisoned
                break;
            // 30..39
            // "Fire resistance bonus", "Magic damage resistance bonus", "Raise dead",
            // "Save vs. death bonus", "Save vs. wand bonus", "Save vs. polymorph bonus",
            // "Save vs. breath bonus", "Save vs. spell bonus", "Silence", "Sleep",
            case 38:    // "Silence"
                AddIcon(Array, 34, GetTextRef(34), ticks, ListMode);          // 34 Silence
                break;
            case 39:    // "Sleep"
                AddIcon(Array, 14, GetTextRef(14), ticks, ListMode);          // 14 Sleep
                break;
            // 40..49
            // "Slow", "Sparkle", "Bonus wizard spells", "Stone to flesh", "Strength bonus", "Stun",
            // "Cure stun", "Remove invisibility", "Vocalize", "Wisdom bonus",
            case 40:    // "Slow"
                AddIcon(Array, 41, GetTextRef(41), ticks, ListMode);          // 41 Slow
                break;
            case 44:    // "Strength bonus"
            case 97:    // "Exceptional strength bonus"
                AddIcon(Array, 21, GetTextRef(21), ticks, ListMode);          // 21 Strength
                break;
            case 45:    // "Stun"
                AddIcon(Array, 55, GetTextRef(55), ticks, ListMode);          // 14 Stun
                break;
            case 48:    // "Vocalize"
                AddIcon(Array, 103, GetTextRef(103), ticks, ListMode);        // 14 Vocalize
                break;
            // 50..59
            // "Character color pulse", "Character tint solid", "Character tint bright",
            // "Animation change", "Base THAC0 bonus", "Slay", "Invert alignment", "Change alignment",
            // "Dispel effects", "Move silently bonus",
            // 60..69
            // "Casting failure", "Unknown (61)", "Bonus priest spells", "Infravision",
            // "Remove infravision", "Blur", "Translucency", "Summon creature", "Unsummon creature",
            // "Nondetection",
            case 60:    // "Casting failure"
                if (effect.nTiming != TIMING_INSTANT_EQUIPPED) {    // temp effect only, not item or some unlimited
                    AddIcon(Array, 105,  GetTextRef(105), ticks, ListMode);   // 105 Miscast Magic
                }
                break;
            case 65:    // "Blur"
                AddIcon(Array, -1, 14062, ticks, ListMode);               // -1 Blur (new)
                break;
            // 70..79
            // "Remove nondetection", "Change gender", "Change AI type", "Attack damage bonus",
            // "Blindness", "Cure blindness", "Feeblemindedness", "Cure feeblemindedness", "Disease",
            // "Cure disease",
            case 74:    // "Blindness"
                AddIcon(Array, 8,   GetTextRef(8), ticks, ListMode);      // 8 Blindness
                break;
            case 76:    // "Feeblemindedness"
                AddIcon(Array, 48,  GetTextRef(48), -1, ListMode);        // 48 Feeblemindedness, unlimited
                break;
            case 78:    // "Disease"
                AddIcon(Array, 7,   GetTextRef(7),  ticks, ListMode);     // 7 Disease
                break;
            // 80..89
            // "Deafness", "Cure deafness", "Set AI script", "Immunity to projectile",
            // "Magical fire resistance bonus", "Magical cold resistance bonus",
            // "Slashing resistance bonus", "Crushing resistance bonus", "Piercing resistance bonus",
            // "Missile resistance bonus",
            case 80:    // "Deafness"
                AddIcon(Array, 112,   GetTextRef(112),  ticks, ListMode);     // 112 Deafness
                break;
            // 90..99
            // "Open locks bonus", "Find traps bonus", "Pick pockets bonus", "Fatigue bonus",
            // "Intoxication bonus", "Tracking bonus", "Change level", "Exceptional strength bonus",
            // "Regeneration", "Modify duration",
            // 100..109
            // "Protection from creature type", "Immunity to effect", "Immunity to spell level",
            // "Change name", "XP bonus", "Remove gold", "Morale break", "Change portrait",
            // "Reputation bonus", "Paralyze",
            case 102:   // "Immunity to spell level"
                Optional = effect.nParam1;  // spell level
                if (Optional > Array[107+64].Optional) { // x > max level
                    AddIcon(Array, 107, GetTextRef(107), ticks, ListMode, Optional);   // increase max level
                } else {
                    AddIcon(Array, 107, GetTextRef(107), ticks, ListMode);    // 107 Spell immunity
                }
                break;
            case 109:   // "Paralyze"
                AddIcon(Array, 13, GetTextRef(13),  ticks, ListMode);         // 13 Hold
                break;
            // 110..119
            // "Retreat from", "Create weapon", "Remove item", "Equip weapon", "Dither",
            // "Detect alignment", "Detect invisible", "Clairvoyance",  "Show creatures", "Mirror image",
            case 111:   // "Create weapon"
                AddIcon(Array, -14, 14104,  ticks, ListMode);                 // Create Weapon (new)
                break;
            case 119:   // "Mirror image"
                // Mirror Image set 119 opcode in .SPL
                // but in-game opcode is 159
                AddIcon(Array, -2, 14774, ticks, ListMode);                   // Mirror Image (new)
                break;
            // 120..129
            // "Immunity to weapons", "Visual animation effect", "Create inventory item",
            // "Remove inventory item", "Teleport", "Unlock", "Movement rate bonus", "Summon monsters",
            // "Confusion", "Aid (non-cumulative)",
            case 120:   // "Immunity to weapons"
                if (effect.nParam2 == 2) {  // weapon type
                    AddIcon(Array, 70, GetTextRef(70),  ticks, ListMode);     // 70 Immunity to normal weapons
                }
                if (effect.nParam2 == 1) {
                    AddIcon(Array, 71, GetTextRef(71),  ticks, ListMode);     // 71 Immunity to magic weapons
                }
                break;
            case 128:   // "Confusion"
                    AddIcon(Array, 3, GetTextRef(3),  ticks, ListMode);       // 3 Confusion
                break;
            case 129:   // "Aid"
                AddIcon(Array, -12, 14122,  ticks, ListMode);                 // Aid (new)
                break;
            // 130..139
            // "Bless (non-cumulative)", "Chant (non-cumulative)", "Draw upon holy might (non-cumulative)",
            // "Luck (non-cumulative)", "Petrification", "Polymorph", "Force visible",
            // "Bad chant (non-cumulative)", "Set animation sequence", "Display string",
            case 130:   // "Bless"
                AddIcon(Array, 17, GetTextRef(17),  ticks, ListMode);         // 17 Bless
                break;
            case 131:   // "Chant"
                AddIcon(Array, 18, GetTextRef(18),  ticks, ListMode);         // 18 Chant
                break;
            case 132:   // "Draw upon holy might"
                AddIcon(Array, 135, GetTextRef(135),  ticks, ListMode);       // 135 Draw upon holy might
                break;
            //case 133: // "Luck"
            //    AddIcon(Array, 32, GetTextRef(32),  ticks, ListMode);       // 32 Luck
            case 135:   // "Polymorph"
                AddIcon(Array, 54, GetTextRef(54), ticks, ListMode);          // 135 - Polymorph, Shapeshift*
                break;
            //case 137:
            //    text.Format("Bad chant (%d secs)", ticks, ListMode);
            // 140..149
            // "Casting glow", "Lighting effects", "Display portrait icon", "Create item in slot",
            // "Disable button", "Disable spellcasting", "Cast spell", "Learn spell",
            // "Cast spell at point", "Identify",
            // 150..159
            // "Find traps", "Replace self", "Play movie", "Sanctuary", "Entangle overlay",
            // "Minor globe overlay", "Protection from normal missiles overlay", "Web effect",
            // "Grease overlay", "Mirror image effect",
            case 153:   // "Sanctuary"
                if (Cre.o.EnemyAlly > EA_CONTROLLEDFORCEADD) {
                    ReturnMessage(Cre, sOwner, colorOwner, colorTextSpells);
                    return;
                } else {
                    AddIcon(Array, -16, 396, ticks, ListMode);      // Invisibility
                }
                break;
            case 154:   // "Entangle overlay"
                AddIcon(Array, 144, GetTextRef(144), ticks, ListMode);        // 144 Entangle     
                break;
            case 155:   // "Minor globe overlay"
                if (effect.nSpellLvl > 4) {   // spell level
                    AddIcon(Array, 121, GetTextRef(121), ticks, ListMode);    // 121 Globe of Invulnerability
                } else {
                    AddIcon(Array, 122, GetTextRef(122), ticks, ListMode);    // 122 Minor globe of Invulnerability
                }
                break;
            case 156:   // "Protection from normal missiles overlay"
                AddIcon(Array, 11, GetTextRef(11), ticks, ListMode);          // 11 Protection from normal missiles
                break;
            case 157:   // "Web effect"
                AddIcon(Array, 129, GetTextRef(129), ticks, ListMode);        // 129 Webbed
                break;
            case 158:   // "Grease overlay"
                AddIcon(Array, 145, GetTextRef(145), ticks, ListMode);        // 145 Grease
                break;
            case 159:   // "Mirror image effect"
                // Reflected Image
                // Mirror image
                AddIcon(Array, -2, 14774, ticks, ListMode);                   // -2 Mirror Image (new)
                break;
            // 160..169
            // "Remove sanctuary", "Remove fear", "Remove paralysis", "Free action",
            // "Remove intoxication", "Pause target", "Magic resistance bonus", "Missile THAC0 bonus",
            // "Remove creature", "Prevent portrait icon",
            case 163:   // "Free action"
                AddIcon(Array, 19, GetTextRef(19), ticks, ListMode);          // 19 Free Action
                break;
            case 166:   // "Magic resistance bonus"
                if (effect.nParam1 < 0) {   // Bonus Value
                    AddIcon(Array, 106, GetTextRef(106), ticks, ListMode);    // 106 Magic Resistance Lowered
                } else {
                    AddIcon(Array, 63, GetTextRef(63), ticks, ListMode);      // 63 Magic Resistance
                }
                break;
            // 170..179
            // "Play damage animation", "Give innate ability", "Remove spell", "Poison resistance bonus",
            // "Play sound", "Hold creature", "Movement rate bonus 2", "Use EFF file",
            // "THAC0 vs. type bonus", "Damage vs. type bonus",
            case 175:   // "Hold creature"
                AddIcon(Array, 13, GetTextRef(13), ticks, ListMode);          // 13 Hold
                break;
            // 180..189
            // "Disallow item", "Disallow item type", "Use EFF file (do not use)",
            // "Use EFF file on equip type", "No collision detection", "Hold creature 2",
            // "Move creature", "Set local variable", "Increase spells cast per round",
            // "Increase casting speed factor",
            //case 188:
            //    text.Format("Increased spells cast (%d secs)", ticks, ListMode);
            //case 189:
            //    text.Format("Increased casting speed (%d secs)", ticks, ListMode);
            // 190..199
            // "Increase attack speed factor", "Casting level bonus", "Find familiar",
            // "Invisibility detection", "Ignore dialogue pause", "Drain CON and HP on death",
            // "Disable familiar", "Physical mirror", "Reflect specified effect", "Reflect spell level",*/
            //case 190:
            //    text.Format("Increase attack speed factor (%d secs)", ticks, ListMode);
            //    break;
            case 197:
                AddIcon(Array, -13, 38577, ticks, ListMode);              // 38577 Physical mirror (new)
                break;
            // 200..209
            // "Spell turning", "Spell deflection", "Reflect spell school", "Reflect spell type",
            // "Protection from spell school", "Protection from spell type", "Protection from spell",
            // "Reflect specified spell", "Minimum HP", "Power word, kill",
            case 200:   // "Spell turning"
                if (effect.nParam2 > 4) {   // spell level
                    AddIcon(Array, 65, 10871, ticks, ListMode);           // 10871 Spell Turning
                } else {
                    if (Array[65 + 64].TextRef != 10871) {      // 10871 - high priority
                        AddIcon(Array, 65, 10850, ticks, ListMode);       // 10850 Minor Spell Turning
                    }
                }
                break;
            case 201:   // "Spell deflection"
                if (effect.nParam2 > 7) {   // spell level
                    AddIcon(Array, 67, 10888, ticks, ListMode);           // 10888 Spell Deflection
                } else {
                    if (Array[67 + 64].TextRef != 10888) {      // 10888 - high priority
                        AddIcon(Array, 67, 10861, ticks, ListMode);       // 10861 Minor Spell Deflection
                    }
                }
                break;
            //case 202:
            //    text.Format("Reflect spell school (%d secs)", ticks, ListMode);
            //case 203:
            //    text.Format("Reflect spell type (%d secs)", ticks, ListMode);
            case 204:   // "Protection from spell school"
                if (effect.nParam2 == 1)
                    AddIcon(Array, -3,  31671, ticks, ListMode);         // Immunity: Abjuration
                if (effect.nParam2 == 2)
                    AddIcon(Array, -4,  31673, ticks, ListMode);         // Immunity: Conjuration
                if (effect.nParam2 == 3)
                    AddIcon(Array, -5,  31677, ticks, ListMode);         // Immunity: Divination
                if (effect.nParam2 == 4)
                    AddIcon(Array, -6,  31678, ticks, ListMode);         // Immunity: Enchantment
                if (effect.nParam2 == 5)
                    AddIcon(Array, -7,  31675, ticks, ListMode);         // Immunity: Illusion
                if (effect.nParam2 == 6)
                    AddIcon(Array, -8,  31676, ticks, ListMode);         // Immunity: Evocation
                if (effect.nParam2 == 7)
                    AddIcon(Array, -9,  31672, ticks, ListMode);         // Immunity: Necromancy
                if (effect.nParam2 == 8)
                    AddIcon(Array, -10, 31674, ticks, ListMode);         // Immunity: Alteration
                if (effect.nParam2 == 9)
                    AddIcon(Array, 107, GetTextRef(107), ticks, ListMode); // 107 Spell Immunity (Generalist)
                break;
            // 210..219
            // "Power word, stun", "Imprisonment", "Freedom", "Maze", "Select spell",
            // "Play visual effect", "Level drain", "Power word, sleep", "Stoneskin effect",
            // "Attack roll penalty",
            case 210:   // "Power word, stun"
                AddIcon(Array, 55, GetTextRef(55), ticks, ListMode);                // 55 Stun
                break;
            case 216:   // "Level drain"
                AddIcon(Array, 53, GetTextRef(53), ticks, ListMode);                // 53 Energy Drain
                break;
            case 217:   // "Power word, sleep"
                AddIcon(Array, 14, GetTextRef(14), ticks, ListMode);                // 14 Sleep
                break;
            case 218:   // "Stoneskin effect"
                AddIcon(Array, 80, GetTextRef(80), ticks, ListMode);                // 80 Stoneskin
                break;
            // 220..229
            // "Remove spell school protections", "Remove spell type protections", "Teleport field",
            // "Spell school deflection", "Restoration", "Detect magic", "Spell type deflection",
            // "Spell school turning", "Spell type turning", "Remove protection by school",
            case 226:   // "Spell type deflection"
                if (effect.nParam2 == 4) // MAGICATTACK 
                    AddIcon(Array, -11,  26228, ticks, ListMode);                    // 26230 Spell Shield (new)
                else
                    AddIcon(Array, 67, GetTextRef(67), ticks, ListMode);             // 67 Spell deflection
                break;
            // 230..239
            // "Remove protection by type", "Time stop", "Cast spell on condition",
            // "Modify proficiencies", "Create contingency", "Wing buffet", "Project image",
            // "Set image type", "Disintegrate", "Farsight",
            case 236:   // "Project image"
                if (Cre.o.EnemyAlly > EA_CONTROLLEDFORCEADD) {
                    ReturnMessage(Cre, sOwner, colorOwner, colorTextSpells);
                    return;
                } else {
                    AddIcon(Array, 77, GetTextRef(77), ticks, ListMode);      // Projected Image
                }
                break;
            // 240..249
            // "Remove portrait icon", "Control creature", "Cure confusion", "Drain item charges",
            // "Drain wizard spells", "Check for berserk", "Berserk effect", "Attack nearest creature",
            // "Melee hit effect", "Ranged hit effect",
            // 250..259
            // "Maximum damage each hit", "Change bard song", "Set trap", "Set automap note",
            // "Remove automap note", "Create item (days)", "Spell sequencer", "Create spell sequencer",
            // "Activate spell sequencer", "Spell trap",
            case 259:   // "Spell trap"
                AddIcon(Array, 117, GetTextRef(117), ticks, ListMode);             // 117 Spell Trap
                break;  
            // 260..269
            // "Activate spell sequencer at point", "Restore lost spells", "Visual range bonus",
            // "Backstab bonus", "Drop item", "Set global variable", "Remove protection from spell",
            // "Disable display string", "Clear fog of war", "Shake screen",
            // 270..279
            // "Unpause target", "Disable creature", "Use EFF file on condition", "Zone of sweet air",
            // "Phase", "Hide in shadows bonus", "Detect illusion bonus", "Set traps bonus",
            // "THAC0 bonus", "Enable button",
            // 280..289
            // "Wild magic", "Wild surge bonus", "Modify script state", "Use EFF file as curse",
            // "Melee THAC0 bonus", "Melee weapon damage bonus", "Missile weapon damage bonus",
            // "Remove feet circle", "Fist THAC0 bonus", "Fist damage bonus",
            // 290..299
            // "Change title", "Disable visual effects", "Immunity to backstab", "Set persistent AI",
            // "Set existence delay", "Disable permanent death", "Immunity to specific animation",
            // "Immunity to turn undead", "Pocket plane", "Chaos shield effect",
            // 300..309
            // "Modify collision behavior", "Critical hit bonus", "Can use any item",
            // "Backstab every hit", "Mass raise dead", "Off-hand THAC0 bonus", "Main hand THAC0 bonus",
            // "Tracking", "Immunity to tracking", "Set local variable",
            // 310..
            // "Immunity to time stop", "Wish", "Immunity to sequester", "High-level ability",
            // "Stoneskin protection", "Remove animation", "Rest", "Haste 2", "Ex: Set stat"};

            default:
                break;
            }
        }
    }

    // apply filter
    for (int i = 0; i < MAXICONS; i++) {
        if (gFilter_Buf[i] == true) {
            Array[i].TextRef = 0;
        }
    }

    // remove doubles or low priorities
    if (Array[43 + 64].TextRef && Array[0 + 64].TextRef != 0) {     // Domination && Charm
        Array[0 + 64].TextRef = 0;                                  // Charm=0
    }

    if (Array[107 + 64].TextRef && 
        (Array[121 + 64].TextRef != 0 || Array[122 + 64].TextRef != 0)) {   // Spell Immunity && *Globe of Inv
        Array[107 + 64].TextRef = 0;                                        // Spell Immunity=0
    }
        
    if (Array[107 + 64].TextRef &&                                  // Spell Immunity && Immunity by schools
            (Array[-3  + 64].TextRef != 0 ||
            Array[-4  + 64].TextRef != 0 ||
            Array[-5  + 64].TextRef != 0 ||
            Array[-6  + 64].TextRef != 0 ||
            Array[-7  + 64].TextRef != 0 ||
            Array[-8  + 64].TextRef != 0 ||
            Array[-9  + 64].TextRef != 0 ||
            Array[-10 + 64].TextRef != 0) ) {
            Array[107 + 64].TextRef = 0;                            // Spell Immunity=0
    }

    if (Array[121 + 64].TextRef && Array[122 + 64].TextRef != 0) {  // Globe of Inv && Minor Globe of Inv
        Array[122 + 64].TextRef = 0;                                // Minor Globe of Inv=0
    }

    if (Array[13 + 64].TextRef && Array[129 + 64].TextRef != 0) {   // Hold && Webbed
        Array[13 + 64].TextRef = 0;                                 // Hold=0
    }

    // output to console
    for (int i = 0; i < MAXICONS; i++) {
        if (Array[i].TextRef !=0) {
            CStrRef  TlkString;
            IECString text;

            g_pChitin->m_TlkTbl.GetTlkString(Array[i].TextRef, TlkString);
            if (    Array[i].Ticks == -1        ||      // unlimited
                    Array[i].Ticks == -2        ||      // hide enemy time
                    Array[i].Ticks > (60 * 60)  ||      // more 60 min 
                    !ShowTicks ) {                      // "no timing" mode

                if (i == 107 + 64) {        // Spell Immunity
                    CStrRef  TlkString2;
                    g_pChitin->m_TlkTbl.GetTlkString(7192, TlkString2); // "level"
                    text.Format("%s < %d %s", (LPCTSTR)TlkString.text, Array[i].Optional + 1, (LPCTSTR)TlkString2.text);
                } else {
                    if (Array[i].Ticks == -2) {
                        text.Format("%s (*)", (LPCTSTR)TlkString.text);
                    } else {
                        text.Format("%s", (LPCTSTR)TlkString.text);
                    }
                }

            } else {                            // limited

                int mins = Array[i].Ticks / 60;
                int secs = Array[i].Ticks % 60;
                if (mins == 0) {
                    if (i == 107 + 64) {        // Spell Immunity
                        CStrRef  TlkString2;
                        g_pChitin->m_TlkTbl.GetTlkString(7192, TlkString2); // "level"
                        text.Format("%s (%ds) < %d %s", (LPCTSTR)TlkString.text, secs, Array[i].Optional + 1, (LPCTSTR)TlkString2.text);
                    } else {
                        text.Format("%s (%ds)", (LPCTSTR)TlkString.text, secs);
                    }
                } else {
                    if (i == 107 + 64) {        // Spell Immunity
                        CStrRef  TlkString2;
                        g_pChitin->m_TlkTbl.GetTlkString(7192, TlkString2); // "level"
                        text.Format("%s (%dm:%02ds) < %d %s", (LPCTSTR)TlkString.text, mins, secs, Array[i].Optional + 1, (LPCTSTR)TlkString2.text);
                    } else {
                        text.Format("%s (%dm:%02ds)", (LPCTSTR)TlkString.text, mins, secs);
                    }
                }

            }
            if (Array[i].LoopMode == 1) {   // Spell loop
                g_pChitin->pScreenWorld->PrintToConsole(sOwner, text, colorOwner, colorTextSpells, -1, 0); 
            } else {                        // Item loop
                g_pChitin->pScreenWorld->PrintToConsole(sOwner, text, colorOwner, colorTextItems,  -1, 0); 
            }
        }
    }

    // created temp weapons
    // BLAKBLAD.ITM 

    // empty separator line
    //IECString textempty;
    //g_pChitin->pWorld->PrintToConsole(sOwner, textempty, colorOwner, colorTextSpells, -1, 0); 
}


void
LogActiveBuffs_AddFilterElement(int const element_start, int const element_end)
{
    int     n;
    char    Buf[4]; // '255'+ end null

    if ((element_end - element_start + 1) > 3) {
        return;}

    memcpy(Buf, & gIniFilter_Buf[element_start], element_end - element_start + 1);
    Buf[element_end - element_start + 1] = '\0';
    sscanf_s(Buf, "%d", &n);
    gFilter_Buf[n + 64] = true;
}


void
LogActiveBuffs_ParseIniSort()
{
    int i, element_start, element_end;

    element_start = 0;
    for (i = 0; gIniFilter_Buf[i] != '\0'; i++ ) {
        if (gIniFilter_Buf[i] != ','  &&
            gIniFilter_Buf[i] != ';'  &&
            gIniFilter_Buf[i] != ' '  &&
            gIniFilter_Buf[i] != '-'  &&
            (gIniFilter_Buf[i] <  '0' || gIniFilter_Buf[i] >  '9')) {
                element_start++;
                continue;
        }

        if (gIniFilter_Buf[i] == ','  ||
            gIniFilter_Buf[i] == ';'  ||
            gIniFilter_Buf[i] == ' ') {
                element_end     = i - 1;
                if (element_start <= element_end) { // no double separators
                    LogActiveBuffs_AddFilterElement(element_start, element_end);
                }
                element_start   = i + 1;
        }
    }

    // last one
    if (gIniFilter_Buf[i] == '\0' ||
        gIniFilter_Buf[i] == ','  ||
        gIniFilter_Buf[i] == ';'  ||
        gIniFilter_Buf[i] == ' ') {
            element_end     = i - 1;
            if (element_start <= element_end) {
                LogActiveBuffs_AddFilterElement(element_start, element_end);
            }
    }
}


extern void GetTweakIniStrValue(LPCTSTR szSection, LPCTSTR szKey, LPSTR sResult, DWORD SizeResult);


void
LogActiveBuffs_Init()
{
    memset(gIniFilter_Buf, 0, sizeof(gIniFilter_Buf));
    memset(gFilter_Buf,    0, sizeof(gFilter_Buf));

    GetTweakIniStrValue("Tweak", "UI:Log Active Buffs Filter List", gIniFilter_Buf, sizeof(gIniFilter_Buf));
    if (strlen(gIniFilter_Buf) != 0)  {
        LogActiveBuffs_ParseIniSort();
    }
}


void __declspec(naked)
CreatureRMClick_asm()
{
__asm {
    push    eax
    push    ecx

    mov     ecx, [ebp-38h]
    push    [ecx+242h]      ; ObjID
    push    [ebp-14h]       ; CCreature
    call    CreatureRMClick__LogActiveBuffs

    pop     ecx
    pop     eax

    mov     edx, [ecx]      ; stolen bytes
    mov     ecx, [ebp-14h]
    ret
}
}