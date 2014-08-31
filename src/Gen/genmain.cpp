/////////////////////////////////////////////////////////////////////////////////
///
///		G A M E   G R A P H I C   E N G I N E   -   G E n
///
///		Main Objeckt, viditelnostni trideni, seznamy obrazku, svetla
///		A: Honza M.D. Krcek (viditelnostni trideni), Petr Altman (svetla)
///
/////////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "genmain.h"

#include "lightcache.h"
#include "normalizer.h"
#include "cache.h"

#include "dx.h"
#include "fs.h"

#include "genelement.h"
#include "genanim.h"
#include "genpackage.h"
#include "musicmod.h"
#include "music.h"

#include "kernel.h"
#include "autos.h"

#define CREATE_TEX 0

#define LIGHTCACHE_SIZE 1987

CGEnMain *GEnMain=0;

int numsortcmp=0;

//////////////////////////////////////////////////////////////////////////////////////////////////

///  Pomocne fce
CVec2 RotateVL(const CVec2 &v) // rotace o 90 stupnu doleva
{
	return CVec2(v.y,-v.x);
}
CVec2 RotateVR(const CVec2 &v) // rotace o 90 stupnu doprava
{
	return CVec2(-v.y,v.x);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
///		C G E n   M a i n
///		Hlavni objekt Herniho Graficke Enginu
///
//////////////////////////////////////////////////////////////////////////////////////////////////

// Konstruktor
CGEnMain::CGEnMain(CBltWindow *_GameWindow) {

	GameWindow=_GameWindow;

	scrollX = scrollY = 0;
	scrollXnow = scrollYnow = 0;
	scroll = 0;

	lightcache=new CLightCache(LIGHTCACHE_SIZE);

	ztransform.x = -0.35f;
	ztransform.y = -0.35f;
	GEnMain = this;
	
	SElements=0;
	SElemsStack = 0;
	GGraphStack = 0;

	toplight=NULL;

	toplight = new CTopLight(-.7f,.7f,-1.0f,.7f,.7f,.7f);
	SetTopLightDefault();
	lightcache->SetTopLight(toplight);


	zshift = new CLightZShift(ztransform.x, ztransform.y,toplight->dx/127.f,toplight->dy/127.f);

	lastupdatetime = 0;
	SetKernelTime(0);

	GEnMain = this;	

//	testGEn = 0;
/*
#if CREATE_TEX==1
	CreateTexFiles();
#endif

	CreateTest();
*/
}

// destruktor
CGEnMain::~CGEnMain (){

	CGEnSortElement *s,*st;
	s=SElemsStack;
	while(s)
	{
		st=s;
		s=s->next;
		delete st;
	}
	SElemsStack=0;
	CGEnGraph *g,*gt;
	g=GGraphStack;
	while(g)
	{
		gt=g;
		g=g->next;
		delete gt;
	}
	GGraphStack=0;

	DeleteAllCursors();

	if(GameWindow && !GameWindow->GetParent())
		SAFE_DELETE(GameWindow);

	SAFE_DELETE(toplight);

	DeleteListElems(lights);
	DeleteListElems(MovingElems);

	SAFE_DELETE(lightcache);
	SAFE_DELETE(zshift);
	
	GEnMain = NULL;

}

//----------------------------------------------------------------------------------------------
// vytvareni elementu

//vytvori novy element
CGEnElement* CGEnMain::DrawElement(float x, float y, float z, CLightTex *tex) 
{
	if(!tex||!tex->ForceLoad()||!tex->GetElSource()) return 0; //musi se zadat textura (a ta musi mit elsource)

	CGEnElement *ge;
	ge = new CGEnElement(x,y,z,tex); //vytvorim element
	GameWindow->AddChildToBottom(ge); //pridam ho do okna

	lightcache->AddElem(ge); //pridam ho do svetelny cache

	return ge;
}

CGEnElement* CGEnMain::DrawElement(float x, float y, float z, CGEnAnim *anim, int frame_in_starttime, UI starttime) 
{
	if(!anim) return 0;

	CLightTex *tex = anim->GetFrameTex(0);
	if(!tex) return 0;

	CGEnElement *el = DrawElement(x,y,z,tex);
	if(el){
		SetAnim(el,anim,frame_in_starttime,starttime);
	}
	return el;
}

CGEnElement* CGEnMain::DrawElement(OPointer objptr, float x, float y, float z, CGEnResource *res, int frame_in_starttime, UI starttime) 
{
	CGEnElement *el=NULL;
	if(!res) return NULL;
	
	eResourceType type = res->GetType();

	if(type==resAnim)
	{
		el = DrawElement(x,y,z,(CGEnAnim*)res,frame_in_starttime,starttime);
		if(el){
			el->ObjPtr = objptr;
		}
	}else
	if(type==resLightTex)
	{
		el = DrawElement(x,y,z,(CLightTex*)res);
		if(el){
			el->ObjPtr = objptr;
		}
	}
	return el;
}

//vytvori novou caru
CGEnLine* CGEnMain::DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, DWORD col, int TopFlag)
{

	CGEnLine *gl;
	gl = new CGEnLine(x1,y1,z1,x2,y2,z2,col,TopFlag); //vytvorim element
	GameWindow->AddChildToBottom(gl); //pridam ho do okna

	return gl;
}

int CGEnMain::DrawLine(CGEnLine *ln)
{
	GameWindow->AddChildToBottom(ln); //pridam caru do okna
	return 1;
}

//kurzor
CGEnCursor* CGEnMain::CreateCursor(int type, float _sx, float _sy, float _sz, DWORD color, DWORD color2)
{
	CGEnCursor *cur=new CGEnCursor;
	if(cur->CreateCursor(type,_sx,_sy,_sz,color,color2))
	{
		cursors.Add(cur);
		return cur;
	}else
	{
		delete cur;
		return NULL;
	}
}

int CGEnMain::DrawCursor(CGEnCursor *cursor, float x, float y, float z)
{
	return cursor->AddToGEn(x,y,z);
}

//----------------------------------------------------------------------------------------------
// ruseni elementu

//zrusi element
int CGEnMain::DeleteElement(CGEnElement *el)
{
	if(!el) return 0;

	if(el->parent)
	{
		el->parent->DeleteChild(el); //vyhodim z okna a smazu ho
	}else{
		delete el; //neni v okne - smazu ho
	}

	return 1;

}

//zrusi caru
int CGEnMain::DeleteLine(CGEnLine *el)
{
	if(!el) return 0;

	if(el->parent)
	{
		el->parent->DeleteChild(el); //vyhodim z okna a smazu ho
	}else{
		delete el; //neni v okne - smazu ho
	}

	return 1;
}

int CGEnMain::RemoveLine(CGEnLine *el)
{
	if(!el) return 0;

	if(el->parent)
		el->parent->RemoveChild(el); //vyhodim z okna

	return 1;
}

//zrusi cursor
int CGEnMain::DeleteCursor(CGEnCursor *cur)
{
	if(!cur) return 0;
	
	cursors.Delete(cur);
	delete cur;

	return 1;
}


//smaze vsechny elementy a cary + kurzory
void CGEnMain::DeleteAllElements()
{
	DeleteAllCursors();

	if(!GameWindow) return;

	GameWindow->DeleteAllChild();

}

//zrusi vsechny svetla
void CGEnMain::DeleteAllLights()
{
	CPLightListIter lit(lights);
	while(!lit.End()){
		lightcache->DeleteLight(lit);
		lit++;
	}
	DeleteListElems(lights);
}

//zrusi vsechny kurzory
void CGEnMain::DeleteAllCursors()
{
	DeleteListElems(cursors);
}

//smaze vsechny elementy,svetla
void CGEnMain::DeleteAll()
{
//	testGEn=0;

	DeleteAllElements(); //smazu elementy+kurzory
	DeleteAllLights(); //smazu svetla
}



//----------------------------------------------------------------------------------------------
// viditelnostni trideni

// Porovnani Dvou Objektu podle viditelnosti
int CGEnMain::CompareTwo(CGEnElement *el1,CGEnElement *el2) {
	int v=0;
	int f;
	float b;
	CVec2 lp,rp;
	CVec2 a;
	CVec2 r1; float r1c;
	CVec2 r2; float r2c, r2d;
	// vraci 0 kdyz neni kolize
	// 1 el1 je pod el2 (el1 je zakryt)
	// 2 el2 je pod el1 (el2 je zakryt)
	// 3 nejednoznacne
	
	float zdz1,zdz2,pz1,pz2;

	numsortcmp++;

	// test zda se obrazky protinaji
	if (el1->x2-EPSILON<el2->x1||el2->x2-EPSILON<el1->x1||
		el1->y2-EPSILON<el2->y1||el2->y2-EPSILON<el1->y1) return 0;
	// porovnani vysky

	zdz1=el1->ElSource()->zdz;
	zdz2=el2->ElSource()->zdz;
	pz1=el1->ElSource()->pbz;
	pz2=el2->ElSource()->pbz;

	if (el1->zz+zdz1-EPSILON<el2->zz/*+pz2*/) v=1;
	if (el2->zz+zdz2-EPSILON<el1->zz/*+pz1*/) {
		if (v==0) v=2; else v=3;
	}
	if (v==1||v==2) return v;
	// kdyz je nastaven DasTepichFlag - prorovnam cisla - element s vetsim cislem je nad elementem s mensim cislem
	if (el1->DasTepichFlag && el2->DasTepichFlag){
		if(el1->DasTepichFlag<el2->DasTepichFlag) return 1;
		if(el1->DasTepichFlag>el2->DasTepichFlag) return 2;
	}
	// kdyz je nastaven DerFlug, porovnani podle pointru (nemenici se cislo pro dany objekt) 
	if (el1->ElSource()->DerFlugFlag&&el1->ElSource()->DerFlugFlag==el2->ElSource()->DerFlugFlag) {
		if (el1<el2) return 1; else return 2;
	}
	v=0;
	// porovnani proti pasu u el2
	rp=el2->ElSource()->BasePoints[el2->ElSource()->RPoint]; 
	lp=el2->ElSource()->BasePoints[el2->ElSource()->LPoint];
	r1 = RotateVL(rp - lp);
	r1c = -(r1*lp);
	r2 = RotateVR(GEnMain->GetZTransform());
	r2c = -(r2*lp);
	r2d = r2*rp + r2c;
	// kdyz tam padne nejaky bod, el1 bude zakryt -> 1
	f=0;
	while (v==0 && f<el1->ElSource()->BasePointsCount) {
		a = el1->ElSource()->BasePoints[f];
		a.x = a.x+el1->zx-el2->zx;
		a.y = a.y+el1->zy-el2->zy;
		if (r1*a + r1c > EPSILON) {
			b = r2*a + r2c;
			if (b > EPSILON && b < r2d - EPSILON) v=1;
		}
		f++;
	}
	// porovnani proti pasu u el1
	rp=el1->ElSource()->BasePoints[el1->ElSource()->RPoint]; 
	lp=el1->ElSource()->BasePoints[el1->ElSource()->LPoint];
	r1 = RotateVL(rp - lp);
	r1c = -(r1*lp);
	r2 = RotateVR(GEnMain->GetZTransform());
	r2c = -(r2*lp);
	r2d = r2*rp + r2c;
	// kdyz tam padne nejaky bod, el2 bude zakryt -> 2
	f=0;
	while (v<=1 && f<el2->ElSource()->BasePointsCount) {
		a = el2->ElSource()->BasePoints[f];
		a.x = a.x+el2->zx-el1->zx;
		a.y = a.y+el2->zy-el1->zy;
		if (r1*a + r1c > EPSILON) {
			b = r2*a + r2c;
			if (b > EPSILON && b < r2d - EPSILON) if (v==1) v=3; else v=2;
		}
		f++;
	}
	if (v==3&&el1->ElSource()->DerRampaFlag&&!el2->ElSource()->DerRampaFlag) v=1;
	else if (v==3&&el2->ElSource()->DerRampaFlag&&!el1->ElSource()->DerRampaFlag) v=2;


	if(v==3)
	{
		v=0;
		if (el1->zz+zdz1-EPSILON<el2->zz+pz2) v=1;
		if (el2->zz+zdz2-EPSILON<el1->zz+pz1) {
			if (v==0) v=2; else v=3;
		}
		
		if(v==0||v==3)
		{
			if (el1->zz+zdz1-EPSILON<el2->zz+zdz2) v=1; else
			if (el2->zz+zdz2-EPSILON<el1->zz+zdz2) v=2;							
		}
		
	}


	return v; // muze vratit 0,1,2,3
}


int CGEnMain::CompareTwo(CGEnElement *el1,CGEnLine *line2) {
	int v=0;
	float b;
	CVec2 lp,rp;
	CVec2 a;
	CVec2 r1; float r1c;
	CVec2 r2; float r2c, r2d;
	// vraci 0 kdyz neni kolize
	// 1 el1 je pod el2 (el1 je zakryt)
	// 2 el2 je pod el1 (el2 je zakryt)
	// 3 nejednoznacne

	numsortcmp++;

	float lx1,lx2,ly1,ly2;
	if(line2->points[0].x<line2->points[1].x) 
		{lx1=line2->points[0].x;lx2=line2->points[1].x;}
	else 
		{lx1=line2->points[1].x;lx2=line2->points[0].x;}

	if(line2->points[0].y<line2->points[1].y) 
		{ly1=line2->points[0].y;ly2=line2->points[1].y;}
	else 
		{ly1=line2->points[1].y;ly2=line2->points[0].y;}
	
	// test zda se obrazky protinaji
/*	if (el1->x2<lx1||lx2<el1->x1||
		el1->y2<ly1||ly2<el1->y1) return 0;
*/
	if ( (lx1>=el1->x2&&lx2>el1->x2) ||
		 (lx2<=el1->x1&&lx1<el1->x1) ||
		 (ly1>=el1->y2&&ly2>el1->y2) ||
		 (ly2<=el1->y1&&ly1<el1->y1) 
		) return 0;


	if(line2->TopFlag) return 1;

	// porovnani vysky
	if (el1->zz+el1->ElSource()->zdz-EPSILON<line2->zz1) v=1;
	if (line2->zz2-EPSILON<el1->zz) {
		if (v==0) v=2; else 
		if (line2->zz1!=line2->zz2) v=3;
	}
	if (v==1||v==2) return v;

	v=0;
	// porovnani proti pasu u el1
	rp=el1->ElSource()->BasePoints[el1->ElSource()->RPoint]; 
	lp=el1->ElSource()->BasePoints[el1->ElSource()->LPoint];
	r1 = RotateVL(rp - lp);
	r1c = -(r1*lp);
	r2 = RotateVR(GEnMain->GetZTransform());
	r2c = -(r2*lp);
	r2d = r2*rp + r2c;
	// kdyz tam padne nejaky bod, line2 bude zakryt -> 2
	a.x = line2->zx1-el1->zx;
	a.y = line2->zy1-el1->zy;
	if (r1*a + r1c > EPSILON) {
		b = r2*a + r2c;
		if (b > EPSILON && b < r2d - EPSILON) v=2;
	}
	if(line2->zx2!=line2->zx1||line2->zy2!=line2->zy1)
	{
		a.x = line2->zx2-el1->zx;
		a.y = line2->zy2-el1->zy;
		if (r1*a + r1c > EPSILON) {
			b = r2*a + r2c;
			if (b > EPSILON && b < r2d - EPSILON) v=2;
		}
	}

	if(v==0) v=1;	

	return v; // muze vratit 0,1,2,3
}

//  Trideni na grafu - Pomoci DFS
void CGEnMain::dfs(CGEnSortElement *el) {
	CGEnGraph *gr;
	
	gr=el->gr;
	el->obarveni=1; // bily->sedy
	while(gr) {
		if (gr->el->obarveni==0) dfs(gr->el);
		gr=gr->next;
	}
	el->obarveni=2; // sedy->cerny
	GameWindow->GEnBringToBottom(el->bltEl);
}


CGEnSortElement* CGEnMain::GetNewSortElem(CBltElem *bltelem)
{
	CGEnSortElement *s;
	if(!SElemsStack)
	{
		s=new CGEnSortElement();
	}else{
		s=SElemsStack;
		SElemsStack=s->next;		
	}
	s->obarveni=0;
	s->gr=0;
	s->bltEl=bltelem;
	
	s->next=SElements;
	SElements=s;

	return s;
}

CGEnGraph* CGEnMain::GetNewGGraph()
{
	CGEnGraph *g;
	if(!GGraphStack)
	{
		g=new CGEnGraph();
	}else{
		g=GGraphStack;
		GGraphStack=g->next;
	}
	return g;
}

void CGEnMain::BuiltSortGraph(CListK<CBltElem*> *elemlist)
{
//	return;
	CGEnElement *gel1,*gel2;
	CGEnLine *gln1,*gln2;

	CListKElem<CBltElem*> *be1,*be2;
	CGEnGraph *gr;

	CGEnSortElement *s1,*s2;

	int sortcmp;

	int hv;

	dxBltType t1,t2;

	be1=elemlist->el;
	while (be1) {
		
		t1 = be1->data->GetType();
		
		if(t1==dxGEnRect)
		{
			gel1 = (CGEnElement*)(be1->data);
			if(!gel1->sortel)
				gel1->sortel=GetNewSortElem(gel1);
			s1=gel1->sortel;
		}
		else
		{
			gln1 = (CGEnLine*)(be1->data);
			if(!gln1->sortel)
				gln1->sortel=GetNewSortElem(gln1);
			s1=gln1->sortel;
		}


		be2=be1->next;
		while (be2){
			t2 = be2->data->GetType();

			hv = sortResultHT.CmpHash(be1->data,be2->data);

			if(!sortResultHT.FindResult(hv,be1->data,be2->data)) //nenalezeno v HT
			{

				if(t2==dxGEnRect)
				{
					gel2 = (CGEnElement*)(be2->data);
					if(!gel2->sortel)
						gel2->sortel=GetNewSortElem(gel2);
					s2=gel2->sortel;

					if(t1==dxGEnRect)
					{
						//1 el, 2 el
						sortcmp = CompareTwo(gel1,gel2);
					}else{
						//1 ln, 2 el
						sortcmp = CompareTwo(gel2,gln1);

						if(sortcmp==1) sortcmp=2; else
						if(sortcmp==2) sortcmp=1;
					}
				}
				else
				{
					gln2 = (CGEnLine*)(be2->data);
					if(!gln2->sortel)
						gln2->sortel=GetNewSortElem(gln2);
					s2=gln2->sortel;

					if(t1==dxGEnRect)
					{
						//1 el, 2 ln
						sortcmp = CompareTwo(gel1,gln2);
					}else{
						//1 ln, 2 ln
						sortcmp = 0;
					}
				}

				sortResultHT.AddResult(hv,be1->data,be2->data);

				switch(sortcmp) {
					case 1:  //el1 je zakryt
						gr = GetNewGGraph();
						gr->el = s2;
						gr->next=s1->gr;
						s1->gr = gr;
						break;
					case 2: //el2 je zakryt
					case 3: //prekryvaji se oba (neda se setridit) el2 byl nekdy predtim zakryt
						gr = GetNewGGraph();
						gr->el = s1;
						gr->next=s2->gr;
						s2->gr = gr;
						break;			
					// sipky vedou od tech dole (zakrytejch) k tem nahore
				}
			}
			be2=be2->next;
		}
		be1=be1->next;
	}


}


//  Hlavni tridici fce. Vola se pri vypadku cache, pote, co je vytvoren
//  spojak (SElements,next2) objektu nutnych k prekresleni
//  Tyto objekty budou viditelnostne setrideny
void CGEnMain::SortIt() {

	CGEnSortElement *el1, *el2;
	CGEnGraph *gr;


	// SORT IT
	el1=SElements;
	while (el1) {
		if (el1->obarveni==0) dfs(el1);
		el1=el1->next;
	}

	// free graph
	el1=SElements;
	el2=0;
	while (el1) {
		gr=el1->gr;
		if(gr)
		{
			while (gr->next) 
				gr=gr->next;
			
			gr->next=GGraphStack;
			GGraphStack=el1->gr;
			el1->gr=0;
		}
		el2=el1;
		el1=el1->next;
		if(el2->bltEl->GetType()==dxGEnRect)
			((CGEnElement*) el2->bltEl)->sortel=0;
		else
			((CGEnLine*) el2->bltEl)->sortel=0;
	}

	if(el2)
	{
		el2->next=SElemsStack;
		SElemsStack=SElements;
	}
	SElements=0;

	sortResultHT.EmptyTable();

	DebugMessage(14,0xFF0000FF,"sort: %i",numsortcmp);
	numsortcmp=0;

}



//----------------------------------------------------------------------------------------------
// scrolling

void CGEnMain::SetScroll(float x, float y) //nascroluje na zadanou pozici (hned)
{
	scroll = 0; //zrusi plynuly scroling

	scrollXnow = scrollX = x;
	scrollYnow = scrollY = y;
	
	CMatrix2D m;

	m.Translate(-x+scrollshiftX,-y+scrollshiftY);

	GameWindow->SetInsideTrans(m);

}

void CGEnMain::GetScroll(float &x, float &y) //vrati posunuti okna
{
	x=scrollXnow;
	y=scrollYnow;
}


//nastavi plynuly scroling
void CGEnMain::Scroll(float startx, float starty, float destx, float desty, UI time)
{

	SetScroll(startx,starty);

	scrollStartTime = kernelTime;
	scrollTime = time;

	scrollStartX = startx; scrollStartY = starty;
	scrollX = destx; scrollY = desty;
	scrollXnow = startx; scrollYnow = starty;

	scrollDX = (destx-startx)/time; scrollDY = (desty-starty)/time;

	scroll = 1; //nastavi scroling

}

//----------------------------------------------------------------------------------------------
// pohyby + animace

//nastavi rovnomerny primocary pohyb
void CGEnMain::MoveElem(CGEnElement *el, float startx, float starty, float startz, float destx, float desty, float destz, UI time)
{

	if(!el) return; //musi se zadat element

	if(el->mel)
	{	//element uz se hejbe nebo se animuje
		el->mel->StartMove(startx,starty,startz,destx,desty,destz,kernelTime,time); //zrusi stary pohyb a nastavi novy
	}else{
		//element se nehejbal ani neanimoval - vytvorim novy MovingElement
		CGEnMoveElem *gmel;
		gmel=new CGEnMoveElem(el,startx,starty,startz,destx,desty,destz,kernelTime,time);
		MovingElems.Add(gmel);
		el->mel=gmel;
	}
}

int CGEnMain::MoveElem(CGEnElement *el, CGEnElement *MovingEl)
{
	if(!el || !MovingEl) return 0;

	float x,y,z;

	MovingEl->GetPos(x,y,z);	
	el->GEnSetPos(x,y,z,1);

	CGEnMoveElem *mel2=MovingEl->mel,*mel;

	if(mel2 && (mel2->type&1)  )
	{
		//MovingEl se hejbe

		if(!el->mel) {
			el->mel = new CGEnMoveElem(el);
			MovingElems.Add(el->mel);
		}
		mel=el->mel;

		mel->type|=1; mel->ack=0;
		mel->startx=mel2->startx; mel->starty=mel2->starty; mel->startz=mel2->startz;
		mel->endx=mel2->endx; mel->endy=mel2->endy; mel->endz=mel2->endz;
		mel->starttime=mel2->starttime; mel->time=mel2->time;
		mel->dx=mel2->dx; mel->dy=mel2->dy; mel->dz=mel2->dz;
		
	}

	return 1;
}


//nastavi animaci
int CGEnMain::SetAnim(CGEnElement *el, CGEnAnim *anim, int frame_in_starttime, UI starttime)
{
	if(!el) return 0; //musi se zadat element

	if(frame_in_starttime==-1)
	{
		frame_in_starttime = 0;
		starttime = kernelTime;
	}

	if(el->mel)
	{	//element uz se hejbe nebo se animuje
		el->mel->StartAnim(anim,frame_in_starttime,starttime); //zrusi starou animaci a nastaci novou
	}else{
		//element se nehejbal ani neanimoval - vytvorim novy MovingElement
		CGEnMoveElem *gmel;
		gmel=new CGEnMoveElem(el,anim,frame_in_starttime,starttime);
		MovingElems.Add(gmel);
		el->mel=gmel;
	}
	return 1;
}

int CGEnMain::SetPicture(CGEnElement *el, CGEnResource *res, int frame_in_starttime, UI starttime)
{
	if(!res||!el) return 0;
	
	eResourceType type = res->GetType();

	if(type==resAnim)
		return SetAnim(el,(CGEnAnim*)res,frame_in_starttime,starttime);
	else
	if(type==resLightTex)
	{
		if(el->mel) el->mel->StopAnim();
		return SetLightTex(el,(CLightTex*)res);
	}

	return 0;
}

int CGEnMain::SetAnimFinishAck(CGEnElement *el, int anim, int move)
{
	if(!el || !el->mel) return 0;
	
	return el->mel->SetAck(anim,move);
}

void CGEnMain::DeleteMovingElem(CGEnMoveElem *mel)
{
	MovingElems.Delete(mel); //vyhodi element ze seznamu
	delete mel; //smazu moving element
}

//----------------------------------------------------------------------------------------------
// update sceny

int CGEnMain::Update(UI curtime)
{
	if(curtime==lastupdatetime) return 0;

	int chng=0;
	float x,y,z;
	UI dt;

	lastupdatetime=curtime;

	CPGEnMoveElemIter melit(MovingElems); //budu prochazet vsechny pohybujici (animujici) elementy

	CGEnMoveElem *el;
	if( !melit.End() ) chng=1;

	while( !melit.End() )
	{
		el=melit;

		if(el->type & 1) //rovnomerny primocary pohyb
		{

			dt = curtime - el->starttime;

			if(dt>=melit->time) //element uz mel dojet do konce
			{
				el->el->GEnSetPos(el->endx,el->endy,el->endz,0); //nastavim konecnou polohu

				if(el->ack&1){
					if(KerMain)
						KerMain->AutosMain->GEnProcessEnded(el->el->ObjPtr, 1); //informuju kernel
					el->ack ^= 1; //shodim 1
				}

				el->type ^= 1; //shodim 1 - zrusi pohyb

			}else
			{
				//hejbnu elementem (rovnomerny prim. pohyb)
				x=floorf(el->startx + dt * el->dx + 0.5f);
				y=floorf(el->starty + dt * el->dy + 0.5f);
				z=floorf(el->startz + dt * el->dz + 0.5f);

				el->el->GEnSetPos(x,y,z,0); //nastavim pozici
			}
		}

		if(el->type & 2) //animace
		{
			if(curtime>=el->nextframetime) //uz je cas na dalsi frame?
			{//ano!
				dt = curtime - el->nextframetime;
				
				int frm = el->nextframeindex; //index dalsiho framu
				int numfr = el->anim->GetNumFrames(); 

				if( dt>=el->anim->GetFrameTime(frm) ) //stihnul jsem to?
				{
					//ne - musim preskocit nekolik obrazku
					do{
						dt-=el->anim->GetFrameTime(frm);
						el->nextframetime += el->anim->GetFrameTime(frm);
						frm++;
						if(frm>=numfr){ //dostal jsem se na konec animacni sekvence
							if(el->anim->repeatflag) //mam opakovat?
							{
								frm=0; //ano
								UI temptm = el->anim->GetTotalTime();
								UI tt2;
								if(dt >= temptm)
								{
									tt2 = dt % temptm;
									el->nextframetime += dt-tt2;
									dt = tt2;
								}
							}
							else
							{
								frm=numfr-1; //ne - zobrazim posledni frame a skoncim
								break;
							}
						}
					}while(dt>=el->anim->GetFrameTime(frm));
				}

				SetLightTex(el->el, el->anim->GetFrameTex(frm)); //nastavi frame

				el->nextframetime+=el->anim->GetFrameTime(frm); //spocte cas dalsiho framu
				
				frm++; //posune na dalsi frame

				if(frm >= numfr) { //dostal jsem se na konec animacni sekvence
					if(el->anim->repeatflag) //mam opakovat?
					{
						frm=0; //ano
					}else{

						if(el->ack&2){
							if(KerMain)
								KerMain->AutosMain->GEnProcessEnded(el->el->ObjPtr, 2); //informuju kernel
							el->ack ^= 2; //shodim 2
						}

						el->type ^= 2; //ne - shodim 2 - konec animace
					}
				}
				
				el->nextframeindex = frm; //nastavim dalsi frame

			}
		}

		if(el->type) //pohyb nebo animace jeste neskoncil
			melit++; //vezmu dalsi
		else
		{   //pohyb i animace skoncil - zrusim MovingElement:
			el->el->mel=NULL; //zrusim MovingElement u elementu
			delete el; //smazu MovingElement
			melit.Remove(); //vyhodim ze seznamu
		}		
	}

	if(scroll) //plynuly scroling
	{
		dt = curtime - scrollStartTime;
		
		if(dt>=scrollTime) //uz jsem mel doscrolovat?
		{//ano
			SetScroll(scrollX,scrollY); //nastavim konecnou pozici (vypne scroling - nastavi scroll na 0)
		}else
		{
			//jeste scroluju...
			CMatrix2D m;

			scrollXnow = x = floorf(scrollStartX + dt * scrollDX + 0.5f);
			scrollYnow = y = floorf(scrollStartY + dt * scrollDY + 0.5f);

			m.Translate(-x+scrollshiftX,-y+scrollshiftY);

			GameWindow->SetInsideTrans(m);
            			
		}
		chng=1;
	}


	return chng;
}

//----------------------------------------------------------------------------------------------
// package

CGEnElSource* CGEnMain::LoadElSource(char *name, CGEnPackage **pkg)
{
	return (CGEnElSource*)PackageMgr.LoadResource(name, resElSource,pkg);
}

CLightTex* CGEnMain::LoadLightTex(char *name, CGEnPackage **pkg)
{
	return (CLightTex*)PackageMgr.LoadResource(name,resLightTex,pkg);
}

CGEnAnim* CGEnMain::LoadAnim(char *name, CGEnPackage **pkg)
{
	return (CGEnAnim*)PackageMgr.LoadResource(name,resAnim,pkg);
}

CTexture* CGEnMain::LoadTexture(char *name, CGEnPackage **pkg)
{
	return (CTexture*)PackageMgr.LoadResource(name,resTex,pkg);
}
CMusicModule* CGEnMain::LoadMusicModule(char *name, CGEnPackage **pkg)
{
	return (CMusicModule*)PackageMgr.LoadResource(name,resMusicModule,pkg);
}

CMusicModule* CGEnMain::LoadMusicModuleDirect(char *name)
{
	CMusicModule *mm = new CMusicModule;
	if(!mm->LoadFromFile(name))
	{
		mm->Release();
		mm=0;
	}

	return mm;			
}

CMusicSample* CGEnMain::LoadMusicSample(char *name, CGEnPackage **pkg)
{
	return (CMusicSample*)PackageMgr.LoadResource(name,resMusicSample,pkg);
}

CGEnResource* CGEnMain::LoadPicture(char *name)
{
	eResourceType type = GetResourceType(name);
	if(type==resAnim)
		return LoadAnim(name);
	else
	if(type==resLightTex)
		return LoadLightTex(name);
	else
		return NULL;
}

eResourceType CGEnMain::GetResourceType(char *name, CGEnPackage **pkg)
{
	return PackageMgr.GetResourceType(name,pkg);
}

//----------------------------------------------------------------------------------------------

CGEnElement* CGEnMain::FindTopElement(float x, float y)
{
	CCache *cache = GameWindow->GetCache();
	CBltElem *fe;

	CGEnElement *el;
	CGEnElement *rel=NULL;

	const CMatrix2D *m;
	GameWindow->GetGlobalTransPtr(&m);

	CVec2 scp;

	scp = m->TransPoint(x,y);

	if(!cache) return 0;

	int num;

	fe = cache->FindFirstEl(scp.x,scp.y,num);

	int cc=0,cmp;
	UC al;

	while(fe)
	{
		el = dynamic_cast<CGEnElement*>(fe);
		if(el)
		{
			if(el->x1<=x && el->y1<y && el->x2>=x && el->y2>y)
			{
				if(el->LightTex){
					al=el->LightTex->GetPixelAlpha((int)(x-el->x1),(int)(y-el->y1));
				}else
					al=1;
				if(al!=0)
				{
					if(rel){
						cmp = CompareTwo(rel,el);
						if(cmp==1) rel=el;
					}else
						rel=el;
					cc++;
				}
			}
		}
		fe=cache->FindNextEl();
	}

	return rel;
}

//----------------------------------------------------------------------------------------------
//vrati polohu okna v souradnicich obrazovky
int CGEnMain::GetWindowPos(float &x1, float &y1, float &x2, float &y2)
{
	if(!GameWindow) return 0;

	const CMatrix2D *m;
	GameWindow->GetGlobalTransPtr(&m);

	CVec2 scp;

	float scx,scy;
	GetScroll(scx,scy);

	float sx,sy;
	GameWindow->GetWindowSize(sx,sy);
    
	scp = m->TransPoint(scx,scy);
	x1=scp.x;y1=scp.y;
	scp = m->TransPoint(scx+sx,scy+sy);
	x2=scp.x;y2=scp.y;

	return 1;
}

int CGEnMain::SetWindowPos(float x1, float y1, float x2, float y2)
{
	if(!GameWindow) return 0;

	CMatrix2D m;

	SetScrollShift(0,0);

	m.Translate(x1,y1);

	GameWindow->SetWindowTrans(m);
	GameWindow->SetWindowSize(x2-x1,y2-y1);

	return 1;
}

int CGEnMain::GetWindowSize(float &x, float &y)
{
	if(!GameWindow) return 0;
	GameWindow->GetWindowSize(x,y);
	return 1;
}

int CGEnMain::AddToWindow( CBltWindow *wnd, int top )
{
	if(!GameWindow) return 0;

	SetScrollShift(0,0);

	if(GameWindow->GetParent())
	{
		GameWindow->GetParent()->RemoveChild(GameWindow);
	}
	if(wnd)
	{
		if(top) 
			wnd->AddChildToTop(GameWindow);
		else
			wnd->AddChildToBottom(GameWindow);
	}
	SetScroll(0,0);
	GameWindow->GetCache()->DeleteAllSurfaces();
	GameWindow->SetNeedRecalc();
	return 1;
}

//----------------------------------------------------------------------------------------------

int CGEnMain::SetVisible(CGEnCursor *cursor, int visible)
{
	if(cursor) 
		return cursor->SetVisible(visible);
	else
		return 0;
}

//----------------------------------------------------------------------------------------------

int CGEnMain::AddLight(int x, int y, int z, UC ir, UC ig, UC ib, int radius )
{
	CLight *l=new CLight(); 
	l->PointLightInt(x,y,z,ir,ig,ib,radius);
	lights.Add(l);
	lightcache->AddLight(l);
	return (int)l;
}

void CGEnMain::DeleteLight(int lightptr)
{
	CLight* l = (CLight*)lightptr;
	if(!l) return;
	
	lightcache->DeleteLight(l);
	lights.Delete(l);

	delete l;
}

void CGEnMain::SetTopLightIntenzity(UC r, UC g, UC b)
{
	if(!toplight) return;
	toplight->SetIntenzity(r,g,b);
	if(GameWindow)
		GameWindow->InvalidateAllTex();;
}

void CGEnMain::GetTopLightIntenzity(UC *R, UC *G, UC *B)
{
	UC r,g,b;
	if(!toplight){
		r=g=b=0;
	}else{
		r=toplight->ir;g=toplight->ig;b=toplight->ib;
	}
	if(R) *R=r;
	if(G) *G=g;
	if(B) *B=b;
}

void CGEnMain::SetTopLightDefault()
{
	if(!toplight) return;

	toplight->SetIntenzity(179,179,179);
}

//----------------------------------------------------------------------------------------------
// testy
/*
CGEnElement *k=0; //jezdici koule
CMusicSample *ms=0;
CLightTex *manik[4];
CGEnAnim *anim=0,*anim2=0;
int pole[20][20][4]; 

#define PKG_PATH "$GAMES$/Krkal_4F88_78B7_A01C_48AB/"
#define PKG_KAMEN "kamen_1234_4BB1_7894_A0E3.pkg"
#define PKG_STENY "steny_1234_4BB1_7894_A0E3.pkg"
#define PKG_PKG1 "package1_1234_4BB1_7894_A0E3.pkg"
#define PKG_PKG2 "package2_1234_4BB1_7894_A0E3.pkg"
#define PKG_BOMBA "bomba_742A_8645_FCA0_1DE4.pkg"
#define PKG_MUSIC "music_65B6_399E_4613_0839.pkg"
#define PKG_SOUND "sound_8A76_670D_A9E4_1A65.pkg"

//vytvori package
void CGEnMain::CreateTexFiles()
{

	CGEnPackage pkg;


	CGEnElSource *podlahaels, *krkalels;
	krkalels = new CGEnElSource();
	krkalels->pbx=-34; krkalels->pby=-34; krkalels->pbz=0;
	krkalels->zdz=40;
   	krkalels->BasePoints = new CVec2[4];
	krkalels->BasePointsCount = 4;
	krkalels->BasePoints[0].x=-20; krkalels->BasePoints[0].y=-20;
	krkalels->BasePoints[1].x=-20; krkalels->BasePoints[1].y=20;
	krkalels->BasePoints[2].x=20; krkalels->BasePoints[2].y=20;
	krkalels->BasePoints[3].x=20; krkalels->BasePoints[3].y=-20;
	krkalels->CalculateBorderPoints();
//	ElSourceList.Add(krkalels);

    
	podlahaels = new CGEnElSource();
	podlahaels->pbx=-20; podlahaels->pby=-20; podlahaels->pbz=0;
	podlahaels->zdz=0;
	podlahaels->BasePoints = new CVec2[4];
	podlahaels->BasePointsCount = 4;
	podlahaels->BasePoints[0].x=-20; podlahaels->BasePoints[0].y=-20;
	podlahaels->BasePoints[1].x=-20; podlahaels->BasePoints[1].y=20;
	podlahaels->BasePoints[2].x=20; podlahaels->BasePoints[2].y=20;
	podlahaels->BasePoints[3].x=20; podlahaels->BasePoints[3].y=-20;
	podlahaels->CalculateBorderPoints();
//	ElSourceList.Add(podlahaels);


	UI sx,sy;
	CLightTex *tex;

	#define IKA "_KRKAL Alpha"
	#define IKAM "_KRKAL Ambient"
	#define IKD "_KRKAL Diffuse"
	#define IKN "_KRKAL Normal"
	#define IKZ "_KRKAL Z Depth"
	#define IKAT "_KRKAL Atmosphere"
	#define IKSH "_KRKAL Shadow"

	#define	PNG ".png"

	if(pkg.CreatePackage(PKG_PATH PKG_PKG1,1))
	{

		pkg.AddResourceFromFile("ptak.jpg","$TEX$/ptakinv.jpg",resTex);
		pkg.AddResourceFromFile("kytka.jpg","$TEX$/kytkainv.jpg",resTex);

		pkg.AddResource("krkal.els",krkalels);
		pkg.AddResource("podlaha.els",podlahaels);

		sx=54;sy=54;
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_KOULE "$MAXIMG$/koule/koule"
		tex->SetAlpha(IMG_KOULE IKA PNG);
		tex->SetDiffuse(IMG_KOULE IKD PNG);
		tex->SetNormal(IMG_KOULE IKN PNG);
		tex->SetZDepth(IMG_KOULE IKZ PNG);
		tex->SetElSource(krkalels);
		//tex->SaveLightTex("$TEX$/lighttest/koule.tex");
		pkg.AddResource("koule.tex",tex);
		tex->Release();



		sx=40;sy=40;
		tex = new CLightTex(sx,sy,LgC_DIFFUSE);
		#define IMG_PODLAHA "$MAXIMG$/podlaha/podlaha"
		tex->SetDiffuse(IMG_PODLAHA IKD PNG);
		tex->SetElSource(podlahaels);
		pkg.AddResource("podlaha.tex",tex);
		tex->Release();

		sx=54;sy=54;
		#define IMG_MANIK "$MAXIMG$/manik/manik" 

		#define MANIK(m) \
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH); \
		tex->SetAlpha(IMG_MANIK m IKA PNG); \
		tex->SetDiffuse(IMG_MANIK m IKD PNG); \
		tex->SetNormal(IMG_MANIK m IKN PNG);\
		tex->SetZDepth(IMG_MANIK m IKZ PNG);\
		tex->SetShadow(IMG_MANIK m IKSH PNG,-34,-34);\
		tex->SetElSource(krkalels);\
		pkg.AddResource("manik" m ".tex",tex);\
		tex->Release();
	
		MANIK("1");
		MANIK("2");
		MANIK("3");
		MANIK("4");

	}

	if(pkg.CreatePackage(PKG_PATH PKG_PKG2,1))
	{
		pkg.AddResource("krkal.els",krkalels);

		sx=54;sy=54;
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_KLIC "$MAXIMG$/klic/klic_cerveny"
		tex->SetAlpha(IMG_KLIC IKA PNG);tex->SetDiffuse(IMG_KLIC IKD PNG);
		tex->SetNormal(IMG_KLIC IKN PNG);tex->SetZDepth(IMG_KLIC IKZ PNG);
		tex->SetShadow(IMG_KLIC IKSH PNG,-34,-34);
		tex->SetElSource(krkalels);
		pkg.AddResource("klic_cerveny.tex",tex);
		tex->Release();
		#undef IMG_KLIC

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_KLIC "$MAXIMG$/klic/klic_modry"
		tex->SetAlpha(IMG_KLIC IKA PNG);tex->SetDiffuse(IMG_KLIC IKD PNG);
		tex->SetNormal(IMG_KLIC IKN PNG);tex->SetZDepth(IMG_KLIC IKZ PNG);
		tex->SetShadow(IMG_KLIC IKSH PNG,-34,-34);
		tex->SetElSource(krkalels);
		pkg.AddResource("klic_modry.tex",tex);
		tex->Release();
		#undef IMG_KLIC

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_KLIC "$MAXIMG$/klic/klic_zluty"
		tex->SetAlpha(IMG_KLIC IKA PNG);tex->SetDiffuse(IMG_KLIC IKD PNG);
		tex->SetNormal(IMG_KLIC IKN PNG);tex->SetZDepth(IMG_KLIC IKZ PNG);
		tex->SetShadow(IMG_KLIC IKSH PNG,-34,-34);
		tex->SetElSource(krkalels);
		pkg.AddResource("klic_zluty.tex",tex);
		tex->Release();
		#undef IMG_KLIC

		sx=54;sy=54;
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_ZAMEK "$MAXIMG$/zamek/zamek"
		tex->SetAlpha(IMG_ZAMEK IKA PNG);
		tex->SetDiffuse(IMG_ZAMEK IKD PNG);
		tex->SetNormal(IMG_ZAMEK IKN PNG);
		tex->SetZDepth(IMG_ZAMEK IKZ PNG);
		tex->SetElSource(krkalels);
		pkg.AddResource("zamek.tex",tex);
		tex->Release();

		sx=54;sy=54;
		CLightTex *t[3];
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_TELEPORT "$MAXIMG$/teleport/teleport1"
		tex->SetAlpha(IMG_TELEPORT IKA PNG); tex->SetDiffuse(IMG_TELEPORT IKD PNG);
		tex->SetNormal(IMG_TELEPORT IKN PNG); tex->SetZDepth(IMG_TELEPORT IKZ PNG);
		tex->SetElSource(krkalels);
		pkg.AddResource("teleport1.tex",tex);
		#undef IMG_TELEPORT
		t[0]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_TELEPORT "$MAXIMG$/teleport/teleport2"
		tex->SetAlpha(IMG_TELEPORT IKA PNG); tex->SetDiffuse(IMG_TELEPORT IKD PNG);
		tex->SetNormal(IMG_TELEPORT IKN PNG); tex->SetZDepth(IMG_TELEPORT IKZ PNG);
		tex->SetElSource(krkalels);
		pkg.AddResource("teleport2.tex",tex);
		#undef IMG_TELEPORT
		t[1]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_TELEPORT "$MAXIMG$/teleport/teleport3"
		tex->SetAlpha(IMG_TELEPORT IKA PNG); tex->SetDiffuse(IMG_TELEPORT IKD PNG);
		tex->SetNormal(IMG_TELEPORT IKN PNG); tex->SetZDepth(IMG_TELEPORT IKZ PNG);
		tex->SetElSource(krkalels);
		pkg.AddResource("teleport3.tex",tex);
		#undef IMG_TELEPORT
		t[2]=tex;

		CGEnAnim *an;
		an = new CGEnAnim();
		an->SetNumFrames(3);
		an->SetFrame(0,t[0],180);
		an->SetFrame(1,t[1],180);
		an->SetFrame(2,t[2],180);
		an->repeatflag=1;
		pkg.AddResource("teleport.ani",an);
		an->Release();

		for(int i=0;i<3;i++)
			t[i]->Release();

	}

	if(pkg.CreatePackage(PKG_PATH PKG_BOMBA,1))
	{
		pkg.AddResource("krkal.els",krkalels);
		
		CLightTex *bomba[8];
		int bb=0;

		sx=54;sy=54;
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba01"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba1.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba02"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba2.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba03"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba3.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba04"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba4.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH|LgC_ATMOSPHERE);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba05"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		tex->SetAtmosphere(IMG_BOMBA IKAT PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba5.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH|LgC_ATMOSPHERE);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba06"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		tex->SetAtmosphere(IMG_BOMBA IKAT PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba6.tex",tex);
		bomba[bb++]=tex;

		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_AMBIENT|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH|LgC_ATMOSPHERE);
		#define IMG_BOMBA "$MAXIMG$/bomba/bomba07"
		tex->SetAlpha(IMG_BOMBA IKA PNG);tex->SetAmbient(IMG_BOMBA IKAM PNG);tex->SetDiffuse(IMG_BOMBA IKD PNG);
		tex->SetNormal(IMG_BOMBA IKN PNG);tex->SetZDepth(IMG_BOMBA IKZ PNG);
		tex->SetAtmosphere(IMG_BOMBA IKAT PNG);
		#undef IMG_BOMBA
		tex->SetElSource(krkalels);
		pkg.AddResource("bomba7.tex",tex);
		bomba[bb++]=tex;

		CGEnAnim *an;
		an = new CGEnAnim();
		an->SetNumFrames(8);
		an->SetFrame(0,bomba[0],200);
		an->SetFrame(1,bomba[1],150);
		an->SetFrame(2,bomba[2],150);
		an->SetFrame(3,bomba[3],150);
		an->SetFrame(4,bomba[4],300);
		an->SetFrame(5,bomba[5],120);
		an->SetFrame(6,bomba[6],120);
		an->SetFrame(7,bomba[7],120);
		an->repeatflag=1;
		pkg.AddResource("bomba.ani",an);
		an->Release();

		for(int i=0;i<8;i++)
			bomba[i]->Release();
	}


	if(pkg.CreatePackage(PKG_PATH PKG_STENY,1))
	{
		pkg.AddResource("stena.els",krkalels);

		#define IMG_STENA "$MAXIMG$/stena/stena"
		sx=54;sy=54;

		#define STENA(s)  \
		tex = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH); \
		tex->SetAlpha(IMG_STENA s IKA PNG); \
		tex->SetDiffuse(IMG_STENA s IKD PNG); \
		tex->SetNormal(IMG_STENA s IKN PNG); \
		tex->SetZDepth(IMG_STENA s IKZ PNG); \
		tex->ClearOutPixels(); \
		tex->SetElSource(krkalels); \
		pkg.AddResource("stena"s".tex",tex); \
		tex->Release();

		STENA("0000");
		STENA("0001");
		STENA("0010");
		STENA("0011");
		STENA("0100");
		STENA("0101");
		STENA("0110");
		STENA("0111");
		STENA("1000");
		STENA("1001");
		STENA("1010");
		STENA("1011");
		STENA("1100");
		STENA("1101");
		STENA("1110");
		STENA("1111");

	}

	if(pkg.CreatePackage(PKG_PATH PKG_KAMEN,1))
	{
		pkg.AddResource("kamen.els",krkalels);

		#define IMG_KAMEN "$MAXIMG$/kamen/kamen"
		sx=54;sy=54;

		int i=0;
		CLightTex *kamen[10];

		#define KAMEN(s)  \
		kamen[i] = new CLightTex(sx,sy,LgC_ALPHA|LgC_DIFFUSE|LgC_NORMAL|LgC_ZDEPTH); \
		kamen[i]->SetAlpha(IMG_KAMEN IKA s PNG); \
		kamen[i]->SetDiffuse(IMG_KAMEN IKD s PNG); \
		kamen[i]->SetNormal(IMG_KAMEN IKN s PNG); \
		kamen[i]->SetZDepth(IMG_KAMEN IKZ s PNG); \
		kamen[i]->ClearOutPixels(); \
		kamen[i]->SetElSource(krkalels); \
		pkg.AddResource("kamen"s".tex",kamen[i]); \
		i++;

		KAMEN("0000");
		KAMEN("0001");
		KAMEN("0002");
		KAMEN("0003");
		KAMEN("0004");
		KAMEN("0005");
		KAMEN("0006");
		KAMEN("0007");
		KAMEN("0008");
		KAMEN("0009");

		CGEnAnim *an;
		an = new CGEnAnim();
		an->SetNumFrames(10);
		for(i=0;i<10;i++)
			an->SetFrame(i,kamen[i],60);
		an->repeatflag=1;
		pkg.AddResource("kamen1.ani",an);
		an->Release();

		an = new CGEnAnim();
		an->SetNumFrames(10);
		an->SetFrame(0,kamen[0],60);
		for(i=1;i<10;i++)
			an->SetFrame(i,kamen[10-i],60);
		an->repeatflag=1;
		pkg.AddResource("kamen2.ani",an);
		an->Release();


		for(i=0;i<10;i++)
			kamen[i]->Release();

	}

	krkalels->Release();
	podlahaels->Release();

}

void CGEnMain::CreateTest()
{
	return;

//	if(testGEn) return;

//	UnloadResources();


	AddToWindow(DXbliter->rootwnd,0);
	SetWindowPos(0,0,640,480);

	int f,g,h;
	int x,y,z;

	CLight *l;
	l=new CLight(); l->PointLight(400,200,70,1,0,1,100);  lights.Add(l);
	l=new CLight(); l->PointLight(600,80,70,1,0,1,100); lights.Add(l);
	l=new CLight(); l->PointLight(410,645,35,.8f,.8f,.8f,60); lights.Add(l);
	l=new CLight(); l->PointLight(120,120,40,0,1,0,70); lights.Add(l);

	CPLightListIter lgit(lights);
	while( !lgit.End() )
	{
		lightcache->AddLight(lgit);
		lgit++;
	}

	char *curdir=NULL;
	FS->GetCurDir(&curdir);

	CLightTex *stena[16],*podlaha,*koule,*klicR,*klicY,*klicB,*zamek,*bomba;
	CGEnAnim *bomba_vybuch,*teleport;

		podlaha = LoadLightTex(PKG_PATH PKG_PKG1 "/podlaha.tex");
		if(!podlaha) throw CExc(eGEn,0,"Error: loading podlaha.tex");

		manik[0] = LoadLightTex(PKG_PATH PKG_PKG1 "/manik1.tex");
		if(!manik[0]) throw CExc(eGEn,0,"Error: loading manik1.tex");
		manik[1] = LoadLightTex(PKG_PATH PKG_PKG1 "/manik2.tex");
		if(!manik[1]) throw CExc(eGEn,0,"Error: loading manik2.tex");
		manik[2] = LoadLightTex(PKG_PATH PKG_PKG1 "/manik3.tex");
		if(!manik[2]) throw CExc(eGEn,0,"Error: loading manik3.tex");
		manik[3] = LoadLightTex(PKG_PATH PKG_PKG1 "/manik4.tex");
		if(!manik[3]) throw CExc(eGEn,0,"Error: loading manik4.tex");

		koule = LoadLightTex(PKG_PATH PKG_PKG1 "/koule.tex");
		if(!koule) throw CExc(eGEn,0,"Error: loading koule.tex");

		klicR = LoadLightTex(PKG_PATH PKG_PKG2 "/klic_cerveny.tex");
		if(!klicR) throw CExc(eGEn,0,"Error: loading klic.tex");
		klicY = LoadLightTex(PKG_PATH PKG_PKG2 "/klic_zluty.tex");
		if(!klicY) throw CExc(eGEn,0,"Error: loading klic.tex");
		klicB = LoadLightTex(PKG_PATH PKG_PKG2 "/klic_modry.tex");
		if(!klicB) throw CExc(eGEn,0,"Error: loading klic.tex");

		zamek = LoadLightTex(PKG_PATH PKG_PKG2 "/zamek.tex");
		if(!zamek) throw CExc(eGEn,0,"Error: loading zamek.tex");

		bomba = LoadLightTex(PKG_PATH PKG_BOMBA "/bomba.tex");
		if(!bomba) throw CExc(eGEn,0,"Error: loading bomba.tex");

		bomba_vybuch = LoadAnim(PKG_PATH PKG_BOMBA "/bomba.ani");
		if(!bomba_vybuch) throw CExc(eGEn,0,"Error: loading bomba.ani");

		teleport = LoadAnim(PKG_PATH PKG_PKG2 "/teleport.ani");
		if(!teleport) throw CExc(eGEn,0,"Error: loading teleport.ani");

		char st[] = PKG_PATH PKG_STENY "/stena0000.tex";
		int pst = strlen(st)-8;
		for(int i=0;i<16;i++)
		{
			if(i&8) st[pst] = '1'; else st[pst] = '0';
			if(i&4) st[pst+1] = '1'; else st[pst+1] = '0';
			if(i&2) st[pst+2] = '1'; else st[pst+2] = '0';
			if(i&1) st[pst+3] = '1'; else st[pst+3] = '0';

			stena[i] = LoadLightTex(st);
			if(!stena[i]) throw CExc(eGEn,0,"Error: loading package steny");
		}


	anim = LoadAnim(PKG_PATH PKG_KAMEN "/kamen1.ani");
	if(!anim) throw CExc(eGEn,0,"Error: loading animation kamen1.ani");
	anim2 = LoadAnim(PKG_PATH PKG_KAMEN "/kamen2.ani");
	if(!anim2) throw CExc(eGEn,0,"Error: loading animation kamen2.ani");
	
	CMusicModule *mm;
	x=rand()%5;
	switch(x)
	{
	case 0:
		mm = LoadMusicModuleDirect(PKG_PATH PKG_MUSIC "/AFRIKA.mm");
		break;
	case 1:
		mm = LoadMusicModuleDirect(PKG_PATH PKG_MUSIC "/europe.mm");
		break;
	case 2:
		mm = LoadMusicModuleDirect(PKG_PATH PKG_MUSIC "/america.mm");
		break;
	case 3:
		mm = LoadMusicModuleDirect(PKG_PATH PKG_MUSIC "/asia.mm");
		break;
	case 4:
		mm = LoadMusicModuleDirect(PKG_PATH PKG_MUSIC "/rhitm.mm");
		break;
	}


	ms = LoadMusicSample(PKG_PATH PKG_SOUND "/klon.ms");
	//if(!ms) throw CExc(eGEn,0,"Error: loading sample");

	if(mm) {
		mm->Reset();
		mm->Play();
		mm->Release();
	}



	for(f=0;f<20;f++) for (g=0;g<20;g++) for (h=0;h<4;h++) pole[f][g][h]=0;

#define DrEl(x,y,z,tex) DrawElement((x)*40.f,(y)*40.f,(z)*40.f,tex),pole[x][y][z]=1

	for(y=0;y<20;y++)
		for(x=0;x<20;x++)
			DrawElement(x*40.f,y*40.f,0,podlaha);

	for(x=1;x<19;x++)
	{
		if(x!=5) DrEl(x,0,0,stena[5]);
		if(x!=8) DrEl(x,19,0,stena[5]);
		if(x!=9) DrEl(0,x,0,stena[10]);
		if(x!=13) DrEl(19,x,0,stena[10]);
	}

	DrEl(0,0,0,stena[6]);
	DrEl(0,19,0,stena[12]);
	DrEl(19,0,0,stena[3]);
	DrEl(19,19,0,stena[9]);

	DrEl(5,0,0,stena[7]);
	DrEl(0,9,0,stena[14]);
	DrEl(8,19,0,stena[13]);
	DrEl(19,13,0,stena[11]);
	for(x=1;x<7;x++)
	{
		DrEl(5,x,0,stena[10]);
		DrEl(x,9,0,stena[5]);
		DrEl(8,19-x,0,stena[10]);
		if(x!=4) DrEl(19-x,13,0,stena[5]);
	}
	DrEl(5,7,0,stena[8]);
	DrEl(7,9,0,stena[1]);
	DrEl(8,(19-7),0,stena[2]);
	DrEl((19-7),13,0,stena[4]);
	
	DrEl((19-4),13,0,stena[15]);
	DrEl((19-4),5,0,stena[2]);
	for(x=6;x<13;x++)
		DrEl((19-4),x,0,stena[10]);
	DrEl((19-4),14,0,stena[10]);
	DrEl((19-4),15,0,stena[8]);

	int r;

	for(int i=0;i<50;i++)
	{
		do{
		x=rand()%20;y=rand()%20;z=0;
		}while(pole[x][y][z]);
		
		r=rand()%27;

		if( r<10) DrEl(x,y,z,koule); else
		if( r<14) DrEl(x,y,z,bomba); else
		if( r<16) DrEl(x,y,z,bomba_vybuch); else
		if( r<17) DrEl(x,y,z,zamek); else
		if( r<21) DrEl(x,y,z,klicR); else
		if( r<23) DrEl(x,y,z,klicB); else 
		if( r<25) DrEl(x,y,z,klicY); else 
			      DrEl(x,y,z,teleport);
	}


	tyx=6;tyy=7;tyz=0;
	ty=DrawElement((float)tyx*40,(float)tyy*40,(float)tyz*40,manik[0]);
	tydx=0; tydy=0; tydz=0;


	k=DrawElement(320,120,0,anim);

	float cx=5*40,cy=2*40,cz=0*40;

	CGEnCursor *cur =  CreateCursor(0,40,40,40,0xFFFFFFFF,0x40FFFFFF);
	DrawCursor(cur,cx,cy,cz);
//	cur->MoveCursor(40,0,0);

	if(curdir)
	{
		FS->ChangeDir(curdir);
		delete[] curdir;
	}

//	testGEn = 1;

}


void CGEnMain::ComeOnBaby(){

	return;

	static int km=0;
//	if(!testGEn) {km=0;return;}

	static float kx=1.f;
	float pos=8;

	static int dl=0;

	DebugMessage(9,"%i",lightcache->numcmptex);
	lightcache->numcmptex=0;

	if(k)
	{
	if(k->zx==320&&km==0){
		SetAnim(k,anim,0);

		MoveElem(k,320,120,0,720,120,0,1700);
		km=1;
	}else
		if(k->zx==720&&km==1){
		SetAnim(k,anim2,0);

		MoveElem(k,720,120,0,320,120,0,1700);	
		km=0;
	}

		if(Input->IsKeyDown(DIK_Q))
		{
			DeleteElement(k);
			k=NULL;
		}

	}


	int sm=-1;

	if (tydx==0&&tydy==0&&tydz==0) {
		if (Input->IsKeyDown(DIK_LEFT)) tydx-=40,sm=1;
		if (Input->IsKeyDown(DIK_RIGHT)) tydx+=40,sm=0;
		if (Input->IsKeyDown(DIK_UP)) tydy-=40,sm=2;
		if (Input->IsKeyDown(DIK_DOWN)) tydy+=40,sm=3;
		if (Input->IsKeyDown(DIK_A)) tydz+=40;
		if (Input->IsKeyDown(DIK_Z)) tydz-=40;

		if(tydx||tydy||tydz)
		{
			int x,y,z;
			x=tyx+tydx/40;y=tyy+tydy/40;z=tyz+tydz/40;
			if(sm>=0) SetLightTex(ty,manik[sm]);
			if(x>=0&&y>=0&&z>=0&&x<20&&y<20&&z<4&&pole[x][y][z]==0)
			{
				MoveElem(ty,(float)tyx*40,(float)tyy*40,(float)tyz*40,(float)(tyx*40+tydx),(float)(tyy*40+tydy),(float)(tyz*40+tydz),165);
				tyx+=tydx/40;tyy+=tydy/40;tyz+=tydz/40;
			}else{
				if(ms) ms->Play();
			}
		}

	} 
	{

		if (tydx<0)tydx+=(int)pos;
		if (tydx>0)tydx-=(int)pos;
		if (tydy<0)tydy+=(int)pos;
		if (tydy>0)tydy-=(int)pos;
		if (tydz>0)tydz-=(int)pos;
		if (tydz<0)tydz+=(int)pos;
	}

	int a=0;

	CLight *l=lights.el->data;
	if (Input->IsKeyDown(DIK_K)){

		lightcache->DeleteLight(l);
		l->px+=8;
		lightcache->AddLight(l);
	}
	if (Input->IsKeyDown(DIK_H)){

		lightcache->DeleteLight(l);
		l->px-=8;
		lightcache->AddLight(l);
	}
	if (Input->IsKeyDown(DIK_U)){

		lightcache->DeleteLight(l);
		l->py-=8;
		lightcache->AddLight(l);
	}
	if (Input->IsKeyDown(DIK_J)){

		lightcache->DeleteLight(l);
		l->py+=8;
		lightcache->AddLight(l);
	}
	if (Input->IsKeyDown(DIK_G)){

		lightcache->DeleteLight(l);
		l->pz-=1;
		lightcache->AddLight(l);
	}
	if (Input->IsKeyDown(DIK_T)){

		lightcache->DeleteLight(l);
		l->pz+=1;
		lightcache->AddLight(l);
	}

	if (Input->IsKeyDown(DIK_B))
	{
		if(!dl)
		{
			CGEnElement* el = FindTopElement((float)l->px,(float)l->py);
			if(el)
			{
				DeleteElement(el);
				dl=1;
			}
		}
	}else
		dl=0;
	//DebugMessage(11,0xFF00FF00,"LP: %i %i %i",l->px,l->py,l->pz);

	static int lgi=255;
	static int lgs=0;
	if (Input->IsKeyDown(DIK_L)) lgs=-32;
	if(lgs)
	{
		CLight *l=lights.el->data;

		lightcache->DeleteLight(l);

		lgi+=lgs;
		if(lgi<0){lgi=0;lgs=-lgs;} else
		if(lgi>255){lgi=255;lgs=0;} 
		l->ir=255-lgi;
		l->ig=lgi;

		lightcache->AddLight(l);
	}


	static float scx=0,scy=0;
	
	{
		float xx=0,yy=0;

		if(Input->IsKeyDown(DIK_NUMPAD8)) yy-=pos,a=1;
		if(Input->IsKeyDown(DIK_NUMPAD2)) yy+=pos,a=1;
		if(Input->IsKeyDown(DIK_NUMPAD4)) xx-=pos,a=1;
		if(Input->IsKeyDown(DIK_NUMPAD6)) xx+=pos,a=1;
		if(a){
			Scroll(scx,scy,scx+xx,scy+yy,33);
			//SetScroll(scx+xx,scy+yy);
			scx+=xx;scy+=yy;
		}

		if(!scroll&&Input->IsKeyDown(DIK_NUMPAD5))
		{
			Scroll(scrollX,scrollY,200,200,1000);
			scx=200;scy=200;
		}
	
	}

}
*/


CSortResultHashTable::CSortResultHashTable()
{
	for(int i=0;i<SORT_HTSIZE;i++)
	{
		ht[i]=0;
		ptrs[i]=-1;
	}
	fp=-1;
	SRstack=0;
}

CSortResultHashTable::~CSortResultHashTable()
{
	EmptyTable();
	CSortResult *s,*sr;
	s=SRstack;
	while(s)
	{
		sr=s;
		s=s->next;
		delete sr;
	}
	SRstack=0;
}

void CSortResultHashTable::EmptyTable()
{
	CSortResult *s;
	int i,tm;
	
	i=fp;
	while(i!=-1)
	{
		s=ht[i];
		if(s){
			while(s->next) s=s->next;
			s->next=SRstack;
			SRstack=ht[i];
		}
		ht[i]=0;
		tm=ptrs[i];
		ptrs[i]=-1;

		i=tm;

	}
	fp=-1;
}