///////////////////////////////////////////////
//
// menu.cpp
//
// Implemementace pro menu (ContextMenu)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "gui.h"
#include "menu.h"


#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIMenuItem
//////////////////////////////////////////////////////////////////////

const int CGUIMenuItem::numEvents = 6;
const int CGUIMenuItem::availableEvents[numEvents] = {EMouseOver,EMouseButton,EClicked,
									   				  EFocusChanged,EDeleted,EKeyboard};

CGUIMenuItem::CGUIMenuItem(CGUIMenu* menu, float _x, float _y, float _sx, float _sy, char *labelString, CGUIRectHost *_picture, char *_help)
	: CGUIElement(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	rootwnd=0;
	active=true;
	markItem=false;
	index=0;
	subMenuIndex=-1;

	key_exclusive_access=false;
	sc_key_ctrl=sc_key_alt=sc_key_shift=false;
	sc_charCode=0;

	SetMark(true);

	if(menu)
		menuID = menu->GetID();
	else
		menuID = 0;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseR|MsgKeyboard|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	SetSize(_sx,_sy);

	if(labelString)
	{
		float menu_picture_strip_size=menu ? menu->menu_picture_size : STD_MENU_PICTURE_STRIP_SIZE;
		label = new CGUIStaticText(0,0,sx-menu_picture_strip_size-STD_MENU_EXTENSION_SIZE-6,labelString,ARIAL,STD_MENU_TEXT_SIZE,false,aLeft,STD_MENU_TEXT_COLOR);
	}else
		label=0;

	picture = pictureInactive = pictureActive = _picture;
	pictureExtender=0;
	keyShortCut=0;

	SetPositionOfLabelPicture();

	if(_help)
	{
		help = newstrdup(_help);
	}else
		help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

void CGUIMenuItem::SetShortCut(char* text, bool key_ctrl, bool key_alt, bool key_shift, int char_code, bool exclusive_access)
{
	CBltElem* pos=0;
	if(keyShortCut)
		pos = keyShortCut->GetElemPosition();
	SAFE_DELETE(keyShortCut);

	sc_key_ctrl = key_ctrl;
	sc_key_alt = key_alt;
	sc_key_shift = key_shift;
	sc_charCode = char_code;
	desktop->AcceptEvent(GetID(),EKeyboard);	// objednam si posilani klaves od desktopu
												// budou mi chodit i kdyz nemam focus
	key_exclusive_access=exclusive_access;

	if(!text)
		return;

	float STD_MENU_PICTURE_STRIP_SIZE=::STD_MENU_PICTURE_STRIP_SIZE;
	float STD_MENU_PICTURE_SIZE=::STD_MENU_PICTURE_SIZE;
	CGUIMenu* menu = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(menuID));
	if(menu)
	{
		STD_MENU_PICTURE_STRIP_SIZE=menu->picture_strip_size;
		STD_MENU_PICTURE_SIZE=menu->menu_picture_size;
	}

	keyShortCut = new CGUIStaticText(0,0,sx-STD_MENU_PICTURE_STRIP_SIZE-STD_MENU_EXTENSION_SIZE-6,text,ARIAL,STD_MENU_TEXT_SIZE,false,aRight,STD_MENU_TEXT_COLOR);
	keyShortCut->RemoveFromTrash();

	SetPositionOfLabelPicture();

	if(keyShortCut && rootwnd)
	{
		if(pos)
			keyShortCut->AddToEngineBackPosition(rootwnd,pos);
		else
			keyShortCut->AddToEngine(rootwnd);
	}
}

void CGUIMenuItem::SetLabel(char *text)
{
	CBltElem* pos=0;
	if(label)
		pos = label->GetElemPosition();
	SAFE_DELETE(label);
	if(!text)
		return;
	
	float STD_MENU_PICTURE_STRIP_SIZE=::STD_MENU_PICTURE_STRIP_SIZE;
	float STD_MENU_PICTURE_SIZE=::STD_MENU_PICTURE_SIZE;
	CGUIMenu* menu = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(menuID));
	if(menu)
	{
		STD_MENU_PICTURE_STRIP_SIZE=menu->picture_strip_size;
		STD_MENU_PICTURE_SIZE=menu->menu_picture_size;
	}

	label = new CGUIStaticText(0,0,sx-STD_MENU_PICTURE_STRIP_SIZE-STD_MENU_EXTENSION_SIZE-6,text,ARIAL,STD_MENU_TEXT_SIZE,false,aLeft,STD_MENU_TEXT_COLOR);
	label->RemoveFromTrash();

	SetPositionOfLabelPicture();

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUIMenuItem::SetLabelFormated(char *text, CGUIFont* font, UINT color)
{
	CBltElem* pos=0;
	if(label)
		pos = label->GetElemPosition();
	SAFE_DELETE(label);
	if(!text)
		return;
	
	float STD_MENU_PICTURE_STRIP_SIZE=::STD_MENU_PICTURE_STRIP_SIZE;
	float STD_MENU_PICTURE_SIZE=::STD_MENU_PICTURE_SIZE;
	CGUIMenu* menu = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(menuID));
	if(menu)
	{
		STD_MENU_PICTURE_STRIP_SIZE=menu->picture_strip_size;
		STD_MENU_PICTURE_SIZE=menu->menu_picture_size;
	}

	//label = new CGUIStaticText(0,0,sx-STD_MENU_PICTURE_STRIP_SIZE-STD_MENU_EXTENSION_SIZE-6,text,ARIAL,STD_MENU_TEXT_SIZE,false,aLeft,STD_MENU_TEXT_COLOR);
	label = new CGUIStaticText(text,font,0,0,color,sx-STD_MENU_PICTURE_STRIP_SIZE-STD_MENU_EXTENSION_SIZE-6,0,aLeft,1);
	label->RemoveFromTrash();

	SetPositionOfLabelPicture();

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUIMenuItem::SetPicture(CGUIRectHost *_picture)
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
		SetPositionOfLabelPicture();
	}else{
		if(picture==pictureActive)
		{
			if(picture)	
				pos = picture->GetElemPosition();
			SAFE_DELETE(picture);
			picture = _picture;
			pictureInactive = _picture;
			SetPositionOfLabelPicture();
		}else
			SAFE_DELETE(pictureActive);
	}

	pictureActive = _picture;
	
	if((active || picture==pictureInactive) && picture && rootwnd)
	{
		if(pos)
			picture->AddToEngineBackPosition(rootwnd,pos);
		else
			picture->AddToEngine(rootwnd);
	}
}

void CGUIMenuItem::ChangeContextHelp(char* text)
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

void CGUIMenuItem::SetPositionOfLabelPicture()
{
	float pos_x, pos_y, siz_x, siz_y;	// atributy buttonu
	float p_x, p_y, p_sx, p_sy;			// atributy obrazku
	float l_x, l_y, l_sx, l_sy;			// atributy popisku

	GetPos(pos_x,pos_y);
    GetSize(siz_x,siz_y);
	

	float STD_MENU_PICTURE_STRIP_SIZE=::STD_MENU_PICTURE_STRIP_SIZE;
	float STD_MENU_PICTURE_SIZE=::STD_MENU_PICTURE_SIZE;
	CGUIMenu* menu = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(menuID));
	if(menu)
	{
		STD_MENU_PICTURE_STRIP_SIZE=menu->picture_strip_size;
		STD_MENU_PICTURE_SIZE=menu->menu_picture_size;
	}

	if(STD_MENU_PICTURE_SIZE>siz_y-2)
		STD_MENU_PICTURE_SIZE=siz_y-2;

	if(label)
	{
		label->GetSize(l_sx,l_sy);
		l_x=pos_x+STD_MENU_PICTURE_STRIP_SIZE+2;
		l_y=pos_y+(siz_y-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));
	}
	
	if(picture)
	{
	    picture->GetSize(p_sx,p_sy);
		if(p_sx>STD_MENU_PICTURE_SIZE && p_sy>STD_MENU_PICTURE_SIZE)
			picture->Resize(STD_MENU_PICTURE_SIZE,STD_MENU_PICTURE_SIZE);
		else if(p_sx>STD_MENU_PICTURE_SIZE)
			picture->Resize(STD_MENU_PICTURE_SIZE,p_sy);
		else if(p_sy>STD_MENU_PICTURE_SIZE)
			picture->Resize(p_sx,STD_MENU_PICTURE_SIZE);

		picture->GetSize(p_sx,p_sy);
		p_x=pos_x+(STD_MENU_PICTURE_STRIP_SIZE-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		picture->Move(floorf(p_x),floorf(p_y));
	}

	if(pictureExtender)
	{
	    pictureExtender->GetSize(p_sx,p_sy);
		if(p_sx>STD_MENU_EXTENSION_SIZE && p_sy>STD_MENU_EXTENSION_SIZE)
			picture->Resize(STD_MENU_EXTENSION_SIZE,STD_MENU_EXTENSION_SIZE);
		else if(p_sx>STD_MENU_EXTENSION_SIZE)
			picture->Resize(STD_MENU_EXTENSION_SIZE,p_sy);
		else if(p_sy>STD_MENU_EXTENSION_SIZE)
			picture->Resize(p_sx,STD_MENU_EXTENSION_SIZE);

		pictureExtender->GetSize(p_sx,p_sy);
		p_x=pos_x+siz_x-STD_MENU_EXTENSION_SIZE+(STD_MENU_EXTENSION_SIZE-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureExtender->Move(floorf(p_x),floorf(p_y));
	}

	if(keyShortCut)
	{
		keyShortCut->GetSize(l_sx,l_sy);
		l_x=pos_x+siz_x-STD_MENU_EXTENSION_SIZE-l_sx-2;
		l_y=pos_y+(siz_y-l_sy)/2;

		if(label)
		{
			float ll_x,ll_y,ll_sx,ll_sy;
			label->GetPos(ll_x,ll_y);
			label->GetSize(ll_sx,ll_sy);
			if((l_x<ll_x+ll_sx))
			{
				if(keyShortCut->IsVisible())
					keyShortCut->SetVisible(0);
			}else
				keyShortCut->SetVisible(1);
		}
		keyShortCut->Move(floorf(l_x),floorf(l_y));
	}
}

CGUIMenuItem::~CGUIMenuItem()
{
	RemoveFromTrash();
	if(label)
		delete label;

	if(pictureActive!=pictureInactive)
	{
		SAFE_DELETE(pictureActive);
		SAFE_DELETE(pictureInactive);
	}else
		SAFE_DELETE(pictureActive);

	SAFE_DELETE(pictureExtender);
	SAFE_DELETE(keyShortCut);

	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	EventArise(EDeleted,0,0,this);
}

void CGUIMenuItem::SetInactivePicture(CGUIRectHost *_pictureInactive)
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

		SetPositionOfLabelPicture();
		
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}
}

void CGUIMenuItem::SetExtenderPicture(CGUIRectHost *_pictureExtender)
{
	CBltElem* pos=0;
	if(_pictureExtender)
		_pictureExtender->RemoveFromTrash();

	if(rootwnd && picture)
	{
		pos = picture->GetElemPosition();
	}

	SAFE_DELETE(pictureExtender);

	pictureExtender = _pictureExtender;
	SetPositionOfLabelPicture();

	if(pictureExtender && rootwnd)
	{
		if(pos)
			pictureExtender->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureExtender->AddToEngine(rootwnd);
	}
}

int CGUIMenuItem::Activate()
// nastavi promennou active a zmeni grafiku polozky na aktivni
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
		SetPositionOfLabelPicture();
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}

	if(pictureExtender)
	{
		pictureExtender->SetVisible(1);
	}

	if(label)
		label->SetColor(STD_MENU_TEXT_COLOR);
	if(keyShortCut)
		keyShortCut->SetColor(STD_MENU_TEXT_COLOR);

	active=true;
	return ret;
}

int CGUIMenuItem::Deactivate()
// nastavi promennou active a zmeni grafiku polozky na neaktivni
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
		SetPositionOfLabelPicture();
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}
	
	if(pictureExtender)
		pictureExtender->SetVisible(0);

	if(label)
		label->SetColor(STD_MENU_TEXT_INACTIVE_COLOR);
	if(keyShortCut)
		keyShortCut->SetColor(STD_MENU_TEXT_INACTIVE_COLOR);

	active=false;
	return ret;
}

int CGUIMenuItem::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	if(mouseState)
	{
		// otevrit podmenu
		EventArise(EMouseButton,0,mouseState);
	}else{

		OnClick();		// klik je na pusteni leveho tlacitka nad polozkou

		EventArise(EClicked);
		EventArise(EMouseButton,0,mouseState);
	}

	return 0;
}

int CGUIMenuItem::MouseOver(float x, float y, UINT over, UINT state)
{
	if(!active)
		return 0;

	EventArise(EMouseOver);

	if(help && over && !(state & MK_LBUTTON) && !(state & MK_RBUTTON) && !(state & MK_MBUTTON))
	{
		CGUIContextHelp::On(this,help);
	}

	return 0;
}

int CGUIMenuItem::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	if(!active)
		return 0;

	return 0;
}

int CGUIMenuItem::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		EventArise(EFocusChanged,focus);
		return 0;
	}

	if(help)
		CGUIContextHelp::Off(this);

	EventArise(EFocusChanged,focus);
	return 0;
}

int CGUIMenuItem::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

void CGUIMenuItem::OnClick()	// k uzivatelskemu pretizeni 
{
}

int CGUIMenuItem::Keyboard(UINT character, UINT state)
{
	if(!active)
		return 0;

	if((character == VK_RETURN) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		OnClick();
		EventArise(EClicked);
	}

	EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	return 0;
}

void CGUIMenuItem::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EKeyboard && (event->pFloat==0 || key_exclusive_access))	// nejedna se o exklusivni pristup, muzu zpracovat prikaz z klavesnice, nebo mam sam exkluzivni pristup povolen
	{
		if(active && event->pInt == sc_charCode && 
			((sc_key_ctrl && event->pInt2!=0) || (!sc_key_ctrl && event->pInt2==0)) &&
			((sc_key_alt && event->pID2!=0) || (!sc_key_alt && event->pID2==0)) &&
			((sc_key_shift && event->pFloat2!=0) || (!sc_key_shift && event->pFloat2==0)))
		{
			// pokud byla stisknuta klavesova zkratka vyvolej akci polozky
			// udalost EKeyboard objednana od desktopu, chodi o kdyz nemam focus

			OnClick();
			EventArise(EClicked);
		}
	}

	delete event;
}

int CGUIMenuItem::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti pravym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	if(!mouseState)
	{
		OnClick();
		EventArise(EClicked);
	}

	EventArise(EMouseButton,1,mouseState);
	
	return 0;
}

void CGUIMenuItem::AddToEngine(CBltWindow *rootwnd)
{
	CGUIMenuItem::rootwnd = rootwnd;
	if(!marker && mark)
	{
		CreateMarkStructures(rootwnd);
	}

	if(label)
		label->AddToEngine(rootwnd);
	if(picture)
		picture->AddToEngine(rootwnd);
	if(pictureExtender)
		pictureExtender->AddToEngine(rootwnd);
	if(keyShortCut)
		keyShortCut->AddToEngine(rootwnd);

}

void CGUIMenuItem::RemoveFromEngine()
{
	CGUIMenuItem::rootwnd=0;
	if(label)
		label->RemoveFromEngine();
	if(picture)
		picture->RemoveFromEngine();
	if(pictureExtender)
		pictureExtender->RemoveFromEngine();
	if(keyShortCut)
		keyShortCut->RemoveFromEngine();
}

void CGUIMenuItem::CreateMarkStructures(CBltWindow* win)
{
	if(win)
	{
		marker = new CGUIMenuMarker(rootwnd,x+1,y+1,x+sx-2,y+sy-2);
		marker->UnMark();
	}
}

void CGUIMenuItem::Move(float _x, float _y)
{
	SetPos(_x,_y);

	SetPositionOfLabelPicture();

	if(marker)
		marker->Rebuild(this);
}


//////////////////////////////////////////////////////////////////////
// CGUIMenu
//////////////////////////////////////////////////////////////////////

const int CGUIMenu::numEvents = 5;
const int CGUIMenu::availableEvents[numEvents] = {EMenu,EAddElement,EDeleted,EOn,EOff};

CGUIMenu::CGUIMenu(float _itemWidth, float _itemHeight)
	:  CGUIMultiWidget(0,0,10,10)
{
	RemoveFromTrash();

	SetAvailableEvents(availableEvents,numEvents);

	activator=0;
	parentMenu=0;
	//subMenuArray=0;
	parentMenuActivatorItem=0;
	menuActivatorItem=0;
	itemWidth = _itemWidth;
	itemHeight = _itemHeight;
	active=false;
	preclick=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseR|MsgMouseM|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD|MsgKeyboard);
	//MsgSetProduce(MsgNone);


	Resize(itemWidth+2,2);

	CGUIStaticPicture* sp = new CGUIStaticPicture(0,0,sx,sy,STD_MENU_BGCOLOR_COLOR);
	sp->SetAnchor(1,1,1,1);
	AddElem(sp);

	strip = new CGUIStaticPicture(0,0,STD_MENU_PICTURE_STRIP_SIZE,sy,STD_MENU_PICTURE_STRIP_COLOR);
	strip->SetAnchor(1,1,1,0);
	AddElem(strip);


	CGUILine* l;
	l = new CGUILine(0,0,sx-1,0,STD_MENU_BORDER_COLOR);
	l->SetAnchor(1,0,1,1);
	AddElem(l);
	l = new CGUILine(sx-1,0,sx-1,sy-1,STD_MENU_BORDER_COLOR);
	l->SetAnchor(1,1,0,1);
	AddElem(l);
	l = new CGUILine(sx-1,sy-1,0,sy-1,STD_MENU_BORDER_COLOR);
	l->SetAnchor(0,1,1,1);
	AddElem(l);
	l = new CGUILine(0,sy-1,0,0,STD_MENU_BORDER_COLOR);
	l->SetAnchor(1,1,1,0);
	AddElem(l);

	SetVisible(0);

	picture_strip_size=STD_MENU_PICTURE_STRIP_SIZE;
	menu_picture_size=STD_MENU_PICTURE_SIZE;

	//AddToTrash();
	desktop->AddFrontElem(this);
	//desktop->AddBackElem(this);
}

CGUIMenu::~CGUIMenu()
{
	//SAFE_DELETE_ARRAY(subMenu);
	if(parent && parent->GetType()>=PTWindow)
		((CGUIWindow*)parent)->RemoveFrontElem(this);

	EventArise(EDeleted,0,0,this);
}

void CGUIMenu::Add()
{
	if(!desktop->ContainsFrontElement(this))
		desktop->AddFrontElem(this);
}

void CGUIMenu::Remove()
{
	desktop->RemoveFrontElem(this);
}

void CGUIMenu::AddElemBehind(CGUIElement *where, CGUIElement *element)
{
	if(element && window){
		if(element->GetList() && !element->GetParent())
		{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
			trash->Remove(element);
		}
		el_queue.AddToBehindObj(where,element);
		element->SetParent(this);
		element->AddToEngine(window);

		if((element->GetTabOrder()>0 && tab_min>element->GetTabOrder()) || tab_min==0)
			tab_min=element->GetTabOrder();
		if(tab_max<element->GetTabOrder())
			tab_max=element->GetTabOrder();


		typeID id;
		CGUIEventProgram* ep = dynamic_cast<CGUIEventProgram*>(element);
		if(ep)
			id=ep->GetID();
		else
			id=0;
		EventArise(EAddElement,0,id,element);
	}
}

void CGUIMenu::MoveItems(CGUIElement *start, float dy, int d_index)
{
	// start jiz neposouvam
	// pokud je start == 0 posouvam vsechny polozky od zacatku seznamu
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);
	bool found=false;

	if(!start)
		found=true;

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			if(found)
			{
				CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(e);
				if(mi)
				{
					mi->MoveRel(0,dy);
					mi->index+=d_index;
					mi->SetTabOrder((float)(mi->index+1));	// taborder polozky odpovida jejimu indexu (poradi)
				}else{
					CGUIMenuGap* mg = dynamic_cast<CGUIMenuGap*>(e);
					if(mg)
					{
						mg->MoveRel(0,dy);
					}
				}
			}
			if(e==start)
				found=true;
		}
		le=el_queue.GetNextListElement(le);
	}
}

CGUIElement* CGUIMenu::FindLastItem()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(e);
			if(mi)
				return mi;
			CGUIMenuGap* mg = dynamic_cast<CGUIMenuGap*>(e);
			if(mg)
				return mg;
		}
		le=el_queue.GetPrevListElement(le);
	}
	return 0;
}

int CGUIMenu::FindIndex(CGUIElement *start)
{
	if(CGUIMenuItem* si = dynamic_cast<CGUIMenuItem*>(start))
	{
		return si->index;
	}

	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElementObj(start);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(e);
			if(mi)
				return mi->index;
		}
		le=el_queue.GetPrevListElement(le);
	}

	return 0;
}


CGUIMenuItem* CGUIMenu::GetItem(int index)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(e);
			if(mi && mi->index==index)
				return mi;
		}
		le=el_queue.GetNextListElement(le);
	}
	return 0;
}

CGUIMenuItem* CGUIMenu::AddMenuItem(typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool first)
{
	CGUIMenuItem *mi;
	CGUIElement* predek;
	float my=1,mx;	// urcuje polohu pro novou polozku
	float msx,msy;
	int index=0;

	predek = dynamic_cast<CGUIElement*>(nameServer->ResolveID(where));	// najde polozku za kterou se bude pridavat
	if(predek && predek->GetParent()==this)
	{
		// pripravi polozky menu pro pridani nove na nalezene misto
		predek->GetPos(mx,my);
		predek->GetSize(msx,msy);
		my+=msy;
		index=FindIndex(predek)+1;
		MoveItems(predek,itemHeight,1);
	}else{
		// polozka pro pridavani nenalezena
		if(first)
		{
			// pridani na zacatek, uvolnit prvni pozici pro novou polozku
			predek=0;
			my=1;
			index=0;
			MoveItems(0,itemHeight,1);
		}else{
			// pridani na konec
			predek = FindLastItem();
			if(predek)
			{
				predek->GetPos(mx,my);
				predek->GetSize(msx,msy);
				my+=msy;
				index=FindIndex(predek)+1;
			}
		}
	}

	mi = new CGUIMenuItem(this,1,my,itemWidth,itemHeight,labelString,_picture,_help);	// vytvorit novou polozku
	mi->SetMark(true);
	mi->index=index;	// nastaveni poradi
	mi->SetTabOrder((float)(index+1));	// taborder polozky odpovida jejimu indexu (poradi)
	mi->AcceptEvent(GetID(),EClicked);	// objednani kliknuti na polozku kvuli posilani centralni udalosti EMenu od celeho menu

	AddElemBehind(predek,mi);	// pridani nove polozke do menu na svoji pozici

	ResizeRel(0,itemHeight);	// zvetseni menu o novou polozku

	return mi;		// vrati adresu nove polozky
}

CGUIMenuItem* CGUIMenu::AddMenuItem(typeID where, CGUIMenuItem* menuItem, bool first)
{
	if(!menuItem)
		return 0;

	CGUIElement* predek;
	float my=1,mx;
	float msx,msy;
	int index=0;
	float mix,miy;

	menuItem->GetSize(mix,miy);

	predek = dynamic_cast<CGUIElement*>(nameServer->ResolveID(where));
	if(predek && predek->GetParent()==this)
	{
		predek->GetPos(mx,my);
		predek->GetSize(msx,msy);
		my+=msy;
		index=FindIndex(predek)+1;
		MoveItems(predek,miy,1);
	}else{
		if(first)
		{
			predek=0;
			my=1;
			index=0;
			MoveItems(0,miy,1);
		}else{
			predek = FindLastItem();
			if(predek)
			{
				predek->GetPos(mx,my);
				predek->GetSize(msx,msy);
				my+=msy;
				index=FindIndex(predek)+1;
			}
		}
	}

	menuItem->Move(1,my);
	//menuItem->Resize(itemWidth,itemHeight);
	menuItem->menuID=GetID();
	menuItem->SetMark(true);
	menuItem->index=index;
	menuItem->SetTabOrder((float)(index+1));
	menuItem->AcceptEvent(GetID(),EClicked);

	AddElemBehind(predek,menuItem);

	ResizeRel(0,miy);

	return menuItem;
}


void CGUIMenu::DeleteMenuItem(CGUIMenuItem* mi)
{
	if(!mi)
		return;

	float mx,my;

	mi->GetSize(mx,my);
	MoveItems(mi,-my,-1);

	DeleteElem(mi);

	ResizeRel(0,-my);
}

CGUIMenuItem* CGUIMenu::AddSubMenuItem(CGUIMenu* subMenu, typeID where, char *labelString, CGUIRectHost *_picture, char *_help, bool first)
{
	CGUIMenuItem* mi = AddMenuItem(where,labelString,_picture,_help,first);	// prida aktivacni polozku subMenu
	if(mi)
	{
		// propoji polozku se subMenu:
		mi->SetExtenderPicture(new CGUIRectHost(0,0,styleSet->Get("MenuExtender")->GetTexture(0)));
		if(subMenu)
		{
			mi->subMenuIndex=subMenuArray.Add(subMenu->GetID());
			subMenu->parentMenu=this;
			subMenu->parentMenuActivatorItem=mi->GetID();
			//SetSubMenuPrograms(subMenu);
		}
	}

	return mi;
}

CGUIMenu* CGUIMenu::DeleteSubMenuItem(CGUIMenuItem* mi)
{
	if(!mi)
		return 0;

	typeID subMenuID;

	subMenuID = subMenuArray.Remove(mi->subMenuIndex);
	DeleteMenuItem(mi);	

	CGUIMenu* m = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(subMenuID));
	if(m)
	{
		m->parentMenu=0;
		m->parentMenuActivatorItem=0;
	}

	return m;
}

int CGUIMenu::SetAutoActivator(CGUIEventSupport* elm, int action)
{
	if(!action)
	{
		if(dynamic_cast<CGUIWindow*>(elm))
			action=EMessage;	// jedna se o okno
		else
			action=EMouseButton;
	}

	return elm->AcceptEvent(GetID(),action);
}


int CGUIMenu::ForwardMsg(CGUIMessage *msg)
{
	CGUIElement *elem=0;
	CGUIMessage *focus_msg=0;
	bool refocus=false;		// zda se ma zmenit focus nebo ne
	
	if(msg->IsCoordMsg())
	{
		float xx=msg->x, yy=msg->y;
		InnerCoords(xx,yy);			// prepocitani souradnic do souradne soustavy v okne (predtim je v sour. s. nadrazeneho okna)

		elem = FindElement(xx,yy);

		///*
        if(last_element_under_cursor && elem != last_element_under_cursor 
			&& (last_element_under_cursor != GetFocusEl()))
		{		// zmenil se element pod kurzorem, posilam focus_zpravu puvodnimu elementu
				// !@#$ pokud se last_element_under_cursor == GetFocusEl() dostane focus_zpravu 2x !!!
				// => jiz osetreno podminkou navic => otestovat !!!!!!!!
			focus_msg = new CGUIMessage(*msg);
			focus_msg->focus_msg=true;
			SendIntoPriorityMsg(focus_msg,last_element_under_cursor);
		}
		if(!msg->focus_msg)
			last_element_under_cursor=elem;
		else
			last_element_under_cursor=0;
//*/

		if(elem && elem == GetFocusEl())	
		{									// nasel jsem element odpovidajici poloze zpravy, 
											// ten je identicky s focusovanym elementem okna
											// posilam jen jednu zpravu a to nalezenemu elementu
			SendIntoMsg(msg,elem);
			
			/*	// ten kdo prida element do fronty ho musi focusovat => pak je vse OK
				// pokud ho nefocusuje, zustava stale focusovany puvodni element, ovsem na 
				// vrcholu klikatelnosti i viditelnosti je novy element
				// naopak focusovanim nove elementu pri jeho vzniku prijde puvodne focusovany element
				// o zpravu, kterou by jinak jako focus dostal (nedostane ji protoze uz neni focusovan)
				// dostane ovsem zparvu jinou - zpravu informujici o ztrate focusu
			if(elem->GetType()==PTWindow && msg->IsMouseButtonMsg() && msg->param && elem->focusable)
				// jistotni vyTOPovani okna (viditelnost i klikatelnost)
				// nekdo mohl pridat okno prede me
			{
				BringElementToTop(elem);	// z hlediska poradi klikatelnosti
				((CGUIWindow *)elem)->BringToTop();		// pri zmene focusu okna se nastavuje i max. uroven viditelnosti
			}
			//*/


		}else if(elem && GetFocusEl())
		{									// mam element odpovidajici poloze zpravy, ale ten je 
											// jiny nez focusovany element => posilam 2 zpravy (obema elementum)
			focus_msg = new CGUIMessage(*msg);
			focus_msg->focus_msg=true;
			SendIntoMsg(focus_msg,GetFocusEl());
			SendIntoMsg(msg,elem);
			refocus=true;
		}else
		{									// oba elementy jsou nulove, nebo alespon jeden
											// posilam tedy max. 1 zpravu a to nenulovemu elementu
											// to je bud element odpovidajici poloze zpravy, nebo focusovany el.
			if(elem)
			{
				SendIntoMsg(msg,elem);
				refocus=true;
			}else if(GetFocusEl())
			{
				msg->focus_msg=true;	
				SendIntoPriorityMsg(msg,GetFocusEl());
			}else{
				delete msg;		// nenalezel jsem element odpovidajici poloze zpravy ani neni zadny focusovany
				return 1;
			}
		}

		if(refocus && elem->focusable)	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem);	// nastavi element jako focusovany v okne
		}else if(!elem)
		{
			SetFocusEl(0);	// nenasel se zadny element, defocusuj vse (odoznaci polozku)
							// mys je mimo menu
		}

	}
	else
	{
		if((elem=GetFocusEl()))
			SendPriorityMsg(msg,elem);
		else{
			delete msg;
			return 1;
		}
						// Here comes msgs, that contains keyboard press or command
						// sent it to focused element
	}
	return 1; 
}

void CGUIMenu::AddGap(typeID where, bool first)
{
	CGUIElement* predek;
	float my=1,mx;
	float msx,msy;

	predek = dynamic_cast<CGUIElement*>(nameServer->ResolveID(where));
	if(predek && predek->GetParent()==this)
	{
		predek->GetPos(mx,my);
		predek->GetSize(msx,msy);
		my+=msy;
		MoveItems(predek,1,0);
	}else{
		if(first)
		{
			predek=0;
			my=1;
			MoveItems(0,1,0);
		}else{
			predek = FindLastItem();
			if(predek)
			{
				predek->GetPos(mx,my);
				predek->GetSize(msx,msy);
				my+=msy;
			}
		}
	}

	CGUIMenuGap* mg = new CGUIMenuGap(STD_MENU_PICTURE_STRIP_SIZE+4,my,sx,my,STD_MENU_GAP_COLOR);
	AddElemBehind(predek,mg);

	ResizeRel(0,1);
}

void CGUIMenu::DeleteGap(typeID where)
{
	CGUIMenuGap* mg = dynamic_cast<CGUIMenuGap*>(nameServer->ResolveID(where));
	if(!mg)
		return;

	float mx,my;

	mg->GetSize(mx,my);
	MoveItems(mg,-my,0);

	DeleteElem(mg);

	ResizeRel(0,-my);
}


int CGUIMenu::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(mouseState)
	{
		CGUIMenu* m = this;
		while(m->parentMenu!=0 && m->parentMenu!=desktop->GetFocusEl())
			m=m->parentMenu;
		m->Off();	// volam vypnuti menu na koreni menu => vypnou se i vsechny podmenu, takze vsechno
	}
	return 1;
}

int CGUIMenu::MouseRightFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(mouseState)
	{
		CGUIMenu* m = this;
		while(m->parentMenu!=0 && m->parentMenu!=desktop->GetFocusEl())
			m=m->parentMenu;
		m->Off();	// volam vypnuti menu na koreni menu => vypnou se i vsechny podmenu, takze vsechno
	}
	return 1;
}

int CGUIMenu::MouseMiddleFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(mouseState)
	{
		CGUIMenu* m = this;
		while(m->parentMenu!=0 && m->parentMenu!=desktop->GetFocusEl())
			m=m->parentMenu;
		m->Off();	// volam vypnuti menu na koreni menu => vypnou se i vsechny podmenu, takze vsechno
	}

	return 1;
}

int CGUIMenu::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	CGUIMenu* m = this;
	while(m->parentMenu!=0 && m->parentMenu!=desktop->GetFocusEl())
		m=m->parentMenu;
	m->Off();	// volam vypnuti menu na koreni menu => vypnou se i vsechny podmenu, takze vsechno
	return 1;
}

int CGUIMenu::FocusChanged(UINT focus, typeID dualID)
{
//	if(!focus)
//		Off();
	if(!focus)
		preclick=false;
	return 1;
}

int CGUIMenu::Keyboard(UINT character, UINT state)
{
	if(!active)
		return 0;

	CGUIMenuItem* mi;
	CGUIElement* el;
	el = GetFocusEl();
	int ret=1;

	if((character == VK_TAB || character == VK_DOWN) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(el && !el->exclusive_keyboard)
		{
			el = FindNextTABElement(el->GetTabOrder());
			if(el)
			{
				SetFocusEl(el);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}else if(!el){
			el = FindNextTABElement(0);
			if(el)
			{
				SetFocusEl(el);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}
	}

	if(((character == VK_TAB && mainGUI->key_shift) || character == VK_UP) && !mainGUI->key_ctrl && !mainGUI->key_alt)
	{
		if(el && !el->exclusive_keyboard)
		{
			el = FindPreviousTABElement(el->GetTabOrder());
			if(el)
			{
				SetFocusEl(el);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}else if(!el){
			el = FindPreviousTABElement(0);
			if(el)
			{
				SetFocusEl(el);
				ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
			}
		}
	}

	if(character == VK_RIGHT && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		mi=dynamic_cast<CGUIMenuItem*>(el);
		if(mi)
		{
			if(mi->subMenuIndex!=-1)
			{
				// aktivuj podmenu
				// !@#$
				ActivateSubMenu(subMenuArray.Get(mi->subMenuIndex),mi);
				CGUIMenu* sm = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(subMenuArray.Get(mi->subMenuIndex)));
				if(sm)
					sm->SetFocusEl(sm->FindNextTABElement(0));
			}

		}
	}

	if(character == VK_LEFT && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		
		if(parentMenu)
		{
			Off();
			desktop->SetFocusEl(parentMenu,1);
			//mi=dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(parentMenuActivatorItem));
			//if(mi)
				//parentMenu->SetFocusEl(mi);
			
		}

		/*
		mi=dynamic_cast<CGUIMenuItem*>(el);
		if(mi)
		{
			if(mi->subMenuIndex!=-1)
			{
				// deaktivuj podmenu (jen posledni pomenu v poradi !!!)
				// !@#$


			}

		}
		*/
	}

	if(character == VK_ESCAPE && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		Off();
		if(parentMenu)
		{
			desktop->SetFocusEl(parentMenu,1);
			//mi=dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(parentMenuActivatorItem));
			//if(mi)
				//parentMenu->SetFocusEl(mi);
			
		}
	}



	return ret;
}

void CGUIMenu::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EClicked)
	{
		CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(event->sender));
		if(mi)
		{
			if(mi->menuID==GetID())	// stiskunti polozky menu
			{
				MenuHandler(activator,event->sender,mi->index);
				EventArise(EMenu,mi->index,event->sender,0,0,0,activator);
				
				if(mi->subMenuIndex >= 0)
				{
					ActivateSubMenu(subMenuArray.Get(mi->subMenuIndex),mi);
				}else{
					// nekdo zmacknul polozku menu => vypni cele menu se vsemi jeho podmenu
					CGUIMenu* m = this;
					while(m->parentMenu!=0)
						m=m->parentMenu;
					m->Off();	// volam vypnuti menu na koreni menu => vypnou se i vsechny podmenu, takze vsechno
				}
			}else{
				On(event->sender);	// stisknuti polozky jineho menu, ktera je nastavena na aktivaci tohoto menu
			}
		}else{
			On(event->sender);	// stisknuti jineho typu elementu, ktery je nastaven na aktivaci menu
		}
	}

	if(event->eventID == EMouseButton)	// prisla udalost EMouseButton od automatickeho aktivatoru menu => aktivuj se
	{
		On(event->sender);
	}

	if(event->eventID == EMessage && event->pInt == MsgMouseR && (event->pID || preclick))
	{
		// prisla udalost EMessage od automatickeho aktivatoru menu => aktivuj se
		if(event->pID)
			preclick=true;
		else
			preclick=false;
		On(event->sender);
	}

	delete event;
}


void CGUIMenu::On(typeID _activator, float mx, float my)
{
	if(active)
		return;
	activator = _activator;
	active=true;

	if(mx==-1 || my==-1)
	{	// urceni pozice podle kurzoru
		float xx;
		float yy;
		mainGUI->cursor->GetPos(xx,yy);

		float dsx, dsy;
		desktop->GetSize(dsx,dsy);


		// menu nesmi vylezt mimo oblast desktopu
		if(yy + sy > dsy)
			yy -= sy-1;

		if(yy<0)
			yy=0;

		if(xx + sx > dsx)
			xx = dsx - sx;

		if(mx!=-1)
			xx=mx;
		if(my!=-1)
			yy=my;

		Move(xx,yy);
	}else{
		Move(mx,my);
	}

	BringToTop();
	SetVisible(1);
	desktop->SetFocusEl(this,1);

	EventArise(EOn);
}


void CGUIMenu::Off()
{
	if(!active)
		return;
	activator=0;
	active=false;

	Move(0,0);
	SetVisible(0);

	if(menuActivatorItem)	// menu obsahuje aktivovane subMenu
	{
		CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(menuActivatorItem));
		if(mi)	// mam aktivacni polozku subMenu
		{
			CGUIMenu* sm = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(subMenuArray.Get(mi->subMenuIndex)));
			if(sm)
			{
				sm->Off();	// vypni subMenu
			}
		}
		menuActivatorItem=0;
	}

	CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(GetFocusEl());
	if(mi)
	{
		if(mi->help)
			CGUIContextHelp::Off(mi);
	}
	SetFocusEl(0);

	EventArise(EOff);
}

/*
void CGUIMenu::SetSubMenuPrograms(CGUIMenu* subMenu)
{
	if(!subMenu)
		return;
	
	CGUIEventList* el = ::Get<CGUIEventList,int>(&eventList, EMenu);

	if(!el)
		return;	// zadne obsluzne programy pro tuto udalost nejsou objednany

	CGUIEventProgramList *p;
	CGUIListElement<CGUIEventProgramList>* le = el->eventProgs.GetNextListElement(0);
	while(le)
	{
		if((p=el->eventProgs.GetObjectFromListElement(le)))
		{
			subMenu->AcceptEvent(p->objID,EMenu);
		}
		le=el->eventProgs.GetNextListElement(le);
	}
}
*/

void CGUIMenu::ActivateSubMenu(typeID subMenuID, CGUIMenuItem* mi)
{
	CGUIMenu* sm = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(subMenuID));	// najdi subMenu
	if(!sm || !mi)
		return;

	if(menuActivatorItem && menuActivatorItem!=mi->GetID())	// menu ma jiz aktivovane jine subMenu => deaktivuj ho
	{
		CGUIMenuItem* ma = dynamic_cast<CGUIMenuItem*>(nameServer->ResolveID(menuActivatorItem));
		if(ma)
		{
			CGUIMenu* sma = dynamic_cast<CGUIMenu*>(nameServer->ResolveID(subMenuArray.Get(ma->subMenuIndex)));
			if(sma)
			{
				sma->Off();
			}
			menuActivatorItem=0;
		}
	}

	// zjisteni pozice pro subMenu:
	float smsx,smsy;
	sm->GetSize(smsx,smsy);
	float misx,misy;
	mi->GetSize(misx,misy);
	float mix=0,miy=0;
	desktop->GetDesktopPos(mi,mix,miy);
	float dsx, dsy;
	desktop->GetSize(dsx,dsy);

	// nastaveni aktivacni polozky + propojeni menu se subMenu
	sm->parentMenuActivatorItem=mi->GetID();
	menuActivatorItem=mi->GetID();

	if(miy+smsy>dsy)
		miy=dsy-smsy;

	// vlastni aktivace subMenu:
	if(mix+misx+smsx+2 > dsx)
		sm->On(activator,mix-smsx-2,miy);
	else
		sm->On(activator,mix+misx+2,miy);
}

void CGUIMenu::SetPictureStripSize(float _picture_strip_size)
{
	picture_strip_size=_picture_strip_size;
	Update();	// upravi polohu obrazku polozek atp.
}

void CGUIMenu::SetMenuPictureSize(float _menu_picture_size)
{
	menu_picture_size=_menu_picture_size;
	Update();	// upravi polohu obrazku polozek atp.
}

void CGUIMenu::Update()
{

	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIMenuItem* mi = dynamic_cast<CGUIMenuItem*>(e);
			if(mi)
				mi->SetPositionOfLabelPicture();
		}
		le=el_queue.GetNextListElement(le);
	}

	if(strip)
		strip->Resize(picture_strip_size,sy);
}

//////////////////////////////////////////////////////////////////////
// CGUIMenuMarker
//////////////////////////////////////////////////////////////////////

CGUIMenuMarker::CGUIMenuMarker(CBltWindow *rootwnd, float x1, float y1, float x2, float y2)
{
	UINT linesColor = STD_MENU_MARK_BORDER_COLOR;
	lines[0] = new CBltLine(x1,y1,x2,y1,linesColor);
	lines[1] = new CBltLine(x2,y1,x2,y2,linesColor);
	lines[2] = new CBltLine(x2,y2,x1,y2,linesColor);
	lines[3] = new CBltLine(x1,y2,x1,y1,linesColor);

	rect = new CBltRect(x1,y1,x2-x1,y2-y1,STD_MENU_MARK_BGCOLOR_COLOR);
	rootwnd->AddChild(rect);

	for(int i=0;i<4;i++)
		rootwnd->AddChild(lines[i]);
	visible=true;
}

CGUIMenuMarker::~CGUIMenuMarker()
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

void CGUIMenuMarker::Mark()
{
	if(rect)
		rect->SetVisible(1);
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(1);
	visible=true;
}

void CGUIMenuMarker::UnMark()
{
	if(rect)
		rect->SetVisible(0);
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(0);
	visible=false;
}

void CGUIMenuMarker::Rebuild(CGUIElement* elm)
{
	float x1,x2,y1,y2;

	elm->GetPos(x1,y1);
	elm->GetSize(x2,y2);

	x2 +=x1-2;
	y2 +=y1-2;
	x1++;
	y1++;

	lines[0]->SetPoints(x1,y1,x2,y1);
	lines[1]->SetPoints(x2,y1,x2,y2);
	lines[2]->SetPoints(x2,y2,x1,y2);
	lines[3]->SetPoints(x1,y2,x1,y1);

	rect->SetPos(x1,y1,x2-x1,y2-y1);
	//rect->Move(x1-xx,y1-yy);
	//rect->Resize(x2-x1,y2-y1);
}
