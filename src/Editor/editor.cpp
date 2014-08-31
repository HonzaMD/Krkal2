///////////////////////////////////////////////
//
// editor.cpp
//
// Implementation pro hlavni objekt editoru
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#include "stdafx.h"
#include "editor.h"
#include "desktop.h"
#include "editorLevelWin.h"
#include "ObjProperty.h"
#include "editorMenu.h"
#include "primitives.h"

#include "kernel.h"
#include "autos.h"
#include "names.h"
#include "KerErrors.h"
#include "KerServices.h"
#include "objects.h"
#include "krkal.h"
#include "levelmap.h"
#include "fs.h"
#include "genmain.h"
#include "ComConstants.h"
#include "ident.h"

#include <string.h>

CEDEditor* editor=0;

//////////////////////////////////////////////////////////////////////
// CEDEditor
//////////////////////////////////////////////////////////////////////

CEDEditor::CEDEditor()
{
	editor = this;
	editMode = 0;
	lastEditMode = editMode;
	continuousMod = 1;
	replaceMod = 0;
	freeMod = 0;
	sameTypeMod = 0;
	largeMod = 0;
	selectLocationMod = 0;

	iconDefaultHandler = new CEDEditorIconHandler(1);
	propertyTB=0;
	globalObjsTB=0;
	gameWindowID=0;
	cellObjectsListID=0;
	debugWindowID=0;
	noConnectDialogID=0;
	aSelectorID=0;

	replaceLimit=0;

	objPropertyID = objVarSelectionID = 0;

	globalObjects = new CGrowingArray<OPointer>;

	tbbID[0] = tbbID[1] = tbbID[2] = tbbID[3] = tbbID[4] = tbbID[5] = tbbID[6] = tbbID[7] = 0;

	globalObjsNoDeleteIndex=0;
}

CEDEditor::~CEDEditor()
{
	SAFE_DELETE(globalObjects);
	delete iconDefaultHandler;
	editor=0;

	int size = shortCutArray.GetSize();
	for(int i = 0; i < size; i++)
	{
		delete shortCutArray.Remove(i);
	}
}

void CEDEditor::Init(char* level, char* script)
{
	try{
		if(GEnMain)
			GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

		if(level)
			LoadLevel(level);
		else if(script)
			LoadScript(script);

#ifdef EDITOR_GAME_MOD
		if(!level && !script)
		{
			LoadLevel("jedna_44A0_721C_001C_BF01.lv\\!level");
			CreateObjectTree();
			LoadGlobalObjects();
			CreateGlobalObjectsToolBar();
			CreateEditorPropertyWindow();
			CreateGameWindow();
			LoadShortCuts();
		}
#endif

		if(level || script)
		{
			CreateObjectTree();
			LoadGlobalObjects();
			CreateGlobalObjectsToolBar();
			CreateEditorPropertyWindow();
			CreateGameWindow();
			LoadShortCuts();
		}
		else
		{
#ifndef EDITOR_GAME_MOD
			((CEDEditorDesktop*)desktop)->editorMenuHandler->CreateNewDialog();
#endif
		}

	} catch (CKernelPanic){
		KernelShutDown();
		//KerErrorFiles->ShowAllLogs(100,90,640,600);
		//SAFE_DELETE(KerMain);
	}
}

void CEDEditor::InitScript(char *script)
{
	try{
		if(GEnMain)
			GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

		LoadScript(script);
		CreateObjectTree();
		LoadGlobalObjects();
		CreateGlobalObjectsToolBar();
		CreateEditorPropertyWindow();
		CreateGameWindow();

		mainGUI->SetEditorScriptInit(script);
	} catch (CKernelPanic){
		KernelShutDown();
		//KerErrorFiles->ShowAllLogs(100,90,640,600);
		//SAFE_DELETE(KerMain);
	}
}

void CEDEditor::InitLevel(char *level)
{
	try{
		if(GEnMain)
			GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)

		LoadLevel(level);
		CreateObjectTree();
		LoadGlobalObjects();
		CreateGlobalObjectsToolBar();
		CreateEditorPropertyWindow();
		CreateGameWindow();
		LoadShortCuts();

		mainGUI->SetEditorLevelInit(level);

	} catch (CKernelPanic){
		KernelShutDown();
		//KerErrorFiles->ShowAllLogs(100,90,640,600);
		//SAFE_DELETE(KerMain);
	}
}

void CEDEditor::ClearLevel()
{
	if(!KerMain)
		return;

	FS->ChangeDir("$GAME$");
	if(!FS->ChangeDirByFile(KerMain->LevelInfo.LevelFile))
		return;

	char* levelName = newstrdup(KerMain->LevelInfo.LevelFile);

	char* skriptName = newstrdup(KerMain->LevelInfo.ScriptsFile);

	CloseLevel();
	InitScript(skriptName);

	SAFE_DELETE_ARRAY(skriptName);

	

	if(KerMain)
	{
		KerMain->LevelInfo.LevelFile = levelName;
	}
}

void CEDEditor::CloseLevel()
{
	try{

		// zruseni herniho okna zobrazujiciho mapu
		DeleteGameWindow();

		// zruseni ToolBaru s globalnimi objekty
		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(globalObjsTB));
		if(tb)
			desktop->DeleteBackElem(tb);
		globalObjsTB=0;
		globalObjsTBPosition.SetPosition(0,0,0,0);

		// vymazani pole globalnich objektu levelu
		SAFE_DELETE(globalObjects);
		globalObjects = new CGrowingArray<OPointer>;

		// zruseni seznamu objektu levelu (skriptu) - seznam objektu k pouziti 
		if(objectList)
			desktop->DeleteBackElem(objectList);
		listPosition.SetPosition(0,0,0,0);

		((CEDEditorDesktop*)desktop)->CloseAllObjPropertyWins();
		objPropertyPosition.SetPosition(0,0,0,0);

		// zruseni ToolBaru s nastavenim editoru
		tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(propertyTB));
		if(tb)
			desktop->DeleteBackElem(tb);
		propertyTB=0;
		editorSettingsPosition.SetPosition(0,0,0,0);

		// zruseni okna s debug hlaskama
		CGUIWindow* dw = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(debugWindowID));
		if(dw)
			desktop->DeleteBackElem(dw);
		debugWindowID=0;
		debugWinPosition.SetPosition(0,0,0,0);

		// vyprazdneni pole ShortCutu
		int size = shortCutArray.GetSize();
		for(int i = 0; i < size; i++)
		{
			delete shortCutArray.Remove(i);
		}
		shortCutArray.Rebuild();

		// zruseni Kernelu s naloudovanym skriptem/levlem
		SAFE_DELETE(KerMain);

	} catch (CKernelPanic){
		KerErrorFiles->ShowAllLogs(100,90,640,600);
		SAFE_DELETE(KerMain);
	}
}

void CEDEditor::LoadScript(char *script)
{
	//FS->ChangeDir("$GAME$");
	if(mainGUI->editorGameMod)
	{
		float dsx, dsy;
		desktop->GetSize(dsx,dsy);
		if(!((CEDEditorDesktop*)desktop)->gameModIndicator)
		{
			((CEDEditorDesktop*)desktop)->gameModIndicator = new CGUIStaticPicture(dsx-16-5,dsy-16-4,styleSet->Get("GM_Indicator")->GetTexture(0));
			desktop->AddFrontElem(((CEDEditorDesktop*)desktop)->gameModIndicator);
		}
		else
			((CEDEditorDesktop*)desktop)->gameModIndicator->SetVisible(1);


		KerCreateKernel(NORMAL_RUN, eKerDBDebug, 1, script);
	}
	else
	{
		if(((CEDEditorDesktop*)desktop)->gameModIndicator)
			((CEDEditorDesktop*)desktop)->gameModIndicator->SetVisible(0);

		KerCreateKernel(EDITOR_RUN, eKerDBDebug, 1, script);
	}
}

void CEDEditor::LoadLevel(char *level)
{
	//KerCreateKernel(EDITOR_RUN, eKerDBDebug, 0, "jedna_44A0_721C_001C_BF01.lv\\!level");
	if(mainGUI->editorGameMod)
	{
		float dsx, dsy;
		desktop->GetSize(dsx,dsy);
		if(!((CEDEditorDesktop*)desktop)->gameModIndicator)
		{
			((CEDEditorDesktop*)desktop)->gameModIndicator = new CGUIStaticPicture(dsx-16-5,dsy-16-4,styleSet->Get("GM_Indicator")->GetTexture(0));
			desktop->AddFrontElem(((CEDEditorDesktop*)desktop)->gameModIndicator);
		}
		else
			((CEDEditorDesktop*)desktop)->gameModIndicator->SetVisible(1);

		KerCreateKernel(NORMAL_RUN, eKerDBDebug, 0, level);
	}
	else
	{
		if(((CEDEditorDesktop*)desktop)->gameModIndicator)
			((CEDEditorDesktop*)desktop)->gameModIndicator->SetVisible(0);

		KerCreateKernel(EDITOR_RUN, eKerDBDebug, 0, level);
	}
}

void CEDEditor::SaveLevel()
{
	if(mainGUI->editorGameMod)	// level nelze ulozit v game modu - herni objekty nejsou v konzistentnim stavu
	{
		CEDUserAnnouncer::Announce("Can't save level in Game MOD",5);
		return;
	}

	if(KerMain)
	{
		try{
			FS->ChangeDir("$GAME$");
			if(KerMain->SaveLevel())
				CEDUserAnnouncer::Announce("Save Successful",5);
		} catch (CKernelPanic){
			KernelShutDown();
			//KerErrorFiles->ShowAllLogs(100,90,640,600);
			//SAFE_DELETE(KerMain);
		}
	}
}

void CEDEditor::LoadGlobalObjects()
{
	if(!KerMain)
		return;

	int numStaticObjsFiltered = 0;

	int num = KerMain->Objs->LoadedObjects.GetCount();	
	for(int i = 0; i < num; i++)
	{
		OPointer o = KerMain->Objs->LoadedObjects.Get(i);
		CKerObject *ko;
		if((ko = KerMain->GetAccessToObject(o))  && !KerMain->MapInfo->IsObjInMap(o))
		{
			if(i<KerMain->StaticObjectsCount && StaticObjectsFilter(ko))
				numStaticObjsFiltered++;
			else
				globalObjects->Add(o);
		}
	}

	globalObjsNoDeleteIndex=KerMain->StaticObjectsCount-numStaticObjsFiltered;	// pocet objektu, ktere se nesmi smazat 
															// jsou umisteny na zacatku seznamu
}

int CEDEditor::StaticObjectsFilter(CKerObject *ko)
{
	CKerOVar* OVar;

	int num = ko->Type->NumVars;

	for(int i = 0; i < num; i++)
	{
		OVar = &(ko->Type->OVars[i]);
		if(CEDObjectProperty::VarFilter(OVar))
			return 0;
	}

	return 1;
}

void CEDEditor::LoadShortCuts()
{
	int size = shortCutArray.GetSize();
	for(int i=0;i<size;i++)
	{
		CEDShortCut* sc = shortCutArray.Get(i);
		if(sc)
			objectList->AddShortCutObj(sc->obj,sc->name);
	}
	objectList->UpdateTree();
}

void CEDEditor::CreateGlobalObjectsToolBar()
{
	if(!KerMain)
		return;

	int num = globalObjects->GetSize();
	CGUIToolBar* tb = new CEDGlobalObjectsToolBar(0,0,640,300);
	tb->SetProperties(max(20,num+10),1,10,1,true,true,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
		styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

	float xx,yy;
	tb->GetSize(xx,yy);
	globalObjsTBPosition.SetPosition(0,0,listPosition.x,yy);
	tb->Resize(globalObjsTBPosition.sx,globalObjsTBPosition.sy);
	

	int k,l;

	tb->SetNextTool(new CGUIRectHost(0,0,styleSet->Get("GOTB_GlobalVars")->GetTexture(0),32,32),0,"GlobalVars",k,l);

	for(int i = 0; i < num; i++)
	{
		OPointer o = globalObjects->Get(i);
		CKerObject *ko=KerMain->Objs->GetObject(o);
		if(ko)   // && !KerMain->MapInfo->IsObjInMap(o)
		{
			char *name = ko->Type->Name->GetUserName();	

			// zjistit texturu pro objekt v ToolBaru
			bool releaseTex=false;
			CTexture* objTex = ko->Type->EditTex;
			if(!objTex)
			{
				CKerName* aName = KerMain->AutosMain->ProcessAuto(ko);
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
				objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);

			
			tb->SetNextTool(new CGUIRectHost(0,0,objTex,32,32),0,name,k,l);

			if(releaseTex && objTex)
				objTex->Release();
		}
		else if(o)
		{
			CTexture* objTex = styleSet->Get("EM_Delete")->GetTexture(0);
			tb->SetNextTool(new CGUIRectHost(0,0,objTex,32,32),0,"DeletedObject",k,l);
		}
	}

	if(objectList)
		tb->AcceptEvent(objectList->objHandler->GetID(),EToolBar);
	desktop->AddBackElem(tb);
	globalObjsTB = tb->GetID();
}

void CEDEditor::CreateObjectBrowser()
{
	if(!KerMain)
		return;

	float dsx, dsy;
	desktop->GetVPSize(dsx,dsy);
	CGUITreeItem* ti;
	CEDObjectBrowser* ob = new CEDObjectBrowser(dsx-250,0,250,400);
	desktop->AddBackElem(ob);

	for(int i = 0; i < KerMain->Objs->NumObjectT; i++)
	{
		char* name = KerMain->Objs->ObjectTypes[i].Name->GetUserName();
		ti = ob->AddTreeItem(0,name,new CGUIRectHost(0,0,styleSet->Get("OB_Object")->GetTexture(0)));
		for(int j = 0; j <KerMain->Objs->ObjectTypes[i].NumVars; j++)
		{
			name = KerMain->Objs->ObjectTypes[i].OVars[j].NameStr;
			name = KerServices.ConverNameToShort(name);
			//ti->AddTreeItem(0,KerMain->Objs->ObjectTypes[i].OVars[j].jmeno);
			ti->AddTreeItem(0,name,new CGUIRectHost(0,0,styleSet->Get("OB_Attribut")->GetTexture(0)));
		}

		for(int j = 0; j <KerMain->Objs->ObjectTypes[i].NumM; j++)
		{
			if(KerMain->Objs->ObjectTypes[i].Methods[j].Name)
				name = KerMain->Objs->ObjectTypes[i].Methods[j].Name->GetNameString();
			else
				name = KerMain->Objs->ObjectTypes[i].Methods[j].MethodName;
			name = KerServices.ConverNameToShort(name);
			ti->AddTreeItem(0,name,new CGUIRectHost(0,0,styleSet->Get("OB_Method")->GetTexture(0)));
		}
	}

	//((CEDEditorDesktop*)desktop)->objectBrowser = ob;
	ob->UpdateTree();

}

void CEDEditor::CreateObjectTree()
{
	float dsx, dsy;
	desktop->GetVPSize(dsx,dsy);

	listPosition.SetPosition(dsx-310,0,310,430);
	objPropertyPosition.SetPosition(listPosition.x,listPosition.sy,listPosition.sx,dsy-listPosition.sy);
	CEDObjectTree* ot = new CEDObjectTree(listPosition.x,listPosition.y,listPosition.sx,listPosition.sy);
	desktop->AddBackElem(ot);
	ot->LoadObjects();
	//ot->Center(true,false);
}

void CEDEditor::CreateGameWindow()
{
	float dsx, dsy;
	desktop->GetVPSize(dsx,dsy);

	mapPosition.SetPosition(0,max(globalObjsTBPosition.y+globalObjsTBPosition.sy,editorSettingsPosition.y+editorSettingsPosition.sy),listPosition.x,dsy-globalObjsTBPosition.sy-editorSettingsPosition.sy-debugWinPosition.sy);
	CEDLevelWindow* gw = new CEDLevelWindow(70,180,600,400);
	desktop->AddBackElem(gw);
	gameWindowID=gw->GetID();

	if(KerMain)
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
	}

	RefreshMapPosition();
}

void CEDEditor::RefreshMapVP(int dx, int dy)
{
	CEDLevelWindow* ID(gw,gameWindowID);
	if(gw)
	{
		gw->GameWindowScroll((float)dx,(float)dy);
		mainGUI->SendCursorPos();
	}
}

void CEDEditor::RefreshMapVPabs(int vx, int vy)
{
	CEDLevelWindow* ID(gw,gameWindowID);
	if(gw)
	{
		float nx, ny;
		float gsx,gsy;
		gw->GetVPPos(nx,ny);
		gw->GetShift(gsx,gsy);
		vx-=(int)nx-(int)gsx;
		vy-=(int)ny-(int)gsy;
		gw->GameWindowScroll((float)vx,(float)vy);
		mainGUI->SendCursorPos();
	}
}

void CEDEditor::RefreshMapPosition()
{
	float dsx, dsy;
	desktop->GetVPSize(dsx,dsy);

	mapPosition.SetPosition(0,max(globalObjsTBPosition.y+globalObjsTBPosition.sy,editorSettingsPosition.y+editorSettingsPosition.sy),listPosition.x,dsy-globalObjsTBPosition.sy-editorSettingsPosition.sy-debugWinPosition.sy);


	CEDLevelWindow* gw = GetObjFromID<CEDLevelWindow>(gameWindowID);
	if(gw)
	{
		gw->Resize(mapPosition.sx,mapPosition.sy);
		gw->Resize(mapPosition.sx,mapPosition.sy);	// resize podruhe kvuli spravne aktualizaci velikosti vzhledem ke scrollbarum

		float gsx,gsy;
		float gx,gy;
		gw->GetSize(gsx,gsy);		

		gx = floorf((mapPosition.sx - gsx) / 2);
		gy = floorf((mapPosition.sy - gsy) / 2);

		gw->Move(gx+mapPosition.x,gy+mapPosition.y);
	}
}

void CEDEditor::DeleteGameWindow()
{
	CEDLevelWindow* gw = GetObjFromID<CEDLevelWindow>(gameWindowID);
	if(gw)
		desktop->DeleteBackElem(gw);
	gameWindowID=0;
	if(GEnMain)
		GEnMain->DeleteAll();		// vyprazdni okno hry (mapa)
}

void CEDEditor::CreateDebugWindow()
{
	if(KerMain)
	{
		float dsx, dsy;
		desktop->GetVPSize(dsx,dsy);

		debugWinPosition.SetPosition(0,dsy-120,globalObjsTBPosition.sx,120);
		CGUIWindow* dw = KerMain->Errors->ShowRunLogWindow(5,debugWinPosition.x,debugWinPosition.y,debugWinPosition.sx,debugWinPosition.sy);
		if(dw)
		{
			debugWindowID = dw->GetID();
			dw->AcceptEvent(iconDefaultHandler->GetID(),EClose);
		}
	}
}

void CEDEditor::CreateEditorPropertyWindow()
{
	editorSettingsPosition.SetPosition(0,globalObjsTBPosition.sy,globalObjsTBPosition.sx,80);

	CGUIToolBar* tb = new CGUIToolBar(editorSettingsPosition.x,editorSettingsPosition.y,globalObjsTBPosition.sx,globalObjsTBPosition.sy,"EditorMod Settings",0);
	tb->SetProperties(10,1,10,1,true,true,43,40,styleSet->Get("ToolBarButton_Up"),styleSet->Get("ToolBarButton_MarkUp"),
		styleSet->Get("ToolBarButton_Down"),styleSet->Get("ToolBarButton_MarkDown"),styleSet->Get("ToolBarButton_Inactive"),true);

	tb->SetFreeSize(7*43+10,40,7*43+10,40);

	tbbID[eEMinsert-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_Insert")->GetTexture(0),32,32),0,"Insert     (I)",0,0);
	tbbID[eEMselect-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_SelectFree")->GetTexture(0),32,32),0,"Select     (S)",43,0);
	tbbID[eEMselectCell-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_SelectCell")->GetTexture(0),32,32),0,"Select Cell     (B)",86,0);
	tbbID[eEMnoConnect-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_NoConnect")->GetTexture(0),32,32),0,"NoConnect     (N)",129,0);
	tbbID[eEMdelete-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_Delete")->GetTexture(0),32,32),0,"Delete     (D)",172,0);
	
	tbbID[eEMmoveObj-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_MoveObj")->GetTexture(0),32,32),0,"Move     (M)",225,0);
	tbbID[eEMselectObj-1] = tb->SetFreeTool(new CGUIRectHost(0,0,styleSet->Get("EM_SelectObj")->GetTexture(0),32,32),0,"Select Obj2Var     (O)",268,0);
	

	CGUIToolBarButton* tbb;
	if(editMode)
	{
		tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[editMode-1]));
		if(tbb)
			tbb->ChangeState(true);
	}

	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[eEMselectObj-1]));
	if(tbb)
		tbb->Deactivate();


	/*
	switch(editMode)
	{
		case eEMnormal :	break;
		case eEMinsert :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[0]));
							if(tbb)
								tbb->ChangeState(true);
							break;
		case eEMselect :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[1]));
							if(tbb)
								tbb->ChangeState(true);
							break;
		case eEMdelete :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[2]));
							if(tbb)
								tbb->ChangeState(true);
							break;
	}
	*/

	float tsx,tsy;
	tb->GetSize(tsx,tsy);
	editorSettingsPosition.SetPosition(0,globalObjsTBPosition.sy,globalObjsTBPosition.sx,tsy);
	tb->Resize(editorSettingsPosition.sx,editorSettingsPosition.sy);

	tb->AcceptEvent(iconDefaultHandler->GetID(),EToolBar);
	tb->AcceptEvent(iconDefaultHandler->GetID(),EClose);

	desktop->AddBackElem(tb);

	editor->propertyTB=tb->GetID();
}

int CEDEditor::ResolveEditMod(UINT character)
{
	if(!KerMain)
		return 0;

	bool found=false;

	switch(character)
	{
		case 'I' :	// insert mod, cell/free podle typu objektu
		case '1' :
			SetEditMod(eEMinsert);
			if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
			{
				if(op->objAlreadyPlaced)
					op->CopyObjectProperty();
			}

			SetContinuousMod(1);
			SetReplaceMod(0);
			SetFreeModByObject();
			SetLargeMod(0);
			found=true;
			break;
		case 'F' :	// insert free mod
			SetEditMod(eEMinsert);
			if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
			{
				if(op->objAlreadyPlaced)
					op->CopyObjectProperty();
			}

			SetContinuousMod(1);
			SetReplaceMod(0);
			SetFreeMod(1);
			SetLargeMod(0);
			found=true;
			break;
		case 'C' :	// insert cell mod
			SetEditMod(eEMinsert);
			if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
			{
				if(op->objAlreadyPlaced)
					op->CopyObjectProperty();
			}

			SetContinuousMod(1);
			SetReplaceMod(0);
			SetFreeMod(0);
			SetLargeMod(0);
			found=true;
			break;
		case VK_ESCAPE : 
			if(editMode==eEMselect)
			{
				if(lastEditMode!=eEMselectObj)
					SetEditMod(lastEditMode);
				else
					SetEditMod(eEMnormal);

				switch(editMode)
				{
					case eEMinsert :	// insert mod, cell/free podle typu objektu
						if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
						{
							if(op->objAlreadyPlaced)
								op->CopyObjectProperty();
						}

						SetFreeModByObject();
						break;
					case eEMselect :	// select mod
						SetFreeMod(1);
						break;
					case eEMselectCell :	// select cell mod
						SetFreeMod(0);
						break;	
					case eEMdelete :	// delete mod
						SetFreeMod(1);
						break;
					case eEMnoConnect :	// no connect mod
						SetFreeMod(1);
						break;
					case eEMmoveObj :	// move obj mod
						if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
						{
							if(op->objAlreadyPlaced)
							{
								SetFreeModByObject();
							}
							else
								SetEditMod(eEMselect);

						}
						else
							SetEditMod(eEMselect);
						break;
				}

			}
			else
			{
				SetEditMod(eEMselect);

				SetFreeMod(1);
				SetContinuousMod(1);
			}
			found=true;
			break;
		case 'S' :	// select mod
			if(mainGUI->key_ctrl)
				break;
		case '2' :
			SetEditMod(eEMselect);

			SetFreeMod(1);
			SetContinuousMod(1);
			found=true;
			break;
		case 'B' :	// select cell mod
		case '3' :
			SetEditMod(eEMselectCell);

			SetFreeMod(0);
			SetContinuousMod(1);
			found=true;
			break;	
		case 'D' :	// delete mod
		case '5' :
			SetEditMod(eEMdelete);

			SetFreeMod(1);
			SetSameTypeMod(0);
			found=true;
			break;
		case 'N' :	// no connect mod
			if(mainGUI->key_ctrl)
				break;
		case '4' :
			SetEditMod(eEMnoConnect);

			SetFreeMod(1);
			SetContinuousMod(1);
			found=true;
			break;
		case 'M' :	// move obj mod
		case '6' :
			if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID))
			{
				if(op->objAlreadyPlaced)
				{
					SetEditMod(eEMmoveObj);
					SetContinuousMod(1);
					SetFreeModByObject();
					SetLargeMod(0);
				}
			}
			found=true;
			break;
		case 'O' :
		case '7' :
			{
				CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(editor->objVarSelectionID);
				if(opl)
					SetEditMod(eEMselectObj);
			}
			break;
		case 0 : 
		case VK_SHIFT :
		case VK_CONTROL:
			found=true;
			break;
	}

	if(found)
	{
		/*
		switch(GetEditMod())
		{
			case eEMinsert :
				/*
				if(mainGUI->key_shift)
					SetLargeMod(1);
				else
					SetLargeMod(0);
					// /

				if(mainGUI->key_ctrl)
					SetReplaceMod(1);
				else
					SetReplaceMod(0);

				break;

			case eEMmoveObj :
				if(mainGUI->key_ctrl)
					SetReplaceMod(1);
				else
					SetReplaceMod(0);

				break;
			/*
			case eEMdelete :
				if(mainGUI->key_shift)
					SetLargeMod(1);
				else
					SetLargeMod(0);

				if(mainGUI->key_ctrl)
					SetSameTypeMod(1);
				else
					SetSameTypeMod(0);

				break;
				// /

		}
*/

		if(mainGUI->key_shift)
			SetLargeMod(1);
		else
			SetLargeMod(0);

		if(mainGUI->key_ctrl)
		{
			SetReplaceMod(1);
			SetSameTypeMod(1);
		}
		else
		{
			SetReplaceMod(0);
			SetSameTypeMod(0);
		}


		CEDLevelWindow* gw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
		float cx,cy;
		mainGUI->cursor->GetPos(cx,cy);
		if(gw && gw->IsIn(cx,cy))
		{
			gw->cursorCellSet=false;
			mainGUI->SendCursorPos();
		}
		else
		{
			CEDGlobalObjectsToolBar* got = GetObjFromID<CEDGlobalObjectsToolBar>(editor->globalObjsTB);
			if(got && got->IsIn(cx,cy))
			{
				mainGUI->SendCursorPos();
			}
		}

	}

	return GetEditMod();
}

void CEDEditor::SetEditMod(int mod)
{
	CGUIToolBarButton* tbb;
	if(editMode)
	{
		tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[editMode-1]));
		if(tbb)
			tbb->ChangeState(false);
	}
	/*
	switch(editMode)
	{
		case eEMnormal :	break;
		case eEMinsert :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[0]));
							if(tbb)
								tbb->ChangeState(false);
							break;
		case eEMselect :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[1]));
							if(tbb)
								tbb->ChangeState(false);
							break;
		case eEMdelete :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[2]));
							if(tbb)
								tbb->ChangeState(false);
							break;
	}
	*/

	lastEditMode=editMode;

	ModSwitch(editMode,mod);
	editMode=mod;


	if(editMode)
	{
		tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[editMode-1]));
		if(tbb)
			tbb->ChangeState(true);
	}
	/*
	switch(editMode)
	{
		case eEMnormal :	break;
		case eEMinsert :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[0]));
							if(tbb)
								tbb->ChangeState(true);
							break;
		case eEMselect :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[1]));
							if(tbb)
								tbb->ChangeState(true);
							break;
		case eEMdelete :	tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tbbID[2]));
							if(tbb)
								tbb->ChangeState(true);
							break;
	}
	*/
}

void CEDEditor::ModSwitch(int oldMod, int newMod)
{

	if(oldMod==eEMselectLocation)
	{
		CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
		if(lw)
			lw->DeleteMarker();
	}

	if(newMod==eEMselectLocation)
	{
		CEDLevelWindow* lw = GetObjFromID<CEDLevelWindow>(editor->gameWindowID);
		if(lw)
			lw->cursorCellSet=false;
	}

	CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID);
	if(op)
	{

		if(newMod!=eEMselectCell && newMod!=eEMselectObj) //  && newMod!=eEMnormal
		{
			CEDObjPropertyList* opl = GetObjFromID<CEDObjPropertyList>(op->handler->icons[2]);
			if(opl)
				desktop->DeleteBackElem(opl);
		}
		CEDObjectProperty* opp = GetObjFromID<CEDObjectProperty>(op->handler->icons[3]);
		if(opp && opp!=op)
			desktop->DeleteBackElem(opp);

		if(op->parentObjPropertyID)
		{
			op->CloseWindow();
			//objectList->SetSelectedObject(op->parentObjPropertyID);
		}
	}


	if(newMod!=eEMselectObj ) // newMod!=eEMselectCell && && newMod!=eEMnormal
	{
		CGUIElement* el = GetObjFromID<CGUITree>(editor->cellObjectsListID);
		if(el)
			desktop->DeleteBackElem(el);
	}

	CGUIElement* el = GetObjFromID<CGUIToolBar>(editor->noConnectDialogID);
	if(el)
		desktop->DeleteBackElem(el);

}

void CEDEditor::SetContinuousMod(int mod)
{
	continuousMod=mod;
}

void CEDEditor::SetReplaceMod(int mod)
{
	if(replaceMod!=mod)
		replaceLimit = KerMain->Objs->GetCounter();

	replaceMod=mod;

	
	/*
	if(replaceMod && KerMain)
		replaceLimit = KerMain->Objs->GetCounter();
	else
		replaceLimit = 0;
		*/
}

void CEDEditor::SetFreeMod(int mod)
{
	freeMod = mod;
}

void CEDEditor::SetFreeModByObject()
{
	CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(objPropertyID);
	if(op)
	{
		CKerObject* ko = KerMain->GetAccessToObject(op->obj);

		if(ko && (ko->Type->EditTag & etbInMap) && (ko->Type->EditTag & etbNoGrid))
			SetFreeMod(1);
		else
			SetFreeMod(0);
	}
	else
		SetFreeMod(0);
}

void CEDEditor::SetSameTypeMod(int mod)
{
	sameTypeMod = mod;
}

void CEDEditor::SetLargeMod(int mod)
{
	largeMod = mod;
}

void CEDEditor::SetSelectLocationMod(int mod)
{
	selectLocationMod = mod;
}

void CEDEditor::AddLevelObject(OPointer obj)
{
	if(KerMain)
	{
		int pos = KerMain->Objs->LoadedObjects.Add(obj);
		if(pos >= 2*KerMain->Objs->LOShakeOffSize)
			KerMain->Objs->LOShakeOff();
	}
}

int CEDEditor::AddGlobalObject(OPointer obj)
{
	if(!KerMain)
		return -1;
	CKerObject *ko;
	if((ko = KerMain->Objs->GetObject(obj))  && !KerMain->MapInfo->IsObjInMap(obj))
	{

		int ret = globalObjects->Add(obj);

		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(globalObjsTB));
		if(tb)
		{
			int k,l;
			char *name = ko->Type->Name->GetUserName();

			// zjistit texturu pro objekt v ToolBaru
			bool releaseTex=false;
			CTexture* objTex = ko->Type->EditTex;
			if(!objTex)
			{
				CKerName* aName = KerMain->AutosMain->ProcessAuto(ko);
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
				objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);

			if(!tb->SetNextTool(new CGUIRectHost(0,0,objTex,32,32),0,name,k,l))
			{
				// objekt se jiz nevesel do toolbaru
				// byl ovsem jiz pridan do seznamu globalnich objektu
				// zrusim toolbar a udelam novy vetsi:
				desktop->DeleteBackElem(tb);
				CreateGlobalObjectsToolBar();	// pri vytvareni toolbaru se do nej umisti vsechny objekty ze seznamu gobalnich objektu
			}

			if(releaseTex && objTex)
				objTex->Release();
		}
		return ret;
	}else
		return -1;
}

int CEDEditor::RemoveGlobalObject(OPointer obj)
{
	int i = globalObjects->FindObj(obj);
	if(globalObjects->Remove(i))
	{
		globalObjects->ShakeDown();
		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(nameServer->ResolveID(globalObjsTB));
		if(tb)
		{
			desktop->DeleteBackElem(tb);
			CreateGlobalObjectsToolBar();
		}
	}

	return 0;
}


void CEDEditor::KernelShutDown()
{
	SAFE_DELETE(KerMain);
	CloseLevel();
	if(KerErrorFiles)
		KerErrorFiles->ShowAllLogs(100,90,640,600);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// CEDEditorIconHandler	- vyvolani programu/oken po stisknuti ikony (spusteni aplikace -> inicializacni kod)
//////////////////////////////////////////////////////////////////////////////////////////////////

void CEDEditorIconHandler::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EToolBar)	// obsluha EditorMod Settings
	{
		/*
		if(event->pID == editor->tbbID[0])
		{
			if(event->pFloat)
			{
				editor->editMode = eEMinsert;
			}else
				editor->editMode = eEMnormal;
		}else if(event->pID == editor->tbbID[1])
		{
			if(event->pFloat)
				editor->editMode = eEMselect;
			else
				editor->editMode = eEMnormal;
		}else if(event->pID == editor->tbbID[2])
		{
			if(event->pFloat)
				editor->editMode = eEMdelete;
			else
				editor->editMode = eEMnormal;
		}
		*/
		if(event->pFloat)
		{
			for(int i=0;i<7;i++)
			{
				if(event->pID == editor->tbbID[i])
				{
					editor->lastEditMode=editor->editMode;
					editor->ModSwitch(editor->editMode,i+1);
					editor->editMode = i+1;

					switch(editor->editMode)
					{
						case eEMinsert :	// insert mod, cell/free podle typu objektu
							editor->SetFreeModByObject();
							break;
						case eEMselect :	// select mod
							editor->SetFreeMod(1);
							break;
						case eEMselectCell :	// select cell mod
							editor->SetFreeMod(0);
							break;	
						case eEMdelete :	// delete mod
							editor->SetFreeMod(1);
							break;
						case eEMnoConnect :	// no connect mod
							editor->SetFreeMod(1);
							break;
						case eEMmoveObj :	// move obj mod
							if(CEDObjectProperty* op = GetObjFromID<CEDObjectProperty>(editor->objPropertyID))
							{
								if(op->objAlreadyPlaced)
								{
									editor->SetFreeModByObject();	
								}
								else
								{
									editor->SetEditMod(eEMinsert);
									editor->SetFreeModByObject();
									/*
									editor->ModSwitch(editor->editMode,eEMnormal);
									editor->editMode = eEMnormal;
									CGUIToolBarButton* tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(event->pID));
									if(tbb)
										tbb->ChangeState(false);
										*/

								}
							}
							else
							{
								editor->SetEditMod(eEMselect);
								editor->SetFreeMod(1);

								/*
								editor->ModSwitch(editor->editMode,eEMnormal);
								editor->editMode = eEMnormal;
								CGUIToolBarButton* tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(event->pID));
								if(tbb)
									tbb->ChangeState(false);
									*/
							}
							break;
					}

					break;
				}
			}
		}
		else
		{
			editor->lastEditMode=editor->editMode;
			editor->ModSwitch(editor->editMode,eEMnormal);
			editor->editMode = eEMnormal;
		}
	}

	if(event->eventID == EClose) 
	{
		if(event->sender == editor->propertyTB)
		{
			editor->editorSettingsPosition.SetPosition(0,0,0,0);
			editor->RefreshMapPosition();
		}
		else if(event->sender == editor->debugWindowID)
		{
			editor->debugWinPosition.SetPosition(0,0,0,0);
			editor->RefreshMapPosition();
		}
	}

	delete event;
}

CEDShortCut::CEDShortCut(OPointer _obj, char* _name)
{
	obj = _obj;
	name = newstrdup(_name);
}

CEDShortCut::~CEDShortCut()
{
	SAFE_DELETE_ARRAY(name);
}

