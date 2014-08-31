///////////////////////////////////////////////
//
// desktop.h
//
// Interface pro desktop editoru
// Interface pro Announcer - oznamovac udalosti pro uzivatele
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef ED_DESKTOP_H
#define ED_DESKTOP_H

#include "gui.h"
#include "element.h"
#include "widget.h"

class CGUIMenu;

class CEDDesktopIconHandler;
class CEDEditorMenuHandler;
class CEDObjectBrowser;
class CEDObjectTree;
class CEDObjectProperty;

typedef DWORD OPointer;  // typ Pointer na Objekt

//////////////////////////////////////////////////////////////////////
// CEDEditorDesktop
//////////////////////////////////////////////////////////////////////

// hlavni okno editoru
// obsahuje panel s hlavnimi ovladacimi prvky editoru
// START menu pro prepinami mezi editorem, hrou atp.
// EDITOR menu umoznujici ukladat level, nahravat atp.
// zarizuje ovladani editoru klavesnici - nejobecnejsi prikazy, tykajici vsech ostatnich oken (prepinani editacnich modu, atp.)

class CEDEditorDesktop : public CGUIDesktop
{
	friend class CEDDesktopIconHandler;
public:
	CEDEditorDesktop();
	~CEDEditorDesktop();

	void InitDefaultIcons();	// vlozi vsechny ikony na panel
	CEDDesktopIconHandler* iconDefaultHandler;	// zakladni ovladac pro udalosti od icon
	CEDEditorMenuHandler* editorMenuHandler;	// ovladac pro udalosti od menu Editoru + jeho soucasti

	void AddStartMenu();
	void AddEditorMenu();

	void CloseAllObjPropertyWins();	
		// zavre vsechny ObjProperty okna v desktopu
		// prochazi vsechna okna umistena v desktopu a ty, ktera jsou typu CEDObjProperty zavre
	CEDObjectProperty* FindObjPropertyWin(OPointer obj);	
		// najde CEDObjectProperty objektu "obj" v desktopu 
		// pokud takove neexistuje vraci 0
	CEDObjectProperty* FindAnyObjPropertyWin(OPointer obj);	
		// pokud ex. v desktopu nejake property okno (jine nez obj) vrati ho (prvni nalezene), jinak vrati 0

	CGUIStaticPicture* gameModIndicator;

protected:
	virtual int Keyboard(UINT character, UINT state);	// zajistuje prepinani editacnich modu editoru atp.
	virtual int KeyboardUp(UINT character, UINT state);	// zajistuje prepinani editacnich modu editoru atp.
	virtual int MouseOver(float x, float y, UINT over,UINT state);	// zajistuje stranovy scrolling mapy
	virtual int TimerImpulse(typeID timerID, float time);	// zajistuje stranovy scrolling mapy

	CGUIMenu* startMenu;
	CGUIMenu* editorMenu;

	bool ResolveKeyShortCuts(UINT character);	
		// zajistuje vyvolani akce po klavesove zkratce (ikony na panelu atp.)
		// vraci zda byla nejaka akce vykonana ci nikoliv

	float levelWindowDX, levelWindowDY;	// pro scrollovani okna s levelem (jak hodne se ma posunout)
	
};


//////////////////////////////////////////////////////////////////////
// CEDDesktopIconHandler
//////////////////////////////////////////////////////////////////////

// spravce udalosti pro desktop
// stara se o ikony a menu na panelu v desktopu
// zarizuje zobrazovani a skryvani menu - start, editor
// vyvolava akce po stisknuti ikony na panelu

class CEDDesktopIconHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CEDDesktopIconHandler() { icons = editorMenuIcons = 0; startButtonDown=editButtonDown=false;startButtonOff=editButtonOff=false; autoSelectFileDlgID=0;};
	virtual ~CEDDesktopIconHandler() { SAFE_DELETE_ARRAY(icons); SAFE_DELETE_ARRAY(editorMenuIcons);};

	void ShowAutoSelectFileDlg();
		// zobrazi dialog pro vyber souboru na automatismy

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
	typeID *editorMenuIcons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
	bool startButtonDown;
	bool startButtonOff;
	bool editButtonDown;
	bool editButtonOff;

	typeID autoSelectFileDlgID;
};


//////////////////////////////////////////////////////////////////////
// CEDUserAnnouncer
//////////////////////////////////////////////////////////////////////

// ohlasovac udalosti editoru (pro uzivatele)
// ohlasi zadany "text" a ponecha ho k precteni po dobu "time"
// pokud je "time==0" ponecha ho dokud neprijde jiny text nebo se nevypne pres "Hide"

class CEDUserAnnouncer : protected CGUIStaticText
{
public:
	static void Announce(char* text, float time=0);	// zobrazi text po dobu time (time==0 naporad)
	static void Hide(float time=0);		// skryre zobrazeny text za dobu time (time==0 ihned)

protected:
	CEDUserAnnouncer(char* text);
	virtual ~CEDUserAnnouncer();

	virtual int TimerImpulse(typeID timerID, float time);
	void SetPlace();
		// najde umisteni pro Announcer (podle vymezeneho mista v editoru a velikosti zobrazovaneho textu

	static CEDUserAnnouncer* announcer;
};


#endif
