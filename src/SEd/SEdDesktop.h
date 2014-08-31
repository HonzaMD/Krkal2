///////////////////////////////////////////////
//
// SEdDesktop.h
//
// Interface pro desktop editoru skriptu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef SED_DESKTOP_H
#define SED_DESKTOP_H

#include "gui.h"
#include "window.h"

class CGUIMenu;
class CSEdDesktopIconHandler;

typedef DWORD OPointer;  // typ Pointer na Objekt

//////////////////////////////////////////////////////////////////////
// CSEdDesktop
//////////////////////////////////////////////////////////////////////

class CSEdDesktop : public CGUIDesktop
{
public:
	CSEdDesktop();
	~CSEdDesktop();

	void InitDefaultIcons();
	CSEdDesktopIconHandler* iconDefaultHandler;	// zakladni ovladac pro udalosti od icon

	void AddStartMenu();


	bool push;

	CGUIMenu* startMenu;
	
	int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	int MouseRight(float x, float y, UINT mouseState, UINT keyState);
	int MouseMiddle(float x, float y, UINT mouseState, UINT keyState);

	void ShowFileBrowser();
	void ShowObjectBrowser();
	void ShowNameBrowser();
	class CSEdObjectBrowser* GetObjectBrowser();
	class CNameBrowser* GetNameBrowser();
	class CSEdFileBrowser* GetFileBrowser();
};

extern CSEdDesktop *SEdDesktop;


//////////////////////////////////////////////////////////////////////
// CSEdDesktopIconHandler
//////////////////////////////////////////////////////////////////////

class CSEdDesktopIconHandler : public CGUIEventProgram
{
	virtual void EventHandler(CGUIEvent *event);
public:
	CSEdDesktopIconHandler(); 
	virtual ~CSEdDesktopIconHandler() { SAFE_DELETE_ARRAY(icons); };

	typeID *icons;	// pole ID icon, kvuli rozeznavani pri prichozi udalosti
	bool startButtonDown;
	bool startButtonOff;

	void ShowFileBrowser();
	void ShowObjectBrowser();
	void ShowNameBrowser();

	class CSEdObjectBrowser* GetObjectBrowser();
	class CNameBrowser* GetNameBrowser();
	class CSEdFileBrowser* GetFileBrowser();

protected:
	typeID filebrowserID,objectbrowserID,namebrowserID;
};


#endif
