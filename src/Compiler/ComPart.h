/////////////////////////////////////////////////////////////////////////////
//
// ComPart.h
//
// Interface CComPart - spolecny predek vsech casti kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>

#include "ComObjectBrowser.h"
#include "ComError.h"

//*******************************************************
// CComFileCahe - jednoducha cache pro vstup nebo
//					vystup do souboru
//*******************************************************

class CComFileCache
{
public:	
	CComFileCache(FILE* _f, bool _in);
	~CComFileCache();

	int GetToken();					// Cteni tokenu
	void PutToken(int token);		// Zapis tokenu
	void Flush();					// Zapsani cache
	void Clear();					// Vymazani cache

private:
	void Fetch();					// Nacteni cache

private:
	FILE* f;		// cacheovany soubor
	bool in;		// vstup (true) / vystup (false)
	int index;		// index aktualniho prvku v cache
	int size;		// velikost cache
	int cache[cFileCacheSize];		// pole cache
};

class CSEdScriptEdit;

//*******************************************************
// CComComPart - zakladni funkcnost kazde casti 
//					kompilatoru
//*******************************************************

class CComPart
{
friend class CComExpTreeNode;
public:
	CListK<CComError> warningList;

	void SetDebug(const char* fileName, bool _debugPos = false);		// Nastavi ladici vystup do souboru.
	void SetOutput(const char* fileName);		// Nastavi vystup do souboru.
	void ReleaseOutput();						// Uzavre vsechny vystupy (zavre soubory).
	int	 Get();									// Precte dalsi token.
	void Run() { while(Get() != lexEOF); }		// Nacita tokeny, dokud neni konec souboru.

	int StartFile(const char* fileName, CComPosition* inclPos = 0);		// Zacne kompilovat ze souboru.
	int StartPart(CComPart* _parent);			// Zacne kompilovat z jine casti kompilatoru.
	int StartFromMemory(const unsigned char* data, int size, const char* fileName);	// Zacne kompilovat z pointeru na text
	int StartFromWindow(CSEdScriptEdit *script);	// Zacne kompilovat z okna editoru skriptu.

	void DebugString(int token, char* buf);		// Generuje ladici vystup k tokenu.
	const char* GetTokenStr(int token);			// Vrati retezec se jmenem tokenu.
	const char* GetVersionStr() { return uTab->Get(pos.version)->name; }	// Vrati retezec prislusne verze.

protected:
	CComPosition pos;				// aktualni pozice ve zdrojaku
	CComShortPosition oldPos;		// minula pozice

	CComOBObject* inObj;			// prave kompilovany objekt
	int inObjID;					// ID prave kompilovaneho objektu
	CComOBMethod* inMethod;			// prave kompilovana metoda

	CDynamicArrayV<CComPosition, 32> includes;	// seznam souboru, ktere se maji includovat
	CDynamicArrayV<int, 32> included;			// seznam souboru, ktere se jiz includovaly
	CListK<CComPosition> posStack;				// zasobnik, na kterem si pamatuji starou pozici pri includovani souboru
	const char* startDir;			// startovni adresar
	char* tmpStr;					// libovolny retezec - bude odalokovan v destruktoru -> muzu si ho naalokovat, nekam ho predat a vic se o nej nestarat
//
	CComPart(CComObjectBrowser* ob, int queueSize);
	virtual ~CComPart();

	int  GetParent();				// Vola dalsi cast kompilatoru - pozadavek o dalsi token.

	virtual void DoPart() = 0;		// Specificka cinnost kazde casti kompilatoru
	void SetStartDir(const char* _startDir);	// nastvi startovni adresar

	void Put(int token, bool putPosition = true, CComShortPosition* alterPosition = 0);	// posle token do vystupni fronty
	int  PutReference(int token = cUnknown, bool putPosition = true);	// posle token na vystup, vrati na nej referenci
	int  RemoveReference(int ref, int testVal = cUnknown);	// odstrani z vystupu drive ulozeny token, na ktery mam referenci
	void SetReference(int ref, int newValue) { queue[refs[ref]] = newValue; }	// zmeni token, na ktery mam referenci
	void RemoveLast() { RemoveReference((queueUp+queueLn-1)%queueLn); }	// odstrani posledni referenci

	int FinishFile();			// uzavre soubor, pripadne obnovi vstup po preruseni kompilace

	int StartStr(const char* _inputStr, int rewind, int lookToSave);	// zahaji kompilaci z retezce
	int FinishStr();			// ukonci kompilaci z retezce, obnovi vstup po preruseni kompilace

	void SetWarning(CComError e);	// ulozi warning do seznamu warningu

	CComSymbolTable* uTab;			// pointery na tabulky kompilatoru
	CComConstantTable* cTab;
	CComSymbolTable* oTab;
	CComSymbolTable* nTab;
	CComSymbolTable* dTab;
	CComSymbolTable* gTab;
	CComSymbolTable* pTab;

	CComSymbolTable* mTab;
	CComSymbolTable* aTab;

	CComSymbolTable* dnTab;
	CComSymbolTable* gnTab;
	CComSymbolTable* sTab;

	CKerNamesMain*	 names;
	CComNameSearch*  nameSearch;
	CComObjectBrowser* objBrowser;

	CComSymbolTable* GetTab(int token);		// najde k tokenu spravnou tabulku a vrati na ni pointer

public:
	// mnoho jednoduchych funkci, ktere testuji vlastnosti tokenu:
	static bool IsLexicalToken(int token)		{ return token>lexFirst && token<lexLast; }
	static bool IsSyntaxToken(int token)		{ return token>synFirst && token<synLast; }
	static bool IsSemanticToken(int token)		{ return token>semFirst && token<semLast; }
	static bool IsOperatorToken(int token)		{ return token>opFirst && token<opLast; } 
	static bool IsKeywordToken(int token)		{ return token>kwFirst && token<kwLast; }
	static bool IsKnownNameAtrToken(int token)	{ return token>=knFirst && token<knSecond; }
	static bool IsKnownNameToken(int token)		{ return token>=knSecond && token<knLast; }
	static bool IsKernelService(int token)		{ return token>=ksFirst && token<ksLast; }
	static bool IsKernelConstant(int token)		{ return token>=kcFirst && token<kcLast; }
	static bool IsUnknownToken(int token)		{ return token>=tokUnknownFirst && token<tokUnknownLast; }
	static bool IsConstantToken(int token)		{ return token>=tokConstantFirst && token<tokConstantLast; }
	static bool IsObjectToken(int token)		{ return token>=tokObjectFirst && token<tokObjectLast; }
	static bool IsStructureToken(int token)		{ return token>=tokStructureFirst && token<tokStructureLast; }
	static bool IsNameToken(int token)			{ return token>=tokNameFirst && token<tokNameLast; }
	static bool IsDirectNameToken(int token)	{ return token>=tokDirectNameFirst && token<tokDirectNameLast; }
	static bool IsGlobalNameToken(int token)	{ return token>=tokGlobalNameFirst && token<tokGlobalNameLast; }
	static bool IsGlobalToken(int token)		{ return token>=tokGlobalFirst && token<tokGlobalLast; }
	static bool IsMethodToken(int token)		{ return token>=tokMethodFirst && token<tokMethodLast; }
	static bool IsAttributeToken(int token)		{ return token>=tokAttributeFirst && token<tokAttributeLast; }
	static bool IsLocalToken(int token)			{ return token>=tokLocalFirst && token<tokLocalLast; }
	static bool IsArgumentToken(int token)		{ return token>=tokArgumentFirst && token<tokArgumentLast; }
	static bool IsTmpExpToken(int token)		{ return token>=tokTmpExpFirst && token<tokTmpExpLast; }
	static bool IsSaveTmpExpToken(int token)	{ return token>=tokSaveTmpExpFirst && token<tokSaveTmpExpLast; }
	static bool IsDataToken(int token)			{ return token>=tokDataFirst; }
	static bool IsDynamicArrayToken(int token)	{ return token==kwCharArray || token==kwDoubleArray || token==kwIntArray || token==kwNameArray || token==kwObjptrArray || token==kwVoidArray; }
	static bool IsNullToken(int token)			{ return token==kwNull || token==kwNNull || token==kwONull || (token>=kwANullChar && token<=kwANullVoid); }
	static bool IsAttributeGroupToken(int token){ return token==kwPoint2D || token==kwPoint3D || token==kwCell2D || token==kwCell3D || token==kwArea2D || token==kwArea3D; }

	static bool IsNameType(int type)		{ return type==kwName || type==kwObjectName || type==kwMethodName || type==kwParamName; }
	static bool IsStringType(int type)		{ return type>=kwString && type <=kwStringLast; }
	static bool IsBaseType(int type)		{ return type==kwInt || type==kwChar || type==kwDouble || type==kwVoid || IsNameType(type) || IsStringType(type); }
	static bool IsObjectType(int type)		{ return IsObjectToken(type) || type==kwObjptr; }
	static bool IsSafeType(int type)		{ return IsBaseType(type) || IsObjectType(type) || IsDynamicArrayToken(type); }
	// SPEEDUP: setridit tato klicova slova za sebou...
	static bool IsSpecialType(int type)	{ return type==kwConstructor || type==kwLConstructor || type==kwCConstructor || type==kwDestructor || type==kwUConstructor; }

	static bool IsAssign(int opIndex)		{ return opIndex>=opAssign && opIndex<=opAssignXor; }
	static bool IsEqual (int opIndex)		{ return opIndex==opEqual  || opIndex==opNotEqual; } 
	static bool IsRelation(int opIndex)		{ return opIndex>=opLess && opIndex<=opGreaterEqual; }
	static bool IsShift(int opIndex)		{ return opIndex==opLeftShift || opIndex==opRightShift; }
	static bool IsAdd(int opIndex)			{ return opIndex==opPlus || opIndex==opMinus; }
	static bool IsMultiply(int opIndex)	{ return opIndex>=opMultiply && opIndex<=opModulo; }
	static bool IsUnaryPrefix(int opIndex) { return IsAdd(opIndex) || opIndex>=opPreIncrement && opIndex<=opIndirection || opIndex==opMultiply || opIndex==opBitAnd || opIndex==opUnaryMinus || opIndex==opUnaryPlus; }

	static int GetOpArity(int opIndex);
	char* GetTypeStr(int type,int pointer);
	static int	GetTypeSize(int type, int pointer);

	// metody, ktere vraci popis typu nebo jmena identifikatoru
	const char* GetTypeString(int type, bool considerUnsigned = true);
	const char* GetLongNameStr(CListK<CComNSENode>* ident);
	const char* GetCollisionTypesStr(int type1, int pointer1, int type2 = cUnknown, int pointer2 = cUnknown, const char* addStr = 0);

	int* queue;				// vystupni fronta tokenu (cyklicka)
	int  queueLn;			// delka fronty
	int  queueUp;			// index konce fronty
	int  queueDn;			// index zacatku fronty
	CDynamicArrayV<int, 128> refs;

	void QueuePut(int token);	// ulozeni tokenu do fronty (na konec)
	bool QueueEmpty() const { return queueDn == queueUp; }	// test, zda je fronta prazdna
	bool QueueFull() const { return (queueUp+1)%queueLn == queueDn; } // test, zda je fronta plna
	int  QueueGet();			// vybrani tokenu ze zacatku fronty
	void QueueExpand(int size = -1);	// zvetseni pole fronty (pokud je jiz plna)

protected:
	CComPart* parent;		// predchozi cast kompilatoru

private:
	int  inputType;			// typ vstupu (konstanty inpXXX)
	bool debugPos;			// true pokud se mai do ladiciho vystupu zapisovat i metatokeny
	int  indexStr;			// index ve stringu pri kompilaci ze stringu (implementace nahrazeni makra)
	int  strRewind;			// kam se mam vratit po ukonceni kompilace ze stringu (delka stringu)

	bool lineChange;		// true, pokud se od posledne zmenila radka
	bool fileChange;		// true, pokud se od posledne zmenil soubor
	int space;				// pocet poslednich whitespace znaku

	CSEdScriptEdit *scriptWindow;		// okno editoru skriptu (0 pokud nekompiluju z editoru)
	const unsigned char* inputFile;		// text vstupniho souboru  (0 pokud nekompiluju ze souboru)
	char* inputStr;						// vstupni string (0 pokud nekompiluju ze stringu)
	FILE* debugFile;		// soubor pro ladici vystup (pripadne 0)
	FILE* outputFile;		// soubor pro vystup (pripadne 0)
	CComFileCache* outputCache;		// cache vystupu

	void SetSource(CComPart* _parent);		// nastaveni zdroje kompilace na jinou cast kompilatoru
	void SetSource(CSEdScriptEdit* script);		// nastaveni zdroje kompilace okno editoru skriptu
	void SetSource(const unsigned char* _inputFile); // nastaveni zdroje kompilace na soubor
	void SetSource(const char* _inputStr, int lookToSave); // nastaveni zdroje kompilace na string
};
