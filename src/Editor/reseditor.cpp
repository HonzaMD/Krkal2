////////////////////////////////////////////////////////////////////////////////////
//
// reseditor.cpp
//
// Editor resourcu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "reseditor.h"

#include "genmain.h"
#include "genpackage.h"
#include "menu.h"
#include "textedit.h"

#include "fs.h"

#include "genmain.h"
#include "lighttex.h"

#include "kernel.h"
#include "kerservices.h"
#include "autos.h"

#include "music.h"
#include "musicmod.h"

CEDResourceEditor* resourceEditor=NULL;

CEDResourceEditor::CEDResourceEditor()
{
	if(resourceEditor) delete resourceEditor;
	resourceEditor = this;
	
	elsclipboard=NULL;

	pkgbrowID = 0;

}

CEDResourceEditor::~CEDResourceEditor()
{
	SAFE_DELETE(elsclipboard);

	resourceEditor = NULL;
}

CPackageBrowser* CEDResourceEditor::ShowPackageBrowser()
{

	CPackageBrowser *pb = GetPackageBrowser();

	if(pb){
		pb->FocusMe();
	}else{
		pb = new CPackageBrowser(20,50,250,400);
		desktop->AddBackElem(pb);
		pb->FocusMe();
		pkgbrowID = pb->GetID();
	}

	return pb;

}

CPackageBrowser* CEDResourceEditor::GetPackageBrowser()
{
	CPackageBrowser *pb = dynamic_cast<CPackageBrowser*>(nameServer->ResolveID(pkgbrowID));
	return pb;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Package browser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CPackageBrowser::CPackageBrowser(float _x, float _y, float _sx, float _sy, char* path, char* label)
: CGUIFileBrowser(_x,_y,_sx,_sy)//,path,label,1)
{

	CGUIRectHost *icon = new CGUIRectHost(0,0,styleSet->Get("BUT_Package_Up")->GetTexture(0));
	title->SetIcon(icon);

	title->SetText("en{{Package Browser}}cs{{Brousiè pakáží}}");

	compareFunction=&Compare;
	SetCompareFunctionSubTree(&Compare);

	CGUITreeItem* ti = AddRootItem(path,label);
	ti->SetRollState(1);


	eventhandler = new CPackageBrowserEventHandler(this);

	dirmenu = new CGUIMenu();
	dirmenu->AddMenuItem(0,"en{{Create Package}}cs{{Vytvoø package}}");
	dirmenu->AcceptEvent(eventhandler->GetID(),EMenu);

	packagemenu = new CGUIMenu(170);
	packagemenu->AddMenuItem(0,"en{{Create ElSource}}cs{{Vytvoø ElSource}}");
	packagemenu->AddMenuItem(0,"en{{Create LightTex}}cs{{Vytvoø LightTex}}");
	packagemenu->AddMenuItem(0,"en{{Create Animation}}cs{{Vytvoø animaci}}");
	packagemenu->AddGap(0);
	packagemenu->AddMenuItem(0,"en{{Create tons of LightTex}}cs{{Vytvoø mraky LightTexù}}");
	packagemenu->AddGap(0);
	packagemenu->AddMenuItem(0,"en{{Add Music}}cs{{Pøidej hudbu}}");
	packagemenu->AddMenuItem(0,"en{{Add Sound}}cs{{Pøidej zvuk}}");
	packagemenu->AddGap(0);
	packagemenu->AddMenuItem(0,"en{{Delete Package}}cs{{Smaž package}}");
	packagemenu->AddGap(0);
	packagemenu->AddMenuItem(0,"en{{Defragment Package}}cs{{Defragmentuj package}}");
	packagemenu->AcceptEvent(eventhandler->GetID(),EMenu);

	resourcemenu = new CGUIMenu();
	resourcemenu->AddMenuItem(0,"en{{Delete Resource}}cs{{Smaž resource}}");
	resourcemenu->AcceptEvent(eventhandler->GetID(),EMenu);

	picturemenu = new CGUIMenu(170);
	picturemenu->AddMenuItem(0,"en{{Delete Resource}}cs{{Smaž resource}}");
	picturemenu->AddGap(0);
	picturemenu->AddMenuItem(0,"en{{Create Automatics}}cs{{Vytvoø automatizmus}}");
	picturemenu->AcceptEvent(eventhandler->GetID(),EMenu);

	musicmenu = new CGUIMenu(190);
	musicmenu->AddMenuItem(0,"en{{Delete Resource}}cs{{Smaž resource}}");
	musicmenu->AddGap(0);
	musicmenu->AddMenuItem(0,"en{{Set as Music to the Level}}cs{{Nastav jako hudbu levlu}}");
	musicmenu->AcceptEvent(eventhandler->GetID(),EMenu);

}

CPackageBrowser::~CPackageBrowser()
{
	SAFE_DELETE(dirmenu);
	SAFE_DELETE(packagemenu);
	SAFE_DELETE(resourcemenu);
	SAFE_DELETE(musicmenu);
	SAFE_DELETE(picturemenu);

	SAFE_DELETE(eventhandler);
}

int CPackageBrowser::Filter(CGUITreeItem* parent, const char *name, int dir, int first)
{
	//if(!dir) return 0;
	return 1;
}

CGUIFileBrowserItem* CPackageBrowser::AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir)
{
	CGUIFileBrowserItem* ni;

	char *ff=NULL;
	char *shn;
	int ext;
	

	shn = KerServices.ConvertFileNameToShort(name,ext,dir,0);

	if(ext==eEXTlevel) return 0;
	if(ext==eEXTauto) return 0;
	if(ext==eEXTscript) return 0;

	ff = newstrdup(name);

	CTexture *s;

	if(dir)
	{
		
		if(ext==eEXTpackage)
			s = styleSet->Get("BUT_Package_Up")->GetTexture(0);
		else
			s = styleSet->Get("FBdir")->GetTexture(0);

		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,s),ff,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,s),ff,true);

		ni->state=1;
		ni->ChangeRoller();
	}else{

		switch(ext)
		{
		case eEXTtex:
			s=styleSet->Get("PBtex")->GetTexture(0);
			break;
		case eEXTani:
			s=styleSet->Get("PBani")->GetTexture(0);
			break;
		case eEXTels:
			s=styleSet->Get("PBels")->GetTexture(0);
			break;
		case eEXTmModule:
			s=styleSet->Get("PBmm")->GetTexture(0);
			break;
		case eEXTmSample:
			s=styleSet->Get("PBms")->GetTexture(0);
			break;
		default:
			s=styleSet->Get("FBfile")->GetTexture(0);
		}

		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,s),ff,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,s),ff,true);

	}

	ni->userTagType = eUTint;
	ni->ut.pInt = ext;

	ni->dir=dir;
	ni->name = ff;

	return ni;
}

int CPackageBrowser::TreeHandler(typeID treeItem, int index, int state)
{

	if(state==1) return 1; //levy mose but.

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
			case resMusicModule:
				if(GEnMain)
				{
					char *s=new char[strlen(dir)+1+strlen(file)+1];
					sprintf(s,"%s/%s",dir,file);

					if(ME) ME->Play(s,1,1);

					delete[]s;
				}
				break;
			case resMusicSample:
				if(GEnMain)
				{
					char *s=new char[strlen(dir)+1+strlen(file)+1];
					sprintf(s,"%s/%s",dir,file);

					CMusicSample *ms = GEnMain->LoadMusicSample(s);
					if(ms) ms->Play();					
					delete[]s;
				}
				break;
			}
		}else
		if(state==2) //right mouse but
		{
			switch(rt)
			{
			case resMusicModule:
				musicmenu->On(treeItem);
				break;
			case resLightTex:
			case resAnim:
				picturemenu->On(treeItem);
				break;
			default:
				resourcemenu->On(treeItem);
			}
		}
	}else
	{
		//adresar
		if(state==2) //right mouse but
		{
			if(GEnMain->GetPackageMgr()->GetPackage(dir))
				packagemenu->On(treeItem);
			else
				dirmenu->On(treeItem);
		}
	}

	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);

	return 1;
}

int CPackageBrowser::Compare(CGUITreeItem *aa, CGUITreeItem *bb)
{
	CGUIFileBrowserItem* a = (CGUIFileBrowserItem*)aa;
	CGUIFileBrowserItem* b = (CGUIFileBrowserItem*)bb;
	if(a && b)
	{
		int adir=0;
		int bdir=0;

        if(a->ut.pInt==eEXTpackage) adir=2; else
        if(a->ut.pInt==eEXTlevel)	adir=1; else
		if(a->dir)					adir=3;
        if(b->ut.pInt==eEXTpackage) bdir=2; else
        if(b->ut.pInt==eEXTlevel)	bdir=1; else
		if(b->dir)					bdir=3;

		if(adir<bdir) return 1;
		if(bdir<adir) return -1;
		return _stricoll(a->name,b->name);
	}else
		return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PackageBrowserEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CPackageBrowserEventHandler::CPackageBrowserEventHandler(CPackageBrowser *pb)
{
	pkgbrow=pb;
	newresname=NULL;
	newresnamedlgID=0;
	deletedlgID=0;
	deletename=0;
	filebrowID=0;
	filebrowpkg = NULL;
	selectedpackage=NULL;
}

CPackageBrowserEventHandler::~CPackageBrowserEventHandler()
{
	CGUIDlgOkCancel *deldlg = GetDeleteDlg();
	if(deldlg && desktop)
	{
		deldlg->CloseWindow();deletedlgID=0;
	}
	if(newresnamedlgID)
	{
		CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(newresnamedlgID));
		if(dlg && desktop) {dlg->CloseWindow(); newresnamedlgID=0;}
	}
	if(filebrowID)
	{
		CGUIFileBrowser *fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(filebrowID));
		if(fb && desktop) {fb->CloseWindow(); filebrowID=0;}
	}
	SAFE_DELETE_ARRAY(deletename);
	SAFE_DELETE_ARRAY(newresname);
	SAFE_DELETE_ARRAY(filebrowpkg);
	SAFE_DELETE_ARRAY(selectedpackage);
}

#define MAX_RESNAME_LEN 100
void CPackageBrowserEventHandler::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	char *dir=0,*file=0;
	typeID activator;
	int menuitem;
	int music=0;

	if(event->eventID == EMenu)
	{
		activator = event->pID2;
		menuitem = event->pInt;

		pkgbrow->GetSelectedFile(activator, &dir, &file);

		if(event->sender == pkgbrow->packagemenu->GetID())
		{			
			switch(menuitem)
			{
			case 0:
			case 1:
			case 2:
				{
					if(newresnamedlgID)
					{
						CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(newresnamedlgID));
						if(dlg) {dlg->CloseWindow(); newresnamedlgID=0;}
					}
					
					if(selectedpackage) SAFE_DELETE_ARRAY(selectedpackage);

					selectedpackage = newstrdup(dir);

					if(!newresname) {newresname = new char[MAX_RESNAME_LEN+5];}
					CGUIDlgEditbox *newresnamedlg;
					if(menuitem==0){
						newresnamedlg = new CGUIDlgEditbox(100,100,300,"en{{New ElSource}}cs{{Nový ElSource}}","en{{ElSource name:}}cs{{Jméno ElSourcu:}}");
						newrestype=0;
					}else 
					if(menuitem==1){
						newresnamedlg = new CGUIDlgEditbox(100,100,300,"en{{New LightTex}}cs{{Nová LightTex}}","en{{Texture name:}}cs{{Jméno textury:}}");
						newrestype=1;
					}else
					if(menuitem==2){
						newresnamedlg = new CGUIDlgEditbox(100,100,300,"en{{New Animation}}cs{{Nová animace}}","en{{Animation name:}}cs{{Jméno animace:}}");
						newrestype=2;
					}

					newresnamedlgID=newresnamedlg->GetID();

					desktop->AddBackElem(newresnamedlg);
					newresnamedlg->AcceptEvent(GetID(),EOk);
					newresnamedlg->BindEditBox(newresname,dtString,MAX_RESNAME_LEN);
					newresnamedlg->FocusMe();

				}
				break;
			case 3:
				//vytvor mraky lighttexu
				{
					CEDManyLightTexEditor *mlted = new CEDManyLightTexEditor(100,100,dir);
					desktop->AddBackElem(mlted);
					mlted->FocusMe();
				}
				break;
			case 4:
				music=1;
			case 5:
				//pridej hudbu/zvuk
				{					
					CGUIFileBrowser *fb=0;
					if(filebrowID)
						fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(filebrowID));
                    
					if(fb) {
						fb->FocusMe();
					}else{

						fb = new CGUIFileBrowser(280,50,250,400);//,"$IMPORTMUSIC$","IMPORTMUSIC",1);
						fb->AddRootItem("$IMPORTMUSIC$","IMPORTMUSIC",1);

						desktop->AddBackElem(fb);
						fb->FocusMe();

						filebrowID = fb->GetID();

						fb->AcceptEvent(GetID(),ETree);

					}

					SAFE_DELETE_ARRAY(filebrowpkg);
					filebrowpkg = newstrdup(dir);

					module_sample=music;

					char tt[1024];
					int ext;
					char *pk = KerServices.ConvertFileNameToShort(file,ext,1,0);
					
					if(module_sample)
						_snprintf(tt,1023,"Vyber hudbu (%s)",pk);
					else
						_snprintf(tt,1023,"Vyber zvuk (%s)",pk);

					fb->title->SetText(tt);
					
				}
				break;
			case 6:
				//smaz package
				{
					CGUIDlgOkCancel *deldlg;
					deldlg = GetDeleteDlg();
					if(deldlg) deldlg->CloseWindow();

					deldlg = GUIMessageBoxOkCancel("en{{Delete Package?}}cs{{Smazat pakáž?}}","en{{Do you really want to delete package?}}cs{{Opravdu chcete smazat pakáž?}}",1,0xFF000000);
					deletedlgID = deldlg->GetID();
					deldlg->AcceptEvent(GetID(),EOk);

					deletetype = 1; //pakaz
					SAFE_DELETE_ARRAY(deletename);
					deletename = newstrdup(dir);
				}
				break;
			case 7:
				//defagmentuj pakaz
				FS->Defragment(dir);
				break;
			}
		}else
		if(event->sender == pkgbrow->dirmenu->GetID())
		{
			switch(menuitem)
			{
			case 0:
				//vytvor package
				if(newresnamedlgID)
				{
					CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(newresnamedlgID));
					if(dlg) {dlg->CloseWindow(); newresnamedlgID=0;}
				}
				
				if(selectedpackage) SAFE_DELETE_ARRAY(selectedpackage);

				selectedpackage = newstrdup(dir);

				if(!newresname) {newresname = new char[MAX_RESNAME_LEN+5];}
				CGUIDlgEditbox *newresnamedlg;

				newresnamedlg = new CGUIDlgEditbox(100,100,300,"en{{New Package}}cs{{Nová Package}}","en{{Package name:}}cs{{Jméno Package:}}");
				newrestype=-1;

				newresnamedlgID=newresnamedlg->GetID();

				desktop->AddBackElem(newresnamedlg);
				newresnamedlg->AcceptEvent(GetID(),EOk);
				newresnamedlg->BindEditBox(newresname,dtString,MAX_RESNAME_LEN);
				newresnamedlg->FocusMe();

				break;
			}
		}else
		if(event->sender == pkgbrow->resourcemenu->GetID() ||
		   event->sender == pkgbrow->musicmenu->GetID() ||
		   event->sender == pkgbrow->picturemenu->GetID()
			)
		{
			switch(menuitem)
			{
			case 0:
				//delete resource
				{
					CGUIDlgOkCancel *deldlg;
					deldlg = GetDeleteDlg();
					if(deldlg) deldlg->CloseWindow();

					deldlg = GUIMessageBoxOkCancel("en{{Delete Resource?}}cs{{Smazat resource?}}","en{{Do you really want to delete resource?}}cs{{Opravdu chcete smazat resource?}}",1,0xFF000000);
					deletedlgID = deldlg->GetID();
					deldlg->AcceptEvent(GetID(),EOk);

                    deletetype = 2; //resource
					SAFE_DELETE_ARRAY(deletename);

					deletename = new char[strlen(dir)+1+strlen(file)+1];
					sprintf(deletename,"%s/%s",dir,file);
					
				}
				break;
			}
		}
		if( event->sender == pkgbrow->musicmenu->GetID() ) 
		{
			switch(menuitem)
			{
			//case 0: je resen spolecne s ostanima
			case 1:
				//nastaveni hudby levelu
				if(KerMain)
				{
					char *relroot,*reldir,*relfile;
					if(pkgbrow->GetSelectedFileRelativ(activator,&relroot,&reldir,&relfile)!=-1)
					{
						SAFE_DELETE(KerMain->LevelInfo.Music);
						char *music;
						int ln=strlen(relroot)+1+strlen(reldir)+1+strlen(relfile)+1;
						music = new char[ln];
						sprintf(music,"%s/%s/%s",relroot,reldir,relfile);
						KerMain->LevelInfo.Music = music;
						delete[]relroot;delete[]reldir;delete[]relfile;
					}					
				}
			}
		}else
		if( event->sender == pkgbrow->picturemenu->GetID() ) 
		{
			switch(menuitem)
			{
			//case 0: je resen spolecne s ostanima
			case 1:
				//vytvoreni automatismu
				if(KerMain)
				{
					if(!KerMain->AutosMain->GetSaveAutosState())
					{
						GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Není otevøen soubor s automatizmem!",1,0xFFFF0000);
					}else
					{
						char *relroot,*reldir,*relfile;
						if(pkgbrow->GetSelectedFileRelativ(activator,&relroot,&reldir,&relfile)!=-1)
						{
							char *pic;
							int ln=strlen(relroot)+1+strlen(reldir)+1+strlen(relfile)+1;
							pic = new char[ln];
							sprintf(pic,"%s/%s/%s",relroot,reldir,relfile);
							
							delete[]relroot;delete[]reldir;delete[]relfile;

							if(!KerMain->AutosMain->AddEasyAuto(pic))
								GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Nepodaøilo se vytvoøi automatizmus!",1,0xFFFF0000);
                            
							delete[]pic;
						}
					}
				}
			}
		}
		delete[] dir; delete[] file;
		
	}

	if(event->sender == deletedlgID && event->eventID==EOk )
	{
		if(deletetype==1) //mazu pakaz
		{
			if(GEnMain->GetPackageMgr()->DeletePackage(deletename))
			{
				pkgbrow->FileSysChanged();
			}
		}else
		if(deletetype==2) //mazu resource
		{
			if(GEnMain->GetPackageMgr()->DeleteResource(deletename))
			{
				pkgbrow->FileSysChanged();
			}
		}
		deletetype=-1;
		SAFE_DELETE_ARRAY(deletename);
	}

	if(event->sender == filebrowID )
	{
		if( event->eventID == ETree && event->pInt2 == 0 )
		{
			CGUIFileBrowser *fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(filebrowID));
			if(fb)
			{
				char *dir=0,*file=0;
				int sf = fb->GetSelectedFile(event->pID,&dir,&file);

				if(sf==0) //vybral se soubor
				{
					char *fp;
					fp = new char[strlen(dir)+1+strlen(file)+1];
					sprintf(fp,"%s/%s",dir,file);

					eResourceType resType = resNone;

					if(ME)
					{
						if(module_sample==0)
						{
							CMusicSample *ms=new CMusicSample;
							if(ms->LoadFromFile(fp))
								resType = resMusicSample;
							ms->Release();
						}else
						{
							CMusicModule *mm=GEnMain->LoadMusicModuleDirect(fp);
							if(mm)
							{
								resType = resMusicModule;
								mm->Release();
							}
						}			
					}
                    
					if(resType==resMusicModule || resType==resMusicSample)
					{
						CGEnPackage *pkg = GEnMain->GetPackageMgr()->GetPackage(filebrowpkg);
						if(pkg)
						{
							char *nm;
							char *d = strrchr(file,'.');
							int fln;
							if(d)
								fln = d-file;
							else
								fln = strlen(file);
							
							nm = new char[fln+5];
							
							memcpy(nm,file,fln);
							nm[fln]=0;
							if(resType==resMusicModule) strcat(nm,".mm"); else
							if(resType==resMusicSample) strcat(nm,".ms"); 
							
							pkg->AddResourceFromFile(nm,fp,resType,1);
							delete[] nm;

							pkgbrow->FileSysChanged();

						}else{
							GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Nejde otevøít pakáž!",1,0xFFFF0000);
						}
					}else{
						if(module_sample)
							GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Vybraný soubor nelze použít jako hudba!",1,0xFFFF0000);
						else
							GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Vybraný soubor nelze použít jako zvuk!",1,0xFFFF0000);
					}

					SAFE_DELETE_ARRAY(fp);

				}

				SAFE_DELETE_ARRAY(dir);SAFE_DELETE_ARRAY(file);

			}
		}
	}

	if(event->sender == newresnamedlgID )
	{
		CGUIDlgEditbox *newresnamedlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(newresnamedlgID));
		if(newresnamedlg && event->eventID == EOk)
		{
			int err=0;
		
			if(newresnamedlg->SyncEditBox()!=0) err=1;

			if(!err)
			{
				int ln=strlen(newresname);
				switch(newrestype)
				{
				case -1:
					//vytvarim package
					if(ln<=4||strcmp(newresname+ln-4,".pkg")!=0) strcat(newresname,".pkg");
					break;
				case 0:
					if(ln<=4||strcmp(newresname+ln-4,".tex")!=0) strcat(newresname,".els");
					break;
				case 1:
					if(ln<=4||strcmp(newresname+ln-4,".tex")!=0) strcat(newresname,".tex");
					break;
				case 2:
					if(ln<=4||strcmp(newresname+ln-4,".ani")!=0) strcat(newresname,".ani");
					break;
				default:
					err=1;
				}
			}
			if(!err)
			{
				if(newrestype==-1)
				{
					CGEnPackage pkg;
					int ll=strlen(newresname);
					char *name = new char[strlen(selectedpackage)+1+ll+25+1];
					char version[25];
					newresname[ll-4]=0;
					KerServices.GenerateVersionNumber(version);
					sprintf(name,"%s/%s_%s.pkg",selectedpackage,newresname,version);
					newresname[ll-4]='.';
					if(!pkg.CreatePackage(name,1)) err=4;
					delete[] name;

					if(resourceEditor)
					{
						CPackageBrowser* pb = resourceEditor->GetPackageBrowser();
						if(pb) pb->FileSysChanged();
					}

				}else{
					CGEnPackage *pkg = GEnMain->GetPackageMgr()->GetPackage(selectedpackage);
					if(pkg){
						int ee = pkg->IsValidResourceName(newresname);
						if(ee==0) err=1; else //neplatne jmeno
						if(ee==2) err=3; //uz je v packagi
					}else
						err=2; //neni package
				}
			}

			if(!err){

				newresnamedlg->CloseWindow();
				newresnamedlg=NULL;				
				if(newrestype==0)
				{
					CEDElSourceEditor *elsed = new CEDElSourceEditor(100,100);
					elsed->CreateElSource(selectedpackage,newresname);
					desktop->AddBackElem(elsed);
					elsed->FocusMe();
				}else
				if(newrestype==1)
				{
					CEDLightTexEditor *texed = new CEDLightTexEditor(100,100);
					texed->CreateLightTex(selectedpackage,newresname);
					desktop->AddBackElem(texed);
					texed->FocusMe();
				}else
				if(newrestype==2)
				{
					CEDAnimEditor *animed = new CEDAnimEditor(100,100);
					animed->CreateAnim(selectedpackage,newresname);
					desktop->AddBackElem(animed);
					animed->FocusMe();
				}
			}else{
				if(err==1)
					newresnamedlg->ShowErrorMsg("Chyba! Neplatné jméno resourcu!");
				else
				if(err==2){
					newresnamedlg->CloseWindow();
					newresnamedlg=NULL;
				}else
				if(err==3)
					newresnamedlg->ShowErrorMsg("Chyba! Resource je již packagi!");
				else
				if(err==4)
					newresnamedlg->ShowErrorMsg("Chyba! Nelze vytvoøit package!");
			}
		}
	}
	delete event;
}

CGUIDlgOkCancel* CPackageBrowserEventHandler::GetDeleteDlg()
{
	if(deletedlgID==0) return 0;
	return dynamic_cast<CGUIDlgOkCancel*>(nameServer->ResolveID(deletedlgID));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDElSourceEditor
//
// editor ElSourcu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TEXTGROUP_COLOR STD_DLG_BORDER_COLOR


CEDElSourceEditor::CEDElSourceEditor(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,230,295,"ElSource",0,true)
{

	autogrowing = 0;
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");
	CGUIFont *font2 = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10.I");

	AddBackElem(new CGUIStaticText("en{{Base shift}}cs{{Posun základny}}",font2,10,10,TEXTGROUP_COLOR));
	AddBackElem(new CGUIStaticText("X:",font,10,30,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("Y:",font,80,30,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("Z:",font,150,30,STD_DLG_TEXT_COLOR));

	edpbx = new CGUIEditWindow(25,28,50);
	edpbx->SetTabOrder(1.1f);
	edpbx->SelectOnFocus();
	edpbx->BindAttribute(&pbx,dtDouble,1,0);
	edpbx->AcceptEvent(GetID(),EOk);
	AddBackElem(edpbx);
	
	edpby = new CGUIEditWindow(95,28,50);
	edpby->SetTabOrder(1.2f);
	edpby->SelectOnFocus();
	edpby->BindAttribute(&pby,dtDouble,1,0);
	edpby->AcceptEvent(GetID(),EOk);
	AddBackElem(edpby);
	
	edpbz = new CGUIEditWindow(165,28,50);
	edpbz->SetTabOrder(1.3f);
	edpbz->SelectOnFocus();
	edpbz->BindAttribute(&pbz,dtDouble,1,0);
	edpbz->AcceptEvent(GetID(),EOk);
	AddBackElem(edpbz);
	//---
	AddBackElem(new CGUILine(10,55,215,55,STD_DLG_BORDER_COLOR));
	//---
	AddBackElem(new CGUIStaticText("en{{Height:}}cs{{Výška:}}",font,10,65,STD_DLG_TEXT_COLOR));
	edzdz = new CGUIEditWindow(60,63,100);
	edzdz->SetTabOrder(2);
	edzdz->SelectOnFocus();
	edzdz->BindAttribute(&zdz,dtDouble,1,0);
	edzdz->AcceptEvent(GetID(),EOk);
	AddBackElem(edzdz);
	//---
	AddBackElem(new CGUILine(10,90,215,90,STD_DLG_BORDER_COLOR));
	//---
	AddBackElem(new CGUIStaticText("en{{Visibility collision solving}}cs{{Øešení kolizí u vidit. tøídìní}}",font2,10,100,TEXTGROUP_COLOR));
	
	CGUIStaticText *rampatext = new CGUIStaticText("RampaFlag",font,30,120,STD_DLG_TEXT_COLOR);
	edrampa = new CGUICheckBox(10,120,rampatext,0);
	edrampa->SetTabOrder(3.1f);
	edrampa->SetMark(1);
	edrampa->BindAttribute(&rampa,dtInt,1,0);
	edrampa->AcceptEvent(GetID(),EClicked);
	AddBackElem(edrampa);

	AddBackElem(new CGUIStaticText("FlugFlag:",font,105,120,STD_DLG_TEXT_COLOR));
	edflug = new CGUIEditWindow(165,118,50);
	edflug->SetTabOrder(3.2f);
	edflug->SelectOnFocus();
	edflug->BindAttribute(&flug,dtInt,1,0);
	edflug->AcceptEvent(GetID(),EOk);
	AddBackElem(edflug);
	//---
	AddBackElem(new CGUILine(10,145,215,145,STD_DLG_BORDER_COLOR));
	//---
	AddBackElem(new CGUIStaticText("en{{Points of base}}cs{{Body základny}}",font2,10,155,TEXTGROUP_COLOR));
	AddBackElem(new CGUIStaticText("en{{Points count:}}cs{{Poèet bodù:}}",font,10,175,STD_DLG_TEXT_COLOR));
	
	ednumpoints = new CGUIEditWindow(90,175,50);
	ednumpoints->SetTabOrder(4);
	ednumpoints->SelectOnFocus();
	ednumpoints->BindAttribute(&tmpnumpoints,dtInt,1,0);
	ednumpoints->AcceptEvent(GetID(),EOk);
	AddBackElem(ednumpoints);

	AddBackElem(new CGUIStaticText("X:",font,30,200,STD_DLG_TEXT_COLOR));
	AddBackElem(new CGUIStaticText("Y:",font,120,200,STD_DLG_TEXT_COLOR));

    //---
	ok = new CGUIButton(50,210,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(100);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(120,210,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(101);
	cancel->SetMark(1);
    AddBackElem(cancel);


	//copy= new CGUIButton(15,210,-1,-1,styleSet->Get("copyBUT"),styleSet->Get("copydownBUT"),styleSet->Get("copymarkBUT"),false,0,0,"Kopírovat");
	//copy->SetInactiveStyle(styleSet->Get("copydisableBUT"));
	copy=new CGUIButton(15,210,-1,-1,"GUI.But.Copy",0,0,"en{{Copy}}cs{{Kopírovat}}");
	copy->AcceptEvent(GetID(),EClicked);
	AddBackElem(copy);

//	paste= new CGUIButton(190,210,-1,-1,styleSet->Get("pasteBUT"),styleSet->Get("pastedownBUT"),styleSet->Get("pastemarkBUT"),false,0,0,"Vložit");
//	paste->SetInactiveStyle(styleSet->Get("pastedisableBUT"));
	paste=new CGUIButton(190,210,-1,-1,"GUI.But.Paste",0,0,"en{{Paste}}cs{{Vložit}}");
	paste->AcceptEvent(GetID(),EClicked);
	if(!(resourceEditor && resourceEditor->elsclipboard))
		paste->Deactivate();
	AddBackElem(paste);

	InitOkCancel();

	//---------

	GetBackWindowNormalSize(bwnsx,bwnsy);

	pbx=pby=pbz=0;
	zdz=0;
	flug=0;
	rampa=0;

	numpoints = 0;
	for(int i=0;i<MAX_ELS_POINTS;i++)
	{
		edpointsx[i]=NULL;
		edpointsy[i]=NULL;
		pointsx[i]=0;
		pointsy[i]=0;
	}

	els=NULL;
	resourcename=NULL;

	ChangeNumPoints(4);

	tmpnumpoints = numpoints;
	SyncAll(0);

	//-----------
	SetFocusEl(edpbx,0);

}

CEDElSourceEditor::~CEDElSourceEditor()
{
	SAFE_RELEASE(els);
	SAFE_DELETE_ARRAY(resourcename);
}

int CEDElSourceEditor::CreateElSource(char *packagenm, char *resnm)
{
	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	char *tt = new char[strlen("ElSource - ")+strlen(resnm)+1];
	sprintf(tt,"ElSource - %s",resnm);
	title->SetText(tt);
	delete[] tt;

	resourcename = resource;
	pbx=pby=pbz=0;
	zdz=0;
	flug=0;
	rampa=0;

	for(int i=0;i<MAX_ELS_POINTS;i++)
	{
		edpointsx[i]=NULL;
		edpointsy[i]=NULL;
		pointsx[i]=0;
		pointsy[i]=0;
	}

	SAFE_RELEASE(els);
	els=NULL;

	ChangeNumPoints(4);

	tmpnumpoints = numpoints;

	SyncAll(0);

	newresource=1;

	return 1;
}

int CEDElSourceEditor::LoadElSource(char *packagenm, char *resnm)
{
	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	CGEnElSource *elsource = GEnMain->LoadElSource(resource);

	if(!elsource) {
		delete[] resource;
		return 0;
	}

	char *tt = new char[strlen("ElSource - ")+strlen(resnm)+1];
	sprintf(tt,"ElSource - %s",resnm);
	title->SetText(tt);
	delete[] tt;


	resourcename = resource;

	SAFE_RELEASE(els);
	els=elsource;
	els->AddRef();

	pbx=els->pbx;pby=els->pby;pbz=els->pbz;
	zdz=els->zdz;
	rampa=els->DerRampaFlag;
	flug=els->DerFlugFlag;

	ChangeNumPoints(els->BasePointsCount);

	for(int i=0;i<numpoints;i++)
	{
		pointsx[i]=els->BasePoints[i].x;
		pointsy[i]=els->BasePoints[i].y;
	}

	SyncAll(0);

	newresource=0;

	return 1;
}

int CEDElSourceEditor::ChangeNumPoints(int num)
{
	if(num==numpoints) return 1;
	if(num<3||num>MAX_ELS_POINTS) return 0;

	int i;

	if(num>numpoints)
	{
		for(i=numpoints;i<num;i++) //pridam dalsi editboxy
		{
			edpointsx[i] = new CGUIEditWindow(30,220.f+25*i,70);
			edpointsx[i]->SetTabOrder(5.f+i);
			edpointsx[i]->SelectOnFocus();
			edpointsx[i]->BindAttribute(&pointsx[i],dtDouble,1,0);
			edpointsx[i]->AcceptEvent(GetID(),EOk);
			edpointsx[i]->Sync(0);
			AddBackElem(edpointsx[i]);

			edpointsy[i] = new CGUIEditWindow(120,220.f+25*i,70);
			edpointsy[i]->SetTabOrder(5.5f+i);
			edpointsy[i]->SelectOnFocus();
			edpointsy[i]->BindAttribute(&pointsy[i],dtDouble,1,0);
			edpointsy[i]->AcceptEvent(GetID(),EOk);
			edpointsy[i]->Sync(0);
			AddBackElem(edpointsy[i]);

		}
	}else{
		for(i=num;i<numpoints;i++) //zrusim prebytecny editboxy
		{
			DeleteBackElem(edpointsx[i]); edpointsx[i]=NULL;
			DeleteBackElem(edpointsy[i]); edpointsy[i]=NULL;
		}
	}

	numpoints = num;

	ok->Move(85,220.f+25*numpoints+10);
	cancel->Move(160,220.f+25*numpoints+10);
	copy->Move(10,225.f+25*numpoints+10);
	paste->Move(35,225.f+25*numpoints+10);

	SetBackWindowNormalSize(bwnsx,bwnsy+25*numpoints);
	SetBWLimitSizes(bwnsx,100,bwnsx,bwnsy+25*numpoints,0,1);
	Resize(230,295.f+25*numpoints);
	
	return 1;
}

void CEDElSourceEditor::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	int err=0;
	int syncout;
	CGUIEditWindow *ew;

	if(event->eventID == EClicked)
	{
		if(event->sender == edrampa->GetID()) //rampa checkbox
		{
			edrampa->Sync();
		}else
		if(event->sender == copy->GetID()) //copy
		{
			if(resourceEditor){
				if(!resourceEditor->elsclipboard)
					resourceEditor->elsclipboard = new CEDElSourceCopy;

				CEDElSourceCopy *ec = resourceEditor->elsclipboard;

				ec->pbx=pbx;ec->pby=pby;ec->pbz=pbz;
				ec->zdz=zdz;
				ec->flug=flug;ec->rampa=rampa;
				ec->numpoints=numpoints;
				for(int i=0;i<numpoints;i++)
				{
					ec->pointsx[i]=pointsx[i];
					ec->pointsy[i]=pointsy[i];
				}
				paste->Activate();
			}
		}else
		if(event->sender == paste->GetID()) //paste
		{
			if(resourceEditor && resourceEditor->elsclipboard){

				CEDElSourceCopy *ec = resourceEditor->elsclipboard;

				pbx=ec->pbx;pby=ec->pby;pbz=ec->pbz;
				zdz=ec->zdz;
				flug=ec->flug;rampa=ec->rampa;
				numpoints = ec->numpoints;
				for(int i=0;i<numpoints;i++)
				{
					pointsx[i] = ec->pointsx[i];
					pointsy[i] = ec->pointsy[i];
				}
				tmpnumpoints=numpoints;
				SyncAll(0);
			}
		}
	}else
	if(event->eventID == EOk)
	{
		ew = dynamic_cast<CGUIEditWindow*>(nameServer->ResolveID(event->sender));

		if(ew)
		{

			syncout = ew->Sync(); //zjistim novou hodnotu

			if(ew==ednumpoints) //pocet bodu zakladny
			{
				if(syncout==0)
				{
					//synchornizace ok -> byl zadan int
					if(!ChangeNumPoints(tmpnumpoints)) //zmenim # bodu
					{
						err=1; //chyba
						tmpnumpoints=numpoints; //nastavim puvodni pocet
					}
				}
			}
			if(syncout!=0 || err){ //chyba - byla zadana neplatna hodnota
				ew->BadInput();
				BadInput(event->sender);
			}
		}		
	}
	
	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}

int CEDElSourceEditor::SaveElSource()
{

	int ok;
	CGEnElSource *e=0;

	if(newresource)
	{
		e=new CGEnElSource();
	}else
		e=els;

	if(!e || !resourcename ) return 0;

	e->SetParams((float)pbx,(float)pby,(float)pbz,(float)zdz,rampa,flug);
	e->SetNumPoints(numpoints);
	for(int i=0; i<numpoints; i++)
	{
		e->BasePoints[i].x = (float) pointsx[i];
		e->BasePoints[i].y = (float) pointsy[i];
	}
	e->CalculateBorderPoints();

	ok = GEnMain->GetPackageMgr()->AddResource(resourcename, e, newresource);

	if(newresource)
		SAFE_RELEASE(e);

	if(resourceEditor)
	{
		CPackageBrowser* pb = resourceEditor->GetPackageBrowser();
		if(pb) pb->FileSysChanged();
	}

	return ok;
}

int CEDElSourceEditor::Ok()
{
	int sv = SaveElSource();
	if(!sv) {
		GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Nezdaøilo se uložit ElSource!",1,0xFFFF0000);
		return 0;
	}
	CloseWindow();
	return 1;
}

int CEDElSourceEditor::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		if(resourceEditor && resourceEditor->elsclipboard)
			paste->Activate();
	}

	return CGUIDlgOkCancel::FocusChanged(focus,dualID);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDLightTexEditor
//
// editor LightTexu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define ELSNAMELN 1000
#define COMPOFFCOL 0x40FFFFFF

CEDLightTexEditor::CEDLightTexEditor(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,300,480,"LightTex",0,true)
{
	autogrowing = 0;

	float xx,yy;
	GetBackWindowSize(xx,yy);
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	AddBackElem(new CGUIStaticText("ElSource:",font,10,15,STD_DLG_TEXT_COLOR));

	elsname = new char[ELSNAMELN+5];
	elsname[0]=0;

	edels = new CGUIEditWindow(70,13,125);
	edels->SetTabOrder(1.1f);
	edels->SelectOnFocus();
	edels->BindAttribute(elsname,dtString,ELSNAMELN+1,0);
	edels->AcceptEvent(GetID(),EOk);
	AddBackElem(edels);

	edels->Sync(0);

	butels  = new CGUIButton(200,13,20,20,"GUI.But.Std","...");
	butels->AcceptEvent(GetID(),EClicked);
	butels->AcceptEvent(GetID(),EStateChanged);
	butels->SetTabOrder(1.2f);
	butels->SetMark(1);
    AddBackElem(butels);

	//---
	CGUILine *ln;
	ln = new CGUILine(10,40,xx-10,40,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(0,0,1,1);
	AddBackElem(ln);
	//---
	
	sx=54;
	sy=54;
	shsx=70;shsy=70;
	shdx=shdy=0;

	int sxx,syy;
	if(sx<70) sxx=70; else sxx=sx;
	if(sy<20) syy=20; else syy=sy;

	compx[0]=10;
	compx[1]=compx[0]+sxx+20;
	compx[2]=compx[1]+sxx+20;
	compy[0]=50;
	compy[1]=compy[0]+syy+25;
	compy[2]=compy[1]+syy+25;
	shy = compy[2]+sy+25;
	if(shsy>=70) 
		shy2=shy+shsy+20; 
	else
		shy2=shy+70+20;

	isalpha=isamb=isdiff=isspec=isspec_h=isnormal=iszdepth=isatmosphere=0;

	CGUIStaticText *chktext;
	chktext = new CGUIStaticText("Alpha",font,0,0,STD_DLG_TEXT_COLOR);
	cbalpha = new CGUICheckBox(compx[0],compy[0],chktext,0);
	cbalpha->SetTabOrder(2.0f);
	cbalpha->SetMark(1);
	cbalpha->BindAttribute(&isalpha,dtInt,1,0);
	cbalpha->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbalpha);

	spalpha = new CGUIStaticPicture(compx[0],compy[0]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spalpha);

	chktext = new CGUIStaticText("Ambient",font,0,0,STD_DLG_TEXT_COLOR);
	cbamb = new CGUICheckBox(compx[1],compy[0],chktext,0);
	cbamb->SetTabOrder(3.0f);
	cbamb->SetMark(1);
	cbamb->BindAttribute(&isamb,dtInt,1,0);
	cbamb->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbamb);

	spamb = new CGUIStaticPicture(compx[1],compy[0]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spamb);

	chktext = new CGUIStaticText("Diffuse",font,0,0,STD_DLG_TEXT_COLOR);
	cbdiff = new CGUICheckBox(compx[2],compy[0],chktext,0);
	cbdiff->SetTabOrder(4.0f);
	cbdiff->SetMark(1);
	cbdiff->BindAttribute(&isdiff,dtInt,1,0);
	cbdiff->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbdiff);

	spdiff = new CGUIStaticPicture(compx[2],compy[0]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spdiff);

	chktext = new CGUIStaticText("Specular",font,0,0,STD_DLG_TEXT_COLOR);
	cbspec = new CGUICheckBox(compx[0],compy[1],chktext,0);
	cbspec->SetTabOrder(5.0f);
	cbspec->SetMark(1);
	cbspec->BindAttribute(&isspec,dtInt,1,0);
	cbspec->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbspec);

	spspec = new CGUIStaticPicture(compx[0],compy[1]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spspec);

	chktext = new CGUIStaticText("Spec_H",font,0,0,STD_DLG_TEXT_COLOR);
	cbspec_h = new CGUICheckBox(compx[1],compy[1],chktext,0);
	cbspec_h->SetTabOrder(6.0f);
	cbspec_h->SetMark(1);
	cbspec_h->BindAttribute(&isspec_h,dtInt,1,0);
	cbspec_h->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbspec_h);

	spspec_h = new CGUIStaticPicture(compx[1],compy[1]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spspec_h);

	chktext = new CGUIStaticText("Atmosphere",font,0,0,STD_DLG_TEXT_COLOR);
	cbatmosphere = new CGUICheckBox(compx[2],compy[1],chktext,0);
	cbatmosphere->SetTabOrder(6.5f);
	cbatmosphere->SetMark(1);
	cbatmosphere->BindAttribute(&isatmosphere,dtInt,1,0);
	cbatmosphere->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbatmosphere);

	spatmosphere = new CGUIStaticPicture(compx[2],compy[1]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spatmosphere);

	chktext = new CGUIStaticText("Normal",font,0,0,STD_DLG_TEXT_COLOR);
	cbnormal = new CGUICheckBox(compx[0],compy[2],chktext,0);
	cbnormal->SetTabOrder(7.0f);
	cbnormal->SetMark(1);
	cbnormal->BindAttribute(&isnormal,dtInt,1,0);
	cbnormal->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbnormal);

	spnormal = new CGUIStaticPicture(compx[0],compy[2]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spnormal);

	chktext = new CGUIStaticText("Z Depth",font,0,0,STD_DLG_TEXT_COLOR);
	cbzdepth = new CGUICheckBox(compx[1],compy[2],chktext,0);
	cbzdepth->SetTabOrder(8.0f);
	cbzdepth->SetMark(1);
	cbzdepth->BindAttribute(&iszdepth,dtInt,1,0);
	cbzdepth->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbzdepth);

	spzdepth = new CGUIStaticPicture(compx[1],compy[2]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(spzdepth);

	splightdemo = new CGUIStaticPicture(compx[2],compy[2]+20,(float)sx,(float)sy,COMPOFFCOL);
	AddBackElem(splightdemo);

//---
	ln1 = new CGUILine(10,shy,xx-10,shy,STD_DLG_BORDER_COLOR);
	ln1->SetAnchor(0,0,1,1);
	AddBackElem(ln1);
//---

	chktext = new CGUIStaticText("en{{casts shadow}}cs{{vrhá stín}}",font,0,0,STD_DLG_TEXT_COLOR);
	cbshadow = new CGUICheckBox(10,shy+15,chktext,0);
	cbshadow->SetTabOrder(20.0f);
	cbshadow->SetMark(1);
	cbshadow->BindAttribute(&isshadow,dtInt,1,0);
	cbshadow->AcceptEvent(GetID(),EClicked);
	AddBackElem(cbshadow);

	stshdx = new CGUIStaticText("en{{x shift:}}cs{{posun x:}}",font,10,shy+40,STD_DLG_TEXT_COLOR);
	AddBackElem(stshdx);

	edshdx = new CGUIEditWindow(65,shy+38,100);
	edshdx->SetTabOrder(21.0f);
	edshdx->SelectOnFocus();
	edshdx->BindAttribute(&shdx,dtInt,1,0);
	//edshdx->AcceptEvent(GetID(),EOk);
	AddBackElem(edshdx);


	stshdy = new CGUIStaticText("en{{y shift}}cs{{posun y:}}",font,10,shy+65,STD_DLG_TEXT_COLOR);
	AddBackElem(stshdy);

	edshdy = new CGUIEditWindow(65,shy+63,100);
	edshdy->SetTabOrder(22.0f);
	edshdy->SelectOnFocus();
	edshdy->BindAttribute(&shdy,dtInt,1,0);
	//edshdy->AcceptEvent(GetID(),EOk);
	AddBackElem(edshdy);

    spshadow = new CGUIStaticPicture(180,shy+10,(float)shsx,(float)shsy,COMPOFFCOL);
	AddBackElem(spshadow);

//---
	ln2 = new CGUILine(10,shy2,xx-10,shy2,STD_DLG_BORDER_COLOR);
	ln2->SetAnchor(0,0,1,1);
	AddBackElem(ln2);
//---

	butcrop = new CGUIButton(10,shy2+10,90,25,"GUI.But.Std","en{{Clip textures}}cs{{Oøež textury}}",0,"en{{Clips textures by elsource}}cs{{Oøeže textury podle elsourcu}}");
	butcrop->AcceptEvent(GetID(),EClicked);
	butcrop->SetTabOrder(100);
	butcrop->SetMark(1);
    AddBackElem(butcrop);

	butloadall = new CGUIButton(200,shy2+10,80,25,"GUI.But.Std","en{{Load All}}cs{{Nahraj vše}}",0,"en{{Loads all textures}}cs{{Nahraje všechny textury}}");
	butloadall->AcceptEvent(GetID(),EClicked);
	butloadall->SetTabOrder(102);
	butloadall->SetMark(1);
    AddBackElem(butloadall);

	ok = new CGUIButton(80,shy2+45,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(170,shy2+45,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edels,0);

	tex=NULL; tex_ld=NULL;
	tex_pkg=NULL;
	resourcename=NULL;
	pkg=NULL;

	elsselID = 0;
	importtexID = 0;
	importtexFilesDlgID = 0;
	importtexOKID = 0;
	fnalpha=fnamb=fndiff=fnspec=fnspec_h=fnnormal=fnzdepth=fnshadow=fnatmosphere=0;
}

CEDLightTexEditor::~CEDLightTexEditor()
{
	CGUITree *oldtr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
	if(oldtr && desktop) oldtr->CloseWindow();
	CGUIFileBrowser* oldfb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(importtexID));
	if(oldfb && desktop) oldfb->CloseWindow();


	DeleteAllFn();
	SAFE_DELETE_ARRAY(elsname);
	SAFE_DELETE_ARRAY(resourcename);
	SAFE_RELEASE(tex);
	SAFE_RELEASE(tex_pkg);
	SAFE_RELEASE(tex_ld);
}

void CEDLightTexEditor::DeleteAllFn()
{
	SAFE_DELETE_ARRAY(fnalpha);SAFE_DELETE_ARRAY(fnamb);SAFE_DELETE_ARRAY(fndiff);
	SAFE_DELETE_ARRAY(fnspec);SAFE_DELETE_ARRAY(fnspec_h);SAFE_DELETE_ARRAY(fnatmosphere);
	SAFE_DELETE_ARRAY(fnnormal);SAFE_DELETE_ARRAY(fnzdepth);
	SAFE_DELETE_ARRAY(fnshadow);
}

int CEDLightTexEditor::LoadLightTex(char *packagenm, char *resnm)
{
	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	CLightTex *lighttex = GEnMain->LoadLightTex(resource,&pkg);

	if(!lighttex) {
		delete[] resource;
		return 0;
	}

	char *tt = new char[strlen("LightTex - ")+strlen(resnm)+1];
	sprintf(tt,"LightTex - %s",resnm);
	title->SetText(tt);
	delete[] tt;

	resourcename = resource;

	SAFE_RELEASE(tex);
	SAFE_RELEASE(tex_pkg);

	tex_pkg=lighttex;
	tex_pkg->AddRef();
	tex = new CLightTex();
	tex->CopyFromTex(tex_pkg);

	tex->GetSize(sx,sy);
	ArrangeDlg();

	UpdateTex();

	CGEnElSource *els = tex->GetElSource();	
	if(els && pkg)
	{
		int ind = pkg->GetResourceIndex(els);
		if(ind!=-1)
		{
			const char *nm = pkg->GetResourceName(ind);
            strncpy(elsname,nm,ELSNAMELN);
			elsname[ELSNAMELN]=0;
		}
	}
	
	SyncAll(0);

	newresource=0;

	return 1;
}

int CEDLightTexEditor::CreateLightTex(char *packagenm, char *resnm)
{
	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	char *tt = new char[strlen("LightTex - ")+strlen(resnm)+1];
	sprintf(tt,"LightTex - %s",resnm);
	title->SetText(tt);
	delete[] tt;

	resourcename = resource;

	pkg = GEnMain->GetPackageMgr()->GetPackage(packagenm);

	SAFE_RELEASE(tex_pkg);
	SAFE_RELEASE(tex);
	tex=new CLightTex;
	tex->Create(54,54,0);
	sx=sy=54;

	ArrangeDlg();
	UpdateTex();

	SyncAll(0);

	newresource=1;

	return 1;
}

int CEDLightTexEditor::UpdateTex()
{
	CTexture *comptex;
#define updcomp(cm,comp) \
	if(comptex = tex->GetComponentTex(comp))\
	{\
		sp##cm->RebuildStaticPicture(comptex,-1,-1,(float)sx,(float)sy);\
		sp##cm->SetColor(0xFFffffff);\
		is##cm=1;\
		comptex->Release();\
	}else{\
		sp##cm->RebuildStaticPicture(NULL,-1,-1,(float)sx,(float)sy);\
		sp##cm->SetColor(COMPOFFCOL);\
		is##cm=0;\
	}
	
	updcomp(alpha,LgC_ALPHA);
	updcomp(amb,LgC_AMBIENT);
	updcomp(diff,LgC_DIFFUSE);
	updcomp(spec,LgC_SPECULAR);
	updcomp(spec_h,LgC_SPEC_H);
	updcomp(atmosphere,LgC_ATMOSPHERE);
	updcomp(normal,LgC_NORMAL);
	updcomp(zdepth,LgC_ZDEPTH);
	
	splightdemo->Resize((float)sx,(float)sy);
	SAFE_RELEASE(tex_ld);
	if(tex && tex->GetElSource() && GEnMain)
	{
		tex->CmpTex(&tex_ld);

		splightdemo->SetTexture(tex_ld);
		splightdemo->SetColor(0xffffffff);
	}else{
		splightdemo->SetTexture(NULL);
		splightdemo->SetColor(COMPOFFCOL);
	}

	comptex = tex->GetShadowTex();
	if(comptex){
		spshadow->RebuildStaticPicture(comptex);
		spshadow->SetColor(0xFFffffff);
		isshadow=1;
		comptex->Release();
	}else{
		spshadow->RebuildStaticPicture(NULL);
		spshadow->SetColor(COMPOFFCOL);
		isshadow=0;
	}

	isshadow = tex->IsShadow();
	if(isshadow)
		tex->GetShadowInfo(shsx,shsy,shdx,shdy);	

	return 1;
}

void CEDLightTexEditor::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	CGUIEventProgram *ep;
	CGUICheckBox *cb;
	CGUIButton *bt;
	CGUIEditWindow *eb;

	ep = nameServer->ResolveID(event->sender);

	//-----------------------------------------------------------------------------------
	eb = dynamic_cast<CGUIEditWindow*>(ep); //editboxy
	if(eb && event->eventID == EOk){
		if(eb == edels){

			CGEnElSource *els=NULL;
			if(eb->Sync(1)==0)
			{
				int ln = strlen(elsname);
				int ext=0;
				if(ln<=4||strcmp(elsname+ln-4,".els")!=0)
				{
					strcat(elsname,".els");ext=1;
				}
				els=(CGEnElSource*)pkg->LoadResource(elsname);
				if(els){
					tex->SetElSource(els);
					if(ext)eb->Sync(0);
				}
			}
			if(!els) {
				eb->BadInput();
				BadInput(event->sender);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	cb = dynamic_cast<CGUICheckBox*>(ep); //checkboxy
	if(cb){
		if(event->eventID == EClicked) 
		{
			cb->Sync(1);
			MarkComponents();
		}
	}
	//-----------------------------------------------------------------------------------
	bt = dynamic_cast<CGUIButton*>(ep); //buttony
	if(bt){
		if(event->eventID == EClicked)
		{
			if(bt == butels && pkg){ //vyber elsourcu

				CGUITree *oldtr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
				if(oldtr)
				{
					oldtr->FocusMe(); //uz existuje okno s elsourcy -> jen ho nafocusuju
				}else{

					float bx=0,by=0;
					desktop->GetDesktopPos(butels,bx,by); //zjistim souradnice tlacitka "..." v desktopu
					
					CGUITree *tr = new CGUITree(bx,by,200,300,"en{{Select ElSource:}}cs{{vyber ElSource:}}"); //vytvorim okno s els. v akt. packagi
					tr->title->SetButtons(true,false,false);
					desktop->AddBackElem(tr);
					desktop->SetFocusEl(tr,0);

					int nr=pkg->GetNumResources();
					for(int i=0;i<nr;i++) //pridam vsechny elsourcy
					{
						if( pkg->GetResourceType(i) == resElSource )
						{
							char *nm;
							nm = newstrdup(pkg->GetResourceName(i));
							CGUITreeItem *it = tr->AddTreeItem(0, nm);
							it->userTagType = eUTint;
							it->ut.pInt = i;
							delete[]nm;
						}
					}
					tr->UpdateTree();
					tr->Normalize();

					tr->AcceptEvent(GetID(),ETree);

					elsselID = tr->GetID();
				}
			}else
			if(bt == butloadall) //button LoadAll
			{
				CGUITree *oldfb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(importtexID));
				if(oldfb)
				{
					oldfb->CloseWindow(); //zavru stary okno
				}else{
					float bx=0,by=0;
					desktop->GetDesktopPos(butloadall,bx,by); //zjistim souradnice tlacitka v desktopu
					CGUIFileBrowser *fb = new CGUIFileBrowser(bx,by-300,300,300); //,"$IMPORTTEX$","IMPORTTEX",1);
					fb->AddRootItem("$IMPORTTEX$","IMPORTTEX",1);
					desktop->AddBackElem(fb);
					fb->FocusMe();

					fb->AcceptEvent(GetID(),ETree);

					importtexID = fb->GetID();
				}
			}else
			if(bt->GetID() == importtexOKID) //nahraju textury
			{
				int comps=0;
				char *ff=fndiff;

				if(ff)
				{
					CTexture *dtex=new CTexture();
					if(dtex->CreateFromFile(fndiff))
					{
						dtex->GetImageSize(&sx,&sy);

						if(fnalpha) comps|=LgC_ALPHA;
						if(fnamb) comps|=LgC_AMBIENT;
						if(fndiff) comps|=LgC_DIFFUSE;
						if(fnspec) comps|=LgC_SPECULAR;
						if(fnspec_h) comps|=LgC_SPEC_H;
						if(fnatmosphere) comps|=LgC_ATMOSPHERE;
						if(fnnormal) comps|=LgC_NORMAL;
						if(fnzdepth) comps|=LgC_ZDEPTH;
						
						tex->Create(sx,sy,comps);

						tex->SetAlpha(fnalpha);
						tex->SetAmbient(fnamb);
						tex->SetDiffuse(fndiff);
						tex->SetSpecular(fnspec);
						tex->SetSpec_H(fnspec_h);
						tex->SetAtmosphere(fnatmosphere);
						tex->SetNormal(fnnormal);
						tex->SetZDepth(fnzdepth);
						tex->SetShadow(fnshadow,shdx,shdy);
		           
						ArrangeDlg();
						UpdateTex();
						SyncAll(0);
					
					}
					dtex->Release();
				}

				//zavru okna
				CGUIDlgOkCancel *dlg = dynamic_cast<CGUIDlgOkCancel*>(nameServer->ResolveID(importtexFilesDlgID));
				if(dlg)
					dlg->CloseWindow();			
				CGUITree *oldfb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(importtexID));
				if(oldfb)
					oldfb->CloseWindow();
			}else
			if(bt == butcrop)
			{
				tex->ClearOutPixels();
				UpdateTex();
			}
		}
		else
		if(event->eventID == EStateChanged)
		{
			CGUIEditWindow *eww = GetObjFromID<CGUIEditWindow>(IDbadElem);
			if(eww == edels && bt == butels )
			{
				int cs = bt->GetState();
				
				if(cs == 2) 
					canceling = 1;
				else
				{
					if(IDbadElem){
						CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
						if(el) SetFocusEl(el,0);
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
		if(event->sender == elsselID && event->pInt2 == 0) //obsluha vyberu elsourcu
		{
			CGUITree *tr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
			{
				int index = it->ut.pInt;
				const char *resnm = pkg->GetResourceName(index);

				if(resnm)
				{
					strncpy(elsname,resnm,ELSNAMELN);
					elsname[ELSNAMELN]=0;
					edels->Sync(0);

					CGEnElSource *els=(CGEnElSource*)pkg->LoadResource(index);
					tex->SetElSource(els);
				}
				
			}		
			if(tr)
				tr->CloseWindow();
		}else
		if(event->sender == importtexID && event->pInt2 == 0) //obsluha importu
		{
			CGUIFileBrowser *fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(importtexID));

			char *dir=NULL,*file=NULL;
			int sf;

			sf = fb->GetSelectedFile(event->pID,&dir,&file);
			
			if(sf==0) //vybral se soubor
			{
				GenerateAllTexFilenames(dir,file);
			}

			SAFE_DELETE_ARRAY(dir);
			SAFE_DELETE_ARRAY(file);
							
		}
	}
	//------------------------------------------------------------------------------------


	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}

int CEDLightTexEditor::GenerateAllTexFilenames(char *dir, char *file)
{
	if(!dir||!file) return 0;

	#define IKA "_KRKAL Alpha"
	#define IKAM "_KRKAL Ambient"
	#define IKD "_KRKAL Diffuse"
	#define IKS "_KRKAL Specular"
	#define IKS_H "_KRKAL Spec_H"
	#define IKN "_KRKAL Normal"
	#define IKZ "_KRKAL Z Depth"
	#define IKSH "_KRKAL Shadow"
	#define IKAT "_KRKAL Atmosphere"

	#define IKA2 "_KRKAL_Alpha"
	#define IKAM2 "_KRKAL_Ambient"
	#define IKD2 "_KRKAL_Diffuse"
	#define IKS2 "_KRKAL_Specular"
	#define IKS_H2 "_KRKAL_Spec_H"
	#define IKN2 "_KRKAL_Normal"
	#define IKZ2 "_KRKAL_Z_Depth"
	#define IKSH2 "_KRKAL_Shadow"
	#define IKAT2 "_KRKAL_Atmosphere"


	char *ss;
	int ln=strlen(file),dir_ln=strlen(dir);
	int fs_ln = dir_ln+1+ln+strlen(IKD)+10;
	char *fs = new char [fs_ln], *fs2;
	int k_i,k_ln=0;
	int pref_ln,suff_i,suff_ln;

	int ki_ln,fok;

#define fndIK(IK) if(k_ln==0){ss = strstr(file,IK); if(ss){ k_ln = strlen(IK); k_i = (int)(ss - file); }} \
				  if(k_ln==0){ss = strstr(file,IK##2); if(ss){ k_ln = strlen(IK##2); k_i = (int)(ss - file); }}

	fndIK(IKA);
	fndIK(IKAM);
	fndIK(IKD);
	fndIK(IKS);
	fndIK(IKS_H);
	fndIK(IKN);
	fndIK(IKZ);
	fndIK(IKSH);
	fndIK(IKAT);

	if(k_ln){

		DeleteAllFn();

		float bx=0,by=0;
		desktop->GetDesktopPos(butloadall,bx,by); //zjistim souradnice tlacitka v desktopu
		CGUIDlgOkCancel *dlgfiles = new CGUIDlgOkCancel(bx,by-300,300,300,"en{{Load textures ...}}cs{{Nahraj textury...}}");
				
		CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");



		pref_ln = k_i;
		suff_i = k_i + k_ln;
		suff_ln = ln - suff_i;

		sprintf(fs,"%s/",dir);
		strncpy(fs+dir_ln+1,file,pref_ln);

#define insIK(IK) ki_ln = strlen(IK); strncpy(fs+dir_ln+1+pref_ln,IK,ki_ln); strncpy(fs+dir_ln+1+pref_ln+ki_ln,file+suff_i,suff_ln); fs[dir_ln+1+pref_ln+ki_ln+suff_ln]=0; fs2 = fs+dir_ln+1; fok = FS->GetFileSize(fs); \
				  if(!fok){ki_ln = strlen(IK##2); strncpy(fs+dir_ln+1+pref_ln,IK##2,ki_ln); strncpy(fs+dir_ln+1+pref_ln+ki_ln,file+suff_i,suff_ln); fs[dir_ln+1+pref_ln+ki_ln+suff_ln]=0; fs2 = fs+dir_ln+1; fok = FS->GetFileSize(fs);}
#define FCOL 0xFFFF0000

		float posy = 10,px1=10,px2=90;

		dlgfiles->AddBackElem(new CGUIStaticText("en{{directory:}}cs{{adresáø:}}",font,px1,posy,STD_DLG_TEXT_COLOR));
		dlgfiles->AddBackElem(new CGUIStaticText(dir,font,px2,posy,FCOL)); 
		posy+=40;

		insIK(IKA); 
		dlgfiles->AddBackElem(new CGUIStaticText("Alpha:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnalpha = newstrdup(fs);
		}
		posy+=20;

		insIK(IKAM); 
		dlgfiles->AddBackElem(new CGUIStaticText("Ambient:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnamb = newstrdup(fs);
		}
		posy+=20;
		
		insIK(IKD); 
		dlgfiles->AddBackElem(new CGUIStaticText("Diffuse:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fndiff = newstrdup(fs);
		}
		posy+=20;

		insIK(IKS); 
		dlgfiles->AddBackElem(new CGUIStaticText("Specular:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnspec = newstrdup(fs);
		}
		posy+=20;

		insIK(IKS_H); 
		dlgfiles->AddBackElem(new CGUIStaticText("Spec_H:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnspec_h = newstrdup(fs);
		}
		posy+=20;

		insIK(IKAT); 
		dlgfiles->AddBackElem(new CGUIStaticText("Atmosphere:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnatmosphere = newstrdup(fs);
		}
		posy+=20;

		insIK(IKN); 
		dlgfiles->AddBackElem(new CGUIStaticText("Normal:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnnormal = newstrdup(fs);
		}
		posy+=20;

		insIK(IKZ); 
		dlgfiles->AddBackElem(new CGUIStaticText("Z Depth:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnzdepth = newstrdup(fs);
		}
		posy+=20;

		insIK(IKSH); 
		dlgfiles->AddBackElem(new CGUIStaticText("Shadow:",font,px1,posy,STD_DLG_TEXT_COLOR));
		if(fok) {
			dlgfiles->AddBackElem(new CGUIStaticText(fs2,font,px2,posy,FCOL)); 
			fnshadow = newstrdup(fs);
		}
		posy+=20;

		posy+=10;

		dlgfiles->ok = new CGUIButton(80,posy,50,25,"GUI.But.Std","OK");
		dlgfiles->ok->SetTabOrder(1);
		dlgfiles->ok->SetMark(1);
		dlgfiles->ok->AcceptEvent(GetID(),EClicked);
		importtexOKID = dlgfiles->ok->GetID();
		dlgfiles->AddBackElem(dlgfiles->ok);

		dlgfiles->cancel = new CGUIButton(170,posy,50,25,"GUI.But.Std","Cancel");
		dlgfiles->cancel->SetTabOrder(2);
		dlgfiles->cancel->SetMark(1);
		dlgfiles->AddBackElem(dlgfiles->cancel);

		dlgfiles->InitOkCancel();

		desktop->AddBackElem(dlgfiles);
		dlgfiles->FocusMe();

		importtexFilesDlgID = dlgfiles->GetID();

	}

	
	delete[] fs;

	return 1;
}


void CEDLightTexEditor::MarkComponents()
{
#define markcomp(comp) \
	if(is##comp) \
		sp##comp->SetColor(0xFFFFFFFF);\
	else\
		sp##comp->SetColor(0x40FFFFFF);

	markcomp(alpha);
	markcomp(amb);
	markcomp(diff);
	markcomp(spec);
	markcomp(spec_h);
	markcomp(atmosphere);
	markcomp(normal);
	markcomp(zdepth);
	markcomp(shadow);
}

void CEDLightTexEditor::ArrangeDlg()
{

	int sxx,syy;
	if(sx<70) sxx=70; else sxx=sx;
	if(sy<20) syy=20; else syy=sy;

	compx[0]=10;
	compx[1]=compx[0]+sxx+20;
	compx[2]=compx[1]+sxx+20;
	compy[0]=50;
	compy[1]=compy[0]+syy+25;
	compy[2]=compy[1]+syy+25;
	shy = compy[2]+sy+25;
	if(shsy>=70) 
		shy2=shy+shsy+20; 
	else
		shy2=shy+70+20;

	float dxx,dyy;
	dxx=compx[2]+sxx+10;
	if(dxx<290) dxx=290;
	dyy=shy2+80;


//---
	cbalpha->Move(compx[0],compy[0]);
	spalpha->Move(compx[0],compy[0]+20);
	cbamb->Move(compx[1],compy[0]);
	spamb->Move(compx[1],compy[0]+20);
	cbdiff->Move(compx[2],compy[0]);
	spdiff->Move(compx[2],compy[0]+20);
	cbspec->Move(compx[0],compy[1]);
	spspec->Move(compx[0],compy[1]+20);
	cbspec_h->Move(compx[1],compy[1]);
	spspec_h->Move(compx[1],compy[1]+20);
	cbatmosphere->Move(compx[2],compy[1]);
	spatmosphere->Move(compx[2],compy[1]+20);
	cbnormal->Move(compx[0],compy[2]);
	spnormal->Move(compx[0],compy[2]+20);
	cbzdepth->Move(compx[1],compy[2]);
	spzdepth->Move(compx[1],compy[2]+20);

	splightdemo->Move(compx[2],compy[2]+20);
//---
	ln1->Move(10,shy);
//---
	cbshadow->Move(10,shy+15);
	
	stshdx->Move(10,shy+40);
	edshdx->Move(65,shy+38);
	stshdy->Move(10,shy+65);
	edshdy->Move(65,shy+63);
	spshadow->Move(180,shy+10);

//---
	ln2->Move(10,shy2);
//---

	butcrop->Move(10,shy2+10);
	butloadall->Move(200,shy2+10);

	ok->Move(80,shy2+45);
	cancel->Move(170,shy2+45);


	SetBackWindowNormalSize(dxx,dyy);
	SetBWLimitSizes(0,0,dxx,dyy,0,1);
	SetWindowSizeVP(dxx,dyy);

}

int CEDLightTexEditor::Ok()
{
	int sv = SaveLightTex();
	if(!sv) {
		GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Nezdaøilo se uložit LightTex!",1,0xFFFF0000);
		return 0;
	}
	CloseWindow();
	return 1;
}

int CEDLightTexEditor::SaveLightTex()
{
	int ok;
	if(!tex || !resourcename ) return 0;

	SyncAll();
	tex->SetShadowPos(shdx,shdy);

	tex->ChangeID();

	if(newresource)
	{
		ok = GEnMain->GetPackageMgr()->AddResource(resourcename, tex, newresource);
	}else{
		tex_pkg->CopyFromTex(tex);
		ok = GEnMain->GetPackageMgr()->AddResource(resourcename, tex_pkg, newresource);
	}

	if(resourceEditor)
	{
		CPackageBrowser* pb = resourceEditor->GetPackageBrowser();
		if(pb) pb->FileSysChanged();
	}

	return ok;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

CEDManyLightTexEditor::CEDManyLightTexEditor(float _x, float _y, char *pkgdir)
:CGUIDlgOkCancel(_x,_y,300,160,"mraky LightTexù",0,true)
{
	float xx,yy,xx2;
	GetBackWindowSize(xx,yy);
	xx2=(float)floor(xx/2);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	AddBackElem(new CGUIStaticText("ElSource:",font,10,15,STD_DLG_TEXT_COLOR));

	elsname = new char[ELSNAMELN+5];
	elsname[0]=0;

	edels = new CGUIEditWindow(70,13,125);
	edels->SetTabOrder(1.1f);
	edels->SelectOnFocus();
	edels->BindAttribute(elsname,dtString,ELSNAMELN+1,0);
	edels->AcceptEvent(GetID(),EOk);
	AddBackElem(edels);

	edels->Sync(0);

	butels  = new CGUIButton(200,13,20,20,"GUI.But.Std","...");
	butels->AcceptEvent(GetID(),EClicked);
	butels->AcceptEvent(GetID(),EStateChanged);
	butels->SetTabOrder(1.2f);
	butels->SetMark(1);
    AddBackElem(butels);

	butseldir = new CGUIButton(xx2-100,50,200,25,"GUI.But.Std","en{{Choose directory with pictures ...}}cs{{Vyber adresáø s obrázky...}}");
	butseldir->AcceptEvent(GetID(),EClicked);
	butseldir->SetTabOrder(2);
	butseldir->SetMark(1);
    AddBackElem(butseldir);

	//---
	CGUILine *ln;
	ln = new CGUILine(10,90,xx-10,90,STD_DLG_BORDER_COLOR);
	ln->SetAnchor(0,0,1,1);
	AddBackElem(ln);
	//---

	ok = new CGUIButton(xx2-60,100,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(xx2+10,100,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(edels,0);

	pkg=0;
	pkg = GEnMain->GetPackageMgr()->GetPackage(pkgdir);
	assert(pkg);

	elsselID = 0;
	seldirID = 0;
	texdir=0;

}

CEDManyLightTexEditor::~CEDManyLightTexEditor()
{
	DeleteAllTexFn();
	CGUITree *oldtr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
	if(oldtr && desktop) oldtr->CloseWindow();
	CGUIFileBrowser* oldfb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(seldirID));
	if(oldfb && desktop) oldfb->CloseWindow();
	SAFE_DELETE_ARRAY(elsname);
	SAFE_DELETE_ARRAY(texdir);
}

void CEDManyLightTexEditor::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	CGUIEventProgram *ep;
	CGUIButton *bt;
	CGUIEditWindow *eb;

	ep = nameServer->ResolveID(event->sender);

	//-----------------------------------------------------------------------------------
	eb = dynamic_cast<CGUIEditWindow*>(ep); //editboxy
	if(eb && event->eventID == EOk){
		if(eb == edels){

			CGEnElSource *els=NULL;
			if(eb->Sync(1)==0)
			{
				int ln = strlen(elsname);
				int ext=0;
				if(ln<=4||strcmp(elsname+ln-4,".els")!=0)
				{
					strcat(elsname,".els");ext=1;
				}
				els=(CGEnElSource*)pkg->LoadResource(elsname);
				if(els){
//					tex->SetElSource(els);
					if(ext)eb->Sync(0);
				}
			}
			if(!els) {
				eb->BadInput();
				BadInput(event->sender);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	bt = dynamic_cast<CGUIButton*>(ep); //buttony
	if(bt){
		if(event->eventID == EClicked)
		{
			if(bt == butels && pkg){ //vyber elsourcu

				CGUITree *oldtr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
				if(oldtr)
				{
					oldtr->FocusMe(); //uz existuje okno s elsourcy -> jen ho nafocusuju
				}else{

					float bx=0,by=0;
					desktop->GetDesktopPos(butels,bx,by); //zjistim souradnice tlacitka "..." v desktopu
					
					CGUITree *tr = new CGUITree(bx,by,200,300,"en{{Select ElSource:}}cs{{vyber ElSource:}}"); //vytvorim okno s els. v akt. packagi
					tr->title->SetButtons(true,false,false);
					desktop->AddBackElem(tr);
					desktop->SetFocusEl(tr,0);

					int nr=pkg->GetNumResources();
					for(int i=0;i<nr;i++) //pridam vsechny elsourcy
					{
						if( pkg->GetResourceType(i) == resElSource )
						{
							char *nm;
							nm = newstrdup(pkg->GetResourceName(i));
							CGUITreeItem *it = tr->AddTreeItem(0, nm);
							it->userTagType = eUTint;
							it->ut.pInt = i;
							delete[]nm;
						}
					}
					tr->UpdateTree();
					tr->Normalize();

					tr->AcceptEvent(GetID(),ETree);

					elsselID = tr->GetID();
				}
			}else
			if(bt == butseldir) //button vyber adresare
			{
				CGUITree *oldfb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(seldirID));
				if(oldfb)
				{
					oldfb->CloseWindow(); //zavru stary okno
				}else{
					float bx=0,by=0;
					desktop->GetDesktopPos(butseldir,bx,by); //zjistim souradnice tlacitka v desktopu
					CGUIFileBrowser *fb = new CGUIFileBrowser(bx,by,300,300); //,"$IMPORTTEX$","IMPORTTEX",1);
					fb->AddRootItem("$IMPORTTEX$","IMPORTTEX",1);
					fb->rollOnClick = 0;

					desktop->AddBackElem(fb);
					fb->FocusMe();

					fb->AcceptEvent(GetID(),ETree);

					seldirID = fb->GetID();
				}
			}
		}else
		if(event->eventID == EStateChanged)
		{
			CGUIEditWindow *eww = GetObjFromID<CGUIEditWindow>(IDbadElem);
			if(eww == edels && bt == butels )
			{
				int cs = bt->GetState();
				
				if(cs == 2) 
					canceling = 1;
				else
				{
					if(IDbadElem){
						CGUIElement *el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(IDbadElem));
						if(el) SetFocusEl(el,0);
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
		if(event->sender == elsselID && event->pInt2 == 0) //obsluha vyberu elsourcu
		{
			CGUITree *tr = dynamic_cast<CGUITree*>(nameServer->ResolveID(elsselID));
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
			{
				int index = it->ut.pInt;
				const char *resnm = pkg->GetResourceName(index);

				if(resnm)
				{
					strncpy(elsname,resnm,ELSNAMELN);
					elsname[ELSNAMELN]=0;
					edels->Sync(0);

					CGEnElSource *els=(CGEnElSource*)pkg->LoadResource(index);
//					tex->SetElSource(els);
				}
				
			}		
			if(tr)
				tr->CloseWindow();
		}else
		if(event->sender == seldirID && event->pInt2 == 0) //obsluha importu
		{
			CGUIFileBrowser *fb = dynamic_cast<CGUIFileBrowser*>(nameServer->ResolveID(seldirID));

			if(fb){
				char *dir=NULL,*file=NULL;
				int sf;

				sf = fb->GetSelectedFile(event->pID,&dir,&file);
				
				if(sf>=0) //vybral se soubor nebo adresar
				{
					FindAllTex(dir);
					fb->CloseWindow();
				}

				SAFE_DELETE_ARRAY(dir);
				SAFE_DELETE_ARRAY(file);
			}
		}
	}
	//------------------------------------------------------------------------------------


	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}


int CEDManyLightTexEditor::FindAllTex( char *dir )
{
	char *olddir=NULL;
	int ok=0;
	const char *fn;
	int d,f;
	char *ff,*tn;

	DeleteAllTexFn();
	SAFE_DELETE(texdir);

	if(!dir) return 0;

	if(!FS->GetCurDir(&olddir)) return 0;

	if(FS->ChangeDir(dir))
	{
		f = FS->FindFirst(&fn,d);
		while(f)
		{
			if(!d)
			{
				if( IsTex(fn)&& (tn=GenerateTexName(fn))!=0 )
				{
					ff = newstrdup(fn);
					texfilenames.Add(ff);
					texnames.Add(tn);
				}
				ok=1;
			}
			f = FS->FindNext(&fn,d);
		}
		FS->FindClose();
	}

	FS->ChangeDir(olddir);
	SAFE_DELETE_ARRAY(olddir);

	if(ok)
		texdir = newstrdup(dir);

	ShowTexNames();
	return ok;
}

void CEDManyLightTexEditor::ShowTexNames()
{
	DeleteAllSTtexFn();

	CListIterator<char*> it(texfilenames);
	CListIterator<char*> it2(texnames);
	char *el, *el2;
	CGUIStaticText *st;

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");
	float yy=100;
	int aa=0;

	char *tm = new char[1001];

	while(!it.End()){
		el2 = it2;
		el = it;

		_snprintf(tm,1000,"%s (%s)",el2,el);

		st = new CGUIStaticText(tm,font,10,yy,STD_DLG_TEXT_COLOR);
		AddBackElem(st);
		STtexFN.Add(st);


		yy+=15;

		it++;it2++;
		aa++;
	}

	delete[]tm;

	if(aa) yy+=10;

	float x,y;
	ok->GetPos(x,y);ok->Move(x,yy);
	cancel->GetPos(x,y);cancel->Move(x,yy);

	float bwnsx,bwnsy;
	GetBackWindowNormalSize(bwnsx,bwnsy);

	bwnsy=136;

	yy-=100;

	SetBackWindowNormalSize(bwnsx,bwnsy+yy);
	if(yy>140) yy=140;
	Resize(bwnsx+20,160+yy);
}

int CEDManyLightTexEditor::IsTex( const char *fn )
{
	#define IKD "_KRKAL Diffuse"
	#define IKD2 "_KRKAL_Diffuse"

	int ok=0;

	if( strstr(fn,IKD) || strstr(fn,IKD2) ) {
		CTexture *t=new CTexture();
		if(t->CreateFromFile(fn)) ok=1;
		t->Release();
	}

	return ok;
}

void CEDManyLightTexEditor::DeleteAllTexFn()
{
	DeleteAllSTtexFn();

	char* el;

	{
		CListIterator<char*> it(texfilenames);
		while(!it.End()){
			el = it;
			SAFE_DELETE_ARRAY( el );
			it++;
		}
		texfilenames.DeleteAll();
	}

	{
		CListIterator<char*> it(texnames);
		while(!it.End()){
			el = it;
			SAFE_DELETE_ARRAY( el );
			it++;
		}
		texnames.DeleteAll();
	}
}

void CEDManyLightTexEditor::DeleteAllSTtexFn()
{
	CListIterator<CGUIStaticText*> it(STtexFN);
	CGUIStaticText* el;
	while(!it.End()){
		el = it;
		DeleteBackElem(el);
		it++;
	}
	STtexFN.DeleteAll();
}

int CEDManyLightTexEditor::Ok()
{
	if(texdir)
	{
		CListIterator<char*> it(texfilenames);
		CListIterator<char*> it2(texnames);
		char* el,*el2;
		while(!it.End()){
			el = it;
			el2= it2;
			
			CreateLightTex(el2,el);

			it++;it2++;
		}
	}

	if(resourceEditor)
	{
		CPackageBrowser* pb = resourceEditor->GetPackageBrowser();
		if(pb) pb->FileSysChanged();
	}


	CloseWindow();
	return 1;
}

int CEDManyLightTexEditor::CreateLightTex(char *texname, char *fn)
{

	if(!pkg) return 0;
	if(!texdir) return 0;

	char *olddir;
	int ok=0;

	CGEnElSource *els;
	if(pkg->GetResourceType(elsname)!=resElSource) return 0;

	els = (CGEnElSource*)pkg->LoadResource(elsname);

	if(!FS->GetCurDir(&olddir)) return 0;

	if(FS->ChangeDir(texdir))
	{
		eResourceType rt = pkg->GetResourceType(texname);
		CLightTex *t;
		int newtex=0;

		CTexture *tt = new CTexture();
		if(!tt->CreateFromFile(fn))
		{
			delete tt;
			FS->ChangeDir(olddir);
			delete[]olddir;
			return 0;
		}


		UINT sx,sy;
		tt->GetImageSize(&sx,&sy);
		
		delete tt;


		if(rt==resNone)
		{
			t=new CLightTex();
			newtex=1;
		}else
		if(rt==resLightTex)
		{
			t=(CLightTex*)pkg->LoadResource(texname);
		}else
		{
			FS->ChangeDir(olddir);
			delete[]olddir;
			return 0;
		}

		#define IKA "_KRKAL Alpha"
		#define IKAM "_KRKAL Ambient"
		#define IKD "_KRKAL Diffuse"
		#define IKS "_KRKAL Specular"
		#define IKS_H "_KRKAL Spec_H"
		#define IKN "_KRKAL Normal"
		#define IKZ "_KRKAL Z Depth"
		#define IKSH "_KRKAL Shadow"
		#define IKAT "_KRKAL Atmosphere"

		#define IKA2 "_KRKAL_Alpha"
		#define IKAM2 "_KRKAL_Ambient"
		#define IKD2 "_KRKAL_Diffuse"
		#define IKS2 "_KRKAL_Specular"
		#define IKS_H2 "_KRKAL_Spec_H"
		#define IKN2 "_KRKAL_Normal"
		#define IKZ2 "_KRKAL_Z_Depth"
		#define IKSH2 "_KRKAL_Shadow"
		#define IKAT2 "_KRKAL_Atmosphere"

		int KDpos,KDlen,ln;
		char *tm;
		char *ff;
		int shx=0,shy=0;
		UI comps=0;

		tm = strstr(fn,IKD); KDlen=strlen(IKD);
		if(!tm) {tm = strstr(fn,IKD2);KDlen=strlen(IKD2);}

		assert(tm);
	    
		ln=strlen(fn);
		KDpos = (int)(tm-fn);

		comps=LgC_DIFFUSE;

		ff = new char [ln+40];
		strncpy(ff,fn,KDpos);

#define TESTIK(IK,LgC) strcpy(ff+KDpos,IK); strcat(ff+KDpos,fn+KDpos+KDlen); if(FS->GetFileSize(ff)) comps|=LgC; else\
					   {strcpy(ff+KDpos,IK##2); strcat(ff+KDpos,fn+KDpos+KDlen); if(FS->GetFileSize(ff)) comps|=LgC;}

		TESTIK(IKA,LgC_ALPHA);
		TESTIK(IKAM,LgC_AMBIENT);
		TESTIK(IKS,LgC_SPECULAR);
		TESTIK(IKS_H,LgC_SPEC_H);
		TESTIK(IKN,LgC_NORMAL);
		TESTIK(IKZ,LgC_ZDEPTH);
		TESTIK(IKAT,LgC_ATMOSPHERE);

		if(!newtex&&t->IsShadow()){
			int xx,yy;
			t->GetShadowInfo(xx,yy,shx,shy);
		}

		if(t->Create(sx,sy,comps))
		{
			t->SetElSource(els);
			ok=1;
			#define ADDIK(IK,LgC,SetIK) \
				if(comps&LgC){\
					strcpy(ff+KDpos,IK); strcat(ff+KDpos,fn+KDpos+KDlen); \
					if(!t->SetIK(ff)){\
						strcpy(ff+KDpos,IK##2); strcat(ff+KDpos,fn+KDpos+KDlen); \
						if(!t->SetIK(ff)) ok=0;\
					}\
				}

			ADDIK(IKD,LgC_DIFFUSE,SetDiffuse);
			ADDIK(IKA,LgC_ALPHA,SetAlpha);
			ADDIK(IKAM,LgC_AMBIENT,SetAmbient);
			ADDIK(IKS,LgC_SPECULAR,SetSpecular);
			ADDIK(IKS_H,LgC_SPEC_H,SetSpec_H);
			ADDIK(IKN,LgC_NORMAL,SetNormal);
			ADDIK(IKZ,LgC_ZDEPTH,SetZDepth);
			ADDIK(IKAT,LgC_ATMOSPHERE,SetAtmosphere);

			strcpy(ff+KDpos,IKSH); strcat(ff+KDpos,fn+KDpos+KDlen); 
			if(!t->SetShadow(ff,shx,shy)){
				strcpy(ff+KDpos,IKSH2); strcat(ff+KDpos,fn+KDpos+KDlen); 
				t->SetShadow(ff,shx,shy);
			}

			
		}

		delete[]ff;

		if(ok)
		{
			pkg->AddResource(texname,t,newtex);
		}

		if(newtex) t->Release();

		ok=1;
	
	}

	FS->ChangeDir(olddir);
	delete[]olddir;

	return ok;
}

char* CEDManyLightTexEditor::GenerateTexName(const char *fn)
{
	#define IKD "_KRKAL Diffuse"
	#define IKD2 "_KRKAL_Diffuse"

	char *texname=0;
	const char *tm;
	int ln,lnD,ln2;
	int KDpos;
	int DotPos=-1;

	tm = strstr(fn,IKD); lnD=strlen(IKD);
	if(!tm) {tm = strstr(fn,IKD2);lnD=strlen(IKD2);}

	if(tm)
	{
		ln=strlen(fn);
		KDpos = (int)(tm-fn);

		tm = strrchr( fn, '.' );
		if(tm) {
			DotPos = (int)(tm-fn);
			if(DotPos<KDpos) DotPos=-1;
		}

		ln2=ln-KDpos-lnD;
		if(DotPos!=-1){
			ln2-=ln-DotPos;
		}

		texname = new char[KDpos+1+ln2+4+1];
		strncpy(texname,fn,KDpos);

		if(ln2)
		{
			texname[KDpos]='_';
			strncpy(texname+KDpos+1,fn+KDpos+lnD,ln2);
			texname[KDpos+1+ln2]=0;
		}else
			texname[KDpos]=0;
		
		strcat(texname,".tex");

	}
	
	return texname;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CEDAnimEditor
//
// editor animaci
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define FRAMENAME_LN 200

CEDAnimEditor::CEDAnimEditor(float _x, float _y)
:CGUIDlgOkCancel(_x,_y,250,220,"en{{Animation}}cs{{Animace}}",0,true)
{
	autogrowing = 0;

	MsgAcceptConsume(MsgTimer);

	float xx,yy;
	GetBackWindowSize(xx,yy);
	
	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");


	maxsx=maxsy=54;
	float x2=xx/2;


	spanim = new CGUIStaticPicture(x2-maxsx/2,10,maxsx,maxsy,COMPOFFCOL);
	AddBackElem(spanim);

	// |< << || > >> >|

	butfirst = new CGUIButton(x2-(10+25)*3+5,10+maxsy+10,25,25,"GUI.But.Std","|<");
	butfirst->SetMark(1);
	butfirst->Deactivate();
	butfirst->SetTabOrder(1);
	butfirst->AcceptEvent(GetID(),EClicked);
	AddBackElem(butfirst);

	butprev = new CGUIButton(x2-(10+25)*2+5,10+maxsy+10,25,25,"GUI.But.Std","<<");
	butprev->SetMark(1);
	butprev->Deactivate();
	butprev->SetTabOrder(2);
	butprev->AcceptEvent(GetID(),EClicked);
	AddBackElem(butprev);

	butstop = new CGUIButton(x2-5-25,10+maxsy+10,25,25,"GUI.But.Std","| |");
	butstop->SetMark(1);
	butstop->Deactivate();
	butstop->SetTabOrder(3);
	butstop->AcceptEvent(GetID(),EClicked);
	AddBackElem(butstop);

	butplay = new CGUIButton(x2+5,10+maxsy+10,25,25,"GUI.But.Std",">");
	butplay->SetMark(1);
	butplay->Deactivate();
	butplay->SetTabOrder(4);
	butplay->AcceptEvent(GetID(),EClicked);
	AddBackElem(butplay);

	butnext = new CGUIButton(x2+5+(25+10),10+maxsy+10,25,25,"GUI.But.Std",">>");
	butnext->SetMark(1);
	butnext->Deactivate();
	butnext->SetTabOrder(5);
	butnext->AcceptEvent(GetID(),EClicked);
	AddBackElem(butnext);

	butlast = new CGUIButton(x2+5+(25+10)*2,10+maxsy+10,25,25,"GUI.But.Std",">|");
	butlast->SetMark(1);
	butlast->Deactivate();
	butlast->SetTabOrder(6);
	butlast->AcceptEvent(GetID(),EClicked);
	AddBackElem(butlast);

	float y1 = 10+maxsy+10+35;
	float y2=y1+10;
//---
	ln1 = new CGUILine(10,y1,xx-10,y1,STD_DLG_BORDER_COLOR);
	ln1->SetAnchor(0,0,1,1);
	AddBackElem(ln1);
//---
	cbrepeat = new CGUICheckBox(50,y2,new CGUIStaticText("en{{repeat}}cs{{opakovat}}",font,0,0,STD_DLG_TEXT_COLOR),0);
	cbrepeat->SetTabOrder(2.0f);
	cbrepeat->SetMark(1);
	cbrepeat->BindAttribute(&repeat,dtInt,1,0);
	cbrepeat->AcceptEvent(GetID(),EClicked);
	cbrepeat->SetTabOrder(7);
	AddBackElem(cbrepeat);

	butaddfirst = new CGUIButton(10,y2,17,17,"GUI.But.Std","+");
	butaddfirst->SetMark(1);
	butaddfirst->SetTabOrder(8);
	butaddfirst->AcceptEvent(GetID(),EClicked);
	AddBackElem(butaddfirst);
	y2+=17+10;
	
//---
	ln2 = new CGUILine(10,y2,xx-10,y2,STD_DLG_BORDER_COLOR);
	ln2->SetAnchor(0,0,1,1);
	AddBackElem(ln2);
//---



	ok = new CGUIButton(x2-10-50,y2+10,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1000);
	ok->SetMark(1);
    AddBackElem(ok);

	cancel = new CGUIButton(x2+10,y2+10,50,25,"GUI.But.Std","Cancel");
	cancel->SetTabOrder(1001);
	cancel->SetMark(1);
    AddBackElem(cancel);

	InitOkCancel();

	SetFocusEl(butaddfirst,0);

	anim=NULL; numframes=0; curframe=0;
	texseldlgID=0;

	resourcename=NULL;
	pkg=NULL;
	playing=0;

	UpdateCurFrame();
	ArrangeDlg();
}

CEDAnimEditor::~CEDAnimEditor()
{
	CloseTexSelDlg();
	timerGUI->DeleteAllRequests(this);
	for(int i=0;i<numframes;i++)
	{
		SAFE_RELEASE(frames[i]);
		SAFE_RELEASE(frametex[i]);
		SAFE_DELETE_ARRAY(framenames[i]);
	}

	SAFE_DELETE_ARRAY(resourcename);
	SAFE_RELEASE(anim);
}

int CEDAnimEditor::LoadAnim(char *packagenm, char *resnm)
{
	CloseTexSelDlg();

	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	CGEnAnim *tm_anim = GEnMain->LoadAnim(resource,&pkg);

	if(!tm_anim) {
		delete[] resource;
		return 0;
	}

	DeleteAllFrames();

	char *tt = new char[strlen("Animace - ")+strlen(resnm)+1];
	sprintf(tt,"Animace - %s",resnm);
	title->SetText(tt);
	delete[] tt;

	resourcename = resource;
	anim=tm_anim;
	anim->AddRef();

	playing=0;
	curframe=0;

	int i;
	for(i=0;i<numframes;i++) {
		SAFE_RELEASE(frames[i]);
		SAFE_RELEASE(frametex[i]);
		SAFE_DELETE_ARRAY(framenames[i]);
	}

	repeat = anim->repeatflag;

	numframes = anim->GetNumFrames();
	if(numframes>MAXFRAMES) numframes = MAXFRAMES;
	if(numframes)
	{

		CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

		float yy = 10+maxsy+10+35 + 10 + 17 + 5;
		char frnum[]="200:";
		int fn;
		UI sxx,syy;

		for(i=0;i<numframes;i++){
			frames[i] = anim->GetFrameTex(i);
			frames[i]->AddRef();
			times[i] = anim->GetFrameTime(i);

			fn=i+1;
			sprintf(frnum,"%i:",fn);
			stframenum[i] = new CGUIStaticText(frnum,font,10,yy,STD_DLG_TEXT_COLOR);
			AddBackElem(stframenum[i]);
			stframetime[i] = new CGUIStaticText("en{{time:}}cs{{èas:}}",font,60,yy+22,STD_DLG_TEXT_COLOR);
			AddBackElem(stframetime[i]);

			edframename[i] = new CGUIEditWindow(50,yy-2,90);
			edframename[i]->SelectOnFocus();
			edframename[i]->SetMark(1);
			edframename[i]->SetTabOrder(10+i+0.1f);
			edframename[i]->AcceptEvent(GetID(),EOk);			
			AddBackElem(edframename[i]);

			edframetime[i] = new CGUIEditWindow(90,yy+20,50);
			edframetime[i]->SelectOnFocus();
			edframetime[i]->SetMark(1);
			edframetime[i]->BindAttribute(&times[i],dtInt,1,0);
			edframetime[i]->SetTabOrder(10+i+0.2f);
			edframetime[i]->AcceptEvent(GetID(),EOk);
			AddBackElem(edframetime[i]);

			butadd[i] = new CGUIButton(10,yy+21,17,17,"GUI.But.Std","+",0,"en{{Adds frame}}cs{{Pøidá obrázek}}");
			butadd[i]->SetMark(1);
			butadd[i]->SetTabOrder(10 + i + 0.9f);
			butadd[i]->AcceptEvent(GetID(),EClicked);
			AddBackElem(butadd[i]);

			butdel[i] = new CGUIButton(31,yy+21,17,17,"GUI.But.Std","x",0,"en{{Deletes frame}}cs{{Smaže obrázek}}");
			butdel[i]->SetMark(1);
			butdel[i]->SetTabOrder(10 + i + 0.95f);
			butdel[i]->AcceptEvent(GetID(),EClicked);
			AddBackElem(butdel[i]);

			frames[i]->CmpTex(&frametex[i]);
			frames[i]->GetSize(sxx,syy);
			framepic[i] = new CGUIStaticPicture(150,yy,frametex[i],(float)sxx,(float)syy);
			framepic[i]->throw_going=false;
			framepic[i]->AcceptEvent(GetID(),EMouseButton);
			AddBackElem(framepic[i]);
			if(syy>37)
				yy+=syy+5;
			else
				yy+=42;

			
			if(frames[i] && pkg)
			{
				int ind = pkg->GetResourceIndex(frames[i]);
				if(ind!=-1)
				{
					const char *nm = pkg->GetResourceName(ind);
					int ln=strlen(nm);
					if(ln<FRAMENAME_LN) ln=FRAMENAME_LN;
					framenames[i] = new char[ln+5];
					strcpy(framenames[i],nm);
					edframename[i]->BindAttribute(framenames[i],dtString,ln,0);
				}
			}

		}
	}

	UpdateCurFrame();
	ArrangeDlg();

	SyncAll(0);

	newresource=0;

	return 1;
}

int CEDAnimEditor::CreateAnim(char *packagenm, char *resnm)
{
	CloseTexSelDlg();

	DeleteAllFrames();

	char *resource=new char[strlen(packagenm)+1+strlen(resnm)+1];
	sprintf(resource,"%s/%s",packagenm,resnm);

	char *tt = new char[strlen("Animace - ")+strlen(resnm)+1];
	sprintf(tt,"Animace - %s",resnm);
	title->SetText(tt);
	delete[] tt;

	resourcename = resource;

	pkg = GEnMain->GetPackageMgr()->GetPackage(packagenm);

	SAFE_RELEASE(anim);

	playing=0;
	curframe=0;

	int i;
	for(i=0;i<numframes;i++) {
		SAFE_RELEASE(frames[i]);
		SAFE_RELEASE(frametex[i]);
		SAFE_DELETE_ARRAY(framenames[i]);
	}
	numframes=0;

	SyncAll(0);

	UpdateCurFrame();
	ArrangeDlg();

	newresource=1;

	return 1;
}

void CEDAnimEditor::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	CGUIEventProgram *ep;
	CGUICheckBox *cb=NULL;
	CGUIButton *bt=NULL;
	CGUIEditWindow *eb=NULL;
	CGUIStaticPicture *sp=NULL;

	ep = nameServer->ResolveID(event->sender);

	do{
	eb = dynamic_cast<CGUIEditWindow*>(ep); if(eb)break; //editboxy 
	cb = dynamic_cast<CGUICheckBox*>(ep); if(cb)break;//checkboxy
	bt = dynamic_cast<CGUIButton*>(ep); if(bt)break;//buttony
	sp = dynamic_cast<CGUIStaticPicture*>(ep); if(sp)break; //StaticPicure
	}while(0);

	//-----------------------------------------------------------------------------------
	if(eb && event->eventID == EOk){
		for(int i=0;i<numframes;i++)
		{
			if(eb == edframename[i] ){

				CLightTex *lt=NULL;
				
				if(eb->Sync(1)==0)
				{
					int ln = strlen(framenames[i]);
					int ext=0;
					if(ln<=4||strcmp(framenames[i]+ln-4,".tex")!=0)
					{
						strcat(framenames[i],".tex");ext=1;
					}

                    lt=(CLightTex*)pkg->LoadResource(framenames[i]);
					if(lt){
						SAFE_RELEASE(frametex[i]); SAFE_RELEASE(frames[i]);
						frames[i]=lt;frames[i]->AddRef();
						frames[i]->CmpTex(&frametex[i]);
						framepic[i]->RebuildStaticPicture(frametex[i]);
						if(ext)eb->Sync(0);

						UpdateCurFrame();
						ArrangeDlg();

					}
					
				}
				if(!lt) {
					eb->BadInput();
					BadInput(event->sender);
				}
				break;
				
			}else
			if(eb == edframetime[i]){
				int err=0;
				int ot = times[i];
				if(eb->Sync(1)==0)
				{
					if(times[i]<=0) err=1;
				}else err=1;
				if(err){
					times[i]=ot;
					eb->BadInput();
					BadInput(event->sender);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------
	if(cb){
		if(event->eventID == EClicked) 
		{
			cb->Sync(1);
		}
	}
	//-----------------------------------------------------------------------------------
	if(bt){
		if(event->eventID == EClicked)
		{
			if(bt==butprev){
				if(playing) {playing=0;timerGUI->DeleteAllRequests(this);}
				curframe--; if(curframe<0) curframe=numframes-1; UpdateCurFrame();
			}else
			if(bt==butnext){
				if(playing) {playing=0;timerGUI->DeleteAllRequests(this);}
				curframe++; if(curframe>=numframes) curframe=0; UpdateCurFrame();
			}else
			if(bt==butfirst){
				if(playing) {playing=0;timerGUI->DeleteAllRequests(this);}
				curframe=0; UpdateCurFrame();
			}else
			if(bt==butlast){
				if(playing) {playing=0;timerGUI->DeleteAllRequests(this);}
				curframe=numframes-1; UpdateCurFrame();
			}else
			if(bt==butplay&&!playing&&numframes){
				timerGUI->AddRequest(this,times[curframe]/1000.0f); 
				playing=1;	
				UpdateCurFrame();
			}else
			if(bt==butstop)
			{
				playing=0;timerGUI->DeleteAllRequests(this);
				UpdateCurFrame();
			}else
			if(bt==butaddfirst)
			{
				if(numframes<MAXFRAMES)
					ShowAddFrame(0,butaddfirst);
			}else
			{
				for(int i=0;i<numframes;i++)
				{
					if(bt==butdel[i])
					{
						CloseTexSelDlg();
						DeleteFrame(i);
						break;
					}else
					if(bt==butadd[i])
					{
						if(numframes<MAXFRAMES)
							ShowAddFrame(i+1,butadd[i]);
						break;
					}
				}
			}

		}
	}
	//------------------------------------------------------------------------------------
	if(sp && event->eventID == EMouseButton && event->pInt==0 && event->pID==1)
	{
		for(int i=0;i<numframes;i++)
		{
			if(sp==framepic[i])
			{
				ShowAddFrame(i,sp,0);
				break;
			}
		}
	}
	//------------------------------------------------------------------------------------
	if(event->eventID == ETree)
	{
		if(event->sender == texseldlgID && event->pInt2 == 0) //obsluha vyberu textury
		{
			CGUITree *tr = dynamic_cast<CGUITree*>(nameServer->ResolveID(texseldlgID));
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
			{
				int index = it->ut.pInt;

				SetFrame(index);
			}		
			if(tr)
				tr->CloseWindow();
		}
	}
	//------------------------------------------------------------------------------------

	CGUIDlgOkCancel::EventHandler(event); //zavolam eventhander predka
}


void CEDAnimEditor::DeleteFrame(int frame)
{
	if(frame<0||frame>=numframes) return;

	DeleteBackElem(stframetime[frame]);
	DeleteBackElem(butadd[frame]);
	DeleteBackElem(butdel[frame]);
	DeleteBackElem(edframename[frame]);
	DeleteBackElem(edframetime[frame]);
	DeleteBackElem(framepic[frame]);
	SAFE_RELEASE(frames[frame]);
	SAFE_DELETE_ARRAY(framenames[frame]);
	SAFE_RELEASE(frametex[frame]);

	DeleteBackElem(stframenum[numframes-1]);

	numframes--;
	if(curframe==numframes) curframe=numframes-1;

	for(int i=frame; i<numframes; i++)
	{
		stframetime[i]=stframetime[i+1];
		butadd[i]=butadd[i+1];
		butdel[i]=butdel[i+1];
		edframename[i]=edframename[i+1];
		edframetime[i]=edframetime[i+1];
		framepic[i]=framepic[i+1];
		frames[i]=frames[i+1];
		framenames[i]=framenames[i+1];
		frametex[i]=frametex[i+1];
		times[i]=times[i+1];
	}

	UpdateCurFrame();
	ArrangeDlg();
}


void CEDAnimEditor::DeleteAllFrames()
{
	if(numframes==0) return;

	for(int frame=0;frame<numframes;frame++)
	{
		DeleteBackElem(stframenum[frame]);
		DeleteBackElem(stframetime[frame]);
		DeleteBackElem(butadd[frame]);
		DeleteBackElem(butdel[frame]);
		DeleteBackElem(edframename[frame]);
		DeleteBackElem(edframetime[frame]);
		DeleteBackElem(framepic[frame]);
		SAFE_RELEASE(frames[frame]);
		SAFE_DELETE_ARRAY(framenames[frame]);
		SAFE_RELEASE(frametex[frame]);
	}

	numframes=0;
	curframe=0;
}


void CEDAnimEditor::ArrangeDlg()
{

	int i;
	maxsx=0;maxsy=0;
	UI sx,sy;
	if(numframes==0) {maxsx=54;maxsy=54;}
	for(i=0;i<numframes;i++)
	{
		frames[i]->GetSize(sx,sy);
		if(sx>maxsx) maxsx=(float)sx;
		if(sy>maxsy) maxsy=(float)sy;
	}

	float bxx,byy,x2;
	GetBackWindowSize(bxx,byy);
	x2=bxx/2;
	
	spanim->Move(x2-maxsx/2,10);
	butfirst->Move(x2-(10+25)*3+5,10+maxsy+10);
	butprev->Move(x2-(10+25)*2+5,10+maxsy+10);
	butstop->Move(x2-5-25,10+maxsy+10);
	butplay->Move(x2+5,10+maxsy+10);
	butnext->Move(x2+5+(25+10),10+maxsy+10);
	butlast->Move(x2+5+(25+10)*2,10+maxsy+10);
	float y1 = 10+maxsy+10+35;
	float y2=y1+10;
//---
	ln1->Move(10,y1);
//---
	cbrepeat->Move(50,y2);
	butaddfirst->Move(10,y2);

	float yy = 10+maxsy+10+35 + 10 + 17 + 5;

	for(i=0;i<numframes;i++){
		stframenum[i]->Move(10,yy);
		stframetime[i]->Move(60,yy+22);
		edframename[i]->Move(50,yy-2);
		edframetime[i]->Move(90,yy+20);
		butadd[i]->Move(10,yy+21);
		butdel[i]->Move(31,yy+21);
		framepic[i]->Move(150,yy);
		sy = frames[i]->GetSY();
		if(sy>37)
				yy+=sy+5;
			else
				yy+=42;	
	}

//---
	ln2->Move(10,yy);
//--

	ok->Move(x2-10-50,yy+10);
	cancel->Move(x2+10,yy+10);

	float dxx,dyy;
	dxx=150+maxsx;
	if(dxx<250)	dxx=250;
	dyy=yy+10+25+10;
		
	SetBackWindowNormalSize(dxx,dyy);
	SetBWLimitSizes(dxx,0,dxx,dyy,1,1);
	
	if(dyy>400) dyy=400;
	SetWindowSizeVP(dxx,dyy);

}

int CEDAnimEditor::Ok()
{
	int sv = SaveAnim();
	if(!sv) {
		GUIMessageBox("en{{Error!}}cs{{Chyba!}}","Chyba! Nezdaøilo se uložit animaci!",1,0xFFFF0000);
		return 0;
	}
	CloseWindow();
	return 1;
}

int CEDAnimEditor::SaveAnim()
{
	int ok;
	if(!resourcename ) return 0;

	SyncAll();

	if(newresource)
		anim = new CGEnAnim();
	
	if(!anim) return 0;
	
	anim->SetNumFrames(numframes);
	for(int i=0;i<numframes;i++)
	{
		anim->SetFrame(i,frames[i],times[i]);
	}
	anim->repeatflag=repeat;
	
	ok = GEnMain->GetPackageMgr()->AddResource(resourcename, anim, newresource);

	if(newresource)
		SAFE_RELEASE(anim);

	if(resourceEditor)
	{
		CPackageBrowser* pb = resourceEditor->GetPackageBrowser();
		if(pb) pb->FileSysChanged();
	}

	return ok;
}


void CEDAnimEditor::UpdateCurFrame()
{
	if(curframe>=numframes) curframe=numframes-1;
	if(curframe<0) curframe=0;

	UI sx=54,sy=54;
	if(numframes)
	{
		frames[curframe]->GetSize(sx,sy);
		spanim->RebuildStaticPicture(frametex[curframe],-1,-1,(float)sx,(float)sy);
		spanim->SetColor(0xFFFFFFFF);

		if(curframe==0)
			butfirst->Deactivate();
		else
			butfirst->Activate();

		if(curframe==numframes-1)
			butlast->Deactivate();
		else
			butlast->Activate();
		
		butprev->Activate();
		butnext->Activate();
		if(playing)
		{
			butplay->Deactivate();
			butstop->Activate();
		}
		else
		{
			butplay->Activate();
			butstop->Deactivate();
		}
	}else{
		spanim->RebuildStaticPicture(NULL,-1,-1,(float)sx,(float)sy);
		spanim->SetColor(COMPOFFCOL);
		butfirst->Deactivate();
		butprev->Deactivate();
		butnext->Deactivate();
		butlast->Deactivate();
		butplay->Deactivate();
		butstop->Deactivate();
	}



}

int CEDAnimEditor::TimerImpulse(typeID timerID, float time)
{	
	if(playing)
	{
		curframe++;
		if(curframe>=numframes)
		{
			curframe=0;
			if(!repeat) playing=0;
		}		
		UpdateCurFrame();
		if(playing)
			timerGUI->AddRequest(this,times[curframe]/1000.0f);
	}
	return 0;
}

void CEDAnimEditor::ShowAddFrame(int frame, CGUIElement *but, int newframe)
{

	float bx=0,by=0;
	CGUITree *oldtr = dynamic_cast<CGUITree*>(nameServer->ResolveID(texseldlgID));
	if(oldtr)
	{
		//uz existuje okno s lightexy
		desktop->GetDesktopPos(but,bx,by); //zjistim souradnice tlacitka "+" v desktopu
		oldtr->Move(bx,by); //pousunu ho
		oldtr->FocusMe(); //nafocusuju
	}else{
		desktop->GetDesktopPos(but,bx,by); //zjistim souradnice tlacitka "+" v desktopu
					
		CGUITree *tr = new CGUITree(bx,by,200,300,"en{{Choose LighTex:}}cs{{vyber LightTex:}}"); //vytvorim okno s tex. v akt. packagi
		tr->title->SetButtons(true,false,false);
		desktop->AddBackElem(tr);
		desktop->SetFocusEl(tr,0);

		int nr=pkg->GetNumResources();
		for(int i=0;i<nr;i++) //pridam vsechny lighttexy
		{
			if( pkg->GetResourceType(i) == resLightTex )
			{
				char *nm;
				nm = newstrdup(pkg->GetResourceName(i));
				CGUITreeItem *it = tr->AddTreeItem(0, nm);
				it->userTagType = eUTint;
				it->ut.pInt = i;
				delete[]nm;
			}
		}
		tr->UpdateTree();
		tr->Normalize();
		tr->AcceptEvent(GetID(),ETree);

		texseldlgID = tr->GetID();		
	}
	texselframe=frame; //nastavim cislo pridavanyho framu
	addframe=newframe;
}


int CEDAnimEditor::SetFrame(int lighttexindex)
{
	if(!pkg) return 0;

	if(pkg->GetResourceType(lighttexindex)!=resLightTex) return 0;
	CLightTex *lt = (CLightTex*) pkg->LoadResource(lighttexindex);
	if(!lt) return 0;

	const char *resnm = pkg->GetResourceName(lighttexindex);

	if(addframe)
	{
		int i;
		for(i=numframes;i>texselframe;i--)
		{
			stframetime[i]=stframetime[i-1];
			butadd[i]=butadd[i-1];
			butdel[i]=butdel[i-1];
			edframename[i]=edframename[i-1];
			edframetime[i]=edframetime[i-1];
			framepic[i]=framepic[i-1];

			frames[i]=frames[i-1];
			framenames[i]=framenames[i-1];
			frametex[i]=frametex[i-1];
			times[i]=times[i-1];
	
			int ln=strlen(framenames[i]);
			if(ln<FRAMENAME_LN) ln=FRAMENAME_LN;
			edframename[i]->BindAttribute(framenames[i],dtString,ln,0);
			edframetime[i]->BindAttribute(&times[i],dtInt,1,0);
	
			edframename[i]->SetTabOrder(10+i+0.1f);
			edframetime[i]->SetTabOrder(10+i+0.2f);
			butadd[i]->SetTabOrder(10 + i + 0.9f);
			butdel[i]->SetTabOrder(10 + i + 0.95f);
		}

		char frnum[10];
		int ff=numframes+1; float yy=10;
		sprintf(frnum,"%i:",ff);
		CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

		stframenum[numframes] = new CGUIStaticText(frnum,font,10,10,STD_DLG_TEXT_COLOR);
		AddBackElem(stframenum[numframes]);

		i=texselframe;

		times[i]=120;
		frames[i]=lt;
		frames[i]->AddRef();
		frametex[i]=NULL;

		stframetime[i] = new CGUIStaticText("en{{time:}}cs{{èas:}}",font,60,yy+22,STD_DLG_TEXT_COLOR);
		AddBackElem(stframetime[i]);

		edframename[i] = new CGUIEditWindow(50,yy-2,90);
		edframename[i]->SelectOnFocus();
		edframename[i]->SetMark(1);
		edframename[i]->SetTabOrder(10+i+0.1f);
		edframename[i]->AcceptEvent(GetID(),EOk);
		AddBackElem(edframename[i]);

		edframetime[i] = new CGUIEditWindow(90,yy+20,50);
		edframetime[i]->SelectOnFocus();
		edframetime[i]->SetMark(1);
		edframetime[i]->BindAttribute(&times[i],dtInt,1,0);
		edframetime[i]->SetTabOrder(10+i+0.2f);
		edframetime[i]->AcceptEvent(GetID(),EOk);
		AddBackElem(edframetime[i]);

		butadd[i] = new CGUIButton(10,yy+21,17,17,"GUI.But.Std","+",0,"en{{Adds frame}}cs{{Pøidá obrázek}}");
		butadd[i]->SetMark(1);
		butadd[i]->SetTabOrder(10 + i + 0.9f);
		butadd[i]->AcceptEvent(GetID(),EClicked);
		AddBackElem(butadd[i]);

		butdel[i] = new CGUIButton(31,yy+21,17,17,"GUI.But.Std","x",0,"en{{Deletes frame}}cs{{Smaže obrázek}}");
		butdel[i]->SetMark(1);
		butdel[i]->SetTabOrder(10 + i + 0.95f);
		butdel[i]->AcceptEvent(GetID(),EClicked);
		AddBackElem(butdel[i]);

		UI sxx,syy;
		frames[i]->CmpTex(&frametex[i]);
		frames[i]->GetSize(sxx,syy);
		framepic[i] = new CGUIStaticPicture(150,yy,frametex[i],(float)sxx,(float)syy);
		framepic[i]->throw_going=false;
		framepic[i]->AcceptEvent(GetID(),EMouseButton);
		AddBackElem(framepic[i]);

		edframetime[texselframe]->Sync(0);

		numframes++;
	}else{
		SAFE_RELEASE(frames[texselframe]);
		SAFE_RELEASE(frametex[texselframe]);

		frames[texselframe] = lt; lt->AddRef();
		lt->CmpTex(&frametex[texselframe]);
		framepic[texselframe]->RebuildStaticPicture(frametex[texselframe]);

		SAFE_DELETE_ARRAY(framenames[texselframe]);

	}

	int ln=strlen(resnm);
	if(ln<FRAMENAME_LN) ln=FRAMENAME_LN;
	framenames[texselframe] = new char[ln+5];
	strcpy(framenames[texselframe],resnm);
	edframename[texselframe]->BindAttribute(framenames[texselframe],dtString,ln,0);

	edframename[texselframe]->Sync(0);

	UpdateCurFrame();
	ArrangeDlg();

	return 1;
}

void CEDAnimEditor::CloseTexSelDlg()
{
	if(!texseldlgID) return;

	CGUITree *tr = dynamic_cast<CGUITree*>(nameServer->ResolveID(texseldlgID));
	if(tr){
		tr->CloseWindow();
	}

	texseldlgID=0;
}