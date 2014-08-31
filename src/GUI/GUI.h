///////////////////////////////////////////////
//
// GUI.h
//
// interface pro CGUI class
// hlavni objekt GUI, globalni servisni objekty GUI
// objekty pro kurzor, styly, timer, trash (garbage collector pro elementy GUI), fonty
// hlavni ovladac udalosti - CGUIMainHandler (zarizuje zmeny globalnich objektu GUI, napr. Desktopu atp.)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef GUI_H
#define GUI_H

#include "GUIConstants.h"
#include "objectList.h"
#include "element.h"

#include "MersenneTwister.h"

class CBltWindow;
class CBltRect;
class CTexture;
class CInput;
class CExc;

class CGUI;
class CGUIDesktop;
class CGUIStyleSet;
class CGUIElement;
class CGUICursor;
class CGUITrash;
class CGUIFont;
class CGUIMainHandler;
class CGUITimer;

extern CGUIDesktop *desktop;		// hlavni okno 
extern CGUIMsgQueue *msgQueue;		// fronta zprav, generovana vstupy (klavesnice, mys, timer)
extern CGUIStyleSet *styleSet;		// seznam stylu (grafika elementu GUI a dalsi parametry)
extern CGUITimer *timerGUI;			// casovac - objednavani casovych zprav
extern CGUI *mainGUI;				// hlavni objekt GUI
extern CGUITrash *trash;			// kos - garbage collector elementu GUI (obsahuje elementy, ktere nejsou umisteny v jinych strukturach GUI)
extern CGUINameServer *nameServer;	// objekt zajistujici preklad jmen a ID na adresy
extern CGUIEventServer *eventServer;// fronta udalosti, generovana elementy (dorucovani na zadost)
extern MTRand mtr;					// generator nahodnych cisel

//////////////////////////////////////////////////////////////////////
// CGUI
//////////////////////////////////////////////////////////////////////

class CGUI		// hlavni objekt GUI, prijima podnety z okoli (klavesnice, mys, timer), vytvari z nich zpravy a ty zpracovava
				// posilanim zprav prvkum, kterym patri
				// inicializuje globalni objekty, stara se o uklid po GUI
{
	friend class CGUIMainHandler;
public:
	CGUI( CBltWindow *_rootwnd, CInput *_input );
	virtual ~CGUI() throw();

	// vstupy od enginu:
	void SendChar(UINT charcode, UINT keydata );		// vstup z klavesnice - WM_CHAR
	void SendKey(UINT charcode, UINT keydata );		// vstup z klavesnice - WM_KEYDOWN
	void SendKeyUp(UINT charcode, UINT keydata );	// vstup z klavesnice - WM_KEYUP
	void MouseMove(int x, int y, UINT keyflags);		// vstup od mysi - WM_MOUSEMOVE
	void MouseBut(UINT msg, int x, int y, UINT keyflags);	// vstup od mysi - WM_L(RM)MOUSEUP(DOWN)
	void MouseWheel(int dz, int x, int y, UINT keyflags);	// vstup od mysi - WM_MOUSEWHEEL
	void MouseRelativeMove(int dx, int dy);					// vstup od mysi - relativni posun
	void Timer(float time);							// impuls casovaci o zmene casu [s]


	void GetEvent(CGUIMessage *msg);	// masazni pumpa - zpracovava frontu zprav
										// zajistuje doruceni vstupni zpravy


	void SendCursorPos();		// vyvola poslani nove zpravy s aktualni polohou kurzoru - MouseMove
								// novou zpravu jen zaradi do fronty, automaticky ji neposila
								// posle se az na ni prijde rada pri zpracovavani fronty

	void CursorInOut(bool in);		// vola engine pri vyjeti kurzoru mimo aplikaci (in==false)
									// a pri navratu zpet nad okno aplikace (in==true)

	void LogError(CExc c);		// logovani chyb do souboru


	// fce pro startovani editoru
	void SetEditorLevelInit(char* level);
	void SetEditorScriptInit(char* script);


	bool key_ctrl;			// stav klavesy (zda je stisknuta / nikoliv)
	bool key_alt;			// stav klavesy (zda je stisknuta / nikoliv)
	bool key_shift;			// stav klavesy (zda je stisknuta / nikoliv)

	HDC dc; 				// device context - pouziva se pri praci s fonty pomoci win32fci
	CGUICursor *cursor;		// kurzor GUI
	CBltWindow *rootwnd;	// pointer na korenove okno grafickeho enginu - pro pridavani elementu k vykresleni
	CGUIMainHandler *handler;	// hlavni zpracovavac udalosti (pouzivat jen pro nejvyssi uroven - napr. vymena desktopu)
	bool mouseOverBlock;	// blokuje posilani masaze MouseOver, posila jen MouseRelativeMove

	CInput *input;			// pro ziskavani vstupu z klavesnice pomoci DirectX

	int editorGameMod;		// zda se levely v editoru oteviraji v GameModu (1) nebo EditorModu (0)

	void StartProfile();	// zobrazi vyber profilu
	void StartOldMainMenu();	// zobrazi stary MainMenu
	void StartLevelEditor();	// spusti editor levlu
	void StartScriptEditor();// spusti editor scriptu
	void StartMainMenu();	// zobrazi MainMenu
	void Exit();				// ukonci program

protected:

	void LoadGUIData();		// nacteni grafickych prvku (textur, stylu, atp.), standardnich fontu
	void CreateCursor();	// vytvori kurzor a nahraje pro nej grafiku (pro vsechny typy kurzoru)

	UINT last_keyflags;		// posledni stav klaves a mysi, ktery prisel od enginu

	// promenne pro editor, uvodni spusteni levlu, ...
	char* RUN_EDITOR_SCRIPT;
	char* RUN_EDITOR_LEVEL;
};



//////////////////////////////////////////////////////////////////////
// CGUICursorImage
//////////////////////////////////////////////////////////////////////

class CGUICursorImage	// objekt spravujici jeden typ kurzoru
						// obsahuje obrazek kurzoru a jeho hotspot
						// hotspot jsou souradnice, (relativni vuci levemu hornimu rohu obrazku), 
						// ktere se berou jako pozice kurzoru v desktopu
{
public:	
	CGUICursorImage(CTexture *texture, int x, int y);	// vytvori kurzor z textury a hotspotu
	~CGUICursorImage();
	int cursor_hs_x, cursor_hs_y;	// souradnice hotspotu
	CBltRect *cursor;				// obrazek kurzoru
private:	
};


//////////////////////////////////////////////////////////////////////
// CGUICursor
//////////////////////////////////////////////////////////////////////

class CGUICursor	// objekt pro spravu vsech kurzoru v aplikaci
					// zajistuje zobrazovani, posun aktualniho kurzoru
					// umoznuje menit aktualni kurzor z nabidky naloudovanych kurzoru
					// loadovat nove kurzory v prubehu aplikace, menit za stare atp.
{
public:
	CGUICursor(CBltWindow *_rootwnd);			// inicializace kurzoru
	~CGUICursor();
	void Load(int index, char *textureName, int x, int y);	
		// vytvori novy kurzor z textury a hotspotu
		// umisti ho do pole pripravenych kurzoru na pozici "index"
		// pokud na pozici "index" uz je jiny kurzor, prepise ho novym
	void Load(int index, CTexture* texture, int x, int y);
		// vytvori novy kurzor z textury a hotspotu
		// umisti ho do pole pripravenych kurzoru na pozici "index"
		// pokud na pozici "index" uz je jiny kurzor, prepise ho novym
	int Set(int index, bool force=false);
		// nastavi aktualni kurzor na "index"
		// vraci 0 pri uspesnem provedeni, 1 - pri chybe 
		// force urcuje zda se provede zmena i pri stejnem indexu kurzoru jako ma aktualni kurzor (true) nebo se neprovede nic (false)
	int GetCursorIndex() {return actual_index;};
	void SetVisible(int vis);
	void Move(float x, float y);
	void GetPos(float &x, float &y);
	void BringToTop();

private:
	float cursor_x, cursor_y;	// poloha kurzoru v desktopu (aplikacnim okne)
	CBltWindow *rootwnd;		// odkaz do korenoveho okna grafickeho enginu
	
	CGUICursorImage *cursorImage[MAX_CURSORS];	// pole  kurzoru (MAX_CURSORS je def. v CGUIConstants.h)
	
	CBltRect *actual;
	int actual_index;
};


//////////////////////////////////////////////////////////////////////
// CGUIStyle
//////////////////////////////////////////////////////////////////////

class CGUIStyle				// objekt pro spravu Stylu = kolekce textur a pole parametru (floatu+paramUINT)
							// vytvareni stylu z textur a parametru
							// kontrolovany pristup k texturam a parametrum stylu
{
	friend class CGUIStyleSet;
	friend class CGUIListElement<CGUIStyle>;
public:
	CGUIStyle(char *_name, int num, int files, ...);// vyvori pojmenovany styl o "num" texturach zadanych jmenem souboru (bitmap)
	CGUIStyle(char *_name, int num, char *addr);	// vyvori pojmenovany styl o "num" texturach zadanych jmenem adresare, kde jsou umisteny (jmena textur jsou 0,1,2,...)
	~CGUIStyle() throw();
	CTexture *GetTexture(int num);					// vrati "num"-tou texturu ze stylu
	int GetNumTextures() {return numTextures;};		// vrati pocet textur ve stylu
	int GetNumParameters() {return num_parameters;};// vrati pocet parametru ve stylu
	
	void SetParameters(float *param, int num)throw();// nastavi pole parametru o num prvcich do stylu (puvodni pole dealokuje)
	float GetParametr(int i);						// vrati i-ty prvek parametru stylu
	float *GetAllParameters(){ return parameters;};	// vrati cele pole parametru

	float operator[] (int i);						// vrati i-ty prvek parametru stylu

	bool operator==(char* _name) throw();			// porovnani na jmeno stylu

	UINT paramUINT;									// UINTovy uzivatelsky parametr
protected:
	int numTextures;
	CTexture **textures;
	float *parameters;
	int num_parameters;
	char *name;
	CGUIList<CGUIStyle> *list;
};

class CGUIStyleSet			// objekt pro spravovani seznamu stylu podle jmena (pridavani, odebirani, vybirani)
{
public:
	CGUIStyleSet();
	~CGUIStyleSet() throw();

	void Add(CGUIStyle *_style);	// prida styl do seznamu stylu
	CGUIStyle *Get(char *name);		// vrati styl dle jmena
	CGUIStyle *Remove(char *name);	// vrati styl dle jmena a vyradi ho ze seznamu
	void Delete(char *name);		// vyradi styl ze seznamu a smaze ho

protected:
	CGUIList<CGUIStyle> styleList;
};


//////////////////////////////////////////////////////////////////////
// CGUITimerRequest
//////////////////////////////////////////////////////////////////////

class CGUITimerRequest		// objekt spravujici pozadavek na timer impulse (zprava timeru)
{							// pozadavek se sklada: z pointeru na zadatele (CGUIElement),
							// casu [s], za ktery bude zadatel upozornen (zpravou Timeru),
							// ID pozadavku (pro zadatele, aby mohl rozlisit timer impulsy, 
							// pokus si objednal vice pozadavku)
public:
	CGUITimerRequest(CGUIElement* el, float _time, typeID& _ID);
	~CGUITimerRequest() throw();

	bool operator==(typeID _ID);
	bool operator==(CGUIElement* _element);
private:
	CGUIElement* element;
	float time;
	typeID rID;
	static typeID lastID;			// citac pro vytvareni jednoznacne identifikace pozadavku

	friend class CGUITimer;
	friend class CGUIListElement<CGUITimerRequest>;
	CGUIList<CGUITimerRequest>* list;
};


//////////////////////////////////////////////////////////////////////
// CGUITimer
//////////////////////////////////////////////////////////////////////

class CGUITimer		// timer pro elementy GUI (prijima pozadavky na zavolani + posila casove impulsy elementum)
{
public:
	CGUITimer();
	~CGUITimer() throw();

	typeID AddRequest(CGUIElement* el, float time);		// prijme pozadavek od Elementu na zavolani za dobu time [s], vrati ID zpracovane zadosti
				// zadatel si musi dat pozor, aby pri svem ruseni odstranil vsechny sve zadosti z fronty timeru !!!
				// pozadavek zaridi jen jedno vygenerovani zpravy, pro pravidelne dorucovani je nutno znovu objednavat
	CGUITimerRequest *RemoveRequest(CGUIElement* el);	// odstrani zadost od Elementu (el) z fronty pozadavku (prvni na rade) a vrati ji
	CGUITimerRequest *RemoveRequest(typeID rID);		// odstrani zadost s ID z fronty pozadavku a vrati ji
	bool DeleteRequest(CGUIElement* el);		// smaze zadost od Elementu (el) z fronty pozadavku (prvni na rade) a vrati zda bylo neco smazano
	bool DeleteRequest(typeID rID);				// smaze zadost s ID z fronty pozadavku a vrati zda bylo neco smazano
	bool DeleteAllRequests(CGUIElement* el);	// smaze vsechny zadosti od Elementu (el) z fronty pozadavku a vrati zda bylo neco smazano
	bool ContainRequest(CGUIElement* el);		// vrati zda timer obsahuje zadost od elementu
	bool ContainRequest(typeID rID);			// vrati zda timer obsahuje zadost s ID

	void TimerImpulse(float time);				// prijme impuls od GUI informujici o zmene casu
												// zjisti zda uz neni cas nejakych pozadavku
												// posle vsem nacasovanym zadatelum zpravu - Timer Impulse
private:
	CGUIList<CGUITimerRequest> timerRequests;

	void SendTimerMessage(CGUITimerRequest* rqst);	// zarizuje zaslani zpravy zadateli o Timer impulse
};


//////////////////////////////////////////////////////////////////////
// CGUITrash
//////////////////////////////////////////////////////////////////////

class CGUITrash		// odpadkovy kos na elementy GUI, ktere nove vznikly a jeste nebyly pridany do zadnych seznamu (okno, multiwidget, obrazek v buttonu, atp.)
					// zajistuje odstranovani techto objektu v pripade vypadku GUI (vyjimky) v dobe mezi vznikem objektu a zarazenim do ostatnich struktur GUI
					// "garbage collector" pro elementy GUI
{
public:
	CGUITrash() {};
	~CGUITrash() throw();

	bool IsEmpty() { return (trash.GetFromHead()==0); };
	void EmptyTrash();		// vyprazdni kos (smaz vsechny elementy v kosi)

	void Add(CGUIElement *el);
	bool IsIn(CGUIElement *el);
	CGUIElement *Remove(CGUIElement *el);
	void Delete(CGUIElement *el);

private:
	CGUIList<CGUIElement> trash;
};

//////////////////////////////////////////////////////////////////////
// CGUIFont
//////////////////////////////////////////////////////////////////////

class CGUIFont : public CRefCnt
{	// objekt na spravu fontu
	// umoznuje vytvorit font dle parametru (velikost, styl, ...)
	// pro uvolneni objektu se vola "CGUIFont->Release();" nikoliv delete !!!
public:
	CGUIFont() { font = 0; };
	virtual ~CGUIFont() { if(font) DeleteObject(font); };
	void CreateFont(int nHeight,int nWidth,int nEscapement,int nOrientation,int nWeight,BYTE bItalic,
			BYTE bUnderline,BYTE cStrikeOut,BYTE nCharSet,BYTE nOutPrecision,BYTE nClipPrecision,
			BYTE nQuality,BYTE nPitchAndFamily,LPCTSTR lpszFacename);
	// obecna funkce na vytvoreni fontu - vytvari presne podle parametru jako std. win. fce
	void CGUIFont::CreateFont(LPCTSTR lpszFacename, int Height, int Weight=0, BYTE bItalic=0,
			BYTE bUnderline=FALSE, BYTE cStrikeOut=FALSE, int Width=0, int Orientation=0);
	// vytvori font podle vybranych nejpouzivanejsich parametru
	// lpszFacename - jmeno fontu jako string (oficialni windowsi nazev)
	// Height - vyska fontu v logical unit (jako velikost pisma ve wordu atp.)
	// Weight - tucnost fontu (0 - standard, FW_THIN, FW_BOLD, FW_NORMAL - dale viz. help)
	// Width - sirka fontu (0 - sirka je urcena podle vysky)
	// Orientation - natoceni fontu (0 - standard), v desetinach stupne proti smeru hodinovych rucicek
	void CGUIFont::CreateFontPS(LPCTSTR lpszFacename, int Height, int Weight=0, BYTE bItalic=0,
			BYTE bUnderline=FALSE, BYTE cStrikeOut=FALSE, int Width=0, int Orientation=0);
	// vytvori font podle vybranych nejpouzivanejsich parametru, jako jednotky velikosti pouziva pixely (nikoliv logical unit)

	HFONT font;
};

//////////////////////////////////////////////////////////////////////
// CGUIMainHandler
//////////////////////////////////////////////////////////////////////

/*
	zarizuje hlavni rizeni aplikace
	- prepinani desktopu
	- ukonceni aplikace

	reaguje na pripravena tlacitka hlavniho menu
		- Level Editor
		- Script Editor
		- Game
		- Quit

	reaguje na prikazy zasilane asynchronne pomoci udalosti ECommand:
		pInt == 1 - Script Editor	StartScriptEditor();
		pInt == 2 - Level Editor	StartLevelEditor();
		pInt == 3 - Game Menu		StartMainMenu();
		pInt == 4 - Quit			KRKAL->Quit();

*/

class CGUIMainHandler : public CGUIEventProgram
{
	friend class CGUI;

	virtual void EventHandler(CGUIEvent *event);
public:
	CGUIMainHandler(int numIcons) { icons = new typeID[numIcons]; for(int i=0;i<numIcons;i++) icons[i]=0;};
	virtual ~CGUIMainHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti

private:
	int StartProfile();	// zobrazi vyber profilu
	int StartOldMainMenu();	// zobrazi stary MainMenu
	int StartLevelEditor();	// spusti editor levlu
	int StartScriptEditor();// spusti editor scriptu
	int StartMainMenu();	// zobrazi MainMenu

};




#endif 
