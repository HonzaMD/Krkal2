///////////////////////////////////////////////
//
// MainMenuDesktop.h
//
// desktop Main Menu
//
// A: Petr Altman
//
///////////////////////////////////////////////

#ifndef MAINMENU_DESKTOP_H
#define MAINMENU_DESKTOP_H

#include "gui.h"
#include "window.h"
#include "tree.h"

extern class CMainMenuDesktop *MMdesktop;

class CGUIMenu;
class CMainMenuDesktopIconHandler;
class CGameMenu;

//////////////////////////////////////////////////////////////////////
// CMainMenuDesktop
//////////////////////////////////////////////////////////////////////

class CMainMenuDesktop : public CGUIDesktop
{
	friend class CMainMenuDesktopIconHandler;
	friend class CMainMenuSelectLevel;
public:
	CMainMenuDesktop();
	~CMainMenuDesktop();

	CMainMenuDesktopIconHandler* iconDefaultHandler;	// zakladni ovladac pro udalosti od icon

	int ShowMainMenu();

	void KernelPanic();

	int ShouldShowCursor(){if(MMmode==1) return 0; else return 1;}

protected:

	int MMmode; //0 = mainmenu, 1=playlevel	

	int PlayLevel(typeID levelID, int restart=0);
	void EditLevel();

	CGUIFileBrowserItem* GetSelectedLevel();	
		// podle zvoleneho levelu v CMainMenuSelectLevel nastavi promenne lRoot, lFile

	void ShowCredits();
	void ShowHelp();

	void LevelChange(typeID levelID);	
	char *quicksavelevel;
	char *GetLevelName(typeID levelID); //vrati jmeno levlu, alokuje novy string, musi se pak zrusit!

	int screenx,screeny;
	class CMainMenuSelectLevel *levelsel;

	virtual int KeyboardChar(UINT character, UINT state);
	virtual int Keyboard(UINT character, UINT state);
	virtual int KeyboardUp(UINT character, UINT state);
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over,UINT state);
	virtual int MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID);
	virtual int MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID);
	virtual int MouseRelativeMove(float dx, float dy);
};


//////////////////////////////////////////////////////////////////////
// CMainMenuDesktopIconHandler
//////////////////////////////////////////////////////////////////////

class CMainMenuDesktopIconHandler : public CGUIEventProgram
{
public:
	CMainMenuDesktopIconHandler(); 

	typeID play,exit,editlevel,editscript,help,about,restartlevel,levelsel,regbrow;
	//CMainMenuDesktop* mmdesktop;
private:
	virtual void EventHandler(CGUIEvent *event);
	void RegBrowEvent(CGUIEvent *event);
};

//////////////////////////////////////////////////////////////////////

class CMainMenuSelectLevel : public CGUIFileBrowser
{
public:
	CMainMenuSelectLevel(float _x, float _y, float _sx, float _sy, char* path, char* label);
	virtual ~CMainMenuSelectLevel();

	virtual CGUIFileBrowserItem* AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir);

protected:
	
	static int Compare(CGUITreeItem *aa, CGUITreeItem *bb);
};



#endif
