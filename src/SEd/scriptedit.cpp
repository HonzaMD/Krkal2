////////////////////////////////////////////////////////////////////////////////////
//
// scriptedit.cpp
//
// editor scriptu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "scriptedit.h"

#include "textedit.h"
#include "dxhighlight.h"

#include "kerservices.h"

#include "fs.h"
#include "SEdOpenedFiles.h"
#include "message.h"
#include "comcompiler.h"
#include "SEDObjectBrowser.h"
#include "namebrowser.h"
#include "SEdDesktop.h"
#include "SEdFileBrowser.h"

#define OB_COMP_INTERVAL 1.f

CSEdScriptEdit::CSEdScriptEdit()
:CGUIEditWindow(0,0,300,200,0,0,"[ScriptEdit]",0,STD_RESIZER_COLOR,0xFFFFFFFF,0xFF000050)
{

    CGUIRectHost *icon = new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0));
	title->SetIcon(icon);

	SetHighlight(1);

		
	CGUIWindowPanel* winpanel = new CGUIWindowPanel(2,STD_TITLE_HEIGHT+2-STD_PANEL_SHIFT,styleSet->Get("ScriptPanel"),this); 
	winpanel->SetAnchor(1,0,1,1);

	butsave = new CGUIButton(0,0,-1,-1,"GUI.But.Save","",0,"Uložit (Ctrl+S)");
	butsave->AcceptEvent(GetID(),EClicked);
	winpanel->AddIcon(butsave);

	butsaveall = new CGUIButton(0,0,-1,-1,"GUI.But.SaveAll","",0,"Uložit vše(Ctrl+Shift+S)");
	butsaveall->AcceptEvent(GetID(),EClicked);
	winpanel->AddIcon(butsaveall);

	winpanel->AddGap(20);

	butcut = new CGUIButton(0,0,-1,-1,"GUI.But.Cut","",0,"Cut");
	butcut->AcceptEvent(GetID(),EClicked);
	winpanel->AddIcon(butcut);
	butcopy = new CGUIButton(0,0,-1,-1,"GUI.But.Copy","",0,"Copy");
	butcopy->AcceptEvent(GetID(),EClicked);
	winpanel->AddIcon(butcopy);
	butpaste = new CGUIButton(0,0,-1,-1,"GUI.But.Paste","",0,"Paste");
	butpaste->AcceptEvent(GetID(),EClicked);
	butpaste->Deactivate();
	winpanel->AddIcon(butpaste);

	winpanel->AddGap(20);

	butundo = new CGUIButton(0,0,-1,-1,"GUI.But.Undo","",0,"Undo");
	butundo->AcceptEvent(GetID(),EClicked);
	butundo->Deactivate();
	winpanel->AddIcon(butundo);
	butredo = new CGUIButton(0,0,-1,-1,"GUI.But.Redo","",0,"Redo");
	butredo->AcceptEvent(GetID(),EClicked);
	butredo->Deactivate();
	winpanel->AddIcon(butredo);

	winpanel->AddGap(40);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");

	butcompile = new CGUIButton(0,0,60,16,"GUI.But.Std","Compile",0,"Zkompiluj to, ty Jirko!!!");
	butcompile->AcceptEvent(GetID(),EClicked);	
	winpanel->AddIcon(butcompile);

	butsetcomp = new CGUIButton(0,0,18,16,"GUI.But.Std","?",0,"Nastaví kompilovaný soubor");
	butsetcomp->AcceptEvent(GetID(),EClicked);	
	winpanel->AddIcon(butsetcomp);


	stcurcompfile = new CGUIStaticText("---",font,330,20,0xFFBB0000,100);
	winpanel->AddElem(stcurcompfile);

	winpanel->AddElem(new CGUIStaticText("Ln:",font,450,20,0xFF000000));
	stlinenum = new CGUIStaticText("1",font,470,20,0xFF000000);
	winpanel->AddElem(stlinenum);
	linenum=1;

	float xx,yy;
	desktop->GetBackWindowSize(xx,yy);
	Resize(xx-250,yy);

	fullpath=NULL;
	dir=0;
	file=0;

	titletext=NULL;
	filetitle=NULL;

	if(!titletext){
		titletext = new char[strlen("[ScriptEdit]")+5];
		strcpy(titletext,"[ScriptEdit]");
	}

	chng=0;

	comMode=0;
	comBlock=NULL;

	timerID=0;
	saveMsgID = 0;

	compfile = NULL;

	ShowCurCompFile();
}

CSEdScriptEdit::~CSEdScriptEdit()
{
	DeleteData();
}

void CSEdScriptEdit::DeleteData()
{
	SAFE_DELETE_ARRAY(dir);
	SAFE_DELETE_ARRAY(file);
	SAFE_DELETE_ARRAY(titletext);
	SAFE_DELETE_ARRAY(filetitle);
	if(fullpath && SEdOpenedFiles)
	{
		SEdOpenedFiles->Delete(fullpath);
	}
	SAFE_DELETE_ARRAY(fullpath);
}

int CSEdScriptEdit::CloseWindow()
{
	if(IsChanged())
	{
		char msg[200];
		_snprintf(msg,199,"Chcete uložit zmìny v %s?", GetFileTitle());
		CGUIMessageBoxYesNoCancel *mb = GUIMessageBoxYesNoCancel("Zavøít script...",msg,1);
		mb->AcceptEvent(GetID(),EClose);
		saveMsgID = mb->GetID();

		return 0;
	}
	else
		return CGUIEditWindow::CloseWindow();
}

int CSEdScriptEdit::TimerImpulse(typeID timerID, float time)
{
	if(timerID==this->timerID)
	{
		CompileForOB();
		this->timerID=0;
		return 0;
	}
	
	return CGUIEditWindow::TimerImpulse(timerID,time);
}


int CSEdScriptEdit::ChangedEvent()
{
	if(text){		
		if(text->CanUndo()) butundo->Activate(); else butundo->Deactivate();
		
		if(text->CanRedo()) butredo->Activate(); else butredo->Deactivate();

		if(timerID) timerGUI->DeleteRequest(timerID);
		timerID = timerGUI->AddRequest(this,OB_COMP_INTERVAL);

		SEdOpenedFiles->SetChange();
	}

	SetTitleChng(1);
	
	return 1;
}

void CSEdScriptEdit::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	if(event->eventID == EClicked)
	{
		if(event->sender == butundo->GetID())
			Undo();
		else
		if(event->sender == butredo->GetID())
			Redo();
		else
		if(event->sender == butsave->GetID())
			Save();
		else
		if(event->sender == butsaveall->GetID())
			SaveAll();
		else
		if(event->sender == butcut->GetID())
			Cut();
		else
		if(event->sender == butcopy->GetID())
			Copy();
		else
		if(event->sender == butpaste->GetID())
			Paste();
		else
		if(event->sender == butcompile->GetID())
			Compile();
		else
		if(event->sender == butsetcomp->GetID())
			SEdOpenedFiles->SetCompileFile(this);
		

	}else
	if(event->eventID == EClose)
	{
		if(event->sender == saveMsgID)
		{

			CGUIMessageBoxYesNoCancel *mb = dynamic_cast<CGUIMessageBoxYesNoCancel*>(nameServer->ResolveID(saveMsgID));
			if(mb){
				int ct = mb->GetCloseType();
				if(ct==1)//yes
				{
					if(Save())
						CloseWindow();
				}else
				if(ct==3) //no
				{
					chng = 0;
					CloseWindow();
				}
			}
		}
	}

	CGUIEditWindow::EventHandler(event);
}

int CSEdScriptEdit::CompileForOB()
{
	SEdOpenedFiles->CompileForOB();
/*	CSEdScriptEdit *csed = SEdOpenedFiles->GetCompileFile();

	if(compiler && csed && compiler->SECompile(csed,false,false) && SEdDesktop)
	{
		CSEdObjectBrowser* ob = SEdDesktop->GetObjectBrowser();
		if(ob) ob->UpdateOB();

		CNameBrowser* nb = SEdDesktop->GetNameBrowser();
		if(nb) nb->UpdateNB();

		return 1;
	}

	if(SEdDesktop)
	{
		CSEdObjectBrowser* ob = SEdDesktop->GetObjectBrowser();
		if(ob) ob->CompileError();

		CNameBrowser* nb = SEdDesktop->GetNameBrowser();
		if(nb) nb->UpdateNB();
	}
*/
	return 0;

}

int CSEdScriptEdit::Compile()
{
	int ok=0;
	SaveAll();

	CSEdScriptEdit *csed = SEdOpenedFiles->GetCompileFile();

	if(!csed) 
	{
		csed = this;
		
		SEdOpenedFiles->SetCompileFile(this);
	}

	if(compiler)
	{
		CComError error;
		if(compiler->SECompile(csed,true,true,&error) && SEdDesktop)
		{
			CSEdObjectBrowser* ob = SEdDesktop->GetObjectBrowser();
			if(ob) ob->UpdateOB();
			CNameBrowser* nb = SEdDesktop->GetNameBrowser();
			if(nb) nb->UpdateNB();

			ok=1;
		}else
		if(SEdDesktop){

			CSEdObjectBrowser* ob = SEdDesktop->GetObjectBrowser();
			if(ob) ob->CompileError();
			CNameBrowser* nb = SEdDesktop->GetNameBrowser();
			if(nb) nb->UpdateNB();


			char tmfnname[MAXOBFNNAMELEN];
			if(compiler->GetFileName(error.pos.fileID,tmfnname,MAXOBFNNAMELEN))
			{
				CSEdScriptEdit *sed;
				typeID id;

				CGUIElement *fel = desktop->GetFocusEl();

				id = SEdOpenedFiles->FocusWnd(tmfnname);
				if(!id)
				{
					sed = new CSEdScriptEdit();
					sed->Load(tmfnname);
					
					desktop->AddBackElem(sed);								
					sed->FocusMe();
				}else{
					sed = dynamic_cast<CSEdScriptEdit*>(nameServer->ResolveID(id));
				}

				if(fel)	desktop->SetFocusEl(fel,0);

				if(sed)
					sed->MoveCursorToLine(error.pos.line-1,error.pos.column-1);
			}
		}		
	}
	return ok;
}


//reakce na klavesnici
int CSEdScriptEdit::Keyboard(UINT ch, UINT state)
{

	int ff=1;

	switch(ch)
	{
	case 'S':
		if(mainGUI->key_ctrl) //ctrl+S - save
		{
			if(mainGUI->key_shift) //ctrl+shift+S - saveall
				SaveAll();
			else			
				Save();
			ff=0;
		}
		break;
	case VK_F7:
		Compile();
		ff=0;
		break;
	case VK_F9:
		CompileForOB();
		ff=0;
		break;
	}

	if(ff)
		return CGUIEditWindow::Keyboard(ch,state);			
	else
		return 0;
}

int CSEdScriptEdit::Load(const char *_dir, const char *_file)
{
	DeleteData();
	
	dir = newstrdup(_dir);
	file= newstrdup(_file);
	char *fp = new char[strlen(dir)+1+strlen(file)+1];
	sprintf(fp,"%s/%s",dir,file);

	SEdOpenedFiles->Add(fp,GetID());
	int sz = FS->GetFileSize(fp);
	char *buf = new char[sz+1];
	if(FS->ReadFile(fp,buf,sz))
	{
		int ext;
		char* shn = KerServices.ConvertFileNameToShort(file,ext,0,0);
		filetitle = newstrdup(shn);
		titletext = new char[strlen(shn)+5];
		strcpy(titletext,shn);
		title->SetText(titletext);
		SetParametr(buf,dtString,sz,0);
	}
	delete[]buf;			
	
	fullpath=fp;

	SetTitleChng(0);

	return 1;
}

int CSEdScriptEdit::Load(const char *_fullpath)
{
	DeleteData();

	int i,ln;
	ln=strlen(_fullpath);
	for(i=ln-1;i>=0;i--) if(_fullpath[i]=='/'||_fullpath[i]=='\\') break;
	if(i>=0)
	{
        file = newstrdup(_fullpath+i+1);
		dir = new char[i+1];
		memcpy(dir,_fullpath,i);
		dir[i]=0;
	}else
		return 0;

	fullpath = newstrdup(_fullpath);

	SEdOpenedFiles->Add(fullpath,GetID());
	int sz = FS->GetFileSize(fullpath);
	char *buf = new char[sz+1];
	if(FS->ReadFile(fullpath,buf,sz))
	{
		int ext;
		char* shn = KerServices.ConvertFileNameToShort(file,ext,0,0);
		filetitle = newstrdup(shn);
		titletext = new char[strlen(shn)+5];
		strcpy(titletext,shn);
		title->SetText(titletext);
		SetParametr(buf,dtString,sz,0);
	}
	delete[]buf;			

	SetTitleChng(0);
	
	return 1;
}

int CSEdScriptEdit::Save(int updfb)
{
	int ok=0;

	char *buf;
	int sz=text->GetText(0,0,1);
	
	buf=new char[sz];

	if(text->GetText(buf,sz,1)==0)
	{
		ok = FS->WriteFile(fullpath,buf,sz-1,0);
	}

	delete[] buf;	

	if(updfb && SEdDesktop)
	{
		CSEdFileBrowser *fb = SEdDesktop->GetFileBrowser();
		if(fb)
			fb->FileSysChanged();
	}

	if(ok) SetTitleChng(0);

	return ok;
}

int CSEdScriptEdit::SaveAll()
{
	CSEdOpenFilesList *ol = SEdOpenedFiles->GetOpenedFiles();
	if(!ol) return 0;
	CSEdOpenFilesListIt it(ol);

	CSEdOFElem *ofel;
	CSEdScriptEdit *se;

	int ch=0;

	while(!it.End())
	{
		ofel = it;

		se = ofel->GetScriptEdit();
		if(se)
		{
			if(se->IsChanged())
			{
				se->Save(0);
				ch=1;
			}
		}

		it++;
	}

	if(ch && SEdDesktop)
	{
		CSEdFileBrowser *fb = SEdDesktop->GetFileBrowser();
		if(fb)
			fb->FileSysChanged();
	}

	return 1;
}

void CSEdScriptEdit::SetTitleChng(int _chng)
{
	if(chng==_chng) return;

	if(chng){
		int ln=strlen(titletext);
		if(titletext[ln-1]=='*')
			titletext[ln-1]=0;
	}else{
		strcat(titletext,"*");
	}
	
	title->SetText(titletext);

	chng=_chng;
}


void CSEdScriptEdit::ClipboardEvent()
{
	butpaste->Activate();
}

void CSEdScriptEdit::ShowCurCompFile()
{
	if(SEdOpenedFiles->GetCompileFile() != compfile )
	{
		compfile = SEdOpenedFiles->GetCompileFile();

		if(compfile)
		{
			if(compfile && compfile!=this)
			{
				char *tt = compfile->GetFileTitle();
				stcurcompfile->ChangeText(tt);
			}else
			{
				char *tt = GetFileTitle();
				stcurcompfile->ChangeText(tt);
			}
		}else
		{
			stcurcompfile->ChangeText("---");
		}
	}
}

int CSEdScriptEdit::FocusChanged(UINT focus, typeID dualID) //zmenil se focus
{	
	if(focus)
	{
		if (!IsClipboardFormatAvailable(CF_TEXT)) 
			butpaste->Deactivate();
		else
			butpaste->Activate();		
	}
	else{
		/*if(timerID)
		{
			timerGUI->DeleteRequest(timerID);
			CompileForOB();
		}*/
	}
	return CGUIEditWindow::FocusChanged(focus,dualID);
}

int CSEdScriptEdit::SeekToStart()
{

	comMode=2;
	comBlock = text->GetFirstBlock();
	comChar = comBlock->GetText();
	
	if(!comBlock) comMode=3;
	return 1;
}

int CSEdScriptEdit::GetNextChar()
{
	int ch=-2; 
	if(!comMode) return -2;

	switch(comMode)
	{
	case 1:
		break;
	case 2:
		if(!comBlock||!comChar) return -2;
		ch = *(UC*)comChar++;
		if(ch==0){
			ch=10;
			comBlock=comBlock->GenNextBlock();
			if(comBlock)
				comChar=comBlock->GetText();
			else
			{
				comChar=0;
				comMode=3;
			}
		}
		break;
	case 3:
		ch = EOF;
		break;
	}

	return ch;
}


void CSEdScriptEdit::ShowLinenum()
{
	char ln[10];

	int l = text->GetCursor()->GetBlock()->GetBlockNum()+1;
	if(l!=linenum)
	{
		linenum=l;
		_snprintf(ln,9,"%i",linenum);
		stlinenum->ChangeText(ln);
	}
}

void CSEdScriptEdit::CursorMoved()
{
	ShowLinenum();
	if(timerID) {
		if(timerGUI->DeleteRequest(timerID))
			timerID = timerGUI->AddRequest(this,OB_COMP_INTERVAL);
	}
}