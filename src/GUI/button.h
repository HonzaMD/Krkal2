///////////////////////////////////////////////
//
// button.h
//
// Interface pro Button
// Interface pro CheckBox
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef BUTTON_H
#define BUTTON_H

#include "element.h"
#include "gui.h"

class CGUIFont;
class CGUIRectHost;
class CGUIStaticText;
class CGUIStyle;
class CGUIStandard9;

//////////////////////////////////////////////////////////////////////
// CGUIButton
//////////////////////////////////////////////////////////////////////

// tlacitko
// 2 zakladni typy
//	- plnohodnotne (postavene na 9 ctvercich, ktere se roztahuji v zavisloti na velikosti tlacitka)
//	- jednoduche (postavene na 1 ctverci, ktery se take muze roztahnout, ale hnusne) - simple_button
// tlacitko je postaveno na 3 stylech
// styl urcuje jak vypada v jednom ze stavu
//	up - tlacitko nahore (neni zmacknute, ani neni nad nim mys)
//	mark - tlacitko neni zmacknute, je nad nim mys
//	down - tlacitko je zmacknute dolu
// pripadne muze byt nastaven i styl "disabled", pro pripad deaktivace tlacitka (nelze mackat)
// reakci na stisk tlacitka si muze klient definovat pretizenim metody OnClick(), nebo
// objednanim udalosti EClicked pres "button->AcceptEvent(klientID, EClicked)" a reakci na prislou
// udalost v klientskem EventHandleru
// tlacitko lze stisknout levym tlacitkem mysi nebo klavesou enter a space

//styl pro button
// udrzuje informace podle kterych se da zkonstruovat button
// lze pouzit pro jednoduchy i kompletni button
// popis obsahu stylu je nize
struct CGUIButtonStyle : public CRefCnt
{
	CGUIButtonStyle(CGUIStyle *up, CGUIStyle *down, CGUIStyle *mark, CGUIStyle *disabled, CGUIFont *font, int simple_button);
	~CGUIButtonStyle();

	CGUIStyle *up,*down,*mark,*disabled;
	CGUIFont *font;
	int simple_button;
};

class CGUIButton : public CGUIWidget, public CGUIEventSupport		// cudlitko
{
public:
	// poloha, velikost, styly (jak cudlik vypada pri stisknuti, pusteni, oznaceni=najeti mysi nad nej)
	// styly musi mit 4 parametry (minimalni (standardni) velikost [x,y], posun textu/obrazku od stredu [x,y])
	// textury stylu jsou ocislovany podle polohy v ramci buttonu takto:	1,2,3
	//																		4,0,5
	//																		6,7,8
	
	// u simpleButtonu (tedy postaveneho na jednom ctverci) musi mit styl 6 parametru
	// prvni 4 jsou stejne jako u normalniho buttonu,
	// 5. a 6. parametr urcuje posunuti grafiky (textury) buttonu vuci jeho poloze
	// 0 = zadny posun, - = posunuti doleva/nahoru, + = posunuti doprava/dolu
	// umoznuje vetsi texturu nez je skutecna (mackatelna) velikost buttonu

	CGUIButton(float _x, float _y, float _sx, float _sy, char *style, char *label, CGUIRectHost *_picture=0, char *_help=0);
		// vytvori tlacitko na miste x,y o velikosti sx,sy
		// style je jmeno tlacitkoveho stylu - CGUIButtonStyle, ktery se vyzvedne z RefMgr, kde musi byt predem registrovan
		// label je popisek na buttonu
		// picture je obrazek na buttonu
		// help je kontextova napoveda
	CGUIButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char *_help=0);	
		// konstruktor pro volne roztazitelne cudlitko (postavene na zaklade 9 ctvercu)
	CGUIButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool _resizable, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char *_help=0);	
		// konstruktor pro jednoduche cudlitko (postavene na jednom ctverci - nelze roztahovat nebo hnusne)
		// pokud se zada velikost zaporna, pouzije se standardni velikost z parametru stylu
	virtual ~CGUIButton();
	
	void SetLabel(CGUIStaticText *_label);
	void SetLabel(char *text, CGUIFont *font=0);	
		// font muze byt i 0 - potom se pouzije font ze stylu (pokud je nastaven)
	void SetLabelColors(UINT _labelColorUp, UINT _labelColorMark, UINT _labelColorDown, UINT _labelColorDisable);
	void SetPicture(CGUIRectHost *_picture);
	void SetPicture(char *picture_name);
	void ReStylize(CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark);	// zmeni styly buttonu (kompletni zmena sady grafiky cudlitka)
	void ChangeContextHelp(char* text);			// zmeni text contexthelpu cudlitka
	void SetInactiveStyle(CGUIStyle *_inactive);
	void SetInactivePicture(CGUIRectHost *_pictureInactive);

	int Activate();		// aktivuje cudlitko z pasivniho stavu na plne funkcni stav
	int Deactivate();	// deaktivuje cudlitko do pasivniho (nefunkcniho) stavu, neklikatelneho
	bool IsActive() { return active; };

	void Click();	
		// vyvola kliknuti cudlitka (vola se fce OnClick() a posilaji udalosti, neprovadi se zadna zmena grafiky)
		// programove kliknuti tlactika (kvuli rozeslani informace o kliknuti vsem observerum tlacitka)

	int GetState() { return button_state; }; // stav cudlitka: up/mark/down (0/1/2)

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	
	virtual void SetVisible(int vis);

	virtual void ParentNotification();

	bool keySensitive;		// zda tlacitko reaguje na klavesnici (zmacknuti, posun na dalsi TAB element apt.)

protected:
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);	// zarizuje zmenu stylu + detekuje click
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	virtual void OnClick();	// pro pretizeni uzivatelem, lze definovat cinnost po kliknuti
	virtual int Keyboard(UINT character, UINT state);
	virtual int KeyboardUp(UINT character, UINT state);


	void MoveLabelPicture(float dx, float dy);	// posune popiskem a obrazkem tlacitka podle zmeny stylu (dx,dy)
	void CenterLabelPicture();	// vycentruje popisek a obrazek tlactika
	void SetStyle(CGUIStyle *style, float dx=0, float dy=0);	// predela rect cudlitka na novy styl 
		// + zmeni velikost rectu  na sx+dx,sy+dy (rozdil ve velikosti mezi novym stylem a up-stylem (podle up-stylu) se urcuje pocatecni velikost tlacitka)
	void CreateButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char *_help=0);	
		// konstruktor pro volne roztazitelne cudlitko (postavene na zaklade 9 ctvercu)
	void CreateSimpleButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool _resizable, CGUIStaticText *_label=0, CGUIRectHost *_picture=0, char *_help=0);	
		// konstruktor pro jednoduche cudlitko (postavene na jednom ctverci - nelze roztahovat nebo hnusne)

	CGUIStandard9 *st9;		// objekt zajistujici zobrazovani buttonu vsech velikosti a stylu
	CGUIRectHost *simpleButton;	// grafika jednoducheho cudlitka (singleFrame) +
		// zda se jedna o jednoduche cudlitko (1 ctverec) nebo plnohodnotne (9 ctvercu ~ == 0)

	bool resizable;			// zda je umozneno menit velikost cudlitka

	bool active;			// zda je tlacitko aktivni, tj. funkcni (pasivni=sede a nemackatelne)
	int button_state;		// stav cudlitka: up/mark/down (0/1/2)
	bool button_was_down;	// zda byl button stlacen a vyjeto mimo nej
	CGUIStyle *up,*down,*mark, *inactive;	// pouzite styly 
	CGUIStaticText *label;		// popisek (nemusi byt)
	CGUIRectHost *picture;	// aktualne pouzity obrazek
	CGUIRectHost *pictureInactive;	// obrazek pro neaktivni cudlik
	CGUIRectHost *pictureActive;	// obrazek pro aktivni cudlik
	char *help;					// kontextova napoveda

	CBltWindow *rootwnd;	// ukazatel do enginu (kvuli meneni grafiky popisku a obrazku)
	bool keyDown;			// zda byla zmacknuta klavesa enter nebo space

	UINT labelColorUp, labelColorMark, labelColorDown, labelColorDisable;
	CGUIFont* labelFont;

	static const int numEvents;
	static const int availableEvents[];
};



//////////////////////////////////////////////////////////////////////
// CGUICheckBox
//////////////////////////////////////////////////////////////////////

// zaskrtavatko
// tlacitko udrzujici si svuj stav - zaskrtnute/nezaskrtnute
// menit stav lze programove - ChangeState(state) nebo
// stisknutim leveho tlacitka mysi na obrazku ci popisku CheckBoxu,
// nebo klavesou return a space
// zjistit stav lze metodou GetState() nebo propojenim pres protokol DataExchange

class CGUICheckBox : public CGUIWidget, public CGUIEventSupport, public CGUIDataExchange
{
public:
	CGUICheckBox(float _x, float _y, CGUIStaticText *_label, int _main_state, bool uncheckable = true, UINT color = STD_CHECKBOX_COLOR, CGUIStyle *_check = styleSet->Get("CB_Check"), CGUIStyle *_uncheck = styleSet->Get("CB_UnCheck"));
		// vytvori CheckBox, _main_state urcuje zda ude zaskrnut (1) nebo nebude (0), uncheckable urcuje zda lze tlacitko odoznacit
		// color a _check, _uncheck urcuji vzhled CheckBoxu
		// styl vzhledu musi mit jednu texturu, urcujici, jak bude CheckBox vypadat 
	CGUICheckBox(float _x, float _y, char* text, int _main_state, bool uncheckable = true, UINT color = STD_CHECKBOX_COLOR, CGUIStyle *_check = styleSet->Get("CB_Check"), CGUIStyle *_uncheck = styleSet->Get("CB_UnCheck"));
		// vytvori CheckBox, _main_state urcuje zda ude zaskrnut (1) nebo nebude (0), uncheckable urcuje zda lze tlacitko odoznacit
		// color a _check, _uncheck urcuji vzhled CheckBoxu
		// styl vzhledu musi mit jednu texturu, urcujici, jak bude CheckBox vypadat 
	virtual ~CGUICheckBox();

	void SetLabel(CGUIStaticText *_label);
	void SetInactiveStyle(CGUIStyle *_check, CGUIStyle *_uncheck, UINT color = STD_CHECKBOX_INACTIVE_COLOR);
		// nastavi, jak bude CheckBox vypadat v neaktivnim stavu (nemackatelnem)
	
	void ChangeState(int state);
		// zmeni stav CheckBoxu (1 - zaskrtnut, 0 - nezaskrtnut)

	int GetState() { return main_state; };	// true == check, false == uncheck

	void ChangeContextHelp(char* text);			// zmeni text contexthelpu cudlitka

	int Activate();		// aktivuje cudlitko z pasivniho stavu na plne funkcni stav
	int Deactivate();	// deaktivuje cudlitko do pasivniho (nefunkcniho) stavu, neklikatelneho
	bool IsActive() { return active; };


	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	
	virtual void SetVisible(int vis);

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);	// zarizuje zmenu stylu + detekuje click
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual void OnClick();
	virtual int SetParametr(void *value, int type, int size, int which=0);
	virtual void ParentNotification();

	int Keyboard(UINT character, UINT state);

	virtual void EventHandler(CGUIEvent *event);

	static const int numEvents;
	static const int availableEvents[];

protected:
	void SetPositionOfLabelPicture();					// nastavi pozici obrazku a popisku
	int main_state;		// hlavni stav up/down (0/1) uncheck/check

	bool active;			// zda je tlacitko aktivni, tj. funkcni (pasivni=sede a nemackatelne)
	int button_state;		// stav cudlitka: up/markUp/down/downMark (0/1/2/3)
	bool button_was_down;	// zda byl button stlacen a vyjeto mimo nej
	CGUIStaticText *label;		// popisek (nemusi byt)
	CGUIRectHost *pictureUnCheck;
	CGUIRectHost *pictureCheck;
	CGUIRectHost *pictureInactiveCheck;
	CGUIRectHost *pictureInactiveUnCheck;
	char *help;		// kontextova napoveda
	CBltWindow *rootwnd;	// ukazatel do enginu (kvuli meneni grafiky popisku a obrazku)

	bool raisable;		// zda lze zamacknute tlacitko vymacknout kliknutim
};

#endif