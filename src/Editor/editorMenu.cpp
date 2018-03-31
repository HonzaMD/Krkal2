///////////////////////////////////////////////
//
// editorMenu.cpp
//
// Implementation pro Menu editoru - New Level, Load/Save Level a souvisejici dialogy
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "editorMenu.h"
#include "editor.h"
#include "textedit.h"
#include "dialogs.h"
#include "ObjProperty.h"
#include "desktop.h"
#include "primitives.h"

#include "fs.h"
#include "ident.h"

#include "KerServices.h"
#include "autos.h"

#include "krkal.h"
#include "levels.h"
#include "register.h"
#include "EdHelp.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// CEDEditorMenuHandler
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDEditorMenuHandler::CreateNewDialog()
{
	CEDScriptSelectorDlg* dlg;
	dlg = GetObjFromID<CEDScriptSelectorDlg>(newLevelDlgID);
	if(dlg)
	{
		dlg->FocusMe();
	}
	else
	{
		dlg = new CEDScriptSelectorDlg(100,100);
		desktop->AddBackElem(dlg);
		//desktop->SetFocusEl(dlg,0);
		newLevelDlgID = dlg->GetID();
		dlg->Center(true,false);
		mainGUI->SendCursorPos();
		dlg->SetModal(1);
	}
}

void CEDEditorMenuHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->sender == icons[0])		// polozka editor menu - New Level
	{	
		CreateNewDialog();
	}

	if(event->sender == icons[1])		// polozka editor menu - Load Level
	{	
		float sy = editor->mapPosition.sy > 500 ? 500 : editor->mapPosition.sy;
		CEDFileSelector* fs = new CEDFileSelector(100,140,300,sy);
		fs->title->SetText("en{{Select Level:}}cs{{Zvol level:}}");
		fs->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("LM_LoadLevel")->GetTexture(0)));
		fs->filterExt = eEXTlevel;
		fs->AcceptEvent(GetID(),ETree);

		CGUICheckBox* cb = new CGUICheckBox(75,10,new CGUIStaticText("GameMOD",(CGUIFont*)RefMgr->Find("GUI.F.Arial.12"),0,0,0xFF000000),0,1,0xFF000000);
		cb->focusable=false;
		cb->BindAttribute(&(mainGUI->editorGameMod),dtInt,1);
		cb->Sync(false);
		chBoxID = cb->GetID();
//		cb->AcceptEvent(GetID(),EClicked);
//		fsLevelSelectorButton=cb->GetID();
		fs->treeItemStartY = 50;
		fs->AddBackElem(cb);
		fs->AddBackElem(new CGUILine(5,35,290,35,0xFF000000));
		cb->Center(true,false);

		desktop->AddBackElem(fs);
		fs->Center(true,true);
		float winX, winY;
		fs->GetPos(winX, winY);
		if (winY < editor->mapPosition.y-8)
			fs->Move(winX, editor->mapPosition.y-8);
		mainGUI->SendCursorPos();
		fs->SetModal(1);
	}

	if(event->sender == icons[2])		// polozka editor menu - Save Level
	{	
		CEDObjectProperty* ID(op,editor->objPropertyID);
		if(op)
			op->UpdateEditedVar();

		editor->SaveLevel();
	}

	if(event->sender == icons[3])		// polozka editor menu - Save As ... (Level)
	{	
		CEDLevelSaveSelectorDlg* dlg;
		dlg = GetObjFromID<CEDLevelSaveSelectorDlg>(saveLevelDlgID);
		if(dlg)
			dlg->FocusMe();
		else
		{
			dlg = new CEDLevelSaveSelectorDlg(100,100);
			desktop->AddBackElem(dlg);
			//desktop->SetFocusEl(dlg,0);
			saveLevelDlgID = dlg->GetID();
			dlg->Center(true,false);
			mainGUI->SendCursorPos();
			dlg->SetModal(1);
		}
	}

	if(event->sender == icons[4])		// polozka editor menu - Clear Level
	{
		CGUIMessageBox* mb = GUIMessageBoxOkCancel("en{{Clear Level}}cs{{Vyprázdni level}}","en{{Are You sure?}}cs{{Jsi si jistý?}}",1);
		mb->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("LM_ClearLevel")->GetTexture(0)));
		mb->AcceptEvent(GetID(),EOk);
		mb->AcceptEvent(GetID(),ECancel);
		msgBoxID = mb->GetID();
		msgBoxIndex = 4;
	}

	if(event->sender == icons[5])		// polozka editor menu - Close Level
	{	
		CGUIMessageBox* mb = GUIMessageBoxOkCancel("en{{Close Level}}cs{{Zavøi level}}","en{{Are You sure?}}cs{{Jsi si jistý?}}",1);
		mb->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("LM_CloseLevel")->GetTexture(0)));
		mb->AcceptEvent(GetID(),EOk);
		mb->AcceptEvent(GetID(),ECancel);
		msgBoxID = mb->GetID();
		msgBoxIndex = 5;
	}

	if (event->sender == icons[6])		// polozka editor menu - Level Properties
	{
		CEDLevelPropertiesDlg* dlg;
		dlg = GetObjFromID<CEDLevelPropertiesDlg>(levelPropertiesDlgID);
		if (dlg)
			dlg->FocusMe();
		else
		{
			dlg = new CEDLevelPropertiesDlg(100, 100);
			desktop->AddBackElem(dlg);
			//desktop->SetFocusEl(dlg,0);
			levelPropertiesDlgID = dlg->GetID();
			dlg->Center(true, false);
			mainGUI->SendCursorPos();
			dlg->SetModal(1);
		}
	}

	if (event->sender == icons[7])		// polozka editor menu - Show Help
	{
		CGUIStdWindow *win = GetObjFromID<CGUIStdWindow>(helpWindow);
		if (win) {
			win->UnMinimize();
			win->FocusMe();
		} 
		else
		{
			CEditorHelp help;
			helpWindow = help.ShowHelp();
		}
	}


	if(event->eventID == ETree)
	{
		if(event->pInt2 == 0)	// potvrzeni polozky => soubor vybran
		{
			CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(event->sender));
			if(fs)
			{
				char *dir=0,*file=0,*root=0;
				int sf;

				sf = fs->GetSelectedFileRelativ(event->pID,&root,&dir,&file);
				if(!sf)
					assert(false);	// neni to adresar - chyba, ve vyberu maji byt pouze adresare
			
				int ext;
				KerServices.ConvertFileNameToShort(file,ext,0,0);

				if(ext==eEXTlevel)
				{
					int len = strlen(root) + strlen(dir) + 10;
					char *dirPath = new char[len];
					if (dir[0] == 0)
						_snprintf(dirPath, len, "%s", root);
					else
						_snprintf(dirPath, len, "%s\\%s", root, dir);


					CMMLevelDir* ldir = MMLevelDirs->LoadDir(dirPath);
					CMMDirFile* dfile = ldir->Files;
					bool ok = false;
					while (dfile)
					{
						if (_strcmpi(dfile->Name, file) == 0 && (dfile->Tags & eMMLTeditable)) {
							ok = true;
							break;
						}
						dfile = dfile->next;
					}
					SAFE_DELETE_ARRAY(dirPath);
					
					if (ok) {
						int len = strlen(dir) + strlen(file) + 10;
						char *level = new char[len];
						if (dir[0] == 0)
							_snprintf(level, len, "%s\\!level", file);
						else
							_snprintf(level, len, "%s\\%s\\!level", dir, file);

						CGUICheckBox* cb = GetObjFromID<CGUICheckBox>(chBoxID);
						cb->Sync(true);

						editor->CloseLevel();
						editor->InitLevel(level);

						fs->CloseWindow();
						SAFE_DELETE_ARRAY(level);
					}
					else 
					{
						CEDUserAnnouncer::Announce("en{{Level is locked}}cs{{Level je zamèený}}", 5);
					}
				}

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
			}
		}
	}

	if(event->sender == msgBoxID)
	{
		if(event->eventID == EOk)
		{
			if(msgBoxIndex==4)
			{
				editor->ClearLevel();
			}
			else if(msgBoxIndex==5)
			{
				editor->CloseLevel();
				CGUIToolBar* tb = GetObjFromID<CGUIToolBar>(editor->propertyTB);
				if(tb)
					tb->CloseWindow();
			}
			msgBoxID=0;
			msgBoxIndex=-1;
		}
		else if(event->eventID == ECancel)
		{
			msgBoxID=0;
			msgBoxIndex=-1;
		}
	}

	delete event;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Selector
////////////////////////////////////////////////////////////////////////////////////////////////////

CEDFileSelector::CEDFileSelector(float _x, float _y, float _sx, float _sy, char* path, char* label)
: CGUIFileBrowser(_x,_y,_sx,_sy) //,path,label,1)
{
	title->SetText("FileSelector");
	MsgAcceptConsume(MsgKeyboard);

	compareFunction=&Compare;
	SetCompareFunctionSubTree(&Compare);

	AddRootItem(path,label,1);

	SetDirMenu();
	SetFileMenu();
}

CEDFileSelector::~CEDFileSelector()
{
}

int CEDFileSelector::Filter(CGUITreeItem* parent, const char *name, int dir, int first)
{
	//if(!dir) return 0;
	return 1;
}

CGUIFileBrowserItem* CEDFileSelector::AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir)
{
	CGUIFileBrowserItem* ni;

	char *ff=NULL;
	char *shn;
	int ext;
	
	ff = newstrdup(name);

	shn = KerServices.ConvertFileNameToShort(name,ext,dir,0);

		if(dir && ext!=eEXTpackage)
		{
			if(ext==eEXTlevel)
			{
				if(fbi)
					ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBlevel")->GetTexture(0)),ff,true);
				else
					ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBlevel")->GetTexture(0)),ff,true);
			}
			else
			{
				if(fbi)
					ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);
				else
					ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);
			}
			
			if(ext!=eEXTlevel || filterExt!=eEXTlevel)
			{
				ni->state=1;
				ni->ChangeRoller();
			}
		}else{
			if(ext==filterExt)
			{
				if(ext==eEXTauto)
				{
					if(fbi)
						ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("STD_AUTO_TEX")->GetTexture(0)),ff,true);
					else
						ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("STD_AUTO_TEX")->GetTexture(0)),ff,true);
				}
				else
				{
					if(fbi)
						ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),ff,true);
					else
						ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),ff,true);
				}
			}
			else
				ni = 0;

		}

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

int CEDFileSelector::TreeHandler(typeID treeItem, int index, int state)
{
	int ret = CGUIFileBrowser::TreeHandler(treeItem,index,state);

	/*
	if(state!=2) return 1; // jen pravy mouse but.

	char *dir=0,*file=0;
	char *res=0;

	int gsf = GetSelectedFile(treeItem, &dir, &file);
	if(gsf==-1) return 1; 

	if(gsf==0) //soubor
	{
		eResourceType rt = GEnMain->GetPackageMgr()->GetResourceType(dir,file);
		if(state==0)
		{
			switch(rt)
			{
			case resElSource:
				{				
				CEDElSourceEditor *elsed = new CEDElSourceEditor(100,100);
				elsed->LoadElSource(dir,file);
				desktop->AddBackElem(elsed);
				elsed->FocusMe();
				}
				break;
			case resLightTex:
				{				
				CEDLightTexEditor *lted = new CEDLightTexEditor(100,100);
				lted->LoadLightTex(dir,file);
				desktop->AddBackElem(lted);
				lted->FocusMe();
				}
				break;
			case resAnim:
				{				
				CEDAnimEditor *animed = new CEDAnimEditor(100,100);
				animed->LoadAnim(dir,file);
				desktop->AddBackElem(animed);
				animed->FocusMe();
				}
				break;
			}
		}else
		if(state==2) //right mouse but
		{
			if(rt!=resNone)
				resourcemenu->On(treeItem);
		}
	}else
	{
		//adresar
	}

	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);
*/

	return ret;
}

int CEDFileSelector::Keyboard(UINT ch, UINT state) //klavesnice
{
	int st = CGUIFileBrowser::Keyboard(ch,state);
	
	if(ch==VK_RETURN)
	{
		if(mainGUI->key_ctrl)
		{
			CloseWindow();
			st=0;
		}
	}
	else if(ch==VK_ESCAPE)
	{
		CloseWindow();
		st=0;
	}
	
	return st;
}

int CEDFileSelector::Compare(CGUITreeItem *aa, CGUITreeItem *bb)
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

int CEDFileSelector::CompareLevelDIR(CGUITreeItem *aa, CGUITreeItem *bb)
{
	CGUIFileBrowserItem* a = (CGUIFileBrowserItem*)aa;
	CGUIFileBrowserItem* b = (CGUIFileBrowserItem*)bb;
	if(a && b)
	{
		int adir = a->dir;
		int bdir = b->dir;
		if(adir==0 && bdir)
			return 1;
		if(bdir==0 && adir)
			return -1;
		if(a->dir && b->dir)
		{
			if(a->ut.pInt==eEXTlevel && b->ut.pInt!=eEXTlevel)
				return 1;
			if(a->ut.pInt!=eEXTlevel && b->ut.pInt==eEXTlevel)
				return -1;
		}
		return _stricoll(a->name,b->name);
	}else
		return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDScriptSelectorDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

#define STRINGLEN 1000

CEDScriptSelectorDlg::CEDScriptSelectorDlg(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,400,227,"en{{New Level}}cs{{Nový level}}",new CGUIRectHost(0,0,styleSet->Get("LM_NewLevel")->GetTexture(0)),false)
{
	autogrowing = 0;
	title->SetButtons(true,false,false);

	float xx,yy;
	GetBackWindowSize(xx,yy);
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	scriptName = new char[STRINGLEN+5];
	strncpy(scriptName,KRKAL->cfg.defaultscript,STRINGLEN);
	levelName = new char[STRINGLEN+5];
	levelName[0]=0;
	levelPath = new char[STRINGLEN+5];
	levelPath[0]=0;


	AddBackElem(new CGUIStaticText("en{{Level Name:}}cs{{Jméno levelu:}}",font,10,15,STD_DLG_TEXT_COLOR));

	edLevel = new CGUIEditWindow(90,13,250);//,0xFF000000,0xFFFFFFFF,"DX.F.Arial.8");
	edLevel->SetTabOrder(1.1f);
	edLevel->SelectOnFocus();
	edLevel->BindAttribute(levelName,dtString,STRINGLEN+1,0);
	edLevel->AcceptEvent(GetID(),EOk);
	AddBackElem(edLevel);
	edLevel->Sync(0);

	//---
	CGUILine *ln;
	ln = new CGUILine(10,40,xx-10,40,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(1,0,1,1);
	AddBackElem(ln);
	//---
	
	AddBackElem(new CGUIStaticText("en{{Advanced Options:}}cs{{Pokroèilé možnosti:}}",(CGUIFont*) RefMgr->Find("GUI.F.Arial.10.B"),10,43,STD_DLG_BORDER_COLOR));
	AddBackElem(new CGUIStaticText("en{{(Don't change anything, if you don't know, what it means!)}}cs{{Nemìò nic, pokud si nejsi jistý, co to znamená!}}",font,10,57,STD_DLG_BORDER_COLOR));


//	AddBackElem(new CGUIStaticText("LevelPath:",font,10,85,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("Script:",font,10,95,STD_DLG_TEXT_COLOR));


	//edLevelPath = new CGUIEditWindow(80,83,260);
	//edLevelPath->SetTabOrder(2.1f);
	//edLevelPath->SelectOnFocus();
	//edLevelPath->BindAttribute(levelPath,dtString,STRINGLEN+1,0);
	//edLevelPath->AcceptEvent(GetID(),EOk);
	//AddBackElem(edLevelPath);
	//edLevelPath->Sync(0);


	edScript = new CGUIEditWindow(90,93,250);
	edScript->SetTabOrder(3.1f);
	edScript->SelectOnFocus();
	edScript->BindAttribute(scriptName,dtString,STRINGLEN+1,0);
	edScript->AcceptEvent(GetID(),EOk);
	AddBackElem(edScript);
	edScript->Sync(0);


	//bLevelSelector  = new CGUIButton(350,83,20,20,"GUI.But.Std","...");
	//bLevelSelector->AcceptEvent(GetID(),EClicked);
	//bLevelSelector->AcceptEvent(GetID(),EStateChanged);
	//bLevelSelector->SetTabOrder(2.2f);
	//bLevelSelector->SetMark(1);
 //   AddBackElem(bLevelSelector);


	bScriptSelector  = new CGUIButton(350,93,20,20,"GUI.But.Std","...");
	bScriptSelector->AcceptEvent(GetID(),EClicked);
	bScriptSelector->AcceptEvent(GetID(),EStateChanged);
	bScriptSelector->SetTabOrder(3.2f);
	bScriptSelector->SetMark(1);
    AddBackElem(bScriptSelector);



	//---
	ln = new CGUILine(10,120,xx-10,120,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(0,0,1,1);
	AddBackElem(ln);
	//---

	CGUICheckBox* cb = new CGUICheckBox(75,127,new CGUIStaticText("GameMOD",(CGUIFont*)RefMgr->Find("GUI.F.Arial.12"),0,0,0xFF000000),0,1,0xFF000000);
	cb->BindAttribute(&(mainGUI->editorGameMod),dtInt,1);
	cb->Sync(false);

	chBoxID = cb->GetID();
	cb->SetMark(true);
	cb->SetTabOrder(5);
	AddBackElem(cb);
	cb->Center(true,false);

	ln = new CGUILine(10,150,xx-10,150,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(0,0,1,1);
	AddBackElem(ln);


	ok = new CGUIButton(80,165,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(270,165,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edLevel,0);

	//SetBackWindowNormalSize(xx,140);
	SetBWLimitSizes(0,0,xx,yy,0,1);
	//SetWindowSizeVP(xx,yy);

	fsScriptSelector=fsLevelSelector=0;
	fsLevelSelectorButton=0;
}

CEDScriptSelectorDlg::~CEDScriptSelectorDlg()
{
	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsScriptSelector));
	if(fs && desktop)
	{
		desktop->DeleteBackElem(fs);
	}
	fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
	if(fs && desktop)
	{
		desktop->DeleteBackElem(fs);
	}
	

	SAFE_DELETE_ARRAY(scriptName);
	SAFE_DELETE_ARRAY(levelName);
	SAFE_DELETE_ARRAY(levelPath);
}


void CEDScriptSelectorDlg::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	CGUIEventProgram *ep;
	CGUIButton *bt;
	CGUIEditWindow *eb;

	ep = nameServer->ResolveID(event->sender);

	//-----------------------------------------------------------------------------------
	eb = dynamic_cast<CGUIEditWindow*>(ep); //editboxy
	if(eb && event->eventID == EOk){
		if(eb == edScript)
		{
			int ret=0;
			if(eb->Sync(1)==0)
			{
				char buf[5];
				FS->ChangeDir("$GAME$");
				ret = FS->ReadFile(scriptName,buf,4);
				if(ret)
				{
					// kontrola typu souboru dle pripony
				}
			}
			if(!ret)
			{
				eb->BadInput();
				BadInput(event->sender);
			}
		}
		else if(eb == edLevel)
		{
			int ret=0;
			if(eb->Sync(1)==0)
			{
				ret = FS->IsValidFilename(levelName);
			}
			if(!ret) {
				eb->BadInput();
				BadInput(event->sender);
			}
		}
		//else if(eb == edLevelPath)
		//{
		//	int ret=0;
		//	if(eb->Sync(1)==0)
		//	{
		//		FS->ChangeDir("$GAME$");
		//		if(levelPath[0]==0)
		//			ret=1;
		//		else
		//			ret = FS->ChangeDir(levelPath);
		//	}
		//	if(!ret)
		//	{
		//		eb->BadInput();
		//		BadInput(event->sender);
		//	}
		//}
	}

	//-----------------------------------------------------------------------------------
	bt = dynamic_cast<CGUIButton*>(ep); //buttony
	if(bt)
	{
		if(event->eventID == EClicked)
		{
			if(bt == bScriptSelector)	//vyber skriptu
			{ 
				CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsScriptSelector));
				if(fs)
				{
					fs->FocusMe(); //uz existuje FileSelector se skripty -> jen ho nafocusuju
				}
				else
				{
					float bx=0,by=0;
					desktop->GetDesktopPos(bScriptSelector,bx,by); //zjistim souradnice tlacitka "..." v desktopu
					
					fs = new CEDFileSelector(bx,by,300,500);
					fs->title->SetText("en{{Select Script:}}cs{{Vyber script}}");
					fs->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("FileBrowser")->GetTexture(0)));
					fs->compareFunction=&CEDFileSelector::CompareLevelDIR;
					fs->SetCompareFunctionSubTree(&CEDFileSelector::CompareLevelDIR);
					fs->title->SetButtons(true,false,false);
					fs->filterExt = eEXTscript;
					fs->AcceptEvent(GetID(),ETree);
					desktop->AddBackElem(fs);
					fs->SetModal(1);
					//fs->FocusMe();

					fsScriptSelector = fs->GetID();
				}
			}
			//else if(bt == bLevelSelector)	//vyber levelu - cesty + eventualne i jmena
			//{ 
			//	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
			//	if(fs)
			//	{
			//		fs->FocusMe(); //uz existuje FileSelector se skripty -> jen ho nafocusuju
			//	}
			//	else
			//	{
			//		float bx=0,by=0;
			//		desktop->GetDesktopPos(bLevelSelector,bx,by); //zjistim souradnice tlacitka "..." v desktopu
			//		
			//		fs = new CEDFileSelector(bx,by,300,500);
			//		fs->title->SetText("Select LevelPath (name):");
			//		fs->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("FileBrowser")->GetTexture(0)));
			//		fs->title->SetButtons(true,false,false);
			//		fs->filterExt = eEXTlevel;
			//		fs->AcceptEvent(GetID(),ETree);

			//		CGUIButton* ok = new CGUIButton(75,10,50,25,"GUI.But.Std","OK");
			//		ok->AcceptEvent(GetID(),EClicked);
			//		fsLevelSelectorButton=ok->GetID();
			//		fs->treeItemStartY = 50;
			//		fs->AddBackElem(ok);
			//		ok->Center(true,false);

			//		desktop->AddBackElem(fs);
			//		fs->SetModal(1);
			//		//fs->FocusMe();

			//		fsLevelSelector = fs->GetID();
			//	}
			//}
			//else if(event->sender == fsLevelSelectorButton)
			//{	// zmacknul se OK button v LevelSelectoru
			//	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
			//	if(fs)
			//	{
			//		fs->CloseWindow();
			//		fsLevelSelectorButton=0;
			//		FocusMe();
			//		SetFocusEl(FindNextTABElement(edLevelPath->GetTabOrder(),false),0);			
			//	}
			//}
		}
		else if(event->eventID == EStateChanged)
		{
			CGUIEditWindow *ew = GetObjFromID<CGUIEditWindow>(IDbadElem);
			if((ew==edScript && bt==bScriptSelector))
			{
				int cs = bt->GetState();
				
				if(cs == 2) 
					canceling = 1;//cancel je zamacklej
				else
				{
					if(IDbadElem){
						CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
						if(el)
							SetFocusEl(el,0);
						IDbadElem = 0;
						//badelfoc=0;
					}
					canceling = 0;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------
	if(event->eventID == ETree)
	{
		if(event->sender == fsScriptSelector && event->pInt2 == 0) //obsluha vyberu jmena scriptu
		{
			CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsScriptSelector));
			if(fs)
			{
				char *dir=0,*file=0,*root=0;
				int sf;

				sf = fs->GetSelectedFileRelativ(event->pID,&root,&dir,&file);
				
				if(sf==0) //vybral se soubor
				{
					if(dir[0]!=0)
						_snprintf(scriptName,STRINGLEN,"%s\\%s",dir,file);
					else
						_snprintf(scriptName,STRINGLEN,"%s",file);
					edScript->Sync(false);
					IDbadElem = 0;

					fs->CloseWindow();
					FocusMe();
					SetFocusEl(FindNextTABElement(bScriptSelector->GetTabOrder(),false),0);					
				}

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
			}		
		}
		//else if(event->sender == fsLevelSelector && event->pInt2 == 0) //obsluha vyberu jmena Levelu / cesty k Levelu
		//{
		//	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
		//	if(fs)
		//	{
		//		char *dir=0,*file=0,*root=0;
		//		int sf;

		//		sf = fs->GetSelectedFileRelativ(event->pID,&root,&dir,&file);
		//		if(!sf)
		//			assert(false);	// neni to adresar - chyba, ve vyberu maji byt pouze adresare
		//	
		//		_snprintf(levelPath,STRINGLEN,"%s",dir);

		//		int ext;
		//		KerServices.ConvertFileNameToShort(file,ext,0,0);

		//		if(ext==eEXTlevel)
		//		{
		//			_snprintf(levelName,STRINGLEN,"%s",file);
		//			edLevel->Sync(false);
		//			//IDbadElem = 0;

		//			fs->CloseWindow();
		//			FocusMe();
		//			SetFocusEl(FindNextTABElement(edLevelPath->GetTabOrder(),false),0);					
		//		}
		//		else
		//		{
		//			if(dir[0]==0)
		//				_snprintf(levelPath,STRINGLEN,"%s",file);
		//			else
		//				_snprintf(levelPath,STRINGLEN,"%s\\%s",dir,file);
		//		}

		//		edLevelPath->Sync(false);
		//		IDbadElem = 0;

		//		SAFE_DELETE_ARRAY(root);
		//		SAFE_DELETE_ARRAY(dir);
		//		SAFE_DELETE_ARRAY(file);
		//	}		
		//}
	}
	//------------------------------------------------------------------------------------

	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}

int CEDScriptSelectorDlg::Ok()
{
	if(levelName[0]==0 || scriptName[0]==0)
		return 0;

	FS->ChangeDir("$GAME$");
	MMLevelDirs->InvalidateDir(levelPath);
	FS->ChangeDir(levelPath);

	if(!FS->ChangeDir(levelName))
	{
		strcat(levelName,"_");
		int len = strlen(levelName);
		KerServices.GenerateVersionNumber(&(levelName[len]));
		strcat(levelName,".lv");
		FS->CreateDir(levelName);
		if(!FS->ChangeDir(levelName))
			return 0;	// cyba, nemohu se prepnout do noveho adresare
	}
	
	int len = strlen(scriptName)+10;
	char *script = new char[len];
	_snprintf(script,len-1,"$GAME$\\%s",scriptName);

	CGUICheckBox* cb = GetObjFromID<CGUICheckBox>(chBoxID);
	cb->Sync(true);

	editor->CloseLevel();
	editor->InitScript(script);

	SAFE_DELETE_ARRAY(script);

	if(KerMain)
	{
		KerMain->LevelInfo.LevelFile = new char[strlen(levelPath)+strlen(levelName)+10];	// + 2 lomitka a !level
		if(levelPath[0]==0)
			sprintf(KerMain->LevelInfo.LevelFile,"%s\\!level",levelName);
		else
			sprintf(KerMain->LevelInfo.LevelFile,"%s\\%s\\!level",levelPath,levelName);
	}


	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsScriptSelector));
	if(fs)
		fs->CloseWindow();
	fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
	if(fs)
		fs->CloseWindow();

	CloseWindow();

	return 1;
}




////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDLevelSaveSelectorDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

CEDLevelSaveSelectorDlg::CEDLevelSaveSelectorDlg(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,400,106,"en{{Save As ...}}cs{{Ulož jako ...}}",new CGUIRectHost(0,0,styleSet->Get("BUT_Save_Up")->GetTexture(0)),false)
{
	autogrowing = 0;
	title->SetButtons(true,false,false);

	float xx,yy;
	GetBackWindowSize(xx,yy);
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

//	AddBackElem(new CGUIStaticText("LevelPath:",font,10,45,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("en{{Level Name:}}cs{{Jméno levelu:}}",font,10,15,STD_DLG_TEXT_COLOR));

	levelName = new char[STRINGLEN+5];
	levelName[0]=0;
	levelPath = new char[STRINGLEN+5];
	levelPath[0]=0;

	//edLevelPath = new CGUIEditWindow(80,43,260);
	//edLevelPath->SetTabOrder(3.1f);
	//edLevelPath->SelectOnFocus();
	//edLevelPath->BindAttribute(levelPath,dtString,STRINGLEN+1,0);
	//edLevelPath->AcceptEvent(GetID(),EOk);
	//AddBackElem(edLevelPath);
	//edLevelPath->Sync(0);


	edLevel = new CGUIEditWindow(90,13,250);
	edLevel->SetTabOrder(2.1f);
	edLevel->SelectOnFocus();
	edLevel->BindAttribute(levelName,dtString,STRINGLEN+1,0);
	edLevel->AcceptEvent(GetID(),EOk);
	AddBackElem(edLevel);
	edLevel->Sync(0);


	bLevelSelector  = new CGUIButton(350,13,20,20,"GUI.But.Std","...");
	bLevelSelector->AcceptEvent(GetID(),EClicked);
	bLevelSelector->AcceptEvent(GetID(),EStateChanged);
	bLevelSelector->SetTabOrder(2.2f);
	bLevelSelector->SetMark(1);
    AddBackElem(bLevelSelector);


	//---
	//CGUILine* ln = new CGUILine(10,80,xx-10,80,STD_DLG_BORDER_COLOR);
	//ln->SetAnchor(0,0,1,1);
	//AddBackElem(ln);
	//---


	ok = new CGUIButton(80,46,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(270,46,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edLevel,0);

	//SetBackWindowNormalSize(xx,140);
	SetBWLimitSizes(0,0,xx,yy,0,1);
	//SetWindowSizeVP(xx,yy);

	fsLevelSelector=0;
}

CEDLevelSaveSelectorDlg::~CEDLevelSaveSelectorDlg()
{
	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
	if(fs && desktop)
	{
		desktop->DeleteBackElem(fs);
	}

	SAFE_DELETE_ARRAY(levelName);
	SAFE_DELETE_ARRAY(levelPath);
}


void CEDLevelSaveSelectorDlg::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	CGUIEventProgram *ep;
	CGUIButton *bt;
	CGUIEditWindow *eb;

	ep = nameServer->ResolveID(event->sender);

	//-----------------------------------------------------------------------------------
	eb = dynamic_cast<CGUIEditWindow*>(ep); //editboxy
	if(eb && event->eventID == EOk){
		if(eb == edLevel)
		{
			int ret=0;
			if(eb->Sync(1)==0)
			{
				ret = FS->IsValidFilename(levelName);
			}
			if(!ret) {
				eb->BadInput();
				BadInput(event->sender);
			}
		}
		//else if(eb == edLevelPath)
		//{
		//	int ret=0;
		//	if(eb->Sync(1)==0)
		//	{
		//		FS->ChangeDir("$GAME$");
		//		if(levelPath[0]==0)
		//			ret=1;
		//		else
		//			ret = FS->ChangeDir(levelPath);
		//	}
		//	if(!ret)
		//	{
		//		eb->BadInput();
		//		BadInput(event->sender);
		//	}
		//}
	}

	//-----------------------------------------------------------------------------------
	bt = dynamic_cast<CGUIButton*>(ep); //buttony
	if(bt)
	{
		if(event->eventID == EClicked)
		{
			if(bt == bLevelSelector)	//vyber levelu - cesty + eventualne i jmena
			{ 
				CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
				if(fs)
				{
					fs->FocusMe(); //uz existuje FileSelector se skripty -> jen ho nafocusuju
				}
				else
				{
					float bx=0,by=0;
					desktop->GetDesktopPos(bLevelSelector,bx,by); //zjistim souradnice tlacitka "..." v desktopu
					
					fs = new CEDFileSelector(bx,by,300,500);
					fs->title->SetText("en{{Select LevelPath (name):}}cs{{Zvol level:}}");
					fs->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("BUT_Save_Up")->GetTexture(0)));
					fs->title->SetButtons(true,false,false);
					fs->filterExt = eEXTlevel;
					fs->AcceptEvent(GetID(),ETree);
					desktop->AddBackElem(fs);
					fs->SetModal(1);

					fsLevelSelector = fs->GetID();
				}
			}
		}
		else if(event->eventID == EStateChanged)
		{
			CGUIEditWindow *ew = GetObjFromID<CGUIEditWindow>(IDbadElem);
			if(ew==edLevel && bt==bLevelSelector)
			{
				int cs = bt->GetState();
				
				if(cs == 2) 
					canceling = 1;//cancel je zamacklej
				else
				{
					if(IDbadElem){
						CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
						if(el)
							SetFocusEl(el,0);
						IDbadElem = 0;
					}
					canceling = 0;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------
	if(event->eventID == ETree)
	{
		if(event->sender == fsLevelSelector && event->pInt2 == 0) //obsluha vyberu jmena Levelu / cesty k Levelu
		{
			CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
			if(fs)
			{
				char *dir=0,*file=0,*root=0;
				int sf;

				sf = fs->GetSelectedFileRelativ(event->pID,&root,&dir,&file);
				if(!sf)
					assert(false);	// neni to adresar - chyba, ve vyberu maji byt pouze adresare
			
				//_snprintf(levelPath,STRINGLEN,"%s",dir);

				int ext;
				KerServices.ConvertFileNameToShort(file,ext,0,0);

				if(ext==eEXTlevel)
				{
					_snprintf(levelName,STRINGLEN,"%s",file);
					edLevel->Sync(false);

					fs->CloseWindow();
					FocusMe();
					SetFocusEl(FindNextTABElement(edLevel->GetTabOrder(),false),0);					
				}
				//else
				//{
				//	if(dir[0]==0)
				//		_snprintf(levelPath,STRINGLEN,"%s",file);
				//	else
				//		_snprintf(levelPath,STRINGLEN,"%s\\%s",dir,file);
				//}

				//edLevelPath->Sync(false);
				IDbadElem = 0;

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
			}		
		}
	}
	//------------------------------------------------------------------------------------

	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}

int CEDLevelSaveSelectorDlg::Ok()
{
	if(mainGUI->editorGameMod)	// level nelze ulozit v game modu - herni objekty nejsou v konzistentnim stavu
	{
		CEDUserAnnouncer::Announce("en{{Can't save level in Game MOD}}cs{{Nemohu uložit level v Game Modu}}",5);
		
		CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
		if(fs)
			fs->CloseWindow();

		CloseWindow();
		return 1;
	}


	if(levelName[0]==0 || !KerMain)
		return 0;


	FS->ChangeDir("$GAME$");
	MMLevelDirs->InvalidateDir(levelPath);
	FS->ChangeDir(levelPath);

	if(!FS->ChangeDir(levelName))
	{
		strcat(levelName,"_");
		int len = strlen(levelName);
		KerServices.GenerateVersionNumber(&(levelName[len]));
		strcat(levelName,".lv");
		FS->CreateDir(levelName);
		if(!FS->ChangeDir(levelName))
			return 0;	// cyba, nemohu se prepnout do noveho adresare
	}
	
	SAFE_DELETE_ARRAY(KerMain->LevelInfo.LevelFile);
	SAFE_DELETE(KerMain->LevelInfo.LocalNames);

	KerMain->LevelInfo.LevelFile = new char[strlen(levelPath)+strlen(levelName)+10];	// + 2 lomitka a !level
	if(levelPath[0]==0)
		sprintf(KerMain->LevelInfo.LevelFile,"%s\\!level",levelName);
	else
		sprintf(KerMain->LevelInfo.LevelFile,"%s\\%s\\!level",levelPath,levelName);


	editor->SaveLevel();

	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsLevelSelector));
	if(fs)
		fs->CloseWindow();

	CloseWindow();

	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDAutoSelectFileDlg
////////////////////////////////////////////////////////////////////////////////////////////////////

CEDAutoSelectFileDlg::CEDAutoSelectFileDlg(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,400,160,"Select A file",new CGUIRectHost(0,0,styleSet->Get("BUT_Save_Up")->GetTexture(0)),false)
{
	autogrowing = 0;
	title->SetButtons(true,false,false);

	float xx,yy;
	GetBackWindowSize(xx,yy);
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	AddBackElem(new CGUIStaticText("APath:",font,10,45,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("AName:",font,10,15,STD_DLG_TEXT_COLOR));

	AName = new char[STRINGLEN+5];
	AName[0]=0;
	APath = new char[STRINGLEN+5];
	APath[0]=0;

	edAPath = new CGUIEditWindow(80,43,260);
	edAPath->SetTabOrder(3.1f);
	edAPath->SelectOnFocus();
	edAPath->BindAttribute(APath,dtString,STRINGLEN+1,0);
	edAPath->AcceptEvent(GetID(),EOk);
	AddBackElem(edAPath);
	edAPath->Sync(0);


	edA = new CGUIEditWindow(80,13,260);
	edA->SetTabOrder(2.1f);
	edA->SelectOnFocus();
	edA->BindAttribute(AName,dtString,STRINGLEN+1,0);
	edA->AcceptEvent(GetID(),EOk);
	AddBackElem(edA);
	edA->Sync(0);


	bASelector  = new CGUIButton(350,13,20,20,"GUI.But.Std","...");
	bASelector->AcceptEvent(GetID(),EClicked);
	bASelector->AcceptEvent(GetID(),EStateChanged);
	bASelector->SetTabOrder(2.2f);
	bASelector->SetMark(1);
    AddBackElem(bASelector);


	//---
	CGUILine* ln = new CGUILine(10,80,xx-10,80,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(0,0,1,1);
	AddBackElem(ln);
	//---


	ok = new CGUIButton(80,100,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(270,100,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edA,0);

	//SetBackWindowNormalSize(xx,140);
	SetBWLimitSizes(0,0,xx,yy,0,1);
	//SetWindowSizeVP(xx,yy);

	fsASelector=0;
}

CEDAutoSelectFileDlg::~CEDAutoSelectFileDlg()
{
	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsASelector));
	if(fs && desktop)
	{
		desktop->DeleteBackElem(fs);
	}

	SAFE_DELETE_ARRAY(AName);
	SAFE_DELETE_ARRAY(APath);
}


void CEDAutoSelectFileDlg::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	CGUIEventProgram *ep;
	CGUIButton *bt;
	CGUIEditWindow *eb;

	ep = nameServer->ResolveID(event->sender);

	//-----------------------------------------------------------------------------------
	eb = dynamic_cast<CGUIEditWindow*>(ep); //editboxy
	if(eb && event->eventID == EOk){
		if(eb == edA)
		{
			int ret=0;
			if(eb->Sync(1)==0)
			{
				ret = FS->IsValidFilename(AName);
			}
			if(!ret) {
				eb->BadInput();
				BadInput(event->sender);
			}
		}
		else if(eb == edAPath)
		{
			int ret=0;
			if(eb->Sync(1)==0)
			{
				FS->ChangeDir("$GAME$");
				if(APath[0]==0)
					ret=1;
				else
					ret = FS->ChangeDir(APath);
			}
			if(!ret)
			{
				eb->BadInput();
				BadInput(event->sender);
			}
		}
	}

	//-----------------------------------------------------------------------------------
	bt = dynamic_cast<CGUIButton*>(ep); //buttony
	if(bt)
	{
		if(event->eventID == EClicked)
		{
			if(bt == bASelector)	//vyber levelu - cesty + eventualne i jmena
			{ 
				CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsASelector));
				if(fs)
				{
					fs->FocusMe(); //uz existuje FileSelector se skripty -> jen ho nafocusuju
				}
				else
				{
					float bx=0,by=0;
					desktop->GetDesktopPos(bASelector,bx,by); //zjistim souradnice tlacitka "..." v desktopu
					
					fs = new CEDFileSelector(bx,by,300,500);
					fs->title->SetText("Select A file (dir):");
					fs->title->SetIcon(new CGUIRectHost(0,0,styleSet->Get("BUT_Save_Up")->GetTexture(0)));
					fs->title->SetButtons(true,false,false);
					fs->compareFunction=&(CEDFileSelector::CompareLevelDIR);
					fs->SetCompareFunctionSubTree(&(CEDFileSelector::CompareLevelDIR));
					fs->filterExt = eEXTauto;
					fs->AcceptEvent(GetID(),ETree);
					desktop->AddBackElem(fs);
					fs->SetModal(1);

					fsASelector = fs->GetID();
				}
			}
		}
		else if(event->eventID == EStateChanged)
		{
			CGUIEditWindow *ew = GetObjFromID<CGUIEditWindow>(IDbadElem);
			if(ew==edA && bt==bASelector)
			{
				int cs = bt->GetState();
				
				if(cs == 2) 
					canceling = 1;//cancel je zamacklej
				else
				{
					if(IDbadElem){
						CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
						if(el)
							SetFocusEl(el,0);
						IDbadElem = 0;
					}
					canceling = 0;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------
	if(event->eventID == ETree)
	{
		if(event->sender == fsASelector && event->pInt2 == 0) //obsluha vyberu jmena Levelu / cesty k Levelu
		{
			CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsASelector));
			if(fs)
			{
				char *dir=0,*file=0,*root=0;
				int sf;

				sf = fs->GetSelectedFileRelativ(event->pID,&root,&dir,&file);
				assert(sf!=-1);

				_snprintf(APath,STRINGLEN,"%s",dir);

				int ext;
				KerServices.ConvertFileNameToShort(file,ext,0,0);


				if(sf==1)	// jde o adresar
				{
					if(dir[0]==0)
						_snprintf(APath,STRINGLEN,"%s",file);
					else
						_snprintf(APath,STRINGLEN,"%s\\%s",dir,file);
				}

				if(ext==eEXTauto)
				{
					_snprintf(AName,STRINGLEN,"%s",file);
					edA->Sync(false);

					fs->CloseWindow();
					FocusMe();
					SetFocusEl(FindNextTABElement(edAPath->GetTabOrder(),false),0);					
				}
				

				edAPath->Sync(false);
				IDbadElem = 0;

				SAFE_DELETE_ARRAY(root);
				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
			}		
		}
	}
	//------------------------------------------------------------------------------------

	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}

int CEDAutoSelectFileDlg::Ok()
{
	if(AName[0]==0 || !KerMain)
		return 0;

	char* file = new char[strlen(APath)+strlen(AName)+70];	// + 2 lomitka a $GAME$ + verze
	char* version = 0;

	if(APath[0]==0)
		sprintf(file,"$GAME$\\%s",AName);
	else
		sprintf(file,"$GAME$\\%s\\%s",APath,AName);

	//FS->ChangeDir("$GAME$");
	//FS->ChangeDir(APath);

	if(!FS->FileExist(file))
	{
		version = new char[50];
		KerServices.GenerateVersionNumber(version);
		strcat(file,"_");
		strcat(file,version);
		strcat(file,".a");
	}

	if(KerMain->AutosMain->OpenFileToSaveAutos(file,version))
	{
		CEDUserAnnouncer::Announce("A file selected",5);
		CGUIButton* ID(icon,((CEDEditorDesktop*)desktop)->iconDefaultHandler->icons[8]);
		if(icon)
			icon->Activate();
	}

	SAFE_DELETE_ARRAY(file);
	SAFE_DELETE_ARRAY(version);

	CEDFileSelector *fs = dynamic_cast<CEDFileSelector*>(nameServer->ResolveID(fsASelector));
	if(fs)
		fs->CloseWindow();

	CloseWindow();

	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// CEDLevelPropertiesDlg
////////////////////////////////////////////////////////////////////////////////////////////////////


const char *GetFileName(const char *path) {
	if (!path)
		return 0;
	const char *ret = path;
	for (const char *ptr = path; *ptr; ptr++) {
		if ((*ptr == '\\' || *ptr == '/') && ptr[1])
			ret = ptr + 1;
	}
	return ret;
}

CEDLevelPropertiesDlg::CEDLevelPropertiesDlg(float _x, float _y)
:CGUIDlgOkCancel(_x, _y, 400, 265, "en{{Level Properties}}cs{{Vlastnosti levelu}}", new CGUIRectHost(0, 0, styleSet->Get("BUT_NameBrowser_Up")->GetTexture(0)), false)
{
	autogrowing = 0;
	title->SetButtons(true, false, false);

	float xx, yy;
	GetBackWindowSize(xx, yy);

	CGUIFont *font = (CGUIFont*)RefMgr->Find("GUI.F.Arial.10");

	AddBackElem(new CGUIStaticText("en{{Author:}}cs{{Autor:}}", font, 10, 15, STD_DLG_TEXT_COLOR));

	autor = new char[STRINGLEN + 5];
	*autor = 0;
	if (KerMain->LevelInfo.Author)
		strcpy_s(autor, STRINGLEN, KerMain->LevelInfo.Author);


	edAutor = new CGUIEditWindow(80, 13, 260);
	edAutor->SetTabOrder(2.1f);
	edAutor->SelectOnFocus();
	edAutor->BindAttribute(autor, dtString, STRINGLEN + 1, 0);
	edAutor->AcceptEvent(GetID(), EOk);
	AddBackElem(edAutor);
	edAutor->Sync(0);

	AddBackElem(new CGUIStaticText("en{{Music:}}cs{{Hudba:}}", font, 10, 45, STD_DLG_TEXT_COLOR));
	if (KerMain->LevelInfo.Music) {
		AddBackElem(new CGUIStaticText(GetFileName(KerMain->LevelInfo.Music), font, 80, 45, STD_DLG_TEXT_COLOR));
	}
	AddBackElem(new CGUIStaticText("en{{Use package browser to set music.}}cs{{K nastavení hudby použij package browser.}}", font, 80, 70, STD_DLG_TEXT_COLOR));


	cbSkip = new CGUICheckBox(10, 100, new CGUIStaticText("en{{Allow To Skip Level}}cs{{Dovol pøeskoèit level}}", font, 0, 0, 0xFF000000), 0, 1, 0xFF000000);
	cbSkip->ChangeState(KerMain->LevelInfo.Tags & eMMLTskipable);
	cbSkip->SetMark(true);
	cbSkip->SetTabOrder(3);
	AddBackElem(cbSkip);

	cbEditable = new CGUICheckBox(10, 130, new CGUIStaticText("en{{Allways Editable}}cs{{Vždy editovatelný}}", font, 0, 0, 0xFF000000), 0, 1, 0xFF000000);
	cbEditable->ChangeState(KerMain->LevelInfo.Tags & eMMLTalwEditable);
	cbEditable->SetMark(true);
	cbEditable->SetTabOrder(4);
	AddBackElem(cbEditable);

	cbPLayable = new CGUICheckBox(10, 160, new CGUIStaticText("en{{Allways Playable}}cs{{Vždy hratelný}}", font, 0, 0, 0xFF000000), 0, 1, 0xFF000000);
	cbPLayable->ChangeState(KerMain->LevelInfo.Tags & eMMLTalwAccess);
	cbPLayable->SetMark(true);
	cbPLayable->SetTabOrder(4);
	AddBackElem(cbPLayable);


	ok = new CGUIButton(80, 200, 50, 25, "GUI.But.Std", "OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
	AddBackElem(ok);

	cancel = new CGUIButton(270, 200, 50, 25, "GUI.But.Std", "Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
	AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edAutor, 0);

	SetBWLimitSizes(0, 0, xx, yy, 0, 1);
}

CEDLevelPropertiesDlg::~CEDLevelPropertiesDlg()
{
	SAFE_DELETE_ARRAY(autor);
}



int CEDLevelPropertiesDlg::Ok()
{
	edAutor->Sync(1);
	SAFE_DELETE_ARRAY(KerMain->LevelInfo.Author);
	if (autor && *autor)
		KerMain->LevelInfo.Author = newstrdup(autor);

	if (cbSkip->GetState())
		KerMain->LevelInfo.Tags |= eMMLTskipable;
	else
		KerMain->LevelInfo.Tags &= ~eMMLTskipable;

	if (cbEditable->GetState())
		KerMain->LevelInfo.Tags |= eMMLTalwEditable;
	else
		KerMain->LevelInfo.Tags &= ~eMMLTalwEditable;

	if (cbPLayable->GetState())
		KerMain->LevelInfo.Tags |= eMMLTalwAccess;
	else
		KerMain->LevelInfo.Tags &= ~eMMLTalwAccess;


	CloseWindow();

	return 1;
}

