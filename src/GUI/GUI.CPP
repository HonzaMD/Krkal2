///////////////////////////////////////////////
//
// GUI.cpp
//
// implementation for the CGUI class
// hlavni objekt GUI, globalni servisni objekty GUI
// objekty pro kurzor, styly, timer, trash (garbage collector pro elementy GUI), fonty
// hlavni ovladac udalosti - CGUIMainHandler (zarizuje zmeny globalnich objektu GUI, napr. Desktopu atp.)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <exception>

#include "input.h"
#include "dx.h"

#include "fs.h"
#include "types.h"

#include "GUI.h"
#include "window.h"
#include "widgets.h"

#include "testy.h"
#include "desktop.h"
#include "editor.h"

#include "SEdDesktop.h"
#include "MainMenuDesktop.h"
#include "ProfileDesktop.h"

#include "krkal.h"
#include "genmain.h"
#include "kernel.h"
#include "ident.h"

#include "mmdesktop.h"

CGUIDesktop *desktop = 0;		// desktop GUI (hlavni okno aplikace)
CGUIMsgQueue *msgQueue = 0;		// hlavni masazni fronta
CGUIStyleSet *styleSet = 0;		// globalni objekt pro spravu stylu
CGUITimer *timerGUI = 0;		// timer pro elementy GUI (prijima pozadavky na zavolani + posila casove impulsy elementum)
CGUI *mainGUI = 0;				// hlavni GUI objekt - interface mezi GUI a enginem krkala
CGUITrash *trash = 0;			// odpadkovy kos na objekty, ktere vznikly a jeste nebyly nikam prirazeny
CGUINameServer *nameServer = 0;	// objekt prekladajici ID, jmena, adresy objektu na adresy objektu (pro dorucovani udalosti a zprav)
CGUIEventServer *eventServer = 0; // objekt zarizujici doruceni udalosti
MTRand mtr;

//////////////////////////////////////////////////////////////////////
// CGUI
//////////////////////////////////////////////////////////////////////


CGUI::CGUI(CBltWindow *_rootwnd, CInput *_input)
{
	cursor = 0;
	handler = 0;
	rootwnd=_rootwnd;
	mouseOverBlock=false;

#ifdef EDITOR_GAME_MOD
	editorGameMod = 1;
#else
	editorGameMod = 0;
#endif

	try
	{
		input=_input;

		mainGUI = this;
		dc = CreateCompatibleDC(0);
		trash = new CGUITrash();
		nameServer = new CGUINameServer();
		msgQueue = new CGUIMsgQueue();
		eventServer = new CGUIEventServer();
		styleSet = new CGUIStyleSet();
		timerGUI = new CGUITimer();
		handler = new CGUIMainHandler(4);

		RUN_EDITOR_SCRIPT=0;
		RUN_EDITOR_LEVEL=0;

		LoadGUIData();		// nalouduje vsechny inicializacni data (styly, fonty, ...)

		CreateCursor();		// vytvori vsechny std. kurzory

		handler->StartProfile();	// vytvori desktop pro vyber profilu

		// Testovaci desktop:
		//desktop = new CGUITestDesktop();
		//desktop->AddToEngine(_rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
		//desktop->InitDefaultIcons();

		cursor->BringToTop();

		last_keyflags=0;
		key_ctrl = key_alt = key_shift = false;
	}
	catch(CExc c)
	{
		// v konstruktoru se provadi jen nutne veci pro fungovani GUI, musi se provest vsechny uspesne
		// pokud neco neni v poradku => ukoncit GUI

		SAFE_DELETE(cursor);
		SAFE_DELETE(desktop);
		SAFE_DELETE(handler);
		SAFE_DELETE(msgQueue);
		SAFE_DELETE(eventServer);
		SAFE_DELETE(styleSet);
		SAFE_DELETE(timerGUI);
		SAFE_DELETE(nameServer);
		SAFE_DELETE(trash);
		mainGUI = 0;
		LogError(c);
		throw; 
	}
}

CGUI::~CGUI()
{
	SAFE_DELETE(editor);
	delete desktop;
	mainGUI = 0;

	delete cursor;
	delete trash;

	delete handler;
	delete msgQueue;
	delete eventServer;
	delete styleSet;
	delete timerGUI;
	delete nameServer;
	DeleteDC(dc);

	SAFE_DELETE_ARRAY(RUN_EDITOR_SCRIPT);
	SAFE_DELETE_ARRAY(RUN_EDITOR_LEVEL);
}

void CGUI::SendChar(UINT charcode, UINT keydata )
{
	//GetEvent(0);

	CGUIMessage *msg = new CGUIMessage(MsgKeyboardChar, 0, 0, charcode, keydata);	// vygeneruje zpravu dle vstupu

	GetEvent(msg);	// doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::SendKey(UINT charcode, UINT keydata )
{
	//GetEvent(0);

	// nastaveni promenych shift, ctrl, alt podle stavu klaves
	// klavesy shift, ctrl a alt se posilaji jen jednou pri stisku a jednou pri vymacknuti (zarizuje GUI viz. dale)
	// ostatni se posilaji periodicky podle nastaveneho repeteru windows
	if(charcode == VK_SHIFT)
	{
		if(key_shift)
			return;	
		key_shift = true;
	}
	else if(charcode == VK_MENU)
	{
		if(key_alt)
			return;
		key_alt = true;
	}
	else if(charcode == VK_CONTROL)
	{
		if(key_ctrl)
			return;
		key_ctrl = true;
	}

	CGUIMessage *msg = new CGUIMessage(MsgKeyboard, 0, 0, charcode, keydata); // vygeneruje zpravu dle vstupu

	GetEvent(msg);	// doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::SendKeyUp(UINT charcode, UINT keydata )
{
	//GetEvent(0);

	if(charcode == VK_SHIFT)
		key_shift = false;
	if(charcode == VK_MENU)
		key_alt = false;
	if(charcode == VK_CONTROL)
		key_ctrl = false;

	CGUIMessage *msg = new CGUIMessage(MsgKeyboardUp, 0, 0, charcode, keydata); // vygeneruje zpravu dle vstupu
																
	GetEvent(msg); // doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::MouseMove(int x, int y, UINT keyflags)
{
	//GetEvent(0);

	if(cursor)
		cursor->Move((float)x,(float)y);	// zmeni polohu kurzoru podle nove polohy mysi

	last_keyflags = keyflags;

	if(!mouseOverBlock)
	{
		CGUIMessage *msg = new CGUIMessage(MsgMouseOver,(float)x, (float)y, 1, keyflags);	 // vygeneruje zpravu dle vstupu
		GetEvent(msg);	// doruci zpravu dle systemu dorucovani zprav
	}

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::MouseRelativeMove(int dx, int dy)
{
	//GetEvent(0);

	CGUIMessage *msg = new CGUIMessage(MsgMouseRelMove,(float)dx, (float)dy, 0, 0);	// vygeneruje zpravu dle vstupu
																
	GetEvent(msg);	// doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::MouseBut(UINT msg, int x, int y, UINT keyflags)
{
	//GetEvent(0);

	UINT param;
	EMsgs type;
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		type = MsgMouseL;
		param = 1;
		break;
	case WM_LBUTTONUP:
		type = MsgMouseL;
		param = 0;
		break;
	case WM_RBUTTONDOWN:
		type = MsgMouseR;
		param = 1;
		break;
	case WM_RBUTTONUP:
		type = MsgMouseR;
		param = 0;
		break;
	case WM_MBUTTONDOWN:
		type = MsgMouseM;
		param = 1;
		break;
	case WM_MBUTTONUP:
		type = MsgMouseM;
		param = 0;
		break;
	default:
		return;	// neznam typ, tedy nezpracovavam a vracim rovnou rizeni enginu
	}
	last_keyflags = keyflags;

	CGUIMessage *m = new CGUIMessage(type,(float)x, (float)y, param, keyflags);	// vygeneruje zpravu dle vstupu
															
	GetEvent(m);	// doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::MouseWheel(int dz, int x, int y, UINT keyflags)
{
	//GetEvent(0);

	last_keyflags = keyflags;
	CGUIMessage *msg = new CGUIMessage((dz == 120) ? MsgMouseWD : MsgMouseWU, (float)x, (float)y, dz, keyflags); // vygeneruje zpravu dle vstupu

	GetEvent(msg);	// doruci zpravu dle systemu dorucovani zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::Timer(float time)
{
//	if(editor)
//		DebugMessage(6,"EM:%d,R:%d",editor->GetEditMod(),editor->GetReplaceMod());

	timerGUI->TimerImpulse(time);	// vygeneruje zpravy dle ulozenych pozadavku a aktualniho casu, ulozi je do fronty zprav

	GetEvent(0);	// prechod do masazni pumpy na zpracovani fronty zprav

	eventServer->DeliverEvent(true);	// doruci vsechny udalosti, ktere vznikly ze vstupni zpravy
}

void CGUI::SendCursorPos()
{
	if(!cursor || mouseOverBlock)
		return;

	float cx, cy;
	cursor->GetPos(cx,cy);
	CGUIMessage *msg = new CGUIMessage(MsgMouseOver,cx,cy, 1, last_keyflags);	// vygeneruje zpravu, jako pri zmene polohy mysi
	msgQueue->AddMsgToQueue(msg,0);	// prida zpravu do fronty zprav, NEDORUCUJE ihned, ale az se zavola GetEvent()
			// pokus se vola v prubehu dorucovani zprav, posle se jakmile na ni prijde rada
}

void CGUI::CursorInOut(bool in)
{
	if(in)	// najeti kurzoru do krkalskeho okna
	{
		if(cursor && (!MMDesktop || MMDesktop->ShouldShowCursor()))
			cursor->SetVisible(1);

	}
	else	// vyjeti kurzoru z krkalskeho okna
	{
		if(cursor)
			cursor->SetVisible(0);

		CEDEditorDesktop* ed = dynamic_cast<CEDEditorDesktop*>(desktop);
		if(ed && timerGUI->ContainRequest(ed))
			timerGUI->DeleteAllRequests(ed);
	}
}

void CGUI::GetEvent(CGUIMessage *msg)
{
	CGUIMessage *queue_msg;
	CGUIElement *to;
	msgQueue->AddMsgToQueue(msg,0);
	try
	{
		while(queue_msg=msgQueue->GetWaitingMsgs())
		{
			if((to=queue_msg->to))
			{
				to->GetMsg(queue_msg);	// zprava ma adresata, posli mu ji
			}else{
				if(desktop)
					desktop->GetMsg(queue_msg); // zprava nema adresata, zjisti ho pres 
												// hierarchicky system oken a elementu
												// posli zpravu desktopu (koren hierarchie)
				else
					throw CExc(eGUI, E_INTERNAL,"CGUI::GetEvent> Fatal error - DESKTOP not exists");
			}
		}
	}
	catch(CExc c)
	{
		delete queue_msg;	// pokud prisla vyjimka, masaz, pri jejimz zpracovani vznikla, nebyla smazana => zmaz ji
							// (pote co se uspesne smaze masaz uz nemuze byt vyjimka generovana)

		LogError(c);

		// vypis informace o chybe na obrazovce jako DebugMessage (prozatimni reseni)
		__time64_t ltime;
		_time64( &ltime );
		DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);

		if(c.errnum != eGUI || (c.errnum == eGUI && c.param == E_INTERNAL))
			throw;	// vyjimky, ktere nejsou od GUI nebo jsou fatalniho charakteru vyhazuji dale do enginu => konec GUI
					// vyjimky typu E_INTERNAL nejsou pro chod GUI fatalne kriticke muze pokracovat dale (nyni se ukoncuje z ladicich duvodu)
	}
	catch (CKernelPanic){
		// vyjimka vznikla pri praci kernelu
		delete queue_msg;	
		
		if(editor)
			editor->KernelShutDown();
		else
		{
			KerErrorFiles->ShowAllLogs(0,40,640,200);
			SAFE_DELETE(KerMain);
		}
	}
}

void CGUI::LogError(CExc c)
{
		FILE *f;
		__time64_t ltime;

		FS->ChangeDir("$DATA$");
		f = fopen("gui_error.log","a");
		_time64( &ltime );
		fprintf(f,"%s\tEtype: %2d, Eparam: %2d, Emsg: %s\n\n", _ctime64( &ltime ), c.errnum, c.param, c.errstr);
		fclose(f);
}



void CGUI::SetEditorLevelInit(char* level)
{
	SAFE_DELETE_ARRAY(RUN_EDITOR_SCRIPT);
	SAFE_DELETE_ARRAY(RUN_EDITOR_LEVEL);

	RUN_EDITOR_LEVEL=newstrdup(level);
}

void CGUI::SetEditorScriptInit(char* script)
{
	SAFE_DELETE_ARRAY(RUN_EDITOR_SCRIPT);
	SAFE_DELETE_ARRAY(RUN_EDITOR_LEVEL);

	RUN_EDITOR_SCRIPT=newstrdup(script);
}

void CGUI::StartProfile()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),5);
}

void CGUI::StartOldMainMenu()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),3);
}

void CGUI::StartLevelEditor()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),2);
}

void CGUI::StartScriptEditor()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),1);
}

void CGUI::StartMainMenu()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),6);
}

void CGUI::Exit()
{
	eventServer->SendPriorityEvent(ECommand,0,handler->GetID(),4);
}

//////////////////////////////////////////////////////////////////////
// CGUICursor
//////////////////////////////////////////////////////////////////////

CGUICursor::CGUICursor(CBltWindow *_rootwnd)
{
	rootwnd = _rootwnd;
	actual = 0;
	actual_index=0;
	for(int i=0; i<MAX_CURSORS;i++) cursorImage[i] = 0;
	cursor_x=0;
	cursor_y=0;
}


CGUICursor::~CGUICursor()
{
	Set(-1);
	for(int i=0; i<MAX_CURSORS;i++) 
		if(cursorImage[i] != 0)
			delete cursorImage[i]; 
}


void CGUICursor::Load(int index, char *textureName, int x, int y)
{
	bool rebuildCursor=false;

	CTexture *texture = new CTexture;
	texture->CreateFromFile(textureName);

	if(cursorImage[index])
	{
		if(actual_index==index)
		{
			rebuildCursor = true;
			if(actual)
				rootwnd->RemoveChild(actual);
			actual=0;
		}
		delete cursorImage[index];
	}

	cursorImage[index] = new CGUICursorImage(texture,x,y);
	cursorImage[index]->cursor_hs_x = x;
	cursorImage[index]->cursor_hs_y = y;
	texture->Release();

	if(rebuildCursor)
		Set(index,true);
}

void CGUICursor::Load(int index, CTexture* texture, int x, int y)
{
	bool rebuildCursor=false;	

	if(cursorImage[index])
	{
		if(actual_index==index)
		{
			rebuildCursor = true;
			if(actual)
				rootwnd->RemoveChild(actual);
			actual=0;
		}
		delete cursorImage[index];
	}

	cursorImage[index] = new CGUICursorImage(texture,x,y);
	cursorImage[index]->cursor_hs_x = x;
	cursorImage[index]->cursor_hs_y = y;

	if(rebuildCursor)
		Set(index,true);
}


int CGUICursor::Set(int index, bool force)
{
	if(actual_index == index && !force)
		return 0;

	if(index >= MAX_CURSORS || (index>=0 && cursorImage[index] == 0))	return 1;

	if(actual != 0) 
	{
		actual->Move(-cursor_x,-cursor_y);
		rootwnd->RemoveChild(actual);
	}

	if (index >= 0)
	{
		rootwnd->AddChild(cursorImage[index]->cursor);
		cursorImage[index]->cursor->Move(cursor_x,cursor_y);	
		actual = cursorImage[index]->cursor;
		
	}
	else
		actual = 0;

	actual_index=index;

	return 0;
}


void CGUICursor::Move(float x, float y)
{
	if(actual)
	{
		actual->Move(x-cursor_x,y-cursor_y);
		cursor_x = x;
		cursor_y = y;
	}
}

void CGUICursor::BringToTop()
{
	if(actual)
		actual->BringToTop();
}

void CGUICursor::SetVisible(int vis)
{
	if(actual)
		actual->SetVisible(vis);
}

void CGUICursor::GetPos(float &x, float &y)
{
	x=cursor_x;
	y=cursor_y;
}

//////////////////////////////////////////////////////////////////////
// CursorImage
//////////////////////////////////////////////////////////////////////

CGUICursorImage::CGUICursorImage(CTexture *texture, int x, int y)
{
	UINT sx, sy;
	texture->GetImageSize(&sx,&sy);
	cursor = new CBltRect((float)-x, (float)-y, (float)sx, (float)sy, 0xFFFFFFFF);
	cursor->SetTexture(texture);
}


CGUICursorImage::~CGUICursorImage()
{
	delete cursor;
}


//////////////////////////////////////////////////////////////////////
// CGUIStyle
//////////////////////////////////////////////////////////////////////

CGUIStyle::CGUIStyle(char *_name, int num, int files, ...)
{
	va_list _list;
	char *s;
	int i=0;

	numTextures = num;
	textures = new CTexture*[numTextures];
	for(i=0;i<numTextures;i++)
		textures[i] = new CTexture;
	i=0;

	va_start( _list, files ); 
	while(num--)
	{
		s=va_arg(_list,char*);

		if(!(textures[i++]->CreateFromFile(s)))
		{
			for(int j=0;j<numTextures;j++)
				textures[j]->Release();
			SAFE_DELETE_ARRAY(textures);
			throw CExc(eGUI,E_FILE_NOT_FOUND,"CGUIStyle::CGUIStyle(%s)> Cannot find texture: %s",_name,s);
		}
	}
	va_end( _list );

	if(i<numTextures)
	{
		for(int j=0;j<numTextures;j++)
			textures[j]->Release();
		SAFE_DELETE_ARRAY(textures);
		throw CExc(eGUI,E_BAD_ARGUMENTS,"CGUIStyle::CGUIStyle(%s)> Too much arguments!",_name);
	}

	name = new char[strlen(_name)+1];
	strcpy(name,_name);

	list = 0;
	parameters=0;
	num_parameters=0;
	paramUINT=0;
}

CGUIStyle::CGUIStyle(char *_name, int num, char *addr)
{
	char fname[255];
	int i;
	numTextures = num;
	textures = new CTexture*[numTextures];
	for(i=0;i<numTextures;i++)
		textures[i] = new CTexture;

	i=0;
	while(i<num)
	{
		sprintf(fname,"%s\\%d.png",addr,i);
		if(!(textures[i++]->CreateFromFile(fname)))
		{
			for(int j=0;j<numTextures;j++)
				textures[j]->Release();
			SAFE_DELETE_ARRAY(textures);
			throw CExc(eGUI,E_FILE_NOT_FOUND,"CGUIStyle::CGUIStyle(%s)> Cannot find texture: %s",_name,fname);
		}		
	}

	name = new char[strlen(_name)+1];
	strcpy(name,_name);

	list = 0;
	parameters=0;
	num_parameters=0;
	paramUINT=0;
}

CGUIStyle::~CGUIStyle()
{
	for(int i=0;i<numTextures;i++)
		textures[i]->Release();
	SAFE_DELETE_ARRAY(textures);
	numTextures=0;
	delete name;
	if(parameters)
		delete[] parameters;
	parameters = 0;

	if(list){
		list->Remove(this);	// pokud je styl jeste zarazen v nejakem seznamu, odstran ho ze seznamu
							// spravne by ale mel jiz byt odstranen
		assert(false);		// !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani ze seznamu)
		//if(!std::uncaught_exception())
		//	throw CExc(eGUI,E_INTERNAL, "CGUIStyle::~CGUIStyle> Incorrect work with list");
	}
}

CTexture *CGUIStyle::GetTexture(int num)
{
	if(num<numTextures && num>=0 && textures[num]) 
		return textures[num];
	else{
		throw CExc(eGUI,E_BAD_ARGUMENTS,"CGUIStyle::GetTexture> Texture number %d is not in style %s",num,name);
	}
}

void CGUIStyle::SetParameters(float *param, int num)
{
	if(parameters)
		delete[] parameters;
	parameters = param;
	num_parameters = num;
}

float CGUIStyle::GetParametr(int i)
{
	if(parameters && i>=0 && i < num_parameters)
		return parameters[i];
	else{
		throw CExc(eGUI,E_BAD_ARGUMENTS,"CGUIStyle::GetParametr> Parametr index  %d is not in style %s",i,name);
	}
}

float CGUIStyle::operator[] (int i)
{
	if(parameters && i>=0 && i < num_parameters)
		return parameters[i];
	else{
		throw CExc(eGUI,E_BAD_ARGUMENTS,"CGUIStyle::operator[]> Parametr index  %d is not in style %s",i,name);
	}
}

bool CGUIStyle::operator==(char* _name)
{
	if(!strcmp(name,_name))
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////
// CGUIStyleSet
//////////////////////////////////////////////////////////////////////

CGUIStyleSet::CGUIStyleSet()
{
}

CGUIStyleSet::~CGUIStyleSet()
{
}

void CGUIStyleSet::Add(CGUIStyle *_style)
{
	styleList.AddToHead(_style);
}

CGUIStyle *CGUIStyleSet::Get(char *name)
{
	return ::Get<CGUIStyle,char*>(&styleList, name);
}

CGUIStyle *CGUIStyleSet::Remove(char *name)
{
	return ::Remove<CGUIStyle,char*>(&styleList, name);
}

void CGUIStyleSet::Delete(char *name)
{
	CGUIStyle *s=Remove(name);
	SAFE_DELETE(s);
}


//////////////////////////////////////////////////////////////////////
// CGUITimerRequest
//////////////////////////////////////////////////////////////////////

typeID CGUITimerRequest::lastID=1;		// citac ID pozadavku

CGUITimerRequest::CGUITimerRequest(CGUIElement* el, float _time, typeID& _ID)
{
	element = el;
	time = _time;
	rID = _ID = lastID++;
	list = 0;
}

CGUITimerRequest::~CGUITimerRequest()
{
	if(list)
	{
		list->Remove(this);	// pokud je pozadavek jeste zarazen v nejakem seznamu, odstran ho ze seznamu
							// spravne by ale mel jiz byt odstranen
		assert(false);		// !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani ze seznamu)
		/*if(!std::uncaught_exception())
			throw CExc(eGUI,E_INTERNAL, "CGUITimerRequest::~CGUITimerRequest> Incorrect work with list");*/
	}

}

bool CGUITimerRequest::operator==(typeID _ID)
{
	return (rID == _ID);
}

bool CGUITimerRequest::operator==(CGUIElement* _element)
{
	return (element==_element);
}


//////////////////////////////////////////////////////////////////////
// CGUITimer
//////////////////////////////////////////////////////////////////////

CGUITimer::CGUITimer()
{
}

CGUITimer::~CGUITimer()
{
}

typeID CGUITimer::AddRequest(CGUIElement* el, float time)
{
	typeID rID;
	CGUITimerRequest* rgst = new CGUITimerRequest(el,time,rID);
	timerRequests.AddToHead(rgst);
	return rID;
}

CGUITimerRequest *CGUITimer::RemoveRequest(CGUIElement* el)
{
	return Remove<CGUITimerRequest,CGUIElement*>(&timerRequests,el);
}

CGUITimerRequest *CGUITimer::RemoveRequest(typeID rID)
{
	return Remove<CGUITimerRequest,typeID>(&timerRequests,rID);
}

bool CGUITimer::DeleteRequest(CGUIElement* el)
{
	CGUITimerRequest* rqst = RemoveRequest(el);
	if(rqst)
	{
		delete rqst;
		return true;
	}else
		return false;
}

bool CGUITimer::DeleteRequest(typeID rID)
{
	CGUITimerRequest* rqst = RemoveRequest(rID);
	if(rqst)
	{
		delete rqst;
		return true;
	}else
		return false;
}

bool CGUITimer::DeleteAllRequests(CGUIElement* el)
{
	bool something_deleted=false;
	while(DeleteRequest(el))
		something_deleted = true;

	return something_deleted;
}

bool CGUITimer::ContainRequest(CGUIElement* el)
{
	return IsMember<CGUITimerRequest,CGUIElement*>(&timerRequests,el);
}

bool CGUITimer::ContainRequest(typeID rID)
{
	return IsMember<CGUITimerRequest,typeID>(&timerRequests,rID);
}

void CGUITimer::TimerImpulse(float time)
// projde seznam vsech pozadavku, odecte od vsech cas, ktery uplynul
// (u pozadavku je cas zbyvajici do vyvolani)
// pro ty, ktere maji cas 0 nebo mensi, vygeneruje zpravu o prichodu Timeru a zrusi pozadavek
{
	CGUITimerRequest *rqst;
	CGUIListElement<CGUITimerRequest>* le = timerRequests.GetNextListElement(0);

	while(le)
	{
		if((rqst=timerRequests.GetObjectFromListElement(le)))
		{
			rqst->time -=time;
			if(rqst->time <= 0)
			{
				CGUIListElement<CGUITimerRequest>* oldle=le;
				le=timerRequests.GetNextListElement(le);
				timerRequests.RemoveListElement(oldle);

				SendTimerMessage(rqst);
			}else
				le=timerRequests.GetNextListElement(le);
		}else
			le=timerRequests.GetNextListElement(le);
	}
}

void CGUITimer::SendTimerMessage(CGUITimerRequest* rqst)
{
	if(!rqst)
		return;

	CGUIMessage *msg = new CGUIMessage(MsgTimer, -rqst->time, 0, rqst->rID, 0, rqst->element);	// add the time stamp
	msgQueue->AddMsgToQueue(msg,rqst->element);
	delete rqst;
}


//////////////////////////////////////////////////////////////////////
// CGUITrash
//////////////////////////////////////////////////////////////////////

CGUITrash::~CGUITrash()
{
	if(trash.GetFromHead())
	{
		EmptyTrash();
		assert(false);		// !@#$ v kosi neco zustalo (prozatimni kontrola, jinak pri vyjimce normalne muze nastat => odstranit assert)
	}
}

void CGUITrash::Add(CGUIElement *el)
{
	trash.AddToHead(el);
}

bool CGUITrash::IsIn(CGUIElement *el)
{
	return trash.IsMember(el);
}

CGUIElement *CGUITrash::Remove(CGUIElement *el)
{
	return trash.Remove(el);
}

void CGUITrash::Delete(CGUIElement *el)
{
	trash.Delete(el);
}

void CGUITrash::EmptyTrash()
{
	CGUIElement *el;
	while(el=trash.RemoveFromHead())
		delete el;
}


//////////////////////////////////////////////////////////////////////
// CGUIFont
//////////////////////////////////////////////////////////////////////

void CGUIFont::CreateFont(int nHeight,int nWidth,int nEscapement,int nOrientation,int nWeight,BYTE bItalic,
			BYTE bUnderline,BYTE cStrikeOut,BYTE nCharSet,BYTE nOutPrecision,BYTE nClipPrecision,
			BYTE nQuality,BYTE nPitchAndFamily,LPCTSTR lpszFacename)
{
	font  = ::CreateFont( nHeight, nWidth, nEscapement, nOrientation, nWeight, bItalic,
                          bUnderline, cStrikeOut, nCharSet, nOutPrecision,
                          nClipPrecision, nQuality,
                          nPitchAndFamily, lpszFacename );
}


void CGUIFont::CreateFontPS(LPCTSTR lpszFacename, int Height, int Weight, BYTE bItalic,
			BYTE bUnderline, BYTE cStrikeOut, int Width, int Orientation)
{
	font  = ::CreateFont( Height, Width, 0, Orientation, Weight, bItalic,
                          bUnderline, cStrikeOut, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                          VARIABLE_PITCH, lpszFacename );
}

void CGUIFont::CreateFont(LPCTSTR lpszFacename, int Height, int Weight, BYTE bItalic,
			BYTE bUnderline, BYTE cStrikeOut, int Width, int Orientation)
{
	Height = -MulDiv(Height, GetDeviceCaps(mainGUI->dc, LOGPIXELSY), 72);
	//Width = -MulDiv(Width, GetDeviceCaps(mainGUI->dc, LOGPIXELSX), 70);
	font  = ::CreateFont( Height, Width, 0, Orientation, Weight, bItalic,
                          bUnderline, cStrikeOut, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                          VARIABLE_PITCH, lpszFacename );
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CGUIMainHandler
//////////////////////////////////////////////////////////////////////////////////////////////////

int CGUIMainHandler::StartProfile()
{
	SAFE_DELETE(desktop);		// smaze puvodni desktop
	msgQueue->DeleteMsgQueue();	// smaze vsechny nevyrizene zpravy
	eventServer->DeleteEventsQueue();// smaze vsechny nevyrizene udalosti
	SAFE_DELETE(editor);			// smaze editor (pokud bezi)
	SAFE_DELETE(KerMain);			// smaze kernel

	GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

	KRKAL->SetScreenRes(KRKAL->cfg.game_resx,KRKAL->cfg.game_resy);	// nastavi rozliseni enginu
	KRKAL->SetFullscreen(KRKAL->cfg.game_fullscreen);

	desktop = new CProfileDesktop();	// vytvori MainMenu desktop
	desktop->AddToEngine(mainGUI->rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
	mainGUI->cursor->BringToTop();

	return 1;
}

int CGUIMainHandler::StartOldMainMenu()
{
	SAFE_DELETE(desktop);		// smaze puvodni desktop
	msgQueue->DeleteMsgQueue();	// smaze vsechny nevyrizene zpravy
	eventServer->DeleteEventsQueue();// smaze vsechny nevyrizene udalosti
	SAFE_DELETE(editor);			// smaze editor (pokud bezi)
	SAFE_DELETE(KerMain);			// smaze kernel

	GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

	KRKAL->SetScreenRes(KRKAL->cfg.game_resx,KRKAL->cfg.game_resy);	// nastavi rozliseni enginu
	KRKAL->SetFullscreen(KRKAL->cfg.game_fullscreen);

	desktop = new CMainMenuDesktop();	// vytvori MainMenu desktop
	desktop->AddToEngine(mainGUI->rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
	mainGUI->cursor->BringToTop();

	return 1;
}

int CGUIMainHandler::StartLevelEditor()
{
	SAFE_DELETE(desktop);		// smaze puvodni desktop
	msgQueue->DeleteMsgQueue();	// smaze vsechny nevyrizene zpravy
	eventServer->DeleteEventsQueue();	// smaze vsechny nevyrizene udalosti
	SAFE_DELETE(editor);				// smaze editor (pokud bezi)
	SAFE_DELETE(KerMain);				// smaze kernel

	GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)
	GEnMain->SetTopLightDefault(); //nastavi intenzitu svetla shora na default

	KRKAL->SetScreenRes(KRKAL->cfg.editor_resx,KRKAL->cfg.editor_resy);		// nastavi rozliseni enginu
	KRKAL->SetFullscreen(KRKAL->cfg.editor_fullscreen);

	desktop = new CEDEditorDesktop();	// vytvori desktop editoru
	desktop->AddToEngine(mainGUI->rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
	mainGUI->cursor->BringToTop();		

	new CEDEditor();						// vytvoreni editoru
	editor->Init(mainGUI->RUN_EDITOR_LEVEL,mainGUI->RUN_EDITOR_SCRIPT);		// rozjeti editoru

	return 1;
}

int CGUIMainHandler::StartScriptEditor()
{
	SAFE_DELETE(desktop);		// smaze puvodni desktop
	msgQueue->DeleteMsgQueue();	// smaze vsechny nevyrizene zpravy
	eventServer->DeleteEventsQueue();	// smaze vsechny nevyrizene udalosti
	SAFE_DELETE(editor);				// smaze editor (pokud bezi)
	SAFE_DELETE(KerMain);				// smaze kernel

	GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

	KRKAL->SetScreenRes(KRKAL->cfg.editor_resx,KRKAL->cfg.editor_resy);		// nastavi rozliseni enginu
	KRKAL->SetFullscreen(KRKAL->cfg.editor_fullscreen);

	desktop = new CSEdDesktop();	// vytvori desktop editoru
	desktop->AddToEngine(mainGUI->rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
	mainGUI->cursor->BringToTop();		

	return 1;
}

int CGUIMainHandler::StartMainMenu()
{
	SAFE_DELETE(desktop);		// smaze puvodni desktop
	msgQueue->DeleteMsgQueue();	// smaze vsechny nevyrizene zpravy
	eventServer->DeleteEventsQueue();// smaze vsechny nevyrizene udalosti
	SAFE_DELETE(editor);			// smaze editor (pokud bezi)
	SAFE_DELETE(KerMain);			// smaze kernel

	GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

	KRKAL->SetScreenRes(KRKAL->cfg.game_resx,KRKAL->cfg.game_resy);	// nastavi rozliseni enginu
	KRKAL->SetFullscreen(KRKAL->cfg.game_fullscreen);

	desktop = new CMMDesktop();	// vytvori MainMenu desktop
	desktop->AddToEngine(mainGUI->rootwnd);	// pripojeni desktopu do vykreslovaciho enginu
	mainGUI->cursor->BringToTop();

	return 1;
}


void CGUIMainHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	// ---------------------------------------------------------------------------------
	// zpracovani udalosti START Menu
	if(event->eventID == EClicked)
	{
		if(event->sender == icons[0])		// polozka start menu - Level Editor
		{	
			if(mainGUI->key_shift)
			{
				mainGUI->editorGameMod = mainGUI->editorGameMod ? 0 : 1;
			}

			StartLevelEditor();
		}

		if(event->sender == icons[1])		// polozka start menu - GAME
		{	
			StartMainMenu();
		}

		if(event->sender == icons[2])	// polozka start menu - QUIT
		{	
			KRKAL->Quit();		// ukonci engine (ne hned, az po konsolidaci)
		}

		if(event->sender == icons[3])		// polozka start menu - Script Editor
		{	
			StartScriptEditor();
		}
	}
	// ---------------------------------------------------------------------------------


	// ---------------------------------------------------------------------------------
	// zpracovani udalosti ECommand:
	if(event->eventID == ECommand)
	{
		switch(event->pInt)
		{
			case 1 : StartScriptEditor();break;
			case 2 : StartLevelEditor();break;
			case 3 : StartOldMainMenu();break;
			case 4 : KRKAL->Quit();break;
			case 5 : StartProfile();break;
			case 6 : StartMainMenu();break;
		}
	}
	// ---------------------------------------------------------------------------------
	delete event;
}




