///////////////////////////////////////////////
//
// MainMenuDesktop.cpp
//
// desktop Main Menu
//
// A: Petr Altman
//
///////////////////////////////////////////////


#include "stdafx.h"
#include "MainMenuDesktop.h"
#include "menu.h"
#include "GameMenu.h"

#include "dx.h"
#include "music.h"
#include "krkal.h"
#include "kerservices.h"
#include "version.h"
#include "comcompiler.h"
#include "md_testy.h"
#include "dialogs.h"
#include "ident.h"

#include "mmhelp.h"

CMainMenuDesktop *MMdesktop=0;

#include "genmain.h"
#include "musicmod.h"

#include "regedit.h"

char lRoot[102] = "";
char lFile[1002] = "";


//////////////////////////////////////////////////////////////////////
// CMainMenuDesktop
//////////////////////////////////////////////////////////////////////

CMainMenuDesktop::CMainMenuDesktop()
	: CGUIDesktop()
{
	try{
		assert(!MMdesktop);
		MMdesktop=this;
		iconDefaultHandler = 0;

		RemoveFromTrash();
		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR|MsgKeyboard);

		iconDefaultHandler = new CMainMenuDesktopIconHandler();

		levelsel = 0;
		MMmode = 0;

		quicksavelevel=0;

		ShowMainMenu();

		if(ME) ME->Play("$GAME$/music_65B6_399E_4613_0839.pkg/AFRIKA.mm",1,2);

		AddToTrash();
	}
	catch(CExc c)
	{
		SAFE_DELETE(iconDefaultHandler);
		mainGUI->LogError(c);

		// vypis informace o chybe na obrazovce jako DebugMessage (prozatimni reseni)
		__time64_t ltime;
		_time64( &ltime );
		DebugMessage(0,"%s : %2d:%2d - %s",_ctime64( &ltime ), c.errnum, c.param, c.errstr);

		MMdesktop=0;
		desktop=0;
		throw CExc(eGUI, E_INTERNAL,"CMainMenuDesktop::CMainMenuDesktop> Fatal error - cannot creat DESKTOP");
	}

}

CMainMenuDesktop::~CMainMenuDesktop()
{
	MMdesktop=0;
	RemoveFromTrash();
	SAFE_DELETE(iconDefaultHandler);
	SAFE_DELETE_ARRAY(quicksavelevel);
}


int CMainMenuDesktop::ShowMainMenu()
{
	SAFE_DELETE_ARRAY(quicksavelevel);

	if(MMmode==1)
	{
		if(GEnMain){
			GEnMain->DeleteAll();
			GEnMain->SetScrollShift(0,0);
		}
		KRKAL->DeleteKeyBoardMessages();
		mainGUI->cursor->SetVisible(1);
	}

	MMmode = 0;

	CGameMenu::DeleteGameMenu();

	DeleteAllBackElems();
	DeleteAllFrontElems();

	KRKAL->GetScreenRes(screenx,screeny);

	CGUIStaticPicture* back = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/back.jpg",(float)screenx,(float)screeny);
	AddBackElem(back);
	float x,y;
	CGUIStaticPicture* krkal = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/krkal.png");
	krkal->GetSize(x,y);
	
	krkal->Move((sx-x)/2,0);
	AddBackElem(krkal);
    
	CGUIButton *play = new CGUIButton(20,170,-1,-1,"GUI.But.MMPlay","",0);
	AddBackElem(play);
	iconDefaultHandler->play=play->GetID();
	play->AcceptEvent(iconDefaultHandler->GetID(),EClicked);

	if(KerServices.QuickSave)
	{
		CGUIButton *but_restart = new CGUIButton(35,230,-1,-1,"GUI.But.MMRestartLevel","",0);
		AddBackElem(but_restart);
		iconDefaultHandler->restartlevel=but_restart->GetID();
		but_restart->AcceptEvent(iconDefaultHandler->GetID(),EClicked);

		CFSRegKey *k=KerServices.QuickSave->FindKey("Level File");
		if(k)
		{
			char *levelname=k->GetDirectAccess();
			if(levelname)
				quicksavelevel=newstrdup(levelname);
		}

	}else
		iconDefaultHandler->restartlevel=0;

	CGUIButton *leveledit = new CGUIButton(20,sy-90-85,-1,-1,"GUI.But.MMEditLevel","",0,0);
	AddBackElem(leveledit);
	iconDefaultHandler->editlevel=leveledit->GetID();
	leveledit->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
	CGUIButton *scriptedit = new CGUIButton(10,sy-90,-1,-1,"GUI.But.MMEditScript","",0,0);
	AddBackElem(scriptedit);
	iconDefaultHandler->editscript=scriptedit->GetID();
	scriptedit->AcceptEvent(iconDefaultHandler->GetID(),EClicked);


	CGUIButton *help = new CGUIButton(0,0,-1,-1,"GUI.But.MMHelp","",0);
	help->GetSize(x,y);
	help->Move(sx-x-15,sy-y-100);
	AddBackElem(help);
	iconDefaultHandler->help=help->GetID();
	help->AcceptEvent(iconDefaultHandler->GetID(),EClicked);
	CGUIButton *about = new CGUIButton(0,0,-1,-1,"GUI.But.MMAbout","",0);
	about->GetSize(x,y);
	about->Move(sx-x-15,sy-y-70);
	AddBackElem(about);
	iconDefaultHandler->about=about->GetID();
	about->AcceptEvent(iconDefaultHandler->GetID(),EClicked);


	CGUIButton *exit = new CGUIButton(0,0,-1,-1,"GUI.But.MMExit","",0);
	exit->GetSize(x,y);
	exit->Move(sx-x-15,sy-y-10);
	AddBackElem(exit);
	iconDefaultHandler->exit=exit->GetID();
	exit->AcceptEvent(iconDefaultHandler->GetID(),EClicked);

	levelsel = new CMainMenuSelectLevel(190,170,sx-190-160,sy-170-10,"$GAMES$","GAMES");
	levelsel->AcceptEvent(iconDefaultHandler->GetID(),ETree);
	levelsel->MarkFileItem(lRoot,lFile);

	AddBackElem(levelsel);
	levelsel->FocusMe();
	iconDefaultHandler->levelsel=levelsel->GetID();

	int victory=KRKAL->GetGameVictory();

	if(victory==0)
	{
		CGUIStaticPicture *sp = new CGUIStaticPicture(sx-125,200,"$TEX$/MainMenu/ko.png");
		AddBackElem(sp);
	}
	else if(victory==1)
	{
		CGUIStaticPicture *sp = new CGUIStaticPicture(sx-130,200,"$TEX$/MainMenu/ok.png");
		AddBackElem(sp);

	}
	KRKAL->ResetGameVictory();

	char version[20];
	_snprintf(version,19,"v%i.%i",KRKAL_MAINVERSION, KS_VERSION);
	CGUIStaticText *stver = new CGUIStaticText(version,(CGUIFont*) RefMgr->Find("GUI.F.Arial.12"),sx-65,3,0xFFFFFF80,60,0,aRight);
	AddBackElem(stver);

	if(levelsel->GetSelectedItem())
		LevelChange( levelsel->GetSelectedItem()->GetID() );

    return 1;
}

int CMainMenuDesktop::PlayLevel(typeID levelID, int restart)
{
	GetSelectedLevel();

	char *level=GetLevelName(levelID);

	int ok=0;
	float x,y;
	
	if(level)
	{
		MMmode = 1;

		DeleteAllBackElems();
		DeleteAllFrontElems();

		mainGUI->cursor->SetVisible(0);

		CGUIStaticPicture* loading = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/loading.png");
		loading->GetSize(x,y);
		
		loading->Move((sx-x)/2,(sy-y)/2);
		AddBackElem(loading);
		
		GEnMain->AddToWindow(DXbliter->rootwnd,0);
		KRKAL->GetScreenRes(screenx,screeny);
		int gwsx,gwsy;
		gwsx=screenx;gwsy=screeny-(int)STD_GAMEMENU_HEIGHT;
		GEnMain->SetWindowPos(0,0,(float)gwsx,(float)gwsy);
		GEnMain->DeleteAll();
		GEnMain->SetTopLightDefault();

		SAFE_DELETE(KerMain);

		ok=1;

		KRKAL->ResetGameVictory(0);

		KRKAL->Redraw();

		DeleteBackElem(loading);

		int showerr=0;
		int load=0;

		try{
			CGameMenu::CreateGameMenu();

			if(restart) SAFE_DELETE(KerServices.QuickSave); //kdyz restartuju tak zrusim save

			if(KerServices.QuickSave && !restart && quicksavelevel && strcmp(level,quicksavelevel)==0 )
			{
				//loadnu level ulozeny savepointem
				if (!KerLoadGameAndKernel("Quick Load",KerServices.QuickSave)) {
					// error nahlasim nekdy jindy...
					
					showerr=1;
					ok=0;
				}
			}else{
					//pustim level normalne od zacatku
					KerCreateKernel(NORMAL_RUN, eKerDBDebug, 0, level);				
			}
			
			if(ok)
			{
				int gsx,gsy,gpx,gpy;
				gsx=KerMain->MapInfo->rightx-KerMain->MapInfo->leftx;
				gsy=KerMain->MapInfo->righty-KerMain->MapInfo->lefty;
				gpx=0;gpy=0;
				if(gsx<gwsx) gpx=(gwsx-gsx)/2;
				if(gsy<gwsy) gpy=(gwsy-gsy)/2;
				GEnMain->SetScrollShift((float)gpx,(float)gpy);
				KerMain->MapInfo->RefreshScroll();
				//GEnMain->SetScroll((float)-KerMain->MapInfo->leftx,(float)-KerMain->MapInfo->lefty); 
				Input->DisableGameKeys(0);
			}
		}
		catch(CKernelPanic)
		{
			SAFE_DELETE(KerMain);
			GEnMain->DeleteAll();
			ok=0;
		}

		delete[]level;

		if(!ok) {
			ShowMainMenu();
			
			if(KerErrorFiles)
				if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
			
			if(showerr==1)
				GUIMessageBox("ERROR","Error while loading - save file is invalid.",1,0xFFFF0000);
		}

	}
	
	return ok;
}

void CMainMenuDesktop::EditLevel()
{
	CGUIFileBrowserItem* fbi = GetSelectedLevel();

	char *level;
	int ln=strlen(lRoot)+1+strlen(lFile)+10;
	level = new char[ln];
	sprintf(level,"%s\\%s\\!level",lRoot,lFile);

	if(fbi && fbi->ut.pInt == eEXTlevel)
		mainGUI->SetEditorLevelInit(level);
	else
		mainGUI->SetEditorLevelInit(0);

	eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),2);

	delete[] level;
}

int CMainMenuDesktop::Keyboard(UINT ch, UINT state)
{

	if(MMmode==1){
		
		switch(ch)
		{
		case VK_ESCAPE:
			SAFE_DELETE(KerMain);
			ShowMainMenu();
			if(KerErrorFiles)
				if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
			break;
		case VK_F4:
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),4);
			break;
		}
		//return 0;
	}else{
		switch(ch)
		{
		case VK_F1:
			GetSelectedLevel();
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),1);
			break;
		case VK_F2:
			EditLevel();
			break;
		case VK_F4:
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),4);
			break;
		case 'L':
			if(mainGUI->key_ctrl)
				KerErrorFiles->ShowAllLogs(0,40,640,200); 
			break;
		case 'C':
			if(mainGUI->key_ctrl)
				#ifdef FILE_TO_COMPILE
					compiler->TestComp(FILE_TO_COMPILE);
				#else
					compiler->TestComp("test_0001_FFFF_0001_0001.kc");
				#endif
			break;
		case 'R':
			if(mainGUI->key_ctrl)
			{
				CGUIFileBrowser* fb = new CGUIFileBrowser(0,0,300,400);//,"$KRKAL$","Krkal:");
				fb->AddRootItem("$KRKAL$","Krkal:");
				fb->AddRootItem("$SCRIPTS$","Skriptíky:");
				fb->AddRootItem("$LEVELS$","Levlíky:");
				fb->UpdateTree();
				desktop->AddBackElem(fb);
				fb->FocusMe();
				iconDefaultHandler->regbrow = fb->GetID();
				fb->AcceptEvent(iconDefaultHandler->GetID(),ETree);				
			}
			break;
		case 'A':
			if(mainGUI->key_ctrl)
				ShowCredits();			
			break;
		case 'H':
			if(mainGUI->key_ctrl)
				ShowHelp();
			break;
		case VK_RETURN:
			if(mainGUI->key_ctrl) //restart
			{
				if(levelsel->GetSelectedItem())
					PlayLevel(levelsel->GetSelectedItem()->GetID(),1);
			}
			break;

		}
	}
	return 1;
}

int CMainMenuDesktop::KeyboardChar(UINT character, UINT state)
{
/*	if(MMmode==1)
		return 0;
	else*/
		return 1;
}

int CMainMenuDesktop::KeyboardUp(UINT character, UINT state)
{
	/*if(MMmode==1)
		return 0;
	else*/
		return 1;
}
int CMainMenuDesktop::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	if(MMmode==1)
		return 0;
	else
		return 1;
}
int CMainMenuDesktop::MouseOver(float x, float y, UINT over,UINT state)
{
	if(MMmode==1)
		return 0;
	else
		return 1;
}
int CMainMenuDesktop::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(MMmode==1)
		return 0;
	else
		return 1;

}
int CMainMenuDesktop::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	if(MMmode==1)
		return 0;
	else
		return 1;

}
int CMainMenuDesktop::MouseRelativeMove(float dx, float dy)
{
	if(MMmode==1)
		return 0;
	else
		return 1;
}

void CMainMenuDesktop::LevelChange(typeID levelID)
{
	
	if(!KerServices.QuickSave) return;

	CGUIButton *ID(restart,iconDefaultHandler->restartlevel);

	if(restart)
	{

		char *level=GetLevelName(levelID);

		if(level&&strcmp(level,quicksavelevel)==0)
			restart->SetVisible(1);
		else
			restart->SetVisible(0);


		SAFE_DELETE_ARRAY(level);

	}

}

char *CMainMenuDesktop::GetLevelName(typeID levelID)
{
	CGUIFileBrowserItem *it = GetObjFromID<CGUIFileBrowserItem>(levelID);
	if(!it || it->ut.pInt != eEXTlevel) return 0;

	char *root=0,*dir=0,*file=0;
	int rr = levelsel->GetSelectedFileRelativ(levelID,&root,&dir,&file);

	char *level=0;
	
	if(rr==1)
	{	
		int ln=strlen(root)+1+strlen(dir)+1+strlen(file)+10;
		level = new char[ln];
		sprintf(level,"%s/%s/%s/!level",root,dir,file);
	}

	SAFE_DELETE_ARRAY(root);
	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);

	return level;

}

void CMainMenuDesktop::KernelPanic()
{
	if(MMmode==1)
		ShowMainMenu();
}

CGUIFileBrowserItem* CMainMenuDesktop::GetSelectedLevel()
{
	typeID levelID = 0;
	char *root=0,*dir=0,*file=0;

	if(!levelsel)
		return 0;

	CGUIFileBrowserItem* ti = dynamic_cast<CGUIFileBrowserItem*>(levelsel->GetSelectedItem());
	if(ti)
		levelID = ti->GetID();

	if(levelsel->GetSelectedFileRelativ(levelID,&root,&dir,&file)==1)
	{
		_snprintf(lRoot,100,"%s",root);
		_snprintf(lFile,1000,"%s\\%s",dir,file);
	}

	SAFE_DELETE_ARRAY(root);
	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);

	return ti;
}

void CMainMenuDesktop::ShowCredits()
{
	CMainMenuHelp h;
	h.ShowCredits();
}

void CMainMenuDesktop::ShowHelp()
{
	CMainMenuHelp h;
	h.ShowHelp();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CMainMenuDesktopIconHandler	- vyvolani programu/oken po stisknuti ikony (spusteni aplikace -> inicializacni kod)
//////////////////////////////////////////////////////////////////////////////////////////////////

CMainMenuDesktopIconHandler::CMainMenuDesktopIconHandler()
{
	//mmdesktop=_mmdesktop;
	play=exit=editlevel=editscript=levelsel=restartlevel=regbrow=0;
}

void CMainMenuDesktopIconHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EClicked )
	{
		if(event->sender==exit)
		{
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),4);
		}else
		if(event->sender==editlevel)
		{
			if(MMdesktop)
				MMdesktop->EditLevel();
		}
		else if(event->sender==editscript)
		{
			if(MMdesktop)
			{
				MMdesktop->GetSelectedLevel();
				eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),1);
			}
		}
		else if(event->sender==play)
		{
			if(MMdesktop && MMdesktop->levelsel)
			{
				CGUITreeItem* ti = MMdesktop->levelsel->GetSelectedItem();
				if(ti)
					MMdesktop->PlayLevel(ti->GetID());
				
			}
		}
		else if(event->sender==restartlevel)
		{
			if(MMdesktop && MMdesktop->levelsel)
			{
				CGUITreeItem* ti = MMdesktop->levelsel->GetSelectedItem();
				if(ti) 
					MMdesktop->PlayLevel(ti->GetID(),1);
			}
		}
		else if(event->sender==help)
		{
			if(MMdesktop)
				MMdesktop->ShowHelp();
		}
		else if(event->sender==about)
		{
			if(MMdesktop)
				MMdesktop->ShowCredits();			
		}				
	}else
	if(event->eventID == ETree)
	{
		if(event->sender==levelsel)
		{
			if(event->pInt2 == 0) //clicked
			{
				if(MMdesktop)
					MMdesktop->PlayLevel(event->pID);
			}

			if(MMdesktop) MMdesktop->LevelChange(event->pID);

		}
		if(event->sender==regbrow)
			RegBrowEvent(event);
	}

	delete event;
}

void CMainMenuDesktopIconHandler::RegBrowEvent(CGUIEvent *event)
{
	if(event->eventID == ETree && event->pInt2 == 0)
	{
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
		if(ti)
		{
			int depth;
			int *path = ti->GetTreePath(depth);
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
				
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);

				char *root;
				ret = fb->GetSelectedFileRelativ(event->pID,&root,&dir,&file);

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
				
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

CMainMenuSelectLevel::CMainMenuSelectLevel(float _x, float _y, float _sx, float _sy, char* path, char* label)
: CGUIFileBrowser(_x,_y,_sx,_sy) //,path,label,1)
{
	//rollOnClick=false;
	keyClose=false;
	keyMaximalize=false;
	compareFunction=&Compare;
	SetCompareFunctionSubTree(&Compare);

	//SetRollersTextures(0,0,0,false,false,true);

	CGUITreeItem* ti = AddRootItem(path,label);
	ti->SetRollState(1,true);
	CGUITreeItem* tti = ti->items->GetFromHead();
	if(tti)
		tti->SetRollState(1);
	//UpdateTree();
	
	background->SetCol(0xAAE9B087);

	title->SetText("Select level:");
	title->SetButtons(0,0,0);
	title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("FBlevel")->GetTexture(0)));
	
	SetLimitPosition(1,1,_x,_y,_x,_y);
	SetLimitSizes(_sx,_sy,_sx,_sy);	
}

CMainMenuSelectLevel::~CMainMenuSelectLevel()
{
	if(MMdesktop)
		MMdesktop->levelsel=0;
}

CGUIFileBrowserItem* CMainMenuSelectLevel::AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir)
{
	CGUIFileBrowserItem* ni;

	char *ff=NULL;
	char *shn;
	int ext;
	
	ff = newstrdup(name);

	shn = KerServices.ConvertFileNameToShort(name,ext,dir,1);

	if(ext==eEXTlevel)
	{
		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBlevel")->GetTexture(0)),ff,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBlevel")->GetTexture(0)),ff,true);
	}
	else
	if(dir && ext!=eEXTpackage)
	{
		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);

		ni->state=1;
		ni->ChangeRoller();
	}else
		ni = 0;
	
	if(ni)
	{
		ni->userTagType = eUTint;
		ni->ut.pInt = ext;

		ni->dir=dir;
		ni->name = ff;
	}else{
		SAFE_DELETE_ARRAY(ff);
	}

	return ni;
}

int CMainMenuSelectLevel::Compare(CGUITreeItem *aa, CGUITreeItem *bb)
{
	CGUIFileBrowserItem* a = (CGUIFileBrowserItem*)aa;
	CGUIFileBrowserItem* b = (CGUIFileBrowserItem*)bb;
	if(a && b)
	{
		int adir = (a->ut.pInt==eEXTlevel) ? 0 : a->dir;
		int bdir = (b->ut.pInt==eEXTlevel) ? 0 : b->dir;
		if(adir==0 && bdir)
			return 1;
		if(bdir==0 && adir)
			return -1;
		return _stricoll(a->name,b->name);
	}else
		return 0;
}

