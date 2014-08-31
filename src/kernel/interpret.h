////////////////////////////////////////////////////////////////////////////////
///
///		I N T E R P R E T   interpretovanych scriptu
///
///		A: Jiri Margaritov (Interpretovani), Honza M.D. Krcek (Loading)
///
////////////////////////////////////////////////////////////////////////////////

#ifndef INTERPRET_H
#define INTERPRET_H
#define myInline inline 

#include "types.h"
#include "kernel.h"

#include "ComInstructions.h"
#include "ComObjectBrowser.h"

// Velikost pameti pro interpretovane skripty:
//#define KER_INTERPRET_MEMORY_SIZE 8  // 4 Mega 
#define KER_INTERPRET_MEMORY_SIZE 4194304  // 4 Mega 
#define KER_INTERPRET_MEMORY_AND (KER_INTERPRET_MEMORY_SIZE-1)

class CKerInterpret;
class CKerMain;
class CKerHeapFree;
class CFSRegister;
class CFSRegKey;
class CKerName;
struct CKerObject;
struct CKerMethod;

// Glob. promenna pro praci s interpretem
extern CKerInterpret *KerInterpret;





//////////////////////////////////////////////////////////////////////////////
///
///		C K e r I n t e r p r e t
///
//////////////////////////////////////////////////////////////////////////////

#define MAX_SAFE_CALL_PARAMS	50

#define COND_JUMP(cond) if(cond){DoJump(CodeStart+instr->intValue);continue;}else{break;}

// Hlavni objekt interpretu
class CKerInterpret {
friend CKerMain;
friend CKerHeapFree;
public:
	CKerInterpret();
	~CKerInterpret();
	void LoadCode(CFSRegKey *code);  // Nahraje Kod
	int LoadNames(int num, CFSRegKey *r); // Nahraje a vytvori jmena, prida na ne odkazy od interpretu 0 - Error, 1 OK
	int LoadGlobals(CFSRegKey *r); // najde umisteni pro globalni promenne a vyplni tabulku pointeru na ne
	void Init_SetStack(); // Inicializace interpretu

	CKerName **GetNamesArray() { return (CKerName **)(KerNamesStart);} // vrati pointr na pole pointru se jmeny
	void **GetGlobalsArray() { return (void **)(GlobalsStart);} // vrati pointr na pole pointru na globalni data

	UC *ISMalloc(unsigned int size); // allokuje pamet na halde. Nikdy nevrati 0, kdyz se malloc nepovede, vyvola Panic Error
	void ISFree(void *pointer);
	UC *ISMalloc(int CodeLine, unsigned int size) { SET_CODE_LINE(CodeLine); return ISMalloc(size); }// allokuje pamet na halde. Nikdy nevrati 0, kdyz se malloc nepovede, vyvola Panic Error
	void ISFree(int CodeLine, void *pointer) { SET_CODE_LINE(CodeLine); ISFree(pointer); }

	UC *GetMemory() { return memory; }

private:
	unsigned char *memory;
	// 0
	unsigned char *CodeStart; // = memory + 0
	  // code
	unsigned char *KerNamesStart; 
	  // KerNames (CKerNames*)
	unsigned char *GlobalsStart;
	  // Pointry na globalni data
	  // Globlani data, ktera nepotrebuji kompilovane skripty
	unsigned char *StackStart;
      // Stack
	unsigned char *StackTop;
	  // Empty Space
	unsigned char *HeapBottom;
	  // Heap
	// memory + KER_INTERPRET_MEMORY_SIZE

	CKerHeapFree *HFstart; // Nejhorejsi volna oblast
	CKerHeapFree *HFend; // vzdy existuje - volna oblast od StackTop do HeapBottom

	// V I R T U A L   M A C H I N E
	// interface: 
public:
	void ResetMachine();										// inicializuje interpret
	void StartMethod(CKerObject* object, CKerMethod* method);	// interface vuci kernelu - interpretuje danou metodu
	void SetIL(int newLimit) { IL = newLimit; }					// nastavi max. pocet instrukci, ktere se pri zpracovani metody provedou; prekroceni se povazuje za zacykleni

private:
	void HardJump(int addr);	// zacne provadet kod od zadane adresy
	myInline void KernelServiceCall(int kerServiceID, int paramCount);	// provede sluzbu kernelu - vetsina teto funkce je kompilatorem generovana automaticky do souboru kernelserv.txt

	myInline int PrepareForSafeCall(int paramCount,	CKerName** argNames, void** argPointers);	// priprava na volani - nastavi jmena (argNames) a hodnoty (argPointers) predavanych parametru
	myInline void CreateCallRec(int retDescriptor, int retIndex, int paramsSize, CKerObject* o); // vytvori na zasobniku volaci zaznam pro direct volani (DCALL), instrukce DRET z nej obnovuje stav pred volanim
	myInline void RestoreFromCallRec(UC* base);	// obnoveni stavu pred volanim direct metody (viz DRET)

	myInline void	Step();						// prechod na dalsi instrukci
	myInline void	Skip(int numToSkip);		// preskoceni zadaneho poctu instrukci (zpracovanych datovych)

	myInline int	MemoryReadI(UC* addr);		// cteni 4B z prislusne adresy v pameti interpretu
	myInline int	MemoryReadC(UC* addr);		// cteni 1B (char)
	myInline double MemoryReadD(UC* addr);		// cteni 8B (double)
	myInline void	MemoryWriteI(UC* addr, int value);	// zapis 4B
	myInline void	MemoryWriteC(UC* addr, int value);	// zapis 1B
	myInline void	MemoryWriteD(UC* addr, double value);	// zapis 8B

	myInline void	DoJump(int addr);		// skok na zadanou adresu

	myInline void	CheckDivisorI(int i);	 // test delitele na nenulovost
	myInline void	CheckDivisorD(double d); // test delitele na nenulovost (varianta pro double)
	myInline void	CheckOverflow(bool stackAction = true, bool over = true);	// test pre/podteceni zasobniku/haldy
	myInline void	CheckPointerForRead(UC* addr, int size);  // overeni, ze ze zadaneho pointeru lze cist
	myInline void	CheckPointerForWrite(UC* addr, int size);	// overeni, ze na zadany pointer lze zapisovat

	CIntInstructionRec*	CB;	// code base	- ukazuje na prvni instrukci
	CIntInstructionRec*	CL;	// code limit	- ukazuje na posledni instrukci
	UC*  ML;		// memory limit	- ukazuje na posledni byte pameti

	int		IC;				// instruction counter - pocet provedenych instrukci
	int		IL;				// instruction limit - maximalni pocet instrukci k provedeni
	int		IP;				// instruction pointer - index aktualni instrukce
	int instructionsNr;		// pocet instrukci v kodu
	
	UC*	OB;		// object base - ukazuje na data objektu v IS
	UC*	PB;		// parameter base - ukazuje na zacatek zasobniku (parametry metody)
	UC* SB;		// stack base - ukazuje za lokalni promenne metody

	CIntInstructionRec* instr;// ukazatel na aktualni instrukci
	int jump;

	// pole, pomoci kterych predava interpret argumenty kernelu
	int* argTypes;	
	CKerName** argNames;
	void** argPointers;

	// pomocne promenne pro interpretovane vypocty a pro predavani parametru sluzbam kernelu:
	UComValues op[cKerServicesMaxParams];	// hodnoty 
	enmOpType opTp[cKerServicesMaxParams];	// typy ulozeni

	bool isDouble[cKerServicesMaxParams];	// hodnota typu double
	bool isChar[cKerServicesMaxParams];		// hodnota typu char
	
	int tab;

	myInline void Load(int i, CIntInstructionRec* instr = 0);		// nahraje operand i
	myInline void LoadPointer(int i, CIntInstructionRec* instr = 0);	// nahraje pointer na operand i
	myInline void LoadParam(int paramStackIndex, int paramTarget, CIntInstructionRec* base, int type, int pointer);		// nahraje parametr pro volani kerneli sluzby
	myInline void Load1() { Load(1); TouchRes(); }					// standardni akce pro unarni operace
	myInline void Load2() { Load(1); Load(2); TouchRes(); }			// standardni akce pro binarni operace
	myInline void TouchRes();		// nahraje pointery pro ulozeni ciloveho operandu
	myInline void InitOp(int tmpInfo, int data, int i);			// nahraje pointer pro ulozeni operandu i
	myInline void Store(int data, int type = cUnknown, int offset = cUnknown);	// ulozi data do ciloveho operandu, nebo na zadany offset
	myInline void Store(double data, int type = cUnknown, int offset = cUnknown);	// ulozeni dat (double varianta)
};

/////////////////////////////////////////////////////////////////////
///
///		C K e r H e a p F r e e
///		Seznam volneho mista na halde
///		Seznam obsahuje vzdy alespon jeden prvek - ten posledni - Je to misto od StackTop do HeapBottom-1 vcetne
///
/////////////////////////////////////////////////////////////////////

class CKerHeapFree {
	friend CKerInterpret;
private:
	CKerHeapFree(UC *laddr, UC *uaddr, CKerHeapFree *_next, CKerHeapFree *_prev) {
		loweraddr = laddr; upperaddr = uaddr;  // pridam se do seznamu
		next = _next; prev = _prev;
		if (next) next->prev = this; else KerInterpret->HFend = this;
		if (prev) prev->next = this; else KerInterpret->HFstart = this;
	}
	~CKerHeapFree() {  // odeberu se ze seznamu
		if (next) next->prev = prev; else KerInterpret->HFend = prev;
		if (prev) prev->next = next; else KerInterpret->HFstart = next;
	}
	CKerHeapFree *next; // volne misto na nizsich adresach
	CKerHeapFree *prev; // volne misto na vyssich adresach
	UC *loweraddr;		// adresy volneho mista. Volne misto je od loweraddr do upperaddr-1 vcetne.
	UC *upperaddr;
};





#endif
