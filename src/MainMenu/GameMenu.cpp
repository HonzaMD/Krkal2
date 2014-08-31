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

	float dsx,dsy;
	desktop->GetVPSize(dsx,dsy);

	Resize(dsx,STD_GAMEMENU_HEIGHT);
	Move(0,dsy-STD_GAMEMENU_HEIGHT);

	SetAnchor(0,1,1,1);

	CGUIStaticPicture* sp = new CGUIStaticPicture(0,0,sx,sy,STD_GAMEMENU_BGCOLOR);
	AddElem(sp);
	sp->SetAnchor(1,1,1,1);

	numObjects=0;

	withProgressBars=false;
	progressBar1=progressBar2=0;
	compressedMenu=false;

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

int CGameMenu::AddItem(CKerName* item)
{
	if(!gameMenu)
		return -1;

	assert(item && item->Type == eKerNTobject);
	gameMenu->numObjects++;

	CGameMenuObject* mo = new CGameMenuObject(item);
	gameMenu->AddElem(mo);
	return gameMenu->objArray.Add(mo);
}


int CGameMenu::AddItem(OPointer obj)
{
	if(!gameMenu)
		return -1;

	assert(obj);
	gameMenu->numObjects++;

	CGameMenuObject* mo = new CGameMenuObject(obj);
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

	CGameMenuObject* mo = new CGameMenuObject(item);
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
		msx-=120;

	float dx = floorf(msx/gameMenu->numObjects);
	float xx = floorf(dx/2)-30;
	if(xx<0)
		xx=0;
	if(dx<65)
	{
		if(!gameMenu->compressedMenu)
			changeCompress=true;
		gameMenu->compressedMenu=true;
		
		if(gameMenu->withProgressBars)
			dx = floorf((gameMenu->sx-80)/gameMenu->numObjects);
		else
			dx = floorf(gameMenu->sx/gameMenu->numObjects);

		if(dx<40)
			dx=40;
		xx = floorf(dx/2)-20;
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

	for(int i = 0; i < gameMenu->objArray.GetSize(); i++)
	{
		CGameMenuObject* gmo = gameMenu->objArray.Get(i);
		if(gmo)
		{
			if(changeCompress)
			{
				gmo->SetCompress(gameMenu->compressedMenu);
			}
			gmo->Move(xx,0);
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

CGameMenuObject::CGameMenuObject(CKerName* item)
: CGUIWidget(0,0)
{
	assert(item && item->Type == eKerNTobject);

	RemoveFromTrash();

	objName = item;
	compressed=false;

	border = new CGUIRectHost(0,0,styleSet->Get("GameMenuItem")->GetTexture(0));

	CreateObjPicture();

	countBorder = new CGUIRectHost(0,0,styleSet->Get("GameMenuCounter")->GetTexture(0));
	count=0;
	countPicture = new CGUIStaticText(" 0",(CGUIFont*)RefMgr->Find("GUI.F.Courier.14.B"),30,30,STD_GAMEMENU_COUNTCOLOR);

	AddToTrash();
}

CGameMenuObject::CGameMenuObject(OPointer obj)
: CGUIWidget(0,0)
{
	assert(obj && KerMain);

	RemoveFromTrash();

	CKerObject* ko = KerMain->Objs->GetObject(obj);

	objName = ko->Type->Name;
	compressed=false;

	border = new CGUIRectHost(0,0,styleSet->Get("GameMenuItem")->GetTexture(0));

	CreateObjPicture(ko);

	countBorder = new CGUIRectHost(0,0,styleSet->Get("GameMenuCounter")->GetTexture(0));
	count=0;
	countPicture = new CGUIStaticText(" 0",(CGUIFont*)RefMgr->Find("GUI.F.Courier.14.B"),30,30,STD_GAMEMENU_COUNTCOLOR);

	AddToTrash();
}


CGameMenuObject::~CGameMenuObject()
{
	delete border;
	delete picture;
	delete countBorder;
	delete countPicture;
}

int CGameMenuObject::Increment()
{
	count++;

	_snprintf(countString,10,"%2d",count);
	countPicture->ChangeText(countString);

	return count;
}

int CGameMenuObject::Decrement()
{
	count--;

	_snprintf(countString,10,"%2d",count);
	countPicture->ChangeText(countString);

	return count;
}

int CGameMenuObject::SetCount(int _count)
{
	count = _count;

	_snprintf(countString,10,"%2d",count);
	countPicture->ChangeText(countString);

	return count;
}

int CGameMenuObject::GetCount()
{
	return count;
}

void CGameMenuObject::SetCompress(bool compress)
{
	compressed=compress;
}

void CGameMenuObject::AddToEngine(CBltWindow *rootwnd)
{
	if(rootwnd)
	{
		border->AddToEngine(rootwnd);
		picture->AddToEngine(rootwnd);
		countBorder->AddToEngine(rootwnd);
		countPicture->AddToEngine(rootwnd);
	}
}

void CGameMenuObject::RemoveFromEngine()
{
	border->RemoveFromEngine();
	picture->RemoveFromEngine();
	countBorder->RemoveFromEngine();
	countPicture->RemoveFromEngine();
}

void CGameMenuObject::BringToTop()
{
	border->BringToTop();
	picture->BringToTop();
	countBorder->BringToTop();
	countPicture->BringToTop();
}


void CGameMenuObject::Resize(float _sx, float _sy)
{
}

void CGameMenuObject::Move(float _x, float _y)
{
	border->Move(_x,_y);
	picture->Move(_x+4,_y+4);

	if(compressed)
	{
		countBorder->Move(_x+5,_y+30);
		countPicture->Move(_x+10,_y+30);
	}
	else
	{
		countBorder->Move(_x+30,_y+30);
		countPicture->Move(_x+35,_y+30);
	}

	SetPos(_x,_y);
}


void CGameMenuObject::SetVisible(int vis)
{
	border->SetVisible(vis);
	picture->SetVisible(vis);
	countBorder->SetVisible(vis);
	countPicture->SetVisible(vis);

	SetVisibility(vis);
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

	picture = new CGUIRectHost(4,4,objTex,30,30);

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


