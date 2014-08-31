///////////////////////////////////////////////
//
// SEdDesktop.cpp
//
// Implementation pro desktop editoru skriptu
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#include "stdafx.h"
#include "SEdDesktop.h"
#include "menu.h"

#include "SEdFileBrowser.h"
#include "SEdObjectBrowser.h"
#include "NameBrowser.h"

#include "dx.h"

#include "SEdOpenedFiles.h"
#include "scriptedit.h"

CSEdDesktop *SEdDesktop=NULL;

//////////////////////////////////////////////////////////////////////
// CSEdDesktop
//////////////////////////////////////////////////////////////////////

CSEdDesktop::CSEdDesktop()
	: CGUIDesktop()
{
	try{
		assert(!SEdDesktop);
		SEdDesktop=this;

		RemoveFromTrash();
		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR);
		// MsgSetProduce(MsgAll);

		CGUIWindowPanel* panel = new CGUIWindowPanel(0,sy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT,styleSet->Get("Panel"),this,true,false,0,0);
													//0,-STD_PANEL_SHIFT (pridani nahoru)
		push=false;

		iconDefaultHandler = new CSEdDesktopIconHandler();
		iconDefaultHandler->icons = new typeID[4];		// zadat velikost pole dle poctu icon (polozek typeID)

		SEdOpenedFiles = new CSEdOpenedFiles();

		AddStartMenu();

		InitDefaultIcons();

		iconDefaultHandler->ShowFileBrowser();

		AddToTrash();
	}
	catch(CExc c)
	{
		mainGUI->LogError(c);

		// vypis informace o chybe na obrazovce jako DebugMessage (prozatimni reseni)
		__time64_t ltime;
		_time64( &ltime );
		DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);

		desktop=0;
		throw CExc(eGUI, E_INTERNAL,"CSEdDesktop::CSEdDesktop> Fatal error - cannot creat DESKTOP");
	}
}

CSEdDesktop::~CSEdDesktop()
{
	assert(SEdDesktop==this);
	SEdDesktop=NULL;

	RemoveFromTrash();
	SAFE_DELETE(iconDefaultHandler);
	SAFE_DELETE(SEdOpenedFiles);
}

void CSEdDesktop::AddStartMenu()
{
	CGUIMenu* menu;
	CGUIMenuItem* mi;

	menu = new CGUIMenu(185);
	menu->SetPictureStripSize(44);
	menu->SetMenuPictureSize(40);

	mi = new CGUIMenuItem(menu,0,0,185,40,"Quit",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));
	CGUIFont *font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Quit",font);
	mi->SetShortCut("F4",false,false,false,VK_F4,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[2]=mi->GetID();

	mi = new CGUIMenuItem(menu,0,0,185,40,"Script Editor",new CGUIRectHost(0,0,styleSet->Get("StartMenu_Script")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Script Editor",font);
	mi->SetShortCut("F1",false,false,false,VK_F1,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[3]=mi->GetID();


	mi = new CGUIMenuItem(menu,0,0,185,40,"Level Editor",new CGUIRectHost(0,0,styleSet->Get("StartMenu_Level")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Level Editor",font);
	mi->SetShortCut("F2",false,false,false,VK_F2,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[0]=mi->GetID();


	mi = new CGUIMenuItem(menu,0,0,185,60,"Game",new CGUIRectHost(0,0,styleSet->Get("StartMenu_Game")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,20);
	mi->SetLabelFormated("GAME",font);
	mi->SetShortCut("F3",false,false,false,VK_F3,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[1]=mi->GetID();


	startMenu=menu;

}


int CSEdDesktop::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)
	{
		/*
		CGUIMenuItem* mi = menu->GetItem(2);
		if(mi)
			menu->DeleteMenuItem(mi);
			*/
	}
	return 1;				
}

int CSEdDesktop::MouseMiddle(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)
	{
		/*
		CGUIMenuItem* mi = menu->GetItem(3);
		if(mi->IsActive())
			mi->Deactivate();
		else
			mi->Activate();
			*/
	}
	return 1;
}

int CSEdDesktop::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)
	{
		//menu->On(0);
	}
	return 1;				
}


void CSEdDesktop::ShowFileBrowser(){
	iconDefaultHandler->ShowFileBrowser();
}
void CSEdDesktop::ShowObjectBrowser(){
	iconDefaultHandler->ShowObjectBrowser();
}
void CSEdDesktop::ShowNameBrowser(){
	iconDefaultHandler->ShowNameBrowser();
}

CSEdObjectBrowser* CSEdDesktop::GetObjectBrowser()
{
	return iconDefaultHandler->GetObjectBrowser();
}
CNameBrowser* CSEdDesktop::GetNameBrowser()
{
	return iconDefaultHandler->GetNameBrowser();
}
CSEdFileBrowser* CSEdDesktop::GetFileBrowser()
{
	return iconDefaultHandler->GetFileBrowser();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CSEdDesktop::InitDefaultIcons	- inicializace ikon na desktopu a panelu pri startu editoru
//////////////////////////////////////////////////////////////////////////////////////////////////

void CSEdDesktop::InitDefaultIcons()
{
	CGUIButton *icon=0;

	((CSEdDesktop*)desktop)->startMenu->AcceptEvent(iconDefaultHandler->GetID(),EOff);

	if(desktop->panel)
	{
		CGUIButton* icon = new CGUIButton(100,100,50,25,styleSet->Get("Start1"),styleSet->Get("Start2"),styleSet->Get("Start3"),false,0,0);
		if(!desktop->panel->AddIcon(icon,2))
		{
			iconDefaultHandler->icons[0] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
			icon->AcceptEvent(iconDefaultHandler->GetID(),EDown);
			icon->AcceptEvent(iconDefaultHandler->GetID(),EUp);
		}else
			delete icon;
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("FileBrowser"),styleSet->Get("FileBrowser2"),styleSet->Get("FileBrowser3"),"FileBrowser");
		if(icon)
		{
			iconDefaultHandler->icons[1] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}

		icon = new CGUIButton(0,0,-1,-1,"GUI.But.ObjectBrowser","",0,"ObjectBrowser");		
		if(icon&&desktop->panel->AddIcon(icon)==0)
		{
			iconDefaultHandler->icons[2] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}

		icon = new CGUIButton(0,0,-1,-1,"GUI.But.NameBrowser","",0,"NameBrowser");		
		if(icon&&desktop->panel->AddIcon(icon)==0)
		{
			iconDefaultHandler->icons[3] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}
	}




}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CSEdDesktopIconHandler	- vyvolani programu/oken po stisknuti ikony (spusteni aplikace -> inicializacni kod)
//////////////////////////////////////////////////////////////////////////////////////////////////

CSEdDesktopIconHandler::CSEdDesktopIconHandler()
{
	icons = 0; 
	startButtonDown=false;startButtonOff=false;

	filebrowserID = 0;
	objectbrowserID = 0;
	namebrowserID = 0;
}

void CSEdDesktopIconHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	

	// Obsluha START Menu ---------------------------------------------
	if(event->sender == icons[0])	// prisla udalost od moji ikony
	{	

		if(event->eventID==EClicked)
		{
			if(startButtonDown)
				startButtonDown=false;
			else{
				CGUIMenu* menu = ((CSEdDesktop*)desktop)->startMenu;
				float msx,msy,dsx,dsy;
				desktop->GetVPSize(dsx,dsy);
				menu->GetSize(msx,msy);
				menu->On(0,0,dsy-msy);
			}
		}

		if(event->eventID==EDown)
		{
			if(startButtonOff)
				startButtonDown=true;
		}
	}

	if(event->eventID == EOff)
	{
		CGUIMenu* menu = ((CSEdDesktop*)desktop)->startMenu;
		if(event->sender == menu->GetID())
		{
			CGUIEvent* e = new CGUIEvent();
			e->eventID = EEvent;
			e->sender = GetID();
			e->recipient = GetID();
			eventServer->AddEventToQueue(e);
			startButtonOff=true;
		}
	}

	if(event->eventID == EEvent)
	{
		// vlastni inicializacni kod "aplikace" (vytvareneho okna)
		CGUIMenu* menu = ((CSEdDesktop*)desktop)->startMenu;
		if(menu->isActive()) //  && !startButtonClicked
		{
			menu->Off();
		}
		startButtonOff=false;		
	}
	// konec obsluhy START Menu ------------------------------------------

	if(event->eventID==EClicked)
	{
		// FileBrowser -----------------------------------------------
		if(event->sender == icons[1])
			ShowFileBrowser();
		else
		// ObjectBrowser ---------------------------------------------
		if(event->sender == icons[2])
			ShowObjectBrowser();
		else
		// NameBrowser -----------------------------------------------
		if(event->sender == icons[3])
			ShowNameBrowser();
	}


	delete event;
}


void CSEdDesktopIconHandler::ShowFileBrowser()
{

	CSEdFileBrowser *fb;

	fb = dynamic_cast<CSEdFileBrowser*>(nameServer->ResolveID(filebrowserID));
	if(fb)
	{
		fb->FocusMe();
	}else
	{
		float xx,yy;
		desktop->GetBackWindowSize(xx,yy);

		fb = new CSEdFileBrowser(xx-250,0,250,yy);
		desktop->AddBackElem(fb);
		fb->FocusMe();

		filebrowserID=fb->GetID();
	}

	CSEdObjectBrowser *ob = GetObjectBrowser();
	if(ob) ob->CloseWindow();
	CNameBrowser *nb = GetNameBrowser();
	if(nb) nb->CloseWindow();
}

void CSEdDesktopIconHandler::ShowObjectBrowser()
{

	CSEdObjectBrowser *ob;

	ob = dynamic_cast<CSEdObjectBrowser*>(nameServer->ResolveID(objectbrowserID));
	if(ob)
	{
		ob->FocusMe();
	}else
	{
		float xx,yy;
		desktop->GetBackWindowSize(xx,yy);

		ob = new CSEdObjectBrowser(xx-250,0,250,yy);
		desktop->AddBackElem(ob);
		ob->FocusMe();

		objectbrowserID=ob->GetID();

		SEdOpenedFiles->CompileForOB();
		
	}
}

void CSEdDesktopIconHandler::ShowNameBrowser()
{

	CNameBrowser *nb;

	nb = dynamic_cast<CNameBrowser*>(nameServer->ResolveID(namebrowserID));
	if(nb)
	{
		nb->FocusMe();
	}else
	{
		float xx,yy;
		desktop->GetBackWindowSize(xx,yy);

		nb = new CNameBrowser(xx-250,0,250,yy);
		desktop->AddBackElem(nb);
		nb->FocusMe();

		namebrowserID=nb->GetID();

		SEdOpenedFiles->CompileForOB();
	}
}


CSEdObjectBrowser* CSEdDesktopIconHandler::GetObjectBrowser()
{
	CSEdObjectBrowser *ob;

	ob = dynamic_cast<CSEdObjectBrowser*>(nameServer->ResolveID(objectbrowserID));

	return ob;
}

CNameBrowser* CSEdDesktopIconHandler::GetNameBrowser()
{
	CNameBrowser *nb;

	nb = dynamic_cast<CNameBrowser*>(nameServer->ResolveID(namebrowserID));

	return nb;
}

CSEdFileBrowser* CSEdDesktopIconHandler::GetFileBrowser()
{
	CSEdFileBrowser *fb;

	fb = dynamic_cast<CSEdFileBrowser*>(nameServer->ResolveID(filebrowserID));

	return fb;
}