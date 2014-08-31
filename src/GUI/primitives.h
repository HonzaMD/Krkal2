///////////////////////////////////////////////
//
// primitives.h
//
// Interface pro zakladni stavebni prvky ostatnich elementu GUI - buttony, ...
// RectHost (spravce textury), Standard9 (spravce 9 textur, napr. buttonu)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "element.h"

class CGUIStyle;

class CBltElem;
class CBltWindow;
class CTexture;
class CBltLine;

//////////////////////////////////////////////////////////////////////
// CGUIRectHost
//////////////////////////////////////////////////////////////////////

class CGUIRectHost : public CGUIElement	// objekt pro spravu obdelniku vykreslovaciho enginu
											// spravuje velikost, polohu, barvu, texturu
											// umoznuje zmenu svych parametru
											// zmena zakladni tridy na element (z Primitiva) z duvodu vytvareni seznamu objektu v trashi
{
public:
	CGUIRectHost(float _x, float _y, char *texName, float _sx = -1, float _sy = -1, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik z textury ulozene v souboru "texName"
		// x,y - umisteni obdelniku
		// sx,sy - velikost na kterou se textura upravi (pokud je -1, velikost se urci dle velikosti textury)
		// color - vynasobeni textury barvou
	CGUIRectHost(float _x, float _y, CTexture *tex, float _sx = -1, float _sy = -1, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik z textury "tex" (muze byt i 0 => vytvori obdelnik bez textury, jen s barvou)
		// x,y - umisteni obdelniku
		// sx,sy - velikost na kterou se textura upravi (pokud je -1, velikost se urci dle velikosti textury)
		// color - vynasobeni textury barvou
	CGUIRectHost(float _x, float _y, float _sx, float _sy, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik bez textury, jen obarveny
		// x,y - umisteni obdelniku
		// sx,sy - velikost obdelniku
		// color - barva obdelniku
	CGUIRectHost(CGUIRectHost &rectHost);
		// vytvori kopii obdelniku (hluboka kopie)
		// kopie je zcela nezavisla na originalu
		// jeden obdlenik lze pouzit jen jednou (na jednom miste), pokud chceme pouzit ten samy znovu (jinde),
		// musime vytvorit kopii a tu pote pouzit
		
	virtual ~CGUIRectHost();

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	void AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos);	// prida cverec do enginu za zadany element (kvuli poradi viditelnosti)
	CBltElem* GetElemPosition();	// vrati element enginu, ktery je pred ctvercem

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);
	virtual void SetVisible(int vis);

	void RebuildRect(CTexture *tex, float _x, float _y, float _sx=-1, float _sy=-1);	
		// prebuduje objekt podle novych parametru
		// parametry maji stejny vyznam jako v konstruktoru
	void SetTexture(CTexture *tex);
		// nastavi texturu obdelniku (muze byt i 0 => zadnou texturu mit nebude)
	CTexture* GetTexture();
	void SetColor(UINT color);
	UINT GetColor();
	void Rotate(float alfa);	// !@#$ zatim nefunkcni => hrozna bitkarina 

protected:
	CBltRect *rect;		// element grafickeho enginu reprezentujici obdelnik
};


//////////////////////////////////////////////////////////////////////
// CGUIStandard9
//////////////////////////////////////////////////////////////////////

// objekt podporujici spravu cudlitka 
// umoznuje vytvorit na zaklade 9 textur volne roztazitelne tlacitko
// textury jsou usporadany do ctverce 3x3
// pevne textury jsou 4 rohove, ostatni se roztahuji vzdy v jednom smeru

class CGUIStandard9 : public CGUIPrimitive	
{
public:
	CGUIStandard9(float _x, float _y, CGUIStyle *style);
		// vytvori grafiku tlacitka na zaklade 9 textur ze stylu
		// usporada je a nastavi velikost podle parametru stylu (popis stylu viz. button)
	~CGUIStandard9();

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	
	virtual void SetVisible(int vis);

	void ReTexturize(CGUIStyle *style);
		// zmeni textuty vsech 9 ctvercu podle noveho stylu
	void SetStyle(CGUIStyle *style, float dx=0, float dy=0);	// predela vsechny recty na novy styl 
						// + zmeni velikost o dx,dy (rozdil ve velikosti mezi soucasnym a novym stylem)
protected:
	CGUIRectHost *rects[9];
	float min_x, min_y;	// minimalni velikost skupiny, urcena z minimalnich velikosti jednotlivych ctvercu
};


//////////////////////////////////////////////////////////////////////
// CGUIMarker
//////////////////////////////////////////////////////////////////////

// interface pro oznacovani elementu
// podle tohoto interfacu GUI automaticky vola oznaceni na elementu, ktery dostal focus
// zda si element vytvori svuj oznacovac nebo pouzije standardni (CGUIStdMarker) je na nem, 
// stejne tak zda si oznacovac vytvori pri vzniku elementu nebo az pri prvnim pouziti

class CGUIMarker
{
public:
	CGUIMarker() { visible=false; };
	virtual ~CGUIMarker() {};

	virtual void Mark() {};	// oznac element
	virtual void UnMark() {};	// odoznac element

	virtual void Rebuild(CGUIElement* elm) {};	// predela oznacovac dle novych vlastnosti elementu

	bool GetVisible() { return visible; };

protected:
	bool visible;
};


//////////////////////////////////////////////////////////////////////
// CGUIStdMarker
//////////////////////////////////////////////////////////////////////

// pokud element chce byt oznacovan pri focusu - nastavi si SetMark(true)
// pokud si neimplementuje vlastni zpusob oznacovani, pouzije se tento standardni
// cerverny ctverec kolem elementu

class CGUIStdMarker : public CGUIMarker, public CGUIEventProgram
{
public:
	CGUIStdMarker(CBltWindow *rootwnd, CGUIElement* elm);	
		// vytvori std. oznacovac pro element a umisti ho do okna enginu
		// std. oznacovac se sklada z cerveneho obdelniku kolem elementu
	virtual ~CGUIStdMarker();

	void OrderEvents(CGUIEventSupport* es);	// objedna u "es" udalosti o zmene pozice a velikosti
											// pomoci nich pak aktualizuje oznaceni elementu
	
	virtual void Rebuild(CGUIElement* elm);	// predela oznacovac dle novych vlastnosti elementu

	void Mark();
	void UnMark();

	bool GetVisible() { return visible; };

protected:
	virtual void EventHandler(CGUIEvent *event);

	CBltLine* lines[4];	// ohranicovaci linky
};


#endif