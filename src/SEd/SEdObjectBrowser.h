////////////////////////////////////////////////////////////////////////////////////
//
// SEdObjectBrowser.h
//
// SEd ObjectBrowser
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////


#ifndef SEDOBJECTBROW_H
#define SEDOBJECTBROW_H

#include "widget.h"
#include "dialogs.h"
#include "tree.h"

class CGUIButton;

class CComOBObject;
class CComOBMethod;
class CComOBAttribute;
class CComOBGlobal;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SEd Objectbrowser
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXOBNAMELEN 256
#define MAXOBFNNAMELEN 1024

class CSEdObjectBrowser : public CGUITree
{
	friend class CSEdOBEventHandler;
public:
	CSEdObjectBrowser(float _x, float _y, float _sx, float _sy);
	~CSEdObjectBrowser();

	void CreateOBitems();
	void UpdateOB();

	void CompileError();

protected:

	int compileerr;

	CGUITreeItem *AddObject(CComOBObject *o, CGUITreeItem *parent_it=0);
	int AddObjectMethodsAndAttributes(CComOBObject *o, CGUITreeItem *object_item);

	CGUITreeItem *AddMethod(CComOBMethod *m, CGUITreeItem *parent_it, char *objKerName);
	CGUITreeItem *AddAttribute(CComOBAttribute *a, CGUITreeItem *parent_it, char *objKerName);
	CGUITreeItem *AddGlobal(CComOBGlobal *a, CGUITreeItem *parent_it=0);
	int AddObjectLayer(CKerName *from, CGUITreeItem *parent_it=0);

	int DeleteOldItems(CGUITreeItem *parent_it=0);

	CGUITreeItem* FindItem(char *name, CGUITreeItem *parent_it, typeID &id, int type);

	virtual int TreeHandler(typeID treeItem, int index, int state);
	virtual int FocusChanged(UINT focus, typeID dualID); //zmenil se focus
	virtual void TreeUpdateHandler(typeID treeItem, int index);
	
	class CSEdOBEventHandler *eventhandler;

	CGUIButton *butFB,*butOB,*butNB;

	char tmname[MAXOBNAMELEN];
	char tmfnname[MAXOBFNNAMELEN];

	CTexture *tex_obj,*tex_met,*tex_atr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CSEdOBEventHandler
//
// obsluha context menu, newdialogu
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CSEdOBEventHandler : public CGUIEventProgram
{
public:
	CSEdOBEventHandler(CSEdObjectBrowser* ob);
	~CSEdOBEventHandler();
private:
	virtual void EventHandler(CGUIEvent *event);
	CSEdObjectBrowser* objectbrow;
	
};

#endif
