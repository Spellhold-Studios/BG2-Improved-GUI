#include "chitin.h"

void InteractiveJournal_QuestEntryAdd_asm();
void InteractiveJournal_ClearText_asm();
void InteractiveJournal_CheckClickedNode_asm();
void InteractiveJournal_SortingInfoLabel_asm();
void InteractiveJournal_FillText_asm();

enum CGameJournalEntryFlag {
    Open=0,
    Closed=1,
    InUse=2
};


typedef struct  _CJournalEntry {
    unsigned long   m_strText;                      // Offset=0x0 Size=0x4
    unsigned long   m_nTime;                        // Offset=0x4 Size=0x4
    enum            CGameJournalEntryFlag m_Flag;   // Offset=0x8 Size=0x4
    unsigned short  m_wType;                        // Offset=0xc Size=0x2
    unsigned char   m_bCharacter;                   // Offset=0xe Size=0x1
    unsigned char   m_nCharacterHasNotRead;         // Offset=0xf Size=0x1
} CJournalEntry;


typedef struct _CNode
{
    struct _CNode*  pNext;
    struct _CNode*  pPrev;
    void*           data;
} CNode;


typedef struct _TextAreaNode {
    struct _TextAreaNode* pNext;
    struct _TextAreaNode* pPrev;

    TextAreaEntry* data;
} TextAreaNode;


typedef struct _FoldEntryBuf {
    unsigned long   hash;
} FoldEntryBuf;

typedef struct _JournalEntryBuf {
    unsigned long   hash;
    unsigned long   Time;
    POSITION        pQuestEntry;
    CJournalEntry*  pJournalEntry;
    unsigned short  wType;
    TextAreaNode*   pTextAreaPtr;
} JournalEntryBuf;

extern const unsigned long  crc_32_tab[];
extern JournalEntryBuf      gQuestBuf[1000];
extern int                  gQuestIndex;

#define UPDCRC32(octet, crc) (crc_32_tab[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))

unsigned long crc32(const char* const Ptr, int Len);

void AddQuest(  POSITION        const pQuestEntry,
                CJournalEntry*  const pJournalEntry,
                unsigned short  wType,
                const char**    const ppQuestText,
                int             const DescriptionLen ); 