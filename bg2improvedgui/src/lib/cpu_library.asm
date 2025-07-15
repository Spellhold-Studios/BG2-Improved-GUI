    .686P
    .model  flat

stub_adr MACRO ExportName, JmpAdr
PUBLIC  ExportName

    ExportName PROC
        push JmpAdr
        ret
    ExportName ENDP

ENDM

stub_mfc MACRO ExportName, ImportName
PUBLIC  ExportName

    ExportName PROC
        jmp ImportName
    ExportName ENDP

ENDM



_TEXT SEGMENT

; CAnimation::CAnimation
; stub_adr    ??0CAnimation@@QAE@XZ,                                                  07F9211h

; CAnimation::~CAnimation
; stub_adr    ??1CAnimation@@UAE@XZ,                                                  087B4C0h

; bool CScreenMageBook::IsSpellAllowedInContingency(int nLevel, IECString sSpell)
stub_adr    ?IsSpellAllowedInContingency@CScreenMageBook@@QAE_NHVIECString@@@Z,     07BDF3Ch

; bool CScreenMageBook::AddContingencySpell(IECString sSpell)
stub_adr    ?AddContingencySpell@CScreenMageBook@@QAE_NVIECString@@@Z,              07BD3A3h

; void CScreenMageBook::SetSpellContingencyState(IECString sSpell, bool bState)
stub_adr    ?SetSpellContingencyState@CScreenMageBook@@QAEXVIECString@@_N@Z,        07BDA4Dh            

; CArea& CInfGame::GetLoadedArea(IECString sAreaName)
stub_adr    ?GetLoadedArea@CInfGame@@QAEAAVCArea@@VIECString@@@Z,                   069A7D4h

; IECString CRuleTables::GetClassString(unsigned char nClass, unsigned int dwKit)
stub_adr    ?GetClassString@CRuleTables@@QAE?AVIECString@@EI@Z,                     062B072h

; IECString CRuleTables::GetAlignmentString(char align)
stub_adr    ?GetAlignmentString@CRuleTables@@QAE?AVIECString@@D@Z,                  062AEA4h

; IECString CRuleTables::GetRaceString(unsigned char nRace)
stub_adr    ?GetRaceString@CRuleTables@@QAE?AVIECString@@E@Z,                       062ACC5h

; IECString CRuleTable::GetDefaultValue()
stub_adr    ?GetDefaultValue@CRuleTable@@QAE?AVIECString@@XZ,                       043C6B0h

; CRuleTables::CRuleTables()
; stub_adr    ??0CRuleTables@@QAE@XZ,                                                 06213DCh

; CRuleTables::~CRuleTables()
; stub_adr    ??1CRuleTables@@QAE@XZ,                                                 06279D1h

; void CLUAConsole::DisplayText(IECString s)
stub_adr    ?DisplayText@CLUAConsole@@QAEXVIECString@@@Z,                           05ACD00h

; CMessageSetAnimationSequence::~CMessageSetAnimationSequence() 
stub_adr    ??1CMessageSetAnimationSequence@@UAE@XZ,                                04B7250h

; ACTIONRESULT CCreatureObject::ActionJumpToAreaEntranceMove(IECString sArea)
stub_adr    ?ActionJumpToAreaEntranceMove@CCreatureObject@@QAEFVIECString@@@Z,      0953CE9h

; CCreatureObject::CCreatureObject
; stub_adr    ??0CCreatureObject@@QAE@PAXIHHHHIHHH@Z,                                 087FB08h

; CDerivedStats::CDerivedStats(CreFileData& stats, CreFileMemorizedSpellLevel* memArrayMage, CreFileMemorizedSpellLevel* memArrayPriest)
; stub_adr    ??0CDerivedStats@@QAE@AAUCreFileData@@PAUCreFileMemorizedSpellLevel@@1@Z,     046CB40h

; CDerivedStats::CDerivedStats()
; stub_adr    ??0CDerivedStats@@QAE@XZ,                                               046D1B2h

; CVisualEffect::CVisualEffect()
; stub_adr    ??0CVisualEffect@@QAE@XZ,                                               065467Dh

; CStore::CStore(ResRef& rName)
stub_adr    ??0CStore@@QAE@AAVResRef@@@Z,                                           0643D4Bh

; CStore::CStore()
stub_adr    ??0CStore@@QAE@XZ,                                                      0643CB0h

; CStore::~CStore()
stub_adr    ??1CStore@@QAE@XZ,                                                      0643E21h

; CUIButton::~CUIButton()
stub_adr    ??1CUIButton@@UAE@XZ,                                                   05A54D0h

; CUICheckButton::~CUICheckButton()
stub_adr    ??1CUICheckButton@@UAE@XZ,                                              05A4A30h

; CUIControl::CUIControl(CPanel& panel, ChuFileControlInfoBase& controlInfo, BOOL b)
; stub_adr    ??0CUIControl@@QAE@AAUCPanel@@AAUChuFileControlInfoBase@@H@Z,           0582650h

; CUIControl::~CUIControl()
stub_adr    ??1CUIControl@@UAE@XZ,                                                  0586530h

; CUIScrollBar::~CUIScrollBar()
stub_adr    ??1CUIScrollBar@@UAE@XZ,                                                05A1F51h

; IECString CUITextField::GetText()
; stub_adr    ?GetText@CUITextField@@QAE?AVIECString@@XZ,                             0590371h

; CUITextField::~CUITextField()
; stub_adr    ??1CUITextField@@UAE@XZ,                                                05A4D30h

; CUITextArea::~CUITextArea()
; stub_adr    ??1CUITextArea@@UAE@XZ,                                                 059CA2Fh

; CUIScrollBar::CUIScrollBar(CPanel& panel, ChuFileControlInfoBase& controlInfo)
stub_adr    ??0CUIScrollBar@@QAE@AAUCPanel@@AAUChuFileControlInfoBase@@@Z,          05A1940h

; CAnimation5000::CAnimation5000(unsigned short wAnimId, ColorRangeValues& colors, int nOrientation)
; stub_adr    ??0CAnimation5000@@QAE@GAAUColorRangeValues@@H@Z,                       0843678h

; IECMapStringToString::IECMapStringToString(int nBlockSize)
stub_adr    ??0IECMapStringToString@@QAE@H@Z,                                       0A4FB1Fh

; IECMapStringToString::~IECMapStringToString() 
stub_adr    ??1IECMapStringToString@@UAE@XZ,                                        0A4FC01h

; IECPtrList::IECPtrList(int nBlockSize)    
stub_adr    ??0IECPtrList@@QAE@H@Z,             0A4E475h

; IECPtrList::~IECPtrList()
stub_adr    ??1IECPtrList@@UAE@XZ,              0A4E4D4h

; CSound::CSound()
stub_adr    ??0CSound@@QAE@XZ,                  09DCF80h

; CSound::~CSound()
stub_adr    ??1CSound@@UAE@XZ,                  09DD3A8h

; CItem::CItem()
stub_adr    ??0CItem@@QAE@XZ,                   05A83F0h

; CItem(CItem& itm)
stub_adr    ??0CItem@@QAE@AAV0@@Z,              05A84BEh

; CVidCell::CVidCell
stub_adr    ??0CVidCell@@QAE@XZ,                09C9D57h

; CVidCell::~CVidCell
; stub_adr    ??1CVidCell@@QAE@XZ,                09CA225h

; CGameObject::CGameObject
stub_adr    ??0CGameObject@@QAE@XZ,              0573470h

; CGameAIBase::CGameAIBase
stub_adr    ??0CGameAIBase@@QAE@XZ,              0476DEDh

; ResEffContainer::ResEffContainer(ResRef r)
stub_adr    ??0ResEffContainer@@QAE@VResRef@@@Z, 04F2810h

; ResEffContainer::~ResEffContainer
stub_adr    ??1ResEffContainer@@QAE@XZ,          04E27C0h

; CVidCell::CVidCell(ResRef cNewResRef, BOOL bDoubleResolution)
stub_adr    ??0CVidCell@@QAE@VResRef@@H@Z,       09C9E7Dh

_TEXT ENDS
END