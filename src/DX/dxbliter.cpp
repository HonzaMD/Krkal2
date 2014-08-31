//////////////////////////////////////////////////////////////////////
//
// DXblitter.cpp
//
// zobrazovaci engine
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "cache.h"
#include "dxbliter.h"


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBBox
//
/////////////////////////////////////////////////////////////////////////////////////////////


void CBBox::TransformBBox(const CMatrix2D *m)
{
	if(!m) return;
	
	CVec2 v[4];

	v[0]=m->TransPoint(minx,miny);
	v[1]=m->TransPoint(maxx,miny);
	v[2]=m->TransPoint(minx,maxy);
	v[3]=m->TransPoint(maxx,maxy);

	minx=v[0].x;maxx=v[0].x;
	miny=v[0].y;maxy=v[0].y;

	for(int i=1;i<4;i++)
	{
		if(v[i].x<minx) minx=v[i].x;
		if(v[i].y<miny) miny=v[i].y;
		if(v[i].x>maxx) maxx=v[i].x;
		if(v[i].y>maxy) maxy=v[i].y;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CDXblitter
//
/////////////////////////////////////////////////////////////////////////////////////////////

class CDXbliter* DXbliter=NULL;

// konstruktor & destruktor ------------------------------------------------------ CDXBlitter

CDXbliter::CDXbliter()
{
	
	d3dDevice=NULL;

	int sx,sy;

    dc  = CreateCompatibleDC( NULL );

	g_pDXapp->GetScreenRes(sx,sy);

	rootwnd = new CBltWindow((float)sx,(float)sy);

	DXbliter=this;

}

CDXbliter::~CDXbliter()
{
	if(dc) DeleteDC(dc);

	DXbliter=NULL;

	if(rootwnd){
		delete rootwnd; rootwnd=NULL;
	}
	DeleteDeviceObjects();
}

// initace a destrukce veci zavislych na DirectX -------------------------------- CDXBlitter

void CDXbliter::InitDeviceObjects(LPDIRECT3DDEVICE8 d3ddev)
{


	d3dDevice=d3ddev;

	RestoreDeviceObjects();

}

void CDXbliter::DeleteDeviceObjects()
{
	d3dDevice=NULL;
}

void CDXbliter::InvalidateDeviceObjects()
{
	if(rootwnd) rootwnd->DeleteDXObjects();
}


void CDXbliter::RestoreDeviceObjects()
{
/*
	float WindowWidth=640;
	float WindowHeight=480;

	D3DXMATRIX Ortho2D;	
	D3DXMATRIX Identity;

 	D3DXMatrixOrthoOffCenterRH(&Ortho2D, 0,WindowWidth, -(WindowHeight),0, -1.0f, 1.0f);

	Ortho2D(1,1)=-Ortho2D(1,1);
	
	D3DXMatrixIdentity(&Identity);

	d3dDevice->SetTransform(D3DTS_PROJECTION, &Ortho2D);
	d3dDevice->SetTransform(D3DTS_WORLD, &Identity);
	d3dDevice->SetTransform(D3DTS_VIEW, &Identity);
*/
	d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	d3dDevice->SetRenderState(D3DRS_CLIPPING, FALSE);


	d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,  TRUE);
	d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	
	d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

	d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	
	D3DDISPLAYMODE Mode;
	d3dDevice->GetDisplayMode(&Mode);
	format=Mode.Format;

}

void CDXbliter::ResizeRootWnd(float sx,float sy)
{
	if(rootwnd)
		rootwnd->SetWindowSize(sx,sy);
}

// rendering ---------------------------------------------------------------------- CDXBlitter

void CDXbliter::Render()
{
	
	numdr=0;

	HRESULT hr;

	d3dDevice->SetVertexShader(D3DFVF_BLTVERTEX);


	hr=d3dDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&backbuffer);
	if(hr!=D3D_OK) return;
	

	//rootwnd->Update(NULL,NULL,0); 

	rootwnd->Render();
	backbuffer->Release();

	d3dDevice->SetTexture(0,NULL);

	DebugMessage(15,"%i",numdr);

}


int CDXbliter::CopyRectToScreen(LPDIRECT3DSURFACE8 surf,  RECT *sourcerect, POINT *destpoint)
{

	HRESULT hr;

	if(!surf) return 0;

	hr=d3dDevice->CopyRects(surf,sourcerect,1,backbuffer,destpoint);

	if(hr!=D3D_OK)
		return 0;


	return 1;
}
int CDXbliter::CopyScreenToRect(LPDIRECT3DSURFACE8 surf, RECT *sourcerect, POINT *destpoint, int sx,int sy)
{

	HRESULT hr;

	if(!surf) return 0;

/*	if(!*surf){
		//d3dDevice->CreateImageSurface(sx,sy,format,surf);
		d3dDevice->CreateRenderTarget(sx,sy,format,D3DMULTISAMPLE_NONE,TRUE,surf);
		if(!*surf) return 0;
	}
*/
	hr=d3dDevice->CopyRects(backbuffer,sourcerect,1,surf,destpoint);


	if(hr!=D3D_OK)
		return 0;
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltElem
//
/////////////////////////////////////////////////////////////////////////////////////////////

// konstruktor & destruktor -------------------------------------------------------- CBltElem

CBltElem::CBltElem()
{
	next=prev=NULL;parent=NULL;

	cached=0;
	redraw=0;

	SetNeedRecalc();
}

int CBltElem::BringToTop()
{
	CBltWindow *w=parent;

	if(!(parent&&next&&prev)) return 0;

	CBltWndHead *h=w->child;

/*	w->RemoveChild(this);
	w->AddChildToTop(this);*/

	DeleteFromCache();

	prev->next=next;
	next->prev=prev;

	h->AddElemTop(this);

	return 1;
}

int CBltElem::SendToBottom()
{
	CBltWindow *w=parent;

	if(!(parent&&next&&prev)) return 0;

	CBltWndHead *h=w->child;

/*	w->RemoveChild(this);
	w->AddChildToBottom(this);*/

	DeleteFromCache();

	prev->next=next;
	next->prev=prev;

	h->AddElemBottom(this);

	return 1;
}

int CBltElem::BringToFront()
{

	CBltElem *el;
	CBltWindow *w=parent;

	if(!(parent&&next&&prev)) return 0;

	CBltWndHead *h=w->child;

	if(prev==h) return 0;

	el=prev;

	w->RemoveChild(this);
	w->AddChildToFrontOfEl(this,el);

	return 1;
}

int CBltElem::SendToBack()
{

	CBltElem *el;

	if(!(parent&&next&&prev)) return 0;

	CBltWindow *w=parent;

	el=next;

	w->RemoveChild(this);
	w->AddChildToBackOfEl(this,el);

	return 1;
}

CBltElem* CBltElem::GetFrontElem()
{
	CBltElem *e=NULL;

	if(parent&&next)
	{
		if(next->GetType()!=dxWndHead) e=next;
	}

	return e;
}

CBltElem* CBltElem::GetBackElem()
{
	CBltElem *e=NULL;

	if(parent&&prev)
	{
		if(prev->GetType()!=dxWndHead) e=prev;
	}

	return e;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltWindow
//
/////////////////////////////////////////////////////////////////////////////////////////////

// konstruktor & destruktor ------------------------------------------------------ CBltWindow

CBltWindow::CBltWindow(CMatrix2D *mat,CMatrix2D *insidemat)
{
	gamewnd=0;
	visible=1;
	InitWindow(mat,insidemat);
}

CBltWindow::CBltWindow(float sizex,float sizey,CMatrix2D *mat,CMatrix2D *insidemat)
{
	gamewnd=0;
	visible=1;
	InitWindow(mat,insidemat);
	
	sx=sizex;sy=sizey;
	cliprect = new CClipRect();

}

CBltWindow::CBltWindow(float x1,float y1, float sizex, float sizey)
{
	gamewnd=0;
	visible=1;
	CMatrix2D m;
	m.Translate(x1,y1);

	InitWindow(&m,NULL);
	
	sx=sizex;sy=sizey;
	cliprect = new CClipRect();
	
}

CBltWindow::~CBltWindow(){

	RemoveCache(primarycache);	
	
	SAFE_DELETE(child);
	SAFE_DELETE(cliprect);
	SAFE_DELETE(insidetrans);
	SAFE_DELETE(trans);
	SAFE_DELETE(globaltrans);
	SAFE_DELETE(globalwndtrans);

}


int CBltWindow::InitWindow(CMatrix2D *mat,CMatrix2D *insidemat)
{

	globaltrans=new CMatrix2D; globaltrans->Identity();
	globalwndtrans=new CMatrix2D; globalwndtrans->Identity();
	trans=NULL;insidetrans=NULL;

	if(mat)
	{
		trans=new CMatrix2D;
		*trans=*mat;
		*globalwndtrans=*mat;
	}
	if(insidemat)
	{
		insidetrans=new CMatrix2D;
		*insidetrans=*insidemat;
		*globaltrans=(*insidetrans)*(*globalwndtrans);
	}
	else
		*globaltrans=*globalwndtrans;
		


	child = new CBltWndHead(this);
	cliprect=NULL;
	SetNeedRecalc();

	primarycache=NULL;
	parentcache=NULL;

	return 1;
}

void CBltWindow::DeleteDXObjects()
{
	if(primarycache) primarycache->DeleteAllSurfaces();
	if(child) child->DeleteDXObjects();
}

// pridavani/ubirani synu -------------------------------------------------------- CBltWindow

int CBltWindow::AddChildToTop(CBltElem *el)
{

	if(el->GetParent()) return 0;

	el->SetNeedRecalc();

	child->AddElemTop(el);

	el->cached=0;

	if(el->GetType()==dxWindow) {
		if(primarycache) el->AssocCache(primarycache);
		else if(parentcache) el->AssocCache(parentcache);
	}

	return 1;
}

int CBltWindow::AddChildToBottom(CBltElem *el)
{

	if(el->GetParent()) return 0;

	el->SetNeedRecalc();
	el->cached=0;

	child->AddElemBottom(el);

	return 1;
}


int CBltWindow::RemoveChild(CBltElem *el)
{

	if(el->parent!=this) return 0;

	el->DeleteFromCache();

	el->parent=NULL;

	el->next->prev=el->prev;
	el->prev->next=el->next;
	el->next=NULL;el->prev=NULL;

	return 1;

}

int CBltWindow::DeleteChild(CBltElem *el)
{

	if(!RemoveChild(el)) return 0;

	delete el;

	return 1;

}


void CBltWindow::DeleteAllChild()
{ 
	if(child) {
		while(child->next!=child)
			DeleteChild(child->next);
	}
}

int CBltWindow::AddChildToFrontOfEl(CBltElem *newel, CBltElem *el)
{
	if(newel->parent!=NULL) return 0;
	if(!el)	el = child;
	if(el->parent!=this) return 0;

	newel->next=el->next;
	newel->prev=el;
	el->next->prev=newel;
	el->next=newel;

	newel->parent=this;

	newel->SetNeedRecalc();
	newel->cached=0;

	return 1;
}

int CBltWindow::AddChildToBackOfEl(CBltElem *newel, CBltElem *el)
{
	if(newel->parent!=NULL) return 0;
	if(!el)	el = child;
	if(el->parent!=this) return 0;

	newel->next=el;
	newel->prev=el->prev;
	el->prev->next=newel;
	el->prev=newel;

	newel->parent=this;

	newel->SetNeedRecalc();
	newel->cached=0;

	return 1;
}

CBltElem* CBltWindow::GetTopChild(void)
{
	if(!child||child->prev->GetType()==dxWndHead) return NULL;
	else
		return child->prev;
}


CBltElem* CBltWindow::GetBottomChild(void)
{
	if(!child||child->next->GetType()==dxWndHead) return NULL;
	else
		return child->next;
}


// transformace ------------------------------------------------------------------ CBltWindow

void CBltWindow::LWindowTrans(const CMatrix2D &mat)
{
	DeleteFromCache();
	if(trans)
	{
		*trans=mat*(*trans);
	}else{
		trans=new CMatrix2D;
		*trans=mat;
	}

//	if(primarycache)primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::RWindowTrans(const CMatrix2D &mat)
{
	DeleteFromCache();
	if(trans)
	{
		*trans=(*trans)*mat;
	}else{
		trans=new CMatrix2D;
		*trans=mat;
	}

//	if(primarycache) primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::SetWindowTrans(const CMatrix2D &mat)
{

	DeleteFromCache();

	if(!trans) trans=new CMatrix2D;
	*trans=mat;

//	if(primarycache) primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::GetWindowTrans(CMatrix2D &mat) const
{
	if(!trans) 
		mat.Identity();
	else
		mat=*trans;
}

void CBltWindow::LInsideTrans(const CMatrix2D &mat)
{
	if(parentcache) DeleteFromCache();

	if(insidetrans)
	{
		*insidetrans=mat*(*insidetrans);
	}else{
		insidetrans=new CMatrix2D;
		*insidetrans=mat;
	}

//	if(primarycache) primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::RInsideTrans(const CMatrix2D &mat)
{
	if(parentcache) DeleteFromCache();

	if(insidetrans)
	{
		*insidetrans=(*insidetrans)*mat;
	}else{
		insidetrans=new CMatrix2D;
		*insidetrans=mat;
	}

//	if(primarycache) primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::SetInsideTrans(const CMatrix2D &mat)
{
	if(parentcache) DeleteFromCache();

	if(!insidetrans) insidetrans=new CMatrix2D;
	*insidetrans=mat;

	//if(primarycache) primarycache->RecalcTrans();

	SetNeedRecalc();
}
void CBltWindow::GetInsideTrans(CMatrix2D &mat) const
{
	if(!insidetrans) 
		mat.Identity();
	else
		mat=*insidetrans;
}

void CBltWindow::SetVisible(int vis) //nastaveni viditelnosti
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


void CBltWindow::SetWindowSize(float _sx,float _sy)
{
	sx=_sx;sy=_sy;

//	if(primarycache) primarycache->RecalcTrans();
	
	SetNeedRecalc();
}

// rendering --------------------------------------------------------------------- CBltWindow


void CBltWindow::Render(int drawall)
{
	if(visible&&child&&(!cliprect||!cliprect->clippedout)){
				
		if(primarycache){

			child->Render(0);
			
			primarycache->Render();
			primarycache->Update();
		}else

			child->Render(drawall);
		

	
	}
}

// vypocet transformaci a clipovacich rovin -------------------------------------- CBltWindow


int CBltWindow::Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc)
{

	int i,ok=1;

	if(parentrecalc||needrecalc)
	{
		
		//vypocet globalni trans. matice
		if(parenttrans){
			if(trans)
				*globalwndtrans = (*trans) * (*parenttrans);
			else
				*globalwndtrans = *parenttrans;

		}
		else 
		{
			if(trans) *globalwndtrans = *trans; 
		}


		if(cliprect)
		{
			if(parentclip && parentclip->clippedout)
			{
				cliprect->clippedout=1;
			}else
			{
				CVec2 coners[4];
				CVec2 transconers[4];

				coners[0].x=0;coners[0].y=0;
				coners[1].x=0;coners[1].y=sy;
				coners[2].x=sx;coners[2].y=sy;
				coners[3].x=sx;coners[3].y=0;

				for(i=0;i<4;i++)
					transconers[i]=globalwndtrans->TransPoint(coners[i]);

				//ToDo: test otoceni

				cliprect->minx=cliprect->maxx=transconers[0].x;
				cliprect->miny=cliprect->maxy=transconers[0].y;

				for(i=1;i<4;i++)
				{
					if(transconers[i].x<cliprect->minx) cliprect->minx=transconers[i].x;
					if(transconers[i].x>cliprect->maxx) cliprect->maxx=transconers[i].x;
					if(transconers[i].y<cliprect->miny) cliprect->miny=transconers[i].y;
					if(transconers[i].y>cliprect->maxy) cliprect->maxy=transconers[i].y;
				}

				if(parentclip) {
					cliprect->minx=max(cliprect->minx,parentclip->minx);
					cliprect->maxx=min(cliprect->maxx,parentclip->maxx);
					cliprect->miny=max(cliprect->miny,parentclip->miny);
					cliprect->maxy=min(cliprect->maxy,parentclip->maxy);
				}

				if(cliprect->minx>cliprect->maxx ||cliprect->miny>cliprect->maxy)
					cliprect->clippedout=1;
				else
					cliprect->clippedout=0;
			}
		}

		if(insidetrans)
			*globaltrans = *insidetrans * *globalwndtrans;	
		else
			*globaltrans = *globalwndtrans;
	

		if(primarycache)
			primarycache->RecalcTrans();

	}

	if(child&&(!cliprect||!cliprect->clippedout)) 
		if(!child->Update(globaltrans,cliprect,needrecalc|parentrecalc)) ok=0;


	if(primarycache)
	{
		primarycache->CheckResize();
		primarycache->SetRedraw();
	}

	if(gamewnd)
		GEnMain->SortIt();


	needrecalc=0;


	return ok;
}

// cache ------------------------------------------------------------------------- CBltWindow

void CBltWindow::CreateCache(int gridsx,int gridsy,float minx,float miny,float maxx,float maxy)
{
	if(primarycache) return;

	primarycache=new CCache;
	
	primarycache->AsociateWin(this,minx,miny,maxx,maxy,gridsx,gridsy);

	//ToDo: vyhazet z parentcache
	parentcache=NULL;

	AssocCache(primarycache);
}


int CBltWindow::ResizeCache(float minx,float miny,float maxx,float maxy)
{
	if(!primarycache) return 0;

	return primarycache->ResizeCache(minx,miny,maxx,maxy);
}

void CBltWindow::RemoveCache(CCache *cache)
{
	if(!cache) return;

	if(child)
		child->RemoveCache(cache);

	if(cache==primarycache) {
		//ToDo: vyhazet z cache
		delete primarycache;
		primarycache=NULL;
	}
	if(cache==parentcache)
	{
		//ToDo: vyhazet z cache
		parentcache=NULL;
	}
}

int CBltWindow::AddToCache(CBltElem *el)
{
	if(primarycache)
	{
		primarycache->Add(el);
		return 1;
	}
	else if(parentcache)
	{
		parentcache->Add(el);
		return 1;
	}
	return 0;

}

void CBltWindow::DeleteFromCache()
{
	if(primarycache||parentcache)
		child->DeleteFromCache();
}


void CBltWindow::DeletePrimitiveFromCache(CBltElem *el)
{
	if(primarycache)
		primarycache->Delete(el);
	else if(parentcache)
		parentcache->Delete(el);

	el->cached=0;
}

void CBltWindow::InvalidElCache(CBltElem *el)
{
	if(primarycache)
		primarycache->InvalidEl(el);
	else if(parentcache)
		parentcache->InvalidEl(el);
}

void CBltWindow::AssocCache(CCache *c)
{
	if(c!=primarycache) 
		parentcache=c;

	if(child) {
		if(!primarycache) child->AssocCache(c);
		else child->AssocCache(primarycache);
	}
}

void CBltWindow::InvalidateAllTex()
{
	if(!child) return;

	CBltElem *el;

	el=child->next;
	while(el->GetType()!=dxWndHead)
	{
		InvalidElCache(el);
		el->SetNeedRecalc(dxrcTex);
		el=el->next;
	}
}

// veci pro herni okno ----------------------------------------------------------- CBltWindow

int CBltWindow::GEnBringToBottom(CBltElem *el)
{
	CBltWndHead *h=child;

	if(el->parent!=this) return 0;

	el->prev->next=el->next;
	el->next->prev=el->prev;

	h->AddElemBottom(el);
	return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltWndHead
//
/////////////////////////////////////////////////////////////////////////////////////////////

// konstruktor & destruktor ----------------------------------------------------- CBltWndHead

CBltWndHead::CBltWndHead(CBltWindow* win)
{
	parent=win;

	next=prev=this;

}

CBltWndHead::~CBltWndHead()
{
	CBltElem *el,*tm;
	el=next;
	while(el->GetType()!=dxWndHead)
	{
		tm=el;el=el->next;
		delete tm;
	}
}


void CBltWndHead::AddElemTop(CBltElem *el)
{
	el->next=this;
	el->prev=prev;
	prev->next=el;
	prev=el;

	el->parent=parent;
}

void CBltWndHead::AddElemBottom(CBltElem *el)
{

	el->prev=this;
	el->next=next;

	next->prev=el;
	next=el;

	el->parent=parent;
}

void CBltWndHead::DeleteDXObjects()
{
	CBltElem *el;

	el=next;
	while(el->GetType()!=dxWndHead)
	{
		el->DeleteDXObjects();
		el=el->next;
	}

}

// vypocet transformaci a clipovacich rovin ------------------------------------- CBltWndHead

int CBltWndHead::Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc)
{

	int ok=1;
	CBltElem *el;

	el=next;
	while(el->GetType()!=dxWndHead)
	{
		if(!el->Update(parenttrans,parentclip,parentrecalc)) ok=0;;
		el=el->next;
	}
	
	return ok;
}

// rendering -------------------------------------------------------------------- CBltWndHead

void CBltWndHead::Render(int drawall)
{

	CBltElem *el=next;

	int game=0;
//	if(parent->IsGameWindow()) game=1;

	while(el->GetType()!=dxWndHead)
	{
		el->Render(drawall);
		el=el->next;
		if(el->redraw==0&&game) break;
	}

}


// cache ------------------------------------------------------------------------ CBltWndHead

void CBltWndHead::RemoveCache(CCache *cache)
{
	CBltElem *el=next;
	while(el->GetType()!=dxWndHead)
	{
		el->RemoveCache(cache);
		el=el->next;
	}
}

void CBltWndHead::AssocCache(CCache *c)
{
	CBltElem *el=next;

	while(el->GetType()!=dxWndHead)
	{
		el->AssocCache(c);
		el=el->next;
	}
}

void CBltWndHead::DeleteFromCache()
{
	CBltElem *el=next;

	while(el->GetType()!=dxWndHead)
	{
		el->DeleteFromCache();
		el=el->next;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltPrimitive
//
/////////////////////////////////////////////////////////////////////////////////////////////


void CBltPrimitive::SetTrans(const CMatrix2D &mat)
{
	if(!trans) trans=new CMatrix2D;
	*trans=mat;
	SetNeedRecalc();
}


void CBltPrimitive::SetTexture(CTexture *texture)
{
	if(tex) {tex->Release();tex=NULL;}

	if(texture)
	{
		tex=texture;
		tex->AddRef();

	}

	if(parent) parent->InvalidElCache(this);

}


void CBltPrimitive::DeleteFromCache()
{
	if(!parent) return;

	parent->DeletePrimitiveFromCache(this);

}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltRect
//
/////////////////////////////////////////////////////////////////////////////////////////////


// konstruktor & destruktor -------------------------------------------------------- CBltRect



// vypocet transformaci a clipovacich rovin ---------------------------------------- CBltRect

int CBltRect::Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc)
{
	if(!visible) return 1;

	if(parentrecalc) needrecalc=dxrcAll;

	if(needrecalc)
	{
		int i;
		int poschng=0;
		const CMatrix2D *m=NULL;
		CMatrix2D mat;

		if( (needrecalc&dxrcPos)  ) //|| ( (needrecalc&dxrcTex) && (tchng==1 || (tchng==2 && nottex) ) ) )
		{
			poschng=1;

			CVec2 vert[8];
			int vvlen=numvert;

			if(parent)
			{			
				parent->GetGlobalTransPtr(&m);
			}

			numvert=4;

			vert[0].x=vert[1].x=x1;
			vert[3].x=vert[2].x=x2;

			vert[0].y=vert[3].y=y2;
			vert[2].y=vert[1].y=y1;
			
			if(m||trans){

				if(!m) m=trans; else
				if(trans) { mat = (*trans) * (*m); m=&mat; }


				for(i=0;i<4;i++)
				{
					vert[i]=m->TransPoint(vert[i]);
				}
			}

			if(parentclip){
				i=parentclip->Clip(vert,numvert);
				numvert=i;
				if(numvert<3) {
					SAFE_DELETE_ARRAY(vertices);

					needrecalc = 0;
					if(IsCmpTex())
						SAFE_RELEASE(tex);
					return 1;
				}
			}

			if(!vertices||numvert!=vvlen)
			{
				SAFE_DELETE_ARRAY(vertices);
				vertices=new CBltVert[numvert];
			}

			for(i=0;i<numvert;i++)
			{
				vertices[i].x=vert[i].x;
				vertices[i].y=vert[i].y;
				vertices[i].z=0;
				vertices[i].rhw=1;
				vertices[i].col=col;
			}
		}

		if((needrecalc & dxrcTex) )
		{
			if(numvert<3) {
					needrecalc = 0;
					return 1;
			}

			int nottex = (tex==NULL);
			int tchng = CmpTex();


			if( tex && (poschng || tchng==1 || (tchng==2 && nottex) ))
			{

				CVec2 P,X,u,v;
				float su,sv;
				UINT tsx,tsy,isx,isy;
				float fsx,fsy;
				
				tex->GetImageSize(&isx,&isy);

				tex->GetTextureSize(&tsx,&tsy);
				fsx=.5f/tsx;fsy=.5f/tsy;

				P.x=x1;P.y=y1; 
				u.x=(x2-x1); u.y=0;
				v.x=0;     v.y=(y2-y1);

				float ttx,tty;

				ttx=(isx-1)/(x2-x1-1)*(x2-x1)/tsx;
				tty=(isy-1)/(y2-y1-1)*(y2-y1)/tsy;


				if(!m)
				{
					if(parent) parent->GetGlobalTransPtr(&m);
					if(m||trans){
						if(!m) m=trans; else
						if(trans) { mat = (*trans) * (*m); m=&mat; }
					}
				}

				if(m)
				{
					P=m->TransPoint(P);
					u=m->TransVector(u);v=m->TransVector(v);
				}

				su=u.x*u.x+u.y*u.y;
				sv=v.x*v.x+v.y*v.y;

				ttx=ttx/su;tty=tty/sv;

				for(i=0;i<numvert;i++)
				{
					X.x=vertices[i].x-P.x;
					X.y=vertices[i].y-P.y;
					vertices[i].u=(X*u) * ttx+ fsx;
					vertices[i].v=(X*v) * tty+ fsy;
				}
			}
		}

		needrecalc = 0;
	}

	if(!cached)
	{
		if(parent->AddToCache(this)) cached=1;
	}


	return 1;
}

// rendering ----------------------------------------------------------------------- CBltRect

void CBltRect::Render(int drawall)
{
	if(!drawall&&!redraw) return;

	redraw=0;

	if(!visible) return;
	if(numvert<3) return;

	LPDIRECT3DDEVICE8 d3dDev=DXbliter->GetD3DDevice();

	DXbliter->numdr++;

	if(tex)
	{
		LPDIRECT3DTEXTURE8 t=tex->GetTexture();
		d3dDev->SetTexture(0,t);
	} else d3dDev->SetTexture(0,NULL);	

	d3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, numvert-2, vertices,sizeof(CBltVert));


}



int CBltRect::GetBBox(CBBox &bbox)
{
	const CMatrix2D *pm;
	
	bbox.minx=x1;bbox.maxx=x2;
	bbox.miny=y1;bbox.maxy=y2;
	
	parent->GetGlobalTransPtr(&pm);

	if(trans)
	{
		CMatrix2D m;
		m = (*trans) * (*pm);
		bbox.TransformBBox(&m);
	}else
	{
		bbox.TransformBBox(pm);
	}

	bbox.valid=1;
	bbox.minx=(float)floor(bbox.minx);
	bbox.miny=(float)floor(bbox.miny);
	bbox.maxx=(float)ceil(bbox.maxx);
	bbox.maxy=(float)ceil(bbox.maxy);
	
	return 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltText
//
/////////////////////////////////////////////////////////////////////////////////////////////


int CBltText::ChkSize(float &sx, float &sy)
{
	int k=1;

	if(sx>(int)g_pDXapp->m_d3dCaps.MaxTextureWidth) sx=(float)g_pDXapp->m_d3dCaps.MaxTextureWidth,k=0;
	if(sy>(int)g_pDXapp->m_d3dCaps.MaxTextureHeight) sy=(float)g_pDXapp->m_d3dCaps.MaxTextureHeight,k=0;

	return k;
}

int CBltText::CmpTex()
{
	if(tex) return 0; //textura uz je spocitana

	if(font==NULL) return 0;

	//create texture
	if(!tex) tex=new CTexture;

	if(!tex->Create(sx,sy,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED))
	{
		SAFE_RELEASE(tex);
		return 0;
	}
	
	// Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  sx;
    bmi.bmiHeader.biHeight      = -sy;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    // Create a DC and a bitmap for the font
	HDC hDC  = DXbliter->dc;

	if(!hDC)
	{
		SAFE_RELEASE(tex);
		return 0;
	}

	HBITMAP hBmp = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );	
	if(!hBmp)
	{
		SAFE_RELEASE(tex);
		return 0;
	}

	HANDLE oldbmp = SelectObject( hDC, hBmp );
    HANDLE oldfont= SelectObject( hDC, font );

    // Set text properties
	SetMapMode( hDC, MM_TEXT );
	SetTextColor( hDC, RGB(0xFF,0xFF,0xFF) );
	SetBkColor(   hDC, 0 );
    SetTextAlign( hDC, TA_TOP );

	//render

	RECT r;
	r.left=(LONG)0;r.top=(LONG)0;r.right=(LONG)sx;r.bottom=(LONG)sy;
	int aa=DrawTextEx(hDC,string,sz,&r,TXfmt,NULL);

	//lock texture

	DWORD *bits;
	int pp;

	int err=0;

	if(tex->Lock((void**)&bits,pp))
	{
		pp=(pp>>2)-sx;
		
		int bAlpha;
		int oo=0;

		for( int y=0; y < sy; y++ )
		{
			for( int x=0; x < sx; x++ )
			{
				bAlpha = (BYTE)((pBitmapBits[oo] & 0xff));
				if (bAlpha)
				{
					*bits++ = (bAlpha << 24) | 0x00FFFFFF;
				}
				else
				{
					*bits++ = 0;
				}
				oo++;
			}
			bits+=pp;
		}


		//unlock & delete
		tex->Unlock();
	}else{
		SAFE_RELEASE(tex)
		err=1;
	}

	SelectObject(hDC,oldbmp);
	SelectObject(hDC,oldfont);

	DeleteObject( hBmp );

	if(err) return 0;

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CBltLine
//
/////////////////////////////////////////////////////////////////////////////////////////////


int CBltLine::Update(const CMatrix2D *parenttrans, const CClipRect *parentclip, int parentrecalc)
{
	if(!visible) return 1;

	if(parentrecalc) needrecalc=dxrcAll;

	if(needrecalc)
	{
		int i;

		const CMatrix2D *m=NULL;

		needrecalc=0;

		CVec2 vert[2];

		if(parent)
		{			
			parent->GetGlobalTransPtr(&m);
		}

		clipstate = 2;

		if(m){
			for(i=0;i<2;i++)
				vert[i]=m->TransPoint(points[i]);
		}else{
			for(i=0;i<2;i++)
				vert[i]=points[i];
		}

		if(parentclip){
			clipstate = parentclip->ClipLine(vert);
			if(clipstate == 0) return 1;			
		}

		for(i=0;i<2;i++)
		{
			vertices[i].x=vert[i].x;//-.5f;
			vertices[i].y=vert[i].y;//-.5f;
			vertices[i].z=0;
			vertices[i].rhw=1;
			vertices[i].col=col;
		}

	}

	if(!cached)
	{
		if(parent->AddToCache(this)) cached=1;
	}


	return 1;
}


void CBltLine::Render(int drawall)
{
	if(!drawall&&!redraw) return;

	redraw=0;

	if(!visible) return;
	if(clipstate==0) return;

	LPDIRECT3DDEVICE8 d3dDev=DXbliter->GetD3DDevice();

	DXbliter->numdr++;

	d3dDev->SetTexture(0,NULL);	

	d3dDev->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices,sizeof(CBltVert));
	if(clipstate==1||drawlastpoint)
		d3dDev->DrawPrimitiveUP(D3DPT_POINTLIST, 1, &vertices[1],sizeof(CBltVert));

}


void CBltLine::DeleteFromCache()
{
	if(!parent) return;

	parent->DeletePrimitiveFromCache(this);

}

int CBltLine::GetBBox(CBBox &bbox)
{
	const CMatrix2D *pm;
	
	if(points[0].x<=points[1].x)
	{
		bbox.minx=points[0].x;
		bbox.maxx=points[1].x;
	}else{
		bbox.minx=points[1].x;
		bbox.maxx=points[0].x;
	}

	if(points[0].y<=points[1].y)
	{
		bbox.miny=points[0].y;
		bbox.maxy=points[1].y;
	}else{
		bbox.miny=points[1].y;
		bbox.maxy=points[0].y;
	}

	parent->GetGlobalTransPtr(&pm);

	bbox.TransformBBox(pm);

	bbox.valid=1;
	bbox.minx=(float)floor(bbox.minx);
	bbox.miny=(float)floor(bbox.miny);
	bbox.maxx=(float)ceil(bbox.maxx)+1;
	bbox.maxy=(float)ceil(bbox.maxy)+1;
	
	return 1;

}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// CClipRect
//
/////////////////////////////////////////////////////////////////////////////////////////////


int CClipRect::ClipLine(CVec2 *v) const
{

	int c[2],a,b;
	int clpend = 2;

#define TEST_C(i) 	c[i]=0; \
					if(v[i].y<miny) c[i] |= 1; else if(v[i].y>=maxy) c[i] |= 2; \
					if(v[i].x<minx) c[i] |= 4; else if(v[i].x>=maxx) c[i] |= 8;

	TEST_C(0);
	TEST_C(1);

	while(1)
	{

		if( (c[0]|c[1]) == 0 ) return clpend;
		if( (c[0]&c[1]) != 0 ) return 0;


		if(c[0]) a=0,b=1; else a=1,b=0;

		if(a==1) clpend=1;

		if(c[a]&1){
			v[a].x+=( v[b].x-v[a].x ) / ( v[b].y-v[a].y ) * (miny - v[a].y);
			v[a].y=miny;			
			TEST_C(a);
		}else
		if(c[a]&2){
			v[a].x+=( v[b].x-v[a].x ) / ( v[b].y-v[a].y ) * (maxy - v[a].y);
			v[a].y=maxy-1;
			TEST_C(a);
		}else
		if(c[a]&4){
			v[a].y+=( v[b].y-v[a].y ) / ( v[b].x-v[a].x ) * (minx-1 - v[a].x);
			v[a].x=minx;
			TEST_C(a);
		}else
		if(c[a]&8){
			v[a].y+=( v[b].y-v[a].y ) / ( v[b].x-v[a].x ) * (maxx-1 - v[a].x);
			v[a].x=maxx-1;
			TEST_C(a);
		}

	}

}


int CClipRect::Clip(CVec2 *vert, int numvert) const
{
	
	CVec2 v[8];

	if(numvert+4>8) return 0;

	numvert=Clip1Side(vert,v,numvert,0);
	numvert=Clip1Side(v,vert,numvert,1);
	numvert=Clip1Side(vert,v,numvert,2);
	numvert=Clip1Side(v,vert,numvert,3);

	return numvert;

}


int CClipRect::Clip1Side(CVec2 *in, CVec2 *out, int numvert, int sidenum) const
{
	
	CVec2 P;
	int ov=0,i;

	const CVec2 *L,*A;

	L=&in[numvert-1];
	A=&in[0];


	switch(sidenum)
	{
	case 0: //leva rovina
		for(i=0;i<numvert;i++)
		{
			if( L->x < minx ) //L je venku
			{
				if(A->x >= minx){ //A je vevnitr

					P.x = minx;
					P.y = A->y + (minx - A->x) * (L->y-A->y)/(L->x-A->x);

					if(P!=*A) out[ov++]=P;
					out[ov++]=*A;

				} //else nic

			}else{ //L je vevnitr

				if(A->x < minx){ //A je venku

					P.x = minx;
					P.y = A->y + (minx - A->x) * (L->y-A->y)/(L->x-A->x);

					out[ov++]=P;

				}else{ //A je vevnitr

					out[ov++]=*A;

				}

			}
		
			L=A;A++;	
		}
		break;


	case 1: //prava rovina
		for(i=0;i<numvert;i++)
		{
			if( L->x > maxx ) //L je venku
			{
				if(A->x <= maxx){ //A je vevnitr

					P.x = maxx;
					P.y = A->y + (maxx-A->x) * (L->y-A->y)/(L->x-A->x);

					if(P!=*A) out[ov++]=P;
					out[ov++]=*A;

				} //else nic

			}else{ //L je vevnitr

				if(A->x > maxx){ //A je venku

					P.x = maxx;
					P.y = A->y + (maxx-A->x) * (L->y-A->y)/(L->x-A->x);

					out[ov++]=P;

				}else{ //A je vevnitr

					out[ov++]=*A;

				}

			}
		
			L=A;A++;
		}
		break;

	case 2: //horni rovina
		for(i=0;i<numvert;i++)
		{
			if( L->y < miny ) //L je venku
			{
				if(A->y >= miny){ //A je vevnitr

					P.y = miny;
					P.x = A->x + (miny - A->y) * (L->x-A->x)/(L->y-A->y);

					if(P!=*A) out[ov++]=P;
					out[ov++]=*A;

				} //else nic

			}else{ //L je vevnitr

				if(A->y < miny){ //A je venku

					P.y = miny;
					P.x = A->x + (miny - A->y) * (L->x-A->x)/(L->y-A->y);

					out[ov++]=P;

				}else{ //A je vevnitr

					out[ov++]=*A;

				}

			}
		
			L=A;A++;	
		}
		break;


	case 3: //dolni rovina
		for(i=0;i<numvert;i++)
		{
			if( L->y > maxy ) //L je venku
			{
				if(A->y <= maxy){ //A je vevnitr

					P.y = maxy;
					P.x = A->x + (maxy-A->y) * (L->x-A->x)/(L->y-A->y);

					if(P!=*A) out[ov++]=P;
					out[ov++]=*A;

				} //else nic

			}else{ //L je vevnitr

				if(A->y > maxy){ //A je venku

					P.y = maxy;
					P.x = A->x + (maxy-A->y) * (L->x-A->x)/(L->y-A->y);

					out[ov++]=P;

				}else{ //A je vevnitr

					out[ov++]=*A;

				}

			}
		
			L=A;A++;
		}
		break;


	}

	return ov;

}

