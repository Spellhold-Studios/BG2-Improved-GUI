#ifndef SCRCORE_H
#define SCRCORE_H

#include "stdafx.h"
#include "rescore.h"

class CGameObject;
class CGameAIBase;
class CCreatureObject;
struct Action;

typedef IECPtrList CActionList; //AA5C5C
typedef IECPtrList CTriggerList; //AA5E28
typedef IECPtrList CScriptBlockList; //AA5E50

static Action* g_pActionTemp = reinterpret_cast<Action*>(0xB79300);

typedef short ACTIONRESULT;
const ACTIONRESULT ACTIONRESULT_STOPPED         = -3; // loads next action
const ACTIONRESULT ACTIONRESULT_ERROR           = -2; // loads next action
const ACTIONRESULT ACTIONRESULT_DONE            = -1; // default, resets the script indices, loads next action, ignore post-action stuff
const ACTIONRESULT ACTIONRESULT_NORMAL          =  0; // continue current action, increment action ticks elapsed, terminates instant action loop, halts script processing
const ACTIONRESULT ACTIONRESULT_SUCCESS         =  1; // stop walking, loads next action, increment action ticks elapsed, terminates instant action loop
const ACTIONRESULT ACTIONRESULT_NOACTION        =  2; // adds to action ticks elapsed

struct IdsEntry;

struct ResIdsContainer { //Size 10h
	BOOL bLoaded; //0h
	ResTxt* pRes; //4h
	ResRef name; //8h
};

class Identifiers { //Size 40h
//Constructor: 0x40FB70
public:
	//AA5CF0
	Identifiers();
	Identifiers& Construct() { return *this; } //dummy

	Identifiers(ResRef rFile);
	Identifiers& Construct(ResRef rFile) { return *this; } //dummy

	virtual ~Identifiers();
	void Deconstruct() { return; } //dummy

	IdsEntry* FindByHead(IECString sValue, BOOL bCaseSensitive);

	ResIdsContainer m_ids; //4h
	IECString u14; //14h
	IECPtrList entries; //18h, contain IdsEntry objects
	BOOL bUseArray; //34h
	int* pDataArray; //38h, ptr to array of pIdsEntry
	int nArraySize; //3ch
};

extern Identifiers& (Identifiers::*Identifiers_Construct_0)();
extern Identifiers& (Identifiers::*Identifiers_Construct_1_ResRef)(ResRef);
extern void (Identifiers::*Identifiers_Deconstruct)();
extern IdsEntry* (Identifiers::*Identifiers_FindByHead)(IECString, BOOL);

struct CVariable { //Size 54h
	CVariable();
	CVariable& Construct() { return *this; } //dummy

	void SetName(IECString s);
	CVariable& operator=(CVariable& var);
	CVariable& OpAssign(CVariable& var) { return *this; } //dummy
	IECString GetName();

	char    szName[32]; //0h
	ushort type;        //20
	ushort resRefType;  //22
	ulong  dWValue;     //24h, dmPt.Y
	long   intValue;    //28h, dmPt.X
	double floatValue;  //2c
	char   stringValue[32]; //34h, for Store Local Variable, = res1 + res2 + res3 (associated with deathmatch?)
};

extern CVariable& (CVariable::*CVariable_Construct)();
extern void (CVariable::*CVariable_SetName)(IECString);
extern CVariable& (CVariable::*CVariable_OpAssign)(CVariable&);
extern IECString (CVariable::*CVariable_GetName)();

struct CVariableMap { //Size 8h CVariableHash
	CVariableMap(int nSize);
	CVariableMap& Construct(int nSize) { return *this; } //dummy

	~CVariableMap();
	void Deconstruct() {} //dummy

	BOOL Add(CVariable& var);
	CVariable& Find(IECString sVar);
	unsigned int GetHash(IECString sVar);
	void Empty();
	void SetSize(int nSize);

	CVariable* pArray;
	unsigned int nArraySize;
};

extern CVariableMap& (CVariableMap::*CVariableMap_Construct)(int);
extern void (CVariableMap::*CVariableMap_Deconstruct)();
extern BOOL (CVariableMap::*CVariableMap_Add)(CVariable&);
extern CVariable& (CVariableMap::*CVariableMap_Find)(IECString);
extern unsigned int (CVariableMap::*CVariableMap_GetHash)(IECString);
extern void (CVariableMap::*CVariableMap_Empty)();
extern void (CVariableMap::*CVariableMap_SetSize)(int);

struct ObjectIds { //Size 5h
	ObjectIds();

	char operator[](unsigned int n);

	//OBJECT.IDS indices
	unsigned char id1;
	unsigned char id2;
	unsigned char id3;
	unsigned char id4;
	unsigned char id5;
};

class Object { //Size 14h
//Constructor: 0x410CEE
public:
	//BCS correlation: OB ea, general, race, class, specific, gender, alignment, id1, id2, id3, id4, id5, name OB
	Object();

	Object(unsigned char EnemyAlly, unsigned char General, unsigned char Race, unsigned char Class, char Specific, char Gender, char Alignment, Enum eTarget, ObjectIds* poids, IECString& sName);
	Object& Construct(unsigned char, unsigned char, unsigned char, unsigned char, char, char, char, Enum, ObjectIds*, IECString&) {return *this;} //dummy

	Object(unsigned char EnemyAlly, unsigned char General, unsigned char Race, unsigned char Class, char Specific, char Gender, char Alignment, Enum eTarget);
	Object& Construct(unsigned char, unsigned char, unsigned char, unsigned char, char, char, char, Enum) {return *this;} //dummy

	bool MatchCriteria(Object& oCriteria, BOOL bAnyIncludesNonScript, BOOL bExcludeNonScript, BOOL bEAGroupMatch);

	void operator=(Object& o);
	void OpAssign(Object&) {} //dummy

	void DecodeIdentifiers(CGameAIBase& spriteSource);
	CGameObject& FindTargetOfType(CGameObject& source, char type, BOOL bCheckMiddleList);
	CGameObject& FindTarget(CGameObject& source, BOOL bCheckMiddleList);
	void SetIdentifiers(ObjectIds& ids);
	Object GetOpposingEAObject();
	unsigned char GetClass();
	void GetDualClasses(unsigned char* pClassNew, unsigned char* pClassOrg);
	BOOL HasActiveSubclass(unsigned char nSubClass, BOOL bThreadAsync);

	BOOL inline operator==(Object& o);
	BOOL IsAnything();
	BOOL IsNonScript();
	BOOL IsNothing();
	BOOL IsAny();
	BOOL IsMyself();

	IECString Name; //0h
	unsigned char EnemyAlly; //4h
	unsigned char General; //5h
	unsigned char Race; //6h
	unsigned char Class; //7h
	Enum eTarget; //8h, set and used after evaluation of Ids
	ObjectIds oids; //ch
	unsigned char Specific; //11h, for GroupOfType
	unsigned char Gender; //12h
	unsigned char Alignment; //13h
};

extern Object& (Object::*Object_Construct_10)(unsigned char, unsigned char, unsigned char, unsigned char, char, char, char, Enum, ObjectIds*, IECString&);
extern Object& (Object::*Object_Construct_8)(unsigned char, unsigned char, unsigned char, unsigned char, char, char, char, Enum);
extern bool (Object::*Object_MatchCriteria)(Object&, BOOL, BOOL, BOOL);
extern void (Object::*Object_OpAssign)(Object&);
extern void (Object::*Object_DecodeIdentifiers)(CGameAIBase&);
extern CGameObject& (Object::*Object_FindTargetOfType)(CGameObject&, char, BOOL);
extern CGameObject& (Object::*Object_FindTarget)(CGameObject&, BOOL);
extern void (Object::*Object_SetIdentifiers)(ObjectIds&);
extern Object (Object::*Object_GetOpposingEAObject)();
extern unsigned char (Object::*Object_GetClass)();
extern void (Object::*Object_GetDualClasses)(unsigned char*, unsigned char*);
extern BOOL (Object::*Object_HasActiveSubclass)(unsigned char, BOOL);

extern Object* poAny;

struct Trigger { //Size 2Eh
//Constructor: 430720h (3 args), 430810h (2 args)
//Note: trigger opcodes (short) from AA5E68-AA5F66
//BCS correlation: TR opcode, i, dwFlags, i2, u22, sName1, sName2, o TR

	Trigger();

	Trigger(short wOpcode, Object& o, int i);
	Trigger& Construct(short wOpcode, Object& o, int i) { return *this; } //dummy

	Trigger(short wOpcode, int i);
	Trigger& Construct(short wOpcode, int i) { return *this; } //dummy

	Trigger& operator=(Trigger& t);
	Trigger& OpAssign(Trigger& t) { return *this; } //dummy

	short GetOpcode();
	void DecodeIdentifiers(CGameAIBase& sprite);
	int GetI();
	int GetI2();
	IECString& GetSName1();
	IECString& GetSName2();

	short opcode; //0h
	int i; //2h, gets Arg1 (I)
	Object o; //6h (O)
	unsigned int dwFlags; //1ah
	//bit0: NOT[!]
	//bit1: TRIGGER_EXECUTED (added internally on purge)
	//bit2: TRIGGER_UPDATED_SPRITE (added internally on add or purge)

	int i2; //1eh (I2)
	int i3; //22h (unknown usage, I3?)
	IECString sName1; //26h, (S), first global, the global name is appended to the global type e.g. "LOCALSDead", "GLOBALTest"
	IECString sName2; //2ah, (S), (unknown usage, second global)
};

extern Trigger& (Trigger::*Trigger_Construct_3)(short, Object&, int);
extern Trigger& (Trigger::*Trigger_Construct_2)(short, int);
extern Trigger& (Trigger::*Trigger_OpAssign)(Trigger&);
extern short (Trigger::*Trigger_GetOpcode)();
extern void (Trigger::*Trigger_DecodeIdentifiers)(CGameAIBase&);
extern int (Trigger::*Trigger_GetI)();
extern int (Trigger::*Trigger_GetI2)();
extern IECString& (Trigger::*Trigger_GetSName1)();
extern IECString& (Trigger::*Trigger_GetSName2)();

struct Action { //Size 5Eh
//Constructor: 0x405820
//Note: action opcodes (short) from AA5980-AA5B32
//BCS correlation: AC opcode, oOverride, oObject, oTarget, i, pt.x, pt.y, i2, i3, sName1, sName2 AC
public:
	void* operator new(size_t size);
	void operator delete(void* mem);

	Action();
	Action& Construct(void) {return *this;} //dummy

	void operator=(Action& a);

	//~Action(); //0x405930

	IECString GetSName1();

	short opcode; //0h (A)
	Object oOverride; //2h (O)
	Object oObject; //16h (O)
	Object oTarget; //2ah (O)
	int m_specificID; //3eh (I)
	int m_specificID2; //42h (I)
	int m_specificID3; //46h (I)
	IECString sName1; //4ah (S)
	IECString sName2; //4eh (S)
	POINT pt; //52h (P)
	unsigned int dwFlags; //5ah, bit0 = do not purge on clear all actions
};

extern Action& (Action::*Action_Construct_0)(void);
extern IECString (Action::*Action_GetSName1)();

struct Response { //Size 24h
//Constructor: 0x416130
	Response::Response();
	Response& Construct(void) {return *this;} //dummy

	void operator=(Response& r);
	void OpEq(Response& r) {} //dummy

	short probability; //0h
	short nResponseIdx; //2h, if chosen, gets response block # in the response list
	short nScriptBlockIdx; //4h, if chosen, gets script block # in the script
	short nScriptIdx; //6h, if chosen, gets script # in the script group (i.e. Override, Race, General, etc.), however this is always 0
	CActionList m_actions; //8h
};

extern Response& (Response::*Response_Construct_0)(void);
extern void (Response::*Response_OpEq)(Response&);

class CResponseList : public IECPtrList { //Size 20h
public:
	//AA5E10

	//IECPtrList m_Responses; //0h
	int totalProbability; //1ch
};

struct CScriptBlock { //Size 3Ch
//Constructor: see 0x41829F
	BOOL Evaluate(CTriggerList& triggers, CGameAIBase& sprite);
	BOOL EvaluateTrigger(Trigger& t, CTriggerList& triggers, CGameAIBase& sprite);

	CTriggerList m_triggers; //0h
	CResponseList m_responses; //1ch
};

extern BOOL (CScriptBlock::*CScriptBlock_Evaluate)(CTriggerList&, CGameAIBase&);
extern BOOL (CScriptBlock::*CScriptBlock_EvaluateTrigger)(Trigger&, CTriggerList&, CGameAIBase&);

struct CScript { //Size 28h
	ResRef name; //0h
	int u8;
	CScriptBlockList blocks; //ch
};

struct CScriptParser { //Size EEh
//Constructor: 0x41926B
	short GetTriggerOpcode(IECString sName);
	int GetOpcode(IECString sValue, IECString sIdsName);
	void SetError(IECString s);
	Object ParseObject(IECString& sArg);
	IECString SpanBefore(IECString s, char c);
	IECString SpanAfter(IECString s, char c);
	IECString GetArgText(int nArgIdx, IECString sFuncDesc);
	IECString GetIdsValue(Identifiers& ids, IECString& sName);
	IECString GetArgTextIdsName(IECString sArgText);

	short mode; //0h, 0 = outside IF...END, 1 = in Trigger, 2 = in Action
	int lineNumber;
	int* u6; //0x28 size, 0x0 ResRef, 0x8 int, 0xc CPtrListAA5E50
	IECPtrList* ua; //contains CPtrListAction objects
	CTriggerList* ue;
	Response* m_response; //12h, response to use after parsing
	IECString sError; //16h;
	CFile u1a;
	IECString u2a;
	Identifiers ACTION; //2eh
	Identifiers TRIGGER; //6eh
	Identifiers OBJECT; //aeh
};

extern short (CScriptParser::*CScriptParser_GetTriggerOpcode)(IECString);
extern int (CScriptParser::*CScriptParser_GetOpcode)(IECString, IECString);
extern void (CScriptParser::*CScriptParser_SetError)(IECString);
extern Object (CScriptParser::*CScriptParser_ParseObject)(IECString&);
extern IECString (CScriptParser::*CScriptParser_SpanBefore)(IECString, char);
extern IECString (CScriptParser::*CScriptParser_SpanAfter)(IECString, char);
extern IECString (CScriptParser::*CScriptParser_GetArgText)(int, IECString);
extern IECString (CScriptParser::*CScriptParser_GetIdsValue)(Identifiers&, IECString&);
extern IECString (CScriptParser::*CScriptParser_GetArgTextIdsName)(IECString);

#endif //SCRCORE_H