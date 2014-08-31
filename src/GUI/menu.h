///////////////////////////////////////////////
//
// menu.h
//
// Interface pro menu (ContextMenu)
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#ifndef MENU_H
#define MENU_H

#include "element.h"
#include "widgets.h"
#include "primitives.h"


class CGUIMenu;
class CGUIMenuMarker;

//////////////////////////////////////////////////////////////////////
// CGUIMenuItem
// polozka menu, zajistuje grafiku i funkcnost jedne polozky menu
// polozka obsahuje: popisek a volitelne obrazek, klavesovou zkratku a napovedu
//////////////////////////////////////////////////////////////////////

class CGUIMenuItem : public CGUIElement, public CGUIEventSupport
{
	friend class CGUIMenu;

public:
	CGUIMenuItem(CGUIMenu* menu, float _x, float _y, float _sx, float _sy, char *labelString, CGUIRectHost *_picture=0, char *_help=0);
	virtual ~CGUIMenuItem();

	void SetLabel(char *text);	// nastavi text polozky, dle standardniho formatu menu
	void SetLabelFormated(char *text, CGUIFont* font, UINT color=STD_MENU_TEXT_COLOR);	// nastavi text polozky, dle zadaneho formatu
	void SetPicture(CGUIRectHost *_picture);
	void ChangeContextHelp(char* text);			// zmeni text contexthelpu 
	void SetInactivePicture(CGUIRectHost *_pictureInactive);	// jaky obrazek bude pouzit v pripade deaktivace polozky menu
	void SetExtenderPicture(CGUIRectHost *_pictureExtender);	// nastavi obrazek symbolizujici, ze polozka otevira dalsi submenu

	int Activate();		// aktivuje polozku menu z pasivniho stavu na plne funkcni stav
	int Deactivate();	// deaktivuje polozku menu do pasivniho (nefunkcniho) stavu, neklikatelneho
	bool IsActive() { return active; };

	void SetShortCut(char* text, bool key_ctrl,	bool key_alt, bool key_shift, int char_code, bool exclusive_access=false);
	// nastavi klavesovou zkratku
	// text je popis zkratky zobrazeny na polozce (pro uzivatele)
	// key_? urcuje zda ma byt klavesa stisknuta pro vyvolani polozky
	// char_code urcuje klavesu zkratky
	// zkratka funguje i kdyz menu neni aktivni (zobrazene), nefunguje ovsem kdyz je polozka deaktivovana
	// "exclusive_access" urcuje zda zkratka funguje i pres nastaveny exkluzivni pristup ke klavesnici u jineho elementu

	typeID GetMenuID() { return menuID;};

	virtual void OnClick();	// fce pro uzivatelske pretizeni - vola se pri stisku polozky


	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void Resize(float _sx, float _sy) {};	
	virtual void Move(float _x, float _y);
	virtual void BringToTop() {};
	virtual void SetVisible(int vis) {};

protected:
	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	void SetPositionOfLabelPicture();
	virtual void CreateMarkStructures(CBltWindow* win);	// pretizeni fce elementu kvuli vytvoreni vlastniho oznacovace, nepouziva se standardni

	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	//virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int Keyboard(UINT character, UINT state);
	virtual void EventHandler(CGUIEvent *event);
	

	static const int numEvents;
	static const int availableEvents[];


	bool active;		// zda je polozka aktivni, tzn. pouzitelna
	bool markItem;		// zda je polozka oznacena

	CGUIStaticText *label;
	CGUIRectHost *picture;		// pointer na aktualne pouzity obrazek (aktivni/neaktivni)
	CGUIRectHost *pictureInactive, *pictureActive;	// pokud pictureInactive neni nastaven, pouzije se v pripade deaktivace polozky pictureActive

	CGUIRectHost* pictureExtender;		// obrazek symbolizujici podmenu

	char *help;

	CBltWindow *rootwnd;		// odkaz na okno enginu

	CGUIStaticText *keyShortCut;	// klavesova zkratka, ktera vyvola stisk polozky (text)
	bool sc_key_ctrl;			// stav klavesy (zda ma byt stisknuta / nikoliv)
	bool sc_key_alt;			// stav klavesy (zda ma byt stisknuta / nikoliv)
	bool sc_key_shift;			// stav klavesy (zda ma byt stisknuta / nikoliv)
	int sc_charCode;			// charCode, ktery ma byt stisknut
	bool key_exclusive_access;	// urcuje zda zkratka funguje i pres nastaveny exkluzivni pristup ke klavesnici u jineho elementu

	typeID menuID;
	int index;			// poradi polozky v menu (0..n-1)
						// taborder polozky odpovida jejimu indexu (poradi)
	int subMenuIndex;	// zda polozka aktivuje submenu (!=-1), o kolikate submenu jde (0..k, k<=n-1)
};


//////////////////////////////////////////////////////////////////////
// CGUIMenu
// kontextove menu
// obsluhuje seznam polozek, polozka muze otevrit i dalsi submenu
// menu se aktivuje (zobrazuje) manualne nebo automaticky
//  - manualne volanim fce On / Off
//  - automaticky nastavenim automatickeho aktivatora + akce na nem (EClick, EMouseButton, ...)
//  - automaticka deaktivace (skryti) probehne potvrzenim nektere polozky menu, kliknutim mimo menu, ESC
//		potvrzeni polozky se uskutecni pomoci: kliknuti levym nebo pravym tlacitkem mysi, klavesnici (enter), nebo nastavenou klavesovou zkratkou (i kdyz neni menu zobrazeno)
// obsluha stisknute polozky menu muze byt naprogramovana tremi zpusoby:
// 1) pretizenim MenuItem::OnClick() = obsluha konretni polozky
// 2) pretizenim  Menu::MenuHandler() = obsluha vsech polozek menu (uz ne jeho subMenu)
// 3) objednanim udalosti EMenu a reakci na ni ve svem EventHandleru (jedna udalost jen na jedno Menu, u kazdeho subMenu se musi objednavat zvlast)
//////////////////////////////////////////////////////////////////////

class CGUIMenu : public CGUIMultiWidget
{
	friend class CGUIMenuItem;
public:
	CGUIMenu(float _itemWidth=STD_MENU_WIDTH, float _itemHeight=STD_MENU_HEIGHT);	// vytvori prazdne menu o zadane velikosti (menu neumi scrollovat)
			// menu se pri konstrukci samo umisti do desktopu a skryje se 
	virtual ~CGUIMenu();
			// pri desktrukci se menu samo vyjme z desktopu

	CGUIMenuItem* AddMenuItem(typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool first=false);
	// vytvori novou polozku s parametry (labelString, _picture, _help) a prida ji do menu za polozku where,
	// pokud je where==0 prida ji na konec (za posledni polozku), pokud je (where==0 && first==true) prida polozku na zacatek (pred prvni polozku)
	// do where se muze zadat i ID oddelovace polozek, potom se nova polozka prida za oddelovac
	// vrati adresu nove polozky
	CGUIMenuItem* AddMenuItem(typeID where, CGUIMenuItem* menuItem, bool first=false);
	// prida polozku "menuItem" do menu za polozku where,
	// pokud je where==0 prida ji na konec (za posledni polozku), pokud je (where==0 && first==true) prida polozku na zacatek (pred prvni polozku)
	// do where se muze zadat i ID oddelovace polozek, potom se nova polozka prida za oddelovac
	// vrati adresu nove polozky
	void DeleteMenuItem(CGUIMenuItem* mi);	// odstrani polozku z menu a ostatni polozky patricne precisluje (index)

	void AddGap(typeID where, bool first=false);	// prida oddelovac polozek (caru) za polozku where
													// pokud je where==0 prida ho na konec (za posledni polozku), pokud je (where==0 && first==true) prida oddelovac na zacatek (pred prvni polozku)
	void DeleteGap(typeID where);

    CGUIMenuItem* AddSubMenuItem(CGUIMenu* subMenu, typeID where, char *labelString, CGUIRectHost *_picture=0, char *_help=0, bool first=false);
	// vytvori novou polozku s parametry (labelString, _picture, _help) a prida ji do menu za polozku where,
	// pokud je where==0 prida ji na konec (za posledni polozku), pokud je (where==0 && first==true) prida polozku na zacatek (pred prvni polozku)
	// do where se muze zadat i ID oddelovace polozek, potom se nova polozka prida za oddelovac
	// + zaridi, ze pri stisku polozky se otevre zadane subMenu
	// udalost EMenu si obsluzny program musi od subMenu objednat sam, zadne automaticke objednavani se neprovadi
	// vrati adresu nove polozky
	CGUIMenu* DeleteSubMenuItem(CGUIMenuItem* mi);	// vrati subMenu, jehoz aktivacni polozka byla odstanena z menu


	virtual void MenuHandler(typeID activator, typeID menuItem, int index) {};
	// fce pro uzivatelske pretizeni - vola se pri stisku nektere polozky menu
	//  - activator je ID elementu, na kterem bylo menu vyvolano
	//  - menuItem je prave stisknuta polozka
	//  - index je poradi stisknute polozky v menu

	void On(typeID _activator, float mx=-1, float my=-1);	// aktivuje menu na pozici mx,my (jinak na pozici kurzoru), activator je element, na kterem se menu vyvolalo
	void Off();	// vypne menu (prestane se zobrazovat) 
	bool isActive() { return active;};

	int SetAutoActivator(CGUIEventSupport* elm, int action = 0);
		// nastavi automatickeho aktivatora na zadany element s akci:
		//	- EMessage> stisknuti praveho tlacitka mysi vevnitr okna mimo element okna
		//	- EClicked> kliknuti mysi nad elementem (leve tlacitko, stisk a spusteni)
		//  - EMouseButton> zmacknuti mysi nad elementem (jakekoliv tlacitko)
		// v pripade, ze je action nenastaveno (==0) vybere se typicka akce podle elementu
		// vraci 0 - pri uspechu, 1 - pri neuspechu

	CGUIMenuItem* GetItem(int index);		// vrati polozku daneho indexu
	typeID GetParentMenuActivatorItem() { return parentMenuActivatorItem;};	// vrati ID polozky rodicovskeho menu, ktera aktivuje toto menu (subMenu)

	void Add();		// prida menu do systemu (do predku desktopu)
					// pri vytvoreni menu se pridava automaticky
	void Remove();	// odebere menu z desktopu

	float GetPictureStripSize() {return picture_strip_size;};	// sirka prouzku u kontextoveho menu (prouzek je podkladem pro obrazky u polozek menu)
	float GetMenuPictureSize() {return menu_picture_size;};		// velikost obrazku u polozek menu
	void SetPictureStripSize(float _picture_strip_size);	// sirka prouzku u kontextoveho menu (prouzek je podkladem pro obrazky u polozek menu)
	void SetMenuPictureSize(float _menu_picture_size);		// velikost obrazku u polozek menu

protected:
	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState){return 1;};
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState){return 1;};
	virtual int MouseMiddle(float x, float y, UINT mouseState, UINT keyState){return 1;};
	virtual int MouseOver(float x, float y, UINT over,UINT state){return 1;};
	virtual int Wheel(float x, float y, int dw, int dz, UINT state){return 1;};
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID){return 1;};
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseMiddleFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int Keyboard(UINT character, UINT state);
	virtual void EventHandler(CGUIEvent *event);
	virtual int ForwardMsg(CGUIMessage *msg);

	void AddElemBehind(CGUIElement *where, CGUIElement *element);	// jako AddElem, ovsem pridava element za where ne na zacatek
																	// pouziva se kvuli tomu, aby polozky v menu byly serazeny podle sve polohy od shora dolu
																	// kvuli pridavani v urcenem poradi
	void MoveItems(CGUIElement *start, float dy, int d_index);	// posune polozky a mezery v menu, pocinaje nasledujici polozkou po startu (start==0 jede od zacatku seznamu) o dy a pricte k indexu polozek d_index
																// pouziva se pri pridavani nove polozky mezi stavajici
	CGUIElement* FindLastItem();	// vrati posledni polozku menu v seznamu (menuItem / menuGap)
	int FindIndex(CGUIElement *start);	// vrati index el. "start", pokud ho nema (nejedna se o polozku), nalezne index nejblizsiho predchudce el. "start" v seznamu
	
	//void SetSubMenuPrograms(CGUIMenu* subMenu);		// objedna udalost EMenu od subMenu vsem programum, kteri maji objednanou udalost EMenu od tohoto menu
	void ActivateSubMenu(typeID subMenuID, CGUIMenuItem* mi);	// aktivuje subMenu "subMenuID" polozkou "mi"

	void Update();		// volat se po nastaveni SetPictureStripSize / SetMenuPictureSize pro aktualizaci polozek menu

	typeID activator;			// ID elementu, na kterem se menu aktivovalo (fci On())
	CGUIMenu* parentMenu;		

	CGrowingArray<typeID> subMenuArray;	// pole ID subMenu
	typeID parentMenuActivatorItem;		// ID polozky rodicovskeho menu, ktera aktivuje toto menu (subMenu)

	typeID menuActivatorItem;	// ID polozky menu, ktera aktivovala subMenu (prave aktivni subMenu)
	bool active;				// zda je menu aktivovano
	bool preclick;				// zda bylo stisknuto prave tlacitko nad aktivacnim elementem
								// kvuli automaticke aktivaci pustenim praveho tlacitka nad nastavenym aktivacnim elementem

	float itemHeight, itemWidth;	// standardni velikost polozky

	float picture_strip_size;		// sirka prouzku (prouzek je podkladem pro obrazky u polozek menu)
	float menu_picture_size;		// velikost obrazku u polozek menu

	CGUIStaticPicture* strip;	// odkaz na prouzek (kvuli aktualizaci velikosti)

	static const int numEvents;
	static const int availableEvents[];
};



//////////////////////////////////////////////////////////////////////
// CGUIMenuMarker
// zajistuje oznacovani polozek menu
// kazda polozka ma vlastni, ktery se zobrazuje/skryva
//////////////////////////////////////////////////////////////////////
class CGUIMenuMarker : public CGUIMarker
{
public:
	CGUIMenuMarker(CBltWindow *rootwnd, float x1, float y1, float x2, float y2);	// vytvori oznacovac na zadanem miste, viditelny
	~CGUIMenuMarker();

	void Mark();	// zviditelni oznacovac
	void UnMark();	// zneviditelni oznacovac

	virtual void Rebuild(CGUIElement* elm);	// prebuduje oznacovac (velikost a polohu) podle zadaneho elementu

	bool GetVisible() { return visible; };

protected:
	CBltLine* lines[4];
	CBltRect* rect;
	bool visible;
};


//////////////////////////////////////////////////////////////////////
// CGUIMenuGap
// oddelovac polozek menu - cara
// nepouziva se klasicka cara kvuli tomu, ze vyzadujeme od oddelovace ID, abychom mohli radit polozky menu
//////////////////////////////////////////////////////////////////////
class CGUIMenuGap : public CGUILine, public CGUIEventProgram
{
public:
	CGUIMenuGap(float x1, float y1, float x2, float y2, UINT color = 0xFFFFFFFF)
		: CGUILine(x1,y1,x2,y2,color) {} ; 
};

#endif