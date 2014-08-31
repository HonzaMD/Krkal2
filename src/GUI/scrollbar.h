///////////////////////////////////////////////
//
// scrollbar.h
//
// Interface pro scrollbar a jeho casti scrollbarButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "element.h"
#include "button.h"

class CGUIStaticText;
class CGUIStaticPicture;
class CGUIScrollbarButton;


//////////////////////////////////////////////////////////////////////
// CGUIScrollBar
//////////////////////////////////////////////////////////////////////

// objekt pro scrollovani v okne

class CGUIScrollBar : public CGUIMultiWidget	
{
	friend class CGUIScrollBar;			// abych mohl lezt na polozky dualniho scrollbaru
	friend class CGUIScrollbarButton;	// pro pristup k polozkam active, shift
	friend class CGUIStdWindow;			// pro pristup k polozce active
	friend class CGUIWindowPanel;		// kvuli zmenseni scrollbaru pri pridavani panelu do okna

public:
	// Scrollbar je sestaven z 5 tlacitek 3 typu (0.typ - up/down, 1.typ - pageUp/pageDown, 2.typ - slider
	// tomu odpovidaji i styly tlacitek, ktere se zadavaji v konstruktoru - parametr "styleName"
	// ke kazdemu typu tlacitka jsou 2 styly - tlacitko nahore a dole (zmacknute), tedy celkem 3x2 jmen stylu
	// na tlacitkach muze byt bitmapa (sipky, ...), ta se zadava parametrem pictures
	// 3 bitmapy odpovidaji: 0-tl. up/left, 1-tl. down/right, 2-tl. slider
	// typ scrollbaru je horizontalni nebo vertikalni, podle typu se pracuje s delkou a sirkou a se smerem scrollovani oknem
	// parametr "shift" rika o kolik pixelu se okno posune pri 1 zmacknuti tlacitka up/down
	// autohide rika, zda se ma scrollbar pokud neni treba skryt ci nikoliv
	// slider_resize rika zda se ma velikost slideru urcit z pomeru velikosti viewportu a back window, nebo zda ma byt konstantni
	// wnd rika s jakym oknem se bude scrollovat, do jakoho okna se scrollbar prida
	// dual je pointer na dualni scrollbar, je kvuli tomu, aby o sobe horizontalni s vertikalnim sb. vedely

	CGUIScrollBar(enum EScrollBarTypes _scrollBarType, float _x, float _y, float length, float width, float _shift, bool _autohide, bool _slider_resize, CGUIWindow *wnd, char* styleName[3][2], CGUIRectHost* pictures[3], CGUIScrollBar* _dual);	
		// konstruktor pro plne resizovatelny scrollbar (postaveny na zaklade 5 plnohodnotnych cudlitek)
	CGUIScrollBar(enum EScrollBarTypes _scrollBarType, float _x, float _y, float length, float width, float _shift, bool _autohide, bool _slider_resize, CGUIWindow *wnd, char* styleName[4][2], CGUIRectHost* pictures[3], CGUIScrollBar* _dual, bool simple);	
		// konstruktor pro jednoduchy scrollbar (resizuje se jen slider, zbytek jsou jednoduche cudlitka)
	virtual ~CGUIScrollBar();
		
	virtual void Resize(float _sx, float _sy);

	void RebuildScrollbar();	// vola se, kdyz se zmeni parametry materskeho okna, 
								// na kterych zavisi vlastnosti scrollbaru (bw_normal_size, vp, vp_sx, ...)

protected:
	virtual void RemoveFromEngine();	// predefinovana kvuli zmene velikosti viewportu pri odstraneni scrollbaru
										// nelze udelat v destruktoru, nebot tam jiz neznam materske okno
	virtual void EventHandler(CGUIEvent *event);

	EScrollBarTypes scrollBarType;
	CGUIScrollbarButton* buttons[6];
		// 0 - Up,
		// 1 - PageUp
		// 2 - Slider,
		// 3 - PageDown
		// 4 - Down
		// 5 - Nothing (pri neaktivnim scrollbaru je uprosted misto 1,2,3 neaktivni plocha - Nothing)
	bool autohide;		// zda se scrollbar automaticky skryva pri nepotrebe (active==false)
	bool active;		// zda je scrollbar aktivni (viewport je mensi nez back window)
	bool slider_resize;	// zda se meni velikost slideru podle pomeru velikosti viewport/back wnd.
	CGUIScrollBar* dual;	// pointer na dualni scrollbar (kdyz ja jsem vertikalni, tak pointer na horizontalni)
	bool long_version;		// zda jsem v dlouhe verzi (tedy pres celou sirku okna)
							// pokud jsou oba scrollbary aktivni musi byt oba v kratke verzi
							// pravy dolni ruzek je volny
	CGUIStaticPicture* corner;	// bitmapa na vyplneni rohu pri zobrazeni obou scrollbaru soucasne
	float shift;		// o kolik se posune viewport pri stisku tlacitka Up, Down, Left, Right
};


//////////////////////////////////////////////////////////////////////
// CGUIScrollbarButton
//////////////////////////////////////////////////////////////////////

enum EScrollBarButtonTypes {
	S_Up,		// left
	S_PageUp,
	S_Slider,
	S_PageDown,
	S_Down,	// right
	S_Nothing	// nereagujici vyplnovaci button
};

class CGUIScrollbarButton : public CGUIButton
{
public:
	CGUIScrollbarButton(CGUIScrollBar* _sb, float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, enum EScrollBarTypes _scrollBarType, enum EScrollBarButtonTypes _scrollBarButtonType, CGUIStaticText *_label=0, CGUIRectHost *_picture=0);	
		// konstruktor pro plnohodnotna cudlitka
	CGUIScrollbarButton(CGUIScrollBar* _sb, float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool resizable, enum EScrollBarTypes _scrollBarType, enum EScrollBarButtonTypes _scrollBarButtonType, CGUIStaticText *_label=0, CGUIRectHost *_picture=0);	
		// konstruktor pro jednoducha cudlitka (jednoctvercova)
	~CGUIScrollbarButton();

protected:
	virtual void OnClick();

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int TimerImpulse(typeID timerID, float time);

	EScrollBarTypes scrollBarType;	// jakeho typu je scrollbar jeho je tlacitko soucasti
	EScrollBarButtonTypes scrollBarButtonType;	// jakeho typu je tlacitko (podle toho se chova)
	CGUIScrollBar* sb;	// odkaz na scrollbar jehoz je tlacitko soucasti
	float px, py;	// misto, kde se stisklo tlacitko mysi nad buttonem
	bool mouse_down;	// zda bylo stisknuto tlacitko mysi nad buttonem
};



#endif