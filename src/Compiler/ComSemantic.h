#pragma once
#include "ComPart.h"
#include "ComExpression.h"

//*******************************************************
// CComSemantic - semanticky analyzator
//*******************************************************

class CComSemantic : public CComPart
{
public:
	CComSemantic(CComObjectBrowser* ob, int _queueSize, CListK<CComOBAttribute*>* _inhAttrList);
	~CComSemantic();

	void DoPart();	// zacatek prace (vola generator kodu)

private:
	CListK<CComOBAttribute*>* inhAttrList;	// seznam zdedenych atributu

	int index;		// Index prave zpracovaneho tokenu (pamatuju si u kazde metody). Kdyz chci zpracovat zdedene metody, tak nehledam dlouze vstupni bod, ale rovnou se na nej naseekuju.
	int look;		// prave zpracovavany token
	int old;		// token zpracovany v minulem kroku
	bool returnsValue;	// true, pokud prave kompilovana metoda vraci hodnotu
	bool doingParams;	// true, pokud prave zpracovavam parametry ve volani metody

	int blockDepth;			// aktualni hloubka vnoreni do prikazovych bloku
	int currentBlock;		// index prave zpracovavaneho bloku
	int totalBlockNr;		// velkovy pocet zpracovanych prikazovych bloku
	int defaultVersion;		// verze, ktera se pouzije jako default pri rozlisovani identifikatoru podle verze
	int nestedCalls;		// pocet vnorenych volani (volani metody v parametrech jine metody)
	int callDepth;			// aktualni hloubla vnorenych volani

	CListK<CComNSENode> ident;		// posledni identifikator
	CListK<bool> doingParamsList;	// zasobnik, ve kterem si pamatuju hodnotu doingParams pred vnorenym volanim
	FILE* postPassInput;			// vstup pro kompilaci zdedenych metod (otevreny soubor)
	CComFileCache* postPassCache;	// cache vstupu pro kompilaci zdedenych metod

	void Step(bool put = true);		// prechod na dalsi token

	// Metody top-down parseru:
	void DoTopDecl();

		void DoScripted();
		void DoObjectDecl();
		void DoAttributeDecl();
		void DoMethodDecl();
			void DoMethodBody();
			void DoCommand(bool putStartBlock = true);
			void DoExpressionMain();
			void DoExpression();
				void DoPrimary();	// ulozi na zasobnik operand
					const char* DoIdentifier(int& outID, int& data1, int& data2, int& version, CComShortPosition& versionPos, bool& foundNothing);
						CComNSEEntry* CheckVeryFirst(CListK<CComNSEEntry>* nList);
						CComOBConstant* DoReadVersion(int& version);
						CComNSEEntry ResolveDirectMethodAmbiguity(CListK<CComNSEEntry>* nList);
						CComNSEEntry ResolveVersionAmbiguity(CListK<CComNSEEntry>* nList, int& version);
						
				// Metody pro parsing volani funkce nebo zpravy
				void DoMethodCall();
					void CheckMethodParams(CComOBMethod* m, int time, CListK<CComExpression>* params, CComShortPosition posName, CComShortPosition posTime);
					void DoSafeCallCheck(CComExpression* exp, CComOBMethod* callMethod);

				void DoDynamicArrayCall(CListK<CComExpression>* params);
				void DoKernelServiceCall(CListK<CComExpression>* params);

					void DoPostCall(char* name, bool returnsValue);
					void DoNextParameter(int& kind, char* name, int& type, int& pointer);

				// parsing prikazu:
				void DoReturn();
				void DoNew();
				void DoDelete();
				void DoSender();
				void DoThis();
				void DoAssigned();
				void DoTypeOf();



// vyhodnocovani vyrazu
	CComExpStack es;			// zasobnik pro vyhodnocovani vyrazu
	CComExpression res;			// vysledek posledniho vyrazu
	CComExpression* resPtr;		// pointer na vysledek posledniho vyrazu
	CComExpression* exps[3];
	
	void DoOp(int op);	// Provede prislusnou operaci s operandy na zasobniku

	CComExpression* Result1();	// ulozeni vysledku unarni operace
	CComExpression* Result2(int setType = cUnknown, bool doConversions = true);	// ulozeni vysledku binarni operace

	// testovaci funkce pro typy:
	bool IsIntDataType(int type)	{ return type==kwChar || type==kwInt; }
	bool SameTypes()				{ return es.e1.type==es.e2.type && es.e1.pointer==es.e2.pointer; }
	bool NonPtrs()					{ return !es.e1.pointer && !es.e2.pointer; }
	bool IsGoodOldPointer(CComExpression* e) { return e->pointer && (!IsObjectType(e->type) && !IsNameType(e->type) || e->pointer > 1) && !e->array.GetCount();}
	void InvalidTypes(int note);

	// konvertovaci funkce pro typy:
	int  Convert(int srcType, int srcPointer, int targetType, int targetPointer);
	int  Convert(CComExpression* src, int targetType, int targetPointer) { return Convert(src->type, src->pointer, targetType, targetPointer); }
	int  KSConvert(CComExpression* src, int targetType, int targetPointer);
	
	// prace po prelozeni celeho programu -> urceni kompilovatelnisti metod a atributu
	void DoPostPass();
		void CompileInheritedMethod(CComOBObject* o, int objID, CComOBMethod* m);

	// dozpracovani edit-tagu (tam, kde se objevilo jmeno apod.)
	void DoDataEditTags(CComOBBaseData* dat);

	// metody pro manipulaci s obsahem fronty
	void QueueTightSubstitute(int find, int replace);
	void QueueBackAndForwardSubstitute(int findBack, int nestedCalls, int findForward, int replace, int extraStepsBack = 0);
};
