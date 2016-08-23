///////////////////////////////////////////////
//
// widgets.h
//
// Interface pro elementy okna (widgety)
//
// CGUIStaticText
// CGUIStaticPicture
// CGUILine
// CGUIContextHelp
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef WIDGETS_H
#define WIDGETS_H

#include "element.h"

class CGUIStaticText;
class CGUIContextHelp;
class CGUIRectHost;


class CBltElem;
class CGUIFont;
class CTexture;
class CBltLine;
class CBltText;



//////////////////////////////////////////////////////////////////////
// CGUIStaticText
//////////////////////////////////////////////////////////////////////

// objekt pro textove vypisy (staticke, tj. needitovatelne uzivatelem)
// umoznuje vytvaret text libovolnym fontem, barvou, velikosti
// umi text zarovnavat doleva, doprava a nastred (vzhledem k zadane poloze a velikosti oblasti)
// umi text rozdelit na nekolik radek (definovanim delky radky, pripadne '\n' v textu)
// umi existujici text zmenit na novy
// umi vratit zobrazeny text jako string

class CGUIStaticText : public CGUIWidget	// objekt pro spravu statickeho textu
{
public:
	CGUIStaticText(float _x, float _y, char *_text, char *fontName, int size, bool pixelSize = true, int color = 0xFFFF0000);	
		// jednoradkovy text (bez specialnich nastaveni)
		// fontName je jmeno fontu jako ve windows ("Times New Roman", "Arial", "Courier New", atp.)
		// size je velikost pisma v pixelech (pixelSize==true) nebo v bodech pisma (pixelSize==false)
		// vytvari si Font dle zadanych parametru
	CGUIStaticText(float _x, float _y, float maxx, char *_text, char *fontName, int size, bool pixelSize = true, EGUIAlignment align = aLeft, int color = 0xFFFF0000);	
		// jednoradkovy text o max. sirce + zarovnani do bunky dane sirky (vlevo, vpravo, doprostred)
		// vytvari si Font dle zadanych parametru
	CGUIStaticText(float _x, float _y, float maxx, float maxy, char *_text, char *fontName, int size, bool pixelSize = true, EGUIAlignment align = aLeft, int color = 0xFFFF0000, float LinesSpaces = 0.0f);	
		// nekolikaradkovy text + zarovnani
		// deli vstupni text na radky podle sirky radky (maxx), pokud lze nerozdeluje slova
		// ve vstupnim textu muze byt uzivatelem urcene radkovani pomoci '\n'
		// omezeni na 256 radek
		// pixelSize urcuje, zda je zadana velikost v pixelech (true) nebo fontovych bodech (false)
		// LinesSpaces je mezera mezi radky (radkovani 0==jednoduche)
		// maxy - urcuje max. vysku textu v bodech 
		// vytvari si Font dle zadanych parametru
	CGUIStaticText(const char *_text, CGUIFont* _font, float _x, float _y, int color = 0xFFFF0000, float maxx = 0, float maxy = 0, EGUIAlignment align = aLeft, int maxLines = 0, float LinesSpaces = 0.0f, UINT dwDTFormat = 0);
		// text dle nastaveneho fontu, barvy, velikosti bunky a zarovnani
		// omezeni na 256 radek
		// funguje stejne jako predchozi funkce, jen se styl pisma zadava pres CGUIFont (uz ho nemusi sam vytvaret)
		// maxx - urcuje maximalni sirku textu v pixelech (zalamuje na cela slova), zarovnava do text dle align do bunky o velikosti maxx (maxx==0 => neomezena velikost radku)
		// maxy - urcuje max. vysku textu v pixelech (orezava na cele radky), (maxy==0 => vyska textu neomezena)
		// maxLines - urcuje max. pocet radek textu (0==neomezene)
		// LinesSpaces - urcuje mezeru mezi radky v pixelech
		// pouzivat nejlepe tento konstruktor, ostatni jsou Obsolete

	void ChangeText(char *_text, CGUIFont* _font=0);
		// zmeni text na novy, pouzije nastaveni dle konstruktoru
		// pokud je _font==0 pouzije puvodni font textu

	virtual ~CGUIStaticText();
	
	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	void AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos);	// prida cverec do enginu za zadany element (kvuli poradi viditelnosti)
	CBltElem* GetElemPosition(); 

	void SetColor(UINT color);
	UINT GetColor();
	char* GetText() { return originalText; };

	virtual void SetVisible(int vis);

	int numChars;	// delka textu

protected:
	void CreateText(char *_text, CGUIFont* _font, float _x, float _y, int color, float maxx, float maxy, EGUIAlignment align, int maxLines, float LinesSpaces);

	CGUIFont *guiFont;	// font jakym je pismo napsano
	CBltText *text;		// textovy element grafickeho enginu
	CBltText **texts;	// pole texttovych elementu grafickeho enginu (pouzito v pripade viceradkoveho textu)

	float maxX, maxY, linesSpaces;	// max. sirka a vyska textu v pixelech, mezera mezi radky v pixelech
	EGUIAlignment alignment;	// zarovnani textu (doleva, doprava, nastred)
	int maxNumLines;			// maximalni pocet radek textu
	
	char* originalText;			// text jako string (okopirovany ze zadani)
	int numLines;				// pocet radek textu
};


//////////////////////////////////////////////////////////////////////
// CGUIStaticPicture
//////////////////////////////////////////////////////////////////////

// objekt reprezentujici obrazek
// obrazek je obdelnik dany svou polohou a velikosti
// muze mit texturu (i pruhlednou)
// muze se zneviditelnit

class CGUIStaticPicture : public CGUIWidget, public CGUIEventSupport
{
public:
	CGUIStaticPicture(float _x, float _y, const char *texName, float _sx = -1, float _sy = -1, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik z textury ulozene v souboru "texName"
		// x,y - umisteni obdelniku
		// sx,sy - velikost na kterou se textura upravi (pokud je -1, velikost se urci dle velikosti textury)
		// color - vynasobeni textury barvou
	CGUIStaticPicture(float _x, float _y, CTexture *tex, float _sx = -1, float _sy = -1, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik z textury "tex" (muze byt i 0 => vytvori obdelnik bez textury, jen s barvou)
		// x,y - umisteni obdelniku
		// sx,sy - velikost na kterou se textura upravi (pokud je -1, velikost se urci dle velikosti textury)
		// color - vynasobeni textury barvou
	CGUIStaticPicture(float _x, float _y, float _sx, float _sy, UINT color = 0xFFFFFFFF);
		// vytvori obdelnik bez textury, jen obarveny
		// x,y - umisteni obdelniku
		// sx,sy - velikost obdelniku
		// color - barva obdelniku
	virtual ~CGUIStaticPicture();

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();
	CBltElem* GetElemPosition();	// vrati element enginu, ktery je pred ctvercem
	

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);
	virtual void SetVisible(int vis);

	void RebuildStaticPicture(CTexture *tex, float _x=-1, float _y=-1, float _sx=-1, float _sy=-1);	
		// prebuduje objekt podle novych parametru
		// parametry maji stejny vyznam jako v konstruktoru
	void SetTexture(CTexture *tex);
		// nastavi texturu obdelniku (muze byt i 0 => zadnou texturu mit nebude)
	void SetColor(UINT color);
	void Rotate(float alfa);	// !@#$ zatim nefunkcni => hrozna bitkarina 

	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);	// kvuli posilani udalosti o stisku tlacitka nad obrazkem
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);

	static const int numEvents;
	static const int availableEvents[];

protected:
	CBltRect *rect;		// element grafickeho enginu reprezentujici obdelnik
};


//////////////////////////////////////////////////////////////////////
// CGUILine
//////////////////////////////////////////////////////////////////////

// objekt reprezentujici caru

class CGUILine : public CGUIWidget	
{
public:
	CGUILine(float x1, float y1, float x2, float y2, UINT color = 0xFFFFFFFF); 
		// udela caru z bodu x1,y1 do bodu x2,y2 (bod x2,y2 uz neni vykreslen !!!!)
	virtual ~CGUILine();

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);
	virtual void SetVisible(int vis);

	void SetColor(UINT color);
	void SetPoints(float x1, float y1, float x2, float y2);
		// prenastavi caru na nove koncove body

protected:
	CBltLine *line;	// cara grafickeho enginu
	bool up_down;	// zda je cara odshora dolu (kvuli prepoctu na body z polohy a velikosti)
	bool left_right;	// zda je cara odleva doprava (kvuli prepoctu na body z polohy a velikosti)
};


//////////////////////////////////////////////////////////////////////
// CGUIContextHelp
//////////////////////////////////////////////////////////////////////

/*
	zobrazuje napovedu nad elementem, ktery si o ni zazada
	implementovano jako singleton, tj. ex. max. jedna instance CGUIContextHelpu
	zobrazuje text v ramecku s pozadim
*/

class CGUIContextHelp : public CGUIStaticText
{
public:
	static void On(CGUIElement* _activator, char* text);		
		// vola se pro zacnuti cekani na zobrazeni helpu (najezd na tlaciko)
		// musi si zajistit kazdy element sam, pokud chce zobrazovat napovedu
		// "activator" je element, ktery chce zobrazit napovedu
		// "text" je text napovedy, kterou chce zobrazit
		// element se nemusi starat zda napoveda existuje, ci nikoliv staci kdyz bude volat v pravych okamzicich On/Off
	static void Off(CGUIElement* _activator);		
		// vola se pro ukonceni cekani na zobrazeni helpu (odejeti z tlacitka)
		// musi zavolat kazdy element sam pro zruseni napovedy, jinak se zavola automaticky az po urcite dobe

	static void OnSpecific(CGUIElement* _activator, char* text, char *fontName="ARIAL", int fontSize=8, float maxx=0, float maxy=0, int textColor = 0xFF000000, int backColor = 0xFFEFE5B0, int borderColor = 0xFF000000);
		// jako On, ale muze si specifikovat vlastnosti contextHelpu

protected:
	CGUIContextHelp(float maxx, float maxy, char *_text, char *fontName, int size, bool pixelSize = true, int textColor = 0xFF000000, int backColor = 0xFFEFE5B0, int borderColor = 0xFF000000);
	virtual ~CGUIContextHelp();	

	virtual void Resize(float _sx, float _sy);
	virtual void Move(float _x, float _y);

	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();
	virtual void BringToTop();

	virtual void SetVisible(int vis);

	virtual int TimerImpulse(typeID timerID, float time);	// zarizuje zobrazeni a skryti helpu po casovych intervalech

	void Refresh(char* text);	// zaridi predelani napovedy na novy text (zmena velikosti ohraniceni, textu atp.)

	CGUIRectHost* background;	// pozadi pod textem
	CBltLine* border[4];		// ramecek kolem textu

	static CGUIContextHelp* contextHelp;	//pointer na vytvorenou instanci contextHelpu

	bool req;					// zda byla poslana zadost timeru na zobrazeni napovedy
	bool active;				// zda je napoveda prave aktivni, tj. zobrazena
	static CGUIElement* activator;		// element, jehoz contexthelp je prave vytvoren
};


//////////////////////////////////////////////////////////////////////
// CGUITextButton
//////////////////////////////////////////////////////////////////////

class CGUITextButton : public CGUIStaticText, public CGUIEventSupport
{
public:
	CGUITextButton(char *_text, CGUIFont* _font, float _x, float _y, int _color = 0xFFFF0000, float maxx=0, float maxy=0, EGUIAlignment align = aLeft, int maxLines = 0, float LinesSpaces = 0.0f);	

	void SetPosSize(float _x, float _y, float _sx, float _sy);	// sefarska funkce na nastaveni pozice a velikosti tlacitka natvrdo
	virtual void Move(float _x, float _y);

	int markColor;
	int color;

protected:
	virtual void OnClick() {};
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);	// zarizuje zmenu stylu + detekuje click
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);

	float psx,psy;	// posun textu vuci pocatku tlacitka

	static const int numEvents;
	static const int availableEvents[];
};

#endif