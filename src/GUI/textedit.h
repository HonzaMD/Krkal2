//////////////////////////////////////////////////////////////////////
//
// TextEdit.h
//
// editor textu - propojeni DXtextu s GUI
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "gui.h"
#include "widget.h"

#include "dxtext.h"
#include "dxfont.h"

class CGUIEditWindow: public CGUIStdWindow, public CGUIDataExchange
{
public:

	//vytvori editacni okno
	/*
	_x,_y - souradnice levyho horniho rohu
	_sx,_sy - delikost v pixelech
	wordwrap - urcuje jestli se maji radky zalamovat
	noscrollbars - 1=bez scrolbaru, 0= pokud wordwrap=0 - oba scrolbary vzdy viditelne, pokud wordwrap=1 - je vzdy viditelny jen vertikalni scrolbar
	*/
	CGUIEditWindow(float _x, float _y, float _sx, float _sy, int wordwrap, int noscrollbars, char *title_text, CGUIRectHost *_icon=0, UINT resizer_color=STD_RESIZER_COLOR, int textColor=0xFFFFFFFF, int bgColor=0xAA0000AA, CDXFont *font=NULL); 
	
	//vytvori viceradkovej editbox 
	CGUIEditWindow(float _x, float _y, float _sx, float _sy, int wordwrap, int noscrollbars=0, int textColor=0xFF000000, int bgColor=0xFFFFFFFF, char *fontname=NULL);
	
	//vytvori jednoradkovej editbox
	//_x,_y - souradnice levyho horniho rohu
	//_sx - sirka v pixelech
	//textcolor - barva textu
	//bgcolor - barva pozadi
	CGUIEditWindow(float _x, float _y, float _sx, int textColor=0xFF000000, int bgColor=0xFFFFFFFF, char *fontname=NULL); 

	virtual ~CGUIEditWindow();

	void SelectOnFocus(int sel=1){selectonfocus=sel;} //nastavi, jestli se ma oznacit cely text pri ziskani focusu

	int SetHighlight(int _highlight); //nastavi jestli se ma highlitovat

	void SetReadOnly(int _readonly=1){readonly=_readonly;}

	static const int numEvents;
	static const int availableEvents[]; //eventy, na ktery okno reaguje

	virtual void ParentNotification(); //notifikace, ze se mi zmenil rodic (nekdo me dal do okna)

	virtual void Resize(float _sx, float _sy); //resizuje editacni okno

	virtual int Keyboard(UINT ch, UINT state); //klavesnice
	virtual int KeyboardChar(UINT ch, UINT state); //prelozene znaky - psani
	virtual int TimerImpulse(typeID timerID, float time); //timer (pro blikani kurzoru a scrolovani mysi)
	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState); //levy mouse button 
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID); //levy mouse button mimo okno
	virtual int MouseOver(float x, float y, UINT over,UINT state); //pohyb mysy nad oknem
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID); //pohyb mysi mimo okno

	virtual int GetParametr(void *value, int type, int size, int which=0); //prenos dat z editacniho okna
	virtual int SetParametr(void *value, int type, int size, int which=0); //pranos dat do editacniho okna

	void BadInput(); //vola se, kdyz zadana hodnota v editboxu neni platna
	void SendEOk();
	
	int changed; //text se zmenil od posledniho posleni EOk

	virtual int ChangedEvent(){return 1;}
	virtual void CursorMoved(){}
	virtual void ClipboardEvent(){}

	int Undo();
	int Redo();
	int Copy();
	int Paste();
	int Cut();

	int MoveCursorToLine(int line, int column=0);
	int SelectWordRight();

protected:

	void ScrollToCursor(int center=0); //nascroluje, tak aby byl videt cursor
	void SetTextSize(); //nastavi velikost scrolbaru podle delky textu

	CFormatedText *text; //formatovany text
	CBltFormatedText *blttext; //zobrazovac formatovaneho textu
	
	int focused; //urcuje jestli ma okno fokus
	int multiline; //flag, jestli je editor viceradkovy
	int allowtab; //flag, jestli je mozne vkladat taby

	int mousesel; //nastavi se na 1, kdyz se zacne mysi oznacovat text

	int highlight; //flag, jestli se ma highligtovat
	CDXHighlighter *highlighter; //highlightovac

	CDXFont *defaultfont; //defalutni font
	
	//initace - vola se z konstruktoru
	void InitEditWindow(float _x, float _y, float _sx, float _sy, CDXFont *font, int textColor, int bgColor, int wordwrap, int noscrollbars, int multiline, int xmargin=4, int ymargin=0, char *title_text=NULL, CGUIRectHost *_icon=0, UINT resizer_color=STD_RESIZER_COLOR); 

	int mousescroll,mousescx,mousescy; //scrolovani mysi
	
	void SetMouseScrollTimer(); //nastavi timer pro scrolovani mysi
	typeID mousescrolltimerID; //ID timeru pro scrolovani mysi
	typeID cursorblinktimerID; //ID timeru pro blikani kurzoru

	int selectonfocus; //ma oznacit cely text pri ziskani focusu

	int readonly; //pokud je =1 - text nelze menit
};

#endif