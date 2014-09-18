//////////////////////////////////////////////////////////////////////
//
// DXbliter.h
//
// zobrazovaci engine
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////
#ifndef DXBLITER_H
#define DXBLITER_H

#include "vector.h"
#include "texture.h"

extern class CDXbliter *DXbliter;

class CCache;

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltVertex - reprezentuje vertex
//
//////////////////////////////////////////////////////////////////////////////////////////////

#define D3DFVF_BLTVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

struct CBltVert{
    FLOAT x, y, z, rhw;
    DWORD col;
    FLOAT u, v;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CClipRect - clipovaci obdelnik
//
//////////////////////////////////////////////////////////////////////////////////////////////

struct CClipRect{
	float minx,maxx,miny,maxy;
	int clippedout;
	CClipRect(){clippedout=0;}

	int Clip(CVec2 *vert, int numvert) const;
	int ClipLine(CVec2 *vert) const;

protected:

	int Clip1Side(CVec2 *in, CVec2 *out, int numvert, int sidenum) const;

};


//////////////////////////////////////////////////////////////////////////////////////////////
//
// CBBox - obalovy obdelnik
//
//////////////////////////////////////////////////////////////////////////////////////////////

struct CBBox
{
	float minx,miny,maxx,maxy;

	char valid;

	CBBox(){valid=0;}

	void TransformBBox(const CMatrix2D *m);
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltElem
//
/////////////////////////////////////////////////////////////////////////////////////////////

//typ elementu
enum dxBltType {dxUnknown,dxWindow,dxWndHead,dxRect,dxText,dxLine,dxFormatedText,dxGEnRect,dxGEnLine};

enum dxNeedRecalcFlags {dxrcPos=1, dxrcTex=2, dxrcTextCursorBlink=4, dxrcAll=0xFF};

// virtualni predek vsech grafickych elementu
class CBltElem
{

	friend class CBltWndHead;
	friend class CBltWindow;
	friend class CDXbliter;
	friend class CCache;

protected:
	CBltElem *next, *prev; //predchozi a nasledujici element, nasledujici element se vykresluje pozdeji
	
	class CBltWindow *parent; //rodicovky okno
	
	virtual void Render(int drawall=1)=0; //nakresli element
	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1)=0; //pocita transformace, texturu, clipuje, ...

	CBltElem();

	UC needrecalc; // 1 = je potreba prepocitat trans... v Update() , nastavuje SetNeedRecalc()
	char redraw;	// 1 = ma se vykreslit
	char cached;	// 1 = je cachovany
	
	virtual void DeleteDXObjects()=0;			// zrusi DX objekty
	virtual void AssocCache(CCache *c){};		
	virtual void RemoveCache(CCache *cache)=0;

	virtual void DeleteFromCache(){};


public:
	
	virtual ~CBltElem(){};

	virtual dxBltType GetType()=0; //vrati typ elementu
	
	CBltWindow* GetParent(){return parent;} //vrati rodicovsky okno (o rootwnd=NULL)

	void SetNeedRecalc(UC rc=dxrcAll){needrecalc|=rc;} //nastavi needrecalc -> Update()

	int BringToTop();   //da element na vrchol v okne
	int SendToBottom(); //da element do spodu okna
	int BringToFront(); //posune element dopredu (o 1)
	int SendToBack();   //posune element dozadu (o 1)

	CBltElem* GetFrontElem(); //vrati element, ktery je nad (kdyz, zadny neni vraci NULL)
	CBltElem* GetBackElem();  //vrati element, ktery je pod (kdyz, zadny neni vraci NULL)

	virtual int GetBBox(CBBox &box){return -1;} //vrati obalovy obdelnik v obrazovkovych souradnicich

	void SetRedraw(){redraw=1;}	
	char GetRedraw(){return redraw;}

};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltWindow
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CBltWindow: public CBltElem
{

	friend class CDXbliter;
	friend class CBltElem;

protected:
	class CBltWndHead *child;
	float sx,sy;
	char gamewnd; //1=herni okno
	char visible;

	CClipRect *cliprect;
	
	CMatrix2D *trans;			//transformace celeho okna
	CMatrix2D *insidetrans;		//transformace vnitrku okna
	CMatrix2D *globalwndtrans;  //globalni transformace bez insidetrans
	CMatrix2D *globaltrans;		//globalni transformace s insidetrans

	virtual void Render(int drawall=1);
	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1);

	int InitWindow(CMatrix2D *mat,CMatrix2D *insidemat); //initace - vola se pouze z konstruktoru

	CCache *primarycache,*parentcache;
	
	virtual void DeleteDXObjects();
	virtual void AssocCache(CCache *c);

public:

	CBltWindow(CMatrix2D *mat=NULL,CMatrix2D *insidemat=NULL); //okno bez clipovani - pouziva rootwnd
	CBltWindow(float sx, float sy,CMatrix2D *mat=NULL,CMatrix2D *insidemat=NULL); //sx,sy - velikost okna, mat - transf. celeho okna, postmat - transf. vnitrku okna
	CBltWindow(float x1,float y1, float sx, float sy); //x1,y1 - souradnice leveho horniho rohu, sx,sy - rozmery
	
	~CBltWindow();

	virtual dxBltType GetType(){return dxWindow;} //vrati typ elementu

	void CreateCache(int gridsx,int gridsy,float minx=0,float miny=0,float maxx=0,float maxy=0);
	int ResizeCache(float minx,float miny,float maxx,float maxy);
	virtual void RemoveCache(CCache *cache);
	int AddToCache(CBltElem *el);
	
	virtual void DeleteFromCache();
	void DeletePrimitiveFromCache(CBltElem *el);

	void InvalidElCache(CBltElem *el);
	CCache *GetCache(){return primarycache;}
	CClipRect *GetClipRect(){return cliprect;}
	
	void InvalidateAllTex(); //zneplatni textury u vsech podelementu

	void GetWindowSize(float &x, float &y){x=sx;y=sy;}

	int AddChildToTop(CBltElem *el);    //prida element na vrchol okna
	int AddChildToBottom(CBltElem *el); //prida element na spod okna
	int AddChild(CBltElem *el) {return AddChildToTop(el);} //prida element na vrchol okna

	int AddChildToFrontOfEl(CBltElem *newel, CBltElem *el); //prida element nad zadany element
	int AddChildToBackOfEl(CBltElem *newel, CBltElem *el);  //prida element pod zadany element
	
	int RemoveChild(CBltElem *el); //vyhodi element z okna - neprovadi delete
	int DeleteChild(CBltElem *el); //vyhodi element z okna a provede delete
	
	void DeleteAllChild(); //smaze vsechny syny (provede delete) - nezrusi okno

	CBltElem* GetTopChild(void); //vrati nejvrchnejsi element
	CBltElem* GetBottomChild(void); //vrati nejspodnejsi element

	void SetWindowSize(float _sx,float _sy); //nastavi velikost okna

	void GetGlobalTransPtr(const CMatrix2D **mat) const {*mat=globaltrans;} //vrati globalni trasnformaci
	void GetGlobalWndTransPtr(const CMatrix2D **mat) const {*mat=globalwndtrans;} //vrati globalni trasnformaci bez insidetrans

	void GetClipRect(const CClipRect **cr) const {*cr=cliprect;} //vrati clipovaci obdelnik

	void LWindowTrans(const CMatrix2D &mat); //vynasobi zleva matici celeho okna  (trans = mat * trans)
	void RWindowTrans(const CMatrix2D &mat); //vynasobi zprava matici celeho okna (trans = trans * mat)
	void SetWindowTrans(const CMatrix2D &mat); //nastavi transformaci celeho okna
	void GetWindowTrans(CMatrix2D &mat) const; //zkopiruje transformaci celeho okna
	void GetWindowTransPtr(const CMatrix2D **p) const {*p=trans;} //vrati pointr na transformacni matici

	void LInsideTrans(const CMatrix2D &mat); //vynasobi zleva matici vnitrku okna  (insidetrans = mat * insidetrans)
	void RInsideTrans(const CMatrix2D &mat); //vynasobi zprava matici vnitrku okna (insidetrans = insidetrans * mat)
	void SetInsideTrans(const CMatrix2D &mat); //nastavi transformaci vnitrku okna
	void GetInsideTrans(CMatrix2D &mat) const; //vrati transformaci vnitrku okna
	void GetInsideTransPtr(const CMatrix2D **p) const {*p=insidetrans;} //vrati pointr na transformacni matici

	virtual int GetBBox(CBBox &box){return -1;} //vrati obalovy obdelnik

	void SetVisible(int vis); //nastavi, jestli se okno zobrzuje

	//veci pouze pro hru:
	void SetGameWindow(){gamewnd=1;} //nastavi, ze okno je herni
	int IsGameWindow(){return gamewnd;}
	int GEnBringToBottom(CBltElem *el); //hodi element dolu, neovlivnuje cache!!!


};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltWndHead
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CBltWndHead: public CBltElem
{

friend class CBltWindow;

public:

	CBltWndHead(CBltWindow* win);
	~CBltWndHead();

	void AddElemTop(CBltElem *el);
	void AddElemBottom(CBltElem *el);
	virtual void DeleteDXObjects();
	virtual void AssocCache(CCache *c);

	virtual void DeleteFromCache();

	virtual dxBltType GetType(){return dxWndHead;} //vrati typ elementu

protected:

	virtual void Render(int drawall=1);
	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1);
		
	virtual void RemoveCache(CCache *cache);

};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltPrimitive
//
/////////////////////////////////////////////////////////////////////////////////////////////


//virtualni predek pro elementy s texturou
class CBltPrimitive: public CBltElem
{
protected:
	
	CTexture *tex;
	CMatrix2D *trans;

	virtual void RemoveCache(CCache *cache){}
	virtual void DeleteDXObjects(){};

	virtual int IsCmpTex(){return 0;};
	virtual int CmpTex(){return 0;};

	char visible;

public:
	
	CBltPrimitive(){tex=NULL;trans=NULL;visible=1;}
	~CBltPrimitive(){
		SAFE_RELEASE(tex);
		SAFE_DELETE(trans);
	}

	void SetTrans(const CMatrix2D &mat); //nastavi transformaci

	virtual void Move(float dx, float dy)=0;

	void SetTexture(CTexture *texture); //nastavi texturu -> zvysi pocet referenci u textury
	CTexture* GetTexture(){return tex;} //vrati texturu (nezvysi pocet referenci u textury)

	void SetVisible(int vis) //nastaveni viditelnosti
	{
		if(vis==visible) return;
		if(vis)
		{
			SetNeedRecalc();
			visible=1;
		}else{
			DeleteFromCache();
			visible=0;
		}
	}

	virtual void DeleteFromCache();
	void InvalidCache(){if(parent) parent->InvalidElCache(this);}

};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltRect
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CBltRect: public CBltPrimitive
{
protected:
	float x1,x2,y1,y2;
	DWORD col;

	CBltVert *vertices;
	UCHAR numvert;

	virtual void Render(int drawall=1);
	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1);
	
public:

	CBltRect(){
		col=0xFFFFFFFF;
		x1=y1=x2=y2=0;
		numvert=0;
		vertices=NULL;
	}

	CBltRect(float _x1,float _y1,float sx,float sy,DWORD c=0xffffffff){
		x1=_x1;x2=x1+sx;y1=_y1;y2=y1+sy;
		col=c;
		numvert=0;
		vertices=NULL;
	}
	~CBltRect(){SAFE_DELETE(vertices);};

	virtual dxBltType GetType(){return dxRect;} //vrati typ elementu

	virtual void Move(float dx,float dy){
		DeleteFromCache();
		
		x1+=dx;x2+=dx;y1+=dy;y2+=dy;SetNeedRecalc();
	}

	void Resize(float xx,float yy){
		DeleteFromCache();
		
		x2=x1+xx;y2=y1+yy;SetNeedRecalc();
	}

	virtual int GetBBox(CBBox &box); //vrati obalovy obdelnik

	void SetCol(DWORD c){col=c;DeleteFromCache();SetNeedRecalc();} //nastavi barvu
	DWORD GetCol() { return col;};
	void SetPos(float _x1,float _y1,float sx, float sy) //nastavi pozici
	{
		DeleteFromCache();		
		x1=_x1;x2=x1+sx;y1=_y1;y2=y1+sy;SetNeedRecalc();
	}

};


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltText
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CBltText: public CBltRect
{
protected:
	char *string;
	int sz;
	HFONT font;
	int sx,sy;
	UINT TXfmt;
	char deletestring;

	virtual int CmpTex();
	virtual int IsCmpTex(){return 1;}

public:

	CBltText(float _x1,float _y1,float _sx,float _sy,char *text, int len,HFONT _font,DWORD c=0xffffffff, UINT dwDTFormat=0, char CopyString=1):CBltRect(_x1,_y1,_sx,_sy,c)
	{
		string=NULL;
		font=_font;		
		if(!ChkSize(_sx,_sy))
			SetPos(_x1,_y1,_sx,_sy);
		sx=(int)_sx;sy=(int)_sy;
		TXfmt=dwDTFormat;

		deletestring=CopyString;

		if(text)
		{
			if(len>=0) sz=len; else sz=strlen(text);

			if(CopyString)
			{
				string=new char[sz];
				memcpy(string,text,sz);
			}else
				string=text;
		}

	}
	~CBltText(){
		if(deletestring)
			SAFE_DELETE_ARRAY(string);
	}

	virtual dxBltType GetType(){return dxText;} //vrati typ elementu

	void SetText(float _x1,float _y1,float _sx,float _sy,char *text, int len,HFONT _font,DWORD c=0xffffffff, UINT dwDTFormat=0, char CopyString=1)
	{
		ChkSize(_sx,_sy);

		DeleteFromCache();

		SetPos(_x1,_y1,_sx,_sy);

		col = c;
		font = _font;
		sx=(int)_sx;sy=(int)_sy;

		TXfmt=dwDTFormat;

		if(deletestring)
			SAFE_DELETE_ARRAY(string)
		else
			string=NULL;

		if(text)
		{
			if(len>=0) sz=len; else sz=strlen(text);

			if(CopyString)
			{
				string=new char[sz];
				memcpy(string,text,sz);
			}else
				string=text;
		}	

		SAFE_RELEASE(tex);

		SetNeedRecalc();

	}

	char* GetText(){return string;}


	int ChkSize(float &sx, float &sy);

};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltLine
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CBltLine: public CBltElem
{
protected:
	
	char visible;
	DWORD col;

	CVec2 points[2];
	CBltVert vertices[2];
	UC clipstate;
	UC drawlastpoint;

	virtual void Render(int drawall=1);
	virtual int Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc=1);

	virtual void RemoveCache(CCache *cache){}
	virtual void DeleteDXObjects(){};

public:
	
	CBltLine(){
		visible=1;
		drawlastpoint=0;
	}

	CBltLine(float _x1,float _y1,float _x2,float _y2, DWORD _col=0xffffffff, UC _drawlastpoint=0){
		points[0].x=_x1;points[0].y=_y1;
		points[1].x=_x2;points[1].y=_y2;
		col=_col;
		visible=1;
		drawlastpoint=_drawlastpoint;
	}

	virtual dxBltType GetType(){return dxLine;} //vrati typ elementu

	void SetDrawLastPoint(UC _drawlastpoint){drawlastpoint=_drawlastpoint;}

	virtual void Move(float dx, float dy){DeleteFromCache();points[0].x+=dx;points[0].y+=dy;points[1].x+=dx;points[1].y+=dy;SetNeedRecalc();}

	void SetPoints(float _x1,float _y1,float _x2,float _y2) //nastavi krajni body
	{
		DeleteFromCache();SetNeedRecalc();
		points[0].x=_x1;points[0].y=_y1;
		points[1].x=_x2;points[1].y=_y2;
	}

	void SetVisible(int vis) //nastaveni viditelnosti
	{
		if(vis==visible) return;
		if(vis)
		{
			SetNeedRecalc();
			visible=1;
		}else{
			DeleteFromCache();
			visible=0;
		}
	}
	void SetCol(DWORD c){col=c;DeleteFromCache();SetNeedRecalc();} //nastavi barvu

	
	virtual int GetBBox(CBBox &box); //vrati obalovy obdelnik
	virtual void DeleteFromCache();
	void InvalidCache(){if(parent) parent->InvalidElCache(this);}

};

//////////////////////////////////////////////////////////////////////////////////////////////
//
// CDXblitter
//
//////////////////////////////////////////////////////////////////////////////////////////////

class CDXbliter
{

public:

	CBltWindow *rootwnd;

	CDXbliter();
	~CDXbliter();

	void ResizeRootWnd(float sx,float sy);

	void InitDeviceObjects(LPDIRECT3DDEVICE8 d3ddev);
	void DeleteDeviceObjects();
	void InvalidateDeviceObjects();
	void RestoreDeviceObjects();

	void Render();
	void Update(){
		//OutputDebugString("DxBlitter Update\n");
		if(rootwnd) rootwnd->Update(NULL,NULL,0);
	}

	LPDIRECT3DDEVICE8 GetD3DDevice(){return d3dDevice;}

	int CopyRectToScreen(LPDIRECT3DSURFACE8 surf, RECT *sourcerect, POINT *destpoint);
	int CopyScreenToRect(LPDIRECT3DSURFACE8 surf, RECT *sourcerect, POINT *destpoint,int sx,int sy);

	int numdr;

	D3DFORMAT *GetFormat(){return &format;}

	HDC dc;

protected:

	LPDIRECT3DDEVICE8 d3dDevice;
	LPDIRECT3DSURFACE8 backbuffer;
	D3DFORMAT format;
};

#endif