///////////////////////////////////////////////
//
// window.cpp
//
// Implementace pro std. okno a jeho std. elementy 
//	- Titulek, Resizery
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "window.h"
#include "primitives.h"
#include "button.h"
#include "widgets.h"
#include "scrollbar.h"
#include "panel.h"

#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIStdWindow
//////////////////////////////////////////////////////////////////////

CGUIStdWindow::CGUIStdWindow(float _x, float _y, float _sx, float _sy, char *title_text, CGUIRectHost *_icon, bool resizable, UINT resizer_color, int withBackground, int bgColor)
	: CGUIWindow(_x,_y,_sx,_sy,withBackground,bgColor)
{
	RemoveFromTrash();
	type=PTStdWindow;

	if(resizable)
		AddResizers(resizer_color,styleSet->Get("Resizers"));
	else
		AddBorders(resizer_color);

	AddTitle(title_text,_icon,STD_TITLE_HEIGHT);
	CGUIStyle &style = *(styleSet->Get("Resizers"));

//	CGUIScrollBar* dual = AddScrollbar(Vertical, sx-style[1]-STD_SCROLLBAR_WIDTH, style[0]+STD_TITLE_HEIGHT, STD_SCROLLBAR_WIDTH, sy - 2*style[0]-STD_TITLE_HEIGHT, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
//	AddScrollbar(Horizontal, style[1], sy-style[0]-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*style[1], STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);
	
	CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-style[1]-STD_SCROLLBAR_WIDTH, style[0]+STD_TITLE_HEIGHT, STD_SCROLLBAR_WIDTH, sy - 2*style[0]-STD_TITLE_HEIGHT, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
	//CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-style[1]-STD_SCROLLBAR_WIDTH, style[0]+STD_TITLE_HEIGHT+STD_PANEL_HEIGHT, STD_SCROLLBAR_WIDTH, sy - 2*style[0]-STD_TITLE_HEIGHT-STD_PANEL_HEIGHT, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
	AddSimpleScrollbar(Horizontal, style[1], sy-style[0]-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*style[1], STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);
	//SetLimitSizes(3*STD_SCROLLBAR_WIDTH+2*style[1],3*STD_SCROLLBAR_WIDTH+STD_TITLE_HEIGHT+2*style[0]);

	SetLimitSizes(4*STD_TITLE_HEIGHT,3*STD_SCROLLBAR_WIDTH+STD_TITLE_HEIGHT+2*style[0]);
	AddToTrash();
}

CGUIStdWindow::CGUIStdWindow(float _x, float _y, float _sx, float _sy, char *title_text, CBltRect *_background, CGUIRectHost *_icon, bool resizable, UINT resizer_color)
	: CGUIWindow(_x,_y,_sx,_sy,_background)
{
	RemoveFromTrash();
	type=PTStdWindow;

	if(resizable)
		AddResizers(resizer_color,styleSet->Get("Resizers"));
	else
		AddBorders(resizer_color);

	AddTitle(title_text,_icon,STD_TITLE_HEIGHT);
	CGUIStyle &style = *(styleSet->Get("Resizers"));

	CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, sx-style[1]-STD_SCROLLBAR_WIDTH, style[0]+STD_TITLE_HEIGHT, STD_SCROLLBAR_WIDTH, sy - 2*style[0]-STD_TITLE_HEIGHT, STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,0,0,0);
	AddSimpleScrollbar(Horizontal, style[1], sy-style[0]-STD_SCROLLBAR_WIDTH, STD_SCROLLBAR_WIDTH, sx - 2*style[1], STD_SCROLLBAR_SHIFT, STD_SCROLLBAR_AUTOHIDE, STD_SCROLLBAR_SLIDER_RESIZE,dual,0,0);
	SetLimitSizes(4*STD_TITLE_HEIGHT,3*STD_SCROLLBAR_WIDTH+STD_TITLE_HEIGHT+2*style[0]);

	AddToTrash();
}

CGUIStdWindow::CGUIStdWindow(float _x, float _y, float _sx, float _sy, int bgColor)
	: CGUIWindow(_x,_y,_sx,_sy,1,bgColor)
{
	type=PTStdWindow;
}

void CGUIStdWindow::AddResizers(UINT color,CGUIStyle *style)
{
	CGUIResizer *res;

	if(!style)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIStdWindow::AddResizers> Style is missing");
	}

	res = new CGUIResizer(UpperLeft,this,style,color);
	res = new CGUIResizer(Up,this,style,color);
	res = new CGUIResizer(UpperRight,this,style,color);
	res = new CGUIResizer(Left,this,style,color);
	res = new CGUIResizer(Right,this,style,color);
	res = new CGUIResizer(BottomLeft,this,style,color);
	res = new CGUIResizer(Bottom,this,style,color);
	res = new CGUIResizer(BottomRight,this,style,color);

	SetViewPortBackWnd((*style)[1], (*style)[0], sx - 2*(*style)[1], sy - 2*(*style)[0]);
}

void CGUIStdWindow::AddTitle(char *title, CGUIRectHost *_icon, float height, bool _closeButton, bool _minimizeButton, bool _maximizeButton, CTexture* texture, int labelColor, int bgActiveColor, int bgDeactiveColor, int text_height)
{
	CGUITitle* t = new CGUITitle(title,_icon,this,height,_closeButton,_minimizeButton,_maximizeButton,texture,labelColor,bgActiveColor,bgDeactiveColor,text_height);
	t->WindowIsDeactive();
	SetViewPortBackWnd(bw_x, bw_y+height, vp_sx, vp_sy - height);
}

void CGUIStdWindow::AddBorders(UINT color)
{
	CGUILine* l;

	l = new CGUILine(0,0,sx,0,color);
	l->SetAnchor(1,0,1,1);
	AddFrontElem(l);
	l = new CGUILine(0,1,sx,1,color);
	l->SetAnchor(1,0,1,1);
	AddFrontElem(l);

	l = new CGUILine(sx-1,0,sx-1,sy,color);
	l->SetAnchor(1,1,0,1);
	AddFrontElem(l);
	l = new CGUILine(sx-2,0,sx-2,sy,color);
	l->SetAnchor(1,1,0,1);
	AddFrontElem(l);

	l = new CGUILine(sx-1,sy-1,0,sy-1,color);
	l->SetAnchor(0,1,1,1);
	AddFrontElem(l);
	l = new CGUILine(sx-1,sy-2,0,sy-2,color);
	l->SetAnchor(0,1,1,1);
	AddFrontElem(l);

	l = new CGUILine(0,sy-1,0,0,color);
	l->SetAnchor(1,1,1,0);
	AddFrontElem(l);
	l = new CGUILine(1,sy-1,1,0,color);
	l->SetAnchor(1,1,1,0);
	AddFrontElem(l);

	SetViewPortBackWnd(2, 2, sx - 2*2, sy - 2*2);
}

void CGUIStdWindow::AddScrollbarsSimple(float width,float shift, bool autohide, bool slider_resize,char* styleNameVertical[4][2], CGUIRectHost* picturesVertical[3],char* styleNameHorizontal[4][2], CGUIRectHost* picturesHorizontal[3])
{
	CGUIScrollBar* dual = AddSimpleScrollbar(Vertical, bw_x+vp_sx-width, bw_y, width, vp_sy, shift, autohide, slider_resize,0,styleNameVertical,picturesVertical);
	AddSimpleScrollbar(Horizontal, bw_x,bw_y+vp_sy-width, width, vp_sx, shift, autohide, slider_resize,dual,styleNameHorizontal,picturesHorizontal);
}

CGUIWindowPanel* CGUIStdWindow::AddWindowPanel(CGUIStyle *style, float iconStartX, float iconEndX)
{
	CGUIWindowPanel* winpanel = new CGUIWindowPanel(bw_x,bw_y-STD_PANEL_SHIFT,style,this,1,1,iconStartX,iconEndX); 
	bw_size_normal_y -=STD_PANEL_HEIGHT;
	bw_size_y=bw_size_normal_y;
	return winpanel;
}

void CGUIStdWindow::SetViewPortBackWnd(float _x, float _y, float _sx, float _sy)
{
	bw_size_normal_x=_sx;	bw_size_normal_y=_sy;	
	bw_size_max_x=0;	bw_size_max_y=0;	// max. velikost std. neomezena
	bw_size_x=bw_size_normal_x;	bw_size_y=bw_size_normal_y;		
	// aktualni velikost back window nastavena na standardni vel.
	bw_x=_x; bw_y=_y;		// poloha (pocatek) back window v souradne soustave okna (front window)
	vp_x=0; vp_y=0;		// poloha viewportu do back window (v souradnicich bw)
	vp_sx=_sx; vp_sy=_sy;		// velikost viewportu do back window 
	// nastavena na celou velikost bw (zaroven velikost materskeho okna)
	vp_min_sx=0; vp_min_sy=0;	// minimalni velikost view portu (neomezena)
	if(back)
	{
		CMatrix2D m;
		m.Translate(bw_x,bw_y);
		back->SetWindowTrans(m);	// move
		back->SetWindowSize(vp_sx, vp_sy);	// resize
	}
}

CGUIScrollBar* CGUIStdWindow::AddScrollbar(enum EScrollBarTypes sb_type, float sb_x, float sb_y, float width, float length, float shift, bool autohide, bool slider_resize, CGUIScrollBar* dual, char* styleName[3][2], CGUIRectHost* pictures[3])
{
	CGUIScrollBar* scrollbar=0;
	CGUIRectHost* default_pictures[3];
	default_pictures[0]=0;default_pictures[1]=0;default_pictures[2]=0;

	try{
		char *ScrollBarStyleDefaultName[3][2] = { {"SB_ButtonUp_Up","SB_ButtonUp_Down"},
												{"SB_ButtonPageUp_Up","SB_ButtonPageUp_Down"},
												{"SB_ButtonSlider_Up","SB_ButtonSlider_Down"} };//*/

		if(!styleName)
			styleName = ScrollBarStyleDefaultName;

		if(!pictures)
		{
			switch(sb_type)
			{
				case Vertical :
					default_pictures[0] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(0));
					default_pictures[1] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(2));
					default_pictures[2] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(1));
					break;
				case Horizontal :
					default_pictures[0] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(3));
					default_pictures[1] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(5));
					default_pictures[2] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(4));
					break;
			}
			pictures = default_pictures;
		}

		scrollbar = new CGUIScrollBar(sb_type, sb_x, sb_y, length, width, shift, autohide, slider_resize, this, styleName, pictures, dual);

		/*
		switch(sb_type)
		{
			case Vertical	: sb_v = scrollbar;break;
			case Horizontal : sb_h = scrollbar;break;
		}*/

	}catch(CExc)
	{
		if(!scrollbar)
		{
			SAFE_DELETE(default_pictures[0]);
			SAFE_DELETE(default_pictures[1]);
			SAFE_DELETE(default_pictures[2]);
		}
		SAFE_DELETE(scrollbar);
		throw;
	}
	return scrollbar;
}

CGUIScrollBar* CGUIStdWindow::AddSimpleScrollbar(enum EScrollBarTypes sb_type, float sb_x, float sb_y, float width, float length, float shift, bool autohide, bool slider_resize, CGUIScrollBar* dual, char* styleName[4][2], CGUIRectHost* pictures[3])
{
	CGUIScrollBar* scrollbar=0;
	CGUIRectHost* default_pictures[3];
	default_pictures[0]=0;default_pictures[1]=0;default_pictures[2]=0;
	char* ScrollBarStyleDefaultName[4][2];
	for(int i=0;i<4;i++)
		for(int j=0;j<2;j++)
			ScrollBarStyleDefaultName[i][j] =  new char[30];

	try{
		switch(sb_type)
		{
			case Vertical :
				strcpy(ScrollBarStyleDefaultName[0][0],"SSB_ButtonUp_Up");
				strcpy(ScrollBarStyleDefaultName[0][1],"SSB_ButtonUp_Down");
				strcpy(ScrollBarStyleDefaultName[1][0],"SSB_ButtonPageUp_Up");
				strcpy(ScrollBarStyleDefaultName[1][1],"SSB_ButtonPageUp_Down");
				if(slider_resize)
				{
					strcpy(ScrollBarStyleDefaultName[2][0],"SB_ButtonSlider_Up");
					strcpy(ScrollBarStyleDefaultName[2][1],"SB_ButtonSlider_Down");
				}else{
					strcpy(ScrollBarStyleDefaultName[2][0],"SSB_ButtonSliderV_Up");
					strcpy(ScrollBarStyleDefaultName[2][1],"SSB_ButtonSliderV_Down");
				}
				strcpy(ScrollBarStyleDefaultName[3][0],"SSB_ButtonDown_Up");
				strcpy(ScrollBarStyleDefaultName[3][1],"SSB_ButtonDown_Down");

				if(!styleName)
					styleName = ScrollBarStyleDefaultName;

				if(!pictures)
				{
					if(slider_resize)
						default_pictures[2] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(1));
					pictures = default_pictures;
				}

				scrollbar = new CGUIScrollBar(sb_type, sb_x, sb_y, length, width, shift, autohide, slider_resize, this, ScrollBarStyleDefaultName, pictures, dual,true);

				//sb_v = scrollbar;
				break;

			case Horizontal :
				strcpy(ScrollBarStyleDefaultName[0][0],"SSB_ButtonLeft_Up");
				strcpy(ScrollBarStyleDefaultName[0][1],"SSB_ButtonLeft_Down");
				strcpy(ScrollBarStyleDefaultName[1][0],"SSB_ButtonPageLeft_Up");
				strcpy(ScrollBarStyleDefaultName[1][1],"SSB_ButtonPageLeft_Down");
				if(slider_resize)
				{
					strcpy(ScrollBarStyleDefaultName[2][0],"SB_ButtonSlider_Up");
					strcpy(ScrollBarStyleDefaultName[2][1],"SB_ButtonSlider_Down");
				}else{
					strcpy(ScrollBarStyleDefaultName[2][0],"SSB_ButtonSliderH_Up");
					strcpy(ScrollBarStyleDefaultName[2][1],"SSB_ButtonSliderH_Down");
				}
				strcpy(ScrollBarStyleDefaultName[3][0],"SSB_ButtonRight_Up");
				strcpy(ScrollBarStyleDefaultName[3][1],"SSB_ButtonRight_Down");

				if(!styleName)
					styleName = ScrollBarStyleDefaultName;

				if(!pictures)
				{
					if(slider_resize)
						default_pictures[2] = new CGUIRectHost(0,0,styleSet->Get("SB_Pictures")->GetTexture(4));
					pictures = default_pictures;
				}

				scrollbar = new CGUIScrollBar(sb_type, sb_x, sb_y, length, width, shift, autohide, slider_resize, this, styleName, pictures, dual,true);

				//sb_h = scrollbar;
				break;
		}
	}catch(CExc)
	{
		if(!scrollbar)
		{
			SAFE_DELETE(default_pictures[0]);
			SAFE_DELETE(default_pictures[1]);
			SAFE_DELETE(default_pictures[2]);
		}
		SAFE_DELETE(scrollbar);
		for(int i=0;i<4;i++)
			for(int j=0;j<2;j++)
				delete ScrollBarStyleDefaultName[i][j];
		throw;
	}
	for(int i=0;i<4;i++)
			for(int j=0;j<2;j++)
				delete ScrollBarStyleDefaultName[i][j];
	return scrollbar;
}

/*
int CGUIStdWindow::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
	{
		if(title)
			title->WindowIsActive();
	}else{
		if(title)
			title->WindowIsDeactive();
	}
	return 1;	// zprava se forwarduje eventualnim dalsim zajemcum (podelementum okna)
}
*/

/*
int CGUIStdWindow::Wheel(float x, float y, int dw, int dz, UINT state)
{
	if(sb_v && sb_v->active)
	{
		SetWindowPositionVP(0,-dz*STD_SCROLLBAR_MWHEEL_COEF*sb_v->shift,false);
		sb_v->RebuildScrollbar();
		
	}else if(sb_h && sb_h->active)
	{
		SetWindowPositionVP(-dz*STD_SCROLLBAR_MWHEEL_COEF*sb_h->shift,0,false);
		sb_h->RebuildScrollbar();
	}

	return 1;
}
*/

/*
void CGUIStdWindow::AddBackElem(CGUIElement *element)
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
				cx = ex+esx+1;
				change=true;
			}else
				cx = bw_size_normal_x;

			if(ey+esy >= bw_size_normal_y)
			{
				cy = ey+esy+1;
				change = true;
			}else
				cy = bw_size_normal_y;

			if(change)
			{
				SetBackWindowNormalSize(cx,cy);
				
				if(sb_v)
					sb_v->RebuildScrollbar();
				if(sb_h)
					sb_h->RebuildScrollbar();
			//SetWindowSizeBWNormal();
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

	}
}
*/


//////////////////////////////////////////////////////////////////////
// CGUIResizer
//////////////////////////////////////////////////////////////////////

CGUIResizer::CGUIResizer(enum EResizerTypes res_type, CGUIWindow *wnd, CGUIStyle *style, UINT color)
	: CGUIWidget(0,0)
{
	timerReq=false;
	cursor_type=0;
	resizerType=res_type;	
	float wx,wy;
	rect = 0;

	if(!style || style->GetNumTextures()<8)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIResizer::CGUIResizer> Style hasn't enought textures");
	}

	if(!wnd)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIResizer::CGUIResizer> Parent window is not defined");
	}

	if(style->GetNumParameters()<3)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIResizer::CGUIResizer> Style hasn't enought parameters");
	}

	width_h = style->GetParametr(0);
	width_v = style->GetParametr(1);
	corner = style->GetParametr(2);
	
	wnd->GetSize(wx,wy);
	
	try{
		switch(resizerType)
		{
			case UpperLeft	:	x=0; y=0; sx=corner; sy=corner; 
								rect = new CGUIRectHost(x,y,style->GetTexture(0),sx,sy,color);
								SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FIX,WIDGET_FREE);
								break;
			case Up			:	x=corner; y=0; sx=wx-2*corner; sy=width_h;
								rect = new CGUIRectHost(x,y,style->GetTexture(1),sx,sy,color);
								SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FIX,WIDGET_FIX);
								break;
			case UpperRight	:	x=wx-corner; y=0; sx=corner; sy=corner;
								rect = new CGUIRectHost(x,y,style->GetTexture(2),sx,sy,color);
								SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE,WIDGET_FIX);
								break;
			case Right		:	x=wx-width_v; y=corner; sx=width_h; sy=wy-2*corner;
								rect = new CGUIRectHost(x,y,style->GetTexture(3),sx,sy,color);
								SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
								break;
			case Left		:	x=0; y=corner; sx=width_v; sy=wy-2*corner;
								rect = new CGUIRectHost(x,y,style->GetTexture(7),sx,sy,color);
								SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FIX,WIDGET_FREE);
								break;
			case BottomLeft	:	x=0; y=wy-corner; sx=corner; sy=corner;
								rect = new CGUIRectHost(x,y,style->GetTexture(6),sx,sy,color);
								SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FIX,WIDGET_FREE);
								break;
			case Bottom		:	x=corner; y=wy-width_h; sx=wx-2*corner; sy=width_h;
								rect = new CGUIRectHost(x,y,style->GetTexture(5),sx,sy,color);
								SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FIX,WIDGET_FIX);
								break;
			case BottomRight:	x=wx-corner; y=wy-corner; sx=corner; sy=corner;
								rect = new CGUIRectHost(x,y,style->GetTexture(4),sx,sy,color);
								SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
								break;
		}

		MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgFocusChange|MsgTimer|MsgMouseWU|MsgMouseWD);
		MsgSetProduce(MsgNone);
		mouse_button_down=false;
		cursor_resizer=false;
		px=py=0;

		wnd->AddFrontElem(this);
		rect->RemoveFromTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		throw;
	}
}


int CGUIResizer::IsInPrecise(float _x, float _y)
{
	switch(resizerType)
	{
		case UpperLeft	: if(_x<width_v || _y<width_h) return 1; else return 0;
		case UpperRight	: if(_x>corner-width_v-1 || _y<width_h) return 1; else return 0;
		case BottomLeft	: if(_x<width_v || _y>corner-width_h-1) return 1; else return 0;
		case BottomRight: if(_x>corner-width_v-1 || _y>corner-width_h-1) return 1; else return 0;
		default			: return 1;
	}
}

CGUIResizer::~CGUIResizer()
{
	timerGUI->DeleteAllRequests(this);
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	SAFE_DELETE(rect);
}

void CGUIResizer::SetVisible(int vis)
{
	SetVisibility(vis);
	if(rect)
		rect->SetVisible(vis);
}

void CGUIResizer::Resize(float _sx, float _sy)
{
	CheckResize(_sx,_sy);
	if(rect)
		rect->Resize(_sx,_sy);
	SetSize(_sx,_sy);
}

void CGUIResizer::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	SetPos(_x,_y);
	if(rect)
		rect->Move(_x,_y);
}

void CGUIResizer::AddToEngine(CBltWindow *rootwnd)
{
	if(rect)
		rect->AddToEngine(rootwnd);
}

void CGUIResizer::RemoveFromEngine()
{
	if(rect)
		rect->RemoveFromEngine();
}

void CGUIResizer::BringToTop()
{
	if(rect)
		rect->BringToTop();
}


int CGUIResizer::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	if(mouseState)
	{			// po stisknuti tlacitka zacinam menit velikost okna (zaznamenavam zmenu souradnic kurzoru)
		mouse_button_down=true;
		px=x;
		py=y;
	}else if(mouse_button_down){		// po pusteni tlacitka dokoncim zmenu velikosti okna
		ChangeWindowSize(x,y);

		mouse_button_down=false;
		px=py=0;
		
		if(parent && parent->GetType() >= PTWindow)
		{
			((CGUIWindow*)parent)->FocusOldTOBackEl();
		}
	}
	return 0;
}

int CGUIResizer::MouseOver(float x, float y, UINT over,UINT state)
{
	// zmenit kurzor na sipky (roztahovak)
	if(!cursor_resizer && !(state & MK_LBUTTON))
	{
		cursor_type = mainGUI->cursor->GetCursorIndex();
		cursor_resizer=true;
		if(resizerType==UpperLeft || resizerType==BottomRight)
			mainGUI->cursor->Set(1);	// !@#$ do budoucna menit na kurzor nastaveny dle stylu resizeru
		else if(resizerType==UpperRight || resizerType==BottomLeft)
			mainGUI->cursor->Set(3);
		else if(resizerType==Up || resizerType==Bottom)
			mainGUI->cursor->Set(2);
		else if(resizerType==Left || resizerType==Right)
			mainGUI->cursor->Set(4);

		mainGUI->cursor->SetVisible(1);
	}

	if(!(state & MK_LBUTTON))
		mouse_button_down=false;
	
	// kontrola na vyjezd s oknem mimo oblast materskeho okna
	if(parent && parent->GetType()>=PTWindow && parent->parent && parent->parent->GetType()>=PTWindow && mouse_button_down)
	{
		CGUIWindow* wnd = (CGUIWindow*) parent;
		CGUIWindow* par = (CGUIWindow*) parent->parent;

		float wx, wy;
		float wsx, wsy;
		float cx, cy;
		wnd->GetPos(wx,wy);
		wnd->GetSize(wsx,wsy);
		if(wnd->back_element)
		{
			cx = wx+x-px;
			cy = wy+y-py;
			//if(cx<par->vp_x || cy<par->vp_y || cx>par->vp_sx || cy>par->vp_sy)
//				return 0;	// nehybat s oknem, jsem mimo oblast otcovskeho okna
			
/*			if(cx<par->vp_x)
				x +=par->vp_x-cx;
			if(cy<par->vp_y)
				y +=par->vp_y-cy;
			if(cx>par->vp_sx+par->vp_x)
				x -=cx-par->vp_sx-par->vp_x;
			if(cy>par->vp_sy+par->vp_y)
				y -=cy-par->vp_sy-par->vp_y;
//*/
			if((resizerType==UpperLeft || resizerType==Left || resizerType==BottomLeft)
				&& (cx<par->vp_x))
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
			}
			if((resizerType==UpperLeft || resizerType==Up || resizerType==UpperRight)
				&& (cy<par->vp_y))
			{
				if(par->vp_y>0)
				{
					par->SetWindowPositionVP(0,cy-par->vp_y,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				y = par->vp_y-wy+py;
			}

			if((resizerType==UpperRight || resizerType==Right || resizerType==BottomRight)
				&& (cx+wsx>par->vp_sx+par->vp_x))
			{
				if(par->vp_x+par->vp_sx < par->bw_size_x)
				{
					par->SetWindowPositionVP(cx+px-par->vp_x-par->vp_sx,0,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				x = par->vp_x + par->vp_sx - wx - wsx + px;
			}
			if((resizerType==BottomLeft || resizerType==Bottom || resizerType==BottomRight)
				&& (cy+wsy>par->vp_sy+par->vp_y))
			{
				if(par->vp_y+par->vp_sy<par->bw_size_y)
				{
					par->SetWindowPositionVP(0,cy+py-par->vp_y-par->vp_sy,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				y = par->vp_y + par->vp_sy - wy  - wsy + py;
			}
		}else{
			cx = wx+x;
			cy = wy+y;
			if(cx<par->x)
				x +=par->x-cx;
			if(cy<par->y)
				y +=par->y-cy;
			if(cx>par->sx)
				x -=cx-par->sx;
			if(cy>par->sy)
				y -=cy-par->sy;
		}
	}

	
	if(mouse_button_down)
	{
		ChangeWindowSize(x,y);
	}

	return 0;
}

int CGUIResizer::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	px=py=0;

	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	if(mouse_button_down && parent && parent->GetType() >= PTWindow)
	{
		((CGUIWindow*)parent)->FocusOldTOBackEl();
	}

	mouse_button_down=false;

	return 0;
}

int CGUIResizer::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	// kontrola na vyjezd s oknem mimo oblast materskeho okna
	if(parent && parent->GetType()>=PTWindow && parent->parent && parent->parent->GetType()>=PTWindow && mouse_button_down && (state & MK_LBUTTON))
	{
		CGUIWindow* wnd = (CGUIWindow*) parent;
		CGUIWindow* par = (CGUIWindow*) parent->parent;

		float wx, wy;
		float wsx, wsy;
		float cx, cy;
		wnd->GetPos(wx,wy);
		wnd->GetSize(wsx,wsy);
		if(wnd->back_element)
		{
			cx = wx+x-px;
			cy = wy+y-py;
			//if(cx<par->vp_x || cy<par->vp_y || cx>par->vp_sx || cy>par->vp_sy)
//				return 0;	// nehybat s oknem, jsem mimo oblast otcovskeho okna
			
/*			if(cx<par->vp_x)
				x +=par->vp_x-cx;
			if(cy<par->vp_y)
				y +=par->vp_y-cy;
			if(cx>par->vp_sx+par->vp_x)
				x -=cx-par->vp_sx-par->vp_x;
			if(cy>par->vp_sy+par->vp_y)
				y -=cy-par->vp_sy-par->vp_y;
//*/
			if((resizerType==UpperLeft || resizerType==Left || resizerType==BottomLeft)
				&& (cx<par->vp_x))
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
			}
			if((resizerType==UpperLeft || resizerType==Up || resizerType==UpperRight)
				&& (cy<par->vp_y))
			{
				if(par->vp_y>0)
				{
					par->SetWindowPositionVP(0,cy-par->vp_y,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				y = par->vp_y-wy+py;
			}

			if((resizerType==UpperRight || resizerType==Right || resizerType==BottomRight)
				&& (cx+wsx>par->vp_sx+par->vp_x))
			{
				if(par->vp_x+par->vp_sx < par->bw_size_x)
				{
					par->SetWindowPositionVP(cx+px-par->vp_x-par->vp_sx,0,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				x = par->vp_x + par->vp_sx - wx - wsx + px;
			}
			if((resizerType==BottomLeft || resizerType==Bottom || resizerType==BottomRight)
				&& (cy+wsy>par->vp_sy+par->vp_y))
			{
				if(par->vp_y+par->vp_sy<par->bw_size_y)
				{
					par->SetWindowPositionVP(0,cy+py-par->vp_y-par->vp_sy,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}
				y = par->vp_y + par->vp_sy - wy  - wsy + py;
			}
		}else{
			cx = wx+x;
			cy = wy+y;
			if(cx<par->x)
				x +=par->x-cx;
			if(cy<par->y)
				y +=par->y-cy;
			if(cx>par->sx)
				x -=cx-par->sx;
			if(cy>par->sy)
				y -=cy-par->sy;
		}
	}


	// zmenit kurzor na normal
	if(mouse_button_down && (state & MK_LBUTTON))
	{		// tlacitko je zmacknute a byla zapocata zmena velikosti okna => pokracuj  
		ChangeWindowSize(x,y);
	}else{
		mouse_button_down=false;
		px=py=0;
		if(cursor_resizer)
		{
			cursor_resizer=false;
			mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
		}
	}

	return 0;
}

int CGUIResizer::TimerImpulse(typeID timerID, float time)
{
	timerReq=false;
	mainGUI->SendCursorPos();
	return 0;
}

int CGUIResizer::FocusChanged(UINT focus, typeID dualID)
{
	if(focus)
		return 0;

	mouse_button_down=false;
	px=py=0;
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}

	return 0;
}

int CGUIResizer::WheelFocus(float x, float y, int dw, int dz, UINT state, typeID ID)
{
	px=py=0;
	if(cursor_resizer)
	{
		cursor_resizer=false;
		mainGUI->cursor->Set(cursor_type);	// !@#$ do budoucna menit na standardni kurzor
	}
	if(mouse_button_down && parent && parent->GetType() >= PTWindow)
	{
		((CGUIWindow*)parent)->FocusOldTOBackEl();
	}

	mouse_button_down=false;

	return 0;
}

void CGUIResizer::ChangeWindowSize(float nx, float ny)
{
	CGUIWindow *wnd = (CGUIWindow*) parent;
	if(!wnd)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIResizer::ChangeWindowSize> Parent window is not found");
	}

	float dx, dy;
	dx=nx-px;
	dy=ny-py;
	px=nx;
	py=ny;

	float old_dx=dx;
	float old_dy=dy;

	switch(resizerType)
	{
		case UpperLeft	:	dx=-dx;
							dy=-dy;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(-dx,-dy);	
							px+=dx-old_dx;
							py+=dy-old_dy;
							px-=dx; py-=dy;	
							break;
		case Up			:	///*
							dx=0;
							dy=-dy;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(0,-dy);
							py+=dy-old_dy;
							py-=dy;
							break;//*/

			/*
							dx=0;
							wnd->CheckPositionRel(dx,dy);
							dy=-dy;
							wnd->CheckResizeRel(dx,dy);

							dy=-dy;
							dx=0;
							wnd->MoveRelWithCheck(dx,dy);
							dy=-dy;
							wnd->ResizeRelWithCheck(dx,dy);
							
							py+=dy-old_dy;
							py-=dy;
							break;
							*/
		case UpperRight	:	dy=-dy;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(0,-dy);
							px+=dx-old_dx;
							py+=dy-old_dy;
							py-=dy;
							break;
		case Left		:	dy=0;
							dx=-dx;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(-dx,0);
							px+=dx-old_dx;
							px-=dx;
							break;
		case BottomLeft	:	dx=-dx;
							wnd->ResizeRelWithCheck(dx,dy);
							wnd->MoveRel(-dx,0);
							px+=dx-old_dx;
							py+=dy-old_dy;
							px-=dx;
							break;
		case Right		:	dy=0; wnd->ResizeRelWithCheck(dx,dy);	px+=dx-old_dx;	break;
		case Bottom		:	dx=0; wnd->ResizeRelWithCheck(dx,dy);	py+=dy-old_dy;	break;
		case BottomRight:	wnd->ResizeRelWithCheck(dx,dy);	px+=dx-old_dx;	py+=dy-old_dy;	break;
	}



}


//////////////////////////////////////////////////////////////////////
// CGUITitle
//////////////////////////////////////////////////////////////////////

const int CGUITitle::numEvents = 1;
const int CGUITitle::availableEvents[numEvents] = {EStateChanged};


CGUITitle::CGUITitle(char *title, CGUIRectHost *_icon, CGUIWindow *wnd, float height, bool _closeButton, bool _minimizeButton, bool _maximizeButton, CTexture* texture, int labelColor, int bgActiveColor, int bgDeactiveColor, int text_height)
	: CGUIWidget(0,0), CGUIEventSupport(availableEvents,numEvents)
{
	assert(wnd);

	float vpsx,vpsy;
	float bwx,bwy;
	wnd->GetVPSize(vpsx,vpsy);
	wnd->GetBWStartPos(bwx,bwy);

	SetPos(bwx,bwy);

	float wx,wy;
	float buttonSizeX,buttonSizeY;
	float butt_pos_x;
	rect = 0;
	label = 0;
	icon=_icon;
	timerReq=false;
	movable=true;

	closeButton=0;
	minimizeButton=0;
	maximizeButton=0;

	if(title)
	{
		titleText=new char[strlen(title)+1];
		strcpy(titleText,title);
	}else{
		titleText=new char[1];
		titleText[0] = 0;
	}


	try{
		wnd->GetSize(wx,wy);

		sx=vpsx;
		sy=height;
		butt_pos_x=sx+2;
		if(texture)
			rect = new CGUIRectHost(x,y,texture,sx,sy,bgActiveColor);
		else
			rect = new CGUIRectHost(x,y,sx,sy,bgActiveColor);

		if(icon)
		{
			float p_sx, p_sy;
			icon->GetSize(p_sx,p_sy);
			if(p_sx>STD_WINICON_HEIGHT && p_sy>STD_WINICON_HEIGHT)
				icon->Resize(STD_WINICON_HEIGHT,STD_WINICON_HEIGHT);
			else if(p_sx>STD_WINICON_HEIGHT)
				icon->Resize(STD_WINICON_HEIGHT,p_sy);
			else if(p_sy>STD_WINICON_HEIGHT)
				icon->Resize(p_sx,STD_WINICON_HEIGHT);
			icon->Move(x+5,floorf(y+(height-STD_WINICON_HEIGHT)/2));

			label = new CGUIStaticText(x+5+STD_WINICON_HEIGHT+6,floorf(y+(height-text_height)/2),titleText,ARIAL,text_height,true,labelColor);
		}else
			label = new CGUIStaticText(x+5,floorf(y+(height-text_height)/2),titleText,ARIAL,text_height,true,labelColor);


		if(_closeButton)
		{
			buttonSizeX = styleSet->Get("CloseButtonUp")->GetParametr(0);
			buttonSizeY = styleSet->Get("CloseButtonUp")->GetParametr(1);
			if(buttonSizeY>height)
			{
				buttonSizeY=height;
			}
			if(buttonSizeX>height)
			{
				buttonSizeX=height;
			}

			closeButton = new CGUIButton(sx-buttonSizeX,floorf(y+(height-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("CloseButtonUp"),styleSet->Get("CloseButtonDown"),styleSet->Get("CloseButtonMark"),true,0,0,"Zavøít");
			closeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
			closeButton->AcceptEvent(GetID(),EClicked);
			closeButton->AcceptEvent(GetID(),EDeleted);
			butt_pos_x=sx-buttonSizeX;
		}

		if(_maximizeButton)
		{
			if(wnd->windowState==WS_Maximized)
			{
				buttonSizeX = styleSet->Get("DeMaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("DeMaximizeButtonUp")->GetParametr(1);
				maximizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(height-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("DeMaximizeButtonUp"),styleSet->Get("DeMaximizeButtonDown"),styleSet->Get("DeMaximizeButtonMark"),true,0,0,"DeMaximalizovat");
				maximizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
				maximizeButton->AcceptEvent(GetID(),EClicked);
				maximizeButton->AcceptEvent(GetID(),EMouseButton);
				maximizeButton->AcceptEvent(GetID(),EDeleted);
				maxim_button=false;
			}else{
				buttonSizeX = styleSet->Get("MaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("MaximizeButtonUp")->GetParametr(1);
				maximizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(height-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("MaximizeButtonUp"),styleSet->Get("MaximizeButtonDown"),styleSet->Get("MaximizeButtonMark"),true,0,0,"Maximalizovat");
				maximizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
				maximizeButton->AcceptEvent(GetID(),EClicked);
				maximizeButton->AcceptEvent(GetID(),EMouseButton);
				maximizeButton->AcceptEvent(GetID(),EDeleted);
				maxim_button=true;
			}
			butt_pos_x-=buttonSizeX+2;
		}

		if(_minimizeButton)
		{
			buttonSizeX = styleSet->Get("MinimizeButtonUp")->GetParametr(0);
			buttonSizeY = styleSet->Get("MinimizeButtonUp")->GetParametr(1);
			minimizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(height-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("MinimizeButtonUp"),styleSet->Get("MinimizeButtonDown"),styleSet->Get("MinimizeButtonMark"),true,0,0,"Minimalizovat");
			minimizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
			minimizeButton->AcceptEvent(GetID(),EClicked);
			minimizeButton->AcceptEvent(GetID(),EMouseButton);
			minimizeButton->AcceptEvent(GetID(),EDeleted);
		}


		SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FIX,WIDGET_FIX);
		
		MsgAcceptConsume(MsgMouseL|MsgMouseOver|MsgTimer);
		MsgSetProduce(MsgNone);
		mouse_button_down=false;
		px=py=0;
		active_color = bgActiveColor;
		deactive_color = bgDeactiveColor;

		wnd->AddFrontElem(this);
		wnd->AcceptEvent(GetID(),EFocusChanged);
		wnd->AcceptEvent(GetID(),ESizeChanged);
		wnd->title=this;

		if(closeButton)
		{
			wnd->AddFrontElem(closeButton);
			closeButton->keySensitive=false;
		}
		if(maximizeButton)
		{
			wnd->AddFrontElem(maximizeButton);
			maximizeButton->keySensitive=false;
		}
		if(minimizeButton)
		{
			wnd->AddFrontElem(minimizeButton);
			minimizeButton->keySensitive=false;
		}

		rect->RemoveFromTrash();
		label->RemoveFromTrash();
		if(icon)
			icon->RemoveFromTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		SAFE_DELETE(label);
		SAFE_DELETE(icon);
		SAFE_DELETE(closeButton);
		SAFE_DELETE(minimizeButton);
		SAFE_DELETE(maximizeButton);
		throw;
	}
	//focusable = false;	// !@#$ kvuli tomu, aby neztratil focus back element okna, kdyz se klikne na titulek
							// problem je, ze potom muzu "ztratit" okna pri jeho posouvani pres titulek (vyjetim z materskeho okna)
}

CGUITitle::~CGUITitle()
{
	timerGUI->DeleteAllRequests(this);
	SAFE_DELETE(rect);
	SAFE_DELETE(label);
	SAFE_DELETE(icon);
	SAFE_DELETE_ARRAY(titleText);
	SAFE_DELETE(closeButton);
	SAFE_DELETE(minimizeButton);
	SAFE_DELETE(maximizeButton);
}

void CGUITitle::SetText(char *text, int labelColor, int text_height)
{
	CBltElem* pos=0;
	if(label)
	{
		pos = label->GetElemPosition();
	}else if(icon)
	{
		pos = icon->GetElemPosition();
	}
	CBltWindow *wnd=pos->GetParent();

	SAFE_DELETE(label);
	SAFE_DELETE_ARRAY(titleText);

	if(text)
	{
		titleText=new char[strlen(text)+1];
		strcpy(titleText,text);
	}else{
		titleText = new char[1];
		titleText[0] = 0;
	}

	if(icon)
	{
		float p_sx, p_sy;
		icon->GetSize(p_sx,p_sy);
		label = new CGUIStaticText(x+5+STD_WINICON_HEIGHT+6,floorf(y+(sy-text_height)/2),titleText,ARIAL,text_height,true,labelColor);
	}else
		label = new CGUIStaticText(x+5,floorf(y+(sy-text_height)/2),titleText,ARIAL,text_height,true,labelColor);

	label->RemoveFromTrash();

	if(wnd)
	{
		if(pos)
			label->AddToEngineBackPosition(wnd,pos);
		else
			label->AddToEngine(wnd);
	}
}

void CGUITitle::SetButtons(bool _closeButton, bool _minimizeButton, bool _maximizeButton)
{
	float buttonSizeX,buttonSizeY;
	CGUIWindow* wnd=0;
	float butt_pos_x=sx+2;
	float bx,by;

	if(parent && parent->GetType()>=PTWindow)
		 wnd = (CGUIWindow*)parent;

	if(_closeButton)
	{
		buttonSizeX = styleSet->Get("CloseButtonUp")->GetParametr(0);
		buttonSizeY = styleSet->Get("CloseButtonUp")->GetParametr(1);
		if(!closeButton)
		{
			if(buttonSizeY>sy)
			{
				buttonSizeY=sy;
			}
			if(buttonSizeX>sy)
			{
				buttonSizeX=sy;
			}
			closeButton = new CGUIButton(sx-buttonSizeX,floorf(y+(sy-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("CloseButtonUp"),styleSet->Get("CloseButtonDown"),styleSet->Get("CloseButtonMark"),true,0,0,"Zavøít");
			closeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
			closeButton->AcceptEvent(GetID(),EClicked);
			closeButton->AcceptEvent(GetID(),EDeleted);
			closeButton->keySensitive=false;
			wnd->AddFrontElem(closeButton);
		}
		butt_pos_x=sx-buttonSizeX;
	}else{
		if(closeButton)
		{
			wnd->DeleteFrontElem(closeButton);
			closeButton=0;
		}
	}

	if(_maximizeButton)
	{
		if(!maximizeButton)
		{
			if(wnd->windowState==WS_Maximized)
			{
				buttonSizeX = styleSet->Get("DeMaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("DeMaximizeButtonUp")->GetParametr(1);
				maximizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(sy-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("DeMaximizeButtonUp"),styleSet->Get("DeMaximizeButtonDown"),styleSet->Get("DeMaximizeButtonMark"),true,0,0,"DeMaximalizovat");
				maximizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
				maximizeButton->AcceptEvent(GetID(),EClicked);
				maximizeButton->AcceptEvent(GetID(),EMouseButton);
				maximizeButton->AcceptEvent(GetID(),EDeleted);
				maxim_button=false;
			}else{
				buttonSizeX = styleSet->Get("MaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("MaximizeButtonUp")->GetParametr(1);
				maximizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(sy-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("MaximizeButtonUp"),styleSet->Get("MaximizeButtonDown"),styleSet->Get("MaximizeButtonMark"),true,0,0,"Maximalizovat");
				maximizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
				maximizeButton->AcceptEvent(GetID(),EClicked);
				maximizeButton->AcceptEvent(GetID(),EMouseButton);
				maximizeButton->AcceptEvent(GetID(),EDeleted);
				maxim_button=true;
			}
			maximizeButton->keySensitive=false;
			wnd->AddFrontElem(maximizeButton);
		}else{
			if(wnd->windowState==WS_Maximized)
			{
				buttonSizeX = styleSet->Get("DeMaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("DeMaximizeButtonUp")->GetParametr(1);
			}else{
				buttonSizeX = styleSet->Get("MaximizeButtonUp")->GetParametr(0);
				buttonSizeY = styleSet->Get("MaximizeButtonUp")->GetParametr(1);
			}
			maximizeButton->GetPos(bx,by);
			maximizeButton->Move(butt_pos_x-buttonSizeX-2,by);
		}
		butt_pos_x-=buttonSizeX+2;
	}else{
		if(maximizeButton)
		{
			wnd->DeleteFrontElem(maximizeButton);
			maximizeButton=0;
		}
	}

	if(_minimizeButton)
	{
		buttonSizeX = styleSet->Get("MinimizeButtonUp")->GetParametr(0);
		buttonSizeY = styleSet->Get("MinimizeButtonUp")->GetParametr(1);
		if(!minimizeButton)
		{
			minimizeButton = new CGUIButton(butt_pos_x-buttonSizeX-2,floorf(y+(sy-buttonSizeY)/2+0.5f),buttonSizeX,buttonSizeY,styleSet->Get("MinimizeButtonUp"),styleSet->Get("MinimizeButtonDown"),styleSet->Get("MinimizeButtonMark"),true,0,0,"Minimalizovat");
			minimizeButton->SetAnchor(WIDGET_FIX,WIDGET_FREE,WIDGET_FREE, WIDGET_FIX);
			minimizeButton->AcceptEvent(GetID(),EClicked);
			minimizeButton->AcceptEvent(GetID(),EMouseButton);
			minimizeButton->AcceptEvent(GetID(),EDeleted);
			minimizeButton->keySensitive=false;
			wnd->AddFrontElem(minimizeButton);
		}else{
			minimizeButton->GetPos(bx,by);
			minimizeButton->Move(butt_pos_x-buttonSizeX-2,by);
		}
	}else{
		if(minimizeButton)
		{
			wnd->DeleteFrontElem(minimizeButton);
			minimizeButton=0;
		}
	}
}

void CGUITitle::SetMovable(bool _movable)
{
	movable=_movable;
}

void CGUITitle::SetIcon(CGUIRectHost *_icon)
{
	CBltElem* pos=0;
	if(icon)
	{
		pos = icon->GetElemPosition();
	}else if(label)
	{
		pos = label->GetElemPosition();
	}
	CBltWindow *wnd=pos->GetParent();

	SAFE_DELETE(icon);

	icon=_icon;

	if(icon)
	{
		float p_sx, p_sy;
		icon->GetSize(p_sx,p_sy);
		if(p_sx>STD_WINICON_HEIGHT && p_sy>STD_WINICON_HEIGHT)
			icon->Resize(STD_WINICON_HEIGHT,STD_WINICON_HEIGHT);
		else if(p_sx>STD_WINICON_HEIGHT)
			icon->Resize(STD_WINICON_HEIGHT,p_sy);
		else if(p_sy>STD_WINICON_HEIGHT)
			icon->Resize(p_sx,STD_WINICON_HEIGHT);
		icon->Move(x+5,floorf(y+(sy-STD_WINICON_HEIGHT)/2));

		if(label)
		{
			float lx,ly;
			label->GetPos(lx,ly);
			label->Move(x+5+STD_WINICON_HEIGHT+6,ly);
		}
	}else if(label)
	{
		float lx,ly;
		label->GetPos(lx,ly);
		label->Move(x+5,ly);
	}

	if(wnd)
	{
		if(pos)
			icon->AddToEngineBackPosition(wnd,pos);
		else
			icon->AddToEngine(wnd);
	}

	EventArise(EStateChanged);
}

void CGUITitle::SetActiveInactiveColor(int bgActiveColor, int bgDeactiveColor)
{
	bool active=true;

	if(rect)
	{
		active = rect->GetColor()==active_color;
	}

	active_color = bgActiveColor;
	deactive_color = bgDeactiveColor;

	if(rect)
	{
		if(active)
			rect->SetColor(active_color);
		else
			rect->SetColor(deactive_color);
	}
}

void CGUITitle::SetVisible(int vis)
{
	SetVisibility(vis);
	if(rect)
		rect->SetVisible(vis);
	if(label)
		label->SetVisible(vis);
	if(icon)
		icon->SetVisible(vis);
	if(closeButton)
		closeButton->SetVisible(vis);
	if(minimizeButton)
		minimizeButton->SetVisible(vis);
	if(maximizeButton)
		maximizeButton->SetVisible(vis);
}

void CGUITitle::Resize(float _sx, float _sy)
{
	CheckResize(_sx,_sy);
	if(rect)
		rect->Resize(_sx,_sy);
	SetSize(_sx,_sy);
}

void CGUITitle::Move(float _x, float _y)
{
	CheckPosition(_x,_y);
	if(label)
		label->MoveRel(_x-x,_y-y);

	SetPos(_x,_y);
	if(rect)
		rect->Move(_x,_y);
	if(icon)
		icon->Move(_x,_y);
}

void CGUITitle::AddToEngine(CBltWindow *rootwnd)
{
	if(rect)
		rect->AddToEngine(rootwnd);
	if(label)
		label->AddToEngine(rootwnd);
	if(icon)
		icon->AddToEngine(rootwnd);
}

void CGUITitle::RemoveFromEngine()
{
	if(rect)
		rect->RemoveFromEngine();
	if(label)
		label->RemoveFromEngine();
	if(icon)
		icon->RemoveFromEngine();

	if(parent && parent->GetType() >= PTWindow)
	{
		if(closeButton)
			closeButton=(CGUIButton*) ((CGUIWindow* )parent)->RemoveFrontElem(closeButton);
		if(minimizeButton)
			minimizeButton=(CGUIButton*) ((CGUIWindow* )parent)->RemoveFrontElem(minimizeButton);
		if(maximizeButton)
			maximizeButton=(CGUIButton*) ((CGUIWindow* )parent)->RemoveFrontElem(maximizeButton);
	}

	if(parent && parent->GetType() >= PTWindow && ((CGUIWindow* )parent)->title == this)
		((CGUIWindow* )parent)->title = 0;
}

void CGUITitle::BringToTop()
{
	if(rect)
		rect->BringToTop();
	if(label)
		label->BringToTop();
	if(icon)
		icon->BringToTop();
	if(closeButton)
		closeButton->BringToTop();
	if(minimizeButton)
		minimizeButton->BringToTop();
	if(maximizeButton)
		maximizeButton->BringToTop();
}

void CGUITitle::WindowIsActive()
{
	if(rect)
		rect->SetColor(active_color);
	/*
	CTexture *t = new CTexture();
	t->CreateFromFile("$TEX$\\title.png");
	if(rect)
		rect->RebuildRect(t,-1,-1,sx,sy);
	t->Release();
	*/
}

void CGUITitle::WindowIsDeactive()
{
	if(rect)
		rect->SetColor(deactive_color);
	/*
	CTexture *t = new CTexture();
	t->CreateFromFile("$TEX$\\title2.png");
	if(rect)
		rect->RebuildRect(t,-1,-1,sx,sy);
	t->Release();
	*/
}

int CGUITitle::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	if(mouseState)
	{			// po stisknuti tlacitka zacinam menit pozici okna (zaznamenavam zmenu souradnic kurzoru)
		mouse_button_down=true;
		px=x;
		py=y;
	}else if(mouse_button_down){		// po pusteni tlacitka dokoncim zmenu polohy okna
		if(movable)
			((CGUIWindow*) parent)->MoveRel(x-px,y-py);

		mouse_button_down=false;
		px=py=0;
		if(parent && parent->GetType() >= PTWindow)
		{
			((CGUIWindow*)parent)->FocusOldTOBackEl();
		}
	}
	return 0;
}

int CGUITitle::MouseOver(float x, float y, UINT over,UINT state)
{
	if(!(state & MK_LBUTTON))
		mouse_button_down=false;
	
	if(mouse_button_down && movable)
	{
		((CGUIWindow*) parent)->MoveRel(x-px,y-py);
	}

	//DebugMessage(7,"Title: (x,y) %f:%f",x,y);

	return 0;
}

int CGUITitle::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	if(mouse_button_down && parent && parent->GetType() >= PTWindow)
	{
		//((CGUIWindow*)parent)->FocusOldTOBackEl();
	}

	return 0;
}

int CGUITitle::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	// kontrola na vyjezd s oknem mimo oblast materskeho okna
	if(parent && parent->GetType()>=PTWindow && parent->parent && parent->parent->GetType()>=PTWindow && mouse_button_down && movable && (state & MK_LBUTTON))
	{
		CGUIWindow* wnd = (CGUIWindow*) parent;
		CGUIWindow* par = (CGUIWindow*) parent->parent;

		float wx, wy;
		float cx, cy;
		wnd->GetPos(wx,wy);
		if(wnd->back_element)
		{
			cx = wx+x;
			cy = wy+y;
			//if(cx<par->vp_x || cy<par->vp_y || cx>par->vp_sx || cy>par->vp_sy)
//				return 0;	// nehybat s oknem, jsem mimo oblast otcovskeho okna
			/*if(cx<par->vp_x)
				x +=par->vp_x-cx;
			if(cy<par->vp_y)
				y +=par->vp_y-cy;*/

			if(cx<par->vp_x)
			{
				if(par->vp_x>0)
				{
					if(cx<0)
						x+=-cx-par->vp_x;
					par->SetWindowPositionVP(cx-par->vp_x,0,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}else
					x +=-cx;
			}
			if(cy<par->vp_y)
			{
				if(par->vp_y>0)
				{
					if(cy<0)
						y+=-cy-par->vp_y;
					par->SetWindowPositionVP(0,cy-par->vp_y,true);
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}else
					y +=-cy;
			}

			if(cx>par->vp_sx+par->vp_x)
			{
				if(par->vp_x+par->vp_sx < par->bw_size_x)
				{
					//if(cx>par->bw_size_x)
						//x -= cx - ((par->bw_size_x-par->vp_sx) - par->vp_x);
					par->SetWindowPositionVP(cx-par->vp_x-par->vp_sx,0,true);
					if(cx>par->bw_size_x)
						x -=cx-par->vp_sx-par->vp_x;
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}else
					x -=cx-par->vp_sx-par->vp_x;
			}
			if(cy>par->vp_sy+par->vp_y)
			{
				if(par->vp_y+par->vp_sy<par->bw_size_y)
				{
					//if(cy>par->bw_size_y)
						//y -= cy - ((par->bw_size_y-par->vp_sy) - par->vp_y);
					par->SetWindowPositionVP(0,cy-par->vp_y-par->vp_sy,true);
					if(cy>par->bw_size_y)
						y -=cy-par->vp_sy-par->vp_y;
					if(!timerReq)
					{
						timerReq=true;
						timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);
					}
				}else
					y -=cy-par->vp_sy-par->vp_y;
			}
		}else{
			cx = wx+x;
			cy = wy+y;
			if(cx<par->x)
				x +=par->x-cx;
			if(cy<par->y)
				y +=par->y-cy;
			if(cx>par->sx)
				x -=cx-par->sx;
			if(cy>par->sy)
				y -=cy-par->sy;
		}

		
	}

	if(mouse_button_down && movable && (state & MK_LBUTTON))
	{		// tlacitko je zmacknute a byla zapocata zmena polohy okna => pokracuj  
		((CGUIWindow*) parent)->MoveRel(x-px,y-py);
	}else{
		mouse_button_down=false;
		px=py=0;
		///*
		if(parent && parent->GetType() >= PTWindow)
		{
			//((CGUIWindow*)parent)->FocusOldTOBackEl();
		}
		//*/
	}

	//DebugMessage(8,"TitleFocus: (x,y) %f:%f",x,y);
	return 0;
}

int CGUITitle::TimerImpulse(typeID timerID, float time)
{
	timerReq=false;
	mainGUI->SendCursorPos();
	return 0;
}

int CGUITitle::FocusChanged(UINT focus, typeID dualID)
{
	mouse_button_down=false;
	px=py=0;
	return 0;
}

void CGUITitle::EventHandler(CGUIEvent *event) 
{
	if(!event)
		return;

	switch(event->eventID)
	{
		case EDeleted:		if(event->pAddr == closeButton)
								closeButton=0;
							if(event->pAddr == minimizeButton)
								minimizeButton=0;
							if(event->pAddr == maximizeButton)
								maximizeButton=0;
							break;
		case EFocusChanged:	if(event->pInt)
								WindowIsActive();
							else
								WindowIsDeactive();
							break;
		case EClicked:		if(nameServer->ResolveID(event->sender) == closeButton && closeButton)
							{
								if(parent && parent->GetType() >= PTWindow)
								{
									CGUIMessage *msg = new CGUIMessage(MsgCommand,ComCloseWnd,closeButton);
									SendMsg(msg,parent);
									// vyvola neprimo zavolani parent->CloseWindow()
									//((CGUIWindow*)parent)->CloseWindow();
								}
							}else if(nameServer->ResolveID(event->sender) == maximizeButton && maximizeButton)
							{
								if(parent && parent->GetType() >= PTWindow)
								{
									CGUIWindow* wnd = (CGUIWindow*)parent;
									if(wnd->windowState==WS_Maximized)
									{
										wnd->UnMaximize();
										if(!maxim_button)
										{	// uprava vzhledu buttonu na maximalizacni
											maximizeButton->ReStylize(styleSet->Get("MaximizeButtonUp"),styleSet->Get("MaximizeButtonDown"),styleSet->Get("MaximizeButtonMark"));
											maximizeButton->ChangeContextHelp("Maximalizovat");
											maxim_button=true;
										}
									}else{
										wnd->Maximize();
										if(maxim_button)
										{	// uprava vzhledu buttonu na demaximalizacni
											maximizeButton->ReStylize(styleSet->Get("DeMaximizeButtonUp"),styleSet->Get("DeMaximizeButtonDown"),styleSet->Get("DeMaximizeButtonMark"));
											maximizeButton->ChangeContextHelp("DeMaximalizovat");
											maxim_button=false;
										}
									}
									mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
									mainGUI->GetEvent(0);

									wnd->FocusOldTOBackEl();
								}

							}else if(nameServer->ResolveID(event->sender) == minimizeButton && minimizeButton)
							{
								if(parent && parent->GetType() >= PTWindow)
								{
									((CGUIWindow*)parent)->Minimize();
									mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
									mainGUI->GetEvent(0);

									((CGUIWindow*)parent)->FocusOldTOBackEl();
								}
							}
							break;
		case EMouseButton:	if((nameServer->ResolveID(event->sender) == maximizeButton && maximizeButton)&&
								(event->pInt == 1 && event->pID == 1))	// right button + push
							{
								if(parent && parent->GetType() >= PTWindow)
								{
									// normalizace velikosti okna "WindowSizeBWNormal"
									((CGUIWindow*)parent)->Normalize();
									mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
									mainGUI->GetEvent(0);

									((CGUIWindow*)parent)->FocusOldTOBackEl();
								}

							}
							if((nameServer->ResolveID(event->sender) == minimizeButton && minimizeButton)&&
								(event->pInt == 1 && event->pID == 1))	// right button + push
							{
								if(parent && parent->GetType() >= PTWindow)
								{
									// maximalizace velikosti okna na plnou velikost rodicovskeho okna
									((CGUIWindow*)parent)->MaximizeFull();
									mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
									mainGUI->GetEvent(0);

									((CGUIWindow*)parent)->FocusOldTOBackEl();
								}

							}
							break;
		case ESizeChanged:	if((parent && parent->GetType() >= PTWindow) &&
								(nameServer->ResolveID(event->sender) == (CGUIWindow*)parent) && maximizeButton)	// udalost o zmene velikosti materskeho okna => zmena stavu okna/maxim. buttonu
							{
								CGUIWindow* wnd = (CGUIWindow*)parent;
								if(wnd->windowState==WS_Maximized)
								{
									if(maxim_button)
									{	// uprava vzhledu buttonu na demaximalizacni
										maximizeButton->ReStylize(styleSet->Get("DeMaximizeButtonUp"),styleSet->Get("DeMaximizeButtonDown"),styleSet->Get("DeMaximizeButtonMark"));
										maximizeButton->ChangeContextHelp("DeMaximalizovat");
										maxim_button=false;
									}
								}else{
									if(!maxim_button)
									{	// uprava vzhledu buttonu na maximalizacni
										maximizeButton->ReStylize(styleSet->Get("MaximizeButtonUp"),styleSet->Get("MaximizeButtonDown"),styleSet->Get("MaximizeButtonMark"));
										maximizeButton->ChangeContextHelp("Maximalizovat");
										maxim_button=true;
									}
								}
							}
							break;
	}

	delete event;
}

