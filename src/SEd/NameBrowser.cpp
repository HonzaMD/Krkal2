////////////////////////////////////////////////////////////////////////////////////
//
// NameBrowser.cpp
//
// prohlizec jmen
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <string.h>

#include "NameBrowser.h"

#include "SEdDesktop.h"

#include "kerservices.h"
#include "comcompiler.h"
#include "comobjectbrowser.h"

#include "textedit.h"

int NBtag=0;

struct CNBTreeData: public CGUITreeItemUserData{
	CNBTreeData(CKerName *n){kername=n;tag=NBtag;}
	void Update(CKerName *n){kername=n;tag=NBtag;}
	CKerName *kername;
	int tag;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NameBrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CNameBrowser::CNameBrowser(float _x, float _y, float _sx, float _sy)
: CGUIStdWindow(_x,_y,_sx,_sy,"NameBrowser",0,1,STD_DLG_BORDER_COLOR,1,STD_DLG_BGCOLOR_COLOR)
{

	eventhandler = new CNBEventHandler(this);

	float xx,yy,sny,sty;
	SetBackWindowNormalSize(10,10);

	AddWindowPanel(styleSet->Get("ObjPropertyPanel"));

	butFB = new CGUIButton(0,0,-1,-1,"GUI.But.FileBrowser","",0,"FileBrowser");
	butFB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butFB);
	butOB = new CGUIButton(0,0,-1,-1,"GUI.But.ObjectBrowser","",0,"ObjectBrowser");
	butOB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butOB);
	butNB = new CGUIButton(0,0,-1,-1,"GUI.But.NameBrowser","",0,"NameBrowser");
	butNB->Deactivate();
	butNB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butNB);


	GetVPSize(xx,yy);
	SetBackWindowNormalSize(xx,yy);

	butclearName2 = new CGUIButton(78,17,19,19,"GUI.But.Std","x",0);
	butclearName2->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddElem(butclearName2);

	butFiltr = new CGUIButton(100,17,50,19,"GUI.But.Std","Filtr...",0);
	butFiltr->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddElem(butFiltr);
	filtrWnd = 0;
	curFiltr = -1; //all

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");
	stFiltr = new CGUIStaticText("*",font,155,19,0xFF000000);
	panel->AddElem(stFiltr);

	onWindowPanel = false;

	CGUIRectHost *icon = new CGUIRectHost(0,0,styleSet->Get("BUT_NameBrowser_Up")->GetTexture(0));
	title->SetIcon(icon);

	treeDown=0;
	treeUp=0;

	sny=20;
	sty=floorf((yy-sny)/2);
	
	treeDown=new CGUITree(0,0,xx,sty);
	treeDown->AddScrollbarsSimple();
	treeDown->onWindowPanel=false;
	treeDown->rollOnClick=false;
	treeDown->AcceptEvent(eventhandler->GetID(),ETree);
	treeDown->AcceptEvent(eventhandler->GetID(),ETreeUpdate);
	AddBackElem(treeDown);
	
	treeUp=new CGUITree(0,sty+sny+1,xx,sty);
	treeUp->AddScrollbarsSimple();
	treeUp->onWindowPanel=false;
	treeUp->rollOnClick=false;
	treeUp->AcceptEvent(eventhandler->GetID(),ETree);
	treeUp->AcceptEvent(eventhandler->GetID(),ETreeUpdate);
	AddBackElem(treeUp);

	AddBackElem(new CGUILine(0,sty,xx,sty,STD_DLG_BORDER_COLOR));
	AddBackElem(new CGUILine(0,sty+sny,xx,sty+sny,STD_DLG_BORDER_COLOR));



	zobakL = styleSet->Get("NB_<<")->GetTexture(0); zobakL->AddRef();
	zobakR = styleSet->Get("NB_>>")->GetTexture(0); zobakR->AddRef();

	NBtag=0;
	startname=NULL;
	ewstartname = new CGUIEditWindow(0,sty+1,xx-20);
	ewstartname->SetReadOnly();
	AddBackElem(ewstartname);

	butclearName = new CGUIButton(xx-20+1,sty+1,19,19,"GUI.But.Std","x",0);
	butclearName->AcceptEvent(eventhandler->GetID(),EClicked);
	AddBackElem(butclearName);

	SetFocusEl(treeDown,0);

	KerNamesMain = compiler->objBrowser->names;

	CreateNBitems();


//	timerGUI->AddRequest(this,OB_REFRESH_TIME);

}

CNameBrowser::~CNameBrowser()
{
	CloseFiltrWnd();

	if(treeDown)
		DeleteBackElem(treeDown);
	if(treeUp)
		DeleteBackElem(treeUp);

	SAFE_DELETE(eventhandler);
	SAFE_DELETE_ARRAY(startname);

	SAFE_RELEASE(zobakL);
	SAFE_RELEASE(zobakR);
}

void CNameBrowser::CreateNBitems()
{
	if(compiler && compiler->objBrowser)
		KerNamesMain = compiler->objBrowser->names;
	else
		KerNamesMain = 0;

	if(KerNamesMain)
	{
		NBtag++;

		CKerName *startkername=GetStartKerName();

		AddNameLayer(startkername,1);
		AddNameLayer(startkername,0);

		DeleteOldItems(1,0);
		DeleteOldItems(0,0);

		treeDown->UpdateTree();
		treeUp->UpdateTree();

	}
}

void CNameBrowser::UpdateNB()
{
	CreateNBitems();
}


int g_curFiltr;

int FiltrFunc(CKerName *kername)
{

	if(g_curFiltr==-1) return 1; //all;
	
	if(g_curFiltr>=0) return kername->Type==g_curFiltr;

	if(g_curFiltr==-2){
		//void+object
		if(kername->Type==eKerNTobjectVoid || kername->Type==eKerNTobject) return 1;
	}else
	if(g_curFiltr==-3){
		//void+auto+shadow
		if(kername->Type==eKerNTautoVoid || kername->Type==eKerNTauto || kername->Type==eKerNTobjectShadow) return 1;
	}

	return 0;
}

int CNameBrowser::AddNameLayer(CKerName *from, int down, CGUITreeItem *parent_it)
{
	CKerNameList *knl,*kl;
	CGUITreeItem *it;
	
	g_curFiltr = curFiltr;
	knl = KerNamesMain->FindLayer(from,&FiltrFunc,-1,down);
	if(knl)
	{
		kl=knl;
		while(kl)
		{
			it = AddName(kl->name,down,parent_it);
			if(it) 
			{
				if(it->state==2)
				{
					AddNameLayer(kl->name,down,it);
					it->prepared = 1;
				}else{
					if( (down&&kl->name->HasChilds()) || (!down&&kl->name->HasParents()) )
						it->state=1;
					else
						it->state=0;
					
					it->ChangeRoller();
					it->prepared = 0;
				}
			}
			
			kl=kl->next;
		}
		knl->DeleteAll();
	}
	return 1;
}


CGUITreeItem *CNameBrowser::AddName(CKerName *n, int down, CGUITreeItem *parent_it)
{
	if(!n) return 0;

	CGUITree *tr=down?treeDown:treeUp;
	CTexture *tex=down?zobakL:zobakR;
	CGUITreeItem *it=0;

	typeID id=0;

	it = FindItem(n,parent_it,id, down);
	if(it)
	{
		((CNBTreeData*)it->ut.userDataClass)->Update(n);
	}else
	{
		if(parent_it)
			it = parent_it->AddTreeItem(id,n->GetShortNameString(),new CGUIRectHost(0,0,tex),n->GetNameString(),1,id?1:0);
		else
		{
			it = tr->AddTreeItem(id,n->GetShortNameString(),new CGUIRectHost(0,0,tex),n->GetNameString(),1,0,id?1:0);
		}

		it->SetRollerPicture(0);
		it->userTagType = eUTdataClass;
		it->ut.userDataClass = new CNBTreeData(n);
		
	}

	return it;
			
}


CGUITreeItem* CNameBrowser::FindItem(CKerName *name, CGUITreeItem *parent_it, typeID &id, int down)
{
	CGUIList<CGUITreeItem> *its = 0;

	int cmp;

	if(!parent_it)
	{
		its = down?treeDown->items:treeUp->items;
	}
	else
		its = parent_it->items;

	char *shn = name->GetShortNameString();
	char *n = name->GetNameString();

	CNBTreeData *data;
	id=0;

	if(its)
	{
		CGUITreeItem *it=NULL;
		while(it = its->GetNext(it))
		{
			data = (CNBTreeData*)it->ut.userDataClass;

			if(data->tag!=NBtag)
			{				
				cmp=strcmp(it->help,n);
				if(cmp==0){
					id = it->GetID();
					return it;
				}
			}

//			if(type==data->GetType())
			{
				cmp=_stricoll(it->label->GetText(),shn);
				if(cmp>0)
				{
					id = it->GetID();
					return 0;
				}
			}
		}
	}


	return 0;
}

int CNameBrowser::DeleteOldItems(int down, CGUITreeItem *parent_it)
{
	CGUIList<CGUITreeItem> *its = 0;
	CGUITree *tr=0;

	if(!parent_it)
	{
		tr=down?treeDown:treeUp;
		its = tr->items;
	}
	else
		its = parent_it->items;

	int tag;

	if(its)
	{
		CGUITreeItem *it=its->GetNext(0), *tm;
		while(it)
		{
			tag = ((CNBTreeData*)it->ut.userDataClass)->tag;

			tm=it;
			it=its->GetNext(it);

			if( tag != NBtag)
			{
				if(parent_it)
					parent_it -> DeleteTreeItem(tm);
				else
					tr->DeleteTreeItem(tm);
			}else{
				DeleteOldItems(down,tm);

				if(tm->state==0)
				{
					if(tm->userTagType==eUTdataClass)
					{
						CNBTreeData* data = (CNBTreeData*)tm->ut.userDataClass;
						if(data->kername)
						{
							if( (down&&data->kername->HasChilds()) || (!down&&data->kername->HasParents()) )
							{
								tm->state=1;
								tm->ChangeRoller();
							}
						}
					}
				}
			}
		}
	}

	return 1;
}

void CNameBrowser::SetStartName(CKerName *kername)
{
	char *name=0;
	SAFE_DELETE_ARRAY(startname);
	if(kername){
		name = kername->GetNameString();
	}
	if(name)
		startname = newstrdup(name);
	else
		startname = 0;

	if(kername)
	{
		ewstartname->SetParametr(kername->GetShortNameString(),dtString,strlen(kername->GetShortNameString()));
	}else{
		ewstartname->SetParametr("",dtString,0);
	}
	UpdateNB();
}

CKerName* CNameBrowser::GetStartKerName()
{
	CKerName *startkername=0;

	if(startname)
		startkername = KerNamesMain->GetNamePointer(startname);

	if(startkername)
	{
		ewstartname->SetParametr(startkername->GetShortNameString(),dtString,strlen(startkername->GetShortNameString()));
	}else{
		ewstartname->SetParametr("",dtString,0);
	}

	return startkername;
}

void CNameBrowser::CloseFiltrWnd()
{
	if(!filtrWnd) return;

	CGUITree *tr = dynamic_cast<CGUITree*>(nameServer->ResolveID(filtrWnd));
	if(tr) tr->CloseWindow();

	filtrWnd=0;
}

void CNameBrowser::ShowFiltrWnd()
{
	float bx,by;

	GetPos(bx,by);

	CGUITree *tr = new CGUITree(bx+70,by+19,130,200,"Filtr...");
	filtrWnd = tr->GetID();

	#define RH(t) (curFiltr==t)?new CGUIRectHost(0,0,zobakR):0

	tr->AddTreeItem(0,"*", RH(-1) ,"zobrazí všechna jména");
	tr->AddTreeItem(0,"void", RH(eKerNTvoid) ) ;
	tr->AddTreeItem(0,"object", RH(eKerNTobject) );
	tr->AddTreeItem(0,"method", RH(eKerNTmethod) );
	tr->AddTreeItem(0,"param", RH(eKerNTparam) );
	tr->AddTreeItem(0,"auto", RH(eKerNTauto) );
	tr->AddTreeItem(0,"objectShadow", RH(eKerNTobjectShadow) );
	tr->AddTreeItem(0,"ovoid+object", RH(-2));
	tr->AddTreeItem(0,"avoid+auto+shadow", RH(-3));
	tr->AcceptEvent(eventhandler->GetID(),ETree);
	tr->UpdateTree();
	tr->Normalize();
	desktop->AddBackElem(tr);
	tr->FocusMe();

	
}

void CNameBrowser::SetFiltr(int filtr)
{
	CloseFiltrWnd();

	FocusMe();

	filtr--;
	switch(filtr)
	{
	case -1:
		curFiltr = -1;
		stFiltr->ChangeText("*");
		break;
	case 0:
		curFiltr = eKerNTvoid;
		stFiltr->ChangeText("void");
		break;
	case 1:
		curFiltr = eKerNTobject;
		stFiltr->ChangeText("object");
		break;
	case 2:
		curFiltr = eKerNTmethod;
		stFiltr->ChangeText("method");
		break;
	case 3:
		curFiltr = eKerNTparam;
		stFiltr->ChangeText("param");
		break;
	case 4:
		curFiltr = eKerNTauto;
		stFiltr->ChangeText("auto");
		break;
	case 5:
		curFiltr = eKerNTobjectShadow;
		stFiltr->ChangeText("objectShadow");
		break;		
	case 6:
		curFiltr = -2;
		stFiltr->ChangeText("ovoid+object");
		break;
	case 7:
		curFiltr = -3;
		stFiltr->ChangeText("avoid+auto+shadow");
		break;
	}

	UpdateNB();

}

int CNameBrowser::FocusChanged(UINT focus, typeID dualID)
{
	if(focus) CloseFiltrWnd();
	return CGUIStdWindow::FocusChanged(focus,dualID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CNBEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CNBEventHandler::CNBEventHandler(CNameBrowser *nb)
{
	namebrow=nb;
}

CNBEventHandler::~CNBEventHandler()
{
}

void CNBEventHandler::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	if(event->eventID==EClicked)
	{
		if(event->sender==namebrow->butFB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowFileBrowser();
		}else
		if(event->sender==namebrow->butOB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowObjectBrowser();
		}else
		if(event->sender==namebrow->butclearName->GetID() || event->sender==namebrow->butclearName2->GetID())
		{
			namebrow->SetStartName(0);
		}else
		if(event->sender==namebrow->butFiltr->GetID())
		{
			namebrow->ShowFiltrWnd();
		}
	}else
	if(event->eventID==ETree)
	{
		if(event->sender==namebrow->treeDown->GetID() && event->pInt2==0)
		{	
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
				namebrow->SetStartName(((CNBTreeData*)it->ut.userDataClass)->kername);
		}else
		if(event->sender==namebrow->treeUp->GetID() && event->pInt2==0)
		{
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
				namebrow->SetStartName(((CNBTreeData*)it->ut.userDataClass)->kername);
		}
		if(event->sender==namebrow->filtrWnd && event->pInt2==1)
		{
			CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->pID));
			if(it)
				namebrow->SetFiltr(event->pInt);			
		}
	}else
	if(event->eventID==ETreeUpdate)
	{
		CGUITree *tr=0;
		int down=0;
		if(event->sender==namebrow->treeDown->GetID())
		{
			tr = namebrow->treeDown;
			down = 1;
		}else
		if(event->sender==namebrow->treeUp->GetID())
		{
			tr = namebrow->treeUp;
		}

		if(tr)
		{
			CGUITreeItem* ID(it,event->pID);
			if(it)
			{
				if(!it->prepared)
				{
					it->prepared=true;

					if(it->userTagType==eUTdataClass)
					{
						CNBTreeData* data = (CNBTreeData*)it->ut.userDataClass;
						if(data->kername)
						{
							namebrow->AddNameLayer(data->kername,down,it);
						}
					}
				}
				tr->UpdateTree();
			}
		}
	}
	delete event;
}