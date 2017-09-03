///////////////////////////////////////////////
//
// ProfileDesktop.cpp
//
// vyber profilu
//
// A: Petr Altman
//
///////////////////////////////////////////////


#include "stdafx.h"

#include "ProfileDesktop.h"
#include "menu.h"
#include "GameMenu.h"

#include "dx.h"
#include "music.h"
#include "krkal.h"
#include "ident.h"
#include "textedit.h"
#include "levels.h"
#include "fs.h"
#include "dialogs.h"
#include "register.h"

CProfileDesktop *ProfileDesktop=0;


//////////////////////////////////////////////////////////////////////
// CProfileDesktop
//////////////////////////////////////////////////////////////////////

CProfileDesktop::CProfileDesktop()
	: CGUIDesktop()
{
	try{
		assert(!ProfileDesktop);
		ProfileDesktop=this;
		DefaultHandler = 0;

		RemoveFromTrash();
		MsgAcceptConsume(MsgMouseL|MsgMouseM|MsgMouseR|MsgKeyboard);

		DefaultHandler = new CProfileDesktopHandler();

		for(int i=0;i<MAX_PROFILES;i++)
		{
			profiles[i]=0;
			profileNames[i]=0;
		}

		cancreatenew=1;
		newuser=0;
		edNewUser=0;
		cnt=0;

		ShowProfile();

		if(ME) ME->Play("$GAME$/music_65B6_399E_4613_0839.pkg/AMERICA.mm",1,2);

		AddToTrash();

	}

	catch(CExc c)
	{
		SAFE_DELETE(DefaultHandler);
		mainGUI->LogError(c);

		ProfileDesktop=0;
		desktop=0;
		throw CExc(eGUI, E_INTERNAL,"CMMDesktop::CMMDesktop> Fatal error - cannot create DESKTOP");
	}

}

CProfileDesktop::~CProfileDesktop()
{
	for(int i=0;i<MAX_PROFILES;i++)
		SAFE_DELETE_ARRAY(profileNames[i]);
			
	ProfileDesktop=0;
	RemoveFromTrash();
	SAFE_DELETE(DefaultHandler);
}


void CProfileDesktop::ShowProfile()
{

	KRKAL->GetScreenRes(screenx,screeny);

	CGUIStaticPicture* back = new CGUIStaticPicture(0,0,"$TEX$/MainMenu2/logo.jpg",(float)screenx,(float)screeny);
	AddBackElem(back);

	if (screenx < 700)
	{
		fontTB = (CGUIFont*)RefMgr->Find("GUI.F.Tahoma.10.B");
		fontT = (CGUIFont*)RefMgr->Find("GUI.F.Tahoma.10");
	}
	else {
		fontTB = (CGUIFont*)RefMgr->Find("GUI.F.Tahoma.12.B");
		fontT = (CGUIFont*)RefMgr->Find("GUI.F.Tahoma.12");
	}

	int x,y;

	cur=1;
	cnt=0;

	FS->ChangeDir("$PROFILES$");
	
	const char *fn;
	int d;
	int k=FS->FindFirst(&fn,d);
	char *s;
	while(k&&cnt<MAX_PROFILES)
	{
		s = MMProfiles->ReadProfileName(fn);
		if(s)
		{
			cnt++;
			profileNames[cnt]=s;
		}
		k=FS->FindNext(&fn,d);
	}
	FS->FindClose();

	if(cnt==MAX_PROFILES)
		cancreatenew=0;


	x=CmpUserPosX(0);
	y=CmpUserPosY(0);

	CGUIStaticText* selectuser = new CGUIStaticText("en{{Select User:}}cs{{Vyber uživatele:}}",fontTB,(float)x,(float)y,0xFFFFFFFF);
	AddBackElem(selectuser);

	float fx;
	selectuser->GetSize(fx,fy);
	y+=(int)fy;

	CGUILine* ln = new CGUILine((float)x,(float)y,(float)scX(800),(float)y,0xFFFFFFFF);
	AddBackElem(ln);

	for(int n=1;n<=cnt;n++)
	{
		profiles[n] = new CGUITextButton(profileNames[n],fontT,(float)CmpUserPosX(n),(float)CmpUserPosY(n),0xFFFFFFFF);
		profiles[n]->markColor=0xFFFFFF00;
		profiles[n]->AcceptEvent(DefaultHandler->GetID(),EClicked);
		AddBackElem(profiles[n]);
	}

	CreateNewUserButton();

	arrow = new CGUIStaticPicture(0,0,"$TEX$/MainMenu2/sipka.png");
	AddBackElem(arrow);
	
	UpdateArrow();
}

void CProfileDesktop::CreateNewUserButton()
{
	if(cnt!=MAX_PROFILES)
	{
		newuser = new CGUITextButton("en{{new user...}}cs{{nový uživatel...}}",fontT,(float)CmpUserPosX(cnt+1),(float)CmpUserPosY(cnt+1),0xFFFFFFFF);
		newuser->markColor=0xFFFFFF00;
		newuser->AcceptEvent(DefaultHandler->GetID(),EClicked);
		AddBackElem(newuser);
	}
}

void CProfileDesktop::UpdateArrow()
{
	arrow->Move((float)CmpUserPosX(cur)-15,(float)CmpUserPosY(cur)+(float)((fy-9)/2)+1);
}

int CProfileDesktop::CmpUserPosX(int n)
{
	if(cnt+cancreatenew-1>4) n=n-(cnt+cancreatenew-1-4);

	if(n<0) n=0;

	int x;

	switch(n)
	{
	case 0: x=649; break;
	case 1: x=645; break;
	case 2: x=629; break;
	case 3: x=600; break;
	case 4: x=558; break;
	default: x=493; break;
	}

	return scX(x);
}

int CProfileDesktop::CmpUserPosY(int n)
{

	int y;
	int sz=30;

	if (cnt+cancreatenew-1<=4) 
		y=381; 
	else
		y=381-sz*(cnt+cancreatenew-1-4);

	y+=n*sz;

	return scY(y);
}

int CProfileDesktop::scX(int x)
{
	return (int)ceil(x*screenx/800.);
}
int CProfileDesktop::scY(int y)
{
	return (int)ceil(y*screeny/600.);
}

int CProfileDesktop::Keyboard(UINT ch, UINT state)
{
	if(edNewUser) return 1;
	switch(ch)
	{
	case VK_DOWN:
	case VK_LEFT:
		if(cur<cnt+cancreatenew)
		{
		  cur++;
		  UpdateArrow();
		}
		break;
	case VK_UP:
	case VK_RIGHT:
		if(cur>1)
		{
		  cur--;
		  UpdateArrow();
		}
		break;
	case VK_END:
	case VK_NEXT:
		if(cur!=cnt+cancreatenew)
		{
			cur=cnt+cancreatenew;
			UpdateArrow();
		}
		break;
	case VK_HOME:
	case VK_PRIOR:
		if(cur!=1)
		{
			cur=1;
			UpdateArrow();
		}
		break;	
	case VK_RETURN:

		if(cur!=cnt+1)
			LoadProfile();
		else
			CreateNewUser();

		break;
	case VK_F4:
		mainGUI->Exit();
		break;

	}
	return 1;
}

void CProfileDesktop::StartMainMenu()
{	
	mainGUI->StartMainMenuNoResChange();
}
int CProfileDesktop::LoadProfile()
{
	if(profileNames[cur] && MMProfiles->Open(profileNames[cur])==FSREGOK)
	{
		ProfileDesktop->StartMainMenu();
		return 1;
	}else{
		GUIMessageBox("Error","Invalid profile!",1,0xFF000000);
	}
	return 0;
}

void CProfileDesktop::CreateNewUser()
{
	cur=cnt+1;UpdateArrow();
	DeleteBackElem(newuser);newuser=0;

	edNewUser = new CGUIEditWindow((float)CmpUserPosX(cnt+1),(float)CmpUserPosY(cnt+1),(float)scX(200),0xFFFFFFFF,0xFF000000,"DX.F.Tahoma.12");
	edNewUser->AcceptEvent(DefaultHandler->GetID(),EOk);
	AddBackElem(edNewUser);
	SetFocusEl(edNewUser,0);
}

void CProfileDesktop::CreateProfile()
{
	if(!edNewUser) return;

	char buf[101];	
	if( edNewUser->GetParametr(buf,dtString,100)==0 )
	{
		if(strcmp(buf,"")==0) return;

		if(MMProfiles->New(buf))
		{
			StartMainMenu();
			return;
		}

	}


	DeleteBackElem(edNewUser);edNewUser=0;
	CreateNewUserButton();

	GUIMessageBox("Error","Invalid profile name!",1,0xFF000000);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// CProfileDesktopHandler
//////////////////////////////////////////////////////////////////////////////////////////////////

CProfileDesktopHandler::CProfileDesktopHandler()
{
}

void CProfileDesktopHandler::EventHandler(CGUIEvent *event)
{
	if(!event || !ProfileDesktop)
		return;

	if(event->eventID == EClicked )
	{
		if(ProfileDesktop->newuser&&event->sender==ProfileDesktop->newuser->GetID())
			ProfileDesktop->CreateNewUser();		
		else
			for(int i=1;i<=ProfileDesktop->cnt;i++)
			if(event->sender==ProfileDesktop->profiles[i]->GetID()){
				ProfileDesktop->cur=i;
				ProfileDesktop->UpdateArrow();
				ProfileDesktop->LoadProfile();					
			}
	}else
	if(event->eventID == EOk )
	{
		if(ProfileDesktop->edNewUser&&event->sender==ProfileDesktop->edNewUser->GetID())
		{
			ProfileDesktop->CreateProfile();
		}
	}

	delete event;
}

