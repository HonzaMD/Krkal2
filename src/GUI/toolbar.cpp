///////////////////////////////////////////////
//
// toolbar.cpp
//
// Implementace pro ToolBar a jeho casti ToolBarButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "toolbar.h"
#include "primitives.h"
#include "widgets.h"


#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIToolBarButton
//////////////////////////////////////////////////////////////////////

const int CGUIToolBarButton::numEvents = 2;
const int CGUIToolBarButton::availableEvents[numEvents] = {EClicked,EDeleted};


CGUIToolBarButton::CGUIToolBarButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, CGUIRectHost *_pictureUp, CGUIRectHost *_pictureDown)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	raisable=true;
	down_stable=true;
	pos_h = pos_v = 0;
	rootwnd=0;
	st9 = new CGUIStandard9(_x,_y,_up);
	simpleButton = 0;
	resizable = true;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	inactive = 0;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIToolBarButton::CGUIToolBarButton> Style is missing");
	}

	active=true;
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	st9->Resize(_sx,_sy);
	st9->GetSize(_sx,_sy);	// mozna zmena velikosti kvuli minimalnim rozmerum cudlitka dle jeho stylu
	SetSize(_sx,_sy);

	label=0;
	pictureUp = _pictureUp;
	pictureDown = _pictureDown;
	pictureInactive = _pictureUp;
	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(pictureUp)
		pictureUp->RemoveFromTrash();
	if(pictureDown)
		pictureDown->RemoveFromTrash();
}

CGUIToolBarButton::CGUIToolBarButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, CGUIRectHost *_pictureUp, CGUIRectHost *_pictureDown)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents), st9(0)
{
	raisable=true;
	down_stable=true;
	pos_h = pos_v = 0;
	rootwnd=0;
	resizable = _resizable;
	CTexture *t;
	UINT ssx, ssy;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	inactive = 0;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIToolBarButton::CGUIToolBarButton> Style is missing");
	}

	if((up->GetNumTextures()<1) || (down->GetNumTextures()<1) || (markUp->GetNumTextures()<1) || (markDown->GetNumTextures()<1))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIToolBarButton::CGUIToolBarButton> Style hasn't enought textures");
	}

	active=true;
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	t = up->GetTexture(0);
	t->GetImageSize(&ssx,&ssy);
	sx = (_sx<0) ? (float) ssx : _sx;
	sy = (_sy<0) ? (float) ssy : _sy;
	simpleButton = new CGUIRectHost(x+(*up)[4],y+(*up)[5],t,sx,sy);
	if(simpleButton)
		simpleButton->RemoveFromTrash();

	label=0;

	pictureUp = _pictureUp;
	pictureDown = _pictureDown;
	pictureInactive = _pictureUp;

	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(pictureUp)
		pictureUp->RemoveFromTrash();
	if(pictureDown)
		pictureDown->RemoveFromTrash();
}


void CGUIToolBarButton::SetProperties(bool _raisable, bool _down_stable)
{
	raisable = _raisable;
	down_stable = _down_stable;
}

void CGUIToolBarButton::SetLabel(CGUIStaticText *_label)
{
	CBltElem* pos=0;
	if(_label)
		_label->RemoveFromTrash();

	if(label)
	{
		pos = label->GetElemPosition();
		delete label;		// v destruktoru dojde k vyjmuti z enginu
	}
	label = _label;
	if(label)
		label->SetVisible(IsVisible());

	SetPositionOfLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
			case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUIToolBarButton::SetPictureUp(CGUIRectHost *_pictureUp)
{
	CBltElem* pos=0;

	if(_pictureUp)
		_pictureUp->RemoveFromTrash();
	
	if(pictureUp==pictureInactive)
		pictureInactive=_pictureUp;
	if(pictureUp)
		pos = pictureUp->GetElemPosition();
	SAFE_DELETE(pictureUp);
	pictureUp=_pictureUp;
	if(pictureUp)
		pictureUp->SetVisible(IsVisible());
	
	
	SetPositionOfLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
			case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	if(pictureUp && rootwnd && (active || pictureUp==pictureInactive))
	{
		if(pos)
			pictureUp->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureUp->AddToEngine(rootwnd);
	}
}

void CGUIToolBarButton::SetPictureDown(CGUIRectHost *_pictureDown)
{
	CBltElem* pos=0;

	if(_pictureDown)
		_pictureDown->RemoveFromTrash();
	
	if(pictureDown)
		pos = pictureDown->GetElemPosition();
	SAFE_DELETE(pictureDown);
	pictureDown=_pictureDown;
	if(pictureDown)
		pictureDown->SetVisible(IsVisible());
	
	
	SetPositionOfLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
			case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	if(pictureDown && rootwnd && active)
	{
		if(pos)
			pictureDown->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureDown->AddToEngine(rootwnd);
	}
}

void CGUIToolBarButton::SetPictureInactive(CGUIRectHost *_pictureInactive)
{
	CBltElem* pos=0;

	if(_pictureInactive)
		_pictureInactive->RemoveFromTrash();
	
	if(pictureUp==pictureInactive)
	{	
		if(pictureUp && !active)
		{
			pictureUp->GetElemPosition();
			pictureUp->SetVisible(0);
		}
		pictureInactive=0;
	}
		
	if(pictureInactive)
		pos = pictureInactive->GetElemPosition();
	
	SAFE_DELETE(pictureInactive);
	pictureInactive=_pictureInactive;
	if(pictureInactive)
		pictureInactive->SetVisible(IsVisible());
	
	
	SetPositionOfLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
			case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	if(pictureInactive && rootwnd && !active)
	{
		if(pos)
			pictureInactive->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureInactive->AddToEngine(rootwnd);
	}
}


void CGUIToolBarButton::MoveLabelPicture(float dx, float dy)
{
	if(label)
		label->MoveRel(dx,dy);
	if(pictureUp)
		pictureUp->MoveRel(dx,dy);
	if(pictureDown)
		pictureDown->MoveRel(dx,dy);
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->MoveRel(dx,dy);
}

void CGUIToolBarButton::SetPositionOfLabelPicture()
{
	float pos_x, pos_y, siz_x, siz_y;	// atributy buttonu
	float p_x, p_y, p_sx, p_sy;			// atributy obrazku
	float l_x, l_y, l_sx, l_sy;			// atributy popisku

	GetPos(pos_x,pos_y);
    GetSize(siz_x,siz_y);
	
	if(label)
	{
		label->GetSize(l_sx,l_sy);
		l_x=pos_x+(siz_x-l_sx)/2;
		l_y=pos_y+(siz_y-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));	
	}
	if(pictureUp)
	{
	    pictureUp->GetSize(p_sx,p_sy);
		if(p_sx>siz_x-4 && p_sy>siz_y-4)
			pictureUp->Resize(siz_x-4,siz_y-4);
		else if(p_sx>siz_x-4)
			pictureUp->Resize(siz_x-4,p_sy);
		else if(p_sy>siz_y-4)
			pictureUp->Resize(p_sx,siz_y-4);

		pictureUp->GetSize(p_sx,p_sy);
		p_x=pos_x+(siz_x-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureUp->Move(floorf(p_x),floorf(p_y));
	}
	if(pictureDown)
	{
	    pictureDown->GetSize(p_sx,p_sy);
		if(p_sx>siz_x-4 && p_sy>siz_y-4)
			pictureDown->Resize(siz_x-4,siz_y-4);
		else if(p_sx>siz_x-4)
			pictureDown->Resize(siz_x-4,p_sy);
		else if(p_sy>siz_y-4)
			pictureDown->Resize(p_sx,siz_y-4);

		pictureDown->GetSize(p_sx,p_sy);
		p_x=pos_x+(siz_x-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureDown->Move(floorf(p_x),floorf(p_y));
	}
	if(pictureInactive && pictureInactive!=pictureUp)
	{
	    pictureInactive->GetSize(p_sx,p_sy);
		if(p_sx>siz_x-4 && p_sy>siz_y-4)
			pictureInactive->Resize(siz_x-4,siz_y-4);
		else if(p_sx>siz_x-4)
			pictureInactive->Resize(siz_x-4,p_sy);
		else if(p_sy>siz_y-4)
			pictureInactive->Resize(p_sx,siz_y-4);

		pictureInactive->GetSize(p_sx,p_sy);
		p_x=pos_x+(siz_x-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureInactive->Move(floorf(p_x),floorf(p_y));
	}
}

CGUIToolBarButton::~CGUIToolBarButton()
{
	RemoveFromTrash();
	if(st9)
		delete st9;
	if(simpleButton)
		delete simpleButton;
	if(label)
		delete label;
	if(pictureUp)
		delete pictureUp;
	if(pictureDown)
		delete pictureDown;
	if(pictureInactive && pictureInactive!=pictureUp)
		delete pictureInactive;
	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	EventArise(EDeleted);
}

void CGUIToolBarButton::ReStylize(CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_markUp, CGUIStyle *_markDown)
{
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(_up,(*_up)[0]-(*up)[0],(*_up)[1]-(*up)[1]);	// zmena stylu z up na _up
						MoveLabelPicture((*_up)[2]-(*up)[2],(*_up)[3]-(*up)[3]);
						break;
			case 1 :	SetStyle(_markUp,(*_markUp)[0]-(*markUp)[0],(*_markUp)[1]-(*markUp)[1]);	
						MoveLabelPicture((*_markUp)[2]-(*markUp)[2],(*_markUp)[3]-(*markUp)[3]);
						break;
			case 2 :	SetStyle(_down,(*_down)[0]-(*down)[0],(*_down)[1]-(*down)[1]);	
						MoveLabelPicture((*_down)[2]-(*down)[2],(*_down)[3]-(*down)[3]);
						break;
			case 3 :	SetStyle(_markDown,(*_markDown)[0]-(*markDown)[0],(*_markDown)[1]-(*markDown)[1]);	
						MoveLabelPicture((*_markDown)[2]-(*markDown)[2],(*_markDown)[3]-(*markDown)[3]);
						break;
		}
	}
	up=_up;down=_down;markUp=_markUp;markDown=_markDown;
	EventArise(EGraphicsChanged);
}

void CGUIToolBarButton::ChangeContextHelp(char* text)
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

void CGUIToolBarButton::SetInactiveStyle(CGUIStyle *_inactive)
{
	if(active)
		inactive = _inactive;
	else{
		if(inactive && _inactive)
		{
			SetStyle(_inactive,(*_inactive)[0]-(*inactive)[0],(*_inactive)[1]-(*inactive)[1]);	// zmena stylu z puvodniho inactive na novy _inactive
			MoveLabelPicture((*_inactive)[2]-(*inactive)[2],(*_inactive)[3]-(*inactive)[3]);
		}else if(_inactive){
			switch(button_state)
			{
				case 0 :	MoveLabelPicture((*_inactive)[2]-(*up)[2],(*_inactive)[3]-(*up)[3]);
							break;
				case 1 :	MoveLabelPicture((*_inactive)[2]-(*markUp)[2],(*_inactive)[3]-(*markUp)[3]);
							break;
				case 2 :	MoveLabelPicture((*_inactive)[2]-(*down)[2],(*_inactive)[3]-(*down)[3]);
							break;
				case 3 :	MoveLabelPicture((*_inactive)[2]-(*markDown)[2],(*_inactive)[3]-(*markDown)[3]);
							break;
			}
		}else if(inactive){
			switch(button_state)
			{
				case 0 :	MoveLabelPicture((*up)[2]-(*inactive)[2],(*up)[3]-(*inactive)[3]);
							break;
				case 1 :	MoveLabelPicture((*markUp)[2]-(*inactive)[2],(*markUp)[3]-(*inactive)[3]);
							break;
				case 2 :	MoveLabelPicture((*down)[2]-(*inactive)[2],(*down)[3]-(*inactive)[3]);
							break;
				case 3 :	MoveLabelPicture((*markDown)[2]-(*inactive)[2],(*markDown)[3]-(*inactive)[3]);
							break;
			}
		}
		inactive = _inactive;
		EventArise(EGraphicsChanged);
	}
}

int CGUIToolBarButton::Activate()
{
	int ret=0;
	if(active)
		return ret;

	if(pictureUp && (main_state==0 || (!pictureDown && main_state==1)))
		pictureUp->SetVisible(1);
	if(pictureDown && main_state==1)
		pictureDown->SetVisible(1);
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->SetVisible(0);


	if(inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(up,(*up)[0]-(*inactive)[0],(*up)[1]-(*inactive)[1]);	
						MoveLabelPicture((*up)[2]-(*inactive)[2],(*up)[3]-(*inactive)[3]);
						break;
			case 1 :	SetStyle(markUp,(*markUp)[0]-(*inactive)[0],(*markUp)[1]-(*inactive)[1]);	
						MoveLabelPicture((*markUp)[2]-(*inactive)[2],(*markUp)[3]-(*inactive)[3]);
						break;
			case 2 :	SetStyle(down,(*down)[0]-(*inactive)[0],(*down)[1]-(*inactive)[1]);	
						MoveLabelPicture((*down)[2]-(*inactive)[2],(*down)[3]-(*inactive)[3]);
						break;
			case 3 :	SetStyle(markDown,(*markDown)[0]-(*inactive)[0],(*markDown)[1]-(*inactive)[1]);	
						MoveLabelPicture((*markDown)[2]-(*inactive)[2],(*markDown)[3]-(*inactive)[3]);
						break;
		}
	}else
		ret=1;

	active=true;
	EventArise(EStateChanged);
	return ret;
}

int CGUIToolBarButton::Deactivate()
{
	int ret=0;
	if(!active)
		return ret;

	if(pictureUp)
	{
		if(pictureInactive!=pictureUp)
			pictureUp->SetVisible(0);
		else
			pictureUp->SetVisible(1);
	}
	if(pictureDown)
		pictureDown->SetVisible(0);
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->SetVisible(1);

	if(inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(inactive,(*inactive)[0]-(*up)[0],(*inactive)[1]-(*up)[1]);	
						MoveLabelPicture((*inactive)[2]-(*up)[2],(*inactive)[3]-(*up)[3]);
						break;
			case 1 :	SetStyle(inactive,(*inactive)[0]-(*markUp)[0],(*inactive)[1]-(*markUp)[1]);	
						MoveLabelPicture((*inactive)[2]-(*markUp)[2],(*inactive)[3]-(*markUp)[3]);
						break;
			case 2 :	SetStyle(inactive,(*inactive)[0]-(*down)[0],(*inactive)[1]-(*down)[1]);	
						MoveLabelPicture((*inactive)[2]-(*down)[2],(*inactive)[3]-(*down)[3]);
						break;
			case 3 :	SetStyle(inactive,(*inactive)[0]-(*markDown)[0],(*inactive)[1]-(*markDown)[1]);	
						MoveLabelPicture((*inactive)[2]-(*markDown)[2],(*inactive)[3]-(*markDown)[3]);
						break;
		}
	}else
		ret=1;

	active=false;

	if(help)
		CGUIContextHelp::Off(this);

	EventArise(EStateChanged);
	return ret;
}


int CGUIToolBarButton::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	if(mouseState)
	{
		if(pictureDown)
		{
			pictureDown->SetVisible(1);
			pictureUp->SetVisible(0);
		}

		if(button_state==1)
		{
			SetStyle(down,(*down)[0]-(*markUp)[0],(*down)[1]-(*markUp)[1]);	// zmena stylu z markUp na down
			MoveLabelPicture((*down)[2]-(*markUp)[2],(*down)[3]-(*markUp)[3]);
		}else if(button_state==3)
		{
			SetStyle(down,(*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
			MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		}
		button_state=2;
		button_was_down=true;
	}else if(button_state==2){
		if((main_state && raisable) || !down_stable)	
		{	// zmena stavu tlacitka z down na up
			
			if(pictureDown)
			{
				pictureDown->SetVisible(0);
				pictureUp->SetVisible(1);
			}

			SetStyle(markUp, (*markUp)[0]-(*down)[0],(*markUp)[1]-(*down)[1]);	// zmena stylu z down na markUp
			MoveLabelPicture((*markUp)[2]-(*down)[2],(*markUp)[3]-(*down)[3]);
			button_state=1;
			main_state=false;
		}else if(down_stable){
			// zmena stavu tlacitka z up na down
			SetStyle(markDown, (*markDown)[0]-(*down)[0],(*markDown)[1]-(*down)[1]);	// zmena stylu z down na markDown
			MoveLabelPicture((*markDown)[2]-(*down)[2],(*markDown)[3]-(*down)[3]);
			button_state=3;
			main_state=true;
		}
	}

	if(button_was_down && !mouseState)	// ??? button_state==2
	{
		EventArise(EClicked);
		OnClick();
		button_was_down=false;
	}
/*	
	if(button_state==2 && !mouseState)
	{
		OnClick();
	}
	button_state=1+mouseState;*/

	return 0;
}

int CGUIToolBarButton::MouseOver(float x, float y, UINT over, UINT state)
{
	if(!active)
		return 0;

	if(button_state==0 || button_state==2)
	{
		if(button_was_down && (state & MK_LBUTTON))
		{
			if(button_state==0)
			{
				if(pictureDown)
				{
					pictureDown->SetVisible(1);
					pictureUp->SetVisible(0);
				}

				SetStyle(down, (*down)[0]-(*up)[0],(*down)[1]-(*up)[1]);	// zmena stylu z up na down
				MoveLabelPicture((*down)[2]-(*up)[2],(*down)[3]-(*up)[3]);
				button_state=2;
			}			
		}else
		{
			button_was_down=false;
			if(button_state==0)
			{
				SetStyle(markUp, (*markUp)[0]-(*up)[0],(*markUp)[1]-(*up)[1]);	// zmena stylu z up na markUp
				MoveLabelPicture((*markUp)[2]-(*up)[2],(*markUp)[3]-(*up)[3]);
				button_state=1;
			}else if(button_state==2)
			{
				SetStyle(markDown, (*markDown)[0]-(*down)[0],(*markDown)[1]-(*down)[1]);	// zmena stylu z down na markDown
				MoveLabelPicture((*markDown)[2]-(*down)[2],(*markDown)[3]-(*down)[3]);
				button_state=3;
			}
		}

		if(help && over && !(state & MK_LBUTTON) && !(state & MK_RBUTTON) && !(state & MK_MBUTTON))
		{
			CGUIContextHelp::On(this,help);
		}
	}

	return 0;
}

int CGUIToolBarButton::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(IsIn(x,y) || !active)
		return 0;	// mys je stale nad tlacitkem => nic nedelat

	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*markUp)[0],(*up)[1]-(*markUp)[1]);	// zmena stylu z markUp na up
		MoveLabelPicture((*up)[2]-(*markUp)[2],(*up)[3]-(*markUp)[3]);
		button_state=0;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
	}else if(button_state==2)
	{
		if(!main_state)
		{
			if(pictureDown)
			{
				pictureDown->SetVisible(0);
				pictureUp->SetVisible(1);
			}

			SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
		}
	}

	button_was_down=false;
	return 0;
}

int CGUIToolBarButton::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
//	if(IsIn(x,y))
	//	return 0;	// !@#$ mys je stale nad tlacitkem => nic nedelat

	if(!active)
		return 0;

	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*markUp)[0],(*up)[1]-(*markUp)[1]);	// zmena stylu z markUp na up
		MoveLabelPicture((*up)[2]-(*markUp)[2],(*up)[3]-(*markUp)[3]);
		button_state=0;
		button_was_down=false;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
		button_was_down=false;
	}else if(button_state==2)
	{
		if(!main_state)
		{
			if(pictureDown)
			{
				pictureDown->SetVisible(0);
				pictureUp->SetVisible(1);
			}

			SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
		}
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

int CGUIToolBarButton::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
		return 0;

	if(help)
		CGUIContextHelp::Off(this);

	if(!active)
		return 0;

	// reaguji jen na odebrani focusu
	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*markUp)[0],(*up)[1]-(*markUp)[1]);	// zmena stylu z markUp na up
		MoveLabelPicture((*up)[2]-(*markUp)[2],(*up)[3]-(*markUp)[3]);
		button_state=0;
		button_was_down=false;
	}else if(button_state==2 && !main_state)
	{
		if(pictureDown)
		{
			pictureDown->SetVisible(0);
			pictureUp->SetVisible(1);
		}
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
	}

	return 0;
}

int CGUIToolBarButton::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	if(!active)
		return 0;

	// reaguji jen na odebrani focusu
	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*markUp)[0],(*up)[1]-(*markUp)[1]);	// zmena stylu z markUp na up
		MoveLabelPicture((*up)[2]-(*markUp)[2],(*up)[3]-(*markUp)[3]);
		button_state=0;
		button_was_down=false;
	}else if(button_state==2 && !main_state)
	{
		if(pictureDown)
		{
			pictureDown->SetVisible(0);
			pictureUp->SetVisible(1);
		}
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
	}

	return 0;
}

void CGUIToolBarButton::OnClick()
{
}

void CGUIToolBarButton::SetVisible(int vis)
{
	SetVisibility(vis);
	if(st9)
		st9->SetVisible(vis);
	else if(simpleButton)
		simpleButton->SetVisible(vis);
	if(label)
		label->SetVisible(vis);

	if(active)
	{
		if(main_state==0 || (main_state==1 && !pictureDown))
		{
			if(pictureUp)
				pictureUp->SetVisible(vis);
		}else{
			if(pictureDown)
				pictureDown->SetVisible(vis);
		}
	}else{
		if(pictureInactive)
			pictureInactive->SetVisible(vis);
		
	}

}

void CGUIToolBarButton::Resize(float _sx, float _sy)
{
	if(!resizable)
		return;

	CheckResize(_sx,_sy);
	SetSize(_sx,_sy);
	if(st9)
	{
		st9->Resize(_sx,_sy);
		st9->GetSize(_sx,_sy);	// mozna zmena velikosti kvuli minimalnim rozmerum cudlitka dle jeho stylu
		SetSize(_sx,_sy);
	}else if(simpleButton){
		simpleButton->Resize(_sx,_sy);
	}
	
	SetPositionOfLabelPicture();
	switch(button_state)
	{
		case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
		case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
		case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
	}
}

void CGUIToolBarButton::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	MoveLabelPicture(_x-x,_y-y);
	SetPos(_x,_y);
	if(st9)
		st9->Move(_x,_y);
	else if(simpleButton)
		simpleButton->Move(_x,_y);
}

void CGUIToolBarButton::AddToEngine(CBltWindow *rootwnd)
{
	CGUIToolBarButton::rootwnd = rootwnd;
	if(st9)
		st9->AddToEngine(rootwnd);
	else if(simpleButton)
		simpleButton->AddToEngine(rootwnd);
	if(label)
		label->AddToEngine(rootwnd);
	if(pictureUp)
		pictureUp->AddToEngine(rootwnd);
	if(pictureDown)
	{
		pictureDown->AddToEngine(rootwnd);
		pictureDown->SetVisible(0);
	}
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->AddToEngine(rootwnd);
}

void CGUIToolBarButton::RemoveFromEngine()
{
	CGUIToolBarButton::rootwnd=0;
	if(st9)
		st9->RemoveFromEngine();
	else if(simpleButton)
		simpleButton->RemoveFromEngine();
	if(label)
		label->RemoveFromEngine();
	if(pictureUp)
		pictureUp->RemoveFromEngine();
	if(pictureDown)
		pictureDown->RemoveFromEngine();
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->RemoveFromEngine();
}

void CGUIToolBarButton::BringToTop()
{
	if(st9)
		st9->BringToTop();
	else if(simpleButton)
		simpleButton->BringToTop();
	if(label)
		label->BringToTop();
	if(pictureUp)
		pictureUp->BringToTop();
	if(pictureDown)
		pictureDown->BringToTop();
	if(pictureInactive && pictureInactive!=pictureUp)
		pictureInactive->BringToTop();
}

void CGUIToolBarButton::SetStyle(CGUIStyle *style, float dx, float dy)
{
	if(simpleButton)
	{
		CTexture *t;

		if(!style || style->GetNumTextures()<1)
		{
			throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIToolBarButton::SetStyle> Style is missing or hasn't enought textures");
		}

		t = style->GetTexture(0);
		dx = (*style)[0]-(*up)[0];
		dy = (*style)[1]-(*up)[1];
		float px, py;
		px = (*style)[4];
		py = (*style)[5];
		simpleButton->RebuildRect(t,x+px,y+py,sx+dx,sy+dy);
	}else if(st9)
		st9->SetStyle(style,dx,dy);
}

void CGUIToolBarButton::ChangeState(bool state)
{
	if(main_state==state || !active)
		return;

	if(state)
	{	// zamacknout
		if(button_state==0)
		{	// up => down
			
			if(pictureUp)
			{
				if(pictureDown)
					pictureUp->SetVisible(0);
				else
					pictureUp->SetVisible(1);
			}
			if(pictureDown)
				pictureDown->SetVisible(1);
			
			SetStyle(down,(*down)[0]-(*up)[0],(*down)[1]-(*up)[1]);	
			MoveLabelPicture((*down)[2]-(*up)[2],(*down)[3]-(*up)[3]);
			button_state=2;
		}else if(button_state==1)
		{	// upMark => downMark
			
			if(pictureUp)
			{
				if(pictureDown)
					pictureUp->SetVisible(0);
				else
					pictureUp->SetVisible(1);
			}
			if(pictureDown)
				pictureDown->SetVisible(1);

			SetStyle(markDown,(*markDown)[0]-(*markUp)[0],(*markDown)[1]-(*markUp)[1]);	
			MoveLabelPicture((*markDown)[2]-(*markUp)[2],(*markDown)[3]-(*markUp)[3]);
			button_state=3;
		}else{
			main_state=state;
		}
	}else{	// vymacknout
		if(button_state==2)
		{	// down => up

			if(pictureUp)
				pictureUp->SetVisible(1);
			if(pictureDown)
				pictureDown->SetVisible(0);

			SetStyle(up,(*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
		}else if(button_state==3)
		{	// markDown => markUp

			if(pictureUp)
				pictureUp->SetVisible(1);
			if(pictureDown)
				pictureDown->SetVisible(0);

			SetStyle(markUp,(*markUp)[0]-(*markDown)[0],(*markUp)[1]-(*markDown)[1]);	
			MoveLabelPicture((*markUp)[2]-(*markDown)[2],(*markUp)[3]-(*markDown)[3]);
			button_state=1;
		}else{
				main_state=state;
		}
	}
	main_state=state;

	if(main_state)
	{
		CGUIToolBar* tb = dynamic_cast<CGUIToolBar*>(parent);
		if(tb)
			tb->selectedTool = GetID();
	}
}

void CGUIToolBarButton::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	delete event;
}


const int CGUIToolBar::numEvents = 23;
const int CGUIToolBar::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EAddElement,EDeleteAllElements,ESetAllElementsVisible,
													EUpdateScrollbars,EFocusChanged, EMouseWheel,
													EMinimize,EMaximize,ENormalize,EClose,EDeleted,
													EKeyboard,EKeyboardChar,EKeyboardUp,EToolBar,EMessage,
													EChangeTABElement};

//////////////////////////////////////////////////////////////////////
// CGUIToolBar
//////////////////////////////////////////////////////////////////////

CGUIToolBar::CGUIToolBar(float _x, float _y, float _sx, float _sy, char *title_text, bool resizers)
	: CGUIStdWindow(_x,_y,_sx,_sy,STD_TOOLBAR_BGCOLOR)
{
	RemoveFromTrash();

	SetAvailableEvents(availableEvents,numEvents);
	float title_height=0;

	if(resizers)
		AddResizers(STD_TOOLBAR_BORDER_COLOR,styleSet->Get("Resizers"));
	else{
		AddBorders(STD_TOOLBAR_BORDER_COLOR);
		/*
		// udelat ohraniceni
		CGUILine* l;

		l = new CGUILine(0,0,sx,0,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,0,1,1);
		AddFrontElem(l);
		l = new CGUILine(0,1,sx,1,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,0,1,1);
		AddFrontElem(l);

		l = new CGUILine(sx-1,0,sx-1,sy,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,1,0,1);
		AddFrontElem(l);
		l = new CGUILine(sx-2,0,sx-2,sy,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,1,0,1);
		AddFrontElem(l);

		l = new CGUILine(sx-1,sy-1,0,sy-1,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(0,1,1,1);
		AddFrontElem(l);
		l = new CGUILine(sx-1,sy-2,0,sy-2,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(0,1,1,1);
		AddFrontElem(l);

		l = new CGUILine(0,sy-1,0,0,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,1,1,0);
		AddFrontElem(l);
		l = new CGUILine(1,sy-1,1,0,STD_TOOLBAR_BORDER_COLOR);
		l->SetAnchor(1,1,1,0);
		AddFrontElem(l);
		*/
	}

	if(title_text)
	{
		AddTitle(title_text,0,STD_TOOLBAR_TITLE_HEIGHT,true,false,false,0,STD_TOOLBAR_TITLE_LABEL_COLOR,STD_TOOLBAR_TITLE_ACTIVE_COLOR,STD_TOOLBAR_TITLE_DEACTIVE_COLOR,12);
		title_height=STD_TOOLBAR_TITLE_HEIGHT;
			/*
		CGUITitle* title = new CGUITitle(title_text,0,this,STD_TOOLBAR_TITLE_HEIGHT,1,0,0,0,STD_TOOLBAR_TITLE_LABEL_COLOR,STD_TOOLBAR_TITLE_ACTIVE_COLOR,STD_TOOLBAR_TITLE_DEACTIVE_COLOR,12);
		title->WindowIsDeactive();
		title_height=STD_TOOLBAR_TITLE_HEIGHT;
		SetViewPortBackWnd(2, 2+title_height, _sx - 2*2, _sy - 2*2-title_height);
		*/
	}	

	
	//CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-2-STD_SCROLLBAR_WIDTH, 2+title_height, STD_SCROLLBAR_WIDTH, sy - 2*2-title_height, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
	//AddSimpleScrollbar(Horizontal, 2, sy-2-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*2, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);

	SetLimitSizes(4*STD_TOOLBAR_TITLE_HEIGHT,3*STD_SCROLLBAR_WIDTH+title_height+2*2);

	num_tools_horizontal = num_tools_vertical = 0;
	tools = 0;
	one_tool_always_selected = false;
	tool_sx = tool_sy = 0;
	up = markUp = markDown = down = inactivate = 0;
	simple_button = true;
	deselect_self = false;
	selectedTool = 0;

	onWindowPanel=false;

	keyMaximalize=false;

	AddToTrash();
}

CGUIToolBar::~CGUIToolBar()
{
	if(tools)
	{
		for(int i = 0;i<num_tools_vertical;i++)
		{
			delete[] tools[i];
		}	
		delete[] tools;
	}
}

void CGUIToolBar::SetProperties(int _num_tools_horizontal, int _num_tools_vertical, int _num_tools_horizontal_visible, int _num_tools_vertical_visible, bool _one_tool_always_selected, bool _deselect_self, float _tool_sx, float _tool_sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, CGUIStyle *_inactivate, bool _simple_button)
{
	if(tools)
	{
		for(int i = 0;i<num_tools_vertical;i++)
		{
			for(int j = 0;j<num_tools_horizontal;j++)
				if(tools[i][j])
				{
					CGUIToolBarButton* tb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tools[i][j]));
					if(tb)
						DeleteBackElem(tb);
				}
			delete[] tools[i];
		}	
		delete[] tools;
		tools=0;
	}

	num_tools_horizontal = _num_tools_horizontal;
	num_tools_vertical = _num_tools_vertical;
	one_tool_always_selected = _one_tool_always_selected;
	deselect_self = _deselect_self;
	tool_sx = _tool_sx;
	tool_sy = _tool_sy;
	simple_button = _simple_button;
	up = _up; down = _down; markUp = _markUp; markDown = _markDown; inactivate = _inactivate;
	selectedTool=0;


	//tools = new typeID[num_tools_vertical][num_tools_horizontal];
	tools = new typeID*[num_tools_vertical];
	for(int i=0;i<num_tools_vertical;i++)
		tools[i] = new typeID[num_tools_horizontal];


	for(int i = 0;i<num_tools_vertical;i++)
		for(int j = 0;j<num_tools_horizontal;j++)
			(tools[i])[j]=0;

	float title_height = 0,tsx,tsy;
	if(title)
	{
		title->GetSize(tsx,tsy);
		title_height = tsy;
	}

	CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-2-STD_SCROLLBAR_WIDTH, 2+title_height, STD_SCROLLBAR_WIDTH, sy - 2*2-title_height, tool_sy, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
	AddSimpleScrollbar(Horizontal, 2, sy-2-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*2, tool_sx, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);

	//SetLimitSizes(4*STD_TOOLBAR_TITLE_HEIGHT,3*STD_SCROLLBAR_WIDTH+title_height+2*2);
	SetBWLimitSizes(0,0,num_tools_horizontal*tool_sx,num_tools_vertical*tool_sy,true,true);

	SetBackWindowNormalSize(num_tools_horizontal*tool_sx,num_tools_vertical*tool_sy);
	bw_size_x=num_tools_horizontal*(tool_sx);
	bw_size_y=num_tools_vertical*(tool_sy);
	SetWindowSizeVP(_num_tools_horizontal_visible*tool_sx,_num_tools_vertical_visible*tool_sy);
//	SetWindowSizeVP(_num_tools_horizontal_visible*tool_sx,_num_tools_vertical_visible*tool_sy);
}

void CGUIToolBar::SetFreeSize(float _sx, float _sy, float vis_sx, float vis_sy)
{
	SetBWLimitSizes(0,0,_sx,_sy,true,true);
	SetBackWindowNormalSize(_sx,_sy);
	bw_size_x=_sx;
	bw_size_y=_sy;
	SetWindowSizeVP(vis_sx,vis_sy);
}

typeID CGUIToolBar::SetTool(CGUIRectHost* pictureUp, CGUIRectHost* pictureDown, char* deskription, int pos_h, int pos_v)
{
	CGUIToolBarButton* tb;

	if(!tools)
	{
		SAFE_DELETE(pictureUp);
		SAFE_DELETE(pictureDown);
		return 0;
	}

	if(pos_h>=num_tools_horizontal || pos_v>=num_tools_vertical)
	{
		SAFE_DELETE(pictureUp);
		SAFE_DELETE(pictureDown);
		return 0;
	}

	if(tools[pos_v][pos_h])
	{
		if(selectedTool==tools[pos_v][pos_h])
			selectedTool=0;
		tb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(tools[pos_v][pos_h]));
		if(tb)
			DeleteBackElem(tb);
		tools[pos_v][pos_h]=0;
	}

	if(simple_button)
		tb = new CGUIToolBarButton(pos_h*tool_sx,pos_v*tool_sy,tool_sx,tool_sy,up,markUp,down,markDown,false,pictureUp,pictureDown);
	else
		tb = new CGUIToolBarButton(pos_h*tool_sx,pos_v*tool_sy,tool_sx,tool_sy,up,markUp,down,markDown,pictureUp,pictureDown);

	tools[pos_v][pos_h]=tb->GetID();
	tb->SetInactiveStyle(inactivate);
	tb->SetProperties(deselect_self,one_tool_always_selected);
	tb->SetPosition(pos_h,pos_v);

	tb->AcceptEvent(GetID(),EClicked);

	if(deskription)
		tb->ChangeContextHelp(deskription);

	AddBackElem(tb);

	return tb->GetID();
}

typeID CGUIToolBar::SetNextTool(CGUIRectHost* pictureUp, CGUIRectHost* pictureDown, char* deskription, int &pos_h, int &pos_v)
{
	CGUIToolBarButton* tb;

	if(!tools)
	{
		SAFE_DELETE(pictureUp);
		SAFE_DELETE(pictureDown);
		return 0;
	}

	pos_v=num_tools_vertical;
	pos_h=num_tools_horizontal;

	bool found=false;

	for(int i = 0;i<num_tools_vertical;i++)
	{
		for(int j = 0;j<num_tools_horizontal;j++)
			if((tools[i])[j]==0)
			{
				pos_v=i;
				pos_h=j;
				found = true;
				break;
			}
		if(found)
			break;
	}

	if(pos_h>=num_tools_horizontal || pos_v>=num_tools_vertical)
	{
		SAFE_DELETE(pictureUp);
		SAFE_DELETE(pictureDown);
		return 0;
	}

	if((tools[pos_v])[pos_h])
	{
		if(selectedTool==(tools[pos_v])[pos_h])
			selectedTool=0;
		tb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID((tools[pos_v])[pos_h]));
		if(tb)
			DeleteBackElem(tb);
		(tools[pos_v])[pos_h]=0;
	}

	if(simple_button)
		tb = new CGUIToolBarButton(pos_h*tool_sx,pos_v*tool_sy,tool_sx,tool_sy,up,markUp,down,markDown,false,pictureUp,pictureDown);
	else
		tb = new CGUIToolBarButton(pos_h*tool_sx,pos_v*tool_sy,tool_sx,tool_sy,up,markUp,down,markDown,pictureUp,pictureDown);

	(tools[pos_v])[pos_h]=tb->GetID();
	tb->SetInactiveStyle(inactivate);
	tb->SetProperties(deselect_self,one_tool_always_selected);
	tb->SetPosition(pos_h,pos_v);

	tb->AcceptEvent(GetID(),EClicked);

	if(deskription)
		tb->ChangeContextHelp(deskription);

	AddBackElem(tb);

	return tb->GetID();
}

typeID CGUIToolBar::SetFreeTool(CGUIRectHost* pictureUp, CGUIRectHost* pictureDown, char* deskription, float tx, float ty)
{
	CGUIToolBarButton* tb;

	if(simple_button)
		tb = new CGUIToolBarButton(tx,ty,tool_sx,tool_sy,up,markUp,down,markDown,false,pictureUp,pictureDown);
	else
		tb = new CGUIToolBarButton(tx,ty,tool_sx,tool_sy,up,markUp,down,markDown,pictureUp,pictureDown);

	tb->SetInactiveStyle(inactivate);
	tb->SetProperties(deselect_self,one_tool_always_selected);

	tb->AcceptEvent(GetID(),EClicked);

	if(deskription)
		tb->ChangeContextHelp(deskription);

	AddBackElem(tb);

	return tb->GetID();
}

CGUIToolBarButton* CGUIToolBar::GetTool(int pos_h, int pos_v)
{
	if(!tools)
	{
		return 0;
	}

	if(pos_h>=num_tools_horizontal || pos_v>=num_tools_vertical)
	{
		return 0;
	}

	return GetObjFromID<CGUIToolBarButton>(tools[pos_v][pos_h]);
}

void CGUIToolBar::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EClicked)
	{
		CGUIToolBarButton* tbb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(event->sender));
		if(tbb)
		{
			if(tbb->main_state)
			{
				if(selectedTool && selectedTool!=event->sender)
				{
					CGUIToolBarButton* sb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(selectedTool));
					if(sb)
					{
						sb->ChangeState(false);
						EventArise(EToolBar,sb->pos_h,selectedTool,0,0,sb->pos_v);
						ToolBarHandler(false,selectedTool,sb->pos_h,sb->pos_v);
					}
				}
				selectedTool=event->sender;
				EventArise(EToolBar,tbb->pos_h,event->sender,0,1,tbb->pos_v);
				ToolBarHandler(true,event->sender,tbb->pos_h,tbb->pos_v);
			}else{
				if(selectedTool && selectedTool!=event->sender)
				{
					CGUIToolBarButton* sb = dynamic_cast<CGUIToolBarButton*>(nameServer->ResolveID(selectedTool));
					if(sb)
					{
						sb->ChangeState(false);
						EventArise(EToolBar,sb->pos_h,selectedTool,0,0,sb->pos_v);
						ToolBarHandler(false,selectedTool,sb->pos_h,sb->pos_v);
					}
				}
				selectedTool=0;
				EventArise(EToolBar,tbb->pos_h,event->sender,0,0,tbb->pos_v);
				ToolBarHandler(false,event->sender,tbb->pos_h,tbb->pos_v);
			}
		}

	}

	delete event;
}


