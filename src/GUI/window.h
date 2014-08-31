///////////////////////////////////////////////
//
// window.h
//
// Interface pro std. okno a jeho std. elementy 
//	- Titulek, Resizery
//
//	ostatni std. elementy maji vlastni soubory - Scrollbar, WinPanel, ...
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef WINDOW_H
#define WINDOW_H

#include "gui.h"

class CGUITitle;
class CGUIRectHost;
class CGUIStaticText;
class CGUIButton;

//////////////////////////////////////////////////////////////////////
// CGUIStdWindow
//////////////////////////////////////////////////////////////////////
// std. okno pro uzivatelske dialogy, atp.
//////////////////////////////////////////////////////////////////////

class CGUIStdWindow : public CGUIWindow
{
public:
	CGUIStdWindow(float _x, float _y, float _sx, float _sy, char *title_text, CGUIRectHost *_icon=0, bool resizable=true, UINT resizer_color=STD_RESIZER_COLOR, int withBackground=1, int bgColor=0xAA0000AA);
		// vytvori standardni okno na pozici x,y o veliosti sx, sy
		// s titulkem - title_text
		// s ikonou - icon
		// umoznujici menit svou velikost - resizable (true) / s pevnou velikosti (false)
		// s pozadim (withBackground) barvy (bgColor)
		// se scrollbarama (zobrazuji se jen pokud jsou potreba - okno je male)
		// s okrajema (umi menit velikost okna / neumi - dle resizable)
	CGUIStdWindow(float _x, float _y, float _sx, float _sy, char *title_text, CBltRect *_background, CGUIRectHost *_icon=0, bool resizable=true, UINT resizer_color=STD_RESIZER_COLOR);
		// vytvori standardni okno s titulkem, ikonou, pozadim a scrollbarama
		// s okrajema (umi menit velikost okna / neumi - dle resizable)
	CGUIStdWindow(float _x, float _y, float _sx, float _sy, int bgColor);
		// vytvori "orezane" okno
		// okno jen s pozadim, bez titulku, scrollbaru atp.
		// veskere dalsi prvky lze pridat nasledujicima fcema


	// fce pro pridavani std. prvku do prazdneho okna:
	// lze pouzit jen hned po vytvoreni okna (nejlepe v konstruktoru)
	// az pote lze definovat vlastni nastaveni BackWindow atp.
	// behem zivota okna jiz nelze menit
	// fce jsou v poradi, v jakem se maji volat

	void AddResizers(UINT color=STD_RESIZER_COLOR, CGUIStyle *style=styleSet->Get("Resizers"));		
		// prida vsechny resizery do okna dle stylu
		// resizer je okraj, ktery umoznuje menit velikost okna
	void AddBorders(UINT color=STD_RESIZER_COLOR);
		// prida okraje okna

	void AddTitle(char *title, CGUIRectHost *_icon=0, float height=STD_TITLE_HEIGHT, bool _closeButton=true, bool _minimizeButton=true, bool _maximizeButton=true, CTexture* texture = styleSet->Get("TitleBar")->GetTexture(0), int labelColor = STD_TITLE_LABEL_COLOR, int bgActiveColor = STD_TITLE_ACTIVE_COLOR, int bgDeactiveColor = STD_TITLE_DEACTIVE_COLOR, int text_height=STD_FONT_SIZE);	
		// prida do okna titulek, dle libosti
		// na titulku muze byt:
		// * text - title
		// * ikona - _icon
		// * tlacitka - _closeButton, _minimizeButton, _maximizeButton (tyto tlacitka ovlivnuji okno ve kterem je titulek umisten)
		// height - urcuje vysku titulku (sirka je urcena sirkou okna - titulek je pres celou sirku okna nahore)
		// labelColor - urcuje barvu textu titulku
		// text_height - urcuje velikost fontu u textu titulku
		// texture - urcuje texturu, ktera je pouzita jako pozadi titulku
		// bgActiveColor / bgDeactiveColor - barva, kterou se prebarvi textura v pripade, ze je okno aktivni / neaktivni (aktivni==fokusovane)

	
	// nasledujici fce lze volat i behem zivota okna a v libovolnem poradi:

	CGUIWindowPanel* AddWindowPanel(CGUIStyle *style = styleSet->Get("ObjPropertyPanel"), float iconStartX = 10, float iconEndX = 0);
		// prida panel do okna
		// v pripade desktopu prida panel do spodku okna, u normalniho okna prida panel nahoru tesne pod titulek
		// pridava do "front" okna
		// iconStartX - urcuje od jake pozice se na panelu budou moci umistovat ikony
		// iconEndX - urcuje maximalni pozici pro ikony (0==neomezeno)
		// krome pridani panelu zmensi BWnormal size o svou velikost !!!
		// vraci pridany panel pro dalsi nastaveni atp.

	void AddScrollbarsSimple(float width=STD_SCROLLBAR_WIDTH,float shift=STD_SCROLLBAR_SHIFT, bool autohide=STD_SCROLLBAR_AUTOHIDE, bool slider_resize=STD_SCROLLBAR_SLIDER_RESIZE,char* styleNameVertical[4][2]=0, CGUIRectHost* picturesVertical[3]=0,char* styleNameHorizontal[4][2]=0, CGUIRectHost* picturesHorizontal[3]=0);
		// prida scrollbary (vertikalni i horizontalni)
		// width - sirka scrollbaru (vyska je dana vyskou okna), u horizontalniho je to opacne
		// shift - je o kolik pixelu se okno posune po jednom stisku scrollbaru
		// autohide - urcuje zda je scrollbar videt stale, nebo jen kdyz je nutny
		// slider_resize - urcuje zda se prostredni prvek scrollbaru (slider) roztahuje / zmensuje dle pomeru velikosti okna a jeho viditelne oblasti
		// styleNameVertical, picturesVertical - urcuje pole stylu pro vytvoreni vzhledu scrolbaru (0 == pouzije se std. styl pro scrollbary)

protected:
	void SetViewPortBackWnd(float _x, float _y, float _sx, float _sy);
		// nastavi parametry back wnd. a viewportu podle vstupu + podle parametru upravi backwnd (CBltWnd)
		// zadava se souradnice leveho horniho rohu back window a velikost back window (v souradnicich okna - vnejsku)

	CGUIScrollBar* AddScrollbar(enum EScrollBarTypes sb_type, float sb_x, float sb_y, float width, float length, float shift, bool autohide, bool slider_resize, CGUIScrollBar* dual, char* styleName[3][2], CGUIRectHost* pictures[3]);
		// prida scrollbar urceneho typu (vertical/horizontal), dual je pointer na druhy typ (0 - neni znam), vraci pointer na vytvoreny scrollbar (pro urceni dualu dalsiho vytvareneho scrollbaru)
		// vytvori scrollbar zalozeny na 5 plnohodnotnych tlacitkach (plne resizovatelnych)
	CGUIScrollBar* AddSimpleScrollbar(enum EScrollBarTypes sb_type, float sb_x, float sb_y, float width, float length, float shift, bool autohide, bool slider_resize, CGUIScrollBar* dual, char* styleName[4][2], CGUIRectHost* pictures[3]);
		// vytvori scrollbar jako fce vyse, ale bude zalozeny na jednoduchych tlacitkach (tj. postavenych na jedne texture, nikoliv 9)
};


//////////////////////////////////////////////////////////////////////
// CGUIResizer
//////////////////////////////////////////////////////////////////////

// objekt slouzi jako ohraniceni okna + umoznuje jeho resizovani
// ohraniceni celeho okna se sklada z 8 resizeru (jejich typy jsou uvedeny dale)
// kazdy typ resizeru resizuje okno v jinem smeru, ma jinou grafiku

//	typy resizeru
enum EResizerTypes {
	UpperLeft,
	Up,
	UpperRight,
	Left,
	Right,
	BottomLeft,
	Bottom,
	BottomRight
};

class CGUIResizer : public CGUIWidget	// objekt pro resizovani okna
{
public:
	CGUIResizer(enum EResizerTypes res_type, CGUIWindow *wnd, CGUIStyle *style, UINT color);	
		// typ resizeru (roh, vrsek, strana, ...), okno do ktereho se ma umistit, graficky styl, ktery bude mit
		// styl musi mit 3 parametry (sirku horizontalniho resizeru, sirku vert. res., velikost rohu)
		// ve stylu je grafika pro vsechny typy resizeru (8 textur + 3 parametry)
	virtual ~CGUIResizer();
		
	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	virtual void SetVisible(int vis);

	virtual int IsInPrecise(float _x, float _y);		// nastaveni presne velikosti pro rohove resizery

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int TimerImpulse(typeID timerID, float time);	// kvuli scrollovani okna pri resizovani vnitrniho okna

protected:
	EResizerTypes resizerType;
	CGUIRectHost *rect;	// grafika resizeru
	bool mouse_button_down;	// zda byl na nem zmacknut kurzor
	bool cursor_resizer;	// zda se zmenil kurzor
	int cursor_type;	// cislo typu kurzoru, ktery byl nastaven pred najetim na resizer (pro navraceni stejneho typu)
	float px,py;	// poloha kurzoru pri zmacknuti mysi nad resizerem
	float width_h, width_v, corner;	// miry resizeru
	bool timerReq;	// zda byl poslan pozadavek na timer

	void ChangeWindowSize(float nx, float ny);	// interni fce, zaridi zmenu velikosti materskeho okna
					// zjisti jak se ma materske okno zmenit (z px,py a nx,ny a typu resizeru)
};


//////////////////////////////////////////////////////////////////////
// CGUITitle
//////////////////////////////////////////////////////////////////////

// titulek v okne
// obsahuje popisek, ikonu, tlacitka na minimalizaci, maximalizaci, zavreni okna
// chycenim titulku mysi lze okno presouvat

class CGUITitle : public CGUIWidget, public CGUIEventSupport	// objekt pro presouvani okna + jmeno a ikona
{
public:
	CGUITitle(char *title, CGUIRectHost *_icon, CGUIWindow *wnd, float height, bool _closeButton=true, bool _minimizeButton=true, bool _maximizeButton=true, CTexture* texture = styleSet->Get("TitleBar")->GetTexture(0), int labelColor = STD_TITLE_LABEL_COLOR, int bgActiveColor = STD_TITLE_ACTIVE_COLOR, int bgDeactiveColor = STD_TITLE_DEACTIVE_COLOR, int text_height=STD_FONT_SIZE);	
		// text titulku, ikona, okno do ktereho se ma umistit, vyska titulkoveho pasu, 
		// zda se ma na titulek umistit tlacitko na zavreni okna, minimalizaci, maximalizaci
		// textura pro podklad titulku (vertikalni pruh o vysce titulku roztazeny pres celou sirku okna)
		// barva textu
		// barva aktivniho / neaktivniho titulku (aktivni je kdyz je okno focusovane)
		// velikost fontu textu
		// umisti se automaticky do okna nahoru (front)

	virtual ~CGUITitle();

	void SetText(char *text, int labelColor = STD_TITLE_LABEL_COLOR, int text_height=STD_FONT_SIZE);
		// nastaveni textu v titulku + vlastnosti pisma
	char* GetText() { return titleText; };
		// vrati text titulku
	void SetButtons(bool _closeButton=true, bool _minimizeButton=true, bool _maximizeButton=true);
		// nastaveni jake tlacitka na titulku budou
	void SetMovable(bool _movable);
		// nastavi, zda titulek muze hybat oknem nebo ne (tahem mysi)
	bool GetMovable() { return movable; };	
		// vrati zda titulek muze hybat oknem (tahem mysi)

	void SetIcon(CGUIRectHost *_icon);
		// nastavi ikonu
	CGUIRectHost* GetIcon() { return icon; };
		// vrati aktualni ikonu titulku
	void SetActiveInactiveColor(int bgActiveColor = STD_TITLE_ACTIVE_COLOR, int bgDeactiveColor = STD_TITLE_DEACTIVE_COLOR);
		// nastaveni barvy aktivniho a neaktivniho titulku
		// aktivni je kdyz je okno focusovane
		// barvou se nasobi podkladova textura titulku (nemusi existovat zadna textura)

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	virtual void SetVisible(int vis);

	virtual void EventHandler(CGUIEvent *event);
		// zpracovava prichozi udalosti
		// kvuli zmene barvy pri focusaci / defocusaci okna
		// prevedeni stisku tlacitek na titulku do operaci na okne - zavreni, minimalizace, ...

	void WindowIsActive();	// zaridi zmenu titulku pri focusaci okna (zmena barvy)
	void WindowIsDeactive();	// zaridi zmenu titulku pri defocusaci okna (zmena barvy)

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int TimerImpulse(typeID timerID, float time);

	static const int numEvents;
	static const int availableEvents[];

protected:
	char* titleText;
	CGUIRectHost *icon;

	CGUIRectHost *rect;	// pozadi titulku
	CGUIStaticText *label;
	CGUIButton *closeButton, *minimizeButton, *maximizeButton;
	bool maxim_button;	// zda je maximizeButton maximalizacni/demaximalizacni

	bool mouse_button_down;	// zda se na titulku zmacknul button mysi
	float px,py;	// kde se zmacknul button mysi
	int active_color, deactive_color;	// jaka je nastavena barva titulku pro aktivni a neaktivni stav
	bool timerReq;	// zda byl poslan pozadavek na timer
	bool movable;	// zda titulek hybe oknem nebo ne
};


#endif