///////////////////////////////////////////////
//
// GameMenu.h
//
//	interface pro herni menu
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#ifndef EDITORLEVELWIN_H
#define EDITORLEVELWIN_H

#include "widget.h"
#include "objectList.h"

class CKerName;
struct CKerObject;
class CBltRect;
class CGameMenuObject;
class CGameMenuProgressBar;

#include "KerConstants.h"

//////////////////////////////////////////////////////////////////////
// CGameMenu
//////////////////////////////////////////////////////////////////////

// fce jsou staticke, lze je volat i kdyz objekt GameMenu neexistuje
// v tomto pripade neprovadeji zadnou cinnost.
// pred prvnim pouzitim se musi objekt vytvorit - CreateGameMenu();
// po vytvoreni menu, dle svych pozadavku zavolat aktualizacni fci - Refresh();
// nakonec hry menu smazat pomoci - DeleteGameMenu();

class CGameMenu : public CGUIMultiWidget
{
public:
	static int CreateGameMenu();
		// vytvori Game Menu, pokud jiz neni vytvoreno

	static int DeleteGameMenu();
		// smaze Game Menu, pokud existuje

	static int Show();
	static int Hide();

	//static void InitKrkalGameMenu();
		// inicializuje menu na std. krkalovske

	static int AddItem(CKerName* item);
		// prida objekt do menu

	static int AddItem(OPointer obj);
		// prida objekt do menu

	static int DeleteItem(int index);
		// smaze objekt s indexem z menu
	
	static int DeleteItem(CKerName* item);
		// smaze objekt "item" z menu

	static int SetItem(CKerName* item, int index);
		// prida objekt do menu na pozici index
		// pokud tam jiz neco bylo, smaze to

	static void Refresh();
		// aktualizuje vykreslovani menu
		// objekty jsou vykreslovany postupne podle sveho indexu (odleva doprava)
		// musi zavolat po pridani nebo odebrani objektu, kvuli prekresleni menu

	static int ObjectIncrement(int index);
	static int ObjectIncrement(CKerName* item);
		// inkrementuje pocet objektu a vrati aktualni stav

	static int ObjectDecrement(int index);
	static int ObjectDecrement(CKerName* item);
		// dekrementuje pocet objektu a vrati aktualni stav

	static int ObjectSetCount(int index, int count);
	static int ObjectSetCount(CKerName* item, int count);
		// nastavi pocet objektu a vrati aktualni stav

	static int ObjectGetCount(int index);
	static int ObjectGetCount(CKerName* item);
		// vrati aktualni pocet objektu

	static int Add2ProgressBars(int color1, int color2);
		// prida 2 ProgressBary na zacatek menu

	static int SetProgressBar(int index, int progress);
		// nastavi progress na PB (index== 1 / 2)

	static int GetProgressBar(int index);
		// vrati progress na PB (index== 1 / 2)

protected:
	CGameMenu();
	virtual ~CGameMenu();

	static CGameMenu* gameMenu;

	CGrowingArray<CGameMenuObject*> objArray;	// pole objektu
	int numObjects;		// pocet objektu v poli

	bool withProgressBars;
	CGameMenuProgressBar* progressBar1;
	CGameMenuProgressBar* progressBar2;

	bool compressedMenu;
};



//////////////////////////////////////////////////////////////////////
// CGameMenuObject
//////////////////////////////////////////////////////////////////////

class CGameMenuObject : public CGUIWidget
{
public:
	CGameMenuObject(CKerName* item);
	CGameMenuObject(OPointer obj);
	virtual ~CGameMenuObject();

	int Increment();	// inkrementuje pocet objektu a vrati aktualni stav
	int Decrement();	// dekrementuje pocet objektu a vrati aktualni stav
	int SetCount(int _count);		// nastavi pocet objektu a vrati aktualni stav
	int GetCount();		// vrati aktualni pocet objektu

	void SetCompress(bool compress);

	CKerName* GetName() { return objName; };

	virtual void Move(float _x, float _y);

protected:
	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	virtual void Resize(float _sx, float _sy);
	virtual void SetVisible(int vis);

	void CreateObjPicture(CKerObject *ko=0);

	CKerName* objName;
	int count;
	char countString[11];

	CGUIRectHost* border;
	CGUIRectHost* picture;
	CGUIRectHost* countBorder;
	CGUIStaticText* countPicture;

	bool compressed;
};


//////////////////////////////////////////////////////////////////////
// CGameMenuProgressBar
//////////////////////////////////////////////////////////////////////

class CGameMenuProgressBar : public CGUIWidget
{
public:
	CGameMenuProgressBar(int color, bool compressed=false);
	virtual ~CGameMenuProgressBar();

	int SetProgress(int _progress);		// nastavi progress vrati aktualni stav (0..100%)
	int GetProgress();		// vrati aktualni progress (0..100%)
	int GetColor();


	virtual void Move(float _x, float _y);

protected:
	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	virtual void Resize(float _sx, float _sy);
	virtual void SetVisible(int vis);


	int progress;
	int fullSize;

	CGUIRectHost* border;
	CGUIRectHost* progressPicture;
};


#endif 