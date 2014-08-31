/////////////////////////////////////////////////////////////////////////////
//
// ComCodeGen.h
//
// Interface CComCodeGen - trida pro generovani kodu pro interpret
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ComPart.h"
#include "ComExpression.h"
#include "ComInstructions.h"

enum enmLinkType
{
	eLkTpInt1,
	eLkTpResult,
	eLkTpJump = eLkTpResult,
	eLkTpInt2 = eLkTpResult,
	eLkTpOperand1,
	eLkTpInt3 = eLkTpOperand1,
	eLkTpOperand2,
	eLkTpInt4 = eLkTpOperand2,
};


// pouze obalka pro data do seznamu - linkovani skoku po skonceni kompilace
class CComLinkElem
{
public:
	CComLinkElem() { }
	CComLinkElem(int _addr, CComOBMethod* _m, int _blk = 0, enmLinkType _op = eLkTpJump) : 
		addr(_addr), m(_m), blk(_blk), operand(_op) { }
	
	CComLinkElem(int _addr, CComExpression _exp, int _blk = 0) : 
		addr(_addr), testExp(_exp), blk(_blk) { }

	~CComLinkElem() { }

	int addr;
	CComOBMethod* m;
	int blk;
	enmLinkType operand;
	CComExpression testExp;
};

//*******************************************************
// CComCodeGen - generator kodu pro interpret
//*******************************************************

class CComCodeGen :	public CComPart
{
friend class CComCompiler;

public:
	CComCodeGen(CComObjectBrowser* ob, int _queueSize);
	~CComCodeGen();

	void DoPart();	// vola compiler -> hlavni smycka - generovani kodu

private:
	// metody top-down parseru:
	void DoTopDecl();

	void DoObjectDecl();
	void DoMethodDecl();
		void DoCommand();

		// metody pro generovani kodu prikazu:
		void DoIf();
		void DoWhile();
		void DoDo();
		void DoFor();
		void DoReturn();
		void DoSwitch();
			void LinkJumpList(CListK<CComLinkElem>* list, int toAddr);

		void DoBreak();
		void DoContinue();

		// metody pro generovani kodu vyrazu:
		void DoNew();
			void SetToConstant(CComExpression* exp, int intValue);
			void SetToConstant(CComExpression* exp, double doubleValue);
			void SetToConstantT(CComExpression* exp, int type, int pointer);
			void StandardNew(CComExpression* target, int type, int pointer, int line);

		void DoDelete();
		void DoSender();
		void DoThis();
		void DoAssigned();
		void DoTypeOf();

		void DoExpressionMain();
		void DoExpression(bool eval = true);
			void DoPushParamOnStack();
			void DoPrimary();
			
			// metody pro generovani kodu volani metod a zprav
			void DoMethodCall();
				void DoKernelServiceCall(CListK<CComExpression>* params);
					CComExpression AllocateStack(int type, int pointer, bool grow = true);
					void DoParamsInInstructions(CListK<CComExpression>* params, int skip);
				void DoDynamicArrayCall(CListK<CComExpression>* params);
				void DoMethodCallDirect(CComExpression* expObj, CComOBMethod* m, CListK<CComExpression>* params);
					int GetParamsSize(CListK<CComExpression>* params);
				void DoMethodCallSafe(CComExpression* expObj, CComOBMethod* m, CComExpression* expMethod, CListK<CComExpression>* params, int _type = kwVoid, int _pointer = 0);
				void DoMethodMessage(CComExpression* expObj, CComExpression* expMethod, CListK<CComExpression>* params, int type, CComExpression* expTimeOrCallend);
					void DoSafeParamsInInstructions(CListK<CComExpression>* params, int num, int& blockSize, int& dataBlockSize, CListK<CComExpression>* values);

			// metody pro generovani kodu vypoctu vyrazu:
			void DoOp(enmOperators op);
				void DoPostOpOnComplex(enmInstructions op, CComExpression opResult);
				void DoPreOpOnComplex(enmInstructions op, CComExpression opResult, CComExpression opSource);
				void DoAddress(CComExpression* target, CComExpression* source);
				void DoBracket(CComExpression* target, CComExpression* e1, CComExpression* e2);
					void DoArrayAccess(CComExpression *expArray);
				void DoRelation(CComExpression* target, CComExpression* e1, CComExpression* e2, enmOperators op);
				int  StructureAccess(CComExpression* e1, CComExpression* e2);
				void DoSmartLogicJump(CComExpression* result);
				void DoConditional(CComExpression* result);
				void CheckImmDoubleOrString(CComExpression* target, CComExpression* expOp, bool arrayAccessPossible = true);
				void GrowStack(int bytes);
				void GrowStack(CComExpression* forWhat);
				void DoConstantOptimalization(CComExpression* target, CComExpression* op1, CComExpression* op2, enmOperators op, int opArity, int targetType);
				void DoPointerAdd(CComExpression* target, CComExpression* expPointer, CComExpression* expIndex, bool structAccess = false);
				void DoPointerSub(CComExpression* target, CComExpression* expPointer1, CComExpression* expPointer2);
				CComExpression* DoOpAssign(CComExpression* target, CComExpression* source);
					void DoAddressesForStringOp(CComExpression* tmp1, CComExpression* tmp2, CComExpression* op1, CComExpression* op2);
					bool IsAssignOptimalizable(enmInstructions opCode);

				void DoMove(CComExpression* target, CComExpression* source);
				void DoAdd(CComExpression* target, CComExpression* e1, CComExpression* e2);
				void DoSub(CComExpression* target, CComExpression* e1, CComExpression* e2);
				void DoMul(CComExpression* target, CComExpression* e1, CComExpression* e2);


private:
	int look;				// prave zpracovavany token
	int old;				// posledni zpracovany token
	int blockDepth;			// hloubka vnoreni do bloku prikazu
	int currentExp;			// index aktualne kompilovaneho vyrazu
	int currentBlock;		// index aktualne kompilovaneho prikazoveho bloku
	int totalBlockNr;		// celkovy pocet prikazovych bloku
	int maxStackTop;		// maximalni dosazena vyska zasobniku (odkladani mezivysledku)
	int stackBottom;		// dno zasobniku
	int currentStackTop;	// aktualni vrchol zasobniku
	int oldStackTop;		// minuly vrchol zasobniku
	int dataInInstructions;	// pocet datovych instrukci, ktere byly vygenerovany v poslednim kroku
	int stackToPlaceParam;	// offset na zasobniku, kam by se mely umistit parametry volane metody

	int codeStart;			// index prvni instrukce prave kompilovane metody
	bool assigning;			// true, pokud provadim prirazeni hodnoty do pole
	bool optimizeAssign;	// true, pokud smim optimalizovat cil predchozi instrukce
	bool doingParams;		// true, pokud zpracovavam parametry volane metody
	bool addressing;		// true, pokud zjistuji adresu polozky pole
	CComOBBaseData* tmpData;	// pomocna promenna, ktera se pouziva pro zjistovani velikosti datovych polozek

	int contJump;			// adresa skoku continue
	CListK<CComLinkElem> defaultBJL, *breakJumpList;	// seznam nevyresenych breaku
	CListK<CComLinkElem> defaultCJL, *continueJumpList;	// seznam nevyresenych continue
	
	CListK<int> toPlaceParamList;	// seznam offsetu pro ulozeni parametru na zasobnik pred volanim metody

	CListK<CComLinkElem> logicJumpList;	// seznam cilu skoku pro zajisteni zkraceneho vyhodnocovani logickych operaci && a ||
	CListK<CComLinkElem> conditionalList;	// seznam cilu skoku pro generovani kodu operatoru ?
	CListK<CComLinkElem> linkList;		// seznam nevyresenych volani direct metod (pokud jeste nebyla metoda zpracovana, neni znama ani jeji adresa v kodu) - resi se po skonceni kompilace

	CDynamicArrayV<CIntInstructionRec> code1;	// primarni kod
	CDynamicArrayV<CIntInstructionRec>* code;	// pointer na kod - sem se generuji instrukce
	
	CDynamicArrayV<CComOBMethod*> debugMethod;	// seznam metod, ktere se zpracovavaly a je tedy treba pro ne vygenerovat ladici vystup assembleru
	CDynamicArrayV<CComExpression>	tmpExpStorage;	// zasobnik docasnych vyrazu - vyuziva se pro uchovani adresy pri dereferenci
	CDynamicArrayV<CComExpression>  arrayAccess;	// zasobnik docasnych vyrazu - uchovava indexy pri pristupu k polim

	CComExpStack es;		// zasobnik pro provadeni operaci s vyrazy
	CComConstStack cs;		// zasobnik pro vypocet konstantnich vyrazu

	CComExpression resExp;	// vysledek posledniho vyrazu
	CComExpression expConst0;	// vyraz "konstanta 0"

	int inputTokens;	// seznam tokenu pro alternativni vstup - vyuziva se pri prekladu pre/post inkrementace/dekrementace slozitych operandu (poli, struktur, atd.)

	void Step(bool put = true);		// prechod na dalsi token
	int GetCurrentAddr() const;		// vrati aktualni adresu v kodu
	int GetCurrentCodeIndex() const;

	// generuje instrukci urcenou kodem opcode, pro zadane vyrazy - automaticky vyplni typy i hodnoty operandu instrukce
	int Instruction(int opcode, CComExpression* res = 0, CComExpression* op1 = 0, CComExpression* op2 = 0);
		int GetInfo(CComExpression* e, unsigned char& toWriteInfo);
		int GetInstructionByteInfo(bool isDouble, bool isChar, enmOpType opType);
	int InstrFromInstr(CIntInstructionRec instr, int opCode, int partRes, int partOp1, int partOp2);

	// zaradi do kodu instukci DBG s udanim aktualni pozice ve zdrojaku
	void DebugToCode();

	// prace po skonceni prekladu -> linkovani nevyresenych skoku
	void DoPostPass();
		void LinkAddress(int toWriteAddr, enmLinkType type, int addr);
		void SetJumpAddress(int toWriteAddr, int addr) { LinkAddress(toWriteAddr, eLkTpJump, addr);	}

	// generuje ladici vystup - kod zapsany v pseudo asembleru
	void DebugCode(const char* file);
		int DebugInstruction(FILE* f, CIntInstructionRec rec, int index);
			void DebugBlock(FILE* f, int blockSize, int startIndex, int paramsCount, bool type);
			void DebugParams(FILE* f, int skip, int startIndex, int paramsCount);

	void SetAlternativeCodeOutput(CDynamicArrayV<CIntInstructionRec>* code2);	// zmeni cilovy kod, dalsi instrukce se budou ukladat sem
	void SetNativeCodeOutput();		// nastavi cil generovani kodu na primarni kod
	void InsertCodeFragment(CDynamicArrayV<CIntInstructionRec>* code2);	// vlozi do kodu jiny kus kodu
};

#define CHECK_BYTE(i)	{assert((i)>=0&&(i)<=255);}
#define CHECK_WORD(i)	{assert((i)>=0&&(i)<=65535);}
