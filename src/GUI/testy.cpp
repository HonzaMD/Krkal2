///////////////////////////////////////////////
//
// testy.cpp
//
// Implementace pro testovaci objekty GUI
//	jedna se o speceialne upraveny desktop, okno, ...
//	definuji se zde objekty odvozene od zakladnich trid + s definovanou konkretni fci
//	jde tedy o priklady skutecnych pouzitelnych elementu GUI 
//  testuje se funkcnost implementace objektu GUI
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "GUI.h"
#include "widget.h"
#include "testy.h"
#include "menu.h"
#include "textedit.h"
#include "tree.h"
#include "regedit.h"

#include "fs.h"
#include "dx.h"
#include "kernel.h"
#include "KerErrors.h"
#include "md_testy.h"
#include "dxhighlight.h"
#include "ComCompiler.h"

#include <time.h>


int WoknoW(float x, float y);
int TestWindow(float x, float y);
int TestEdit(float x, float y);
int TestTree(float x, float y);
int TestBrowser(float x, float y);


CGUITestWindow::CGUITestWindow(float _x, float _y, float _sx, float _sy, char* title, CGUIRectHost* icon )
		: CGUIStdWindow(_x,_y,_sx,_sy,title,icon) 
{
	RemoveFromTrash();
		MsgAcceptConsume(MsgMouseM|MsgMouseR);
	//	MsgSetProduce(MsgMouseL|MsgMouseOver);
	//	SetLimitSizes(60,30);
		autogrowing = true;
		program=0;
		AddToTrash();

		//CGUIWindowPanel* panel = new CGUIWindowPanel(0,sy-STD_PANEL_HEIGHT,styleSet->Get("Panel"),this);
}

int CGUITestWindow::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{
	/*
	if(!mouseState)
	{
		CGUIMessage *msg = new CGUIMessage(MsgCommand,ComCloseWnd,this);
		SendMsg(msg,this);
	}
	return 0;					// This message will be deleted
	*/
	float xx, yy;
	xx=x;
	yy=y;

	InnerCoords(x,y);
	if(TransformCoordsFrontToBack(x,y))
	{
		DebugMessage(4,"%2d:%2d",(int)x, (int)y);
		DebugMessage(6,"%2d",mtr.randInt(20));
	}else{
		InnerCoords(xx,yy);
		DebugMessage(4,"------");
		DebugMessage(5,"%2d:%2d",(int)xx, (int)yy);
	}
	return 1;
}

int CGUITestWindow::MouseMiddle(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)
	{
		SetVisible(0);
		/*
		CGUIElement *win = desktop->FindElement((float)x,(float)y);	
		if(win)
		{
//			if(win->IsVisible())
				win->SetVisible(0);
		}
		//*/
	}
	return 0;					// I'll not produce this message
}

CGUITestDesktop::CGUITestDesktop()
	: CGUIDesktop()
{
	RemoveFromTrash();

	try{

		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR);
		// MsgSetProduce(MsgAll);

	/*
		background->SetCol(0xFFFF0000);
		CTexture *eizo = new CTexture();
		eizo->CreateFromFile("tex\\eizo2.png");
		background->SetTexture(eizo);
		eizo->Release();
	*/
		//AddToEngineBottom(_rootwnd);

		CGUIWindowPanel* panel = new CGUIWindowPanel(0,sy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT,styleSet->Get("Panel"),this,true,false,0,0);
													//0,-STD_PANEL_SHIFT (pridani nahoru)
		//sy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT
		push=false;

		iconDefaultHandler = new CGUITestIconHandler();
		iconDefaultHandler->icons = new typeID[13];		// zadat velikost pole dle poctu icon (polozek typeID)

		AddContextMenu();
		AddStartMenu();

		InitDefaultIcons();

	}
	catch(CExc c)
	{
		mainGUI->LogError(c);

		// vypis informace o chybe na obrazovce jako DebugMessage (prozatimni reseni)
		__time64_t ltime;
		_time64( &ltime );
		DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);

		desktop=0;
		throw CExc(eGUI, E_INTERNAL,"CGUITestDesktop::CGUITestDesktop> Fatal error - cannot creat DESKTOP");
	}

	/*
	CGUIFont* f = new CGUIFont();
	f->CreateFont(ARIAL,13);

//	AddBackElem(new CGUIStaticText("GAMESA",f,50,50,0xFFFF0000,60,0,aLeft,1));
	//AddBackElem(new CGUIStaticText("GAMESA",f,50,80,0xFFFF0000,60));
	//AddBackElem(new CGUIStaticText("GAMESA",f,50,110,0xFFFF0000));
	AddBackElem(new CGUIStaticText(50,50,"GAMESA",ARIAL,12,0,0xFFFF0000));
	AddBackElem(new CGUIStaticText(50,70,"GAMESA",ARIAL,13,0,0xFFFF0000));
	AddBackElem(new CGUIStaticText(50,90,"GAMESA",ARIAL,14,0,0xFFFF0000));
	AddBackElem(new CGUIStaticText(50,110,"GAMESA",ARIAL,15,0,0xFFFF0000));

	f->Release();
*/

	/*
	CGUIFont* f = new CGUIFont();
	f->CreateFontPS(ARIAL,22);
	AddBackElem(new CGUIStaticPicture(50,60,300,60,0xFFFFFFFF));
	AddBackElem(new CGUIStaticText("Script Editor",f,50,50,0xFFFF0000,100,0,aLeft,1));
	AddBackElem(new CGUIStaticText("Script Editor",f,50,70,0xFFFF0000,60));
	//AddBackElem(new CGUIStaticText("Script Editor",f,50,90,0xFFFF0000));

	f->Release();

	AddBackElem(new CGUIStaticText(50,110,"Script Editor",ARIAL,14,0,0xFFFF0000));
	AddBackElem(new CGUIStaticText(50,130,"Level Editor",ARIAL,14,0,0xFFFF0000));

*/

	AddToTrash();
}

CGUITestDesktop::~CGUITestDesktop()
{
	RemoveFromTrash();
	SAFE_DELETE(iconDefaultHandler);
}

void CGUITestDesktop::AddContextMenu()
{
	CGUIMenu* menu;
	CGUIMenuItem* mi;

	menu = new CGUIMenu(170);
	mi = menu->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");

	if(mi)
	{
		mi->SetShortCut("Ctrl+A",true,false,false,'A');
	}

	CGUIMenu* subMenu = new CGUIMenu(130);
	subMenu->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	menu->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");

	CGUIMenu* subMenu2 = new CGUIMenu(130);
	subMenu2->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu2->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu2->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu2->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	menu->AddSubMenuItem(subMenu,0,"Podmeòátko...",0,"Copak se z toho vyklube?");
	menu->AddGap(0);
	menu->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	menu->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");
	menu->AddSubMenuItem(subMenu2,0,"Podmeòátko222",0,"Copak se z toho vyklube?");


	subMenu2 = new CGUIMenu(130);
	subMenu2->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu2->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu2->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu2->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	subMenu->AddSubMenuItem(subMenu2,0,"Podmeòátko...",0,"Copak se z toho vyklube?");

	//AddFrontElem(menu);

	mi = menu->GetItem(1);
	mi->SetInactivePicture(new CGUIRectHost(0,0,styleSet->Get("KrkalTB4")->GetTexture(0)));
	menu->AddMenuItem(mi->GetID(),"Pøídavek?",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB3")->GetTexture(0)),"fachá to?");

	AcceptEvent(menu->GetID(),EMessage);
}

void CGUITestDesktop::AddStartMenu()
{
	CGUIMenu* menu;
	CGUIMenuItem* mi;
	//typeID miID;

	menu = new CGUIMenu(170);
	menu->SetPictureStripSize(30);
	menu->SetMenuPictureSize(26);

	/*
	miID = menu->AddMenuItem(0,"Script Editor",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));

	mi = dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(miID));
	if(mi)
	{
		mi->SetShortCut("F1",false,false,false,VK_F1);
	}
	*/

	mi = new CGUIMenuItem(menu,0,0,170,30,"Quit",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));
	CGUIFont *font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Quit",font);
	mi->SetShortCut("F4",false,false,false,VK_F4,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[2]=mi->GetID();



	mi = new CGUIMenuItem(menu,0,0,170,30,"Script Editor",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Script Editor",font);
	mi->SetShortCut("F1",false,false,false,VK_F1,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[3]=mi->GetID();



	mi = new CGUIMenuItem(menu,0,0,170,30,"Level Editor",new CGUIRectHost(0,0,styleSet->Get("EyeTB3")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("Level Editor",font);
	mi->SetShortCut("F2",false,false,false,VK_F2,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[0]=mi->GetID();


	mi = new CGUIMenuItem(menu,0,0,170,60,"Game",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB3")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,20);
	mi->SetLabelFormated("GAMESA",font);
	mi->SetShortCut("F3",false,false,false,VK_F3,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[1]=mi->GetID();


	startMenu=menu;

	//startMenu->AcceptEvent(mainGUI->iconDefaultHandler->GetID(),EOff);


	/*

	CGUIMenu* subMenu = new CGUIMenu(130);
	subMenu->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	menu->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");

	CGUIMenu* subMenu2 = new CGUIMenu(130);
	subMenu2->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu2->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu2->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu2->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	menu->AddSubMenuItem(subMenu,0,"Podmeòátko...",0,"Copak se z toho vyklube?");
	menu->AddGap(0);
	menu->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	menu->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");
	menu->AddSubMenuItem(subMenu2,0,"Podmeòátko222",0,"Copak se z toho vyklube?");


	subMenu2 = new CGUIMenu(130);
	subMenu2->AddMenuItem(0,"Jdi do školy...",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)),"ahoj");
	subMenu2->AddMenuItem(0,"Nepùjdu...",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)),"cau");
	subMenu2->AddMenuItem(0,"Ale pùjdeš, kámo!",0,"fujjj");
	subMenu2->AddMenuItem(0,"Nenadávej!!!",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB2")->GetTexture(0)),"neštvi mì");

	subMenu->AddSubMenuItem(subMenu2,0,"Podmeòátko...",0,"Copak se z toho vyklube?");

	//AddFrontElem(menu);

	mi = menu->GetItem(1);
	mi->SetInactivePicture(new CGUIRectHost(0,0,styleSet->Get("KrkalTB4")->GetTexture(0)));
	menu->AddMenuItem(mi->GetID(),"Pøídavek?",new CGUIRectHost(0,0,styleSet->Get("SlaviaTB3")->GetTexture(0)),"fachá to?");

	AcceptEvent(menu->GetID(),EMessage);
	*/
}

int CGUITestDesktop::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
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

int CGUITestDesktop::MouseMiddle(float x, float y, UINT mouseState, UINT keyState)
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

int CGUITestDesktop::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{
	if(!mouseState)
	{
		//menu->On(0);
	}
	return 1;				
}


CGUITestButton::CGUITestButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label, CGUIRectHost *_picture, char* _help)
	: CGUIButton(_x,_y,_sx,_sy,_up,_down,_mark, _label, _picture, _help)
{
	//SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FIX,WIDGET_FIX);
	to=5;
}

void CGUITestButton::OnClick()
{
	//CGUIStaticText* text = new CGUIStaticText((float)(rand()%30)+20,(float)(rand()%30)+20,"PRDEL Hovínek",TIMES,18, 0xff50ccdd);
	//((CGUIWindow*)parent)->AddFrontElem(text);	
	//((CGUIWindow*)parent)->AddBackElem(text);	

	//((CGUIWindow*)parent)->SetBackWindowNormalSize(150,150);


	//((CGUIWindow*)parent)->ResizeRel(10,10);
	//((CGUIWindow*)parent)->ResizeRel(-10,-10);

	//((CGUIWindow*)parent)->SetWindowSizeBWNormal();

//	CGUIButton *lbutton = new CGUIButton((float)(rand()%300),(float)(rand()%300),14,14,styleSet->Get("Slavia1"),styleSet->Get("Slavia2"),styleSet->Get("Slavia1"),false);
//	((CGUIWindow*)parent)->AddBackElem(lbutton);
//	CGUIWindow* win = ((CGUIWindow*)parent);
//win->SetWindowSizeBWNormal();
				/*	
				CGUILine* l;
				l = new CGUILine(10,10,100,10);
				l->SetAnchor(1,0,1,1);
				win->AddBackElem(l);
				l = new CGUILine(100,10,100,100);
				l->SetAnchor(1,1,0,1);
				win->AddBackElem(l);
				l = new CGUILine(100,100,10,100);
				l->SetAnchor(0,1,1,1);
				win->AddBackElem(l);
				l = new CGUILine(10,100,10,10);
				l->SetAnchor(1,1,1,0);
				win->AddBackElem(l);




				l = new CGUILine(10,10,100,100);
				l->SetAnchor(1,1,1,1);
				win->AddBackElem(l);

				l = new CGUILine(10,100,100,10);
				l->SetAnchor(1,1,1,1);
				win->AddBackElem(l);*/

		CGUIWindow* win = (CGUIWindow*)parent;


		CGUITestWindow *win2 = new CGUITestWindow((float)100,(float)100,(float)100,(float)100,"TestWindow",0);
		//win->autogrowing = false;
    	win2->SetBackWindowNormalSize(101,101);
		win2->SetWindowSizeBWNormal();
		win2->SetTabOrder(to++);
		//win2->SetMark(true);
		win->AddBackElem(win2);	
				
				//win2->ResizeRel(10,10);
				//win2->ResizeRel(-10,-10);

		CGUIButton *button = new CGUITestButton(10,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSize 150 150");
		win2->AddBackElem(button);

		button = new CGUITestButton2(60,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSizeNormal");
		win2->AddBackElem(button);

				//DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);
				//DebugMessage(4,"%s",_ctime64( &ltime ));

		CGUILine* l;

		l = new CGUILine(10,10,100,10);
		l->SetAnchor(1,0,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(100,10,100,100);
		l->SetAnchor(1,1,0,1);
		win2->AddBackElem(l);

		l = new CGUILine(100,100,10,100);
		l->SetAnchor(0,1,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(10,100,10,10);
		l->SetAnchor(1,1,1,0);
		win2->AddBackElem(l);

		l = new CGUILine(10,10,100,100);
		l->SetAnchor(1,1,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(10,100,100,10);
		l->SetAnchor(1,1,1,1);
		win2->AddBackElem(l);

		//		win->SetBackWindowNormalSize(70,70);
		//		win->SetWindowSizeBWNormal();

		win->SetFocusEl(win2,0);

}

CGUITestButton2::CGUITestButton2(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label, CGUIRectHost *_picture, char* _help)
	: CGUIButton(_x,_y,_sx,_sy,_up,_down,_mark, _label, _picture, _help)
{
	//SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FIX,WIDGET_FIX);
}

void CGUITestButton2::OnClick()
{
	//CGUIStaticText* text = new CGUIStaticText((float)(rand()%30)+20,(float)(rand()%30)+20,"PRDEL Hovínek",TIMES,18, 0xff50ccdd);
	//((CGUIWindow*)parent)->AddFrontElem(text);	
	//((CGUIWindow*)parent)->AddBackElem(text);	

	//((CGUIWindow*)parent)->SetBackWindowNormalSize(450,150);
	//((CGUIWindow*)parent)->SetWindowSizeBWNormal();
	//((CGUIWindow*)parent)->ResizeRel(10,10);
	//((CGUIWindow*)parent)->ResizeRel(-10,-10);

//	CGUIButton *lbutton = new CGUIButton((float)(rand()%300),(float)(rand()%300),14,14,styleSet->Get("Slavia1"),styleSet->Get("Slavia2"),styleSet->Get("Slavia1"),false);
//	((CGUIWindow*)parent)->AddBackElem(lbutton);
//	CGUIWindow* win = ((CGUIWindow*)parent);
//win->SetWindowSizeBWNormal();
				/*	
				CGUILine* l;
				l = new CGUILine(10,10,100,10);
				l->SetAnchor(1,0,1,1);
				win->AddBackElem(l);
				l = new CGUILine(100,10,100,100);
				l->SetAnchor(1,1,0,1);
				win->AddBackElem(l);
				l = new CGUILine(100,100,10,100);
				l->SetAnchor(0,1,1,1);
				win->AddBackElem(l);
				l = new CGUILine(10,100,10,10);
				l->SetAnchor(1,1,1,0);
				win->AddBackElem(l);




				l = new CGUILine(10,10,100,100);
				l->SetAnchor(1,1,1,1);
				win->AddBackElem(l);

				l = new CGUILine(10,100,100,10);
				l->SetAnchor(1,1,1,1);
				win->AddBackElem(l);*/


	if(desktop->panel)
	{
		desktop->panel->AddIcon(styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),"BUTTON");
	}
}

void CGUIMyTestProg::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	//char s[20];
	DebugMessage(5,"Event: %d from %I64d",event->eventID,event->sender);
	//DebugMessage(6,"Event: %s from %I64d",_itoa(event->eventID,s,10),event->sender);

/*
		FILE *f;
	__time64_t ltime;

	FS->ChangeDir("$KRKAL$");
	f = fopen("gui_events.log","a");
	_time64( &ltime );
	fprintf(f,"%s\tEtype: %2d, senderID: %I64d, recipientID: %I64d\n\n", _ctime64( &ltime ), event->eventID, event->sender, event->recipient);
	fclose(f);
*/

	if(event->eventID == EClicked)
	{

	}

	/*if(event->eventID == EOk) // event->eventID == EClicked || 
	{
		textWord->Sync(true);
		textWord->GetParametr(&vint,dtInt,1);
		textWord->GetParametr(&vdouble,dtDouble,1);
		DebugMessage(6,"String:%s",string);
		DebugMessage(7,"Int:%i",vint);
		DebugMessage(8,"Double:%f",vdouble);

		if(button)
			button->SetPicture("$TEX$\\lebky.png");
	}*/

	/*if(event->eventID == ECancel)
	{
		strcpy(string,"HOVNO");
		textWord->Sync(false);
		textWord->Refresh();

		if(button)
			button->SetLabel(new CGUIStaticText(0,0,0,"ahoj",TIMES,10));
	}*/

	delete event;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// CGUITestDesktop::InitDefaultIcons	- inicializace ikon na desktopu a panelu pri startu GUI
//////////////////////////////////////////////////////////////////////////////////////////////////

void CGUITestDesktop::InitDefaultIcons()
{
	CGUIButton *icon;
	CGUITestIconHandler* ih = iconDefaultHandler;
	
	//ih->icons = new typeID[12];	// zadat velikost pole dle poctu icon

	icon = new CGUIButton(600,20,14,14,styleSet->Get("Slavia1"),styleSet->Get("Slavia2"),styleSet->Get("Slavia3"),false,0,0,"Šedivák");	// vytvorit novou ikonu
	ih->icons[0] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu

	icon=0;
	
	((CGUITestDesktop*)desktop)->startMenu->AcceptEvent(ih->GetID(),EOff);

	if(desktop->panel)
	{
		CGUIButton* icon = new CGUIButton(100,100,50,25,styleSet->Get("Start1"),styleSet->Get("Start2"),styleSet->Get("Start3"),false,0,0);
		if(!desktop->panel->AddIcon(icon,2))
		{
			ih->icons[11] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
			icon->AcceptEvent(ih->GetID(),EDown);
			icon->AcceptEvent(ih->GetID(),EUp);
		}else
			delete icon;
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("KrkalTB1"),styleSet->Get("KrkalTB2"),styleSet->Get("KrkalTB3"),"WoknoW");
		if(icon)
		{
			ih->icons[1] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
		}
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),"TestWindow");
		if(icon)
		{
			icon->SetInactiveStyle(styleSet->Get("SlaviaTB4"));
			ih->icons[2] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
		}
	}
	if(desktop->panel)
	{
		desktop->panel->AddGap(10);
		icon = desktop->panel->AddIcon(styleSet->Get("LebkyTB1"),styleSet->Get("LebkyTB2"),styleSet->Get("LebkyTB3"),"Run Kernel !");
		if(icon)
		{
			icon->SetInactiveStyle(styleSet->Get("LebkyTB4"));
			ih->icons[3] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
		}
	}
	if(desktop->panel)
	{
		//CGUIButton* icon = new CGUIButton(100,100,STD_PANEL_ICON_SIZE,STD_PANEL_ICON_SIZE,styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),false,0,0,"Show Debug && Error Log");
		CGUIButton* icon = new CGUIButton(100,100,20,27,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"Show Debug && Error Log");
		icon->SetInactiveStyle(styleSet->Get("EyeTB4"));
		if(!desktop->panel->AddIcon(icon,-2))
		{
			ih->icons[4] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
		}else
			delete icon;
	}

	if(desktop->panel)
	{
		desktop->panel->AddGap(10);
		//CGUIButton* icon = new CGUIButton(100,100,STD_PANEL_ICON_SIZE,STD_PANEL_ICON_SIZE,styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),false,0,0,"Show Debug && Error Log");
		CGUIButton* icon = new CGUIButton(100,100,20,27,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"Zobraz plochu / obnov okna");
		icon->SetInactiveStyle(styleSet->Get("EyeTB4"));
		if(!desktop->panel->AddIcon(icon,-2))
		{
			ih->icons[5] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
			icon->AcceptEvent(ih->GetID(),EMouseButton);
		}else
			delete icon;
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),"Stromeèek");
		if(icon)
		{
			ih->icons[10] = icon->GetID();
			icon->AcceptEvent(ih->GetID(),EClicked);
		}

	}

	icon = new CGUIButton(600,50,14,14,styleSet->Get("Slavia1"),styleSet->Get("Slavia2"),styleSet->Get("Slavia3"),false,0,0,"TB_1");	// vytvorit novou ikonu
	ih->icons[6] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu

	icon = new CGUIButton(600,70,14,14,styleSet->Get("KrkalTB1"),styleSet->Get("KrkalTB2"),styleSet->Get("KrkalTB3"),false,0,0,"TB_2");	// vytvorit novou ikonu
	ih->icons[7] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu

	icon = new CGUIButton(600,90,14,14,styleSet->Get("LebkyTB1"),styleSet->Get("LebkyTB2"),styleSet->Get("LebkyTB3"),false,0,0,"TB_3");	// vytvorit novou ikonu
	ih->icons[8] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu

	icon = new CGUIButton(600,110,14,14,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"TB_4");	// vytvorit novou ikonu
	ih->icons[9] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu

	icon = new CGUIButton(600,140,20,27,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"Zkompiluj to ty sráèi!");	// vytvorit novou ikonu
	ih->icons[12] = icon->GetID();	// zapamatovat si ID nove ikony
	icon->AcceptEvent(ih->GetID(),EClicked);	// objedant si doruceni udalosti o kliknuti
	desktop->AddBackElem(icon);			// pridat ikonu do desktopu
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CGUIIconDefaultHandler	- vyvolani programu/oken po stisknuti ikony (spusteni aplikace -> inicializacni kod)
//////////////////////////////////////////////////////////////////////////////////////////////////

void CGUITestIconHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	
	if(event->eventID == EClicked)
	{
		if(event->sender == icons[0])	// prisla udalost od moji ikony
		{	
			// vlastni inicializacni kod "aplikace" (vytvareneho okna)
			CGUIButton* b = dynamic_cast<CGUIButton*>(nameServer->ResolveID(icons[2]));
			if(b)
			{
				if(b->IsActive())
					b->Deactivate();
				else
					b->Activate();
			}
		}
		if(event->sender == icons[12])	// prisla udalost od moji ikony
		{	
			// vlastni inicializacni kod "aplikace" (vytvareneho okna)
			compiler->TestComp("test_0001_FFFF_0001_0001.kc");
		}
		if(event->sender == icons[2])	// prisla udalost od moji ikony
		{	
			// vlastni inicializacni kod "aplikace" (vytvareneho okna)
			TestEdit(350,50);
		}
		if(event->sender == icons[1])	// prisla udalost od moji ikony
		{	
			// vlastni inicializacni kod "aplikace" (vytvareneho okna)
			WoknoW(150,150);
		}

		if(event->sender == icons[5])	// prisla udalost od moji ikony
		{	
			if(desktop->minimized_all)
				desktop->UnMinimizeAll();
			else
				desktop->MinimizeAll();
		}

		if(event->sender == icons[3] && (!KerMain))	// spusti Kernel
		{	
			try{


				KerCreateKernel(NORMAL_RUN, eKerDBDebug, 0, "jedna_44A0_721C_001C_BF01.lv\\!level");
//				KerCreateKernel(NORMAL_RUN, eKerDBDebug, 1, "test_0001_FFFF_0001_0001.kc");
				MDTestKernel();

//				kernelTime=KerMain->GetTime();

			} catch (CKernelPanic){
				SAFE_DELETE(KerMain);
			}
		}
		if(event->sender == icons[4]) {	// zobrazi Full Log
			KerErrorFiles->ShowAllLogs(0,40,640,200);
		}


		if(event->sender == icons[6])	
		{	
	
			CGUIToolBar* tb = new CGUIToolBar(20,50,100,300,"ToolBar #1"); 
			tb->SetProperties(4,7,4,7,true,false,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
				styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

			
			int k,l;
			for(int i=0;i<4*7;i++)
			{
				if(rand()%2)
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Kytka")->GetTexture(0),32,32),0,"Kytka",k,l);
				else
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Ptak")->GetTexture(0),32,32),0,"Ptak",k,l);
			}
			

			desktop->AddBackElem(tb);
			tb->AcceptEvent(GetID(),EToolBar);
		}
		if(event->sender == icons[7])	
		{	
			CGUIToolBar* tb = new CGUIToolBar(220,50,100,300,"ToolBar #2",0);
			tb->SetProperties(3,8,3,5,true,true,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
				styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

			int k,l;
			for(int i=0;i<3*8;i++)
			{
				if(rand()%2)
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Kytka")->GetTexture(0),32,32),0,"Kytka",k,l);
				else
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Ptak")->GetTexture(0),32,32),0,"Ptak",k,l);
			}

			tb->AcceptEvent(GetID(),EToolBar);
			desktop->AddBackElem(tb);
		}
		if(event->sender == icons[8])	
		{	
			CGUIToolBar* tb = new CGUIToolBar(380,50,100,300,"",0);
			tb->SetProperties(3,8,2,4,false,false,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
				styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

			int k,l;
			for(int i=0;i<3*8;i++)
			{
				if(rand()%2)
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Kytka")->GetTexture(0),32,32),0,"Kytka",k,l);
				else
					tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("Ptak")->GetTexture(0),32,32),0,"Ptak",k,l);
			}

			tb->AcceptEvent(GetID(),EToolBar);
			desktop->AddBackElem(tb);
		}
		if(event->sender == icons[9])	
		{	
			CGUIToolBar* tb = new CGUIToolBar(500,50,100,300,0,0);
			tb->SetProperties(2,5,2,5,true,false,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
				styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);


			tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0),32,32),0,"Kytka",10,10);
			tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0),32,32),0,"Kytka",10,50);
			tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EyeTB3")->GetTexture(0),32,32),0,"Kytka",10,100);
			tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("SlaviaTB3")->GetTexture(0),32,32),0,"Kytka",10,140);

			tb->SetFreeSize(63,190,60,190);

			tb->AcceptEvent(GetID(),EToolBar);
			desktop->AddBackElem(tb);
		}


	}

	if(event->sender == icons[10])	// prisla udalost od moji ikony
	{	
		// vlastni inicializacni kod "aplikace" (vytvareneho okna)
		TestBrowser(200,100);
		//TestTree(200,100);
	}

	if(event->sender == icons[11])	// prisla udalost od moji ikony
	{	

		if(event->eventID==EClicked)
		{
			if(startButtonDown)
				startButtonDown=false;
			else{
				CGUIMenu* menu = ((CGUITestDesktop*)desktop)->startMenu;
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
			//startButtonOff=false;
		}



/*
		if((!startButtonClicked && (event->eventID==EDown)) || (startButtonClicked && (event->eventID==EUp)))
		{
			CGUIMenu* menu = ((CGUITestDesktop*)desktop)->startMenu;
			float msx,msy,dsx,dsy;
			desktop->GetVPSize(dsx,dsy);
			menu->GetSize(msx,msy);
			menu->On(0,0,dsy-msy);
		}

		if(event->eventID==EDown)
			startButtonClicked=startButtonClicked ? false : true;

			*/

		/*
		if(event->eventID==EUp)
			startButtonClicked=true;
		else
			startButtonClicked=false;
			*/
	}

	
	if(event->eventID == EOff)
	{
		// vlastni inicializacni kod "aplikace" (vytvareneho okna)
		CGUIMenu* menu = ((CGUITestDesktop*)desktop)->startMenu;
		if(event->sender == menu->GetID())
		{
			CGUIEvent* e = new CGUIEvent();
			e->eventID = EEvent;
			e->sender = GetID();
			e->recipient = GetID();
			eventServer->AddEventToQueue(e);
		}
		startButtonOff=true;
	}

	if(event->eventID == EEvent)
	{
		// vlastni inicializacni kod "aplikace" (vytvareneho okna)
		CGUIMenu* menu = ((CGUITestDesktop*)desktop)->startMenu;
		if(menu->isActive()) //  && !startButtonClicked
			menu->Off();
		startButtonOff=false;
	}

	// mackani pravym tlacitkem
	if(event->eventID == EMouseButton)
	{
		if(event->sender == icons[5])	
		{	
			if(event->pInt == 1 && event->pID == 1)
				desktop->SetAllWindowVisible();
		}
	}
	
	// test toolbaru
	if(event->eventID == EToolBar)
	{
		if(event->pFloat==0)
			DebugMessage(7,"TB:%I64d> toolID:%I64d (%d,%d) zrusen",event->sender,event->pID,event->pInt2,event->pInt);
		else
			DebugMessage(8,"TB:%I64d> toolID:%I64d (%d,%d) vybran",event->sender,event->pID,event->pInt2,event->pInt);

	}

	// test tree
	if(event->eventID == ETree && event->pInt2 == 0)
	{
		//DebugMessage(7,"T:%I64d> treeItemID:%I64d index:%d",event->sender,event->pID,event->pInt);
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
		if(ti)
		{
			int depth;
			int *path = ti->GetTreePath(depth);
			//DebugMessage(8,"%d:%d:%d:%d",path[0],path[1],path[2],path[3]);
			delete[] path;

			CGUIFileBrowser* fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(event->sender));
			if(fb)
			{
				char *dir, *file;
				int ret;

				ret = fb->GetSelectedFile(event->pID,&dir,&file);
				if(ret==0)
				{
					CEDRegEdit* re = new CEDRegEdit(20,20,200,300,dir,file);
					re->AddSlider();
					desktop->AddBackElem(re);
				}
				
				//DebugMessage(6,"%d, DIR:%s FILE:%s ",ret,dir,file);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);

				char *root;
				ret = fb->GetSelectedFileRelativ(event->pID,&root,&dir,&file);

				//DebugMessage(7,"%d, ROOT:%s DIR:%s FILE:%s ",ret,root,dir,file);

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
				
			}
		}
	}



	if(event->eventID == EClose)
	{
		CGUIWindow* win = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(event->sender));
		if(win)
		{
			win->GetID();
		}
	}

	delete event;
}











//////////////////////////////////////////////////////////////////////////////////////////////////
// InitFunctions	- inicializacni kod "aplikaci"
//////////////////////////////////////////////////////////////////////////////////////////////////

int WoknoW(float x, float y)
{
	CGUITestWindow *win = 0;
	CGUIStaticText *label = 0;
	CGUIRectHost *picture = 0;
	CGUIButton *button = 0;

	try{
		CGUIRectHost *picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
		CGUITestWindow *win = new CGUITestWindow((float)x,(float)y,(float)200,(float)(rand()%120+85),"WoknoW",picture);

		//win->scrollChildWindowRather=true;

		win->AcceptEvent(((CGUITestDesktop*)desktop)->iconDefaultHandler->GetID(),EClose);
		float wx, wy;
		win->GetSize(wx,wy);
		//CGUIWindowPanel* winpanel = new CGUIWindowPanel(0,-STD_PANEL_SHIFT,styleSet->Get("Panel"),win); //wy-STD_PANEL_HEIGHT
		CGUIWindowPanel* winpanel = new CGUIWindowPanel(2,STD_TITLE_HEIGHT+2-STD_PANEL_SHIFT,styleSet->Get("Panel"),win); //wy-STD_PANEL_HEIGHT
		//winpanel->position_up=true;
		winpanel->SetAnchor(1,0,1,1);
		desktop->AddBackElem(win); //wy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT
		//desktop->AddFrontElem(win);
		CGUIButton* icon = new CGUIButton(100,100,20,27,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"Show Debug && Error Log");
		winpanel->AddIcon(icon,-2);
		

		//CGUIContextHelp *tt = new CGUIContextHelp(70,70,200,0,"ToTo je Kontext Help !!! M.D. je kámoš >/}",TIMES,8,false);
		//desktop->AddBackElem(tt);

		CGUIEditWindow *editbox = new CGUIEditWindow(20,175,100);
		editbox->SetTabOrder(2);
		editbox->SetMark(true);

		CGUIStaticText *label = new CGUIStaticText(0,0,"OK",TIMES,14);
		picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
		CGUIButton *button = new CGUITestButton(15,40,70,70,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),label,picture,"Funkèní nápovìda,\nže jo?");
		button->SetTabOrder(3);
		button->SetMark(true);


		const int (*events);
		int num;
		button->GetAvailableEvents(&events,&num);
		win->program = new CGUIMyTestProg();
		//button->AcceptAllEvents(win->program->GetID());
		button->AcceptEvent(win->program->GetID(),EClicked);
		button->AcceptEvent(win->program->GetID(),EMouseOver);
		button->AcceptEvent(win->program->GetID(),EMouseButton);
		button->AcceptEvent(win->program->GetID(),EUp);
		button->AcceptEvent(win->program->GetID(),EDown);
//		textWord2->BindAttribute(win->program->string,dtString,20);
//		win->program->textWord=textWord2;
//		textWord2->AcceptEvent(win->program->GetID(),EOk);
//		textWord2->AcceptEvent(win->program->GetID(),ECancel);

		editbox->BindAttribute(win->program->string,dtString,20);
		editbox->AcceptEvent(win->program->GetID(),EOk);
		win->program->button=button;



//		button->SetAnchor(1,1,1,1);
		win->AddBackElem(button);
		//button = new CGUITestButton(50,60,70,70,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"JoJo");
		//win->AddBackElem(button);
//			picture->Rotate(90);


		CGUICheckBox* cb = new CGUICheckBox(100,50,new CGUIStaticText("CheckBox",(CGUIFont*)RefMgr->Find("GUI.F.Times.12"),0,0),0);
		win->AddBackElem(cb);
		//cb->Move(150,100);

		float nsx, nsy;
		win->GetBackWindowNormalSize(nsx,nsy);
		//CGUIStaticPicture* panel = new CGUIStaticPicture(0,0,"$TEX$\\Panel\\panel.png",nsx,25);
		//panel->full_window_anchor=true;
		//panel->SetAnchor(1,0,1,1);
		//win->AddBackElem(panel);

		//picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
		//CGUIWinPanelButton* panelButton = new CGUIWinPanelButton(30,3,100,20,styleSet->Get("PanelButton_Up"),styleSet->Get("PanelButton_UpMark"),styleSet->Get("PanelButton_Down"),styleSet->Get("PanelButton_DownMark"),true,"A ja su malá céreèka, ja som mala syneèka!",picture);
		//win->AddBackElem(panelButton);

		CGUIStaticText *md = new CGUIStaticText(0,40,196,"M.D. je dobrej BugLocator!",TIMES,8, false, aLeft, 0xFFFFE13B );
		win->AddBackElem(md);
		md = new CGUIStaticText(0,120,196,"M.D. je dobrej BugLocator!",TIMES,9,false, aRight, 0xFFFFE13B);
		win->AddBackElem(md);
		//md = new CGUIStaticText(0,80,196,"M.D. je dobrej BugLocator!",TIMES,10,false,aCenter, 0xFFFFE13B);
		//win->AddBackElem(md);
		

/*		int pint[3] = {1,2,3};
		int iii = 0;
		char pstring[] = "Ahoj";
		md->SetElementAttributes(pint,dtInt,3);
		md->BindAttribute(&iii,dtInt,1,0);
		md->Sync(true);
*/

		win->AddBackElem(editbox);

		desktop->SetFocusEl(win,0);

		CGUITestWindow *win2 = new CGUITestWindow((float)100,(float)100,(float)100,(float)100,"TestWindow",0);
		//win->autogrowing = false;
    	win2->SetBackWindowNormalSize(101,101);
		win2->SetWindowSizeBWNormal();
		win2->SetWindowLimitSizesBW(0,0,120,120,false);
		win->AddBackElem(win2);	
		
		//win2->ResizeRel(10,10);
		//win2->ResizeRel(-10,-10);

		button = new CGUITestButton(10,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSize 150 150");
		win2->AddBackElem(button);

		button = new CGUITestButton2(60,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSizeNormal");
		win2->AddBackElem(button);

		//DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);
		//DebugMessage(4,"%s",_ctime64( &ltime ));

		CGUILine* l;

		l = new CGUILine(10,10,100,10);
		l->SetAnchor(1,0,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(100,10,100,100);
		l->SetAnchor(1,1,0,1);
		win2->AddBackElem(l);

		l = new CGUILine(100,100,10,100);
		l->SetAnchor(0,1,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(10,100,10,10);
		l->SetAnchor(1,1,1,0);
		win2->AddBackElem(l);


		

		l = new CGUILine(10,10,100,100);
		l->SetAnchor(1,1,1,1);
		win2->AddBackElem(l);

		l = new CGUILine(10,100,100,10);
		l->SetAnchor(1,1,1,1);
		win2->AddBackElem(l);

		win->SetFocusEl(win->FindTABElement(0,false),0);


		win->SetBackWindowNormalSize(200,200);
		win->SetBWLimitSizes(0,0,200,200,0,1);
		win->SetBackWindowNormalSize(250,250);
		//win->SetWindowLimitSizesBW(10,10,100,100);
		
		win->SetWindowSizeBWNormal();
		win->SetBWLimitSizes(0,0,200,200,0,1);
		win->SetWindowSizeBWNormal();

		//win->title->SetMovable(false);

		//win->SetBackWindowNormalSize(150,150);
		//win->SetWindowSizeBWNormal();

//		win->SetBackWindowNormalSize(70,70);
//		win->SetWindowSizeBWNormal();

//		win->SetFocusEl(win2,0);
		//SetFocusEl(win2,1);

//		win->SetFocusEl(editbox,false);

		//CGUIStaticPicture* sp = new CGUIStaticPicture(300,300,"$TEX$\\Panel\\panel.png",25,25);
		//panel->full_window_anchor=true;
		//panel->SetAnchor(1,0,1,1);
		//win->AddBackElem(sp);


		//win->GetBackWindowNormalSize(wx,wy);
		//CGUIWindowPanel* winpanel = new CGUIWindowPanel(0,wy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT,styleSet->Get("Panel"),win); //wy-STD_PANEL_HEIGHT
	}catch(CExc)
	{
		if(win && button)
			win->RemoveBackElem(button);
		SAFE_DELETE(label);
		SAFE_DELETE(picture);
		SAFE_DELETE(button);
		if(win)
			desktop->RemoveFrontElem(win);
		SAFE_DELETE(win);
		throw;
	}

	return 0;				// wow I eat this message (and it was so good)
}

int TestWindow(float x, float y)
{
	CGUITestWindow *win = 0;
	CGUIStaticPicture *picture = 0;

	try{
		CGUIRectHost *icon = new CGUIRectHost(0,0,"$TEX$\\s1.png");
		CGUITestWindow *win = new CGUITestWindow((float)x,(float)y,(float)100,(float)100,"TestWindow",icon);
		//win->autogrowing = false;
    	win->SetBackWindowNormalSize(101,101);
		//win->SetLimitSizes(0,0,101,101);
		win->SetWindowLimitSizesBW(0,0,130,130,false);
		win->SetWindowSizeBWNormal();
		//win->onWindowPanel=false;
		desktop->AddBackElem(win);	
		
		//win->ResizeRel(10,10);
		//win->ResizeRel(-10,-10);

		CGUIButton *button = new CGUITestButton(10,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSize 150 150");
		win->AddBackElem(button);

		button = new CGUITestButton2(60,10,30,30,styleSet->Get("ButtonUp"),styleSet->Get("ButtonDown"),styleSet->Get("ButtonMark"),0,0,"Nastavi BackWindowSizeNormal");
		win->AddBackElem(button);

		//DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);
		//DebugMessage(4,"%s",_ctime64( &ltime ));

		CGUILine* l;

		l = new CGUILine(10,10,100,10);
		l->SetAnchor(1,0,1,1);
		win->AddBackElem(l);

		l = new CGUILine(100,10,100,100);
		l->SetAnchor(1,1,0,1);
		win->AddBackElem(l);

		l = new CGUILine(100,100,10,100);
		l->SetAnchor(0,1,1,1);
		win->AddBackElem(l);

		l = new CGUILine(10,100,10,10);
		l->SetAnchor(1,1,1,0);
		win->AddBackElem(l);


		

		l = new CGUILine(10,10,100,100);
		l->SetAnchor(1,1,1,1);
		win->AddBackElem(l);

		l = new CGUILine(10,100,100,10);
		l->SetAnchor(1,1,1,1);
		win->AddBackElem(l);


		CGUIButton *lbutton = new CGUIButton((float)60,(float)60,14,14,styleSet->Get("Slavia1"),styleSet->Get("Slavia2"),styleSet->Get("Slavia3"),false);
		win->AddBackElem(lbutton);

//		win->SetBackWindowNormalSize(70,70);
//		win->SetWindowSizeBWNormal();

		desktop->SetFocusEl(win,0);

		return 0;
	}catch(CExc)
	{
		if(win)
			desktop->RemoveFrontElem(win);
		SAFE_DELETE(win);
		throw;
	}

	return 0;
}


int TestEdit(float x, float y)
{
	CGUIEditWindow *win = 0;
	//CGUIStaticPicture *picture = 0;

	try{
		CGUIRectHost *icon = new CGUIRectHost(0,0,"$TEX$\\s1.png");
		
		//win = new CGUIEditWindow((float)x,(float)y,(float)200,(float)200,1,"TestWindow",icon);
		CDXFont* font;
		DWORD col;
		DXHighLightFormat->GetDefault(font,col);
		win = new CGUIEditWindow((float)x,(float)y,(float)200,(float)200,0,0,"TestWindow",icon,STD_RESIZER_COLOR,col,0xAA0000AA,font);
		win->SetHighlight(1);

		desktop->AddBackElem(win);	
		
		desktop->SetFocusEl(win,0);

		return 0;
	}catch(CExc)
	{
		if(win)
			desktop->RemoveFrontElem(win);
		SAFE_DELETE(win);
		throw;
	}

	return 0;
}

int TestBrowser(float x, float y)
{
	CGUIFileBrowser* fb = new CGUIFileBrowser(x,y,200,200);
	fb->AddRootItem("$KRKAL$","Krkal:");
	fb->AddRootItem("$SCRIPTS$","Skriptíky:");
	fb->AddRootItem("$LEVELS$","Levlíky:");
	fb->AddRootItem("e:\\");
	fb->AddRootItem("e:\\Films\\s.txt");
	//fb->AddRootItem("e:\\Films\\ss.txt");
	fb->UpdateTree();
	desktop->AddBackElem(fb);
	fb->FocusMe();
	
	fb->AcceptEvent(((CGUITestDesktop*)desktop)->iconDefaultHandler->GetID(),ETree);

	return 0;
}

int TestTree(float x, float y)
{
	CGUITree* t = new CGUITree(x,y,140,200,"Stromeèek",new CGUIRectHost(0,0,"$TEX$\\strom.png"));
	desktop->AddBackElem(t);
	CGUITreeItem* tree = t->AddTreeItem(0,"Koøen stromu",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Prostì koøen!!!");

	t->AcceptEvent(((CGUITestDesktop*)desktop)->iconDefaultHandler->GetID(),ETree);

	CGUITreeItem* a[5];
	a[0] = tree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem první, heè!");
	CGUITreeItem* subtree = a[1] = tree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Jsem druhý!");
	a[2] = tree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem tøetí, co se dá dìlat");
	a[3] = tree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem poslední :-(((");

	
	a[4] = tree->AddTreeItem(subtree->GetID(),"za 1 (1)",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"za 2");
	
	tree->AddTreeItem(a[0]->GetID(),"za 0 (2)",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"za 2");
	tree->AddTreeItem(a[4]->GetID(),"za (1) (3)",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"za 2");
	tree->AddTreeItem(a[1]->GetID(),"za 1 (4)",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"za 2");



	subtree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	CGUITreeItem* subtree2 = subtree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)));
	subtree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));

	subtree2->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree2->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree2->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree2->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));

	CGUITestWindow *win = new CGUITestWindow((float)x,(float)y,(float)200,(float)(rand()%120+85),"WoknoW",0);
	subtree2->SetItemElement(win,false);
	//subtree2->UpdateTree();

	tree = t->AddTreeItem(0,"Koøen stromu",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Prostì koøen!!!");

	tree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem první, heè!");
	subtree = tree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Jsem druhý!");
	tree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem tøetí, co se dá dìlat");
	tree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem poslední :-(((");

	subtree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	subtree2 = subtree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)));
	subtree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));

	CGUITreeItem* subtree3 = subtree2->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	CGUITreeItem* subtree4 = subtree2->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	CGUITreeItem* subtree5 = subtree2->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	CGUITreeItem* subtree6 = subtree2->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));

	CGUIStaticText *label = new CGUIStaticText(0,0,"OK",TIMES,14);
	//CGUIRectHost* picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
	CGUIButton *button = new CGUITestButton(0,0,70,STD_TREE_HEIGHT,styleSet->Get("GreyButtonUp"),styleSet->Get("GreyButtonDown"),styleSet->Get("GreyButtonUp"),label,0,"Funkèní nápovìda,\nže jo?");
	subtree3->SetItemElement(button,true);

	label = new CGUIStaticText(0,0,"OK",TIMES,14);
	//picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
	button = new CGUITestButton(0,0,70,STD_TREE_HEIGHT,styleSet->Get("GreyButtonUp"),styleSet->Get("GreyButtonDown"),styleSet->Get("GreyButtonUp"),label,0,"Funkèní nápovìda,\nže jo?");
	subtree4->SetItemElement(button,true);

	label = new CGUIStaticText(0,0,"OK",TIMES,14);
	//picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
	button = new CGUITestButton(0,0,70,STD_TREE_HEIGHT,styleSet->Get("GreyButtonUp"),styleSet->Get("GreyButtonDown"),styleSet->Get("GreyButtonUp"),label,0,"Funkèní nápovìda,\nže jo?");
	subtree5->SetItemElement(button,true);

	label = new CGUIStaticText(0,0,"OK",TIMES,14);
	//picture = new CGUIRectHost(0,0,"$TEX$\\krkal.png");
	button = new CGUITestButton(0,0,70,STD_TREE_HEIGHT,styleSet->Get("GreyButtonUp"),styleSet->Get("GreyButtonDown"),styleSet->Get("GreyButtonUp"),label,0,"Funkèní nápovìda,\nže jo?");
	subtree6->SetItemElement(button,true);


	t->UpdateTree();

	return 0;
}
/*
int TestTree(float x, float y)
{
	typeID subID;
	CGUIStdWindow* win = new CGUIStdWindow(x,y,140,200,"Stromeèek",new CGUIRectHost(0,0,"$TEX$\\strom.png"),STD_TREE_BORDER_COLOR,1,STD_TREE_BGCOLOR_COLOR);
	CGUITreeItem* tree = new CGUITreeItem(win,10,10,"Koøen stromu",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Prostì koøen!!!");
	//tree->rootTree=true;
	win->AddBackElem(tree);

	desktop->AddBackElem(win);

	tree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem první, heè!");
	subID = tree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)),"Jsem druhý!")->GetID();
	tree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem tøetí, co se dá dìlat");
	tree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)),"Jsem poslední :-(((");

	CGUITreeItem* subtree = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(subID));
	if(subtree)
	{
		subtree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		subtree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		subID = subtree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB1")->GetTexture(0)))->GetID();
		subtree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	}

	subtree = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(subID));
	if(subtree)
	{
		subtree->AddTreeItem(0,"Prvòáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		subtree->AddTreeItem(0,"Druhák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		subtree->AddTreeItem(0,"Tøeáèek",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
		subtree->AddTreeItem(0,"Ètvrák",new CGUIRectHost(0,0,styleSet->Get("KrkalTB2")->GetTexture(0)));
	}



return 0;
}
*/

