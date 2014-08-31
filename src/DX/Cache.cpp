//////////////////////////////////////////////////////////////////////
//
// cache.cpp
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>

#include "dxbliter.h"

#include "Cache.h"
#include "krkal.h"

#define MIN_NONCACHEDEL_TO_RESIZE 20
#define RESIZE_SQ 2
#define MAX_SQ (512*512)

CCache::CCache()
{
	cachewnd=NULL;
	sq=NULL;
	ncflag=0;

	findelem=0;

	LRU = 0;

	time = 1;
}

CCache::~CCache()
{
	cachewnd=NULL;
	SAFE_DELETE(LRU);
	SAFE_DELETE_ARRAY(sq);
}

void CCache::DeleteAllSurfaces()
{
	if(!sq) return;

	for(int i=0;i<sqy*sqx;i++)
	{
		sq[i].valid=0;
		sq[i].time=time;
	}
	
	LRU->DeleteAllSquares();
}

int CCache::ResizeCache(float _minx, float _miny, float _maxx, float _maxy)
{
	int dxs,dys;
	int _sqx,_sqy;
	int xx,yy,x,y;
	CListKElem<CBltElem*> *el;
	CListK<CBltElem*> _noncachedelems;
	
	CCacheSquare *_sq=NULL;

	if(!cachewnd) return 0;

	//spocte minx,miny,maxx,maxy tak, aby se zvecovalo o cele ctverce cache

	if(_minx<minx)
		dxs=-(int)ceil((minx-_minx)/gridsx);
	else
		dxs=(int)floor((_minx-minx)/gridsx);
			
	_minx=minx+dxs*gridsx;

	if(_miny<miny)
		dys=-(int)ceil((miny-_miny)/gridsy);
	else
		dys=(int)floor((_miny-miny)/gridsy);

	_miny=miny+dys*gridsy;

	_sqx=(int)ceil((_maxx-_minx)/gridsx);	//spocte novy pocet ctvercu
	_sqy=(int)ceil((_maxy-_miny)/gridsy);	

	_maxx=_minx+gridsx*_sqx;
	_maxy=_miny+gridsy*_sqy;
	
	

	if(_sqx!=sqx||_sqy!=sqy||dxs||dys){ //zmenila se velikost cache

		findelem=0; //zrusim find

		if(_sqx*_sqy>MAX_SQ) return 0;

		_sq=new CCacheSquare[_sqx*_sqy]; //vytvori se novy pole ctvercu

   		_noncachedelems.el=noncachedelems.el; //schova si elementy, ktery zasahujou mimo pole
		noncachedelems.el=NULL;
		noncachedelems.num=0;

		for(y=0,yy=-dys; y<sqy; y++,yy++) //prekopiruje data ze staryho pole
		{			

			for(x=0,xx=-dxs; x<sqx; x++,xx++)
			{
				if(xx>=0 && xx<_sqx && yy>=0 && yy<_sqy)
				{
					_sq[xx+yy*_sqx]=sq[x+y*sqx]; //kopiruju
					if(sq[x+y*sqx].surfElem)
						sq[x+y*sqx].surfElem->SetSQ(&_sq[xx+yy*_sqx]);
				}
				else
				{					
					el=sq[x+y*sqx].elemlist.el;
					while(el)
					{
						noncachedelems.AddUnique(el->data);
						el=el->next;
					}
					if(sq[x+y*sqx].surfElem)
						sq[x+y*sqx].surfElem->SetSQ(0);
				}
				sq[x+y*sqx].elemlist.el=NULL;
				
			}
		}

		delete[] sq;

		sq=_sq;
		minx=_minx; miny=_miny; maxx=_maxx; maxy=_maxy;
		sqx=_sqx;sqy=_sqy;
	
		RecalcTrans();

		el=_noncachedelems.el; //zakesuje stary noncache el.
		while(el)
		{
			el->data->cached=0;
			el=el->next;
		}

	}

	return 1;
}

int CCache::CheckResize()
{
	if(ncflag<MIN_NONCACHEDEL_TO_RESIZE) return 0;
	int cnt=0;
	float x1,y1,x2,y2;
	float xx1,yy1,xx2,yy2;
	CBBox b;

	CListKElem<CBltElem*> *el=noncachedelems.el;
	
	while(el)
	{
		cnt++;
		el=el->next;
	}

	if(cnt<MIN_NONCACHEDEL_TO_RESIZE)
	{
		ncflag=cnt;
		return 0;
	}

	x1=minx;y1=miny;x2=maxx;y2=maxy;

	el=noncachedelems.el;
	while(el)
	{
		el->data->GetBBox(b);
		xx1=b.minx-WinTranslation.x-WinMinCoord.x;
		xx2=b.maxx-1-WinTranslation.x-WinMinCoord.x;
		yy1=b.miny-WinTranslation.y-WinMinCoord.y;
		yy2=b.maxy-1-WinTranslation.y-WinMinCoord.y;

		if(xx1<x1) x1=xx1;
		if(yy1<y1) y1=yy1;
		if(xx2>x2) x2=xx2;
		if(yy2>y2) y2=yy2;
		el=el->next;
	}

	if(x1<minx) x1-=gridsx*RESIZE_SQ;
	if(y1<miny) y1-=gridsy*RESIZE_SQ;
	if(x2>maxx) x2+=gridsx*RESIZE_SQ;
	if(y2>maxy) y2+=gridsy*RESIZE_SQ;

    ncflag=0;
	return ResizeCache(x1,y1,x2,y2);
}


int CCache::AsociateWin(CBltWindow *win, float _minx, float _miny, float _maxx, float _maxy, int _gridsx, int _gridsy)
{

	SAFE_DELETE(LRU);
	SAFE_DELETE_ARRAY(sq);

	findelem=0; //zrusim find
	
	cachewnd=win;
	minx=_minx;miny=_miny;maxx=_maxx;maxy=_maxy;
	gridsx=_gridsx;gridsy=_gridsy;

	sqx=(int)ceil((maxx-minx)/gridsx);
	sqy=(int)ceil((maxy-miny)/gridsy);

	maxx=minx+gridsx*sqx;
	maxy=miny+gridsy*sqy;

	sq=new CCacheSquare[sqx*sqy];
	int maxel,scx,scy;
	KRKAL->GetScreenRes(scx,scy);
	
	maxel = ((scx*scy)/(gridsx*gridsy))*3;
	
	LRU = new CCacheLRU(this,maxel);

	RecalcTrans();

	return 1;
}

void CCache::DeasocWin()
{
	if(!cachewnd) return;
	findelem=0; //zrusim find
	cachewnd=NULL;
	SAFE_DELETE_ARRAY(sq);
}

int CCache::Add(CBltElem *el)
{
	int x1,y1,x2,y2,x,y,st;
	CCacheSquare *s;
	
	findelem=0; //zrusim find

	st=CmpCacheSq(el,x1,y1,x2,y2);
	if(st==-1) 
		return 0;

	if(st==0)
	{
		ncflag++;
		noncachedelems.AddUnique(el);
	}

	for(y=y1;y<=y2;y++)
	{
		for(x=x1;x<=x2;x++)
		{
			s=&sq[sqx*y+x];

			s->elemlist.AddUnique(el);
			s->valid=0;
			LRU->MoveElBottom(s->surfElem);
			s->time=time;
		}
	}

	return 1;
}

int CCache::InvalidEl(CBltElem *el)
{
	int x1,y1,x2,y2,x,y,st;
	CCacheSquare *s;

	findelem=0; //zrusim find
	
	st=CmpCacheSq(el,x1,y1,x2,y2);
	if(st==-1) return 0;

	for(y=y1;y<=y2;y++)
	{
		for(x=x1;x<=x2;x++)
		{
			s=&sq[sqx*y+x];
			s->valid=0;
			LRU->MoveElBottom(s->surfElem);
			s->time=time;
		}
	}
	return 1;
}

int CCache::Delete(CBltElem *el)
{
	int x1,y1,x2,y2,x,y,st;
	CCacheSquare *s;
	
	findelem=0; //zrusim find

	st=CmpCacheSq(el,x1,y1,x2,y2);
	if(st==-1) return 0;

	if(st==0)
	{
		noncachedelems.Delete(el);
	}



	for(y=y1;y<=y2;y++)
	{
		for(x=x1;x<=x2;x++)
		{
			s=&sq[sqx*y+x];

			if(s->elemlist.Delete(el))
			{
				s->valid=0;
				LRU->MoveElBottom(s->surfElem);
				s->time=time;
			}
		}
	}
	el->cached=0;
	return 1;
}

int CCache::CmpCacheSq(CBBox *box, int &x1, int &y1, int &x2, int &y2)
{
	int o=0;
	if(!box) return -1;

	x1=(int)floor((box->minx-WinTranslation.x-minx-WinMinCoord.x)/gridsx);
	if(box->maxx<=box->minx)
		x2=x1+1;
	else
	x2=(int)floor((box->maxx-1-WinTranslation.x-minx-WinMinCoord.x)/gridsx);
	
	y1=(int)floor((box->miny-WinTranslation.y-miny-WinMinCoord.y)/gridsy);
	if(box->maxy<=box->miny)
		y2=y1+1;
	else
		y2=(int)floor((box->maxy-1-WinTranslation.y-miny-WinMinCoord.y)/gridsy);


	if(x1<0) {x1=0;o=1;}
	if(y1<0) {y1=0;o=1;}
	if(x2>=sqx) {x2=sqx-1;o=1;}
	if(y2>=sqy) {y2=sqy-1;o=1;}


	if(o) return 0;

	return 1;
}

int CCache::CmpCacheSq(CBltElem *el, int &x1, int &y1, int &x2, int &y2)
{
	CBBox box;

	if(el->GetBBox(box)==-1) return -1;
	
	return CmpCacheSq(&box,x1,y1,x2,y2);
}

void CCache::RecalcTrans()
{
	const CMatrix2D *m;
	
	cachewnd->GetInsideTransPtr(&m);

	if(m){

		WinTranslation.x=m->Get_a(2,0);
		WinTranslation.y=m->Get_a(2,1);
	
	}else{

		WinTranslation.x=WinTranslation.y=0;

	}

	cachewnd->GetGlobalWndTransPtr(&m);

	CVec2 a;//,b;
	//float t;

	a.x=a.y=0;
	//cachewnd->GetWindowSize(b.x,b.y);

	//a.x = cachewnd->cliprect->minx;

/*	if(m)
	{
		a=m->TransPoint(a);
		b=m->TransPoint(b);		
		
		if(a.x>b.x) {t=b.x;b.x=a.x;a.x=t;}
		if(a.y>b.y) {t=b.y;b.y=a.y;a.y=t;}

		WinTranslation=m->TransVector(WinTranslation);

	}
	WinMinCoord=a;WinMaxCoord=b;
*/

	CClipRect *r = cachewnd->GetClipRect();

	WinMinCoord.x=r->minx;
	WinMinCoord.y=r->miny;
	WinMaxCoord.x=r->maxx;
	WinMaxCoord.y=r->maxy;
	if(m)
	{
		a=m->TransPoint(a);
		WinTranslation=m->TransVector(WinTranslation)+a-WinMinCoord;
	}else
		WinTranslation=m->TransVector(WinTranslation)-WinMinCoord;

}

int CCache::SetRedraw()
{
	CBBox b;
	CCacheSquare *s;
	CListKElem<CBltElem*> *le;

	int x1,y1,x2,y2,x,y;
	float xx,yy;
	RECT r;

	int game=cachewnd->IsGameWindow();

	b.minx=WinMinCoord.x;b.miny=WinMinCoord.y;
	b.maxx=WinMaxCoord.x;b.maxy=WinMaxCoord.y;

	CmpCacheSq(&b,x1,y1,x2,y2);

	yy=WinMinCoord.y+y1*gridsy+WinTranslation.y+miny;

	int ad=sqx-(x2-x1)-1;
	s=&sq[sqx*y1+x1];

	for(y=y1;y<=y2;y++)
	{
		
		xx=WinMinCoord.x+x1*gridsx+WinTranslation.x+minx;
		for(x=x1;x<=x2;x++)
		{
			//s=&sq[sqx*y+x];

			if(s->valid && s->GetSurface() )
			{
				r.bottom=gridsy;r.right=gridsx;
				if(yy<WinMinCoord.y) 
				{
					r.top=(int)(WinMinCoord.y-yy);					
				} else r.top=0;
				
				if(xx<WinMinCoord.x) 
				{
					r.left=(int)(WinMinCoord.x-xx);
				} else r.left=0;

				if(yy+gridsy>WinMaxCoord.y) 
				{
					r.bottom=(int)(WinMaxCoord.y-yy);
				} else r.bottom=gridsy;
				
				if(xx+gridsx>WinMaxCoord.x) 
				{
					r.right=(int)(WinMaxCoord.x-xx);
				} else r.right=gridsx;



				if(s->y1>r.top||s->x1>r.left||s->x2<r.right||s->y2<r.bottom) {
					s->valid=0;
					LRU->MoveElBottom(s->surfElem);
					s->time=time;
				}
				
			}

			if(!s->valid||!s->GetSurface())
			{				

				le=s->elemlist.el;
				while(le)
				{
					le->data->SetRedraw();
					le=le->next;
				}

				if(game)
					GEnMain->BuiltSortGraph(&s->elemlist);
			}
			
			xx+=gridsx;
			s++;
		}
		yy+=gridsy;
		s+=ad;
	}

	le=noncachedelems.el;
	if(game)
		GEnMain->BuiltSortGraph(&noncachedelems);

	while(le)
	{
		le->data->SetRedraw();
		le=le->next;
	}

	return 1;
}

int CCache::Render()
{
	if(!DXbliter)return 0;


	CBBox b;
	CCacheSquare *s;
	int x1,y1,x2,y2,x,y;
	float xx,yy;
	RECT r;
	POINT p;

	b.minx=WinMinCoord.x;b.miny=WinMinCoord.y;
	b.maxx=WinMaxCoord.x;b.maxy=WinMaxCoord.y;

	CmpCacheSq(&b,x1,y1,x2,y2);

	r.left=0;r.top=0;
	r.right=(int)gridsx;r.bottom=(int)gridsy;

	yy=WinMinCoord.y+y1*gridsy+WinTranslation.y+miny;

	int ad=sqx-(x2-x1)-1;
	s=&sq[sqx*y1+x1];

	for(y=y1;y<=y2;y++)
	{
		xx=WinMinCoord.x+x1*gridsx+WinTranslation.x+minx;
		for(x=x1;x<=x2;x++)
		{
			//s=&sq[sqx*y+x];
			
			if(s->valid&&s->GetSurface())
			{
				
				p.x=(int)xx;
				p.y=(int)yy;
				if(yy<WinMinCoord.y) 
				{
					r.top=(int)(WinMinCoord.y-yy);
					p.y=(int)WinMinCoord.y;
				} else r.top=0;
				
				if(xx<WinMinCoord.x) 
				{
					r.left=(int)(WinMinCoord.x-xx);
					p.x=(int)WinMinCoord.x;
				} else r.left=0;

				if(yy+gridsy>WinMaxCoord.y) 
				{
					r.bottom=(int)(WinMaxCoord.y-yy);
				} else r.bottom=gridsy;
				
				if(xx+gridsx>WinMaxCoord.x) 
				{
					r.right=(int)(WinMaxCoord.x-xx);
				} else r.right=gridsx;
			
				DXbliter->CopyRectToScreen(s->GetSurface(),&r,&p);
				LRU->MoveElTop(s->surfElem);
			}

			xx+=gridsx;
			s++;
		}
		yy+=gridsy;
		s+=ad;
	}

	return 1;
}

int CCache::Update()
{
	if(!DXbliter)return 0;

	time++;

	CBBox b;
	CCacheSquare *s;
	int x1,y1,x2,y2,x,y;
	float xx,yy;
	RECT r;
	POINT p;

	b.minx=WinMinCoord.x;b.miny=WinMinCoord.y;
	b.maxx=WinMaxCoord.x;b.maxy=WinMaxCoord.y;

	CmpCacheSq(&b,x1,y1,x2,y2);
	
	p.x=0;
	p.y=0;

	LPDIRECT3DSURFACE8 surf;

	int ad=sqx-(x2-x1)-1;
	s=&sq[sqx*y1+x1];
	
	yy=WinMinCoord.y+y1*gridsy+WinTranslation.y+miny;
	for(y=y1;y<=y2;y++)
	{
		xx=WinMinCoord.x+x1*gridsx+WinTranslation.x+minx;
		for(x=x1;x<=x2;x++)
		{
			//s=&sq[sqx*y+x];

			surf = s->GetSurface();
			
			if(!s->valid||!surf)
			{
				r.left=(int)xx;
				r.top=(int)yy;
				r.right=(int)(xx+gridsx);
				r.bottom=(int)(yy+gridsy);
				s->x1=0;s->y1=0;s->x2=gridsx;s->y2=gridsy;

				if(xx<WinMinCoord.x)
				{
					r.left=(int)WinMinCoord.x;
					p.x=(int)(WinMinCoord.x-xx);
					s->x1=(short)p.x;
				}else p.x=0;
				if(yy<WinMinCoord.y)
				{
					r.top=(int)WinMinCoord.y;
					p.y=(int)(WinMinCoord.y-yy);
					s->y1=(short)p.y;
				}else p.y=0;
				if(xx+gridsx>WinMaxCoord.x)
				{
					r.right=(int)WinMaxCoord.x;
					s->x2=(int)(WinMaxCoord.x-xx);
				}
				if(yy+gridsy>WinMaxCoord.y)
				{
					r.bottom=(int)WinMaxCoord.y;
					s->y2=(int)(WinMaxCoord.y-yy);
				}
				
				if(s->time+4<time)
				{
					if(!surf)
					{
						LRU->GetNewSquare(s);
						surf = s->GetSurface();
					}
					
					if( surf && DXbliter->CopyScreenToRect(surf,&r,&p,gridsx,gridsy) ) 
						s->valid=1;
				}
			}

			xx+=gridsx;
			s++;
		}
		yy+=gridsy;
		s+=ad;
	}

	return 1;
}


CBltElem* CCache::FindFirstEl(float x, float y, int &numelm)
{
	numelm=0;

	findnoncache=0;findelem=0; //zrusim find

	int x1,y1,x2,y2;
	CBBox bb;

	CCacheSquare *findsquare;

	bb.minx=x;bb.miny=y;bb.maxx=x;bb.maxy=y;
    bb.valid=1;

	int st=CmpCacheSq(&bb,x1,y1,x2,y2);
	if(st==-1) 
		return 0;

	if(st!=0)
	{
		findsquare=&sq[sqx*y1+x1];
		findelem=findsquare->elemlist.el;
		numelm=findsquare->elemlist.num;
	}
	if(!findelem)
	{
		findelem=noncachedelems.el;
		findnoncache=1;
	}

	numelm+=noncachedelems.num;
	
	if(!findelem) return 0;

	return findelem->data;
}

CBltElem* CCache::FindNextEl()
{
	if(!findelem) return 0;
	
	findelem=findelem->next;
 
	if(!findelem&&!findnoncache) //zacnu prohledavat noncache elementy
	{
		findelem=noncachedelems.el;
		findnoncache=1;
	}

	if(!findelem) return 0;

	return findelem->data;
}

///////////////////////////////////////////////////////////////////////////////////////////


CCacheSquare::~CCacheSquare()
{ 
	if(surfElem) surfElem->sq = 0; 
}

CCacheLRUElem::~CCacheLRUElem()
{ 
	SAFE_RELEASE(surface); 
	if(sq) sq->surfElem=0; 
}

void CCacheLRUElem::SetSQ(CCacheSquare *newsq)
{
	if(sq) sq->surfElem=0;
	sq=newsq;
	if(sq) sq->surfElem=this;
}

/////////////////////////////////

CCacheLRU::CCacheLRU(CCache *c, int _max)
{
	cache = c;

	head = new CCacheLRUElem();

	numel = 0;
	maxel = _max;
}

CCacheLRU::~CCacheLRU()
{
	DeleteAllSquares();
	delete head;
}

void CCacheLRU::DeleteAllSquares()
{
	CCacheLRUElem *el,*tm;
	el=head->next;
	while(el!=head){
		tm=el;
		if(tm->sq) tm->sq->surfElem=0;
		el=el->next;
		delete tm;
	}
	head->next=head;
	head->prev=head;
	numel=0;
}

void CCacheLRU::DeleteSquare(CCacheSquare *sq)
{
	if(!sq) return;

	CCacheLRUElem *el = sq->surfElem;
	if(!el) return;

	el->sq = 0;
	sq->surfElem = 0;

	//vyndam ze seznamu
	el->prev->next=el->next;
	el->next->prev=el->prev;

	numel--;
	delete el;
}

int CCacheLRU::GetNewSquare(CCacheSquare *sq)
{
	CCacheLRUElem *el;

	if(numel<maxel)
	{
		el=new CCacheLRUElem();

		DXbliter->GetD3DDevice()->CreateRenderTarget( cache->gridsx,cache->gridsy,
													*DXbliter->GetFormat(),D3DMULTISAMPLE_NONE,TRUE,
													&el->surface);
		if(!el->surface) {
			delete el;
			return 0;
		}

		numel++;

		el->sq = sq;
		sq->surfElem = el;

		el->prev=head;
		el->next=head->next;
		el->prev->next=el;
		el->next->prev=el;
	}else
	{
		el=head->prev;

		if(el->sq) el->sq->surfElem=0;
		el->sq = sq;
		sq->surfElem =el;

		MoveElTop(el);
	}

	return 1;
}

void CCacheLRU::MoveElTop(CCacheLRUElem *el)
{
	assert(el);
	//vyndam ze seznamu
	el->prev->next=el->next;
	el->next->prev=el->prev;
	//nastavim pointry na sousedy
	el->prev = head;
	el->next = head->next;
	//nastavi aby sousedi ukazovali na me
	el->prev->next = el;
	el->next->prev = el;
}
void CCacheLRU::MoveElBottom(CCacheLRUElem *el)
{
	if(!el) return;
	//vyndam ze seznamu
	el->prev->next=el->next;
	el->next->prev=el->prev;
	//nastavim pointry na sousedy
	el->next = head;
	el->prev = head->prev;
	//nastavi aby sousedi ukazovali na me
	el->prev->next = el;
	el->next->prev = el;
}
