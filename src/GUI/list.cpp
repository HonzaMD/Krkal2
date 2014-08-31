///////////////////////////////////////////////
//
// list.cpp
//
/
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "list.h"
#include "gui.h"
#include "refmgr.h"
#include "widgets.h"

#include "primitives.h"
#include "krkal.h"

#include "levels.h"

//////////////////////////////////////////////////////////////////////
// CGUIListBox
//////////////////////////////////////////////////////////////////////

const int CGUIListBox::numEvents = 23;
const int CGUIListBox::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													EChangeTABElement,EList};


CGUIListBox::CGUIListBox(float _x, float _y, float _sx, float _sy)
: CGUIWindow(_x,_y,_sx,_sy,0)
{
	SetAvailableEvents(availableEvents,numEvents);
	items = new CGUIList<CGUIListItem>(false);
	markListItem = 0;

	listItemHeight = 20;
	//columnWidth = 150;
	numberGap = 25;
	numberLimit = 20;

	listIndex = 1;

	columnGap = 15;

	columnWidth = floorf((sx-columnGap)/2);

	numLines = (int)(sy/listItemHeight);


	numColumns = 1;


	marker = new CGUIRectHost(0,0,columnWidth,listItemHeight,0xFFAA0000);
	marker->SetVisible(0);
	AddBackElem(marker);
}

CGUIListBox::~CGUIListBox()
{
	if(items)
		delete items;

}



CGUIListItem* CGUIListBox::AddItem(char *labelString, UINT color, CMMDirFile* dirFile)
{
	CGUIListItem* ti;

	ti = new CGUIListItem(listIndex++,labelString,color,this);
	ti->AcceptEvent(GetID(),EClicked);
	ti->dirFile = dirFile;

	items->AddToBack(ti);

	AddBackElem(ti);

	return ti;
}


void CGUIListBox::UpdateList()
{
	CGUIListItem *e;
	CGUIListElement<CGUIListItem>* le = items->GetNextListElement(0);

	float px=-columnWidth-columnGap, py=0;
	int i=0;

	numColumns=0;

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if((i++)%numLines==0)
			{
				py=0;
				px+=columnWidth+columnGap;
				numColumns++;
			}

			e->Move(px,py);
			if(!e->IsVisible())
				e->SetVisible(1);
			py+=listItemHeight;

		}
		le=items->GetNextListElement(le);
	}	

	SetBackWindowNormalSize(numColumns * (columnWidth+columnGap), listItemHeight*numLines);
}



void CGUIListBox::ScrollToMarked() {
	float x,y;
	markListItem->GetPos(x,y);

	float nx, ny;
	GetVPPos(nx, ny);

	if (x < nx) {
		SetWindowPositionVP(x-nx, 0);
	} else if (x > nx + columnWidth+columnGap){
		SetWindowPositionVP((x - (columnWidth+columnGap)) - nx, 0);
	}
}


void CGUIListBox::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EKillMe)
	{	
		CGUIElement* el = GetObjFromID<CGUIElement>(event->sender);
		//CGUIElement* el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->sender));
		if(el)
		{
			if(el->back_element)
				DeleteBackElem(el);
			else
				DeleteFrontElem(el);
		}
	}

	/*
	if(event->eventID == EChangeTABElement)
	{
		CGUIElement* el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->sender));
		if(el && !(focus_elm && focus_elm->GetType()>=PTWindow && ((CGUIWindow*)focus_elm)->IsModal()))
		{
			if(lastTreeItem && lastTreeItem->elem==el)
			{
				CGUITreeItem* ti=0;
				if(event->pInt==1)
					ti=lastTreeItem->SwitchItem(true);
				else
					ti=lastTreeItem->SwitchItem(false);
	
				if(!ti)
				{
					if(event->pInt==1)
						ti = items->GetFromHead();
					else
						ti = items->GetFromBack();

					ti = Switch(ti);
				}

				if(ti && ti->elem)
				{
					SetFocusEl(ti->elem,0);
				}
			}
		}
	}
	*/

	if(event->eventID == EClicked)
	{
		CGUIListItem* ti = dynamic_cast<CGUIListItem*>(nameServer->ResolveID(event->sender));
		if(ti)
		{
			if(ti->listBox==this)
			{
				EventArise(EList,0,event->sender,0,0,0);
			}
		}
	}

	delete event;
}

/*
int CGUIListBox::FocusChanged(UINT focus, typeID dualID)
{
	if(focus_elm)
	{
		if(focus)
			focus_elm->MarkElement();
		else
			focus_elm->UnMarkElement();
	}

	if(!focus)
	{
		if(hold)
		{
			mainGUI->cursor->SetVisible(1);
			cursorChanged=false;
			mainGUI->mouseOverBlock=false;
			KRKAL->SetCursorPos((int)old_cursor_x,(int)old_cursor_y);
			KRKAL->ReleaseCapture();
			hold=false;
			hold_x=hold_y=0;
		}
		if(cursorChanged)
		{
			mainGUI->cursor->SetVisible(1);
			cursorChanged=false;
		}

		marker->SetVisible(0);
	}
	else
	{
		marker->SetVisible(1);
		if(!focus_elm)
		{
			
			// neni zadny el. focusovany a okno dostalo focus => focusuj prvni el. vevnitr okna
			CGUIListItem* ti = items->GetFromHead();
			if(ti)
			{
				SetFocusEl(ti,0);
				ti->Mark();
			}
		}
	}

	EventArise(EFocusChanged,focus);
	return 1;
}
*/



void CGUIListBox::MoveLeft() {
	if (markListItem) {
		CGUIListItem *item = markListItem;
		for (int f=0; f<numLines && items->GetPrev(item); f++) {
			item = items->GetPrev(item);
		}
		item->Mark();
	}
}
void CGUIListBox::MoveRight() {
	if (markListItem) {
		CGUIListItem *item = markListItem;
		for (int f=0; f<numLines && items->GetNext(item); f++) {
			item = items->GetNext(item);
		}
		item->Mark();
	}
}
void CGUIListBox::MoveUp() {
	if (markListItem) {
		CGUIListItem *item = items->GetPrev(markListItem);
		if (item)
			item->Mark();
	}
}
void CGUIListBox::MoveDown() {
	if (markListItem) {
		CGUIListItem *item = items->GetNext(markListItem);
		if (item)
			item->Mark();
	}
}


int CGUIListBox::Keyboard(UINT character, UINT state)
{
	if(character == VK_RETURN)
	{
		if(markListItem)
			EventArise(EList,0,markListItem->GetID(),0,0,0);
	}

	if (character == VK_UP) {
		MoveUp();
	}

	if (character == VK_DOWN) {
		MoveDown();
	}

	if (character == VK_RIGHT) {
		MoveRight();
	}

	if (character == VK_LEFT) {
		MoveLeft();
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// CGUIListItem
//////////////////////////////////////////////////////////////////////

const int CGUIListItem::numEvents = 5;
const int CGUIListItem::availableEvents[numEvents] = {EMouseOver,EClicked,
									   				  EFocusChanged,EDeleted,EKeyboard,
													 };

CGUIListItem::CGUIListItem(int listNumber, char *labelString, UINT labelColor, CGUIListBox *_list)
: CGUIElement(0,0), CGUIEventSupport(availableEvents, numEvents)
{
	listBox = _list;
	assert(listBox);

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgKeyboard|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);


	CGUIFont* font;
	font=(CGUIFont*)RefMgr->Find("GUI.F.Tahoma.12");

	char nn[10];
	sprintf(nn,"%d",listNumber);
	number = new CGUIStaticText(nn,font,0,0,labelColor,listBox->numberLimit,0,aRight);
	number->RemoveFromTrash();

	if(labelString)
	{
		label = new CGUIStaticText(labelString,font,listBox->numberGap,0,labelColor,listBox->columnWidth - listBox->numberGap,0,aLeft,1);
	}else
		label=0;

	SetSize(listBox->columnWidth,listBox->listItemHeight);

	SetVisible(0);

	if(label)
		label->RemoveFromTrash();

	AddToTrash();
}

CGUIListItem::~CGUIListItem()
{
	RemoveFromTrash();

	if(label)
		delete label;
	if(number)
		delete number;

	EventArise(EDeleted,0,0,this);
}

void CGUIListItem::Mark()
{
	if(listBox->markListItem==this)
		return;

	if(listBox->markListItem)
		listBox->markListItem->UnMark();
	listBox->markListItem=this;
	listBox->ScrollToMarked();

	// zobraz oznacovac:
	if(!listBox->marker->IsVisible())
		listBox->marker->SetVisible(1);
	listBox->marker->Move(x,y);
}

void CGUIListItem::UnMark()
{
	if(listBox->markListItem!=this)
		return;

	listBox->markListItem=0;

	// zrus oznacovac:

}


void CGUIListItem::AddToEngine(CBltWindow *rootwnd)
{
	if(label)
		label->AddToEngine(rootwnd);
	if(number)
		number->AddToEngine(rootwnd);
}


void CGUIListItem::RemoveFromEngine()
{
	if(label)
		label->RemoveFromEngine();
	if(number)
		number->RemoveFromEngine();
}

void CGUIListItem::SetVisible(int vis)
{
	SetVisibility(vis);
	if(label)
		label->SetVisible(vis);
	if(number)
		number->SetVisible(vis);
}

void CGUIListItem::BringToTop()
{
	if(label)
		label->BringToTop();
	if(number)
		number->BringToTop();
}

void CGUIListItem::Move(float _x, float _y)
{
	if(label)
		label->Move(_x+listBox->numberGap,_y);
	if(number)
	{
		float dx,dy;
		dx=_x-x;
		dy=_y-y;

		number->MoveRel(dx,dy);
	}

	SetPos(_x,_y);
}


int CGUIListItem::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)

	if(mouseState)
	{
		if(listBox->markListItem==this)
		{
			OnClick();
			EventArise(EClicked);
		}
		else
		{
			Mark();
		}
	}

	return 0;
}

int CGUIListItem::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		EventArise(EFocusChanged,focus);
		return 0;
	}

	//UnMark();

	EventArise(EFocusChanged,focus);
	return 0;
}

int CGUIListItem::MouseOver(float x, float y, UINT over, UINT state)
{
	EventArise(EMouseOver);

	return 0;
}

int CGUIListItem::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{

	return 0;
}

int CGUIListItem::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{

	return 0;
}

void CGUIListItem::OnClick()
{
}

int CGUIListItem::Keyboard(UINT character, UINT state)
{
/*
	bool rollOnClick = tree ? tree->rollOnClick : true;

	if((character == VK_RETURN) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(rollOnClick)
		{
			// rozbalit podstrom
			if(CGUITreeItem::state==1)
			{
				CGUITreeItem::state=2;
				stateChange=true;
				if(manualUpdate)
				{
					EventArise(ETreeItemUpdate,index);
				}else
					UpdateTree();
			}else if(CGUITreeItem::state==2)
			{
				CGUITreeItem::state=1;
				stateChange=true;
				if(manualUpdate)
				{
					EventArise(ETreeItemUpdate,index);
				}else
					UpdateTree();
			}else if(CGUITreeItem::state==0 && manualUpdate)
				EventArise(ETreeItemUpdate,index);

			EventArise(ETreeItemRoller,index,0,0,0,CGUITreeItem::state);
		}

		OnClick();
		EventArise(EClicked);
	}

	if((character == VK_RIGHT) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		// rozbalit podstrom
		if(CGUITreeItem::state==1)
		{
			CGUITreeItem::state=2;
			stateChange=true;
			if(manualUpdate)
			{
				EventArise(ETreeItemUpdate,index);
			}else
				UpdateTree();
		}else if(CGUITreeItem::state==0 && manualUpdate)
			EventArise(ETreeItemUpdate,index);

		EventArise(ETreeItemRoller,index,0,0,0,CGUITreeItem::state);

		if(rollOnClick)
		{
			OnClick();
			EventArise(EClicked);
		}
	}

	if((character == VK_LEFT) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(CGUITreeItem::state==2)
		{
			CGUITreeItem::state=1;
			stateChange=true;
			if(manualUpdate)
			{
				EventArise(ETreeItemUpdate,index);
			}else
				UpdateTree();
		}

		EventArise(ETreeItemRoller,index,0,0,0,CGUITreeItem::state);

		if(rollOnClick)
		{
			OnClick();
			EventArise(EClicked);
		}
	}

	if((character == VK_DOWN) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		SwitchItem(true);
		if(associatedWindow)
		{
			CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(associatedWindow->GetFocusEl());	// switch zmenul focus na novou polozku, vrati mi to nove oznacenou polozku
			ti->EventArise(EEvent);	// domluvena udalost pro Tree, ktera mu rekne, ze bylo najeto na polozku klavesnici - Tree vola TreeHandler
		}
	}

	if((character == VK_UP) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		SwitchItem(false);
		if(associatedWindow)
		{
			CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(associatedWindow->GetFocusEl());	// switch zmenul focus na novou polozku, vrati mi to nove oznacenou polozku
			ti->EventArise(EEvent);	// domluvena udalost pro Tree, ktera mu rekne, ze bylo najeto na polozku klavesnici - Tree vola TreeHandler
		}
	}

	if((character == VK_PRIOR) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		int i;
		float asx,asy;
		if(associatedWindow)
			associatedWindow->GetVPSize(asx,asy);
		if(tree)
			i = (int)(asy/tree->treeItemHeight);
		else
			i = (int)(asy/STD_TREE_HEIGHT);
		CGUITreeItem* ti=this;
		while(i--)
			if(ti)
				ti=ti->SwitchItem(false);
			else
				break;
		if(associatedWindow)
		{
			CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(associatedWindow->GetFocusEl());	// switch zmenul focus na novou polozku, vrati mi to nove oznacenou polozku
			ti->EventArise(EEvent);	// domluvena udalost pro Tree, ktera mu rekne, ze bylo najeto na polozku klavesnici - Tree vola TreeHandler
		}

	}

	if((character == VK_NEXT) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		int i;
		float asx,asy;
		if(associatedWindow)
			associatedWindow->GetVPSize(asx,asy);
		if(tree)
			i = (int)(asy/tree->treeItemHeight);
		else
			i = (int)(asy/STD_TREE_HEIGHT);
		CGUITreeItem* ti=this;
		while(i--)
			if(ti)
				ti=ti->SwitchItem(true);
			else
				break;
		if(associatedWindow)
		{
			CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(associatedWindow->GetFocusEl());	// switch zmenul focus na novou polozku, vrati mi to nove oznacenou polozku
			ti->EventArise(EEvent);	// domluvena udalost pro Tree, ktera mu rekne, ze bylo najeto na polozku klavesnici - Tree vola TreeHandler
		}
	}


	EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	*/
	return 0;
}
