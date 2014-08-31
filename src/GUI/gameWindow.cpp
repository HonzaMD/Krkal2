///////////////////////////////////////////////
//
// gameWindow.cpp
//
//	implementace pro propojeni GUI s game window enginu
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "gameWindow.h"
#include "editor.h"
#include "desktop.h"
#include "primitives.h"

#include "genmain.h"
#include "kernel.h"
#include "levelmap.h"
#include "ComConstants.h"


CGUIGameWindow::CGUIGameWindow(float _x, float _y, float _sx, float _sy)
	: CGUIStdWindow(_x,_y,_sx,_sy,"Game",new CGUIRectHost(0,0,styleSet->Get("EI_Map")->GetTexture(0)),1,STD_RESIZER_COLOR,1,STD_GAMEWINDOW_BACKGROUND_COLOR)
{
	shiftX = shiftY = 0;

	GEnMain->AddToWindow(back);	// prida herni okno dovnitr GUI okna

	GEnMain->SetWindowPos(0,0,vp_sx,vp_sy);	// nastavi pozici a velikost vykreslovane plochy

	markerWin = new CBltWindow(bw_x,bw_y,bw_size_x,bw_size_y);	// vytvoreni okna pro oznacovani casti mapy
	if(front && markerWin)										// vrstva nad hrou, ale v okne
		front->AddChild(markerWin);
}


CGUIGameWindow::~CGUIGameWindow()
{
	GEnMain->AddToWindow(0);		// pred znicenim GUI okna se musi odebrat herni okno

	if(markerWin){
		CBltWindow *wnd = markerWin->GetParent();
		if(wnd){
			wnd->DeleteChild(markerWin);
		}else{
			delete markerWin;
		}
		markerWin=0;
	}
}

void CGUIGameWindow::ChangeViewPortBWSize(float &dx, float &dy)
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
	{
		GEnMain->SetWindowPos(0,0,vp_sx,vp_sy);	// PREPROGRAMOVANA CAST -----------------------------
		back->SetWindowSize(vp_sx,vp_sy);		// zmena velikost vp se provadi zmenou velikosti back
		markerWin->SetWindowSize(vp_sx,vp_sy);
	}
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

void CGUIGameWindow::SetWindowPositionVP(float dx, float dy, bool updateScrollbars)
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
		GEnMain->SetScroll(vp_x-shiftX,vp_y-shiftY);  // PREPROGRAMOVANA CAST -----------------------------
		
		CMatrix2D tm;
		tm.Translate(-vp_x, -vp_y);
		markerWin->SetInsideTrans(tm);
		
	}
	if(updateScrollbars)
		EventArise(EUpdateScrollbars);
}

void CGUIGameWindow::GameWindowScroll(float dx, float dy)
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
		//GEnMain->SetScroll(vp_x-shiftX,vp_y-shiftY);  // s hernim oknem jiz nehybat, s tim jiz scrolloval kernel
		
		// posunuti okna nad hrou (oznacovaci okno)
		CMatrix2D tm;
		tm.Translate(-vp_x, -vp_y);
		markerWin->SetInsideTrans(tm);
		
	}

	EventArise(EUpdateScrollbars);	// aktualizace scrollbaru
}

void CGUIGameWindow::SetShift(float shift_x, float shift_y)
{
	shiftX = shift_x;
	shiftY = shift_y;

	SetWindowPositionVP(0,0);
}

void CGUIGameWindow::GetShift(float &shift_x, float &shift_y)
{
	shift_x=shiftX;
	shift_y=shiftY;
}

