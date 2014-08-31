//////////////////////////////////////////////////////////////////////
//
// cache.h
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef CACHE_H
#define CACHE_H

#include "types.h"
#include "dx.h"
#include "dxbliter.h"

class CCache;
struct CCacheLRUElem;
class CCacheSquare;

struct CCacheLRUElem{
	IDirect3DSurface8 *surface;
	CCacheSquare *sq;
	CCacheLRUElem *prev,*next;

	void SetSQ(CCacheSquare *newsq);

	CCacheLRUElem(){ prev=next=this;surface=0;sq=0; }
	~CCacheLRUElem();
};


class CCacheSquare
{
/*	friend class CCache;
	friend class CCacheLRU;
	friend struct CCacheLRUElem;
*/
public:
	CCacheSquare(){valid=0;surfElem=NULL;time=0;}
	~CCacheSquare();
/*protected:*/
	CListK<CBltElem*> elemlist;
	CCacheLRUElem *surfElem;

	char valid;
	short x1,y1,x2,y2;
	UI time;


	IDirect3DSurface8 *GetSurface(){if(!surfElem)return 0; return surfElem->surface;}
};

class CCacheLRU{
public:
	CCacheLRU(CCache *cache, int maxel);
	~CCacheLRU();

	void DeleteSquare(CCacheSquare *sq);
	int GetNewSquare(CCacheSquare *sq);

	void DeleteAllSquares();

	void MoveElTop(CCacheLRUElem *el);
	void MoveElBottom(CCacheLRUElem *el);

private:

	CCacheLRUElem *head;
	CCache *cache;
	int numel,maxel;
};


class CCache
{
	friend class CCacheLRU;
public:
	CCache();
	~CCache();

	int AsociateWin(CBltWindow *win, float minx, float miny, float maxx, float maxy, int gridsx, int gridsy);
	void DeasocWin();
	
	int Add(CBltElem *el);
	int Delete(CBltElem *el);
	int InvalidEl(CBltElem *el);

	void RecalcTrans();

	int Render();
	int Update();
	int SetRedraw();

	void DeleteAllSurfaces();

	int ResizeCache(float minx,float miny,float maxx, float maxy);
	int CheckResize();

	CBltElem* FindFirstEl(float x, float y, int &numelm);
	CBltElem* FindNextEl();

protected:

	int CmpCacheSq(CBltElem *el, int &minx, int &miny, int &maxx, int &maxy);
	int CmpCacheSq(CBBox *b, int &x1, int &y1, int &x2, int &y2);

	CBltWindow *cachewnd;
	
	CVec2 WinTranslation;			//posunuti ve obraz. souradnicich
	CVec2 WinMinCoord,WinMaxCoord;	//min a max souradnice okna na obrazovce


	float minx,miny,maxx,maxy;
	
	int gridsx,gridsy;
	int sqx,sqy;

	CCacheSquare *sq;

	CListK<CBltElem*> noncachedelems;
	int ncflag;

	int findnoncache;
	CListKElem<CBltElem*> *findelem;

	CCacheLRU *LRU;


	UI time;
};


#endif
