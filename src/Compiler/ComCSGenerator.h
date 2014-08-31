/////////////////////////////////////////////////////////////////////////////
//
// ComCSGenerator.h
//
// Interface CComCSGenerator - trida pro generovani kompilovanych skriptu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ComPart.h"
#include "ComExpression.h"

//*******************************************************
// CComCSGenerator - generator kompilovanych skriptu
//*******************************************************

class CComCSGenerator :	public CComPart
{
public:
	CComCSGenerator(CComObjectBrowser* ob, int _queueSize);
	virtual ~CComCSGenerator();

	void DoPart() {}				// hlavni smycka je prazdna, vstup beru ze souboru a vystup nikam neposilam
	void CreateCompiledScripts();	// vytvori kompilovane skripty
	void CreateKernelServiceHandlerProc();	// vytvori switch volani sluzeb  (pro interpret)

private:
	FILE* of;			// vystupni soubor (tam se generuje kompilovany skript)
	FILE* inputFile;	// vstup (binarni soubor s vystupem semanticke analyzy)
	
	CComExpStack es;	// zasobnik, pomoci ktereho buduji strom kazdeho vyrazu
	CComExpTree tree;	// strom prave generovaneho vyrazu

	int tab;			// kolik tabu mam dat na zacatek radky
	int look;			// token, ktery prave zpracovavam
    int lastObject;		// identifikace posledniho objektu

	// pomocne funkce pro generovani souboru assign_KSxxx.h:
	FILE* StartFile(const char* inFile, FILE** fIn = 0, bool log = true);	// Otevre novy soubora zkopiruje do nej cely soubor inFile, pointer na novy soubor potom vrati.
	void FinishFile(FILE* f, FILE* fIn = 0);		// Uzavre soubor drive ziskany pomoci StartFile

	int FindKSVG(CComOBObject* o);		// nalezne vhodnou KSVG

	// metody top-down parseru:
	void DoMethodBody(CComOBObject* o, CComOBMethod* m, FILE* f);
		bool SeekToMethod(CComOBObject* o, CComOBMethod* m, FILE* f);
			void Step();
		void DoCommand();
			void DoReturn();
		void DoLocalDef();
		void DoExpressionMain();
		void DoExpression();
		void DoNew();
		void DoDelete();
		void DoAssigned();
		void DoMethodCall();
		void DoDynamicArrayCall(CListK<CComExpression>* params);
		void DoKernelServiceCall(CListK<CComExpression>* params, int line);


		void Tab() { for(int i=0; i<tab; i++) fprintf(of, "\t"); }
		void Nl() { fprintf(of, "\n"); }
};
