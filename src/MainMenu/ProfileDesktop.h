///////////////////////////////////////////////
//
// ProfileDesktop.h
//
// vyber profilu
//
// A: Petr Altman
//
///////////////////////////////////////////////

#ifndef PROFILE_DESKTOP_H
#define PROFILE_DESKTOP_H

#include "gui.h"
#include "window.h"

extern class CProfileDesktop *ProfileDesktop;

class CGUIStaticPicture;
class CGUITextButton;
class CGUIEditWindow;

#define MAX_PROFILES 15


//////////////////////////////////////////////////////////////////////
// CProfileDesktop
//////////////////////////////////////////////////////////////////////

class CProfileDesktop : public CGUIDesktop
{
	friend class CProfileDesktopHandler;
public:
	CProfileDesktop();
	~CProfileDesktop();

	CProfileDesktopHandler* DefaultHandler;	
protected:

	void ShowProfile();
	void UpdateArrow();
	virtual int Keyboard(UINT character, UINT state);

	void StartMainMenu();
	int LoadProfile();
	void CreateNewUser();
	void CreateProfile();
	void CreateNewUserButton();
	
	int scX(int x);
	int scY(int y);

	int CmpUserPosX(int n);
	int CmpUserPosY(int n);

	int screenx,screeny;
	float fy; //text pixel height

	int cnt; //pocet profilu (hracu)
	int cancreatenew; //muze se vytvorit dalsi profil?
	int cur; //vybrany profil (kdyz cur=cnt+1 => novy profil)

	char *profileNames[MAX_PROFILES+1]; //jmena profilu

	CGUIStaticPicture *arrow;
	CGUITextButton* profiles[MAX_PROFILES+1];
	CGUITextButton* newuser;
	CGUIEditWindow* edNewUser;

	CGUIFont *fontTB,*fontT;

};


//////////////////////////////////////////////////////////////////////
// CProfileDesktopHandler
//////////////////////////////////////////////////////////////////////

class CProfileDesktopHandler : public CGUIEventProgram
{
public:
	CProfileDesktopHandler(); 

private:
	virtual void EventHandler(CGUIEvent *event);
};



#endif
