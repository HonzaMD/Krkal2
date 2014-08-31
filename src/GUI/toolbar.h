///////////////////////////////////////////////
//
// toolbar.h
//
// Interface pro ToolBar a jeho casti ToolBarButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "element.h"
#include "window.h"

class CGUIStaticText;
class CGUIStandard9;
class CGUIRectHost;



//////////////////////////////////////////////////////////////////////
// CGUIToolBar
//////////////////////////////////////////////////////////////////////

// toolbar se sklada z okna a tlacitek v nem
// krome obycejneho okna s tlacitky umoznuje spousty funkci navic
// - vytvorit si toolbar presne podle velikosti tlacitek (napr. 3x5 tlacitek)
// - zajistuje, ze max. jedno tlacitko je zmacknute (nastroj je vybran)
// - umoznuje sofostikovanejsi pridavani a odebirani nastroju (tlacitek)
// ovladani toolbaru:
// - klient si muze pretizit fci ToolBarHandler(), ktera se vola pri kazde zmene toolbaru
//	 v ni se dozvi, jaky nastroj byl vybran (podle ID, i indexu)
// - nebo si muze objednat udalost EToolBar, ktera ma stejne parametry jako ToolBarHandler

class CGUIToolBar : public CGUIStdWindow
{
	friend class CGUIToolBarButton;
public:
	CGUIToolBar(float _x, float _y, float _sx, float _sy, char *title_text, bool resizers=true);
		// vyvori toolbar na pozici x,y o velikosti sx,sy s titulkem? a resizery?
	virtual ~CGUIToolBar();
	void SetProperties(int _num_tools_horizontal, int _num_tools_vertical, int _num_tools_horizontal_visible, int _num_tools_vertical_visible, bool _one_tool_always_selected, bool _deselect_self,float _tool_sx, float _tool_sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, CGUIStyle *_inactivate, bool _simple_button);
		// nastavi vlastnosti toolbaru:
		//	pocet nastroju na radce, pocet nastroju ve sloupci, zda je vzdy jeden nastroj oznacen (zamacknut),
		//  kolik radku a sloupcu nastroju je videt najednou
		//	velikost jednoho nastroje, styly pro tlacitko nastroje, zda se jedna o jednoduchy cudlik (jednoctvercovy)
		//  vymaze vsechny jiz umistene nastroje toolbaru
	typeID SetTool(CGUIRectHost* pictureUp, CGUIRectHost* pictureDown, char* deskription, int pos_h, int pos_v);
		// nastavi nastroj na pozici [pos_v][pos_h] - obrazek + popisek
		// vrati ID nastroje (0 - neslo vlozit)
		// pokud se na dane pozici uz nachazi jiny nastroj prepise ho novym
		// pokud je pictureDown==0 pouzije se pictureUp
	typeID SetNextTool(CGUIRectHost* picture, CGUIRectHost* pictureDown, char* deskription, int &pos_h, int &pos_v);
		// nastavi nastroj (obrazek+popisek) na prvni volnou pozici a vrati jeho polohu pos_h,pos_v 
		// vrati ID nastroje (0 - neslo vlozit)
		// pokud je pictureDown==0 pouzije se pictureUp
	typeID SetFreeTool(CGUIRectHost* pictureUp, CGUIRectHost* pictureDown, char* deskription, float tx, float ty);
		// nastavi nastroj na libovolnou pozici tx,ty - obrazek + popisek
		// vrati ID nastroje (0 - neslo vlozit)
		// pokud je pictureDown==0 pouzije se pictureUp
	void SetFreeSize(float _sx, float _sy, float vis_sx, float vis_sy);
		// nastaveni velikosi toolbaru volne (nemusi byt zarovnano na nasobky velikosti nastroje)

	CGUIToolBarButton* GetTool(int pos_h, int pos_v);
		// vrati nastroj na zadane pozici

	virtual void ToolBarHandler(bool selected, typeID toolID, int pos_h, int pos_v){};
		// obsluzna fce pro toolbar
		// vola se pri pouziti nastroje na pozici [pos_v][pos_h] s toolID
		// selected urcuje zda byl nastroj vybran (true) nebo zrusen (false) - pri nastaveni one_tool_always_selected==true
		//		pokud one_tool_always_selected==false je parametr selected nastaven vzdy na false (zadny tool neni vybran, jen zmacknut)
		// pretizenim si uzivatel nadefinuje vlastni obsluhu

protected:
	virtual void EventHandler(CGUIEvent *event);
		// toolbar si od kazdeho sveho nastroje objedna EClicked
		// potom tyto udalosti prevadi na volani ToolBarHandler a posilani udalosti EToolBar

	static const int numEvents;
	static const int availableEvents[];

	int num_tools_horizontal, num_tools_vertical;
	typeID **tools;	// pole nastroju (tlacitek)
	bool one_tool_always_selected;
	float tool_sx, tool_sy;	// velikost nastroje (vsechny musi byt stejne velke)
	CGUIStyle *up, *markUp, *down, *markDown, *inactivate;
	bool simple_button;
	bool deselect_self;		// cudlik se muze sam vymacknout (jinak jen prepnutim na jiny cudlik)
	typeID selectedTool;
};

//////////////////////////////////////////////////////////////////////
// CGUIToolBarButton
//////////////////////////////////////////////////////////////////////

// reprezentuje nastroj v toolbaru
// od klasickeho tlacitka se lisi pedevsim tim, ze muze zustat zamacknuty
// obsahuje 4 styly (up, upMark, down, downMark) - navic tedy downMark, ktery se pouzije kdyz je nastroj vybran (tlactiko je zmacknute) a najede se nad nej mysi
// obrazek na tlacitku muze byt ruzny pro stav up a down (pokud je pictureDown==0 pouzije se pictureUp)
// ma 2 hlavni stavy "main_state" (0 up / 1 down) down == nastroj vybran
// nastroj muze byt neaktivni (nemackatelny)

class CGUIToolBarButton : public CGUIWidget, public CGUIEventSupport
{
	friend class CGUIToolBar;
public:
	CGUIToolBarButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, CGUIRectHost *_pictureUp, CGUIRectHost *_pictureDown=0);
	CGUIToolBarButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, CGUIRectHost *_pictureUp, CGUIRectHost *_pictureDown=0);
	virtual ~CGUIToolBarButton();

	void SetProperties(bool _raisable, bool _down_stable);
		// _raisable urcuje zda lze tlacitko vymacknout
		//	tj. pokud je nastroj vybran, zda lze zrusit vyber opetovnym stiskem tlacitka, 
		//	nebo zda se musi vybrat nejaky jiny nastroj
		//_down_stable urcuje zda tlacitko po zmacknuti zustane dole (zamacknute)
		//	nebo se hned vrati nahoru
	void SetPosition(int _pos_h, int _pos_v) {pos_h = _pos_h; pos_v = _pos_v;};
		// nastavi pozici v ramci matice nastroju v toolbaru

	void SetLabel(CGUIStaticText *_label);
	void SetPictureUp(CGUIRectHost *_pictureUp);
	void SetPictureDown(CGUIRectHost *_pictureDown);
	void SetPictureInactive(CGUIRectHost *_pictureInactive);
		
	void ChangeState(bool state);	// meni hlavni stav nastroje 
									// hlavni stav up/down (0/1) down == selected
		
	bool GetState() { return main_state;};	// true==down, false==up

	void ChangeContextHelp(char* text);			// zmeni text contexthelpu cudlitka
	void SetInactiveStyle(CGUIStyle *_inactive);

	int Activate();		// aktivuje cudlitko z pasivniho stavu na plne funkcni stav
	int Deactivate();	// deaktivuje cudlitko do pasivniho (nefunkcniho) stavu, neklikatelneho
	bool IsActive() { return active; };

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	
	virtual void SetVisible(int vis);

protected:
	void ReStylize(CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_markUp, CGUIStyle *_markDown);	// zmeni styly buttonu (kompletni zmena sady grafiky cudlitka)
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

	bool active;			// zda je tlacitko aktivni, tj. funkcni (pasivni=sede a nemackatelne)
	int button_state;		// stav cudlitka: up/markUp/down/downMark (0/1/2/3)
	bool main_state;		// hlavni stav up/down (0/1)	
	bool button_was_down;	// zda byl button stlacen a vyjeto mimo nej
	CGUIStyle *up,*down,*markUp, *markDown, *inactive;	// pouzite styly 
	CGUIStaticText *label;		// popisek (nemusi byt)
//	CGUIStaticPicture *picture;	// ikona na cudlitku (nemusi byt) ?? misto StaticPicture jen RectHost ??
	CGUIRectHost *pictureUp;
	CGUIRectHost *pictureDown;
	CGUIRectHost *pictureInactive;
	char *help;		// kontextova napoveda
	CBltWindow *rootwnd;	// ukazatel do enginu (kvuli meneni grafiky popisku a obrazku)

	bool raisable;		// zda lze zamacknute tlacitko vymacknout kliknutim
						// zamacknute tlacitko symbolizuje oznaceny nastroj
	bool down_stable;	// zda pri zmacknuti zustane dole, nebo se rovnou vraci nahoru jako normalni cudlik
	int pos_h, pos_v;	// pozice tlacitka v ramci toolbaru (matice nastroju)
};


#endif