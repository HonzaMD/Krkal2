///////////////////////////////////////////////
//
// button.cpp
//
// Implementace pro Button
// Implementace pro CheckBox
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "button.h"
#include "primitives.h"
#include "widgets.h"
#include "refmgr.h"
#include "krkal.h"

#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIButton
//////////////////////////////////////////////////////////////////////

CGUIButtonStyle::CGUIButtonStyle(CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStyle *_disabled, CGUIFont *_font,int _simple_button)
{
	up=_up;
	down=_down;
	mark=_mark;
	disabled=_disabled;
	simple_button=_simple_button;
	font=_font;
	if(font) font->AddRef();
}

CGUIButtonStyle::~CGUIButtonStyle()
{
	if(font) font->Release();
}

const int CGUIButton::numEvents = 15;
const int CGUIButton::availableEvents[numEvents] = {ESizeChanged,EPositionChanged,EEngineStateChanged,EVisibilityChanged,
													EMouseOver,EMouseButton,EClicked,EUp,EDown,
													EFocusChanged,EDeleted,EGraphicsChanged,EKeyboard,EChangeTABElement,EStateChanged};

CGUIButton::CGUIButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label, CGUIRectHost *_picture, char *_help)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	CreateButton(_x,_y,_sx,_sy,_up,_down,_mark,_label,_picture,_help);
}

CGUIButton::CGUIButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool _resizable, CGUIStaticText *_label, CGUIRectHost *_picture, char *_help)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	CreateSimpleButton(_x,_y,_sx,_sy,_up,_down,_mark,_resizable,_label,_picture,_help);
}

CGUIButton::CGUIButton(float _x, float _y, float _sx, float _sy, char *style, char *label, CGUIRectHost *_picture, char *_help)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	CGUIButtonStyle* bs = (CGUIButtonStyle*)RefMgr->Find(style);
	if(bs)
	{
		CGUIStaticText* st=0;
		if(label && bs->font)
			st	= new CGUIStaticText(label,bs->font,0,0,bs->up->paramUINT);

		if(bs->simple_button)
			CreateSimpleButton(_x,_y,_sx,_sy,bs->up,bs->down,bs->mark,true,st,_picture,_help);
		else
			CreateButton(_x,_y,_sx,_sy,bs->up,bs->down,bs->mark,st,_picture,_help);
		if(bs->disabled)
			SetInactiveStyle(bs->disabled);
		labelFont = bs->font;
	}else{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIButton::CGUIButton> ButtonStyle is missing");
	}
}

void CGUIButton::CreateButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, CGUIStaticText *_label, CGUIRectHost *_picture, char *_help)
{
	rootwnd=0;
	keyDown=false;
	st9 = new CGUIStandard9(_x,_y,_up);
	simpleButton = 0;
	resizable = true;
	labelFont = 0;
	keySensitive=true;

	up = _up;
	down = _down;
	mark = _mark;
	inactive = 0;
	if(!up || !down || !mark)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIButton::CreateButton> Style is missing");
	}

	if((up->GetNumParameters()<4) || (down->GetNumParameters()<4) || (mark->GetNumParameters()<4))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIButton::CreateButton> Style hasn't enought parameters");
	}

	labelColorUp = up->paramUINT;
	labelColorDown = down->paramUINT;
	labelColorMark = mark->paramUINT;
	labelColorDisable = 0;

	active=true;
	button_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseR|MsgKeyboard|MsgKeyboardUp|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	st9->Resize(_sx,_sy);
	st9->GetSize(_sx,_sy);	// mozna zmena velikosti kvuli minimalnim rozmerum cudlitka dle jeho stylu
	SetSize(_sx,_sy);

	label = _label;
	picture = pictureActive = pictureInactive = _picture;
	CenterLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(_help)
	{
		help = KRKAL->CopyOutStrByLanguage(_help);
	}else
		help = 0;


	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();

}

void CGUIButton::CreateSimpleButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool _resizable, CGUIStaticText *_label, CGUIRectHost *_picture, char *_help)
{
	st9=0;
	rootwnd=0;
	keyDown=false;
	resizable = _resizable;
	CTexture *t;

	up = _up;
	down = _down;
	mark = _mark;
	inactive = 0;
	labelFont = 0;
	keySensitive=true;

	if(!up || !down || !mark)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIButton::CreateSimpleButton> Style is missing");
	}

	if((up->GetNumTextures()<1) || (down->GetNumTextures()<1) || (mark->GetNumTextures()<1))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIButton::CreateSimpleButton> Style hasn't enought textures");
	}

	if((up->GetNumParameters()<6) || (down->GetNumParameters()<6) || (mark->GetNumParameters()<6))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIButton::CreateSimpleButton> Style hasn't enought parameters");
	}

	labelColorUp = up->paramUINT;
	labelColorDown = down->paramUINT;
	labelColorMark = mark->paramUINT;
	labelColorDisable = 0;

	active=true;
	button_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseR|MsgKeyboard|MsgKeyboardUp|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	t = up->GetTexture(0);
	sx = (_sx<0) ? up->GetParametr(0) : _sx;
	sy = (_sy<0) ? up->GetParametr(1) : _sy;
	simpleButton = new CGUIRectHost(x+(*up)[4],y+(*up)[5],t,sx,sy);
	if(simpleButton)
		simpleButton->RemoveFromTrash();

	label = _label;
	picture = pictureActive = pictureInactive = _picture;
	CenterLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(_help)
	{
		help = KRKAL->CopyOutStrByLanguage(_help);
	}else
		help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

void CGUIButton::SetLabel(CGUIStaticText *_label)
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
	{
		labelColorUp = label->GetColor();
		labelColorDown = label->GetColor();
		labelColorMark = label->GetColor();
		labelColorDisable = label->GetColor();
	}

	CenterLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*mark)[2],(*mark)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
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

void CGUIButton::SetLabel(char *text, CGUIFont *font)
{
	/*
	CBltElem* pos=0;
	if(label)
		pos = label->GetElemPosition();
		*/

	if(!text)
	{
		SAFE_DELETE(label);
		return;
	}

	
	CGUIFont* f = font;
	if(!f)
		f = labelFont;
	
	if(!f)
		return;

	if(label)
		label->ChangeText(text,f);
	else
	{
		label = new CGUIStaticText(text,f,0,0,labelColorUp);
		label->RemoveFromTrash();
		if(rootwnd)
			label->AddToEngine(rootwnd);
	}

	CenterLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*mark)[2],(*mark)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	/*
	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
	*/
}

void CGUIButton::SetLabelColors(UINT _labelColorUp, UINT _labelColorMark, UINT _labelColorDown, UINT _labelColorDisable)
{
	labelColorUp = _labelColorUp;
	labelColorMark = _labelColorMark;
	labelColorDown = _labelColorDown;
	labelColorDisable = _labelColorDisable;
}

void CGUIButton::SetPicture(CGUIRectHost *_picture)
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
		CenterLabelPicture();
	}else{
		if(picture==pictureActive)
		{
			if(picture)	
				pos = picture->GetElemPosition();
			SAFE_DELETE(picture);
			picture = _picture;
			pictureInactive = _picture;
			CenterLabelPicture();
		}else
			SAFE_DELETE(pictureActive);
	}

	pictureActive = _picture;
	
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*mark)[2],(*mark)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		}
	}else
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);

	if((active || picture==pictureInactive) && picture && rootwnd)
	{
		if(pos)
			picture->AddToEngineBackPosition(rootwnd,pos);
		else
			picture->AddToEngine(rootwnd);
	}
}

void CGUIButton::SetPicture(char *picture_name)
{
	CGUIRectHost* _picture=0;

	if(picture_name)
		_picture= new CGUIRectHost(0,0,picture_name);
	else
		_picture = 0;
	
	SetPicture(_picture);
}

void CGUIButton::ReStylize(CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark)
{
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(_up,(*_up)[0]-(*up)[0],(*_up)[1]-(*up)[1]);	// zmena stylu z up na _up
						MoveLabelPicture((*_up)[2]-(*up)[2],(*_up)[3]-(*up)[3]);
						break;
			case 1 :	SetStyle(_mark,(*_mark)[0]-(*mark)[0],(*_mark)[1]-(*mark)[1]);	// zmena stylu z up na _up
						MoveLabelPicture((*_mark)[2]-(*mark)[2],(*_mark)[3]-(*mark)[3]);
						break;
			case 2 :	SetStyle(_down,(*_down)[0]-(*down)[0],(*_down)[1]-(*down)[1]);	// zmena stylu z up na _up
						MoveLabelPicture((*_down)[2]-(*down)[2],(*_down)[3]-(*down)[3]);
						break;
		}
	}
	up=_up;down=_down;mark=_mark;
	EventArise(EGraphicsChanged);
}

void CGUIButton::ChangeContextHelp(char* text)
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

void CGUIButton::MoveLabelPicture(float dx, float dy)
{
	if(label)
		label->MoveRel(dx,dy);
	if(picture)
		picture->MoveRel(dx,dy);
}

void CGUIButton::CenterLabelPicture()
{
	float pos_x, pos_y, siz_x, siz_y;	// atributy buttonu
	float p_x, p_y, p_sx, p_sy;			// atributy obrazku
	float l_x, l_y, l_sx, l_sy;			// atributy popisku

	GetPos(pos_x,pos_y);
    GetSize(siz_x,siz_y);
	
	if(label && picture)
	{
	    picture->GetSize(p_sx,p_sy);
	    label->GetSize(l_sx,l_sy);

		p_x=pos_x+ceilf(2*(siz_x-p_sx-l_sx)/5);
		p_y=pos_y+(siz_y-p_sy)/2;
		l_x=pos_x+ p_sx + floorf(3*(siz_x-p_sx-l_sx)/5);
		l_y=pos_y+(siz_y-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));
		picture->Move(floorf(p_x),floorf(p_y));
	}else if(label)
	{
	    label->GetSize(l_sx,l_sy);

		l_x=pos_x+(siz_x-l_sx)/2;
		l_y=pos_y+(siz_y-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));
	}else if(picture)
	{
	    picture->GetSize(p_sx,p_sy);

		p_x=pos_x+(siz_x-p_sx)/2;
		p_y=pos_y+(siz_y-p_sy)/2;
		picture->Move(floorf(p_x),floorf(p_y));
	}
}

CGUIButton::~CGUIButton()
{
	RemoveFromTrash();
	if(st9)
		delete st9;
	if(simpleButton)
		delete simpleButton;
	if(label)
		delete label;
	if(picture)
		delete picture;
	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	EventArise(EDeleted,0,0,this);
}

void CGUIButton::SetInactiveStyle(CGUIStyle *_inactive)
{
	if(active)
	{
		inactive = _inactive;
	}else{
		if(inactive && _inactive)
		{
			SetStyle(_inactive,(*_inactive)[0]-(*inactive)[0],(*_inactive)[1]-(*inactive)[1]);	// zmena stylu z puvodniho inactive na novy _inactive
			MoveLabelPicture((*_inactive)[2]-(*inactive)[2],(*_inactive)[3]-(*inactive)[3]);
		}else if(_inactive){
			switch(button_state)
			{
				case 0 :	MoveLabelPicture((*_inactive)[2]-(*up)[2],(*_inactive)[3]-(*up)[3]);
							break;
				case 1 :	MoveLabelPicture((*_inactive)[2]-(*mark)[2],(*_inactive)[3]-(*mark)[3]);
							break;
				case 2 :	MoveLabelPicture((*_inactive)[2]-(*down)[2],(*_inactive)[3]-(*down)[3]);
							break;
			}
		}else if(inactive){
			switch(button_state)
			{
				case 0 :	MoveLabelPicture((*up)[2]-(*inactive)[2],(*up)[3]-(*inactive)[3]);
							break;
				case 1 :	MoveLabelPicture((*mark)[2]-(*inactive)[2],(*mark)[3]-(*inactive)[3]);
							break;
				case 2 :	MoveLabelPicture((*down)[2]-(*inactive)[2],(*down)[3]-(*inactive)[3]);
							break;
			}
		}
		inactive = _inactive;
		EventArise(EGraphicsChanged);
	}

	if(inactive)
		labelColorDisable = inactive->paramUINT;
}

void CGUIButton::SetInactivePicture(CGUIRectHost *_pictureInactive)
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

		CenterLabelPicture();
		
		if(!inactive)
		{
			switch(button_state)
			{
				case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
				case 1 : MoveLabelPicture((*mark)[2],(*mark)[3]);break;
				case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
			}
		}else
			MoveLabelPicture((*inactive)[2],(*inactive)[3]);

		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}
}

int CGUIButton::Activate()
{
	int ret=0;
	CBltElem* pos=0;
	if(active)
		return ret;

	keyDown=false;

	if(pictureActive!=picture)
	{
		if(picture)
		{
			pos = picture->GetElemPosition();
			picture->RemoveFromEngine();
		}
		picture = pictureActive;
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}

	if(inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(up,(*up)[0]-(*inactive)[0],(*up)[1]-(*inactive)[1]);	
						MoveLabelPicture((*up)[2]-(*inactive)[2],(*up)[3]-(*inactive)[3]);
						break;
			case 1 :	SetStyle(mark,(*mark)[0]-(*inactive)[0],(*mark)[1]-(*inactive)[1]);	
						MoveLabelPicture((*mark)[2]-(*inactive)[2],(*mark)[3]-(*inactive)[3]);
						break;
			case 2 :	SetStyle(down,(*down)[0]-(*inactive)[0],(*down)[1]-(*inactive)[1]);	
						MoveLabelPicture((*down)[2]-(*inactive)[2],(*down)[3]-(*inactive)[3]);
						break;
		}
	}else
		ret=1;

	active=true;
	EventArise(EStateChanged);
	return ret;
}

int CGUIButton::Deactivate()
{
	int ret=0;
	CBltElem* pos=0;
	if(!active)
		return ret;

	keyDown=false;

	if(pictureInactive!=picture)
	{
		if(picture)
		{
			pos = picture->GetElemPosition();
			picture->RemoveFromEngine();
		}
		picture = pictureInactive;
		if(picture && rootwnd)
		{
			if(pos)
				picture->AddToEngineBackPosition(rootwnd,pos);
			else
				picture->AddToEngine(rootwnd);
		}
	}

	if(inactive)
	{
		switch(button_state)
		{
			case 0 :	SetStyle(inactive,(*inactive)[0]-(*up)[0],(*inactive)[1]-(*up)[1]);	
						MoveLabelPicture((*inactive)[2]-(*up)[2],(*inactive)[3]-(*up)[3]);
						break;
			case 1 :	SetStyle(inactive,(*inactive)[0]-(*mark)[0],(*inactive)[1]-(*mark)[1]);	
						MoveLabelPicture((*inactive)[2]-(*mark)[2],(*inactive)[3]-(*mark)[3]);
						break;
			case 2 :	SetStyle(inactive,(*inactive)[0]-(*down)[0],(*inactive)[1]-(*down)[1]);	
						MoveLabelPicture((*inactive)[2]-(*down)[2],(*inactive)[3]-(*down)[3]);
						break;
		}
	}else
		ret=1;

	active=false;
	EventArise(EStateChanged);
	return ret;
}

int CGUIButton::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active || keyDown)
		return 0;
	if(mouseState)
	{
		SetStyle(down,(*down)[0]-(*mark)[0],(*down)[1]-(*mark)[1]);	// zmena stylu z mark na down
		MoveLabelPicture((*down)[2]-(*mark)[2],(*down)[3]-(*mark)[3]);
		EventArise(EMouseButton,0,mouseState);
		EventArise(EDown);
		EventArise(EStateChanged);
	}else if(button_state==2){
		SetStyle(mark, (*mark)[0]-(*down)[0],(*mark)[1]-(*down)[1]);	// zmena stylu z down na mark
		MoveLabelPicture((*mark)[2]-(*down)[2],(*mark)[3]-(*down)[3]);
		EventArise(EMouseButton,0,mouseState);
		EventArise(EUp);
		EventArise(EStateChanged);
	}

	if(button_state==2 && !mouseState)
	{
		EventArise(EClicked);
		OnClick();
	}
	button_state=1+mouseState;
	return 0;
}

int CGUIButton::MouseOver(float x, float y, UINT over, UINT state)
{
	if(!active || keyDown)
		return 0;

	if(!button_state)
	{
		if(button_was_down && (state & MK_LBUTTON))
		{
			SetStyle(down, (*down)[0]-(*up)[0],(*down)[1]-(*up)[1]);	// zmena stylu z up na down
			MoveLabelPicture((*down)[2]-(*up)[2],(*down)[3]-(*up)[3]);
			button_state=2;
			EventArise(EMouseOver);
			EventArise(EDown);
		}else
		{
			SetStyle(mark, (*mark)[0]-(*up)[0],(*mark)[1]-(*up)[1]);	// zmena stylu z up na mark
			MoveLabelPicture((*mark)[2]-(*up)[2],(*mark)[3]-(*up)[3]);
			button_state=1;
			EventArise(EMouseOver);
		}

		if(help && over && !(state & MK_LBUTTON) && !(state & MK_RBUTTON) && !(state & MK_MBUTTON))
		{
			CGUIContextHelp::On(this,help);
		}
	}

	return 0;
}

int CGUIButton::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
		
	

//	if(IsIn(x,y))
//		return 0;	// mys je stale nad tlacitkem => nic nedelat

	if(!active || keyDown)
		return 0;

	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*mark)[0],(*up)[1]-(*mark)[1]);	// zmena stylu z mark na up
		MoveLabelPicture((*up)[2]-(*mark)[2],(*up)[3]-(*mark)[3]);
		button_state=0;
		//EventArise(EMouseButton,0,mouseState);
		EventArise(EStateChanged);
	}else if(button_state==2)
	{
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
		//EventArise(EMouseButton,0,mouseState);
		EventArise(EUp);
		EventArise(EStateChanged);
	}else
		EventArise(EStateChanged);


	button_was_down=false;
	return 0;
}

int CGUIButton::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
//	if(IsIn(x,y))
	//	return 0;	// !@#$ mys je stale nad tlacitkem => nic nedelat
	
	if(help)
		CGUIContextHelp::Off(this);

	if(!active || keyDown)
		return 0;

	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*mark)[0],(*up)[1]-(*mark)[1]);	// zmena stylu z mark na up
		MoveLabelPicture((*up)[2]-(*mark)[2],(*up)[3]-(*mark)[3]);
		button_state=0;
		button_was_down=false;
		EventArise(EMouseOver);
	}else if(button_state==2)
	{
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
		button_was_down=true;
		EventArise(EMouseOver);
	}

	return 0;
}

int CGUIButton::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		EventArise(EFocusChanged,focus);
		return 0;
	}

	if(active)
	{
		// reaguji jen na odebrani focusu
		if(button_state==1)
		{
			SetStyle(up, (*up)[0]-(*mark)[0],(*up)[1]-(*mark)[1]);	// zmena stylu z mark na up
			MoveLabelPicture((*up)[2]-(*mark)[2],(*up)[3]-(*mark)[3]);
			button_state=0;
			button_was_down=false;
		}else if(button_state==2)
		{
			SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
			button_was_down=false;
		}
	}

	if(help)
		CGUIContextHelp::Off(this);

	keyDown=false;

	EventArise(EFocusChanged,focus);
	return 0;
}

int CGUIButton::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(active)
	{
		// reaguji jen na odebrani focusu
		if(button_state==1)
		{
			SetStyle(up, (*up)[0]-(*mark)[0],(*up)[1]-(*mark)[1]);	// zmena stylu z mark na up
			MoveLabelPicture((*up)[2]-(*mark)[2],(*up)[3]-(*mark)[3]);
			button_state=0;
			button_was_down=false;
		}else if(button_state==2)
		{
			SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
			button_was_down=true;
		}
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

void CGUIButton::Click()
{
	EventArise(EStateChanged);
	EventArise(EClicked);
	OnClick();
}

void CGUIButton::OnClick()
{
}


void CGUIButton::ParentNotification()
{
	if(parent)
	{
		CGUIEventSupport *es = dynamic_cast<CGUIEventSupport*>(parent);
		if(es)
			AcceptEvent(es->GetID(),EChangeTABElement);
	}
}

int CGUIButton::Keyboard(UINT character, UINT state)
{
	if(!active || !keySensitive)
		return 0;

	if((character == VK_RETURN || character == VK_SPACE) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(!keyDown)
		{
			keyDown=true;

			if(button_state==0)
			{
				SetStyle(down,(*down)[0]-(*up)[0],(*down)[1]-(*up)[1]);	// zmena stylu z up na down
				MoveLabelPicture((*down)[2]-(*up)[2],(*down)[3]-(*up)[3]);
			}
			if(button_state==1)
			{
				SetStyle(down,(*down)[0]-(*mark)[0],(*down)[1]-(*mark)[1]);	// zmena stylu z mark na down
				MoveLabelPicture((*down)[2]-(*mark)[2],(*down)[3]-(*mark)[3]);
			}

			if(button_state!=2)
			{
				EventArise(EDown);
				EventArise(EStateChanged);
			}

			button_state=2;
		}
	}


	if(character == VK_UP && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		EventArise(EChangeTABElement,-1);
	}

	if(character == VK_DOWN && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		EventArise(EChangeTABElement,1);
	}

	EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	return 0;
}

int CGUIButton::KeyboardUp(UINT character, UINT state)
{
	if(!active  || !keySensitive)
		return 0;

	if((character == VK_RETURN || character == VK_SPACE) && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		if(keyDown)
		{
			keyDown=false;
			if(button_state==2)
			{
				SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
				MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
				button_state=0;
			}
		
			EventArise(EUp);
			EventArise(EStateChanged);

			mainGUI->SendCursorPos();

			if(character == VK_RETURN)
			{
				//EventArise(EChangeTABElement,1);
				
				/*
				CGUIWindow* wnd = 0;
				if(parent && parent->GetType()>=PTWindow)
					wnd = (CGUIWindow*) parent;
				if(wnd)
				{
					CGUIElement* el = wnd->FindNextTABElement(GetTabOrder(),back_element ? false : true);
					if(el)
					{
						wnd->SetFocusEl(el,el->back_element ? 0 : 1);
					}
				}
				*/
			}

			OnClick();
			EventArise(EClicked);
		}
	}

	EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	return 0;
}

int CGUIButton::MouseRight(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti pravym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active || keyDown)
		return 0;

	EventArise(EMouseButton,1,mouseState);
	
	return 0;
}


void CGUIButton::SetVisible(int vis)
{
	SetVisibility(vis);
	if(st9)
		st9->SetVisible(vis);
	else if(simpleButton)
		simpleButton->SetVisible(vis);
	if(label)
		label->SetVisible(vis);
	if(picture)
		picture->SetVisible(vis);
	EventArise(EVisibilityChanged);

}

void CGUIButton::Resize(float _sx, float _sy)
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
	
	CenterLabelPicture();
	if(active || !inactive)
	{
		switch(button_state)
		{
			case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
			case 1 : MoveLabelPicture((*mark)[2],(*mark)[3]);break;
			case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		}
	}else{
		MoveLabelPicture((*inactive)[2],(*inactive)[3]);
	}
	EventArise(ESizeChanged);
}

void CGUIButton::Move(float _x, float _y)
{
	float ox,oy;
	GetPos(ox,oy);
	CheckPosition(_x,_y);
	MoveLabelPicture(_x-x,_y-y);
	SetPos(_x,_y);
	if(st9)
		st9->Move(_x,_y);
	else if(simpleButton)
	{
		simpleButton->MoveRel(_x-ox,_y-oy);
	}
	EventArise(EPositionChanged);
}

void CGUIButton::AddToEngine(CBltWindow *rootwnd)
{
	CreateMarkStructures(rootwnd);
	CGUIButton::rootwnd = rootwnd;
	if(st9)
		st9->AddToEngine(rootwnd);
	else if(simpleButton)
		simpleButton->AddToEngine(rootwnd);
	if(label)
		label->AddToEngine(rootwnd);
	if(picture)
		picture->AddToEngine(rootwnd);
	EventArise(EEngineStateChanged);
}

void CGUIButton::RemoveFromEngine()
{
	CGUIButton::rootwnd=0;
	if(st9)
		st9->RemoveFromEngine();
	else if(simpleButton)
		simpleButton->RemoveFromEngine();
	if(label)
		label->RemoveFromEngine();
	if(picture)
		picture->RemoveFromEngine();
	EventArise(EEngineStateChanged);
}

void CGUIButton::BringToTop()
{
	if(st9)
		st9->BringToTop();
	else if(simpleButton)
		simpleButton->BringToTop();
	if(label)
		label->BringToTop();
	if(picture)
		picture->BringToTop();
	EventArise(EVisibilityChanged);
}

void CGUIButton::SetStyle(CGUIStyle *style, float dx, float dy)
{
	if(simpleButton)
	{
		CTexture *t;

		if(!style || style->GetNumTextures()<1)
		{
			throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIButton::SetStyle> Style is missing or hasn't enought textures");
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

	if(label)
	{
		if(style==up)
			label->SetColor(labelColorUp);
		if(style==down)
			label->SetColor(labelColorDown);
		if(style==mark)
			label->SetColor(labelColorMark);
		if(style==inactive)
			label->SetColor(labelColorDisable);
	}

	EventArise(EGraphicsChanged);
}


//////////////////////////////////////////////////////////////////////
// CGUICheckBox
//////////////////////////////////////////////////////////////////////

const int CGUICheckBox::numEvents = 3;
const int CGUICheckBox::availableEvents[numEvents] = {EClicked,EDeleted,EChangeTABElement};


CGUICheckBox::CGUICheckBox(float _x, float _y, CGUIStaticText *_label, int _main_state, bool uncheckable, UINT color, CGUIStyle *_check, CGUIStyle *_uncheck)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents), CGUIDataExchange()
{
	SetElementAttributes(&main_state,dtInt);
	raisable=uncheckable;
	rootwnd=0;

	CTexture *t;
	UINT ssx, ssy;

	active=true;
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD|MsgKeyboard);
	MsgSetProduce(MsgNone);

	label=_label;

	pictureCheck = new CGUIRectHost(0,0,_check->GetTexture(0),-1,-1,color);
	pictureCheck->SetVisible(0);
	pictureUnCheck = new CGUIRectHost(0,0,_uncheck->GetTexture(0),-1,-1,color);
	pictureInactiveCheck=0;
	pictureInactiveUnCheck=0;
	SetInactiveStyle(_check,_uncheck);
	pictureCheck->RemoveFromTrash();
	pictureUnCheck->RemoveFromTrash();

	t = _check->GetTexture(0);
	t->GetImageSize(&ssx,&ssy);

	if(label)
	{
		float lsx,lsy;
		label->GetSize(lsx,lsy);
		ssx+=(UINT)lsx+5;
		ssy = max((UINT)lsy,ssy);
		label->RemoveFromTrash();
	}


	sx =  (float)ssx;
	sy =  (float)ssy;

	SetPositionOfLabelPicture();
	help = 0;

	AddToTrash();

	ChangeState(_main_state);
}

CGUICheckBox::CGUICheckBox(float _x, float _y, char* text, int _main_state, bool uncheckable, UINT color, CGUIStyle *_check, CGUIStyle *_uncheck)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents), CGUIDataExchange()
{
	SetElementAttributes(&main_state,dtInt);
	raisable=uncheckable;
	rootwnd=0;

	CTexture *t;
	UINT ssx, ssy;

	active=true;
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD|MsgKeyboard);
	MsgSetProduce(MsgNone);

	if(text)
	{
		label= new CGUIStaticText(text,(CGUIFont*)RefMgr->Find("GUI.F.Arial.12"),0,0,0xFFFFFFFF);
	}

	pictureCheck = new CGUIRectHost(0,0,_check->GetTexture(0),-1,-1,color);
	pictureCheck->SetVisible(0);
	pictureUnCheck = new CGUIRectHost(0,0,_uncheck->GetTexture(0),-1,-1,color);
	pictureInactiveCheck=0;
	pictureInactiveUnCheck=0;
	SetInactiveStyle(_check,_uncheck);
	pictureCheck->RemoveFromTrash();
	pictureUnCheck->RemoveFromTrash();

	t = _check->GetTexture(0);
	t->GetImageSize(&ssx,&ssy);

	if(label)
	{
		float lsx,lsy;
		label->GetSize(lsx,lsy);
		ssx+=(UINT)lsx+5;
		ssy = max((UINT)lsy,ssy);
		label->RemoveFromTrash();
	}


	sx =  (float)ssx;
	sy =  (float)ssy;

	SetPositionOfLabelPicture();
	help = 0;

	AddToTrash();

	ChangeState(_main_state);
}


void CGUICheckBox::SetLabel(CGUIStaticText *_label)
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

	float ssx=0, ssy=0;
	if(pictureCheck)
	{
		pictureCheck->GetSize(ssx,ssy);
	}
	if(label)
	{
		float lsx,lsy;
		label->GetSize(lsx,lsy);
		ssx+=lsx+5;
		ssy = max(lsy,ssy);
	}
	sx =  ssx;
	sy =  ssy;

	if(label)
		label->SetVisible(IsVisible());

	SetPositionOfLabelPicture();

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd,pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUICheckBox::SetInactiveStyle(CGUIStyle *_check, CGUIStyle *_uncheck, UINT color)
{
	CBltElem* pos=0;

	if(pictureInactiveCheck)
		pos = pictureInactiveCheck->GetElemPosition();
	else if(pictureCheck)
		pos = pictureCheck->GetElemPosition();

	SAFE_DELETE(pictureInactiveCheck);
	SAFE_DELETE(pictureInactiveUnCheck);


	pictureInactiveCheck = new CGUIRectHost(0,0,_check->GetTexture(0),-1,-1,color);
	pictureInactiveUnCheck = new CGUIRectHost(0,0,_check->GetTexture(0),-1,-1,color);

	if(pictureInactiveCheck)
		pictureInactiveCheck->RemoveFromTrash();
	if(pictureInactiveUnCheck)
		pictureInactiveUnCheck->RemoveFromTrash();

	if(pictureInactiveCheck)
		if(IsVisible() && !active)
			pictureInactiveCheck->SetVisible(1);
		else
			pictureInactiveCheck->SetVisible(0);
	if(pictureInactiveUnCheck)
		if(IsVisible() && !active)
			pictureInactiveUnCheck->SetVisible(1);
		else
			pictureInactiveUnCheck->SetVisible(0);
	
	SetPositionOfLabelPicture();

	if(pictureInactiveCheck && pictureInactiveUnCheck && rootwnd)
	{
		if(pos)
			pictureInactiveCheck->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureInactiveCheck->AddToEngine(rootwnd);

		if(pos)
			pictureInactiveUnCheck->AddToEngineBackPosition(rootwnd,pos);
		else
			pictureInactiveUnCheck->AddToEngine(rootwnd);
	}
}

void CGUICheckBox::SetPositionOfLabelPicture()
{
	float pos_x, pos_y, siz_x, siz_y;	// atributy buttonu
	float p_x, p_y, p_sx, p_sy;			// atributy obrazku
	float l_x, l_y, l_sx, l_sy;			// atributy popisku

	GetPos(pos_x,pos_y);
    GetSize(siz_x,siz_y);
	
	if(pictureCheck)
	{
	    pictureCheck->GetSize(p_sx,p_sy);
		p_x=pos_x;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureCheck->Move(floorf(p_x),floorf(p_y));
	}
	if(pictureUnCheck)
	{
	    pictureUnCheck->GetSize(p_sx,p_sy);
		p_x=pos_x;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureUnCheck->Move(floorf(p_x),floorf(p_y));
	}
	if(pictureInactiveCheck)
	{
	    pictureInactiveCheck->GetSize(p_sx,p_sy);
		p_x=pos_x;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureInactiveCheck->Move(floorf(p_x),floorf(p_y));
	}
	if(pictureInactiveUnCheck)
	{
	    pictureInactiveUnCheck->GetSize(p_sx,p_sy);
		p_x=pos_x;
		p_y=pos_y+(siz_y-p_sy)/2;
		pictureInactiveUnCheck->Move(floorf(p_x),floorf(p_y));
	}
	if(label)
	{
		label->GetSize(l_sx,l_sy);
		l_x=p_x+p_sx+5;
		l_y=pos_y+(siz_y-l_sy)/2;
		label->Move(floorf(l_x),floorf(l_y));	
	}
}

CGUICheckBox::~CGUICheckBox()
{
	RemoveFromTrash();
	if(label)
		delete label;
	if(pictureCheck)
		delete pictureCheck;
	if(pictureUnCheck)
		delete pictureUnCheck;
	if(pictureInactiveCheck)
		delete pictureInactiveCheck;
	if(pictureInactiveUnCheck)
		delete pictureInactiveUnCheck;
	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	EventArise(EDeleted);
}

void CGUICheckBox::ChangeContextHelp(char* text)
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


int CGUICheckBox::Activate()
{
	if(active)
		return 0;

	if(pictureCheck)
	{
		if(main_state)
			pictureCheck->SetVisible(1);
		else
			pictureCheck->SetVisible(0);
	}
	if(pictureUnCheck)
	{
		if(main_state)
			pictureUnCheck->SetVisible(0);
		else
			pictureUnCheck->SetVisible(1);
	}
	if(pictureInactiveCheck)
	{
		pictureInactiveCheck->SetVisible(0);
	}
	if(pictureInactiveUnCheck)
	{
		pictureInactiveUnCheck->SetVisible(0);
	}

	active=true;
	EventArise(EStateChanged);
	return 0;
}

int CGUICheckBox::Deactivate()
{
	if(!active)
		return 0;


	if(pictureCheck)
	{
		pictureCheck->SetVisible(0);
	}
	if(pictureUnCheck)
	{
		pictureUnCheck->SetVisible(0);
	}
	if(pictureInactiveCheck)
	{
		if(main_state)
			pictureInactiveCheck->SetVisible(1);
		else
			pictureInactiveCheck->SetVisible(0);
	}
	if(pictureInactiveUnCheck)
	{
		if(main_state)
			pictureInactiveUnCheck->SetVisible(1);
		else
			pictureInactiveUnCheck->SetVisible(0);
	}

	active=false;
	EventArise(EStateChanged);
	return 0;
}


int CGUICheckBox::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(!active)
		return 0;

	if(mouseState)
	{
		button_state=2;
		button_was_down=true;
	}else if(button_state==2){
		if((main_state && raisable))	
		{	// zmena stavu tlacitka z down na up
			if(pictureCheck)
				pictureCheck->SetVisible(0);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(1);

			button_state=1;
			main_state=0;
		}else{
			// zmena stavu tlacitka z up na down
			if(pictureCheck)
				pictureCheck->SetVisible(1);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(0);
			button_state=3;
			main_state=1;
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

int CGUICheckBox::MouseOver(float x, float y, UINT over, UINT state)
{
	if(!active)
		return 0;

	if(button_state==0 || button_state==2)
	{
		if(button_was_down && (state & MK_LBUTTON))
		{
			if(button_state==0)
			{
				if(pictureCheck)
					pictureCheck->SetVisible(0);
				if(pictureUnCheck)
					pictureUnCheck->SetVisible(1);
				button_state=2;
			}			
		}else
		{
			button_was_down=false;
			if(button_state==0)
			{
				button_state=1;
			}else if(button_state==2)
			{
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

int CGUICheckBox::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(IsIn(x,y) || !active)
		return 0;	// mys je stale nad tlacitkem => nic nedelat

	if(button_state==1)
	{
		button_state=0;
	}else if(button_state==3)
	{
		button_state=2;
	}else if(button_state==2)
	{
		if(!main_state)
		{
			if(pictureCheck)
				pictureCheck->SetVisible(0);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(1);
			button_state=0;
		}
	}

	button_was_down=false;
	return 0;
}

int CGUICheckBox::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
//	if(IsIn(x,y))
	//	return 0;	// !@#$ mys je stale nad tlacitkem => nic nedelat

	if(!active)
		return 0;

	if(button_state==1)
	{
		button_state=0;
		button_was_down=false;
	}else if(button_state==3)
	{
		button_state=2;
		button_was_down=false;
	}else if(button_state==2)
	{
		if(!main_state)
		{
			if(pictureCheck)
				pictureCheck->SetVisible(0);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(1);

			button_state=0;
		}
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

int CGUICheckBox::FocusChanged(UINT focus, typeID dualID)
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
		button_state=0;
		button_was_down=false;
	}else if(button_state==2 && !main_state)
	{
		if(pictureCheck)
			pictureCheck->SetVisible(0);
		if(pictureUnCheck)
			pictureUnCheck->SetVisible(1);
		button_state=0;
	}else if(button_state==3)
	{
		button_state=2;
	}

	return 0;
}

int CGUICheckBox::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(help)
		CGUIContextHelp::Off(this);

	if(!active)
		return 0;

	// reaguji jen na odebrani focusu
	if(button_state==1)
	{
		button_state=0;
		button_was_down=false;
	}else if(button_state==2 && !main_state)
	{
		if(pictureCheck)
			pictureCheck->SetVisible(0);
		if(pictureUnCheck)
			pictureUnCheck->SetVisible(1);
		button_state=0;
	}else if(button_state==3)
	{
		button_state=2;
	}

	return 0;
}

int CGUICheckBox::SetParametr(void *value, int type, int size, int which)
{
	if((type==dtInt) && (size==1) && (which==0))
	{
		ChangeState(*((int*)value));
		return 0;
	}else
		return CGUIDataExchange::SetParametr(value,type,size,which);
}

void CGUICheckBox::ParentNotification()
{
	if(parent)
	{
		CGUIEventSupport *es = dynamic_cast<CGUIEventSupport*>(parent);
		if(es)
			AcceptEvent(es->GetID(),EChangeTABElement);
	}
}

int CGUICheckBox::Keyboard(UINT character, UINT state)
{
	if(!active)
		return 0;

	if(character == VK_RETURN  && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ChangeState(!main_state);
		OnClick();
		EventArise(EChangeTABElement,1);
		EventArise(EClicked);
	}

	if(character == VK_SPACE  && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		ChangeState(!main_state);
		OnClick();
		EventArise(EClicked);
	}

	if(character == VK_UP  && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		EventArise(EChangeTABElement,-1);
	}

	if(character == VK_DOWN  && !mainGUI->key_ctrl && !mainGUI->key_alt && !mainGUI->key_shift)
	{
		EventArise(EChangeTABElement,1);
	}

	EventArise(EKeyboard,character,state,0,0,mainGUI->key_ctrl,mainGUI->key_alt,mainGUI->key_shift);
	return 0;
}


void CGUICheckBox::OnClick()
{
}

void CGUICheckBox::SetVisible(int vis)
{
	SetVisibility(vis);
	if(label)
		label->SetVisible(vis);

	if(active)
	{
		if(main_state)
		{
			if(pictureCheck)
				pictureCheck->SetVisible(vis);
		}else{
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(vis);
		}
	}else{
		if(main_state)
		{
			if(pictureInactiveCheck)
				pictureInactiveCheck->SetVisible(vis);
		}else{
			if(pictureInactiveUnCheck)
				pictureInactiveUnCheck->SetVisible(vis);
		}
	}
}

void CGUICheckBox::Resize(float _sx, float _sy)
{
//	CheckResize(_sx,_sy);
//	SetSize(_sx,_sy);
}

void CGUICheckBox::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	SetPos(_x,_y);
	SetPositionOfLabelPicture();
	/*
	if(label)
		label->Move(_x,_y);
	if(pictureCheck)
		pictureCheck->Move(_x,_y);
	if(pictureUnCheck)
		pictureUnCheck->Move(_x,_y);
	if(pictureInactiveCheck)
		pictureInactiveCheck->Move(_x,_y);
	if(pictureInactiveUnCheck)
		pictureInactiveUnCheck->Move(_x,_y);
		*/
}

void CGUICheckBox::AddToEngine(CBltWindow *rootwnd)
{
	CGUICheckBox::rootwnd = rootwnd;
	if(label)
		label->AddToEngine(rootwnd);
	if(pictureCheck)
		pictureCheck->AddToEngine(rootwnd);
	if(pictureUnCheck)
		pictureUnCheck->AddToEngine(rootwnd);
	if(pictureInactiveCheck)
		pictureInactiveCheck->AddToEngine(rootwnd);
	if(pictureInactiveUnCheck)
		pictureInactiveUnCheck->AddToEngine(rootwnd);
}

void CGUICheckBox::RemoveFromEngine()
{
	CGUICheckBox::rootwnd=0;
	if(label)
		label->RemoveFromEngine();
	if(pictureCheck)
		pictureCheck->RemoveFromEngine();
	if(pictureUnCheck)
		pictureUnCheck->RemoveFromEngine();
	if(pictureInactiveCheck)
		pictureInactiveCheck->RemoveFromEngine();
	if(pictureInactiveUnCheck)
		pictureInactiveUnCheck->RemoveFromEngine();
}

void CGUICheckBox::BringToTop()
{
	if(label)
		label->BringToTop();
	if(pictureCheck)
		pictureCheck->BringToTop();
	if(pictureUnCheck)
		pictureUnCheck->BringToTop();
	if(pictureInactiveCheck)
		pictureInactiveCheck->BringToTop();
	if(pictureInactiveUnCheck)
		pictureInactiveUnCheck->BringToTop();
}

void CGUICheckBox::ChangeState(int state)
{
	if(main_state==state)
		return;

	if(active)
	{
		if(state)
		{	// zamacknout
			if(pictureCheck)
				pictureCheck->SetVisible(1);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(0);
		}else{	// vymacknout
			if(pictureCheck)
				pictureCheck->SetVisible(0);
			if(pictureUnCheck)
				pictureUnCheck->SetVisible(1);
		}
	}else{
		if(state)
		{	// zamacknout
			if(pictureInactiveCheck)
				pictureInactiveCheck->SetVisible(1);
			if(pictureInactiveUnCheck)
				pictureInactiveUnCheck->SetVisible(0);
		}else{	// vymacknout
			if(pictureInactiveCheck)
				pictureInactiveCheck->SetVisible(0);
			if(pictureInactiveUnCheck)
				pictureInactiveUnCheck->SetVisible(1);
		}
	}
	main_state=state;
}

void CGUICheckBox::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	delete event;
}

