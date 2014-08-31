/////////////////////////////////////////////////////////////////////////////
//
// ComCompiler.h
//
// Interface CComCompiler - hlavni trida kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined COM_COMCOMPILER_H__INCLUDED
#define COM_COMCOMPILER_H__INCLUDED

#include "ComLexical.h"
#include "ComSyntax.h"
#include "ComSemantic.h"
#include "ComCodeGen.h"
#include "ComCSGenerator.h"


class CFSRegister;
class CSEdScriptEdit;

//*******************************************************
// CComCompiler - hlavni trida kompilatoru
//*******************************************************

class CComCompiler
{
public:
	CComObjectBrowser* objBrowser;		// pointer na object browser

	// jednotlive casti kompilatoru:
	CComLexical*  lexical;				
	CComSyntax*	  syntax;
	CComSemantic* semantic;
	CComCodeGen*  codegen;
	CComCSGenerator* csgen;

	CComCompiler();
	virtual ~CComCompiler();

	void Reset(bool resetUTab);			// vyresetuje kompilator - vymaze vsechny tabulky, pokud je parametr true, tak i tabulky identifikatoru a konstant

	// Kompilace pro editor skriptu - druhy parametr urcuje, jestli se ma generovat kod a kompilovane skripty.
	int  SECompile(CSEdScriptEdit *script, bool createCompiledScripts = false, bool showMsgBoxInfo = true, CComError* error = 0, int* warningNr = 0, CComError** warnings = 0, bool slowMode = false);

	// Kompilace pro kernel, vzdy se generuje kod i kompilovane skripty.
	int TestComp(const char* source, bool slowMode = false);

	// Metody pro praci s warningy - nevyuzivaji se.
	int GetWarningNr() const { return warningNr; }
	CComError GetWarning(int i);

	// pomocne funkce pro editor skriptu
	bool IsHighlight(const char* word, int* type = 0);		// Rozhodne o slovu, zda se ma v editoru skriptu barevne zvyraznit.

	// Zpracuje hlavicku a informace z ni preda ve strukture info - pomocna funkce, kterou vola editor skriptu. Pri chybe vraci 0, 
	// jinak nenulovou hodnotu. Soubor metoda dostane v bufferu buf, velikost v bufSize. V codeStart vraci ukazatel za hlavicku
	int  ParseHead(char* buf, int bufSize, SComHeadInfo* info, char** codeStart);	// 0 = chyba

	const char* GetFileName(int tabID, char* buf, int bufSize, int* minBufSize = 0);

private:
	// tabulky kompilatoru
	CComSymbolTable* uTab;		
	CComConstantTable* cTab;
	CComSymbolTable* oTab;
	CComSymbolTable* nTab;
	CComSymbolTable* dnTab;
	CComSymbolTable* gnTab;
	CComSymbolTable* sTab;
	CComSymbolTable* dTab;
	CComSymbolTable* gTab;
	CComSymbolTable* pTab;

	CComSymbolTable* mTab;
	CComSymbolTable* aTab;
	CKerNamesMain*	names;
	CComNameSearch* nameSearch;

	CComSymbolTable* toRemove;
	CComSymbolTable* toModify;

	int warningNr;
	CComError* warningTable;


	bool slowMode;	// slowMode pro syntaktickou analyzu (testovani zavislosti jmen kvuli cyklum)

	void Success();	// Pomocna funkce - akce po uspecne kompilaci)
	CComPart* SetupForOB(CSEdScriptEdit *script, bool createCompiledScripts, bool resetUTab);	// nastavi kompilator na kompilaci pro object browser (preklad konci semantickou analyzou)
	CComPart* SetupForComplete(const char* source, bool resetUTab);		// nastavi kompilator pro kompletni preklad s generovanim kodu i kompilovanych skriptu
	bool DirtyOpTest();			// vraci true, pokud se po kompilaci vyskytly nevyresene neciste operace
	void DoError(CComError e);	// Akce, ktere se provedou, pokud kompilace skonci chybou.
	CComError DoSecondPassOB(CSEdScriptEdit *script, bool createCompiledScripts);	// Druhy pruchod pro object browser - reseni necistych operaci
	CComError DoSecondPassComplete(const char* source);	// druhy pruchod pro kompletni kompilaci - reseni necistych operaci
	void SetupForSecondPass();	// pripravi kompilator na druhy pruchod
	CComError GetUnresolvedDirtyOp();	// generuje chybove hlaseni kvuli nevyresene neciste operaci

	void FillTabs();	// naplni tabulky kompilatoru
	void CreateScripts(const char* source);		// po zkompilovani zapise informace o skriptu do registru
		CFSRegister* DoData(CComOBBaseData* d, CFSRegister* ka);	// pomocna funkce - zapis datove polozky
		void DoDataDefaults(CComOBBaseData* d, CFSRegister* e);		// pomocna funkce - zapis default hodnoty (edit tag nebo default parametr)

};

extern CComCompiler* compiler;

#endif