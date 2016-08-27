///////////////////////////////////////////////
//
// element.cpp
//
//	implementation for the CGUIPrimitiv & CGUIElement & CGUIWidget & CGUIWindow class
//		=> zakladni hierarchicka rada objektu GUI
//	CGUIPrimitiv - definuje zakladni vlastnosti (velikost, poloha, typ, viditelnost, ...)
//	CGUIElement - rozsiruje funkcnost predevsim o zpracovavani a reakce na zpravy
//	CGUIWidget - rozsiruje vlastnosti o ukotveni elementu v okne, zmena velikosti a polohy dle ukotveni
//			   - zajistuje kontrolu a dodrzovani mezi pro velikost a polohu elementu
//	CGUIWindow - zajistuje zakladni vlastnosti okna = pridavani a odebirani elementu (predni/zadni cast)
//			   - hledani elementu v okne dle polohy zpravy, forwarding zprav podelementum okna
//			   - focusovani elementu, viditelnost
//
//	implementace CGUIDataExchange
//		- objekt pro podporu predavani dat mezi elementy GUI a uzivatelskymi programy
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"
#include <exception>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "GUI.h"

#include "primitives.h"
#include "panel.h"

#include "menu.h"
#include "tree.h"

#include "dx.h"
#include "krkal.h"
//////////////////////////////////////////////////////////////////////
// CGUIElement
//////////////////////////////////////////////////////////////////////

CGUIElement::CGUIElement(float _x, float _y)
{
	SetPos(_x,_y);
	SetSize(0,0);
	SetVisibility(1);
	MsgSetConsume(MsgNone);
//	MsgSetProduce(MsgNone);	
	MsgSetProduce(MsgAll);	// !@#$
	parent=0;
	type = PTElement;
	list = 0;
	focusable=true;
	throw_going=false;
	back_element=false;
	tab_order = 0;
	mark=false;
	exclusive_keyboard = false;
	marker=0;
}

CGUIElement::CGUIElement(float _x, float _y, float _sx, float _sy)
{
	SetPos(_x,_y);
	SetSize(_sx,_sy);
	SetVisibility(1);
	MsgSetConsume(MsgNone);
//	MsgSetProduce(MsgNone);
	MsgSetProduce(MsgAll);	// !@#$
	parent=0;
	type = PTElement;
	list = 0;
	focusable=true;
	throw_going=false;
	back_element=false;
	tab_order = 0;
	mark=false;
	exclusive_keyboard = false;
	marker=0;
}

/*
CGUIElement::CGUIElement()
{
	x=0;
	y=0;
	sx=0;
	sy=0;
	visibility = 1;
	MsgSetConsume(MsgNone);
	MsgSetProduce(MsgNone);
	parent=0;
	type = PTElement;
}
*/

CGUIElement::~CGUIElement()
{
	//bool error = false;

	if(parent){ // !@#$ kdyz se rusi element, musi byt vyhozen z front elementu u sveho rodice
				// uz mel byt vyhozen drive (zde je jen jistotni pojistka)
		if(parent->type>=PTWindow){
			if(((CGUIWindow*)parent)->last_element_under_cursor==this)	
				((CGUIWindow*)parent)->last_element_under_cursor=0;	// odstraneni ruseneho elementu z ukazatele na posledni objekt pod kurzorem
			if(((CGUIWindow*)parent)->GetFocusEl() == this)	
				((CGUIWindow*)parent)->SetFocusEl(0,0);	// odstraneni ruseneho elementu z ukazatele na focusovany element
			((CGUIWindow*)parent)->RemoveBackElem(this);
			((CGUIWindow*)parent)->RemoveFrontElem(this);
			assert(false); // !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani z fronty rodice)
	//		error = true;
		}
		if(parent->type==PTMultiWidget){
			if(((CGUIMultiWidget*)parent)->last_element_under_cursor==this)	
				((CGUIMultiWidget*)parent)->last_element_under_cursor=0;	// odstraneni ruseneho elementu z ukazatele na posledni objekt pod kurzorem
			if(((CGUIMultiWidget*)parent)->GetFocusEl() == this)	
				((CGUIMultiWidget*)parent)->SetFocusEl(0);	// odstraneni ruseneho elementu z ukazatele na focusovany element
			((CGUIMultiWidget*)parent)->RemoveElem(this);
			assert(false); // !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani z fronty rodice)
	//		error = true;
		}

	}

	if(list){
		list->Remove(this);	// pokud je styl jeste zarazen v nejakem seznamu, odstran ho ze seznamu
							// spravne by ale mel jiz byt odstranen
		assert(false);		// !@#$ jen pro ladeni seznamu (kontrola spravneho odstranovani ze seznamu)
		//error = true;
	}

	msgQueue->DeleteAllMsgFromQueue(this);	// odstrani vsechny zpravy adresovane tomuto elementu

	//if(error && std::uncaught_exception())
	//	throw CExc(eGUI,E_INTERNAL, "CGUIElement::~CGUIElement> Incorrect work with list or parent");

	DeleteMarkStructures();	// odstraneni struktur pro oznaceni elementu
}

void CGUIElement::AddToTrash()
{
	if(!list && !parent)
	{	// element neni v seznamu, ani nema otce => neni v nicem => dam ho do trashe
		trash->Add(this);
	}
}




void CGUIElement::RemoveFromTrash()
{
	if(list && !parent)
	{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
		trash->Remove(this);
	}
}

void CGUIElement::Center(bool _x, bool _y)
{
	if(parent)
	{
		if(parent->GetType()>=PTWindow)
		{
			float wsx,wsy;
			float nx,ny;
			GetPos(nx,ny);
			CGUIWindow* w = (CGUIWindow*)parent;

			w->GetBackWindowSize(wsx,wsy);
			if(_x)
				nx=floorf((wsx-sx)/2);
			if(_y)
				ny=floorf((wsy-sy)/2);
			Move(nx,ny);
		}
	}
}

bool CGUIElement::IsFocused()
{
	if(parent)
	{
		if(parent->GetType()==PTMultiWidget)
			if(((CGUIMultiWidget*)parent)->GetFocusEl()==this)
				return true;
		if(parent->GetType()>=PTWindow)
			if(((CGUIWindow*)parent)->GetFocusEl()==this)
				return true;
	}

	return false;
}

void CGUIElement::SetTabOrder(float to)
{
	if(parent && to>0)
	{
		if(parent->GetType()==PTMultiWidget)
		{
			CGUIMultiWidget* mw = (CGUIMultiWidget*) parent;
			if((to>0 && mw->tab_min>to) || mw->tab_min==0)
				mw->tab_min=to;
			if(mw->tab_max<to)
				mw->tab_max=to;
		}
		if(parent->GetType()>=PTWindow)
		{
			CGUIWindow* w = (CGUIWindow*) parent;
			if(back_element)
			{
				if((to>0 && w->tab_min_back>to) || w->tab_min_back==0)
					w->tab_min_back=to;
				if(w->tab_max_back<to)
					w->tab_max_back=to;
			}else{
				if((to>0 && w->tab_min_front>to) || w->tab_min_front==0)
					w->tab_min_front=to;
				if(w->tab_max_front<to)
					w->tab_max_front=to;
			}
		}
	} 
	tab_order = to;
}

void CGUIElement::SetMark(bool _mark)
{
	mark = _mark;
}

void CGUIElement::MarkElement()
{
	if(!mark)
		return;		

	if(marker)
		marker->Mark();
	else{
		CreateMarkStructures(0);
		marker->Mark();
	}
}

void CGUIElement::UnMarkElement()
{
	if(!mark)
		return;		

	if(marker)
		marker->UnMark();

}

void CGUIElement::CreateMarkStructures(CBltWindow* win)
{
	if(!mark)
		return;
	if(!win && parent)
	{
		if(parent->GetType()>=PTWindow)
		{
			if(back_element)
				win = ((CGUIWindow*)parent)->back;
			else
				win = ((CGUIWindow*)parent)->front;
		}
		if(parent->GetType()==PTMultiWidget)
		{
			win = ((CGUIMultiWidget*)parent)->window;
		}
	}

	if(win)
	{
		marker = new CGUIStdMarker(win, this);
		((CGUIStdMarker*)marker)->OrderEvents(dynamic_cast<CGUIEventSupport*>(this));	// !@#$ objednam si udalosti o zmene polohy a velikosti oznacovaneho elementu, aby se mohl marker prizpusobit
	}
	UnMarkElement();
}

void CGUIElement::DeleteMarkStructures()
{
	SAFE_DELETE(marker);
}

//////////////////////////////////////////////////////////////////////
// CGUIPrimitive
//////////////////////////////////////////////////////////////////////


CGUIPrimitive::CGUIPrimitive()
{
	x = y = sx = sy = 0;
	visible = 1;
	type = PTPrimitive;
}

CGUIPrimitive::~CGUIPrimitive()
{
}

int CGUIPrimitive::IsIn(float _x, float _y)
{
	if (visible && (x <= _x && (x + sx) > _x) && (y <= _y && (y + sy) > _y))
		return IsInPrecise(_x-x,_y-y);	// preziznost se pocita uz ve vlasnim souradnem systemu (ne nadrazeneho objektu)

	return 0;
}

void CGUIPrimitive::InnerCoords(float &_x, float &_y)
{
	_x -= x;
	_y -= y;
}

int CGUIPrimitive::ResizeRelWithCheck(float &dx, float &dy)
{
	float old_sx=sx, old_sy=sy, new_sx=sx+dx, new_sy=sy+dy;
	int ret;
	ret =  ResizeWithCheck(new_sx,new_sy);
	dx = new_sx-old_sx;
	dy = new_sy-old_sy;
	return ret;
}

int CGUIPrimitive::MoveRelWithCheck(float &dx, float &dy)
{
	float old_x=x, old_y=y, new_x=x+dx, new_y=y+dy;
	int ret;
	ret =  MoveWithCheck(new_x,new_y);
	dx = new_x-old_x;
	dy = new_y-old_y;
	return ret;
}

int CGUIPrimitive::ResizeWithCheck(float &_sx, float &_sy)
{
	int ret=0;
	Resize(_sx,_sy);
	if(_sx!=sx || _sy!=sy)
		ret = 1;
	_sx=sx;
	_sy=sy;
	return ret;
}

int CGUIPrimitive::MoveWithCheck(float &_x, float &_y)
{
	int ret=0;
	Move(_x,_y);
	if(_x!=x || _y!=y)
		ret = 1;
	_x=x;
	_y=y;
	return ret;
}

void CGUIPrimitive::ResizeRel(float dx, float dy)
{
	Resize(sx+dx,sy+dy);
}

void CGUIPrimitive::MoveRel(float dx, float dy)
{
	Move(x+dx,y+dy);
}


//////////////////////////////////////////////////////////////////////
// CGUIWidget
//////////////////////////////////////////////////////////////////////

CGUIWidget::CGUIWidget(float _x, float _y)
	: CGUIElement(_x,_y)
{
	anchor_b = anchor_r = WIDGET_FREE;
	anchor_u = anchor_l = WIDGET_FIX;	// standardne je prvek fixovan horni a levou stranou okna
	full_window_anchor=false;
	type = PTWidget;
	SetLimitSizes();
	SetLimitPosition();
}


CGUIWidget::CGUIWidget(float _x, float _y, float _sx, float _sy)
	: CGUIElement(_x,_y,_sx,_sy)
{
	full_window_anchor=false;
	anchor_b = anchor_r = WIDGET_FREE;
	anchor_u = anchor_l = WIDGET_FIX;	// standardne je prvek fixovan horni a levou stranou okna
	type = PTWidget;
	SetLimitSizes();
	SetLimitPosition();
}


CGUIWidget::~CGUIWidget()
{

}

int CGUIWidget::ForwardMsg(CGUIMessage *msg)
{
	if(!focusable && parent)
	{
		if(parent->GetType()>=PTWindow)
			return ((CGUIWindow*)parent)->ForwardMsgToDeeperElement(msg,this);
		else if(parent->GetType()==PTMultiWidget)
			return ((CGUIMultiWidget*)parent)->ForwardMsgToDeeperElement(msg,this);
		else
			delete msg;
	}else{
		// !@#$ mozna, jen smazat msg ??? a nevyhazovat vyjimku (normalni stav)
		// jsem koncovy element => mel bych mit nastaveno, ze nic neprodukuji
		//							nebo jsem pruklepovy element, tedy nefocusovatelny
		// delete msg;
		throw CExc(eGUI, E_INTERNAL, "CGUIWidget::ForwardMsg> Widget is not able to ReForwardMsgs #%d (%d) [%d, %d]",
			msg->type, msg->param, msg->x, msg->y);
	}
	return 0;
}

void CGUIWidget::SetLimitSizes(float _wWidthMIN, float _wHeightMIN, float _wWidthMAX, float _wHeightMAX)
{
	if(_wWidthMAX && _wWidthMIN>_wWidthMAX)
		return;
	if(_wHeightMAX && _wHeightMIN>_wHeightMAX)
		return;

	wWidthMIN=_wWidthMIN;
	wHeightMIN=_wHeightMIN;
	wWidthMAX=_wWidthMAX;
	wHeightMAX=_wHeightMAX;

	float dsx=0, dsy=0;
	if(wWidthMAX && sx>wWidthMAX)
		dsx = wWidthMAX - sx;
	if(wHeightMAX && sy>wHeightMAX)
		dsy = wHeightMAX - sy;
	if(sx < wWidthMIN)
		dsx = wWidthMIN - sx;
	if(sy < wWidthMIN)
		dsy = wHeightMIN - sy;

	if(dsx || dsy)
		ResizeRel(dsx,dsy);
}

void CGUIWidget::SetLimitPosition(bool _min_position, bool _max_position, float _wMinX, float _wMinY, float _wMaxX, float _wMaxY)
{
	if(_min_position && _max_position && (_wMinX>_wMaxX || _wMinY>_wMaxY))
		return;
	min_position = _min_position;
	max_position = _max_position;
	wMinX = _wMinX;
	wMinY = _wMinY;
	wMaxX = _wMaxX;
	wMaxY = _wMaxY;

	float dx=0,dy=0;
	if(max_position && x>wMaxX)
		dx=wMaxX-x;
	if(max_position && y>wMaxY)
		dy=wMaxY-y;
	if(min_position && x<wMinX)
		dx=wMinX-x;
	if(min_position && y<wMinY)
		dy=wMinY-y;

	if(dx || dy)
		MoveRel(dx,dy);
}

void CGUIWidget::WindowSizeChanged(float dx, float dy)
{
	if(dy!=0)
	{		
		if(anchor_u == WIDGET_FIX && anchor_b == WIDGET_FIX)
			ResizeRel(0,dy);
		else if(anchor_b == WIDGET_FIX)
			MoveRel(0,dy);
	}
	if(dx!=0)
	{		
		if(anchor_l == WIDGET_FIX && anchor_r == WIDGET_FIX)
			ResizeRel(dx,0);
		else if(anchor_r == WIDGET_FIX)
			MoveRel(dx,0);
	}
}


int CGUIWidget::CheckResize(float &dx, float &dy)
{
	int ret=0;

	if(wWidthMAX && dx>wWidthMAX)
	{
		dx=wWidthMAX;
		ret=1;
	}
	if(wHeightMAX && dy>wHeightMAX)
	{
		dy=wHeightMAX;
		ret=1;
	}
	
	if(dx<wWidthMIN)
	{
		dx=wWidthMIN;
		ret=1;
	}
	if(dy<wHeightMIN)
	{
		dy=wHeightMIN;
		ret=1;
	}

	return ret;
}

int CGUIWidget::CheckPosition(float &px, float &py)
{
	int ret=0;
	if(min_position && px<wMinX)
	{
		px=wMinX;
		ret=1;
	}
	if(min_position && py<wMinY)
	{
		py=wMinY;
		ret=1;
	}
	if(max_position && px>wMaxX)
	{
		px=wMaxX;
		ret=1;
	}
	if(max_position && py>wMaxY)
	{
		py=wMaxY;
		ret=1;
	}

	return ret;
}

int CGUIWidget::CheckPositionRel(float &dx, float &dy)
{
	int ret=0;
	float px,py;
	GetPos(px,py);

	dx+=px;
	dy+=py;
	
	ret = CheckPosition(dx,dy);

	dx-=px;
	dy-=py;

	return ret;
}

int CGUIWidget::CheckResizeRel(float &dx, float &dy)
{
	int ret=0;
	float px,py;
	GetSize(px,py);

	dx+=px;
	dy+=py;
	
	ret = CheckResize(dx,dy);

	dx-=px;
	dy-=py;

	return ret;
}


//////////////////////////////////////////////////////////////////////
// CGUIMultiWidget
//////////////////////////////////////////////////////////////////////

const int CGUIMultiWidget::numEvents = 10;
const int CGUIMultiWidget::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,
														EVisibilityChanged,EMouseOver,EMouseButton,
														EAddElement,EDeleteAllElements,
														ESetAllElementsVisible,EMessage};


CGUIMultiWidget::CGUIMultiWidget(float _x, float _y, float _sx, float _sy)
	: CGUIWidget(_x,_y,_sx,_sy), CGUIEventSupport(availableEvents, numEvents)
{
	window = new CBltWindow(_x,_y,_sx,_sy);
	focus_elm=0;
	type = PTMultiWidget;
	last_element_under_cursor=0;
	tab_min = tab_max = 0;
	AddToTrash();
}

CGUIMultiWidget::CGUIMultiWidget(float _x, float _y)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	window = new CBltWindow(_x,_y,1,1);	// !@#$ velikost okna se musi pozdeji po vytvoreni upravit
										// stejne tak i velikost samotneho MultiWidgetu
	focus_elm=0;
	type = PTMultiWidget;
	last_element_under_cursor=0;
	tab_min = tab_max = 0;
	AddToTrash();
}

CGUIMultiWidget::~CGUIMultiWidget()
{
	RemoveFromTrash();

	CGUIElement *elm;
	while(elm=el_queue.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		delete elm;
	}

	if(window && window->GetTopChild())
	{
		//throw CExc(eGUI, E_INTERNAL,"CGUIMultiWidget::~CGUIMultiWidget> Engine window (CBltWindow) is not clean after deleting all elements");
		assert(false);
	}

	if(window)
	{
		CBltWindow *wnd = window->GetParent();
		if(wnd){
			wnd->DeleteChild(window);
		}else{
			delete window;
		}
		window=0;
	}
}


void CGUIMultiWidget::AddToEngine(CBltWindow *rootwnd)
{
	if(window && rootwnd)
		rootwnd->AddChild(window);
}

void CGUIMultiWidget::RemoveFromEngine()
{
	CBltWindow *wnd;
	if(window)
	{
		wnd = window->GetParent();
		if(wnd)
			wnd->RemoveChild(window);
	}
}

void CGUIMultiWidget::BringToTop()
{
	if(window)
		window->BringToTop();
}

void CGUIMultiWidget::BringElementToTop(CGUIElement* el)	// nastavi element ve fronte na vrsek (pri hledani ma prednost)
{
	if(el && el_queue.Remove(el))
		el_queue.AddToHead(el);
}


void CGUIMultiWidget::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	SetPos(_x,_y);
	CMatrix2D m;
	m.Translate(_x,_y);

	// !@#$ - rusim eventualni puvodni transformaci a nastavuji napevno jen posunuti
	// mohu tak prijit napr. o rotaci atp.
	if(window)
		window->SetWindowTrans(m);
}

/*
void CGUIMultiWidget::AddToEngine(CBltWindow *rootwnd)
{
	if(!rootwnd)
		return;

	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			e->AddToEngine(rootwnd);
		}
		le=el_queue.GetNextListElement(le);
	}
}

void CGUIMultiWidget::RemoveFromEngine()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			e->RemoveFromEngine();
		}
		le=el_queue.GetNextListElement(le);
	}
}

void CGUIMultiWidget::BringToTop()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			e->BringToTop();
		}
		le=el_queue.GetPrevListElement(le);
	}
}

void CGUIMultiWidget::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	SetPos(_x,_y);

	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			e->Move(_x,_y);
		}
		le=el_queue.GetNextListElement(le);
	}
}
*/

void CGUIMultiWidget::Resize(float _sx, float _sy)
{
	float dx, dy;

	CheckResize(_sx,_sy);
	GetSize(dx,dy);
	
	dx=_sx-dx;	// rozdily ve velikostech
	dy=_sy-dy;

	SetSize(_sx,_sy);

	if(window)
		window->SetWindowSize(_sx, _sy);
	
	// Resize vsech elementu(widgetu) 
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWidget)
				((CGUIWidget*)e)->WindowSizeChanged(dx,dy);
		}
		le=el_queue.GetNextListElement(le);
	}
}

void CGUIMultiWidget::SetVisible(int vis)
{
	SetVisibility(vis);

	if(window)
		window->SetVisible(vis);
}

void CGUIMultiWidget::SetAllElementVisible(int vis)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			e->SetVisible(vis);
		}
		le=el_queue.GetNextListElement(le);
	}
}

void CGUIMultiWidget::AddElem(CGUIElement *element)
{
	if(element && window){
		if(element->GetList() && !element->GetParent())
		{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
			trash->Remove(element);
		}
		el_queue.AddToHead(element);
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

CGUIElement *CGUIMultiWidget::RemoveElem(CGUIElement *element)
{
	
	CGUIElement *e = el_queue.Remove(element);
	if(e){
		e->DeleteMarkStructures();
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;
	}

	return e;
}

void CGUIMultiWidget::DeleteElem(CGUIElement *element)
{
	CGUIElement *e = el_queue.Remove(element);
	if(e){
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;

		delete e;
	}
}

void CGUIMultiWidget::DeleteAllElems(void)
{
	CGUIElement *elm;
	while(elm=el_queue.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		delete elm;
	}
	last_element_under_cursor=0;
	focus_elm=0;
}

bool CGUIMultiWidget::ContainsElement(CGUIElement *element)
{
	return el_queue.IsMember(element);
}


CGUIElement *CGUIMultiWidget::GetNextElement(CGUIElement *element)
{
	return el_queue.GetNext(element);
}


CGUIElement *CGUIMultiWidget::FindElement(float ex, float ey)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			if(!e->throw_going && e->IsIn(ex,ey))
				return e;
		}
		le=el_queue.GetNextListElement(le);
	}
	return 0;
}

CGUIElement *CGUIMultiWidget::FindDeeperElement(CGUIElement *fe, float ex, float ey)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetNextListElementObj(fe);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			if(!e->throw_going && e->IsIn(ex,ey))
				return e;
		}
		le=el_queue.GetNextListElement(le);
	}
	return 0;
}

CGUIElement *CGUIMultiWidget::GetFocusEl(void)				// vrati focusovany element 
{
	return focus_elm;
}

void CGUIMultiWidget::SetFocusEl(CGUIElement* el)	// nastavi focusovany element 
{		// puvodne focusovanemu elementu posle zpravu o ztrate focusu, novemu o ziskani focusu
	typeID id=0;
	CGUIMessage* msg_defocus=0;
	CGUIMessage* msg_focus=0;

	if(focus_elm && focus_elm!=el)
	{
		msg_defocus = new CGUIMessage(MsgFocusChange, id, (UINT)0, focus_elm);
		id = msg_defocus->mID;

		focus_elm->UnMarkElement();
	}

	if(el && el_queue.IsMember(el))
		focus_elm = el;
	else
		focus_elm = 0;

	if(focus_elm)
	{
		msg_focus = new CGUIMessage(MsgFocusChange, id, (UINT)1, focus_elm);
		id = msg_focus->mID;

		focus_elm->MarkElement();
	}else
		id = 0;

	if(msg_defocus)
	{
		msg_defocus->param = id;
		SendPriorityMsg(msg_defocus,msg_defocus->to);
	}

	if(msg_focus)
	{
		SendPriorityMsg(msg_focus,focus_elm);
	}

	if(focus_elm)
	{
		if(focus_elm->GetType()>=PTWindow && ((CGUIWindow *)focus_elm)->topOnFocus)
		{
			BringElementToTop(focus_elm);	// max. uroven klikatelnosti
			((CGUIWindow *)focus_elm)->BringToTop();		// pri zmene focusu okna se nastavuje i max. uroven viditelnosti
		}
	}
}

int CGUIMultiWidget::ForwardMsg(CGUIMessage *msg)
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

		if(!elem)
			EventArise(EMessage,msg->type,msg->param,msg->element,msg->x,msg->state,0,msg->y);

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

		if(refocus && msg->IsMouseButtonMsg() && msg->param && elem->focusable)	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem);	// nastavi element jako focusovany v okne
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

int CGUIMultiWidget::ForwardMsgToDeeperElement(CGUIMessage *msg, CGUIElement *fe)
{	// zprava se forwarduje od nefocusovatelneho elementu (pruklep elementu)
	// k nemu se dostala pres normalni forward, musela tedy byt uz poslana i focusovanemu elementu
	// nyni tedy uz neposilam focusovanemu elementu zadnou focus_zpravu
	CGUIElement *elem=0;
	
	if(msg->IsCoordMsg() && !msg->focus_msg)
	{// forwarduje jen zpravy urcene pozici kurzoru (element pod pruklepovym widgetem),
	 // zprava nesmi byt focusovana (mohlo by dochazet k nechtenemu nespravnemu doruceni zpravy, 
	 // krome toho focus zprava neni urcena elementu pod kurzorem)

		elem = FindDeeperElement(fe,msg->x,msg->y);
		// pokud je treba provede se transformace souradnic zpravy z front do back wnd.
		if(elem)	
		{									// nasel jsem element odpovidajici poloze zpravy, 
											// posilam jen jednu zpravu a to nalezenemu elementu
			SendMsg(msg,elem);
		}else{
			delete msg;						// nenalezel jsem element odpovidajici poloze zpravy
			return 1;
		}
			

		if(elem && msg->IsMouseButtonMsg() && msg->param && elem->focusable && elem !=GetFocusEl())	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem);	// nastavi element jako focusovany v okne
		}

	}else
		delete msg;

	return 1; 
}

CGUIElement* CGUIMultiWidget::FindNextTABElement(float to)
{
	CGUIElement *e;
	CGUIElement *toe = 0, *min_e = 0, *max_e = 0;
	CGUIListElement<CGUIElement>* le = 0;
	float tao;	// nejmensi vetsi nalezeny to
	float t;
	float min = 0,max = 0;

	if(tab_max==0)
		return 0;	// ve fronte neni zadny TAB element
	tao = tab_max;
	le = el_queue.GetNextListElement(0);
	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			t=e->GetTabOrder();
			if(t>0)
			{
				if(t>to && t<=tao)
				{
					tao=t;
					toe=e;
				}
				if(t>max)
				{
					max = t;
					max_e = e;
				}
				if(t<min || min == 0)
				{
					min = t;
					min_e = e;
				}

			}
		}
		le=el_queue.GetNextListElement(le);
	}

	if(tab_max > max)
		tab_max = max;
	if(tab_min < min || min == 0)
		tab_min = min;
	if(!toe)
		toe=min_e;
		
	return toe;
}

CGUIElement* CGUIMultiWidget::FindPreviousTABElement(float to)
{
	CGUIElement *e;
	CGUIElement *toe = 0, *min_e = 0, *max_e = 0;
	CGUIListElement<CGUIElement>* le = 0;
	float tao;	// nejvetsi mensi nalezeny to
	float t;
	float min = 0,max = 0;

	if(tab_min==0)
		return 0;	// ve fronte neni zadny TAB element
	tao = tab_min;
	le = el_queue.GetNextListElement(0);
	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			t=e->GetTabOrder();
			if(t>0)
			{
				if(t<to && t>=tao)
				{
					tao=t;
					toe=e;
				}
				if(t>max)
				{
					max = t;
					max_e = e;
				}
				if(t<min || min == 0)
				{
					min = t;
					min_e = e;
				}

			}
		}
		le=el_queue.GetNextListElement(le);
	}

	if(tab_max > max)
		tab_max = max;
	if(tab_min < min || min == 0)
		tab_min = min;
	if(!toe)
		toe=max_e;
		
	return toe;
}

//////////////////////////////////////////////////////////////////////
// CGUIWindow
//////////////////////////////////////////////////////////////////////

const int CGUIWindow::numEvents = 22;
const int CGUIWindow::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													EChangeTABElement};


CGUIWindow::CGUIWindow(float _x, float _y, float _sx, float _sy, int withBackground, int bgColor)
	: CGUIWidget(_x,_y,_sx,_sy), CGUIEventSupport(availableEvents, numEvents)
//	: CGUIElement(_x,_y,_sx,_sy)
{
	title=0;
	panel=0;
	onWindowPanel=true;
	windowState=WS_Standard;
	std_x=_x;std_y=_y;std_sx=_sx;std_sy=_sy;
	maximize_before_minimize=false;
	bw_size_normal_x=_sx;	bw_size_normal_y=_sy;	
	// normalni velikost back window je standardne nastavena na velikost materskeho okna (front win.)
	bw_size_max_x=0;	bw_size_max_y=0;	// max. velikost std. neomezena
	bw_size_x=bw_size_normal_x;	bw_size_y=bw_size_normal_y;		
	// aktualni velikost back window nastavena na standardni vel.
	bw_x=0; bw_y=0;		// poloha (pocatek) back window v souradne soustave okna (front window)
	vp_x=0; vp_y=0;		// poloha viewportu do back window (v souradnicich bw)
	vp_sx=_sx; vp_sy=_sy;		// velikost viewportu do back window 
	// nastavena na celou velikost bw (zaroven velikost materskeho okna)
	vp_min_sx=0; vp_min_sy=0;	// minimalni velikost view portu (neomezena)
	autogrowing = false;
	minimized_all=false;
	minimized=false;
	focus_window_before_minimize_all=0;
	tab_min_front = tab_max_front = 0;
	tab_min_back = tab_max_back = 0;
	oldTO=0;
	hold=false;
	hold_x=hold_y=0;
	old_cursor_x = old_cursor_y = 0;
	modal=0;
	cursorChanged=false;
	scrollChildWindowRather=false;

	topOnFocus=true;

	keyClose=true;
	keyMaximalize=true;
	escapeClose=false;
	excapeMinimize = false;

	background = 0;
	front = back = 0;

	back = new CBltWindow(bw_x,bw_y,bw_size_x,bw_size_y);
	if(withBackground)
	{
		background = new CBltRect(0,0,bw_size_x,bw_size_y,bgColor);
		back->AddChild(background);
	}
		
	front = new CBltWindow((float)_x,(float)_y,(float)_sx,(float)_sy);
	if(front && back)
	{
		front->AddChild(back);
	}

	SetVisibility(1);
	front_focus=0;
	focus_elm=0;
	type = PTWindow;
	last_element_under_cursor=0;
	MsgAcceptConsume(MsgCommand|MsgFocusChange|MsgMouseWD|MsgMouseWU|MsgKeyboard|MsgKeyboardChar|MsgKeyboardUp|MsgMouseL|MsgMouseOver|MsgMouseRelMove);
	AddToTrash();
}

CGUIWindow::CGUIWindow(float _x, float _y, float _sx, float _sy, CBltRect *_background)
	: CGUIWidget(_x,_y,_sx,_sy), CGUIEventSupport(availableEvents, numEvents)
//	: CGUIElement(_x,_y,_sx,_sy)
{
	title=0;
	panel=0;
	onWindowPanel=true;
	windowState=WS_Standard;
	std_x=_x;std_y=_y;std_sx=_sx;std_sy=_sy;
	maximize_before_minimize=false;
	bw_size_normal_x=_sx;	bw_size_normal_y=_sy;	
	// normalni velikost back window je standardne nastavena na velikost materskeho okna (front win.)
	bw_size_max_x=0;	bw_size_max_y=0;	// max. velikost std. neomezena
	bw_size_x=bw_size_normal_x;	bw_size_y=bw_size_normal_y;		
	// aktualni velikost back window nastavena na standardni vel.
	bw_x=0; bw_y=0;		// poloha (pocatek) back window v souradne soustave okna (front window)
	vp_x=0; vp_y=0;		// poloha viewportu do back window (v souradnicich bw)
	vp_sx=_sx; vp_sy=_sy;		// velikost viewportu do back window 
	// nastavena na celou velikost bw (zaroven velikost materskeho okna)
	vp_min_sx=0; vp_min_sy=0;	// minimalni velikost view portu (neomezena)
	autogrowing = false;
	minimized_all=false;
	minimized=false;
	focus_window_before_minimize_all=0;
	tab_min_front = tab_max_front = 0;
	tab_min_back = tab_max_back = 0;
	topOnFocus=true;
	oldTO=0;
	hold=false;
	hold_x=hold_y=0;
	old_cursor_x = old_cursor_y = 0;
	modal=0;
	cursorChanged=false;
	scrollChildWindowRather=false;

	keyClose=true;
	keyMaximalize=true;
	escapeClose=false;
	excapeMinimize = false;

	back = new CBltWindow(bw_x,bw_y,bw_size_x,bw_size_y);
	background = _background;
	if(back && background)
	{
		background->Resize(bw_size_x,bw_size_y);
		back->AddChild(background);
	}

	front = new CBltWindow((float)_x,(float)_y,(float)_sx,(float)_sy);
	if(front && back)
	{
		front->AddChild(back);
	}

	SetVisibility(1);
	front_focus=0;
	focus_elm=0;
	type = PTWindow;
	last_element_under_cursor=0;
	MsgAcceptConsume(MsgCommand|MsgFocusChange|MsgMouseWD|MsgMouseWU|MsgKeyboard|MsgKeyboardChar|MsgKeyboardUp|MsgMouseL|MsgMouseOver|MsgMouseRelMove);
	AddToTrash();
}

CGUIWindow::~CGUIWindow()
{
	RemoveFromTrash();

	if(cursorChanged)
	{
		cursorChanged=false;
		mainGUI->cursor->SetVisible(1);
	}

	focus_elm=0;
	last_element_under_cursor=0;

	CGUIElement *elm;
	while(elm=queue_back.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		delete elm;
	}
	while(elm=queue_front.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		delete elm;
	}

	if(back && background)
		back->DeleteChild(background);
	background=0;
	
	if(back->GetTopChild())
	{
		//throw CExc(eGUI, E_INTERNAL,"CGUIWindow::~CGUIWindow> Engine window (CBltWindow) - back is not clean after deleting all elements");
		assert(false);
	}

	if(back){
		CBltWindow *wnd = back->GetParent();
		if(wnd){
			wnd->DeleteChild(back);
		}else{
			delete back;
		}
		back=0;
	}

	if(front->GetTopChild())
	{
		//throw CExc(eGUI, E_INTERNAL,"CGUIWindow::~CGUIWindow> Engine window (CBltWindow) - front is not clean after deleting all elements");
		assert(false);
	}

	if(front){
		CBltWindow *wnd = front->GetParent();
		if(wnd){
			wnd->DeleteChild(front);
		}else{
			delete front;
		}
		front=0;
	}

	EventArise(EDeleted,0,0,this);
}

void CGUIWindow::AddToEngine(CBltWindow *rootwnd)
{
	CreateMarkStructures(rootwnd);
	if(!rootwnd)
		return;
	if(front)
		rootwnd->AddChild(front);
	// back je umisten uvnitr front jako podokno
}

void CGUIWindow::RemoveFromEngine()
{
	CBltWindow *wnd;
	if(front)
	{
		wnd = front->GetParent();
		if(wnd)
			wnd->RemoveChild(front);
	}
}

void CGUIWindow::BringToTop()
{
	if(front)
		front->BringToTop();
}

void CGUIWindow::Move(float x, float y)
{
	CheckPosition(x,y);
	SetPos(x,y);
	CMatrix2D m;
	m.Translate(x,y);

	// !@#$ - rusim eventualni puvodni transformaci a nastavuji napevno jen posunuti
	// mohu tak prijit napr. o rotaci atp.
	/*
	if(back)
		back->SetWindowTrans(m);
		*/
	if(front)
		front->SetWindowTrans(m);

	EventArise(EPositionChanged);
}

void CGUIWindow::ChangeViewPortBWSize(float &dx, float &dy)
{
	bool changeBWsize = false;
	float bw_dx=0, bw_dy=0;	// zmena velikosti bw

	if(vp_sx+dx < vp_min_sx)
		dx = vp_min_sx - vp_sx;		// konrola minimalni meze viewportu x
	if(vp_sy+dy < vp_min_sy)
		dy = vp_min_sy - vp_sy;		// konrola minimalni meze viewportu y

/*
	if(bw_size_max_x && bw_size_x+dx > bw_size_max_x)
		dx = bw_size_max_x - bw_size_x;	// konrola maximalni meze viewportu x (vp nesmi byt vetsi nez maximalni velikost back window)
	if(bw_size_max_y && bw_size_y+dy > bw_size_max_y)
		dy = bw_size_max_y - bw_size_y; // konrola maximalni meze viewportu y
*/
	if(bw_size_max_x && vp_sx+dx > bw_size_max_x)
		dx = bw_size_max_x - vp_sx;	// konrola maximalni meze viewportu x (vp nesmi byt vetsi nez maximalni velikost back window)
	if(bw_size_max_y && vp_sy+dy > bw_size_max_y)
		dy = bw_size_max_y - vp_sy; // konrola maximalni meze viewportu y
	// dx a dy uz je v povolenych mezich velikosti ViewPortu
	

	if(vp_sx <= bw_size_normal_x)
	{	// vp je mensi nez normalni velikost bw
		if(vp_x + vp_sx + dx <= bw_size_normal_x)
		{	// vse se vejde do bw
			vp_sx +=dx;
		}else{
			// vp se snazi vycuhovat z bw
			if(vp_sx + dx <= bw_size_normal_x)
			{	// vp se po posunuti pocatku vejde do bw
				vp_sx +=dx;
				
				//vp_x = bw_size_normal_x - vp_sx;
				SetWindowPositionVP(bw_size_normal_x - vp_sx - vp_x,0,false);

			}else{
				// vp se nevejde do bw => zvetsim bw nad normalni velikost (velikost vp = vel. bw)
				
				//vp_x=0;
				SetWindowPositionVP(-vp_x,0,false);
				
				vp_sx +=dx;
				bw_size_x = vp_sx;
				changeBWsize = true;	// zmenil jsem velikost bw => budu o tom muset informovat elementy v bw
				bw_dx = bw_size_x - bw_size_normal_x;
			}
		}
	}else{	// vp je vetsi nez normalni velikost bw
		if(vp_sx+dx >= bw_size_normal_x)
		{	// vp zustane vetsi nez normalni velikost bw
			bw_size_x +=dx;
			vp_sx +=dx;
			changeBWsize = true;
			bw_dx = dx;
		}else{
			// vp zmensi svoji velikost pod normalni velikost bw
			vp_sx +=dx;
			changeBWsize = true;
			bw_dx = bw_size_normal_x - bw_size_x;
			bw_size_x = bw_size_normal_x;
		}
	}

	if(vp_sy <= bw_size_normal_y)
	{	// vp je mensi nez normalni velikost bw
		if(vp_y + vp_sy + dy <= bw_size_normal_y)
		{	// vse se vejde do bw
			vp_sy +=dy;
		}else{
			// vp se snazi vycuhovat z bw
			if(vp_sy + dy <= bw_size_normal_y)
			{	// vp se po posunuti pocatku vejde do bw
				vp_sy +=dy;

				// vp_y = bw_size_normal_y - vp_sy;
				SetWindowPositionVP(0, bw_size_normal_y - vp_sy - vp_y,false);

			}else{
				// vp se nevejde do bw => zvetsim bw nad normalni velikost (velikost vp = vel. bw)
				
				// vp_y=0;
				SetWindowPositionVP(0, -vp_y,false);

				vp_sy +=dy;
				bw_size_y = vp_sy;
				changeBWsize = true;	// zmenil jsem velikost bw => budu o tom muset informovat elementy v bw
				bw_dy = bw_size_y - bw_size_normal_y;
			}
		}
	}else{	// vp je vetsi nez normalni velikost bw
		if(vp_sy+dy >= bw_size_normal_y)
		{	// vp zustane vetsi nez normalni velikost bw
			bw_size_y +=dy;
			vp_sy +=dy;
			bw_dy = dy;
		}else{
			// vp zmensi svoji velikost pod normalni velikost bw
			vp_sy +=dy;
			bw_dy = bw_size_normal_y - bw_size_y;
			bw_size_y = bw_size_normal_y;
		}
		changeBWsize = true;
	}


	if(back)
		back->SetWindowSize(vp_sx,vp_sy);		// zmena velikost vp se provadi zmenou velikosti back
	if(background && changeBWsize)
		background->Resize(bw_size_x,bw_size_y);	// background ma velikost back window
	
	if(changeBWsize)
	{
		// Resize vsech elementu(widgetu) okna (v bw) (zmenila se velikost bw)
		CGUIElement *e;
		CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				if(e->GetType() >= PTWidget)
					((CGUIWidget*)e)->WindowSizeChanged(bw_dx,bw_dy);
			}
			le=queue_back.GetNextListElement(le);
		}
	}

}

void CGUIWindow::SetBackWindowNormalSize(float nsx, float nsy)
{
/*	float dx, dy;
	dx = bw_size_normal_x - bw_size_x;
	dy = bw_size_normal_y - bw_size_y;
	
	ChangeViewPortBWSize(dx,dy);	// nastavi velikost bw na puvodni bw_normal_size (eventualne zmensi i vp)
	
	bw_size_normal_x = bw_size_x = nsx;
	bw_size_normal_y = bw_size_y = nsy;
	if(background)
		background->Resize(bw_size_x,bw_size_y);
	
	dx=-dx; dy=-dy;
	ChangeViewPortBWSize(dx,dy);	// zvetsi vp o tolik, kolik se pred chvili zmensil
	*/

//	float o_vp_sx, o_vp_sy;
//	o_vp_sx=vp_sx;
//	o_vp_sy=vp_sy;

	// kontrola prekroceni maximalni velikosti BW
	if(bw_size_max_x && nsx>bw_size_max_x)
		bw_size_max_x=nsx;
	if(bw_size_max_y && nsy>bw_size_max_y)
		bw_size_max_y=nsy;

	float ext_x, ext_y;
	ext_x = nsx - bw_size_normal_x;	// rozdil ve velikostech BWnormalSize (nova a stara)
	ext_y = nsy - bw_size_normal_y;
	if(ext_x<0)
		ext_x = 0;
	if(ext_y<0)
		ext_y=0;

	float dx, dy;
	dx = bw_size_normal_x - vp_sx;
	dy = bw_size_normal_y - vp_sy;

	float vdx, vdy;
	vdx = vp_x;
	vdy = vp_y;
	
	ChangeViewPortBWSize(dx,dy);	// nastavi velikost bw na puvodni bw_normal_size (eventualne zmensi i vp)
	
	if(bw_size_normal_x<nsx)
		bw_size_x=nsx;
	if(bw_size_normal_y<nsy)
		bw_size_y=nsy;

	bw_size_normal_x = nsx;
	bw_size_normal_y = nsy;
	
	if(background)
		background->Resize(bw_size_x,bw_size_y);
	
	dx=-dx; dy=-dy;
	ChangeViewPortBWSize(dx,dy);	// zvetsi vp o tolik, kolik se pred chvili zmensil

	vdx -= vp_x;
	vdy -= vp_y;
	SetWindowPositionVP(vdx,vdy);

	if(ext_x || ext_y)
	{
		// Resize vsech elementu(widgetu) okna (v bw) (zmenila se velikost bw)
		CGUIElement *e;
		CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				if(e->GetType() >= PTWidget && ((CGUIWidget*)e)->full_window_anchor)
					((CGUIWidget*)e)->WindowSizeChanged(ext_x,ext_y);
			}
			le=queue_back.GetNextListElement(le);
		}
	}

	// scrollbary se updatuji pres SetWindowPositionVP
}

void CGUIWindow::SetWindowSizeVP(float _vp_sx, float _vp_sy)
{
	// problem je, ze nevim jak jsou velke okraje atp. kolem BW, vim jen velikost okna a velikost VP
	// musim tedy zvetsovat relativne a to prinasi problem, ze pri zvetseni se muze zmenit velikost
	// viewportu (VP) napr. tim, ze zmizi scrollbary, to odstranuji naslednym druhym relativnim zvetsenim
	float dx,dy;
	dx = _vp_sx - vp_sx;
	dy = _vp_sy - vp_sy;
//	ChangeViewPortBWSize(dx,dy);
	if(dx || dy)
		ResizeRel(dx,dy);
	dx = _vp_sx - vp_sx;
	dy = _vp_sy - vp_sy;
	if(dx || dy)
		ResizeRel(dx,dy);

	dx = _vp_sx - vp_sx;
	dy = _vp_sy - vp_sy;
	if(dx || dy)
		ResizeRel(dx,dy);
}

void CGUIWindow::SetWindowSizeBWNormal()		
{
	// problem je, ze nevim jak jsou velke okraje atp. kolem BW, vim jen velikost okna a velikost VP
	// musim tedy zvetsovat relativne a to prinasi problem, ze pri zvetseni se muze zmenit velikost
	// viewportu (VP) napr. tim, ze zmizi scrollbary, to odstranuji naslednym druhym relativnim zvetsenim
	float dx,dy;
	dx = bw_size_normal_x - vp_sx;
	dy = bw_size_normal_y - vp_sy;
//	ChangeViewPortBWSize(dx,dy);
	if(dx || dy)
		ResizeRel(dx,dy);

	dx = bw_size_normal_x - vp_sx;
	dy = bw_size_normal_y - vp_sy;
	if(dx || dy)
		ResizeRel(dx,dy);
//	Resize(bw_size_normal_x,bw_size_normal_y);
}

void CGUIWindow::SetWindowPositionVP(float dx, float dy, bool updateScrollbars)
{
	if(vp_x + dx < 0)
		vp_x = 0;
	else if(vp_x + dx + vp_sx > bw_size_x)
		vp_x = bw_size_x - vp_sx;
	else
		vp_x += dx;

	if(vp_y + dy < 0)
		vp_y = 0;
	else if(vp_y + dy + vp_sy > bw_size_y)
		vp_y = bw_size_y - vp_sy;
	else
		vp_y += dy;

	if(back)
	{
		CMatrix2D tm;
		tm.Translate(-vp_x, -vp_y);
		back->SetInsideTrans(tm);
	}
	if(updateScrollbars)
		EventArise(EUpdateScrollbars);
}


void CGUIWindow::Resize(float _sx, float _sy)
{
	float dx, dy;

	CheckResize(_sx,_sy);
	GetSize(dx,dy);
	
	dx=_sx-dx;	// rozdily ve velikostech
	dy=_sy-dy;

	ChangeViewPortBWSize(dx,dy);	// zmeni patricne velikost ViewPortu (polohu vp),
									// eventualne i velikost bw
									// kontroluje meze velikosti + upravuje podle nich dx, dy
	SetSize(sx+dx,sy+dy);

	if(front)
		front->SetWindowSize(sx, sy);
	
	// Resize vsech elementu(widgetu) okna (v predni fronte)
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_front.GetNextListElement(0);

	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWidget)
				((CGUIWidget*)e)->WindowSizeChanged(dx,dy);
		}
		le=queue_front.GetNextListElement(le);
	}
	windowState=WS_Standard;
	EventArise(ESizeChanged);
}

void CGUIWindow::ResizeFrontOnly(float _sx, float _sy)
{
	float dx, dy;

	CheckResize(_sx,_sy);
	GetSize(dx,dy);
	
	dx=_sx-dx;	// rozdily ve velikostech
	dy=_sy-dy;

//	ChangeViewPortBWSize(dx,dy);	// zmeni patricne velikost ViewPortu (polohu vp),
									// eventualne i velikost bw
									// kontroluje meze velikosti + upravuje podle nich dx, dy
	SetSize(sx+dx,sy+dy);

	if(front)
		front->SetWindowSize(sx, sy);
	
	// Resize vsech elementu(widgetu) okna (v predni fronte)
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_front.GetNextListElement(0);

	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWidget)
				((CGUIWidget*)e)->WindowSizeChanged(dx,dy);
		}
		le=queue_front.GetNextListElement(le);
	}
}


void CGUIWindow::AddBackElem(CGUIElement *element)
{
	if(element && back){
		if(autogrowing)
		{
			float ex,ey,esx,esy;
			float cx=0,cy=0;
			bool change=false;

			element->GetPos(ex,ey);
			element->GetSize(esx,esy);

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

		if(element->GetList() && !element->GetParent())
		{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
			trash->Remove(element);
		}
		queue_back.AddToHead(element);
		element->SetParent(this);
		element->AddToEngine(back);
		element->back_element=true;

		// !@#$ kdyz davam novy element do okna roztazeneho nad svoji normalni veliost
		// musim element take roztahnout nad svoji velikost (pokud jsou patricne nastaveny kotvy)
		if(element->GetType() >= PTWidget)
					((CGUIWidget*)element)->WindowSizeChanged(bw_size_x - bw_size_normal_x,bw_size_y - bw_size_normal_y);


		if((element->GetTabOrder()>0 && tab_min_back>element->GetTabOrder()) || tab_min_back==0)
			tab_min_back=element->GetTabOrder();
		if(tab_max_back<element->GetTabOrder())
			tab_max_back=element->GetTabOrder();

		typeID id;
		CGUIEventProgram* ep = dynamic_cast<CGUIEventProgram*>(element);
		if(ep)
			id=ep->GetID();
		else
			id=0;

		minimized_all=false;

		//if(!focus_elm)
			//SetFocusEl(element,false);

		EventArise(EAddElement,0,id,element);
	}
}

CGUIElement *CGUIWindow::RemoveBackElem(CGUIElement *element)
{
	
	CGUIElement *e = queue_back.Remove(element);
	if(e){
		e->DeleteMarkStructures();
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;
	}

	return e;
}

bool CGUIWindow::ContainsBackElement(CGUIElement *element)
{
	return queue_back.IsMember(element);
}

bool CGUIWindow::ContainsFrontElement(CGUIElement *element)
{
	return queue_front.IsMember(element);
}


void CGUIWindow::DeleteBackElem(CGUIElement *element)
{
	CGUIElement *e = queue_back.Remove(element);
	if(e){
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;

		delete e;
	}
}

void CGUIWindow::DeleteAllBackElems(void)
{
	CGUIElement *elm;
	while(elm=queue_back.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		if(elm==last_element_under_cursor)
			last_element_under_cursor=0;
		delete elm;
	}
	if(!front_focus)
		focus_elm=0;
}

void CGUIWindow::DeleteAllFrontElems(void)
{
	CGUIElement *elm;
	while(elm=queue_front.RemoveFromHead())
	{
		elm->RemoveFromEngine();
		elm->SetParent(0);
		if(elm==last_element_under_cursor)
			last_element_under_cursor=0;
		delete elm;
	}
	if(front_focus)
		focus_elm=0;
}


CGUIElement *CGUIWindow::GetNextBackElement(CGUIElement *element)
{
	return queue_back.GetNext(element);
}


void CGUIWindow::AddFrontElem(CGUIElement *element)
{
	if(element && front){
		if(element->GetList() && !element->GetParent())
		{	// element je v seznamu, ale nema otce => je v trashi, odstran ho z trashe
			trash->Remove(element);
		}
		queue_front.AddToHead(element);
		element->SetParent(this);
		element->AddToEngine(front);
		element->back_element=false;

		if((element->GetTabOrder()>0 && tab_min_front>element->GetTabOrder()) || tab_min_front==0)
			tab_min_front=element->GetTabOrder();
		if(tab_max_front<element->GetTabOrder())
			tab_max_front=element->GetTabOrder();

		typeID id;
		CGUIEventProgram* ep = dynamic_cast<CGUIEventProgram*>(element);
		if(ep)
			id=ep->GetID();
		else
			id=0;

		minimized_all=false;
		EventArise(EAddElement,0,id,element);
	}
}

CGUIElement *CGUIWindow::RemoveFrontElem(CGUIElement *element)
{
	CGUIElement *e = queue_front.Remove(element);
	if(e){
		e->DeleteMarkStructures();
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;
	}

	return e;
}

void CGUIWindow::DeleteFrontElem(CGUIElement *element)
{
	CGUIElement *e = queue_front.Remove(element);
	if(e){
		e->RemoveFromEngine();
		e->SetParent(0);
		if(e==last_element_under_cursor)
			last_element_under_cursor=0;
		if(e==focus_elm)
			focus_elm=0;
		delete e;
	}
}

CGUIElement *CGUIWindow::GetNextFrontElement(CGUIElement *element)
{
	return queue_front.GetNext(element);
}

CGUIElement *CGUIWindow::FindElement(float ex, float ey, int *front)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_front.GetNextListElement(0);

	if(front)
		*front=1;
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(!e->throw_going && e->IsIn(ex,ey))
				return e;
		}
		le=queue_front.GetNextListElement(le);
	}

	if(TransformCoordsFrontToBack(ex,ey))	
	{	// transformuje mi souradnice do bw + otestuje zda jsem kliknul do oblasti viewportu
		if(front)
			*front=0;
		le = queue_back.GetNextListElement(0);
		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				if(!e->throw_going && e->IsIn(ex,ey))
				{
					return e;
				}
			}
			le=queue_back.GetNextListElement(le);
		}
	}
	return 0;
}

CGUIElement *CGUIWindow::FindDeeperElement(CGUIElement *fe, float &ex, float &ey, int *front)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_front.GetNextListElementObj(fe);

	if(front)
		*front=1;
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(!e->throw_going && e->IsIn(ex,ey))
				return e;
		}
		le=queue_front.GetNextListElement(le);
	}


	if(front)
		*front=0;

	if(queue_front.IsMember(fe))
	{
		le = queue_back.GetNextListElement(0);		// "fe" je clenem predni fronty, 
				//ale nebyl nalezen zadny naslednik v predni fronte, pokracuj od zacatku zadni fronty
		if(!TransformCoordsFrontToBack(ex,ey))	
			return 0;	// pokud jsou souradnice mimo vnitrni okno, nehledej element v zadni fronte
	}else
		le = queue_back.GetNextListElementObj(fe);	// "fe" je clenem zadni fronty, pokracuj jeho naslednikem


	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(!e->throw_going && e->IsIn(ex,ey))
				return e;
		}
		le=queue_back.GetNextListElement(le);
	}

	return 0;
}


CGUIElement *CGUIWindow::GetFocusEl(void)				// vrati focusovany element okna (i podle front_focusu)
{
	/*
	if(front_focus)
		return queue_front.GetFromHead();
	else
		return queue_back.GetFromHead();
		*/
	return focus_elm;
}

void CGUIWindow::SetFocusEl(CGUIElement* el, int _front_focus)	// nastavi focusovany element v predni/zadni fronte + front_focus
{		// puvodne focusovanemu elementu posle zpravu o ztrate focusu, novemu o ziskani focusu
	typeID id=0;
	CGUIMessage* msg_defocus=0;
	CGUIMessage* msg_focus=0;

	if(focus_elm && focus_elm->GetType()>=PTWindow && ((CGUIWindow*)focus_elm)->IsModal() && 
		!(el && el->GetType()>=PTWindow && ((CGUIWindow*)el)->IsModal()) &&
		!(dynamic_cast<CGUIMenu*>(el)))
		return;	// modalni okno lze prefocusovat jedine na jine modalni okno nebo menu


	if(focus_elm && !front_focus)
	{
		oldTO = focus_elm->GetTabOrder();
	}

	if(focus_elm && focus_elm!=el)
	{
		msg_defocus = new CGUIMessage(MsgFocusChange, id, (UINT)0, focus_elm);
		id = msg_defocus->mID;

		focus_elm->UnMarkElement();	// odoznaci puvodne oznaceny element
	}

	if(el)
	{
		if(_front_focus)
		{
			if(queue_front.IsMember(el))
				focus_elm = el;
			else
				focus_elm = 0;
		}else{
			if(queue_back.IsMember(el))
				focus_elm = el;		
			else
				focus_elm = 0;
		}
	}else
		focus_elm = 0;

	if(focus_elm)
	{
		msg_focus = new CGUIMessage(MsgFocusChange, id, (UINT)1, focus_elm);
		id = msg_focus->mID;

		focus_elm->MarkElement();	// oznac fokusovany element (predevsim kvuli oznacovani elementu pri prepinani TABem)
	}else
		id = 0;

	if(msg_defocus)
	{
		msg_defocus->param = id;
		SendPriorityMsg(msg_defocus,msg_defocus->to);
	}

	if(msg_focus)
	{
		SendPriorityMsg(msg_focus,focus_elm);
	}


	front_focus=_front_focus;

	if(focus_elm)
	{
		if(focus_elm->GetType()>=PTWindow && ((CGUIWindow *)focus_elm)->topOnFocus)
		{
			BringElementToTop(focus_elm);	// max. uroven klikatelnosti
			((CGUIWindow *)focus_elm)->BringToTop();		// pri zmene focusu okna se nastavuje i max. uroven viditelnosti
		}
	}
}

void CGUIWindow::BringElementToTop(CGUIElement* el)	// nastavi element v predni/zadni fronte na vrsek (pri hledani ma prednost)
{
	if(el)
	{
		if(queue_front.Remove(el))
		{
			queue_front.AddToHead(el);
		}else if(queue_back.Remove(el))
		{
			queue_back.AddToHead(el);
		}
	}
}

void CGUIWindow::SetVisible(int vis)
{
	SetVisibility(vis);

	if(front)
		front->SetVisible(vis);

}

void CGUIWindow::SetAllElementVisible(int vis)				// nastavi viditelnost vsech podelementu okna (viditelnost vlastniho okna necha nezmenenou)
{
	if(background)
		background->SetVisible(vis);

	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			e->SetVisible(vis);
		}
		le=queue_back.GetNextListElement(le);
	}

	le = queue_front.GetNextListElement(0);
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			e->SetVisible(vis);
		}
		le=queue_front.GetNextListElement(le);
	}
}

void CGUIWindow::AddBackground(CBltRect *_background)
{
	if(!back)
		return;

	if(background){
		back->DeleteChild(background);
		background = 0;
	}
	if(_background){
		back->AddChildToBottom(_background);
		background = _background;
	}
}

void CGUIWindow::RemoveBackground()
{
	if(back && background)
		back->DeleteChild(background);
	background=0;
}

bool CGUIWindow::TransformCoordsFrontToBack(float &tx, float &ty)
{
	//if((tx>=bw_x && tx<bw_x+bw_size_x) && (ty>=bw_y && ty<bw_y+bw_size_y))
	if((tx>=bw_x && tx<bw_x+vp_sx) && (ty>=bw_y && ty<bw_y+vp_sy))
	{	// souradnice padly do back wnd. pres viewport v materskem okne
		tx=tx-bw_x + vp_x;	// souradnice kliku do viewportu + poloha vp vuci bw
		ty=ty-bw_y + vp_y;	// souradnice kliku do viewportu + poloha vp vuci bw
		return true;
	}else{
		tx=tx-bw_x + vp_x;	// souradnice kliku do viewportu + poloha vp vuci bw
		ty=ty-bw_y + vp_y;	// souradnice kliku do viewportu + poloha vp vuci bw
		return false;
	}
}

bool CGUIWindow::TransformCoordsBackToFront(float &tx, float &ty)
{
	bool ret;

	if((tx>=vp_x && tx<vp_x+vp_sx) && (ty>=vp_y && ty<vp_y+vp_sy))
		ret=true;
	else
		ret=false;

	tx+=bw_x-vp_x;
	ty+=bw_y-vp_y;

	return ret;
}

int CGUIWindow::ForwardMsg(CGUIMessage *msg)
{
	CGUIElement *elem=0;
	CGUIMessage *focus_msg=0;
	bool refocus=false;		// zda se ma zmenit focus nebo ne
	int front;
	bool only_focus_msg=false;
	
	if(msg->IsCoordMsg())
	{
		float xx=msg->x, yy=msg->y;
		InnerCoords(xx,yy);			// prepocitani souradnic do souradne soustavy v okne (predtim je v sour. s. nadrazeneho okna)

		elem = FindElement(xx,yy,&front);

		if(focus_elm && focus_elm->GetType()>=PTWindow && ((CGUIWindow*)focus_elm)->IsModal() && elem!=focus_elm)
		{
			only_focus_msg=true;
		}

		///*
		if(!only_focus_msg)
		{
			if(msg->type!=MsgMouseWU &&  msg->type!=MsgMouseWD)
			{
			if(last_element_under_cursor && (elem != last_element_under_cursor )
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
			else	// !@#$ ????
				last_element_under_cursor=0;	// !@#$ ????
			}
		}
//*/

		if(!elem && !msg->focus_msg)
			EventArise(EMessage,msg->type,msg->param,msg->element,msg->x,msg->state,0,msg->y);

		if(elem && elem == GetFocusEl())	
		{									// nasel jsem element odpovidajici poloze zpravy, 
											// ten je identicky s focusovanym elementem okna
											// posilam jen jednu zpravu a to nalezenemu elementu
			if(!only_focus_msg)
			{
				SendIntoMsg(msg,elem);
				if(msg->IsMouseButtonMsg() && msg->param && elem->focusable && elem->GetType()>=PTWindow && ((CGUIWindow *)elem)->topOnFocus)
				{
					BringElementToTop(elem);	// z hlediska poradi klikatelnosti
					((CGUIWindow *)elem)->BringToTop();		// pri zmene focusu okna se nastavuje i max. uroven viditelnosti
				}
			}
			else
			{
				delete msg;
				return 1;
			}
			
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
			SendIntoMsg(focus_msg,GetFocusEl());	// neposilam prioritne, ale posilam drive nez normalni zpravu
			if(!only_focus_msg)
				SendIntoMsg(msg,elem);
			else{
				delete msg;
				return 1;
			}
			refocus=true;
		}else
		{									// oba elementy jsou nulove, nebo alespon jeden
											// posilam tedy max. 1 zpravu a to nenulovemu elementu
											// to je bud element odpovidajici poloze zpravy, nebo focusovany el.
			if(elem)
			{
				if(!only_focus_msg)
					SendIntoMsg(msg,elem);
				else{
					delete msg;
					return 1;
				}
				refocus=true;
			}else if(GetFocusEl())
			{
				bool foc_msg = msg->focus_msg;
				msg->focus_msg=true;	
				SendIntoPriorityMsg(msg,GetFocusEl());

				if(front_focus != front && msg->IsMouseButtonMsg() && msg->param && !foc_msg)
				{				
					CGUIElement* el;
								// na zadny element jsem nekliknul, ale kliknul jsem na jinou plochu nez je focusovana (predek/zadek okna)
					if(front)	// focusuji prvni element na plose, na kterou jsem kliknul
					{
						el = FindNextTABElement(0,true);
						if(!el)
							el = GetNextFrontElement(0);
						SetFocusEl(el,front);
					}else{
						FocusOldTOBackEl();
					}
				}
			}else{
				if(front_focus != front && msg->IsMouseButtonMsg() && msg->param && !msg->focus_msg)
				{				
					CGUIElement* el;
								// na zadny element jsem nekliknul, ale kliknul jsem na jinou plochu nez je focusovana (predek/zadek okna)
					if(front)	// focusuji prvni element na plose, na kterou jsem kliknul
					{
						el = FindNextTABElement(0,true);
						if(!el)
							el = GetNextFrontElement(0);
						SetFocusEl(el,front);
					}else{
						FocusOldTOBackEl();
					}
				}

				delete msg;		// nenalezel jsem element odpovidajici poloze zpravy ani neni zadny focusovany
				return 1;
			}
		}

		if(refocus && msg->IsMouseButtonMsg() && msg->param && elem->focusable && !only_focus_msg)	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem,front);	// nastavi element jako focusovany v okne
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

int CGUIWindow::ForwardMsgToDeeperElement(CGUIMessage *msg, CGUIElement *fe)
{	// zprava se forwarduje od nefocusovatelneho elementu (pruklep elementu)
	// k nemu se dostala pres normalni forward, musela tedy byt uz poslana i focusovanemu elementu
	// nyni tedy uz neposilam focusovanemu elementu zadnou focus_zpravu
	CGUIElement *elem=0;
	int front;
	
	if(msg->IsCoordMsg() && !msg->focus_msg)
	{// forwarduje jen zpravy urcene pozici kurzoru (element pod pruklepovym widgetem),
	 // zprava nesmi byt focusovana (mohlo by dochazet k nechtenemu nespravnemu doruceni zpravy, 
	 // krome toho focus zprava neni urcena elementu pod kurzorem)

		elem = FindDeeperElement(fe,msg->x,msg->y,&front);
		// pokud je treba provede se transformace souradnic zpravy z front do back wnd.
		if(elem)	
		{									// nasel jsem element odpovidajici poloze zpravy, 
											// posilam jen jednu zpravu a to nalezenemu elementu
			SendMsg(msg,elem);
		}else{

			if(front_focus != front && msg->IsMouseButtonMsg() && msg->param)
			{				// na zadny element jsem nekliknul, ale kliknul jsem na jinou plochu nez je focusovana (predek/zadek okna)
				if(front)	// focusuji prvni element na plose, na kterou jsem kliknul
					SetFocusEl(GetNextFrontElement(0),front);
				else
					SetFocusEl(GetNextBackElement(0),front);
			}

			delete msg;						// nenalezel jsem element odpovidajici poloze zpravy
			return 1;
		}
			

		if(elem && msg->IsMouseButtonMsg() && msg->param && elem->focusable && elem !=GetFocusEl())	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem,front);	// nastavi element jako focusovany v okne
		}

	}else
		delete msg;

	return 1; 
}

void CGUIWindow::SetWindowLimitSizesBW(float _wWidthMIN, float _wHeightMIN, float _wWidthMAX, float _wHeightMAX, bool min, bool max)
{
	if(min && max)
	{
		if(_wWidthMAX && _wWidthMIN>_wWidthMAX)
			return;
		if(_wHeightMAX && _wHeightMIN>_wHeightMAX)
			return;
	}

	float dsx, dsy;
	dsx = sx - vp_sx;
	dsy = sy - vp_sy;
	// !@#$ neni presne, ani nelze, zalezi na aktualnim stavu okna, zda je zobrazen scrollbar atp.
	// pro presne pouziti je lepsi nastavit SetBWLimitSizes

	if(min)
	{
		wWidthMIN=dsx+_wWidthMIN;
		wHeightMIN=dsy+_wHeightMIN;
	}

	if(max)
	{
		if(_wWidthMAX)
			wWidthMAX=dsx+_wWidthMAX;
		else
			wWidthMAX=0;
		if(_wHeightMAX)
			wHeightMAX=dsy+_wHeightMAX;
		else
			wHeightMAX=0;
	}

	dsx = 0; dsy = 0;
	if(wWidthMAX && sx>wWidthMAX)
		dsx = wWidthMAX - sx;
	if(wHeightMAX && sy>wHeightMAX)
		dsy = wHeightMAX - sy;
	if(sx < wWidthMIN)
		dsx = wWidthMIN - sx;
	if(sy < wWidthMIN)
		dsy = wHeightMIN - sy;

	if(dsx || dsy)
		ResizeRel(dsx,dsy);
}


void CGUIWindow::SetBWLimitSizes(float _wWidthMIN, float _wHeightMIN, float _wWidthMAX, float _wHeightMAX, bool min, bool max)
{
	if(min && max)
	{
		if(_wWidthMAX && _wWidthMIN>_wWidthMAX)
			return;
		if(_wHeightMAX && _wHeightMIN>_wHeightMAX)
			return;
	}

	if(max)
	{
		float nx,ny;
		nx = min(bw_size_normal_x,_wWidthMAX);
		ny = min(bw_size_normal_y,_wHeightMAX);
		if(nx!=bw_size_normal_x || ny!=bw_size_normal_y)	// normalni velikost BW je vetsi nez prave nastavovana max. velikost (zmensim normalni na maximalni)
			SetBackWindowNormalSize(nx,ny);
		bw_size_max_x=_wWidthMAX;
		bw_size_max_y=_wHeightMAX;
	}
	if(min)
	{
		vp_min_sx = _wWidthMIN;
		vp_min_sy = _wHeightMIN;
	}

	float dx=0, dy=0;
	if(bw_size_max_x && bw_size_max_x<bw_size_x)
		dx=bw_size_max_x-bw_size_x;
	if(bw_size_max_y && bw_size_max_y<bw_size_y)
		dy=bw_size_max_y-bw_size_y;
	if(vp_min_sx > vp_sx)
		dx = vp_min_sx - vp_sx;
	if(vp_min_sy > vp_sy)
		dy = vp_min_sy - vp_sy;

	if(dx || dy)
		ResizeRel(dx,dy);
}

int CGUIWindow::KeyboardChar(UINT character, UINT state)
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

int CGUIWindow::Keyboard(UINT character, UINT state)
{
	int ret = 1;
	CGUIElement* el;
	el = GetFocusEl();
	if(el && el->exclusive_keyboard)
		EventArise(EKeyboard,character,state,0,1,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	else
		EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	if(!(focus_elm && focus_elm->GetType()>=PTWindow && ((CGUIWindow*)focus_elm)->IsModal()))
	{
		if(character == VK_TAB && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
		{
			if(el && !el->exclusive_keyboard)
			{
				el = FindNextTABElement(el->GetTabOrder(),el->back_element ? false : true);
				if(el)
				{
					SetFocusEl(el,el->back_element ? 0 : 1);
					ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
				}
			}else if(!el){
				el = FindNextTABElement(0,false);
				if(el)
				{
					SetFocusEl(el,el->back_element ? 0 : 1);
					ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
				}
			}
		}

		if(character == VK_TAB && !mainGUI->key_ctrl && !mainGUI->key_alt && mainGUI->key_shift)
		{
			if(el && !el->exclusive_keyboard)
			{
				el = FindPreviousTABElement(el->GetTabOrder(),el->back_element ? false : true);
				if(el)
				{
					SetFocusEl(el,el->back_element ? 0 : 1);
					ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
				}
			}else if(!el){
				el = FindPreviousTABElement(0,false);
				if(el)
				{
					SetFocusEl(el,el->back_element ? 0 : 1);
					ret = 0;	// klavesa byla pouzita na prepnuti okna, dale jiz neposilat
				}
			}
		}
	}

	if(keyClose && character == VK_F5 && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret=0;
		CloseWindow();
	}

	if(escapeClose && character == VK_ESCAPE && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret=0;
		CloseWindow();
	}

	if (excapeMinimize  && character == VK_ESCAPE && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ret = 0;
		Minimize();
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
int CGUIWindow::KeyboardUp(UINT character, UINT state)
{
	CGUIElement* el;
	el = GetFocusEl();
	if(el && el->exclusive_keyboard)
		EventArise(EKeyboardUp,character,state,0,1,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	else
		EventArise(EKeyboardUp,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);

	return 1;
}


int CGUIWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	//if(mouseState && mainGUI->input->IsKeyDown(DIK_SPACE)) 
	if(mouseState && (keyState & MK_CONTROL)) 
	{			// po stisknuti tlacitka zacinam menit pozici okna (zaznamenavam zmenu souradnic kurzoru)
		if(scrollChildWindowRather)
		{
			InnerCoords(x,y);

			CGUIElement* elem = FindElement(x,y,0);
			if(elem && elem->GetType()>=PTWindow)
			{
				float nsx,nsy,vpsx,vpsy;
				CGUIWindow* win = (CGUIWindow*)elem;
				win->GetBackWindowNormalSize(nsx,nsy);
				win->GetVPSize(vpsx,vpsy);

				if(vpsx<nsx || vpsy<nsy)
					return 1;	// nasel jsem scrollovatelne okno pod kurzorem => nescrolluj s materskym oknem
			}
		}

		if(vp_sx>=bw_size_normal_x && vp_sy>=bw_size_normal_y)
			return 1;	// materske okno nescroluuje, posli udalost synovskym elementum

		hold=true;
		hold_x=x;
		hold_y=y;
		mainGUI->cursor->GetPos(old_cursor_x,old_cursor_y);
		mainGUI->cursor->SetVisible(0);
		cursorChanged=true;
		mainGUI->cursor->Move(-10,-10);
		mainGUI->SendCursorPos();
		KRKAL->SetCapture();
		return 0;
	}else if(hold){	// po pusteni tlacitka dokoncim zmenu polohy okna
		/*
		if(STD_WINDOW_HOLDER_SCROLLING)
			;//SetWindowPositionVP(hold_x-x,hold_y-y);
		else
			;//SetWindowPositionVP(x-hold_x,y-hold_y);
		*/
		hold=false;
		hold_x=hold_y=0;
		mainGUI->cursor->SetVisible(1);
		cursorChanged=false;
		mainGUI->mouseOverBlock=false;
		KRKAL->SetCursorPos((int)old_cursor_x,(int)old_cursor_y);
		KRKAL->ReleaseCapture();
		return 0;
	}
	return 1;
}

int CGUIWindow::MouseOver(float x, float y, UINT over,UINT state)
{
	return 1;
}

int CGUIWindow::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(hold)
	{
		/*
		if(STD_WINDOW_HOLDER_SCROLLING)
			;//SetWindowPositionVP(hold_x-x,hold_y-y);
		else
			;//SetWindowPositionVP(x-hold_x,y-hold_y);
		*/
		mainGUI->cursor->SetVisible(1);
		cursorChanged=false;
		mainGUI->mouseOverBlock=false;
		KRKAL->SetCursorPos((int)old_cursor_x,(int)old_cursor_y);
		KRKAL->ReleaseCapture();
		hold=false;
		hold_x=hold_y=0;
		return 0;
	}
	return 1;
}

int CGUIWindow::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	if(hold)
	{
		mainGUI->mouseOverBlock=true;
		//return 0;
	}
	return 1;
}

int CGUIWindow::MouseRelativeMove(float dx, float dy)
{
	if(hold)
	{
		if(STD_WINDOW_HOLDER_SCROLLING)
		{
			SetWindowPositionVP(floorf(-dx*STD_WINDOW_SCROLLING_BOOSTER),floorf(-dy*STD_WINDOW_SCROLLING_BOOSTER));
		}else{
			SetWindowPositionVP(floorf(dx*STD_WINDOW_SCROLLING_BOOSTER),floorf(dy*STD_WINDOW_SCROLLING_BOOSTER));
		}
		return 0;
	}

	return 1;
}

int CGUIWindow::Wheel(float x, float y, int dw, int dz, UINT state)
{
	if(scrollChildWindowRather)
	{
		InnerCoords(x,y);

		CGUIElement* elem = FindElement(x,y,0);
		if(elem && elem->GetType()>=PTWindow)
		{
			float nsx,nsy,vpsx,vpsy;
			CGUIWindow* win = (CGUIWindow*)elem;
			win->GetBackWindowNormalSize(nsx,nsy);
			win->GetVPSize(vpsx,vpsy);

			if(vpsx<nsx || vpsy<nsy)
				return 1;	// nasel jsem scrollovatelne okno pod kurzorem => nescrolluj s materskym oknem
		}
	}

	EventArise(EMouseWheel,dz);

	if(vp_sx>=bw_size_normal_x && vp_sy>=bw_size_normal_y)
		return 1;	// materske okno nescroluuje, posli udalost synovskym elementum
	else
		return 0;	// materske okno scrolluje, nic dal neposilej
}

int CGUIWindow::Command(typeID command, CGUIMessage *msg)
{
	switch(command)
	{
		case ComCloseWnd:
			// rusim sebe => provedu konsolidaci dat a pres CloseWindow poslu zpravu otci at me zrusi
			CloseWindow();
			break;
		case ComCloseElement:
			// rusim svuj podelement (nevim zda je predni/zadni)
			// !@#$ mam provest jeho konsolidaci dat, nebo uz je provedena ? 
			DeleteFrontElem(msg->element);
			DeleteBackElem(msg->element);
			break;
		default:
			throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIWindow::Command> Unknow command #%d", command);
	}
	return 0;
}

/*
int CGUIWindow::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	if(mouseState && (keyState & MK_CONTROL))
	{			// po stisknuti tlacitka zacinam menit pozici okna (zaznamenavam zmenu souradnic kurzoru)
		hold=true;
		hold_x=x+vp_x;
		hold_y=y+vp_y;
		return 0;
	}else if(hold && (keyState & MK_CONTROL)){	// po pusteni tlacitka dokoncim zmenu polohy okna
		float vx, vy;
		GetVPPos(vx,vy);
		if(STD_WINDOW_HOLDER_SCROLLING)
			SetWindowPositionVP(hold_x-x-vx,hold_y-y-vy);
		else
			SetWindowPositionVP(x-hold_x,y-hold_y);
		hold=false;
		hold_x=hold_y=0;
		return 0;
	}
	return 1;
}

int CGUIWindow::MouseOver(float x, float y, UINT over,UINT state)
{
	if(!(state & MK_LBUTTON) || !(state & MK_CONTROL))
		hold=false;
	
	if(hold)
	{
		float vx, vy;
		GetVPPos(vx,vy);
		if(STD_WINDOW_HOLDER_SCROLLING)
		{
			SetWindowPositionVP(hold_x-x-vx,hold_y-y-vy);
			//hold_x=vx-vp_x;
			//hold_y=vy-vp_y;
		}else{
			SetWindowPositionVP(x-hold_x,y-hold_y);
			hold_x=vp_x-x;
			hold_y=vp_y-y;
		}
		//hold_x=x;
		//hold_y=y;
		return 0;
	}
	return 1;
}

int CGUIWindow::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	return 1;
}

int CGUIWindow::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	if(hold && (state & MK_LBUTTON) && (state & MK_CONTROL))
	{		// tlacitko je zmacknute a byla zapocata zmena polohy okna => pokracuj  
		float vx,vy;
		GetVPPos(vx,vy);
		if(STD_WINDOW_HOLDER_SCROLLING)
		{
			SetWindowPositionVP(hold_x-x-vx,hold_y-y-vy);
			//hold_x=vx-vp_x;
			//hold_y=vy-vp_y;
		}else{
			SetWindowPositionVP(vx+x-hold_x,vy+y-hold_y);
		}
		//hold_x=x;
		//hold_y=y;
		return 0;
	}else{
		hold=false;
		hold_x=hold_y=0;
	}
	return 1;
}


int CGUIWindow::Command(typeID command, CGUIMessage *msg)
{
	switch(command)
	{
		case ComCloseWnd:
			// rusim sebe => provedu konsolidaci dat a pres CloseWindow poslu zpravu otci at me zrusi
			CloseWindow();
			break;
		case ComCloseElement:
			// rusim svuj podelement (nevim zda je predni/zadni)
			// !@#$ mam provest jeho konsolidaci dat, nebo uz je provedena ? 
			DeleteFrontElem(msg->element);
			DeleteBackElem(msg->element);
			break;
		default:
			throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIWindow::Command> Unknow command #%d", command);
	}
	return 0;
}
*/

void CGUIWindow::ParentNotification()
{
	if(parent)
	{
		CGUITree *t = dynamic_cast<CGUITree*>(parent);
		if(t)
			AcceptEvent(t->GetID(),EChangeTABElement);
	}
}

void CGUIWindow::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EKillMe)
	{
		CGUIElement* el = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->sender));
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
			CGUIElement* oldEl=el;
			if(event->pInt==1)
				el=FindNextTABElement(el->GetTabOrder(),false);		
			else
				el=FindPreviousTABElement(el->GetTabOrder(),false);	
			
			bool treeTABChange = false;
			if(CGUITree* tree = dynamic_cast<CGUITree*>(parent))
			{
				// muj otec je strom
				// pokud dojedu na konec svych tab_elementu poslu stromu udalost na prepnuti na dalsi polozku
				float tt = (event->pInt==1) ? tab_min_back : tab_max_back;
				if(!el)
				{
					treeTABChange=true;
					EventArise(EChangeTABElement,event->pInt);
				}
				else if(el && el->GetTabOrder() == tt && !(tab_min_back==tab_max_back && oldEl!=el))	// prejel jsem koncovy tab_element
				{
					treeTABChange=true;
					EventArise(EChangeTABElement,event->pInt);
				}
			}

			if(el && !treeTABChange)
			{
				SetFocusEl(el,el->back_element ? 0 : 1);
			}
		}
	}

	delete event;
}

int CGUIWindow::CloseWindow()
{
	// konsolidace:
	if(modal)
		SetModal(0);
	if(parent && parent->GetType() >= PTWindow)
		((CGUIWindow*)parent)->RefocusToNextWindow(this);	// zmena focusu okna

	EventArise(EClose,0,0,this);

	// poslani zpravy otci, at me znici:
	if(parent)
	{
		//EventArise(EKillMe);
		if(parent->GetType() >= PTWindow)
		{
			// poslani udalosti rodici okna, aby me znicil
			CGUIEvent *evt = new CGUIEvent();
			evt->eventID = EKillMe;
			evt->sender = GetID();
			evt->recipient = ((CGUIWindow*)parent)->GetID();
			eventServer->AddEventToQueue(evt);
		}

	}else{
		// !@#$ ????????? - nemam otce => jsem desktop / okno nevlozene do enginu
		// ukoncit GUI / aplikaci ????
	}

	return 0;
}

void CGUIWindow::SyncAll(bool out)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			CGUIDataExchange* d = dynamic_cast<CGUIDataExchange*>(e);
			if(d)
				d->Sync(out);
		}
		le=queue_back.GetNextListElement(le);
	}

	le = queue_front.GetNextListElement(0);
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			CGUIDataExchange* d = dynamic_cast<CGUIDataExchange*>(e);
			if(d)
				d->Sync(out);
		}
		le=queue_front.GetNextListElement(le);
	}
}

void CGUIWindow::RefocusToNextWindow(CGUIWindow* wnd)
{
	int front=1;

	if(wnd)
	{
		if(queue_back.IsMember(wnd))
		{
			CGUIElement *e;
			CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

			while(le)
			{
				if((e=queue_back.GetObjectFromListElement(le)))
				{
					if(e!=wnd && e->IsVisible() && e->GetType()>=PTWindow && ((CGUIWindow*)e)->windowState!=WS_Minimized && ((CGUIWindow*)e)->onWindowPanel)
					{
						SetFocusEl(e,0);
						return;
					}
				}
				le=queue_back.GetNextListElement(le);
			}
			front=0;
		}

		if(queue_front.IsMember(wnd))
		{
			CGUIElement *e;
			CGUIListElement<CGUIElement>* le = queue_front.GetNextListElement(0);
			while(le)
			{
				if((e=queue_front.GetObjectFromListElement(le)))
				{
					if(e!=wnd && e->IsVisible() && e->GetType()>=PTWindow && ((CGUIWindow*)e)->windowState!=WS_Minimized && ((CGUIWindow*)e)->onWindowPanel)
					{
						SetFocusEl(e,1);
						return;
					}
				}
				le=queue_front.GetNextListElement(le);
			}
			front=1;
		}

	}else{	// wnd==0
		CGUIElement *e;
		CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				if(e->IsVisible() && e->GetType()>=PTWindow && ((CGUIWindow*)e)->windowState!=WS_Minimized && ((CGUIWindow*)e)->onWindowPanel)
				{
					SetFocusEl(e,0);
					return;
				}
			}
			le=queue_back.GetNextListElement(le);
		}

		le = queue_front.GetNextListElement(0);
		while(le)
		{
			if((e=queue_front.GetObjectFromListElement(le)))
			{
				if(e->IsVisible() && e->GetType()>=PTWindow && ((CGUIWindow*)e)->windowState!=WS_Minimized && ((CGUIWindow*)e)->onWindowPanel)
				{
					SetFocusEl(e,1);
					return;
				}
			}
			le=queue_front.GetNextListElement(le);
		}
	}

	CGUIElement* el;
	if(wnd)
	{
		float to = wnd->GetTabOrder();
		if(queue_front.IsMember(wnd))
		{
			el = FindNextTABElement(to,true);
			if(el)
				SetFocusEl(el,1);
		}else{
			el = FindNextTABElement(to,false);
			if(el)
				SetFocusEl(el,0);
		}
	}else{
		el = FindNextTABElement(0,0);
		if(el)
			SetFocusEl(el,0);
		else
			SetFocusEl(0,front);	// pokud jsem nenalezl zadne okna na ktere bych mohl prefocusovat,
									// tak nefocusuji na nic
	}
}

typeID CGUIWindow::GetNextWindowID(typeID windowID)
{
	CGUIWindow* wnd = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(windowID));
	if(wnd)
	{
		if(queue_back.IsMember(wnd))
		{
			CGUIElement *e;
			CGUIListElement<CGUIElement>* le = queue_back.GetNextListElementObj(wnd);

			while(le)
			{
				if((e=queue_back.GetObjectFromListElement(le)))
				{
					if(e->GetType()>=PTWindow)
					{
						return ((CGUIWindow*)e)->GetID();
					}
				}
				le=queue_back.GetNextListElement(le);
			}
		}

		if(queue_front.IsMember(wnd))
		{
			CGUIElement *e;
			CGUIListElement<CGUIElement>* le = queue_front.GetNextListElementObj(wnd);
			while(le)
			{
				if((e=queue_front.GetObjectFromListElement(le)))
				{
					if(e->GetType()>=PTWindow)
					{
						return ((CGUIWindow*)e)->GetID();
					}
				}
				le=queue_front.GetNextListElement(le);
			}
		}

	}else{	// wnd==0
		CGUIElement *e;
		CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);

		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				if(e->GetType()>=PTWindow)
				{
					return ((CGUIWindow*)e)->GetID();
				}
			}
			le=queue_back.GetNextListElement(le);
		}

		le = queue_front.GetNextListElement(0);
		while(le)
		{
			if((e=queue_front.GetObjectFromListElement(le)))
			{
				if(e->GetType()>=PTWindow)
				{
					return ((CGUIWindow*)e)->GetID();
				}
			}
			le=queue_front.GetNextListElement(le);
		}
	}
	return 0;
}

int CGUIWindow::FocusChanged(UINT focus, typeID dualID)
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
		CGUIElement* el=0;
		el = FindNextTABElement(0,false);
		if(!el)
			el = queue_back.GetFromHead();
		if(el)
		{
			SetFocusEl(el,0);
		}
	}

	EventArise(EFocusChanged,focus);
	return 1;
}

void CGUIWindow::FocusMe()
{
	if(parent && parent->GetType() >= PTWindow)
	{
		((CGUIWindow*)parent)->SetFocusEl(this,back_element ? 0 : 1);
	}else if(topOnFocus)
		BringToTop();	
}

void CGUIWindow::Minimize()
{
	if(modal && parent && parent->GetType()>=PTWindow && ((CGUIWindow*)parent)->GetFocusEl()==this)
		return;

	SetVisible(0);

	if(parent && parent->GetType() >= PTWindow)
		((CGUIWindow*)parent)->RefocusToNextWindow(this);

	if(windowState==WS_Maximized)
		maximize_before_minimize=true;
	else
		maximize_before_minimize=false;
	windowState=WS_Minimized;
	EventArise(EMinimize,1);
}

void CGUIWindow::UnMinimize()
{
	SetVisible(1);

	if(parent && parent->GetType() >= PTWindow)
	{
		((CGUIWindow*)parent)->minimized_all=false;
		((CGUIWindow*)parent)->SetFocusEl(this,back_element ? 0 : 1);
	}else
		BringToTop();	

	if(maximize_before_minimize)
		windowState=WS_Maximized;
	else
		windowState=WS_Standard;
	EventArise(EMinimize,0);
}

void CGUIWindow::Maximize()
{
	float msx, msy;
	float psx, psy;
	float dsx=0, dsy=0;
	float div_panel = 0;
	GetPos(std_x,std_y);
	GetSize(std_sx,std_sy);
	if(parent && parent->GetType() >= PTWindow)
	{
		CGUIWindow* wp = (CGUIWindow*)parent;
		//wp->GetSize(msx,msy);
		wp->GetVPSize(msx,msy);
		/*
		if(wp->panel)
			wp->panel->GetSize(psx,psy);
		else{
			psx=0;psy=0;
		}
		*/
		//if(wp != desktop && wp->panel)
		if(wp->panel && wp->panel->back_element)
		{
			if(wp->panel->position_up && wp->vp_y<STD_PANEL_HEIGHT)
			{	
				msy-= div_panel = STD_PANEL_HEIGHT-wp->vp_y;
			}

			if(!wp->panel->position_up && wp->vp_y > wp->bw_size_y-wp->vp_sy-STD_PANEL_HEIGHT)
			{
				msy -= div_panel = wp->vp_y - (wp->bw_size_y-wp->vp_sy-STD_PANEL_HEIGHT);
			}
			//msy-=STD_PANEL_HEIGHT;
		}
		psx = msx; psy = msy;
		CheckResize(msx,msy);

		dsx = sx - vp_sx;	// rozdil ve velikosti vnitrku od vnejsku okna
		dsy = sy - vp_sy;

		//if(wp == desktop || !wp->panel || (wp->panel && !wp->panel->position_up))
		if((wp->panel && !wp->panel->back_element) || !wp->panel || (wp->panel && !wp->panel->position_up))
		{
			if((x+msx<=psx && y+msy<=psy) || 
				(bw_size_max_x && bw_size_max_y && x+bw_size_max_x+dsx<=psx && y+bw_size_max_y+dsy<=psy))	// pripad nastavene max. velikosti bw_size (ta nasledne omezi zvetseni okna)
			{
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
				Resize(msx,msy);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,(y+msy<=wp->vp_y+psy) ? y : wp->vp_y+psy-msy);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
				Resize(msx,msy);
			}else{
				if(msx<=psx)
					Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,wp->vp_y);
				else if(msy<=psy)
					Move(wp->vp_x,(y+msy<=wp->vp_y+psy) ? y : wp->vp_y+psy-msy);
				else
					Move(wp->vp_x,wp->vp_y);

				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);

				Resize(msx,msy);
			}
		}else{
			// nejedna se o desktop, okno ma panel, ktery je nahore a neni v predni fronte
			if((x+msx<=psx && y+msy<=psy+div_panel) || 
				(bw_size_max_x && bw_size_max_y && x+bw_size_max_x+dsx<=psx && y+bw_size_max_y+dsy<=psy+div_panel))	// pripad nastavene max. velikosti bw_size (ta nasledne omezi zvetseni okna)
			{
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
				Resize(msx,msy);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,(y+msy<=wp->vp_y+psy+div_panel) ? y : wp->vp_y+psy+div_panel-msy);
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
				Resize(msx,msy);
			}else{
				if(msx<=psx)
					Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,wp->vp_y);
				else if(msy<=psy)
				{
					Move(wp->vp_x,(y+msy<=wp->vp_y+psy+div_panel) ? y : wp->vp_y+psy+div_panel-msy);
				}else
					Move(wp->vp_x,wp->vp_y);

				if(y<STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);

				Resize(msx,msy);
			}
		}
	}
	windowState=WS_Maximized;
	EventArise(EMaximize,1);
}

void CGUIWindow::MaximizeFull()
{
	float msx, msy;
	float psx, psy;
	float dsx=0, dsy=0;
	GetPos(std_x,std_y);
	GetSize(std_sx,std_sy);
	if(parent && parent->GetType() >= PTWindow)
	{
		CGUIWindow* wp = (CGUIWindow*)parent;
		//wp->GetSize(msx,msy);
		wp->GetBackWindowSize(msx,msy);
		/*
		if(wp->panel)
			wp->panel->GetSize(psx,psy);
		else{
			psx=0;psy=0;
		}
		*/
		//if(wp != desktop && wp->panel)
		if(wp->panel && wp->panel->back_element)
		{
			msy-=STD_PANEL_HEIGHT;
		}
		psx = msx; psy = msy;
		CheckResize(msx,msy);
		dsx = sx - vp_sx;	// rozdil ve velikosti vnitrku od vnejsku okna
		dsy = sy - vp_sy;
		//if(wp == desktop || !wp->panel || (wp->panel && !wp->panel->position_up))
		if((wp->panel && !wp->panel->back_element) || !wp->panel || (wp->panel && !wp->panel->position_up))
		{
			if((x+msx<=psx && y+msy<=psy) || 
				(bw_size_max_x && bw_size_max_y && x+bw_size_max_x+dsx<=psx && y+bw_size_max_y+dsy<=psy))	// pripad nastavene max. velikosti bw_size (ta nasledne omezi zvetseni okna)
			{
				if(y<0)
					Move(x,0);
				if(x<0)
					Move(0,y);
				Resize(msx,msy);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=psx) ? x : psx-msx,(y+msy<=psy) ? y : psy-msy);
				if(y<0)
					Move(x,0);
				if(x<0)
					Move(0,y);
				Resize(msx,msy);
			}else{
				if(msx<=psx)
					Move((x+msx<=psx) ? x : psx-msx,0);
				else if(msy<=psy)
					Move(0,(y+msy<=psy) ? y : psy-msy);
				else
					Move(0,0);

				if(y<0)
					Move(x,0);
				if(x<0)
					Move(0,y);

				Resize(msx,msy);
			}
		}else{
			// nejedna se o desktop, okno ma panel, ktery je nahore a neni v predni fronte
			if((x+msx<=psx && y+msy<=psy+STD_PANEL_HEIGHT) || 
				(bw_size_max_x && bw_size_max_y && x+bw_size_max_x+dsx<=psx && y+bw_size_max_y+dsy<=psy+STD_PANEL_HEIGHT))	// pripad nastavene max. velikosti bw_size (ta nasledne omezi zvetseni okna)
			{
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(x<0)
					Move(0,y);
				Resize(msx,msy);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=psx) ? x : psx-msx,(y+msy<=psy+STD_PANEL_HEIGHT) ? y : psy+STD_PANEL_HEIGHT-msy);
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(x<0)
					Move(0,y);
				Resize(msx,msy);
			}else{
				if(msx<=psx)
					Move((x+msx<=psx) ? x : psx-msx,0);
				else if(msy<=psy)
				{
					Move(0,(y+msy<=psy+STD_PANEL_HEIGHT) ? y : psy+STD_PANEL_HEIGHT-msy);
				}else
					Move(0,0);

				if(y<STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(x<0)
					Move(0,y);

				Resize(msx,msy);
			}
		}
	}
	windowState=WS_Maximized;
	EventArise(EMaximize,1);
}


void CGUIWindow::UnMaximize()
{
	Move(std_x,std_y);
	Resize(std_sx,std_sy);
	windowState=WS_Standard;
	EventArise(EMaximize,0);
}

void CGUIWindow::Normalize()
{
	SetWindowSizeBWNormal();

	float msx, msy;
	float psx, psy;
	float div_panel = 0;
	if(parent && parent->GetType() >= PTWindow)
	{
		CGUIWindow* wp = (CGUIWindow*)parent;
		//wp->GetSize(msx,msy);
		wp->GetVPSize(msx,msy);
		/*
		if(wp->panel)
			wp->panel->GetSize(psx,psy);
		else{
			psx=0;psy=0;
		}
		*/
		//if(wp != desktop && wp->panel)
		if(wp->panel && wp->panel->back_element)
		{
			if(wp->panel->position_up && wp->vp_y<STD_PANEL_HEIGHT)
			{	
				msy-= div_panel = STD_PANEL_HEIGHT-wp->vp_y;
			}

			if(!wp->panel->position_up && wp->vp_y > wp->bw_size_y-wp->vp_sy-STD_PANEL_HEIGHT)
			{
				msy -= div_panel = wp->vp_y - (wp->bw_size_y-wp->vp_sy-STD_PANEL_HEIGHT);
			}
			//msy-=STD_PANEL_HEIGHT;
		}
		psx = msx; psy = msy;

		// SetWindowSizeBWNormal(); // !@#$ proc volat znovu ???
		GetSize(msx,msy);
		
		//if(wp == desktop || !wp->panel || (wp->panel && !wp->panel->position_up))
		if((wp->panel && !wp->panel->back_element) || !wp->panel || (wp->panel && !wp->panel->position_up))
		{
			if(x+msx<=psx && y+msy<=psy)
			{
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,(y+msy<=wp->vp_y+psy) ? y : wp->vp_y+psy-msy);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
			}else{
				if(msx<=psx)
					Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,wp->vp_y);
				else if(msy<=psy)
					Move(wp->vp_x,(y+msy<=wp->vp_y+psy) ? y : wp->vp_y+psy-msy);
				else
					Move(wp->vp_x,wp->vp_y);

				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);

				if(psy<msy && msx+STD_SCROLLBAR_WIDTH<=psx)
				{
					if(x+msx+STD_SCROLLBAR_WIDTH>wp->vp_x+psx)
						MoveRel(wp->vp_x+psx-(x+msx+STD_SCROLLBAR_WIDTH),0);
					Resize(msx+STD_SCROLLBAR_WIDTH,psy);
				}else if(psx<msx && msy+STD_SCROLLBAR_WIDTH<=psy)
				{
					if(y+msy+STD_SCROLLBAR_WIDTH>wp->vp_y+psy)
						MoveRel(wp->vp_y+psy-(y+msy+STD_SCROLLBAR_WIDTH),0);
					Resize(psx,msy+STD_SCROLLBAR_WIDTH);
				}else
					Resize(min(msx,psx),min(msy,psy));
			}
		}else{
			// nejedna se o desktop, okno ma panel, ktery je nahore (+ v back window)
			if(x+msx<=psx && y+msy<=psy+div_panel)
			{
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
			}else if(msx<=psx && msy<=psy){
				Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,(y+msy<=wp->vp_y+psy+div_panel) ? y : wp->vp_y+psy+div_panel-msy);
				if(y<=STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);
			}else{
				if(msx<=psx)
					Move((x+msx<=wp->vp_x+psx) ? x : wp->vp_x+psx-msx,wp->vp_y);
				else if(msy<=psy)
				{
					Move(wp->vp_x,(y+msy<=wp->vp_y+psy+div_panel) ? y : wp->vp_y+psy+div_panel-msy);
				}else
					Move(wp->vp_x,wp->vp_y);

				if(y<STD_PANEL_HEIGHT)
					Move(x,STD_PANEL_HEIGHT);
				if(y<wp->vp_y)
					Move(x,wp->vp_y);
				if(x<wp->vp_x)
					Move(wp->vp_x,y);

				Resize(min(msx,psx),min(msy,psy));
			}
		}


	}
	windowState=WS_Normalized;
	EventArise(ENormalize);
}


void CGUIWindow::MinimizeAll()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	CGUIWindow *wnd=0;
	typeID fc_wnd = 0;

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState != WS_Minimized && wnd->IsVisible())
				{
					if(!fc_wnd)
						fc_wnd = wnd->GetID();
					wnd->SetVisible(0);

					if(wnd->windowState==WS_Maximized)
						wnd->maximize_before_minimize=true;
					else
						wnd->maximize_before_minimize=false;
					wnd->windowState=WS_Minimized;
					wnd->minimized=true;	// nastavim, ze okno bylo minimalizovano pri celkove minimalizaci
					//wnd->EventArise(EMinimize,1);
				}else
					wnd->minimized=false;
			}
		}
		le=queue_back.GetNextListElement(le);
	}

	le = queue_front.GetNextListElement(0);
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState != WS_Minimized && wnd->IsVisible())
				{
					if(!fc_wnd)
						fc_wnd = wnd->GetID();
					wnd->SetVisible(0);

					if(wnd->windowState==WS_Maximized)
						wnd->maximize_before_minimize=true;
					else
						wnd->maximize_before_minimize=false;
					wnd->windowState=WS_Minimized;
					wnd->minimized=true;	// nastavim, ze okno bylo minimalizovano pri celkove minimalizaci
					//wnd->EventArise(EMinimize,1);
				}else	
					wnd->minimized=false;
			}			
		}
		le=queue_front.GetNextListElement(le);
	}
	
	e=GetFocusEl();
	if(e && e->GetType() >= PTWindow)
		focus_window_before_minimize_all = ((CGUIWindow*)e)->GetID();
	else
		focus_window_before_minimize_all = fc_wnd;

	SetFocusEl(0,1);
	minimized_all=true;
}

void CGUIWindow::UnMinimizeAll()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	CGUIWindow *wnd=0;

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState == WS_Minimized && wnd->minimized)
				{
					wnd->SetVisible(1);

					if(wnd->maximize_before_minimize)
						wnd->windowState=WS_Maximized;
					else
						wnd->windowState=WS_Standard;
					wnd->minimized=false;	
					//wnd->EventArise(EMinimize,0);
				}
			}
		}
		le=queue_back.GetNextListElement(le);
	}

	le = queue_front.GetNextListElement(0);
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState == WS_Minimized && wnd->minimized)
				{
					wnd->SetVisible(1);

					if(wnd->maximize_before_minimize)
						wnd->windowState=WS_Maximized;
					else
						wnd->windowState=WS_Standard;
					wnd->minimized=false;	
					//wnd->EventArise(EMinimize,0);
				}
			}		
		}
		le=queue_front.GetNextListElement(le);
	}
	
	minimized_all=false;

	wnd = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(focus_window_before_minimize_all));
	if(wnd)
	{
		SetFocusEl(wnd,wnd->back_element ? 0 : 1);
	}
}

void CGUIWindow::SetAllWindowVisible()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = queue_back.GetNextListElement(0);
	CGUIWindow *wnd=0;

	while(le)
	{
		if((e=queue_back.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState == WS_Minimized)
				{
					wnd->SetVisible(1);

					if(wnd->maximize_before_minimize)
						wnd->windowState=WS_Maximized;
					else
						wnd->windowState=WS_Standard;
					wnd->minimized=false;	
					//wnd->EventArise(EMinimize,0);
				}
			}
		}
		le=queue_back.GetNextListElement(le);
	}

	le = queue_front.GetNextListElement(0);
	while(le)
	{
		if((e=queue_front.GetObjectFromListElement(le)))
		{
			if(e->GetType() >= PTWindow)
			{
				wnd = (CGUIWindow*) e;
				if(wnd->windowState == WS_Minimized)
				{
					wnd->SetVisible(1);

					if(wnd->maximize_before_minimize)
						wnd->windowState=WS_Maximized;
					else
						wnd->windowState=WS_Standard;
					wnd->minimized=false;	
					//wnd->EventArise(EMinimize,0);
				}
			}		
		}
		le=queue_front.GetNextListElement(le);
	}
	
	if(minimized_all)
	{
		wnd = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(focus_window_before_minimize_all));
		if(wnd)
		{
			SetFocusEl(wnd,wnd->back_element ? 0 : 1);
		}
	}

	minimized_all=false;
}

CGUIElement* CGUIWindow::FindNextTABElement(float to, bool frontEl)
{
	CGUIElement *e;
	CGUIElement *toe = 0, *min_e = 0, *max_e = 0;
	CGUIListElement<CGUIElement>* le = 0;
	float tao;	// nejmensi vetsi nalezeny to
	float t;
	float min = 0,max = 0;

	if(frontEl)
	{
		if(tab_max_front==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_max_front;
		le = queue_front.GetNextListElement(0);
		while(le)
		{
			if((e=queue_front.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t>to && t<=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_front.GetNextListElement(le);
		}

		if(tab_max_front > max)
			tab_max_front = max;
		if(tab_min_front < min || min == 0)
			tab_min_front = min;
		if(!toe)
			toe=min_e;
		
	}else{
		if(tab_max_back==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_max_back;
		le = queue_back.GetNextListElement(0);
		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t>to && t<=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_back.GetNextListElement(le);
		}

		if(tab_max_back > max)
			tab_max_back = max;
		if(tab_min_back < min || min == 0)
			tab_min_back = min;
		if(!toe)
			toe=min_e;

	}

	return toe;
}

CGUIElement* CGUIWindow::FindTABElement(float to, bool frontEl)
{
	CGUIElement *e;
	CGUIElement *toe = 0, *min_e = 0, *max_e = 0;
	CGUIListElement<CGUIElement>* le = 0;
	float tao;	// nejmensi vetsi nalezeny to
	float t;
	float min = 0,max = 0;

	if(frontEl)
	{
		if(tab_max_front==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_max_front;
		le = queue_front.GetNextListElement(0);
		while(le)
		{
			if((e=queue_front.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t==to)
						return e;
					if(t>to && t<=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_front.GetNextListElement(le);
		}

		if(tab_max_front > max)
			tab_max_front = max;
		if(tab_min_front < min || min == 0)
			tab_min_front = min;
		if(!toe)
			toe=min_e;
		
	}else{
		if(tab_max_back==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_max_back;
		le = queue_back.GetNextListElement(0);
		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t==to)
						return e;
					if(t>to && t<=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_back.GetNextListElement(le);
		}

		if(tab_max_back > max)
			tab_max_back = max;
		if(tab_min_back < min || min == 0)
			tab_min_back = min;
		if(!toe)
			toe=min_e;

	}

	return toe;
}


CGUIElement* CGUIWindow::FindPreviousTABElement(float to, bool frontEl)
{
	CGUIElement *e;
	CGUIElement *toe = 0, *min_e = 0, *max_e = 0;
	CGUIListElement<CGUIElement>* le = 0;
	float tao;	// nejvetsi mensi nalezeny to
	float t;
	float min = 0,max = 0;

	if(frontEl)
	{
		if(tab_min_front==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_min_front;
		le = queue_front.GetNextListElement(0);
		while(le)
		{
			if((e=queue_front.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t<to && t>=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_front.GetNextListElement(le);
		}

		if(tab_max_front > max)
			tab_max_front = max;
		if(tab_min_front < min || min == 0)
			tab_min_front = min;
		if(!toe)
			toe=max_e;
		
	}else{
		if(tab_min_back==0)
			return 0;	// ve fronte neni zadny TAB element
		tao = tab_min_back;
		le = queue_back.GetNextListElement(0);
		while(le)
		{
			if((e=queue_back.GetObjectFromListElement(le)))
			{
				t=e->GetTabOrder();
				if(t>0)
				{
					if(t<to && t>=tao)
					{
						tao=t;
						toe=e;
					}
					if(t>max)
					{
						max = t;
						max_e = e;
					}
					if(t<min || min == 0)
					{
						min = t;
						min_e = e;
					}

				}
			}
			le=queue_back.GetNextListElement(le);
		}

		if(tab_max_back > max)
			tab_max_back = max;
		if(tab_min_back < min || min == 0)
			tab_min_back = min;
		if(!toe)
			toe=max_e;

	}

	return toe;
}

void CGUIWindow::FocusOldTOBackEl()
{
	CGUIElement* el;

	if(!front_focus)
		return;

	if(oldTO)
		el = FindTABElement(oldTO,false);
	else
		el = FindNextTABElement(0,false);
	if(!el)
		el = GetNextBackElement(0);
	if(el)
		SetFocusEl(el,0);
}

void CGUIWindow::SetModal(int _modal, bool set_exclusive_keyboard_like_modality)
{
	modal=_modal;
	if(set_exclusive_keyboard_like_modality)
		exclusive_keyboard = modal ? true : false;
	if(modal && parent)
		FocusMe();
}

//////////////////////////////////////////////////////////////////////
// CGUIDesktop
//////////////////////////////////////////////////////////////////////

const int CGUIDesktop::numEvents = 23;
const int CGUIDesktop::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EMessage,
													EChangeTABElement,EMouseRelMove};


CGUIDesktop::CGUIDesktop()
	: CGUIWindow(0,0,10,10,0)
{
	RemoveFromTrash();
	SetAvailableEvents(availableEvents,numEvents);

	keyClose=false;
	keyMaximalize=false;

	float desktop_sx, desktop_sy;

	mainGUI->rootwnd->GetWindowSize(desktop_sx, desktop_sy);	// velikost desktopu se nastavi podle velikosti okna enginu

	Resize(desktop_sx, desktop_sy);
	
	desktop = this;

	MsgAcceptConsume(MsgMouseRelMove);
	// MsgSetProduce(MsgAll);

	AddToTrash();
}

CGUIDesktop::~CGUIDesktop()
{
	RemoveFromTrash();
}

bool CGUIDesktop::GetDesktopPos(CGUIElement* elm, float &_x, float &_y)
	// prochazi od elementu k desktopu pres odkazy na rodice a prepocitava postupne polohu
	// bere v uvahu i scrollovani vnitrku okna
	// pokud element neni umisten v desktopu, poloha nelze zjistit
{
	CGUIElement* p;
	float xx,yy;
	float px,py;
	bool ret=true;

	elm->GetPos(xx,yy);
	xx+=_x; yy+=_y;

	while(p=elm->GetParent())
	{
		if(p->GetType()==PTMultiWidget)
		{
			p->GetPos(px,py);
			xx+=px; yy+=py;
		}else if(p->GetType()>=PTWindow)
		{
			if(elm->back_element)
			{
				if(!((CGUIWindow*)p)->TransformCoordsBackToFront(xx,yy))
					ret=false;	// souradnice jsou mimo viditelnou oblast back wnd.
			}
			p->GetPos(px,py);
			xx+=px; yy+=py;
		}
		elm=p;
	}

	if(elm!=this)
		ret=false;	// nedosel jsem az k destopu

	_x=xx;
	_y=yy;

	return ret;
}

int CGUIDesktop::MouseRelativeMove(float dx, float dy)
{
	EventArise(EMouseRelMove,0,0,0,dx,0,0,dy);
	return 1;
}


//////////////////////////////////////////////////////////////////////
// CGUIDataExchange
//////////////////////////////////////////////////////////////////////

CGUIDataExchange::CGUIDataExchange(int _numAttributes)
{
	numAttributes = _numAttributes; 
	attribute = new void*[numAttributes]; 
	attrib_type = new int[numAttributes]; 
	attrib_size = new int[numAttributes]; 

	for(int i=0; i<numAttributes; i++)
	{
		attribute[i] = 0; attrib_type[i] = 0; attrib_size[i] = 0;
	}

	data=0;
	data_type=0;
	data_size=0;
}

CGUIDataExchange::~CGUIDataExchange() 
{ 
	SAFE_DELETE_ARRAY(attribute); 
	SAFE_DELETE_ARRAY(attrib_type); 
	SAFE_DELETE_ARRAY(attrib_size); 
	SAFE_DELETE_ARRAY(data);
	SAFE_DELETE_ARRAY(data_type);
	SAFE_DELETE_ARRAY(data_size);
}

void CGUIDataExchange::SetElementAttributes(void* *_data, int *_data_type, int *_data_size)
{
	data = _data;
	data_type = _data_type;
	data_size = _data_size;
}

void CGUIDataExchange::SetElementAttributes(void* _data, int type, int size)
{
	data = new void*[1]; 
	data[0] = _data; 
	data_type = new int[1]; 
	data_type[0] = type;
	data_size = new int[1];
	data_size[0] = size;
}

int CGUIDataExchange::BindAttribute(void* _attribute, int _type, int _size, int which)
{
	if(which>=numAttributes)
		return 1;
	attribute[which] = _attribute;
	attrib_type[which] = _type;
	attrib_size[which] = _size;
	return 0;
}

//----------------------------------------------------------------------
/*
Synchronizuje hodnoty v datovych polozkach elementu GUI a uzivatelskych svazanych atributech
Synchronizace je obousmerna (tj. lze zvolit smer, jakym bude probihat)
Toto je standardni automaticka synchronizace, element si muze implementovat vlastnim zpusobem,
pomoci pretizeni funkce Sync.
Synchronizace je postavena na datovych polozkach, ktere je treba predem nastavit:
data[]			- pole ukazatelu na atributy elementu GUI (vyplnuje element)
data_type[]		- pole typu atributu GUI (EGUIDataTypes)
data_size[]		- pole velikosti atributu GUI (v poctu polozek typu (napr. pocet intu v poli (ne pocet bytu pole intu))
	- tyto datove struktury nastavuje element pres SetElementAttributes, pokud chce vyuzivat tuto automatickou synchronizaci

attribute[]		- pole ukazatelu na uzivatelske atributy (vyplnuje uzivatel pres fci BindAttribute)
attrib_type[]	- pole typu uzivatelskych atributu
attrib_size[]	- pole velikosti uzivatelskych atributu
	- tyto datove polozky se nastavuje uzivatel pomoci fce BindAttribute (po jedne)

Pri synchronizaci se prochazi obe pole najednou po polozkach. Pokud jsou obe odpovidajici si
polozky vyplneny (nastaveny) dojde ke kontrolovanemu prekopirovani dat. (smer urcuje parametr out)
out == true - zapisuje se do svazaneho uzivatelskeho atributu, hodnota se cte z elementu GUI
out == false - zapisuje se do elementu GUI, hodnota se cte ze svazaneho uzivatelskeho atributu

pravidla prenosu hodnot, zavislost na typech obou atributu 
(uzivatelskeho i elementu GUI, tj. vstupni a vystupni promenna):

attrib_size == 1 urcuje, ze se jedna o jednoduchou datovou polozku typu attrib_type
attrib_size > 1  urcuje, ze jde o pole nebo string (attrib_type==string)

ukladani do jednoducheho typu:
	zdrojove data se prevedou na cilovy typ a ulozi se
	pokud je zdroj string, prevede se na prislusne cislo (int, double,...) a ulozi se
	pokud je zdroj pole, ulozi se prvni prvek pole

ukladani do pole:
	zdrojove data se prevedou na cilovy typ
	pokud je zdroj jednoducheho typu ulozi se do prvni pozice ciloveho pole
	pokud je zdroj pole, prekopiruje se cele pole 
	(jede se od zacatku obou poli, jakmile jedno pole skonci, prestane se s kopirovanim)
	pokud je zdroj string, prevede se na cislo a ulozi do prvniho prvku pole

ukladani do stringu:
	pokud je zdroj jednoducheho typu, prevede se na string (int, double,... => string) a ulozi se (kontroluje se velikost stringu)
		pokud je typ==char misto prevodu cisla na string se char pouzije jako pismeno, tj. ulozi se jako jednopismeny string + ukonceni '\0'
	pokud je zdroj pole, vezme se prvni prvek pole, prevede se na string a ulozi se (kontroluje se velikost stringu)
	pokud je zdroj string, okopiruje se string na string (kontroluje se velikost stringu)

*/
//----------------------------------------------------------------------

/*
int CGUIDataExchange::Sync(bool out)
{
	if(!data)
		return 1;
	
	int ii;

	for(int i = 0; i< numAttributes; i++)
	{
		if(!attribute[i] || !data[i] || !attrib_size[i] || !data_size[i])
			continue;
		if(out)
		{
			// ukladani hodnot z elementu do svazanych uzivatelskych atributu

			if(attrib_size[i] == 1)		// jednoduche datove typy
				switch(attrib_type[i])
				{
				case dtInt :
					switch(data_type[i])
					{
					case dtInt		: *((int*)attribute[i]) = *((int*)data[i]); break;
					case dtChar		: *((int*)attribute[i]) = *((char*)data[i]); break;
					case dtFloat	: *((int*)attribute[i]) = (int)*((float*)data[i]); break;
					case dtDouble	: *((int*)attribute[i]) = (int)*((double*)data[i]); break;
					case dtString	: *((int*)attribute[i]) = atoi((char*)data[i]); break;
					}
					break;
				case dtChar :
					switch(data_type[i])
					{
					case dtInt		: *((char*)attribute[i]) = (char)*((int*)data[i]); break;
					case dtChar		: *((char*)attribute[i]) = *((char*)data[i]); break;
					case dtFloat	: *((char*)attribute[i]) = (char)*((float*)data[i]); break;
					case dtDouble	: *((char*)attribute[i]) = (char)*((double*)data[i]); break;
					case dtString	: *((char*)attribute[i]) = (char)atoi((char*)data[i]); break;
					}
					break;
				case dtFloat :
					switch(data_type[i])
					{
					case dtInt		: *((float*)attribute[i]) = (float)*((int*)data[i]); break;
					case dtChar		: *((float*)attribute[i]) = (float)*((char*)data[i]); break;
					case dtFloat	: *((float*)attribute[i]) = (float)*((float*)data[i]); break;
					case dtDouble	: *((float*)attribute[i]) = (float)*((double*)data[i]); break;
					case dtString	: *((float*)attribute[i]) = (float)atof((char*)data[i]); break;
					}
					break;
				case dtDouble :
					switch(data_type[i])
					{
					case dtInt		: *((double*)attribute[i]) = (double)*((int*)data[i]); break;
					case dtChar		: *((double*)attribute[i]) = (double)*((char*)data[i]); break;
					case dtFloat	: *((double*)attribute[i]) = (double)*((float*)data[i]); break;
					case dtDouble	: *((double*)attribute[i]) = (double)*((double*)data[i]); break;
					case dtString	: *((double*)attribute[i]) = atof((char*)data[i]); break;
					}
					break;
				case dtString :
					// do stringu velikosti 1 nema smysl nic ukladat ! '\0'
					break;
				}
			else				// pole a stringy
				switch(attrib_type[i])
				{
				case dtInt :
					switch(data_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)attribute[i]+ii) = *((int*)data[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)attribute[i]+ii) = *((char*)data[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)attribute[i]+ii) = (int)*((float*)data[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)attribute[i]+ii) = (int)*((double*)data[i]+ii); ii++; }
						break;
					case dtString	: *((int*)attribute[i]) = atoi((char*)data[i]); break;
					}
					break;
				case dtChar :
					switch(data_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)attribute[i]+ii) = (char)*((int*)data[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)attribute[i]+ii) = *((char*)data[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)attribute[i]+ii) = (char)*((float*)data[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)attribute[i]+ii) = (char)*((double*)data[i]+ii); ii++; }
						break;
					case dtString	: *((char*)attribute[i]) = (char)atoi((char*)data[i]); break;
					}
					break;
				case dtFloat :
					switch(data_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)attribute[i]+ii) = (float)*((int*)data[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)attribute[i]+ii) = (float)*((char*)data[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)attribute[i]+ii) = (float)*((float*)data[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)attribute[i]+ii) = (float)*((double*)data[i]+ii); ii++; }
						break;
					case dtString	: *((float*)attribute[i]) = (float)atof((char*)data[i]); break;
					}
					break;
				case dtDouble :
					switch(data_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)attribute[i]+ii) = (double)*((int*)data[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)attribute[i]+ii) = (double)*((char*)data[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)attribute[i]+ii) = (double)*((float*)data[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)attribute[i]+ii) = (double)*((double*)data[i]+ii); ii++; }
						break;
					case dtString	: *((double*)attribute[i]) = atof((char*)data[i]); break;
					}
					break;
				case dtString :
					switch(data_type[i])
					{
					char buf[100];
					case dtInt:
						// zkonvertovat int na string a ulozit ho do attribute (pozor na velikost stringu)
						strncpy(((char*)attribute[i]),_itoa(*((int*)data[i]),buf,10),attrib_size[i]);
						break;
					case dtChar:
						*((char*)attribute[i]) = *((char*)data[i]);
						*((char*)attribute[i]+1) = '\0';
						break;
					case dtFloat:
						_snprintf(((char*)attribute[i]),attrib_size[i],"%f",*((float*)data[i]));
						break;
					case dtDouble:
						_snprintf(((char*)attribute[i]),attrib_size[i],"%f",*((double*)data[i]));
						break;
					case dtString	: 
						strncpy(((char*)attribute[i]),((char*)data[i]),attrib_size[i]); break;
					}
					break;
				}
		}else{		
			// ukladani hodnot od uzivatele do elementu

			if(data_size[i] == 1)		// jednoduche datove typy
				switch(data_type[i])
				{
				case dtInt :
					switch(attrib_type[i])
					{
					case dtInt		: *((int*)data[i]) = *((int*)attribute[i]); break;
					case dtChar		: *((int*)data[i]) = *((char*)attribute[i]); break;
					case dtFloat	: *((int*)data[i]) = (int)*((float*)attribute[i]); break;
					case dtDouble	: *((int*)data[i]) = (int)*((double*)attribute[i]); break;
					case dtString	: *((int*)data[i]) = atoi((char*)attribute[i]); break;
					}
					break;
				case dtChar :
					switch(attrib_type[i])
					{
					case dtInt		: *((char*)data[i]) = (char)*((int*)attribute[i]); break;
					case dtChar		: *((char*)data[i]) = *((char*)attribute[i]); break;
					case dtFloat	: *((char*)data[i]) = (char)*((float*)attribute[i]); break;
					case dtDouble	: *((char*)data[i]) = (char)*((double*)attribute[i]); break;
					case dtString	: *((char*)data[i]) = (char)atoi((char*)attribute[i]); break;
					}
					break;
				case dtFloat :
					switch(attrib_type[i])
					{
					case dtInt		: *((float*)data[i]) = (float)*((int*)attribute[i]); break;
					case dtChar		: *((float*)data[i]) = (float)*((char*)attribute[i]); break;
					case dtFloat	: *((float*)data[i]) = (float)*((float*)attribute[i]); break;
					case dtDouble	: *((float*)data[i]) = (float)*((double*)attribute[i]); break;
					case dtString	: *((float*)data[i]) = (float)atof((char*)attribute[i]); break;
					}
					break;
				case dtDouble :
					switch(attrib_type[i])
					{
					case dtInt		: *((double*)data[i]) = (double)*((int*)attribute[i]); break;
					case dtChar		: *((double*)data[i]) = (double)*((char*)attribute[i]); break;
					case dtFloat	: *((double*)data[i]) = (double)*((float*)attribute[i]); break;
					case dtDouble	: *((double*)data[i]) = (double)*((double*)attribute[i]); break;
					case dtString	: *((double*)data[i]) = atof((char*)attribute[i]); break;
					}
					break;
				case dtString :
					// do stringu velikosti 1 nema smysl nic ukladat ! '\0'
					break;
				}
			else				// pole a stringy
				switch(data_type[i])
				{
				case dtInt :
					switch(attrib_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)data[i]+ii) = *((int*)attribute[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)data[i]+ii) = *((char*)attribute[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)data[i]+ii) = (int)*((float*)attribute[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((int*)data[i]+ii) = (int)*((double*)attribute[i]+ii); ii++; }
						break;
					case dtString	: *((int*)data[i]) = atoi((char*)attribute[i]); break;
					}
					break;
				case dtChar :
					switch(attrib_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)data[i]+ii) = (char)*((int*)attribute[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)data[i]+ii) = *((char*)attribute[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)data[i]+ii) = (char)*((float*)attribute[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((char*)data[i]+ii) = (char)*((double*)attribute[i]+ii); ii++; }
						break;
					case dtString	: *((char*)data[i]) = (char)atoi((char*)attribute[i]); break;
					}
					break;
				case dtFloat :
					switch(attrib_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)data[i]+ii) = (float)*((int*)attribute[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)data[i]+ii) = (float)*((char*)attribute[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)data[i]+ii) = (float)*((float*)attribute[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((float*)data[i]+ii) = (float)*((double*)attribute[i]+ii); ii++; }
						break;
					case dtString	: *((float*)data[i]) = (float)atof((char*)attribute[i]); break;
					}
					break;
				case dtDouble :
					switch(attrib_type[i])
					{
					case dtInt:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)data[i]+ii) = (double)*((int*)attribute[i]+ii); ii++; }
						break;
					case dtChar:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)data[i]+ii) = (double)*((char*)attribute[i]+ii); ii++; }
						break;
					case dtFloat:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)data[i]+ii) = (double)*((float*)attribute[i]+ii); ii++; }
						break;
					case dtDouble:
						ii = 0;
						while(ii < attrib_size[i] && ii < data_size[i])
							{	*((double*)data[i]+ii) = (double)*((double*)attribute[i]+ii); ii++; }
						break;
					case dtString	: *((double*)data[i]) = atof((char*)attribute[i]); break;
					}
					break;
				case dtString :
					switch(attrib_type[i])
					{
					char buf[100];
					case dtInt:
						// zkonvertovat int na string a ulozit ho do attribute (pozor na velikost stringu)
						strncpy(((char*)data[i]),_itoa(*((int*)attribute[i]),buf,10),data_size[i]);
						break;
					case dtChar:
						*((char*)data[i]) = *((char*)attribute[i]);
						*((char*)data[i]+1) = '\0';
						break;
					case dtFloat:
						_snprintf(((char*)data[i]),data_size[i],"%f",*((float*)attribute[i]));
						break;
					case dtDouble:
						_snprintf(((char*)data[i]),data_size[i],"%f",*((double*)attribute[i]));
						break;
					case dtString	: 
						strncpy(((char*)data[i]),((char*)attribute[i]),data_size[i]); break;
					}
					break;
				}
		}
	}

	return 0;
}
*/

int CGUIDataExchange::Sync(bool out)
{
	int ret=0;
	int a;
	if(data)
	{
		for(int i = 0; i< numAttributes; i++)
		{
			if(!attribute[i] || !data[i] || !attrib_size[i] || !data_size[i])
				continue;
			if(out)
			{
				ret = (a=GetParametr(attribute[i],attrib_type[i],attrib_size[i],i)) ? a : ret;
			}else{
				ret = (a=SetParametr(attribute[i],attrib_type[i],attrib_size[i],i)) ? a : ret;
			}
		}
	}else{
		for(int i = 0; i< numAttributes; i++)
		{
			if(!attribute[i] || !attrib_size[i])
				continue;
			if(out)
			{
				ret = (a=GetParametr(attribute[i],attrib_type[i],attrib_size[i],i)) ? a : ret;
			}else{
				ret = (a=SetParametr(attribute[i],attrib_type[i],attrib_size[i],i)) ? a : ret;
			}
		}
	}

	return ret;
}


int CGUIDataExchange::GetParametr(void *value, int type, int size, int which)
{
	if(which>=numAttributes)
		return 1;

	if(!data)
		return 1;
	
	int ii;

	if(!value || !data[which] || !size || !data_size[which])
		return 1;

	if(size == 1)		// jednoduche datove typy
		switch(type)
		{
		case dtInt :
			switch(data_type[which])
			{
			case dtInt		: *((int*)value) = *((int*)data[which]); break;
			case dtChar		: *((int*)value) = *((char*)data[which]); break;
			case dtFloat	: *((int*)value) = (int)*((float*)data[which]); break;
			case dtDouble	: *((int*)value) = (int)*((double*)data[which]); break;
			case dtString	: *((int*)value) = atoi((char*)data[which]); break;
			}
			break;
		case dtChar :
			switch(data_type[which])
			{
			case dtInt		: *((char*)value) = (char)*((int*)data[which]); break;
			case dtChar		: *((char*)value) = *((char*)data[which]); break;
			case dtFloat	: *((char*)value) = (char)*((float*)data[which]); break;
			case dtDouble	: *((char*)value) = (char)*((double*)data[which]); break;
			case dtString	: *((char*)value) = (char)atoi((char*)data[which]); break;
			}
			break;
		case dtFloat :
			switch(data_type[which])
			{
			case dtInt		: *((float*)value) = (float)*((int*)data[which]); break;
			case dtChar		: *((float*)value) = (float)*((char*)data[which]); break;
			case dtFloat	: *((float*)value) = (float)*((float*)data[which]); break;
			case dtDouble	: *((float*)value) = (float)*((double*)data[which]); break;
			case dtString	: *((float*)value) = (float)atof((char*)data[which]); break;
			}
			break;
		case dtDouble :
			switch(data_type[which])
			{
			case dtInt		: *((double*)value) = (double)*((int*)data[which]); break;
			case dtChar		: *((double*)value) = (double)*((char*)data[which]); break;
			case dtFloat	: *((double*)value) = (double)*((float*)data[which]); break;
			case dtDouble	: *((double*)value) = (double)*((double*)data[which]); break;
			case dtString	: *((double*)value) = atof((char*)data[which]); break;
			}
			break;
		case dtString :
			// do stringu velikosti 1 nema smysl nic ukladat ! '\0'
			break;
		}
	else				// pole a stringy
		switch(type)
		{
		case dtInt :
			switch(data_type[which])
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)value+ii) = *((int*)data[which]+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)value+ii) = *((char*)data[which]+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)value+ii) = (int)*((float*)data[which]+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)value+ii) = (int)*((double*)data[which]+ii); ii++; }
				break;
			case dtString	: *((int*)value) = atoi((char*)data[which]); break;
			}
			break;
		case dtChar :
			switch(data_type[which])
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)value+ii) = (char)*((int*)data[which]+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)value+ii) = *((char*)data[which]+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)value+ii) = (char)*((float*)data[which]+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)value+ii) = (char)*((double*)data[which]+ii); ii++; }
				break;
			case dtString	: *((char*)value) = (char)atoi((char*)data[which]); break;
			}
			break;
		case dtFloat :
			switch(data_type[which])
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)value+ii) = (float)*((int*)data[which]+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)value+ii) = (float)*((char*)data[which]+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)value+ii) = (float)*((float*)data[which]+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)value+ii) = (float)*((double*)data[which]+ii); ii++; }
				break;
			case dtString	: *((float*)value) = (float)atof((char*)data[which]); break;
			}
			break;
		case dtDouble :
			switch(data_type[which])
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)value+ii) = (double)*((int*)data[which]+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)value+ii) = (double)*((char*)data[which]+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)value+ii) = (double)*((float*)data[which]+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)value+ii) = (double)*((double*)data[which]+ii); ii++; }
				break;
			case dtString	: *((double*)value) = atof((char*)data[which]); break;
			}
			break;
		case dtString :
			switch(data_type[which])
			{
			char buf[100];
			case dtInt:
				// zkonvertovat int na string a ulozit ho do attribute (pozor na velikost stringu)
				strncpy(((char*)value),_itoa(*((int*)data[which]),buf,10),size);
				*((char*)value+size-1) = '\0';
				break;
			case dtChar:
				*((char*)value) = *((char*)data[which]);
				*((char*)value+1) = '\0';
				break;
			case dtFloat:
				_snprintf(((char*)value),size,"%f",*((float*)data[which]));
				break;
			case dtDouble:
				_snprintf(((char*)value),size,"%f",*((double*)data[which]));
				break;
			case dtString	: 
				strncpy(((char*)value),((char*)data[which]),size);
				*((char*)value+size-1) = '\0';
				break;
			}
			break;
		}

	return 0;
}

int CGUIDataExchange::SetParametr(void *value, int type, int size, int which)
{
	// ukladani hodnot od uzivatele do elementu

	if(which>=numAttributes)
		return 1;

	if(!data)
		return 1;
	
	int ii;

	if(!value || !data[which] || !size || !data_size[which])
		return 1;


	if(data_size[which] == 1)		// jednoduche datove typy
		switch(data_type[which])
		{
		case dtInt :
			switch(type)
			{
			case dtInt		: *((int*)data[which]) = *((int*)value); break;
			case dtChar		: *((int*)data[which]) = *((char*)value); break;
			case dtFloat	: *((int*)data[which]) = (int)*((float*)value); break;
			case dtDouble	: *((int*)data[which]) = (int)*((double*)value); break;
			case dtString	: *((int*)data[which]) = atoi((char*)value); break;
			}
			break;
		case dtChar :
			switch(type)
			{
			case dtInt		: *((char*)data[which]) = (char)*((int*)value); break;
			case dtChar		: *((char*)data[which]) = *((char*)value); break;
			case dtFloat	: *((char*)data[which]) = (char)*((float*)value); break;
			case dtDouble	: *((char*)data[which]) = (char)*((double*)value); break;
			case dtString	: *((char*)data[which]) = (char)atoi((char*)value); break;
			}
			break;
		case dtFloat :
			switch(type)
			{
			case dtInt		: *((float*)data[which]) = (float)*((int*)value); break;
			case dtChar		: *((float*)data[which]) = (float)*((char*)value); break;
			case dtFloat	: *((float*)data[which]) = (float)*((float*)value); break;
			case dtDouble	: *((float*)data[which]) = (float)*((double*)value); break;
			case dtString	: *((float*)data[which]) = (float)atof((char*)value); break;
			}
			break;
		case dtDouble :
			switch(type)
			{
			case dtInt		: *((double*)data[which]) = (double)*((int*)value); break;
			case dtChar		: *((double*)data[which]) = (double)*((char*)value); break;
			case dtFloat	: *((double*)data[which]) = (double)*((float*)value); break;
			case dtDouble	: *((double*)data[which]) = (double)*((double*)value); break;
			case dtString	: *((double*)data[which]) = atof((char*)value); break;
			}
			break;
		case dtString :
			// do stringu velikosti 1 nema smysl nic ukladat ! '\0'
			break;
		}
	else				// pole a stringy
		switch(data_type[which])
		{
		case dtInt :
			switch(type)
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)data[which]+ii) = *((int*)value+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)data[which]+ii) = *((char*)value+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)data[which]+ii) = (int)*((float*)value+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((int*)data[which]+ii) = (int)*((double*)value+ii); ii++; }
				break;
			case dtString	: *((int*)data[which]) = atoi((char*)value); break;
			}
			break;
		case dtChar :
			switch(type)
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)data[which]+ii) = (char)*((int*)value+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)data[which]+ii) = *((char*)value+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)data[which]+ii) = (char)*((float*)value+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((char*)data[which]+ii) = (char)*((double*)value+ii); ii++; }
				break;
			case dtString	: *((char*)data[which]) = (char)atoi((char*)value); break;
			}
			break;
		case dtFloat :
			switch(type)
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)data[which]+ii) = (float)*((int*)value+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)data[which]+ii) = (float)*((char*)value+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)data[which]+ii) = (float)*((float*)value+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((float*)data[which]+ii) = (float)*((double*)value+ii); ii++; }
				break;
			case dtString	: *((float*)data[which]) = (float)atof((char*)value); break;
			}
			break;
		case dtDouble :
			switch(type)
			{
			case dtInt:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)data[which]+ii) = (double)*((int*)value+ii); ii++; }
				break;
			case dtChar:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)data[which]+ii) = (double)*((char*)value+ii); ii++; }
				break;
			case dtFloat:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)data[which]+ii) = (double)*((float*)value+ii); ii++; }
				break;
			case dtDouble:
				ii = 0;
				while(ii < size && ii < data_size[which])
					{	*((double*)data[which]+ii) = (double)*((double*)value+ii); ii++; }
				break;
			case dtString	: *((double*)data[which]) = atof((char*)value); break;
			}
			break;
		case dtString :
			switch(type)
			{
			char buf[100];
			case dtInt:
				// zkonvertovat int na string a ulozit ho do attribute (pozor na velikost stringu)
				strncpy(((char*)data[which]),_itoa(*((int*)value),buf,10),data_size[which]);
				*((char*)data[which]+data_size[which]-1) = '\0';
				break;
			case dtChar:
				*((char*)data[which]) = *((char*)value);
				*((char*)data[which]+1) = '\0';
				break;
			case dtFloat:
				_snprintf(((char*)data[which]),data_size[which],"%f",*((float*)value));
				break;
			case dtDouble:
				_snprintf(((char*)data[which]),data_size[which],"%f",*((double*)value));
				break;
			case dtString	: 
				strncpy(((char*)data[which]),((char*)value),data_size[which]);
				*((char*)data[which]+data_size[which]-1) = '\0';
				break;
			}
			break;
		}


	return 0;
}


