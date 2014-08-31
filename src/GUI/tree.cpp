///////////////////////////////////////////////
//
// tree.cpp
//
// Implemementace pro tree (seznam polozek usporadanych do stromu)
// a jeho konkretni vyuziti - CGUIFileBrowser (prochazec souboru a adresaru na disku) 
//
// A: Jan Poduska
//
///////////////////////////////////////////////


#include "stdafx.h"

#include "tree.h"


#include "gui.h"
#include "menu.h"

#include "fs.h"
#include "dialogs.h"

#include "kernel.h"
#include "krkal.h"
#include "dxbliter.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////
// CGUITreeItem
//////////////////////////////////////////////////////////////////////

const int CGUITreeItem::numEvents = 10;
const int CGUITreeItem::availableEvents[numEvents] = {EMouseOver,EMouseButton,EClicked,
									   				  EFocusChanged,EDeleted,EKeyboard,
													  ETreeItemUpdate,EEvent,EBadInput,
													  ETreeItemRoller};

CGUITreeItem::CGUITreeItem(CGUIWindow* win, float _x, float _y, char *labelString, CGUIRectHost *_picture, char *_help)
	: CGUIElement(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	rootwnd=0;
	active=true;
	markItem=false;
	index=0;
	itemMarker=0;
	manualUpdate=false;
	prepared=false;
	userTagType=0;
	memset(&ut,0,sizeof(userTag));
	compareFunction=&Compare;

	rootBackground=false;
	itemBackground=0;

	items = new CGUIList<CGUITreeItem>(false);
	associatedWindow=win;
	tree=0;
	rootTree=true;
	parentItem=0;
	state=0;
	stateChange=false;
	pictureRoller=0;

	elem=0;
	inLineElem=false;
	itemVisible=true;
	sliderPosition=0;

	SetMark(false);

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseR|MsgKeyboard|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	SetSize(0,STD_TREE_HEIGHT);

	if(labelString)
		label = new CGUIStaticText(labelString,(CGUIFont*)RefMgr->Find("GUI.F.Arial.10"),0,0,STD_TREE_TEXT_COLOR);
	else
		label=0;

	picture = pictureInactive = pictureActive = _picture;

	SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Rollers")->GetTexture(2)));

	float nsx;
	SetPositionOfLabelPicture(&nsx);

	SetSize(nsx,STD_TREE_HEIGHT);

	if(_help)
	{
		help = newstrdup(_help);
	}else
		help = 0;

	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();

	SetVisible(0);

	AddToTrash();
}

CGUITreeItem::CGUITreeItem(CGUITree* _tree, char *labelString, bool _rootTree, CGUIRectHost *_picture, char *_help, bool rootItemBackground)
	: CGUIElement(0,0), CGUIEventSupport(availableEvents, numEvents)
{
	rootwnd=0;
	active=true;
	markItem=false;
	index=0;
	itemMarker=0;
	manualUpdate=false;
	prepared=false;
	userTagType=0;
	memset(&ut,0,sizeof(userTag));
	compareFunction=&Compare;

	items = new CGUIList<CGUITreeItem>(false);
	associatedWindow=_tree;
	tree=_tree;
	rootTree=_rootTree;
	parentItem=0;
	state=0;
	stateChange=false;
	pictureRoller=0;

	elem=0;
	inLineElem=false;
	itemVisible=true;
	sliderPosition=0;

	SetMark(false);

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseR|MsgKeyboard|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	float itemHeight;
	if(tree)
	{
		if(rootTree)
			itemHeight=tree->treeRootItemHeight;
		else
			itemHeight=tree->treeItemHeight;
	}else
		itemHeight=STD_TREE_HEIGHT;

	SetSize(0,itemHeight);

	if(labelString)
	{
		int color;
		CGUIFont* font;

		if(tree)
		{
			color=tree->treeTextColor;
			if(rootTree)
				font=tree->treeRootTextFont;
			else
				font=tree->treeTextFont;
		}else{
			color=STD_TREE_TEXT_COLOR;
			font=(CGUIFont*)RefMgr->Find("GUI.F.Arial.10");
		}
		label = new CGUIStaticText(labelString,font,0,0,color);
	}else
		label=0;

	picture = pictureInactive = pictureActive = _picture;

	if(tree)
		SetRollerPicture(new CGUIRectHost(0,0,tree->itemTex));
	else
		SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller0")->GetTexture(0)));

	float nsx;
	SetPositionOfLabelPicture(&nsx);

	SetSize(nsx,itemHeight);

	rootBackground=rootItemBackground;
	if(rootBackground)
	{
		float itemWidth,yy;
		UINT color;
		UINT linesColor;
		if(tree)
		{
			tree->GetBackWindowSize(itemWidth,yy);
			color=tree->treeRootItemBackgroundColor;
			linesColor=tree->treeRootItemLinesColor;
		}else{
			itemWidth=sx;
			color=STD_TREE_ROOTITEM_BACKGROUND_COLOR;
			linesColor=STD_TREE_ROOTITEM_LINES_COLOR;
		}

		itemBackground=new CGUITreeRootItemBackground(0,0,itemWidth,itemHeight,color,linesColor);
		itemBackground->RemoveFromTrash();
	}else
		itemBackground=0;


	if(_help)
	{
		help = newstrdup(_help);
	}else
		help = 0;

	SetVisible(0);

	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();

	AddToTrash();
}

void CGUITreeItem::SetLabel(char *text)
{
	CBltElem* pos=0;
	if(label)
		pos = label->GetElemPosition();
	SAFE_DELETE(label);
	if(!text)
		return;
	
	int color;
	CGUIFont* font;

	if(tree)
	{
		color=tree->treeTextColor;
		if(rootTree)
			font=tree->treeRootTextFont;
		else
			font=tree->treeTextFont;
	}else{
		color=STD_TREE_TEXT_COLOR;
		font=(CGUIFont*)RefMgr->Find("GUI.F.Arial.10");
	}

	if(!font)
		font=(CGUIFont*)RefMgr->Find("GUI.F.Arial.10");

	label = new CGUIStaticText(text,font,0,0,color);
	label->RemoveFromTrash();


	float nsx;
	SetPositionOfLabelPicture(&nsx);
	SetSize(nsx,sy);

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUITreeItem::SetPicture(CGUIRectHost *_picture)
{
	CBltElem* pos=0;
	if(_picture)
		_picture->RemoveFromTrash();
	
	if(active)
	{
		if(picture)
			pos = picture->GetElemPosition();
		if(picture==pictureInactive)
			pictureInactive = _picture;
		SAFE_DELETE(picture);
		picture = _picture;
		SetPositionOfLabelPicture(0);
	}else{
		if(picture==pictureActive)
		{
			if(picture)	
				pos = picture->GetElemPosition();
			SAFE_DELETE(picture);
			picture = _picture;
			pictureInactive = _picture;
			SetPositionOfLabelPicture(0);
		}else
			SAFE_DELETE(pictureActive);
	}

	pictureActive = _picture;

	if(picture)
		picture->SetVisible(IsVisible());

	if((active || picture==pictureInactive) && picture && rootwnd)
	{
		if(pos)
			picture->AddToEngineBackPosition(rootwnd,pos);
		else
			picture->AddToEngine(rootwnd);
	}
}

void CGUITreeItem::ChangeContextHelp(char* text)
{
	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	if(text)
	{
		help = newstrdup(text);
	}else
		help = 0;
}

void CGUITreeItem::SetPositionOfLabelPicture(float *ti_sx)
{
	float pos_x=0, pos_y=0;					// atributy polozky
	float p_x=0, p_y=0, p_sx=0, p_sy=0;			// atributy obrazku
	float l_x=0, l_y=0, l_sx=0, l_sy=0;			// atributy popisku

	GetPos(pos_x,pos_y);
	
	float itemHeight=sy;
	float pictureSize;

	if(tree)
		pictureSize = min(itemHeight,tree->treeItemPictureSize);
	else
		pictureSize = min(itemHeight,STD_TREE_PICTURE_SIZE);

	if(pictureRoller)
	{
		pictureRoller->GetSize(p_sx,p_sy);
		p_x=pos_x+(STD_TREE_ROLLER_SIZE-p_sx)/2;
		p_y=pos_y+(itemHeight-p_sy)/2;
		pictureRoller->Move(floorf(p_x),floorf(p_y));
	}

	if(picture)
	{
	    picture->GetSize(p_sx,p_sy);
		if(p_sx>pictureSize && p_sy>pictureSize)
			picture->Resize(pictureSize,pictureSize);
		else if(p_sx>pictureSize)
			picture->Resize(pictureSize,p_sy);
		else if(p_sy>pictureSize)
			picture->Resize(p_sx,pictureSize);

		picture->GetSize(p_sx,p_sy);
		p_x=pos_x+STD_TREE_ROLLER_SIZE+STD_TREE_GAP+(pictureSize-p_sx)/2;
		p_y=pos_y+(itemHeight-p_sy)/2;
		picture->Move(floorf(p_x),floorf(p_y));
	}


	if(label)
	{
		label->GetSize(l_sx,l_sy);
		l_x=pos_x+STD_TREE_ROLLER_SIZE+pictureSize+2*STD_TREE_GAP;
		l_y=pos_y+(itemHeight-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));
	}

	if(ti_sx)
		*ti_sx=STD_TREE_ROLLER_SIZE+pictureSize+2*STD_TREE_GAP+l_sx;
}

CGUITreeItem::~CGUITreeItem()
{
	RemoveFromTrash();

	switch(userTagType)
	{
		case eUTnone	: break;	// neplatna polozka v userTag
		case eUTstring	: SAFE_DELETE_ARRAY(ut.string);
		case eUTkerName	: if(KerMain && ut.obj)
							try
							{
								KerMain->DeleteObject(0,ut.obj);
							}catch(CKernelPanic){
								KerErrorFiles->ShowAllLogs(0,40,640,200);
								SAFE_DELETE(KerMain);
							}
						  break;
		case eUTopointer: break;
		case eUTautomatism: break;
		case eUTint : break;
		case eUTdataClass : SAFE_DELETE(ut.userDataClass);break;
		case eUTOVar : break;
		case eUTOVarDEL : SAFE_DELETE(ut.OVar);break;
		case eUTregister : break;
	}

	if(label)
		delete label;

	if(itemBackground)
		delete itemBackground;

	if(pictureActive!=pictureInactive)
	{
		SAFE_DELETE(pictureActive);
		SAFE_DELETE(pictureInactive);
	}else
		SAFE_DELETE(pictureActive);

	SAFE_DELETE(pictureRoller);

	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}

	if(rootTree && itemMarker)
		delete itemMarker;

	if(items)
		delete items;

	EventArise(EDeleted,0,0,this);
}

void CGUITreeItem::SetInactivePicture(CGUIRectHost *_pictureInactive)
{
	if(_pictureInactive)
		_pictureInactive->RemoveFromTrash();

	if(active)
	{
		if(picture!=pictureInactive)
			SAFE_DELETE(pictureInactive);
		pictureInactive = _pictureInactive;
	}else{

		CBltElem* pos=0;
		
		if(picture)	
			pos = picture->GetElemPosition();

		if(picture==pictureActive)
		{
			picture = _pictureInactive;
			pictureInactive = _pictureInactive;
		}else{
			SAFE_DELETE(picture);
			picture = _pictureInactive;
			pictureInactive = _pictureInactive;
		}

		SetPositionOfLabelPicture(0);
		
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}
}

void CGUITreeItem::SetRollerPicture(CGUIRectHost *_pictureRoller)
{
	CBltElem* pos=0;
	if(_pictureRoller)
		_pictureRoller->RemoveFromTrash();

	if(rootwnd && pictureRoller)
	{
		pos = pictureRoller->GetElemPosition();
	}

	SAFE_DELETE(pictureRoller);

	pictureRoller = _pictureRoller;

	SetPositionOfLabelPicture(0);

	if(pictureRoller && rootwnd)
	{
		//pictureRoller->AddToEngine(rootwnd);
		///*
		if(pos)
			pictureRoller->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureRoller->AddToEngine(rootwnd);
		//	*/
	}


}

int CGUITreeItem::Activate()
{
	int ret=0;
	CBltElem* pos=0;
	if(active)
		return ret;

	if(pictureActive!=picture)
	{
		if(picture)
		{
			pos = picture->GetElemPosition();
			picture->RemoveFromEngine();
		}
		picture = pictureActive;
		SetPositionOfLabelPicture(0);
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}


	if(label)
	{
		if(tree)
			label->SetColor(tree->treeTextColor);
		else
			label->SetColor(STD_MENU_TEXT_COLOR);
	}

	active=true;
	return ret;
}

int CGUITreeItem::Deactivate()
{
	int ret=0;
	CBltElem* pos=0;
	if(!active)
		return ret;

	if(pictureInactive!=picture)
	{
		if(picture)
		{
			pos = picture->GetElemPosition();
			picture->RemoveFromEngine();
		}
		picture = pictureInactive;
		SetPositionOfLabelPicture(0);
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}
	
	if(label)
	{
		if(tree)
			label->SetColor(tree->treeTextInactiveColor);
		else
			label->SetColor(STD_TREE_TEXT_INACTIVE_COLOR);
	}

	active=false;
	return ret;
}

void CGUITreeItem::SetRollState(int roll_state, bool synchronous)	// 0 - zabalit, 1 - rozbalit
{
	bool change=false;
	if(state==1 && roll_state==1)
	{
		state=2;
		stateChange=true;
		if(manualUpdate)
		{
			if(synchronous && tree)
			{
				tree->TreeUpdateHandler(GetID(),index);
			}
			else
				EventArise(ETreeItemUpdate,index);
		}else
			UpdateTree();
		change=true;
	}else if(state==2 && roll_state==0)
	{
		state=1;
		stateChange=true;
		if(manualUpdate)
		{
			if(synchronous && tree)
			{
				tree->TreeUpdateHandler(GetID(),index);
			}
			else
				EventArise(ETreeItemUpdate,index);
		}else
			UpdateTree();
		change=true;
	}else if(state==0 && roll_state==1)
	{
		if(manualUpdate)
		{
			if(synchronous && tree)
			{
				tree->TreeUpdateHandler(GetID(),index);
			}
			else
				EventArise(ETreeItemUpdate,index);
		}
		change=true;
	}

	if(change)
		EventArise(ETreeItemRoller,index,0,0,0,state);
}

int CGUITreeItem::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	bool rollOnClick = tree ? tree->rollOnClick : true;

	if(mouseState)
	{
		InnerCoords(x,y);
		if(markItem || (x<STD_TREE_ROLLER_SIZE))
		{
			if(rollOnClick || (x<STD_TREE_ROLLER_SIZE))
			{
				// rozbalit podstrom
				if(state==1)
				{
					state=2;
					stateChange=true;
					if(manualUpdate)
					{
						EventArise(ETreeItemUpdate,index);
					}else
						UpdateTree();
				}else if(state==2)
				{
					state=1;
					stateChange=true;
					if(manualUpdate)
					{
						EventArise(ETreeItemUpdate,index);
					}else
						UpdateTree();
				}else if(state==0)
				{
					if(manualUpdate)
						EventArise(ETreeItemUpdate,index);
				}

				EventArise(ETreeItemRoller,index,0,0,0,state);
			}
	
			if(!(!rollOnClick && (x<STD_TREE_ROLLER_SIZE)))
			{
				OnClick();
				//EventArise(EMouseButton,0,mouseState);
				EventArise(EClicked);
			}
		}else{
		}
		if(!markItem)
		{
			Mark();
		}

		if(rollOnClick || (x>STD_TREE_ROLLER_SIZE))
			EventArise(EMouseButton,0,mouseState);

	}else{

		
	}

	return 0;
}

int CGUITreeItem::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		EventArise(EFocusChanged,focus);
		return 0;
	}

	//UnMark();

	if(help)
		CGUIContextHelp::Off(this);

	EventArise(EFocusChanged,focus);
	return 0;
}

int CGUITreeItem::MouseOver(float x, float y, UINT over, UINT state)
{
	if(!active)
		return 0;

	EventArise(EMouseOver);

	if(help && over && !(state & MK_LBUTTON) && !(state & MK_RBUTTON) && !(state & MK_MBUTTON))
	{
		InnerCoords(x,y);
		if(x>STD_TREE_ROLLER_SIZE)
			CGUIContextHelp::On(this,help);
	}

	return 0;
}

int CGUITreeItem::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

int CGUITreeItem::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

void CGUITreeItem::OnClick()
{
}

int CGUITreeItem::Keyboard(UINT character, UINT state)
{
	if(!active)
		return 0;

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
	return 0;
}

CGUITreeItem* CGUITreeItem::SwitchItem(bool next)
{
	CGUITreeItem* ret=this;
	if(state==0 || state==1 || (state==2 && !next))
	{
		if(parentItem)
			ret=parentItem->SwitchItemIndex(index,next);	// zavalom rodickovkou polozku se zadosti o posun, predam ji svoje poradi (index) a smer posunu
		if(rootTree && tree)
			ret=tree->SwitchItemIndex(index,next);	// jsem korenova polozka, zavalom strom se zadosti o posun, predam mu svoje poradi (index) a smer posunu
	}else{	
		// state==2 && next
		ret=Switch(items->GetFromHead());	// jsem na koreni rozbaleneho podstromu a chci jit dolu
											// prejdu na prvni polozku ve svem podstromu (prvni syn korene)
	}
	return ret;
}

CGUITreeItem* CGUITreeItem::Switch(CGUITreeItem* si)
{
	if(!si)
		return 0;

	if(associatedWindow)
	{
		CGUITreeItem* oi = dynamic_cast<CGUITreeItem*>(associatedWindow->GetFocusEl());
		if(oi)
			oi->UnMark();	// odoznaci starou polozku
		associatedWindow->SetFocusEl(si,0);	// focusuje novou polozku

		float xx, yy;
		float isx, isy;

		si->GetPos(xx,yy);
		si->GetSize(isx,isy);

		float wx, wy;
		float wsx, wsy;

		associatedWindow->GetVPPos(wx,wy);
		associatedWindow->GetVPSize(wsx,wsy);

		if(yy<wy)
			associatedWindow->SetWindowPositionVP(0,yy-wy-10);	// nascroluje okno na polozku, pokud uz by jinak byla mimo okno
		else if(yy+isy>wy+wsy)
			associatedWindow->SetWindowPositionVP(0,yy+isy-wy-wsy+10);	// nascroluje okno na polozku, pokud uz by jinak byla mimo okno
	}

	si->Mark();	// oznaci novou polozku
	return si;
}

CGUITreeItem* CGUITreeItem::SwitchItemIndex(int ind, bool next)
{
	CGUITreeItem* ti;

	if(next)
	{
		ti = FindIndex(ind+1);	// hleda polozku nasledujiciho indexu (mezi svymi syny)
		if(ti)
		{
			return Switch(ti);	// nasel ji, presun se na ni (jsem hotov)
		}else{
			// nenasel ji (jsem na konci sveho podstromu, prepni se na nasledujici polozku rodicovskeho podstromu)
			if(parentItem)
				return parentItem->SwitchItemIndex(index,next);	// zavolej rekurzivni pozadavek na otci
			if(rootTree && tree)
				return tree->SwitchItemIndex(index,next);	// zavolej rekurzivni pozadavek na stromu (jsem korenova polozka, nemam otce)
		}
	}else{
		ti = FindIndex(ind-1);	// hleda polozku predchoziho indexu (mezi svymi syny)
		if(ti)
		{
			// nasel ji
			if(ti->state==0 || ti->state==1)	// predchazejici polozka je list, nebo zavreny podstrom
				return Switch(ti);	// prepni se na ni
			else{
				// predchazejici polozka je rozbaleny podstrom
				return Switch(ti->FindLastSubItem());	// prepni se na posledni polozku v rozbalenem podstromu
			}
		}else{
			return Switch(this);	// zadna predchazejici polozka podstromu neni, prepni se na koren, to jsem ja (this)
		}
	}
	return this;
}

CGUITreeItem* CGUITreeItem::FindIndex(int ind)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->index==ind)
				return e;
		}
		le=items->GetNextListElement(le);
	}

	return 0;
}

CGUITreeItem* CGUITreeItem::FindLastSubItem()
{
	CGUITreeItem* ti = items->GetFromBack();
	if(!ti)
		return this;
	if(ti->state==2)
		return ti->FindLastSubItem();
	else
		return ti;
}

int* CGUITreeItem::GetTreePath(int &size)
{
	size=1;
	CGUITreeItem* ti=this;

	while(ti->parentItem)
	{
		ti=ti->parentItem;
		size++;
	}

	int *path = new int[size];

	ti=this;
	int i=size;
	while(ti->parentItem)
	{
		path[--i]=ti->index;
		ti=ti->parentItem;
	}
	path[0]=ti->index;

	return path;
}

char* CGUITreeItem::GetLabelText()
{
	if(label)
		return label->GetText();
	else
		return 0;
}

CGUITreeItem* CGUITreeItem::FindItemFromElement(CGUIElement* elem)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *retItem=0;

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->elem == elem)
				return e;
			retItem = e->FindItemFromElement(elem);
			if(retItem)
				return retItem;
		}
		le=items->GetNextListElement(le);
	}
	return 0;
}

CGUITreeItem* CGUITreeItem::GetItemWithIndex(int index)
{
	if(!items)
		return 0;
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->index == index)
				return e;
		}
		le=items->GetNextListElement(le);
	}
	return 0;
}


void CGUITreeItem::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;


	delete event;
}

int CGUITreeItem::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti pravym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	if(mouseState)
	{
		if(markItem)
		{
//			OnClick();
//			EventArise(EMouseButton,1,mouseState);
//			EventArise(EClicked);
		}else{
			Mark();
		}

		EventArise(EMouseButton,1,mouseState);
	}else{

	}

	return 0;
}

void CGUITreeItem::AddToEngine(CBltWindow *rootwnd)
{
	CGUITreeItem::rootwnd = rootwnd;
	if(!itemMarker && rootTree)	// korenova polozka se stara o oznacovani sebe a celeho sveho podstromu
	{
		itemMarker = new CGUITreeMarker(x,y,x+sx+2,y+sy,rootwnd);
		itemMarker->UnMark();
	}

	//if(itemBackground)
		//itemBackground->AddToEngine(rootwnd);

	if(label)
		label->AddToEngine(rootwnd);
	if(picture)
		picture->AddToEngine(rootwnd);
	if(pictureRoller)
		pictureRoller->AddToEngine(rootwnd);
}

void CGUITreeItem::AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos)
{
	CGUITreeItem::rootwnd = rootwnd;

	if(!itemMarker && rootTree)	// korenova polozka se stara o oznacovani sebe a celeho sveho podstromu
	{
		itemMarker = new CGUITreeMarker(x,y,x+sx+2,y+sy,rootwnd,pos);
		itemMarker->UnMark();
	}

	if(rootBackground)
	{
		//if(itemBackground)
		//itemBackground->AddToEngine(rootwnd);

		if(label)
			label->AddToEngine(rootwnd);
		if(picture)
			picture->AddToEngine(rootwnd);
		if(pictureRoller)
			pictureRoller->AddToEngine(rootwnd);
	}else{
		//if(itemBackground)
			//itemBackground->AddToEngineBackPosition(rootwnd,pos);

		if(label)
			label->AddToEngineBackPosition(rootwnd,pos);
		if(picture)
			picture->AddToEngineBackPosition(rootwnd,pos);
		if(pictureRoller)
			pictureRoller->AddToEngineBackPosition(rootwnd,pos);
	}
}

void CGUITreeItem::RemoveFromEngine()
{
	CGUITreeItem::rootwnd=0;
	//if(itemBackground)
	//	itemBackground->RemoveFromEngine();
	if(itemBackground && tree)
		tree->RemoveBackElem(itemBackground);
	if(label)
		label->RemoveFromEngine();
	if(picture)
		picture->RemoveFromEngine();
	if(pictureRoller)
		pictureRoller->RemoveFromEngine();
}

void CGUITreeItem::SetVisible(int vis)
{
	SetVisibility(vis);
	if(itemBackground)
		itemBackground->SetVisible(vis);
	if(label)
		label->SetVisible(vis);
	if(picture)
		picture->SetVisible(vis);
	if(pictureRoller)
		pictureRoller->SetVisible(vis);
}

void CGUITreeItem::BringToTop()
{
	if(itemBackground)
		itemBackground->BringToTop();
	if(label)
		label->BringToTop();
	if(picture)
		picture->BringToTop();
	if(pictureRoller)
		pictureRoller->BringToTop();
}

void CGUITreeItem::Move(float _x, float _y)
{
	SetPos(_x,_y);

/*
	if(label)
		label->Move(_x,_y);

	if(pictureActive)
		pictureActive->Move(_x,_y);

	if(pictureInactive && pictureInactive!=pictureActive)
		pictureInactive->Move(_x,_y);

	if(pictureExtender)
		pictureExtender->Move(_x,_y);
*/
	SetPositionOfLabelPicture(0);

	if(itemBackground)
		itemBackground->Move(0,y);

	if(itemMarker && markItem)
		itemMarker->Rebuild(this);

	/*
	if(marker)
		marker->Rebuild(this);
		*/
}

void CGUITreeItem::UpdateRootBackground(float bsx)
{
	if(itemBackground)
		itemBackground->Resize(bsx,sy);
}

void CGUITreeItem::ChangeRoller()
{
	if(tree)
	{
		switch(state)
		{
			case 0 : SetRollerPicture(new CGUIRectHost(0,0,tree->itemTex));break;
			case 1 : SetRollerPicture(new CGUIRectHost(0,0,tree->rollTex));break;
			case 2 : SetRollerPicture(new CGUIRectHost(0,0,tree->unrollTex));break;
		}
	}else
	{
		switch(state)
		{
			case 0 : SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller0")->GetTexture(0)));break;
			case 1 : SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller+")->GetTexture(0)));break;
			case 2 : SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller-")->GetTexture(0)));break;
		}
	}
	pictureRoller->SetVisible(IsVisible());
	SetPositionOfLabelPicture(0);
}

void CGUITreeItem::DistributeUpdate(float &px, float &py, float &maxx, bool visible)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			e->Update(px,py,maxx,visible);
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITreeItem::UpdateTree()
{
	float px,py;
	bool visible;
	CGUITreeItem* ti;
	float min_x, min_y, max_x, max_y;

	if(tree)	// pokud jsem zarazen ve strome, volam update na cely strom
	{
		tree->UpdateTree();
		return;
	}

	ti=this;
	while(ti && !ti->rootTree)	// hledam polozku nejvyssi urovne - korenovou
		ti=ti->parentItem;

	if(!ti)
		return;

	if(ti->tree)	// pokud je polozka zarazena ve strome, volam update na cely strom
	{
		ti->tree->UpdateTree();
		return;
	}

	ti->GetPos(px,py);

	ti->GetSize(max_x,max_y);
	min_x=px;min_y=py;
	max_x+=min_x;max_y+=min_y;

	if(ti->elem)	// upadate polohy pridruzeneho elementu, vypocet pozice pro nasledujici polozky
	{
		float esx,esy;
		ti->elem->GetSize(esx,esy);
		if(ti->inLineElem)
			py+=(STD_TREE_HEIGHT<esy) ? esy : STD_TREE_HEIGHT;
		else{
			ti->elem->Move(px+STD_TREE_SHIFT,py+STD_TREE_HEIGHT+STD_TREE_ELEMENT_SHIFT);
			py+=STD_TREE_HEIGHT+esy+2*STD_TREE_ELEMENT_SHIFT;
		}
	}else
		py+=STD_TREE_HEIGHT;

	px+=STD_TREE_SHIFT;
	if(ti->stateChange)
		ti->ChangeRoller();

	if(ti->state==0)
		return;
	else if(ti->state==1)
		visible=0;
	else if(ti->state==2)
		visible=1;

	ti->DistributeUpdate(px,py,max_x,visible);		// update celeho sveho podstromu
	max_y=py;

	SetSliderPositionFlooding(max_x+10);	// nastaveni pozice slideru pro radkove elementy

	UpdateInlineElems(max_x);	// update radkovych elementu podle nove pozice slideru
	
	associatedWindow->SetBackWindowNormalSize(max_x+STD_TREE_END_GAP_X,max_y+STD_TREE_END_GAP_Y);	// nastaveni velikosti okna podle zmeny velikosti stromu
}

void CGUITreeItem::Update(float &px, float &py, float &maxx, bool visible)
{
	if(state==0)
	{
		if(!visible){
			SetVisible(0);
			if(elem)
				elem->SetVisible(0);
		}
		else
		{
			Move(px,py);
			if(elem)
			{
				float esx,esy;
				elem->GetSize(esx,esy);
				if(inLineElem)
					py+=(sy<esy) ? esy : sy;
				else{
					if(itemVisible)
					{
						elem->Move(px+STD_TREE_SHIFT,py+sy+STD_TREE_ELEMENT_SHIFT);
						py+=sy+esy+2*STD_TREE_ELEMENT_SHIFT;
					}else{
						if(IsVisible())
							SetVisible(0);
						elem->Move(px,py+STD_TREE_ELEMENT_SHIFT);
						py+=esy+2*STD_TREE_ELEMENT_SHIFT;
					}
				}
				if(!elem->IsVisible())
					elem->SetVisible(1);
			}else
				py+=sy;
			maxx = (maxx < px+sx) ? px+sx : maxx;
			if(!IsVisible() && itemVisible)
			{
				SetVisible(1);
			}
		}
	}

	if(state==1)
	{
		if(!visible)
		{
			SetVisible(0);
			if(elem)
				elem->SetVisible(0);
		}
		else
		{
			Move(px,py);
			if(elem)
			{
				float esx,esy;
				elem->GetSize(esx,esy);
				if(inLineElem)
					py+=(sy<esy) ? esy : sy;
				else{
					if(itemVisible)
					{
						elem->Move(px+STD_TREE_SHIFT,py+sy+STD_TREE_ELEMENT_SHIFT);
						py+=sy+esy+2*STD_TREE_ELEMENT_SHIFT;
					}else{
						if(IsVisible())
							SetVisible(0);
						elem->Move(px,py+STD_TREE_ELEMENT_SHIFT);
						py+=esy+2*STD_TREE_ELEMENT_SHIFT;
					}
				}
				if(!elem->IsVisible())
					elem->SetVisible(1);
			}else
				py+=sy;

			maxx = (maxx < px+sx) ? px+sx : maxx;
			if(!IsVisible() && itemVisible)
			{
				SetVisible(1);
			}
			if(stateChange)
				DistributeUpdate(px,py,maxx,false);
		}
	}

	if(state==2)
	{
		if(!visible)
		{
			SetVisible(0);
			if(elem)
				elem->SetVisible(0);
			DistributeUpdate(px,py,maxx,false);
		}
		else
		{
			Move(px,py);
			if(elem)
			{
				float esx,esy;
				elem->GetSize(esx,esy);
				if(inLineElem)
					py+=(sy<esy) ? esy : sy;
				else{
					if(itemVisible)
					{
						elem->Move(px+STD_TREE_SHIFT,py+sy+STD_TREE_ELEMENT_SHIFT);
						py+=sy+esy+2*STD_TREE_ELEMENT_SHIFT;
					}else{
						if(IsVisible())
							SetVisible(0);
						elem->Move(px,py+STD_TREE_ELEMENT_SHIFT);
						py+=esy+2*STD_TREE_ELEMENT_SHIFT;
					}
				if(!elem->IsVisible())
					elem->SetVisible(1);
				}
			}else
				py+=sy;

			if(rootTree && tree)
				py+=tree->treeRootNormalItemGap;

			float xx = px+STD_TREE_SHIFT;
			maxx = (maxx < px+sx) ? px+sx : maxx;
			if(!IsVisible() && itemVisible)
			{
				SetVisible(1);
			}
			DistributeUpdate(xx,py,maxx,true);

			if(rootTree && tree)
				py+=tree->treeRootNormalItemGap;
		}
	}

	if(stateChange)
		ChangeRoller();
	stateChange=false;
}

void CGUITreeItem::UpdateInlineElems(float &maxx)
{
	float ex, ey, esx, esy;
	float max;
	if(elem && elem->IsVisible())
	{
		elem->GetSize(esx,esy);
		if(inLineElem)
		{
			float py;
			if(esy>=sy)
				py=y;
			else
				py=floorf(y+(sy-esy)/2+1);

			if(sliderPosition)
				elem->Move(sliderPosition+STD_TREE_AFTER_SLIDER_GAP,py);
			else if(tree)
				elem->Move(tree->sliderPosition+STD_TREE_AFTER_SLIDER_GAP,py);
		}
		elem->GetPos(ex,ey);
		max = ex+esx;
		maxx = (max>maxx) ? max : maxx;
	}

	if(items && state==2)
	{
		CGUITreeItem *e;
		CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

		while(le)
		{
			if((e=items->GetObjectFromListElement(le)))
			{
				e->UpdateInlineElems(maxx);
			}
			le=items->GetNextListElement(le);
		}
	}
}

void CGUITreeItem::Mark()
{
	if(markItem)
		return;

	///*
	if(tree && tree->lastTreeItem && tree->lastTreeItem!=this && tree->lastTreeItem->markItem)
	{
		tree->lastTreeItem->UnMark();
	}
	//*/

	/*
	if(tree && tree->lastTreeItem)
	{
		tree->lastTreeItem->UnMark();
	}else{
		tree->lastTreeItem=tree->lastTreeItem;
	}
	//*/

	if(rootBackground)
	{
		UINT color;
		UINT linesColor;
		if(tree)
		{
			color=tree->treeRootItemMarkBackgroundColor;
			linesColor=tree->treeRootItemLinesColor;
		}else{
			color=STD_TREE_ROOTITEM_BACKGROUND_MARK_COLOR;
			linesColor=STD_TREE_ROOTITEM_LINES_COLOR;
		}
		if(itemBackground)
			itemBackground->SetColor(color,linesColor);
	}else if(itemMarker)
	{
		itemMarker->Rebuild(this);
		itemMarker->Mark();
	}
	if(label)
		label->SetColor(STD_TREE_MARK_TEXT_COLOR);
	markItem=true;

	if(tree)
		tree->lastTreeItem=this;
}

void CGUITreeItem::UnMark()
{
	if(!markItem)
		return;

	if(rootBackground)
	{
		UINT color;
		UINT linesColor;
		if(tree)
		{
			color=tree->treeRootItemBackgroundColor;
			linesColor=tree->treeRootItemLinesColor;
		}else{
			color=STD_TREE_ROOTITEM_BACKGROUND_COLOR;
			linesColor=STD_TREE_ROOTITEM_LINES_COLOR;
		}
		if(itemBackground)
			itemBackground->SetColor(color,linesColor);
	}else if(itemMarker)
	{
		itemMarker->UnMark();
	}
	if(label)
		label->SetColor(STD_TREE_TEXT_COLOR);
	markItem=false;
}

void CGUITreeItem::ReIndex(CGUITreeItem *start, int d_index, bool inclusive)
{
	// (inclusive==false) start jiz neposouvam, (inclusive==true) posouvam i start
	// pokud je start == 0 posouvam vsechny polozky od zacatku seznamu
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	bool found;

	if(!start)
		found=true;
	else
		found=false;

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(found)
			{
				e->index+=d_index;
			}
			if(e==start)
			{
				found=true;
				if(inclusive)
					e->index+=d_index;
			}
		}
		le=items->GetNextListElement(le);
	}
}

CGUITreeItem* CGUITreeItem::AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool _manualUpdate, bool first)
{
	CGUITreeItem *ti;
	CGUITreeItem* predek;
	int index=0;

	predek = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(where));
	if(predek && predek->parentItem==this)
	{
		if(first)
		{
			index=predek->index;
			ReIndex(predek,1,true);
		}
		else
		{
			index=predek->index+1;
			ReIndex(predek,1);
		}
	}else{
		if(first)
		{
			predek=0;
			index=0;
			ReIndex(0,1);
		}else{
			predek = items->GetFromBack();
			if(predek)
			{
				index=predek->index+1;
			}
		}
	}

	if(tree)
		ti = new CGUITreeItem(tree,labelString,false,_picture,_help);
	else
		ti = new CGUITreeItem(associatedWindow,0,0,labelString,_picture,_help);
	ti->index=index;
	ti->rootTree=false;
	ti->itemMarker=itemMarker;
	ti->parentItem=this;
	ti->tree=tree;
	ti->compareFunction=compareFunction;
	ti->manualUpdate=_manualUpdate;
	if(_manualUpdate)
		ti->AcceptEvent(tree->GetID(),ETreeItemUpdate);
	ti->AcceptEvent(tree->GetID(),EClicked);
	ti->AcceptEvent(tree->GetID(),EMouseButton);
	ti->AcceptEvent(tree->GetID(),EEvent);

	if(predek)
	{
		if(first)
			items->AddToFrontObj(predek,ti);
		else
			items->AddToBehindObj(predek,ti);
	}
	else
	{
		items->AddToHead(ti);
	}

	if(state==0)
	{
		state=1;
		ChangeRoller();
		//SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller+")->GetTexture(0)));
	}

	if(tree)
		tree->AddTreeItemEngine(ti);
	else
		associatedWindow->AddBackElem(ti);
	ti->SetVisible(0);

	return ti;
}



CGUITreeItem* CGUITreeItem::AddTreeItem(typeID where, CGUITreeItem* treeItem, bool _manualUpdate, bool first)
{
	// !@#$ dodelat 

	return 0;
}


int CGUITreeItem::DeleteTreeItem(CGUITreeItem* ti)
{
	if(!ti)
		return 1;

	if(!items || !items->IsMember(ti))
		return 1;

	// rekurzivni smazani vsech podpolozek mazane polozky
	while(CGUITreeItem* tti = ti->items->GetFromHead())
	{
		ti->DeleteTreeItem(tti);
	}

	if(tree && ti==tree->lastTreeItem)
	{
		tree->lastTreeItem->UnMark();
		tree->lastTreeItem=0;
	}

	ReIndex(ti,-1);

	items->Remove(ti);
	if(ti->elem && associatedWindow)
		associatedWindow->DeleteBackElem(ti->elem);
	if(associatedWindow)
		associatedWindow->DeleteBackElem(ti);

	if(!items->GetFromHead())
	{
		state=0;
		ChangeRoller();
	}

	return 0;
}

int CGUITreeItem::Compare(CGUITreeItem *a, CGUITreeItem *b)
{
	if(a && b)
		return _stricoll(a->label->GetText(),b->label->GetText());
	else
		return 0;
}

void CGUITreeItem::Sort()
{
	CGUIListElement<CGUITreeItem>* max=0;
	while(SortItems(&max));	// setridi podstrom polozek (syni prvni urovne), pracuje na listech seznamu synu, v kazdem pruchodu najde minimum a presune ho nakonec + zarazka (max)

	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	// precislovani poradi polozek (polozky jsou nove serazeny, ale maji jeste sve puvodni indexy => precislovat dle noveho poradi)
	int i=0;
	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			ti->index=i++;
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITreeItem::SortFlooding()
{
	Sort();	// setridi prvni uroven synu polozky

	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			if(ti->state>0)	
				ti->SortFlooding();	// pokud je syn podstrom, setrid rekurzivne jeho syny
		}
		le=items->GetNextListElement(le);
	}
}

/*
void CGUITreeItem::SortItems()
{
	CGUITreeItem* ti;
	CGUIList<CGUITreeItem> *result = new CGUIList<CGUITreeItem>(false);

	ti = items->GetFromBack();

	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* a;
	CGUIListElement<CGUITreeItem>* b = a = items->GetNextListElement(0);

	while(b)
	{
		if((e=items->GetObjectFromListElement(b)))
		{
			if(e->index > ti->index/2)
				break;
		}
		b=items->GetNextListElement(b);
	}

}
*/

int CGUITreeItem::SortItems(CGUIListElement<CGUITreeItem>** max)
{
	CGUITreeItem *ea, *eb;
	CGUIListElement<CGUITreeItem>* a = items->GetNextListElement(0);	// najdi prvni prvek seznamu
	if(a==*max)
		return 0;	// seznam je jiz setriden (narazil jsem na naraznik, ukonci trideni)
	ea=items->GetObjectFromListElement(a);	// polozka v prvnim prvku seznamu (prozatimni minimum)
	CGUIListElement<CGUITreeItem>* b = items->GetNextListElement(a);	// druhy prvek seznamu

	while(b && b!=*max)
	{
		if((eb=items->GetObjectFromListElement(b)))
		{
			if((*compareFunction)(ea,eb)>0)	
				ea=eb;	// hodnota v dalsim prvku (eb) je mensi nez dosud nalezene minimum, dosad ji do minima
		}
		b=items->GetNextListElement(b);
	}

	if(ea)	// mam nalezeno minimum
	{
		items->Remove(ea);	// odstran ho ze seznamu
		items->AddToBack(ea);	// pridej ho na konec seznamu
		if(*max==0)
			*max=items->GetPrevListElement(0);	// nastaveni narazniku pri prvnim pruchodu tridenim
		return 1;
	}else
		return 0;
}

void CGUITreeItem::SetCompareFunctionSubTree(int (*compareFunction)(CGUITreeItem *, CGUITreeItem *))
{
	if(!items)
		return;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			ti->compareFunction=compareFunction;
			if(ti->state)	// polozka je korenem podstromu
				ti->SetCompareFunctionSubTree(compareFunction);	// nastav porovnavaci fci i u podstromu polozky
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITreeItem::SetSliderPositionFlooding(float sliderPos)
{
	sliderPosition=sliderPos;

	if(!items || state==0)
		return;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			ti->SetSliderPositionFlooding(sliderPos);
		}
		le=items->GetNextListElement(le);
	}
}

int CGUITreeItem::SetItemElement(CGUIElement* el, bool inLineEl)
{
	if(!associatedWindow)
		return 1;

	if(elem)
		associatedWindow->DeleteBackElem(elem);

	elem=el;
	if(elem)
	{
		elem->SetVisible(0);
		associatedWindow->AddBackElem(elem);
	}
	inLineElem=inLineEl;

	return 0;
}

void CGUITreeItem::SetItemVisibility(bool vis)
{
	itemVisible=vis;
}

//////////////////////////////////////////////////////////////////////
// CGUITree
//////////////////////////////////////////////////////////////////////

const int CGUITree::numEvents = 24;
const int CGUITree::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													EChangeTABElement,ETree,ETreeUpdate};

CGUITree::CGUITree(float _x, float _y, float _sx, float _sy, char *title_text, CGUIRectHost *_icon, UINT resizer_color, int withBackground, int bgColor)
	: CGUIStdWindow(_x, _y,_sx,_sy,title_text,_icon,true,resizer_color,withBackground,bgColor)
{
	SetAvailableEvents(availableEvents,numEvents);
	items = new CGUIList<CGUITreeItem>(false);
	compareFunction=&Compare;
	sliderPosition=0;
	lastTreeItem=0;
	treeSlider=0;
	sliderBackground=0;
	automaticSliderPos=true;
	rollTex=unrollTex=itemTex=0;
	SetRollersStyle(styleSet->Get("Rollers"));

	MsgAcceptConsume(MsgMouseL|MsgMouseR|MsgKeyboard|MsgFocusChange);

	rollOnClick=true;

	treeTextColor = STD_TREE_TEXT_COLOR;
	treeTextInactiveColor = STD_TREE_TEXT_INACTIVE_COLOR;

	treeTextFont = treeRootTextFont = (CGUIFont*)RefMgr->Find("GUI.F.Arial.10");
	treeTextFont->AddRef();
	treeRootTextFont->AddRef();

	treeItemHeight=STD_TREE_HEIGHT;
	treeRootItemBackgroundColor=STD_TREE_ROOTITEM_BACKGROUND_COLOR;
	treeRootItemMarkBackgroundColor=STD_TREE_ROOTITEM_BACKGROUND_MARK_COLOR;
	treeRootItemLinesColor=STD_TREE_ROOTITEM_LINES_COLOR;
	treeItemStartX=STD_TREE_ITEM_START_X;
	treeItemStartY=STD_TREE_ITEM_START_Y;
	treeEndGapX=STD_TREE_END_GAP_X;
	treeEndGapY=STD_TREE_END_GAP_Y;
	treeRootItemHeight=STD_TREE_HEIGHT;
	treeRootNormalItemGap=0;
	treeItemPictureSize=STD_TREE_PICTURE_SIZE;
}

CGUITree::CGUITree(float _x, float _y, float _sx, float _sy, int bgColor)
	: CGUIStdWindow(_x, _y,_sx,_sy,bgColor)
{
	SetAvailableEvents(availableEvents,numEvents);
	items = new CGUIList<CGUITreeItem>(false);
	compareFunction=&Compare;
	sliderPosition=0;
	lastTreeItem=0;
	treeSlider=0;
	sliderBackground=0;
	automaticSliderPos=true;
	rollTex=unrollTex=itemTex=0;
	SetRollersStyle(styleSet->Get("Rollers"));

	MsgAcceptConsume(MsgMouseL|MsgMouseR|MsgKeyboard|MsgFocusChange);

	rollOnClick=true;

	treeTextColor = STD_TREE_TEXT_COLOR;
	treeTextInactiveColor = STD_TREE_TEXT_INACTIVE_COLOR;

	treeTextFont = treeRootTextFont = (CGUIFont*)RefMgr->Find("GUI.F.Arial.10");
	treeTextFont->AddRef();
	treeRootTextFont->AddRef();

	treeItemHeight=STD_TREE_HEIGHT;
	treeRootItemBackgroundColor=STD_TREE_ROOTITEM_BACKGROUND_COLOR;
	treeRootItemMarkBackgroundColor=STD_TREE_ROOTITEM_BACKGROUND_MARK_COLOR;
	treeRootItemLinesColor=STD_TREE_ROOTITEM_LINES_COLOR;
	treeItemStartX=STD_TREE_ITEM_START_X;
	treeItemStartY=STD_TREE_ITEM_START_Y;
	treeEndGapX=STD_TREE_END_GAP_X;
	treeEndGapY=STD_TREE_END_GAP_Y;
	treeRootItemHeight=STD_TREE_HEIGHT;
	treeRootNormalItemGap=0;
	treeItemPictureSize=STD_TREE_PICTURE_SIZE;
}

CGUITree::~CGUITree()
{
	if(items)
		delete items;

	if(treeTextFont)
		treeTextFont->Release();
	if(treeRootTextFont)
		treeRootTextFont->Release();
/*
	if(treeSlider)
		DeleteBackElem(treeSlider);
*/
}

void CGUITree::SetTextFont(CGUIFont* font)
{
	if(treeTextFont)
		treeTextFont->Release();

	treeTextFont = font;
	if(treeTextFont)
		treeTextFont->AddRef();
}

void CGUITree::SetRootTextFont(CGUIFont* font)
{
	if(treeRootTextFont)
		treeRootTextFont->Release();

	treeRootTextFont = font;
	if(treeRootTextFont)
		treeRootTextFont->AddRef();
}

void CGUITree::SetRollersStyle(CGUIStyle* style)
{
	if(style->GetNumTextures()>=3)
	{
		rollTex = style->GetTexture(0);
		unrollTex = style->GetTexture(1);
		itemTex = style->GetTexture(2);
	}
}

void CGUITree::SetRollersTextures(CTexture* roll, CTexture* unroll, CTexture* item, bool setRoll, bool setUnroll, bool setItem)
{
	if(setRoll)
		rollTex=roll;
	if(setUnroll)
		unrollTex=unroll;
	if(setItem)
		itemTex=item;
}


CGUITreeItem* CGUITree::AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool _manualUpdate, bool _rootItemBackground, bool first)
{
	CGUITreeItem *ti;
	CGUITreeItem* predek;
	int index=0;

	predek = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(where));
	if(predek && predek->associatedWindow==this)
	{
		if(first)
		{
			index=predek->index;
			ReIndex(predek,1,true);
		}
		else
		{
			index=predek->index+1;
			ReIndex(predek,1);
		}
	}else{
		if(first)
		{
			predek=0;
			index=0;
			ReIndex(0,1);
		}else{
			predek = items->GetFromBack();
			if(predek)
			{
				index=predek->index+1;
			}
		}
	}

	//ti = new CGUITreeItem(this,0,0,labelString,_picture,_help);
	ti = new CGUITreeItem(this,labelString,true,_picture,_help,_rootItemBackground);
	ti->index=index;
	ti->tree=this;
	ti->compareFunction=compareFunction;
	ti->manualUpdate=_manualUpdate;
	if(_manualUpdate)
		ti->AcceptEvent(GetID(),ETreeItemUpdate);
	ti->AcceptEvent(GetID(),EClicked);
	ti->AcceptEvent(GetID(),EMouseButton);
	ti->AcceptEvent(GetID(),EEvent);

	if(predek)
	{
		if(first)
			items->AddToFrontObj(predek,ti);
		else
			items->AddToBehindObj(predek,ti);
	}
	else
	{
		items->AddToHead(ti);
	}

	//AddBackElem(ti);
	AddTreeItemEngine(ti);

	//UpdateTree();

	return ti;
}

CGUITreeItem* CGUITree::AddTreeItem(typeID where, CGUITreeItem* treeItem, bool _manualUpdate, bool _rootItemBackground, bool first)
{
	// !@#$ dodelat 

	return 0;
}

int CGUITree::DeleteTreeItem(CGUITreeItem* ti)
{
	int ret = 0;

	if(!ti)
		return 1;

	if(!items->IsMember(ti))
		return 1;

	// rekurzivni smazani vsech podpolozek mazane polozky
	while(CGUITreeItem* tti = ti->items->GetFromHead())
	{
		ti->DeleteTreeItem(tti);
	}

	if(ti==lastTreeItem)
		lastTreeItem=0;

	ReIndex(ti,-1);

	if(!items->Remove(ti))
		ret=1;

	if(ti->elem)
		DeleteBackElem(ti->elem);
	DeleteBackElem(ti);

	return ret;
}

void CGUITree::ReIndex(CGUITreeItem *start, int d_index, bool inclusive)
{
	// (inclusive==false) start jiz neposouvam, (inclusive==true) posouvam i start
	// pokud je start == 0 posouvam vsechny polozky od zacatku seznamu
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	bool found;

	if(!start)
		found=true;
	else
		found=false;

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(found)
			{
				e->index+=d_index;
			}
			if(e==start)
			{
				found=true;
				if(inclusive)
					e->index+=d_index;
			}
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::UpdateRootItemsBackgrounds()
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	float bsx,bsy;
	GetBackWindowSize(bsx,bsy);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			e->UpdateRootBackground(bsx+20);
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::BringTopRootItemsBackgrounds()
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->rootBackground)
			{
				if(e->itemBackground)
					BringElementToTop(e->itemBackground);
				BringElementToTop(e);
			}

		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::DistributeUpdate(float &px, float &py, float &maxx)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			e->Update(px,py,maxx,true);
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::DistributeUpdateInlineEl(float &maxx)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			e->UpdateInlineElems(maxx);
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::UpdateTree()
{
	float px,py;
	float min_x, min_y, max_x, max_y;

	px=treeItemStartX;
	py=treeItemStartY;
	min_x=px;min_y=py;
	max_x=min_x;max_y=min_y;

	DistributeUpdate(px,py,max_x);	// aktualizuje stav vsech polozek stromu
	max_y=py;

	if(automaticSliderPos)
		SetSliderPos(max_x+STD_TREE_BEFORE_SLIDER_GAP);	// nastavi pozici rozdelovace - slider

	DistributeUpdateInlineEl(max_x);	// aktualizuje pozice radkovych elementu polozek, podle nove pozice slideru

	SetBackWindowNormalSize(max_x+treeEndGapX,max_y+treeEndGapY);	// nastavi velikost vnitrku okna, podle toho kolik zabiraji viditelne polozky a elementy

	if(treeSlider)	// pokud je manualni slider, aktualizuj jeho casti (podklad radkovych elementu atp.)
	{
		float tsx,tsy;
		float wsx,wsy;
		GetBackWindowSize(wsx,wsy);
		treeSlider->GetSize(tsx,tsy);
		treeSlider->Resize(tsx,wsy);
		if(sliderBackground)
			sliderBackground->Resize(wsx-sliderPosition,wsy);
	}
	UpdateRootItemsBackgrounds();
}

void CGUITree::AddSlider(UINT _color, UINT _moveColor, UINT background)
{
	if(!treeSlider)
	{
		automaticSliderPos=false;

		float wsx,wsy;
		GetBackWindowSize(wsx,wsy);
		sliderBackground = new CGUIStaticPicture(sliderPosition,0,wsx-sliderPosition,wsy,background);
		sliderBackground->SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FIX,WIDGET_FIX);
		sliderBackground->focusable=false;
		sliderBackground->throw_going=true;
		AddBackElem(sliderBackground);

		treeSlider = new CGUITreeSlider(this,2);
		AddBackElem(treeSlider);


		BringAllElementTop();
		BringTopRootItemsBackgrounds();
		//BringElementToTop(treeSlider);

		/*
		if(queue_back.Remove(treeSlider))
		{
			queue_back.AddToFrontObj(sliderBackground,treeSlider);
		}
		//*/
	}
}

void CGUITree::SetSliderPos(float pos)
{
	sliderPosition=pos;
	if(treeSlider)
		treeSlider->Move(pos,0);
	if(sliderBackground)
		sliderBackground->Move(pos,0);
}


CGUITreeItem* CGUITree::FindItemFromElement(CGUIElement* elem)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *retItem=0;

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->elem == elem)
				return e;
			retItem = e->FindItemFromElement(elem);
			if(retItem)
				return retItem;
		}
		le=items->GetNextListElement(le);
	}
	return 0;
}

CGUITreeItem* CGUITree::GetItemWithIndex(int index)
{
	if(!items)
		return 0;
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->index == index)
				return e;
		}
		le=items->GetNextListElement(le);
	}
	return 0;
}

int CGUITree::RollTreeToItem(CGUITreeItem* ti)
{
	if(ti->IsVisible())
		return 0;

	while(ti->parentItem)
	{
		if(ti->parentItem->IsVisible())
		{
			ti->parentItem->SetRollState(1);
			return 0;
		}
		else
		{
			ti->parentItem->SetRollState(1);
			ti = ti->parentItem;
		}
	}
	return 1;
}



void CGUITree::BringAllElementTop()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetPrevListElement(0);
	CGUITreeItem* ti;

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(ti = dynamic_cast<CGUITreeItem*>(e))
			{
				if(ti->tree != this || ti->rootBackground)
				{
					e->BringToTop();
				}
			}else if(e!=treeSlider && e!=sliderBackground)
			{
				e->BringToTop();
			}
		}
		le=queue_back.GetPrevListElement(le);
	}
}

void CGUITree::AddTreeItemEngine(CGUITreeItem* ti)
{
	if(ti && ti->rootBackground)
		AddBackElem(ti->itemBackground);

	if(ti && back){
		if(autogrowing)
		{
			float ex,ey,esx,esy;
			float cx=0,cy=0;
			bool change=false;

			ti->GetPos(ex,ey);
			ti->GetSize(esx,esy);

			if(ex+esx >= bw_size_normal_x)
			{
				cx = ex+esx;
				change=true;
			}else
				cx = bw_size_normal_x;

			if(ey+esy >= bw_size_normal_y)
			{
				cy = ey+esy;
				change = true;
			}else
				cy = bw_size_normal_y;

			if(change)
			{
				SetBackWindowNormalSize(cx,cy);
			}
		}

		if(ti->GetList() && !ti->GetParent())
		{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
			trash->Remove(ti);
		}

		if(sliderBackground && !(ti->rootBackground))
			queue_back.AddToBehindObj(sliderBackground,ti);
		else
			queue_back.AddToHead(ti);
		

		ti->SetParent(this);
		CBltElem* pos=0;
		if(sliderBackground)
		{
			pos=sliderBackground->GetElemPosition();
			pos=pos->GetBackElem();
		}

		if(pos)
			ti->AddToEngineBackPosition(back,pos);
		else
			ti->AddToEngine(back);
		ti->back_element=true;

		// !@#$ kdyz davam novy element do okna roztazeneho nad svoji normalni veliost
		// musim element take roztahnout nad svoji velikost (pokud jsou patricne nastaveny kotvy)
		if(ti->GetType() >= PTWidget)
					((CGUIWidget*)ti)->WindowSizeChanged(bw_size_x - bw_size_normal_x,bw_size_y - bw_size_normal_y);


		if((ti->GetTabOrder()>0 && tab_min_back>ti->GetTabOrder()) || tab_min_back==0)
			tab_min_back=ti->GetTabOrder();
		if(tab_max_back<ti->GetTabOrder())
			tab_max_back=ti->GetTabOrder();

		typeID id;
		CGUIEventProgram* ep = dynamic_cast<CGUIEventProgram*>(ti);
		if(ep)
			id=ep->GetID();
		else
			id=0;

		minimized_all=false;

		//if(!focus_elm)
			//SetFocusEl(ti,false);

		EventArise(EAddElement,0,id,ti);
	}
}

int CGUITree::Keyboard(UINT character, UINT state)
{
	int ret = 1;
	CGUIElement* el;
	el = GetFocusEl();
	if(el && el->exclusive_keyboard)
		EventArise(EKeyboard,character,state,0,1,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	else
		EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	if(character == VK_TAB && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(el && !el->exclusive_keyboard)
		{
			if(lastTreeItem==el && lastTreeItem->elem)
			{
				SetFocusEl(lastTreeItem->elem,0);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}else if(lastTreeItem && lastTreeItem->elem==el)
			{
				SetFocusEl(lastTreeItem,0);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}else if(!el){
			el = items->GetFromHead();
			if(el)
			{
				SetFocusEl(el,0);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}
	}

	if(keyClose && character == VK_F5 && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret=0;
		CloseWindow();
	}

	if(keyMaximalize && character == VK_F6 && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret=0;
		if(windowState==WS_Maximized)
			UnMaximize();
		else
			Maximize();
		mainGUI->SendCursorPos();
	}

	if(keyMaximalize && character == VK_F6 && mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret=0;
		Normalize();
		mainGUI->SendCursorPos();
	}

	return ret;
}

int CGUITree::KeyboardChar(UINT character, UINT state)
{
	CGUIElement* el;
	el = GetFocusEl();
	if(el && el->exclusive_keyboard)
		EventArise(EKeyboardChar,character,state,0,1,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	else
		EventArise(EKeyboardChar,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	if(character == VK_TAB && !mainGUI->key_ctrl && !mainGUI->key_alt)
	{
		if(el && !el->exclusive_keyboard)
			return 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
	}

	return 1;
}

int CGUITree::KeyboardUp(UINT character, UINT state)
{
	CGUIElement* el;
	el = GetFocusEl();
	if(el && el->exclusive_keyboard)
		EventArise(EKeyboardUp,character,state,0,1,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	else
		EventArise(EKeyboardUp,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	return 1;
}


int CGUITree::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{
	int ret = CGUIStdWindow::MouseLeft(x,y,mouseState,keyState);

	if(ret)
	{
		ret=TreeMouseClick(x,y,0,mouseState);
	}

	return ret;
}

int CGUITree::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	return CGUIStdWindow::MouseLeftFocus(x,y,mouseState,keyState,ID);
}

int CGUITree::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{
	int ret = CGUIStdWindow::MouseRight(x,y,mouseState,keyState);

	if(ret)
	{
		ret=TreeMouseClick(x,y,1,mouseState);
	}

	return ret;
}

int CGUITree::MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	return CGUIStdWindow::MouseRightFocus(x,y,mouseState,keyState,ID);
}

int CGUITree::TreeMouseClick(float px, float py, int mouse, int mouseState)
{
	CGUITreeItem *ti=0;

	InnerCoords(px,py);			// prepocitani souradnic do souradne soustavy v okne (predtim je v sour. s. nadrazeneho okna)
	if(TransformCoordsFrontToBack(px,py))
	{

		ti = FindTreeItem(px,py);		// najde polozku stromu, ktera odpovida mistu kliknuti mysi
										// je to polozka na kterou se kliknulo, nebo na jejiz pridruzeny element se kliknulo,
										// nebo se kliknulo do horizontalniho pasma ve vysce polozky a jejiho elementu
		if(!ti || !ti->IsActive())	
			return 1;	// zadna aktivni polozka nenalezena, nic neoznacuj

		if(treeSlider && treeSlider->IsIn(px,py))	// kliknul jsem na slider (posuvnik), nic neoznacuj
			return 1;

		if((!ti->elem || !ti->elem->IsIn(px,py)) && !ti->IsIn(px,py) && mouseState)
		{	// kliknul jsem mimo polozku i jeji pripadny element
			// zaridim praci, jako bych kliknul na polozku:
			if(ti->markItem)
			{
				if(!mouse)	// jde o leve tlacitko
				{
					if(ti->state!=0 && rollOnClick)	// nejde o list
						ti->SetRollState((ti->state==1) ? 1 : 0);	// rozbal/zabal podstrom

					ti->OnClick();
					ti->EventArise(EClicked);
				}
			}else{
				ti->Mark();
				SetFocusEl(ti,0);
			}
			ti->EventArise(EMouseButton,mouse,mouseState);
		}
		else if(!ti->IsIn(px,py) && !ti->markItem)
		{	// kliknul jsem mimo polozku, ale na jeji element, nebo pravym tlacitkem atp.
			// jen oznacim polozku, neprovedu kliknuti na ni:
			ti->Mark();
			SetFocusEl(ti,0);
		}
	}
	return 1; 
}

int CGUITree::FocusChanged(UINT focus, typeID dualID)
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
	}
	else if(!focus_elm)
	{
		// neni zadny el. focusovany a okno dostalo focus => focusuj prvni el. vevnitr okna
		CGUITreeItem* ti = items->GetFromHead();
		if(ti)
		{
			SetFocusEl(ti,0);
			ti->Mark();
		}
	}

	EventArise(EFocusChanged,focus);
	return 1;
}

CGUITreeItem* CGUITree::FindTreeItem(float px, float py)
// najde polozku stromu, ktera odpovida mistu kliknuti mysi
// je to polozka na kterou se kliknulo, nebo na jejiz pridruzeny element se kliknulo,
// nebo se kliknulo do horizontalniho pasma ve vysce polozky a jejiho elementu
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(e))
			{
				float tx,ty,tsx,tsy;
				if(ti->IsVisible() && ti->IsActive())
				{
					ti->GetPos(tx,ty);
					ti->GetSize(tsx,tsy);
					if(py>=ty && py<ty+tsy)	// polozka musi byt viditelna, aktivni a klik odpovida jeji vertikalni poloze (horizontalne muze byt vedle)
						return ti;
				}
				if(ti->elem && ti->elem->IsVisible())
				{
					ti->elem->GetPos(tx,ty);
					ti->elem->GetSize(tsx,tsy);
					if(py>=ty && py<ty+tsy)	// pridruzeny element polozky musi byt viditelny a klik odpovida jeho vertikalni poloze (horizontalne muze byt vedle)
						return ti;
				}
			}
		}
		le=queue_back.GetNextListElement(le);
	}

	return 0;
}

CGUITreeItem* CGUITree::Switch(CGUITreeItem* si)
{
	if(!si)
		return 0;
	CGUITreeItem* oi = dynamic_cast<CGUITreeItem*>(GetFocusEl());
	if(oi)
		oi->UnMark();
	SetFocusEl(si,0);

	float xx, yy;
	float isx, isy;

	si->GetPos(xx,yy);
	si->GetSize(isx,isy);

	float wx, wy;
	float wsx, wsy;

	GetVPPos(wx,wy);
	GetVPSize(wsx,wsy);

	if(yy<wy)
		SetWindowPositionVP(0,yy-wy-10);
	else if(yy+isy>wy+wsy)
		SetWindowPositionVP(0,yy+isy-wy-wsy+10);

	si->Mark();
	return si;
}

CGUITreeItem* CGUITree::SwitchItemIndex(int ind, bool next)
{
	CGUITreeItem* ti;

	if(next)
	{
		ti = FindIndex(ind+1);
		if(ti)
		{
			return Switch(ti);
		}
	}else{
		ti = FindIndex(ind-1);
		if(ti)
		{
			if(ti->state==0 || ti->state==1)
				return Switch(ti);
			else{
				return Switch(ti->FindLastSubItem());
			}
		}
	}
	return 0;
}

CGUITreeItem* CGUITree::FindIndex(int ind)
{
	CGUITreeItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=items->GetObjectFromListElement(le)))
		{
			if(e->index==ind)
				return e;
		}
		le=items->GetNextListElement(le);
	}

	return 0;
}

void CGUITree::EventHandler(CGUIEvent *event)
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

	if(event->eventID == EClicked)
	{
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->sender));
		if(ti)
		{
			if(ti->tree==this)
			{
				if(TreeHandler(event->sender,ti->index,0))
					EventArise(ETree,ti->index,event->sender,0,0,0);
			}
		}
	}

	if(event->eventID == EMouseButton)
	{
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->sender));
		if(ti)
		{
			if(ti->tree==this)
			{
				if(TreeHandler(event->sender,ti->index,event->pInt+1))
					EventArise(ETree,ti->index,event->sender,0,0,event->pInt+1);
			}
		}
	}

	if(event->eventID == EEvent)
	{
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->sender));
		if(ti)
		{
			if(ti->tree==this)
			{
				if(TreeHandler(event->sender,ti->index,3))
					EventArise(ETree,ti->index,event->sender,0,0,3);
			}
		}
	}


	if(event->eventID == ETreeItemUpdate)
	{
		CGUITreeItem* ti = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(event->sender));
		if(ti)
		{
			if(ti->tree==this)
			{
				TreeUpdateHandler(event->sender,ti->index);
				EventArise(ETreeUpdate,ti->index,event->sender);
			}
		}
	}

	delete event;
}

void CGUITree::Resize(float _sx, float _sy)
{
	CGUIStdWindow::Resize(_sx,_sy);
	UpdateRootItemsBackgrounds();
}

CGUITreeItem* CGUITree::GetSelectedItem()
{
	return lastTreeItem;
	//return dynamic_cast<CGUITreeItem*>(GetFocusEl());
}

int CGUITree::Compare(CGUITreeItem *a, CGUITreeItem *b)
{
	if(a && b)
		return _stricoll(a->label->GetText(),b->label->GetText());
	else
		return 0;
}

void CGUITree::Sort()
{
	CGUIListElement<CGUITreeItem>* max=0;
	while(SortItems(&max));

	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	int i=0;
	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			ti->index=i++;
		}
		le=items->GetNextListElement(le);
	}
}

int CGUITree::SortItems(CGUIListElement<CGUITreeItem>** max)
{
	CGUITreeItem *ea, *eb;
	CGUIListElement<CGUITreeItem>* a = items->GetNextListElement(0);
	if(a==*max)
		return 0;
	ea=items->GetObjectFromListElement(a);
	CGUIListElement<CGUITreeItem>* b = items->GetNextListElement(a);

	while(b && b!=*max)
	{
		if((eb=items->GetObjectFromListElement(b)))
		{
			if((*compareFunction)(ea,eb)>0)
				ea=eb;
		}
		b=items->GetNextListElement(b);
	}

	if(ea)
	{
		items->Remove(ea);
		items->AddToBack(ea);
		if(*max==0)
			*max=items->GetPrevListElement(0);
		return 1;
	}else
		return 0;
}

void CGUITree::SortFlooding()
{
	Sort();

	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			if(ti->state>0)
				ti->SortFlooding();
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::SetCompareFunctionSubTree(int (*compareFunction)(CGUITreeItem *, CGUITreeItem *))
{
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);
	CGUITreeItem *ti;

	while(le)
	{
		if((ti=items->GetObjectFromListElement(le)))
		{
			ti->compareFunction=compareFunction;
			if(ti->state)
				ti->SetCompareFunctionSubTree(compareFunction);
		}
		le=items->GetNextListElement(le);
	}
}

void CGUITree::FocusOldTOBackEl()
{
	if(!front_focus)
		return;
	if(lastTreeItem)
	{
		SetFocusEl(lastTreeItem,0);
		lastTreeItem->Mark();
	}else
		CGUIStdWindow::FocusOldTOBackEl();
}

//////////////////////////////////////////////////////////////////////
// CGUITreeMarker
//////////////////////////////////////////////////////////////////////

CGUITreeMarker::CGUITreeMarker(float x1, float y1, float x2, float y2, CBltWindow *rootwnd, CBltElem* pos)
{
	UINT linesColor = STD_TREE_MARK_BORDER_COLOR;
	lines[0] = new CBltLine(x1,y1,x2,y1,linesColor);
	lines[1] = new CBltLine(x2,y1,x2,y2,linesColor);
	lines[2] = new CBltLine(x2,y2,x1,y2,linesColor);
	lines[3] = new CBltLine(x1,y2,x1,y1,linesColor);

	rect = new CBltRect(x1,y1,x2-x1,y2-y1,STD_TREE_MARK_BGCOLOR_COLOR);
	rootwnd->AddChildToBackOfEl(rect,pos);

	for(int i=0;i<4;i++)
		rootwnd->AddChildToBackOfEl(lines[i],pos);
	visible=true;
}

CGUITreeMarker::~CGUITreeMarker()
{
	CBltWindow *wnd=0;
	if(lines[0])
		wnd = lines[0]->GetParent();
	if(wnd)
	{
		if(rect)
			wnd->DeleteChild(rect);
		for(int i=0;i<4;i++)
		{
			if(lines[i])
				wnd->DeleteChild(lines[i]);
		}
	}else{
		SAFE_DELETE(rect);
		for(int i=0;i<4;i++)
			SAFE_DELETE(lines[i]);
	}
}

void CGUITreeMarker::Mark()
{
	if(rect)
		rect->SetVisible(1);
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(1);
	visible=true;
}

void CGUITreeMarker::UnMark()
{
	if(rect)
		rect->SetVisible(0);
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(0);
	visible=false;
}

void CGUITreeMarker::Rebuild(CGUIElement* elm)
{
	float x1,x2,y1,y2;

	elm->GetPos(x1,y1);
	elm->GetSize(x2,y2);

	x2 +=x1+2;
	y2 +=y1;
	x1--;
	y1--;

	lines[0]->SetPoints(x1,y1,x2,y1);
	lines[1]->SetPoints(x2,y1,x2,y2);
	lines[2]->SetPoints(x2,y2,x1,y2);
	lines[3]->SetPoints(x1,y2,x1,y1);

	rect->SetPos(x1,y1,x2-x1,y2-y1);
	//rect->Move(x1-xx,y1-yy);
	//rect->Resize(x2-x1,y2-y1);
}


//////////////////////////////////////////////////////////////////////
// CGUITreeSlider
//////////////////////////////////////////////////////////////////////

CGUITreeSlider::CGUITreeSlider(CGUITree* _tree, float width, UINT _color, UINT _moveColor)
	: CGUIWidget(0,0)
{
	timerReq=false;
	cursor_type=0;
	float wx,wy;
	rect = 0;
	tree=_tree;
	color=_color;
	moveColor=_moveColor;

	tree->GetBackWindowSize(wx,wy);
	
	x=tree->sliderPosition; y=0; sx=width; sy=wy; 
	rect = new CGUIRectHost(x,y,sx,sy,color);
	rect->RemoveFromTrash();
	SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FIX,WIDGET_FREE);

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgTimer|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);
	mouse_button_down=false;
	cursor_resizer=false;
	px=py=0;
	AddToTrash();
}

CGUITreeSlider::~CGUITreeSlider()
{
	RemoveFromTrash();

	timerGUI->DeleteAllRequests(this);
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	SAFE_DELETE(rect);
}

void CGUITreeSlider::SetVisible(int vis)
{
	SetVisibility(vis);
	if(rect)
		rect->SetVisible(vis);
}

void CGUITreeSlider::Resize(float _sx, float _sy)
{
	CheckResize(_sx,_sy);
	if(rect)
		rect->Resize(_sx,_sy);
	SetSize(_sx,_sy);
}

void CGUITreeSlider::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	SetPos(_x,_y);
	if(rect)
		rect->Move(_x,_y);
}

void CGUITreeSlider::AddToEngine(CBltWindow *rootwnd)
{
	if(rect)
		rect->AddToEngine(rootwnd);
}

void CGUITreeSlider::RemoveFromEngine()
{
	if(rect)
		rect->RemoveFromEngine();
}

void CGUITreeSlider::BringToTop()
{
	if(rect)
		rect->BringToTop();
}

void CGUITreeSlider::FocusOldTreeItem()
{
	if(tree && tree->lastTreeItem)
	{
		tree->SetFocusEl(tree->lastTreeItem,0);
		tree->lastTreeItem->Mark();
	}
}

int CGUITreeSlider::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	if(mouseState)
	{			// po stisknuti tlacitka zacinam menit velikost okna (zaznamenavam zmenu souradnic kurzoru)
		mouse_button_down=true;
		px=x;
		py=y;
		rect->SetColor(moveColor);
	}else if(mouse_button_down){		// po pusteni tlacitka dokoncim zmenu velikosti okna
		ChangeTreeSliderPos(x);

		mouse_button_down=false;
		px=py=0;
		rect->SetColor(color);
		
		FocusOldTreeItem();
	}
	return 0;
}

int CGUITreeSlider::MouseOver(float x, float y, UINT over,UINT state)
{
	// zmenit kurzor na sipky (roztahovak)
	if(!cursor_resizer && !(state & MK_LBUTTON))
	{
		cursor_type = mainGUI->cursor->GetCursorIndex();
		cursor_resizer=true;
		mainGUI->cursor->Set(4);
	}

	if(!(state & MK_LBUTTON))
	{
		mouse_button_down=false;
		rect->SetColor(color);
	}
	
	// kontrola na vyjezd s oknem mimo oblast materskeho okna
	if(tree && mouse_button_down)
	{
		CGUITree* par = tree;

		float wx, wy;
		float wsx, wsy;
		float cx, cy;
		GetPos(wx,wy);
		GetSize(wsx,wsy);

		cx = wx+x-px;
		cy = wy+y-py;

		float test;

		if(cx<par->vp_x)
		{
			if(par->vp_x>0)
			{
				par->SetWindowPositionVP(cx-par->vp_x,0,true);
				if(!timerReq)
				{
					timerReq=true;
					timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
				}
			}
			x = par->vp_x-wx+px;
		}else if(cx+wsx>par->vp_sx+par->vp_x)
		{
			if(par->vp_x+par->vp_sx < par->bw_size_x)
			{
				test=cx+px-par->vp_x-par->vp_sx-wx;
				par->SetWindowPositionVP(test,0,true);
				if(!timerReq)
				{
					timerReq=true;
					timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
				}
			}
			x = par->vp_x + par->vp_sx - wx - wsx + px;
		}
	}

	
	if(mouse_button_down)
	{
		ChangeTreeSliderPos(x);
	}

	return 0;
}

int CGUITreeSlider::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	px=py=0;

	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	if(mouse_button_down)
	{
		rect->SetColor(color);
		FocusOldTreeItem();
	}

	mouse_button_down=false;

	return 0;
}

int CGUITreeSlider::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	// kontrola na vyjezd s oknem mimo oblast materskeho okna
	if(tree && mouse_button_down && (state & MK_LBUTTON))
	{
		CGUITree* par = tree;

		float wx, wy;
		float wsx, wsy;
		float cx, cy;
		GetPos(wx,wy);
		GetSize(wsx,wsy);
		cx = wx+x-px;
		cy = wy+y-py;

		if(cx<par->vp_x)
		{
			if(par->vp_x>0)
			{
				par->SetWindowPositionVP(cx-par->vp_x,0,true);
				if(!timerReq)
				{
					timerReq=true;
					timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
				}
			}
			x = par->vp_x-wx+px;
		}else if(cx+wsx>par->vp_sx+par->vp_x)
		{
			if(par->vp_x+par->vp_sx < par->bw_size_x)
			{
				par->SetWindowPositionVP(cx+px-par->vp_x-par->vp_sx-wx,0,true);
				if(!timerReq)
				{
					timerReq=true;
					timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
				}
			}
			x = par->vp_x + par->vp_sx - wx - wsx + px;
		}
	}


	// zmenit kurzor na normal
	if(mouse_button_down && (state & MK_LBUTTON))
	{		// tlacitko je zmacknute a byla zapocata zmena velikosti okna => pokracuj  
		ChangeTreeSliderPos(x);
	}else{
		mouse_button_down=false;
		px=py=0;
		rect->SetColor(color);
		if(cursor_resizer)
		{
			cursor_resizer=false;
			mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
		}
	}

	return 0;
}

int CGUITreeSlider::TimerImpulse(typeID timerID, float time)
{
	timerReq=false;
	mainGUI->SendCursorPos();
	return 0;
}

int CGUITreeSlider::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
		return 0;

	mouse_button_down=false;
	rect->SetColor(color);
	px=py=0;
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}

	return 0;
}

int CGUITreeSlider::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	px=py=0;
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	if(mouse_button_down && tree)
	{
		rect->SetColor(color);
		FocusOldTreeItem();
	}

	mouse_button_down=false;

	return 0;
}

void CGUITreeSlider::ChangeTreeSliderPos(float nx)
{
	CGUIWindow *wnd = tree;
	if(!tree)
		return;

	float dx;
	dx=nx-px;
	px=nx;

	float bsx,bsy,nsx,nsy;

	MoveRel(dx,0);
	tree->SetSliderPos(x);
	tree->GetBackWindowSize(bsx,bsy);
	tree->UpdateTree();
	tree->GetBackWindowSize(nsx,nsy);
	if(nsx<bsx)
		if(!timerReq)
		{
			timerReq=true;
			timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
		}


	/*
		case Left		:	
							dx=-dx;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(-dx,0);
							px+=dx-old_dx;
							px-=dx;
							break;
		case Right		:	wnd->ResizeRelWithCheck(dx,dy);	px+=dx-old_dx;	break;
		*/
}


//////////////////////////////////////////////////////////////////////
// CGUITreeRootItemBackground
//////////////////////////////////////////////////////////////////////

CGUITreeRootItemBackground::CGUITreeRootItemBackground(float _x, float _y, float _sx, float _sy, UINT color, UINT linesColor)
	: CGUIWidget(_x,_y,_sx,_sy)
{
	rect = new CGUIRectHost(x,y,sx,sy,color);
	up = new CGUILine(x,y,x+sx,y,linesColor);
	down = new CGUILine(x,y+sy-1,x+sx,y+sy-1,linesColor);

	throw_going=false;
	focusable=false;
	full_window_anchor=true;
	MsgSetConsume(MsgNone);
	MsgSetProduce(MsgNone);	

	AddToTrash();
}
CGUITreeRootItemBackground::~CGUITreeRootItemBackground()
{
	RemoveFromTrash();

	SAFE_DELETE(rect);
	SAFE_DELETE(up);
	SAFE_DELETE(down);
}


void CGUITreeRootItemBackground::AddToEngine(CBltWindow *rootwnd)
{
	rect->AddToEngine(rootwnd);
	up->AddToEngine(rootwnd);
	down->AddToEngine(rootwnd);
}

void CGUITreeRootItemBackground::RemoveFromEngine()
{
	rect->RemoveFromEngine();
	up->RemoveFromEngine();
	down->RemoveFromEngine();
}

void CGUITreeRootItemBackground::BringToTop()
{
	rect->BringToTop();
	up->BringToTop();
	down->BringToTop();
}


void CGUITreeRootItemBackground::Resize(float _sx, float _sy)
{
	SetSize(_sx,_sy);
	rect->Resize(sx,sy);
	up->Resize(sx,sy);
	down->Resize(sx,sy);
}

void CGUITreeRootItemBackground::Move(float _x, float _y)
{
	rect->Move(_x,_y);
	up->Move(_x,_y);
	down->Move(_x,_y+sy-1);

	SetPos(_x,_y);
}

void CGUITreeRootItemBackground::SetVisible(int vis)
{
	rect->SetVisible(vis);
	up->SetVisible(vis);
	down->SetVisible(vis);
	SetVisibility(vis);
}

void CGUITreeRootItemBackground::SetColor(UINT backgroundColor, UINT linesColor)
{
	rect->SetColor(backgroundColor);
	up->SetColor(linesColor);
	down->SetColor(linesColor);
}


//////////////////////////////////////////////////////////////////////
// CGUIFileBrowser
//////////////////////////////////////////////////////////////////////

CGUIFileBrowser::CGUIFileBrowser(float _x, float _y, float _sx, float _sy) // , char* path, char *label, int roll_state)
: CGUITree(_x,_y,_sx,_sy,"FileBrowser",new CGUIRectHost(0,0,styleSet->Get("FileBrowser")->GetTexture(0)))
{
	dirMenuID=fileMenuID=0;
	msgBoxID=0;
	msgBoxIndex=-1;
	selecetedFileItemID=0;
	newDirName=0;

	compareFunction=&Compare;

	/*
	CGUITreeItem* ti = AddRootItem(path,label);

	UpdateTree();

	if(roll_state)
		ti->SetRollState(1);
		*/
}

CGUIFileBrowser::~CGUIFileBrowser()
{
	SAFE_DELETE_ARRAY(newDirName);
}

CGUITreeItem* CGUIFileBrowser::AddRootItem(char* path, char* label, int roll_state)
{
	int dir=FS->ChangeDir(path);

	char* rootPath;

	if(!FS->GetFullPath(path,&rootPath))
		return 0;
	SAFE_DELETE_ARRAY(rootPath);

	if(!label)
		label=path;

	CGUIFileBrowserItem* ti = AddItem(0,label,dir);
	
	if(ti)
	{
		SAFE_DELETE_ARRAY(ti->name);
		ti->name = newstrdup(path);
	}

	if(roll_state)
		ti->SetRollState(1);

	/*
	CGUIFileBrowserItem* ti=0;
	if(dir)
	{
		ti = (CGUIFileBrowserItem*)AddTreeItem(0,label,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),0,true);
		ti->state=1;
		ti->ChangeRoller();
	}else{
		ti = (CGUIFileBrowserItem*)AddTreeItem(0,label,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),0,true);
	}

	ti->name = newstrdup(path);
	ti->dir = dir;
*/

	return ti;
}

int CGUIFileBrowser::TreeHandler(typeID treeItem, int index, int state)
{
	// zaridim otevreni menu pro soubory / adresare, pokud jsou vytvoreny

	if(state!=2)	
		return 1;	// nejedna se o prave tlacitko mysi

	CGUIMenu* menu=0;
	CGUIFileBrowserItem* fbi = GetObjFromID<CGUIFileBrowserItem>(treeItem);
	if(fbi)
	{
		if(fbi->dir && dirMenuID)
		{
			menu = GetObjFromID<CGUIMenu>(dirMenuID);
			if(menu)
			{
				float dx=0,dy=0;
				float fsx,fsy;
				desktop->GetDesktopPos(fbi,dx,dy);
				fbi->GetSize(fsx,fsy);
				selecetedFileItemID=fbi->GetID();
				menu->On(fbi->GetID(),dx+10,dy+fsy);
			}
		}
		else if(!fbi->dir && fileMenuID)
		{
			menu = GetObjFromID<CGUIMenu>(fileMenuID);
			if(menu)
			{
				float dx=0,dy=0;
				float fsx,fsy;
				desktop->GetDesktopPos(fbi,dx,dy);
				fbi->GetSize(fsx,fsy);
				selecetedFileItemID=fbi->GetID();
				menu->On(fbi->GetID(),dx+10,dy+fsy);
			}
		}
	}

	return 1;
}

void CGUIFileBrowser::TreeUpdateHandler(typeID treeItem, int index)
{
	CGUIFileBrowserItem* ti = dynamic_cast<CGUIFileBrowserItem*>(nameServer->ResolveID(treeItem));
	if(ti && !ti->prepared)
	{
		if(ti->state==0 && ti->dir==0)
			return;
		if(!SetDir(ti))
			return;

		if(!ti->prepared)
		{
			const char *filename;
			int dir;

			if(FS->FindFirst(&filename,dir) && strcmp(filename,"..") && Filter(ti,filename,dir,1))
			{
				AddItem(ti,filename,dir);
			}

			while(FS->FindNext(&filename,dir) && Filter(ti,filename,dir,0))
			{
				AddItem(ti,filename,dir);
			}
			ti->Sort();
		}
		ti->prepared=true;


	}

	UpdateTree();
}

CGUIFileBrowserItem* CGUIFileBrowser::AddItem(CGUIFileBrowserItem* fbi, const char* name, int dir)
{
	CGUIFileBrowserItem* ni;
	char* ff = new char[strlen(name)+1];
	strcpy(ff,name);
	if(dir)
	{
		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,ff,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),0,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,ff,new CGUIRectHost(0,0,styleSet->Get("FBdir")->GetTexture(0)),0,true);
		ni->state=1;
		ni->ChangeRoller();
	}else{
		if(fbi)
			ni = (CGUIFileBrowserItem*)fbi->AddTreeItem(0,ff,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),0,true);
		else
			ni = (CGUIFileBrowserItem*)AddTreeItem(0,ff,new CGUIRectHost(0,0,styleSet->Get("FBscriptfile")->GetTexture(0)),0,true);
	}
	ni->dir=dir;
	ni->name = ff;

	return ni;
}

int CGUIFileBrowser::SetDir(CGUIFileBrowserItem* ti)
{
	if(!ti)
		return 0;

	if(ti->parentItem)
		SetDir((CGUIFileBrowserItem*)(ti->parentItem));

	return FS->ChangeDir(ti->name);
}

int CGUIFileBrowser::GetSelectedFile(typeID treeItem, char **dir, char **file)
{
	CGUIFileBrowserItem* ti = dynamic_cast<CGUIFileBrowserItem*>(nameServer->ResolveID(treeItem));
	if(!ti)
		return -1;
	if(ti->tree!=this)
		return -1;
	
	SetDir(ti);

	FS->GetCurDir(dir);
	*file = new char[strlen(ti->name)+1];
	strcpy(*file,ti->name);

	return ti->dir;
}

int CGUIFileBrowser::GetSelectedFileRelativ(typeID treeItem, char **root, char **dir, char **file)
{
	CGUIFileBrowserItem *fbi = GetObjFromID<CGUIFileBrowserItem>(treeItem);
	CGUIFileBrowserItem *rootItem = 0;
	if(!fbi)
		return -1;

	if(fbi->parentItem)
		*dir = GetDirString((CGUIFileBrowserItem*)fbi->parentItem,0,&rootItem);
	else
	{
		*dir = new char[1];
		*dir[0]=0;
	}

	if(rootItem)
	{
		*root = new char[strlen(rootItem->name)+1];
		strcpy(*root,rootItem->name);
	}
	else
	{
		*root = new char[1];
		*root[0]=0;
	}

	*file = new char[strlen(fbi->name)+1];
	strcpy(*file,fbi->name);
	return fbi->dir;
}

char* CGUIFileBrowser::GetDirString(CGUIFileBrowserItem* fbi, int len, CGUIFileBrowserItem** root)
{
	char *s=0;
	if(fbi->parentItem)
	{
		if(fbi->name)
			len += strlen(fbi->name) +1;

		s = GetDirString((CGUIFileBrowserItem*)fbi->parentItem,len,root);
		if(s[0]!=0)
			strcat(s,"\\");
		strcat(s,fbi->name);
	}
	else if(fbi->rootTree)
	{
		s = new char[len+5];
		s[0]=0;
		*root=fbi;
	}else
		assert(false);

	return s;
}

int CGUIFileBrowser::Compare(CGUITreeItem *aa, CGUITreeItem *bb)
{
	CGUIFileBrowserItem* a = (CGUIFileBrowserItem*)aa;
	CGUIFileBrowserItem* b = (CGUIFileBrowserItem*)bb;
	if(a && b)
	{
		if(a->dir==0 && b->dir)
			return 1;
		if(b->dir==0 && a->dir)
			return -1;
		return _stricoll(a->name,b->name);
	}else
		return 0;
}

CGUITreeItem* CGUIFileBrowser::AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool _manualUpdate, bool _rootItemBackground, bool first)
{
	CGUITreeItem *ti;
	CGUITreeItem* predek;
	int index=0;

	predek = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(where));
	if(predek && predek->associatedWindow==this)
	{
		index=predek->index+1;
		ReIndex(predek,1);
	}else{
		if(first)
		{
			predek=0;
			index=0;
			ReIndex(0,1);
		}else{
			predek = items->GetFromBack();
			if(predek)
			{
				index=predek->index+1;
			}
		}
	}

	//ti = new CGUITreeItem(this,0,0,labelString,_picture,_help);
	ti = new CGUIFileBrowserItem(this,labelString,true,_picture,_help,_rootItemBackground);
	ti->index=index;
	ti->tree=this;
	ti->compareFunction=compareFunction;
	ti->manualUpdate=_manualUpdate;
	if(_manualUpdate)
		ti->AcceptEvent(GetID(),ETreeItemUpdate);
	ti->AcceptEvent(GetID(),EClicked);
	ti->AcceptEvent(GetID(),EMouseButton);
	ti->AcceptEvent(GetID(),EEvent);

	items->AddToBehindObj(predek,ti);

	//AddBackElem(ti);
	AddTreeItemEngine(ti);

	//UpdateTree();

	return ti;
}


void CGUIFileBrowser::SetDirMenu(bool create)
{
	CGUIMenu* menu;

	menu = GetObjFromID<CGUIMenu>(dirMenuID);
	SAFE_DELETE(menu);
	dirMenuID=0;

	if(create)
	{
		menu = new CGUIMenu(170);
		menu->AddMenuItem(0,"Create Dir",new CGUIRectHost(0,0,styleSet->Get("KrkalTB3")->GetTexture(0)));
		menu->AddMenuItem(0,"Delete Dir",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));


		menu->AcceptEvent(GetID(),EMenu);
		dirMenuID = menu->GetID();
	}
}

void CGUIFileBrowser::SetFileMenu(bool create)
{
	CGUIMenu* menu;

	menu = GetObjFromID<CGUIMenu>(fileMenuID);
	SAFE_DELETE(menu);
	fileMenuID=0;

	if(create)
	{
		menu = new CGUIMenu(170);
		menu->AddMenuItem(0,"Delete File",new CGUIRectHost(0,0,styleSet->Get("LebkyTB3")->GetTexture(0)));


		menu->AcceptEvent(GetID(),EMenu);
		fileMenuID = menu->GetID();
	}
}

void CGUIFileBrowser::FileSysChanged()
{
	CGUIFileBrowserItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUIFileBrowserItem*)items->GetObjectFromListElement(le)))
		{
			if(!FS->FileExist(e->name))	// soubor/adresar daneho jmena ve FS neexistuje
			{
				// delete, pokracuj na dalsi korenovy prvek (soubor/adresar)
				le=items->GetNextListElement(le);
				DeleteTreeItem(e);
			}
			else
			{
				e->FileSysChanged();
				le=items->GetNextListElement(le);
			}
		}
		else
			le=items->GetNextListElement(le);
	}

	UpdateTree();
}


void CGUIFileBrowser::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	bool eventDone=false;	// udalost zpracovana, jiz nevolat zpracovani u predka

	if(event->eventID == EMenu)
	{
		CGUIFileBrowserItem* fbi = GetObjFromID<CGUIFileBrowserItem>(event->pID2);
		if(event->sender == dirMenuID)
		{
			switch(event->pInt)
			{
				case 0 : // Create Dir
					// !@#$ vytvorit CGUIDlgEditbox a z nej precist zadane jmeno noveho adresare
					// umistim ho jako podadresar "fbi"
					if(msgBoxID)
					{
						CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(msgBoxID));
						if(dlg)
						{
							dlg->CloseWindow();
							msgBoxID=0;
						}
					}
					

					if(!newDirName)
						newDirName = new char[200+1];

					CGUIDlgEditbox *dlg;
					dlg = new CGUIDlgEditbox(100,100,300,"New Dir","Directory Name:");
					msgBoxID=dlg->GetID();
					msgBoxIndex = 0;

					desktop->AddBackElem(dlg);
					dlg->AcceptEvent(GetID(),EOk);
					dlg->BindEditBox(newDirName,dtString,200);
					dlg->Center();
					dlg->SetModal(1);
					//dlg->FocusMe();

					break;
				case 1 : // Delete Dir
					char message[100];
					_snprintf(message,99,"Delete directory %s?",fbi->name);
					CGUIMessageBox* mb = GUIMessageBoxOkCancel("Delete Dir",message,1);
					mb->AcceptEvent(GetID(),EOk);
					mb->AcceptEvent(GetID(),ECancel);
					msgBoxID = mb->GetID();
					msgBoxIndex = 1;
					break;
			}
			eventDone=true;
		}
		else if(event->sender == fileMenuID)
		{
			switch(event->pInt)
			{
				case 0 : // Delete file
					char message[100];
					_snprintf(message,99,"Delete file %s?",fbi->name);
					CGUIMessageBox* mb = GUIMessageBoxOkCancel("Delete file",message,1);
					mb->AcceptEvent(GetID(),EOk);
					mb->AcceptEvent(GetID(),ECancel);
					msgBoxID = mb->GetID();
					msgBoxIndex = 2;
					break;
			}
			eventDone=true;
		}
	}

	if(event->sender == msgBoxID)
	{
		CGUIFileBrowserItem* fbi = GetObjFromID<CGUIFileBrowserItem>(selecetedFileItemID);
		if(event->eventID == EOk && fbi)
		{
			if(msgBoxIndex==0)	// Create Dir
			{
				CGUIDlgEditbox *dlg = dynamic_cast<CGUIDlgEditbox*>(nameServer->ResolveID(msgBoxID));
				if(dlg)
				{
					if(dlg->SyncEditBox()==0)
					{
						if(FS->IsValidFilename(newDirName))
						{
							SetDir(fbi);				
							if(FS->FileExist(newDirName))
							{
								dlg->ShowErrorMsg("Same directory exists yet");
							}
							else
							{
								// vytvor adresar se jmenem "newDirName"

								FS->CreateDir(newDirName);
								if(fbi->prepared)
								{
									AddItem(fbi,newDirName,1);
									fbi->Sort();
									UpdateTree();
								}

								dlg->CloseWindow();
								msgBoxID=0;
								msgBoxIndex=-1;
								selecetedFileItemID=0;
								SAFE_DELETE_ARRAY(newDirName);
							}
						}
						else
						{
							dlg->ShowErrorMsg("Name not valid");
						}
					}
				}
			}
			else if(msgBoxIndex==1)	// Delete Dir
			{
				SetDir(fbi);
				if(FS->ChangeDir(".."))
				{
					FS->Delete(fbi->name);

					fbi->SwitchItem(false);
					CGUITreeItem* ti = fbi->parentItem;
					if(ti)
						ti->DeleteTreeItem(fbi);
					else
						DeleteTreeItem(fbi);
					if(ti && ti->state==0)
					{
						ti->state=1;
						ti->ChangeRoller();
					}

					UpdateTree();
				}

				msgBoxID=0;
				msgBoxIndex=-1;
				selecetedFileItemID=0;
			}
			else if(msgBoxIndex==2)	// Delete file
			{
				SetDir(fbi);
				FS->Delete(fbi->name);

				fbi->SwitchItem(false);
				CGUITreeItem* ti = fbi->parentItem;
				if(ti)
					ti->DeleteTreeItem(fbi);
				else
					DeleteTreeItem(fbi);
				if(ti && ti->state==0)
				{
					ti->state=1;
					ti->ChangeRoller();
				}

				UpdateTree();

				msgBoxID=0;
				msgBoxIndex=-1;
				selecetedFileItemID=0;
			}
		}
		else if(event->eventID == ECancel)
		{
			msgBoxID=0;
			msgBoxIndex=-1;
			selecetedFileItemID=0;
		}
		eventDone=true;
	}



	if(eventDone)
		delete event;
	else
		CGUITree::EventHandler(event);
}

int CGUIFileBrowser::MarkFileItem(char* root, char *file)
{
	if(!root)
		return 1;

	CGUIFileBrowserItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUIFileBrowserItem*)items->GetObjectFromListElement(le)))
		{
			if(!strcmp(e->name,root))
			{
				file = strtok(file,"\\");
				return e->MarkFileItem(file);
			}

		}
		le=items->GetNextListElement(le);
	}

	return 1;
}


int CGUIFileBrowserItem::MarkFileItem(char* name)
{
	if(!prepared)
		tree->TreeUpdateHandler(GetID(),index);

	if(!name)
	{
		// dosel jsem na konec cesty => tato polozka odpovida hledanemu souboru
		tree->RollTreeToItem(this);
		UpdateTree();
		tree->Switch(this);
		return 0;
	}

	CGUIFileBrowserItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUIFileBrowserItem*)items->GetObjectFromListElement(le)))
		{
			if(!strcmp(e->name,name))
			{
				name = strtok(NULL,"\\");	// najdi dalsi token cesty (adresar/soubor)
				return e->MarkFileItem(name);
			}
		}
		le=items->GetNextListElement(le);
	}

	return 1;
}


CGUIFileBrowserItem::CGUIFileBrowserItem(CGUITree* _tree, char *labelString, bool _rootTree, CGUIRectHost *_picture, char *_help, bool rootItemBackground)
	: CGUITreeItem(_tree,labelString,_rootTree,_picture,_help,rootItemBackground)
{
	dir=0;
	name=0;
}

CGUIFileBrowserItem::~CGUIFileBrowserItem()
{
	SAFE_DELETE_ARRAY(name);
}


CGUITreeItem* CGUIFileBrowserItem::AddTreeItem(typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool _manualUpdate, bool first)
{
	CGUITreeItem *ti;
	CGUITreeItem* predek;
	int index=0;

	predek = dynamic_cast<CGUITreeItem*>(nameServer->ResolveID(where));
	if(predek && predek->parentItem==this)
	{
		index=predek->index+1;
		ReIndex(predek,1);
	}else{
		if(first)
		{
			predek=0;
			index=0;
			ReIndex(0,1);
		}else{
			predek = items->GetFromBack();
			if(predek)
			{
				index=predek->index+1;
			}
		}
	}

	if(tree)
		ti = new CGUIFileBrowserItem(tree,labelString,false,_picture,_help);
	else
		ti = new CGUITreeItem(associatedWindow,0,0,labelString,_picture,_help);
	ti->index=index;
	ti->rootTree=false;
	ti->itemMarker=itemMarker;
	ti->parentItem=this;
	ti->tree=tree;
	ti->compareFunction=compareFunction;
	ti->manualUpdate=_manualUpdate;
	if(_manualUpdate)
		ti->AcceptEvent(tree->GetID(),ETreeItemUpdate);
	ti->AcceptEvent(tree->GetID(),EClicked);
	ti->AcceptEvent(tree->GetID(),EMouseButton);
	ti->AcceptEvent(tree->GetID(),EEvent);

	items->AddToBehindObj(predek,ti);

	if(state==0)
	{
		state=1;
		ChangeRoller();
		//SetRollerPicture(new CGUIRectHost(0,0,styleSet->Get("Roller+")->GetTexture(0)));
	}

	if(tree)
		tree->AddTreeItemEngine(ti);
	else
		associatedWindow->AddBackElem(ti);
	ti->SetVisible(0);

	return ti;
}


void CGUIFileBrowserItem::FileSysChanged()
{
	if(!prepared || state==0)
		return;

	if(state==1)	// zabaleny uzel => smazu vsechny jeho podpolozky a nastavim prepared==false
					// pri pristim rozbaleni se nacte znovu jeho obsah z disku
	{
		while(CGUITreeItem* tti = items->GetFromHead())
		{
			DeleteTreeItem(tti);
		}

		prepared=false;
	}
	else if(state==2)	// jedna se o rozbaleny uzel
	{
		// zruseni polozek po smazanych souborech
		CGUIFileBrowserItem *e;
		CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

		while(le)
		{
			if((e=(CGUIFileBrowserItem*)items->GetObjectFromListElement(le)))
			{
				((CGUIFileBrowser*)tree)->SetDir(this);
				if(!FS->FileExist(e->name))	// soubor/adresar daneho jmena ve FS neexistuje
				{
					// delete, pokracuj na dalsi korenovy prvek (soubor/adresar)
					le=items->GetNextListElement(le);
					DeleteTreeItem(e);
				}
				else
				{
					e->FileSysChanged();
					le=items->GetNextListElement(le);
				}
			}
			else
				le=items->GetNextListElement(le);
		}

		//pridani novych polozek pro nove soubory
		UpdateNewFiles();
	}
}

void CGUIFileBrowserItem::UpdateNewFiles()
{
	if(dir==0)
		return;

	CGUIFileBrowser* fb = (CGUIFileBrowser*)tree;
	if(!fb)
		return;

	if(!fb->SetDir(this))
		return;

	const char *filename;
	int dir;

	if(FS->FindFirst(&filename,dir) && strcmp(filename,"..") && fb->Filter(this,filename,dir,1))
	{
		if(!FindFile(filename))
			fb->AddItem(this,filename,dir);
	}

	while(FS->FindNext(&filename,dir) && fb->Filter(this,filename,dir,0))
	{
		if(!FindFile(filename))
			fb->AddItem(this,filename,dir);
	}
	Sort();
}

CGUIFileBrowserItem* CGUIFileBrowserItem::FindFile(const char* name)
{
	CGUIFileBrowserItem *e;
	CGUIListElement<CGUITreeItem>* le = items->GetNextListElement(0);

	while(le)
	{
		if((e=(CGUIFileBrowserItem*)items->GetObjectFromListElement(le)))
		{
			if(!strcmp(e->name,name))
				return e;
		}
		le=items->GetNextListElement(le);
	}

	return 0;
}
