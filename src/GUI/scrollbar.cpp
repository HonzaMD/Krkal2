///////////////////////////////////////////////
//
// scrollbar.cpp
//
// Implementace pro scrollbar a jeho casti scrollbarButtons
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "scrollbar.h"
#include "widgets.h"

#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIScrollBar
//////////////////////////////////////////////////////////////////////

CGUIScrollBar::CGUIScrollBar(enum EScrollBarTypes _scrollBarType, float _x, float _y, float length, float width, float _shift, bool _autohide, bool _slider_resize, CGUIWindow *wnd, char* styleName[3][2], CGUIRectHost* pictures[3], CGUIScrollBar* _dual)
	: CGUIMultiWidget(_x,_y)
{
	/*-------------------------------------------------------------------------
		Velikost scrollbaru:	- delka aspon 2x vetsi nez sirka
			- slider je minimalne velky jako up, down buttony (sirka x sirka)
			- sirka a vyska musi byt aspon takova, aby se tam vesly aspon up, down buttony
			- pageUp, pageDown, Nothing buttony jsou pod ostatnima, aby neprekryvaly pri malych velikostech 
											(vel. mensi nez je mozna velikost buttonu)
			- pokud se nevejde slider => pageUp, PageDown buttony se nezobrazuji, 
					misto mezi up a down je vyplneno nereagujici bitmapou (buttonem?)
	-------------------------------------------------------------------------*/
	RemoveFromTrash();

	SetAvailableEvents(0,0);

	float len, pos;
	int i;
	scrollBarType = _scrollBarType;
	autohide = _autohide;
	slider_resize=_slider_resize;
	active=false;
	corner = 0;
	shift = _shift;

	if(!wnd)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIScrollBar::CGUIScrollBar> Parent window is not defined");
	}

	if(length<=2*width)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIScrollBar::CGUIScrollBar> Height & width of scrollbar are badly defined, scrollbar cannot construct");
	}

	for(i=0;i<6;i++)
		buttons[i] = 0;

	dual = _dual;
	if(dual && (dual->active || !dual->autohide))
	{
		long_version=false;
		length -=width;
	}else{
		long_version=true;
	}

	try{
		switch(scrollBarType)
		{
			case Vertical	:	sx = width;
								sy = length;
								SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
								if(wnd->vp_sy >= wnd->bw_size_normal_y)
								{	// viewport je vetsi nez okno
									// nezobrazovat? a nepouzivat scrollbary
									active=false;

									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Up, 0, pictures[0]);	
									buttons[4] = new CGUIScrollbarButton(this, 0,length-width,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Down, 0, pictures[1]);

									// stredove buttony se nezobrazuji, jen se vytvori (o velikosti celeho vnitrku scrollbaru)
									buttons[2] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
									buttons[1] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
									buttons[3] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);

									// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
									buttons[5] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), scrollBarType, S_Nothing);

									if(autohide)
									{	// nezobrazovat zadnoou cast scrollbaru
										SetVisible(0);
										for(i=0;i<6;i++)
											buttons[i]->SetVisible(0);	// buttony jeste nejsou ve fronte, musi se zneviditelnit manualne
									}else{
										// zobrazit jen kraje a neaktivni stred (5)
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
										//buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);

									}
								}else{
									active=true;
									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Up, 0, pictures[0]);
									buttons[4] = new CGUIScrollbarButton(this, 0,length-width,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Down, 0, pictures[1]);
									if(slider_resize)
										len = floorf((wnd->vp_sy / wnd->bw_size_normal_y) * (length - 2*width));	// !@#$ zaokrouhlovani ????
									else
										len = width;
									if(len<width)
										len=width;	// kontrola velikosti slideru

									pos = floorf((wnd->vp_y / (wnd->bw_size_normal_y - wnd->vp_sy)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
									buttons[5] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), scrollBarType, S_Nothing);
									if((length - 2*width) < width)
									{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
										// misto nich jen neaktivni plocha
										buttons[2] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									}else{
										buttons[2] = new CGUIScrollbarButton(this, 0,width+pos,width,len,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, 0,width,width,pos,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, 0,width+pos+len,width,(length-2*width)-(pos+len),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);
										buttons[5]->SetVisible(0);
									}
								}

								// zmenseni viewportu o misto, ktere zabira scrollbar
								if(active || !autohide)
								{
									float dx, dy;
									dx = -width;
									dy = 0;
									wnd->ChangeViewPortBWSize(dx,dy);

									if(dx>-width)
									{
										float ox,oy;
										wnd->GetSize(ox,oy);
										wnd->ResizeFrontOnly(width-dx+ox,oy);
									}
								}

								if(dual)
									if(long_version)
										corner = new CGUIStaticPicture(x,y+length-width,styleSet->Get("SB_Pictures")->GetTexture(6),width,width);
									else
										corner = new CGUIStaticPicture(x,y+length,styleSet->Get("SB_Pictures")->GetTexture(6),width,width);

								break;
			case Horizontal	:	sx = length;
								sy = width;
								SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FIX,WIDGET_FIX);
								if(wnd->vp_sx >= wnd->bw_size_normal_x)
								{
									// nezobrazovat? a nepouzivat scrollbary
									active=false;

									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Up, 0, pictures[0]);	
									buttons[4] = new CGUIScrollbarButton(this, length-width,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Down, 0, pictures[1]);

									// stredove buttony se nezobrazuji, jen se vytvori (o velikosti celeho vnitrku scrollbaru)
									buttons[2] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
									buttons[1] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
									buttons[3] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);

									// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
									buttons[5] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), scrollBarType, S_Nothing);

									if(autohide)
									{	// nezobrazovat zadnoou cast scrollbaru
										SetVisible(0);
										for(i=0;i<6;i++)
											buttons[i]->SetVisible(0);// buttony jeste nejsou ve fronte, musi se zneviditelnit manualne
									}else{
										// zobrazit jen kraje a neaktivni stred (5)
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
										//buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);
									}
								}else{
									active=true;
									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Up, 0, pictures[0]);
									buttons[4] = new CGUIScrollbarButton(this, length-width,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), scrollBarType, S_Down, 0, pictures[1]);
									if(slider_resize)
										len = floorf((wnd->vp_sx / wnd->bw_size_normal_x) * (length - 2*width));	// !@#$ zaokrouhlovani ????
									else
										len = width;
									if(len<width)
										len=width;	// kontrola velikosti slideru

									pos = floorf((wnd->vp_x / (wnd->bw_size_normal_x - wnd->vp_sx)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
									buttons[5] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), scrollBarType, S_Nothing);
									if((length - 2*width) < width)
									{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
										// misto nich jen neaktivni plocha
										buttons[2] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									}else{
										buttons[2] = new CGUIScrollbarButton(this, width+pos,0,len,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, width,0,pos,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, width+pos+len,0,(length-2*width)-(pos+len),width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);
										buttons[5]->SetVisible(0);
									}
								}

								// zmenseni viewportu o misto, ktere zabira scrollbar
								if(active || !autohide)
								{
									float dx, dy;
									dx = 0;
									dy = -width;
									wnd->ChangeViewPortBWSize(dx,dy);

									if(dy>-width)
									{
										float ox,oy;
										wnd->GetSize(ox,oy);
										wnd->ResizeFrontOnly(ox,width-dy+oy);
									}
								}

								if(dual)
									if(long_version)
										corner = new CGUIStaticPicture(x+length-width, y, styleSet->Get("SB_Pictures")->GetTexture(6),width,width);
									else
										corner = new CGUIStaticPicture(x+length, y, styleSet->Get("SB_Pictures")->GetTexture(6),width,width);

								break;
		}

		AddElem(buttons[5]);	// nejspodneji je neaktivni stred
		AddElem(buttons[1]);	// pageup
		AddElem(buttons[3]);	// pageDown
		AddElem(buttons[2]);	// slider
		AddElem(buttons[0]);	// na vrcholu up
		AddElem(buttons[4]);	//			a down
		// kvuli moznym prekryvum pri prislis male velikosti tlacitka (mensi nez mozne minimum)

	//	MsgSetConsume(MsgMouseL|MsgMouseOver);	// nesmim konzumovat musim preposilat az k buttonum

		SetSize(sx,sy);
		if(window)
			window->SetWindowSize(sx, sy);

		wnd->AddFrontElem(this);
		wnd->AcceptEvent(GetID(),EUpdateScrollbars);
		wnd->AcceptEvent(GetID(),EMouseWheel);

		if(corner)
		{
			corner->MsgSetProduce(MsgNone);
			corner->SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
			wnd->AddFrontElem(corner);
		}

		if(dual)
		{
			dual->dual=this;
			if((dual->active || !dual->autohide) && (active || !autohide))	// && dual->long_version
			{
				dual->RebuildScrollbar();
			}else if(corner)
				corner->SetVisible(0);
		}

		for(i=0;i<6;i++)
			buttons[i]->RemoveFromTrash();

	}catch(CExc)
	{
		for(i=0;i<6;i++)
			SAFE_DELETE(buttons[i]);
		if(corner)
			wnd->RemoveFrontElem(corner);
		SAFE_DELETE(corner);
		throw;
	}
}

CGUIScrollBar::CGUIScrollBar(enum EScrollBarTypes _scrollBarType, float _x, float _y, float length, float width, float _shift, bool _autohide, bool _slider_resize, CGUIWindow *wnd, char* styleName[4][2], CGUIRectHost* pictures[3], CGUIScrollBar* _dual, bool simple)
	: CGUIMultiWidget(_x,_y)
{
	/*-------------------------------------------------------------------------
		Velikost scrollbaru:	- delka aspon 2x vetsi nez sirka
			- slider je minimalne velky jako up, down buttony (sirka x sirka)
			- sirka a vyska musi byt aspon takova, aby se tam vesly aspon up, down buttony
			- pageUp, pageDown, Nothing buttony jsou pod ostatnima, aby neprekryvaly pri malych velikostech 
											(vel. mensi nez je mozna velikost buttonu)
			- pokud se nevejde slider => pageUp, PageDown buttony se nezobrazuji, 
					misto mezi up a down je vyplneno nereagujici bitmapou (buttonem?)
	-------------------------------------------------------------------------*/
	RemoveFromTrash();

	SetAvailableEvents(0,0);

	float len, pos;
	int i;
	scrollBarType = _scrollBarType;
	autohide = _autohide;
	slider_resize=_slider_resize;
	active=false;
	corner = 0;
	shift = _shift;

	if(!wnd)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIScrollBar::CGUIScrollBar> Parent window is not defined");
	}

	if(length<=2*width)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIScrollBar::CGUIScrollBar> Height & width of scrollbar are badly defined, scrollbar cannot construct");
	}

	for(i=0;i<6;i++)
		buttons[i] = 0;

	dual = _dual;
	if(dual && (dual->active || !dual->autohide))
	{
		long_version=false;
		length -=width;
	}else{
		long_version=true;
	}

	try{
		switch(scrollBarType)
		{
			case Vertical	:	sx = width;
								sy = length;
								SetAnchor(WIDGET_FIX,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
								if(wnd->vp_sy >= wnd->bw_size_normal_y)
								{	// viewport je vetsi nez okno
									// nezobrazovat? a nepouzivat scrollbary
									active=false;

									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), false, scrollBarType, S_Up, 0, pictures[0]);	
									buttons[4] = new CGUIScrollbarButton(this, 0,length-width,width,width,styleSet->Get(styleName[3][0]),styleSet->Get(styleName[3][1]),styleSet->Get(styleName[3][0]), false, scrollBarType, S_Down, 0, pictures[1]);

									// stredove buttony se nezobrazuji, jen se vytvori (o velikosti celeho vnitrku scrollbaru)
									if(slider_resize)
										buttons[2] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
									else
										buttons[2] = new CGUIScrollbarButton(this, 0,width,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), false, scrollBarType, S_Slider,0,pictures[2]);
									buttons[1] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true,scrollBarType, S_PageUp);
									buttons[3] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageDown);

									// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
									buttons[5] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_Nothing);

									if(autohide)
									{	// nezobrazovat zadnoou cast scrollbaru
										SetVisible(0);
										for(i=0;i<6;i++)
											buttons[i]->SetVisible(0);	// buttony jeste nejsou ve fronte, musi se zneviditelnit manualne
									}else{
										// zobrazit jen kraje a neaktivni stred (5)
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
										//buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);

									}
								}else{
									active=true;
									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), false, scrollBarType, S_Up, 0, pictures[0]);
									buttons[4] = new CGUIScrollbarButton(this, 0,length-width,width,width,styleSet->Get(styleName[3][0]),styleSet->Get(styleName[3][1]),styleSet->Get(styleName[3][0]), false, scrollBarType, S_Down, 0, pictures[1]);
									if(slider_resize)
										len = floorf((wnd->vp_sy / wnd->bw_size_normal_y) * (length - 2*width));	// !@#$ zaokrouhlovani ????
									else
										len = width;
									if(len<width)
										len=width;	// kontrola velikosti slideru

									pos = floorf((wnd->vp_y / (wnd->bw_size_normal_y - wnd->vp_sy)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
									buttons[5] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_Nothing);
									if((length - 2*width) < width)
									{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
										// misto nich jen neaktivni plocha
										if(slider_resize)
											buttons[2] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										else
											buttons[2] = new CGUIScrollbarButton(this, 0,width,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), false, scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, 0,width,width,length - 2*width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageDown);
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									}else{
										if(slider_resize)
											buttons[2] = new CGUIScrollbarButton(this, 0,width+pos,width,len,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										else
											buttons[2] = new CGUIScrollbarButton(this, 0,width+pos,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), false, scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, 0,width,width,pos,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]),true, scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, 0,width+pos+len,width,(length-2*width)-(pos+len),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageDown);
										buttons[5]->SetVisible(0);
									}
								}

								// zmenseni viewportu o misto, ktere zabira scrollbar
								if(active || !autohide)
								{
									float dx, dy;
									dx = -width;
									dy = 0;
									wnd->ChangeViewPortBWSize(dx,dy);

									if(dx>-width)
									{
										float ox,oy;
										wnd->GetSize(ox,oy);
										wnd->ResizeFrontOnly(width-dx+ox,oy);
									}
								}

								if(dual)
									if(long_version)
										corner = new CGUIStaticPicture(x,y+length-width,styleSet->Get("SB_Pictures")->GetTexture(6),width,width);
									else
										corner = new CGUIStaticPicture(x,y+length,styleSet->Get("SB_Pictures")->GetTexture(6),width,width);

								break;
			case Horizontal	:	sx = length;
								sy = width;
								SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FIX,WIDGET_FIX);
								if(wnd->vp_sx >= wnd->bw_size_normal_x)
								{
									// nezobrazovat? a nepouzivat scrollbary
									active=false;

									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), false, scrollBarType, S_Up, 0, pictures[0]);	
									buttons[4] = new CGUIScrollbarButton(this, length-width,0,width,width,styleSet->Get(styleName[3][0]),styleSet->Get(styleName[3][1]),styleSet->Get(styleName[3][0]), false, scrollBarType, S_Down, 0, pictures[1]);

									// stredove buttony se nezobrazuji, jen se vytvori (o velikosti celeho vnitrku scrollbaru)
									if(slider_resize)
										buttons[2] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
									else
										buttons[2] = new CGUIScrollbarButton(this, width,0,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), false,scrollBarType, S_Slider,0,pictures[2]);
									buttons[1] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageUp);
									buttons[3] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageDown);

									// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
									buttons[5] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_Nothing);

									if(autohide)
									{	// nezobrazovat zadnoou cast scrollbaru
										SetVisible(0);
										for(i=0;i<6;i++)
											buttons[i]->SetVisible(0);// buttony jeste nejsou ve fronte, musi se zneviditelnit manualne
									}else{
										// zobrazit jen kraje a neaktivni stred (5)
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
										//buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);
									}
								}else{
									active=true;
									buttons[0] = new CGUIScrollbarButton(this, 0,0,width,width,styleSet->Get(styleName[0][0]),styleSet->Get(styleName[0][1]),styleSet->Get(styleName[0][0]), false, scrollBarType, S_Up, 0, pictures[0]);
									buttons[4] = new CGUIScrollbarButton(this, length-width,0,width,width,styleSet->Get(styleName[3][0]),styleSet->Get(styleName[3][1]),styleSet->Get(styleName[3][0]), false, scrollBarType, S_Down, 0, pictures[1]);
									if(slider_resize)
										len = floorf((wnd->vp_sx / wnd->bw_size_normal_x) * (length - 2*width));	// !@#$ zaokrouhlovani ????
									else
										len = width;
									if(len<width)
										len=width;	// kontrola velikosti slideru

									pos = floorf((wnd->vp_x / (wnd->bw_size_normal_x - wnd->vp_sx)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
									buttons[5] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][0]), true,scrollBarType, S_Nothing);
									if((length - 2*width) < width)
									{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
										// misto nich jen neaktivni plocha
										if(slider_resize)
											buttons[2] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										else
											buttons[2] = new CGUIScrollbarButton(this, width,0,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, width,0,length - 2*width,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), scrollBarType, S_PageDown);
										buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									}else{
										if(slider_resize)
											buttons[2] = new CGUIScrollbarButton(this, width+pos,0,len,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), scrollBarType, S_Slider,0,pictures[2]);
										else
											buttons[2] = new CGUIScrollbarButton(this, width+pos,0,width,width,styleSet->Get(styleName[2][0]),styleSet->Get(styleName[2][1]),styleSet->Get(styleName[2][0]), false, scrollBarType, S_Slider,0,pictures[2]);
										buttons[1] = new CGUIScrollbarButton(this, width,0,pos,width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageUp);
										buttons[3] = new CGUIScrollbarButton(this, width+pos+len,0,(length-2*width)-(pos+len),width,styleSet->Get(styleName[1][0]),styleSet->Get(styleName[1][1]),styleSet->Get(styleName[1][0]), true, scrollBarType, S_PageDown);
										buttons[5]->SetVisible(0);
									}
								}

								// zmenseni viewportu o misto, ktere zabira scrollbar
								if(active || !autohide)
								{
									float dx, dy;
									dx = 0;
									dy = -width;
									wnd->ChangeViewPortBWSize(dx,dy);

									if(dy>-width)
									{
										float ox,oy;
										wnd->GetSize(ox,oy);
										wnd->ResizeFrontOnly(ox,width-dy+oy);
									}
								}

								if(dual)
									if(long_version)
										corner = new CGUIStaticPicture(x+length-width, y, styleSet->Get("SB_Pictures")->GetTexture(6),width,width);
									else
										corner = new CGUIStaticPicture(x+length, y, styleSet->Get("SB_Pictures")->GetTexture(6),width,width);

								break;
		}

		AddElem(buttons[5]);	// nejspodneji je neaktivni stred
		AddElem(buttons[1]);	// pageup
		AddElem(buttons[3]);	// pageDown
		AddElem(buttons[2]);	// slider
		AddElem(buttons[0]);	// na vrcholu up
		AddElem(buttons[4]);	//			a down
		// kvuli moznym prekryvum pri prislis male velikosti tlacitka (mensi nez mozne minimum)
		
	//	MsgSetConsume(MsgMouseL|MsgMouseOver);	// nesmim konzumovat musim preposilat az k buttonum

		SetSize(sx,sy);
		if(window)
			window->SetWindowSize(sx, sy);

		wnd->AddFrontElem(this);
		wnd->AcceptEvent(GetID(),EUpdateScrollbars);
		wnd->AcceptEvent(GetID(),EMouseWheel);

		if(corner)
		{
			corner->MsgSetProduce(MsgNone);
			corner->SetAnchor(WIDGET_FREE,WIDGET_FIX,WIDGET_FREE,WIDGET_FIX);
			wnd->AddFrontElem(corner);
		}

		if(dual)
		{
			dual->dual=this;
			if((dual->active || !dual->autohide) && (active || !autohide))	// && dual->long_version
			{
				dual->RebuildScrollbar();
			}else if(corner)
				corner->SetVisible(0);
		}

		for(i=0;i<6;i++)
			buttons[i]->RemoveFromTrash();

	}catch(CExc)
	{
		for(i=0;i<6;i++)
			SAFE_DELETE(buttons[i]);
		if(corner)
			wnd->RemoveFrontElem(corner);
		SAFE_DELETE(corner);
		throw;
	}
}


CGUIScrollBar::~CGUIScrollBar()
{
}

void CGUIScrollBar::RemoveFromEngine()
{
	CGUIMultiWidget::RemoveFromEngine();

	float dx, dy;

	if(!parent || parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollBar::RemoveFromEngine> Parent window is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) parent;

	if(active || !autohide)
	{
		switch(scrollBarType)
		{
			case Vertical	:	dx = sx;
								dy = 0;
								break;
			case Horizontal	:	dx = 0;
								dy = sy;
								break;
		}
		float ox, oy;
		ox=dx;
		oy=dy;
		wnd->ChangeViewPortBWSize(dx,dy);
		dx-=ox;dy-=oy;
		if(dx<0||dy<0)
		{
			wnd->GetSize(ox,oy);
			wnd->ResizeFrontOnly(ox+dx,oy+dy);
		}
	}

	if(corner)
	{
		wnd->DeleteFrontElem(corner);
		corner=0;
	}

	if(dual)
	{
		dual->dual=0;
		if((dual->active || !dual->autohide) && (active || !autohide))
			dual->RebuildScrollbar();
		dual = 0;
	}
	active=false;
	autohide=false;
}

void CGUIScrollBar::Resize(float _sx, float _sy)
{
	float dx, dy;

	CheckResize(_sx,_sy);
	GetSize(dx,dy);
	
	dx=_sx-dx;	// rozdily ve velikostech
	dy=_sy-dy;

	SetSize(_sx,_sy);

	if(window)
		window->SetWindowSize(sx, sy);
	
	RebuildScrollbar();
}

void CGUIScrollBar::RebuildScrollbar()
{
	// vola se pri zmene parametru okna nebo scrollbaru
	// zmena: sx, sy, vp_x/vp_y, vp_sx/vp_sy, bw_normal_size_x/bw_normal_size_y
	float width, length;
	float len, pos;
	//int i;
	bool resize = false;
	float dx, dy;
	bool old_state=active;	// stary stav "active" pred zmenou parametru scrollbaru (kvuli uprave velikosti viewportu => detekce zmeny)

	if(!parent || parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollBar::RebuildScrollbar> Parent window is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) parent;

	switch(scrollBarType)
	{
		case Vertical	:	width = sx;
							length = sy;
							dx = sx;
							dy = 0;
							break;
		case Horizontal	:	width = sy;
							length = sx;
							dx = 0;
							dy = sy;
							break;
	}

	if(dual && (dual->active || !dual->autohide ))
	{
		if(long_version)
		{
			long_version=false;
			length -=width;
			resize=true;
		}
	}else{
		if(!long_version)
		{
			long_version=true;
			length +=width;
			resize=true;
		}
	}
	
	if(resize)
	{
		switch(scrollBarType)
		{
			case Vertical	:	Resize(width, length); return;	// resize vyvola rekurzivne tuto proceduru
			case Horizontal	:	Resize(length, width); return;
		}
	}

	switch(scrollBarType)
	{
		case Vertical	:	if((wnd->vp_sy >= wnd->bw_size_normal_y) || 
								(active && dual && dual->active && wnd->vp_sy+dual->sy>= wnd->bw_size_normal_y && (wnd->vp_sx + sx >= wnd->bw_size_normal_x)))
							{	// viewport je vetsi nez okno
								// nezobrazovat? a nepouzivat scrollbary
								active=false;
								
								// button[0] beze zmeny
								buttons[4]->Move(0,length-width);

								// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
								buttons[5]->Move(0,width);
								buttons[5]->Resize(width, length - 2*width);

								if(autohide)
								{	// nezobrazovat zadnoou cast scrollbaru
									SetVisible(0);
									//for(i=0;i<6;i++)
									//	buttons[i]->SetVisible(0);
								}else{
									//SetVisible(1);
									// zobrazit jen kraje a neaktivni stred (5)
									SetVisible(1);
									buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);
								}
							}else{
								if(!active && autohide)
									SetVisible(1);
								active=true;

								//SetVisibility(1);
								// buttons[0] beze zmeny
								buttons[4]->Move(0,length-width);
								buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);

								if(slider_resize)
									len = floorf((wnd->vp_sy / wnd->bw_size_normal_y) * (length - 2*width));	// !@#$ zaokrouhlovani ????
								else
									len = width;
								if(len<width)
									len=width;	// kontrola velikosti slideru

								pos = floorf((wnd->vp_y / (wnd->bw_size_normal_y - wnd->vp_sy)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
								if((length - 2*width) < width)
								{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
									// misto nich jen neaktivni plocha
									buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									buttons[5]->Move(0,width);
									buttons[5]->Resize(width,length - 2*width);
									buttons[5]->SetVisible(1);
								}else{
									buttons[2]->Move(0,width+pos);
									buttons[2]->Resize(width,len);	
									buttons[1]->Move(0,width);
									buttons[1]->Resize(width,pos);
									buttons[3]->Move(0,width+pos+len);
									buttons[3]->Resize(width,(length-2*width)-(pos+len));
									buttons[1]->SetVisible(1);buttons[3]->SetVisible(1);buttons[2]->SetVisible(1);
									buttons[5]->SetVisible(0);
								}
							}
							break;
		case Horizontal	:	if((wnd->vp_sx >= wnd->bw_size_normal_x) ||
								(active && dual && dual->active && wnd->vp_sx+dual->sx>= wnd->bw_size_normal_x && (wnd->vp_sy + sy >= wnd->bw_size_normal_y)))
							{	// viewport je vetsi nez okno
								// nezobrazovat? a nepouzivat scrollbary
								active=false;
								
								// button[0] beze zmeny
								buttons[4]->Move(length-width,0);

								// pri neaktivnim zobrazenem scrollbaru je ve stredni casti jen neaktivni plocha
								buttons[5]->Move(width,0);
								buttons[5]->Resize(length - 2*width,width);

								if(autohide)
								{	// nezobrazovat zadnoou cast scrollbaru
									SetVisible(0);
									//for(i=0;i<6;i++)
									//	buttons[i]->SetVisible(0);
								}else{
									// zobrazit jen kraje a neaktivni stred (5)
									//SetVisibility(1);
									SetVisible(1);
									buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);buttons[5]->SetVisible(1);
								}
							}else{
								if(!active && autohide)
									SetVisible(1);
								active=true;

								//SetVisibility(1);
								// buttons[0] beze zmeny
								buttons[4]->Move(length-width,0);
								buttons[0]->SetVisible(1);buttons[4]->SetVisible(1);

								if(slider_resize)
									len = floorf((wnd->vp_sx / wnd->bw_size_normal_x) * (length - 2*width));	// !@#$ zaokrouhlovani ????
								else
									len = width;
								if(len<width)
									len=width;	// kontrola velikosti slideru

								pos = floorf((wnd->vp_x / (wnd->bw_size_normal_x - wnd->vp_sx)) * (length - 2*width - len));	// !@#$ zaokrouhlovani ????
								if((length - 2*width) < width)
								{	// scrollbar je prislis maly => nezobrazuj stredova tlacitka (1,2,3)
									// misto nich jen neaktivni plocha
									buttons[1]->SetVisible(0);buttons[2]->SetVisible(0);buttons[3]->SetVisible(0);
									buttons[5]->Move(width,0);
									buttons[5]->Resize(length - 2*width, width);
									buttons[5]->SetVisible(1);
								}else{
									buttons[2]->Move(width+pos, 0);
									buttons[2]->Resize(len, width);	
									buttons[1]->Move(width, 0);
									buttons[1]->Resize(pos, width);
									buttons[3]->Move(width+pos+len, 0);
									buttons[3]->Resize((length-2*width)-(pos+len), width);
									buttons[1]->SetVisible(1);buttons[3]->SetVisible(1);buttons[2]->SetVisible(1);
									buttons[5]->SetVisible(0);
								}
							}
							break;
	}

	// zmenseni/zvetseni viewportu o misto, ktere zabira/nezabira scrollbar
	if(autohide && old_state!=active)
	{
		float ox, oy;
		if(old_state && !active)
		{
			ox=dx;
			oy=dy;
			wnd->ChangeViewPortBWSize(dx,dy);
			dx-=ox;dy-=oy;
			if(dx<0||dy<0)
			{
				wnd->GetSize(ox,oy);
				wnd->ResizeFrontOnly(ox+dx,oy+dy);
			}
		}else if(!old_state && active)
		{
			dx = ox = - dx;
			dy = oy = - dy;
			wnd->ChangeViewPortBWSize(dx,dy);

			if(dx>ox || dy>oy)
			{
				wnd->GetSize(ox,oy);
				if(scrollBarType==Vertical && dx>-width)
					wnd->ResizeFrontOnly(width-dx+ox,oy);
				if(scrollBarType==Horizontal && dy>-width)
					wnd->ResizeFrontOnly(ox,width-dy+oy);
			}

		}
		if(dual)
			dual->RebuildScrollbar();
	}

	if(corner)
	{
		if(dual && (dual->active || !dual->autohide) && (active || !autohide))
			corner->SetVisible(1);
		else
			corner->SetVisible(0);
	}

/*
	if(dual && (dual->active || !dual->autohide) && dual->long_version && (active || !autohide))
	{
		dual->RebuildScrollbar();
	}
	if(dual && (dual->active || !dual->autohide) && !dual->long_version && !active && autohide)
	{
		dual->RebuildScrollbar();
	}
*/
}

void CGUIScrollBar::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	if(event->eventID == EUpdateScrollbars)
		RebuildScrollbar();

	if(event->eventID == EMouseWheel && active && parent && parent->GetType() >= PTWindow)
	{
		if(scrollBarType == Vertical)
		{
			((CGUIWindow*)parent)->SetWindowPositionVP(0,-event->pInt*STD_SCROLLBAR_MWHEEL_COEF*shift,false);
			RebuildScrollbar();
			mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
			mainGUI->GetEvent(0);
		}else if(!dual || (dual && !dual->active))
		{
			((CGUIWindow*)parent)->SetWindowPositionVP(-event->pInt*STD_SCROLLBAR_MWHEEL_COEF*shift,0,false);
			RebuildScrollbar();
			mainGUI->SendCursorPos();	// !@#$ aktualizace kurzoru
			mainGUI->GetEvent(0);
		}

	}

	delete(event);
}

//////////////////////////////////////////////////////////////////////
// CGUIScrollbarButton
//////////////////////////////////////////////////////////////////////

CGUIScrollbarButton::CGUIScrollbarButton(CGUIScrollBar* _sb, float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, enum EScrollBarTypes _scrollBarType, enum EScrollBarButtonTypes _scrollBarButtonType, CGUIStaticText *_label, CGUIRectHost *_picture)
	: CGUIButton(_x,_y,_sx,_sy,_up,_down,_mark,_label, _picture)
{
	//RemoveFromTrash();
	scrollBarType = _scrollBarType;
	scrollBarButtonType = _scrollBarButtonType;
	sb = _sb;
	px = py = 0;
	mouse_down=false;
	MsgAcceptConsume(MsgTimer);
	keySensitive=false;
	//AddToTrash();
}

CGUIScrollbarButton::CGUIScrollbarButton(CGUIScrollBar* _sb, float _x, float _y, float _sx, float _sy, CGUIStyle *_up, CGUIStyle *_down, CGUIStyle *_mark, bool resizable, enum EScrollBarTypes _scrollBarType, enum EScrollBarButtonTypes _scrollBarButtonType, CGUIStaticText *_label, CGUIRectHost *_picture)
	: CGUIButton(_x,_y,_sx,_sy,_up,_down,_mark, resizable, _label, _picture)
{
	//RemoveFromTrash();
	scrollBarType = _scrollBarType;
	scrollBarButtonType = _scrollBarButtonType;
	sb = _sb;
	px = py = 0;
	mouse_down=false;
	MsgAcceptConsume(MsgTimer);
	keySensitive=false;
	//AddToTrash();
}


CGUIScrollbarButton::~CGUIScrollbarButton()
{
	timerGUI->DeleteAllRequests(this);
}

void CGUIScrollbarButton::OnClick()
{
	// podle typu scrollovat
	// nepouziva se OnClick, ale reaguje se uz na zpravy nizsi urovne - MouseLeft, MouseMove, ...
	// viz. dale
}

int CGUIScrollbarButton::MouseLeft(float x, float y, UINT mouseState, UINT keyState)
{	// mouseState 1 == stisknuti cudlitka
	CGUIButton::MouseLeft(x,y,mouseState, keyState);


	if(!sb || !sb->active)
		return 0;		// scrollbar neni aktivni

	if(!sb->parent || sb->parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollbarButton::MouseLeft> Parent window is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) sb->parent;

	switch(scrollBarButtonType)
	{
		case S_Up		:	if(button_state==2)
							{
								if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,-sb->shift,false);
								else
									wnd->SetWindowPositionVP(-sb->shift,0,false);

								sb->RebuildScrollbar();

								timerGUI->DeleteRequest(this);
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER1);
							}else
								timerGUI->DeleteRequest(this);
							break;
		case S_PageUp	:	if(button_state==2)
							{
								if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,-wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(-wnd->vp_sx,0,false);

								sb->RebuildScrollbar();

								timerGUI->DeleteRequest(this);
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER1);
							}else
								timerGUI->DeleteRequest(this);
							break;
		case S_Slider	:	if(button_state==2)
							{
								mouse_down=true;
								float bx,by;
								GetPos(bx,by);
								px=x-bx;
								py=y-by;
							}else{
								mouse_down=false;
								px=0;
								py=0;
							}
							break;
		case S_PageDown	:	if(button_state==2)
							{
								if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(wnd->vp_sx,0,false);

								sb->RebuildScrollbar();

								timerGUI->DeleteRequest(this);
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER1);
							}else
								timerGUI->DeleteRequest(this);
							break;
		case S_Down		:	if(button_state==2)
							{
								if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,sb->shift,false);
								else
									wnd->SetWindowPositionVP(sb->shift,0,false);

								sb->RebuildScrollbar();

								timerGUI->DeleteRequest(this);
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER1);
							}else
								timerGUI->DeleteRequest(this);
							break;
		case S_Nothing	:	break;
	}

	if(!mouseState)
		wnd->FocusOldTOBackEl();

	return 0;
}

int CGUIScrollbarButton::MouseOver(float x, float y, UINT over,UINT state)
{
	int old_button_state = button_state;
	CGUIButton::MouseOver(x,y,over,state);

	if(!sb || !sb->active)
		return 0;		// scrollbar neni aktivni

	if(!sb->parent || sb->parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollbarButton::MouseOver> Parent window of scrollbar is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) sb->parent;

	float bx, by;
	float width, length;
	float dd;
	GetPos(bx,by);

	if(scrollBarButtonType == S_Slider && button_state==2)
	{
		switch(scrollBarType)
		{
			case Vertical :		sb->GetSize(width, length);
								if((length - 2*width - sy) && (wnd->bw_size_normal_y - wnd->vp_sy)) // kontrola kvuli deleni nulou (viz. nasledujici radek)
									dd = floorf((y-by-py) / (length - 2*width - sy) * (wnd->bw_size_normal_y - wnd->vp_sy));
								else
									dd=0;

								wnd->SetWindowPositionVP(0,dd,false);
								sb->RebuildScrollbar();
								break;
			case Horizontal :	sb->GetSize(length, width);
								if((length - 2*width - sx) && (wnd->bw_size_normal_x - wnd->vp_sx))	// kontrola kvuli deleni nulou (viz. nasledujici radek)
									dd = floorf((x-bx-px) / (length - 2*width - sx) * (wnd->bw_size_normal_x - wnd->vp_sx));
								else
									dd = 0;
								wnd->SetWindowPositionVP(dd, 0,false);
								sb->RebuildScrollbar();
								break;
		}
	}else if(scrollBarButtonType != S_Slider && old_button_state!=button_state && button_state==2)
	{	// zmenil se stav buttonu (najel jsem zpet nad tlacitko po predchozim zmacknuti a vyjeti) a tlacitko je zmacknute
		switch(scrollBarButtonType)
		{
			case S_Up		:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,-sb->shift,false);
								else
									wnd->SetWindowPositionVP(-sb->shift,0,false);
								break;
			case S_PageUp	:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,-wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(-wnd->vp_sx,0,false);
								break;
			case S_PageDown	:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(wnd->vp_sx,0,false);
								break;
			case S_Down		:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,sb->shift,false);
								else
									wnd->SetWindowPositionVP(sb->shift,0,false);
								break;
			case S_Nothing	:	break;
		}

		if(scrollBarButtonType!= S_Nothing)
		{
			sb->RebuildScrollbar();

			timerGUI->DeleteRequest(this);
			timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER1);
		}
	}else if((scrollBarButtonType == S_PageUp || scrollBarButtonType == S_PageDown) 
		&& button_state==2 && !timerGUI->ContainRequest(this))
		// zarizuje cinnost za TimerImpulse
		// v TimerImpulse nevime zda se kurzor stale naleza nad tlacitkem, to zjistime az 
		// poslanim zadosti o polohu kurzoru, pokud je odpoved kladna, dostaneme se sem a tady zaridime
		// posunuti okna, ktere mel puvodne vykonat TimerImpulse
	{
		switch(scrollBarButtonType)
		{
			case S_PageUp	:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,-wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(-wnd->vp_sx,0,false);

								sb->RebuildScrollbar();
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);

								break;

			case S_PageDown	:	if(scrollBarType == Vertical)
									wnd->SetWindowPositionVP(0,wnd->vp_sy,false);
								else
									wnd->SetWindowPositionVP(wnd->vp_sx,0,false);

								sb->RebuildScrollbar();
								timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);

								break;
		}
	}

	return 0;
}

int CGUIScrollbarButton::MouseLeftFocus(float x, float y, UINT mouseState, UINT keyState, typeID ID)
{
	CGUIButton::MouseLeftFocus(x,y,mouseState, keyState,ID);

	if(scrollBarButtonType == S_Slider)
	{
		mouse_down=false;
		px=0;
		py=0;
	}

	return 0;
}

int CGUIScrollbarButton::MouseOverFocus(float x, float y, UINT over,UINT state, typeID ID)
{
	int old_button_state = button_state;
	CGUIButton::MouseOverFocus(x,y,over,state,ID);

	if(!sb || !sb->active)
		return 0;		// scrollbar neni aktivni

	if(!sb->parent || sb->parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollbarButton::MouseOverFocus> Parent window of scrollbar is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) sb->parent;

	if(mouse_down && !(state & MK_LBUTTON))
	{
		mouse_down=0;
		px=py=0;
	}

	float bx, by;
	float width, length;
	float dd;
	GetPos(bx,by);

	if(scrollBarButtonType == S_Slider && mouse_down)
	{
		switch(scrollBarType)
		{
			case Vertical :		sb->GetSize(width, length);
								if((length - 2*width - sy) && (wnd->bw_size_normal_y - wnd->vp_sy))// kontrola kvuli deleni nulou (viz. nasledujici radek)
									dd = floorf((y-by-py) / (length - 2*width - sy) * (wnd->bw_size_normal_y - wnd->vp_sy));
								else
									dd = 0;
								wnd->SetWindowPositionVP(0,dd,false);
								sb->RebuildScrollbar();
								break;
			case Horizontal :	sb->GetSize(length, width);
								if((length - 2*width - sx) && (wnd->bw_size_normal_x - wnd->vp_sx))// kontrola kvuli deleni nulou (viz. nasledujici radek)
									dd = floorf((x-bx-px) / (length - 2*width - sx) * (wnd->bw_size_normal_x - wnd->vp_sx));
								else
									dd = 0;
								wnd->SetWindowPositionVP(dd, 0,false);
								sb->RebuildScrollbar();
								break;
		}
	}else if(scrollBarButtonType != S_Slider && scrollBarButtonType != S_Nothing && 
		old_button_state == 2 && button_state != old_button_state)
	{
		timerGUI->DeleteRequest(this);
	}

	return 0;
}

int CGUIScrollbarButton::TimerImpulse(typeID timerID, float time)
{
	if(!sb || !sb->active || button_state!=2)
		return 0;		// scrollbar neni aktivni

	if(!sb->parent || sb->parent->GetType() < PTWindow)
	{
		throw CExc(eGUI, E_INTERNAL,"CGUIScrollbarButton::TimerImpulse> Parent window of scrollbar is not found");
	}
	CGUIWindow *wnd = (CGUIWindow*) sb->parent;

	switch(scrollBarButtonType)
	{
		case S_Up		:	if(scrollBarType == Vertical)
								wnd->SetWindowPositionVP(0,-sb->shift,false);
							else
								wnd->SetWindowPositionVP(-sb->shift,0,false);
							break;
		case S_PageUp	:	mainGUI->SendCursorPos();
							return 0;
							/*
							if(scrollBarType == Vertical)
								wnd->SetWindowPositionVP(0,-wnd->vp_sy,false);
							else
								wnd->SetWindowPositionVP(-wnd->vp_sx,0,false);
								*/
							break;
		case S_PageDown	:	mainGUI->SendCursorPos();
							return 0;
							/*
							if(scrollBarType == Vertical)
								wnd->SetWindowPositionVP(0,wnd->vp_sy,false);
							else
								wnd->SetWindowPositionVP(wnd->vp_sx,0,false);
								*/
							break;
		case S_Down		:	if(scrollBarType == Vertical)
								wnd->SetWindowPositionVP(0,sb->shift,false);
							else
								wnd->SetWindowPositionVP(sb->shift,0,false);
							break;
	}

	sb->RebuildScrollbar();
	timerGUI->AddRequest(this,STD_SCROLLBAR_TIMER2);

	return 0;
}
