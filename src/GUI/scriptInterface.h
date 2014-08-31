///////////////////////////////////////////////
//
// scriptInterface.h
//
//	interface pro propojeni GUI se skriptama
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef SCRIPT_GUI_INTERFACE_H
#define SCRIPT_GUI_INTERFACE_H

#include "KerConstants.h"
#include "tree.h"

struct CKerOVar;
class CKerName;
class CGUITreeItem;
class CEDObjectProperty;

class CGUIFont;

//////////////////////////////////////////////////////////////////////
// CGUIScriptVariableInterface
//////////////////////////////////////////////////////////////////////

// navratove hodnoty:
// - presne urcene
// - obecne 1 - OK
//			0 - ERROR

class CGUIScriptVariableInterface
{
public:
	static int ScriptedVariableInit(CEDObjectProperty* _op, CKerOVar* _OVar, CGUITreeItem* _rootTI);
		// vola se u skriptu, aby vytvoril editacni prvky pro promennou OVar pomoci nasledujicich fci:
		// fce se vola z editoru

	static int ScriptInterfaceInit(CEDObjectProperty* _op, CGUITreeItem* _rootTI);
		// inicializuje promenne pro nasledujici praci skriptu s interfacem
		// fce se vola z editoru

	static int ScriptedVariableFinish();
		// vola se nakonec, po vytvoreni vsech editacnich prvku
		// vola se take nakonec, po provedeni vsech zmen editacnich prvku (insert/delete) v obsluzne fci na tlacitko
		// aktualizuje struktury GUI, vynuluje staticke promenne CGUIScriptVariableInterface

	static int AddItem(CKerOVar* OVar, int where=0, int before=0);
		// prida novou editacni polozku pro zakladni promennou urcenou OVarem do stromu
		// where urcuje kolem jake existujici polozky se nova prida
		// before urcuje zda se prida pred ex. nebo za
		// where==0 && before==0 => pridani na konec seznamu
		// where==0 && before==1 => pridani na zacatek seznamu
		// vrati index nove polozky (1..n) 0 = chyba
		// puvodni polozky, ktere se nyni octnou za nove pridanou se precisluji (jejich index se o 1 zvetsi)

	static int AddGap(int where=0, int before=0);
		// prida oddelovac na zadanou pozici
		// urceni pozice funguje stejne jako u AddItem
		// vrati index oddelovace (1..n)

	static int AddText(char* text, char* help=0, int where=0, int before=0);
		// prida text na zadanou pozici
		// urceni pozice funguje stejne jako u AddItem
		// vrati index textu (1..n)

	static int DeleteItem(int index);
		// smaze polozku s indexem "index"
		// umi mazat jak editacni polozky, tak oddelovace, tak texty
		// vraci 0 - objekt neexistuje (1 - objekt smazan)

	static int AddButton(int where=0, int shift=1, char* label=0, char* help=0, int userID=0, CKerName* scriptFunction=0);
		// prida tlacitko podle pozice where a shift
		//		where urcuje index polozky (radku), kam se bude pridavat
		//		shift urcuje zda se prida na radek (where) (shift==0), za radek (shift==1), ci pred radek (shift==-1)
		// label - popisek na tlacitku
		// help - kontextova napoveda na tlacitku
		// userID - int urceny klientem, ktery se mu preda pri zmacknuti tlacitka (krome vraceneho buttonID)
		// scriptFunction(int groupIndex, int buttonID, int userID) - funkce, ktera se zavola po zmacknuti tlacitka
		//		pokud zadna fce neni zadana - vola se std. fce ButtonClicked(int groupIndex, int buttonID, int userID)
		// vraci buttonID nove vytvoreneho tlacitka

	static int DeleteButton(int buttonID);
		// smaze tlacitko s buttonID

	// ---------------------------------------------------------------------
	// SKUPINY:
	
	static int AddGroupItem(char* label, char* help, int where=0, int before=0);
		// vytvori novou skupinu (novy "adresar" pro editacni polozky)
		// urceni pozice funguje stejne jako u AddItem
		// vrati index skupiny (groupIndex) - 1..n
		// groupIndex==0 ma korenova polozka

	static int DeleteGroupItem(int groupIndex);
		// smaze skupinu s groupIndex vcetne vsech jejich polozek
		// vraci 0 - objekt neexistuje (1 - objekt smazan)


	static int SelectGroupItem(int groupIndex);
		// vybere skupinu, se kterou se bude dale pracovat (oznaci ji za aktivni)
		// fce AddItem, AddGap, DeleteItem, AddButton, DeleteButton pracuji vzdy s aktivni skupinou
		// groupIndex==0 znamena praci v korenove polozce
		// vraci 1 - polozka nenalezena (zvolena korenova polozka), 0 - OK

	static int DeleteAllGroupItems(int groupIndex, int buttonsLet=0);
		// smaze vsechny polozky ve skupine groupIndex
		// buttonsLet urcuje, zda ponecha tlactika (==1) nebo nikoliv (==0, tzn. smaze vsechno)
		// samotna skupinova polozka (adresar) se nemaze
		// groupIndex==0 => smaze vsechno krome korenove polozky odpovidajici skriptovano promenne
		// vraci 0 - provedo, 1 - nastala chyba (polozka nenalezena = nic nesmazano, ...)
	// --------------------------------------------------------------------


protected:
	static int DeleteButtonRecursiv(CGUITreeItem* gi, int buttonID);
	static int DeleteButtonElem(CGUITreeItem* ti, int buttonID);

	static CEDObjectProperty* op;
	static CGUITreeItem* rootTI;

	static CGUITreeItem* selectedGroupTI;

	static CGUITreeItem* groupItem;
	static int numGroupItems;

	static int buttonIDCounter;
};


//////////////////////////////////////////////////////////////////////
// CGUIScriptButtonsItemData
//////////////////////////////////////////////////////////////////////

// struktura drzici informace o kazdem tlacitku, ktere je vytvoreno pres skript
// - kvuli moznosti mazani pres buttonID
// - kvuli volani fce skriptu po stisknuti tlacitka

struct CScriptButtonData
{
	CScriptButtonData(typeID _buttonGUI_ID, int _buttonID, int _userID, CKerName* _scriptFunction);
	~CScriptButtonData();

	int buttonID;	// ID, pod kterym vidi button skript
	int userID;		// ID, ktere buttonu skript priradil (je mu posilano pri zmacknuti)
	CKerName* scriptFunction;	// fce skriptu, ktera se vola po zmacknuti tlacitka
	
	typeID buttonGUI_ID;	// ID buttonu, pod kterym je zaregistrovan v GUI - objID
};

//////////////////////////////////////////////////////////////////////
// CGUIScriptButtonsItemData
//////////////////////////////////////////////////////////////////////

// struktura, ktera je pripojena u polozky stromu, ktera je urcena pro tlacitka skriptu
// - udrzuje si informaci o tlacitkach, ktere si skriptovana promenna vytvorila a vlozila na tuto pozici stromu

class CGUIScriptButtonsItemData : public CGUITreeItemUserData
{
public:
	CGUIScriptButtonsItemData();
	virtual ~CGUIScriptButtonsItemData();

	int AddButton(CScriptButtonData* button);
	typeID RemoveButton(int buttonID);
		// vraci 0 - nic neodstraneno (button se nenasel), buttonGUI_ID - ID odstraneno buttonu
		// button se odtrani jen ze seznamu buttonu, ne ze struktur GUI !!!
	CScriptButtonData* GetButtonData(typeID buttonGUI_ID);

	int GetNumButtons() { return numButtons; };

	void RefreshButtonsPositions(float winWidth, float buttonsWidth);
		// aktualizuje pozice - vyrovna dle veliskoti okna (winWidth), poctu tlacitek a jejich velikosti (buttonsWidth)
		// vola se po kazdem pridani/odebrani tlacitka ze seznamu
		// pro jednoduchost maji vsechna tlactika stejnou velikost

protected:

	int numButtons;
	CGrowingArray<CScriptButtonData*> buttonsArray;
};


//////////////////////////////////////////////////////////////////////
// CGUIScriptButtonsHandler
//////////////////////////////////////////////////////////////////////

// obsluzny program pro vsechny skriptovane tlacitka objektu
// u kazdeho tlacitka se pri vytvareni objedna udalost EClicked
// po stisknuti tlacitka udalost prijde do tohoto obsluzneho objektu
// ten zjisti o jake tlacitko jde, ktere skriptovane promenne patri 
// a zavola prislusnou obsluznou fci skriptu
// - vyuziva informaci ze struktury CScriptButtonData

class CGUIScriptButtonsHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);

	CEDObjectProperty* op;

public:
	CGUIScriptButtonsHandler(CEDObjectProperty* _op) { op = _op; };
};

#endif 