///////////////////////////////////////////////
//
// panel.h
//
// Interface pro WindowPanel a jeho casti WinPanelButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef PANEL_H
#define PANEL_H

#include "element.h"

class CGUIButton;
class CGUIStaticText;
class CGUIRectHost;
class CGUIStyle;
class CGUIStandard9;

class CGUIWinPanelButton;

//////////////////////////////////////////////////////////////////////
// CGUIWindowPanel
//////////////////////////////////////////////////////////////////////

// objekt predstavujici neco jako je taskbar v MS Windows
// jedna se o misto, na kterem jsou umisteny ovladaci prvky:
// - ikony, ktere vytvari dialogy (okna)
// - ikony, ktere rozbaluji menu
// - "tlacitka", ktera odpovidaji zobrazenym oknum (dialogum),
//		pomoci nich lze prepinat mezi okny, minimalizovat a obnovovat okna
//		proste chovani je podobne jako ve windows
// umoznuje prepinani mezi okny pomoci CTRL+TAB - cykli mezi vsemi vytvorenymi okny, vybira se to, ktere dostane focus
// CTRL+~ - zrusi zapocaty vyber a navrati focus do puvodniho stavu
// CTRL+Shift - cykli mezi vnitrnimi okny focusovaneho okna

// ikony lze na panel pridavat pomoci fce: AddIcon()
// mezi ikony lze vkladat mezery pro prehlednost cleneni - AddGap()
// "tlacitka" odpovidajici zobrazenym oknum se vytvari automaticky s vytvorenim okna
//	- lze zakazat pro konkretni okno - atribut "onWindowPanel"

class CGUIWindowPanel : public CGUIMultiWidget
{
	friend class CGUIWindow;
public:
	CGUIWindowPanel(float _x, float _y, CGUIStyle *style, CGUIWindow* _panelWindow, bool front = true, bool _position_up = true, float iconStartX = 10, float iconEndX = 0);
		// vytvori panel na pozici x,y
		// style musi obsahovat jednu texturu, ktera se pouzije jako podklad panelu (svisly pruh se rotahne na sirku panelu)
		// panel se umisti do _panelWindow, ktere si rovnou upravi sve paramtery velikosti (backwindow atp.)
		// front urcuje zda se panel vlozi do predku okna nebo dovnitr
		// _position_up urcuje zda se vlozi nahoru (hned pod titulek okna) nebo do spodku okna
		// iconStartX - urcuje od jake pozice se na panelu budou moci umistovat ikony
		// iconEndX - urcuje maximalni pozici pro ikony (0==neomezeno)
		// panel je vetsi, nez skutecne zabira (vytvari si vetsi okno pro sebe),
		//	kvuli tomu, aby ikony, ktera ma na sobe mohly precuhovat

	virtual ~CGUIWindowPanel();

	CGUIButton* AddIcon(CGUIStyle *up, CGUIStyle *down, CGUIStyle *mark, char *help=0);
		// styly odpovidaji stylum pro simpleButton, velikost textur (up=down=STD_PANEL_ICON_SIZE, mark=MARK_PANEL_ICON_SIZE)
		// vrati ID buttonu, ktery vytvoril	
	int	AddIcon(CGUIButton* icon, float vertical_shift = STD_ICON_SHIFT);
		// prida na task_bar jiz vytvorenou ikonu (kvuli pridavani vlastnich uzivatelskych cudlitek (oddedenych kvuli predefinovani OnClick()))
		// vrati 0 kdyz bylo pridano, 1 kdyz nebylo (malo mista, ...)
		// vertical_shift urcuje o kolik je cudlitko posunuto dolu vuci kraji task_baru
	void AddGap(float gap);	// vlozi mezeru mezi cudlitka
	void DeleteIcon(typeID icon);	// smaze ikonu z panelu

protected:
	void AddButton(CGUIWindow* wnd);
		// prida "tlacitko" na panel odpovidajici zobrazenemu oknu "wnd"
		// tlacitko se svaze s oknem, pouzije jeho ikonu a titulek
	void DeleteButton(CGUIWinPanelButton* wpb);
		// smaze "tlacitko" na panelu


	virtual void Resize(float _sx, float _sy);
	virtual int IsInPrecise(float _x, float _y);	// oddeluje skutecnou velikost panelu od precuhovaci casti
	virtual int ForwardMsg(CGUIMessage *msg);	// kvuli zmene viditelnosti elementu pod mysi se musi predefinovat od standardu

	virtual void EventHandler(CGUIEvent *event);
		// reaguje na pridavani a odebirani oken tvorbou a rusenim tlacitek
		// zajistuje prepinani mezi okny pomoci klavesnice (ctrl+TAB,...)

	void ResizeButtons();	// "tlacitka" meni svou velikost v zavislosti na poctu tlacitek a velikosti panelu, aby se jich na panel veslo co nejvice
	void MoveButtons();		// zmena polohy tlacitek po odebrani tlacitka z uvodu (ostatni se posunou o jeho misto doleva)
	void CorrectVisibilityButtons();	// spravi viditelnost cudlitek podle jejich poctu, velikosti a velikosti vymezene oblasti na panelu
										// pokud se jiz tlacitka na panel nevejdou (ani zmensena), nejsou videt
										// po odebrani nekterych tlacitek se zviditelni
	typeID GetSelectedButton();		// jedno z tlacitek je "vybranne"
		// tj. zamacknute (odpovida oknu, ktere ma focus)
	CGUIWinPanelButton* GetAssociatedButton(typeID windowID);
		// fce pro zadane okno najde tlacitko, ktere mu odpovida

	CGUIWindow* panelWindow;	// odkaz na okno, ve kterem je CGUIWindowPanel
	float newX;					// x-poloha, kam se bude umistovat nove cudlitko
	float sizeX;				// soucasna velikost cudlitek na panelu
	float minX, maxX;			// minimalni a maximalni pozice pro cudlitka
	float originalMinX;			// puvodni minimalni pozice pro cudlitka (minX se muze zvetsit pridavanim ikon)
	int numButtons;				// pocet cudlitek na panelu
	bool overfull;				// zda je panel jiz preplnen cudlitky (dalsi se jiz nevejde, uz ani nelze zmensit)

	bool ctrl_pressed;			// zda byl stisknut control (kvuli prepinani oken)
	typeID selectedButtonID;	// ID tlacitka, ktere je nyni vybrano pomoci prepinani mezi okny pres CTRL+TAB
	typeID originalButtonID;	// ID tlacitka, ktere bylo vybrano pred zapocetim prepinani

	bool position_up;			// zda je panel umisten nahore v okne nebo dole v okne

	float startX;	// pozice kde zacina oblast pro ikony na panelu
	float endX;		// maximalni pozice pro ikony (za tuto pozici nemouhou byt pridany => tudiz se jiz dalsi neprida)
					// == 0 (bez omezeni)
	float iconX;	// pozice pro umisteni nove ikony
};


//////////////////////////////////////////////////////////////////////
// CGUIWinPanelButton
//////////////////////////////////////////////////////////////////////

class CGUIWinPanelButton : public CGUIWidget, public CGUIEventSupport
{
	friend class CGUIWindowPanel;
public:
	// vytvori tlacitko podle zadanych parametru, ktere odpovidaji konstruktoru Buttonu
	// navic je parametr _windowID, ktery urcuje ID okna, s nimz bude tlacitko svazano
	// tlacitko se lisi od klasickeho buttonu tim, ze ma 4 stavy namisto 3
	// krome up, mark, down je tu navic downMark
	// klasicke tlacitko se po zmacknuti rovnou vraci do stavu "up"
	// panelove tlacitko ma 2 hlavni stavy (up/down), kazdy muze byt navic oznacen najetim mysi (upMark, downMark)
	// mezi hlavnimi stavy se prepina stiskem tlacitka (jeden stisk na prechod z up do down, dalsi stisk na prechod z down na up)
	CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, char *_labelString=0, CGUIRectHost *_picture=0);
	CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, char *_labelString=0, CGUIRectHost *_picture=0);
	CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, typeID _windowID);
	CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, typeID _windowID);
	virtual ~CGUIWinPanelButton();
	
	void SetLabel(char *text);
	void SetPicture(CGUIRectHost *_picture);
	void SetPicture(char *picture_name);
	void ChangeState(bool state);	// meni hlavni stav tlacitka - up/down (0/1)	

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	
	virtual void SetVisible(int vis);

protected:
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);	// zarizuje zmenu stylu + detekuje cl
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual void OnClick();


	virtual void EventHandler(CGUIEvent *event);

	static const int numEvents;
	static const int availableEvents[];

	void MoveLabelPicture(float dx, float dy);		// posune obrazek a popisek, podle stavu tlacitka (nahore/dole)
	void SetPositionOfLabelPicture();					// nastavi pozici obrazku a popisku
	void SetStyle(CGUIStyle *style, float dx=0, float dy=0);	// predela rect cudlitka na novy styl 
	// + zmeni velikost rectu  na sx+dx,sy+dy (rozdil ve velikosti mezi novym stylem a up-stylem (podle up-stylu) se urcuje pocatecni velikost tlacitka)

	CGUIStandard9 *st9;		// objekt zajistujici zobrazovani buttonu vsech velikosti a stylu
	CGUIRectHost *simpleButton;	// grafika jednoducheho cudlitka (singleFrame) +
	// zda se jedna o jednoduche cudlitko (1 ctverec) nebo plnohodnotne (9 ctvercu ~ == 0)

	bool resizable;			// zda je umozneno menit velikost cudlitka

	int button_state;		// stav cudlitka: up/markUp/down/downMark (0/1/2/3)
	bool main_state;		// hlavni stav up/down (0/1)	
	bool button_was_down;	// zda byl button stlacen a vyjeto mimo nej
	CGUIStyle *up,*down,*markUp, *markDown;	// pouzite styly 
	CGUIStaticText *label;		// popisek (nemusi byt)
	bool fullsizelabel;			// zda se cely labelString vesel do labelu
	CGUIRectHost *picture;
	char *help;		// kontextova napoveda

	CBltWindow *rootwnd;	// ukazatel do enginu (kvuli meneni grafiky popisku a obrazku)

	char* labelString;		// popisek ve stringu (kvuli meneni popisku pri zmene velikosti tlacitka)
	typeID windowID;		// ID okna, ktere je asociovano s tlacitkem
};


#endif