#include "RenderPortrait.h"

short gTabPressed = 0;

// Shortcut toggles
uint  gStaticIconsMode = 1;
bool  gEnableShowHP    = false;

bool RenderPortrait_IsAllowedScreen() {
    CEngine*   pCurrentScreen = g_pChitin->pEngineActive;

    if (pCurrentScreen  == (CEngine*) g_pChitin->pScreenWorld       ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenInventory   ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenMap         ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenCharacter   ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenWizSpell    ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenPriestSpell ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenJournal     ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenOptions     ||
        pCurrentScreen  == (CEngine*) g_pChitin->pScreenStore      )
        return true;
    else
        return false;
}


void __stdcall
RenderPortrait(CCreatureObject& Cre, RECT& rInside, RECT *rClipIn, int bDoubleResolution ) {
    static CVidCell VidCell;
    static ResRef   Font;
    static int      ggg;
    bool            showHP= false, showAction=false;

    // crash test
    // char *p = NULL;
    // *p = 0;

    if (Font.IsEmpty()) {
        Font = "DIGITFNT";
        VidCell.bam.SetResRef(&Font, 1, 1);
        VidCell.bam.pBam->Demand();               // never released
        VidCell.bam.pBam->m_bCopyResData = TRUE;
        VidCell.bDoubleResolution = bDoubleResolution;
    }

    if (!RenderPortrait_IsAllowedScreen())
        return;

    if (gTabPressed) {
        showHP = true;
        showAction = true;
    }

    if (!pGameOptionsEx)    // if tobex is destroyed on exit
        return;

    if (pGameOptionsEx->bUI_ShowHitPointsOnPortrait_Always ||
        gEnableShowHP) {
        showHP = true;
    }

    if (pGameOptionsEx->bUI_ShowActionOnPortrait_Always) {
        showAction = true;
    }

    if (pGameOptionsEx->bUI_ShowHitPointsOnPortrait && showHP) {
        short CurrentHP = Cre.BaseStats.currentHP;
        short MaxHP     = Cre.cdsCurrent.maxHP;
        CSize FrameSize;

        if ((Cre.cdsCurrent.stateFlags & 0x800 ) || CurrentHP <= 0 )    // dead creature
            return;
        else {
            IECString  String;
            String.Format("%d/%d", CurrentHP, MaxHP);
            int Strlen = String.GetLength();
            int StrlenPixels = 0;

            for (int i = 0; i < Strlen; i++)
            {   
                char chr = String.GetAt(i);
                VidCell.SequenceSet(chr - 1);
                VidCell.FrameSet(0);
                VidCell.GetCurrentFrameSize(&FrameSize, 1);
                StrlenPixels += FrameSize.cx;
            }

            #define SHIFT_DIGITS_X     0
            #define SHIFT_DIGITS_Y     1
            #define REAL_PANEL_X      -3
            #define REAL_PANEL_Y      +2

            int Y = rInside.top + ((1 + bDoubleResolution) * SHIFT_DIGITS_Y);
            int X_Center = rInside.left + (rInside.right - rInside.left)/2;
            int X = X_Center - (StrlenPixels/2) + ((1 + bDoubleResolution) * SHIFT_DIGITS_X);
            RECT NewRect=rInside;
            NewRect.left  += (1 + bDoubleResolution) * REAL_PANEL_X;
            NewRect.right += (1 + bDoubleResolution) * REAL_PANEL_Y;

            for (int i = 0; i < Strlen; i++)
            {   
                char chr = String.GetAt(i);
                VidCell.SequenceSet(chr - 1);
                VidCell.FrameSet(0);
                VidCell.GetCurrentFrameSize(&FrameSize, 1);
                VidCell.Render(0, X, Y, &NewRect, 0, 0, 0, -1);

                X += FrameSize.cx;
            }
        }
    } // showHP    

    if (pGameOptionsEx->bUI_ShowActionOnPortrait && showAction) {
        Action* action = & Cre.currentAction;
        ResRef IconRes = "";

        //////////////////////////////
        //#ifdef _DEBUG
        //        if ( Cre.queuedActions.GetCount() > 0) {
        //            Action* action1 = (Action*) Cre.queuedActions.GetHead();
        //            //console.writef("Render() action= %d Queueaction= %d \n", action->opcode, action1->opcode);
        //        } else {
        //            if (action->opcode) {
        //                //console.writef("Render() action= %d Queueaction empty \n", action->opcode);
        //            }
        //        }
        //#endif
        //////////////////////////////

        if (action->opcode == 0 && Cre.queuedActions.GetCount() > 0) {
            action = (Action*) Cre.queuedActions.GetHead();
        }

        if (!action || action->opcode == 0)
            return;

        short opcode = action->opcode;
        switch ( opcode ) {
         case ACTION_ATTACK:
         case ACTION_GROUPATTACK:
         case ACTION_ATTACKNOSOUND:
         case ACTION_ATTACKONEROUND:
         case ACTION_ATTACKREEVALUATE:

         case ACTION_SPELL:
         case ACTION_SPELL_POINT:
         case ACTION_SPELL_NO_DEC:
         case ACTION_SPELL_POINT_NO_DEC:
         case ACTION_FORCESPELL:
         case ACTION_FORCESPELL_POINT:
         case ACTION_USEITEM:
         case ACTION_USEITEM_POINT:

            CVidCell VidCell2;
            POINT    FrameCenter;
            CSize    FrameSize;
            if (opcode == ACTION_SPELL              ||
                opcode == ACTION_SPELL_POINT        ||
                opcode == ACTION_SPELL_NO_DEC       ||
                opcode == ACTION_SPELL_POINT_NO_DEC ||
                opcode == ACTION_FORCESPELL         ||
                opcode == ACTION_FORCESPELL_POINT) {
                    //IECString   SpellName;
                    //if (action->sName1.IsEmpty()) {
                    //    Cre.GetSpellIdsName(action->m_specificID, &SpellName);
                    //} else {
                    //    SpellName = action->sName1;
                    //}
                    //
                        //console.writef("sName1=%s sName2=%s SpellName=%s\n",
                        //    (LPCTSTR)action->sName1,
                        //    (LPCTSTR)action->sName2,
                        //    (LPCTSTR)SpellName);
                    //

                    //ResRef      SpellName = action->sName1;
                    //ResSplContainer Spell(SpellName);
                if (!action->sName1.IsEmpty()) {
                    ResSplContainer Spell(action->sName1);
                    Spell.Demand();
                    SplFileAbility* pAbility = Spell.GetSpellAbility(0);
                    if (pAbility) {
                        IconRes = pAbility->memIcon;                
                    } else {
                        console.writef("error: spell %s has empty ability0 \n", (LPCTSTR) action->sName1);
                    }
                    Spell.Release();
                }
            }

            if (opcode == ACTION_USEITEM  ||
                opcode == ACTION_USEITEM_POINT) {
                if (action->m_specificID != -1) {
                    CItem& Item = * Cre.Inventory.items[action->m_specificID];
                    if (&Item) {
                        Item.Demand();
                        IconRes = Item.itm.pRes->pFile->m_rItemIcon;
                        Item.Release();
                    }
                }
            }

            if (pGameOptionsEx->bUI_ShowActionOnPortraitWeapon) {
                if (opcode ==  ACTION_ATTACK ||
                    opcode ==  ACTION_GROUPATTACK ||
                    opcode ==  ACTION_ATTACKNOSOUND ||
                    opcode ==  ACTION_ATTACKONEROUND ||
                    opcode ==  ACTION_ATTACKREEVALUATE) {
                    CItem&  Item = * Cre.Inventory.items[Cre.Inventory.nSlotSelected];
                    if (&Item) {
                        short   launcherSlot;
                        Item.Demand();

                        ItmFileAbility* ItemAbility = Item.GetAbility(Cre.Inventory.nAbilitySelected);
                        if (ItemAbility) {
                            CItem* ItemLauncher = Cre.GetFirstEquippedLauncherOfAbility(ItemAbility, launcherSlot);
                            if (ItemLauncher) {
                                ItemLauncher->Demand();
                                IconRes = ItemLauncher->itm.pRes->pFile->m_rItemIcon;
                                ItemLauncher->Release();
                            } else {
                                //IconRes = ItemAbility->useIcon;
                                IconRes = Item.itm.pRes->pFile->m_rItemIcon;
                            }
                        } else {
                            IconRes = Item.itm.pRes->pFile->m_rItemIcon;
                        }

                        Item.Release();
                    }
                }
            }
            
            if (IconRes.IsEmpty()) {
                break;
            }

            IconRes.MakeUpper();
            VidCell2.bam.SetResRef(&IconRes, 1, 1);

            if (!VidCell2.bam.pBam)
                break; // non exist icon

            VidCell2.bam.pBam->m_bCopyResData = TRUE;
            VidCell2.bDoubleResolution = bDoubleResolution;
            VidCell2.bam.pBam->Demand();
            VidCell2.SequenceSet(1);
            VidCell2.FrameSet(0);
            VidCell2.GetCurrentCenterPoint(&FrameCenter, 1);

            #define REAL_PANELACTION_X  -6
            #define SHIFT_ACTION_X      -6
            #define SHIFT_ACTION_Y      20

            RECT NewRect = rInside;
            int X = NewRect.left;
            int Y = NewRect.top;
            X += FrameCenter.x;
            Y += FrameCenter.y;
            X += (1 + bDoubleResolution) * SHIFT_ACTION_X;
            Y += (1 + bDoubleResolution) * SHIFT_ACTION_Y;
            NewRect.left += (1 + bDoubleResolution) * REAL_PANELACTION_X;
            VidCell2.Render(0, X, Y, &NewRect, 0, 0, 0, -1);

            VidCell2.bam.pBam->Release();
            break;
        } // switch
    } // showAction    
}


void __stdcall
TabPressed (short State, short Key) {

    if (Key != 0x59)        // 'TAB' key
        return;
    else {
        State &= 0x8000;    // Pressed
        if (gTabPressed == State)   // no changes
            return;
        else
            gTabPressed = State;

        // pressed/unpressed
        if (RenderPortrait_IsAllowedScreen()) {
            CPanel& Panel  = g_pChitin->pEngineActive->manager.GetPanel(1);   // Portrait Panel
            Panel.SetRedraw(NULL);    // force redraw
        }
    }
}


void
RenderPortrait_SetCurrentAction(CCreatureObject& Cre, short prev_opcode) {
    if (!Cre.statistics.bInParty)
        return;

    if (!RenderPortrait_IsAllowedScreen())
        return;

    ////////////////////////////
    //#ifdef _DEBUG
    //    if (prev_opcode || Cre.currentAction.opcode)
    //        console.writef("SetCurrentAction oldopcode= %d newopcode= %d \n", prev_opcode, Cre.currentAction.opcode);
    //#endif
    ////////////////////////////

    short current_opcode = Cre.currentAction.opcode;

    if ( current_opcode == ACTION_NO_ACTION &&
             (prev_opcode == ACTION_SPELL               ||
              prev_opcode == ACTION_SPELL_POINT         ||
              prev_opcode == ACTION_SPELL_NO_DEC        ||
              prev_opcode == ACTION_SPELL_POINT_NO_DEC  ||
              prev_opcode == ACTION_FORCESPELL          ||
              prev_opcode == ACTION_FORCESPELL_POINT    ||
              prev_opcode == ACTION_USEITEM             ||
              prev_opcode == ACTION_USEITEM_POINT) ) {
                //clear prev action

                //////////////////////////////
                //#ifdef _DEBUG
                //        console.writef("Panel SetRedraw, clear prev action \n");
                //#endif
                //////////////////////////////

                CPanel& Panel  = g_pChitin->pEngineActive->manager.GetPanel(1);
                Panel.SetRedraw(NULL);
    } else {
        //new action
        switch ( current_opcode ) {
            case ACTION_SPELL:
            case ACTION_SPELL_POINT:
            case ACTION_SPELL_NO_DEC:
            case ACTION_SPELL_POINT_NO_DEC:
            case ACTION_FORCESPELL:
            case ACTION_FORCESPELL_POINT:
            case ACTION_USEITEM:
            case ACTION_USEITEM_POINT:

            //////////////////////////////
            //#ifdef _DEBUG
            //        console.writef("Panel SetRedraw, new action \n");
            //#endif
            //////////////////////////////

            CPanel& Panel  = g_pChitin->pEngineActive->manager.GetPanel(1);
            Panel.SetRedraw(NULL);
            break;
        }
    }
}


IECPtrList    gIconList;                  // New icon list

void
AddIcon(int Icon) {
    if (!gIconList.Find((void*)Icon))   // no doubles
        gIconList.AddTail((void*) Icon);
}

void
AddPortraitIcons(CCreatureObject& Cre, int Icon) {
    if (Cre.PortraitIcons.Find((void*)Icon))    // if exist
        if (!gIconList.Find((void*)Icon))       // no doubles
            gIconList.AddTail((void*) Icon);
}

extern void PlayToggleSound(bool On_Off);

void ToggleStaticIcons() {
    gStaticIconsMode++;
    if (gStaticIconsMode > 2) {
        gStaticIconsMode = 0;
        PlayToggleSound(false);
    } else {
        PlayToggleSound(true);
    }

    // pressed/unpressed
    if (RenderPortrait_IsAllowedScreen()) {
        CPanel& Panel  = g_pChitin->pEngineActive->manager.GetPanel(1);   // Portrait Panel
        Panel.SetRedraw(NULL);    // force redraw
    }
}


void ToggleShowHPPortrait() {
    gEnableShowHP = !gEnableShowHP;

    if (gEnableShowHP)
        PlayToggleSound(true);
    else
        PlayToggleSound(false);

    // pressed/unpressed
    if (RenderPortrait_IsAllowedScreen()) {
        CPanel& Panel  = g_pChitin->pEngineActive->manager.GetPanel(1);   // Portrait Panel
        Panel.SetRedraw(NULL);    // force redraw
    }
}


POSITION __stdcall
RenderIcons(CCreatureObject& Cre) {
    if (gStaticIconsMode == 0) {
        return Cre.PortraitIcons.GetHeadPosition();
    }

    if ((Cre.cdsCurrent.stateFlags & 0x800 ) || Cre.BaseStats.currentHP <= 0 )    // dead
        return gIconList.GetHeadPosition(); // empty list

    if (!RenderPortrait_IsAllowedScreen())
        return gIconList.GetHeadPosition(); // empty list

    CEffectList& SpellEffectList = Cre.GetMainEffectsList();
    POSITION pos_spells = SpellEffectList.GetHeadPosition();
    CEffect*    pEff = NULL;

    Action* QueuedAct = NULL;
    if (Cre.queuedActions.GetCount() > 0)
        QueuedAct = (Action*)Cre.queuedActions.GetHead();

    while (pos_spells) {
        pEff = (CEffect *) SpellEffectList.GetNext(pos_spells);
        if (pEff == NULL) {
            continue;
        }

        EffFileData&    effect = pEff->effect;
        int             IconRef;
        int OpCode    = effect.nOpcode;
        int ticks     = ((int)effect.nDuration - (int)g_pChitin->pGame->m_WorldTimer.nGameTime) / 15; // x/TICKS_PER_SECOND

        if ((ticks <= 0 && effect.nDuration != 0) ||    // effect is out
             (effect.nDuration == 0 &&                  // persistent effect

             // show negative effects even with infinite duration
             OpCode != 4   &&   //  Berserk
             OpCode != 24  &&   //  Panic
             OpCode != 25  &&   //  Poison
             OpCode != 38  &&   //  Silence
             OpCode != 39  &&   //  Sleep
             OpCode != 40  &&   //  Slow
             OpCode != 45  &&   //  Stun
             OpCode != 74  &&   //  Blindness
             OpCode != 76  &&   //  Feeblemindedness
             OpCode != 78  &&   //  Disease
             OpCode != 80  &&   //  Deafness
             OpCode != 109 &&   //  Paralyze
             OpCode != 135 &&   //  Polymorph
             OpCode != 142 &&   //  Display Icon
             OpCode != 154 &&   //  Entangle overlay
             OpCode != 158 &&   //  Grease overlay
             OpCode != 175 &&   //  Hold creature
             OpCode != 185 &&   //  Hold creature 2
             OpCode != 211 &&   //  Imprisonment
             OpCode != 213 &&   //  Maze
             OpCode != 216      //  Level drain
             )          
           )
            continue;
        
        if (Cre.cdsCurrent.stateFlags & STATE_IMPROVEDINVISIBILITY)  
            if (pGameOptionsEx->bEff_ImprovedInvisibilityIcon)
                AddIcon(109);    // Ex Find Traps

        //if (Cre.cdsCurrent.stateFlags & STATE_INVISIBLE)  
        //    if (pGameOptionsEx->bEff_ImprovedInvisibilityIcon)
        //        AddIcon(114);    // Ex Infravision


        IconRef = effect.nParam2;
        if (OpCode == 142) {            // 142 - Display Icon
            if (effect.nDuration != 0)   
                AddIcon(IconRef);
            else                        // show negative icons even with infinite duration
                if (IconRef == 0   ||   // Charm
                    IconRef == 1   ||   // Dire Charm
                    IconRef == 2   ||   // Rigid Thinking
                    IconRef == 3   ||   // Confused
                    IconRef == 4   ||   // Berserk
                    IconRef == 5   ||   // Intoxicated
                    IconRef == 6   ||   // Poisoned
                    IconRef == 7   ||   // Diseased
                    IconRef == 8   ||   // Blind
                    IconRef == 13  ||   // Held
                    IconRef == 14  ||   // Sleep
                    IconRef == 33  ||   // Bad luck
                    IconRef == 34  ||   // Silenced
                    IconRef == 35  ||   // Cursed
                    IconRef == 36  ||   // Panic
                    IconRef == 39  ||   // Fatigue
                    IconRef == 41  ||   // Slow
                    IconRef == 43  ||   // Domination
                    IconRef == 44  ||   // Hopelessness
                    IconRef == 45  ||   // Malison
                    IconRef == 48  ||   // Feebleminded
                    IconRef == 51  ||   // Dying
                    IconRef == 53  ||   // Energy Drain
                    IconRef == 54  ||   // Polymorph Self
                    IconRef == 55  ||   // Stun
                    IconRef == 59  ||   // Energy Drain
                    IconRef == 72  ||   // Tensor's Transformation
                    IconRef == 78  ||   // Maze
                    IconRef == 79  ||   // Imprisonment
                    IconRef == 83  ||   // Spell Failure
                    IconRef == 86  ||   // Intelligence drained
                    IconRef == 91  ||   // Ability Score Drained
                    IconRef == 94  ||   // Magnetized
                    IconRef == 101 ||   // Decaying
                    IconRef == 102 ||   // Acid
                    IconRef == 105 ||   // Miscast Magic
                    IconRef == 112 ||   // Deaf
                    IconRef == 113 ||   // Enfeebled
                    IconRef == 124 ||   // Polymorphed
                    IconRef == 129 ||   // Webbed
                    IconRef == 130 ||   // Unconscious
                    IconRef == 137 ||   // Bleeding
                    IconRef == 144 ||   // Entangled
                    IconRef == 145      // Grease
                    )
                        AddIcon(IconRef);
        } else {
            if (gStaticIconsMode == 1 ||// Effects with embedded icons:
                OpCode == 16  ||        // Haste
                OpCode == 317 ||        // Improved Haste
                OpCode == 39  ||        // Sleep
                OpCode == 40  ||        // Slow
                OpCode == 45  ||        // Stun
                OpCode == 175 ||        // Hold creature
                OpCode == 185 ||        // Hold creature 2
                OpCode == 198 ||        // Reflect specified effect
                OpCode == 199 ||        // Reflect spell level
                OpCode == 200 ||        // Spell turning
                OpCode == 201 ||        // Spell deflection
                OpCode == 202 ||        // Reflect spell school
                OpCode == 203 ||        // Reflect spell type
                OpCode == 210 ||        // Power word, stun
                OpCode == 213 ||        // Maze
                OpCode == 216 ||        // Level drain
                OpCode == 218 ||        // Stoneskin
                OpCode == 223 ||        // Spell school deflection
                OpCode == 226 ||        // Spell type deflection
                OpCode == 227 ||        // Spell school turning
                OpCode == 228 ||        // Spell type turning
                //OpCode == 232 ||        // Cast spell on condition, see add non-spell icons
                OpCode == 246 ||        // Berserk effect
                //OpCode == 256 ||        // Spell sequencer, see add non-spell icons
                OpCode == 259 ||        // Spell trap
                OpCode == 299 ||        // Chaos shield effect
                OpCode == 314           // Stoneskin Golem
                ) {
                // effects without "display icon" opcode
                switch (OpCode) {
                // 0..9
                // "AC bonus", "Modify attacks per round", "Cure sleep", "Berserk", "Cure berserk",
                // "Charm creature", "Charisma bonus", "Set color", "Set color glow solid",
                // "Set color glow pulse",
                case 3:     // "Berserk"
                    AddIcon(4);            // 4 Berserk
                    break;
                case 5:     // "Charm creature"
                    if (effect.nParam2 == 2    ||
                        effect.nParam2 == 3    ||
                        effect.nParam2 == 1002 ||
                        effect.nParam2 == 1003 ) {
                        if (effect.nParam2 == 1002 ||
                            effect.nParam2 == 1003 ) 
                            AddIcon(43);        // 43 Domination
                        else
                            AddIcon(1);         // 1 Dire Charm
                    } else
                        AddIcon(0);             // 0 Charm

                    break;
                // 10..19
                // "Constitution bonus", "Cure poison", "Damage", "Kill target", "Defrost",
                // "Dexterity bonus", "Haste", "Current HP bonus", "Maximum HP bonus", "Intelligence bonus",
                case 16:    // "Haste"
                case 317:   // "Haste 2"
                    if (effect.nParam2 == 0 || effect.nParam2 == 2) {  // normal haste
                        AddIcon(38);      // 38 Haste
                    }
                    if (effect.nParam2 == 1) {  // improved haste
                        AddIcon(110);     // 110 Improved Haste
                    }
                    break;
                // 20..29
                // "Invisibility", "Lore bonus", "Luck bonus", "Reset morale", "Panic", "Poison",
                // "Remove curse", "Acid resistance bonus", "Cold resistance bonus",
                // "Electricity resistance bonus",
                case 20:   // "Invisibility"
                    if (effect.nParam2 == 1) {  // Improved Invisibility
                        if (pGameOptionsEx->bEff_ImprovedInvisibilityIcon)
                            AddIcon(109);    // Ex Find Traps
                    }
                    break;
                case 24:    // "Panic"
                    AddIcon(36);          // 36 Panic
                    break;
                case 25:    // "Poison"
                    AddIcon(6);            // 6 Poisoned
                    break;
                // 30..39
                // "Fire resistance bonus", "Magic damage resistance bonus", "Raise dead",
                // "Save vs. death bonus", "Save vs. wand bonus", "Save vs. polymorph bonus",
                // "Save vs. breath bonus", "Save vs. spell bonus", "Silence", "Sleep",
                case 38:    // "Silence"
                    AddIcon(34);          // 34 Silence
                    break;
                case 39:    // "Sleep"
                    AddIcon(14);          // 14 Sleep
                    break;
                // 40..49
                // "Slow", "Sparkle", "Bonus wizard spells", "Stone to flesh", "Strength bonus", "Stun",
                // "Cure stun", "Remove invisibility", "Vocalize", "Wisdom bonus",
                case 40:    // "Slow"
                    AddIcon(41);          // 41 Slow
                    break;
                case 44:    // "Strength bonus"
                case 97:    // "Exceptional strength bonus"
                    if (effect.nParam1 > 0)   // no penalty bonus
                        AddIcon(21);          // 21 Strength
                    break;
                case 45:    // "Stun"
                    AddIcon(55);          // 14 Stun
                    break;
                case 48:    // "Vocalize"
                    AddIcon(103);        // 14 Vocalize
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
                    //AddIcon(105);   // 105 Miscast Magic
                    break;
                case 63:    // "Infravision"
                    AddIcon(114);   // 114 Infravision
                    break;
                case 65:    // "Blur"
                    if ((pGameOptionsEx->bEff_BlurAndMirrorImage_Icon)) {
                        AddIcon(61);    // TobEx Cloak of Fear
                    }
                    break;
                case 69:    // "Nondetection"
                    AddIcon(31);   // 31 Non-detectable
                    break;
                // 70..79
                // "Remove nondetection", "Change gender", "Change AI type", "Attack damage bonus",
                // "Blindness", "Cure blindness", "Feeblemindedness", "Cure feeblemindedness", "Disease",
                // "Cure disease",
                case 74:    // "Blindness"
                    AddIcon(8);      // 8 Blindness
                    break;
                case 76:    // "Feeblemindedness"
                    AddIcon(48);        // 48 Feeblemindedness, unlimited
                    break;
                case 78:    // "Disease"
                    AddIcon(7);     // 7 Disease
                    break;
                // 80..89
                // "Deafness", "Cure deafness", "Set AI script", "Immunity to projectile",
                // "Magical fire resistance bonus", "Magical cold resistance bonus",
                // "Slashing resistance bonus", "Crushing resistance bonus", "Piercing resistance bonus",
                // "Missile resistance bonus",
                case 80:    // "Deafness"
                    AddIcon(112);     // 112 Deafness
                    break;
                // 90..99
                // "Open locks bonus", "Find traps bonus", "Pick pockets bonus", "Fatigue bonus",
                // "Intoxication bonus", "Tracking bonus", "Change level", "Exceptional strength bonus",
                // "Regeneration", "Modify duration",
                case 98:   // "Regeneration"
                    AddIcon(56);        // 56 Regeneration
                    break;
                // 100..109
                // "Protection from creature type", "Immunity to effect", "Immunity to spell level",
                // "Change name", "XP bonus", "Remove gold", "Morale break", "Change portrait",
                // "Reputation bonus", "Paralyze",
                case 102:   // "Immunity to spell level"
                    //AddIcon(107); // 107 Spell Immunity
                    break;
                case 109:   // "Paralyze"
                    AddIcon(13);         // 13 Hold
                    break;
                // 110..119
                // "Retreat from", "Create weapon", "Remove item", "Equip weapon", "Dither",
                // "Detect alignment", "Detect invisible", "Clairvoyance",  "Show creatures", "Mirror image",
                case 111:   // "Create weapon"
                    //AddIcon();
                    break;
                case 116:   // "Detect invisible"
                    AddIcon(108);               // True Seeing
                    break;
                case 119:   // "Mirror image"
                    // Mirror Image set 119 opcode in .SPL
                    // but in-game opcode is 159
                    break;
                // 120..129
                // "Immunity to weapons", "Visual animation effect", "Create inventory item",
                // "Remove inventory item", "Teleport", "Unlock", "Movement rate bonus", "Summon monsters",
                // "Confusion", "Aid (non-cumulative)",
                case 120:   // "Immunity to weapons"
                    if (effect.nParam2 == 2) {  // weapon type
                        AddIcon(70);     // 70 Immunity to normal weapons
                    }
                    if (effect.nParam2 == 1) {
                        AddIcon(71);     // 71 Immunity to magic weapons
                    }
                    break;
                case 126:   // "Movement rate bonus"
                    if (effect.nParam1 == 0 && effect.nParam2 == 1) // Set = 0
                        AddIcon(13);          // 13 Hold
                    break;
                case 128:   // "Confusion"
                    AddIcon(3);       // 3 Confusion
                    break;
                case 129:   // "Aid"
                    //AddIcon();
                    break;
                // 130..139
                // "Bless (non-cumulative)", "Chant (non-cumulative)", "Draw upon holy might (non-cumulative)",
                // "Luck (non-cumulative)", "Petrification", "Polymorph", "Force visible",
                // "Bad chant (non-cumulative)", "Set animation sequence", "Display string",
                case 130:   // "Bless"
                    AddIcon(17);         // 17 Bless
                    break;
                case 131:   // "Chant"
                    AddIcon(18);         // 18 Chant
                    break;
                case 132:   // "Draw upon holy might"
                    AddIcon(135);       // 135 Draw upon holy might
                    break;
                case 133:   // "Luck"
                    AddIcon(32);       // 32 Luck
                    break;
                case 135:   // "Polymorph"
                    AddIcon(54);          // 54 - Polymorph, Shapeshift*
                    break;
                // 140..149
                // "Casting glow", "Lighting effects", "Display portrait icon", "Create item in slot",
                // "Disable button", "Disable spellcasting", "Cast spell", "Learn spell",
                // "Cast spell at point", "Identify",
                // 150..159
                // "Find traps", "Replace self", "Play movie", "Sanctuary", "Entangle overlay",
                // "Minor globe overlay", "Protection from normal missiles overlay", "Web effect",
                // "Grease overlay", "Mirror image effect",
                case 153:   // "Sanctuary"
                    //AddIcon();
                    break;
                case 154:   // "Entangle overlay"
                    AddIcon(144);        // 144 Entangle     
                    break;
                case 155:   // "Minor globe overlay"
                    if (effect.nSpellLvl > 4) {   // spell level
                        AddIcon(121);    // 121 Globe of Invulnerability
                    } else {
                        AddIcon(122);    // 122 Minor globe of Invulnerability
                    }
                    break;
                case 156:   // "Protection from normal missiles overlay"
                    AddIcon(11);          // 11 Protection from normal missiles
                    break;
                case 157:   // "Web effect"
                    AddIcon(129);        // 129 Webbed
                    break;
                case 158:   // "Grease overlay"
                    AddIcon(145);        // 145 Grease
                    break;
                case 159:   // "Mirror image effect"
                    if ((pGameOptionsEx->bEff_BlurAndMirrorImage_Icon)) {
                        AddIcon(61);    // TobEx Cloak of Fear
                    }
                    break;
                // 160..169
                // "Remove sanctuary", "Remove fear", "Remove paralysis", "Free action",
                // "Remove intoxication", "Pause target", "Magic resistance bonus", "Missile THAC0 bonus",
                // "Remove creature", "Prevent portrait icon",
                case 163:   // "Free action"
                    AddIcon(19);          // 19 Free Action
                    break;
                case 166:   // "Magic resistance bonus"
                    if (effect.nParam1 < 0) {   // Bonus Value
                        AddIcon(106);    // 106 Magic Resistance Lowered
                    } else {
                        AddIcon(63);      // 63 Magic Resistance
                    }
                    break;
                // 170..179
                // "Play damage animation", "Give innate ability", "Remove spell", "Poison resistance bonus",
                // "Play sound", "Hold creature", "Movement rate bonus 2", "Use EFF file",
                // "THAC0 vs. type bonus", "Damage vs. type bonus",
                case 175:   // "Hold creature"
                    AddIcon(13);          // 13 Hold
                    break;
                case 176:   // "Movement rate bonus 2"
                    if (effect.nParam1 == 0 && effect.nParam2 == 1) // Set = 0
                        AddIcon(13);          // 13 Hold
                    break;
                // 180..189
                // "Disallow item", "Disallow item type", "Use EFF file (do not use)",
                // "Use EFF file on equip type", "No collision detection", "Hold creature 2",
                // "Move creature", "Set local variable", "Increase spells cast per round",
                // "Increase casting speed factor",
                case 185:   // "Hold creature 2"
                    AddIcon(13);          // 13 Hold
                    break;
                // 190..199
                // "Increase attack speed factor", "Casting level bonus", "Find familiar",
                // "Invisibility detection", "Ignore dialogue pause", "Drain CON and HP on death",
                // "Disable familiar", "Physical mirror", "Reflect specified effect", "Reflect spell level",*/
                case 193:    // "Invisibility detection"
                    AddIcon(108);               // True Seeing
                    break;
                case 197:   // "Physical mirror"
                    //AddIcon();
                    break;
                case 198:    // "Reflect specified effect"
                case 199:    // "Reflect spell level"
                    AddIcon(67);    // 67 Spell Deflection
                    break;
                // 200..209
                // "Spell turning", "Spell deflection", "Reflect spell school", "Reflect spell type",
                // "Protection from spell school", "Protection from spell type", "Protection from spell",
                // "Reflect specified spell", "Minimum HP", "Power word, kill",
                case 200:   // "Spell turning"
                    AddIcon(65);    // 65 Spell turning
                    break;
                case 201:   // "Spell deflection"
                case 202:   // "Reflect spell school"
                case 203:   // "Reflect spell type"
                    AddIcon(67);    // 67 Spell Deflection
                    break;
                case 204:   // "Protection from spell school"
                    AddIcon(107);   // 107 Spell Immunity
                    break;
                // 210..219
                // "Power word, stun", "Imprisonment", "Freedom", "Maze", "Select spell",
                // "Play visual effect", "Level drain", "Power word, sleep", "Stoneskin effect",
                // "Attack roll penalty",
                case 210:   // "Power word, stun"
                    AddIcon(55);                // 55 Stun
                    break;
                case 211:   // "Imprisonment"
                    AddIcon(79);                // 79 Imprisonment
                    break;
                case 213:   // "Maze"
                    AddIcon(78);                // 78 Maze
                    break;
                case 216:   // "Level drain"
                    AddIcon(53);                // 53 Energy Drain
                    break;
                case 217:   // "Power word, sleep"
                    AddIcon(14);                // 14 Sleep
                    break;
                case 218:   // "Stoneskin effect"
                    AddIcon(80);                // 80 Stoneskin
                    break;
                // 220..229
                // "Remove spell school protections", "Remove spell type protections", "Teleport field",
                // "Spell school deflection", "Restoration", "Detect magic", "Spell type deflection",
                // "Spell school turning", "Spell type turning", "Remove protection by school",
                case 223:   // "Spell school deflection"
                case 226:   // "Spell type deflection"
                    AddIcon(73);    // 73 Spell Shield
                    break;
                case 227:   // "Spell school turning"
                case 228:   // "Spell type turning"
                    AddIcon(67);    // 67 Spell Deflection
                    break;
                // 230..239
                // "Remove protection by type", "Time stop", "Cast spell on condition",
                // "Modify proficiencies", "Create contingency", "Wing buffet", "Project image",
                // "Set image type", "Disintegrate", "Farsight",
                //case 232:   // "Cast spell on condition"
                //    AddIcon(75);    // 75 Contingency Active
                //    break;
                case 236:   // "Project image"
                    AddIcon(77);    // 77 Project image
                    break;
                case 239:   // "Farsight"
                    AddIcon(120);   // 120 Farsight
                    break;
                // 240..249
                // "Remove portrait icon", "Control creature", "Cure confusion", "Drain item charges",
                // "Drain wizard spells", "Check for berserk", "Berserk effect", "Attack nearest creature",
                // "Melee hit effect", "Ranged hit effect",
                case 246:     // "Berserk effect"
                    AddIcon(4);     // 4 Berserk
                    break;
                // 250..259
                // "Maximum damage each hit", "Change bard song", "Set trap", "Set automap note",
                // "Remove automap note", "Create item (days)", "Spell sequencer", "Create spell sequencer",
                // "Activate spell sequencer", "Spell trap",
                //case 256:   // "Spell sequencer"
                //    AddIcon(92);    // 92 Spell Sequencer Active
                //    break; 
                case 259:   // "Spell trap"
                    AddIcon(117);   // 117 Spell Trap
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
                case 299:   // "Chaos shield effect"
                    if (effect.nParam1 < 25)    // Wild surge bonus
                        AddIcon(163);           // 163 Chaos shield
                    else
                        AddIcon(162);           // 162 Improved Chaos shield
                    break;
                // 300..309
                // "Modify collision behavior", "Critical hit bonus", "Can use any item",
                // "Backstab every hit", "Mass raise dead", "Off-hand THAC0 bonus", "Main hand THAC0 bonus",
                // "Tracking", "Immunity to tracking", "Set local variable",
                // 310..
                // "Immunity to time stop", "Wish", "Immunity to sequester", "High-level ability",
                // "Stoneskin protection", "Remove animation", "Rest", "Haste 2", "Ex: Set stat"};
                case 314:   // "Stoneskin protection/Stoneskin Golem"
                    AddIcon(80);                // 80 Stoneskin
                    break;

                default:
                    break;
                }
            }
        }
    }

    // add non-spell icons
    AddPortraitIcons(Cre, 5);     // Intoxication
    AddPortraitIcons(Cre, 39);    // Fatigue
    AddPortraitIcons(Cre, 40);    // Bard Song
    //AddPortraitIcons(Cre, 56);    // Regeneration
    AddPortraitIcons(Cre, 57);    // Perception
    AddPortraitIcons(Cre, 58);    // Master Thievery
    AddPortraitIcons(Cre, 66);    // Repulsing Undead
    AddPortraitIcons(Cre, 75);    // Contingency Active
    //AddPortraitIcons(Cre, 87);    // Regeneration
    AddPortraitIcons(Cre, 88);    // In Dialog
    AddPortraitIcons(Cre, 89);    // In Store
    AddPortraitIcons(Cre, 92);    // Spell Sequencer Active
    AddPortraitIcons(Cre, 96);    // Setting Trap
    //AddPortraitIcons(Cre, 108);   // True Seeing 
    AddPortraitIcons(Cre, 109);   // Detecting Traps
    //AddPortraitIcons(Cre, 115);   // Friends
    AddPortraitIcons(Cre, 133);   // Repulse Undead
    AddPortraitIcons(Cre, 151);   // Magic Flute
    AddPortraitIcons(Cre, 155);   // Avoid Death

    ////////////////////////////////////////////
    // Remove doubles or low priorities

    // Otiluke's Resilient Sphere, remove 70 71 107 63
    if (gIconList.Find((void*) 125)) {
        POSITION pos;

        pos = gIconList.Find((void*) 70);   // Immunity to normal weapons
        if (pos)
            gIconList.RemoveAt(pos);

        pos = gIconList.Find((void*) 71);   // Immunity to magic weapons
        if (pos)
            gIconList.RemoveAt(pos);

        pos = gIconList.Find((void*) 107);  // Spell Immunity
        if (pos)
            gIconList.RemoveAt(pos);

        pos = gIconList.Find((void*) 63);   // Magic Resistance
        if (pos)
            gIconList.RemoveAt(pos);
    }

    // Off/Def Spin, remove Blur
    POSITION pos_blur = gIconList.Find((void*) 61); // Blur
    if (pos_blur) {
        if (gIconList.Find((void*) 84)) {   // Effensive Spin
            gIconList.RemoveAt(pos_blur);
        } else
        if (gIconList.Find((void*) 85)) {   // Defensive Spin
            gIconList.RemoveAt(pos_blur);
        }
    }

    // Web/Entangle/Grease/Defensive Spin, remove Hold
    POSITION pos_hold = gIconList.Find((void*) 13); // Hold
    if (pos_hold) {
        if (gIconList.Find((void*) 129)) {  // Web
            gIconList.RemoveAt(pos_hold);
        } else
        if (gIconList.Find((void*) 144)) {  // Entangle
            gIconList.RemoveAt(pos_hold);
        } else
        if (gIconList.Find((void*) 145)) {  // Grease
            gIconList.RemoveAt(pos_hold);
        } else
        if (gIconList.Find((void*) 85)) {   // Defensive Spin
            gIconList.RemoveAt(pos_hold);
        }
    }

    // Dire Charm && Charm
    if (gIconList.Find((void*) 1)) {     // Dire Charm
        POSITION pos;

        pos = gIconList.Find((void*) 0); // Charm
        if (pos)
            gIconList.RemoveAt(pos);
    }

    // Domination && Charm/Dire Charm
    if (gIconList.Find((void*) 43)) {    // Domination
        POSITION pos;

        pos = gIconList.Find((void*) 0); // Charm
        if (pos)
            gIconList.RemoveAt(pos);

        pos = gIconList.Find((void*) 1); // Dire Charm
        if (pos)
            gIconList.RemoveAt(pos);
    }

    // Righteous Magic/Polymorph vs Strength
    POSITION pos_strength = gIconList.Find((void*) 21); // Strength
    if (pos_strength) {
        if (gIconList.Find((void*) 64)) {     // Righteous Magic
            gIconList.RemoveAt(pos_strength);
        } else
        if (gIconList.Find((void*) 54)) {     // Polymorph
            gIconList.RemoveAt(pos_strength);
        }
    }

    if (pGameOptionsEx->bUI_MoraleBreakIcon) {
        CDerivedStats& cds = Cre.cdsCurrent;
        if (Cre.BaseStats.morale <= cds.moraleBreak ||  // Morale Failure
            Cre.bMoraleBroken) { 
            AddIcon(36); // Panic
        }

        if (Cre.berserkActive &&
            QueuedAct &&
            QueuedAct->opcode == 124   // Berserk-In-Panic opcode
           ) {
            AddIcon(4);  // Berserk
            AddIcon(36); // Panic
        }
    }

    // Maze
    if (gIconList.Find((void*) 78)) {   // Maze
        gIconList.RemoveAll();          // Hide anything
        AddIcon(78);
    }


    // return our list
    return gIconList.GetHeadPosition();
}


void __stdcall
RenderIconsClear() {
    gIconList.RemoveAll();
}


void __declspec(naked)
RenderPortrait_asm() {
__asm
{
    push    eax
    push    ecx
    push    edx

    push    [ebp+20h]       ; bDoubleResolution
    push    [ebp+1Ch]       ; rClipIn
    lea     eax, [ebp-18h]
    push    eax             ; rInside
    push    [ebp-26Ch]      ; CCreatureObject
    call    RenderPortrait

    pop     edx
    pop     ecx
    pop     eax

    mov     edx, [ebp-26Ch]  ; Stolen bytes
    ret
}
}


void __declspec(naked)
TabPressed_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-24h]   ; key 
    push    eax         ; keystate after GetAsyncKeyState()
    call    TabPressed

    pop     edx
    pop     ecx

    movsx   eax, word ptr [ebp-1Ch]  ; Stolen bytes
    test    eax, eax
    ret
}
}


void __declspec(naked)
RenderIcons_asm() {
__asm
{
    push    ecx
    push    edx

    push    [ebp-26Ch]  ; Cre
    call    RenderIcons
    // eax - PortraitIcons PtrList

    pop     edx
    pop     ecx

    ret
}
}


void __declspec(naked)
RenderIconsClear_asm() {
__asm
{
    push    ecx
    push    edx

    call    RenderIconsClear

    pop     edx
    pop     ecx

    cmp     dword ptr [edx+63CEh], 0    // stolen bytes
    ret
}
}