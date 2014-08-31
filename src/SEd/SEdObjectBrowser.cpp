////////////////////////////////////////////////////////////////////////////////////
//
// SEdObjectBrowser.cpp
//
// SEd ObjectBrowser
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <string.h>

#include "SEdObjectBrowser.h"
#include "SEDOpenedFiles.h"
#include "scriptedit.h"
#include "SEdDesktop.h"

#include "menu.h"
#include "fs.h"
#include "kerservices.h"

#include "comcompiler.h"
#include "comobjectbrowser.h"

#include "tree.h"

#define OB_REFRESH_TIME 1

int OBtag=0;

class CSEdOBTreeData: public CGUITreeItemUserData{
public:
	CSEdOBTreeData(CComOBBase *comOB, int _type){ com=comOB; tag = OBtag; type=_type; kername=0; }
	void Update(CComOBBase *comOB){ com=comOB; tag = OBtag; }

	CComOBBase *GetCom(){return com;}
	int GetTag(){return tag;}
	int GetType(){return type;}

	CKerName *kername;

protected:
	CComOBBase *com;
	int type;
	int tag;
};

#define OB_O 1
#define OB_M 2
#define OB_A 3
#define OB_G 4

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SEdFileBrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CSEdObjectBrowser::CSEdObjectBrowser(float _x, float _y, float _sx, float _sy)
: CGUITree(_x,_y,_sx,_sy,"ObjectBrowser")
{

	compileerr=0;

	eventhandler = new CSEdOBEventHandler(this);

	AddWindowPanel(styleSet->Get("ObjPropertyPanel"));

	butFB = new CGUIButton(0,0,-1,-1,"GUI.But.FileBrowser","",0,"FileBrowser");
	butFB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butFB);
	butOB = new CGUIButton(0,0,-1,-1,"GUI.But.ObjectBrowser","",0,"ObjectBrowser");
	butOB->AcceptEvent(eventhandler->GetID(),EClicked);
	butOB->Deactivate();
	panel->AddIcon(butOB);
	butNB = new CGUIButton(0,0,-1,-1,"GUI.But.NameBrowser","",0,"NameBrowser");
	butNB->AcceptEvent(eventhandler->GetID(),EClicked);
	panel->AddIcon(butNB);

	onWindowPanel = false;

	CGUIRectHost *icon = new CGUIRectHost(0,0,styleSet->Get("BUT_ObjectBrowser_Up")->GetTexture(0));
	title->SetIcon(icon);

	rollOnClick = 0;

	tex_obj = styleSet->Get("OB_Object")->GetTexture(0); tex_obj->AddRef();
	tex_met = styleSet->Get("OB_Method")->GetTexture(0); tex_met->AddRef();
	tex_atr = styleSet->Get("OB_Attribut")->GetTexture(0); tex_atr->AddRef();

	CompileError();

	//SetRollersTextures(0,0,0,false,false,true);

	UpdateOB();
}

CSEdObjectBrowser::~CSEdObjectBrowser()
{
	SAFE_RELEASE(tex_obj);
	SAFE_RELEASE(tex_met);
	SAFE_RELEASE(tex_atr);

	SAFE_DELETE(eventhandler);
}

void CSEdObjectBrowser::CompileError()
{
	if(!compileerr) background->SetCol(0xFF888888);
    compileerr=1;	
}

int CSEdObjectBrowser::TreeHandler(typeID treeItem, int index, int state)
{
	if(compileerr) return 0;

	CGUITreeItem *it = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));

	if(it){
		if(state==0){

			if(it->userTagType == eUTdataClass)
			{
				CComOBBase *ob = ((CSEdOBTreeData*)it->ut.userDataClass)->GetCom();
				if(!ob) return 1;
				if(compiler->GetFileName(ob->posDefined.fileID,tmfnname,MAXOBFNNAMELEN))
				{
					CSEdScriptEdit *sed;
					typeID id;
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
					if(sed)
						sed->MoveCursorToLine(ob->posDefined.line-1,ob->posDefined.column-1);
						sed->SelectWordRight();
				}
			}
		}

	}
	return 1;
}


void CSEdObjectBrowser::CreateOBitems()
{
	if(compiler && compiler->objBrowser)
	{
		if(compileerr) background->SetCol(STD_TREE_BGCOLOR_COLOR);
		compileerr=0;
		OBtag++;

		CComObjectBrowser *ob = compiler->objBrowser;

		int count;
		CComOBGlobal *g;

		AddObjectLayer(0);

		count=ob->gTab->GetCount();
		for(int j=0;j<count;j++)
		{
			g=ob->GetGlobal(j);
			AddGlobal(g);
		}

		DeleteOldItems();

		UpdateTree();

	}
}

void CSEdObjectBrowser::TreeUpdateHandler(typeID treeItem, int index)
{
	CGUITreeItem* ID(it,treeItem);
	
	if(!it) return;
    
	if(!it->prepared && !compileerr )
	{
		it->prepared=true;

		if(it->userTagType==eUTdataClass)
		{
			CSEdOBTreeData* data = (CSEdOBTreeData*)it->ut.userDataClass;
			if(data->GetType()==OB_O && data->kername)
			{
				AddObjectMethodsAndAttributes((CComOBObject*)data->GetCom(),it);
				AddObjectLayer(data->kername,it);
			}
		}
	}

	UpdateTree();
}


int CSEdObjectBrowser::AddObjectLayer(CKerName *from, CGUITreeItem *parent_it)
{
	CComOBObject *o;
	CKerNameList *knl,*kl;
	CGUITreeItem *it;
	CComObjectBrowser *ob = compiler->objBrowser;
	knl = ob->names->FindLayer(from,0,eKerNTobject);
	if(knl)
	{
		kl=knl;
		while(kl)
		{
			o = kl->name->objBrowserLink;
			if(o)
			{
				it = AddObject(o,parent_it);
				if(it) {
					((CSEdOBTreeData*)it->ut.userDataClass)->kername = kl->name;
					if(it->state==2)
					{
						AddObjectMethodsAndAttributes(o,it);
						AddObjectLayer(kl->name,it);
						it->prepared = 1;
					}else{
						if(kl->name->HasChilds()||o->mTab->GetCount()||o->aTab->GetCount())
							it->state=1;
						else
							it->state=0;

						it->ChangeRoller();						
						it->prepared = 0;
					}
				}
			}
			kl=kl->next;
		}
		knl->DeleteAll();
	}
	return 1;
}

CGUITreeItem *CSEdObjectBrowser::AddObject(CComOBObject *o, CGUITreeItem *parent_it)
{
	if(!o) return 0;

	CGUITreeItem *it=0;

	CSEdOBTreeData *data;

	typeID id=0;

	char *nm;

	assert(o->kerName);
	nm=KerServices.ConverNameToShort(o->kerName);

	it = FindItem(nm,parent_it,id, OB_O);
	if(it)
	{
		((CSEdOBTreeData*)it->ut.userDataClass)->Update(o);
	}else
	{
		if(parent_it)
			it = parent_it->AddTreeItem(id,nm,new CGUIRectHost(0,0,tex_obj),o->kerName,1,id?1:0);
		else
			it = AddTreeItem(id,nm,new CGUIRectHost(0,0,tex_obj),o->kerName,1,0,id?1:0);

		//it->SetRollersTextures(0,0,0,false,false,true);
		data = new CSEdOBTreeData(o,OB_O);
		it->userTagType = eUTdataClass;
		it->ut.userDataClass = data;
	}

    return it;
			
}

int CSEdObjectBrowser::AddObjectMethodsAndAttributes(CComOBObject *o, CGUITreeItem *it)
{
	if(!o||!it) return 0;

	CComOBMethod *m;
	CComOBAttribute *a;

	int cnt;
	cnt = o->mTab->GetCount();
	for(int j=0;j<cnt;j++)
	{
		m=o->GetMethod(j);
		AddMethod(m,it,o->kerName);
	}

	cnt = o->aTab->GetCount();
	for(int j=0;j<cnt;j++)
	{
		a=o->GetAttribute(j);
		AddAttribute(a,it,o->kerName);
	}

	return 1;
}

CGUITreeItem *CSEdObjectBrowser::AddMethod(CComOBMethod *m, CGUITreeItem *parent_it, char *objKerName)
{
	if(!m||!parent_it) return 0;

	CGUITreeItem *itm;

	CSEdOBTreeData *data;
	CComOBArgument *arg;
	char *nm;
	typeID id=0;

	assert(m->kerName);
	nm = KerServices.ConverNameToShort(m->kerName,0,objKerName); 

	int pos;

	sprintf(tmfnname,"%s( ",nm);

	pos = strlen(tmfnname);

	int k;

	for(k=0;k<m->aTab->GetCount();k++)
	{
		if(k!=0) {
			strcat(tmfnname,", ");
			pos+=2;
		}
		arg = m->GetArgument(k);

		assert(arg->kerName);
		nm = KerServices.ConverNameToShort(arg->kerName,0,m->kerName); 

		pos+=strlen(nm);
		if(pos<MAXOBFNNAMELEN-10)
			strcat(tmfnname,nm);
		else
		{
			strcat(tmfnname,"...");
			break;
		}
	}
	if(k>0)
		strcat(tmfnname," )");
	else
		strcat(tmfnname,")");

	itm = FindItem(tmfnname,parent_it,id, OB_M);
	if(itm)
	{
		((CSEdOBTreeData*)itm->ut.userDataClass)->Update(m);
	}else
	{
		itm = parent_it->AddTreeItem(id,tmfnname,new CGUIRectHost(0,0,tex_met),m->kerName,0,id?1:0);
		//itm->SetRollersTextures(0,0,0,false,false,true);
		data = new CSEdOBTreeData(m,OB_M);
		itm->ut.userDataClass = data;
		itm->userTagType = eUTdataClass;
	}

	return itm;
			
}


CGUITreeItem *CSEdObjectBrowser::AddAttribute(CComOBAttribute *a, CGUITreeItem *parent_it, char *objKerName)
{
	if(!a||!parent_it) return 0;

	CGUITreeItem *itm;
	CSEdOBTreeData *data;
	char *nm;
	typeID id=0;

	assert(a->kerName);
	nm = KerServices.ConverNameToShort(a->kerName,0,objKerName); 

	itm = FindItem(nm,parent_it,id, OB_A);
	if(itm)
	{
		((CSEdOBTreeData*)itm->ut.userDataClass)->Update(a);
	}else
	{
		itm = parent_it->AddTreeItem(id,nm,new CGUIRectHost(0,0,tex_atr),a->kerName,0,id?1:0);
		//itm->SetRollersTextures(0,0,0,false,false,true);
		data = new CSEdOBTreeData(a,OB_A);
		itm->ut.userDataClass = data;
		itm->userTagType = eUTdataClass;
	}

	return itm;
}

CGUITreeItem *CSEdObjectBrowser::AddGlobal(CComOBGlobal *g, CGUITreeItem *parent_it)
{
	if(!g) return 0;
	
	char *nm;
	CGUITreeItem *it;
	CSEdOBTreeData *data;
	typeID id=0;

	assert(g->kerName);
	nm = KerServices.ConverNameToShort(g->kerName); 

	it = FindItem(nm,parent_it,id,OB_G);
	if(it)
	{
		((CSEdOBTreeData*)it->ut.userDataClass)->Update(g);
	}else
	{
		if(!parent_it)
			it = AddTreeItem(id,nm,new CGUIRectHost(0,0,tex_atr),g->kerName,0,0,id?1:0);
		else
			it = parent_it->AddTreeItem(id,nm,new CGUIRectHost(0,0,tex_atr),g->kerName,0,id?1:0);

		//it->SetRollersTextures(0,0,0,false,false,true);
		data = new CSEdOBTreeData(g,OB_G);
		it->ut.userDataClass = data;
		it->userTagType = eUTdataClass;	
	}
	return it;
}

int CSEdObjectBrowser::DeleteOldItems(CGUITreeItem *parent_it)
{
	CGUIList<CGUITreeItem> *its = 0;
	if(!parent_it)
		its = items;
	else
		its = parent_it->items;

	int tag;

	if(its)
	{
		CGUITreeItem *it=its->GetNext(0), *tm;
		while(it)
		{
			tag = ((CSEdOBTreeData*)it->ut.userDataClass)->GetTag();

			tm=it;
			it=its->GetNext(it);

			if( tag != OBtag)
			{
				if(parent_it)
					parent_it -> DeleteTreeItem(tm);
				else
					DeleteTreeItem(tm);
			}else{
				DeleteOldItems(tm);
			
				if(tm->state==0)
				{
					if(tm->userTagType==eUTdataClass)
					{
						CSEdOBTreeData* data = (CSEdOBTreeData*)tm->ut.userDataClass;
						if(data->GetType()==OB_O && data->kername)
						{
							CComOBObject *o = (CComOBObject*) data->GetCom();
							if(data->kername->HasChilds() || o->mTab->GetCount() || o->aTab->GetCount() )
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

CGUITreeItem* CSEdObjectBrowser::FindItem(char *name, CGUITreeItem *parent_it, typeID &id, int type)
{
	CGUIList<CGUITreeItem> *its = 0;

	int cmp;

	if(!parent_it)
		its = items;
	else
		its = parent_it->items;

	CSEdOBTreeData *data;
	id=0;

	if(its)
	{
		CGUITreeItem *it=NULL;
		while(it = its->GetNext(it))
		{
			data = (CSEdOBTreeData*)it->ut.userDataClass;

			if(data->GetType()==type && data->GetTag()!=OBtag)
			{
				cmp=strcmp(it->label->GetText(),name);				
				if(cmp==0){
					id = it->GetID();
					return it;
				}
			}

			if(type==data->GetType())
			{
				cmp=_stricoll(it->label->GetText(),name);
				if(cmp>0)
				{
					//if(prev) 
					id = it->GetID();
					return 0;
				}
			}else
				if(type<data->GetType())
				{
					//if(prev) 
					id = it->GetID();
					return 0;
				}
			//prev=it;
		}
	}

	//if(prev) 
	//id = it->GetID();

	return 0;
}

void CSEdObjectBrowser::UpdateOB()
{
	CreateOBitems();
}

int CSEdObjectBrowser::FocusChanged(UINT focus, typeID dualID) //zmenil se focus
{	
	if(focus)
	{
		SEdOpenedFiles->CompileForOB();
	}
	return CGUITree::FocusChanged(focus,dualID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CSEdOBEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CSEdOBEventHandler::CSEdOBEventHandler(CSEdObjectBrowser *ob)
{
	objectbrow=ob;
}

CSEdOBEventHandler::~CSEdOBEventHandler()
{
}

void CSEdOBEventHandler::EventHandler(CGUIEvent *event)
{
	if(!event) return;

	if(event->eventID==EClicked)
	{
		if(event->sender==objectbrow->butFB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowFileBrowser();
		}else
		if(event->sender==objectbrow->butNB->GetID())
		{
			CSEdDesktop *dd = dynamic_cast<CSEdDesktop*>(desktop);
			if(dd)
				dd->ShowNameBrowser();
		}
	}

	delete event;
}