///////////////////////////////////////////////
//
// panel.cpp
//
// Implementace pro WindowPanel a jeho casti WinPanelButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "panel.h"
#include "button.h"
#include "widgets.h"
#include "primitives.h"
#include "gui.h"
#include "scrollbar.h"
#include "window.h"

#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIWinPanelButton
//////////////////////////////////////////////////////////////////////

const int CGUIWinPanelButton::numEvents = 1;
const int CGUIWinPanelButton::availableEvents[numEvents] = {EKillMe};


CGUIWinPanelButton::CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, char *_labelString, CGUIRectHost *_picture)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	//SetAvailableEvents(availableEvents,numEvents);
	windowID=0;
	rootwnd=0;
	labelString=0;
	st9 = new CGUIStandard9(_x,_y,_up);
	simpleButton = 0;
	resizable = true;
	fullsizelabel=true;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIWinPanelButton::CGUIWinPanelButton> Style is missing");
	}
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	st9->Resize(_sx,_sy);
	st9->GetSize(_sx,_sy);	// mozna zmena velikosti kvuli minimalnim rozmerum cudlitka dle jeho stylu
	SetSize(_sx,_sy);

	if(_labelString)
	{
		int length = strlen(_labelString);
		labelString = new char[length+1];
		strcpy(labelString,_labelString);
		label = new CGUIStaticText(0,0,sx-12,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
		if(label->numChars==length)
			fullsizelabel=true;
		else
			fullsizelabel=false;
	}else{
		labelString=0;
		label=0;
	}


	picture = _picture;
	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(labelString)
	{
		help = newstrdup(labelString);
	}else
		help = 0;


	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

CGUIWinPanelButton::CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, char *_labelString, CGUIRectHost *_picture)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents), st9(0)
{
	//SetAvailableEvents(availableEvents,numEvents);
	windowID=0;
	rootwnd=0;
	labelString=0;
	resizable = _resizable;
	CTexture *t;
	UINT ssx, ssy;
	fullsizelabel=true;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIWinPanelButton::CGUIWinPanelButton> Style is missing");
	}

	if((up->GetNumTextures()<1) || (down->GetNumTextures()<1) || (markUp->GetNumTextures()<1) || (markDown->GetNumTextures()<1))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIWinPanelButton::CGUIWinPanelButton> Style hasn't enought textures");
	}

	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	t = up->GetTexture(0);
	t->GetImageSize(&ssx,&ssy);
	sx = (_sx<0) ? (float) ssx : _sx;
	sy = (_sy<0) ? (float) ssy : _sy;
	simpleButton = new CGUIRectHost(x,y,t,sx,sy);
	if(simpleButton)
		simpleButton->RemoveFromTrash();

	if(_labelString)
	{
		int length = strlen(_labelString);
		labelString = new char[length+1];
		strcpy(labelString,_labelString);
		label = new CGUIStaticText(0,0,sx-12,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
		if(label->numChars==length)
			fullsizelabel=true;
		else
			fullsizelabel=false;
	}else{
		labelString=0;
		label=0;
	}

	picture = _picture;
	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(labelString)
	{
		help = newstrdup(labelString);
	}else
		help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

CGUIWinPanelButton::CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, typeID _windowID)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents)
{
	//SetAvailableEvents(availableEvents,numEvents);
	windowID = _windowID;
	rootwnd=0;
	labelString=0;
	st9 = new CGUIStandard9(_x,_y,_up);
	simpleButton = 0;
	resizable = true;
	fullsizelabel=true;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIWinPanelButton::CGUIWinPanelButton> Style is missing");
	}
	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	st9->Resize(_sx,_sy);
	st9->GetSize(_sx,_sy);	// mozna zmena velikosti kvuli minimalnim rozmerum cudlitka dle jeho stylu
	SetSize(_sx,_sy);

	if(windowID)
	{
		CGUIWindow* window = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(windowID));
		if(window && window->title)
		{
			CGUITitle *t = window->title;
			char* tt=t->GetText();
			if(tt)
			{
				int length = strlen(tt);
				labelString = new char[length+1];
				strcpy(labelString,tt);
				label = new CGUIStaticText(0,0,sx-12,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
				if(label->numChars==length)
					fullsizelabel=true;
				else
					fullsizelabel=false;
			}else{
				labelString=0;
				label=0;
			}
			
			
			CGUIRectHost* ti = t->GetIcon();
			if(ti)
			{
				if(ti->GetTexture())
					picture = new CGUIRectHost(0,0,ti->GetTexture(),STD_WINICON_HEIGHT,STD_WINICON_HEIGHT,ti->GetColor());
				else
					picture = new CGUIRectHost(0,0,STD_WINICON_HEIGHT,STD_WINICON_HEIGHT,ti->GetColor());
			}else
				picture = 0;
		}else{
			label=0;
			picture=0;
		}

		if(window)
		{
			window->AcceptEvent(GetID(),EFocusChanged);
			window->AcceptEvent(GetID(),EMinimize);
			window->AcceptEvent(GetID(),EClose);
			window->AcceptEvent(GetID(),EDeleted);
		}
	}else{
		labelString=0;
		label=0;
		picture = 0;
	}

	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(labelString)
	{
		help = newstrdup(labelString);
	}else
		help = 0;


	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

CGUIWinPanelButton::CGUIWinPanelButton(float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_markUp, CGUIStyle *_down, CGUIStyle *_markDown, bool _resizable, typeID _windowID)
	: CGUIWidget(_x,_y), CGUIEventSupport(availableEvents, numEvents), st9(0)
{
	//SetAvailableEvents(availableEvents,numEvents);
	windowID = _windowID;
	rootwnd=0;
	labelString=0;
	resizable = _resizable;
	CTexture *t;
	UINT ssx, ssy;
	fullsizelabel=true;

	up = _up;
	down = _down;
	markUp = _markUp;
	markDown = _markDown;
	if(!up || !down || !markUp || !markDown)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIButton::CGUIButton> Style is missing");
	}

	if((up->GetNumTextures()<1) || (down->GetNumTextures()<1) || (markUp->GetNumTextures()<1) || (markDown->GetNumTextures()<1))
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUILightButton::CGUILightButton> Style hasn't enought textures");
	}

	button_state=0;
	main_state=0;
	button_was_down=false;

	MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgMouseWU|MsgMouseWD);
	MsgSetProduce(MsgNone);

	t = up->GetTexture(0);
	t->GetImageSize(&ssx,&ssy);
	sx = (_sx<0) ? (float) ssx : _sx;
	sy = (_sy<0) ? (float) ssy : _sy;
	simpleButton = new CGUIRectHost(x,y,t,sx,sy);
	if(simpleButton)
		simpleButton->RemoveFromTrash();

	if(windowID)
	{
		CGUIWindow* window = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(windowID));
		if(window && window->title)
		{
			CGUITitle *t = window->title;
			char* tt=t->GetText();
			if(tt)
			{
				int length = strlen(tt);
				labelString = new char[length+1];
				strcpy(labelString,tt);
				label = new CGUIStaticText(0,0,sx-12,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
				if(label->numChars==length)
					fullsizelabel=true;
				else
					fullsizelabel=false;
			}else{
				labelString=0;
				label=0;
			}
			CGUIRectHost* ti = t->GetIcon();
			if(ti)
			{
				if(ti->GetTexture())
					picture = new CGUIRectHost(0,0,ti->GetTexture(),STD_WINICON_HEIGHT,STD_WINICON_HEIGHT,ti->GetColor());
				else
					picture = new CGUIRectHost(0,0,STD_WINICON_HEIGHT,STD_WINICON_HEIGHT,ti->GetColor());
			}else
				picture = 0;

			t->AcceptEvent(GetID(),EStateChanged);
		}else{
			label=0;
			picture=0;
		}
		if(window)
		{
			window->AcceptEvent(GetID(),EFocusChanged);
			window->AcceptEvent(GetID(),EMinimize);
			window->AcceptEvent(GetID(),EClose);
			window->AcceptEvent(GetID(),EDeleted);
		}
	}else{
		labelString=0;
		label=0;
		picture = 0;
	}

	SetPositionOfLabelPicture();
	MoveLabelPicture((*up)[2],(*up)[3]);

	if(labelString)
	{
		help = newstrdup(labelString);
	}else
		help = 0;

	AddToTrash();
	if(label)
		label->RemoveFromTrash();
	if(picture)
		picture->RemoveFromTrash();
}

void CGUIWinPanelButton::SetLabel(char *text)
{
	CBltElem* pos=0;
	if(label)
		pos = label->GetElemPosition();
	SAFE_DELETE(label);

	if(!text)
		return;
	
	SAFE_DELETE_ARRAY(labelString);

	int length = strlen(text);
	labelString = new char[length+1];
	strcpy(labelString,text);
	label = new CGUIStaticText(0,0,sx-12,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
	if(label->numChars==length)
		fullsizelabel=true;
	else
		fullsizelabel=false;
	label->SetVisible(IsVisible());
	label->RemoveFromTrash();

	if(help)
	{
		SAFE_DELETE_ARRAY(help);
		CGUIContextHelp::Off(this);
	}
	if(labelString)
	{
		help = newstrdup(labelString);
	}else
		help = 0;

	SetPositionOfLabelPicture();
	switch(button_state)
	{
		case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
		case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
		case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
	}

	if(label && rootwnd)
	{
		if(pos)
			label->AddToEngineBackPosition(rootwnd, pos);
		else
			label->AddToEngine(rootwnd);
	}
}

void CGUIWinPanelButton::SetPicture(CGUIRectHost *_picture)
{
	if(_picture)
		_picture->RemoveFromTrash();

	CBltElem* pos=0;
	if(picture)
		pos = picture->GetElemPosition();
	SAFE_DELETE(picture);
	picture = _picture;
	if(picture)
		picture->SetVisible(IsVisible());

	SetPositionOfLabelPicture();
	switch(button_state)
	{
		case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
		case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
		case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
	}

	if(picture && rootwnd)
	{
		if(pos)
			picture->AddToEngineBackPosition(rootwnd,pos);
		else
			picture->AddToEngine(rootwnd);
	}
}

void CGUIWinPanelButton::SetPicture(char *picture_name)
{
	CBltElem* pos=0;
	if(picture)
		pos = picture->GetElemPosition();
	SAFE_DELETE(picture);
	if(!picture_name)
		return;

	picture = new CGUIRectHost(0,0,picture_name);
	if(picture)
	{
		picture->SetVisible(IsVisible());
		picture->RemoveFromTrash();
	}

	SetPositionOfLabelPicture();
	switch(button_state)
	{
		case 0 : MoveLabelPicture((*up)[2],(*up)[3]);break;
		case 1 : MoveLabelPicture((*markUp)[2],(*markUp)[3]);break;
		case 2 : MoveLabelPicture((*down)[2],(*down)[3]);break;
		case 3 : MoveLabelPicture((*markDown)[2],(*markDown)[3]);break;
	}
	if(picture && rootwnd)
	{
		if(pos)
			picture->AddToEngineBackPosition(rootwnd,pos);
		else
			picture->AddToEngine(rootwnd);
	}
}

void CGUIWinPanelButton::MoveLabelPicture(float dx, float dy)
{
	if(label)
		label->MoveRel(dx,dy);
	if(picture)
		picture->MoveRel(dx,dy);
}

void CGUIWinPanelButton::SetPositionOfLabelPicture()
{
	float pos_x, pos_y, siz_x, siz_y;	// atributy buttonu
	float p_x, p_y, p_sx, p_sy;			// atributy obrazku
	float l_x, l_y, l_sx, l_sy;			// atributy popisku

	GetPos(pos_x,pos_y);
    GetSize(siz_x,siz_y);
	
	if(label && picture)
	{
	    picture->GetSize(p_sx,p_sy);
		if(p_sx>STD_WINICON_HEIGHT && p_sy>STD_WINICON_HEIGHT)
			picture->Resize(STD_WINICON_HEIGHT,STD_WINICON_HEIGHT);
		else if(p_sx>STD_WINICON_HEIGHT)
			picture->Resize(STD_WINICON_HEIGHT,p_sy);
		else if(p_sy>STD_WINICON_HEIGHT)
			picture->Resize(p_sx,STD_WINICON_HEIGHT);

		p_x=pos_x+7;
		p_y=pos_y+6;
		picture->Move(floorf(p_x),floorf(p_y));

		label->GetSize(l_sx,l_sy);
		if(l_sx>siz_x-7-STD_WINICON_HEIGHT-6-5 || l_sy>STD_WINICON_HEIGHT || !fullsizelabel)
		{
			SAFE_DELETE(label);
			label = new CGUIStaticText(0,0,siz_x-7-STD_WINICON_HEIGHT-6-5,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
			if(label->numChars==strlen(labelString))
				fullsizelabel=true;
			else
				fullsizelabel=false;
			label->SetVisible(IsVisible());
			if(label && rootwnd)
				label->AddToEngine(rootwnd);
		}

		l_x=pos_x+7+STD_WINICON_HEIGHT+6;
		l_y=pos_y+6;
		label->Move(floorf(l_x),floorf(l_y));
	}else if(label)
	{
	    label->GetSize(l_sx,l_sy);
		if(l_sx>siz_x-7-5 || l_sy>STD_WINICON_HEIGHT || !fullsizelabel)
		{
			SAFE_DELETE(label);
			label = new CGUIStaticText(0,0,siz_x-7-5,labelString,ARIAL,STD_PANEL_TEXT_SIZE,false,aLeft,0xFFFFFFFF);
			if(label->numChars==strlen(labelString))
				fullsizelabel=true;
			else
				fullsizelabel=false;
			label->SetVisible(IsVisible());
			if(label && rootwnd)
				label->AddToEngine(rootwnd);
		}

		l_x=pos_x+7;
		l_y=pos_y+6;
		label->Move(floorf(l_x),floorf(l_y));
	}else if(picture)
	{
	    picture->GetSize(p_sx,p_sy);
		if(p_sx>STD_WINICON_HEIGHT && p_sy>STD_WINICON_HEIGHT)
			picture->Resize(STD_WINICON_HEIGHT,STD_WINICON_HEIGHT);
		else if(p_sx>STD_WINICON_HEIGHT)
			picture->Resize(STD_WINICON_HEIGHT,p_sy);
		else if(p_sy>STD_WINICON_HEIGHT)
			picture->Resize(p_sx,STD_WINICON_HEIGHT);

		p_x=pos_x+7;
		p_y=pos_y+6;
		picture->Move(floorf(p_x),floorf(p_y));
	}
}

CGUIWinPanelButton::~CGUIWinPanelButton()
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
	if(labelString)
		delete labelString;
}

int CGUIWinPanelButton::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(mouseState)
	{
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
		if(main_state)	
		{	// zmena stavu tlacitka z down na up
			SetStyle(markUp, (*markUp)[0]-(*down)[0],(*markUp)[1]-(*down)[1]);	// zmena stylu z down na markUp
			MoveLabelPicture((*markUp)[2]-(*down)[2],(*markUp)[3]-(*down)[3]);
			button_state=1;
			main_state=0;
		}else{
			// zmena stavu tlacitka z up na down
			SetStyle(markDown, (*markDown)[0]-(*down)[0],(*markDown)[1]-(*down)[1]);	// zmena stylu z down na markDown
			MoveLabelPicture((*markDown)[2]-(*down)[2],(*markDown)[3]-(*down)[3]);
			button_state=3;
			main_state=1;
		}
	}

	if(button_was_down && !mouseState)	// ??? button_state==2
	{
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

int CGUIWinPanelButton::MouseOver(float x, float y, UINT over, UINT state)
{
	if(button_state==0 || button_state==2)
	{
		if(button_was_down && (state & MK_LBUTTON))
		{
			if(button_state==0)
			{
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
			CGUIContextHelp::OnSpecific(this,help,"ARIAL",8,300);
		}
	}

	return 0;
}

int CGUIWinPanelButton::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{		// kliknuti levym tlacitkem (mouseState 0 = pusteni, 1 = stisknuti cudlitka)
	if(IsIn(x,y))
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
	}

	button_was_down=false;
	return 0;
}

int CGUIWinPanelButton::MouseOverFocus(float x, float y, UINT over, UINT state, typeID ID)
{
//	if(IsIn(x,y))
	//	return 0;	// !@#$ mys je stale nad tlacitkem => nic nedelat

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
			SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
		}
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

int CGUIWinPanelButton::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
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
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

int CGUIWinPanelButton::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	if(button_state==1)
	{
		SetStyle(up, (*up)[0]-(*markUp)[0],(*up)[1]-(*markUp)[1]);	// zmena stylu z markUp na up
		MoveLabelPicture((*up)[2]-(*markUp)[2],(*up)[3]-(*markUp)[3]);
		button_state=0;
		button_was_down=false;
	}else if(button_state==2 && !main_state)
	{
		SetStyle(up, (*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	// zmena stylu z down na up
		MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
		button_state=0;
	}else if(button_state==3)
	{
		SetStyle(down, (*down)[0]-(*markDown)[0],(*down)[1]-(*markDown)[1]);	// zmena stylu z markDown na down
		MoveLabelPicture((*down)[2]-(*markDown)[2],(*down)[3]-(*markDown)[3]);
		button_state=2;
	}

	if(help)
		CGUIContextHelp::Off(this);

	return 0;
}

void CGUIWinPanelButton::OnClick()
{
	CGUIWindow* wnd = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(windowID));
	if(!wnd)
		return;

	if(!main_state)
	{	// minimalizovat okno
		wnd->Minimize();
		//mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
	}else{
		// focusovat okno / deminimalizovat
		if(wnd->windowState==WS_Minimized)
			wnd->UnMinimize();
		else
			wnd->FocusMe();
		//mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
	}
}

void CGUIWinPanelButton::SetVisible(int vis)
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
}

void CGUIWinPanelButton::Resize(float _sx, float _sy)
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

void CGUIWinPanelButton::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	MoveLabelPicture(_x-x,_y-y);
	SetPos(_x,_y);
	if(st9)
		st9->Move(_x,_y);
	else if(simpleButton)
		simpleButton->Move(_x,_y);
}

void CGUIWinPanelButton::AddToEngine(CBltWindow *rootwnd)
{
	CGUIWinPanelButton::rootwnd = rootwnd;
	if(st9)
		st9->AddToEngine(rootwnd);
	else if(simpleButton)
		simpleButton->AddToEngine(rootwnd);
	if(label)
		label->AddToEngine(rootwnd);
	if(picture)
		picture->AddToEngine(rootwnd);
}

void CGUIWinPanelButton::RemoveFromEngine()
{
	CGUIWinPanelButton::rootwnd=0;
	if(st9)
		st9->RemoveFromEngine();
	else if(simpleButton)
		simpleButton->RemoveFromEngine();
	if(label)
		label->RemoveFromEngine();
	if(picture)
		picture->RemoveFromEngine();
}

void CGUIWinPanelButton::BringToTop()
{
	if(st9)
		st9->BringToTop();
	else if(simpleButton)
		simpleButton->BringToTop();
	if(label)
		label->BringToTop();
	if(picture)
		picture->BringToTop();
}

void CGUIWinPanelButton::SetStyle(CGUIStyle *style, float dx, float dy)
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
		simpleButton->RebuildRect(t,x,y,sx+dx,sy+dy);
	}else if(st9)
		st9->SetStyle(style,dx,dy);
}

void CGUIWinPanelButton::ChangeState(bool state)
{
	if(main_state==state)
		return;

	if(state)
	{	// zamacknout
		if(button_state==0)
		{	// up => down
			SetStyle(down,(*down)[0]-(*up)[0],(*down)[1]-(*up)[1]);	
			MoveLabelPicture((*down)[2]-(*up)[2],(*down)[3]-(*up)[3]);
			button_state=2;
		}else if(button_state==1)
		{	// upMark => downMark
			SetStyle(markDown,(*markDown)[0]-(*markUp)[0],(*markDown)[1]-(*markUp)[1]);	
			MoveLabelPicture((*markDown)[2]-(*markUp)[2],(*markDown)[3]-(*markUp)[3]);
			button_state=3;
		}else{
			main_state=state;
		}
	}else{	// vymacknout
		if(button_state==2)
		{	// down => up
			SetStyle(up,(*up)[0]-(*down)[0],(*up)[1]-(*down)[1]);	
			MoveLabelPicture((*up)[2]-(*down)[2],(*up)[3]-(*down)[3]);
			button_state=0;
		}else if(button_state==3)
		{	// markDown => markUp
			SetStyle(markUp,(*markUp)[0]-(*markDown)[0],(*markUp)[1]-(*markDown)[1]);	
			MoveLabelPicture((*markUp)[2]-(*markDown)[2],(*markUp)[3]-(*markDown)[3]);
			button_state=1;
		}else{
				main_state=state;
		}
	}
	main_state=state;
}

void CGUIWinPanelButton::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->sender==windowID)
	{
		switch(event->eventID)
		{
			case EFocusChanged:	if(event->pInt)
								{	// nabyti focusu
									ChangeState(true);
								}else{	// ztrata focusu
									ChangeState(false);
								}
								break;
			case EMinimize:		if(event->pInt)
								{	// minimalizace
									ChangeState(false);

								}else{	// deminimalizace
									ChangeState(true);
								}
								break;
			case EClose:		EventArise(EKillMe);
								break;
			case EDeleted:		EventArise(EKillMe);
								break;
		}
	}
	if(event->eventID==EStateChanged)
	{
		CGUITitle* t = dynamic_cast<CGUITitle*>(nameServer->ResolveID(event->sender));
		if(t)
		{
			CGUIRectHost* ti = t->GetIcon();
			CGUIRectHost* r=0;
			if(ti)
				r = new CGUIRectHost(*(ti));
			SetPicture(r);
		}
	}


	delete event;
}


//////////////////////////////////////////////////////////////////////
// CGUIWindowPanel
//////////////////////////////////////////////////////////////////////

CGUIWindowPanel::CGUIWindowPanel(float _x, float _y, CGUIStyle *style, CGUIWindow* _panelWindow, bool front, bool _position_up, float iconStartX, float iconEndX)
	: CGUIMultiWidget(_x,_y)
{
	RemoveFromTrash();
	position_up=_position_up;
	focusable=false;
	ctrl_pressed=false;
	selectedButtonID = originalButtonID = 0;
	SetAvailableEvents(0,0);
	float wsx=0,wsy=0;
	if(!style || style->GetNumTextures()<1)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIWindowPanel::CGUIWindowPanel> Style is missing or hasn't enought textures");
	}

	panelWindow = _panelWindow;

	if(panelWindow)
	{
		if(panelWindow == desktop)
		{
			panelWindow->GetSize(wsx,wsy);
		}else{
			if(front)
			{
				panelWindow->GetSize(wsx,wsy);
				wsx -= 2*panelWindow->bw_x;	// sirka okraju (!@#$ resit obecne)
			}else
				panelWindow->GetBackWindowNormalSize(wsx,wsy);
		}
		panelWindow->panel=this;
	}

	SetSize(wsx,STD_PANEL_HEIGHT+STD_PANEL_SHIFT);
	if(window)
	{
		window->SetWindowSize(sx, sy);
	}

	CGUIStaticPicture* panel = new CGUIStaticPicture(0,0+STD_PANEL_SHIFT,style->GetTexture(0),sx,STD_PANEL_HEIGHT);
	panel->SetAnchor(1,0,1,1);
	AddElem(panel);

	if(panelWindow)
	{
		SetAnchor(0,1,1,1);
		if(panelWindow == desktop)
		{
			desktop->AddFrontElem(this);
			desktop->vp_sy -=STD_PANEL_HEIGHT;
			desktop->bw_size_normal_y -=STD_PANEL_HEIGHT;
			desktop->bw_size_y -=STD_PANEL_HEIGHT;
		/*	// pro pridani panelu nahoru do desktopu
			SetAnchor(1,0,1,1);
			desktop->bw_y=STD_PANEL_HEIGHT;
			if(desktop->back)
			{
				CMatrix2D m;
				m.Translate(desktop->bw_x,desktop->bw_y);
				desktop->back->SetWindowTrans(m);	// move
				desktop->back->SetWindowSize(desktop->vp_sx, desktop->vp_sy);	// resize
			}//*/
		}else{
			if(front)
			{
				SetAnchor(1,0,1,1);
				panelWindow->bw_y+=STD_PANEL_HEIGHT;
				panelWindow->vp_sy -=STD_PANEL_HEIGHT;

				if(panelWindow->back)
				{
					CMatrix2D m;
					m.Translate(panelWindow->bw_x,panelWindow->bw_y);
					panelWindow->back->SetWindowTrans(m);	// move
					panelWindow->back->SetWindowSize(panelWindow->vp_sx, panelWindow->vp_sy);	// resize
				}

				// zmenseni scrollbaru pokud jsou
				CGUIElement *el=0;
				CGUIScrollBar* sb=0;
				while(el=panelWindow->GetNextFrontElement(el))
				{
					sb = dynamic_cast<CGUIScrollBar*>(el);
					if(sb && sb->scrollBarType == Vertical)
					{
						sb->ResizeRel(0,-STD_PANEL_HEIGHT);
						sb->MoveRel(0,STD_PANEL_HEIGHT);
						break;
					}
				}

				panelWindow->AddFrontElem(this);
				panelWindow->EventArise(EUpdateScrollbars);
			}else{
				full_window_anchor=true;
				panelWindow->AddBackElem(this);
			}
		}

		panelWindow->AcceptEvent(GetID(),EAddElement);
		panelWindow->AcceptEvent(GetID(),EKeyboard);
		panelWindow->AcceptEvent(GetID(),EKeyboardUp);
	}

	originalMinX = minX=iconStartX;
	maxX=sx-20;
	newX = minX;
	sizeX = STD_PANEL_BUTTON_SIZE;
	numButtons=0;
	overfull=false;

	startX = iconX = iconStartX;
	endX = iconEndX;
}

CGUIWindowPanel::~CGUIWindowPanel()
{
	if(panelWindow && panelWindow->panel==this)
		panelWindow->panel=0;
}

int CGUIWindowPanel::IsInPrecise(float _x, float _y)
{
	if(_y>=STD_PANEL_SHIFT)
		return 1;
	else
		return 0;
}

void CGUIWindowPanel::Resize(float _sx, float _sy)
{
	float dx, dy;
	GetSize(dx,dy);
	CGUIMultiWidget::Resize(_sx,_sy);	// zmena velikosti panelu
	dx = sx-dx;
	dy = sy-dy;

	maxX +=dx;
	if(maxX <= minX)
		maxX = minX+1;

	// zmena velikosti tlacitek 
	if(overfull)
	{
		float oldSize = sizeX;
		if(numButtons>0)
			sizeX = floorf((maxX-minX) / numButtons);
		else
			sizeX = MIN_PANEL_BUTTON_SIZE;
		if(sizeX < MIN_PANEL_BUTTON_SIZE)
		{
			sizeX = MIN_PANEL_BUTTON_SIZE;
			overfull=true;
			// zneviditelnit buttony, ktere se nevejdou do oblasti
		}else if(sizeX > STD_PANEL_BUTTON_SIZE)
		{
			sizeX = STD_PANEL_BUTTON_SIZE;
			overfull=false;
		}else{
			overfull=false;
		}
		CorrectVisibilityButtons();

		if(oldSize!=sizeX)
			ResizeButtons();
	}else{
		if(numButtons>0)
			sizeX = floorf((maxX-minX) / numButtons);
		else
			sizeX = STD_PANEL_BUTTON_SIZE;
		if(sizeX < MIN_PANEL_BUTTON_SIZE)
		{
			sizeX = MIN_PANEL_BUTTON_SIZE;
			overfull=true;
			// zneviditelnit buttony, ktere se nevejdou do oblasti
			CorrectVisibilityButtons();
		}
		if(sizeX > STD_PANEL_BUTTON_SIZE)
		{
			sizeX = STD_PANEL_BUTTON_SIZE;
		}
		ResizeButtons();
	}
}

void CGUIWindowPanel::AddButton(CGUIWindow* wnd)
{
	CGUIWinPanelButton* panelButton = new CGUIWinPanelButton(newX,STD_BUTTON_SHIFT+STD_PANEL_SHIFT,sizeX,STD_PANEL_BUTTON_HEIGHT,styleSet->Get("PanelButton_Up"),styleSet->Get("PanelButton_UpMark"),styleSet->Get("PanelButton_Down"),styleSet->Get("PanelButton_DownMark"),true,wnd->GetID());
	AddElem(panelButton);
	panelButton->AcceptEvent(GetID(),EKillMe);
	if(wnd->parent && wnd->parent->GetType()>=PTWindow && ((CGUIWindow*)wnd->parent)->GetFocusEl() == wnd)
		panelButton->ChangeState(true);


	newX += sizeX;
	numButtons++;

	if(overfull)
		panelButton->SetVisible(0);
	else
		if(newX>maxX)
		{
			if(numButtons>0)
				sizeX = floorf((maxX-minX) / numButtons);
			else
				sizeX = STD_PANEL_BUTTON_SIZE;
			if(sizeX < MIN_PANEL_BUTTON_SIZE)
			{
				sizeX =	floorf((maxX-minX)/floorf((maxX-minX)/MIN_PANEL_BUTTON_SIZE));
				//sizeX = MIN_PANEL_BUTTON_SIZE;
				overfull=true;
				panelButton->SetVisible(0);
			}

			ResizeButtons();
		}
}

void CGUIWindowPanel::DeleteButton(CGUIWinPanelButton* wpb)
{
	DeleteElem(wpb);
	numButtons--;

	if(overfull)
	{
		if(numButtons>0)
			sizeX = floorf((maxX-minX) / numButtons);
		else
			sizeX = MIN_PANEL_BUTTON_SIZE;
		if(sizeX < MIN_PANEL_BUTTON_SIZE)
		{
			sizeX =	floorf((maxX-minX)/floorf((maxX-minX)/MIN_PANEL_BUTTON_SIZE));
			//sizeX = MIN_PANEL_BUTTON_SIZE;
		}else if(sizeX > STD_PANEL_BUTTON_SIZE)
		{
			sizeX = STD_PANEL_BUTTON_SIZE;
			overfull=false;
		}else{
			overfull=false;
		}
		ResizeButtons();
		CorrectVisibilityButtons();
	}else{
		if(sizeX == STD_PANEL_BUTTON_SIZE)
			MoveButtons();
		else{
			if(numButtons>0)
				sizeX = floorf((maxX-minX) / numButtons);
			else
				sizeX = STD_PANEL_BUTTON_SIZE;
			if(sizeX > STD_PANEL_BUTTON_SIZE)
				sizeX = STD_PANEL_BUTTON_SIZE;
			ResizeButtons();
		}
	}
}

typeID CGUIWindowPanel::GetSelectedButton()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(e);
			if(wpb)
			{	
				if(wpb->main_state)
					return wpb->GetID();
			}
		}
		le=el_queue.GetPrevListElement(le);
	}
	
	return 0;
}

CGUIWinPanelButton* CGUIWindowPanel::GetAssociatedButton(typeID windowID)
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	if(windowID == 0)
		return 0;

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(e);
			if(wpb && wpb->windowID == windowID)
				return wpb;
		}
		le=el_queue.GetPrevListElement(le);
	}
	
	return 0;
}

void CGUIWindowPanel::ResizeButtons()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	float posX = minX;

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(e);
			if(wpb)
			{	
				wpb->Resize(sizeX,STD_PANEL_BUTTON_HEIGHT);
				wpb->Move(posX,STD_BUTTON_SHIFT+STD_PANEL_SHIFT);
				posX += sizeX;
			}
		}
		le=el_queue.GetPrevListElement(le);
	}
	newX = posX;
}

void CGUIWindowPanel::MoveButtons()
{
	CGUIElement *e;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	float posX = minX;

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(e);
			if(wpb)
			{	
				wpb->Move(posX,STD_BUTTON_SHIFT+STD_PANEL_SHIFT);
				posX += sizeX;
			}
		}
		le=el_queue.GetPrevListElement(le);
	}
	newX = posX;
}

void CGUIWindowPanel::CorrectVisibilityButtons()
{
	CGUIElement *e;
	float ex,ey;
	float sex, sey;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);

	while(le)
	{
		if((e=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(e);
			if(wpb)
			{
				wpb->GetPos(ex,ey);
				wpb->GetSize(sex,sey);
				if(ex+sex<=maxX)
					wpb->SetVisible(1);
				else
					wpb->SetVisible(0);
			}
		}
		le=el_queue.GetPrevListElement(le);
	}
}

void CGUIWindowPanel::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EAddElement)
	{
		CGUIElement* e=0;
		if(event->pID)
			e = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->pID));
		if(e && e->GetType() >=PTWindow && ((CGUIWindow*)e)->onWindowPanel)
		{
			AddButton((CGUIWindow*)e);
		}

	}else if(event->eventID == EKillMe)
	{
		CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(event->sender));
		if(wpb)
			DeleteButton(wpb);
	}

	if(event->eventID == EKeyboard)
	{
		if(event->pInt == VK_CONTROL)
			if(!ctrl_pressed)
			{
				selectedButtonID = originalButtonID = GetSelectedButton();
				ctrl_pressed = true;
			}

		if((event->pInt == VK_TAB) && ctrl_pressed && panelWindow==desktop)	// || event->pInt == VK_F6
		{
			if(panelWindow)
			{
				typeID winID=0;
				CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(selectedButtonID));
				if(wpb)
					winID = wpb->windowID;

				CGUIWinPanelButton* nextWPB=0;
				winID = panelWindow->GetNextWindowID(winID);
				if(winID==0)
					winID = panelWindow->GetNextWindowID(winID);
				while(winID)
				{
					if((nextWPB = GetAssociatedButton(winID)))
					{
						if(wpb)
							wpb->ChangeState(false);
						nextWPB->ChangeState(true);
						selectedButtonID = nextWPB->GetID();
						break;
					}
					winID = panelWindow->GetNextWindowID(winID);
				}
			}
		}

		if((event->pInt == VK_SHIFT) && ctrl_pressed && panelWindow!=desktop)
		{
			if(panelWindow)
			{
				typeID winID=0;
				CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(selectedButtonID));
				if(wpb)
					winID = wpb->windowID;

				CGUIWinPanelButton* nextWPB=0;
				winID = panelWindow->GetNextWindowID(winID);
				if(winID==0)
					winID = panelWindow->GetNextWindowID(winID);
				while(winID)
				{
					if((nextWPB = GetAssociatedButton(winID)))
					{
						if(wpb)
							wpb->ChangeState(false);
						nextWPB->ChangeState(true);
						selectedButtonID = nextWPB->GetID();
						break;
					}
					winID = panelWindow->GetNextWindowID(winID);
				}
			}
		}

		if(event->pInt == 192 && ctrl_pressed)	// ctrl+~
		{
			if(selectedButtonID!=originalButtonID)
			{
				CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(selectedButtonID));
				if(wpb)
					wpb->ChangeState(false);
				wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(originalButtonID));
				if(wpb)
					wpb->ChangeState(true);
				selectedButtonID=originalButtonID;
			}

		}


//		DebugMessage(6,"EKeyboard:%d",event->pInt);
	}

	if(event->eventID == EKeyboardUp)
	{
		if(event->pInt == VK_CONTROL && ctrl_pressed)
		{
			if(selectedButtonID!=originalButtonID)
			{
				CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(nameServer->ResolveID(selectedButtonID));
				if(wpb)
				{
					CGUIWindow* wnd = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(wpb->windowID));
					if(wnd)
						if(wnd->windowState == WS_Minimized)
							wnd->UnMinimize();
						else
							wnd->FocusMe();
				}
			}

			ctrl_pressed = false;
		}
//		DebugMessage(8,"EKeyboardUp:%d",event->pInt);
	}

	delete event;
}

CGUIButton* CGUIWindowPanel::AddIcon(CGUIStyle *up, CGUIStyle *down, CGUIStyle *mark, char *help)
{
	CGUIButton *icon=0;

	if((endX==0) || (iconX+MARK_PANEL_ICON_SIZE<endX))
	{
		icon = new CGUIButton(iconX,STD_ICON_SHIFT+STD_PANEL_SHIFT,STD_PANEL_ICON_SIZE,STD_PANEL_ICON_SIZE,up,down,mark,false,0,0,help);
		AddElem(icon);

		iconX += STD_PANEL_ICON_SIZE+STD_PANEL_ICON_GAP;

		if(iconX >= minX)	// ikony zasahuji do tlacitek task_baru, posunout tlacitka
		{	
			minX = iconX + 5;
			if(maxX <= minX)
				maxX = minX+1;

			Resize(sx,sy);	// zaridi prislusne zmeny u tlacitek na task_baru
		}
	}

	return icon;
}

int	CGUIWindowPanel::AddIcon(CGUIButton* icon, float vertical_shift)
{
	if(!icon)
		return 1;

	float ix, iy;
	icon->GetSize(ix,iy);

	if((endX==0) || (iconX+(ix+(MARK_PANEL_ICON_SIZE-STD_PANEL_ICON_SIZE)/2+1)<endX))
	{
		icon->Move(iconX,vertical_shift+STD_PANEL_SHIFT);
		AddElem(icon);

		//iconX += ix+(MARK_PANEL_ICON_SIZE-STD_PANEL_ICON_SIZE)/2+1;
		iconX += ix+5;

		if(iconX >= minX)	// ikony zasahuji do tlacitek task_baru, posunout tlacitka
		{	
			minX = iconX + 5;
			if(maxX <= minX)
				maxX = minX+1;

			Resize(sx,sy);	// zaridi prislusne zmeny u tlacitek na task_baru
		}
		return 0;
	}else
		return 1;
}

void CGUIWindowPanel::AddGap(float gap)
{
	iconX += gap;
}


void CGUIWindowPanel::DeleteIcon(typeID icon)
{
	//DeleteElem(dynamic_cast<CGUIElement*>(nameServer->ResolveID(icon)));

	CGUIElement *elem;
	CGUIListElement<CGUIElement>* le = el_queue.GetPrevListElement(0);
	bool deleted=false;
	float posun = STD_PANEL_ICON_GAP+STD_PANEL_ICON_SIZE;
	
	while(le)
	{
		if((elem=el_queue.GetObjectFromListElement(le)))
		{
			CGUIWinPanelButton* wpb = dynamic_cast<CGUIWinPanelButton*>(elem);
			if(wpb)
				le=el_queue.GetPrevListElement(le);
			else{
				CGUIButton* b = dynamic_cast<CGUIButton*>(elem);
				if(b)
				{
					if(b->GetID() == icon)
					{
						CGUIListElement<CGUIElement>* oldle=le;
						le=el_queue.GetPrevListElement(le);
						el_queue.RemoveListElement(oldle);

						if(elem){
							float esx,esy;
							elem->GetSize(esx,esy);
							posun = esx+STD_PANEL_ICON_GAP;
							elem->RemoveFromEngine();
							elem->SetParent(0);
							if(elem==last_element_under_cursor)
								last_element_under_cursor=0;
							if(elem==focus_elm)
								focus_elm=0;
							delete elem;
						}

						deleted=true;
					}else{
						if(deleted)
						{
							b->MoveRel(-posun,0);
						}
						le=el_queue.GetPrevListElement(le);
					}
				}else
					le=el_queue.GetPrevListElement(le);
			}
		}else
			le=el_queue.GetPrevListElement(le);
	}

	if(deleted)
	{
		iconX -=posun;
		if(originalMinX<minX)
		{
			minX = max(minX-posun,originalMinX);
			Resize(sx,sy);	// zaridi prislusne zmeny u tlacitek na task_baru
		}
	}
}


int CGUIWindowPanel::ForwardMsg(CGUIMessage *msg)
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

		if(refocus && msg->IsMouseButtonMsg() && msg->param && elem->focusable)	// musi byt jiny nalezeny a fokusovany element + tlacitkova zprava + tlacitko dole
		{											// po forwardu zpravy zmen focusovane okno (element)
			SetFocusEl(elem);	// nastavi element jako focusovany v okne
		}
		if(refocus && msg->IsCoordMsg() && elem->focusable)
		{
			((CGUIWindow *)elem)->BringToTop();	// nastaveni max. viditelnosti (navic od std. multiwidgetu)
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
