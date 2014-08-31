///////////////////////////////////////////////
//
// tree.h
//
// Interface pro tree (seznam polozek usporadanych do stromu)
// a jeho konkretni vyuziti - CGUIFileBrowser (prochazec souboru a adresaru na disku) 
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#ifndef TREE_H
#define TREE_H

#include "element.h"
#include "window.h"

#include "KerConstants.h"	// kvuli typu OPointer

class CKerName;
struct CKerOVar;
class CBltElem;

class CGUITree;
class CGUITreeMarker;
class CGUITreeSlider;
class CGUITreeRootItemBackground;
class CGUITreeItemUserData;
class CGUIFileBrowser;
class CGUIFileBrowserItem;
class CFSRegister;
class CGUIStaticPicture;
class CGUILine;

class CGUIFont;
class CGUIRectHost;
class CBltLine;


// typy uzivatelske struktury, kterou obsahuje kazda polozka stromu:
enum ETreeItemUserTagType {
	eUTnone = 0,
	eUTstring,		// ut.string			- vola destruktor
	eUTkerName,		// ut.kerName, ut.obj	- vola destruktor pri ukonceni
	eUTopointer,	// ut.kerName, ut.obj	- nevola destruktor pri ukonceni
	eUTautomatism,	// ut.aKerName, ut.objTI	- nevola destruktor
	eUTint,			// ut.pInt				- nevola d.
	eUTdataClass,	// ut.userDataClass		- vola d.
	eUTOVar,		// ut.OVar				- nevola d.
	eUTOVarDEL,		// ut.OVar				- VOLA d.
	eUTregister		// ut.registerFS		- nevola d.
};

//////////////////////////////////////////////////////////////////////
// CGUITreeItem
// polozka stromu
// list (koncova polozka), koren noveho podstromu (zabaleny/rozbaleny podstrom)
// obsahuje obrazek typu polozky (Roller), uzivatelsky obrazek, text, context help, uzivatelska data
// kazda polozka muze mit u sebe pridruzeny libovolny element GUI (bud na stejne radce jako je ona, nebo pod sebou - pro vetsi elementy)
// poloha radkoveho elementu u polozky se bud urcuje automaticky (vsechny radkove elementy polozek jsou vertikalne zarovnany tak, aby nezasahovaly do rozbalenych polozek stromu (jsou od nich napravo)), 
// nebo manualne posuvnikem - sliderem (vertikalni cara, ktera rozdeluje plochu stromu na 2 casti, levou pro polozky a pravou pro jejich radkove elementy)
//////////////////////////////////////////////////////////////////////

class CGUITreeItem : public CGUIElement, public CGUIEventSupport
{
	friend class CGUITree;
	friend class CGUIFileBrowser;
	friend class CGUIFileBrowserItem;
	friend class CGUIListElement<CGUITreeItem>;
	friend class CGUIScriptVariableInterface;
public:
	CGUITreeItem(CGUIWindow* win, float _x, float _y, char *labelString, CGUIRectHost *_picture=0, char *_help=0);
		// konstruktor polozky pro pripad, ze se neumistuje do stromu, ale jen do obycejneho okna
		// win - odkaz na okno, kde bude umistena (sama se neumisti, musi zaridit uzivatel)
		// x,y - pozice umisteni do okna
		// labelString, picture, help - parametry polozky
	CGUITreeItem(CGUITree* _tree, char *labelString, bool _rootTree, CGUIRectHost *_picture=0, char *_help=0, bool rootItemBackground=false);
		// konstruktor polozky pro pripad, ze se umistuje do stromu (vyuziva vlastnosti, ktere jsou konfigurovany u stromu)
		// tree - odkaz na strom, kde bude umistena, podle jehoz vlastnosti bude vytvorena (sama se neumisti, musi zaridit uzivatel)
		// labelString, picture, help - parametry polozky
		// rootItemBackground - urcuje, zda ma mit polozka specialni pozadi urcene pro zvyraznene korenove polozky (korenova polozka nemusi mit specialni pozadi, jen kdyz to chce)
	virtual ~CGUITreeItem();

	void SetLabel(char *text);
	void SetPicture(CGUIRectHost *_picture);
	void ChangeContextHelp(char* text);			// zmeni text contexthelpu 
	void SetInactivePicture(CGUIRectHost *_pictureInactive);
		// nastavi obrazek pro pripad deaktivace polozky (jinak se pouzije stejny obrazek jako pro aktivni polozku)
	void SetRollerPicture(CGUIRectHost *_pictureRoller);
		// nastavi obrazek urcujici typ polozky (list, koren podstromu - zabaleny/rozbaleny)

	virtual CGUITreeItem* AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool _manualUpdate = false, bool first=false);
		// prida do podstromu polozky novou polozku s parametry (labelString, picture, help)
		// where urcuje za kterou polozku se nova polozka umisti,
		// where==0 umisti se na konec
		// first==true otaci smer umistovani => umistuje novou polozku pred polozku "where", where==0 && first==true umisti se na zacatek
		// manualUpdate urcuje zda se pri rozbalovani polozky automaticky rozbali jeji pripadny podstrom (manualUpdate==false),
		// nebo se zavola funkce stromu TreeUpdateHandler, kde si uzivatel naprogramuje vlastni rozbaleni (muze pridavat nove polozky atp.)
	virtual CGUITreeItem* AddTreeItem(typeID where, CGUITreeItem* treeItem, bool _manualUpdate = false, bool first=false);
		// prida do podstromu polozky novou polozku "treeItem"
		// where urcuje za kterou polozku se nova polozka umisti,
		// where==0 umisti se na konec
		// first==true otaci smer umistovani => umistuje novou polozku pred polozku "where", where==0 && first==true umisti se na zacatek
		// manualUpdate urcuje zda se pri rozbalovani polozky automaticky rozbali jeji pripadny podstrom (manualUpdate==false),
		// nebo se zavola funkce stromu TreeUpdateHandler, kde si uzivatel naprogramuje vlastni rozbaleni (muze pridavat nove polozky atp.)
	int DeleteTreeItem(CGUITreeItem* ti);
		// odstrani polozku ze sveho podstromu
		// vrati 0 - uspech
		// 1 - neuspech (polozka nesmazana - nebyla v seznamu polozek atp.)
	
	int Activate();		// aktivuje polozku tree z pasivniho stavu na plne funkcni stav
	int Deactivate();	// deaktivuje polozku tree do pasivniho (nefunkcniho) stavu, neklikatelneho
	bool IsActive() { return active; };

	void SetRollState(int roll_state, bool synchronous=false);	
		// 0 - zabalit, 1 - rozbalit podstrom polozky
		// volba se provede ihned, pokud polozka nema zvolen manualni update
		// pri manualnim updatu se volba provadi asynchronne, tzn. az ve smycce udalosti
		// ma to tu vyhodu, ze fci lze volat odkudkoliv a nedojde napr. k mazani polozky ze sebe sama atp. (zalezi na implementaci updatu polozky, co vsechno dela, zda jen pridava nebo i maze napr. pri zavirani podstromu)
		// pokud presto chci zavolat pri manualnim updatu fci synchrone (na vlastni riziko - tzn. musim vedat jakym zpusobem je implementovan update polozky), 
    	// nastavim parametr "synchronous" na true

	void ChangeRoller();	// zmeni grafiku rolleru, podle aktualniho stavu polozky (state)
							// roller znazornuje zda se jenda o list nebo koren podstromu (rozbaleny/zabaleny)

	void SetSliderPositionFlooding(float sliderPos);	// nastavi pozici slideru (posuvniku) u sebe a celeho sveho podstromu rekurzivne

	void UpdateTree();	// aktualizuje strom podle stavu uzlu (najde nejvyssi polozku/cely strom a od nej aktualizuje rekurzivnim sestupem)
						// polozka pri zmene sveho stavu jen prenastavi svou promennou "state", potom se zavola fce UpdateTree, ktera provede zmeny ve strome, jenz ze zmeny stavu vyplynou (zmena polohy a viditelnosti polozek atp.)

	void Mark();		// oznaci polozku
	void UnMark();		// odoznaci polozku

	int* GetTreePath(int &size);	
		// vytvori pole intu, vyplni do nej cestu od korene k polozce - indexy uzlu pres ktere se musi projit
		// size - vraci hloubku zanoreni (pocet vyplnenych polozek pole)
		// vrati pointer na nove naalokovane pole s cestou
		// po ukonceni prace s polem musi uzivatel sam dealokovat !!!

	char* GetLabelText();	// vrati text polozky
			
	CGUITreeItem* GetItemWithIndex(int index);
		// vrati polozku s indexem nebo 0 (zadna takova neni)
		// hleda polozku mezi svymi podpolozkami

	int GetIndex() { return index; };
		// vrati index polozky

	void Sort();	// setridi podstrom polozky (jen jednu uroven - sve syny)
	void SortFlooding();	// setridi podstrom polozky kompletne az do listu (rekurzivni sestup)

	int SetItemElement(CGUIElement* el, bool inLineEl);
		// nastavi pridruzeny element k polozce
		// pokud uz nejaky nastaveny mela, smaze puvodni
		// umisti element do okna polozky (tree / associatedWindow)
		// nutne nakonec volat UpdateTree kvuli aktualizaci stromu
	
	void SetItemVisibility(bool vis);
		// nastavi zda je viditelna polozka (roller + icon + text)
		// kvuli uzivatelskemu zakryti polozky (klasicka polozka nebude videt, bude videt jen jeji element)
		// kdyz polozka neni videt, nezabira ani zadne misto
		// nutne nakonec volat UpdateTree kvuli aktualizaci stromu

	void SendBadInputEvent(typeID elID) { EventArise(EBadInput,0,elID); };	
		// posle udalost o spatnem vstupu
		// pouziva se pri kontrole zadavanych hodnot do pridruzenych elementu polozky

	void SetCompareFunctionSubTree(int (*compareFunction)(CGUITreeItem *, CGUITreeItem *));
		// nastavi fci, ktera urcuje poradi mezi dvema polozkama, kvuli trideni dle uzivatelskych kriterii
		// nastavi ji u celeho sveho podstromu rekurzivne 
		// (u sebe necha puvodni, ta se nastavuje zmenou atributu "compareFunction")

	int (*compareFunction)(CGUITreeItem *, CGUITreeItem *);	// ukazatel na porovnavaci funkci, ktera se pouzije pri trideni teto polozky
		// < 0 elem1 less than elem2 
		// 0 elem1 equivalent to elem2 
		// > 0 elem1 greater than elem2 

	CGUITreeItem* FindItemFromElement(CGUIElement* elem);
		// najde polozku stromu, ke ktere je pridruzen zadany element "elem" a vrati ji
		// pomala fce, prochazi vsechny polozky stromu (pouzivat jen kdyz neni vyhnuti)


	int state;		// zda se jedna o list/zabaleny/rozbaleny uzel (0/1/2)
	CGUIStaticText *label;	// popisek
	char *help;

	CGUITreeItem* parentItem;	// odkaz na materskou polozku (nadrazeny uzel stromu)
	bool rootTree;	// zda jde o korenovou polozku stromu
	CGUITree *tree;		// odkaz na strom, ve kterem je polozka umistena (muze byt 0 - polozka je jen v okne)

	bool prepared;	// pro manualni update (zda je jiz podstrom polozky pripraven (vytvoren))

	int userTagType;	// ktera polozka v unionu je platna
						// 0 - zadna (1,2,...)
	union userTag{	// data pro uzivatelske vyuziti
		char* string;
		struct {
			CKerName* kerName;
			OPointer obj;
		};
		struct {
			CKerName* aKerName;
			CGUITreeItem* objTI;
		};
		CKerOVar* OVar;
		int pInt;
		CFSRegister* registerFS;
		CGUITreeItemUserData* userDataClass;
	} ut;

	CGUIElement* elem;	// pripojeny element
	bool inLineElem;	// zda je pripojeny element na stejne radce jako polozka, nebo pod polozkou
	bool itemVisible;	// zda je viditelna polozka (roller + icon + text)
						// kvuli uzivatelskemu zakryti polozky (klasicka polozka nebude videt, bude videt jen jeji element)
	float sliderPosition;	// x-ova souradnice posunu elementu od polozky (od zacatku tree)
							// pokud je 0 pouzije se sliderPosition rodicovskeho tree

	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void Resize(float _sx, float _sy) {};
	virtual void Move(float _x, float _y);
	virtual void BringToTop();
	virtual void SetVisible(int vis);

	CGUIList<CGUITreeItem> *items;	// seznam synu - polozek podstromu
									// verejne kvuli jendoduchemu prochazeni seznamu pro cteni 
									// (nepouzivat primo pro pridavani polozek, mazani atp.!!!!)


protected:
	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void AddToEngine(CBltWindow *rootwnd);
	void AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos);	// prida polozku do enginu za zadany element (kvuli poradi viditelnosti)
	virtual void RemoveFromEngine();

	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over, UINT state);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int Keyboard(UINT character, UINT state);
	virtual void EventHandler(CGUIEvent *event);

	virtual void OnClick();	// fce pro uzivatelske pretizeni a definici co se ma stat po kliknuti na polozku

	void Update(float &px, float &py, float &maxx, bool visible);
		// aktualizuje polozku podle argumentu:
		// premisti ji na novou pozici, nastavi jeji viditelnost (nastavi viditelnost i jejiho pripadneho pridruzeneho elementu)
		// vypocita maxx, tj. maximalni x-ova souradnice ze vstupu a teto polozky
		// pokud je tato polozka korenem podstromu, vola rekurzivne update i na svuj podstrom
		// v px, py vraci pozici pro nasledujici polozku stejne urovne
		// v maxx se udrzuje maximalni x-ova souradnice ze vsech viditelnych polozek (kvuli vypoctu automaticke pozice slideru)
	void UpdateInlineElems(float &maxx);
		// aktualizuje pridruzeny radkovy element
		// premisti ho podle aktualni pozice slideru
		// rekurzivne aktualizuje vsechny radkove elementy v podstromu polozky
		// maxx - vypocita maximalni x-ovou souradnici ze vstupu a pridruzeneho elementu
		// v maxx se tedy udrzuje maximalni x-ova souradnice ze vsech viditelnych elementu (kvuli vypoctu sirky okna)


	static int Compare(CGUITreeItem *a, CGUITreeItem *b);	
		// fce pro porovnani dvou polozek, kvuli trideni
		// std. implementace porovnava lexikograficky popisky polozek
								// < 0 elem1 less than elem2 
								// 0 elem1 equivalent to elem2 
								// > 0 elem1 greater than elem2 


	void SetPositionOfLabelPicture(float *ti_sx);	
		// nastavi pozice rolleru, obrazku a popisku polozky
		// pokud je zadano "ti_sx" (!=0) zapise se tam sirka polozky pro ulozeni vsech obrazku a popisku
	void ReIndex(CGUITreeItem *start, int d_index, bool inclusive=false);
		// zmeni poradi polozek ve strome, pocinaje nasledujici polozkou po startu (inclusive==true - vcetne polozky start)  (start==0 jede od zacatku seznamu) pricte k indexu polozek d_index
		// pouziva se pri pridavani nove polozky mezi stavajici
	void DistributeUpdate(float &px, float &py, float &maxx, bool visible);
		// updatuje vsechny sve syny - rekurzivni sestup updatu (vola fci Update)
	void UpdateRootBackground(float bsx);	// aktualizuje velikost pozadi specialni korenove polozky (pri zmene velikosti okna)

	CGUITreeItem* SwitchItem(bool next);	
		// nalezne nasledujici (next==true) nebo predchozi (next==false) polozku ve strome
		// bere v uvahu aktualni stav stromu, rozbaleni/zabaleni vetvi podstromu atp.
		// pouziva se pri lezeni po strome pomoci klavesnice
		// nalezenou polozku oznaci a focusuje ji + vrati jeji adresu

	// pomocne fce pro SwitchItem:
	CGUITreeItem* SwitchItemIndex(int ind, bool next);
		// zaridi prechod od polozky podstromu o indexu "ind" ve smeru next
	CGUITreeItem* FindIndex(int ind);	// vrati polozku podstromu o indexu "ind", pokud neexistuje vrati 0
	CGUITreeItem* Switch(CGUITreeItem* si);
		// provadi prechod od stare oznacene polozky na novou "si"
		// odoznaci starou polozku, focusuje novou polozku "si", nascroluje okno na polozku, pokud uz by jinak byla mimo okno, oznaci novou polozku
		// vola se, kdyz uz jsem nasel polozku na kterou chci prejit
	CGUITreeItem* FindLastSubItem();
		// hleda rekurzivne posledni polozku v podstromu (v jeho rozbalenych vetvich) a vrati ji

	int SortItems(CGUIListElement<CGUITreeItem>** max);
		// pomocna fce pro Sort
		// provadi trideni hledanim minima a jeho presunem na konec
		// "max" je zarazka, kdyz (max==0) ulozi se do max posledni prvek seznamu (tj. prvni setrideny tedy minimum), (max!=0) hodnota max se nemeni
		// trideni se uplatnuje na prvky od zacatku seznamu do polozky max (ta uz ne), vybere se z nich minimum a to se premisti na konec seznamu
		// vzhledem ke zpusobu implementace seznamu a moznymi operacemi nad nim neni toto trideni o moc pomalejsi nez jine lepsi varianty (krom toho pocet polozek stromu neni typicky moc velky)
		// pocet pruchodu = pocet prvku seznamu
		// do budoucna predelat na MergeSort



	bool active;	// zda je polozka aktivni, tzn. pouzitelna (stisknutelna)
	bool markItem;	// zda je polozka oznacena


	CGUIRectHost *picture;		// pointer na aktualne pouzity obrazek (aktivni/neaktivni)
	CGUIRectHost *pictureInactive, *pictureActive;	// // pokud pictureInactive neni nastaven, pouzije se v pripade deaktivace polozky pictureActive 
	CGUIRectHost *pictureRoller;	// rozbalovaci obrazek (urcuje typ polozky - list, koren podstromu - rozbaleny/zabaleny)

	CGUITreeMarker *itemMarker;	// oznacovac polozky

	CBltWindow *rootwnd;	// okno enginu - kvuli pridavani a zmene grafiky behem prace s polozkou

	CGUIWindow *associatedWindow;	// odkaz na okno, ve kterem je polozka umistena
	int index;		// poradi polozky ve stromu (na sve vetvi) 0..n

	bool stateChange;	// zda se prave zmenil stav polozky (state)
	bool manualUpdate;	// zda ma byt polozka aktualizovana manualne / automaticky (rozbalovana/zabalovana)

	bool rootBackground;	// zda ma polozka specialni pozadi korenove polozky
	CGUITreeRootItemBackground *itemBackground;	// odkaz na specialni pozadi korenove polozky

	CGUIList<CGUITreeItem>* list;	// odkaz na seznam, ve kterem je polozka umistena

	static const int numEvents;
	static const int availableEvents[];
};


//////////////////////////////////////////////////////////////////////
// CGUITree
// specializovane okno pro polozky stromu - TreeItem
// - spravuje prvni vrstvu stromu - korenove polozky
// - zajistuje nastavovavani vlastnosti stromu a jeho polozek
// - obsahuje centralni obsluznou fci pro vsechny polozky stromu - TreeHandler
// a centralni fci pro manualni aktualizaci stromu - pridavani polozek do podstromu - TreeUpdateHandler
// - spravuje treeSlider - posuvnuk urcujuci pomer mezi casti pro polozky stromu a casti pro radkove elementy polozek
//////////////////////////////////////////////////////////////////////

class CGUITree : public CGUIStdWindow
{
	friend class CGUITreeItem;
	friend class CGUIFileBrowserItem;
	friend class CGUITreeSlider;
	friend class CNameBrowser;
public:
	CGUITree(float _x, float _y, float _sx, float _sy,  char *title_text, CGUIRectHost *_icon=0, UINT resizer_color=STD_TREE_BORDER_COLOR, int withBackground=1, int bgColor=STD_TREE_BGCOLOR_COLOR);
		// konstruktor jako u normalniho okna, "resizer_color" je barva okraju okna
	CGUITree(float _x, float _y, float _sx, float _sy, int bgColor=STD_TREE_BGCOLOR_COLOR);
		// konstruktor pro "orezany" strom - okno bez titulku, okraju atp. (jen s pozadim - bgColor), vse ostatni lze pridat manualne pres fce StdWindow
	virtual ~CGUITree();

	virtual CGUITreeItem* AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool _manualUpdate = false, bool _rootItemBackground=false, bool first=false);
		// prida novou korenovou polozku s parametry (labelString, picture, help, _rootItemBackground)
		// "rootItemBackground" urcuje, zda bude mit korenova polozka specialni pozadi - zvyrazneni korenovych polozek (nelze u ni pouzit radkovy element), zvyrazneni korenove polozky je pres celou sirku stromu
		// where urcuje za kterou polozku se nova polozka umisti,
		// where==0 umisti se na konec
		// first==true otaci smer umistovani => umistuje novou polozku pred polozku "where", where==0 && first==true umisti se na zacatek
		// manualUpdate urcuje zda se pri rozbalovani polozky automaticky rozbali jeji pripadny podstrom (manualUpdate==false),
		// nebo se zavola funkce stromu TreeUpdateHandler, kde si uzivatel naprogramuje vlastni rozbaleni (muze pridavat nove polozky atp.)
	virtual CGUITreeItem* AddTreeItem(typeID where, CGUITreeItem* treeItem, bool _manualUpdate = false, bool _rootItemBackground=false, bool first=false);
		// prida novou korenovou polozku "treeItem"
		// "rootItemBackground" urcuje, zda bude mit korenova polozka specialni pozadi - zvyrazneni korenovych polozek (nelze u ni pouzit radkovy element), zvyrazneni korenove polozky je pres celou sirku stromu
		// where urcuje za kterou polozku se nova polozka umisti,
		// where==0 umisti se na konec
		// first==true otaci smer umistovani => umistuje novou polozku pred polozku "where", where==0 && first==true umisti se na zacatek
		// manualUpdate urcuje zda se pri rozbalovani polozky automaticky rozbali jeji pripadny podstrom (manualUpdate==false),
		// nebo se zavola funkce stromu TreeUpdateHandler, kde si uzivatel naprogramuje vlastni rozbaleni (muze pridavat nove polozky atp.)
	int DeleteTreeItem(CGUITreeItem* ti);
		// odstrani korenovou polozku ze stromu
		// vrati 0 - uspech
		// 1 - neuspech (polozka nesmazana - nebyla v seznamu polozek atp.)

	virtual void UpdateTree();	// aktualizuje strom podle stavu uzlu (jede od prvni polozky k posledni s rekurzivnim sestupem)
		// polozka pri zmene sveho stavu jen prenastavi svou promennou "state", potom se zavola fce UpdateTree, ktera provede zmeny ve strome, jenz ze zmeny stavu vyplynou (zmena polohy a viditelnosti polozek atp.)

	void AddSlider(UINT _color=STD_TREE_SLIDER_COLOR, UINT _moveColor=STD_TREE_SLIDER_MOVECOLOR, UINT background=STD_TREE_SLIDER_BACKGROUND_COLOR);
		// prida manualni posuvnik urcujuci pomer mezi casti pro polozky stromu a casti pro radkove elementy polozek
		// color - barva posuvniku (vertikalni cara)
		// moveColor -barva posuvniku pri pohybu
		// background - barva pozadi pod radkovymi elementy polozek (cela prava cast stromu)
	void SetSliderPos(float pos);
		// nastavi pozici posuvniku


	virtual int TreeHandler(typeID treeItem, int index, int state) {return 1;};	
		// centralni obsluzna fce pro vsechny polozky stromu - pro uzivatelske pretizeni
		// treeItem - cinna polozka, index - jeji poradi v jejim podstromu, state - cim je polozka cinna:
		// state = (0/1/2/3) Clicked/MouseButtonLeft/MouseButtonRight/Keyboard focus (najeto na polozku klavesnici)
		// vrati-li 1 - posle se i udalost, vrati-li 0 udalost se jiz neposle
	virtual void TreeUpdateHandler(typeID treeItem, int index) {};
		// fce pro uzivatelske pretizeni
		// vola se u polozek s nastavenym manualnim updatem
		// o zmenu stavu se postara uzivatel manualne - muze pridavat polozky do podstromu atp.
		// jako vstupni parametr dostane polozku, ktera vyvolala pozadavek o manualni update a jeji index
		// dale lze vyuzit atribut polozky prepared - ktery rika, zda uz byla pripravena (jeji podstrom vytvoren atp.)
		// atribut prepared si uzivatel sam nastavuje


	void Sort();	// setridi korenove polozky (jen korenove, bez jejich podstromu)
	void SortFlooding();	// setridi cely strom kompletne az do listu (rekurzivni sestup)

	void SetCompareFunctionSubTree(int (*compareFunction)(CGUITreeItem *, CGUITreeItem *));
		// nastavi fci, ktera urcuje poradi mezi dvema polozkama, kvuli trideni dle uzivatelskych kriterii
		// nastavi ji u celeho sveho podstromu rekurzivne 
		// (u sebe necha puvodni, ta se nastavuje zmenou atributu "compareFunction")

	int (*compareFunction)(CGUITreeItem *, CGUITreeItem *);	// ukazatel na porovnavaci funkci, ktera se pouzije pri trideni teto polozky
		// < 0 elem1 less than elem2 
		// 0 elem1 equivalent to elem2 
		// > 0 elem1 greater than elem2 


	CGUITreeItem* GetSelectedItem();	// vrati oznacenou polozku stromu

	CGUITreeItem* Switch(CGUITreeItem* si);
		// provadi prechod od stare oznacene polozky na novou "si"
		// odoznaci starou polozku, focusuje novou polozku "si", nascroluje okno na polozku, pokud uz by jinak byla mimo okno, oznaci novou polozku
		// vola se, kdyz uz jsem nasel polozku na kterou chci prejit
		// vrati nove oznacenou polozku

	CGUITreeItem* FindItemFromElement(CGUIElement* elem);
		// najde polozku stromu, ke ktere je pridruzen zadany element "elem" a vrati ji
		// pomala fce, prochazi vsechny polozky stromu (pouzivat jen kdyz neni vyhnuti)

	int RollTreeToItem(CGUITreeItem* ti);
		// rozbali strom tak, aby byla polozka "ti" videt - tedy aby byla mezi rozbalenymi polozkami
		// vrati zda se to povedlo (0-ano, 1-ne)

	CGUITreeItem* GetItemWithIndex(int index);
		// vrati polozku s indexem nebo 0 (zadna takova neni)
		// hleda polozku mezi svymi podpolozkami

	void SetTextFont(CGUIFont* font);
	void SetRootTextFont(CGUIFont* font);

	void SetRollersStyle(CGUIStyle* style);	// nastavi styl pro textury obrazku u polozek (+/-/item)
	void SetRollersTextures(CTexture* roll, CTexture* unroll, CTexture* item, bool setRoll=true, bool setUnroll=true, bool setItem=true);	// nastavi textury obrazku u polozek (+/-/item) (bool urcuje zda se tato textura nastavuje nebo nechava puvodni nastaveni)

	bool rollOnClick;	// zda se polozka rozbali pri kliknuti (potvrzeni oznacene polozky)
						// std == true
						// pokud je == false, rozbali se jen pri kliknuti na "+" (pri kliknuti na "+" se neposle Click)

	UINT treeTextColor;
	UINT treeTextInactiveColor;

	float treeItemHeight;
	float treeRootItemHeight;
	float treeItemPictureSize;
	float treeRootNormalItemGap;	// jaka je vertikalni mezera (navic) mezi korenovou polozkou a jeji prvni podpolozkou
	UINT treeRootItemBackgroundColor;
	UINT treeRootItemLinesColor;
	UINT treeRootItemMarkBackgroundColor;
	float treeItemStartX,treeItemStartY;	// kde je umistena prvni korenova polozka stromu
	float treeEndGapX,treeEndGapY;			// jaka je mezera za posledni polozkou stromu ke kraji okna

	virtual void FocusOldTOBackEl();	// zmena oproti std. oknu je v oznacovani puvodne oznacene polozky + jeji focusace
	virtual void Resize(float _sx, float _sy);	// zmena oproti std. oknu jen v tom, ze se aktualizuje velikost specialniho pozadi u korenovych polozek
protected:
	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int KeyboardChar(UINT character, UINT state);
	virtual int Keyboard(UINT character, UINT state);
	virtual int KeyboardUp(UINT character, UINT state);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual void EventHandler(CGUIEvent *event);

	void AddTreeItemEngine(CGUITreeItem* ti);	// prida novou polozku do enginu
												// uprava od std. okna kvuli zajisteni spravne pozice viditelnosti (kvuli pozadi pod radkovymi elementy)
	void BringAllElementTop();	// zmeni viditelnost (poradi) u vsech elementu ve strome
		// viditelnost u polozek necha stejnou, jen specialni korenove zvedne nahoru
		// kvuli pridavani manualniho slideru do jiz vytvoreneho stromu

	int SortItems(CGUIListElement<CGUITreeItem>** max);
		// pomocna fce pro Sort
		// provadi trideni hledanim minima a jeho presunem na konec
		// "max" je zarazka, kdyz (max==0) ulozi se do max posledni prvek seznamu (tj. prvni setrideny tedy minimum), (max!=0) hodnota max se nemeni
		// trideni se uplatnuje na prvky od zacatku seznamu do polozky max (ta uz ne), vybere se z nich minimum a to se premisti na konec seznamu
		// vzhledem ke zpusobu implementace seznamu a moznymi operacemi nad nim neni toto trideni o moc pomalejsi nez jine lepsi varianty (krom toho pocet polozek stromu neni typicky moc velky)
		// pocet pruchodu = pocet prvku seznamu
		// do budoucna predelat na MergeSort

	static int Compare(CGUITreeItem *a, CGUITreeItem *b);
		// fce pro porovnani dvou polozek, kvuli trideni
		// std. implementace porovnava lexikograficky popisky polozek
								// < 0 elem1 less than elem2 
								// 0 elem1 equivalent to elem2 
								// > 0 elem1 greater than elem2 



	void DistributeUpdate(float &px, float &py, float &maxx);
		// updatuje vsechny sve syny - rekurzivni sestup updatu (vola fci Update)
	void DistributeUpdateInlineEl(float &maxx);
		// vola UpdateInlineElems, cimz aktualizuje vsechny radkove elementy ve strome

	void ReIndex(CGUITreeItem *start, int d_index, bool inclusive=false);
		// zmeni poradi polozek ve strome, pocinaje nasledujici polozkou po startu (inclusive==true - vcetne polozky start) (start==0 jede od zacatku seznamu) pricte k indexu polozek d_index
		// pouziva se pri pridavani nove polozky mezi stavajici


	// pomocne fce pro SwitchItem:
	CGUITreeItem* SwitchItemIndex(int ind, bool next);
		// zaridi prechod od polozky podstromu o indexu "ind" ve smeru next
	CGUITreeItem* FindIndex(int ind);	// vrati polozku o indexu "ind", pokud neexistuje vrati 0



	void UpdateRootItemsBackgrounds();
		// aktualizuje velikost pozadi vsech specialnich korenovych polozek (pri zmene velikosti okna)
	void BringTopRootItemsBackgrounds();
		// nastavi maximalni klikatelnost u korenovych specialnich polozek (se specialnim zvyraznenym pozadim), 
		// kvuli pridavani manualniho slideru do jiz vytvoreneho stromu

	int TreeMouseClick(float px, float py, int mouse, int mouseState);	// poloha mysi, tlacitko (0-leve, 1-prave), stav tlacitka (1-dole,0-nahore)
		// provadi urcitou praci pri kliknuti mysi do okna se stromem
		// oznacuje polozku, ke ktere kliknuti patri (nemusi se kliknout primo na polozku),
		// focusuje ji a pripadne rozbaluje jeji podstrom
	CGUITreeItem* FindTreeItem(float px, float py);
		// pomocna fce pro TreeMouseClick
		// hleda polozku, ke ktere nalezi kliknuti do okna se stromem
		// nemusi se kliknout primo na polozku, staci do jeji vysky, nebo do vysky jejiho pridruzeneho elementu (horizontalni poloha se zanedbava, nemuzou byt 2 polozky ve stejne vysce)


	float sliderPosition;	// x-ova souradnice posunu elementu od polozky (od zacatku tree)
	bool automaticSliderPos;	// automaticky / manualni posuvnik - Slider (nastavovani jeho pozice)
								// automaticky - tj. vsechny radkove elementy polozek jsou vertikalne zarovnany tak, aby nezasahovaly do rozbalenych polozek stromu (jsou od nich napravo)


	CGUITreeSlider* treeSlider;	// posuvnik (jen v pripade manualniho)
	CGUIStaticPicture *sliderBackground;	// pozadi pod pravou casti stromu (pod radkovymi elementy stromu) , jen v pripade manualniho posuvniku

	CGUIList<CGUITreeItem> *items;	// seznam korenovych polozek stromu
	
	CGUITreeItem* lastTreeItem;		// naposled oznacena polozka stromu

	CTexture *rollTex, *unrollTex, *itemTex;

	CGUIFont* treeTextFont;
	CGUIFont* treeRootTextFont;

	static const int numEvents;
	static const int availableEvents[];
};


//////////////////////////////////////////////////////////////////////
// CGUITreeMarker
// oznacovac polozky stromu
// kazda korenova polozka ma jeden oznacovac pro cely svuj podstrom,
// ten se upravuje podle velikosti polozky, kterou zrovna oznacuje
//////////////////////////////////////////////////////////////////////
class CGUITreeMarker
{
public:
	CGUITreeMarker(float x1, float y1, float x2, float y2, CBltWindow *rootwnd, CBltElem* pos=0);
		// vytvori oznacovac dle zadane velikosti a polohy + vlozi ho do enginu na zadanou viditelnosti pozici
	virtual ~CGUITreeMarker();

	void Mark();
	void UnMark();

	virtual void Rebuild(CGUIElement* elm);	// podle zadaneho elementu predela oznacovac (polohu + velikost)

	bool GetVisible() { return visible; };

protected:
	CBltLine* lines[4];
	CBltRect* rect;
	bool visible;
};


//////////////////////////////////////////////////////////////////////
// CGUITreeSlider
// posuvnik, urcujici pomer mezi levou a pravou casti stromu
// (vertikalni cara, ktera rozdeluje plochu stromu na 2 casti, levou pro polozky a pravou pro jejich radkove elementy)
// ovladani:
// chytne se levym tlacitkem mysi a tahne se doleva ci doprava, pusti se tlacitko mysi => vznikne nova pozice pro posuvnik
// umi i scrollovat oknem pri pokusu o vyjezd mimo viditelnou cast okna se stromem
//////////////////////////////////////////////////////////////////////

class CGUITreeSlider : public CGUIWidget	// objekt pro nastavovani pomeru mezi levou a pravou casti stromu
{
public:
	CGUITreeSlider(CGUITree* _tree, float width, UINT _color=STD_TREE_SLIDER_COLOR, UINT _moveColor=STD_TREE_SLIDER_MOVECOLOR);
		// "color" - barva posuvniku (vertikalni cara o sirce "width")
		// "moveColor" -barva posuvniku pri pohybu

	virtual ~CGUITreeSlider();
		
	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	virtual void SetVisible(int vis);

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int TimerImpulse(typeID timerID, float time);

protected:
	CGUITree* tree;		
	CGUIRectHost *rect;
	bool mouse_button_down;
	bool cursor_resizer;
	float px,py;
	bool timerReq;	// zda byl poslan pozadavek na timer
	int cursor_type;	// typ kurzoru pred zmenou na posuvnikovy kurzor
	UINT color;
	UINT moveColor;

	void FocusOldTreeItem();
	void ChangeTreeSliderPos(float nx);	
};


//////////////////////////////////////////////////////////////////////
// CGUITreeRootItemBackground
// pozadi pod specialni korenovou polozkou
// slozeno ze ctverce a dvou car (nad a pod ctvercem)
// kvuli zvyrazneni korenove polozky
// je nad celou sirkou okna se stromem
//////////////////////////////////////////////////////////////////////
class CGUITreeRootItemBackground : public CGUIWidget	
{
public:
	CGUITreeRootItemBackground(float _x, float _y, float _sx, float _sy, UINT backgroundColor = 0xFFFFFFFF, UINT linesColor = 0xFFFFFFFF);
	virtual ~CGUITreeRootItemBackground();

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();


	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);
	virtual void SetVisible(int vis);

	void SetColor(UINT color, UINT linesColor);

protected:
	CGUIRectHost* rect;
	CGUILine *up, *down;
};


//////////////////////////////////////////////////////////////////////
// CGUITreeItemUserData
// volne definovana trida pro uzivatelska data u polozky stromu
// oddededim se ziskaji netusene moznosti - mit u kazde polozky stromu cokoliv potrebuji
//////////////////////////////////////////////////////////////////////

class CGUITreeItemUserData
{
public:
	CGUITreeItemUserData() {};
	virtual ~CGUITreeItemUserData() {};
};


//////////////////////////////////////////////////////////////////////
// CGUIFileBrowser
// vybirac souboru na disku ve forme stromu
// polozky FB jsou adresare nebo soubory
// zobrazovana jmena u polozek muzou byt libovolna (uzivatelem definovana)
// zpristupni disk od zadane cesty k listum
// neprida vsechny soubory zadane cesty najednou pri vytvoreni, 
// ale postupne po prochazenych adresarich
// pokud uz byl adresar rozbalen, zustavaji jeho polozky vytvoreny a znovu se jiz netvori ani neaktualizuji (napr. pri zmene jmena souboru atp.)
// lze aktualizivat manualne volanim fce - FileSysChanged()
//////////////////////////////////////////////////////////////////////

class CGUIFileBrowser : public CGUITree
{
	friend class CGUIFileBrowserItem;
public:
	CGUIFileBrowser(float _x, float _y, float _sx, float _sy); 
		// vytvori okno pro FB 
		
	virtual ~CGUIFileBrowser();

	CGUITreeItem* AddRootItem(char* path, char* label=0, int roll_state=0);
		// prida dalsi korenovou polozku do stromu urcenou adresarem pro brouzdani po disku "path" a jeho popisek "label"
		// polozka je urcena cestou ke koreni odkud se zacne s brouzdanim po disku "path", popiskem "label", "roll_state" urcuje zda bude adresar rozbalen ci nikoliv
	int GetSelectedFile(typeID treeItem, char **dir, char **file);	
		// vrati soubor odpovidajici polozce "treeItem"
		// v "dir" je string urcujici cestu k souboru (adresari)
		// ve "file" je string urcujici nazev souboru (adresare)
		// dir i file se newvuje => dealokovat
		// vraci -1 pri chybe, 0 - jde o soubor, 1 - jde o adresar (zadana polozka je adresar)
		// nastavi aktualni cestu FS na hlednany soubor

	int GetSelectedFileRelativ(typeID treeItem, char **root, char **dir, char **file);	
		// vrati soubor odpovidajici polozce "treeItem"
		// v "root" je string urcujici cestu ke koreni stromu - muze byt i link ($GAMES$ atp.), pripadne soubor (koren neni cesta, ale primo soubor)
		// v "dir" je string urcujici cestu k souboru (adresari) relativne vzhledem k "root"	(pri zvoleni korenove polozky je "dir" prazdny string dir[0]==0)
		// relativni cesta je bez lomitka na zacatku a na konci
		// ve "file" je string urcujici nazev souboru (adresare)
		// root, dir i file se newvuje => dealokovat
		// vraci -1 pri chybe, 0 - jde o soubor, 1 - jde o adresar (zadana polozka je adresar)

	void SetDirMenu(bool create=true);	
		// prida menu do FB, ktere se vyvola stisknutim praveho tlacitka mysi nad adresarem
		// v menu jsou polozky ovladajici adresar - vytvoreni noveho podadresare, smazani adresare
		// pokud je "create" false, menu zrusi
	void SetFileMenu(bool create=true);
		// prida menu do FB, ktere se vyvola stisknutim praveho tlacitka mysi nad souborem
		// v menu jsou polozky ovladajici doubor - smazani souboru
		// pokud je "create" false, menu zrusi

	void FileSysChanged();
		// vola se po zmene FS, aby se podle zmeny FS prislusne zaktualizoval i strom

	int MarkFileItem(char* root, char *file);
		// rozbali strom k polozce odpovidajici zadanemu souboru (i s relativni cestou od korene stromu)
		// nalezenou polozku oznaci 
		// vrati zda se akce povedla (0) nebo nepovedla (1) - polozka ve stromu neni atp.

protected:
	int SetDir(CGUIFileBrowserItem* ti);	// nastavi aktualni cestu na disku odpovidajici ceste k zadane polozce "ti"
	char* GetDirString(CGUIFileBrowserItem* fbi, int len, CGUIFileBrowserItem** root);	// vrati relativni cestu od korenove polozky stromu k polozce "fbi", 
		//vraceny string dealokovat (v len se predava potrebna velikost stringu pri rekurivnim zanorovani)
		// v "root" vrati adresu korenove polozky od ktere se relativni cesta udava
		// relativni cesta je bez lomitka na zacatku a na konci
	virtual int Filter(CGUITreeItem* parent, const char *name, int dir, int first) { return 1; };
		// urceno k pretizeni uzivatelem
		// filter urcuje zda novou polozku zaradit do stromu nebo ne
		// nova polozka je popsana jmenem souboru/adresare "name", "dir" + zda jde o prvni nalezenou polozku v podadresari
		// "parent" je odkaz na jeji rodicovskou polozku, z ktere lze zjisti cestu na disku k nove polozce
		// vraci 0 - nezarazovat novou polozku, 1 - zaradit do stromu

	static int Compare(CGUITreeItem *a, CGUITreeItem *b);
		// porovnani dvou poozek kvuli razeni
		// nejdrive jsou adresare podle abecedy, potom soubory podle abecedy

	virtual int TreeHandler(typeID treeItem, int index, int state);
		// urceno k pretizeni uzivatelem
		// muze si sem rovnou naprogramovat reakce na zmacknutou polozku
		// zarizuje otevreni menu pro soubory / adresare, pokud jsou vytvoreny
	virtual void TreeUpdateHandler(typeID treeItem, int index);
		// zarizuje pridavani souboru do stromu po prvnim kliknuti na adresar

	virtual CGUIFileBrowserItem* AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir);
		// pomocna fce pro TreeUpdateHandler
		// zarizuje vytvoreni nove polozky a jeji pridani do stromu
		// "fbi" - materska polozka, kam se bude pridavat nova polozka ("name","dir")
		// uzivatel muze predefinovat / upravit => napr ruzne ikony podle typu souboru atp.
		
	virtual CGUITreeItem* AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool _manualUpdate = false, bool _rootItemBackground=false, bool first=false);
		// kvuli pridavani oddedene polozky CGUIFileBrowserItem misto klasicke CGUITreeItem

	virtual void EventHandler(CGUIEvent *event);	// pridava zpracovani udalosti od menu FB (createDir, delete, ...)

	typeID dirMenuID, fileMenuID;	// menu pro vytvareni adresaru, mazani atp.
	typeID msgBoxID;	// pro menu - potvrzovani volby MsgBoxem
	int msgBoxIndex;	// ktera polozka v menu byla vybrana
	typeID selecetedFileItemID;
	char* newDirName;
};

// oddedena polozka stromu od CGUITreeItem kvuli novym parametrum - "dir", "name" nutnym pro funkcnost FB
class CGUIFileBrowserItem : public CGUITreeItem
{
public:
	CGUIFileBrowserItem(CGUITree* _tree, char *labelString, bool _rootTree, CGUIRectHost *_picture=0, char *_help=0, bool rootItemBackground=false);
	virtual ~CGUIFileBrowserItem();

	virtual CGUITreeItem* AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool _manualUpdate = false, bool first=false);

	void FileSysChanged();
	void UpdateNewFiles();		// prida do polozky stromu nove soubory a adresare
	CGUIFileBrowserItem* FindFile(const char* name);	// vrati zda adresar obsahuje zadany soubor 

	int MarkFileItem(char* name);

	int dir;	// zda je polozka adresar/soubor
	char* name;	// jmeno souboru/adresare - string
};

#endif