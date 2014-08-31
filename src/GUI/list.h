///////////////////////////////////////////////
//
// list.h
//
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#ifndef LIST_H
#define LIST_H

#include "element.h"
#include "objectList.h"

class CGUIListItem;
class CGUIStaticText;
class CGUIRectHost;
struct CMMDirFile;

class CGUIListBox : public CGUIWindow
{
	friend class CGUIListItem;
	//friend class CGUIListElement;

public:
	CGUIListBox(float _x, float _y, float _sx, float _sy);
	virtual ~CGUIListBox();

	CGUIListItem* AddItem(char *labelString, UINT color = 0xFFFFFFFF, CMMDirFile* dirFile = 0);

	void UpdateList();

	float listItemHeight;
	float columnWidth;
	float numberGap;	// mezera mezi cislovanim seznamu a textem polozky
	float numberLimit;	// prava mez pro cislovani (zarazka pro zarovnani)

	float columnGap;	// mezera mezi sloupci

	int listIndex;	// kolik polozek ma seznam (index nasledujici pridavane)

	int numLines;	// kolik je max. polozek ve sloupci (pocet radek)

	CGUIListItem* GetMarkListItem() {return markListItem;}

	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

protected:

	int numColumns;	// kolik je prave vytvoreno sloupcu

	virtual int Keyboard(UINT character, UINT state);
	virtual void EventHandler(CGUIEvent *event);

	CGUIList<CGUIListItem> *items;	// seznam korenovych polozek stromu
	CGUIListItem* markListItem;		// naposled oznacena polozka stromu
	void ScrollToMarked();

	//virtual int FocusChanged(UINT focus, typeID dualID);

	CGUIRectHost* marker;

	static const int numEvents;
	static const int availableEvents[];
};


class CGUIListItem : public CGUIElement, public CGUIEventSupport
{
	friend class CGUIListElement<CGUIListItem>;
	friend class CGUIListBox;
public:
	CGUIListItem(int listNumber, char *labelString, UINT labelColor, CGUIListBox* _list);
	virtual ~CGUIListItem();

	void Mark();		// oznaci polozku
	void UnMark();		// odoznaci polozku

	virtual void Resize(float _sx, float _sy) {};
	virtual void Move(float _x, float _y);
	virtual void BringToTop();
	virtual void SetVisible(int vis);

	CMMDirFile* GetData() { return dirFile; }

protected:
	virtual void AddToEngine(CBltWindow *rootwnd);
	virtual void RemoveFromEngine();

	// funkce zajistujici funkcnost elementu (reakce na vstupy):
	virtual int MouseLeft(float x, float y, UINT mouseState, UINT keyState);
	virtual int MouseOver(float x, float y, UINT over, UINT state);
	virtual int MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID);
	virtual int FocusChanged(UINT focus, typeID dualID);
	virtual int WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID);
	virtual int Keyboard(UINT character, UINT state);
	//virtual void EventHandler(CGUIEvent *event);

	virtual void OnClick();	// fce pro uzivatelske pretizeni a definici co se ma stat po kliknuti na polozku


	CMMDirFile* dirFile;

	CGUIStaticText *label;	// popisek
	CGUIStaticText *number; // cislovani

	CGUIListBox *listBox;		// odkaz na seznam, ve kterem je polozka umistena (muze byt 0 - polozka je jen v okne)

	CGUIList<CGUIListItem>* list;	// odkaz na seznam, ve kterem je polozka umistena

	static const int numEvents;
	static const int availableEvents[];
};


#endif