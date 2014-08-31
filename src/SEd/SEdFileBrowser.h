////////////////////////////////////////////////////////////////////////////////////
//
// SEdFileBrowser.h
//
// SEd FileBrowser
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#ifndef SEDFILEBROW_H
#define SEDFILEBROW_H

#include "widget.h"
#include "dialogs.h"
#include "tree.h"

class CGUIButton;
class CGUIEditWindow;
class CGUICheckBox;
class CGUIStaticPicture;
class CGUIStaticText;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SEd filebrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CSEdFileBrowser : public CGUIFileBrowser
{
	friend class CSEdFBEventHandler;
public:
	CSEdFileBrowser(float _x, float _y, float _sx, float _sy, char* path="$GAMES$", char* label="GAMES");
	~CSEdFileBrowser();

protected:

	virtual int TreeHandler(typeID treeItem, int index, int state);
	virtual int Filter(CGUITreeItem* parent, const char *name, int dir, int first);
	virtual CGUIFileBrowserItem* AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir);

	static int Compare(CGUITreeItem *aa, CGUITreeItem *bb);

	class CGUIMenu *dirmenu,*scriptmenu;
	class CSEdFBEventHandler *eventhandler;

	void EditScript(char *dir, char *file);
	void EditScript(char *filepath);

	CGUIButton *butOB,*butFB,*butNB;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CSEdFBEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CSEdFBEventHandler : public CGUIEventProgram
{
public:
	CSEdFBEventHandler(CSEdFileBrowser* fb);
	~CSEdFBEventHandler();
private:
	virtual void EventHandler(CGUIEvent *event);
	CSEdFileBrowser* filebrow;

	typeID dlgnscrID,dlgdelID;
	int newmode;
	char *newname;
	char *path;
	
};

#endif
