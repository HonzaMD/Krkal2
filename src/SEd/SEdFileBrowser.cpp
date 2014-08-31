////////////////////////////////////////////////////////////////////////////////////
//
// SEdFileBrowser.cpp
//
// SEd FileBrowser
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#include "SEdFileBrowser.h"
#include "SEDOpenedFiles.h"
#include "scriptedit.h"
#include "SEdDesktop.h"

#include "dialogs.h"
#include "menu.h"
#include "fs.h"
#include "kerservices.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SEdFileBrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CSEdFileBrowser::CSEdFileBrowser(float _x, float _y, float _sx, float _sy, char* path, char* label)
: CGUIFileBrowser(_x,_y,_sx,_sy)//,path,label,1)
{
	eventhandler = new CSEdFBEventHandler(this);

	compareFunction=&Compare;
	SetCompareFunctionSubTree(&Compare);

	CGUITreeItem* ti = AddRootItem(path,label);
	ti->SetRollState(1);


	AddWindowPanel(styleSet->Get("ObjPropertyPanel"));

	butFB = new CGUIButton(0,0,-1,-1,"GUI.But.FileBrowser","",0,"FileBrowser");
	butFB->Deactivate();
	butFB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butFB);
	butOB = new CGUIButton(0,0,-1,-1,"GUI.But.ObjectBrowser","",0,"ObjectBrowser");
	butOB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butOB);
	butNB = new CGUIButton(0,0,-1,-1,"GUI.But.NameBrowser","",0,"NameBrowser");
	butNB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butNB);


	onWindowPanel = false;

	title->SetText("File Browser");
	CGUIRectHost *icon = new CGUIRectHost(0,0,styleSet->Get("FileBrowser")->GetTexture(0));
	title->SetIcon(icon);

	
	dirmenu = new CGUIMenu();
	dirmenu->AddMenuItem(0,"New script...");
	dirmenu->AddMenuItem(0,"New directory...");
	dirmenu->AddGap(0);
	dirmenu->AddMenuItem(0,"Delete directory...");
	dirmenu->AcceptEvent(eventhandler->GetID(),EMenu);

	scriptmenu = new CGUIMenu();
	scriptmenu->AddMenuItem(0,"Edit script");
	scriptmenu->AddGap(0);
	scriptmenu->AddMenuItem(0,"Delete script...");
	scriptmenu->AcceptEvent(eventhandler->GetID(),EMenu);

}

CSEdFileBrowser::~CSEdFileBrowser()
{
	SAFE_DELETE(dirmenu);
	SAFE_DELETE(scriptmenu);

	SAFE_DELETE(eventhandler);
}

int CSEdFileBrowser::Filter(CGUITreeItem* parent, const char *name, int dir, int first)
{
	//if(!dir) return 0;
	return 1;
}

CGUIFileBrowserItem* CSEdFileBrowser::AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir)
{
	CGUIFileBrowserItem* ni=NULL;

	char *ff=NULL;
	char *shn;
	int ext;
	
	ff = newstrdup(name);

	shn = KerServices.ConvertFileNameToShort(name,ext,dir,0);
	
	if(dir)
	{
		if(ext!=eEXTpackage)
		{
			if(fbi)
				ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);
			else
				ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),ff,true);
			ni->state=1;
			ni->ChangeRoller();
		}
	}else{

		if(ext==eEXTscript)
		{
			if(fbi)
				ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),ff,true);
			else
				ni = (CGUIFileBrowserItem*)AddTreeItem(0,shn,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),ff,true);
		}
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

int CSEdFileBrowser::TreeHandler(typeID treeItem, int index, int state)
{

	if(state==1) return 1; //levy mose but.

	char *dir=0,*file=0;
	char *res=0;

	int gsf = GetSelectedFile(treeItem, &dir, &file);
	if(gsf==-1) return 1; 

	CGUIFileBrowserItem *it = dynamic_cast<CGUIFileBrowserItem*>(nameServer->ResolveID(treeItem));

	if(it){

	if(gsf==0) //soubor
	{
		if(state==0)
		{
			switch(it->ut.pInt)
			{
			case eEXTscript:
				EditScript(dir,file);
				break;
			}
		}else
		if(state==2) //right mouse but
		{
			scriptmenu->On(treeItem);
		}
	}else
	{
		//adresar
		if(state==2) //right mouse but
		{
			dirmenu->On(treeItem);
		}
	}

	}

	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);

	return 1;
}

void CSEdFileBrowser::EditScript(char *dir, char *file)
{
	char *fp = new char[strlen(dir)+1+strlen(file)+1];
	sprintf(fp,"%s/%s",dir,file);

	if(!SEdOpenedFiles->FocusWnd(fp))
	{
		CSEdScriptEdit *sed = new CSEdScriptEdit();
		sed->Load(dir,file);
		desktop->AddBackElem(sed);
		sed->FocusMe();

		if(!SEdOpenedFiles->GetCompileFile())
		{
			SEdOpenedFiles->SetCompileFile(sed);
		}
	}
	delete[] fp;
}

void CSEdFileBrowser::EditScript(char *fp)
{
	if(!SEdOpenedFiles->FocusWnd(fp))
	{
		CSEdScriptEdit *sed = new CSEdScriptEdit();
		sed->Load(fp);
		desktop->AddBackElem(sed);
		sed->FocusMe();

		if(!SEdOpenedFiles->GetCompileFile())
		{
			SEdOpenedFiles->SetCompileFile(sed);
		}
	}
}

int CSEdFileBrowser::Compare(CGUITreeItem *aa, CGUITreeItem *bb)
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
//
// CSEdFBEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_NEWNAME_LEN 100

CSEdFBEventHandler::CSEdFBEventHandler(CSEdFileBrowser *fb)
{
	filebrow=fb;
	dlgnscrID = 0;
	dlgdelID = 0;
	newname = 0;
	path = 0;
}

CSEdFBEventHandler::~CSEdFBEventHandler()
{
	SAFE_DELETE_ARRAY(newname);
	SAFE_DELETE_ARRAY(path);
}

void CSEdFBEventHandler::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	if(event->eventID==EClicked)
	{
		if(event->sender==filebrow->butOB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowObjectBrowser();
		}else
		if(event->sender==filebrow->butNB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowNameBrowser();
		}
	}else
	if(event->eventID==EMenu )
	{
		typeID activator = event->pID2;
		int menuitem = event->pInt;
		char *dir,*file;

		filebrow->GetSelectedFile(activator, &dir, &file);

		if(event->sender == filebrow->scriptmenu->GetID())
		{			
			switch(menuitem)
			{
			case 0:
				//edit script
				filebrow->EditScript(dir,file);
				break;
			case 1:
				//delete script
				{
					#define MSGDSC "Do you realy want to delete the selected file %s?"
					char *msg = new char [strlen(MSGDSC)+strlen(file)+1];
                    
					sprintf(msg,MSGDSC,file);

					CGUIMessageBox *dlgd =  GUIMessageBoxOkCancel("Delete file",msg,1);
					dlgdelID = dlgd->GetID();
					delete[] msg;

					dlgd->AcceptEvent(GetID(),EOk);

					SAFE_DELETE_ARRAY(path);

					path=new char[strlen(dir)+1+strlen(file)+1];
                    sprintf(path,"%s/%s",dir,file);
				}
				break;
			}
		}else
		if(event->sender == filebrow->dirmenu->GetID())
		{			
			switch(menuitem)
			{
			case 0:
				//novy script
				{

					SAFE_DELETE(newname);
					newname = new char[MAX_NEWNAME_LEN+5];

					CGUIDlgEditbox *dlgn = new CGUIDlgEditbox(100,100,300,"New script","Script name:");
					dlgnscrID = dlgn->GetID();
					desktop->AddBackElem(dlgn);
					dlgn->AcceptEvent(GetID(),EOk);
					dlgn->BindEditBox(newname,dtString,MAX_NEWNAME_LEN);
					dlgn->SetModal(1);
					dlgn->FocusMe();

					SAFE_DELETE_ARRAY(path);
					path = newstrdup(dir);
					newmode = 0;
				}
				break;
			case 1:
				//novy adresar
				{

					SAFE_DELETE(newname);
					newname = new char[MAX_NEWNAME_LEN+5];

					CGUIDlgEditbox *dlgn = new CGUIDlgEditbox(100,100,300,"New directory","Directory name:");
					dlgnscrID = dlgn->GetID();
					desktop->AddBackElem(dlgn);
					dlgn->AcceptEvent(GetID(),EOk);
					dlgn->BindEditBox(newname,dtString,MAX_NEWNAME_LEN);
					dlgn->SetModal(1);
					dlgn->FocusMe();

					SAFE_DELETE_ARRAY(path);
					path = newstrdup(dir);
					newmode = 1;
				}
				break;
			case 2:
				//smazat adresar
				{
					#define MSG "Do you realy want to delete the selected directory %s?"
					char *msg = new char [strlen(MSG)+strlen(file)+1];
                    
					sprintf(msg,MSG,file);

					CGUIMessageBox *dlgd =  GUIMessageBoxOkCancel("Delete directory",msg,1);
					dlgdelID = dlgd->GetID();
					delete[] msg;

					dlgd->AcceptEvent(GetID(),EOk);

					SAFE_DELETE_ARRAY(path);
					path = newstrdup(dir);
				}

				break;
			}
		}

		delete[] dir; delete[] file;
	}else
	if(event->eventID==EOk)
	{
		if(event->sender == dlgdelID)
		{
			//smazat adresar/soubor

			FS->Delete(path);
			filebrow->FileSysChanged();


		}else
		if(event->sender == dlgnscrID)
		{
			//novy script/adresar
			CGUIDlgEditbox *dlgn = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(dlgnscrID));
			if(dlgn)
			{
				int err=0;
		
				if(dlgn->SyncEditBox()!=0) err=1;

				if(!err)
				{
					if(newmode==0)
					{
						//novy script
						int ln=strlen(newname);
						if(ln<=3||strcmp(newname+ln-3,".kc")!=0) { strcat(newname,".kc"); ln = strlen(newname); }
						
						if(!FS->IsValidFilename(newname)) err=1;
						else{
							char *name = new char[strlen(path)+1+ln+25+1];
							char version[25];
							newname[ln-3]=0;
							KerServices.GenerateVersionNumber(version);
							sprintf(name,"%s/%s_%s.kc",path,newname,version);
							newname[ln-3]='.';

							#define	HEAD_STR "#head\ngame\t?\nauthor\t?\nversion\t%s\n#endhead\n\n"

							char *nf = new char[sizeof(HEAD_STR)+25];
							
							sprintf(nf,HEAD_STR,version);

							if(!FS->WriteFile(name,nf,strlen(nf),0))
								err = 1;
							else
							{
								filebrow->FileSysChanged();
								filebrow->EditScript(name);
							}

							delete[] nf;
							delete[] name;	
						}
					}else{
						//novy adresar
						if(!FS->IsValidFilename(newname)) err=1;
						else{
							char *dot = strrchr(newname,'.');
							char *name = new char[strlen(path)+1+strlen(newname)+25+1];

							char version[25];
							KerServices.GenerateVersionNumber(version);

							if(dot)
							{
								char *ext = dot+1;
								*dot = 0;
								sprintf(name,"%s/%s_%s.%s",path,newname,version,ext);
								*dot = '.';
							}else{
								sprintf(name,"%s/%s_%s",path,newname,version);
							}

							if(!FS->CreateDir(name)) err=1;
							else
								filebrow->FileSysChanged();

							delete[]name;
						}
					}
				}

				if(!err)
				{
					dlgn->CloseWindow();
				}else{
					if(newmode==0)
						dlgn->ShowErrorMsg("Error! Invalid filename!");
					else
						dlgn->ShowErrorMsg("Error! Invalid directory name!");
				}

				
			}
		}
	}


	delete event;
}