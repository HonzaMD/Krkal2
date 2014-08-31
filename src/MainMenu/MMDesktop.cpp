///////////////////////////////////////////////
//
// MMDesktop.cpp
//
// Main Menu Desktop (verze 2)
//
// A: Petr Altman
//
///////////////////////////////////////////////


#include "stdafx.h"

#include "MMDesktop.h"

#include "krkal.h"
#include "levels.h"
#include "kerservices.h"
#include "register.h"
#include "version.h"

#include "list.h"
#include "widgets.h"
#include "tree.h"
#include "regedit.h"

#include "comcompiler.h"

#include "dialogs.h"

#include "dxbliter.h"
#include "genmain.h"
#include "gamemenu.h"

#include "mmhelp.h"
#include "MMLevelIntro.h"


CMMDesktop *MMDesktop=0;


//////////////////////////////////////////////////////////////////////
// CMMDesktop
//////////////////////////////////////////////////////////////////////

CMMDesktop::CMMDesktop()
	: CGUIDesktop()
{
	try{
		assert(!MMDesktop);
		MMDesktop=this;
		DefaultHandler = 0;
		levelIntro = 0;

		RemoveFromTrash();
		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR|MsgKeyboard);

		DefaultHandler = new CMMDesktopHandler();

		levBrowser=0;
		MMmode = 0;
		playingFile=0;
		quicksave=0;

		ShowMainMenu();

		AddToTrash();

	}

	catch(CExc c)
	{
		SAFE_DELETE(DefaultHandler);
		mainGUI->LogError(c);

		MMDesktop=0;
		desktop=0;
		throw CExc(eGUI, E_INTERNAL,"CMainMenuDesktop::CMainMenuDesktop> Fatal error - cannot create DESKTOP");
	}

}

CMMDesktop::~CMMDesktop()
{			
	MMDesktop=0;
	RemoveFromTrash();
	SAFE_DELETE(DefaultHandler);

	SAFE_DELETE(levBrowser);
	SAFE_DELETE_ARRAY(quicksave);
}



int CMMDesktop::ShowMainMenu()
{

	SAFE_DELETE_ARRAY(quicksave);
	SAFE_DELETE(levBrowser);

	if(MMmode==1)
	{
		if(GEnMain){
			GEnMain->DeleteAll();
			GEnMain->SetScrollShift(0,0);
		}
		KRKAL->DeleteKeyBoardMessages();
		mainGUI->cursor->SetVisible(1);

		int victory=KRKAL->GetGameVictory();
		if(victory)
		{
			if(playingFile) MMProfiles->CompleteLev(playingFile->Name);
		}
	}

	MMmode = 0;

	DeleteAllBackElems();
	DeleteAllFrontElems();

	KRKAL->GetScreenRes(screenx,screeny);
	float x,y;


	CTexture *back = styleSet->Get("MainMenu_Back")->GetTexture(0);
	CTexture *toplogo = styleSet->Get("MainMenu_Top")->GetTexture(0);

	CGUIStaticPicture* sp;
	
	sp=new CGUIStaticPicture(0,0,back,(float)screenx,(float)screeny);
	AddBackElem(sp);

	sp=new CGUIStaticPicture(0,0,toplogo,(float)screenx,(float)scY(99));
	AddBackElem(sp);

	
	CGUIStaticText *stver = new CGUIStaticText(KRKAL_VERSION_STRING,(CGUIFont*) RefMgr->Find("GUI.F.Arial.12"),sx-150,5,0xFFFFFF80,145,0,aRight);
	AddBackElem(stver);


	CGUIButton *play = new CGUIButton(20,170-30,-1,-1,"GUI.But.MMPlay","",0);
	AddBackElem(play);
	DefaultHandler->play=play->GetID();
	play->AcceptEvent(DefaultHandler->GetID(),EClicked);


	if(KerServices.QuickSave)
	{
		CGUIButton *but_restart = new CGUIButton(35,230-30,-1,-1,"GUI.But.MMRestartLevel","",0);
		AddBackElem(but_restart);
		DefaultHandler->restartlevel=but_restart->GetID();
		but_restart->AcceptEvent(DefaultHandler->GetID(),EClicked);

		CFSRegKey *k=KerServices.QuickSave->FindKey("Level File");
		if(k)
		{
			char *levelname=k->GetDirectAccess();
			if(levelname)
				quicksave=newstrdup(levelname);
		}

	} else {
		DefaultHandler->restartlevel = 0;
	}

	CGUIButton *leveledit = new CGUIButton(20,sy-90-85,-1,-1,"GUI.But.MMEditLevel","",0,0);
	AddBackElem(leveledit);
	DefaultHandler->editlevel=leveledit->GetID();
	leveledit->AcceptEvent(DefaultHandler->GetID(),EClicked);
	CGUIButton *scriptedit = new CGUIButton(10,sy-90,-1,-1,"GUI.But.MMEditScript","",0,0);
	AddBackElem(scriptedit);
	DefaultHandler->editscript=scriptedit->GetID();
	scriptedit->AcceptEvent(DefaultHandler->GetID(),EClicked);


	CGUIButton *help = new CGUIButton(0,0,-1,-1,"GUI.But.MMHelp","",0);
	help->GetSize(x,y);
	help->Move(scX(302),sy-y-40);
	AddBackElem(help);
	DefaultHandler->help=help->GetID();
	help->AcceptEvent(DefaultHandler->GetID(),EClicked);
	CGUIButton *about = new CGUIButton(0,0,-1,-1,"GUI.But.MMAbout","",0);
	about->GetSize(x,y);
	about->Move(scX(302),sy-y-10);
	AddBackElem(about);
	DefaultHandler->about=about->GetID();
	about->AcceptEvent(DefaultHandler->GetID(),EClicked);


	CGUIButton *exit = new CGUIButton(0,0,-1,-1,"GUI.But.MMExit","",0);
	exit->GetSize(x,y);
	exit->Move(sx-x-15,sy-y-10);
	AddBackElem(exit);
	DefaultHandler->exit=exit->GetID();
	exit->AcceptEvent(DefaultHandler->GetID(),EClicked);


	levBrowser=new CMMLevelBrowser();

	return 1;
}

float CMMDesktop::scX(int x)
{
	return floorf(x*screenx/800.f);
}
float CMMDesktop::scY(int y)
{
	return floorf(y*screeny/600.f);
}
float CMMDesktop::scX2(int x)
{
	return ceilf(x*screenx/800.f);
}
float CMMDesktop::scY2(int y)
{
	return ceilf(y*screeny/600.f);
}


int CMMDesktop::Keyboard(UINT ch, UINT state)
{
	if(MMmode==1){
		if (KRKAL->IsPaused()) {
			SAFE_DELETE(levelIntro);
			KRKAL->ResumeGame();
		} else {		
			switch(ch)
			{
			case VK_ESCAPE:
				SAFE_DELETE(KerMain);
				ShowMainMenu();
				if(KerErrorFiles)
					if(KerErrorFiles->WasError) KerErrorFiles->ShowAllLogs(0,0,640,480);
				break;
			case VK_F4:
				mainGUI->Exit();
				break;
			}
		}
	}else{
		switch(ch)
		{
		case VK_F1:
//			GetSelectedLevel();
			mainGUI->StartScriptEditor();
			break;
		case VK_F2:
			EditLevel();
			break;
		case VK_F4:
			mainGUI->Exit();
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
				DefaultHandler->regbrow = fb->GetID();
				fb->AcceptEvent(DefaultHandler->GetID(),ETree);				
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
/*			if(mainGUI->key_ctrl) //restart
			{
				if(levelsel->GetSelectedItem())
					PlayLevel(levelsel->GetSelectedItem()->GetID(),1);
			}*/
			break;

		}
	}
	return 1;
}



void CMMDesktop::EditLevel()
{
	mainGUI->SetEditorLevelInit(0);
	CGUIListItem *li = levBrowser->GetSelectedItem();
	if (li) {
		CMMDirFile *file = li->GetData();
		if (file->Tags & eMMLTeditable) {
			mainGUI->SetEditorLevelInit(file->LevelPath);
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),2);
		}
	}
	
//	eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),2);

}


//////////////////////////////////////////////////////////////////////////////////////////////////
// CMMLevelBrowser
//////////////////////////////////////////////////////////////////////////////////////////////////

#define scX(x) MMDesktop->scX(x)
#define scY(y) MMDesktop->scY(y)
#define scX2(x) MMDesktop->scX2(x)
#define scY2(y) MMDesktop->scY2(y)

CMMLevelBrowser::CMMLevelBrowser()
{

	w = new CGUIWindow(scX(286),scY(50),scX2(504),scY2(474),0);
	w->keyClose=false;
	MMDesktop->AddBackElem(w);
	w->FocusMe();
	
	CGUIStaticPicture* back = new CGUIStaticPicture(0,0,"$TEX$/MainMenu2/mmLevelBack.png",scX2(504),scY2(474));
	w->AddBackElem(back);

	numTabs=3;
	actTab=1;
	ShowTabs();

	CGUIButton *scrollLeft = new CGUIButton(scX(240),scY(450),-1,-1,"GUI.But.MM2_Left","",0);
	w->AddBackElem(scrollLeft);
	MMDesktop->DefaultHandler->mmLeft=scrollLeft->GetID();
	scrollLeft->AcceptEvent(MMDesktop->DefaultHandler->GetID(),EClicked);

	CGUIButton *scrollRight = new CGUIButton(scX(440),scY(450),-1,-1,"GUI.But.MM2_Right","",0);
	w->AddBackElem(scrollRight);
	MMDesktop->DefaultHandler->mmRight=scrollRight->GetID();
	scrollRight->AcceptEvent(MMDesktop->DefaultHandler->GetID(),EClicked);

	CGUIStaticPicture *ln=new CGUIStaticPicture(scX(256),scY(454)+3,scX2(180),1,0xFFAAAAAA);
	w->AddBackElem(ln);		
	ln=new CGUIStaticPicture(scX(256),scY(454)+4,scX2(180),1,0xFF000000);
	w->AddBackElem(ln);		


	lb=new CGUIListBox(scX(43),scY(25),scX2(422),scY2(408));

	//CMMLevelDir *dir = MMLevelDirs->LoadActualDir();
	CMMLevelDir *dir = MMLevelDirs->LoadDir("$GAME$");

	dir->SetAccessTags();

	CMMDirFile *f=dir->Files;

	CGUIListItem *it,*its=0;

	int dis=0;

	if(MMDesktop->playingFile) dis=1;

	while(f)
	{
		if(f->Type==eEXTdirectory||f->Type==eEXTlevel)
		{
			it=lb->AddItem(f->UserName,f->Tags&eMMLTaccessible?(f->Tags&eMMLTcompleted?0xFFFFFFFF:0xFFFFFF00):0xFF909090,f);
			if(!its)
			{
				if(dis==0 && (f->Tags&eMMLTaccessible) && !(f->Tags&eMMLTcompleted)  ) 
					its=it;
				if(dis==1) {
					if(MMDesktop->playingFile==f)
					{
						if(KRKAL->GetGameVictory()) 
							dis=2;
						else
							its=it;
					}
				}else
				if(dis==2) its=it;


			}
			
			
		}

		f=f->next;
	}
	lb->UpdateList();

	lb->AcceptEvent(MMDesktop->DefaultHandler->GetID(),EList);

	w->AddBackElem(lb);

	w->SetFocusEl(lb,0);
	if(its) its->Mark();

}

CMMLevelBrowser::~CMMLevelBrowser()
{
}

CGUIListItem* CMMLevelBrowser::GetSelectedItem() {
	return lb->GetMarkListItem();
}

void CMMLevelBrowser::MoveLeft() { lb->MoveLeft();w->SetFocusEl(lb,0);}
void CMMLevelBrowser::MoveRight() { lb->MoveRight();w->SetFocusEl(lb,0);}


void CMMLevelBrowser::ShowTabs()
{

	CTexture *zal1 = styleSet->Get("MainMenu_Level_Zal1")->GetTexture(0);
	CTexture *zal2 = styleSet->Get("MainMenu_Level_Zal2")->GetTexture(0);

	int sz=(403-18*(numTabs-1))/numTabs;
	int sza=403-((sz+18)*(numTabs-1));

	float zx,zy;
	zx=scX(18);
	zy=scY(18);
	
	CGUIStaticPicture *sp;
	CGUIStaticPicture *ln;

	int xx;
	xx=15;
	for(int i=1;i<actTab;i++)
	{

		xx+=sz;
		sp=new CGUIStaticPicture( scX(xx),0, zal1,zx,zy );
		w->AddBackElem(sp);
		xx+=18;
	}

	xx+=sza;
	for(int i=actTab;i<numTabs;i++)
	{
		sp=new CGUIStaticPicture( scX(xx),0, zal2,zx,zy );
		w->AddBackElem(sp);
		xx+=18;				
		xx+=sz;
	}

//---------	
	xx=15;
	for(int i=1;i<actTab;i++)
	{
		ln=new CGUIStaticPicture(scX(xx-2),0,scX2(sz+11),scY2(3),0xFF000000);
		w->AddBackElem(ln);		

		xx+=sz;		
		xx+=18;

	}

	ln=new CGUIStaticPicture(scX(xx-2),0,scX2(sza+4),scY2(3),0xFF000000);
	w->AddBackElem(ln);

	if(actTab>1)
	{
		ln=new CGUIStaticPicture(scX(3),scY(15),scX2(xx-3-18+1),scY2(3),0xFF000000);
		w->AddBackElem(ln);
	}

	xx+=sza;

	if(actTab<numTabs)
	{
		ln=new CGUIStaticPicture(scX(xx+18-1),scY(15),scX2(495-xx-18+1),scY2(3),0xFF000000);
		w->AddBackElem(ln);
	}


	for(int i=actTab;i<numTabs;i++)
	{
		xx+=18;
		
		ln=new CGUIStaticPicture(scX(xx-10),0,scX2(sz+11),scY2(3),0xFF000000);
		w->AddBackElem(ln);
		
		xx+=sz;
	}
	

}


void CMMDesktop::KernelPanic()
{
	if(MMmode==1)
		ShowMainMenu();
}


int CMMDesktop::PlayLevel(int restart) {
	CGUIListItem *li = levBrowser->GetSelectedItem();
	if (li) {
		return PlayLevel(li->GetData(), restart);
	}
	return 0;
}


int CMMDesktop::PlayLevel(CMMDirFile *levelFile, int restart)
{
	if(!(levelFile->Tags&eMMLTaccessible)) return 0;

	char *level=levelFile->LevelPath;

	if(!level) return 0;

	int ok=0;
	float x,y;
	
	if(level)
	{
		MMmode = 1;
		playingFile = levelFile;

		DeleteAllBackElems();
		DeleteAllFrontElems();

		mainGUI->cursor->SetVisible(0);

		CGUIStaticPicture* loading = new CGUIStaticPicture(0,0,"$TEX$/MainMenu/loading.png");
		loading->GetSize(x,y);
		
		loading->Move((screenx-x)/2,(screeny-y-10));
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

		SAFE_DELETE(levelIntro);
		levelIntro = new MMLevelIntro(levelFile->LevelDir);
		if (levelIntro->WindowShown()) {
			KRKAL->PauseGame();
		} else {
			SAFE_DELETE(levelIntro);
		}

		KRKAL->Redraw();
		

		DeleteBackElem(loading);

		int showerr=0;
		int load=0;
		try{
			CGameMenu::CreateGameMenu();

			if(restart) SAFE_DELETE(KerServices.QuickSave); //kdyz restartuju tak zrusim save

			if(KerServices.QuickSave && !restart && quicksave && strcmp(level,quicksave)==0 )
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

//////////////////////////////////////////////////////////////////////////////////////////////////
// CMMDesktopHandler
//////////////////////////////////////////////////////////////////////////////////////////////////

CMMDesktopHandler::CMMDesktopHandler()
{
	mmLeft = mmRight = play = exit = editlevel = editscript = help = about = restartlevel = levelsel = regbrow = 0;
}

void CMMDesktopHandler::EventHandler(CGUIEvent *event)
{
	if(!event || !MMDesktop)
		return;

	if(event->eventID == EClicked )
	{
		if(event->sender==exit)
		{
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),4);
		}else
		if(event->sender==editlevel)
		{
			MMDesktop->EditLevel();
		}
		else if(event->sender==editscript)
		{
			//MMdesktop->GetSelectedLevel();
			eventServer->SendPriorityEvent(ECommand,GetID(),mainGUI->handler->GetID(),1);
		}
		else if(event->sender==play)
		{
			MMDesktop->PlayLevel(0);
		}
		else if(event->sender==restartlevel)
		{
			MMDesktop->PlayLevel(1);
		}
		else if(event->sender==help)
		{
			MMDesktop->ShowHelp();
		}
		else if(event->sender==about)
		{
			MMDesktop->ShowCredits();			
		}				
		else if(event->sender==mmLeft)
		{
			MMDesktop->levBrowser->MoveLeft();
		}				
		else if(event->sender==mmRight)
		{
			MMDesktop->levBrowser->MoveRight();
		}				
	}

	if(event->eventID == EList )
	{
		CGUIListItem* ID(it,event->pID);

		if(it){
			MMDesktop->PlayLevel(it->GetData(),mainGUI->key_ctrl);
		}

	}

	if(event->eventID == ETree)
	{
		if(event->sender==regbrow)
			RegBrowEvent(event);
	}

	delete event;
}

void CMMDesktopHandler::RegBrowEvent(CGUIEvent *event)
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





void CMMDesktop::ShowCredits()
{
	CMainMenuHelp h;
	h.ShowCredits();
}

void CMMDesktop::ShowHelp()
{
	CMainMenuHelp h;
	h.ShowHelp();
}
