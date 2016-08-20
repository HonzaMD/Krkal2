///////////////////////////////////////////////
//
// GameMenu.cpp
//
//	implementace pro herni menu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "GameMenu.h"

#include "kernel.h"
#include "names.h"
#include "autos.h"
#include "LevelMap.h"
#include "genmain.h"
#include "primitives.h"

CGameMenu* CGameMenu::gameMenu=0;	// herni menu pro Krkala

//////////////////////////////////////////////////////////////////////
// CGameMenu
//////////////////////////////////////////////////////////////////////


#include "cache.h"

CGameMenu::CGameMenu()
: CGUIMultiWidget(0,0)
{
	RemoveFromTrash();

	numObjects = 0;
	withManik = false;
	withProgressBars = false;
	progressBar1 = progressBar2 = 0;
	manik = 0;
	manikObj = 0;
	manikObjShown = 0;
	manikX = 0; manikY = 0;
	compressedMenu = false;
	manikKeyText = 0;

	float dsx,dsy;
	desktop->GetVPSize(dsx,dsy);

	Resize(dsx,STD_GAMEMENU_HEIGHT);
	Move(0,dsy-STD_GAMEMENU_HEIGHT);
	MsgSetProduce(MsgNone);

	SetAnchor(0,1,1,1);

	CGUIRectHost* sp = new CGUIRectHost(0, 0, styleSet->Get("GameMenuRamp")->GetTexture(0), sx, sy);
	AddElem(sp);

	desktop->AddFrontElem(this);

	window->CreateCache(64,64,0,0,dsx,STD_GAMEMENU_HEIGHT);
	gameMenu=this;

}

CGameMenu::~CGameMenu()
{
	if(desktop)
		desktop->RemoveFrontElem(this);

	gameMenu=0;
}

int CGameMenu::CreateGameMenu()
{
	if(!gameMenu)
	{
		new CGameMenu();
		return 1;
	}

	return 0;
}


int CGameMenu::DeleteGameMenu()
{
	int ret=gameMenu ? 0 : 1;
	SAFE_DELETE(gameMenu);
	return ret;
}

int CGameMenu::Show()
{
	if(!gameMenu)
	{
		return 0;
	}

	gameMenu->SetVisible(1);

	return 1;
}

int CGameMenu::Hide()
{
	if(!gameMenu)
	{
		return 0;
	}

	gameMenu->SetVisible(0);

	return 1;
}


/*
void CGameMenu::InitKrkalGameMenu()
{
	assert(KerMain);

	CKerName* kn;
	kn = KerMain->KerNamesMain->GetNamePointer("_KSID_floor_0001_FFFF_0001_0001");
	if(kn)
		AddItem(kn);
	kn = KerMain->KerNamesMain->GetNamePointer("_KSID_wall_0001_FFFF_0001_0001");
	if(kn)
		AddItem(kn);
	kn = KerMain->KerNamesMain->GetNamePointer("_KSID_mina_0001_FFFF_0001_0001");
	if(kn)
		AddItem(kn);
	kn = KerMain->KerNamesMain->GetNamePointer("_KSID_oMagnet_0001_FFFF_0001_0001");
	if(kn)
		AddItem(kn);
	kn = KerMain->KerNamesMain->GetNamePointer("_KSID_oLaser_0001_FFFF_0001_0001");
	if(kn)
		AddItem(kn);

	Add2ProgressBars(STD_GAMEMENU_PROGRESSBAR1_COLOR,STD_GAMEMENU_PROGRESSBAR2_COLOR);

	Refresh();
}
*/



int CGameMenu::RefreshManikPicture(CKerObject *ko)
{
	if (!gameMenu)
		return -1;

	if (ko->thisO == gameMenu->manikObj)
		return SetManikPicture(ko->thisO);

	return 1;
}

int CGameMenu::SetManikPicture(OPointer obj, CKerName* key)
{
	if (!gameMenu)
		return -1;

	gameMenu->withManik = true;

	if (!gameMenu->manikKeyText && key && key->Type == eKerNTkey && key->KeyStruct->keyName && *key->KeyStruct->keyName) {
		gameMenu->manikKeyText = new CGUIStaticText(key->KeyStruct->keyName, (CGUIFont*)RefMgr->Find("GUI.F.Arial.14PX"), gameMenu->manikX + 42, gameMenu->manikY + 41, STD_GAMEMENU_COUNTCOLOR);
		gameMenu->AddElem(gameMenu->manikKeyText);
	}

	CTexture* objTex = 0;
	CKerObject* ko = KerMain->Objs->GetObject(obj);
	gameMenu->manikObj = obj;

	if (gameMenu->manikObjShown == obj) // kvuli perfu neaktualizuju kazdou zmenu automatizmu ;(
		return 1;

	if (ko && ko->PlacedInfo && ko->PlacedInfo->AktivAuto)
	{
		CKerName *aName = ko->PlacedInfo->AktivAuto;
		if (aName->Auto->SonsNum>0)
		{
			CLightTex* lt = dynamic_cast<CLightTex*>(aName->Auto->textures[0].texture);

			if (lt)
			{
				lt->CmpTex(&objTex);
			}
			else
			{
				CGEnAnim* anim = dynamic_cast<CGEnAnim*>(aName->Auto->textures[0].texture);
				lt = 0;
				if (anim)
					lt = anim->GetFrameTex(0);
				if (lt)
					lt->CmpTex(&objTex);
			}
		}
	}

	if (gameMenu->manik) {
		if (objTex) {
			gameMenu->manik->SetTexture(objTex);
			gameMenu->manik->SetVisible(1);
			gameMenu->manikObjShown = obj;
		}
		else {
			gameMenu->manik->SetVisible(0);
			gameMenu->manikObjShown = 0;
		}
	}
	else if (objTex) {
		gameMenu->manik = new CGUIRectHost(gameMenu->manikX, gameMenu->manikY, objTex);
		gameMenu->AddElem(gameMenu->manik);
		gameMenu->manikObjShown = obj;
	}

	if (objTex)
		objTex->Release();

	return 1;
}

int CGameMenu::AddItem(CKerName* item, CKerName* key)
{
	if(!gameMenu)
		return -1;

	assert(item && item->Type == eKerNTobject);
	gameMenu->numObjects++;

	CGameMenuObject* mo = new CGameMenuObject(item, key);
	gameMenu->AddElem(mo);
	return gameMenu->objArray.Add(mo);
}


int CGameMenu::AddItem(OPointer obj, CKerName* key)
{
	if(!gameMenu)
		return -1;

	assert(obj);
	gameMenu->numObjects++;

	CGameMenuObject* mo = new CGameMenuObject(obj, key);
	gameMenu->AddElem(mo);
	return gameMenu->objArray.Add(mo);
}


int CGameMenu::DeleteItem(int index)
{
	if(!gameMenu)
		return -1;

	CGameMenuObject* gmo = gameMenu->objArray.Remove(index);
	if(gmo)
	{
		gameMenu->numObjects--;
		gameMenu->DeleteElem(gmo);
		return 0;
	}

	return 1;
}

int CGameMenu::DeleteItem(CKerName* item)
{
	if(!gameMenu)
		return -1;

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo && gmo->GetName() == item)
		{
			gameMenu->objArray.Remove(i);
			gameMenu->numObjects--;
			return gameMenu->DeleteItem(i);
		}
	}

	return 1;
}

int CGameMenu::SetItem(CKerName* item, int index)
{
	if(!gameMenu)
		return -1;

	assert(item && item->Type == eKerNTobject);
	CGameMenuObject* gmo = gameMenu->objArray.Remove(index);
	if(gmo)
	{
		gameMenu->numObjects--;
		gameMenu->DeleteElem(gmo);
	}

	gameMenu->numObjects++;

	CGameMenuObject* mo = new CGameMenuObject(item, 0);
	gameMenu->AddElem(mo);

	return gameMenu->objArray.Set(mo,index);
}

void CGameMenu::Refresh()
{
	if(!gameMenu)
		return;

	bool changeCompress=false;
	float msx=gameMenu->sx;

	if(gameMenu->withProgressBars)
		msx-=130;
	int numObjects = gameMenu->numObjects;
	if (gameMenu->withManik)
		numObjects++;

	float dx = floorf(msx / numObjects);
	float xx = floorf(dx/2)-22;
	if(xx<0)
		xx=0;
	if(dx<65)
	{
		if(!gameMenu->compressedMenu)
			changeCompress=true;
		gameMenu->compressedMenu=true;
		
		if(gameMenu->withProgressBars)
			dx = floorf((gameMenu->sx - 80) / numObjects);
		else
			dx = floorf(gameMenu->sx / numObjects);

		if(dx<44)
			dx=44;
		xx = floorf(dx/2)-22;
		if(xx<0)
			xx=0;
	}
	else
	{
		if(gameMenu->compressedMenu)
			changeCompress=true;
		gameMenu->compressedMenu=false;
	}

	if(gameMenu->withProgressBars)
	{
		if(changeCompress)
		{
			int p1,p2;
			int c1,c2;
			p1 = gameMenu->progressBar1->GetProgress();
			p2 = gameMenu->progressBar2->GetProgress();
			c1 = gameMenu->progressBar1->GetColor();
			c2 = gameMenu->progressBar2->GetColor();
			gameMenu->DeleteElem(gameMenu->progressBar1);
			gameMenu->DeleteElem(gameMenu->progressBar2);
			gameMenu->AddElem(gameMenu->progressBar1 = new CGameMenuProgressBar(c1,gameMenu->compressedMenu));
			gameMenu->AddElem(gameMenu->progressBar2 = new CGameMenuProgressBar(c2,gameMenu->compressedMenu));
			gameMenu->progressBar1->SetProgress(p1);
			gameMenu->progressBar1->SetProgress(p2);
		}

		if(gameMenu->compressedMenu)
		{
			gameMenu->progressBar1->Move(5,3);
			gameMenu->progressBar2->Move(5,26);
			xx += 80;
		}
		else
		{
			gameMenu->progressBar1->Move(10,3);
			gameMenu->progressBar2->Move(10,26);
			xx += 120;
		}

	}

	if (gameMenu->withManik) {
		gameMenu->manikX = xx-8;
		gameMenu->manikY = -5;
		if (gameMenu->manik)
			gameMenu->manik->Move(gameMenu->manikX, gameMenu->manikY);
		if (gameMenu->manikKeyText)
			gameMenu->manikKeyText->Move(gameMenu->manikX + 42, gameMenu->manikY + 41);
		xx += dx;
	}

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo)
		{
			gmo->Move(xx,3);
			xx+=dx;
		}
	}

}

int CGameMenu::ObjectIncrement(int index)
{
	if(!gameMenu)
		return -1;

	CGameMenuObject* gmo = gameMenu->objArray.Get(index);
	if(gmo)
		return gmo->Increment();

	return -1;
}

int CGameMenu::ObjectIncrement(CKerName* item)
{
	if(!gameMenu)
		return -1;

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo =gameMenu-> objArray.Get(i);
		if(gmo->GetName() == item)
		{
			return gmo->Increment();
		}
	}

	return -1;
}

int CGameMenu::ObjectDecrement(int index)
{
	if(!gameMenu)
		return -1;

	CGameMenuObject* gmo = gameMenu->objArray.Get(index);
	if(gmo)
		return gmo->Decrement();

	return -1;
}

int CGameMenu::ObjectDecrement(CKerName* item)
{
	if(!gameMenu)
		return -1;

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo->GetName() == item)
		{
			return gmo->Decrement();
		}
	}

	return -1;
}

int CGameMenu::ObjectSetCount(int index, int count)
{
	if(!gameMenu)
		return -1;

	CGameMenuObject* gmo = gameMenu->objArray.Get(index);
	if(gmo)
		return gmo->SetCount(count);

	return -1;
}

int CGameMenu::ObjectSetCount(CKerName* item, int count)
{
	if(!gameMenu)
		return -1;

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo->GetName() == item)
		{
			return gmo->SetCount(count);
		}
	}

	return -1;
}

int CGameMenu::ObjectGetCount(int index)
{
	if(!gameMenu)
		return -1;

	CGameMenuObject* gmo = gameMenu->objArray.Get(index);
	if(gmo)
		return gmo->GetCount();

	return -1;
}

int CGameMenu::ObjectGetCount(CKerName* item)
{
	if(!gameMenu)
		return -1;

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo->GetName() == item)
		{
			return gmo->GetCount();
		}
	}

	return -1;
}

int CGameMenu::Add2ProgressBars(int color1, int color2)
{
	if(!gameMenu)
		return -1;

	gameMenu->withProgressBars=true;
	gameMenu->AddElem(gameMenu->progressBar1 = new CGameMenuProgressBar(color1));
	gameMenu->AddElem(gameMenu->progressBar2 = new CGameMenuProgressBar(color2));

	return 0;
}

int CGameMenu::SetProgressBar(int index, int progress)
{
	if(!gameMenu)
		return -1;

	if(index==1)
		return gameMenu->progressBar1->SetProgress(progress);
	else if(index==2)
		return gameMenu->progressBar2->SetProgress(progress);

	return 0;
}

int CGameMenu::GetProgressBar(int index)
{
	if(!gameMenu)
		return -1;

	if(index==1)
		return gameMenu->progressBar1->GetProgress();
	else if(index==2)
		return gameMenu->progressBar2->GetProgress();

	return 0;
}

//////////////////////////////////////////////////////////////////////
// CGameMenuObject
//////////////////////////////////////////////////////////////////////

CGameMenuObject::CGameMenuObject(CKerName* item, CKerName* key)
: CGUIMultiWidget(-1, -1, 56, 46)
{
	assert(item && item->Type == eKerNTobject);

	RemoveFromTrash();

	for (int f = 0; f < 12; f++)
		svetla[f] = 0;

	objName = item;

	CGUIRectHost *border = new CGUIRectHost(0, 0, styleSet->Get("GameMenuPozadi")->GetTexture(0));
	AddElem(border);

	CreateObjPicture();

	if (key && key->Type == eKerNTkey && key->KeyStruct->keyName && *key->KeyStruct->keyName) {
		CGUIStaticText *str = new CGUIStaticText(key->KeyStruct->keyName, (CGUIFont*)RefMgr->Find("GUI.F.Arial.14PX"), 47, 33, STD_GAMEMENU_COUNTCOLOR);
		AddElem(str);
	}

	count=0;

	AddToTrash();
}

CGameMenuObject::CGameMenuObject(OPointer obj, CKerName* key)
: CGUIMultiWidget(-1, -1, 46, 46)
{
	assert(obj && KerMain);

	RemoveFromTrash();

	for (int f = 0; f < 12; f++)
		svetla[f] = 0;

	CKerObject* ko = KerMain->Objs->GetObject(obj);

	objName = ko->Type->Name;

	CGUIRectHost *border = new CGUIRectHost(0, 0, styleSet->Get("GameMenuPozadi")->GetTexture(0));
	AddElem(border);

	CreateObjPicture(ko);

	if (key && key->Type == eKerNTkey && key->KeyStruct->keyName && *key->KeyStruct->keyName) {
		CGUIStaticText *str = new CGUIStaticText(key->KeyStruct->keyName, (CGUIFont*)RefMgr->Find("GUI.F.Arial.14PX"), 47, 33, STD_GAMEMENU_COUNTCOLOR);
		AddElem(str);
	}

	count = 0;

	AddToTrash();
}


CGameMenuObject::~CGameMenuObject()
{
}

int CGameMenuObject::Increment()
{
	count++;
	UpdateSvetlo(count - 1);
	return count;
}

int CGameMenuObject::Decrement()
{
	count--;
	UpdateSvetlo(count);
	return count;
}

int CGameMenuObject::SetCount(int _count)
{
	count = _count;

	for (int f = 0; f < 12; f++)
		UpdateSvetlo(f);

	return count;
}

int CGameMenuObject::GetCount()
{
	return count;
}


void CGameMenuObject::UpdateSvetlo(int pos) {
	if (pos >= 12 || pos < 0)
		return;

	if (pos < count) {
		if (!svetla[pos]) {
			float x, y;

			switch (pos) {
			case 0:
				x = -1; y = -1; break;
			case 1:
				x = 11; y = -1; break;
			case 2:
				x = 23; y = -1; break;
			case 3:
				x = 35; y = -1; break;
			case 4:
				x = 35; y = 11; break;
			case 5:
				x = 35; y = 23; break;
			case 6:
				x = 35; y = 35; break;
			case 7:
				x = 23; y = 35; break;
			case 8:
				x = 11; y = 35; break;
			case 9:
				x = -1; y = 35; break;
			case 10:
				x = -1; y = 23; break;
			case 11:
				x = -1; y = 11; break;
			}

			svetla[pos] = new CGUIRectHost(x, y, styleSet->Get("GameMenuSvetlo")->GetTexture(0));
			AddElem(svetla[pos]);
		}
		else if (!svetla[pos]->IsVisible()) {
			svetla[pos]->SetVisible(1);
		}
	}
	else if (svetla[pos] && svetla[pos]->IsVisible()){
		svetla[pos]->SetVisible(0);
	}
}



void CGameMenuObject::CreateObjPicture(CKerObject *ko)
{
	assert(KerMain);

	bool releaseTex=false;
	CTexture* objTex=0;

	OPointer obj=0;

	if(!ko)
	{
		obj = KerMain->NewObject(0,objName);
		ko = KerMain->Objs->GetObject(obj);
	}

	if(ko)
	{
		// zjistit texturu pro objekt
		objTex = ko->Type->EditTex;
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
			objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);

	}
	else
		objTex = styleSet->Get("GOTB_StdObjectTex")->GetTexture(0);

	if(obj)
		KerMain->DeleteObject(0,obj);	// vytvarel jsem si vlastni objekt, zrusim ho

	CGUIRectHost *picture = new CGUIRectHost(8, 8, objTex, 28, 28);
	AddElem(picture);

	if(releaseTex && objTex)
		objTex->Release();
}



//////////////////////////////////////////////////////////////////////
// CGameMenuProgressBar
//////////////////////////////////////////////////////////////////////

CGameMenuProgressBar::CGameMenuProgressBar(int color, bool compressed)
: CGUIWidget(0,0)
{
	RemoveFromTrash();

	if(compressed)
		border = new CGUIRectHost(0,0,styleSet->Get("GameMenuProgressBarCompressed")->GetTexture(0));
	else
		border = new CGUIRectHost(0,0,styleSet->Get("GameMenuProgressBar")->GetTexture(0));
	progressPicture = new CGUIRectHost(0,0,0.0f,12.0f,color);

	progress=0;
	if(compressed)
		fullSize=62;
	else
		fullSize=92;

	AddToTrash();
}

CGameMenuProgressBar::~CGameMenuProgressBar()
{
	delete border;
	delete progressPicture;
}

int CGameMenuProgressBar::SetProgress(int _progress)
{
	progress = _progress;

	if(progress<0)
		progress=0;
	else if(progress>100)
		progress=100;

	progressPicture->Resize(floorf((float)progress/100*fullSize),12);
	
	return progress;
}

int CGameMenuProgressBar::GetProgress()
{
	return progress;
}

int CGameMenuProgressBar::GetColor()
{
	return progressPicture->GetColor();
}

void CGameMenuProgressBar::AddToEngine(CBltWindow *rootwnd)
{
	if(rootwnd)
	{
		border->AddToEngine(rootwnd);
		progressPicture->AddToEngine(rootwnd);
	}
}

void CGameMenuProgressBar::RemoveFromEngine()
{
	border->RemoveFromEngine();
	progressPicture->RemoveFromEngine();
}

void CGameMenuProgressBar::BringToTop()
{
	border->BringToTop();
	progressPicture->BringToTop();
}


void CGameMenuProgressBar::Resize(float _sx, float _sy)
{
}

void CGameMenuProgressBar::Move(float _x, float _y)
{
	border->Move(_x,_y);
	progressPicture->Move(_x+4,_y+4);

	SetPos(_x,_y);
}


void CGameMenuProgressBar::SetVisible(int vis)
{
	border->SetVisible(vis);
	progressPicture->SetVisible(vis);

	SetVisibility(vis);
}


