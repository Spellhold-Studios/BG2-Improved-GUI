#include "InteractiveJournal.h"

class CUITextAreaCopy : public CUITextArea {
public:
    POSITION AppendInject(  char*           const pQuestText,
                            int             const DescriptionLen,
                            CJournalEntry*  const pEntry,
                            IECString&            sLeft,
                            IECString&            sRight,
                            ABGR            const colLeftIn,
                            ABGR            const colRightIn,
                            int             const nUserArg,
                            bool            const bResetScrollbar );
    void ClearTextInject(CPtrArray* const ChapterList, int const nChapter, CUITextAreaCopy* const CUITextArea);
};


//////////////////////////////////////////////////////////////////////////
// static

JournalEntryBuf  gQuestBuf[1000];    // maybe overkill
int              gQuestIndex = 0;

static FoldEntryBuf     gFoldBuf[1000];     // maybe overkill
static int              gFoldIndex = -1;

static unsigned short   glastJournalPosition[5] = {0,0,0,0,0};
static unsigned char    glastJournalMode = 0;

// static
//////////////////////////////////////////////////////////////////////////



// CRC "Slice-by-8" ripped from some modem software...
const unsigned long crc_32_tab[] = { // CRC polynomial
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

unsigned long crc32(const char* const Ptr, int Len)
{
    register unsigned long crc32;

    crc32 = 0xFFFFFFFF;
    while (Len >= 0)
      {
            crc32 = UPDCRC32(Ptr[Len], crc32);
            Len--;
      }
    return crc32;
}


void
AddQuest(POSITION       const pQuestEntry,
        CJournalEntry*  const pJournalEntry,
        unsigned short  wType,
        const char**    const ppQuestText,
        int             const DescriptionLen) 
{
    unsigned long hash; 

    hash = crc32(*ppQuestText, DescriptionLen - 1);
    hash = UPDCRC32(wType, hash);   // unique hashes for open/done/xxx quests even with same name

    gQuestBuf[gQuestIndex].hash          = hash;
    gQuestBuf[gQuestIndex].pQuestEntry   = pQuestEntry;
    gQuestBuf[gQuestIndex].pJournalEntry = pJournalEntry;
    gQuestBuf[gQuestIndex].wType         = wType;
    gQuestBuf[gQuestIndex].Time          = pJournalEntry->m_nTime;
    gQuestIndex++;
}


void
AddEmpty(POSITION       const pQuestEntry,
        CJournalEntry*  const pJournalEntry,
        unsigned short  wType) 
{
    gQuestBuf[gQuestIndex].hash          = ~0;
    gQuestBuf[gQuestIndex].pQuestEntry   = pQuestEntry;
    gQuestBuf[gQuestIndex].pJournalEntry = pJournalEntry;
    gQuestBuf[gQuestIndex].wType         = wType;
    gQuestBuf[gQuestIndex].Time          = pJournalEntry->m_nTime;
    gQuestIndex++;
}


void
SortInterchange(JournalEntryBuf* const entry1, JournalEntryBuf* const entry2)
{
    int             temphash; 
    POSITION        ptempQuestEntry; 
    CJournalEntry*  ptempJournalEntry;
    unsigned long   temptime;
    unsigned short  tempwType;

    temphash                = entry1->hash;
    ptempQuestEntry         = entry1->pQuestEntry;
    ptempJournalEntry       = entry1->pJournalEntry;
    tempwType               = entry1->wType;
    temptime                = entry1->Time;

    entry1->hash            = entry2->hash;
    entry1->pQuestEntry     = entry2->pQuestEntry;
    entry1->pJournalEntry   = entry2->pJournalEntry;
    entry1->wType           = entry2->wType;
    entry1->Time            = entry2->Time;

    entry2->hash            = temphash;
    entry2->pQuestEntry     = ptempQuestEntry;
    entry2->pJournalEntry   = ptempJournalEntry;
    entry2->wType           = tempwType;
    entry2->Time            = temptime;
}


void
SortByDescriptors()
{
    int top, search, i;

    // sort by date
    for(top = 0; top < gQuestIndex - 1; top++)
        for(search = top + 1; search < gQuestIndex; search++) {
            if ( gQuestBuf[search].Time < gQuestBuf[top].Time &&
                (gQuestBuf[search].pJournalEntry->m_wType == 0x01 ||
                 gQuestBuf[search].pJournalEntry->m_wType == 0x02) ) {
                    SortInterchange(&gQuestBuf[search], &gQuestBuf[top] );
                    continue;
            }
        }

    // sort by hash,time
    for(top = gQuestIndex - 1; top > 0; top--) {
        if (gQuestBuf[top].hash == gQuestBuf[top - 1].hash) {
            continue;
        }

        for(search = top - 1; search >= 0; search--) {
            if (gQuestBuf[search].hash == gQuestBuf[top].hash) {
                for (i = search; i < top - 1; i++) { // shift loop
                    if (gQuestBuf[i].hash != gQuestBuf[i + 1].hash) {
                        SortInterchange(&gQuestBuf[i], &gQuestBuf[i + 1] );
                    } else {
                        break;                       // end shift loop
                    }
                }
            }
        }
    }
}


void
Update_CGameJournal(CPtrList& ChapterList)
{   
    POSITION  pos;
    CNode*    pNode;
    int       Index = 0;

    pos = ChapterList.GetHeadPosition();
    while (pos != NULL) {
        pNode = (CNode *) pos;
        pNode->data = gQuestBuf[Index].pJournalEntry;
        ChapterList.GetNext(pos);
        Index++;
    }
}


bool
IsUnderGroup(const CJournalEntry* const pJournalEntry)
{
    for (int i = 0; i < gQuestIndex ;i++) {
        if (gQuestBuf[i].pJournalEntry == pJournalEntry) {
            if (i == 0) {
                return false;   // first in list
            }

            if (gQuestBuf[i - 1].hash == gQuestBuf[i].hash) {
                return true;    // we are not head of group
            }

            return false;       // head of group
        }
    }

    return false;
}


bool
IsFold(CJournalEntry* const pEntry, IECString& String)
{
    int             i, DescriptionLen;
    unsigned long   hash;
    char*           Ptr;
    char**          sLeft;

    sLeft  = (char **) &String;
    Ptr = *sLeft;

    DescriptionLen = strlen(Ptr);
    hash = crc32(Ptr, DescriptionLen - 1);

    for (i = 0; i <= gFoldIndex ; i++) {
        if (gFoldBuf[i].hash == hash) {
            return false;
        }
    }

    return true;
}


void
InvertFold(TextAreaEntry* const pTextAreaEntry)
{
    int             i, j, DescriptionLen;
    unsigned long   hash;
    char**          sLeft;
    char*           Ptr;

    sLeft  = (char **) &pTextAreaEntry->sLeft;

    if (**sLeft == '>' || **sLeft == '<')
        Ptr = *sLeft + 2;   // skip '> <'
    else
        Ptr = *sLeft;

    DescriptionLen = strlen(Ptr);
    hash = crc32(Ptr, DescriptionLen - 1);

    if (pTextAreaEntry->nRows > 2) {
        // clicked on unfolded description
        for (i = 0; i <= gFoldIndex ; i++) {
            if (gFoldBuf[i].hash == hash) {
                gFoldBuf[i].hash = 0;           // remove from unfold array

                //cleanup
                if (i == gFoldIndex) {
                    gFoldIndex--;

                    for (j = gFoldIndex; j >= 0; j--) {
                            if (gFoldBuf[j].hash == 0)
                                gFoldIndex= j - 1;
                            else
                                break;
                    }
                }       

                return;
            }
        }
    }
    else {
        // clicked on folded description

        // find possible hole
        for (i = 0; i <= gFoldIndex ; i++) {
            if (gFoldBuf[i].hash == 0) {
                gFoldBuf[i].hash = hash;
                return;
            }
        }

        // no holes, insert new
        gFoldIndex++;
        gFoldBuf[gFoldIndex].hash = hash;
    }
}


#ifdef _DEBUG
void
plstStringslog(CTextAreaEntryList* const pList, TextAreaNode* const pLastpos)
{
    POSITION        pos, node;
    TextAreaEntry   *data;

    pos = pList->GetHeadPosition();
    while (pos) {
        node = pos;
        data = (TextAreaEntry *) pList->GetNext(pos);
        IECString sleft= data->sLeft.Left(7);
        IECString sright= data->sRight.Left(7);
        console.writef("Node=0x%x Data=0x%x L=0x%08x R=0x%08x RowIdx=%d BOSS=0x%x Ind=%d nRows=%d Ret=0x%x %s_%s\n",
            &node,
            &data,
            &data->sLeft,
            &data->sRight, 
            &data->wRowIdx,
            &data->posBoss,
            &data->wIndent,
            &data->nRows,
            &pLastpos,
            (LPCTSTR)sleft,
            (LPCTSTR)sright);
    }
}
#endif

void
MergeNodes(
    TextAreaNode* const boss,
    TextAreaNode* const separator,
    TextAreaNode* const lasttextline)
{

    TextAreaNode* const bosstext = (TextAreaNode *)lasttextline->data->posBoss;
    int const totalrows = boss->data->nRows + separator->data->nRows + bosstext->data->nRows;
    short Indx = boss->data->nRows;
    boss->data->nRows = totalrows;

    TextAreaNode* curnode = separator;
    while (curnode) {
         TextAreaNode *nextnode = curnode->pNext;

         // payload
         curnode->data->wRowIdx = Indx;
         curnode->data->posBoss = (POSITION)boss;
         curnode->data->nRows   = 0;
         Indx++;
         //

         curnode = nextnode;
    }
}


#define TEXTCOLORGREEN 0x216021
#define TEXTCOLORRED   0x212180
#define TEXTCOLORBLUE  0x802121
#define TEXTCOLOREMPTY 0x2c2121
#define TEXTCOLORWHITE 0xFFFFFF


POSITION
CUITextAreaCopy::AppendInject(
    char*           const pQuestText,
    int             const DescriptionLen,
    CJournalEntry*  const pQuestEntry,
    IECString&      sLeft,
    IECString&      sRight,
    ABGR            const colLeftIn,
    ABGR            const colRightIn,
    int             const nUserArg,
    bool            const bResetScrollbar)
{
    TextAreaNode    *pos_, *tail, *pretail, *bossnode, *separatornode, *textnode, *headpos;
    IECString* const pEmptyIECString  =  (IECString *) 0xB72010;
    char*      const pEmptyIECString2 =  (char *)      0xB72024;
    //char**           ppEmptyIECString2 =  (char**) &pEmptyIECString2;
    bool             fold;
    bool             UnderGroup;
    int              posEndLine;
    POSITION         pTailPosition;

    UnderGroup = IsUnderGroup(pQuestEntry);
    fold = IsFold(pQuestEntry, sLeft);

    #ifdef _DEBUG
        //console.writef("\n PreAppend \n");
        //plstStringslog(this->m_plstStrings, (TextAreaNode *)this->m_posCurrString);
    #endif

    // remove node with empty text line
    pTailPosition = this->m_plstStrings->GetTailPosition();
    tail    = (TextAreaNode *) pTailPosition;
    
    if (tail && fold) {
        char** dataL = (char **) &tail->data->sLeft;
        char** dataR = (char **) &tail->data->sRight;
        char** datapreR;

        this->m_plstStrings->GetPrev(pTailPosition);
        pretail = (TextAreaNode *) pTailPosition;
        if (pretail) 
            datapreR = (char **) &pretail->data->sRight;
        else
            datapreR = (char**) &pEmptyIECString2;

        if ( *dataL    == pEmptyIECString2 &&
             *dataR    == pEmptyIECString2 &&
             *datapreR == pEmptyIECString2)
        {
            this->m_plstStrings->RemoveTail();

            #ifdef _DEBUG
                //console.writef("\n PostRemoveTail \n");
                //plstStringslog(this->m_plstStrings, (TextAreaNode *)this->m_posCurrString);
            #endif
        }
    }

    if (UnderGroup && fold)                     // hide subevents
        return (POSITION) this->m_plstStrings->GetTailPosition();

    if  (fold) {
        // Head=Blue, No Ticks, No Text
        pos_ = (TextAreaNode *) Append("> " + sLeft, *pEmptyIECString, TEXTCOLORBLUE, colRightIn, nUserArg, bResetScrollbar);       

    } else {
        if (UnderGroup) {
            // No Head, Ticks=Green, Text=Normal
            posEndLine = sRight.FindOneOf("\n\r"); 
            pos_ = (TextAreaNode *) Append(sRight.Left(posEndLine), sRight.Mid(posEndLine), TEXTCOLORGREEN, colRightIn, nUserArg, bResetScrollbar);

        } else {
            // Head=Red
            headpos  = (TextAreaNode *) Append("< " + sLeft, *pEmptyIECString, TEXTCOLORRED, colRightIn, nUserArg, bResetScrollbar);
            bossnode = (TextAreaNode *) this->m_plstStrings->GetTailPosition();

            //separator
            posEndLine  = sRight.FindOneOf("\n\r"); 
            pos_ = (TextAreaNode *) Append(*pEmptyIECString, *pEmptyIECString, TEXTCOLOREMPTY, TEXTCOLOREMPTY, nUserArg, bResetScrollbar);
            separatornode = (TextAreaNode *) this->m_plstStrings->GetTailPosition();

            //Ticks=Green, Text=Normal
            pos_ = (TextAreaNode *) Append(sRight.Left(posEndLine), sRight.Mid(posEndLine), TEXTCOLORGREEN, colRightIn, nUserArg, bResetScrollbar);
            textnode = (TextAreaNode *) this->m_plstStrings->GetTailPosition();

            MergeNodes(bossnode, separatornode, textnode);
            pos_ = bossnode;
        }
    }
    
    #ifdef _DEBUG
        //console.writef("\n PostAppend \n");
        //plstStringslog(this->m_plstStrings, pos_);
    #endif

    return (POSITION) pos_;
}


void 
CUITextAreaCopy::ClearTextInject(
    CPtrArray*  const       m_CGameJournal,
    int         const       nChapter,
    CUITextAreaCopy* const  pCUITextArea)
{
    CRITICAL_SECTION    CriticalSection;
    POSITION            pos_, current_pos;
    CJournalEntry*      pJournalEntry;
    unsigned long       strref;
    CStrRef             TlkString;
    int                 posEndLine;

    // sort journal by quest description
    InitializeCriticalSection(&CriticalSection);
    EnterCriticalSection(&CriticalSection);

    gQuestIndex = 0;
    CPtrList& ChapterList  = *( (CPtrList *) m_CGameJournal->GetAt(nChapter) );

    pos_ = ChapterList.GetHeadPosition();
    while (pos_ != NULL) {
        current_pos = pos_;
        pJournalEntry = (CJournalEntry *) ChapterList.GetNext(pos_);

        if (!(pJournalEntry->m_wType & 0x01) &&   // journal/user records/other
            !(pJournalEntry->m_wType & 0x02)) {
                AddEmpty(current_pos, pJournalEntry, pJournalEntry->m_wType);
                continue;
        }

        strref = pJournalEntry->m_strText;
        g_pChitin->m_TlkTbl.GetTlkString(strref, TlkString);

        posEndLine = TlkString.text.FindOneOf("\n\r");
        if (posEndLine >= 1024)     // stupid overflow protection 
            break;

        AddQuest(current_pos, pJournalEntry, pJournalEntry->m_wType, (const char **) &TlkString.text, posEndLine);
    }

    SortByDescriptors();

    Update_CGameJournal(ChapterList);

    LeaveCriticalSection(&CriticalSection);

    ClearText(); // stolen bytes
}



void __stdcall
CheckClickedNode(
    CUITextAreaCopy* const pCUITextArea,
    TextAreaEntry*   const pLineNode,
    unsigned         const ClickedRow,
    TextAreaNode*    const pCurrentNode,
    TextAreaNode*    const pZeroNode)
{
    TextAreaNode*   pNode;
    const char* const pEmptyIECString2 =  (char *) 0xB72024;
    char** const    sLeft  = (char **) &pLineNode->sLeft;
    char** const    sRight = (char **) &pLineNode->sRight;

    if (ClickedRow > 0)
        pNode = pCurrentNode;
    else
        pNode = pZeroNode;

    #ifdef _DEBUG
        //IECString sleft  = pLineNode->sLeft.Left(10);
        //IECString sright = pLineNode->sRight.Left(10);

        //console.writef("\n Click Row=%d Node=0x%08x %s _ %s \n",
        //                ClickedRow,
        //                &pNode,
        //                (LPCTSTR)sleft,
        //                (LPCTSTR)sright );
        //console.writef("wCurStr=%hd pos=0x%08x u54=%hd u56=%hd ua52=%hd ua54=%hd NewRows=%hd RowsPerPage=%hd NumStrings=%hd \n",
        //    pCUITextArea->wCurrString,
        //    pCUITextArea->m_posCurrString,
        //    pCUITextArea->u54,
        //    pCUITextArea->u56,
        //    pCUITextArea->ua52,
        //    pCUITextArea->ua54,
        //    pCUITextArea->m_nNewRows,
        //    pCUITextArea->wRowsPerPage,
        //    pCUITextArea->wNumStrings
        //    );

        //plstStringslog(pCUITextArea->m_plstStrings, (TextAreaNode *) pCUITextArea->m_posCurrString);
    #endif

    if ( *sRight == pEmptyIECString2 &&
         (pLineNode->rgbLeft == TEXTCOLORRED || pLineNode->rgbLeft == TEXTCOLORBLUE) ) {

        //  clicked on quest head
        short old_wCurrString = pCUITextArea->wCurrString;

        InvertFold(pLineNode);
        g_pChitin->pScreenJournal->UpdateMainPanel();         // will destroy old PtrListArray

        int Count = pCUITextArea->m_plstStrings->GetCount();
        if (Count > pCUITextArea->wRowsPerPage) {
            // set position to previous line & scroll
            POSITION    pos   = pCUITextArea->m_plstStrings->GetHeadPosition();
            int         Index = 0;
            while (pos) {
                POSITION old_pos = pos;
                pCUITextArea->m_plstStrings->GetNext(pos);
                if (Index == old_wCurrString) {
                    pCUITextArea->m_posCurrString = old_pos;
                    //pCUITextArea->wCurrString     = old_wCurrString;
                    break;
                }
                Index++;
            }
            
            pCUITextArea->UpdateScrollBar();
            pCUITextArea->pPanel->SetRedraw(NULL);    // force redraw
            //pCUITextArea->SetRedraw();
        }
    }
}


void __stdcall
FillText( CInfGame::CGameJournal& m_CGameJournal,
          int              const  nChapter,
          CUITextAreaCopy* const  pCUITextArea,
          unsigned char           Mode)

{
    short   old_wCurrString = pCUITextArea->wCurrString;
    bool    Modechanged = false;

    if (Mode > 4)
        Mode = 0;

    glastJournalPosition[glastJournalMode] = old_wCurrString;
    
    if (Mode != glastJournalMode) {
        old_wCurrString = glastJournalPosition[Mode];
    }

    glastJournalMode = Mode;
   
    m_CGameJournal.UpdateTextDisplay(nChapter, pCUITextArea, Mode); // Stolen bytes

    int Count = pCUITextArea->m_plstStrings->GetCount();
    if (Count > pCUITextArea->wRowsPerPage) {
        // set position to previous line & scroll
        POSITION    pos   = pCUITextArea->m_plstStrings->GetHeadPosition();
        int         Index = 0;
        while (pos) {
            POSITION old_pos = pos;
            pCUITextArea->m_plstStrings->GetNext(pos);
            if (Index == old_wCurrString) {
                pCUITextArea->m_posCurrString = old_pos;
                pCUITextArea->wCurrString     = old_wCurrString;
                pCUITextArea->UpdateScrollBar();
                break;
            }
            Index++;
        }
    }

}




#define CONTROLID_SortInfoLabel 10

bool __stdcall
ClearSortingInfoLabel(CEngine& Screen, CPanel& Panel, CUIButton& Button, unsigned char const Mode)
{
    if (Mode == 1 ||    // Quest/Done Quests
        Mode == 2) {
            Button.SetText(IECString(""));
            UpdateLabel(Screen, Panel, 0x10000000 + CONTROLID_SortInfoLabel, "");
            return true;
    }

    return false;
}


//void __cdecl
//CScreenRecord__PrintF_TextArea(CScreenRecord *Screen, CUITextArea *TextArea, char *Format, char *Text, int Value)
//{
//
//    IECString StrEmpty;
//
//    IECString StrLeft = IECString(Text);  //  = IECString(Buf1);
//    //StrLeft.Format("%s: ", Text);
//
//    IECString StrRight = IECString(Text); // = IECString(Buf2);
//    //StrRight.Format("%d", Value);
//
//    //TextArea->Append(StrLeft, StrRight, TextArea->colLeft, TextArea->colRight, -1, 0);
//    TextArea->Append(StrLeft, StrRight, TextArea->colLeft, TEXTCOLORWHITE, 0, 0);
//}



void __declspec(naked)
InteractiveJournal_QuestEntryAdd_asm() {
__asm
{
    push    [esp+  6*4]             ; bResetScrollbar
    push    [esp+  6*4]
    push    [esp+  6*4]
    push    [esp+  6*4]
    push    [esp+  6*4]
    push    [esp+  6*4]             ; sLeft

    push    [ebp-0B8h]                      ; QuestEntry
    push    [ebp-24h]                       ; Description EndOfLine
    push    [ebp-94h]                       ; TextRefString
    call    CUITextAreaCopy::AppendInject

    ret     6*4
}
}

void __declspec(naked)
InteractiveJournal_ClearText_asm() {
__asm
{
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    ecx                             ; CUITextArea
    push    [ebp+8h]                        ; nChapter
    push    [ebp-6ch]                       ; m_CGameJournal
    call    CUITextAreaCopy::ClearTextInject

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    ret
}
}


void __declspec(naked)
InteractiveJournal_FillText_asm() {
__asm
{
    push    [esp+  3*4]     ; Mode
    push    [esp+  3*4]     ; CUITextArea
    push    [esp+  3*4]     ; nChapter

    push    ecx             ; mGameJournal
    call    FillText

    ret
}
}


void __declspec(naked)
InteractiveJournal_CheckClickedNode_asm() {
__asm
{
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    [ebp-1Ch]           ; NodeZero
    push    [ebp-2Ch]           ; Node
    push    [ebp+8h]            ; ClickedRow
    push    [ebp-8h]                ; TextAreaEntry
    push    [ebp-40h]           ; CUITextArea
    call    CheckClickedNode

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    neg     eax             ; Stolen bytes
    sbb     eax, eax
    inc     eax

    ret
}
}



void __declspec(naked)
InteractiveJournal_SortingInfoLabel_asm() {
__asm
{
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi

    push    ecx                 ; SortMode
    push    [ebp-2Ch]           ; Button
    push    [ebp-20h]           ; Panel
    push    [ebp-108h]          ; pScreenJournal
    call    ClearSortingInfoLabel

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx

    test    al,al
    jz      return_
    
    pop     eax             ; kill retadr
    mov     eax, 0758365h   ; hardcoded ret
    jmp     eax

return_:
    mov     eax, [ebp-108h]    ; Stolen bytes

    ret
}
}


