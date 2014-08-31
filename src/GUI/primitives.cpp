///////////////////////////////////////////////
//
// primitives.cpp
//
// Interface pro zakladni stavebni prvky ostatnich elementu GUI - buttony, ...
// RectHost (spravce textury), Standard9 (spravce 9 textur, napr. buttonu)
//
// A: Jan Poduska
//
///////////////////////////////////////////////

#include "stdafx.h"

#include "primitives.h"
#include "gui.h"

#include "dxbliter.h"

//////////////////////////////////////////////////////////////////////
// CGUIRectHost
//////////////////////////////////////////////////////////////////////

CGUIRectHost::CGUIRectHost(float _x, float _y, char *texName, float _sx, float _sy, UINT color)
	: CGUIElement(_x, _y)
{
	UINT ssx, ssy;
	CTexture *texture = new CTexture;
	rect = 0;
	bool released = false;

	if ( !( texture->CreateFromFile(texName) ) )
	{
		texture->Release();
		throw CExc(eGUI,E_FILE_NOT_FOUND,"CGUIRectHost::CGUIRectHost> Cannot find texture: %s",texName);
	}

	try{
		texture->GetImageSize(&ssx,&ssy);
		if(_sx>0)
			ssx=(UINT)_sx;
		if(_sy>0)
			ssy=(UINT)_sy;

		rect = new CBltRect(_x, _y, (float)ssx, (float)ssy, color);
		rect->SetTexture(texture);
		texture->Release();
		released=true;

		SetSize((float)ssx,(float)ssy);
		SetPos(_x,_y);

		type = PTRectHost;
		AddToTrash();
	}catch(CExc)
	{
		if(!released)
			texture->Release();
		SAFE_DELETE(rect);
		throw;
	}
}


CGUIRectHost::CGUIRectHost(float _x, float _y, CTexture *texture, float _sx, float _sy, UINT color)
	: CGUIElement(_x, _y)
{
	UINT ssx=0, ssy=0;
	rect = 0;

	try{
		if(texture)
			texture->GetImageSize(&ssx,&ssy);
		if(_sx>0)
			ssx=(UINT)_sx;
		if(_sy>0)
			ssy=(UINT)_sy;

		rect = new CBltRect((float)_x, (float)_y, (float)ssx, (float)ssy, color);
		if(texture)
			rect->SetTexture(texture);

		SetSize((float)ssx,(float)ssy);
		SetPos(_x,_y);
		type = PTRectHost;
		AddToTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		throw;
	}
}

CGUIRectHost::CGUIRectHost(float _x, float _y, float _sx, float _sy, UINT color)
	: CGUIElement(_x, _y)
{
	type = PTRectHost;
	rect = 0;
	try{
		rect = new CBltRect((float)_x, (float)_y, (float)_sx, (float)_sy, color);
		SetSize(_sx,_sy);
		SetPos(_x,_y);
		AddToTrash();
	}catch(CExc)
	{
		SAFE_DELETE(rect);
		throw;
	}
}

CGUIRectHost::CGUIRectHost(CGUIRectHost &rectHost)
	: CGUIElement(0,0)
{
	rectHost.GetPos(x,y);
	rectHost.GetSize(sx,sy);
	rect = new CBltRect(x,y,sx,sy,rectHost.rect->GetCol());
	rect->SetTexture(rectHost.rect->GetTexture());
}

CGUIRectHost::~CGUIRectHost()
{
	RemoveFromTrash();
	if(rect)
	{
		CBltWindow *wnd = rect->GetParent();
		if(wnd)
			wnd->DeleteChild(rect);
		else
			delete rect;
		rect = 0;
	}
}


void CGUIRectHost::AddToEngine(CBltWindow *rootwnd)
{
	if(rootwnd && rect)
		rootwnd->AddChild(rect);
}

void CGUIRectHost::AddToEngineBackPosition(CBltWindow *rootwnd, CBltElem *pos)
{
	if(rootwnd && rect && pos)
		rootwnd->AddChildToBackOfEl(rect,pos);
}

void CGUIRectHost::RemoveFromEngine()
{
	if(!rect)
	{
		throw CExc(eGUI,E_INTERNAL,"CGUIRectHost::RemoveFromEngine> rect not found");
	}

	CBltWindow *wnd = rect->GetParent();
	if(wnd)
		wnd->RemoveChild(rect);
	else
	{
		throw CExc(eGUI,E_INTERNAL,"CGUIRectHost::RemoveFromEngine> rect not found in window");
	}
}

void CGUIRectHost::BringToTop()
{
	if(rect)
		rect->BringToTop();
}


void CGUIRectHost::Resize(float _sx, float _sy)
{
	SetSize(_sx,_sy);
	rect->Resize(sx,sy);
}

void CGUIRectHost::Move(float _x, float _y)
{
	rect->Move(_x-x,_y-y);
	SetPos(_x,_y);
}

void CGUIRectHost::Rotate(float alfa)
{
	
	CMatrix2D r,p,pp,c;
	r.Rotate(alfa);
	p.Translate(-x,-y-sy);
	pp.Translate(x,y);
	c = p*r*pp;
	

	rect->SetTrans(c);

//	SetSize();

}


void CGUIRectHost::SetVisible(int vis)
{
	rect->SetVisible(vis);
	SetVisibility(vis);
}

void CGUIRectHost::SetTexture(CTexture *tex)
{
	if(rect)
		rect->SetTexture(tex);
}

void CGUIRectHost::SetColor(UINT color)
{
	if(rect)
		rect->SetCol(color);
}

void CGUIRectHost::RebuildRect(CTexture *tex, float _x, float _y, float _sx, float _sy)
{
	if(rect)
	{
		rect->Move(_x-x,_y-y);
		SetPos(_x,_y);
		if(_sx>=0 && _sy>=0)
		{
			rect->Resize(_sx,_sy);
			SetSize(_sx,_sy);
		}
		if(tex)
		{
			if(_sx<0 || _sy<0)
			{
				UINT ssx, ssy;
				tex->GetImageSize(&ssx,&ssy);
				rect->Resize((float)ssx,(float)ssy);
				SetSize((float)ssx,(float)ssy);
			}
		}
		rect->SetTexture(tex);
	}
}

CBltElem* CGUIRectHost::GetElemPosition()
{
	if(rect)
		return rect->GetFrontElem();
	else
		return 0;
}

CTexture* CGUIRectHost::GetTexture()
{
	if(rect)
		return rect->GetTexture();
	else
		return 0;
}

UINT CGUIRectHost::GetColor()
{
	if(rect)
		return rect->GetCol();
	else return 0;
}

//////////////////////////////////////////////////////////////////////
// CGUIStandard9
//////////////////////////////////////////////////////////////////////

CGUIStandard9::CGUIStandard9(float _x, float _y, CGUIStyle *style)
	: CGUIPrimitive() 
{
	CTexture *t;
	UINT _sx, _sy;
	float max1=0,max2=0,maxx;
	float x1,x2;
	float y1,y2,y3;

	SetPos(_x,_y);

	for(int i=0;i<9;i++)
		rects[i]=0;

	if(!style)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS, "CGUIStandard9::CGUIStandard9> Style is missing");
	}


	if(style->GetNumTextures()<9)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStandard9::CGUIStandard9> Style hasn't enought textures");
	}
	
	for(int i=1;i<=3;i++)
	{
		t = style->GetTexture(i);
		t->GetImageSize(&_sx,&_sy);
		max1+=(float)_sx;
	}
	for(int i=6;i<=8;i++)
	{
		t = style->GetTexture(i);
		t->GetImageSize(&_sx,&_sy);
		max2+=(float)_sx;
	}

	try{
		maxx = (max1>max2) ? max1 : max2;
		maxx +=x;

		t = style->GetTexture(1);
		t->GetImageSize(&_sx,&_sy);
		x1=(float)(x+_sx);
		y1=(float)(y+_sy);
		rects[1] = new CGUIRectHost(x,y,t);

		t = style->GetTexture(2);
		t->GetImageSize(&_sx,&_sy);
		x2=(float)(x1+_sx);
		rects[2] = new CGUIRectHost(x1,y,t,(float)_sx,y1-y);

		t = style->GetTexture(3);
		t->GetImageSize(&_sx,&_sy);
		x2=(float)(maxx-_sx);
		rects[2]->Resize(x2-x1,y1-y);
		rects[3] = new CGUIRectHost(x2,y,t,(float)_sx,y1-y);

		t = style->GetTexture(4);
		t->GetImageSize(&_sx,&_sy);
		y2=(float)(y1+_sy);
		rects[4] = new CGUIRectHost(x,y1,t,x1-x,(float)_sy);

		t = style->GetTexture(0);
		rects[0] = new CGUIRectHost(x1,y1,t,x2-x1,y2-y1);

		t = style->GetTexture(5);
		rects[5] = new CGUIRectHost(x2,y1,t,maxx-x2,y2-y1);

		t = style->GetTexture(6);
		t->GetImageSize(&_sx,&_sy);
		y3=(float)(y2+_sy);
		x1=(float)(x+_sx);
		rects[6] = new CGUIRectHost(x,y2,t,(float)_sx,(float)_sy);

		t = style->GetTexture(7);
		t->GetImageSize(&_sx,&_sy);
		x2=(float)(x1+_sx);
		rects[7] = new CGUIRectHost(x1,y2,t,(float)_sx,y3-y2);

		t = style->GetTexture(8);
		t->GetImageSize(&_sx,&_sy);
		x2=(float)(maxx-_sx);
		rects[7]->Resize(x2-x1,y3-y2);
		rects[8] = new CGUIRectHost(x2,y2,t,(float)_sx,y3-y2);

		min_x = maxx-x;
		min_y = y3 - y;
		SetSize(min_x,min_y);
		for( int i=0; i<9; i++)
		{
			rects[i]->RemoveFromTrash();
		}
	}catch(CExc)
	{
		for( int i=0; i<9; i++)
		{
			SAFE_DELETE(rects[i]);
		}
		throw;
	}
}

CGUIStandard9::~CGUIStandard9()
{
	for( int i=0; i<9; i++)
	{
		SAFE_DELETE(rects[i]);
	}
}

void CGUIStandard9::SetVisible(int vis)
{
	SetVisibility(vis);
	for( int i=0; i<9; i++)
	{
		if(rects[i])
			rects[i]->SetVisible(vis);
	}
}

void CGUIStandard9::Resize(float _sx, float _sy)
{

//	CheckResize(_sx,_sy);
	if(_sx >= min_x)
	{
		rects[2]->ResizeRel(_sx-sx,0);
		rects[7]->ResizeRel(_sx-sx,0);
		rects[0]->ResizeRel(_sx-sx,0);
		rects[3]->MoveRel(_sx-sx,0);
		rects[5]->MoveRel(_sx-sx,0);
		rects[8]->MoveRel(_sx-sx,0);
		SetSize(_sx,sy);
	}else
		Resize(min_x, _sy);
	if(_sy >= min_y)
	{
		rects[4]->ResizeRel(0,_sy-sy);
		rects[5]->ResizeRel(0,_sy-sy);
		rects[0]->ResizeRel(0,_sy-sy);
		rects[6]->MoveRel(0,_sy-sy);
		rects[7]->MoveRel(0,_sy-sy);
		rects[8]->MoveRel(0,_sy-sy);
		SetSize(sx,_sy);
	}else
		Resize(_sx, min_y);
}

void CGUIStandard9::Move(float _x, float _y)
{
//	CheckPosition(_x,_y);
	for(int i=0;i<9;i++)
		if(rects[i])
			rects[i]->MoveRel(_x-x,_y-y);
	SetPos(_x,_y);
}

void CGUIStandard9::AddToEngine(CBltWindow *rootwnd)
{
	for( int i=0; i<9; i++)
	{
		if(rects[i])
			rects[i]->AddToEngine(rootwnd);
	}
}

void CGUIStandard9::RemoveFromEngine()
{
	for( int i=0; i<9; i++)
	{
		if(rects[i])
			rects[i]->RemoveFromEngine();
	}
}

void CGUIStandard9::BringToTop()
{
	for( int i=0; i<9; i++)
	{
		if(rects[i])
			rects[i]->BringToTop();
	}
}

void CGUIStandard9::ReTexturize(CGUIStyle *style)
{
	if(style && style->GetNumTextures()>=9)
	{
		for(int i=0;i<9;i++)
			if(rects[i])
				rects[i]->SetTexture(style->GetTexture(i));
	}else{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStandard9::ReTexturize> Style is missing or hasn't enought textures");
	}
}

void CGUIStandard9::SetStyle(CGUIStyle *style, float dx, float dy)
{
	CTexture *t;
	UINT _sx, _sy;
	float max1=0,max2=0,maxx;
	float x1,x2;
	float y1,y2,y3;

	if(!style || style->GetNumTextures()<9)
	{
		throw CExc(eGUI, E_BAD_ARGUMENTS,"CGUIStandard9::SetStyle> Style is missing or hasn't enought textures");
	}
	
	for(int i=1;i<=3;i++)
	{
		t = style->GetTexture(i);
		t->GetImageSize(&_sx,&_sy);
		max1+=(float)_sx;
	}
	for(int i=6;i<=8;i++)
	{
		t = style->GetTexture(i);
		t->GetImageSize(&_sx,&_sy);
		max2+=(float)_sx;
	}

	maxx = (max1>max2) ? max1 : max2;
	maxx +=x;

	t = style->GetTexture(1);
	t->GetImageSize(&_sx,&_sy);
	x1=(float)(x+_sx);
	y1=(float)(y+_sy);
	rects[1]->RebuildRect(t,x,y);

	t = style->GetTexture(2);
	t->GetImageSize(&_sx,&_sy);
	x2=(float)(x1+_sx);
	rects[2]->RebuildRect(t,x1,y,(float)_sx,y1-y);

	t = style->GetTexture(3);
	t->GetImageSize(&_sx,&_sy);
	x2=(float)(maxx-_sx);
	rects[2]->Resize(x2-x1,y1-y);
	rects[3]->RebuildRect(t,x2,y,(float)_sx,y1-y);

	t = style->GetTexture(4);
	t->GetImageSize(&_sx,&_sy);
	y2=(float)(y1+_sy);
	rects[4]->RebuildRect(t,x,y1,x1-x,(float)_sy);

	t = style->GetTexture(0);
	rects[0]->RebuildRect(t,x1,y1,x2-x1,y2-y1);

	t = style->GetTexture(5);
	rects[5]->RebuildRect(t,x2,y1,maxx-x2,y2-y1);

	t = style->GetTexture(6);
	t->GetImageSize(&_sx,&_sy);
	y3=(float)(y2+_sy);
	x1=(float)(x+_sx);
	rects[6]->RebuildRect(t,x,y2,(float)_sx,(float)_sy);

	t = style->GetTexture(7);
	t->GetImageSize(&_sx,&_sy);
	x2=(float)(x1+_sx);
	rects[7]->RebuildRect(t,x1,y2,(float)_sx,y3-y2);

	t = style->GetTexture(8);
	t->GetImageSize(&_sx,&_sy);
	x2=(float)(maxx-_sx);
	rects[7]->Resize(x2-x1,y3-y2);
	rects[8]->RebuildRect(t,x2,y2,(float)_sx,y3-y2);

	min_x = maxx-x;
	min_y = y3 - y;
	float ssx, ssy;
	GetSize(ssx,ssy);
	SetSize(min_x,min_y);
	Resize(ssx+dx,ssy+dy);
}

//////////////////////////////////////////////////////////////////////
// CGUIStdMarker
//////////////////////////////////////////////////////////////////////

CGUIStdMarker::CGUIStdMarker(CBltWindow *rootwnd, CGUIElement* elm)
{
	float x1,y1,x2,y2;
	elm->GetPos(x1,y1);
	elm->GetSize(x2,y2);
	x2+=x1;
	y2+=y1;

	x1+=-3;
	y1+=-2;
	x2+=2;
	y2+=1;

	UINT linesColor = 0xFFFF0000;
	lines[0] = new CBltLine(x1,y1,x2,y1,linesColor);
	lines[1] = new CBltLine(x2,y1,x2,y2,linesColor);
	lines[2] = new CBltLine(x2,y2,x1,y2,linesColor);
	lines[3] = new CBltLine(x1,y2,x1,y1,linesColor);

	for(int i=0;i<4;i++)
		if(lines[i])
			rootwnd->AddChild(lines[i]);
	visible=true;
}

CGUIStdMarker::~CGUIStdMarker()
{
	CBltWindow *wnd=0;
	if(lines[0])
		wnd = lines[0]->GetParent();
	if(wnd)
	{
		for(int i=0;i<4;i++)
		{
			if(lines[i])
				wnd->DeleteChild(lines[i]);
		}
	}else{
		for(int i=0;i<4;i++)
			SAFE_DELETE(lines[i]);
	}
}

void CGUIStdMarker::Mark()
{
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(1);
	visible=true;
}

void CGUIStdMarker::UnMark()
{
	for(int i=0;i<4;i++)
		if(lines[i])
			lines[i]->SetVisible(0);
	visible=false;
}

void CGUIStdMarker::Rebuild(CGUIElement* elm)
{
	float x1,y1,x2,y2;
	elm->GetPos(x1,y1);
	elm->GetSize(x2,y2);
	x2+=x1;
	y2+=y1;

	x1+=-3;
	y1+=-2;
	x2+=2;
	y2+=1;

	if(lines[0])
		lines[0]->SetPoints(x1,y1,x2,y1);
	if(lines[1])
		lines[1]->SetPoints(x2,y1,x2,y2);
	if(lines[2])
		lines[2]->SetPoints(x2,y2,x1,y2);
	if(lines[3])
		lines[3]->SetPoints(x1,y2,x1,y1);
}

void CGUIStdMarker::OrderEvents(CGUIEventSupport* es)
{
	if(es)
	{
		es->AcceptEvent(GetID(),EPositionChanged);
		es->AcceptEvent(GetID(),ESizeChanged);
	}
}

void CGUIStdMarker::EventHandler(CGUIEvent *event)
{
	if(!event)
		return;

	CGUIElement* e;

	if(event->eventID == EPositionChanged)
	{
		e = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->sender));
		if(e)
		{
			// zmena polohy car
			Rebuild(e);

		}
	}

	if(event->eventID == ESizeChanged)
	{
		e = dynamic_cast<CGUIElement*>(nameServer->ResolveID(event->sender));
		if(e)
		{
			// zmena polohy a delky car
			Rebuild(e);
		}
	}

	delete event;
}
