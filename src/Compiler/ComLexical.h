/////////////////////////////////////////////////////////////////////////////
//
// ComLexical.h
//
// Interface CComLexical - trida pro lexikalni analyzu zdrojoveho kodu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ComPart.h"
#include "ComHeadInfo.h"

//*******************************************************
// CComLexical - lexikalni analyzator
//*******************************************************

class CComLexical : public CComPart
{
public:
	CComLexical(CComObjectBrowser* ob, int queueSize);
	~CComLexical();

	void DoPart();			// prace lexikalniho analyzatoru

	void DoReadHead(SComHeadInfo* info);	// nacteni hlavicky skriptu (pro editor skriptu)
	void SetStartDir(const char* _startDir) { CComPart::SetStartDir(_startDir); }
	const char* GetStartDir() { return startDir; }

	char author[cMaxFileNameLn+1];	// jmeno autora
	char game[cMaxFileNameLn+1];	// jmeno hry

private:
	int look;					// znak, na ktery se prave divam
	bool defined;				// jsem v definovane casti (bud mimo #ifdef a nebo podminka plati)
	CListK<bool> ifdefStack;		// zasobnik, kam si ukladam definovanost (vzhledem k #ifdef)

	void Step();				// prechod na dalsi znak
	int SkipWhite(bool canIncLine = true);		// preskoceni bilych znaku
	int DoIdentifier();			// zpracovani identifikatoru
	int DoKnownName();			// zpracovani znameho jmena
	int DoNumber();				// zpracovani ciselne konstanty
	int DoVersion(char* buf, int i);	// zprecovani verze - buf obsahuje doposud zpracovanou cast
	int DoString();				// zpracovani retezcove konstanty (uzavrene do uvozovek)
	int DoChar();				// zpracovani znakove konstanty (uzavrene do apostrofu)
	int DoEscape();				// zpracovani escape sekvence ve znakovych a retezcovych konstantach
	int DoOperator();			// zpracovani operatoru
		void DoComment(const char* buf);	// zpracovani komentare (buf obsahuje doposud zpracovanou cast uvodniho symbolu komentare)

	bool ReadToken(char* buf, int maxlen, int(*test)(int),bool negate = false);	// Do buf nacte nejvyse maxlen znaku ze vstupu. Vstup kopiruje, dokud je splnena podminka test, pripadne jeji negace.
	void ReadTokenE(char* buf, int maxlen, int(*test)(int),bool negate = false,int error = 0, char* lookat = 0); // Totez jako ReadToken, ale v pripade presazeni maximalni delky hlasi chybu.

	void DoPreprocessor();		// zpracovani direktiv preprocesoru
		void DoDefine();		// zpracovani direktivy define 
			void DoSymbolName(CComOBBase** ste,int note,char* name = 0);	// zpracovani definovaneho symbolu
		void DoUndef();			// zpracovani direktivy undef
		void DoIfdef(int note);	// zpracovani direktivy ifdef
		void DoEndif();			// zpracovani direktivy endif
		void DoHead();			// zpracovani hlavicky zdrojoveho skriptu
			void CheckVersion(const char* versionStr);	// overeni platnosti identifikatoru verze

};
