////////////////////////////////////////////////////////////////////////////////////
//
// NameBrowser.h
//
// prohlizec jmen
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#ifndef NAMEBROWSER_H
#define NAMEBROWSER_H

#include "widget.h"
#include "dialogs.h"
#include "tree.h"

class CGUIButton;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NameBrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CNameBrowser : public CGUIStdWindow
{
	friend class CNBEventHandler;
public:
	CNameBrowser(float _x, float _y, float _sx, float _sy);
	~CNameBrowser();

	void CreateNBitems();
	void UpdateNB();

	void SetStartName(CKerName* kername);

protected:

	int AddNameLayer(CKerName *from, int down=1, CGUITreeItem *parent_it=NULL);
	CGUITreeItem* AddName(CKerName *name, int down, CGUITreeItem *parent_it);
	CGUITreeItem* FindItem(CKerName *name, CGUITreeItem *parent_it, typeID &id, int down);
	int DeleteOldItems(int down, CGUITreeItem *parent_it);
	class CKerName* GetStartKerName();

	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus

	CGUITree *treeDown, *treeUp;
	CGUIButton *butFB,*butOB,*butNB,*butclearName,*butclearName2,*butFiltr;
	class CNBEventHandler *eventhandler;

	typeID filtrWnd;
	int curFiltr;
	void ShowFiltrWnd();
	void CloseFiltrWnd();
	void SetFiltr(int filtr);
	class CGUIStaticText *stFiltr;

	class CKerNamesMain *KerNamesMain;

	char *startname;
	class CGUIEditWindow *ewstartname;


	CTexture *zobakL,*zobakR;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CNBEventHandelr
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CNBEventHandler : public CGUIEventProgram
{
public:
	CNBEventHandler(CNameBrowser* nb);
	~CNBEventHandler();
private:
	virtual void EventHandler(CGUIEvent *event);
	CNameBrowser* namebrow;
};

#endif
