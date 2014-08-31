///////////////////////////////////////////////
//
// message.h
//
// interface pro tridy zabyvajici se zpravami a udalostmi, jejich dorucovanim
// interface for the	CGUIMessage, CGUIMsgQueue - zpravy (generovane vstupy z enginu - mys, klavesnice, timer)
//						EEvents, CGUIEvent, CGUIEventServer	- udalosti (generovane elementy GUI a posilane na zadost EventProgramum)
//						CGUIEventProgram, CGUIEventSupport	- podpora pro prijem (Observer) a posilani udalosti (Subject)
//						CGUINameServer - zajistuje preklad ID na adresy
//	typy zprav - EMsgs
//	typy udalosti - EEvents
//	
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef MESSAGE_H
#define MESSAGE_H

#include "GUIConstants.h"
#include "objectList.h"

#include <assert.h>

class CGUIElement;

//////////////////////////////////////////////////////////////////////
// CGUIMessage
//////////////////////////////////////////////////////////////////////

#define DELIVER_ACCEPTED		1
#define DELIVER_NOT_ACCEPTED	0

// typy zprav:
enum EMsgs
{		MsgNone			=    0,			// no message
		MsgMouseL		=    1,			// mouse left click
		MsgMouseR		=    2,			// mouse right click
		MsgMouseM	    =    4,			// mouse middle click
		MsgMouseDL		=    8,			// mouse left double click
		MsgMouseDR		=   16,			// mouse right double click
		MsgMouseWU		=   32,			// mouse wheel up
		MsgMouseWD		=   64,			// mouse wheel down
		MsgMouseOver	=  128,			// mouse over
		MsgKeyboard		=  256,			// keyboard	(key is pressed)
		MsgKeyboardCtrl =  512,			// keyboard ctrl
		MsgKeyboardAlt  = 1024,			// keyboard alt
		MsgTimer		= 2048,			// timer impulse
		MsgCommand      = 4096,			// command
		MsgFocusChange	= 8192,			// focus is changed
		MsgKeyboardUp	= 16384,		// keyboard (key is released)
		MsgKeyboardChar	= 32768,		// keyboard char (key is pressed)
		MsgMouseRelMove	= 65536,		// relative mouse move
		MsgAll		    = 131071		// All messages
};
	// jako filtr prijmu zprav se pouziva bitova maska z typu (proto ty mocniny 2)

// typy zpravy command:
enum EComs
{
	ComCloseWnd			=    1,			// close window (okno zavri se!)
	ComCloseElement		=	 2			// znic svuj podelement
//	ComXXX				=    3,			// XXX command
};


class CGUIMessage
// objekt reprezentujici zpravu
// nese veskere parametry zpravy (veskerou informaci)
// umi zpravu clenit podle typu
{
	friend class CGUIListElement<CGUIMessage>;	// pro frontovani zprav
	CGUIList<CGUIMessage>* list;	// pro frontovani zprav (pointer na seznam, ve kterem jsem zarazen)

	static typeID lastID;			// citac pro vytvareni jednoznacne identifikace zprav
public:
	CGUIMessage(enum EMsgs _type, float _x, float _y, typeID _param, UINT _state, CGUIElement *_to=0, UINT _time=0);
	CGUIMessage(enum EMsgs _type, typeID _param, UINT _state, CGUIElement *_to=0, UINT _time=0);
	CGUIMessage(enum EMsgs _type, typeID _command, CGUIElement *_element, CGUIElement *_to=0, UINT _time=0);
	~CGUIMessage();

	void InnerCoords(float dx, float dy);	// odecte od mista zpravy (x,y) rozdilove slozky dx,dy => prevod mezi souradnema systemama
	int IsCoordMsg()	// zda se jedna o zpravu nesouci informace o poloze (mouse Click atp.)
		{return type & (MsgMouseL|MsgMouseR|MsgMouseM|MsgMouseDL|MsgMouseDR|MsgMouseWU|MsgMouseWD|MsgMouseOver);};
	int IsKeyboardMsg() {return type & (MsgKeyboard|MsgKeyboardCtrl|MsgKeyboardAlt|MsgKeyboardUp|MsgKeyboardChar);};
	int IsCommandMsg() {return type & MsgCommand;};
	int IsMouseButtonMsg() {return type & (MsgMouseL|MsgMouseR|MsgMouseM|MsgMouseDL|MsgMouseDR);};
		

	int type;	/*     1 mouse left 
					   2 mouse right 
					   4 mouse middle 
					   8 mouse left double click
					  16 mouse right double click
					  32 mouse wheel up
					  64 mouse wheel down
					 128 mouse over
					 256 keyboard
					 512 keyboard ctrl
					1024 keyboard alt
					2048 timer impulse
					4096 command
					8192 focus is changed
				*/
	float x,y;			// spot of event type 1 & 2   ( v souradnem systemu nadrazeneho objektu)
						//	type == timer:
						//						x == time delay from timer requested time

	UINT time;			// time of event // !@#$ zatim nepouzito
	UINT state;			// state of Shift, Ctrl and Alt (and mouse buttons state)
						// type == timer:	state == 0
						// type == 8192 (focus)
						//		0 focus is lost
						//		1 focus is acquired
	CGUIElement	*element;	// pointer to element, which sends the command (sender)
							// pouziva se jen pri posilani prikazu (command) 
							// zatim jen u ruseni okna pres CloseButton + closeWindow()
	typeID param;			/*
							type == 1, 2 or 4
								1 button down
								0 button up
							type == 32 || 64
								jak hodne se kolecko otocilo (v nasobcich 120ti)
							type == 128
								0 mouse out
								1 mouse over
							type == 256
								charcode
							type == 2048
								timer (timerRequest ID)
							type == 4096
								command
							type == 8192
								ID zpravy, ktera je adresovana dualnimu elementu vzhledem k focusu
								dualni ve smyslu dvojice (focusovany / defocusovany element)
						*/
	CGUIElement *to;	// pointer to element, which will receive the command (recipient)
						// !@#$ POZOR POZOR - mozny problem
						// nesmi se vyskytnout zprava urcena objektu, ktery jiz neexistuje
						// pri ruseni objektu se musi zrusit vsechny zpravy pro nej a take
						// zajistit, aby se zadne dalsi negenerovaly !!!!!
						// mazani zajisteno automaticky v destruktoru Elementu
						// zpravy se generuji jen automatickym systemem GUI - reakce na pohyb mysi, stisk klavesy atp.
						// uzivatel jiz pracuje s udalostmi, takze nehrozi, ze by zpravu vytvoril v nevhofny okamzik

	bool focus_msg;		// zda jde o zpravu, ktera je navic posilana focusovanemu elementu
						// tato zprava je tedy dorucena jak svemu adresatovi, tak i focusovanemu elementu
						// elm. ji muze vyuzit pro zmenu sveho stavu atp. (je tedy informativni, nenalezi elementu jako prikaz)
						// tento priznak se nenastavuje u standardnich zprav, ktere se posilaji jen fokusovanym elementum (keyboard, command, ...)
						//			( takove zpravy asi neexistuji, nebot vsechno lze poslat komukoliv, tedy i napr. klavesu nefokusovanemu prvku)
						// element tuto zpravu dostane JEN tehdy, pokud by ji nedostal standardni cestou
						//		=> nelze dostat tutez zpravu 2x (jednou focus a jednou normal)
		// pokud ex. zprava s nastavenym priznakem "focus_msg" z toho vyplyva:
		//		- ex. stejna zprava se stejnym ID ve fronte zprav urcena jinemu elementu (urcenemu nejpravdepodobneji polohou mysi)
		//		- do fronty zprav se posila nejdrive "focus_msg" a teprve potom "normal" teto zpravy
		// element, ktery obdrzi focus verzi zpravy muze z fronty zprav odebrat jeji normal verzi podle ID, pokud nechce aby byla dorucena
	
	typeID mID;	// jednoznacna identifikace zpravy
	// 2 zpravy maji stejne ID tehdy a jen tehde pokud jedna je focus_msg a druha normalni zprava tehoz obsahu

	bool operator==(typeID _ID) { return (mID==_ID); }
	bool operator==(CGUIElement* _to) { return (to==_to); }
};


class CGUIMsgQueue
// fronta zprav
// zajistuje ulozeni dosud nedorucenych zprav, jejich postupne rozesilani adresatum
// obsahuje 2 fronty - normalni a prioritni (pri dorucovani se nejdrive berou vsechny zpravy z prioritni a potom teprve z normalni)
// umoznuje smazat zpravu z fronty podle adresata nebo podle ID zpravy
{
public:
	CGUIMsgQueue() {};
	~CGUIMsgQueue() {};

	CGUIMessage *GetWaitingMsgs();							// vrati prvni cekajici zpravu (odstrani ji z fronty), nejdrive z prioritni fronty, pokud v ni nic neni vynda zpravu z normalni fronty
	void AddMsgToQueue(CGUIMessage *msg, CGUIElement *to);	// prida zpravu na konec fronty ("to" je adresat, muze byt 0, potom se posle nejdrive desktopu, ktery ji preposle podle systemu dorucovani zprav)
	void AddPriorityMsgToQueue(CGUIMessage *msg, CGUIElement *to);	// prida zpravu na konec prioritni fronty ("to" je adresat, muze byt 0, potom se posle nejdrive desktopu, ktery ji preposle podle systemu dorucovani zprav)

	CGUIMessage *RemoveMsgFromQueue(CGUIElement *to);	// odstrani prvni nalezenou zpravu urcenou 
														// elementu (to) z fronty a vrati ji 
	void DeleteAllMsgFromQueue(CGUIElement *to);// zrusi vsechny zpravy urcene elementu (to)
	void DeleteMsgQueue();

	CGUIMessage *GetMsg(typeID mID);		// vrati zpravu z fronty, podle jejiho ID
	CGUIMessage *RemoveMsg(typeID mID);	// vrati zpravu z fronty, podle jejiho ID + odstrani z fronty
	void DeleteMsg(typeID mID);			// smaze zpravu ve fronte podle ID

protected:
	CGUIList<CGUIMessage> message_queue;
	CGUIList<CGUIMessage> priority_message_queue;	// fronta prioritnich zprav
};



// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
//		Sprava udalosti - Events
// --------------------------------------------------------------------------------------
//
//		CGUIEventProgram	- objekt, ktery chce pracovat s udalostmi 
//								(skoro vsechny elementy GUI, uzivatelsky program prijimajici udalosti od GUI)
//							- Observer
//							- obecny predek, konkretni cinnost se zaridi pretizenim fce EventHandler
//							- objekt lze pojmenovat a potom pri prichodu udalosti porovnavat odesilatele na jmeno
//
//		CGUIEventSupport	- objekt, ktery je schopen generovat udalosti (Subject)
//							- umi je i prijimat (je to potomek CGUIEventProgram)
//
//		Event		- objekt reprezentujici udalost (parametry udalosti - EventID, ElementID, params, ...)
//					- obsahuje vzdy typ udalosti (eventID), adresata (recipient), odesilate (sender) a pripadne dalsi parametry udalosti (dle tabulky viz. dale)
//
//		NameServer	- objekt realizujici preklad ElementID, ElementName  -> ElementAddr		
//		EventServer	- objekt zajistujici dorucovani udalosti
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

// typy udalosti:
enum EEvents{
	ENoEvent = 0,			// zadna udalost (chyba)
		// pInt = 0, pId = 0, pAddr = 0, pFloat = 0, pInt2 = 0, pID2 = 0, pFloat2 = 0
	EEvent,					// obecna udalost pro vlastni pouziti (parametry si definuje uzivatel sam)
	ECommand,				// obecny prikaz 
		// konkretni parametry (protokol) si definuji elementy mezi sebou, dle vlastni potreby
		// tato udalost se neobjednava, ale posila primo pres eventServer->SendEvent(...)
	ESizeChanged,			// Resize
	EPositionChanged,		// Move
	EEngineStateChanged,	// AddToEngine, RemoveFromEngine
	EVisibilityChanged,		// BringToTop, SetVisible
	EMouseOver,
	EMouseButton,
		// pInt = 0/1/2 (left/right/middle button), pID = 0/1 (pusteni/stisknuti tlacitka) 
	EMouseWheel,
		// pInt = dz (kolikrat se kolecko otocilo, dopredu+, dozadu-)
	EClicked,
	EUp,					// ButtonUp
	EDown,					// ButtonDown
	EFocusChanged,
		// pInt = focus (1 = nabyti focusu, 0 = ztrata focusu)
	EDeleted,
		// pAddr = adresa smazaneho elementu (nesmi se jiz pristupovat (objekt je jiz smazan), jen pro porovnani)
	EOn,					// ContextHelp, ContextMenu - on
	EOff,					// ContextHelp, ContextMenu - off
	EStateChanged,			// zmena stavu elementu
	EGraphicsChanged,		// zmena vzhledu (grafiky) el.
	EValueChanged,			// zmena hodnoty atributu el.
	EAddElement,			// novy element pridan do okna/multiwidgetu
		// pID = ID pridaneho elementu (pokud ID ma, jinak 0), pAddr = adresa pridaneho elementu
	EDeleteAllElements,
	ESetAllElementsVisible,
	EUpdateScrollbars,		// zmenilo se nastaveni okna / scrollbaru => je potreba updatovat scrollbary (vzhled)
	EOk,					// povtzeni vysledku / akce (napr. po napsani hodnoty v editboxu)
	ECancel,				// zruseni vysledku / akce
	EMinimize,				// minimalizace okna
		// pInt == 0/1 (1=mimimalizece, 0=obnoveni po minimalizaci (opak minimalizace))
	EMaximize,				// maximalizace okna
		// pInt == 0/1 (1=maximalizace, 0=obnoveni po maximalizace (opak maximalizace))
	ENormalize,				// normalizace okna
	EClose,					// zavreni okna
		// pAddr = adresa zaviraneho okna
	EKillMe,				// zadost o zabiti (delete elementu rodicem)
	EKeyboard,				// vstup z klavesnice, stisknuti klavesy (WM_KEYDOWN)
		// pInt = wParam (charcode), pID = lParam (keydata), pFloat = 0/1 (1 - podelement okna ma exkluzivni pristup ke klavesnici, muzes zpracovat jen klavesove prikazy nejvyssi urovne, jako zavri okno atp.)
		// pInt2 = key_ctrl, pID2 = key_alt, pFloat2 = key_shift (0/1 - 1==stisknuto)
	EKeyboardChar,			// vstup z klavesnice, prelozeny na pismeno, podle nastaveni klavesnice (WM_CHAR)
		// pInt = wParam (charcode), pID = lParam (keydata), pFloat = 0/1 (1 - podelement okna ma exkluzivni pristup ke klavesnici, muzes zpracovat jen klavesove prikazy nejvyssi urovne, jako zavri okno atp.)
		// pInt2 = key_ctrl, pID2 = key_alt, pFloat2 = key_shift (0/1 - 1==stisknuto)
	EKeyboardUp,			// vstup z klavesnice, pusteni klavesy (WM_KEYUP)
		// pInt = wParam (charcode), pID = lParam (keydata), pFloat = 0/1 (1 - podelement okna ma exkluzivni pristup ke klavesnici, muzes zpracovat jen klavesove prikazy nejvyssi urovne, jako zavri okno atp.)
		// pInt2 = key_ctrl, pID2 = key_alt, pFloat2 = key_shift (0/1 - 1==stisknuto)
	EToolBar,				// pouziti nastroje na toolbaru (stisknuti tlacitka)
		// pID = toolID, pInt = pos_h, pInt2 = pos_v, pFloat = 0/1 (0 - tool zrusen, 1 - tool vybran, podrobnosti viz. CGUIToolBar::ToolBarHandler)
	EMenu,					// stisknuti polozky menu
		// pID = menuItemID, pInt = index polozky, pID2 = activatorID
	EMessage,
		// pInt = Msg.type, pFloat = Msg.x, pFloat2 = Msg.y, pInt2 = Msg.state, pID = Msg.param, pAddr = Msg.element (sender)
	ETree,					// stisknuti polozky menu
		// pID = treeItemID, pInt = index polozky, pInt2 = 0/1/2/3 (Clicked/MouseButtonLeft/MouseButtonRightKeyboard focus (najeto na polozku klavesnici))
	ETreeItemUpdate,		// manualni update tree po stisknuti rozbalovace (udalost od treeItem)
		// pInt = index polozky
	ETreeUpdate,			// manualni update tree po stisknuti rozbalovace (udalost od tree)
		// pID = treeItemID, pInt = index polozky
	ETreeItemRoller,		// stisknuti rozbalovace (udalost od treeItem) (nemusi byt zapnut manualni update)
		// pInt = index polozky, pInt2 = aktualni "state" polozky
	EChangeTABElement,		// prikaze oknu, aby posunulo focus na jiny TAB element
		// pInt = -1/1 (predchozi/nasledujici)
	EMouseRelMove,			// ralativni posun mysi
		// pFloat = dx, pFloat2 = dy 
	EBadInput,				// generuje se, kdyz uzivatel zada neplatnou hodnotu v okne
		// pID = elemID (id spatne zadanyho elementu)
	EList					// stisknuti polozky seznamu
		// pID = listItemID
};

class CGUIEvent
{
	friend class CGUIListElement<CGUIEvent>;
	CGUIList<CGUIEvent> *list;		// pointer na seznam, ve kterem je obsazena
public:
	CGUIEvent() {list = 0; eventID=0; recipient=0; sender=0; pInt=0; pID=0; pAddr=0; pFloat=0; pInt2=0; pID2=0; pFloat2=0;};
	~CGUIEvent() {if(list) assert(false);};

	bool operator==(typeID eID) { return (recipient == eID); };	// pro vyhledavani podle adresata

	int eventID;		// ID udalosti (mousebuttonclicked, valuechanged, ...)
	typeID recipient;	// adresat (ID)
	typeID sender;		// odesilatel (ID)

	int pInt;		// volitelne parametry dle typu udalosti
	typeID pID;		// volitelne parametry dle typu udalosti
	void*	pAddr;	// volitelne parametry dle typu udalosti
	float pFloat;	// volitelne parametry dle typu udalosti
	int pInt2;		// volitelne parametry dle typu udalosti
	typeID pID2;	// volitelne parametry dle typu udalosti
	float pFloat2;	// volitelne parametry dle typu udalosti
};



class CGUIEventProgram
{
	friend class CGUIListElement<CGUIEventProgram>;
	friend class CGUINameServer;
	
	static typeID lastID;			// citac pro vytvareni jednoznacne identifikace objektu (objID)

public:
	CGUIEventProgram();		// vytvori jendoznacne ID objektu, zaregistruje ho u NameServru
	virtual ~CGUIEventProgram();	// odregistruje ID ruseneho objektu u NameServru

	void SetName(char *name);				// pojmenuje objekt, zaregistruje jmeno u NameServru (zrusi pripadnou registraci stareho jmena)
	char* GetName() { return objName; };	// vrati jmeno objektu
	typeID GetID() { return objID; };		// vrati jednoznacne ID objektu

	bool operator==(typeID _ID) { return (objID == _ID); };
	bool operator==(char* name) { 	return (strcmp(objName,name) ? false : true); };

	virtual void EventHandler(CGUIEvent *event) {if(event) delete event;};	// fce pro pretizeni uzivatelem, zde si definuje reakce na prichozi udalosti

private:
	typeID	objID;	// ID objektu - jednoznacna identifikace pro dorucovani udalosti, ...
	char*	objName;//	jmeno objektu - objekt se muze pojmenovat + dorucovani na jmeno (volitelne)
	
	CGUIList<CGUIEventProgram> *list;	// pointer na seznam, ve kterem je obsazen
										// nepouziva se, jen kvuli nutnosti, aby se mohl pouzit seznam
	CGUIList<CGUIEventProgram> *nameHT;		// pointer na hasovaci tabulku dle jmena objektu (objName)
	CGUIList<CGUIEventProgram> *idHT;		// pointer na hasovaci tabulku dle ID objektu (objID)
};


class CGUINameServer
{
	friend class CGUIEventProgram;
	friend class CGUI;
public:
	CGUIEventProgram* ResolveID(typeID objID);		// prelozi objID na &objektu pres HT
	CGUIEventProgram* ResolveName(char *objName);	// prelozi objName na &objektu pres HT
	CGUIEventProgram* operator[](typeID objID);

private:
	CGUINameServer();
	~CGUINameServer();

	int	Register(CGUIEventProgram* program);	// registruje objekt (zaradi ho do vsech hasovacich tabulek - id, jmeno (jen kdyz ma jmeno nastaveno))
												// vraci 0 - OK, 1 - chyba
	
	// odregistruje objekt ze vsech hasovacich tabulek (vynda ho ze seznamu, v zadnem pripade objekt nerusi)
	// vraci 0 - OK, 1 - chyba
	int	UnRegister(CGUIEventProgram* program);	
	int	UnRegister(typeID objID);
	int	UnRegister(char *objName);

	int	UnRegisterName(char *objName);	// odregistruje objekt jen z hasovaci tabulky pro jmena, v ostatnich ho necha
	int	RegisterName(CGUIEventProgram* program);	// registruje objekt jen u hasovaci tabulky pro jmena, ostatnich necha beze zmeny
	// vraci 0  - pri uspesnem provedeni registracnich a odregistracnich fcich, 1 - pri neuspechu

	int hashID(typeID ID);		// hashovaci fce pro ID objektu
	int hashName(char* name);	// hashovaci fce pro jmena objektu

	CGUIList<CGUIEventProgram> **nameHT;		//   hasovaci tabulka dle jmena objektu (objName)
	CGUIList<CGUIEventProgram> **idHT;			//   hasovaci tabulka dle ID objektu (objID)
};


// objekt nesouci informace o obsluznem objektu CGUIEventProgram, kteremu se ma dorucovat udalost
// z techto objektu je tvoren seznam obsluznych objektu u kazde objednane udalosti v elementu
class CGUIEventProgramList
{
	friend class CGUIListElement<CGUIEventProgramList>;
	CGUIList<CGUIEventProgramList>* list;
public:
	CGUIEventProgramList(typeID _objID = 0, int _eventID = 0) { objID=_objID; eventID=_eventID;list = 0;};
	~CGUIEventProgramList() {if(list) assert(false);};

	bool operator==(typeID _objID) { return (objID == _objID); };

	typeID  objID;		// ID objektu, ktery si objednal posilani udalosti
	int eventID;		// ID udalosti, kterou dostane (muze byt prejmenovana)
};

// objekt obsahujici seznam obsluznych programu pro danou udalost (seznam objektu, ktere si objednali danou udalost)
class CGUIEventList
{
	friend class CGUIListElement<CGUIEventList>;
	CGUIList<CGUIEventList>* list;
public:
	CGUIEventList(int _eventID = 0) {eventID = _eventID; list = 0;};
	~CGUIEventList() {if(list) assert(false);};

	bool operator==(int _eventID) { return (eventID == _eventID); };

	int eventID;		// ID udalosti, pro kterou mam seznam obsluznych objektu
	CGUIList<CGUIEventProgramList> eventProgs;	// seznam obsluznych objektu, ktere si objednaly tuto udalost posilat
};


// objekt realizujici podporu pro generovani a posilani udalosti
// je zakladem kazdeho elementu, ktery je schopen generovat udalosti
class CGUIEventSupport : public CGUIEventProgram
{
public:
	CGUIEventSupport(const int *_availableEvents = 0, int _numEvents = 0) : availableEvents(_availableEvents),numEvents(_numEvents) {};
		// v konstruktoru se nastavuje pointer na pole se seznamem udalosti, ktere je schopen objekt generovat a jejich pocet
		// pokud nelze nastavit v konstruktoru (jedna se jiz o potomka jineho objektu), lze pole udalosti nastavit fci - SetAvailableEvents

	int SetMask(typeID eventProgramID, int mask[], int num);		// nastavi masku udalosti, ktere chce posilat ep (mask je pole ID udalosti, num je velikost pole)
																	// budou posilany presne ty udalosti, ktere jsou v masce (pokud byly objednany nejake jine predtim, ty budou zruseny)
	int RenameEvent(typeID eventProgramID, int stdEventID, int newEventID);	// prejmenuje generovanou udalost na nove ID (element bude dostavat misto std. ID udalosti jeho vymyslene cislo)
																			// pokud udalost jeste neni registrovana, zaregistruje ji rovnou pod novym jmenem (ID)
	int SetMaskName(typeID eventProgramID, int mask[], int nameMask[], int num);	// jako SetMask + vsechny udalosti rovnou prejmenuje na nove jmena podle nameMask
	int AcceptEvent(typeID eventProgramID, int event);		// ep chce posilat udalost event
	int RejectEvent(typeID eventProgramID, int event);		// ep nechce posilat udalost event
	int RejectNameEvent(typeID eventProgramID, int event);	// ep nechce posilat udalost event, kterou si prejmenoval (event neni std. cislo udalosti, ale uzivatelske cislo)
	int AcceptAllEvents(typeID eventProgramID);				// ep chce posilat vsechny udalosti, ktere element generuje
	int RejectAllEvents(typeID eventProgramID);				// ep nechce posilat zadnou udalost od elementu
	// nastavovaci fce pro udalosti vraci 0 - pri uspechu, 1 - pri neuspechu
	bool IsEventOrder(typeID eventProgramID, int event);	// vrati zda je udalost objednana ci nikoliv

	void GetAvailableEvents(const int **events, int *num)		// vrati pole udalosti, ktere element muze generovat + jeho velikost
			{ *events = availableEvents; *num = numEvents;};	// pole implementovano v kazdem el. jako staticky atribut

	bool IsPossibleEvent(int eventID);	// vrati zda el. je schopen generovat danou udalost															
	
protected:
	int numEvents;				// pocet moznych udalosti, ktere element generuje
	const int *availableEvents;	// ukazatel na pole udalosti, ktere je element schopen generovat

	void EventArise(int eventID, int pInt=0, typeID pID=0, void* pAddr=0, float pFloat = 0, int pInt2 = 0, typeID pID2 = 0, float pFloat2 = 0);		// podle seznamu posle udalosti nastavenym eventprogramum - vola el. pri akci, ktera vyvolava udalost
	void EventArise(CGUIEvent* event);	// podle seznamu posle udalosti nastavenym eventprogramum - vola el. pri akci, ktera vyvolava udalost

	void SetAvailableEvents(const int *events, int num) // nastaveni statickych promennych elementu do obsluzneho objektu, aby mohl zjistovat potrebne informace
		{ availableEvents = events; numEvents = num;};	// normalne se nastavuje v konstruktoru, pokud to nelze,  pouzije se tato fce

	CGUIList<CGUIEventList> eventList;	// seznam observeru (eventprogramu)
};

class CGUIEventServer
{
public:
	int DeliverEvent(bool allEvents=false);		// doruci prvni udalost ve fronte jejimu adresatovi
								// pokud zadna udalost ve fronte neni vraci 1, jinak 0
								// doruceni = zavolani metody EventHandler u objektu EventProgram
								// pokud je nastaven parametr allEvents doruci vsechny udalosti ve fronte
	CGUIEvent *GetWaitingEvent();						// vrati prvni cekajici udalost (odstrani ji z fronty)
	void AddEventToQueue(CGUIEvent *event);				// prida udalost na konec fronty
	void AddPriorityEventToQueue(CGUIEvent *event);		// prida udalost na zacatek fronty

	CGUIEvent *RemoveEventFromQueue(typeID ID);	// odstrani prvni nalezenou udalost urcenou 
														// eventProgramu (ID) z fronty a vrati ji 
	void DeleteAllEventsFromQueue(typeID ID);	// zrusi vsechny udalosti urcene eventProgramu (ID)
	void DeleteEventsQueue();	// smaze celou frontu udalosti

	void SendEvent(int eventID, typeID sender, typeID recipient, int pInt=0, typeID pID=0, void* pAddr=0, float pFloat=0, int pInt2=0, typeID pID2=0, float pFloat2=0);
		// posle udalost podle zadanych parametru
		// vytvori novou udalost, vyplni jeji parametry a zaradi do fronty udalosti (na konec fronty)
		// vhodne pro posilani neobjednanych udalosti (uzivatelske udalosti EEvent, asynchronni prikazy atp.)

	void SendPriorityEvent(int eventID, typeID sender, typeID recipient, int pInt=0, typeID pID=0, void* pAddr=0, float pFloat=0, int pInt2=0, typeID pID2=0, float pFloat2=0);
		// posle udalost podle zadanych parametru
		// vytvori novou udalost, vyplni jeji parametry a zaradi do prioritni fronty udalosti (na zacatek fronty)
		// vhodne pro posilani neobjednanych udalosti (uzivatelske udalosti EEvent, asynchronni prikazy atp.)

protected:
	CGUIList<CGUIEvent> eventsQueue;	// fronta udalosti
};

template <class T>	T* GetObjFromID(typeID objID)		// ziska z ID pres NameServer adresu objektu, tu pak zkusi dynamicky pretypovat na pozadovany typ a vrati ji
														// vraci 0 pri neuspechu
	{
		return dynamic_cast<T*>(nameServer->ResolveID(objID));
	}


template <class T>  T* GetObjFromID(typeID objID,T**)
{
    return dynamic_cast<T*>(nameServer->ResolveID(objID));
	//return dynamic_cast<T*>((*nameServer)[ID]);
}

/* dabelske pretypovaci makro
    pouziti:

        1) NakyTyp* ID(test,nakyID); 
			//deklaruje promennou test, typu *NakyTyp a priradi do ni objekt s ID=nakyID (pokud nelze, tak priradi NULL)

        2) //nekde uz je deklarovana promenna NakyTyp* test, pak
           ID(test,nakyID); 
				//priradi do test objekt s ID=nakyID 
*/

#define ID(name,id) name = GetObjFromID(id,&name)

#endif 
