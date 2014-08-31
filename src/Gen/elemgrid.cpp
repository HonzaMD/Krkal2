////////////////////////////////////////////////////////////////////////////////////////////////////
//
// elemgrid.cpp
//
// Mriz GEn elementu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "elemgrid.h"


#include "normalizer.h"

#include "genmain.h"


//////////////////////////////////////////////////////////////////////////////////////////
//
// CElemGrid
//
//////////////////////////////////////////////////////////////////////////////////////////

CElemGrid::CElemGrid(int _minx,int _miny,int _minz,int _maxx,int _maxy,int _maxz, int _sqsx, int _sqsy, int _sqsz)
{
	sqsx=_sqsx;sqsy=_sqsy;sqsz=_sqsz;

	minx=_minx;miny=_miny;minz=_minz;

    numsqx=(_maxx-_minx)/sqsx + 1;
	numsqy=(_maxy-_miny)/sqsy + 1;
	numsqz=(_maxz-_minz)/sqsz + 1;

	maxx=minx + numsqx*sqsx;
	maxy=miny + numsqy*sqsy;
	maxz=miny + numsqz*sqsz;

	map=new CPGEnElemList[numsqx*numsqy*numsqz];

	numelemsout=0;
	maxelemsout=5;

}

CElemGrid::~CElemGrid()
{

	SAFE_DELETE_ARRAY(map);
}

int CElemGrid::AddElem(CGEnElement *el, int addingoutelem)
{
	int x1,x2,y1,y2,z1,z2;
	int sqx1,sqx2,sqy1,sqy2,sqz1,sqz2;
	
	int x,y,z;

	CPGEnElemList *box;
	int ady,adx;
	int rv;

	x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz + el->ElSource()->pbz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

	rv=CmpSQ(x1,y1,z1,x2,y2,z2,sqx1,sqy1,sqz1,sqx2,sqy2,sqz2);

	if(!rv){
		numelemsout++;
		elemsout.Add(el);
	}

	box=&map[sqz1 + numsqz * ( sqx1 + sqy1 * numsqx) ];

	adx=numsqz - (sqz2-sqz1);
	ady=numsqz * ( numsqx - (sqx2-sqx1) );

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			for(z=sqz1;z<sqz2;z++)
			{
				if(addingoutelem) box->AddUnique(el); else box->Add(el);
				box++;
			}
			box+=adx;
		}
		box+=ady;
	}

	if(numelemsout>maxelemsout && !addingoutelem )
	{
		Resize();
		rv=CmpSQ(x1,y1,z1,x2,y2,z2,sqx1,sqy1,sqz1,sqx2,sqy2,sqz2);
	}

	return 1;
}

int CElemGrid::DeleteElem(CGEnElement *el)
{
	int x1,x2,y1,y2,z1,z2;
	int sqx1,sqx2,sqy1,sqy2,sqz1,sqz2;

	int x,y,z;

	CPGEnElemList *box;
	int ady,adx;
	int rv;

	x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz + el->ElSource()->pbz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

	rv=CmpSQ(x1,y1,z1,x2,y2,z2,sqx1,sqy1,sqz1,sqx2,sqy2,sqz2);
	
	if(!rv)
	{
		if(elemsout.Delete(el)) numelemsout--;
	}

	box=&map[sqz1 + numsqz * ( sqx1 + sqy1 * numsqx) ];

	adx=numsqz - (sqz2-sqz1);
	ady=numsqz * ( numsqx - (sqx2-sqx1) );

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			for(z=sqz1;z<sqz2;z++)
			{
				box->Delete(el);
				box++;
			}
			box+=adx;
		}
		box+=ady;
	}
	return 1;
}

int CElemGrid::InvalidElems(int x1,int y1,int z1,int x2, int y2, int z2)
{

	int sqx1,sqx2,sqy1,sqy2,sqz1,sqz2;
	
	CPGEnElemList *box;
	CPGEnElemListElem *el;
	int ady,adx;
	int rv;

	rv=CmpSQ(x1,y1,z1,x2,y2,z2,sqx1,sqy1,sqz1,sqx2,sqy2,sqz2);

	if(!rv)
	{
		el=elemsout.el;
		while(el)
		{
			el->data->InvalidCache();
			el->data->SetNeedRecalc(dxrcTex);
			el=el->next;
		}
	}

	box=&map[sqz1 + numsqz * ( sqx1 + sqy1 * numsqx) ];

	adx=numsqz - (sqz2-sqz1);
	ady=numsqz * ( numsqx - (sqx2-sqx1) );

	for(int y=sqy1;y<sqy2;y++)
	{
		for(int x=sqx1;x<sqx2;x++)
		{
			for(int z=sqz1;z<sqz2;z++)
			{
				el=box->el;
				while(el)
				{
					el->data->InvalidCache();
					el->data->SetNeedRecalc(dxrcTex);
					el=el->next;
				}	
				box++;
			}
			box+=adx;
		}
		box+=ady;
	}

	return 1;
}


int CElemGrid::CmpSQ(int x1, int y1, int z1, int x2, int y2, int z2, int &sqx1, int &sqy1, int &sqz1, int &sqx2, int &sqy2, int &sqz2)
{

	int dd;
	int FG=1;

	dd=x1-minx; if(dd>=0) sqx1=dd/sqsx; else sqx1=(dd+1)/sqsx-1;
	dd=y1-miny; if(dd>=0) sqy1=dd/sqsy; else sqy1=(dd+1)/sqsy-1;
	dd=z1-minz; if(dd>=0) sqz1=dd/sqsz; else sqz1=(dd+1)/sqsz-1;

	dd=x2-minx; if(dd>=0) sqx2=dd/sqsx+1; else sqx2=(dd+1)/sqsx;
	dd=y2-miny; if(dd>=0) sqy2=dd/sqsy+1; else sqy2=(dd+1)/sqsy;
	dd=z2-minz; if(dd>=0) sqz2=dd/sqsz+1; else sqz2=(dd+1)/sqsz;


#define SET_IN_FG(x,min,max) if(x<min){x=0;FG=0;}else if(x>max){x=max;FG=0;}


	SET_IN_FG(sqx1,0,numsqx);SET_IN_FG(sqx2,0,numsqx);
	SET_IN_FG(sqy1,0,numsqy);SET_IN_FG(sqy2,0,numsqy);
	SET_IN_FG(sqz1,0,numsqz);SET_IN_FG(sqz2,0,numsqz);

	return FG;
}

int CElemGrid::Resize()
{

	if(numelemsout==0) return 1;

	int x1,y1,z1,x2,y2,z2;
	int t;

	CPGEnElemListElem *le=elemsout.el;
	CGEnElement *el;
	el=le->data;

	x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz + el->ElSource()->pbz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

	le=le->next;
	while(le)
	{
		el=le->data;
		
		t = (int)(el->zx + el->ElSource()->minx); if(t<x1) x1=t;
		t = (int)(el->zx + el->ElSource()->maxx)-1; if(t>x2) x2=t;
		t = (int)(el->zy + el->ElSource()->miny); if(t<y1) y1=t;
		t = (int)(el->zy + el->ElSource()->maxy)-1; if(t>y2) y2=t;
		t = (int)(el->zz + el->ElSource()->pbz); if(t<z1) z1=t;
		if(el->ElSource()->zdz>0) t = (int)(el->zz + el->ElSource()->zdz)-1;
		if(t>z2) z2=t;

		le=le->next;
	}

	return Resize(x1,y1,z1,x2,y2,z2);

}

int CElemGrid::Resize(int x1,int y1,int z1,int x2,int y2,int z2)
{
	int t;
	int xsq1=0,ysq1=0,zsq1=0,xsq2=0,ysq2=0,zsq2=0;

	t=minx-x1;
	if(t>0) xsq1=(t-1)/sqsx+1;

	t=miny-y1;
	if(t>0) ysq1=(t-1)/sqsy+1;

	t=minz-z1;
	if(t>0) zsq1=(t-1)/sqsz+1;

	t=x2-maxx;
	if(t>=0) xsq2=t/sqsx+1;

	t=y2-maxy;
	if(t>=0) ysq2=t/sqsy+1;

	t=z2-maxz;
	if(t>=0) zsq2=t/sqsz+1;

	minx-=sqsx*xsq1; miny-=sqsy*ysq1; minz-=sqsz*zsq1;
	maxx+=sqsx*xsq2; maxy+=sqsy*ysq2; maxz+=sqsz*zsq2;

	int ssx,ssy,ssz;
	ssx=numsqx+xsq1+xsq2;
	ssy=numsqy+ysq1+ysq2;
	ssz=numsqz+zsq1+zsq2;

	int x,y,z;
	CPGEnElemList *nmap=new CPGEnElemList[ssx*ssy*ssz];

	CPGEnElemList *oldl,*newl;

	oldl=map;
	newl=&nmap[ zsq1 + ssz*(xsq1 + ysq1*ssx) ];

	int adx,ady;

	adx = ssz-numsqz;
	ady = (ssx-numsqx)*ssz;

	for(y=0;y<numsqy;y++)
	{
		for(x=0;x<numsqx;x++)
		{
			for(z=0;z<numsqz;z++)
			{
				newl->el = oldl->el;
				oldl->el = 0;

				newl++;
				oldl++;
			}

			newl+=adx;

		}

		newl+=ady;
	}


	numsqx=ssx;
	numsqy=ssy;
	numsqz=ssz;
	
	delete[] map;
	map=nmap;

	CPGEnElemList oldelemsout;
	CPGEnElemListElem *le;

	oldelemsout.el=elemsout.el; elemsout.el=NULL;
	numelemsout=0;

	le=oldelemsout.el;
	while(le)
	{
		AddElem(le->data,1);
		le=le->next;
	}

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CShadowGrid
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CShadowGrid::CShadowGrid(int _minx,int _miny,int _maxx,int _maxy,int _sqsx, int _sqsy)
{
	sqsx=_sqsx;sqsy=_sqsy;

	minx=_minx;miny=_miny;

    numsqx=(_maxx-_minx)/sqsx + 1;
	numsqy=(_maxy-_miny)/sqsy + 1;

	maxx=minx + numsqx*sqsx;
	maxy=miny + numsqy*sqsy;

	shadows=new CPShGrInstList[numsqx*numsqy];
	elems = new CPGEnElemList[numsqx*numsqy];

	numshadowsout=0; maxshadowsout=3;
	numelemsout=0; maxelemsout=5;

}

CShadowGrid::~CShadowGrid()
{
	for(int i=0;i<numsqx*numsqy;i++)
		ReleaseListElems(shadows[i]);

	ReleaseListElems(shadowsout);

	SAFE_DELETE_ARRAY(shadows);
	SAFE_DELETE_ARRAY(elems);
}


int CShadowGrid::Resize()
{

	if(numshadowsout==0&&maxelemsout==0) return 1;

	int x1=minx,y1=miny,x2=maxx,y2=maxy;
	int xx1,yy1,xx2,yy2;

	CPShGrInstListElem *le=shadowsout.el;
	CShadowGridInstance *shi;

	while(le)
	{
		shi=le->data;

		x1 = MIN(x1,shi->x1);
		y1 = MIN(y1,shi->y1);
		x2 = MAX(x2,shi->x2);
		y2 = MAX(y2,shi->y2);
		
		le=le->next;
	}

	CPGEnElemListElem *leelem=elemsout.el;

	while(leelem)
	{
		CmpElemBRect(leelem->data,xx1,yy1,xx2,yy2);
		x1 = MIN(x1,xx1);
		y1 = MIN(y1,yy1);
		x2 = MAX(x2,xx2);
		y2 = MAX(y2,yy2);
		leelem=leelem->next;
	}

	return Resize(x1,y1,x2,y2);

}

int CShadowGrid::Resize(int x1,int y1,int x2,int y2)
{
	int t;
	int xsq1=0,ysq1=0,xsq2=0,ysq2=0;

	t=minx-x1;
	if(t>0) xsq1=(t-1)/sqsx+1;

	t=miny-y1;
	if(t>0) ysq1=(t-1)/sqsy+1;

	t=x2-maxx;
	if(t>=0) xsq2=t/sqsx+1;

	t=y2-maxy;
	if(t>=0) ysq2=t/sqsy+1;

	minx-=sqsx*xsq1; miny-=sqsy*ysq1;
	maxx+=sqsx*xsq2; maxy+=sqsy*ysq2;

	int ssx,ssy;
	ssx=numsqx+xsq1+xsq2;
	ssy=numsqy+ysq1+ysq2;

	int x,y;
	CPShGrInstList *nshadows=new CPShGrInstList[ssx*ssy];
	CPGEnElemList *nelems = new CPGEnElemList[ssx*ssy];


	CPShGrInstList *oldl,*newl;
	CPGEnElemList *oldeleml, *neweleml;

	oldl=shadows;
	newl=&nshadows[ xsq1 + ysq1*ssx ];

	oldeleml=elems;
	neweleml=&nelems[ xsq1 + ysq1*ssx ];

	int ady;

	ady = ssx-numsqx;

	for(y=0;y<numsqy;y++)
	{
		for(x=0;x<numsqx;x++)
		{
			newl->el=oldl->el;
			oldl->el=0;
			newl++; oldl++;

			neweleml->el=oldeleml->el;
			oldeleml->el=0;
			neweleml++; oldeleml++;
		}

		newl+=ady;
		neweleml+=ady;
	}

	numsqx=ssx;
	numsqy=ssy;
	
	delete[] shadows;
	delete[] elems;
	shadows=nshadows;
	elems=nelems;

	CPShGrInstList oldshadowsout; CPShGrInstListElem *lesh;
	CPGEnElemList oldelemsout; CPGEnElemListElem *leelem;

	numshadowsout=0; numelemsout=0;
	oldshadowsout.el=shadowsout.el;shadowsout.el=NULL;
	oldelemsout.el=elemsout.el;elemsout.el=NULL;

	lesh = oldshadowsout.el;
	while(lesh)
	{
		AddOutShadow(lesh->data);
		lesh->data->Release();
		lesh=lesh->next;
	}
	leelem = oldelemsout.el;
	while(leelem)
	{
		AddOutElem(leelem->data);
		leelem=leelem->next;
	}


	return 1;
}

void CShadowGrid::AddOutElem(CGEnElement *el)
{
	int sqx1,sqx2,sqy1,sqy2;
	int x,y;
	CPGEnElemList *elemrect;
	int ady;
	int rv;

	rv = CmpElemSquare(el,sqx1,sqy1,sqx2,sqy2);

	if(!rv)
	{
		elemsout.Add(el);
		numelemsout++;
	}

	elemrect = &elems[ sqx1 + sqy1 * numsqx ];
	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			elemrect->AddUnique(el);
			elemrect++;
		}
		elemrect+=ady;
	}
}

void CShadowGrid::AddOutShadow(CShadowGridInstance *shgi)
{
	int sqx1,sqx2,sqy1,sqy2;
	
	int x,y;

	CPShGrInstList *box;
	int ady;
	int rv;

	rv=CmpSQ(shgi->x1,shgi->y1,shgi->x2,shgi->y2,sqx1,sqy1,sqx2,sqy2);

	if(!rv){
		numshadowsout++;
		shgi->AddRef();
		shadowsout.Add(shgi);
	}

	box=&shadows[ sqx1 + sqy1 * numsqx ];

	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			if(box->AddUnique(shgi))
				shgi->AddRef();
			box++;
		}
		box+=ady;
	}
}


int CShadowGrid::CmpSQ(int x1, int y1, int x2, int y2, int &sqx1, int &sqy1, int &sqx2, int &sqy2)
{
	int dd;
	int FG=1;

	dd=x1-minx; if(dd>=0) sqx1=dd/sqsx; else sqx1=(dd+1)/sqsx-1;
	dd=y1-miny; if(dd>=0) sqy1=dd/sqsy; else sqy1=(dd+1)/sqsy-1;

	dd=x2-minx; if(dd>=0) sqx2=dd/sqsx+1; else sqx2=(dd+1)/sqsx;
	dd=y2-miny; if(dd>=0) sqy2=dd/sqsy+1; else sqy2=(dd+1)/sqsy;


#define SET_IN_FG(x,min,max) if(x<min){x=0;FG=0;}else if(x>max){x=max;FG=0;}


	SET_IN_FG(sqx1,0,numsqx);SET_IN_FG(sqx2,0,numsqx);
	SET_IN_FG(sqy1,0,numsqy);SET_IN_FG(sqy2,0,numsqy);

	return FG;
}


int CShadowGrid::AddElem(CGEnElement *el)
{
	int x1,x2,y1,y2;
	int sqx1,sqx2,sqy1,sqy2;
	
	int x,y;

	CPShGrInstList *box;
	CPGEnElemList *elemrect;
	int ady;
	int rv;

	rv = CmpElemSquare(el,sqx1,sqy1,sqx2,sqy2);

	if(!rv)
	{
		elemsout.Add(el);
		numelemsout++;
	}

	elemrect = &elems[ sqx1 + sqy1 * numsqx ];
	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			elemrect->Add(el);
			elemrect++;
		}
		elemrect+=ady;
	}

	CLightTex *shtex = el->LightTex;
	if(!shtex||!shtex->IsShadow()) return 1;

	int zx,zy;
	zshift->CmpShadowShift((int)(el->zz+el->ElSource()->pbz),zx,zy);
	x1 = (int)el->zx + shtex->shadow_dx + zx;
	y1 = (int)el->zy + shtex->shadow_dy + zy;
	x2 = x1 + shtex->shadow_sx - 1;
	y2 = y1 + shtex->shadow_sy - 1;

	rv=CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);

	CShadowGridInstance *shi=new CShadowGridInstance;

	shi->x1=x1;shi->y1=y1;shi->x2=x2;shi->y2=y2;
	shi->zmax=(int)(el->zz + el->ElSource()->zdz);
	shi->shadowtex=shtex;
	shi->shadow_element=el;

	if(!rv){
		numshadowsout++;
		shi->AddRef();
		shadowsout.Add(shi);
	}

	box=&shadows[ sqx1 + sqy1 * numsqx ];

	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			shi->AddRef();
			box->Add(shi);
			box++;
		}
		box+=ady;
	}

	///////////////////
	CPGEnElemListElem *leelem;

	elemrect = &elems[ sqx1 + sqy1 * numsqx ];
	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			leelem=elemrect->el;
			while(leelem)
			{
				if(!leelem->data->needrecalc && TestShadow(shi,leelem->data))
				{
					leelem->data->InvalidCache();
					leelem->data->SetNeedRecalc(dxrcTex);
				}
				leelem=leelem->next;
			}
			elemrect++;
		}
		elemrect+=ady;
	}

	leelem = elemsout.el;
	while(leelem)
	{
		if(!leelem->data->needrecalc && TestShadow(shi,leelem->data))
		{
			leelem->data->InvalidCache();
			leelem->data->SetNeedRecalc(dxrcTex);
		}		
		leelem=leelem->next;
	}

	//////////////

	shi->Release(); //zrusim 1 kopii - vzdy, kdyz jsem nekam shi daval jsem zvysil pocet referenci


	if(numshadowsout>maxshadowsout||numelemsout>maxelemsout)
	{
		Resize();
		rv=CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);
	}


	return 1;
}

int CShadowGrid::DeleteElem(CGEnElement *el)
{
	int x1,x2,y1,y2;
	int sqx1,sqx2,sqy1,sqy2;
	
	int x,y;

	CPShGrInstList *box;
	CPGEnElemList *elemrect;
	int ady;
	int rv;


	rv = CmpElemSquare(el,sqx1,sqy1,sqx2,sqy2);

	if(!rv)
	{
		if(elemsout.Delete(el)) numelemsout--;
	}

	elemrect = &elems[ sqx1 + sqy1 * numsqx ];
	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			elemrect->Delete(el);
			elemrect++;
		}
		elemrect+=ady;
	}

	///////////////////////

	CShadowGridInstance *shi=NULL;

	CLightTex *shtex = el->LightTex;
	if(!shtex||!shtex->IsShadow()) return 1;

	int zx,zy;
	zshift->CmpShadowShift((int)(el->zz+el->ElSource()->pbz),zx,zy);
	x1 = (int)el->zx + shtex->shadow_dx + zx;
	y1 = (int)el->zy + shtex->shadow_dy + zy;
	x2 = x1 + shtex->shadow_sx - 1;
	y2 = y1 + shtex->shadow_sy - 1;

	rv=CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);

	CPShGrInstListElem **le;
	CPShGrInstListElem *tm;

	if(!rv){
		le=&shadowsout.el;
		while(*le)
		{
			if((*le)->data->shadow_element == el)
			{
				tm=*le;
				shi=tm->data;
				//tm->data->Release(); //zrusim pozdeji pomoci shi
				*le=tm->next;
				delete tm;
				numshadowsout--;
				break;
			}
			le=&(*le)->next;
		}
	}

	box=&shadows[ sqx1 + sqy1 * numsqx ];

	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			le=&box->el;
			while(*le)
			{
				if((*le)->data->shadow_element == el)
				{
					tm=*le;
					if(!shi) shi=tm->data; 
					else
						tm->data->Release(); //pokud jsem zatim nemel shi - tak nerusim, zrusim pommoci shi (u 1 elementu je shi vzdy stejny)
					*le=tm->next;
					tm->next=NULL;
					delete tm;
					break;
				}
				le=&(*le)->next;
			}
			box++;
		}
		box+=ady;
	}

	///////////////////
	CPGEnElemListElem *leelem;

	if(shi){

	elemrect = &elems[ sqx1 + sqy1 * numsqx ];
	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			leelem=elemrect->el;
			while(leelem)
			{
				if(!leelem->data->needrecalc && TestShadow(shi,leelem->data))
				{
					leelem->data->InvalidCache();
					leelem->data->SetNeedRecalc(dxrcTex);
				}
				leelem=leelem->next;
			}
			elemrect++;
		}
		elemrect+=ady;
	}

    leelem = elemsout.el;
	while(leelem)
	{
		if(!leelem->data->needrecalc && TestShadow(shi,leelem->data))
		{
			leelem->data->InvalidCache();
			leelem->data->SetNeedRecalc(dxrcTex);
		}		
		leelem=leelem->next;
	}

	shi->Release();
	
	}

	return 1;
}

void CShadowGrid::CmpElemBRect(CGEnElement *el, int &xx1, int &yy1, int &xx2, int &yy2)
{
	int x1,y1,z1,x2,y2,z2;
	int zx,zy;

    x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

    zshift->CmpShadowShift(z1,zx,zy);
	x1+=zx;x2+=zx;y1+=zy;y2+=zy;

	if(z1!=z2){
		zshift->CmpShadowShift(z2-z1,zx,zy);
		if(zx<0) {
			xx1=x1+zx; xx2=x2;
		}else{
			xx1=x1; xx2=x2+zx;
		}
		if(zy<0) {
			yy1=y1+zy; yy2=y2;
		}else{
			yy1=y1; yy2=y2+zy;
		}

	}else
	{
		xx1=x1;yy1=y1;xx2=x2;yy2=y2;
	}

}

void CShadowGrid::CmpElemBRect2(CGEnElement *el, int &xx1, int &yy1, int &xx2, int &yy2, int &bx1, int &by1, int &bx2, int &by2)
{
	int x1,y1,z1,x2,y2,z2;
	int zx,zy;

    x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

    zshift->CmpShadowShift(z1,zx,zy);
	x1+=zx;x2+=zx;y1+=zy;y2+=zy;
	bx1=x1;bx2=x2;by1=y1;by2=y2;

	if(z1!=z2){
		zshift->CmpShadowShift(z2-z1,zx,zy);
		if(zx<0) {
			xx1=x1+zx; xx2=x2;
		}else{
			xx1=x1; xx2=x2+zx;
		}
		if(zy<0) {
			yy1=y1+zy; yy2=y2;
		}else{
			yy1=y1; yy2=y2+zy;
		}

	}else
	{
		xx1=x1;yy1=y1;xx2=x2;yy2=y2;
	}

}

int CShadowGrid::CmpElemSquare(CGEnElement *el, int &sqx1, int &sqy1, int &sqx2, int &sqy2)
{
	int x1,y1,x2,y2;
	CmpElemBRect(el,x1,y1,x2,y2);
	return CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);
}

void CShadowGrid::GetShadows(int maxshadows, int &numshadows, CShadowGridInstance **shadowelems, CGEnElement *el)
{

	int sqx1,sqy1,sqx2,sqy2;
	
	int x,y;

	CPShGrInstList *box;
	int ady;
	int rv;

	CPShGrInstListElem *shel;
	CShadowGridInstance *shgi;

	numshadows = 0;

	int x1,x2,y1,y2,z1,z2;
	int bx1,bx2,by1,by2;
	int zx,zy,dx,dy;

    x1 = (int)(el->zx + el->ElSource()->minx);
	x2 = (int)(el->zx + el->ElSource()->maxx)-1;
	y1 = (int)(el->zy + el->ElSource()->miny);
	y2 = (int)(el->zy + el->ElSource()->maxy)-1;
	z1 = (int)(el->zz);
	z2 = (int)(el->zz + el->ElSource()->zdz)-1;
	if(z2<z1) z2=z1;

    zshift->CmpShadowShift(z1,zx,zy);
	x1+=zx;x2+=zx;y1+=zy;y2+=zy;
	bx1=x1;bx2=x2;by1=y1;by2=y2;

	dx=(int)el->zx+zx;dy=(int)el->zy+zy;

	if(z1!=z2){
		zshift->CmpShadowShift(z2-z1,zx,zy);
		if(zx<0) {
			x1=x1+zx; x2=x2;
		}else{
			x1=x1; x2=x2+zx;
		}
		if(zy<0) {
			y1=y1+zy; y2=y2;
		}else{
			y1=y1; y2=y2+zy;
		}

	}

	rv = CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);

	box=&shadows[ sqx1 + sqy1 * numsqx ];

	ady= numsqx - (sqx2-sqx1) ;

	for(y=sqy1;y<sqy2;y++)
	{
		for(x=sqx1;x<sqx2;x++)
		{
			shel = box->el;
			while(shel)
			{
				shgi=shel->data;
				if(!shgi->tmtag&&numshadows<maxshadows)
				{
					if(TestShadow(shgi,el,x1,y1,x2,y2,bx1,by1,bx2,by2))
					{
						shgi->tmtag=1;
						shgi->dx = shgi->x1 - dx;
						shgi->dy = shgi->y1 - dy;

						shadowelems[numshadows]=shgi;

						numshadows++;

					}
				}
				shel=shel->next;				
			}
			box++;
		}
		box+=ady;
	}

	shel = shadowsout.el;
	while(numshadows<maxshadows && shel)
	{
		shgi=shel->data;
		if(!shgi->tmtag&& TestShadow(shgi,el,x1,y1,x2,y2,bx1,by1,bx2,by2))
		{
			shgi->tmtag=1;
			shgi->dx = shgi->x1 - dx;
			shgi->dy = shgi->y1 - dy;

			shadowelems[numshadows]=shgi;

			numshadows++;

		}
		shel=shel->next;				

	}

	for(x=0;x<numshadows;x++)
	{
		shadowelems[x]->tmtag=0;
	}



	//setridit stiny - podle textury, pak podle dx, pak dy

	CShadowGridInstance *tmp;
	int c;

	for(x=1;x<numshadows;x++) //insertsort
	{
		for(y=x-1;y>=0;y--)
		{
			c = shadowelems[y+1]->Cmp(shadowelems[y]); //porovnam - vraci 2, kdyz y > (y+1)
			if(c!=2) break;

			tmp = shadowelems[y+1]; //prohodim je
			shadowelems[y+1] = shadowelems[y];
			shadowelems[y] = tmp;

		}
	}

}

int CShadowGrid::TestShadow(CShadowGridInstance *shgi, CGEnElement *el,int x1,int y1,int x2,int y2,int bx1,int by1,int bx2,int by2)
{
	CGEnElement *shel=shgi->shadow_element;
	CGEnElSource *els;

	if(shel==el) return 0; //nehazim stin sam na sebe

	if( shgi->x1 > x2 || shgi->x2 < x1 || shgi->y1 > y2 || shgi->y2 < y1 ) return 0; //stin se neprotina s prumetem elementu

	if( (el->zz + el->ElSource()->zdz)<=shel->zz+shel->ElSource()->pbz ) return 1; //predmet vhajici stin je nad elementem
	if( (int)(el->zz + el->ElSource()->pbz)>=shgi->zmax ) return 0; //predmet vrhajici stin je pod elementem

	//if( shgi->x1 > bx2 || shgi->x2 < bx1 || shgi->y1 > by2 || shgi->y2 < by1 ) return 0; //neprotina se zakladna se stinem

	float ex1,ex2,ey1,ey2;

	els = el->ElSource();
	ex1=el->zx+els->minx;
	ex2=el->zx+els->maxx;
	ey1=el->zy+els->miny;
	ey2=el->zy+els->maxy;

	if( shgi->x1 > ex2 || 
		shgi->x2 < ex1 || 
		shgi->y1 > ey2 || 
		shgi->y2 < ey1 
		) return 0; //neprotina se zakladna se stinem


	//porovnam jestli se protinaji zakladny - pokud ne vrhnu stin
	int t;
	els = shel->ElSource();
	t = (int)(shel->zx + els->minx);
	if(t>=ex2) return 1;
	t = (int)(shel->zx + els->maxx)-1;
	if(t<=ex1) return 1;
	t = (int)(shel->zy + els->miny);
	if(t>=ey2) return 1;
	t = (int)(shel->zy + els->maxy)-1;
	if(t<=ey1) return 1;

	//zakladny se protinaji - nebudu vrhat stin

	return 0;

}

int CShadowGrid::TestShadow(CShadowGridInstance *shgi, CGEnElement *el)
{
	int x1,y1,x2,y2,bx1,bx2,by1,by2;

	CmpElemBRect2(el,x1,y1,x2,y2,bx1,by1,bx2,by2);

	return TestShadow(shgi,el,x1,y1,x2,y2,bx1,by1,bx2,by2);
}
