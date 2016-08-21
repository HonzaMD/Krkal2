///////////////////////////////////////////////
//
// editor.cpp
//
// Implementation pro desktop editoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#include "stdafx.h"
#include "desktop.h"
#include "editor.h"
#include "ObjProperty.h"
#include "menu.h"
#include "editorMenu.h"
#include "textedit.h"
#include "editorLevelWin.h"

#include "kernel.h"
#include "KerErrors.h"
#include "dx.h"
#include "autos.h"

#include "reseditor.h" //!!! jen pro testovani
#include "dialogs.h" //!!! jen pro testovani

//////////////////////////////////////////////////////////////////////
// CEDEditorDesktop
//////////////////////////////////////////////////////////////////////

CEDEditorDesktop::CEDEditorDesktop()
	: CGUIDesktop()
{
	try{
		gameModIndicator=0;
		RemoveFromTrash();
		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR|MsgKeyboard|MsgKeyboardUp|MsgMouseOver|MsgTimer);
		// MsgSetProduce(MsgAll);

		// test vyjimek pri konstrukci desktopu>
		//throw CExc(eGUI, E_FILE_NOT_FOUND,"CGUIStaticPicture::CGUIStaticPicture> Cannot find texture:");
		
		CGUIWindowPanel* panel = new CGUIWindowPanel(0,sy-STD_PANEL_HEIGHT-STD_PANEL_SHIFT,styleSet->Get("Panel"),this,true,false,0,0);
													//0,-STD_PANEL_SHIFT (pridani nahoru)
		iconDefaultHandler = new CEDDesktopIconHandler();
		iconDefaultHandler->icons = new typeID[9];		// zadat velikost pole dle poctu icon (polozek typeID)

		editorMenuHandler = new CEDEditorMenuHandler(7);

		levelWindowDX=levelWindowDY=0;

		new CEDResourceEditor;

		//AddContextMenu();
		AddStartMenu();

		InitDefaultIcons();

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
		throw CExc(eGUI, E_INTERNAL,"CEDEditorDesktop::CEDEditorDesktop> Fatal error - cannot creat DESKTOP");
	}
}

CEDEditorDesktop::~CEDEditorDesktop()
{
	RemoveFromTrash();
	timerGUI->DeleteAllRequests(this);
	SAFE_DELETE(iconDefaultHandler);
	SAFE_DELETE(editorMenuHandler);
	SAFE_DELETE(resourceEditor);
}

void CEDEditorDesktop::AddStartMenu()
{
	CGUIMenu* menu;
	CGUIMenuItem* mi;

	menu = new CGUIMenu(260);
	menu->SetPictureStripSize(44);
	menu->SetMenuPictureSize(40);

	mi = new CGUIMenuItem(menu, 0, 0, 260, 40, "en{{Quit}}cs{{Konec}}", new CGUIRectHost(0, 0, styleSet->Get("LebkyTB3")->GetTexture(0)));
	CGUIFont *font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("en{{Quit}}cs{{Konec}}",font);
	mi->SetShortCut("F4",false,false,false,VK_F4,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[2]=mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 40, "en{{Script Editor}}cs{{Editor skriptù}}", new CGUIRectHost(0, 0, styleSet->Get("StartMenu_Script")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("en{{Script Editor}}cs{{Editor skriptù}}",font);
	mi->SetShortCut("F1",false,false,false,VK_F1,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[3]=mi->GetID();


	mi = new CGUIMenuItem(menu, 0, 0, 260, 40, "en{{Restart Editor}}cs{{Restartni editor}}", new CGUIRectHost(0, 0, styleSet->Get("StartMenu_Level")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,13);
	mi->SetLabelFormated("en{{Restart Editor}}cs{{Restartni editor}}",font);
	mi->SetShortCut("F2",false,false,false,VK_F2,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[0]=mi->GetID();


	mi = new CGUIMenuItem(menu, 0, 0, 260, 60, "en{{Game}}cs{{Hra}}", new CGUIRectHost(0, 0, styleSet->Get("StartMenu_Game")->GetTexture(0)));
	font = new CGUIFont();
	font->CreateFont(ARIAL,20);
	mi->SetLabelFormated("en{{Game}}cs{{Hra}}",font);
	mi->SetShortCut("F3",false,false,false,VK_F3,true);
	font->Release();
	menu->AddMenuItem(0,mi);

	mi->AcceptEvent(mainGUI->handler->GetID(),EClicked);
	mainGUI->handler->icons[1]=mi->GetID();


	font = new CGUIFont();
	font->CreateFont(ARIAL, 13);


	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Level Properties}}cs{{Vlastnosti levelu}}", new CGUIRectHost(0, 0, styleSet->Get("BUT_NameBrowser_Up")->GetTexture(0)));
	mi->SetLabelFormated("en{{Level Properties}}cs{{Vlastnosti levelu}}", font);
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[6] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{New Level}}cs{{Nový level}}", new CGUIRectHost(0, 0, styleSet->Get("LM_NewLevel")->GetTexture(0)));
	mi->SetLabelFormated("en{{New Level}}cs{{Nový level}}", font);
	mi->SetShortCut("Ctrl+N", true, false, false, 'N');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[0] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Load Level}}cs{{Nahraj level}}", new CGUIRectHost(0, 0, styleSet->Get("LM_LoadLevel")->GetTexture(0)));
	mi->SetLabelFormated("en{{Load Level}}cs{{Nahraj level}}", font);
	mi->SetShortCut("Ctrl+L", true, false, false, 'L');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[1] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Save Level}}cs{{Ulož level}}", new CGUIRectHost(0, 0, styleSet->Get("BUT_Save_Up")->GetTexture(0)));
	mi->SetLabelFormated("en{{Save Level}}cs{{Ulož level}}", font);
	mi->SetShortCut("Ctrl+S", true, false, false, 'S');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[2] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Save As ...}}cs{{Ulož jako ...}}", new CGUIRectHost(0, 0, styleSet->Get("BUT_Save_Up")->GetTexture(0)));
	mi->SetLabelFormated("en{{Save As ...}}cs{{Ulož jako ...}}", font);
	mi->SetShortCut("Ctrl+Shift+S", true, false, true, 'S');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[3] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Clear Level}}cs{{Vyprázdni level}}", new CGUIRectHost(0, 0, styleSet->Get("LM_ClearLevel")->GetTexture(0)));
	mi->SetLabelFormated("en{{Clear Level}}cs{{Vyprázdni level}}", font);
	mi->SetShortCut("Ctrl+Shift+N", true, false, true, 'N');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[4] = mi->GetID();

	mi = new CGUIMenuItem(menu, 0, 0, 260, 30, "en{{Close Level}}cs{{Zavøi level}}", new CGUIRectHost(0, 0, styleSet->Get("LM_CloseLevel")->GetTexture(0)));
	mi->SetLabelFormated("en{{Close Level}}cs{{Zavøi level}}", font);
	mi->SetShortCut("Ctrl+X", true, false, false, 'X');
	menu->AddMenuItem(0, mi);
	mi->AcceptEvent(editorMenuHandler->GetID(), EClicked);
	editorMenuHandler->icons[5] = mi->GetID();

	font->Release();

	startMenu=menu;

}



void CEDEditorDesktop::CloseAllObjPropertyWins()
{
	CGUIElement* e;
	CEDObjectProperty* op;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)) && (op=dynamic_cast<CEDObjectProperty*>(e)))
		{
			CGUIListElement<CGUIElement>* oldle=le;
			le=queue_back.GetNextListElement(le);

			queue_back.RemoveListElement(oldle);
			e->RemoveFromEngine();
			e->SetParent(0);
			if(e==last_element_under_cursor)
				last_element_under_cursor=0;
			if(e==focus_elm)
				focus_elm=0;
			delete e;

		}else
			le=queue_back.GetNextListElement(le);
	}
}

CEDObjectProperty* CEDEditorDesktop::FindObjPropertyWin(OPointer obj)
{
	CGUIElement* e;
	CEDObjectProperty* op;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)) && (op=dynamic_cast<CEDObjectProperty*>(e)))
		{
			if(op->obj==obj)
				return op;
		}
		le=queue_back.GetNextListElement(le);
	}
	return 0;
}

CEDObjectProperty* CEDEditorDesktop::FindAnyObjPropertyWin(OPointer obj)
{
	CGUIElement* e;
	CEDObjectProperty* op;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)) && (op=dynamic_cast<CEDObjectProperty*>(e)))
		{
			if(op->obj!=obj)
				return op;
		}
		le=queue_back.GetNextListElement(le);
	}
	return 0;
}


int CEDEditorDesktop::MouseOver(float x, float y, UINT over,UINT state)
{
	bool scroll=false;

	CEDLevelWindow* ID(lw,editor->gameWindowID);
	if(!lw || lw->scrolling)
		return 1;

	if(x<STD_GAMEWIN_SCROLLING_LIMIT)
	{
		scroll=true;
		levelWindowDX=-STD_GAMEWIN_SCROLLING_SHIFT;
	}
	else if(x>sx-STD_GAMEWIN_SCROLLING_LIMIT)
	{
		scroll=true;
		levelWindowDX=+STD_GAMEWIN_SCROLLING_SHIFT;
	}
	else
		levelWindowDX=0;


	if(y<STD_GAMEWIN_SCROLLING_LIMIT)
	{
		scroll=true;
		levelWindowDY=-STD_GAMEWIN_SCROLLING_SHIFT;
	}
	else if(y>sy-STD_GAMEWIN_SCROLLING_LIMIT)
	{
		scroll=true;
		levelWindowDY=+STD_GAMEWIN_SCROLLING_SHIFT;
	}
	else
		levelWindowDY=0;

	if(scroll && !timerGUI->ContainRequest(this))
		timerGUI->AddRequest(this,STD_GAMEWIN_SCROLLING_TIMER);

	return 1;
}

int CEDEditorDesktop::TimerImpulse(typeID timerID, float time)
{
	float vx,vy,nx,ny;

	CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
	if(!lw || lw->scrolling)
		return 0;

	lw->GetVPPos(vx,vy);
	lw->SetWindowPositionVP(levelWindowDX,levelWindowDY);
	lw->GetVPPos(nx,ny);

	if(nx!=vx || ny!=vy)
	{
		timerGUI->AddRequest(this,STD_GAMEWIN_SCROLLING_TIMER);
	}
	else
	{
		levelWindowDX=0;
		levelWindowDY=0;
	}

	return 0;
}

int CEDEditorDesktop::Keyboard(UINT character, UINT state)
{
	int ret = CGUIDesktop::Keyboard(character,state);

	if(ret)
	{
		if(character == VK_F2 && mainGUI->key_shift)	// nastartuj editor v opacnem gamemodu, nez je nastaven ted
		{
			CGUIEvent* event = new CGUIEvent();
			event->eventID = EClicked;
			event->sender = mainGUI->handler->icons[0];	// jako polozka start menu - Load Level
			event->recipient = mainGUI->handler->GetID(); // spravce start menu
			eventServer->AddEventToQueue(event);

			ret = 0;
		}
	}

	if(ret)
	{
		CGUIElement* focEl = GetFocusEl();
		CGUIElement* el1 = GetObjFromID<CGUIToolBar>(editor->globalObjsTB);
		CGUIElement* el2 = GetObjFromID<CGUIWindow>(editor->gameWindowID);
		CEDObjectProperty* el3 = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
		CGUIElement* el4 = GetObjFromID<CGUITree>(editor->cellObjectsListID);
		CGUIElement* el5 = GetObjFromID<CGUIToolBar>(editor->propertyTB);
		CGUIElement* el6 = GetObjFromID<CGUIWindow>(editor->debugWindowID);
		CGUIElement* el7 = GetObjFromID<CGUITree>(editor->objVarSelectionID);
		CGUIElement* el8 = GetObjFromID<CGUIToolBar>(editor->noConnectDialogID);

		bool processed=false;
		if(el3 && el3!=focEl && (!focEl  || !focEl->exclusive_keyboard))
		{
			switch(character)
			{
				case VK_DELETE :
					if(el3)
						el3->DeleteObject();
					processed=true;
					break;
			}
		}

		if(!processed)
			processed = ResolveKeyShortCuts(character);

		if(!processed)
		{
			if(!focEl || (el1==focEl) || (el2==focEl) || (objectList==focEl) || (el4==focEl) || (el5==focEl) || (el6==focEl) || (el7==focEl) || (el8==focEl))
			{
				editor->ResolveEditMod(character);
			}
			else if(el3==focEl)
			{
				bool editText = false;
				if(!dynamic_cast<CGUIEditWindow*>(el3->GetFocusEl()))
				{
					if(CGUIWindow* win = dynamic_cast<CGUIWindow*>(el3->GetFocusEl()))
					{
						if(!dynamic_cast<CGUIEditWindow*>(win->GetFocusEl()))
							editText=false;
						else
							editText=true;
					}
					else
						editText=false;
				}
				else
					editText=true;

				if(editText)
					editor->ResolveEditMod(0);
				else
				{
					if(character == VK_DELETE)
						el3->DeleteObject();
					else
						editor->ResolveEditMod(character);
				}
			}
		}

	}

	return ret;
}

int CEDEditorDesktop::KeyboardUp(UINT character, UINT state)
{
	int ret = CGUIDesktop::KeyboardUp(character,state);

	if(ret)
	{
		editor->ResolveEditMod(0);
	}

	return ret;
}

bool CEDEditorDesktop::ResolveKeyShortCuts(UINT character)
{
	switch(character)
	{
		case 'P' :
			if(mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
			{
				if(resourceEditor)
					resourceEditor->ShowPackageBrowser();

				return true;
			}
			break;

		case 'W' :
			if(mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
			{
				if(KerErrorFiles)
					KerErrorFiles->ShowAllLogs(0,40,640,200);

				return true;
			}
			break;

		case 'G' :
			if(mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
			{
				if(editor)
				{
					editor->CreateDebugWindow();
					editor->RefreshMapPosition();
				}

				return true;
			}
			break;

		case 'A' :
			if(mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
			{
				iconDefaultHandler->ShowAutoSelectFileDlg();
				return true;
			}
			else if(mainGUI->key_ctrl && !mainGUI->key_alt && mainGUI->key_shift)
			{
				if(KerMain && KerMain->AutosMain->SaveChangedAutos())
					CEDUserAnnouncer::Announce("A saved successful",5);

				return true;
			}

			break;

	}



	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CEDEditorDesktop::InitDefaultIcons	- inicializace ikon na desktopu a panelu pri startu editoru
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDEditorDesktop::InitDefaultIcons()
{
	CGUIButton *icon=0;

	((CEDEditorDesktop*)desktop)->startMenu->AcceptEvent(iconDefaultHandler->GetID(),EOff);

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

	//if(desktop->panel)
	//{
	//	desktop->panel->AddGap(10);
	//	CGUIButton* icon = new CGUIButton(100,100,20,20,styleSet->Get("LevelMenuTB1"),styleSet->Get("LevelMenuTB2"),styleSet->Get("LevelMenuTB3"),false,0,0);
	//	if(!desktop->panel->AddIcon(icon,4))
	//	{
	//		iconDefaultHandler->icons[6] = icon->GetID();
	//		icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
	//		icon->AcceptEvent(iconDefaultHandler->GetID(),EDown);
	//		icon->AcceptEvent(iconDefaultHandler->GetID(),EUp);
	//	}else
	//		delete icon;
	//}


	if(desktop->panel)
	{
		desktop->panel->AddGap(10);
		icon = desktop->panel->AddIcon(styleSet->Get("AutoDlgTB1"),styleSet->Get("AutoDlgTB1"),styleSet->Get("AutoDlgTB3"),"Set A file");
		if(icon)
		{
			iconDefaultHandler->icons[7] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("AutoSaveTB1"),styleSet->Get("AutoSaveTB1"),styleSet->Get("AutoSaveTB3"),"Save A file");
		if(icon)
		{
			iconDefaultHandler->icons[8] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
			icon->Deactivate();
		}
	}


	if(desktop->panel)
	{
		desktop->panel->AddGap(10);
		//CGUIButton* icon = new CGUIButton(100,100,STD_PANEL_ICON_SIZE,STD_PANEL_ICON_SIZE,styleSet->Get("SlaviaTB1"),styleSet->Get("SlaviaTB2"),styleSet->Get("SlaviaTB3"),false,0,0,"Show Debug && Error Log");
		CGUIButton* icon = new CGUIButton(100,100,20,27,styleSet->Get("EyeTB1"),styleSet->Get("EyeTB2"),styleSet->Get("EyeTB3"),false,0,0,"en{{Show Debug && Error Log}}cs{{Zobraz Debug && Error Log}}");
		icon->SetInactiveStyle(styleSet->Get("EyeTB4"));
		if(!desktop->panel->AddIcon(icon,-2))
		{
			iconDefaultHandler->icons[1] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}else
			delete icon;
	}

	if(desktop->panel)
	{
		icon = desktop->panel->AddIcon(styleSet->Get("KrkalTB1"),styleSet->Get("KrkalTB2"),styleSet->Get("KrkalTB3"),"en{{Show RunLogWindow}}cs{{Zobraz RunLogWindow}}");
		if(icon)
		{
			iconDefaultHandler->icons[3] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}
	}


	if(desktop->panel)
	{
		desktop->panel->AddGap(10);
		CGUIButton* icon = new CGUIButton(0,0,-1,-1,"GUI.But.Package","",0,"Package Browser");
		if(!desktop->panel->AddIcon(icon,6))
		{
			iconDefaultHandler->icons[2] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
		}
	}

	if (desktop->panel)
	{
		desktop->panel->AddGap(10);
		icon = desktop->panel->AddIcon(styleSet->Get("EditorModTB1"), styleSet->Get("EditorModTB2"), styleSet->Get("EditorModTB3"), "EditorMod Settings");
		if (icon)
		{
			iconDefaultHandler->icons[4] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(), EClicked);
		}
	}

	if (desktop->panel)
	{
		CGUIButton* icon = new CGUIButton(100, 100, 40, 13, styleSet->Get("GlobalObjsTB1"), styleSet->Get("GlobalObjsTB2"), styleSet->Get("GlobalObjsTB3"), false, 0, 0, "GlobalObjects ToolBar");
		if (!desktop->panel->AddIcon(icon, 8))
		{
			iconDefaultHandler->icons[5] = icon->GetID();
			icon->AcceptEvent(iconDefaultHandler->GetID(), EClicked);
		}
		else
			delete icon;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CGUIEditorIconHandler	- vyvolani programu/oken po stisknuti ikony (spusteni aplikace -> inicializacni kod)
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDDesktopIconHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	

	// Obsluha START Menu ---------------------------------------------
	// finta pro zavreni menu pri opetovnem stisku tlacitka menu
	if(event->sender == icons[0])	// prisla udalost od moji ikony
	{	

		if(event->eventID==EClicked)
		{
			if(startButtonDown)
				startButtonDown=false;
			else{
				CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->startMenu;
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
		CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->startMenu;
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
		CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->startMenu;
		if(menu->isActive()) //  && !startButtonClicked
		{
			menu->Off();
		}
		startButtonOff=false;		
	}
	// konec obsluhy START Menu ------------------------------------------


	// Obsluha Editor Menu ---------------------------------------------
	// finta pro zavreni menu pri opetovnem stisku tlacitka menu
	//if(event->sender == icons[6])	// prisla udalost od moji ikony
	//{	

	//	if(event->eventID==EClicked)
	//	{
	//		if(editButtonDown)
	//			editButtonDown=false;
	//		else{
	//			CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->editorMenu;
	//			float msx,msy,dsx,dsy;
	//			desktop->GetVPSize(dsx,dsy);
	//			menu->GetSize(msx,msy);
	//			CGUIButton* b =  GetObjFromID<CGUIButton>(event->sender);
	//			float bx=0,by=0;
	//			desktop->GetDesktopPos(b,bx,by);
	//			menu->On(0,bx,dsy-msy-10);
	//		}
	//	}

	//	if(event->eventID==EDown)
	//	{
	//		if(editButtonOff)
	//			editButtonDown=true;
	//	}
	//}

	//if(event->eventID == EOff)
	//{
	//	CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->editorMenu;
	//	if(event->sender == menu->GetID())
	//	{
	//		CGUIEvent* e = new CGUIEvent();
	//		e->eventID = EEvent;
	//		e->sender = GetID();
	//		e->recipient = GetID();
	//		eventServer->AddEventToQueue(e);
	//		editButtonOff=true;
	//	}
	//}

	//if(event->eventID == EEvent)
	//{
	//	// vlastni inicializacni kod "aplikace" (vytvareneho okna)
	//	CGUIMenu* menu = ((CEDEditorDesktop*)desktop)->editorMenu;
	//	if(menu->isActive()) //  && !startButtonClicked
	//	{
	//		menu->Off();
	//	}
	//	editButtonOff=false;		
	//}
	// konec obsluhy Editor Menu ------------------------------------------


	if(event->sender == icons[1]) {	// zobrazi Full Log
			KerErrorFiles->ShowAllLogs(0,40,640,200);
	}

	if(event->sender == icons[2])
	{
		if(resourceEditor)
			resourceEditor->ShowPackageBrowser();
	}

	if(event->sender == icons[3]) {
		editor->CreateDebugWindow();
		editor->RefreshMapPosition();
	}

	if(event->sender == icons[4])
	{
		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(editor->propertyTB));
		if(tb)
			tb->UnMinimize();
		else
		{
			editor->CreateEditorPropertyWindow();
			editor->RefreshMapPosition();
		}
	}

	if(event->sender == icons[5])
	{
		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(editor->globalObjsTB));
		if(tb)
			tb->UnMinimize();
		else
		{
			editor->CreateGlobalObjectsToolBar();
			editor->RefreshMapPosition();
		}
	}

	if(event->sender == icons[7])
	{
		ShowAutoSelectFileDlg();
	}

	if(event->sender == icons[8])
	{
		if(KerMain && KerMain->AutosMain->SaveChangedAutos())
			CEDUserAnnouncer::Announce("A saved successful",5);
	}

	delete event;
}

void CEDDesktopIconHandler::ShowAutoSelectFileDlg()
{
	CEDAutoSelectFileDlg* dlg;
	dlg = GetObjFromID<CEDAutoSelectFileDlg>(autoSelectFileDlgID);
	if(dlg)
		dlg->FocusMe();
	else
	{
		dlg = new CEDAutoSelectFileDlg(100,100);
		desktop->AddBackElem(dlg);
		//desktop->SetFocusEl(dlg,0);
		autoSelectFileDlgID = dlg->GetID();
		dlg->Center(true,false);
		mainGUI->SendCursorPos();
		dlg->SetModal(1);
	}	
}

//////////////////////////////////////////////////////////////////////
// CEDUserAnnouncer
//////////////////////////////////////////////////////////////////////

CEDUserAnnouncer* CEDUserAnnouncer::announcer=0;

CEDUserAnnouncer::CEDUserAnnouncer(char* text)
: CGUIStaticText(text,(CGUIFont*)RefMgr->Find("GUI.F.Courier.18.B"),0,0,STD_ANNOUNCER_COLOR)
{
	MsgAcceptConsume(MsgTimer);

	SetPlace();
}

CEDUserAnnouncer::~CEDUserAnnouncer()
{
	timerGUI->DeleteAllRequests(this);
	CEDUserAnnouncer::announcer=0;
}


void CEDUserAnnouncer::Announce(char* text, float time)
{
	if(!announcer)
	{
		announcer = new CEDUserAnnouncer(text);
		desktop->AddFrontElem(announcer);
		if(time)
			timerGUI->AddRequest(announcer,time);
	}
	else
	{
		announcer->SetVisible(1);
		announcer->ChangeText(text);
		announcer->SetPlace();
		timerGUI->DeleteAllRequests(announcer);
		if(time)
			timerGUI->AddRequest(announcer,time);
	}
}

void CEDUserAnnouncer::Hide(float time)
{
	if(!announcer)
		return;

	if(time)
		timerGUI->AddRequest(announcer,time);
	else
		announcer->SetVisible(0);
}


int CEDUserAnnouncer::TimerImpulse(typeID timerID, float time)
{
	SetVisible(0);
	return 1;
}

void CEDUserAnnouncer::SetPlace()
{
	float px,py;

	if(editor)
	{
		float epsx,epsy;
		CGUIElement* ID(ep,editor->propertyTB);
		if(ep)
			ep->GetSize(epsx,epsy);

		px = editor->editorSettingsPosition.x + epsx + floorf((editor->editorSettingsPosition.sx - epsx - sx) / 2);
		py = editor->editorSettingsPosition.y+20;
		
		Move(px,py);
	}
}