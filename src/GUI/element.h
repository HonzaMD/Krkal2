///////////////////////////////////////////////
//
// element.h
//
//	interface for the CGUIPrimitiv & CGUIElement & CGUIWidget & CGUIWindow class
//		=> zakladni hierarchicka rada objektu GUI
//	CGUIPrimitiv - definuje zakladni vlastnosti (velikost, poloha, typ, viditelnost, ...)
//	CGUIElement - rozsiruje funkcnost predevsim o zpracovavani a reakce na zpravy
//	CGUIWidget - rozsiruje vlastnosti o ukotveni elementu v okne, zmena velikosti a polohy dle ukotveni
//			   - zajistuje kontrolu a dodrzovani mezi pro velikost a polohu elementu
//	CGUIWindow - zajistuje zakladni vlastnosti okna = pridavani a odebirani elementu (predni/zadni cast)
//			   - hledani elementu v okne dle polohy zpravy, forwarding zprav podelementum okna
//			   - focusovani elementu, viditelnost
//
//	interface pro CGUIDataExchange
//		- objekt pro podporu predavani dat mezi elementy GUI a uzivatelskymi programy
// 
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef ELEMENT_H
#define ELEMENT_H

#include "message.h"
#include "GUIConstants.h"
#include "objectList.h"

class CBltWindow;
class CBltRect;


class CGUITitle;
class CGUIMarker;
class CGUIWindowPanel;

//////////////////////////////////////////////////////////////////////
// CGUIPrimitive
//////////////////////////////////////////////////////////////////////

class CGUIPrimitive		// zakladni stavebni kamen vsech objektu v GUI
						// definuje nekolik virtualnich fci, ktere musi kazdy odvozeny objekt naimplementovat
						// dle svych konkretnich vlastnosti (AddTo/RemoveFromEngine, Resize, Move, SetVisible, BringToTop)
						// obsahuje zakladni vlastnosti objektu GUI (typ, velikost, poloha, viditelnost)
{
public:
	CGUIPrimitive();
	virtual ~CGUIPrimitive();

	virtual void AddToEngine(CBltWindow *rootwnd) = 0;	// prida do vykreslovaciho enginu
	virtual void RemoveFromEngine() = 0;	// odebere z vykreslovaciho enginu
	virtual void BringToTop() = 0;	// zmena viditelnosti na vrsek (nejvice videt)
	int IsIn(float _x, float _y);	// pocita ze souradnic nadrazeneho objektu a sve polohy v nem, zda je _x,_y v objektu
	virtual int IsInPrecise(float _x, float _y) { return 1;};	// zjisti zda je vevnitr podle detailniho tvaru objektu
					// preziznost se pocita uz ve vlasnim souradnem systemu (ne nadrazeneho objektu)

	virtual void Resize(float _sx, float _sy) = 0;	// nastavi velikost objektu 
	void ResizeRel(float dx, float dy);				// nastavi velikost objektu RELATIVNE k puvodni velikosti
	virtual void Move(float _x, float _y) = 0;		// nastavi polohu objektu
	void MoveRel(float dx, float dy);				// nastavi polohu objektu RELATIVNE k puvodni poloze

	int ResizeWithCheck(float &_sx, float &_sy);	// nastavi velikost objektu 
	// (vrati zda se zmenily vstupni parametry + zmenene vstupni parametry, 
	// vstupni parametry se mohou zmenit kvuli omezujicim podminkam na velikost objektu 
	// (min. / max. velikost) nebo polohu
	int ResizeRelWithCheck(float &dx, float &dy);	// nastavi velikost objektu relativne
	int MoveWithCheck(float &_x, float &_y);		// nastavi polohu objektu 
	// (vrati zda se zmenily vstupni parametry + zmenene vstupni parametry, 
	// vstupni parametry se mohou zmenit kvuli omezujicim podminkam na velikost objektu 
	// (min. / max. velikost) nebo polohu
	int MoveRelWithCheck(float &dx, float &dy);		// nastavi polohu objektu relativne

	void SetSize(float _sx, float _sy) { sx = _sx; sy = _sy;};	// jen nastavi promenne objektu
	void GetSize(float &_sx, float &_sy) { _sx = sx; _sy = sy;};
	void SetPos(float _x, float _y) { x = _x; y = _y;};			// jen nastavi promenne objektu
	void GetPos(float &_x, float &_y) { _x = x; _y = y;};
	
	void InnerCoords(float &_x, float &_y);		// prepocita vnejsi souradnice na souradnice uvnitr objektu

	int IsVisible(){return visible;}
	void SetVisibility(int vis) {visible = vis;};	// nastavi pouze promenou visible
	virtual void SetVisible(int vis) = 0;			// zviditelni objekt
	
	EGUIPrimitivType GetType() { return type;}

protected:
	float x,y;
	float sx, sy;
	int visible;
	EGUIPrimitivType type;
};

//////////////////////////////////////////////////////////////////////
// CGUIElement
//////////////////////////////////////////////////////////////////////


class CGUIElement : public CGUIPrimitive
{
	friend class CGUIListElement<CGUIElement>;
public:
	CGUIElement(float _x, float _y);
	CGUIElement(float _x, float _y, float _sx, float _sy);	// poloha, velikost, 
	virtual ~CGUIElement();

	void SetParent(CGUIElement *_parent) { parent = _parent; ParentNotification();};	// nastavi rodice + da vedet o zmene rodice, pokud si uzivatel pretizil ParentNotification
	CGUIElement *GetParent(void) { return parent; };	// vrati odkaz na rodice
	CGUIList<CGUIElement> *GetList(void) { return list; };	// vrati odkaz na seznam elementu, ve kterem jsem umisten (napr. seznam u okna)

	void AddToTrash();		// pokud element neni v nicem  zarazen, dam ho do trashe
							// lze pouzit jen na konci konstruktoru elementu, kde je jistota,
							// ze uz se za tim nestane vyjimka => element bude uspesne zkonstruovan
							// jinak bude v kosi pointer na objekt, ktery neexistuje a bude se na nej volat destruktor !!!
							// nepouzivat v konstruktorech elementu, ktere nejsou na konci hierarchie elementu
							// tedy je od nich odvozena dalsi trida (nevim co se stane v konstruktoru syna)
							// pripadne v konstruktoru syna na zacatku vyndat z kose a nakonce zase vratit do kose
	void RemoveFromTrash();	// pokud je element v trashi, tak ho z nej odstrani

	int GetMsg(CGUIMessage *msg);	// prijme zpravu a zacne ji zpracovavat (podle typu ji rozradi a zavola prislusnou obsluznou fci + pripadne preposle dal)
	void SendIntoMsg(CGUIMessage *msg, CGUIElement *to);	// posle zpravu elementu "to" s prepocitanim souradnic na jeho nadrazenou soustavu
															// jen pro posilani elementum pode mnou (v okne atp.)
															// poslani je realizovano neprimo, tedy zarazenim do fronty zprav nakonec + postupnym vybiranim zprav ve smyce
	void SendMsg(CGUIMessage *msg,  CGUIElement *to);		// posle zpravu elementu "to" - v podstate jen zaradi zpravu do hlavni fronty zprav + nastavi adresata
	void SendIntoPriorityMsg(CGUIMessage *msg, CGUIElement *to);	// jako SendIntoMsg, ale zaradi do prioritni fronty (zpracovava se drive nez standardni fronta)
	void SendPriorityMsg(CGUIMessage *msg,  CGUIElement *to);		// jako SendMsg, ale zaradi do prioritni fronty (zpracovava se drive nez standardni fronta)
	virtual int ForwardMsg(CGUIMessage *msg);		// preposle zpravu svym vnitrnim podelementum (implementuje kazdy objekt podle sveho charakteru (okno, multiwidget), ne kazdy objekt je schopen preposilat zpravy (nema zadne sve vnitrni elementy))

	void MsgAcceptConsume(int mask) {msgConsume |= (mask & MsgAll);};	// prida typ zprav k tem, ktere je schopen prijmout (zpracovat)
	void MsgRejectConsume(int mask) {msgConsume &= (~mask & MsgAll);};	// odebere typ zprav od tech, ktere je schopen prijmout (zpracovat)
	void MsgSetConsume(int mask) {msgConsume = (mask & MsgAll);};		// nastavi typy zprav, ktere je schopen prijmout (zpracovat) - presne ty uvedene, zadne jine
	void MsgAcceptProduce(int mask) {msgProduce |= (mask & MsgAll);};	// prida typ zprav k tem, ktere je schopen preposilat (ForwardMsg)
	void MsgRejectProduce(int mask) {msgProduce &= (~mask & MsgAll);};	// odebere typ zprav od tech, ktere je schopen preposilat (ForwardMsg)
	void MsgSetProduce(int mask) {msgProduce = (mask & MsgAll);};		// nastavi typy zprav, ktere je schopen preposilat (ForwardMsg) - presne ty uvedene, zadne jine
	

	// ------------------------------------------------------------------------------------------------------
	// fce pro obsluhu konkretni zpravy, ktera vznikla akci uzivatele (pohyb mysi, stisk tlacitka, klavesnice)
	// potomek si pretizenim fce definuje svou reakci na zpravu
	// podle toho co vrati se se zpravou naklada dale
	//	vrati 1 - zprava se preposila dalsim elementum (podelementum okna, multiwidgetu) pres ForwardMsg
	//  vrati 0 - zprava se jiz dale neposila

	// mouseState == 1 odpovida stisknutemu tlacitku
	// x,y - souradnice mysi v souradne soustave nadrazeneho objektu (napr. materskeho okna)
	// pro prevedeni souradnic na vnitrni souradnou soustavu slouzi fce InnerCoords(x,y)
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState){return 1;};
		// x,y - poloha kurzoru mysi
		// mouseState - 1 button down
		//				0 button up
		// keyState - state of Shift, Ctrl and Alt (and mouse buttons state) == WINDOWS VARIABLE
	virtual int MouseRight(float x, float y, UINT mouseState, UINT keyState){return 1;};
	virtual int MouseMiddle(float x, float y, UINT mouseState, UINT keyState){return 1;};
	virtual int DblClickLeft(float x, float y, UINT keyState){return 1;};
	virtual int DblClickRight(float x, float y, UINT keyState){return 1;};
	virtual int Wheel(float x, float y, int dw, int dz, UINT state){return 1;};
		// dw - zmena kolecka mysi (otoceni nahoru / dolu)
		//		1 mouse wheel up
		//		0 mouse wheel down
		// dz - skutecne otoceni kolecka (kolikrat se otocilo dopredu+ / dozadu-)
		// state - state of Shift, Ctrl and Alt (and mouse buttons state) == WINDOWS VARIABLE
	virtual int MouseOver(float x, float y, UINT over,UINT state){return 1;};
		// over - kurzor mysi nad elementem / mimo element
		//		- 0 mouse out
		// 	   	  1 mouse over
	virtual int MouseRelativeMove(float dx, float dy){return 1;};
		// dostava pouze focusovany element

	virtual int KeyboardChar(UINT character, UINT state){return 1;};
		// character - char code zmacknute klavesy (prelozeno pomoci MS Windows - WM_CHAR)
	virtual int Keyboard(UINT character, UINT state){return 1;};
		// character - char code zmacknute klavesy	(WM_KEYDOWN)
	virtual int KeyboardUp(UINT character, UINT state){return 1;};
		// character - char code pustene klavesy	(WM_KEYUP)
	virtual int Command(typeID command, CGUIMessage *msg){return 1;};
		// command - druh prikazu z vyctu EComs
	virtual int MouseDown(float x, float y, UINT state){ return 1;};
		// state - state of Shift, Ctrl and Alt (and mouse buttons state) == WINDOWS VARIABLE
	virtual int TimerImpulse(typeID timerID, float time) { return 1;};
		// timerID - ID zadosti o timer impulse (pro rozliseni mezi jednotlivyma zadostma)
		// time - zpozdeni mezi pozadovanym impulsem a skutecnym impulsem
	virtual int FocusChanged(UINT focus, typeID dualID) { return 1;};
		// focus - na co se focus zmenil 
		//			0 - focus ztracen
		//			1 - focus nabyd
		// dualID - ID zpravy, kterou obdrzi (obdrzel) dualni element
		//				(kdyz ja jsem dostal focus, dualni element ho ztratil)
		//	FocusChanged zprava je pri zmene focusu nejdrive posilana pozbyvateli, potom nabyvateli

	// fce ktere implementuje objekt, jenz chce reagovat na zpravy typu "focus_msg",
	// tedy zpravy, ktere nejsou urcene jemu, ale dostane je nebot je focusovany
	// parametr ID urcuje ID zpravy, ktera je ve fronte zprav a ceka na spracovani skutecnym adresatem
	// ID se muze hodit napr. k smazani teto zpravy, nebo pozmeneni, atp.
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID){return 1;};
	virtual int MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID){return 1;};
	virtual int MouseMiddleFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID){return 1;};
	virtual int DblClickLeftFocus(float x, float y, UINT keyState, typeID ID){return 1;};
	virtual int DblClickRightFocus(float x, float y, UINT keyState, typeID ID){return 1;};
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID){return 1;};
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID){return 1;};
	virtual int KeyboardCharFocus(UINT character, UINT state, typeID ID){return 1;};
	virtual int KeyboardFocus(UINT character, UINT state, typeID ID){return 1;};
	virtual int KeyboardUpFocus(UINT character, UINT state, typeID ID){return 1;};
	virtual int CommandFocus(typeID command, CGUIMessage *msg, typeID ID){return 1;};
	virtual int MouseDownFocus(float x, float y, UINT state, typeID ID){ return 1;}; 
	virtual int TimerImpulseFocus(typeID timerID, float time, typeID ID) { return 1;};	// ??? tezko se muze tato zprava vyskytnout ve focusovane verzi
	virtual int FocusChangedFocus(UINT focus, typeID dualID, typeID ID) { return 1;};		// ??? tezko se muze tato zprava vyskytnout ve focusovane verzi
	

	float GetTabOrder() { return tab_order;};	// vrati poradi elementu pro prepinani TABem
	void SetTabOrder(float to);					// nastavi poradi elementu pro prepinani TABem (==0 - neprepina se tabem)
	bool GetMark() { return mark;};		// vrati zda chce byt element oznacovan (standardnim oznacovacem)
	void SetMark(bool _mark);			// nastavi zda chce byt element oznacovan (standardnim oznacovacem)

	bool IsFocused();	

	void Center(bool _x=true, bool _y=true);	// vycentruje element v okne (desktopu) podle osy x,y
												// !!!! element jiz musi byt umisten v okne, jinak nedela nic

	virtual void ParentNotification() {};	// vola se pri nastaveni otce elementu
											// pro uzivatelske pretizeni, pokud se chci dovedet o zmene nastaveni meho otce

	bool focusable;			// zda je mozne tento element focusovat (pokud nelze => tzn., pri pokusu o automaticky refocus (ve ForwardMsg) se nic nestane)
							// urcuje take moznost preposilani u widgetu (blize u widgetu - ForwardMsg)
	bool throw_going;		// pokud je element "pruchozi" nedostava zpravy od mysi
							// zpravy od mysi se posilaji eventualnimu dalsimu elementu v poradi (z fronty u okna/multiwidgetu)
	bool back_element;		// zda se jedna o element na predku okna (false) nebo o vnitrni element okna (true)
							// pokud element neni zarazen do okna (false), nastavuje prirazujici okeni fce
							// parametr pro urceni zda provadet transformaci souradnic pri dorucovani zprav (true)=>provadet
	
	CGUIElement *parent;	// pointer na matersky objekt (otce)

	bool exclusive_keyboard;	// urcuje zda si element chce vyhradit exkluzivni pristup ke klavesnici, 
								// tzn., ze jeho materske okno nebude zpracovavat vstup z klavesnice a 
								// rovnou ho preposle tomuto elementu
								// exkluzivita funguje jen na focusovany element

	virtual void MarkElement();	// oznaci element (nejakym zpusobem el. zvyrazni, aby bylo videt, ze je oznaceny = focusovany)
								// aby se element mohl oznacit musi mit nastaveno tab_order, jinak se neoznacuje
								// std. oznaceni je obdelnik kolem elementu
								// kazdy element muze fci pretizit a udelat si vlastni oznaceni dle libosti
	virtual void UnMarkElement();	// vola se pro odoznaceni elementu (pri ztrate focusu)
	virtual void CreateMarkStructures(CBltWindow* win);	// vytvori struktury (objekt) pro oznacovani elementu
	virtual void DeleteMarkStructures();	// vola se pri ruseni elementu / vyndavani elementu z okna atp. (kvuli zruseni struktur potrebnych pro oznacovani elementu, uz nebudou treba)

protected:
	float tab_order;		// poradi elementu pro prepinani mezi elementy okna (multiwidgetu) TABem
							// 0 - element je mimo mnozinu prepinatelnych elementu (nelze na nej prepnout TABem)
							//		ani se nevykresluje oznaceni o vyberu (focusu) tohoto elementu
							// >0 - cim vyssi cislo tim dale ve fronte elementu
							// - materske okno si uderzuje tab_min a tab_max (minimalni a maximalni hodnotu tab_orderu svych podelementu)
	bool mark;				// zda chce byt element oznacovan
	CGUIMarker* marker;		// objekt na std. oznacovani elementu

	int msgConsume;		// maska zprav, ktere je element schopen prijmout (zpracovat)
	int msgProduce;		// maska zprav, ktere je element schopen preposilat (Forward)
	
	CGUIList<CGUIElement> *list;	// pointer na seznam, ve kterem je obsazen
};


//////////////////////////////////////////////////////////////////////
// CGUIWidget
//////////////////////////////////////////////////////////////////////

class CGUIWidget : public CGUIElement	// zaklad pro vytvareni elementu (ovladacich prvku, popisku, ...) okna
{
public:
	CGUIWidget(float _x, float _y);
	CGUIWidget(float _x, float _y, float _sx, float _sy);
	virtual ~CGUIWidget();

	void SetAnchor(int up, int bottom, int left, int right) {anchor_u=up, anchor_b=bottom, anchor_l=left, anchor_r=right;};	
		// nastaveni ukotveni prvku
		// ktere strany okna se widget drzi (je k ni ukotven)
		// standardne je ukotven k leve a horni strane
		// pokud je ukotven k protilehlym stranam, meni widget svou velikost v zavislosti na velikosti okna
	void SetLimitSizes(float _wWidthMIN = 0, float _wHeightMIN = 0, float _wWidthMAX = 0, float _wHeightMAX = 0);
		// nastavi meze pro velikost widgetu (0 = neomezeno)
		// zaridi pripadnou zmenu velikosti okna, aby vyhovovalo nastavenym mezim

	void SetLimitPosition(bool _min_position = false, bool _max_position = false, float _wMinX = 0, float _wMinY = 0, float _wMaxX = 0, float _wMaxY = 0);
		// nastavi meze pro velikost widgetu (0 = neomezeno)
		// zaridi pripadnou zmenu polohy okna, aby vyhovovalo nastavenym mezim



	virtual int ForwardMsg(CGUIMessage *msg);	// jedna se o jiny zpusob forwardovani zprav nez u okna !
									// widget nema zadne sve podelementy, kterym by preposilal zpravy
									// jedna se o poslani zpravy elementu rodicovskeho okna, ktery je
									// dalsi v poradi elementu okna a pripada vzhledem k poloze zpravy
									// v uvahu (2 elementy pres sebe)
									// zprava se tedy propusti na dalsi element na stejnem miste, ovsem
									// hloubeji zanoreny
									// element, ktery tento "pruklep" umoznuje nemuze byt focusovatelny
									// presneji: focusovatelny element neumoznuje tento typ nasledneho Forwardingu !!!

	void WindowSizeChanged(float dx, float dy);
								// rika widgetu, ze se zmenila velikost jeho materskeho okna
								// widget podle svych kotev zmeni svou polohu a velikost

	int CheckResize(float &dx, float &dy);	// zkontroluje zda pozadovana nova velikost widgetu (dx,dy)
	// nevybocuje z povolenych mezi pro velikost widgetu, eventualne upravi zmenu velikosti tak,
	// aby odpovidala mezim (vrati 1 pokud upravil nejakou hodnotu, 0 = bez uprav (meze v poradku))
	int CheckPosition(float &px, float &py);	// zkontroluje zda pozadovana nova poloha widgetu (px,py)
	// nevybocuje z povolenych mezi pro polohu widgetu, eventualne upravi zmenu polohy tak,
	// aby odpovidala mezim (vrati 1 pokud upravil nejakou hodnotu, 0 = bez uprav (meze v poradku))
	int CheckPositionRel(float &dx, float &dy);	// kontrola relativne k aktualni poloze
	int CheckResizeRel(float &dx, float &dy);	// kontrola relativne k aktualni velikosti

	bool full_window_anchor;		// zda se widget roztahuje pres celou sirku okna (dulezite pro widgety v back window pri nastavovani backWindowNormalSize)
protected:
	int anchor_u, anchor_b, anchor_l, anchor_r;
	float wWidthMIN, wWidthMAX, wHeightMIN, wHeightMAX;
	float wMinX, wMaxX, wMinY, wMaxY;
	bool min_position, max_position;	// zda se pouzije nastaveni omezeni pro polohu
};


//////////////////////////////////////////////////////////////////////
// CGUIMultiWidget
//////////////////////////////////////////////////////////////////////

class CGUIMultiWidget : public CGUIWidget, public CGUIEventSupport	
							// Widget obsahujici frontu elementu 
							//(objekt sestaveny z nekolika elementu (takove orezane "okno" - jen jedna fronta, bez scrollingu))
							// propaguje prijate zpravy svym podelementum
{
	friend class CGUIElement;
	friend class CGUIWidget;
public:
	CGUIMultiWidget(float _x, float _y);
	CGUIMultiWidget(float _x, float _y, float _sx, float _sy);
	virtual ~CGUIMultiWidget();

	virtual void BringToTop();						
	void BringElementToTop(CGUIElement* el);

	virtual void Move(float _x, float _y);
	virtual void Resize(float _sx, float _sy);
	virtual void SetVisible(int vis);				// nastavi viditelnost MultiWidgetu (CBltWindow), viditelnost podelementu necha nezmenenou
	void SetAllElementVisible(int vis);				// nastavi viditelnost vsech podelementu (viditelnost vlastniho MultiWidgetu (CBltWindow) necha nezmenenou)

	void AddElem(CGUIElement *element);
	CGUIElement *RemoveElem(CGUIElement *element);
	void DeleteElem(CGUIElement *element);
	void DeleteAllElems(void);
	CGUIElement *GetNextElement(CGUIElement *element);	// vrati nasledujici element z fronty podelementu multiwidgetu (0 -  vrati prvni element)
	bool ContainsElement(CGUIElement *element);		// vrati zda obsahuje element

	CGUIElement* FindNextTABElement(float to);		// najde nasledujici element ve fronte, podle nastaveneho poradi pro prepinani TABem
	CGUIElement* FindPreviousTABElement(float to);	// najde predchozi element ve fronte, podle nastaveneho poradi pro prepinani TABem

	CGUIElement *GetFocusEl(void);		// vrati focusovany element 
	void SetFocusEl(CGUIElement* el);	// nastavi focusovany element 

protected:
	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void AddToEngine(CBltWindow *rootwnd);	
	virtual void RemoveFromEngine();				

	virtual int ForwardMsg(CGUIMessage *msg);
	int ForwardMsgToDeeperElement(CGUIMessage *msg, CGUIElement *fe);	// zprava, pruklepovy element

	// pomocne fce pro ForwardMsg:
	CGUIElement *FindElement(float ex, float ey);	// najde podelement obsahujici bod x,y (nejvyssi v poradi fronty elementu)
	CGUIElement *FindDeeperElement(CGUIElement *fe, float ex, float ey);	// jako FindElement, ale nezacina od zacatku fronty, ale az za elementem "fe"

	CGUIElement *last_element_under_cursor;

	float tab_min;	// minimalni poradi TAB orderu (prepinani elementu TABem)
	float tab_max;	// maximalni poradi TAB orderu (prepinani elementu TABem)

	CBltWindow *window;		// okno enginu pro vykreslovani
	CGUIList<CGUIElement> el_queue;	// fronta vnitnich podelementu
	CGUIElement *focus_elm;	// pointer na focusovany element z fronty elementu

	static const int numEvents;
	static const int availableEvents[];
};


//////////////////////////////////////////////////////////////////////
// CGUIWindow
//////////////////////////////////////////////////////////////////////

// stavy okna:
enum EWindowState{
	WS_Standard = 0,	// jakakoliv velikost
	WS_Minimized,	// okno je minimalizovano
	WS_Maximized,	// okno je maximalizovano
	WS_Normalized	// okno je normalizovano (velikost "WindowSizeBWNormal")
};


class CGUIWindow : public CGUIWidget, public CGUIEventSupport
{
	friend class CGUIMainHandler;		// kvuli pridavani desktopu do enginu (AddToEngine
	friend class CGUIScrollBar;
	friend class CGUIScrollbarButton;
	friend class CGUITitle;
	friend class CGUIResizer;
	friend class CGUIWindowPanel;
	friend class CGUIElement;
	friend class CGUIWidget;	// kvuli volani ForwardMsgToDeeperElement
public:
	CGUIWindow(float _x, float _y, float _sx, float _sy, int withBackground=1, int bgColor=0xAA0000AA);
	CGUIWindow(float _x, float _y, float _sx, float _sy, CBltRect *_background);
	virtual ~CGUIWindow();
	
	virtual void BringToTop();						// nastavi okno na nejvyssi uroven viditelnosti v enginu 
													//(v ramci sveho rodicovskeho (nadrazeneho) okna)
													// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	void BringElementToTop(CGUIElement* el);		// nastavi element v predni/zadni fronte na vrsek 
													//(pri hledani ma prednost, viditelnost se nemeni)
													// zmena viditelnosti se provadi pomoci el->BringToTop()
													// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	virtual void Move(float x, float y);			// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	virtual void Resize(float _sx, float _sy);		// nastavi velikost celeho okna - vnejsku (patricne upravi velikost vnitrku, scrollbary atp.)
													// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	void ResizeFrontOnly(float _sx, float _sy);		// nastavi velikost vnejsku okna (front), vnitrek necha stejny (back)
													// kvuli zmenseni okna po odebrani scrollbaru pri max. velikosti bw (bw jiz nelze zvetsit o odebrany scrollbar => musi se zmensit okno, ale jen predek)

	virtual void SetVisible(int vis);				// nastavi viditelnost okna (viditelnost vsech jeho elementu necha nezmenenou)
													// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	void SetAllElementVisible(int vis);				// nastavi viditelnost vsech podelementu okna (viditelnost vlastniho okna necha nezmenenou)
													// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)
	
	void AddBackground(CBltRect *_background);
	void RemoveBackground();

	virtual int CloseWindow();		// zavre okno, provede veskerou konsolidacni praci vcetne
									// poslani zpravy nadrizenemu oknu, at me vyradi ze seznamu 
									// svych podelementu a zrusi (nemuzu se zrusit sam)
									// vola se pri prichodu zpravy (command:ComCloseWnd)
									// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)

	void SyncAll(bool out=true);	// synchronizuje vsechny elementy okna pres CGUIDataExchange (prekopiruje data mezi svazanyma promennyma)


	// prace s vnitrnimi elementy okna (front):
	void AddBackElem(CGUIElement *element);
	CGUIElement *RemoveBackElem(CGUIElement *element);
	void DeleteBackElem(CGUIElement *element);
	void DeleteAllBackElems(void);
	CGUIElement *GetNextBackElement(CGUIElement *element);	// (element==0 => vrati prvni zadni element)
	bool ContainsBackElement(CGUIElement *element);	// vrati zda okno obsahuje element

	// prace s vnejsimi elementy okna (back):
	void AddFrontElem(CGUIElement *element);
	CGUIElement *RemoveFrontElem(CGUIElement *element);
	void DeleteFrontElem(CGUIElement *element);
	void DeleteAllFrontElems(void);
	CGUIElement *GetNextFrontElement(CGUIElement *element);	// (element==0 => vrati prvni predni element)
	bool ContainsFrontElement(CGUIElement *element);	// vrati zda okno obsahuje element

	// prace s focusem:
	CGUIElement *GetFocusEl(void);				// vrati focusovany element okna 
												// z predni nebo zadni fronty (podle front_focusu)
												// v okne je jen jeden element focusovany (a to bud v predni nebo zadni fronte)
	int IsFrontFocus(void) { return front_focus; };	// zda je focus na predek (1) / zadek (0)
	virtual void SetFocusEl(CGUIElement* el, int _front_focus);	// nastavi focusovany element v predni/zadni fronte + front_focus
	void FocusMe();			// focusuje toto okno + nastavi i max. klikatelnost a viditelnost


	// transformace souradnic:
	bool TransformCoordsFrontToBack(float &tx, float &ty);	
	// prevede souradnice materskeho okna do souradnic back wnd. (dle polohy viewportu 
	// v materskem okne a dle polohy viewportu v back wnd.)
	// vrati zda vstupni souradnice padly do back wnd. pres viewport ci nikoliv (klik do front wnd.)

	bool TransformCoordsBackToFront(float &tx, float &ty);
		// prevede souradnice back wnd. do souradnic front wnd.
		// pokud zadane souradnice jsou ve viditelne casti back wnd. vrati true, jinak false


	// prace s TAB Order elementy:
	CGUIElement* FindNextTABElement(float to, bool frontEl);	// najde nasledujici element ve fronte, podle nastaveneho poradi pro prepinani TABem (frontEl urcuje v jake fronte se bude hledat - front/back)
	CGUIElement* FindPreviousTABElement(float to, bool frontEl);// najde predchozi element ve fronte, podle nastaveneho poradi pro prepinani TABem (frontEl urcuje v jake fronte se bude hledat - front/back)
	CGUIElement* FindTABElement(float to, bool frontEl);		// najde element ve fronte, podle nastaveneho poradi pro prepinani TABem, pokud ve fronte neni element s TAB Orderem==to vrati prvni element s nastavenym TAB Orderem (tab_min) (frontEl urcuje v jake fronte se bude hledat - front/back)
	
	virtual void FocusOldTOBackEl();	// focusuje posledni oznaceny element (s nastavenym TAB Orderem) v zadni fronte, pred focusaci na predek okna 
										// kvuli navratu focusu do zadku okna po provedeni akce v predku (resize okna resizerem, posun titulkem, maximalizace cudlitkam atp.)

	// nastavovani velikosti a pozice vnitrku okna, mezni velikosti atp.:
	virtual void SetWindowPositionVP(float dx, float dy, bool updateScrollbars = true);		// zmeni pozici viewportu do okna (relativne) - scrollovani oknem
																					// aktualizuje nastaveni scrollbaru (pokud jsou zapnuty a je to vyzadano)
																					// !@#$ nutne po zavolani aktualizovat kurzor (mohla se zmenit jeho pozice vuci elementum GUI)

	void SetBackWindowNormalSize(float nsx, float nsy);	// nastavi velikost back wnd 
								// + patricne upravi aktualni viewport podle novych parametru
								// (elementy v okne mohou byt na pozici 0..bw_normal-1)
								// bw_normal je sirka (vyska) plochy pro elementy ve vnitrku okna
								// aktualizuje nastaveni scrollbaru (pokud jsou zapnuty)
								// BW normal size je hranicni velikost rikajici:
								// pokud je okno (VP) mensi jak BW normal size, zobrazuji se scrollbary, ukotvene elementy nemeni svou velikost
								// pokud je okno (VP) vetsi jak BW normal size, scrollbary se nezobrazuji (nejsou zapotrebi, je videt cely vnitrek okna), ukotvene elementy se zvetsuji (roztahuji - podle ukotveni)
								// elementy v okne se navrhuji na velikost okna prave BW normal size
								// pokud je nastavovana BW normal size vetsi nez max. BW size, upravi max. BW size dle nove BW normal size (BW max. size nesmi byt mensi nez BW normal size)
								
	// !@#$ ??? uvidime :-)		// NEMELO by se volat s takovymi parametry, ktere by 
								// zpusobily zmenseni BWnormalSize v jedne ose a zvetseni
								// BWnormalSize v druhe ose
								// toto volani muze zpusobit u nekterych widgetu 
								// (full_window_anchor==true) spatne zmeneni velikosti
								// podle kotveni (nedostatecne zmenseni)
								// radeji volat nadvakrat (napr. nejdrive zvetsit + 0,
								// potom zmensit + 0, tj. druhy parametr beze zmeny)

	void GetBackWindowNormalSize(float &nsx, float &nsy) 	{ nsx = bw_size_normal_x; nsy = bw_size_normal_y; };
	void GetBackWindowSize(float &nsx, float &nsy) 	{ nsx = bw_size_x; nsy = bw_size_y; };
	void GetVPSize(float &nsx, float &nsy) { nsx = vp_sx; nsy = vp_sy; };
	void GetVPPos(float &nx, float &ny) { nx = vp_x; ny = vp_y; };
	void GetBWStartPos(float &b_x, float &b_y) { b_x = bw_x; b_y = bw_y; };

	void SetWindowSizeVP(float _vp_sx, float _vp_sy);	// nastavi velikost ViewPortu dle parametru
							// a patricne nastavi velikost hlavniho okna, aby se do nej vesel VP
	void SetWindowSizeBWNormal();		// upravi velikost okna, aby se do nej veslo cele back window

	void SetWindowLimitSizesBW(float _wWidthMIN = 0, float _wHeightMIN = 0, float _wWidthMAX = 0, float _wHeightMAX = 0, bool min = true, bool max = true);
			// nastavi mezni velikosti celeho okna podle hodnot zadanych jako mezni velikosti BW (min, max urcuje zda se maji hodnoty nastavit nebo ignorovat - napr. pro nastaveni pouze maxima)
			// pozor, nekombinovat s nastavenim limit pro ViewPort a BW (viz. dalsi fce) - muze zpusobit nastaveni spatne velikosti okna (napr. kdyz se BW (ci VP) nevejde do celeho okna, pozor na scrollbary)
	
	void SetBWLimitSizes(float _wWidthMIN = 0, float _wHeightMIN = 0, float _wWidthMAX = 0, float _wHeightMAX = 0, bool min = true, bool max = true);
			// nastavi maximalni velikost BW (bw_size_max_x) a minimalni velikost VP (vp_min_sx) (min, max urcuje zda se maji hodnoty nastavit nebo ignorovat - napr. pro nastaveni pouze maxima)
			// BW normal size nesmi byt vetsi nez BW max. size (fce to kontroluje a pripadne zmensi BW normal size)
			// pozor, nekombinovat s nastavenim limit pro cele okno (viz. predchozi fce, SetLimitSizes) - muze zpusobit nastaveni spatne velikosti okna (napr. kdyz se BW (ci VP) nevejde do celeho okna, pozor na scrollbary)


	virtual void Minimize();		// minimalizace okna
	virtual void Maximize();		// maximalizace okna do aktualni velikosti nadrazeneho okna (viewportu)
	virtual void MaximizeFull();	// maximalizace okna do plne velikosti nadrazeneho okna (bw_size)
	virtual void Normalize();		// normalizace okna (nastavi velikost na "WindowSizeBWNormal")
	virtual void UnMinimize();		// vrati minimalizovane okna do puvodniho stavu (opak minimalizace)
	virtual void UnMaximize();		// vrati maximalizovane okna do puvodniho stavu (opak maximalizace)

	void MinimizeAll();		// minimalizuje vsechny okna (jen viditelne)
	void UnMinimizeAll();	// vrati minimalizovane okna do puvodniho stavu (jen ty, ktere byly predtim minimalizovane pres MinimizeAll() )
	void SetAllWindowVisible();	// zobrazi vsechny vlastni podokna

	int IsModal(){ return modal;};
	void SetModal(int _modal, bool set_exclusive_keyboard_like_modality=true);
		// nastavi okno do modalniho rezimu (lze klikat pouze do nej, dokud se modalni rezim nezrusi ci okno nezavre)
		// funkcnost se zarizuje u rodicovskeho okna (desktopu) ve ForwardMsg
		// modalni okno lze prefocusovat pomoci SetFocusEl, ale pouze na jine modalni okno, nebo menu, jinak nelze
		// "set_exclusive_keyboard_like_modality" urcuje zda se nastavi i "exclusive_keyboard" podle hodnoty "modal"
		// exkluzivni pristup ke klavesnici mimo jine zaridi, ze nebudou funkcni klavesove zkratky vsech menu (krome tech s nastavenym povolenim k exkluzivnimu pristupu)

	bool autogrowing;		// zda pri pridavani noveho elementu do vnitrku okna mimo velikost 
		// back window se velikost back window zvetsi podle polohy a velikosti pridavaneho elementu


	CGUITitle* title;		// odkaz na titulek (pokud okno nema titulek == 0)
							// kvuli urychleni vyhledavani informaci v titulku (jmeno okna, ikona)

	CGUIWindowPanel* panel;	// odkaz na panel (pokud okno nema panel == 0)
							// kvuli pridavani ikon na panel
							// kvuli zjistovani plochy u rodice pro maximalizaci okna
	bool onWindowPanel;		// zda muze byt okno na panelu (zda se pri pridavani do systemu prida i na panel sveho rodice nebo nikoliv)
	bool topOnFocus;		// zda se okno topuje pri focusaci

	int windowState;		// stav okna (dle EWindowStates)
	bool minimized_all;			// zda byly vsechny podokna minimalizovany (MinimizeAll())
								// kvuli rozhodnuti zda mam minimalizovat nebo obnovovat po minimalizaci

	bool scrollChildWindowRather;	// nastavuje zpusob chovani scrollingu pomoci kolecka mysi a "chytani okna spacem"
    	//(true) => v pripade, ze pod kurzorem okna je dalsi okno (synovske okno), rodicovskym oknem nescrolluje, preda pozadavek na synovske okno  
		//(false==default) => scrolluje materskym oknem, pozadavek na scrollovani se dale nepredava (k synovskemu oknu se jiz nedostane)
		//						pokud materske okno nemuze scrollovat, pozadavek se posle synum

	bool keyClose;			// zda se okno zavre na klavesu F5
	bool keyMaximalize;		// zda se okno maximalizuje na klavesu F6 (pri opakovani se vrati do puvodni velikosti)
							// CTRL+F6 (normalizace)
	bool escapeClose;	    // zda se okno zavre pri ESC
	bool excapeMinimize;    // zda se okno minimalizuje pri ESC

protected:
	// funkce zajistujici spolupraci s enginem (graficka cast):
	virtual void AddToEngine(CBltWindow *rootwnd);	// prida okno (front, back) do vykreslovaciho enginu
	virtual void RemoveFromEngine();				// odebere okno z vykreslovaciho enginu

	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int FocusChanged(UINT focus, typeID dualID); 
	virtual int Wheel(float x, float y, int dw, int dz, UINT state);
	virtual int KeyboardChar(UINT character, UINT state);
	virtual int Keyboard(UINT character, UINT state);
	virtual int KeyboardUp(UINT character, UINT state);
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int MouseRelativeMove(float dx, float dy);

	virtual int Command(typeID command, CGUIMessage *msg);	// reakce na zpravy typu prikaz

	virtual void EventHandler(CGUIEvent *event);	// reaguje na udalosti
													// zabiji elementy na zadost - udalost EKillMe
													// prepina elementy, dle TAB Orderu, na zadost - udalost EChangeTABElement

	virtual void ParentNotification();				// zarizuje objednani udalosti EChangeTABElement rodici (v pripade ze se jedna o strom)
													// v pripade, ze se protoci fronta TAB Orderu posle udalost otci (stromu) aby se prepnul na nasledujici polozku

	virtual int ForwardMsg(CGUIMessage *msg);		// preposilani zprav svym podelementum (podle polohy a focusu)
	int ForwardMsgToDeeperElement(CGUIMessage *msg, CGUIElement *fe);	// zprava, pruklepovy element

	// pomocne fce pro ForwardMsg:
	CGUIElement *FindElement(float ex, float ey, int *front=0);	// najde element okna 
			//obsahujici bod x,y (nejvyssi v poradi fronty elementu)
			// pokud se jedna o element back_window provede transformaci vstupnich souradnic
	CGUIElement *FindDeeperElement(CGUIElement *fe, float &ex, float &ey, int *front);	// jako FindElement, ale nezacina od zacatku fronty, ale az za elementem "fe"


	void RefocusToNextWindow(CGUIWindow* wnd);	// prepne focus na nasledujici okno ve fronte
												// nastavi i max. klikatelnost a viditelnost na focusovane okno
												// focusuje jen okna, ktera nejsou minimalizovana a jsou videt a jsou na panelu (onWindowPanel)
												// pokud je wnd==0 hleda prvni mozne okno v zadni fronte potom v predni
												// pokud se nenaslo zadne okno k focusovani, focusuje nasledujici TAB element
	typeID GetNextWindowID(typeID windowID);	// vrati ID dalsiho okna ve fronte elementu


	virtual void ChangeViewPortBWSize(float &dx, float &dy);	// zmeni patricne velikost ViewPortu (polohu vp),
														// eventualne i velikost bw (zvetsi bw nad bw_normal_size)
														// kontroluje meze velikosti + upravuje podle nich dx, dy
														// dx,dy - relativni zmena velikosti
														// !!! INTERNI fce, neni urcena pro uzivatele GUI




	CGUIList<CGUIElement> queue_front, queue_back;	// predni a zadni fronta elementu okna
	int front_focus;	// zda je focus na predek (1) / zadek (0)
	CGUIElement *focus_elm;	// pointer na focusovany element z fronty elementu
	CGUIElement *last_element_under_cursor;	// element, nad kterym byl naposled kurzor - kvuli posilani focus zpravy MouseOverFocus atp.

	CBltWindow *front, *back;	// struktury pro graficky engine - vykreslovani okna
	CBltRect *background;


	// atributy pro spravu BACK WINDOW a jeho ViewPortu
	float bw_size_normal_x, bw_size_max_x;	// meze velikosti back window (normal, maximal) (prvky v okne mohou byt na pozici 0..bw_normal-1)
	float bw_size_normal_y, bw_size_max_y;	// meze velikosti back window (normal, maximal)
	float bw_size_x, bw_size_y;		// aktualni velikost back window (bw_size >= vp_s, bw_size>=bw_size_normal)
	float bw_x, bw_y;				// poloha (pocatek) back window v souradne soustave okna (front window)
									// hodi se napr. pro prevod souradnic mezi oknem a back window pri posilani zprav
	float vp_x, vp_y;		// poloha viewportu do back window (v souradnicich bw)
	float vp_sx, vp_sy;		// velikost viewportu do back window
	float vp_min_sx, vp_min_sy;	// minimalni velikost view portu
								// max. velikost odpovida bw_size_max

	bool minimized;				// zda bylo okno minimalizovano pomoci MinimizeAll(), tzn. ze se ma obnovit pri UnMinimizeAll()
	typeID focus_window_before_minimize_all;
	float oldTO;	// tabOrder zadniho elementu, ktery byl focusovan pred focusaci na predek okna

	bool hold;		// zda bylo okno chyceno pomoci mysi a CTRL - scrollovaci mod
	float hold_x, hold_y;	// pozice chyceni v okne
	float old_cursor_x, old_cursor_y;	// pozice kurzoru pred prechodem do scrollovaciho modu (hold), kvuli jeho navratu
	bool cursorChanged;		// zda jsem menil kurzor (jiny typ, zneviditelneni atp.)
							// kvuli navratu kurzoru do puvodniho stavu napr. pri destrukci okna, ztrate focusu atp.

	int modal;			// zda je okno modalni 

	float std_x, std_y, std_sx, std_sy;	// standartdni parametry okna (kvuli obnove po maximalizaci)
	bool maximize_before_minimize;	// zda bylo okno pred minimalizaci v maximalizovanem stavu


	float tab_min_front;	// minimalni tab_order elementu okna (v predni fronte)
	float tab_max_front;	// maximalni tab_order elementu okna
	float tab_min_back;		// minimalni tab_order elementu okna (v zadni fronte)
	float tab_max_back;		// maximalni tab_order elementu okna

	static const int numEvents;
	static const int availableEvents[];
};


//////////////////////////////////////////////////////////////////////
// CGUIDesktop
//////////////////////////////////////////////////////////////////////

class CGUIDesktop : public CGUIWindow		// hlavni okno programu
											// stara se o pridavani oken do GUI
{
public:
	CGUIDesktop();
	virtual ~CGUIDesktop() throw();

	bool GetDesktopPos(CGUIElement* elm, float &_x, float &_y);	
	// z elementu a pozice kurzovu v ramci jeho souradneho systemu (0,0 - levy horni roh elementu) vypocte pozici kurzoru v souradne soustave desktopu
	// vraci true pri spravnem vysledku, false - nelze vypocitat (element neni umisten v desktopu nebo neni videt (je odscrollovany mimo viditelnou oblast sveho okna))

protected:
	virtual int MouseRelativeMove(float dx, float dy);	// kvuli dorucovani udalosti EMouseRelMove

	static const int numEvents;
	static const int availableEvents[];
};


//////////////////////////////////////////////////////////////////////
// CGUIDataExchange
// objekt pro podporu predavani dat mezi elementy GUI a uzivatelskymi programy
// zajistuje standardni protokoly pro synchronizaci dat, kazdy element si implementuje pripadne odlisnosti
// element si muze naimplementovat i dalsi fce pro prenos dat (podle jeho charakteru), ovsem predevsim musi mit tyto standardni, kvuli centralni synchronizaci a jednotnemu pristupu
//////////////////////////////////////////////////////////////////////

class CGUIDataExchange
{
public:
	CGUIDataExchange(int _numAttributes = 1);
	~CGUIDataExchange() ;

	virtual int BindAttribute(void* _attribute, int _type, int _size, int which = 0);	// svaze datovou polozku elementu s atributem uzivatelskeho programu (daneho typu a velikosti), which urcuje se kterou datovou polozkou elementu se bude svazovat
																						// pokud je _attribute == 0 svazani s atributem se zrusi
																						// typ je podle EGUIDataTypes
																						// size urcuje zda se jedna o jednoduchy datovy typ (typu typ) size==1, nebo zda jde o pole (v tomto pripade urcuje velikost pole) size >1
																						// pokud je typ==string size urcuje velikost stringu (max. delku)
	virtual int Sync(bool out = true);			// synchronizuje hodnoty uzivatelskych atributu a dat v elementu (out urcuje smer synchronizace, tj. odkud se cte a kam se zapisuje)
												// out == true  - element zapise hodnotu stavu do svazaneho atributu
												// out == false - element zmeni hodnotu sveho stavu dle hodnoty nactene ze svazaneho atributu
	virtual int GetNumAttributes() { return numAttributes;};	// vrati kolik atributu element obsahuje (muze byt dynamicky promeny pocet)

	// tyto fce si implementuje kazdy element pro datove prenosy sam: (podle sveho charakteru)
	// int GetParametr(int which = 0);	// vrati hodnotu atributu elementu (which urcuje ktereho), typ je urcen navratovou hodnotou fce
	// void GetParametr(int which=0, int *x, int *y);	// vrati hodnotu atributu elementu pres parametry fce (zadavaji se adresy, kam ulozit hodnoty), pocet adres podle toho kolik ma atribut hodnotovych stavu (podle typu elementu - vim s cim pracuji)
	// void GetParametr(int which=0, CGUIParam **p);	// vraci hodnotu atributu pres objekt (slozita struktura atributu - uzivatel, ktery s ni chce pracovat jeji strukturu zna)
	// void SetParametr(int which=0, int value);		// jako Get, ale opacne, tj. zadanou hodnotu zapise do elementu
	
	// funkce pro prenos hodnot mezi nesvazanymi atributy
	// element muze mit pro kazdy svuj atribut pouze jednu svazanou uzivatelskou promennou
	// tyto fce umoznuji prenos hodnot pro dalsi promenne	(automaticka implementace, jako sync)
	// vraci nulu pri uspechu, jinak potrebnou velikost
	virtual int GetParametr(void *value, int type, int size, int which=0);	
	virtual int SetParametr(void *value, int type, int size, int which=0);


		
protected:
	// fce pro automatickou implementaci BindAttribute a Sync
	// element jen rekne kde, jakoho typu a velikosti data ma, CGUIDataExchange uz zaridi sam implementaci BindAttribute, GetParametr, SetParametr a Sync pro element (lze pouzit jen pro jednoduche elementy - pevne dany pocet atributu)
	void SetElementAttributes(void* *_data, int *_data_type, int *_data_size);  // na vstupu ocekavam 3 pole o velikosti numAttributes (urceno v konstruktoru) obsahujici adresy dat, jejich typ a velikost (po skonceni prace se pole rusi zde, element je jen vytvari a predava sem!!!)
	void SetElementAttributes(void* _data, int type, int size = 1);	// jednoducha verze - element obsahuje jen jeden atribut

	int numAttributes;	// pocet atributu elementu (# stavovych hodnot)
	void* *attribute;	// pole adres, kam se budou ukladat atributy elementu pri synchronizaci (adresy dat uzivatele)
	int *attrib_type;	// pole typu dat, jak je chce dostat uzivatel
	int *attrib_size;	// pole velikosti dat
						// velikost dat je kvuli polim a stringum (udava se v poctu polozek pole)
						// jednoduche datove typy maji velikost 1

	void* *data;		// pole adres, kde jsou ulozeny data elementu (pro automatickou spravu dat)
	int *data_type;		// pole typu dat, jak je chce dostat uzivatel
	int *data_size;		// pole velikosti dat
};

#endif 