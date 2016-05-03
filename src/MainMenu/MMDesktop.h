///////////////////////////////////////////////
//
// MMDesktop.h
//
// Main Menu Desktop (verze 2)
//
// A: Petr Altman
//
///////////////////////////////////////////////

#ifndef MM_DESKTOP_H
#define MM_DESKTOP_H

#include "gui.h"
#include "window.h"

extern class CMMDesktop *MMDesktop;

//////////////////////////////////////////////////////////////////////
// CMMDesktop
//////////////////////////////////////////////////////////////////////

class CMMDesktop : public CGUIDesktop
{
	friend class CMMDesktopHandler;
	friend class CMMLevelBrowser;
public:
	CMMDesktop();
	~CMMDesktop();


	int ShowMainMenu();

	int PlayLevel(struct CMMDirFile *level, int restart=0);
	int PlayLevel(int restart=0);

	void KernelPanic();

	int ShouldShowCursor(){if(MMmode==1) return 0; else return 1;}

	void ShowLevelIntro(const char *textPath, const char *header = 0);

protected:

	int MMmode; //0 = mainmenu, 1=playlevel	

	CMMDesktopHandler* DefaultHandler;	

	int screenx,screeny;
	float scX(int x);
	float scY(int y);
	float scX2(int x);
	float scY2(int y);

	class CMMLevelBrowser *levBrowser;
	struct CMMDirFile *playingFile;
	char *quicksave;

	virtual int Keyboard(UINT character, UINT state);
	void EditLevel();
	void ShowCredits();
	void ShowHelp();

	class MMLevelIntro *levelIntro;

};


class CMMLevelBrowser{
public:
	CMMLevelBrowser();
	~CMMLevelBrowser();

	class CGUIListItem* GetSelectedItem();
	void MoveLeft();
	void MoveRight();

protected:

	CGUIWindow *w;
	class CGUIListBox *lb;

	void ShowTabs();
	int numTabs;
	int actTab;

};



//////////////////////////////////////////////////////////////////////
// CMMDesktopHandler
//////////////////////////////////////////////////////////////////////

class CMMDesktopHandler : public CGUIEventProgram
{
public:
	CMMDesktopHandler(); 
	
	typeID play,exit,editlevel,editscript,help,about,restartlevel,levelsel,regbrow, mmLeft, mmRight;

private:
	virtual void EventHandler(CGUIEvent *event);
	void RegBrowEvent(CGUIEvent *event);
};



#endif
