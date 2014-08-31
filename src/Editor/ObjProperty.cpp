///////////////////////////////////////////////
//
// ObjProperty.cpp
//
//	implementace pro stromove nastavovani vlastnosti hernich objektu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "ObjProperty.h"
#include "desktop.h"
#include "editor.h"
#include "menu.h"
#include "textedit.h"
#include "dialogs.h"
#include "editorLevelWin.h"
#include "scriptInterface.h"

#include "kernel.h"
#include "autos.h"
#include "names.h"
#include "KerServices.h"
#include "KerErrors.h"
#include "levelmap.h"
#include "ComConstants.h"
#include "genmain.h"


CEDObjectTree* objectList=0;


//////////////////////////////////////////////////////////////////////
// CEDObjectTree
//////////////////////////////////////////////////////////////////////

CEDObjectTree::CEDObjectTree(float _x, float _y, float _sx, float _sy)
: CGUITree(_x,_y,_sx,_sy,"ObjectList",new CGUIRectHost(0,0,styleSet->Get("BUT_ObjectBrowser_Up")->GetTexture(0)))
{
	RemoveFromTrash();

	rollOnClick=false;
	keyClose=false;
	treeItemHeight=20;
	treeRootItemHeight=20;
	treeItemPictureSize=18;

	objHandler = new CEDObjectTreeHandler(1);
	// icon[0] == ContextMenu (delete ShortCut)

	objectList=this;
	title->SetButtons(false,true,true);

	CreateContextMenu();

	AddToTrash();
}

CEDObjectTree::~CEDObjectTree()
{
	SAFE_DELETE(objHandler);
	objectList=0;
}



void CEDObjectTree::LoadObjects()
{
	if(KerMain)
	{
		CKerNameList* nl;
		nl = KerMain->KerNamesMain->FindLayer(0,&FindNameObject,eKerNTobject,1);
		AddLayer(0,nl);
		UpdateTree();
	}
}

int CEDObjectTree::FindNameObject(CKerName* kn)
{
	if(!kn || !kn->ObjectType)
		return 0;

	if((kn->ObjectType->EditTag) & etbPodu)
		return 1;
	else
		return 0;
}

void CEDObjectTree::AddLayer(CGUITreeItem* ti, CKerNameList* nl)
{
	if(!KerMain)
		return;

	CKerNameList* l=nl;
	CGUITreeItem* nti=0;

	while(l)
	{
		OPointer nObj = 0;
		if((l->name->Type == eKerNTobject) && ((l->name->ObjectType->EditTag & etbInMap) || (l->name->ObjectType->EditTag & etbOutMap)))
			nObj = KerMain->NewObject(0,l->name);
		CTexture* objTex;
		bool releaseTex=false;

		PrepareObjTexture(nObj,&objTex,releaseTex);

		if(ti)
		{
			nti = ti->AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex),l->name->ObjectType->Comment,false);

			//nti = ti->AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,styleSet->Get("OB_Object")->GetTexture(0)));
			nti->userTagType=eUTkerName;
			nti->ut.kerName=l->name;
			nti->ut.obj=nObj;
		}else{
			nti = AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex),l->name->ObjectType->Comment,false);

			//nti = AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,styleSet->Get("OB_Object")->GetTexture(0)));
			nti->userTagType=eUTkerName;
			nti->ut.kerName=l->name;
			nti->ut.obj=nObj;
		}

		if(releaseTex && objTex)
			objTex->Release();

		CKerNameList* ll = KerMain->KerNamesMain->FindLayer(l->name,&FindNameObject,eKerNTobject,1);
		if(ll)
		{
			AddLayer(nti,ll);
			/*
			ll->DeleteAll();
			nti->state = 1;	// zabaleny uzel
			nti->ChangeRoller();
			*/
		}
		//AddLayer(nti,KerMain->KerNamesMain->FindLayer(l->name,&FindNameObject,eKerNTobject,1));
		l = l->next;
	}

	if(nl)
		nl->DeleteAll();

	if(ti)
		ti->Sort();
	else
		Sort();
}

void CEDObjectTree::TreeUpdateHandler(typeID treeItem, int index)
{
	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));
	if(ti && !ti->prepared && ti->userTagType == eUTkerName)
	{
		AddLayer(ti,KerMain->KerNamesMain->FindLayer(ti->ut.kerName,&FindNameObject,eKerNTobject,1));
		ti->prepared=true;
	}

	UpdateTree();
}


void CEDObjectTree::AddLayerWithAutos(CGUITreeItem* objTI, CGUITreeItem* parrTI, CKerNameList* nl)
{
	CKerNameList* l=nl;
	CGUITreeItem* nti=0;

	while(l)
	{
		if(parrTI)
		{
			CTexture* objTex;
			bool releaseTex=false;
			PrepareAutoTexture(l->name,&objTex,releaseTex);
			nti = parrTI->AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex));
			if(releaseTex && objTex)
				objTex->Release();
			nti->userTagType=eUTautomatism;
			nti->ut.aKerName=l->name;
			nti->ut.objTI=objTI;
		}
		if(KerMain)
			AddLayerWithAutos(objTI,nti,KerMain->KerNamesMain->FindLayer(l->name,0,eKerNTauto,1));
		l = l->next;
	}

	if(nl)
		nl->DeleteAll();

	if(parrTI)
		parrTI->Sort();
}

void CEDObjectTree::PrepareAutoTexture(CKerName* aName, CTexture** objTex, bool& releaseTex)
{
	releaseTex=false;

	if(aName)
	{
		if(aName->Auto->EditTex)
			*objTex=aName->Auto->EditTex;
		else
		{
			if(aName->Auto->Type == eKATpicture && aName->Auto->SonsNum>0)
			{
				//CGEnResource* gr = aName->Auto->textures[0].texture;
				//if(gr->GetType() == resLightTex)
				CLightTex* lt = dynamic_cast<CLightTex*>(aName->Auto->textures[0].texture);
				if(lt)
				{
					//lt = (CLightTex*) gr;
					if(lt && lt->CmpTex(objTex))
						releaseTex=true;
				}
				else
				{
					CGEnAnim* anim = dynamic_cast<CGEnAnim*>(aName->Auto->textures[0].texture);
					lt=0;
					if(anim)
						lt=anim->GetFrameTex(0);
					if(lt && lt->CmpTex(objTex))
						releaseTex=true;
				}
			}
			else
				*objTex = styleSet->Get("STD_AUTO_TEX")->GetTexture(0);
		}
	}
	else
	{
		*objTex = styleSet->Get("STD_AUTO_TEX")->GetTexture(0);
	}
}

void CEDObjectTree::PrepareObjTexture(OPointer obj, CTexture** objTex, bool& releaseTex)
{
	assert(KerMain && objTex);
	
	//*objTex = styleSet->Get("OB_Object")->GetTexture(0);
	//return;

	releaseTex=false;

	CKerObject *ko;
	ko = KerMain->Objs->GetObject(obj);

	if(ko)
	{
		*objTex=ko->Type->EditTex;
		if(!(*objTex)) 
		{
			CKerName* aName = KerMain->AutosMain->ProcessAuto(ko);
			if(aName)
			{
				if(aName->Auto->EditTex)
					*objTex=aName->Auto->EditTex;
				else
				{
					if(aName->Auto->SonsNum>0)
					{
						CLightTex* lt = dynamic_cast<CLightTex*>(aName->Auto->textures[0].texture);

						if(lt)
						{
							if(lt && lt->CmpTex(objTex))
								releaseTex=true;
						}
						else
						{
							CGEnAnim* anim = dynamic_cast<CGEnAnim*>(aName->Auto->textures[0].texture);
							lt=0;
							if(anim)
								lt=anim->GetFrameTex(0);
							if(lt && lt->CmpTex(objTex))
								releaseTex=true;
						}
					}
				}
			}
		}

		if(!(*objTex))
		{
			if((ko->Type->EditTag & etbInMap) || (ko->Type->EditTag & etbOutMap))
				*objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);
			else
				*objTex = styleSet->Get("OB_Object")->GetTexture(0);
		}
	}
	else
	{
		*objTex = styleSet->Get("OB_Object")->GetTexture(0);
	}
}


int CEDObjectTree::TreeHandler(typeID treeItem, int index, int state)
{
	if(state==3)
		return 1;

	CKerName *autoName=0;
	CEDObjectProperty *op=0;

	CGUITreeItem* ID(ti,treeItem);
	if(ti)
	{
		if(editor->GetEditMod()==eEMselectObj && ti->userTagType==eUTkerName && KerMain)
		{
			CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(editor->objVarSelectionID);
			if(opl)
			{
				OPointer o = KerMain->NewObject(0,ti->ut.kerName);
				if(ti->ut.kerName->ObjectType->EditTag & etbOutMap)
				{
					editor->AddLevelObject(o);
					editor->AddGlobalObject(o);
				}
				opl->SetObject(o);
			}
			return 1;
		}

		if(ti->userTagType==eUTautomatism)
		{
			autoName=ti->ut.aKerName;
			ti = ti->ut.objTI;
			if(!ti)
				return 1;
		}

		/*	// pridavani vrstev s automatismama vypnuto
		if(ti->state==0 && !ti->prepared && ti->userTagType==eUTkerName && KerMain)
		{
			AddLayerWithAutos(ti,ti,KerMain->KerNamesMain->FindLayer(ti->ut.kerName->ObjectType->AName,0,eKerNTauto,1));
			//AddLayerWithAutos(ti,ti,KerMain->KerNamesMain->FindLayer(ti->ut.kerName->ObjectType->DefAuto,0,eKerNTauto,1));
			UpdateTree();
		}
		*/

		if(op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID)))
		{
			if(ti->userTagType==eUTkerName)
			{
				if(op->obj != ti->ut.obj || !op->obj)
				{
					desktop->DeleteBackElem(op);
					if(CEDObjectProperty* opp=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID)))
						desktop->DeleteBackElem(opp);

					SetSelectedObject(0);
//					editor->SetEditMod(0);
				}
				else
				{
					CKerName **kn=0;
					CKerObject* ko = KerMain->GetAccessToObject(ti->ut.obj);
					if(ko)
						kn = (CKerName**)KerMain->GetAccessToVar3(ko,eKVUaPicture);
					if(kn)
					{
						if(autoName)
							*kn=autoName;
						else
							*kn=KnownNames[eKKNdefaultAuto].Name;

						if(op->aPictureTI)
							op->ShowObjValue(op->aPictureTI);

						/*
						if(op->aPictureTI)
						{
							op->ShowObjValue(op->aPictureTI);
							if(*kn)
							{
								char *name = (*kn)->GetShortNameString();
								op->aPictureEW->SetParametr(name,dtString,strlen(name));
							}else
								op->aPictureEW->SetParametr("0",dtString,2);
						}
						*/
					}

					op->FocusMe();
					SetSelectedObject(editor->objPropertyID);
					editor->SetEditMod(eEMinsert);
					editor->SetFreeModByObject();

					ShowMenu(state,treeItem);
					return 1;
				}
			}
		}

		if(ti->userTagType==eUTkerName && KerMain && (ti->ut.kerName->Type == eKerNTobject) &&((ti->ut.kerName->ObjectType->EditTag & etbInMap) || (ti->ut.kerName->ObjectType->EditTag & etbOutMap)))
		{
			if(op = ((CEDEditorDesktop*)desktop)->FindObjPropertyWin(ti->ut.obj))
			{
				op->FocusMe();
				if(objectList)
					objectList->SetSelectedObject(op->GetID());
				editor->SetEditMod(eEMinsert);
				editor->SetFreeModByObject();

				ShowMenu(state,treeItem);
				return 1;	// property okno pro objekt "obj" jiz v desktopu je, nove nevytvaret
			}		

			if(!ti->ut.obj || !KerMain->Objs->GetObject(ti->ut.obj))
			{
				ti->ut.obj = KerMain->NewObject(0,ti->ut.kerName);	// !@#$ osetrit vyjimku nebo nemusim ??
			}

			CKerName **kn=0;
			CKerObject* ko = KerMain->GetAccessToObject(ti->ut.obj);
			if(ko)
				kn = (CKerName**)KerMain->GetAccessToVar3(ko,eKVUaPicture);
			if(kn)
			{
				if(autoName)
					*kn=autoName;
				else
					*kn=KnownNames[eKKNdefaultAuto].Name;
			}


			op = new CEDObjectProperty(editor->objPropertyPosition.x,editor->objPropertyPosition.y,editor->objPropertyPosition.sx,editor->objPropertyPosition.sy,ti->ut.obj,treeItem);
			desktop->AddBackElem(op);

			desktop->SetFocusEl(op,0);
			op->AcceptEvent(objHandler->GetID(),EClose);
			op->AcceptEvent(objHandler->GetID(),EFocusChanged);

			SetSelectedObject(op->GetID());
			editor->SetEditMod(eEMinsert);
			editor->SetFreeModByObject();

			ShowMenu(state,treeItem);
		}
		else if(ti->state==0)	// jedna se o list (u polozek reprezentujicich skupinu objektu nechci chybovou hlasku zobrazovat)
		{
			//GUIMessageBox("Chyba p¯i vytv·¯enÌ objektu!","Objekt nelze vytvo¯it neboù nenÌ umÌstiteln˝ do mapy ani mezi glob·lnÌ objekty!");
		}

	}

	return 1;
}

void CEDObjectTree::ShowMenu(int state, typeID treeItem)
{
	if(state==2)	// prave tlacitko mysi
	{
		CGUIMenu* menu=0;
		CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(treeItem);
		if(ti && ti->userTagType==eUTkerName && ti->ut.obj)
		{
			menu = GetObjFromID<CGUIMenu>(objHandler->icons[0]);
			if(menu)
			{
				float dx=0,dy=0;
				float fsx,fsy;
				desktop->GetDesktopPos(ti,dx,dy);
				ti->GetSize(fsx,fsy);
				menu->On(ti->GetID(),dx+10,dy+fsy);
			}
		}
	}
}

void CEDObjectTree::SetSelectedObject(typeID _selectedObjectPropertyID)
{
	if(!editor)
		return;

	editor->objPropertyID = _selectedObjectPropertyID;

	if(KerMain && editor->objPropertyID)
	{
		CEDObjectProperty* op = dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
		if(op)
		{

			if(CKerObject *ko = KerMain->Objs->GetObject(op->obj))
			{
				char *name = ko->Type->Name->GetUserName();
				char winname[100];
				_snprintf(winname,99,"OL - %s",name);
				title->SetText(winname);
				return;
			}
		}
	}

	title->SetText("ObjectList");
}

void CEDObjectTree::AddShortCutObj(OPointer obj, char* scName)
{
	CKerName* kn = KerMain->GetObjType(obj);
	if(!kn)
		return;

	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUITreeItem*)items->GetObjectFromListElement(le)))
		{
			if(e->userTagType == eUTkerName && e->ut.kerName == kn)
			{
				MakeShortCutObjItem(e,kn,obj,scName);
				break;
			}
			if(e->state && e->userTagType == eUTkerName)
				if(AddShortCutObjItem(e,kn,obj,scName))
					break;
		}
		le=items->GetNextListElement(le);
	}

	UpdateTree();
}

int CEDObjectTree::AddShortCutObjItem(CGUITreeItem* ti, CKerName* kn, OPointer obj, char* scName)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = ti->items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUITreeItem*)ti->items->GetObjectFromListElement(le)))
		{
			if(e->userTagType == eUTkerName && e->ut.kerName == kn)
			{
				MakeShortCutObjItem(e,kn,obj,scName);
				return 1;
			}
			if(e->state && e->userTagType == eUTkerName)
				if(AddShortCutObjItem(e,kn,obj,scName))
					return 1;
		}
		le=ti->items->GetNextListElement(le);
	}

	return 0;
}

void CEDObjectTree::MakeShortCutObjItem(CGUITreeItem* ti, CKerName* kn, OPointer obj, char* scName)
{
	CGUITreeItem* nti;

	char name[301];
	_snprintf(name,300,"%s - %s",kn->GetUserName(),scName);

	CTexture* objTex;
	bool releaseTex=false;
	PrepareObjTexture(obj,&objTex,releaseTex);

	if(ti->parentItem)
	{
		nti = ti->parentItem->AddTreeItem(ti->GetID(),name,new CGUIRectHost(0,0,objTex),kn->ObjectType->Comment);
		nti->userTagType=eUTkerName;
		nti->ut.kerName=kn;
		nti->ut.obj=obj;
	}else{
		nti = AddTreeItem(ti->GetID(),name,new CGUIRectHost(0,0,objTex),kn->ObjectType->Comment);
		nti->userTagType=eUTkerName;
		nti->ut.kerName=kn;
		nti->ut.obj=obj;
	}

	if(releaseTex && objTex)
		objTex->Release();
}

void CEDObjectTree::CreateContextMenu()
{
	CGUIMenu* menu;

	menu = new CGUIMenu(170);
	menu->AddMenuItem(0,"Make ShortCut",new CGUIRectHost(0,0,styleSet->Get("OP_ShortCutTB1")->GetTexture(0)));
	menu->AddMenuItem(0,"Delete ShortCut",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));


	menu->AcceptEvent(objHandler->GetID(),EMenu);
	objHandler->icons[0] = menu->GetID();
}

void CEDObjectTree::RefreshAutos(CKerName* kn)
{
	if(!kn)
		return;

	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUITreeItem*)items->GetObjectFromListElement(le)))
		{
			if(e->userTagType == eUTkerName && e->ut.kerName == kn)
			{
				RefresItemAutos(e);
				break;
			}
			if(e->state && e->userTagType == eUTkerName)
				if(RefresAutosItemRecursive(e,kn))
					break;
		}
		le=items->GetNextListElement(le);
	}

	UpdateTree();
}

int CEDObjectTree::RefresAutosItemRecursive(CGUITreeItem* ti, CKerName* kn)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = ti->items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUITreeItem*)ti->items->GetObjectFromListElement(le)))
		{
			if(e->userTagType == eUTkerName && e->ut.kerName == kn)
			{
				RefresItemAutos(e);
				return 1;
			}
			if(e->state && e->userTagType == eUTkerName)
				if(RefresAutosItemRecursive(e,kn))
					return 1;
		}
		le=ti->items->GetNextListElement(le);
	}

	return 0;
}

void CEDObjectTree::RefresItemAutos(CGUITreeItem* ti)
{
	if(ti->state)
		while(CGUITreeItem* tti = ti->items->GetFromHead())
		{
			if(tti->userTagType == eUTautomatism)
				ti->DeleteTreeItem(tti);
		}

	AddLayerWithAutos(ti,ti,KerMain->KerNamesMain->FindLayer(ti->ut.kerName->ObjectType->AName,0,eKerNTauto,1));
}

//////////////////////////////////////////////////////////////////////
// CEDObjectProperty
//////////////////////////////////////////////////////////////////////

CEDObjectProperty::CEDObjectProperty(float _x, float _y, float _sx, float _sy, OPointer _obj, typeID _objectListItemID, bool _objAlreadyPlaced)
	: CGUITree(_x,_y,_sx,_sy,"ObjectProperty",new CGUIRectHost(0,0,styleSet->Get("EI_ObjProperty")->GetTexture(0)))
{
	RemoveFromTrash();

	if(!KerMain)
	{
		throw CExc(eGUI,E_INTERNAL,"CEDObjectProperty::CEDObjectProperty> KerMain not exist");
	}

	try{
		//onWindowPanel=false;
		ownObject=false;
		objAlreadyPlaced=_objAlreadyPlaced;
		obj = _obj;
		objectListItemID = _objectListItemID;
		menuID=0;
		aPictureTI=0;
		posXTI=posYTI=posZTI=0;
		errorMsg=0;
		globalVars=false;
		parentObjPropertyID=0;
		shortCutName=0;
		editedGroupItem=0;
		scriptHandler=0;
		beforeLocationEditMod=eEMnormal;

		objComment=0;

		CKerObject *ko=0;
		if((ko = KerMain->Objs->GetObject(obj)))
		{
			char *name = ko->Type->Name->GetUserName();
			char winname[100];
			if(objAlreadyPlaced)
			{
				_snprintf(winname,99,"%s - %d property",name,obj);
				title->SetActiveInactiveColor(0xFF00DDFF, 0xFF00BBCC);
			}
			else
				_snprintf(winname,99,"%s property",name);
			title->SetText(winname);

			if(ko->Type->DefAuto==0)	// objekty bez automatismu
			{
				SetErrorMsg("Default A not exists!");
			}
		}
		
		AddHandlerAndPanel();


		AddContextMenu();

		CreateVarProperty();

		PrepareObjCursor();

		if(ko && ko->Type->Comment)
		{
			float nsx,nsy;
			float ssx,ssy;
			GetBackWindowSize(nsx,nsy);
			objComment = new CGUIStaticText(ko->Type->Comment,treeTextFont,0,0,0xFF000000,nsx-30);
			AddBackElem(objComment);
			objComment->GetSize(ssx,ssy);
			objComment->SetAnchor(0,1,1,0);

			treeEndGapY+=ssy+10;
			UpdateTree();
		}

	}catch (...){
		CGUIMenu* ID(menu,menuID);
		if(menu)
			delete menu;

		SAFE_DELETE(handler);

		throw;
	}


	AddToTrash();
}

CEDObjectProperty::CEDObjectProperty(float _x, float _y, float _sx, float _sy, bool _globalVars)
	: CGUITree(_x,_y,_sx,_sy,"Global Vars Property",new CGUIRectHost(0,0,styleSet->Get("EI_ObjProperty")->GetTexture(0)))
{
	RemoveFromTrash();

	if(!KerMain)
	{
		throw CExc(eGUI,E_INTERNAL,"CEDObjectProperty::CEDObjectProperty> KerMain not exist");
	}

	try{
		//onWindowPanel=false;
		ownObject=false;
		objAlreadyPlaced=false;
		obj = 0;
		objectListItemID = 0;
		menuID=0;
		aPictureTI=0;
		posXTI=posYTI=posZTI=0;
		errorMsg=0;
		globalVars=_globalVars;
		parentObjPropertyID=0;
		shortCutName=0;
		editedGroupItem=0;
		scriptHandler=0;
		beforeLocationEditMod=eEMnormal;

		objComment=0;

		AddHandlerAndPanel();

		AddContextMenu();

		CreateVarProperty();

		PrepareObjCursor();

	}catch(...)
	{
		CGUIMenu* ID(menu,menuID);
		if(menu)
			delete menu;

		SAFE_DELETE(handler);

		throw;
	}

	AddToTrash();
}

CEDObjectProperty::~CEDObjectProperty()
{
	CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(handler->icons[2]);
	if(opl && desktop)
		opl->CloseWindow();
	CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(handler->icons[3]);
	if(op && desktop)
		op->CloseWindow();

	CEDNoConnectDialog* ncd = GetObjFromID<CEDNoConnectDialog>(handler->icons[4]);
	if(ncd && desktop)
		desktop->DeleteBackElem(ncd);


	if(parentObjPropertyID && objectList)
	{
		objectList->SetSelectedObject(parentObjPropertyID);
	}

	CGUIMenu* ID(menu,menuID);
	if(menu)
		delete menu;

	SAFE_DELETE(handler);
	SAFE_DELETE(scriptHandler);

	// vymacknuti tlacitka na toolbaru, ktere odpovida prave rusenemu objProperty
	if(editor)
	{
		CGUIToolBar* tb = GetObjFromID<CGUIToolBar>(editor->globalObjsTB);
		if(tb)
		{
			if(globalVars)
			{
				CGUIToolBarButton* tbb = tb->GetTool(0,0);
				if(tbb)
					tbb->ChangeState(false);
			}
			else if(CKerObject *ko = KerMain->Objs->GetObject(obj))
			{
				int index = editor->globalObjects->FindObj(obj);
				if(index !=-1)
				{
					CGUIToolBarButton* tbb = tb->GetTool(index+1,0);
					if(tbb)
						tbb->ChangeState(false);
				}
			}
		}

		if(editor->GetEditMod() == eEMselectLocation)
		{
			CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
			if(lw)
				lw->DeleteMarker();

			editor->editMode=eEMnormal;
		}

	}

	if(ownObject && KerMain && obj)
		try{
			KerMain->DeleteObject(0,obj);
		}catch(CKernelPanic){
			editor->KernelShutDown();
			//KerErrorFiles->ShowAllLogs(100,90,640,600);
			//SAFE_DELETE(KerMain);
		}

	SAFE_DELETE_ARRAY(shortCutName);
}

int CEDObjectProperty::VarFilter(CKerOVar* OVar)
{
	if(!(OVar->Use & eKVUBeditable) && !GroupVarFilter(OVar))	// promenna se nelouduje z levelu (!@#$ kontrolovat edit tag atp.)
		return 0;												//  && !(OVar->Type == eKTvoid && OVar->EditType == eKETscripted))

	return 1;
}

int CEDObjectProperty::GroupVarFilter(CKerOVar* OVar)
{
	if(OVar->Type == eKTvoid && OVar->EditType >= eKET2Dcell && OVar->EditType <= eKET3DcellArea) 
		return 1;
	else
		return 0;
}

CGUITreeItem* CEDObjectProperty::AddEditItem(CGUITreeItem* groupItem, CKerOVar* OVar, typeID where, bool first, bool ownOVar)
{
	char *name;
	char *help;

	if(OVar->UserName)
	{
		name=OVar->UserName;
		if(OVar->Comment)
			help=OVar->Comment;
		else
			help=KerServices.ConverNameToShort(OVar->NameStr,1);
	}
	else
	{
		name = KerServices.ConverNameToShort(OVar->NameStr,1);
		help = OVar->Comment;
	}

	CGUITreeItem* ti=0;
	if(groupItem)
		ti = groupItem->AddTreeItem(where,name,0,help,false,first); 
	else
		ti = AddTreeItem(where,name,0,help,false,first); 

	if(ownOVar)
		ti->userTagType = eUTOVarDEL;
	else
		ti->userTagType = eUTOVar;

	ti->ut.OVar = OVar;
	ti->AcceptEvent(handler->GetID(),EBadInput);
	ti->AcceptEvent(handler->GetID(),ETreeItemRoller);
	ti->AcceptEvent(handler->GetID(),EFocusChanged);

	return ti;
}

CGUITreeItem* CEDObjectProperty::AddEditArrayItem(CGUITreeItem* root, CKerOVar* OVar, int index, typeID where, bool first)
{
	char name[50];
	char *help=0;

	if(OVar->Comment)
		help=OVar->Comment;

	_snprintf(name,49,"%d. value:",index);

	CGUITreeItem* ti = root->AddTreeItem(where,name,0,help,false,first); 
	ti->userTagType = eUTint;
	ti->ut.pInt = index;
	ti->AcceptEvent(handler->GetID(),EBadInput);

	return ti;
}


CGUIEditWindow* CEDObjectProperty::CreateEditWindow()
{
	CGUIEditWindow* ed;

	ed = new CEDEditWindow(0,1,130,0xFF000000,0xFFFFFFFF,"DX.F.Arial.10");
	ed->SelectOnFocus();
	ed->AcceptEvent(handler->GetID(),EOk);
	ed->AcceptEvent(handler->GetID(),ECancel);

	return ed;
}

CGUICheckBox* CEDObjectProperty::CreateCheckBox(char* text)
{
	CGUICheckBox* cb;

	//cb = new CGUICheckBox(0,0,text,0,1,0xFF000000);
	cb = new CGUICheckBox(0,0,new CGUIStaticText(text,(CGUIFont*)RefMgr->Find("GUI.F.Arial.10"),0,0,0xFF000000),0,1,0xFF000000);
	cb->SetMark(true);
	cb->AcceptEvent(handler->GetID(),EClicked);

	return cb;
}


// ------------------------------------------------------------------------------------------
// pridavani promenych ruznych typu:
void CEDObjectProperty::AddIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	int valueType=OVar->EditType;	// tag rozhodujici jaka hodnota se ma editovat (int/bool)

	int pInt=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrInt* pArray = *(CKerArrInt**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pInt = pArray->Read(arrIndex-1);
		}
	}
	else
	{
		pInt = *(int*)GetAccessToVar(ko,OVar);	// jedna se o jeden int
	}

	if(valueType==eKETdefault || valueType==eKETnumeric)	// int
	{
		CGUIEditWindow* ed = CreateEditWindow();
		ed->SetParametr(&pInt,dtInt,1);

		int useTag = (OVar->Use >> KERVARUSESSHIFT);
		if(useTag != eKVUnone)
		{
			if(useTag == eKVUx)
				posXTI = ti;
			if(useTag == eKVUy)
				posYTI = ti;
			if(useTag == eKVUz)
				posZTI = ti;
		}

		if(OVar->LimintsCfg==eKLClist)	// seznam hodnot => vytvorim buton pro rozbaleni nabidky hodnot (musi se dat cele do okna (polozka unese jen jeden element))
		{
			CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
			win->onWindowPanel=false;

			ed->SetTabOrder(1);
			
			CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
			b->AcceptEvent(handler->GetID(),EClicked);
			b->SetMark(true);
			b->SetTabOrder(2);
			win->AddBackElem(b);

			win->AddBackElem(ed);
			win->SetFocusEl(0,0);

			ti->SetItemElement(win,true);
		}
		else
			ti->SetItemElement(ed,true);

		ed->changed=0;
	}
	else if(valueType==eKETboolean)		// bool
	{
		char *name;

		if(OVar->UserName)
			name=OVar->UserName;
		else
			name = KerServices.ConverNameToShort(OVar->NameStr,1);

		CGUICheckBox* cb = CreateCheckBox(name);
		cb->SetParametr(&pInt,dtInt,1);

		ti->SetItemElement(cb,true);
	}
	else if(valueType==eKETconnectionMask)		// bool
	{
		CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
		b->AcceptEvent(handler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder(1);
		ti->SetItemElement(b,true);
	}
}

void CEDObjectProperty::AddCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	int valueType=OVar->EditType;	// !@#$ tag rozhodujici jaka hodnota se ma editovat (char/int/bool)

	int pChar=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrChar* pArray = *(CKerArrChar**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pChar = (unsigned char)pArray->Read(arrIndex-1);
		}
	}
	else
		pChar = *(unsigned char*)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	if(valueType==eKETdefault || valueType==eKETcharacter)	// char
	{
		CGUIEditWindow* ed = CreateEditWindow();
		ed->SetParametr(&pChar,dtChar,1);

		if(OVar->LimintsCfg==eKLClist)	// seznam hodnot => vytvorim buton pro rozbaleni nabidky hodnot (musi se dat cele do okna (polozka unese jen jeden element))
		{
			CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
			win->onWindowPanel=false;

			ed->SetTabOrder(1);
			
			CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
			b->AcceptEvent(handler->GetID(),EClicked);
			b->SetMark(true);
			b->SetTabOrder(2);
			win->AddBackElem(b);

			win->AddBackElem(ed);
			win->SetFocusEl(0,0);

			ti->SetItemElement(win,true);
		}
		else
			ti->SetItemElement(ed,true);

		ed->changed=0;
	}
	else if(valueType==eKETnumeric)	// int (cislo)
	{
		CGUIEditWindow* ed = CreateEditWindow();
		ed->SetParametr(&pChar,dtInt,1);

		if(OVar->LimintsCfg==eKLClist)	// seznam hodnot => vytvorim buton pro rozbaleni nabidky hodnot (musi se dat cele do okna (polozka unese jen jeden element))
		{
			CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
			win->onWindowPanel=false;

			ed->SetTabOrder(1);
			
			CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
			b->AcceptEvent(handler->GetID(),EClicked);
			b->SetMark(true);
			b->SetTabOrder(2);
			win->AddBackElem(b);

			win->AddBackElem(ed);
			win->SetFocusEl(0,0);

			ti->SetItemElement(win,true);
		}
		else
			ti->SetItemElement(ed,true);

		ed->changed=0;

	}
	else if(valueType==eKETboolean)	// bool
	{
		char *name;

		if(OVar->UserName)
			name=OVar->UserName;
		else
			name = KerServices.ConverNameToShort(OVar->NameStr,1);

		CGUICheckBox* cb = CreateCheckBox(name);
		cb->SetParametr(&pChar,dtInt,1);

		ti->SetItemElement(cb,true);
	}
}

void CEDObjectProperty::AddDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	CGUIEditWindow* ed = CreateEditWindow();

	double pDouble=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrDouble* pArray = *(CKerArrDouble**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pDouble = pArray->Read(arrIndex-1);
		}
	}
	else
		pDouble = *(double*)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	ed->SetParametr(&pDouble,dtDouble,1);

	if(OVar->LimintsCfg==eKLClist)	// seznam hodnot => vytvorim buton pro rozbaleni nabidky hodnot (musi se dat cele do okna (polozka unese jen jeden element))
	{
		CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
		win->onWindowPanel=false;

		ed->SetTabOrder(1);
		
		CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
		b->AcceptEvent(handler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder(2);
		win->AddBackElem(b);

		win->AddBackElem(ed);
		win->SetFocusEl(0,0);

		ti->SetItemElement(win,true);
	}
	else
		ti->SetItemElement(ed,true);

	ed->changed=0;
}

void CEDObjectProperty::AddStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti)
{
	CGUIEditWindow* ed = CreateEditWindow();

	char *string = (char*)GetAccessToVar(ko,OVar);
	if(string)
		ed->SetParametr(string,dtString,strlen(string));

	if(OVar->LimintsCfg==eKLClist)	// seznam hodnot => vytvorim buton pro rozbaleni nabidky hodnot (musi se dat cele do okna (polozka unese jen jeden element))
	{
		CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
		win->onWindowPanel=false;

		ed->SetTabOrder(1);
		
		CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
		b->AcceptEvent(handler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder(2);
		win->AddBackElem(b);

		win->AddBackElem(ed);
		win->SetFocusEl(0,0);

		ti->SetItemElement(win,true);
	}
	else
		ti->SetItemElement(ed,true);

	ed->changed=0;
}

void CEDObjectProperty::AddKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	CGUIEditWindow* ed = CreateEditWindow();

	CKerName* kn=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrName* pArray = *(CKerArrName**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			kn = pArray->Read(arrIndex-1);
		}
	}
	else
		kn = *(CKerName**)GetAccessToVar(ko,OVar);	// jedna se o jeden int

	ed->SetReadOnly();

	if(kn)
	{
		if(KerMain->KerNamesMain->TestPointerValidity(kn))
		{
			ed->SetParametr(kn->GetUserName(),dtString,strlen(kn->GetUserName()));
			if((OVar->Use >> KERVARUSESSHIFT) == eKVUaPicture)
				aPictureTI = ti;
		}
		else
		{
			ed->SetParametr("X - Bad Name - X",dtString,20);
		}
	}else
		ed->SetParametr("0",dtString,2);

	CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
	win->onWindowPanel=false;

	//ed->SetTabOrder(1);
		
	CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
	b->AcceptEvent(handler->GetID(),EClicked);
	b->SetMark(true);
	b->SetTabOrder(2);
	
	win->AddBackElem(ed);
	win->AddBackElem(b);

	win->SetFocusEl(0,0);

	ti->SetItemElement(win,true);


//	ti->SetItemElement(ed,true);
	ed->changed=0;
}

void CEDObjectProperty::AddObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	CGUIEditWindow* ed = CreateEditWindow();

	OPointer pObj=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrObject* pArray = *(CKerArrObject**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pObj = pArray->Read(arrIndex-1);
		}
	}
	else
		pObj = *(OPointer*)GetAccessToVar(ko,OVar);	// jedna se o jeden int

	ed->SetReadOnly();


	CKerObject* k  = KerMain->Objs->GetObject(pObj);

	if(k)
	{
		char text[201];
		_snprintf(text,200,"%d - %s",pObj,k->Type->Name->GetUserName());
		ed->SetParametr(text,dtString,strlen(text));
	}
	else
	{
		if(pObj)
			ed->SetParametr("X - Bad Object - X",dtString,20);
		else
			ed->SetParametr("0",dtString,2);
	}

	CGUIWindow* win = new CGUIWindow(0,0,152,18,0);
	win->onWindowPanel=false;

	//ed->SetTabOrder(1);
		
	CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-");
	b->AcceptEvent(handler->GetID(),EClicked);
	b->AcceptEvent(handler->GetID(),EMouseButton);
	b->SetMark(true);
	b->SetTabOrder(2);
	
	win->AddBackElem(ed);
	win->AddBackElem(b);
	
	win->SetFocusEl(0,0);

	ti->SetItemElement(win,true);

	//ti->SetItemElement(ed,true);
	ed->changed=0;
}


void *CEDObjectProperty::GetAccessToVar(CKerObject* ko, CKerOVar* Var)
{ 
	if(globalVars)
	{
		assert(Var->KSVar);
		return *(Var->KSVar);
	}
	else
	{
		assert(KerMain);
		return KerMain->GetAccessToVar2(ko,Var);
	}
}


void CEDObjectProperty::CreateVarProperty()
{
	treeItemPictureSize=-6;
	treeItemStartX=8;
	treeItemStartY=40;
	treeRootNormalItemGap=5;

	CGUITreeItem* ti=0;
	CKerOVar* OVar;

	CGUITreeItem* groupItem=0;
	int numGroupItems=0;

	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;

	int num;
	if(globalVars)
		num = KerMain->NumGV;
	else
		num = ko->Type->NumVars;

	for(int i = 0; i <num; i++)
	{
		if(globalVars)
			OVar = &(KerMain->GlobalVar[i]);
		else
			OVar = &(ko->Type->OVars[i]);

		if(!VarFilter(OVar))	// filtr, ktere promenne jsou pro editor zajimave
			continue;

		ti = AddEditItem(groupItem,OVar);	// vytvori polozku stromu (jmeno, komentar, zakladni nastaveni polozky)

		AddVarProperty(ko,OVar,ti,groupItem,numGroupItems);
		/*
		if(!groupItem && GroupVarFilter(OVar))
		{
			groupItem=ti;

			CGUIButton* b = new CGUIButton(135,2,14,14,"GUI.But.Std","-",0,"Clear");
			b->AcceptEvent(handler->GetID(),EClicked);
			b->SetMark(true);
			b->SetTabOrder(1);
			groupItem->SetItemElement(b,true);

			if(OVar->EditType == eKET2Dcell || OVar->EditType == eKET2Dpoint)
				numGroupItems=2;
			else if(OVar->EditType == eKET3Dcell || OVar->EditType == eKET3Dpoint)
				numGroupItems=3;
			else if(OVar->EditType == eKET2Darea)
				numGroupItems=4;
			else if(OVar->EditType == eKET3Darea)
				numGroupItems=6;
			else
				numGroupItems=0;
		}
		else
		{
			if(numGroupItems)
				numGroupItems--;
			
			if(numGroupItems<=0)
			{
				groupItem=0;
				numGroupItems=0;
			}
		}

		int type = OVar->Type;
		if(type>=eKTstring && type<=eKTstring+250)
			AddStringVar(ko,OVar,ti);
		else switch(type)
		{
			case eKTchar :
				AddCharVar(ko,OVar,ti);
				break;
			case eKTdouble :
				AddDoubleVar(ko,OVar,ti);
				break;
			case eKTint :
				AddIntVar(ko,OVar,ti);
				break;
			case eKTobject :
				AddObjectVar(ko,OVar,ti);
				break;
			case eKTname :
				AddKerNameVar(ko,OVar,ti);
				break;
			case eKTarrChar :
			case eKTarrDouble :
			case eKTarrInt :
			case eKTarrObject :
			case eKTarrName :
				AddArrayVarProperty(ko,OVar,ti);
				break;
			case eKTvoid :
				if(OVar->EditType == eKETscripted)
					CGUIScriptVariableInterface::ScriptedVariableInit(this,OVar,ti);
				break;
		}
		*/
	}
	UpdateTree();
}

void CEDObjectProperty::AddVarProperty(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, CGUITreeItem* &groupItem, int &numGroupItems)
{
	if(!groupItem && GroupVarFilter(OVar))
	{
		groupItem=ti;

		CGUIButton* b = new CGUIButton(135,2,24,14,"GUI.But.Std","x",0,"Clear");
		b->AcceptEvent(handler->GetID(),EClicked);
		b->SetMark(true);
		b->SetTabOrder(1);
		groupItem->SetItemElement(b,true);

		if(OVar->EditType == eKET2Dcell || OVar->EditType == eKET2Dpoint)
			numGroupItems=2;
		else if(OVar->EditType == eKET3Dcell || OVar->EditType == eKET3Dpoint)
			numGroupItems=3;
		else if(OVar->EditType == eKET2Darea)
			numGroupItems=4;
		else if(OVar->EditType == eKET3Darea)
			numGroupItems=6;
		else if(OVar->EditType == eKET2DcellArea)
			numGroupItems=4;
		else if(OVar->EditType == eKET3DcellArea)
			numGroupItems=6;
		else
			numGroupItems=0;
	}
	else
	{
		if(numGroupItems)
			numGroupItems--;
		
		if(numGroupItems<=0)
		{
			groupItem=0;
			numGroupItems=0;
		}
	}

	int type = OVar->Type;
	if(type>=eKTstring && type<=eKTstring+250)
		AddStringVar(ko,OVar,ti);
	else switch(type)
	{
		case eKTchar :
			AddCharVar(ko,OVar,ti);
			break;
		case eKTdouble :
			AddDoubleVar(ko,OVar,ti);
			break;
		case eKTint :
			AddIntVar(ko,OVar,ti);
			break;
		case eKTobject :
			AddObjectVar(ko,OVar,ti);
			break;
		case eKTname :
			AddKerNameVar(ko,OVar,ti);
			break;
		case eKTarrChar :
		case eKTarrDouble :
		case eKTarrInt :
		case eKTarrObject :
		case eKTarrName :
			AddArrayVarProperty(ko,OVar,ti);
			break;
		case eKTvoid :
			if(OVar->EditType == eKETscripted)
				CGUIScriptVariableInterface::ScriptedVariableInit(this,OVar,ti);
			break;
	}
}


void CEDObjectProperty::AddArrayVarProperty(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* root)
{
	CGUITreeItem* ti=0;
	int j;

	int count = 0;
	int created = 0;

	switch(OVar->Type)
	{
		case eKTarrChar :
			{
				CKerArrChar* pArray = *(CKerArrChar**)GetAccessToVar(ko,OVar);
				created = pArray ? 1 : 0;
				if(pArray && pArray->CheckIntegrity())
				{
					count = pArray->GetCount();
				}
			}
			break;
		case eKTarrDouble :
			{
				CKerArrDouble* pArray = *(CKerArrDouble**)GetAccessToVar(ko,OVar);
				created = pArray ? 1 : 0;
				if(pArray && pArray->CheckIntegrity())
				{
					count = pArray->GetCount();
				}
			}
			break;
		case eKTarrInt :
			{
				CKerArrInt* pArray = *(CKerArrInt**)GetAccessToVar(ko,OVar);
				created = pArray ? 1 : 0;
				if(pArray && pArray->CheckIntegrity())
				{
					count = pArray->GetCount();
				}
			}
			break;
		case eKTarrObject :
			{
				CKerArrObject* pArray = *(CKerArrObject**)GetAccessToVar(ko,OVar);
				created = pArray ? 1 : 0;
				if(pArray && pArray->CheckIntegrity())
				{
					count = pArray->GetCount();
				}
			}
			break;
		case eKTarrName :
			{
				CKerArrName* pArray = *(CKerArrName**)GetAccessToVar(ko,OVar);
				created = pArray ? 1 : 0;
				if(pArray && pArray->CheckIntegrity())
				{
					count = pArray->GetCount();
				}
			}
			break;
	}



	for(int i = 0; i <count; i++)
	{
		j=i+1;
		ti = AddEditArrayItem(root,OVar,j);	// vytvori polozku stromu (index, komentar)

		switch(OVar->Type)
		{
			case eKTarrChar :
				AddCharVar(ko,OVar,ti,j);
				break;
			case eKTarrDouble :
				AddDoubleVar(ko,OVar,ti,j);
				break;
			case eKTarrInt :
				AddIntVar(ko,OVar,ti,j);
				break;
			case eKTarrObject :
				AddObjectVar(ko,OVar,ti,j);
				break;
			case eKTarrName :
				AddKerNameVar(ko,OVar,ti,j);
				break;
		}
	}

	//pridat add/del button na konec

	ti = root->AddTreeItem(0,"Item:");

	CGUIWindow* win = new CGUIWindow(0,0,152,26,0);
	win->onWindowPanel=false;

	CGUIButton* b = new CGUIButton(5,2,40,22,"GUI.But.Std","Add",0,"Add new item to the end");
	b->AcceptEvent(handler->GetID(),EClicked);
	b->SetMark(true);
	b->SetTabOrder(1);
	win->AddBackElem(b);
	typeID addButtonID = b->GetID();

	b = new CGUIButton(55,2,50,22,"GUI.But.Std","Delete",0,"Delete last item");
	b->AcceptEvent(handler->GetID(),EClicked);
	b->SetMark(true);
	b->SetTabOrder(2);
	win->AddBackElem(b);
	typeID delButtonID = b->GetID();

	CGUICheckBox* ch = new CGUICheckBox(110,2,"C.",created);
	ch->AcceptEvent(handler->GetID(),EClicked);
	ch->SetMark(true);
	ch->SetTabOrder(3);
	ch->ChangeContextHelp("Array Created");
	win->AddBackElem(ch);
	typeID createCheckBoxID = ch->GetID();

	win->SetFocusEl(0,0);
	ti->SetItemElement(win,true);

	// nastavit data u korenove polozky odpovidajici celemu poli
	bool delOvar = (root->userTagType==eUTOVarDEL) ? true : false;
	root->userTagType = eUTdataClass;
	root->ut.userDataClass = new CEDVarArrayItemData(OVar,addButtonID,delButtonID,createCheckBoxID,delOvar);
}

void CEDObjectProperty::AddNewArrayVarItemProperty(CGUITreeItem* root, CKerOVar* OVar, typeID treeItemID)
{
	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;


	CGUITreeItem* ti=0;

	switch(OVar->Type)
	{
		case eKTarrChar :
			{
				CKerArrChar** pArray = (CKerArrChar**)GetAccessToVar(ko,OVar);

				if(*pArray && (*pArray)->CheckIntegrity())
				{
					int count = 0;
					char val=0;
					if(OVar->DefaultMember)
						val=OVar->DefaultMember->Dchar;

					(*pArray)->Add(val);
					count = (*pArray)->GetCount();

					ti = AddEditArrayItem(root,OVar,count,treeItemID,true);	// vytvori polozku stromu (index, komentar)
					AddCharVar(ko,OVar,ti,count);
				}
			}
			break;
		case eKTarrDouble :
			{
				CKerArrDouble** pArray = (CKerArrDouble**)GetAccessToVar(ko,OVar);
				if(*pArray && (*pArray)->CheckIntegrity())
				{
					int count = 0;
					double val=0;
					if(OVar->DefaultMember)
						val=OVar->DefaultMember->Ddouble;

					(*pArray)->Add(val);
					count = (*pArray)->GetCount();

					ti = AddEditArrayItem(root,OVar,count,treeItemID,true);	// vytvori polozku stromu (index, komentar)
					AddDoubleVar(ko,OVar,ti,count);
				}
			}
			break;
		case eKTarrInt :
			{
				CKerArrInt** pArray = (CKerArrInt**)GetAccessToVar(ko,OVar);
				if(*pArray && (*pArray)->CheckIntegrity())
				{
					int count = 0;
					int val=0;
					if(OVar->DefaultMember)
						val=OVar->DefaultMember->Dint;

					(*pArray)->Add(val);
					count = (*pArray)->GetCount();

					ti = AddEditArrayItem(root,OVar,count,treeItemID,true);	// vytvori polozku stromu (index, komentar)
					AddIntVar(ko,OVar,ti,count);
				}
			}
			break;
		case eKTarrObject :
			{
				CKerArrObject** pArray = (CKerArrObject**)GetAccessToVar(ko,OVar);
				if(*pArray && (*pArray)->CheckIntegrity())
				{
					int count = 0;
					OPointer val=0;
					if(OVar->DefaultMember)
						val=OVar->DefaultMember->Dobject;

					(*pArray)->Add(val);
					count = (*pArray)->GetCount();

					ti = AddEditArrayItem(root,OVar,count,treeItemID,true);	// vytvori polozku stromu (index, komentar)
					AddObjectVar(ko,OVar,ti,count);
				}
			}
			break;
		case eKTarrName :
			{
				CKerArrName** pArray = (CKerArrName**)GetAccessToVar(ko,OVar);
				if(*pArray && (*pArray)->CheckIntegrity())
				{
					int count = 0;
					CKerName* val=0;
					if(OVar->DefaultMember)
						val=OVar->DefaultMember->Dname;

					(*pArray)->Add(val);
					count = (*pArray)->GetCount();

					ti = AddEditArrayItem(root,OVar,count,treeItemID,true);	// vytvori polozku stromu (index, komentar)
					AddKerNameVar(ko,OVar,ti,count);
				}
			}
			break;
	}

	UpdateTree();

}

void CEDObjectProperty::CreateOrDeleteArrayVar(CGUITreeItem* root, CKerOVar* OVar, CGUITreeItem* treeItem, CGUICheckBox* chBox)
{
	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;

	CGUITreeItem* ti=0;

	switch(OVar->Type)
	{
		case eKTarrChar :
			{
				CKerArrChar** pArray = (CKerArrChar**)GetAccessToVar(ko,OVar);

				if(!(*pArray) || !(*pArray)->CheckIntegrity())
				{
					*pArray = new CKerArrChar();
					chBox->ChangeState(1);
				}
				else
				{
					ti=root->items->GetPrev(treeItem);
					while(ti)
					{
						root->DeleteTreeItem(ti);
						ti=root->items->GetPrev(treeItem);
					}
					if((*pArray) && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray=0;
					chBox->ChangeState(0);
				}
			}
			break;
		case eKTarrDouble :
			{
				CKerArrDouble** pArray = (CKerArrDouble**)GetAccessToVar(ko,OVar);
				if(!(*pArray) || !(*pArray)->CheckIntegrity())
				{
					*pArray = new CKerArrDouble();
					chBox->ChangeState(1);
				}
				else
				{
					ti=root->items->GetPrev(treeItem);
					while(ti)
					{
						root->DeleteTreeItem(ti);
						ti=root->items->GetPrev(treeItem);
					}
					if((*pArray) && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray=0;
					chBox->ChangeState(0);
				}
			}
			break;
		case eKTarrInt :
			{
				CKerArrInt** pArray = (CKerArrInt**)GetAccessToVar(ko,OVar);
				if(!(*pArray) || !(*pArray)->CheckIntegrity())
				{
					*pArray = new CKerArrInt();
					chBox->ChangeState(1);
				}
				else
				{
					ti=root->items->GetPrev(treeItem);
					while(ti)
					{
						root->DeleteTreeItem(ti);
						ti=root->items->GetPrev(treeItem);
					}
					if((*pArray) && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray=0;
					chBox->ChangeState(0);
				}
			}
			break;
		case eKTarrObject :
			{
				CKerArrObject** pArray = (CKerArrObject**)GetAccessToVar(ko,OVar);
				if(!(*pArray) || !(*pArray)->CheckIntegrity())
				{
					*pArray = new CKerArrObject();
					chBox->ChangeState(1);
				}
				else
				{
					ti=root->items->GetPrev(treeItem);
					while(ti)
					{
						root->DeleteTreeItem(ti);
						ti=root->items->GetPrev(treeItem);
					}
					if((*pArray) && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray=0;
					chBox->ChangeState(0);
				}
			}
			break;
		case eKTarrName :
			{
				CKerArrName** pArray = (CKerArrName**)GetAccessToVar(ko,OVar);
				if(!(*pArray) || !(*pArray)->CheckIntegrity())
				{
					*pArray = new CKerArrName();
					chBox->ChangeState(1);
				}
				else
				{
					ti=root->items->GetPrev(treeItem);
					while(ti)
					{
						root->DeleteTreeItem(ti);
						ti=root->items->GetPrev(treeItem);
					}
					if((*pArray) && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray=0;
					chBox->ChangeState(0);
				}
			}
			break;
	}

	UpdateTree();
}


void CEDObjectProperty::DeleteLastArrayVarItemProperty(CGUITreeItem* root, CKerOVar* OVar, CGUITreeItem* treeItem)
{
	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;


	CGUITreeItem* ti=0;

	switch(OVar->Type)
	{
		case eKTarrChar :
			{
				CKerArrChar** pArray = (CKerArrChar**)GetAccessToVar(ko,OVar);
				if(!(*pArray && (*pArray)->CheckIntegrity()))
				{
					*pArray = 0;
				}
				else
				{
					int count = (*pArray)->GetCount();
					if(count>0)
						(*pArray)->SetCount(count-1);

					root->DeleteTreeItem(root->items->GetPrev(treeItem));
				}
			}
			break;
		case eKTarrDouble :
			{
				CKerArrDouble** pArray = (CKerArrDouble**)GetAccessToVar(ko,OVar);
				if(!(*pArray && (*pArray)->CheckIntegrity()))
				{
					*pArray = 0;
				}
				else
				{
					int count = (*pArray)->GetCount();
					if(count>0)
						(*pArray)->SetCount(count-1);

					root->DeleteTreeItem(root->items->GetPrev(treeItem));
				}
			}
			break;
		case eKTarrInt :
			{
				CKerArrInt** pArray = (CKerArrInt**)GetAccessToVar(ko,OVar);
				if(!(*pArray && (*pArray)->CheckIntegrity()))
				{
					*pArray = 0;
				}
				else
				{
					int count = (*pArray)->GetCount();
					if(count>0)
						(*pArray)->SetCount(count-1);

					root->DeleteTreeItem(root->items->GetPrev(treeItem));
				}
			}
			break;
		case eKTarrObject :
			{
				CKerArrObject** pArray = (CKerArrObject**)GetAccessToVar(ko,OVar);
				if(!(*pArray && (*pArray)->CheckIntegrity()))
				{
					*pArray = 0;
				}
				else
				{
					int count = (*pArray)->GetCount();
					if(count>0)
						(*pArray)->SetCount(count-1);

					root->DeleteTreeItem(root->items->GetPrev(treeItem));
				}
			}
			break;
		case eKTarrName :
			{
				CKerArrName** pArray = (CKerArrName**)GetAccessToVar(ko,OVar);
				if(!(*pArray && (*pArray)->CheckIntegrity()))
				{
					*pArray = 0;
				}
				else
				{
					int count = (*pArray)->GetCount();
					if(count>0)
						(*pArray)->SetCount(count-1);

					root->DeleteTreeItem(root->items->GetPrev(treeItem));
				}
			}
			break;
	}

	UpdateTree();
}


CGUIEditWindow* CEDObjectProperty::GetEditWindow(CGUITreeItem* ti)
{
	if(!ti || !ti->elem)
		return 0;

	CGUIEditWindow* ew = dynamic_cast<CGUIEditWindow*>(ti->elem);
	if(ew)
		return ew;

	CGUIWindow* w = dynamic_cast<CGUIWindow*>(ti->elem);
	if(w)
	{
		CGUIElement* el=0;
		while(el=w->GetNextBackElement(el))
		{
			if(ew = dynamic_cast<CGUIEditWindow*>(el))
				return ew;
		}
	}
	return 0;
}

void CEDObjectProperty::SelectLocationEditModByOVar(CGUITreeItem* groupItem, CKerOVar* OVar, bool on)
{
	CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
	if(!on)
	//if(editor->GetEditMod() == eEMselectLocation && groupItem==editedGroupItem)
	{
		if(lw)
			lw->DeleteMarker();

		/*
		if(editor->GetLastEditMod()!=eEMselectLocation)
			editor->SetEditMod(editor->GetLastEditMod());
		else
			editor->SetEditMod(eEMnormal);
			*/

		editor->SetEditMod(beforeLocationEditMod);

		editedGroupItem=0;
	}
	else
	{
		editedGroupItem=groupItem;

		if(editor->GetEditMod()!=eEMselectLocation)
			beforeLocationEditMod=editor->GetEditMod();

		editor->SetEditMod(eEMselectLocation);
		//editor->SetFreeMod(1);

		if(lw)
			lw->DeleteMarker();

		if(OVar->EditType == eKET3Dpoint || OVar->EditType == eKET2Dpoint)
		{
			int lx,ly,lz;
			GetLocationVar(lx,ly,lz);
			editor->SetSelectLocationMod(0);
			if(lw)
				lw->ShowPointMarker(lx,ly,lz);
		}
		else if(OVar->EditType == eKET3Dcell || OVar->EditType == eKET2Dcell)
		{
			int lx,ly,lz;
			GetLocationVar(lx,ly,lz);
			editor->SetSelectLocationMod(1);
			if(lw)
				lw->ShowCellMarker(lx,ly,lz);
		}
		else if(OVar->EditType == eKET3Darea || OVar->EditType == eKET2Darea)
		{
			int x1,y1,z1,x2,y2,z2;
			GetLocationAreaVar(x1,y1,z1,x2,y2,z2);
			editor->SetSelectLocationMod(2);
			if(lw)
				lw->ShowAreaMarker(x1,y1,z1,x2,y2,z2);
		}
		else if(OVar->EditType == eKET3DcellArea || OVar->EditType == eKET2DcellArea)
		{
			int x1,y1,z1,x2,y2,z2;
			GetLocationAreaVar(x1,y1,z1,x2,y2,z2);
			editor->SetSelectLocationMod(3);
			if(lw)
				lw->ShowCellAreaMarker(x1,y1,z1,x2,y2,z2);
		}
	}
}

void CEDObjectProperty::RefreshLocation(CGUITreeItem* groupItem)
{
	CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
	if(editor->GetEditMod() == eEMselectLocation && groupItem==editedGroupItem && editedGroupItem && (editedGroupItem->userTagType==eUTOVar || editedGroupItem->userTagType==eUTOVarDEL) && editedGroupItem->ut.OVar)
	{
		CKerOVar* OVar = editedGroupItem->ut.OVar;

		if(OVar->EditType == eKET3Dpoint || OVar->EditType == eKET2Dpoint)
		{
			int lx,ly,lz;
			GetLocationVar(lx,ly,lz);
			if(lw)
				lw->ShowPointMarker(lx,ly,lz);
		}
		else if(OVar->EditType == eKET3Dcell || OVar->EditType == eKET2Dcell)
		{
			int lx,ly,lz;
			GetLocationVar(lx,ly,lz);
			if(lw)
				lw->ShowCellMarker(lx,ly,lz);
		}
		else if(OVar->EditType == eKET3Darea || OVar->EditType == eKET2Darea)
		{
			int x1,y1,z1,x2,y2,z2;
			int xx1,yy1,zz1,xx2,yy2,zz2;
			GetLocationAreaVar(xx1,yy1,zz1,xx2,yy2,zz2);

			x1 = min(xx1,xx2);
			x2 = max(xx1,xx2);
			y1 = min(yy1,yy2);
			y2 = max(yy1,yy2);
			z1 = min(zz1,zz2);
			z2 = max(zz1,zz2);

			SaveLocationVar(1,x1,y1,z1);
			SaveLocationVar(2,x2,y2,z2);

			if(lw)
				lw->ShowAreaMarker(x1,y1,z1,x2,y2,z2);
		}
		else if(OVar->EditType == eKET3DcellArea || OVar->EditType == eKET2DcellArea)
		{
			int x1,y1,z1,x2,y2,z2;
			int xx1,yy1,zz1,xx2,yy2,zz2;
			GetLocationAreaVar(xx1,yy1,zz1,xx2,yy2,zz2);

			x1 = min(xx1,xx2);
			x2 = max(xx1,xx2);
			y1 = min(yy1,yy2);
			y2 = max(yy1,yy2);
			z1 = min(zz1,zz2);
			z2 = max(zz1,zz2);

			SaveLocationVar(1,x1,y1,z1);
			SaveLocationVar(2,x2,y2,z2);

			if(lw)
				lw->ShowCellAreaMarker(x1,y1,z1,x2,y2,z2);
		}
	}
}

void CEDObjectProperty::ClearLocation(CGUITreeItem* groupItem)
{
	if(groupItem && (groupItem->userTagType==eUTOVar || groupItem->userTagType==eUTOVarDEL) && groupItem->ut.OVar)
	{
		CKerOVar* OVar = groupItem->ut.OVar;

		CGUITreeItem* ti = groupItem->items->GetFromHead();
		while(ti)
		{
			SaveValue(0,ti);
			ti = groupItem->items->GetNext(ti);
		}
		UpdateAllVars(editedGroupItem);
	}
}

void CEDObjectProperty::SaveLocationVar(int type, int lx, int ly, int lz)
{
	if(editedGroupItem && (editedGroupItem->userTagType==eUTOVar||editedGroupItem->userTagType==eUTOVarDEL) && editedGroupItem->ut.OVar)
	{
		CKerOVar* OVar = editedGroupItem->ut.OVar;
		if(type==0)
		{
			if(OVar->EditType == eKET3Dpoint || OVar->EditType == eKET3Dcell)
			{
				CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
				SaveValue(lx,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(ly,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(lz,ti);
			}
			else if(OVar->EditType == eKET2Dpoint || OVar->EditType == eKET2Dcell)
			{
				CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
				SaveValue(lx,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(ly,ti);
			}
		}
		else if(type==1)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			SaveValue(lx,ti);
			ti = editedGroupItem->items->GetNext(ti);
			SaveValue(ly,ti);
			if(OVar->EditType == eKET3Darea || OVar->EditType == eKET3DcellArea)
			{
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(lz,ti);
			}
		}
		else if(type==2)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			if(OVar->EditType == eKET2Darea || OVar->EditType == eKET2DcellArea)
			{
				ti = editedGroupItem->items->GetNext(ti);
				ti = editedGroupItem->items->GetNext(ti);
		
				SaveValue(lx,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(ly,ti);
			}
			else if(OVar->EditType == eKET3Darea || OVar->EditType == eKET3DcellArea)
			{
				ti = editedGroupItem->items->GetNext(ti);
				ti = editedGroupItem->items->GetNext(ti);
				ti = editedGroupItem->items->GetNext(ti);

				SaveValue(lx,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(ly,ti);
				ti = editedGroupItem->items->GetNext(ti);
				SaveValue(lz,ti);
			}
		}

		UpdateAllVars(editedGroupItem);
	}
}

int CEDObjectProperty::GetLocationVar(int &lx, int &ly, int &lz)
{
	if(editedGroupItem && (editedGroupItem->userTagType==eUTOVar||editedGroupItem->userTagType==eUTOVarDEL) && editedGroupItem->ut.OVar)
	{
		CKerOVar* OVar = editedGroupItem->ut.OVar;
		if(OVar->EditType == eKET3Dpoint || OVar->EditType == eKET2Dpoint)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			GetValue(lx,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(ly,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(lz,ti);
			return 0;
		}
		else if(OVar->EditType == eKET3Dcell || OVar->EditType == eKET2Dcell)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			GetValue(lx,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(ly,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(lz,ti);
			return 1;
		}
		else if(OVar->EditType == eKET3Darea || OVar->EditType == eKET2Darea) 
		{
			lx=ly=lz=0;
			return 2;
		}else if(OVar->EditType == eKET3DcellArea || OVar->EditType == eKET2DcellArea)
		{
			lx=ly=lz=0;
			return 3;
		}
	}
	return -1;
}

void CEDObjectProperty::GetLocationAreaVar(int &x1, int &y1, int &z1, int &x2, int &y2, int &z2)
{
	if(editedGroupItem && (editedGroupItem->userTagType==eUTOVar||editedGroupItem->userTagType==eUTOVarDEL) && editedGroupItem->ut.OVar)
	{
		CKerOVar* OVar = editedGroupItem->ut.OVar;
		if(OVar->EditType == eKET3Darea || OVar->EditType == eKET3DcellArea)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			GetValue(x1,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(y1,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(z1,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(x2,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(y2,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(z2,ti);
		}
		else if(OVar->EditType == eKET2Darea || OVar->EditType == eKET2DcellArea)
		{
			CGUITreeItem* ti = editedGroupItem->items->GetFromHead();
			GetValue(x1,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(y1,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(x2,ti);
			ti = editedGroupItem->items->GetNext(ti);
			GetValue(y2,ti);
			z1=z2=0;
		}
	}
}

int CEDObjectProperty::GetValue(int &value, CGUITreeItem* ti)
{
	value=0;

	if(!KerMain)
		return 1;

	if(!ti)
		return 1;

	if(ti->userTagType!=eUTOVar && ti->userTagType!=eUTOVarDEL)
		return 1;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko)
		return 1;


	CKerOVar* OVar;
	OVar =  ti->ut.OVar;

	if(!OVar || OVar->Type != eKTint)
		return 1;

	value = *(int*)GetAccessToVar(ko,OVar);	// jedna se o jeden int

	return 0;
}

int CEDObjectProperty::SaveValue(int value, CGUITreeItem* ti)
{
	if(!KerMain)
		return 1;

	if(!ti)
		return 1;

	if(ti->userTagType!=eUTOVar && ti->userTagType!=eUTOVarDEL)
		return 1;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko)
		return 1;


	CKerOVar* OVar;
	OVar =  ti->ut.OVar;


	if(!OVar || OVar->Type != eKTint)
		return 1;


	int* ovInt = (int*)GetAccessToVar(ko,OVar);	// jedna se o jeden int
	*ovInt = value;

	return 0;
}


// ---------------------------------------------------------------------------------------------
// Zmena hodnoty promenych ruznych typu
int CEDObjectProperty::SaveIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value, int arrIndex)
{
	int ret=1;
	int pInt=0;
	int *ovInt=0;

	int valueType=OVar->EditType;	// !@#$ tag rozhodujici jaka hodnota se ma editovat (int/bool)

	if(valueType==eKETdefault || valueType==eKETnumeric)	// int
	{
		if(value)
		{
			pInt = *(int*)value;
		}
		else
		{
			CGUIEditWindow* ed=GetEditWindow(ti); 
			ret = ed->GetParametr(&pInt,dtInt,1);
			if(ret!=0)	// GetParametr se nepovedl (spatny typ, velikost atp.)
				return 0;
		}
	}
	else if(valueType==eKETboolean)	// bool
	{
		CGUICheckBox* cb = dynamic_cast<CGUICheckBox*>(ti->elem);
		if(cb)
			cb->GetParametr(&pInt,dtInt,1);
	}

	if(OVar->LimintsCfg!=eKLCnone)
	{
		if(OVar->LimintsCfg==eKLCinterval)	// interval
		{
			if(OVar->LimitsListCount>=2)
			{
				int min,max;
				min = OVar->LimitsList[0].Dint;
				max = OVar->LimitsList[1].Dint;
				if(min>max)
					std::swap<int>(min,max);

				if(OVar->Use & eKVUBexclusive)
				{
					if(pInt<=min || pInt>=max)
						return 0;
				}
				else
				{
					if(pInt<min || pInt>max)
						return 0;
				}
			}
		}
		else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
		{
			ret=0;
			for(int i=0;i<OVar->LimitsListCount;i++)
			{
				if(OVar->LimitsList[i].Dint==pInt)
				{
					ret=1;
					break;
				}
			}
			if(!ret)
				return 0;
		}
		else if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jednostranny interval
		{
			if(OVar->LimitsListCount>=1)
			{
				int limit = OVar->LimitsList[0].Dint;

				if(OVar->LimintsCfg==eKLCup)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pInt<=limit)
							return 0;
					}
					else
					{
						if(pInt<limit)
							return 0;
					}
				}
				if(OVar->LimintsCfg==eKLCdown)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pInt>=limit)
							return 0;
					}
					else
					{
						if(pInt>limit)
							return 0;
					}
				}
			}
		}
	}


	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrInt* pArray = *(CKerArrInt**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Get(arrIndex-1) = pInt;
		}
	}
	else
	{
		ovInt = (int*)GetAccessToVar(ko,OVar);	// jedna se o jeden int
		*ovInt = pInt;
	}
	
	return 1;
}

int CEDObjectProperty::SaveCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value, int arrIndex)
{
	int ret=1;
	int pChar=0;
	unsigned char *ovChar=0;

	int valueType=OVar->EditType;	// !@#$ tag rozhodujici jaka hodnota se ma editovat (int/bool)

	if(valueType==eKETdefault || valueType==eKETcharacter)	// char
	{
		if(value)
		{
			pChar = *(unsigned char*)value;
		}
		else
		{
			CGUIEditWindow* ed=GetEditWindow(ti); 
			ret = ed->GetParametr(&pChar,dtChar,1);
			if(ret!=0)	// GetParametr se nepovedl (spatny typ, velikost atp.)
				return 0;
		}
	}
	else if(valueType==eKETnumeric)	// int
	{
		if(value)
		{
			pChar = *(unsigned char*)value;
		}
		else
		{
			CGUIEditWindow* ed=GetEditWindow(ti); 
			ret = ed->GetParametr(&pChar,dtInt,1);
			if(ret!=0)	// GetParametr se nepovedl (spatny typ, velikost atp.)
				return 0;
		}
	}
	else if(valueType==eKETboolean)	// bool
	{
		CGUICheckBox* cb = dynamic_cast<CGUICheckBox*>(ti->elem);
		if(cb)
			cb->GetParametr(&pChar,dtInt,1);
	}

	if(OVar->LimintsCfg!=eKLCnone)
	{
		if(OVar->LimintsCfg==eKLCinterval)	// interval
		{
			if(OVar->LimitsListCount>=2)
			{
				int min,max;
				min = OVar->LimitsList[0].Dchar;
				max = OVar->LimitsList[1].Dchar;
				if(min>max)
					std::swap<int>(min,max);

				if(OVar->Use & eKVUBexclusive)
				{
					if(pChar<=min || pChar>=max)
						return 0;
				}
				else
				{
					if(pChar<min || pChar>max)
						return 0;
				}
			}
		}
		else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
		{
			ret=0;
			for(int i=0;i<OVar->LimitsListCount;i++)
			{
				if(OVar->LimitsList[i].Dchar==pChar)
				{
					ret=1;
					break;
				}
			}
			if(!ret)
				return 0;
		}
		else if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jednostranny interval
		{
			if(OVar->LimitsListCount>=1)
			{
				char limit = OVar->LimitsList[0].Dchar;

				if(OVar->LimintsCfg==eKLCup)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pChar<=limit)
							return 0;
					}
					else
					{
						if(pChar<limit)
							return 0;
					}
				}
				if(OVar->LimintsCfg==eKLCdown)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pChar>=limit)
							return 0;
					}
					else
					{
						if(pChar>limit)
							return 0;
					}
				}
			}
		}
	}

	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrChar* pArray = *(CKerArrChar**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Get(arrIndex-1) = (unsigned char)pChar;
		}
	}
	else
	{
		ovChar = (unsigned char*)GetAccessToVar(ko,OVar);
		*ovChar = (unsigned char)pChar;
	}

	return 1;
}

int CEDObjectProperty::SaveDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value, int arrIndex)
{
	int ret=1;
	double pDouble=0;
	double *ovDouble=0;

	if(value)
	{
		pDouble = *(double*)value;
	}
	else
	{
		CGUIEditWindow* ed=GetEditWindow(ti); 
		ret = ed->GetParametr(&pDouble,dtDouble,1);
		if(ret!=0)	// GetParametr se nepovedl (spatny typ, velikost atp.)
			return 0;
	}

	if(OVar->LimintsCfg!=eKLCnone)
	{
		if(OVar->LimintsCfg==eKLCinterval)	// interval
		{
			if(OVar->LimitsListCount>=2)
			{
				double min,max;
				min = OVar->LimitsList[0].Ddouble;
				max = OVar->LimitsList[1].Ddouble;
				if(min>max)
					std::swap<double>(min,max);

				if(OVar->Use & eKVUBexclusive)
				{
					if(pDouble<=min || pDouble>=max)
						return 0;
				}
				else
				{
					if(pDouble<min || pDouble>max)
						return 0;
				}
			}
		}
		else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
		{
			ret=0;
			for(int i=0;i<OVar->LimitsListCount;i++)
			{
				if(OVar->LimitsList[i].Ddouble==pDouble)
				{
					ret=1;
					break;
				}
			}
			if(!ret)
				return 0;
		}
		else if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jednostranny interval
		{
			if(OVar->LimitsListCount>=1)
			{
				double limit = OVar->LimitsList[0].Ddouble;

				if(OVar->LimintsCfg==eKLCup)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pDouble<=limit)
							return 0;
					}
					else
					{
						if(pDouble<limit)
							return 0;
					}
				}
				if(OVar->LimintsCfg==eKLCdown)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(pDouble>=limit)
							return 0;
					}
					else
					{
						if(pDouble>limit)
							return 0;
					}
				}
			}
		}
	}


	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrDouble* pArray = *(CKerArrDouble**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Get(arrIndex-1) = pDouble;
		}
	}
	else
	{
		ovDouble = (double*)GetAccessToVar(ko,OVar);	// jedna se o jeden int
		*ovDouble = pDouble;
	}

	
	return 1;
}

int CEDObjectProperty::SaveStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value)
{
	int ret=1;
	char* pString=0;
	char buff[301];
	char *ovString=0;

	if(value)
	{
		pString = (char*)value;
	}
	else
	{
		CGUIEditWindow* ed=GetEditWindow(ti); 
		ret = ed->GetParametr(buff,dtString,300);
		pString=buff;
		if(ret!=0)	// GetParametr se nepovedl (spatny typ, velikost atp.)
			return 0;
	}

	if(OVar->LimintsCfg!=eKLCnone)
	{
		if(OVar->LimintsCfg==eKLCinterval)	// interval
		{
			if(OVar->LimitsListCount>=2)
			{
				char *min,*max;
				min = OVar->LimitsList[0].Dstring;
				max = OVar->LimitsList[1].Dstring;
				if(strcmp(min,max)>0)
					std::swap<char*>(min,max);

				if(OVar->Use & eKVUBexclusive)
				{
					if(strcmp(pString,min)<=0 || strcmp(pString,max)>=0)
						return 0;
				}
				else
				{
					if(strcmp(pString,min)<0 || strcmp(pString,max)>0)
						return 0;
				}
			}
		}
		else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
		{
			ret=0;
			for(int i=0;i<OVar->LimitsListCount;i++)
			{
				if(!strcmp(OVar->LimitsList[i].Dstring,pString))
				{
					ret=1;
					break;
				}
			}
			if(!ret)
				return 0;
		}
		else if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jednostranny interval
		{
			if(OVar->LimitsListCount>=1)
			{
				char* limit = OVar->LimitsList[0].Dstring;

				if(OVar->LimintsCfg==eKLCup)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(strcmp(pString,limit)<=0)
							return 0;
					}
					else
					{
						if(strcmp(pString,limit)<0)
							return 0;
					}
				}
				if(OVar->LimintsCfg==eKLCdown)
				{
					if(OVar->Use & eKVUBexclusive)
					{
						if(strcmp(pString,limit)>=0)
							return 0;
					}
					else
					{
						if(strcmp(pString,limit)>0)
							return 0;
					}
				}
			}
		}
	}

	int count = OVar->Type - eKTstring;

	ovString = (char*)GetAccessToVar(ko,OVar);
	strncpy(ovString,pString,count);
	ovString[count]=0;
	return 1;
}

int CEDObjectProperty::SaveKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value, int arrIndex)
{
	int ret=1;
	CKerName* pKerName = (CKerName*)value;
	CKerName** ovKerName=0;

	if(KerMain->KerNamesMain->TestPointerValidity(pKerName))
	{
		if(OVar->LimintsCfg!=eKLCnone)
		{
			// vybrat kontrolu podle toho zda jde o vycet hodnot CKerName nebo vymezujici mnozinu - jedno CKerName + smer + ostrost

			if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jmeno urcujici mnozinu
			{
				// mnozina urcena pomoci jednoho CKerName + smeru (nahoru/dolu + ostre/neostre)
				
				int direction = 1;
				if(OVar->LimintsCfg==eKLCup)
					direction=0;

				int type = -1;	
				if(OVar->EditType == eKETautomaticGr)
					type = eKerNTauto;

				CEDObjPropertyList::nameMask = OVar->NamesMask;
				CEDObjPropertyList::eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;;
				CEDObjPropertyList::eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
				CKerNameList* knl;
				if(OVar->LimitsListCount>0)
					knl = KerMain->KerNamesMain->FindSet(OVar->LimitsList[0].Dname,&CEDObjPropertyList::NameFilter,type,direction);
				CKerNameList* kn =  knl;

				ret=0;
				while(knl)
				{
					if(knl->name==pKerName)
					{
						ret=1;
						break;
					}else
						knl=knl->next;
				}
				if(kn)
					kn->DeleteAll();

				if(!(OVar->Use & eKVUBexclusive) && !ret && (OVar->LimitsListCount>0))
				{
					if(pKerName==OVar->LimitsList[0].Dname)
						ret=1;
				}

				if(OVar->EditType == eKETautomaticGr && !ret)
				{
					if(pKerName==KnownNames[eKKNdefaultAuto].Name)
						ret=1;
				}
				if(!ret)
					return 0;
			}
			else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
			{
				ret=0;
				for(int i=0;i<OVar->LimitsListCount;i++)
				{
					if(OVar->LimitsList[i].Dname==pKerName)
					{
						ret=1;
						break;
					}
				}
				if(OVar->EditType == eKETautomaticGr && !ret)
				{
					if(pKerName==KnownNames[eKKNdefaultAuto].Name)
						ret=1;
				}
				if(!ret)
					return 0;
			}
		}
		else
		{	// zadna vymezujici mnozina neni zadana, ale stale muzu filtrovat pomoci masky jmen
			int type = -1;	
			CKerName* from = 0;

			if(OVar->EditType == eKETautomaticGr)
			{
				type = eKerNTauto;
				from = ko->Type->AName;
			}

			CEDObjPropertyList::nameMask = OVar->NamesMask;
			CEDObjPropertyList::eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;;
			CEDObjPropertyList::eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
			CKerNameList* knl = knl = KerMain->KerNamesMain->FindSet(from,&CEDObjPropertyList::NameFilter,type,1);
			CKerNameList* kn =  knl;

			ret=0;
			while(knl)
			{
				if(knl->name==pKerName)
				{
					ret=1;
					break;
				}else
					knl=knl->next;
			}
			if(kn)
				kn->DeleteAll();

			if(OVar->EditType == eKETautomaticGr && !ret)
			{
				if(pKerName==KnownNames[eKKNdefaultAuto].Name)
					ret=1;
			}
			if(!ret)
				return 0;
		}

		if(pKerName->Type == eKerNTobject)
		{
			if(pKerName->ObjectType)
			{
				if((OVar->Use & eKVUBobjOutMap) && !((pKerName->ObjectType->EditTag) & etbOutMap))
					return 0;	// vybrany objektove jmeno neodpovida tagum - InMap/OutMap
				if((OVar->Use & eKVUBobjInMap) && !((pKerName->ObjectType->EditTag) & etbInMap))
					return 0;	// vybrany objektove jmeno neodpovida tagum - InMap/OutMap
			}
			else
				return 0;
		}

	}
	else
	{
		pKerName=0;
		if(!(OVar->Use & eKVUBincludeNull))
			return 0;	// nelze ulozit nulovou hodnotu
	}

	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrName* pArray = *(CKerArrName**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Get(arrIndex-1) = pKerName;
		}
	}
	else
	{
		ovKerName = (CKerName**)GetAccessToVar(ko,OVar);
		*ovKerName = pKerName;
	}

	return 1;
}


int CEDObjectProperty::SaveObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, void* value, int arrIndex)
{
	int ret=1;

	if(!value)
		return 0;

	OPointer pOPointer = *(OPointer*)value;
	OPointer* ovOPointer=0;

	if(CKerObject* objKo = KerMain->Objs->GetObject(pOPointer))
	{
		if(OVar->LimintsCfg!=eKLCnone)
		{
			// vybrat kontrolu podle toho zda jde o vycet hodnot CKerName nebo vymezujici mnozinu - jedno CKerName

			if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// jmeno urcujici mnozinu
			{
				// mnozina urcena pomoci jednoho CKerName + smeru (nahoru/dolu + ostre/neostre)

				int direction = 1;
				if(OVar->LimintsCfg==eKLCup)
					direction=0;

				CKerNameList* knl;
				if(OVar->LimitsListCount>0)
					knl = KerMain->KerNamesMain->FindSet(OVar->LimitsList[0].Dname,&CEDObjPropertyList::FindNameObject,eKerNTobject,direction);
				CKerNameList* kn =  knl;

				ret=0;
				while(knl)
				{
					if(knl->name==objKo->Type->Name)
					{
						ret=1;
						break;
					}else
						knl=knl->next;
				}
				if(kn)
					kn->DeleteAll();

				if(!(OVar->Use & eKVUBexclusive) && !ret && (OVar->LimitsListCount>0))
				{
					if(objKo->Type->Name==OVar->LimitsList[0].Dname)
						ret=1;
				}
			}
			else if(OVar->LimintsCfg==eKLClist)	// vycet hodnot
			{
				ret=0;
				for(int i=0;i<OVar->LimitsListCount;i++)
				{
					if(OVar->LimitsList[i].Dname==objKo->Type->Name)
					{
						ret=1;
						break;
					}
				}
				if(!ret)
					return 0;
			}
		}

		if((OVar->Use & eKVUBobjInMap) && !(objKo->Type->EditTag & etbInMap))
			return 0;	// vybrany objekt neodpovida tagum - InMap/OutMap

		if((OVar->Use & eKVUBobjOutMap) && !(objKo->Type->EditTag & etbOutMap))
			return 0;	// vybrany objekt neodpovida tagum - InMap/OutMap
	}
	else
	{
		pOPointer=0;	// objekt neexistuje
		if(!(OVar->Use & eKVUBincludeNull))
			return 0;	// nelze ulozit nulovou hodnotu
	}


	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrObject* pArray = *(CKerArrObject**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Get(arrIndex-1) = pOPointer;
		}
	}
	else
	{
		ovOPointer = (OPointer*)GetAccessToVar(ko,OVar);
		*ovOPointer = pOPointer;
	}

	return 1;
}


int CEDObjectProperty::VarValueChanged(CGUITreeItem* ti, void* value)
{
	int ret=1;

	if(!KerMain)
		return 1;

	if(!ti)
		return 1;

	if(ti->userTagType!=eUTOVar && ti->userTagType!=eUTOVarDEL && ti->userTagType!=eUTint)
		return 1;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return 1;


	CKerOVar* OVar;
	int arrIndex=0;
	if(ti->userTagType==eUTOVar || ti->userTagType==eUTOVarDEL)
	{
		OVar =  ti->ut.OVar;
	}
	else
	{
		if(ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
		{
			CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
			if(aid)
			{
				OVar = aid->OVar;
				arrIndex=ti->ut.pInt;
			}
			else
				return 1;
		}
	}

	if(!OVar)
		return 1;
	
	if(OVar->Type>=eKTstring && OVar->Type<=eKTstring+250)
	{
		ret=SaveStringVar(ko,OVar,ti,value);
		ShowStringVar(ko,OVar,ti);
	}
	else
		switch(OVar->Type)
		{
			case eKTchar :
				ret=SaveCharVar(ko,OVar,ti,value);
				if(ret)
					ShowCharVar(ko,OVar,ti);
				break;
			case eKTdouble :
				ret=SaveDoubleVar(ko,OVar,ti,value);
				if(ret)
					ShowDoubleVar(ko,OVar,ti);
				break;
			case eKTint :
				ret=SaveIntVar(ko,OVar,ti,value);
				if(ret)
					ShowIntVar(ko,OVar,ti);
				break;
			case eKTobject :
				ret=SaveObjectVar(ko,OVar,ti,value);
				if(ret)
					ShowObjectVar(ko,OVar,ti);
				break;
			case eKTname :
				ret=SaveKerNameVar(ko,OVar,ti,value);
				if(ret)
					ShowKerNameVar(ko,OVar,ti);
				break;
			case eKTarrChar :
				ret=SaveCharVar(ko,OVar,ti,value,arrIndex);
				if(ret)
					ShowCharVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrDouble :
				ret=SaveDoubleVar(ko,OVar,ti,value,arrIndex);
				if(ret)
					ShowDoubleVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrInt :
				ret=SaveIntVar(ko,OVar,ti,value,arrIndex);
				if(ret)
					ShowIntVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrObject :
				ret=SaveObjectVar(ko,OVar,ti,value,arrIndex);
				if(ret)
					ShowObjectVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrName :
				ret=SaveKerNameVar(ko,OVar,ti,value,arrIndex);
				if(ret)
					ShowKerNameVar(ko,OVar,ti,arrIndex);
				break;
		}

	if(ret && ((OVar->Use & eKVUBauto) || ((OVar->Use >> KERVARUSESSHIFT)) == eKVUaPicture))
	{
		RefreshObject();
		//KerServices.ResetAuto(obj);
		//PrepareObjCursor();
	}

	return ret;
}

void CEDObjectProperty::ShowIntVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	int valueType=OVar->EditType;	// tag rozhodujici jaka hodnota se ma editovat (int/bool)
	
	int pInt=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrInt* pArray = *(CKerArrInt**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pInt = pArray->Read(arrIndex-1);
		}
	}
	else
		pInt = *(int*)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	if(valueType==eKETdefault || valueType==eKETnumeric)	// int
	{
		CGUIEditWindow* ed=GetEditWindow(ti); 
		if(ed)
		{
			ed->SetParametr(&pInt,dtInt,1);
			ed->changed=0;
		}
	}
	else if(valueType==eKETboolean)	// bool
	{
		CGUICheckBox* cb = dynamic_cast<CGUICheckBox*>(ti->elem);
		if(cb)
		{
			cb->SetParametr(&pInt,dtInt,1);
		}
	}
}

void CEDObjectProperty::ShowCharVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	int valueType=OVar->EditType;	// tag rozhodujici jaka hodnota se ma editovat (int/bool)

	int pInt=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrChar* pArray = *(CKerArrChar**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pInt = (unsigned char)pArray->Read(arrIndex-1);
		}
	}
	else
		pInt = *(unsigned char*)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	if(valueType==eKETdefault || valueType==eKETcharacter)	// char
	{
		CGUIEditWindow* ed=GetEditWindow(ti); 
		if(ed)
		{
			ed->SetParametr(&pInt,dtChar,1);
			ed->changed=0;
		}
	}
	else if(valueType==eKETnumeric)	// int
	{
		CGUIEditWindow* ed=GetEditWindow(ti); 
		if(ed)
		{
			ed->SetParametr(&pInt,dtInt,1);
			ed->changed=0;
		}
	}
	else if(valueType==eKETboolean)	// bool
	{
		CGUICheckBox* cb = dynamic_cast<CGUICheckBox*>(ti->elem);
		if(cb)
		{
			cb->SetParametr(&pInt,dtInt,1);
		}
	}

}

void CEDObjectProperty::ShowDoubleVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	double pDouble=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrDouble* pArray = *(CKerArrDouble**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pDouble = pArray->Read(arrIndex-1);
		}
	}
	else
		pDouble = *(double*)GetAccessToVar(ko,OVar);	// jedna se o jeden int

	CGUIEditWindow* ed=GetEditWindow(ti); 
	if(ed)
	{
		ed->SetParametr(&pDouble,dtDouble,1);
		ed->changed=0;
	}
}

void CEDObjectProperty::ShowStringVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti)
{
	CGUIEditWindow* ed=GetEditWindow(ti); 
	if(ed)
	{
		char* pString = (char*)GetAccessToVar(ko,OVar);
		if(pString)
			ed->SetParametr(pString,dtString,strlen(pString));
		else
		{
			char b=0;
			ed->SetParametr(&b,dtChar,1);
		}
		ed->changed=0;
	}
}


void CEDObjectProperty::ShowKerNameVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	CKerName* kn=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrName* pArray = *(CKerArrName**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			kn = pArray->Read(arrIndex-1);
		}
	}
	else
		kn = *(CKerName**)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	CGUIEditWindow* ed=GetEditWindow(ti); 
	if(ed)
	{
		if(kn)
		{
			if(KerMain->KerNamesMain->TestPointerValidity(kn))
			{
				char *name = kn->GetUserName();
				ed->SetParametr(name,dtString,strlen(name));
			}
			else
			{
				ed->SetParametr("X - Bad Name - X",dtString,20);
			}
		}else
			ed->SetParametr("0",dtString,2);

		ed->changed=0;
	}
}

void CEDObjectProperty::ShowObjectVar(CKerObject* ko, CKerOVar* OVar, CGUITreeItem* ti, int arrIndex)
{
	OPointer pObj=0;
	
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrObject* pArray = *(CKerArrObject**)GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pObj = pArray->Read(arrIndex-1);
		}
	}
	else
		pObj = *(OPointer*)GetAccessToVar(ko,OVar);	// jedna se o jeden int


	CGUIEditWindow* ed=GetEditWindow(ti); 
	if(ed)
	{
		if(CKerObject* k  = KerMain->Objs->GetObject(pObj))
		{
			char text[201];
			_snprintf(text,200,"%d - %s",pObj,k->Type->Name->GetUserName());
			ed->SetParametr(text,dtString,strlen(text));
		}
		else
		{
			if(pObj)
				ed->SetParametr("X - Bad Object - X",dtString,20);
			else
				ed->SetParametr("0",dtString,2);
		}

		ed->changed=0;
	}
}


void CEDObjectProperty::ShowObjValue(CGUITreeItem* ti)
{
	if(!KerMain)
		return;

	if(!ti || (ti->userTagType!=eUTOVar && ti->userTagType!=eUTOVarDEL && ti->userTagType!=eUTint && ti->userTagType!=eUTdataClass))
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;


	CKerOVar* OVar=0;
	int arrIndex=0;
	if(ti->userTagType==eUTOVar || ti->userTagType==eUTOVarDEL)
	{
		OVar =  ti->ut.OVar;
	}
	else if(ti->userTagType == eUTdataClass)
	{
			CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->ut.userDataClass);
			if(aid)
			{
				// jedna se o korenovou polozku reprezentujici kerneli pole
				// kerneli pole se mohlo zmenit (pridani / odebrani nekterych prvku, ci vytvoreni / zruseni celeho pole)
				// smaz vsechny podpolozky a vytvor je znovu podle nastaveni noveho pole

				// smazani starych polozek pole:
				CGUITreeItem* di = ti->items->GetFromHead();
				while(di)
				{
					ti->DeleteTreeItem(di);
					di = ti->items->GetFromHead();
				}

				OVar = aid->OVar;

				delete aid;
				ti->ut.userDataClass=0;


				// nove vytvoreni polozek pole:
				AddArrayVarProperty(ko,OVar,ti);
				UpdateTree();

				return;
			}
	}
	else
	{
		if(ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
		{
			CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
			if(aid)
			{
				OVar = aid->OVar;
				arrIndex=ti->ut.pInt;
			}
			else
				return;
		}
	}

	if(!OVar)
		return; 
	
	if(OVar->Type>=eKTstring && OVar->Type<=eKTstring+250)
		ShowStringVar(ko,OVar,ti);
	else
		switch(OVar->Type)
		{
			case eKTchar :
				ShowCharVar(ko,OVar,ti);
				break;
			case eKTdouble :
				ShowDoubleVar(ko,OVar,ti);
				break;
			case eKTint :
				ShowIntVar(ko,OVar,ti);
				break;
			case eKTobject :
				ShowObjectVar(ko,OVar,ti);
				break;
			case eKTname :
				ShowKerNameVar(ko,OVar,ti);
				break;
			case eKTarrChar :
				ShowCharVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrDouble :
				ShowDoubleVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrInt :
				ShowIntVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrObject :
				ShowObjectVar(ko,OVar,ti,arrIndex);
				break;
			case eKTarrName :
				ShowKerNameVar(ko,OVar,ti,arrIndex);
				break;
		}
}

int CEDObjectProperty::PlaceObjToMap(int ox, int oy, int oz, bool cell, bool check)
{
	if(!KerMain)
		return 1;

	bool place=true;

	CKerObject* ko = KerMain->GetAccessToObject(obj);

	if(!ko || ko->Type->DefAuto==0)	// objekty bez automatismu nevkladam do mapy - nejdou zobrazit
		return 1;

	if(objAlreadyPlaced)
	{	
		if(ko && (ko->Type->EditTag & etbInMap) && ((ko->Type->EditTag & etbNoGrid) || cell))
		{
			if(KerMain->MapInfo->IsObjInMap(obj))
			{
				int dx = ox - ko->PlacedInfo->x;
				int dy = oy - ko->PlacedInfo->y;
				int dz = oz - ko->PlacedInfo->z;
				
				if(KerMain->MapInfo->IsObjInCollision(0,obj,dx,dy,dz))
				{
					if(editor->GetReplaceMod())
					{
						CKerArrObject* oa = KerMain->MapInfo->FindCollidingObjs(0,obj,0,dx,dy,dz);
						int num = oa->GetCount();
						for(int i = 0; i < num; i++)
						{
							OPointer oo = oa->Read(i);
							KerMain->DeleteObject(0,oo);
						}
						delete oa;

						KerMain->MapInfo->MoveObjTo(0,obj,ox,oy,oz);
						ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
					}
				}
				else
				{
					KerMain->MapInfo->MoveObjTo(0,obj,ox,oy,oz);
					ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
				}
			}
			else
			{
				KerMain->MapInfo->WriteObjCoords(ko,ox,oy,oz);
				if(editor->GetReplaceMod())
				{
					KerMain->MapInfo->PlaceObjToMapWithKill(0,obj);
					editor->RemoveGlobalObject(obj);
					ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
				}
				else if(!KerMain->MapInfo->IsObjInCollision(0,obj))
				{
					KerMain->MapInfo->PlaceObjToMap(0,obj);
					editor->RemoveGlobalObject(obj);
					ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
				}
			}
		}
	}
	else
	{
		if(ko && (ko->Type->EditTag & etbInMap) && ((ko->Type->EditTag & etbNoGrid) || cell))
		{
			KerMain->MapInfo->WriteObjCoords(ko,ox,oy,oz);
			ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
			if(editor->GetReplaceMod())
			{
				if(check)
				{
					CKerArrObject* oa = KerMain->MapInfo->FindCollidingObjs(0,obj);
					int num = oa->GetCount();
					for(int i = 0; i < num; i++)
					{
						OPointer oo = oa->Read(i);

						/*
						if(KerMain->GetObjType(oo) == KerMain->GetObjType(obj))	// podminka pro umisteni 
						{														// objekty stejneho typu pres sebe neumistuji
																				// kontrolovat i podle nastaveni objektu (detailnejsi porovnani)
							place=false;
							break;
						}
						//*/
						///*
						if(oo >= (editor->replaceLimit))
						{
							place=false;
							break;
						}
						//*/

					}
					delete oa;
				}
				else
				{
					editor->replaceLimit = KerMain->Objs->GetCounter();
				}
				
				if(place)
				{
					OPointer o;
					o = KerMain->CopyObject(0,obj);
					KerMain->MapInfo->PlaceObjToMapWithKill(0,o);
					editor->AddLevelObject(o);
				}
			}else if(!KerMain->MapInfo->IsObjInCollision(0,obj))
			{
				OPointer o;
				o = KerMain->CopyObject(0,obj);
				KerMain->MapInfo->PlaceObjToMap(0,o);
				editor->AddLevelObject(o);
			}
		}
	}

	return 0;
}

int CEDObjectProperty::IsPossiblePlaceObjToMap(int ox, int oy, int oz)
{
	assert(KerMain);

	CKerObject* ko = KerMain->GetAccessToObject(obj);

	if(!ko || ko->Type->DefAuto==0)	// objekty bez automatismu nevkladam do mapy - nejdou zobrazit
		return -1;

	if(objAlreadyPlaced)
	{	
		if(ko && (ko->Type->EditTag & etbInMap))
		{
			if(KerMain->MapInfo->IsObjInMap(obj))
			{
				int dx = ox - ko->PlacedInfo->x;
				int dy = oy - ko->PlacedInfo->y;
				int dz = oz - ko->PlacedInfo->z;
				
				if(KerMain->MapInfo->IsObjInCollision(0,obj,dx,dy,dz))
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				KerMain->MapInfo->WriteObjCoords(ko,ox,oy,oz);
				if(KerMain->MapInfo->IsObjInCollision(0,obj))
					return 0;
				else
					return 1;
			}
		}else
			return -1;
	}
	else
	{
		if(ko && (ko->Type->EditTag & etbInMap))
		{
			KerMain->MapInfo->WriteObjCoords(ko,ox,oy,oz);
			if(KerMain->MapInfo->IsObjInCollision(0,obj))
				return 0;
			else
				return 1;
		}else
			return -1;
	}

	return 0;
}


int CEDObjectProperty::CreateGlobalObject()
{
	if(!KerMain)
		return 1;

	if(objAlreadyPlaced)
	{
		if(editor->globalObjects->FindObj(obj)==-1)	// objekt neni v seznamu globalnich objektu => umistim ho tam
		{
			CKerObject* ko = KerMain->GetAccessToObject(obj);
			if(ko && (ko->Type->EditTag & etbOutMap))	
			{
				if(KerMain->MapInfo->IsObjInMap(obj))
				{
					KerMain->MapInfo->RemoveObjFromMap(0,obj);
				}
				editor->AddGlobalObject(obj);
				return 0;
			}
			else
			{
				CGUIMessageBox* mb = new CGUIMessageBox(0,0,"Error in placing object","Object is not able to place out of map");
				desktop->AddBackElem(mb);
				mb->FocusMe();
				mb->Center();
			}
		}
	}
	else
	{
		CKerObject* ko = KerMain->GetAccessToObject(obj);
		if(ko && (ko->Type->EditTag & etbOutMap))	
		{
			OPointer o;
			o = KerMain->CopyObject(0,obj);
			ko = KerMain->GetAccessToObject(o);
			if(ko)
			{
				editor->AddGlobalObject(o);
				editor->AddLevelObject(o);
				return 0;
			}else
			{
				KerMain->DeleteObject(0,o);
				return 1;
			}
		}
		else
		{
			CGUIMessageBox* mb = new CGUIMessageBox(0,0,"Error in placing object","Object is not able to place out of map");
			desktop->AddBackElem(mb);
			mb->FocusMe();
			mb->Center();
		}
	}

	return 0;
}

void CEDObjectProperty::SetToDefault()
{
	CKerOVar* OVar;

	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko && !globalVars)
		return;

	for(int i = 0; i <ko->Type->NumVars; i++)
	{
		OVar = &(ko->Type->OVars[i]);

		if(!OVar->DefaultValue)
			continue;

		int type = OVar->Type;
		if(type>=eKTstring && type<=eKTstring+250)
		{
			if(OVar->DefaultValue->Dstring)
			{
				char *ovString = (char*)GetAccessToVar(ko,OVar);
				int len = min((int)strlen(OVar->DefaultValue->Dstring),type-eKTstring);
				strncpy(ovString,OVar->DefaultValue->Dstring,len);
				ovString[len]=0;
			}
		}
		else switch(type)
		{
			case eKTchar :
				{
					char *value = (char*)GetAccessToVar(ko,OVar);
					*value = OVar->DefaultValue->Dchar;
				}
				break;
			case eKTdouble :
				{
					double *value = (double*)GetAccessToVar(ko,OVar);
					*value = OVar->DefaultValue->Ddouble;
				}
				break;
			case eKTint :
				{
					int *value = (int*)GetAccessToVar(ko,OVar);
					*value = OVar->DefaultValue->Dint;
				}
				break;
			case eKTobject :
				{
					OPointer *value = (OPointer*)GetAccessToVar(ko,OVar);
					*value = OVar->DefaultValue->Dobject;
				}
				break;
			case eKTname :
				{
					CKerName **value = (CKerName**)GetAccessToVar(ko,OVar);
					*value = OVar->DefaultValue->Dname;
				}
				break;
			case eKTarrChar :
				{
					CKerArrChar** pArray = (CKerArrChar**)GetAccessToVar(ko,OVar);
					if(*pArray && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray = 0;

					CKerArrChar* sArray = OVar->DefaultValue->Achar;
					if(sArray && sArray->CheckIntegrity())
					{
						*pArray = new CKerArrChar();

						int count = sArray->GetCount();
						for(int i = 0; i < count; i++)
						{
							(*pArray)->Add(sArray->Get(i));
						}
					}
				}
				break;
			case eKTarrDouble :
				{
					CKerArrDouble** pArray = (CKerArrDouble**)GetAccessToVar(ko,OVar);
					if(*pArray && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray = 0;

					CKerArrDouble* sArray = OVar->DefaultValue->Adouble;
					if(sArray && sArray->CheckIntegrity())
					{
						*pArray = new CKerArrDouble();

						int count = sArray->GetCount();
						for(int i = 0; i < count; i++)
						{
							(*pArray)->Add(sArray->Get(i));
						}
					}
				}
				break;
			case eKTarrInt :
				{
					CKerArrInt** pArray = (CKerArrInt**)GetAccessToVar(ko,OVar);
					if(*pArray && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray = 0;

					CKerArrInt* sArray = OVar->DefaultValue->Aint;
					if(sArray && sArray->CheckIntegrity())
					{
						*pArray = new CKerArrInt();

						int count = sArray->GetCount();
						for(int i = 0; i < count; i++)
						{
							(*pArray)->Add(sArray->Get(i));
						}
					}
				}
				break;
			case eKTarrObject :
				{
					CKerArrObject** pArray = (CKerArrObject**)GetAccessToVar(ko,OVar);
					if(*pArray && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray = 0;

					CKerArrObject* sArray = OVar->DefaultValue->Aobject;
					if(sArray && sArray->CheckIntegrity())
					{
						*pArray = new CKerArrObject();

						int count = sArray->GetCount();
						for(int i = 0; i < count; i++)
						{
							(*pArray)->Add(sArray->Get(i));
						}
					}
				}
				break;
			case eKTarrName :
				{
					CKerArrName** pArray = (CKerArrName**)GetAccessToVar(ko,OVar);
					if(*pArray && (*pArray)->CheckIntegrity())
						delete (*pArray);
					*pArray = 0;

					CKerArrName* sArray = OVar->DefaultValue->Aname;
					if(sArray && sArray->CheckIntegrity())
					{
						*pArray = new CKerArrName();

						int count = sArray->GetCount();
						for(int i = 0; i < count; i++)
						{
							(*pArray)->Add(sArray->Get(i));
						}
					}
				}
				break;
		}
	}

	UpdateAllVars();
}


void CEDObjectProperty::UpdateAllVars(CGUITreeItem* ti)
{
	CGUIList<CGUITreeItem> *itemsList = ti ? ti->items : items;

	if(itemsList)
	{
		CGUITreeItem* ni = itemsList->GetNext(0);
		while(ni)
		{
			ShowObjValue(ni);
			if(ni->state)
			{
				if(ni->userTagType == eUTdataClass)
				{
					CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ni->ut.userDataClass);
					if(!aid)	
						UpdateAllVars(ni);	// nejedna se o podstrom reprezentujici kerneli pole => proved update polozek
				}
				else
					UpdateAllVars(ni);	// nejedna se o podstrom reprezentujici kerneli pole => proved update polozek
			}

			ni = itemsList->GetNext(ni);
		}

	}
}

void CEDObjectProperty::DeleteObject()
{
	// vyradit objekt z mapy, vsech moznych seznamu atp.
	if(!KerMain)
		return;

	CKerObject *ko = KerMain->Objs->GetObject(obj);
	if(ko)
	{
		if(KerMain->MapInfo->IsObjInMap(obj))
		{
			KerMain->MapInfo->RemoveObjFromMap(0,obj);
		}else
		{
			int index = editor->globalObjects->FindObj(obj);
			if(index != -1)
			{

				if(editor && index < editor->globalObjsNoDeleteIndex)
					return;

				if(editor)
					editor->RemoveGlobalObject(obj);
			}
		}
	}

	if(editor)
	{
		CEDCellObjectsList* ID(col,editor->cellObjectsListID);
		if(col)
			col->RemoveObjectFromList(obj);
	}

	// smazat objekt a property okno
	if(obj)
	{
		KerMain->DeleteObject(0,obj);
		obj=0;
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(objectListItemID));
		if(ti)
			ti->ut.obj=0;
	}

	CloseWindow();
}

void CEDObjectProperty::ResetObject()
{
	if(!KerMain || objAlreadyPlaced)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko)
		return;

	OPointer o = KerMain->NewObject(0,ko->Type->Name);

	KerMain->DeleteObject(0,obj);

	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(objectListItemID));
	if(ti)
		ti->ut.obj=o;

	CEDObjectProperty* op = new CEDObjectProperty(x,y,sx,sy,o,objectListItemID);
	op->ownObject=ownObject;

	desktop->AddBackElem(op);
	desktop->SetFocusEl(op,0);

	if(objectList)
	{
		if(editor->objPropertyID == GetID())
			editor->objPropertyID=op->GetID();

		op->AcceptEvent(objectList->objHandler->GetID(),EClose);
		op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);

		objectList->SetSelectedObject(op->GetID());
		editor->SetEditMod(1);
	}

	CloseWindow();
}

void CEDObjectProperty::CopyObjectProperty()
{
	OPointer o;
	o = KerMain->CopyObject(0,obj);

	CEDObjectProperty* op = new CEDObjectProperty(x,y,sx,sy,o);
	op->ownObject=true;

	desktop->AddBackElem(op);
	desktop->SetFocusEl(op,0);

	op->AcceptEvent(objectList->objHandler->GetID(),EClose);
	op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);

	objectList->SetSelectedObject(op->GetID());
	//editor->SetEditMod(eEMinsert);

	CloseWindow();
}

void CEDObjectProperty::RefreshObject()
{
	if(!KerMain)
		return;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(!ko)
		return;

	// aktualizace polohy u objektu umisteneho v mape + aktualizace grafiky (automatismu, noConnect) 
	// vse udelano odebranim z mapy a naslednym pridanim + kontrola kolizi
	if(KerMain->MapInfo->IsObjInMap(obj))
	{
		int *px, *py, *pz;
		int ox=0, oy=0, oz=0;
		px = (int*)KerMain->GetAccessToVar3(ko,eKVUx);
		if(px)
			ox=*px;
		py = (int*)KerMain->GetAccessToVar3(ko,eKVUy);
		if(py)
			oy=*py;
		pz = (int*)KerMain->GetAccessToVar3(ko,eKVUz);
		if(pz)
			oz=*pz;

		int dx = ox - ko->PlacedInfo->x;
		int dy = oy - ko->PlacedInfo->y;
		int dz = oz - ko->PlacedInfo->z;
		
		if(dx || dy || dz)
		{
			if(KerMain->MapInfo->IsObjInCollision(0,obj,dx,dy,dz))
			{
				if(editor->GetReplaceMod())
				{
					KerMain->MapInfo->RemoveObjFromMap(0,obj);
					KerMain->MapInfo->PlaceObjToMapWithKill(0,obj);

/*
					CKerArrObject* oa = KerMain->MapInfo->FindCollidingObjs(0,obj,0,dx,dy,dz);
					int num = oa->GetCount();
					for(int i = 0; i < num; i++)
					{
						OPointer oo = oa->Read(i);
						KerMain->DeleteObject(0,oo);
					}
					delete oa;

					KerMain->MapInfo->MoveObjTo(0,obj,ox,oy,oz);
*/
				}
				else
				{
					if(px)
						*px=ko->PlacedInfo->x;
					if(py)
						*py=ko->PlacedInfo->y;
					if(pz)
						*pz=ko->PlacedInfo->z;

					KerMain->MapInfo->RemoveObjFromMap(0,obj);
					KerMain->MapInfo->PlaceObjToMap(0,obj);
				}
			}
			else
			{
				//KerMain->MapInfo->MoveObjTo(0,obj,ox,oy,oz);
				KerMain->MapInfo->RemoveObjFromMap(0,obj);
				KerMain->MapInfo->PlaceObjToMap(0,obj);
			}
		}
		else
		{
			KerMain->MapInfo->RemoveObjFromMap(0,obj);
			KerMain->MapInfo->PlaceObjToMap(0,obj);
		}

		ShowObjValue(posXTI);ShowObjValue(posYTI);ShowObjValue(posZTI);
	}
	else
	{
		//KerServices.ResetAuto(obj,0,1);
	}

	
	PrepareObjCursor();

	// aktualizace velikosti mapy
	if(obj == KerMain->MapInfo->OMapa)
	{	// jedna se o mapu
		KerMain->MapInfo->ResizeMap();

		CEDLevelWindow* ID(gw,editor->gameWindowID);
		if(gw)
		{
			gw->SetShift((float)-KerMain->MapInfo->leftx,(float)-KerMain->MapInfo->lefty);

			float gsx,gsy;
			gsx = (float)KerMain->MapInfo->rightx-KerMain->MapInfo->leftx;
			gsy = (float)KerMain->MapInfo->righty-KerMain->MapInfo->lefty;
			if(gsx>700 || gsy>480)
				gw->SetWindowSizeVP(min(gsx,700),min(gsy,480));
			else
				gw->SetWindowSizeVP(gsx,gsy);
			gw->SetBackWindowNormalSize(gsx,gsy);
			gw->SetBWLimitSizes(0,0,gsx,gsy,false,true);

			editor->RefreshMapPosition();
		}
	}


}

void CEDObjectProperty::SetA()
{
	CEDASelector* ID(oldA,editor->aSelectorID);
	if(oldA)
	{
		desktop->DeleteBackElem(oldA);
	}

	if(!KerMain)
		return;

	if(!KerMain->AutosMain->GetSaveAutosState())
		return;

	CKerObject* ko = KerMain->Objs->GetObject(obj);
	if(ko)
	{
		CEDASelector* as = new CEDASelector(ko->Type,editor->listPosition.x,editor->listPosition.y,editor->listPosition.sx,editor->listPosition.sy);
		desktop->AddBackElem(as);
		as->FocusMe();
	}
}

void CEDObjectProperty::UpdateEditedVar()
{
	CGUIEditWindow* ew = dynamic_cast<CGUIEditWindow*>(GetFocusEl());
	CGUIElement* el=0;

	if(!ew)
	{
		if(CGUIWindow* win = dynamic_cast<CGUIWindow*>(GetFocusEl()))
		{
			ew = dynamic_cast<CGUIEditWindow*>(win->GetFocusEl());
		}
	}

	if(ew)	// mam zmeneny editacni element
	{
		if(ew->parent && ew->parent!=this && ew->parent->GetType()>=PTWindow)
		{
			el=ew->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)
		}
		else
		{
			el=ew;
		}

		CGUITreeItem* ti = 	GetSelectedItem();	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
		if((ti && ti->elem != el) || !ti)	
		{
			ti = FindItemFromElement(el);	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
		}

		if(ti && ti->elem == el)
		{
			int bi = VarValueChanged(ti);	// informuji polozku o zmene, ta zaridi pripadne prekopirovani hodnoty do promenne objektu + vrati zda se jedna o platnou hodnotu
			if(ti->parentItem && (ti->parentItem->userTagType==eUTOVar||ti->parentItem->userTagType==eUTOVarDEL) && ti->parentItem->ut.OVar && ti->parentItem->ut.OVar->EditType >= eKETgroupBit)
			{	// jedna se o skupinovou promennou => aktualizuj vybranou oblast/bunku/bod podle zmenene promenne
				RefreshLocation(ti->parentItem);
			}
			/*
			if(!bi && (!handler->IDbadElem  || ew->GetID()==handler->IDbadElem))
			{
				handler->badElFoc=0;
				handler->IDbadElem = ew->GetID();
				ti->SendBadInputEvent(ew->GetID());	// nova hodnota je neplatna, vyvolej akci k prenastaveni hodnoty
			}
			*/
		}
	}

}

/*
void CEDObjectProperty::SetTitle(int active)
{
	if(active)
		title->WindowIsActive();
	else
		title->WindowIsDeactive();
}
*/

void CEDObjectProperty::SetErrorMsg(char *msg)
{
	if(errorMsg)
		DeleteBackElem(errorMsg);

	if(msg)
	{
		errorMsg = new CGUIStaticText(msg,(CGUIFont*)RefMgr->Find("GUI.F.Arial.10"),10,10,0xFF000000);
		AddBackElem(errorMsg);
	}else
		errorMsg=0;
}

void CEDObjectProperty::SetCursorSelectObj()
{
	mainGUI->cursor->Set(7);
	//mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro selectObj (free)
	cursorChanged=true;
}

void CEDObjectProperty::SetNormalCursor()
{
	// zobrazim std. kurzor
	if(!hold)
	{
		mainGUI->cursor->Set(0);
		cursorChanged=false;
		//mainGUI->cursor->SetVisible(1);
		mainGUI->SendCursorPos();
	}
}


void CEDObjectProperty::AddContextMenu()
{
	CGUIMenu* menu;
	CGUIMenuItem* mi;

	menu = new CGUIMenu(190);
	menu->AddMenuItem(0,"Copy ObjectProperty",new CGUIRectHost(0,0,styleSet->Get("BUT_Copy_Up")->GetTexture(0)));
	menu->AddMenuItem(0,"Create GlobalObject",new CGUIRectHost(0,0,styleSet->Get("GOTB_StdObjectTex")->GetTexture(0)));

	mi = menu->AddMenuItem(0,"Reset Object",new CGUIRectHost(0,0,styleSet->Get("OP_SetToDefautTB1")->GetTexture(0)));
	if(mi)
	{
		mi->SetInactivePicture(new CGUIRectHost(0,0,styleSet->Get("OP_SetToDefautTB4")->GetTexture(0)));
	}

	mi = menu->AddMenuItem(0,"Set to default",new CGUIRectHost(0,0,styleSet->Get("OP_SetToDefautTB1")->GetTexture(0)));
	if(mi)
	{
		mi->SetShortCut("Ctrl+Q",true,false,false,'Q');
	}

	mi = menu->AddMenuItem(0,"Delete Object",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));
	if(mi)
	{
		mi->SetShortCut("Del",false,true,false,VK_DELETE);
	}

	menu->AddMenuItem(0,"Create ShortCut",new CGUIRectHost(0,0,styleSet->Get("OP_ShortCutTB1")->GetTexture(0)));

	mi = menu->AddMenuItem(0,"Refresh Object",new CGUIRectHost(0,0,styleSet->Get("OP_RefreshTB1")->GetTexture(0)));
	if(mi)
	{
		mi->SetShortCut("Ctrl+R",true,false,false,'R');
	}

	menu->AddMenuItem(0,"Set A",new CGUIRectHost(0,0,styleSet->Get("STD_AUTO_TEX")->GetTexture(0)));

	if(objAlreadyPlaced)
	{
		CGUIMenuItem* mi = menu->GetItem(2);
		mi->Deactivate();
	}

	AcceptEvent(menu->GetID(),EMessage);
	menu->AcceptEvent(handler->GetID(),EMenu);
	menuID = menu->GetID();
}

void CEDObjectProperty::AddHandlerAndPanel()
{
	handler = new CEDObjectPropertyHandler(9);	
	handler->op = this;

	// 0 - copy button
	// 1 - set to default button
	// 2 - CEDObjPropertyList ID
	// 3 - CEDObjectProperty ID objektu ulozeneho v promenne
	// 4 - CEDNoConnectDialog
	// 5 - CreateShortCut Dialog
	// 6 - CreateShortCut Button (na panelu)
	// 7 - Refresh Button
	// 8 - Set A Button

	for(int i = 0; i < 9; i++)
		handler->icons[i] = 0;

	CGUIWindowPanel* winpanel = new CGUIWindowPanel(2,STD_TITLE_HEIGHT+2-STD_PANEL_SHIFT,styleSet->Get("ObjPropertyPanel"),this); 


	CGUIButton* copy=new CGUIButton(sx-30,15,-1,-1,"GUI.But.Copy",0,0,"Copy Object");
	copy->AcceptEvent(handler->GetID(),EClicked);
	handler->icons[0] = copy->GetID();
	winpanel->AddIcon(copy);

	CGUIButton* butt;
	butt = winpanel->AddIcon(styleSet->Get("OP_SetToDefautTB1"),styleSet->Get("OP_SetToDefautTB1"),styleSet->Get("OP_SetToDefautTB3"),"Set to default");
	butt->AcceptEvent(handler->GetID(),EClicked);
	handler->icons[1] = butt->GetID();

	butt = winpanel->AddIcon(styleSet->Get("OP_ShortCutTB1"),styleSet->Get("OP_ShortCutTB1"),styleSet->Get("OP_ShortCutTB3"),"Make ShortCut");
	butt->AcceptEvent(handler->GetID(),EClicked);
	handler->icons[6] = butt->GetID();

	butt = winpanel->AddIcon(styleSet->Get("OP_RefreshTB1"),styleSet->Get("OP_RefreshTB1"),styleSet->Get("OP_RefreshTB3"),"Refresh Object");
	butt->AcceptEvent(handler->GetID(),EClicked);
	handler->icons[7] = butt->GetID();

	butt = winpanel->AddIcon(styleSet->Get("AutoDlgTB1"),styleSet->Get("AutoDlgTB1"),styleSet->Get("AutoDlgTB3"),"Set A");
	butt->AcceptEvent(handler->GetID(),EClicked);
	handler->icons[8] = butt->GetID();
}

void CEDObjectProperty::MakeShortCutDlg()
{
	CGUIDlgEditbox* dlg = new CGUIDlgEditbox(0,0,250,"Make ShortCut to Object","ShortCut Name:",new CGUIRectHost(0,0,styleSet->Get("OP_ShortCutTB1")->GetTexture(0)));
	desktop->AddBackElem(dlg);
	dlg->AcceptEvent(handler->GetID(),EOk);
	handler->icons[5] = dlg->GetID();

	if(!shortCutName)
		shortCutName = new char[201];
	dlg->BindEditBox(shortCutName,dtString,200);

	dlg->Center();
	dlg->SetModal(true);
}

void CEDObjectProperty::ProcessShortCutDlg(CGUIEvent* event)
{
	if(event->eventID == EOk && KerMain)
	{
		CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(handler->icons[5]));
		if(dlg)
		{
			if(dlg->SyncEditBox()==0)
			{
				OPointer o = KerMain->CopyObject(0,obj);

				editor->shortCutArray.Add(new CEDShortCut(o,shortCutName));
				objectList->AddShortCutObj(o,shortCutName);

				dlg->CloseWindow();
				handler->icons[5]=0;
				SAFE_DELETE_ARRAY(shortCutName);
			}
		}
	}
	else if(event->eventID == ECancel)
	{
		handler->icons[5]=0;
		SAFE_DELETE_ARRAY(shortCutName);
	}
}

void CEDObjectProperty::PrepareObjCursor()
{
	CKerObject *ko;
	if(KerMain &&  (ko = KerMain->Objs->GetObject(obj)))
	{
		// zjistit texturu pro objekt v ToolBaru
		bool releaseTex=false;
		CTexture* objTex = ko->Type->EditTex;
		if(!objTex)
		{
			CKerName* aName;
			
			if(ko->PlacedInfo)
				aName = ko->PlacedInfo->AktivAuto;
			else
				aName = KerMain->AutosMain->ProcessAuto(ko);

			if(aName)
			{
				if(aName->Auto->EditTex)
					objTex=aName->Auto->EditTex;
				else
				{
					if(aName->Auto->SonsNum>0)
					{
						CLightTex* lt = dynamic_cast<CLightTex*>(aName->Auto->textures[0].texture);

						if(lt)
						{
							if(lt && lt->CmpTex(&objTex))
								releaseTex=true;
						}
						else
						{
							CGEnAnim* anim = dynamic_cast<CGEnAnim*>(aName->Auto->textures[0].texture);
							lt=0;
							if(anim)
								lt=anim->GetFrameTex(0);
							if(lt && lt->CmpTex(&objTex))
								releaseTex=true;
						}
					}
				}
			}
		}
		if(!objTex)
		{
			if((ko->Type->EditTag & etbInMap) || (ko->Type->EditTag & etbOutMap) || objAlreadyPlaced)
				objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);
			else
				objTex = styleSet->Get("OB_Object")->GetTexture(0);
		}

		
		UINT tsx, tsy;
		objTex->GetImageSize(&tsx,&tsy);
		mainGUI->cursor->Load(10,objTex,tsx/2,tsy/2);

		// aktualizace obrazku v seznamu objektu - CEDObjectTree
		CGUITreeItem* ID(objListTI,objectListItemID);
		if(objListTI)
			objListTI->SetPicture(new CGUIRectHost(0,0,objTex));

		// aktualizace obrazku v toolbaru globalnich objektu
		if(objAlreadyPlaced && !KerMain->MapInfo->IsObjInMap(obj))
		{
			int i = editor->globalObjects->FindObj(obj);
			CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(editor->globalObjsTB));
			if(tb)
			{
				CGUIToolBarButton* tbb = tb->GetTool(i+1,0);
				if(tbb)
					tbb->SetPictureUp(new CGUIRectHost(0,0,objTex));
			}
		}

		if(releaseTex && objTex)
			objTex->Release();
	}
	else
	{
		mainGUI->cursor->Load(10,styleSet->Get("OB_Object")->GetTexture(0),0,0);

		CGUITreeItem* ID(objListTI,objectListItemID);
		if(objListTI)
			objListTI->SetPicture(new CGUIRectHost(0,0,styleSet->Get("OB_Object")->GetTexture(0)));
	}
}


void CEDObjectProperty::UpdateTree()
{
	float px,py;
	float min_x, min_y, max_x, max_y;

	px=treeItemStartX;
	py=treeItemStartY;
	min_x=px;min_y=py;
	max_x=min_x;max_y=min_y;

	DistributeUpdate(px,py,max_x);	// aktualizuje stav vsech polozek stromu
	max_y=py;

	if(automaticSliderPos)
		SetSliderPos(max_x+STD_TREE_BEFORE_SLIDER_GAP);	// nastavi pozici rozdelovace - slider

	DistributeUpdateInlineEl(max_x);	// aktualizuje pozice radkovych elementu polozek, podle nove pozice slideru

	SetBackWindowNormalSize(max_x+treeEndGapX,max_y+treeEndGapY);	// nastavi velikost vnitrku okna, podle toho kolik zabiraji viditelne polozky a elementy

	if(treeSlider)	// pokud je manualni slider, aktualizuj jeho casti (podklad radkovych elementu atp.)
	{
		float tsx,tsy;
		float wsx,wsy;
		GetBackWindowSize(wsx,wsy);
		treeSlider->GetSize(tsx,tsy);
		treeSlider->Resize(tsx,wsy);
		if(sliderBackground)
			sliderBackground->Resize(wsx-sliderPosition,wsy);
	}
	UpdateRootItemsBackgrounds();


	if(objComment)
	{
		float nsx, nsy;
		float ssx,ssy;
		GetBackWindowSize(nsx,nsy);
		objComment->GetSize(ssx,ssy);

		objComment->Move(15,nsy-ssy-10);
	}
}


//////////////////////////////////////////////////////////////////////
// CEDVarArrayItemData
//////////////////////////////////////////////////////////////////////

CEDVarArrayItemData::~CEDVarArrayItemData()
{
	if(delOvar)
		SAFE_DELETE(OVar);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CEDObjectTreeHandler	
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDObjectTreeHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	
	if(event->eventID == EClose) 
	{
		if(CEDObjectProperty *op = dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(event->sender)))	// jedna se o zavreni Property okna
		{
			if(op->GetID() == editor->objPropertyID)
			{
				objectList->SetSelectedObject(0);
			}
		}
	}

	if(event->eventID == EFocusChanged && editor->GetEditMod()!=eEMselectObj && editor->GetEditMod()!=eEMselectCell && editor->GetEditMod()!=eEMselectLocation)
	{
		if(event->pInt)
		{
			objectList->SetSelectedObject(event->sender);
			if(CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID)))
			{
				if(!op->objAlreadyPlaced && !op->globalVars)
				{
					editor->SetEditMod(eEMinsert);
					editor->SetFreeModByObject();
				}
			}
		}
		else if(editor->GetEditMod() == eEMinsert)
		{
		}
	}

	if(event->eventID == EToolBar && KerMain)
	{
		if(event->pFloat==1)
		{
			if(event->pInt==0)
			{
				CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
				if(op)
					desktop->DeleteBackElem(op);
			
				op = new CEDObjectProperty(editor->objPropertyPosition.x,editor->objPropertyPosition.y,editor->objPropertyPosition.sx,editor->objPropertyPosition.sy,true);
				desktop->AddBackElem(op);
				//editor->objPropertyID = op->GetID();

				desktop->SetFocusEl(op,0);
				if(objectList)
				{
					op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);
					op->AcceptEvent(objectList->objHandler->GetID(),EClose);
					objectList->SetSelectedObject(op->GetID());
				}
			}
			else
			{
				OPointer o = editor->globalObjects->Get(event->pInt-1);
				if(editor->GetEditMod()==eEMselectObj)	// SelectObj MOD
				{
					CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(editor->objVarSelectionID);
					if(opl)
						opl->SetObject(o);

					// vymacknuti tlacitka na toolbaru, ktere odpovida prave rusenemu objProperty
					if(editor)
					{
						CGUIToolBar* tb = GetObjFromID<CGUIToolBar>(editor->globalObjsTB);
						if(tb)
						{
							CGUIToolBarButton* tbb = tb->GetTool(event->pInt,0);
							if(tbb)
								tbb->ChangeState(false);
						}
					}

				}
				else if(editor->GetEditMod()==eEMdelete && o)	// Delete MOD
				{
					CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
					if(op && op->obj == o)
						op->DeleteObject();
					else
					{
						CKerObject *ko = KerMain->Objs->GetObject(o);
						if(ko && editor)
						{
							if(event->pInt-1 >= editor->globalObjsNoDeleteIndex)
							{
								editor->RemoveGlobalObject(o);
								KerMain->DeleteObject(0,o);
							}
							else
							{
								CGUIToolBar* tb = GetObjFromID<CGUIToolBar>(editor->globalObjsTB);
								if(tb)
								{
									CGUIToolBarButton* tbb = tb->GetTool(event->pInt,0);
									if(tbb)
										tbb->ChangeState(false);
								}
							}
						}
					}
				}
				else if(CKerObject *ko = KerMain->Objs->GetObject(o))	// Select MOD a zbytek
				{
					CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
					if(op)
						desktop->DeleteBackElem(op);
				
					if(op = ((CEDEditorDesktop*)desktop)->FindObjPropertyWin(o))
					{
						op->FocusMe();
						if(objectList)
							objectList->SetSelectedObject(op->GetID());
					}
					else
					{
						op = new CEDObjectProperty(editor->objPropertyPosition.x,editor->objPropertyPosition.y,editor->objPropertyPosition.sx,editor->objPropertyPosition.sy,o,0,true);
						desktop->AddBackElem(op);
						editor->objPropertyID = op->GetID();

						desktop->SetFocusEl(op,0);
						if(objectList)
						{
							op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);
							op->AcceptEvent(objectList->objHandler->GetID(),EClose);
							objectList->SetSelectedObject(op->GetID());
							//editor->SetEditMod(eEMinsert);
						}
					}

					if(editor->GetEditMod()==eEMnoConnect)	// NoConnect MOD
					{
						new CEDNoConnectDialog(o);
					}
				}
			}
		}
		else
		{
			CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
			if(op && (event->pInt==0 || op->obj == editor->globalObjects->Get(event->pInt-1)) && (editor->GetEditMod()!=eEMselectObj))
				desktop->DeleteBackElem(op);
		}
	}

	if(event->eventID == EMenu)
	{
		if(event->sender == icons[0])
		{
			CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(event->pID2);
			if(event->pInt==0)	// Make ShortCut
			{
				CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
				if(op)
					op->MakeShortCutDlg();
			}
			else if(event->pInt==1 && ti && ti->userTagType == eUTkerName)	// Delete ShortCut
			{
				int size = editor->shortCutArray.GetSize();
				for(int i = 0; i < size; i++)
				{
					CEDShortCut* sc = editor->shortCutArray.Get(i);
					if(sc && sc->obj == ti->ut.obj)
					{
						CEDObjectProperty *op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
						if(op && op->obj == sc->obj)
							op->DeleteObject();

						delete editor->shortCutArray.Remove(i);
						
						if(ti->parentItem)
							ti->parentItem->DeleteTreeItem(ti);
						else
							objectList->DeleteTreeItem(ti);

						objectList->UpdateTree();

						break;
					}
				}
			}
		}
	}

	delete event;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// CEDObjectPropertyHandler	
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDObjectPropertyHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;
	
	bool processed=false;

	if(event->eventID == EClicked)
	{
		op->UpdateEditedVar();

		if(event->sender == icons[0])	// copy button
		{
			op->CopyObjectProperty();
			processed=true;
		}
		else if(event->sender == icons[1])	// set to default button
		{
			op->SetToDefault();
			processed=true;
		}
		else if(event->sender == icons[6])	// Make ShortCut button
		{
			op->MakeShortCutDlg();
			processed=true;
		}
		else if(event->sender == icons[7])	// Refresh button
		{
			op->RefreshObject();
			processed=true;
		}
		else if(event->sender == icons[8])	// Set A button
		{
			op->SetA();
			processed=true;
		}
	}

	if(event->eventID == EMenu)
	{
		op->UpdateEditedVar();

		if(event->pInt==0)	// Copy ObjectProperty
		{
			op->CopyObjectProperty();
		}else if(event->pInt==4)	// Delete Object
		{
			op->DeleteObject();
		}else if(event->pInt==2)	// reset
		{
			op->ResetObject();
		}else if(event->pInt==1)	// global object
		{
			op->CreateGlobalObject();
		}else if(event->pInt==3)	// set to default values
		{
			op->SetToDefault();
		}
		else if(event->pInt==5)	// Make ShortCut 
		{
			op->MakeShortCutDlg();
		}
		else if(event->pInt==6)	// Refresh
		{
			op->RefreshObject();
		}
		else if(event->pInt==7)	// Refresh
		{
			op->SetA();
		}

		processed=true;
	}

	if(event->sender == icons[5])	// prisla udalost od dialogu na vytvareni ShortCutu
	{
		op->ProcessShortCutDlg(event);
		processed=true;
	}

	if(event->eventID == ETreeItemRoller)
	{
		CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(event->sender);
		if(ti && (ti->userTagType==eUTOVar||ti->userTagType==eUTOVarDEL) && ti->ut.OVar && ti->ut.OVar->EditType >= eKETgroupBit)
		{
			if(event->pInt2 == 2)
				op->SelectLocationEditModByOVar(ti,ti->ut.OVar,true);
			else
				op->SelectLocationEditModByOVar(ti,ti->ut.OVar,false);
		}
		processed=true;
	}

	if(event->eventID == EFocusChanged)
	{
		CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(event->sender);
		if(ti && (ti->userTagType==eUTOVar||ti->userTagType==eUTOVarDEL) && ti->ut.OVar && ti->ut.OVar->EditType >= eKETgroupBit)
		{
			if(event->pInt)
			{
				if(ti->state==2)
				{
					op->SelectLocationEditModByOVar(ti,ti->ut.OVar,true);
				}
				else
				{
					if(op->editedGroupItem)
					{
						op->SelectLocationEditModByOVar(op->editedGroupItem,op->editedGroupItem->ut.OVar,false);
					}
				}
				processed=true;
			}
		}
	}


	/*
	if(event->eventID == ETree)
	{
		if(event->sender == icons[2] && event->pInt2 == 0)
		{
			CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(icons[2]);
			if(opl)
			{
				opl->CloseWindow();
				icons[2]=0;
			}
		}
	}
	*/

	if(event->sender == IDbadElem && badElFoc){
		IDbadElem=0;
	}
	
	if(event->eventID == EOk)	// prisla udalost o zmene hodnoty v editacnim elementu
	{
		CGUIElement* ID(el,event->sender);
		if(el)	// mam zmeneny editacni element
		{
			if(el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
				el=el->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)

			CGUITreeItem* ti = 	op->GetSelectedItem();	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
			if((ti && ti->elem != el) || !ti)	
			{
				ti = op->FindItemFromElement(el);	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
			}

			if(ti && ti->elem == el)
			{
				int bi = op->VarValueChanged(ti);	// informuji polozku o zmene, ta zaridi pripadne prekopirovani hodnoty do promenne objektu + vrati zda se jedna o platnou hodnotu
				if(ti->parentItem && (ti->parentItem->userTagType==eUTOVar||ti->parentItem->userTagType==eUTOVarDEL) && ti->parentItem->ut.OVar && ti->parentItem->ut.OVar->EditType >= eKETgroupBit)
				{	// jedna se o skupinovou promennou => aktualizuj vybranou oblast/bunku/bod podle zmenene promenne
					op->RefreshLocation(ti->parentItem);
				}
				if(!bi && (!IDbadElem  || event->sender==IDbadElem))
				{
					badElFoc=0;
					IDbadElem = event->sender;
					ti->SendBadInputEvent(event->sender);	// nova hodnota je neplatna, vyvolej akci k prenastaveni hodnoty
				}
			}
		}
	}
	
	if(event->eventID == ECancel)	// prisla udalost o zruseni nove naeditovane hodnoty a vraceni k puvodni
	{
		CGUIElement* ID(el,event->sender);
		if(el)	// mam editacni element
		{
			if(el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
				el=el->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)

			CGUITreeItem* ti = 	op->GetSelectedItem();	
			if(ti && ti->elem == el)	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
			{
				op->ShowObjValue(ti);	// vratim puvodni hodnotu do editacniho elementu z promenne objektu
			}
			else if(ti = op->FindItemFromElement(el))	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
			{
				op->ShowObjValue(ti);	// vratim puvodni hodnotu do editacniho elementu z promenne objektu
			}
		}
	}

	if(event->eventID == EClicked && !processed)	
	{
		CGUIElement* ID(el,event->sender);
		if(dynamic_cast<CGUICheckBox*>(el))	// prisla udalost o zmene hodnoty v checkboxu
		{
			CGUICheckBox* chBox = (CGUICheckBox*)el;
			if(el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
				el=el->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)

			bool found=false;

			CGUITreeItem* ti = 	op->GetSelectedItem();
			if(ti && ti->elem == el)	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
			{
				found=true;
			}
			else if(ti = op->FindItemFromElement(el))	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
			{
				found=true;
			}

			if(found)
			{
				if(ti->userTagType == eUTnone && ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
				{
					// stisknuti checkboxu create/delete array
					CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
					if(aid && aid->createCheckBox == event->sender)
					{
						op->CreateOrDeleteArrayVar(ti->parentItem,aid->OVar,ti,chBox);
					}
				}
				else
				{
					int bi = op->VarValueChanged(ti);	// informuji polozku o zmene, ta zaridi pripadne prekopirovani hodnoty do promenne objektu + vrati zda se jedna o platnou hodnotu
					if(!bi && (!IDbadElem  || event->sender==IDbadElem))
					{
						badElFoc=0;
						IDbadElem = event->sender;
						ti->SendBadInputEvent(event->sender);	// nova hodnota je neplatna, vyvolej akci k prenastaveni hodnoty
					}
				}
			}
		}
		else if(dynamic_cast<CGUIButton*>(el))
		{											// prisla udalost o stisknuti tlacitka
			CGUIElement* button = el;
			if(el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
				el=el->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)

			bool found=false;

			CGUITreeItem* ti = 	op->GetSelectedItem();
			if(ti && ti->elem == el)	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
			{
				found=true;
			}
			else if(ti = op->FindItemFromElement(el))	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
			{
				found=true;
			}

			if(found)
			{
				// zjistit o jake stisknute tlacitko jde

				if(ti->userTagType == eUTnone && ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
				{
					// stisknuti tlacitka add/delete item u pole polozek
					CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
					if(aid && aid->addButton == event->sender)
					{
						op->AddNewArrayVarItemProperty(ti->parentItem,aid->OVar,ti->GetID());
					}
					else if(aid && aid->delButton == event->sender)
					{
						op->DeleteLastArrayVarItemProperty(ti->parentItem,aid->OVar,ti);
					}
				}
				else
				{
					// zpracovat stisknuti tlacitka u polozky
					// rozbaleni nabidky moznosti vybereru atp. (podle typu polozky)

					CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(icons[2]);
					if(opl)
						desktop->DeleteBackElem(opl);

					CEDNoConnectDialog* ncd = GetObjFromID<CEDNoConnectDialog>(icons[4]);
					if(ncd)
						desktop->DeleteBackElem(ncd);

					if((ti->userTagType==eUTOVar||ti->userTagType==eUTOVarDEL) && ti->ut.OVar->EditType == eKETconnectionMask)
					{
						ncd = new CEDNoConnectDialog(op->obj);
						icons[4] = ncd->GetID();
						ncd->FocusMe();
						ncd->SetModal(true);
					}
					else if((ti->userTagType==eUTOVar||ti->userTagType==eUTOVarDEL) && ti->ut.OVar && ti->ut.OVar->EditType >= eKETgroupBit)
					{
						op->ClearLocation(ti);
						op->RefreshLocation(ti);
					}
					else
					{
						//float px=0,py=0;
						//desktop->GetDesktopPos(button,px,py);
						opl = new CEDObjPropertyList(editor->listPosition.x,editor->listPosition.y,editor->listPosition.sx,editor->listPosition.sy,ti,op);
						desktop->AddBackElem(opl);
						icons[2] = opl->GetID();
						//opl->AcceptEvent(GetID(),ETree);
						//opl->Normalize();
						opl->FocusMe();
						//opl->SetModal(1);
					}
				}
			}
		}
	}

	if(event->eventID == EMouseButton && event->pInt == 1 && !processed && KerMain)
	{
		if(CGUIElement* ID(el,event->sender))
		{
			CGUIElement* button = el;
			if(el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
				el=el->parent;	// prisla udalost od editacniho okna umisteneho v okne (ne primo u polozky)

			bool found=false;

			CGUITreeItem* ti = 	op->GetSelectedItem();
			if(ti && ti->elem == el)	// najdu k nemu pridruzenou polozku stromu (zkusim prave zda nejde o prave oznacenou)
			{
				found=true;
			}
			else if(ti = op->FindItemFromElement(el))	// najdu k nemu pridruzenou polozku stromu (oznacena to nebyla)
			{
				found=true;
			}

			if(found)
			{
				// zpracovat stisknuti praveho tlacitka u polozky 
				// otevreni property okna objektu ulozeneho v promenne

				CEDObjectProperty* opl = GetObjFromID<CEDObjectProperty>(icons[3]);
				if(opl && opl!=op)
					desktop->DeleteBackElem(opl);

				if(((ti->userTagType == eUTOVar||ti->userTagType == eUTOVarDEL) && ti->ut.OVar->Type == eKTobject) || (ti->userTagType == eUTint))
				{
					CKerObject* ko = KerMain->GetAccessToObject(op->obj);
					OPointer pObj = 0;
					if(ko || op->globalVars)
					{
						if(ti->userTagType == eUTint)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
						{
							CKerOVar* OVar=0;
							CKerArrObject* pArray=0;

							if(ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
							{
								CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
								if(aid)
									OVar = aid->OVar;
							}

							if(OVar && OVar->Type == eKTarrObject)
								pArray = *(CKerArrObject**)op->GetAccessToVar(ko,OVar);
							if(pArray)
								pObj = pArray->Get(ti->ut.pInt-1);
						}
						else if(ti->userTagType == eUTOVar || ti->userTagType == eUTOVarDEL)
							pObj = *(OPointer*)op->GetAccessToVar(ko,ti->ut.OVar);
					}
						
					if(pObj)
					{
						if(opl = ((CEDEditorDesktop*)desktop)->FindObjPropertyWin(pObj))
						{
							icons[3] = opl->GetID();
							opl->FocusMe();
							if(objectList)
								objectList->SetSelectedObject(opl->GetID());
						}
						else
						{
							/*
							CEDObjectProperty *opo=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID));
							if(opo && editor->GetPropertyMod())
								desktop->DeleteBackElem(opo);
								*/

							//opl = new CEDObjectProperty(editor->objPropertyPosition.x,editor->objPropertyPosition.y,editor->objPropertyPosition.sx,editor->objPropertyPosition.sy,pObj,0,true);

							editor->SetEditMod(eEMmoveObj);
							
							opl = new CEDObjectProperty(editor->listPosition.x,editor->listPosition.y,editor->listPosition.sx,editor->listPosition.sy,pObj,0,true);
							opl->parentObjPropertyID=op->GetID();
							desktop->AddBackElem(opl);
							icons[3] = opl->GetID();

							desktop->SetFocusEl(opl,0);
							if(objectList)
							{
								opl->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);
								opl->AcceptEvent(objectList->objHandler->GetID(),EClose);
								objectList->SetSelectedObject(opl->GetID());
								editor->SetFreeModByObject();
							}
						}
					}
				}
			}
		}
	}

	if(event->eventID == EBadInput)	// prisla udalost o spatnem vstupu
	{
		CGUIElement* ID(el,event->pID);	// najdu spatne nestaveny editacni element
		if(!op->IsFrontFocus() && op->IsFocused() ) // 
		{
			CGUITreeItem* ID(ti,event->sender);	// najdu polozku stromu odpovidajici editacnimu elementu
			if(ti && ti!=op->GetSelectedItem())	// polozka stromu neni oznacena => oznacim ji
			{
				if(!ti->IsVisible())	// polozka neni mezi rozbalenymi polozkami (viditelnymi)
					op->RollTreeToItem(ti);	// rozbalim strom na polozku
				op->Switch(ti);	// prepnu se na polozku
			}

			if(el && el->parent && el->parent!=op && el->parent->GetType()>=PTWindow)
			{
				op->SetFocusEl(el->parent,0);
				((CGUIWindow*)el->parent)->SetFocusEl(el,0);
			}
			else if(el)
				op->SetFocusEl(el,0);	// focusuji editacni element

			if(CGUIEditWindow* ew = dynamic_cast<CGUIEditWindow*>(el))	// jedna se o EditWindow
				ew->BadInput();	// nastavim EW na spatny vstup
			badElFoc=1;
		}
		else
		{
			if(CGUIEditWindow* ew = dynamic_cast<CGUIEditWindow*>(el))	// jedna se o EditWindow
				ew->BadInput();	// nastavim EW na spatny vstup
		}
	}


	delete event;
}


//////////////////////////////////////////////////////////////////////
// CEDObjectBrowser
//////////////////////////////////////////////////////////////////////

CEDObjectBrowser::CEDObjectBrowser(float _x, float _y, float _sx, float _sy)
: CGUITree(_x,_y,_sx,_sy,"ObjectBrowser",new CGUIRectHost(0,0,"$TEX$\\strom.png"))
{

}

CEDObjectBrowser::~CEDObjectBrowser()
{

}

//////////////////////////////////////////////////////////////////////
// CEDGlobalObjectsToolBar
//////////////////////////////////////////////////////////////////////

CEDGlobalObjectsToolBar::CEDGlobalObjectsToolBar(float _x, float _y, float _sx, float _sy)
: CGUIToolBar(_x,_y,_sx,_sy,"Global Objects",0)
{
	MsgAcceptConsume(MsgMouseL|MsgMouseOver);
}

CEDGlobalObjectsToolBar::~CEDGlobalObjectsToolBar()
{

}

int CEDGlobalObjectsToolBar::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	int ret = CGUIToolBar::MouseLeft(x,y,mouseState,keyState);

	if(ret)
	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			if(editor->GetEditMod() == eEMmoveObj && mouseState)
			{
				CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
				if(op)
					op->CreateGlobalObject();

				return 0;
			}else if(editor->GetEditMod() == eEMinsert && mouseState)
			{
				CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID);
				if(op && !op->objAlreadyPlaced)
					op->CreateGlobalObject();

				return 0;
			}
		}
	}

	return ret;
}

int CEDGlobalObjectsToolBar::MouseOver(float x, float y, UINT over,UINT state)
{
	int ret = CGUIToolBar::MouseOver(x,y,over,state);

	if(ret)
	{
		InnerCoords(x,y);
		if(TransformCoordsFrontToBack(x,y))
		{
			ShowCursorByEditMod();
		}
		else
		{
			ShowStdCursor();
		}
	}

	return ret;
}

int CEDGlobalObjectsToolBar::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	int ret = CGUIToolBar::MouseOverFocus(x,y,over,state,ID);

	if(cursorChanged)
		ShowStdCursor();

	return ret;
}

void CEDGlobalObjectsToolBar::ShowStdCursor()
{
	if(!hold && mainGUI->cursor->GetCursorIndex()>4)
	{
		// zobrazim std. kurzor
		if(editor && editor->GetEditMod()==eEMselectObj)
			mainGUI->cursor->Set(7);
		else
			mainGUI->cursor->Set(0);
	}

	if(!hold)
	{
		if(editor && editor->GetEditMod()!=eEMselectObj)
			cursorChanged=false;
		//mainGUI->cursor->SetVisible(1);
	}
}

void CEDGlobalObjectsToolBar::ShowCursorByEditMod()
{
	if(editor->GetEditMod()==eEMnormal || editor->GetEditMod()==eEMselectCell || editor->GetEditMod()==eEMselect)
	{
		mainGUI->cursor->Set(0);
		if(!hold)
		{
			cursorChanged=false;
			//mainGUI->cursor->SetVisible(1);
		}
	}
	else if(editor->GetEditMod()==eEMinsert || editor->GetEditMod()==eEMmoveObj)
	{
		mainGUI->cursor->Set(10);
		if(!hold)
		{
			//mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vkladani objektu (free)
			cursorChanged=true;
		}
	}
	else if(editor->GetEditMod()==eEMdelete)
	{
		mainGUI->cursor->Set(8);
		if(!hold)
		{
			//mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro vyber objektu na smazani
			cursorChanged=true;
		}
	}
	else if(editor->GetEditMod()==eEMnoConnect)
	{
		mainGUI->cursor->Set(9);
		if(!hold)
		{
			//mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro noConnect (free)
			cursorChanged=true;
		}
	}
	else if(editor->GetEditMod()==eEMselectObj)
	{
		mainGUI->cursor->Set(7);
		if(!hold)
		{
			//mainGUI->cursor->SetVisible(1);		// zobraz kurzor pro selectObj (free)
			cursorChanged=true;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CEDCellObjectsList
//////////////////////////////////////////////////////////////////////

CEDCellObjectsList::CEDCellObjectsList(float _x, float _y, float _sx, float _sy, CKerArrObject* oa)
: CGUITree(_x,_y,_sx,_sy,"CellObjectsList",new CGUIRectHost(0,0,styleSet->Get("EI_CellSelector")->GetTexture(0)))
{
	RemoveFromTrash();

	treeItemHeight = treeRootItemHeight= 30;
	treeItemPictureSize = 26;

	if(KerMain && oa)
	{
		int num = oa->GetCount();
		for(int i = 0; i < num; i++)
		{
			OPointer obj = oa->Get(i);
			if(CKerObject *ko = KerMain->Objs->GetObject(obj))
			{
				char *name = ko->Type->Name->GetUserName();
				char itemName[100];
				_snprintf(itemName,99,"%s - %d",name,obj);
				CTexture* objTex=0;
				bool releaseTex=false;
				if(objectList)
				{
					objTex = ko->Type->EditTex;
					if(!objTex)
					{
						CKerName* aName;
						
						if(ko->PlacedInfo)
							aName = ko->PlacedInfo->AktivAuto;
						else
							aName = KerMain->AutosMain->ProcessAuto(ko);

						objectList->PrepareAutoTexture(aName,&objTex,releaseTex);
					}
				}
				else
					objTex = styleSet->Get("OB_Object")->GetTexture(0);

				CGUITreeItem* nti = AddTreeItem(0,itemName,new CGUIRectHost(0,0,objTex));
				nti->userTagType=eUTopointer;
				nti->ut.kerName=ko->Type->Name;
				nti->ut.obj=obj;

				if(releaseTex && objTex)
					objTex->Release();
			}
		}
		delete oa;

		UpdateTree();
	}

	MsgAcceptConsume(MsgKeyboard);

	AddToTrash();
}

CEDCellObjectsList::~CEDCellObjectsList()
{
}

int CEDCellObjectsList::RemoveObjectFromList(OPointer obj)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->userTagType==eUTopointer && e->ut.obj == obj)
			{
				int ret = DeleteTreeItem(e);
				UpdateTree();
				return ret;
			}
		}
		le=items->GetNextListElement(le);
	}

	return 1;
}

int CEDCellObjectsList::TreeHandler(typeID treeItem, int index, int state)
{
	if(state==3)
		return 1;

	CEDObjectProperty* op=0;

	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));

	if(editor->GetEditMod()==eEMselectObj && ti && ti->userTagType==eUTopointer)
	{
		CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(editor->objVarSelectionID);
		if(opl)
			opl->SetObject(ti->ut.obj);
	}
	else if(ti && ti->userTagType==eUTopointer)
	{
		if(op=dynamic_cast<CEDObjectProperty*>(nameServer->ResolveID(editor->objPropertyID)))
		{
			if(op->obj != ti->ut.obj)
			{
				desktop->DeleteBackElem(op);
				if(objectList)
					objectList->SetSelectedObject(0);
				//editor->SetEditMod(0);
			}else
			{
				op->FocusMe();
				if(objectList)
					objectList->SetSelectedObject(editor->objPropertyID);
				//editor->SetEditMod(eEMinsert);
				return 1;
			}
		}

		if(op = ((CEDEditorDesktop*)desktop)->FindObjPropertyWin(ti->ut.obj))
		{
			op->FocusMe();
			if(objectList)
				objectList->SetSelectedObject(op->GetID());
			//editor->SetEditMod(eEMinsert);
			return 1;	// property okno pro objekt "obj" jiz v desktopu je, nove nevytvaret
		}		

		CKerObject* ko=0;
		if(ti->userTagType==eUTopointer && KerMain && ti->ut.obj && (ko = KerMain->Objs->GetObject(ti->ut.obj)))
		{
			op = new CEDObjectProperty(editor->objPropertyPosition.x,editor->objPropertyPosition.y,editor->objPropertyPosition.sx,editor->objPropertyPosition.sy,ti->ut.obj,treeItem,true);
			desktop->AddBackElem(op);

			desktop->SetFocusEl(op,0);
			if(objectList)
			{
				op->AcceptEvent(objectList->objHandler->GetID(),EClose);
				op->AcceptEvent(objectList->objHandler->GetID(),EFocusChanged);
				objectList->SetSelectedObject(op->GetID());
				//editor->SetEditMod(eEMinsert);
			}
		}
	}

	return 1;
}

int CEDCellObjectsList::Keyboard(UINT ch, UINT state) //klavesnice
{
	int st = CGUITree::Keyboard(ch,state);
	
	if(ch==VK_ESCAPE)
	{
		CloseWindow();
		st=0;
	}
	
	return st;
}

//////////////////////////////////////////////////////////////////////
// CEDObjPropertyList
//////////////////////////////////////////////////////////////////////

int CEDObjPropertyList::nameMask = 0xFFFFFFFF;
int CEDObjPropertyList::eKVUBobjInMapPL=0;
int CEDObjPropertyList::eKVUBobjOutMapPL=0;


CEDObjPropertyList::CEDObjPropertyList(float _x, float _y, float _sx, float _sy, CGUITreeItem* ti, CEDObjectProperty* op)
: CGUITree(_x,_y,_sx,_sy,"Select option:",new CGUIRectHost(0,0,styleSet->Get("EI_VarSelector")->GetTexture(0)))
{
	RemoveFromTrash();

	propertyItem=ti;
	objProperty=op;

	mouseClick=closeOnClick=false;
	rollOnClick=false;

	editor->objVarSelectionID = GetID();

	CKerOVar* OVar = 0;
	int arrIndex = 0;

	if(ti)
	{
		if(ti->userTagType==eUTOVar || ti->userTagType==eUTOVarDEL)
			OVar =  ti->ut.OVar;
		else if(ti->userTagType==eUTint)
		{
			if(ti->parentItem && ti->parentItem->userTagType==eUTdataClass)
			{
				CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(ti->parentItem->ut.userDataClass);
				if(aid)
				{
					OVar = aid->OVar;
					arrIndex=ti->ut.pInt;
				}
			}
		}
	}

	if(OVar)
	{
		if(OVar->EditType != eKETautomaticGr)
			treeItemPictureSize=-6;

		int valueType=OVar->EditType;	// !@#$ tag rozhodujici jaka hodnota se ma editovat (char/int/bool)

		if(OVar->LimintsCfg==eKLClist)	// limity jsou urceny vyctem hodnot
		{
			if(OVar->EditType == eKETautomaticGr && (OVar->Type==eKTname || OVar->Type==eKTarrName))
			{
				// pokud se jedna o automatismus => pridej polozku DefAuto
				CKerName* defAuto = KnownNames[eKKNdefaultAuto].Name;
				CGUITreeItem *nti = AddTreeItem(0,defAuto->GetUserName());
				nti->userTagType=eUTopointer;
				nti->ut.kerName=defAuto;
			}

			for(int i=0;i<OVar->LimitsListCount;i++)
			{
				CKerValue* val = &(OVar->LimitsList[i]);
				char string[261];
				string[0]=0;

				switch(OVar->Type)
				{
					case eKTchar :
						if(valueType==eKETdefault || valueType==eKETcharacter)
						{
							_snprintf(string,260,"%c",val->Dchar);
						}
						else if(valueType==eKETnumeric || valueType==eKETboolean)
						{
							int a = val->Dchar;
							_snprintf(string,260,"%d",a);
						}
						AddTreeItem(0,string);
						break;
					case eKTdouble :
						_snprintf(string,260,"%lf",val->Ddouble);
						AddTreeItem(0,string);
						break;
					case eKTint :
						_snprintf(string,260,"%d",val->Dint);
						AddTreeItem(0,string);
						break;
					case eKTobject :
					case eKTname :
						// vycet pripustnych CKerName
						{
							CKerName* kn = val->Dname;
							if(kn)
							{
								CGUITreeItem* ni = AddTreeItem(0,kn->GetUserName());
								ni->userTagType = eUTopointer;
								ni->ut.kerName = kn;
							}
							else{
								strcpy(string,"0");
								CGUITreeItem* ni = AddTreeItem(0,string);
								ni->userTagType = eUTopointer;
								ni->ut.kerName = 0;
							}
						}
						break;
					case eKTarrChar :
						if(valueType==eKETdefault || valueType==eKETcharacter)
						{
							_snprintf(string,260,"%c",val->Dchar);
						}
						else if(valueType==eKETnumeric || valueType==eKETboolean)
						{
							int a = val->Dchar;
							_snprintf(string,260,"%d",a);
						}
						AddTreeItem(0,string);
						break;
					case eKTarrDouble :
						_snprintf(string,260,"%lf",val->Ddouble);
						AddTreeItem(0,string);
						break;
					case eKTarrInt :
						_snprintf(string,260,"%d",val->Dint);
						AddTreeItem(0,string);
						break;
					case eKTarrObject :
					case eKTarrName :
						// vycet pripustnych CKerName
						{
							CKerName* kn = val->Dname;
							if(kn)
							{
								CGUITreeItem* ni = AddTreeItem(0,kn->GetUserName());
								ni->userTagType = eUTopointer;
								ni->ut.kerName = kn;
							}
							else{
								strcpy(string,"0");
								CGUITreeItem* ni = AddTreeItem(0,string);
								ni->userTagType = eUTopointer;
								ni->ut.kerName = 0;
							}
						}
						break;
					default: 
						if(OVar->Type>=eKTstring && OVar->Type<eKTstring+250) // jedna se o string
						{
							_snprintf(string,260,"%d",val->Dstring);
							AddTreeItem(0,string);
						}
				}
			}
		}
		else	// limity nejsou urceny vyctem
		{	// limity jsou urceny pomoci CKerName, ktera urcuje mnozinu moznych objektu / jmen
			if((KerMain)&&(OVar->Type == eKTarrObject || OVar->Type == eKTarrName || OVar->Type == eKTobject || OVar->Type == eKTname))
			{	// pouziva je jen pro promenne typu eKTobject a eKTname
				int direction = 1;
				if(OVar->LimintsCfg==eKLCup)
					direction=0;
				int planar=0;
				if(OVar->Use & eKVUBplannarNames)
					planar=1;

				/*
				if(OVar->EditType == eKETautomaticGr)
					planar=0;

				if((OVar->Use >> KERVARUSESSHIFT) == eKVUaPicture)
					planar=0;	// jedna se o automatismy => udelej vrstvy
					*/

				int addNull=(OVar->Use & eKVUBincludeNull);	// zda pridavat polozku odpovidajici 0

				if(addNull)
				{
					// pokud se jedna o automatismus => pridej polozku DefAuto
					CGUITreeItem *nti = AddTreeItem(0,"0");
					nti->userTagType=eUTopointer;
					nti->ut.kerName=0;
				}

				if(OVar->EditType == eKETautomaticGr)
				{
					// pokud se jedna o automatismus => pridej polozku DefAuto
					CKerName* defAuto = KnownNames[eKKNdefaultAuto].Name;
					CGUITreeItem *nti = AddTreeItem(0,defAuto->GetUserName());
					nti->userTagType=eUTopointer;
					nti->ut.kerName=defAuto;
				}

				if(OVar->LimintsCfg==eKLCup || OVar->LimintsCfg==eKLCdown)	// je zadana mnozina pomoci CKerName
				{
					if(OVar->LimitsListCount>0)
					{
						CKerValue* val = &(OVar->LimitsList[0]);

						CGUITreeItem* inclusive=0;
						if(!(OVar->Use & eKVUBexclusive) 
							&& ((OVar->Type != eKTarrObject && OVar->Type != eKTobject) || !val->Dname || (val->Dname->Type == eKerNTobject && ((val->Dname->ObjectType->EditTag) & etbOutMap))))
						{
							if(val->Dname)
								inclusive = AddTreeItem(0,val->Dname->GetUserName());
							else
								inclusive = AddTreeItem(0,"0");

							inclusive->userTagType = eUTopointer;
							inclusive->ut.kerName = val->Dname;
						}

						if(planar)
						{
							CKerNameList* nl=0;
							if(OVar->Type == eKTarrObject ||  OVar->Type == eKTobject)
							{
								eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
								eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
								nl = KerMain->KerNamesMain->FindSet(val->Dname,&FindNameObject,eKerNTobject,direction);
							}
							else
							{
								int type=-1;
								if(OVar->EditType == eKETautomaticGr)
									type=eKerNTauto;

								nameMask = OVar->NamesMask;
								eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
								eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
								nl = KerMain->KerNamesMain->FindSet(val->Dname,&NameFilter,type,direction);
							}

							CKerNameList* l = nl;
							while(l)
							{
								CGUITreeItem* ni = AddTreeItem(0,l->name->GetUserName());
								ni->userTagType = eUTopointer;
								ni->ut.kerName = l->name;
								l = l->next;
							}

							if(nl)
								nl->DeleteAll();
						}
						else
						{
							if(OVar->Type == eKTarrObject ||  OVar->Type == eKTobject)
							{
								eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
								eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
								AddLayer(inclusive,KerMain->KerNamesMain->FindLayer(val->Dname,&FindNameObject,eKerNTobject,direction),eKerNTobject,direction,&FindNameObject);
							}
							else
							{
								int type=-1;
								if(OVar->EditType == eKETautomaticGr)
									type=eKerNTauto;

								nameMask = OVar->NamesMask;
								eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
								eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
								AddLayer(inclusive,KerMain->KerNamesMain->FindLayer(val->Dname,&NameFilter,type,direction),type,direction,&NameFilter);
							}
							rollOnClick=false;
						}
					}
				}
				else	// zadna mnozina neni zadana
				{
					if(OVar->EditType == eKETautomaticGr)
					{
						// pridej vrstvy jmen:
						CKerObject* ko = KerMain->Objs->GetObject(objProperty->obj);
						if(ko)
						{
							eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
							eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
							AddLayer(0,KerMain->KerNamesMain->FindLayer(ko->Type->AName,&NameFilter,eKerNTauto,1),eKerNTauto,1,&NameFilter);
						}
						rollOnClick=false;
					}
					else if(planar)
					{
						CKerNameList* nl=0;
						if(OVar->Type == eKTarrObject ||  OVar->Type == eKTobject)
						{
							eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
							eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
							nl = KerMain->KerNamesMain->FindSet(0,&FindNameObject,eKerNTobject,direction);
						}
						else
						{
							nameMask = OVar->NamesMask;
							eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
							eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
							nl = KerMain->KerNamesMain->FindSet(0,&NameFilter,-1,direction);
						}

						CKerNameList* l = nl;
						while(l)
						{
							CGUITreeItem* ni = AddTreeItem(0,l->name->GetUserName());
							ni->userTagType = eUTopointer;
							ni->ut.kerName = l->name;
							l = l->next;
						}

						if(nl)
							nl->DeleteAll();
					}
					else
					{
						if(OVar->Type == eKTarrObject ||  OVar->Type == eKTobject)
						{
							eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
							eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
							AddLayer(0,KerMain->KerNamesMain->FindLayer(0,&FindNameObject,eKerNTobject,direction),eKerNTobject,direction,&FindNameObject);
						}
						else
						{
							nameMask = OVar->NamesMask;
							eKVUBobjInMapPL = OVar->Use & eKVUBobjInMap;
							eKVUBobjOutMapPL = OVar->Use & eKVUBobjOutMap;
							AddLayer(0,KerMain->KerNamesMain->FindLayer(0,&NameFilter,-1,direction),-1,direction,&NameFilter);
						}
						rollOnClick=false;
					}

				}
			}
		}

		if(OVar->Type == eKTarrObject ||  OVar->Type == eKTobject || ((OVar->NamesMask & eEdNTobject) && (OVar->Type == eKTarrName || OVar->Type == eKTname)))
		{
			CGUIToolBarButton* tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(editor->tbbID[eEMselectObj-1]));
			if(tbb)
				tbb->Activate();

			editor->SetEditMod(eEMselectObj);
			objProperty->SetCursorSelectObj();

			if(mainGUI->key_shift && (OVar->Type == eKTarrObject ||  OVar->Type == eKTobject))
				DeletePreviousObject(KerMain->Objs->GetObject(objProperty->obj),OVar,arrIndex);
		}
	}else
		assert(false);


	Sort();

	UpdateTree();

	Switch(items->GetFromHead());

	MsgAcceptConsume(MsgKeyboard);

	AddToTrash();
}

CEDObjPropertyList::~CEDObjPropertyList()
{
	if(editor && objProperty)
	{
		CGUIToolBarButton* tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(editor->tbbID[eEMselectObj-1]));
		if(tbb)
			tbb->Deactivate();

		editor->objVarSelectionID=0;
		if(editor->GetEditMod()==eEMselectObj)
		{
			editor->SetEditMod(eEMnormal);
			//objProperty->SetNormalCursor();
		}
		if(editor->GetEditMod()==eEMselectObj || editor->GetEditMod()==eEMnormal)
			objProperty->SetNormalCursor();
	}
	if(objProperty)
		objProperty->ShowObjValue(propertyItem);
}

void CEDObjPropertyList::DeletePreviousObject(CKerObject* ko, CKerOVar* OVar, int arrIndex)
{
	OPointer pObj=0;
	if(!ko || !OVar)
		return;

	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrObject* pArray = *(CKerArrObject**)objProperty->GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pObj = pArray->Read(arrIndex-1);
		}
	}
	else
		pObj = *(OPointer*)objProperty->GetAccessToVar(ko,OVar);	// jedna se o jeden int


	CKerObject *kObj = KerMain->Objs->GetObject(pObj);
	if(kObj)
	{
		if(KerMain->MapInfo->IsObjInMap(pObj))
		{
			KerMain->MapInfo->RemoveObjFromMap(0,pObj);
		}else if(editor->globalObjects->FindObj(pObj)!=-1)
		{
			editor->RemoveGlobalObject(pObj);
		}

	}
	// smazat objekt 
	if(pObj)
	{
		KerMain->DeleteObject(0,pObj);
		pObj=0;
	}

	// zapise nulu do promenne
	if(arrIndex)	// jedna se o pole Intu (arrIndex je index do pole [1..n])
	{
		CKerArrObject* pArray = *(CKerArrObject**)objProperty->GetAccessToVar(ko,OVar);
		if(pArray)
		{
			pArray->Read(arrIndex-1) = 0;
		}
	}
	else
		*(OPointer*)objProperty->GetAccessToVar(ko,OVar) = 0;	// jedna se o jeden int

	objProperty->ShowObjValue(propertyItem);
}


int CEDObjPropertyList::TreeHandler(typeID treeItem, int index, int state)
{
	if((state!=0 && state!=1)||closeOnClick)
		return 1;

	//CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(treeItem);

	CKerOVar* OVar =  0;
	int arrIndex = 0;

	if(propertyItem)
	{
		if(propertyItem->userTagType==eUTOVar || propertyItem->userTagType==eUTOVarDEL)
			OVar =  propertyItem->ut.OVar;
		else if(propertyItem->userTagType==eUTint)
		{
			if(propertyItem->parentItem && propertyItem->parentItem->userTagType==eUTdataClass)
			{
				CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(propertyItem->parentItem->ut.userDataClass);
				if(aid)
				{
					OVar = aid->OVar;
					arrIndex=propertyItem->ut.pInt;
				}
			}
		}
	}

	if(!OVar)
		return 1;

	if(OVar->Type==eKTobject || OVar->Type==eKTname || OVar->Type==eKTarrObject || OVar->Type==eKTarrName)
	{
		CGUITreeItem* ti = GetObjFromID<CGUITreeItem>(treeItem);
		if(ti && ti->userTagType==eUTopointer)
		{
			if(OVar->Type==eKTobject || OVar->Type==eKTarrObject)
			{
				// OPointer
				// musim vytvorit novy objekt
				OPointer o = 0;
				if(KerMain)
				{
					if(ti->ut.kerName && ti->ut.kerName->Type==eKerNTobject)
					{
						o=KerMain->NewObject(0,ti->ut.kerName);

						if(ti->ut.kerName->ObjectType->EditTag & etbOutMap)
						{
							editor->AddLevelObject(o);
							editor->AddGlobalObject(o);
						}
					}
					objProperty->VarValueChanged(propertyItem,&o);
					objProperty->ShowObjValue(propertyItem);
				}
			}
			else
			{	// CKerName
				objProperty->VarValueChanged(propertyItem,ti->ut.kerName);
				objProperty->ShowObjValue(propertyItem);
			}
		}

	}
	else
	{
		if(index<OVar->LimitsListCount)
		{
			objProperty->VarValueChanged(propertyItem,&(OVar->LimitsList[index].Dint));
			objProperty->ShowObjValue(propertyItem);
		}
	}

	if(state || mouseClick)
		closeOnClick=true;
	else
		CloseWindow();


	return 1;
}

void CEDObjPropertyList::SetObject(OPointer obj)
{
	CKerOVar* OVar=0;

	if(propertyItem)
	{
		if(propertyItem->userTagType==eUTOVar || propertyItem->userTagType==eUTOVarDEL)
			OVar =  propertyItem->ut.OVar;
		else if(propertyItem->userTagType==eUTint)
		{
			if(propertyItem->parentItem && propertyItem->parentItem->userTagType==eUTdataClass)
			{
				CEDVarArrayItemData* aid = dynamic_cast<CEDVarArrayItemData*>(propertyItem->parentItem->ut.userDataClass);
				if(aid)
				{
					OVar = aid->OVar;
				}
			}
		}
	}

	if(KerMain && OVar && (OVar->Type == eKTarrName || OVar->Type == eKTname))
	{
		CKerName* name=KerMain->GetObjType(obj);
		objProperty->VarValueChanged(propertyItem,name);
	}
	else
	{
		objProperty->VarValueChanged(propertyItem,&obj);
	}

	objProperty->ShowObjValue(propertyItem);
}

int CEDObjPropertyList::Keyboard(UINT ch, UINT state) //klavesnice
{
	int st = CGUITree::Keyboard(ch,state);
	
	if(ch==VK_ESCAPE)
	{
		CloseWindow();
		st=0;
	}
	
	mouseClick = false;

	return st;
}

int CEDObjPropertyList::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	int st = CGUITree::MouseLeft(x,y,mouseState,keyState);

	mouseClick = true;

	if(closeOnClick && !mouseState)
		CloseWindow();

	return st;
}

void CEDObjPropertyList::AddLayer(CGUITreeItem* parrTI, CKerNameList* nl, int type, int direction, int (*FilterFce)(CKerName *name))
{
	CKerNameList* l=nl;
	CGUITreeItem* nti=0;

	while(l)
	{
		CTexture* objTex=0;
		bool releaseTex=false;

		if(type==eKerNTauto && objectList)
			objectList->PrepareAutoTexture(l->name,&objTex,releaseTex);

		if(parrTI)
		{
			if(objTex)
				nti = parrTI->AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex));
			else
				nti = parrTI->AddTreeItem(0,l->name->GetUserName());
			nti->userTagType=eUTopointer;
			nti->ut.kerName=l->name;
		}
		else
		{
			if(objTex)
				nti = AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex));
			else
				nti = AddTreeItem(0,l->name->GetUserName());
			nti->userTagType=eUTopointer;
			nti->ut.kerName=l->name;
		}

		if(releaseTex && objTex)
			objTex->Release();


		if(KerMain)
			AddLayer(nti,KerMain->KerNamesMain->FindLayer(l->name,FilterFce,type,direction),type,direction,FilterFce);
		l = l->next;
	}

	if(nl)
		nl->DeleteAll();

	if(parrTI)
		parrTI->Sort();
	else
		Sort();
}

int CEDObjPropertyList::FindNameObject(CKerName* kn)
{
	if(!kn || !kn->ObjectType)
		return 0;

	//if(kn->Type == eKerNTobject && (((kn->ObjectType->EditTag) & etbOutMap) || ((kn->ObjectType->EditTag) & etbInMap)))
	//	return 1;

	if(kn->Type == eKerNTobject)
	{
		if(eKVUBobjInMapPL && !(kn->ObjectType->EditTag & etbInMap))
			return 0;
		if(eKVUBobjOutMapPL && !(kn->ObjectType->EditTag & etbOutMap))
			return 0;

		return 1;
	}
	else
		return 0;
}


int CEDObjPropertyList::NameFilter(CKerName* kn)
{
	if(!kn)
		return 0;

	if(kn->Type == eKerNTobject && kn->ObjectType)
	{
		if(eKVUBobjInMapPL && !(kn->ObjectType->EditTag & etbInMap))
			return 0;
		if(eKVUBobjOutMapPL && !(kn->ObjectType->EditTag & etbOutMap))
			return 0;
	}

	int bit = 1 << (kn->Type);

	if (kn == KnownNames[eKKNeverything].Name || kn == KnownNames[eKKNnothing].Name)
	{
		if ((1 << (eKerNTobjectVoid)) & nameMask) 
			return 1;
	}

	if(bit & nameMask)
		return 1;
	else
		return 0;
}



//////////////////////////////////////////////////////////////////////
// CEDNoConnectDialog
//////////////////////////////////////////////////////////////////////

CEDNoConnectDialog::CEDNoConnectDialog(OPointer _obj)
: CGUIToolBar(0,0,10,10,"NoConnect Settings",0)
{
	RemoveFromTrash();

	CGUIElement* el = GetObjFromID<CGUIToolBar>(editor->noConnectDialogID);
	if(el)
		desktop->DeleteBackElem(el);

	obj=_obj;

	SetProperties(5,12,5,12,true,true,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
		styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

	SetFreeSize(3*43+40,9*40+140,3*43+40,9*40+140);

	typeID butts[27];
	float tx=20,ty=30;

	for(int i=0;i<27;i++)
	{
		butts[i] = SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("NCD_Connect")->GetTexture(0),32,32),new CGUIRectHost(0,0,styleSet->Get("NCD_NoConnect")->GetTexture(0),32,32),0,tx,ty);
		tx += 43;

		if((i+1) % 3 == 0)
		{
			tx=20;
			ty+=40;
		}
		if((i+1) % 9 == 0)
		{
			ty+=30;
		}
	}

	ReMapButtonsIndex(butts);

	CGUIFont *font = (CGUIFont*) RefMgr->Find("GUI.F.Arial.10");
	AddBackElem(new CGUIStaticText("Up:",font,10,10,0xFF000000));
	AddBackElem(new CGUIStaticText("Base:",font,10,160,0xFF000000));
	AddBackElem(new CGUIStaticText("Down:",font,10,310,0xFF000000));


	CGUIButton *ok, *cancel;
	ok = new CGUIButton(20,465,50,25,"GUI.But.Std","OK");
	ok->SetTabOrder(1);
	ok->SetMark(1);
	ok->AcceptEvent(GetID(),EClicked);
	buttOK=ok->GetID();
    AddBackElem(ok);

	cancel = new CGUIButton(100,465,50,25,"GUI.But.Std","Storno");
	cancel->SetTabOrder(2);
	cancel->SetMark(1);
	cancel->AcceptEvent(GetID(),EClicked);
	buttCancel=cancel->GetID();
    AddBackElem(cancel);

	SetFocusEl(ok,0);


	LoadConfiguration();

	editor->noConnectDialogID = GetID();

	if(background)
		background->SetCol(STD_TREE_BGCOLOR_COLOR);

	desktop->AddBackElem(this);
	//desktop->SetFocusEl(this,0);
	Center();

	int *ncMask=0;
	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(ko)
		ncMask = (int*)KerMain->GetAccessToVar3(ko,eKVUaNoConnect);

	if(!ncMask)
		CloseWindow();
}

CEDNoConnectDialog::~CEDNoConnectDialog()
{
	/*
	if(editor)
		editor->noConnectDialogID=0;
		*/
}

void CEDNoConnectDialog::ReMapButtonsIndex(typeID butts[27])
{
	buttons[0] = butts[12];
	buttons[1] = butts[16];
	buttons[2] = butts[14];
	buttons[3] = butts[10];
	buttons[4] = butts[15];
	buttons[5] = butts[17];
	buttons[6] = butts[11];
	buttons[7] = butts[9];
	buttons[8] = butts[13];
	buttons[9] = butts[4];
	buttons[10] = butts[22];
	buttons[11] = butts[3];
	buttons[12] = butts[7];
	buttons[13] = butts[5];
	buttons[14] = butts[1];
	buttons[15] = butts[21];
	buttons[16] = butts[25];
	buttons[17] = butts[23];
	buttons[18] = butts[19];
	buttons[19] = butts[6];
	buttons[20] = butts[8];
	buttons[21] = butts[2];
	buttons[22] = butts[0];
	buttons[23] = butts[24];
	buttons[24] = butts[26];
	buttons[25] = butts[20];
	buttons[26] = butts[18];
}

void CEDNoConnectDialog::LoadConfiguration()
{
	if(!KerMain)
		return;

	int *ncMask=0;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(ko)
		ncMask = (int*)KerMain->GetAccessToVar3(ko,eKVUaNoConnect);

	if(ncMask && *ncMask)
	{
		int msk = *ncMask;
		CGUIToolBarButton* tbb;
		
		for(int i=0;i<27;i++)
		{
			if(msk & 1)
			{
				tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(buttons[i]));
				if(tbb)
					tbb->ChangeState(true);
			}
			else
			{
			}

			msk >>=1;
			if(!msk)
				break;
		}
	}
}

void CEDNoConnectDialog::SaveConfiguration()
{
	if(!KerMain)
		return;

	int *ncMask=0;

	CKerObject* ko = KerMain->GetAccessToObject(obj);
	if(ko)
		ncMask = (int*)KerMain->GetAccessToVar3(ko,eKVUaNoConnect);

	if(ncMask)
	{
		int msk = 0;
		CGUIToolBarButton* tbb;
		
		for(int i=0;i<27;i++)
		{
			tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(buttons[i]));
			if(tbb)
			{
				if(tbb->GetState())
				{
					msk |= (1 << i);
				}
				else
				{

				}
			}
			else
				assert(false);
		}

		*ncMask = msk;

		KerServices.ResetAuto(obj,0,1);
	}
}

void CEDNoConnectDialog::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EClicked)
	{
		if(event->sender == buttOK)
		{
			SaveConfiguration();
			CloseWindow();
		}
		else if(event->sender == buttCancel)
			CloseWindow();
	}

//	delete event;
	CGUIWindow::EventHandler(event);
}



//////////////////////////////////////////////////////////////////////
// CEDEditWindow
//////////////////////////////////////////////////////////////////////

CEDEditWindow::CEDEditWindow(float _x, float _y, float _sx, int textColor, int bgColor, char *fontname)
: CGUIEditWindow(_x,_y,_sx,textColor,bgColor,fontname)
{
}



int CEDEditWindow::FocusChanged(UINT focus, typeID dualID)
{
	if(focused!=focus) 
	{
		text->DoneUndo();

		focused = focus;

		if(selectonfocus && focused)
		{
			text->UnMark();
			text->GetCursor()->MoveTextEnd();
			text->SetMarkStart();
			text->GetCursor()->MoveTextHome();
			text->SetMarkEnd();
			ScrollToCursor();
		}
		
		blttext->BlinkCursor(focused);
		blttext->SetShowMark(focused);

		if(focused)
		{
			if(!cursorblinktimerID )
				cursorblinktimerID = timerGUI->AddRequest(this,.5);
		}else{
			timerGUI->DeleteRequest(cursorblinktimerID);cursorblinktimerID=0;
			timerGUI->DeleteRequest(mousescrolltimerID);mousescrolltimerID=0;
		}

		if(!focus && changed )
		{
			///*
			if(editor)
			{
				CEDObjectProperty* ID(op,editor->objPropertyID);
				if(op)
					op->UpdateEditedVar();
			}
			//*/
			EventArise(EOk);
			changed = 0;
		}
	}

	return CGUIStdWindow::FocusChanged(focus,dualID);
}




//////////////////////////////////////////////////////////////////////
// CEDASelector
//////////////////////////////////////////////////////////////////////

CEDASelector::CEDASelector(CKerObjectT* _objType, float _x, float _y, float _sx, float _sy)
: CGUITree(_x,_y,_sx,_sy,"AList",new CGUIRectHost(0,0,styleSet->Get("BUT_ObjectBrowser_Up")->GetTexture(0)))
{
	RemoveFromTrash();

	MsgAcceptConsume(MsgKeyboard);

	rollOnClick=false;

	objType = _objType;

	if(KerMain)
	{
		CKerNameList* nl;
		nl = KerMain->KerNamesMain->FindLayer(0,&FindNameObject,eKerNTauto,1); 
		AddLayerWithAutos(0,nl);
		UpdateTree();
	}

	editor->aSelectorID=GetID();

	AddToTrash();
}

CEDASelector::~CEDASelector()
{
	if(editor)
		editor->aSelectorID=0;
}

int CEDASelector::FindNameObject(CKerName* kn)
{
	return 1;

	/*
	if(!kn || !kn->ObjectType)
		return 0;

	if((kn->ObjectType->EditTag) & etbPodu)
		return 1;
	else
		return 0;
		*/
}

void CEDASelector::AddLayerWithAutos(CGUITreeItem* parrTI, CKerNameList* nl)
{
	CKerNameList* l=nl;
	CGUITreeItem* nti=0;

	while(l)
	{
		CTexture* objTex;
		bool releaseTex=false;
		PrepareAutoTexture(l->name,&objTex,releaseTex);
		
		if(parrTI)
			nti = parrTI->AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex));
		else
			nti = AddTreeItem(0,l->name->GetUserName(),new CGUIRectHost(0,0,objTex));

		if(releaseTex && objTex)
			objTex->Release();
		nti->userTagType=eUTautomatism;
		nti->ut.aKerName=l->name;
		nti->ut.objTI=0;

		if(KerMain)
			AddLayerWithAutos(nti,KerMain->KerNamesMain->FindLayer(l->name,0,eKerNTauto,1));
		l = l->next;
	}

	if(nl)
		nl->DeleteAll();
}

void CEDASelector::PrepareAutoTexture(CKerName* aName, CTexture** objTex, bool& releaseTex)
{
	releaseTex=false;

	if(aName)
	{
		if(aName->Auto->EditTex)
			*objTex=aName->Auto->EditTex;
		else
		{
			if(aName->Auto->Type == eKATpicture && aName->Auto->SonsNum>0)
			{
				CLightTex* lt = dynamic_cast<CLightTex*>(aName->Auto->textures[0].texture);
				if(lt)
				{
					if(lt && lt->CmpTex(objTex))
						releaseTex=true;
				}
				else
				{
					CGEnAnim* anim = dynamic_cast<CGEnAnim*>(aName->Auto->textures[0].texture);
					lt=0;
					if(anim)
						lt=anim->GetFrameTex(0);
					if(lt && lt->CmpTex(objTex))
						releaseTex=true;
				}
			}
			else
				*objTex = styleSet->Get("STD_AUTO_TEX")->GetTexture(0);
		}
	}
	else
	{
		*objTex = styleSet->Get("STD_AUTO_TEX")->GetTexture(0);
	}
}

int CEDASelector::TreeHandler(typeID treeItem, int index, int state)
{
	if(state!=0) //==3
		return 1;

	CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(treeItem));
	if(ti && ti->userTagType==eUTautomatism && KerMain)
	{
		if(KerMain->AutosMain->GetSaveAutosState())
		{
			KerMain->AutosMain->AddEasyAuto(objType,ti->ut.aKerName);
			CEDUserAnnouncer::Announce("A selected",5);

//			if(objectList)
//				objectList->RefreshAutos(objType->Name);

			CEDObjectProperty* ID(op,editor->objPropertyID);
			if(op)
			{
				CKerObject* ko = KerMain->Objs->GetObject(op->obj);
				if(ko && ko->Type->DefAuto)
				{
					op->SetErrorMsg(0);
				}

				op->PrepareObjCursor();
			}

		}
		CloseWindow();
	}

	return 1;
}

int CEDASelector::Keyboard(UINT ch, UINT state) //klavesnice
{
	int st = CGUITree::Keyboard(ch,state);
	
	if(st && ch==VK_ESCAPE)
	{
		CloseWindow();
		st=0;
	}
	
	return st;
}

