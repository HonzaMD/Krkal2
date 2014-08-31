////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lightcache.cpp
//
// Cache na nasviceny textury
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "lightcache.h"
#include "dx.h"

#include "normalizer.h"


CLightCache::CLightCache(int sz)
{
	size=sz; //velikost hash tabulky nasvicenych textur
	table=new CLightCacheList[size]; //ht nasvicenych textur
	lru=new CLightCacheLRU; //LRU seznam nasvicenych textur

	lghash=new CLightGroupHash(64);  //ht skupiny svetel
	sghash=new CShadowGroupHash(128); //ht skupiny vrzenych stinu
	
	tmplightarraysize=20; //max pocet svetel sviticich na element
	tmplightarray=new CLight*[tmplightarraysize]; //pomocne pole svetel sviticich na element
	
	tmpshadowarraysize=20; //max pocet stinu vrzenych na element
	tmpshadowarray=new CShadowGridInstance*[tmpshadowarraysize]; //pomocne pole stinu vrzenych na element
	
/*	lightmap=new CLightMap(0,0,1000,1000,40,40); //mapa svetel
	elemgrid=new CElemGrid(0,0,0,1000,1000,80,40,40,40); //mapa elementu
	shadowgrid=new CShadowGrid(0,0,1000,1000,40,40); //mapa stinu
*/
	lightmap=new CLightMap(-20,-20,1000-20,1000-20,40,40); //mapa svetel
	elemgrid=new CElemGrid(-20,-20,0,1000-20,1000-20,40,40,40,40); //mapa elementu
	shadowgrid=new CShadowGrid(-20,-20,1000-20,1000-20,40,40); //mapa stinu

	toplight=NULL; //svetlo shora
}

CLightCache::~CLightCache()
{
	SAFE_DELETE(lightmap);
	SAFE_DELETE_ARRAY(table);
	SAFE_DELETE(lru);
	SAFE_DELETE(sghash);
	SAFE_DELETE(lghash);
	SAFE_DELETE(elemgrid);
	SAFE_DELETE(shadowgrid);
	SAFE_DELETE_ARRAY(tmplightarray);
	SAFE_DELETE_ARRAY(tmpshadowarray);
}



int CLightCache::GetTex(CGEnElement *el, CTexture **outtex)
{
	//el je element, ktery mam nasvitit
	//v outtex vratim nasvicenou texturu

	CLightTex *lt=el->LightTex; //zjistim svetelnou texturu elemntu
	if(!lt) return 0; //neni texture - error

	int numlight=0;

	lightmap->GetLights(el, tmplightarraysize,numlight,tmplightarray); //zjistim svetla, ktery svitej na element
	//pole svetel je jiz setridene; vzdy stejne pro stejne rozmistena svetla (dulezite!)

	int numshadows=0;
	shadowgrid->GetShadows(tmpshadowarraysize, numshadows, tmpshadowarray, el); //zjistim stiny, ktery jsou vrzeny na element
	//pole stinu je jiz setridene; vzdy stejne pro stejne rozmistene stiny (dulezite!)
    
	CLightGroupElement *lge; int flge;
	CShadowGroupElement *sge; int fsge;

	flge = lghash->FindLightGroup(numlight,tmplightarray,&lge,toplight); //najdu v ht polozku odpovidajici skupine svetel
	//(pokud polozka neexistovala, tak se vytvori nova)
	// flge = 0: vytvorila se nove
	// flge = 1: nasla se v ht
	fsge = sghash->FindShadowGroup(numshadows,tmpshadowarray,&sge); //najdu v ht polozku odpovidajici skupine stinu
	//(pokud polozka neexistovala, tak se vytvori nova)
	// fsge = 0: vytvorila se nove
	// fsge = 1: nasla se v ht

	int f[3]; //do f ulozim pozici 1. svetla
	if(numlight)
	{
		f[0]=tmplightarray[0]->px;
		f[1]=tmplightarray[0]->py;
		f[2]=tmplightarray[0]->pz;
	}

	int hashval=CmpHash(el,lge,sge,f); //spocitam hashovaci funkci

	if(Find(hashval,el,lge,sge,f,outtex)) return 1; //podivam se jestli nemam nasvicenou texturu 

	//nemam - musim ji spocitat

	if(flge==1) //jiz existoval - zvetsim # referenci (kdyz neexistoval, tak pocet ref je uz 1)
		lge->AddRef();

	if(fsge==1) //jiz existoval - zvetsim # referenci (kdyz neexistoval, tak pocet ref je uz 1)
		sge->AddRef();

	CTexture *tex=new CTexture; //vytvorim novou texturu
	tex->Create(lt->GetSX(),lt->GetSY(),0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED);
	
	//spocitam nasvicenou texturu
	lt->CmpLight(tex,numlight,tmplightarray,toplight,el,numshadows,sge->shadows);

	numcmptex++; 

	*outtex=tex; 

	Add(hashval,el,lge,sge,f,tex); //pridam do hash tabulky a do LRU seznamu

	DebugMessage(10,"%i",lru->GetNumEl());

	return 1;
}


int CLightCache::Add(int hashval, CGEnElement *el, CLightGroupElement *lge, CShadowGroupElement *sge, int *pos, CTexture *tex)
{
	CLightCacheElem *cel=new CLightCacheElem;
	
	cel->id.Create(el,lge,sge,pos); //jednoznacny ID nasvicene textury
	cel->tex=tex; //nasvicena textura

	table[hashval].AddFirst(cel); //pridam do hash tabulky
	lru->AddFirst(cel); //pridam do LRU seznamu

	if(lru->GetNumEl()>lru->maxel) //pokud jiz mam moc nasvicenych textur, tak zahodim nejstarsi
	{
		cel=lru->GetLast(); //nejstarsi
		if(cel){
			cel->RemoveHash(); //vyhodim z ht
			lru->Remove(cel); //vyhodim z LRU
			delete cel; //smazu nasvicenou texturu
		}
	}

	return 1;
}

int CLightCache::Find(int hashval, CGEnElement *el, CLightGroupElement *lge, CShadowGroupElement *sge, int *pos, CTexture **tex)
{

	CLightCacheElem *cel=table[hashval].GetFirst(); //prvni nasvicena textura se hash fci = hashval

	if(!cel) return 0; //zadna neni

	while(cel) //prochazim nasviceny textury se stejnou hash fci
	{
		if(cel->id.Compare(el,lge,sge,pos)) //je to hledana nasvicena textura?
		{//ano
			*tex=cel->tex; //vratim nasvicenou texturu

			lru->Remove(cel); //presunu texturu v LRU seznamu na zacatek
			lru->AddFirst(cel);

			return 1;
		}

		cel=cel->GetHashNext();
	}


	return 0; //nenasel jsem nasvicenou texturu
}

int CLightCache::AddLight(CLight *l)
{

	int x1,y1,z1,x2,y2,z2;

	l->GetBBox(x1,y1,z1,x2,y2,z2);
	
	elemgrid->InvalidElems(x1,y1,z1,x2,y2,z2);

	return lightmap->AddLight(l);
}

int CLightCache::DeleteLight(CLight *l)
{

	int x1,y1,z1,x2,y2,z2;

	l->GetBBox(x1,y1,z1,x2,y2,z2);
	
	elemgrid->InvalidElems(x1,y1,z1,x2,y2,z2);

	return lightmap->DeleteLight(l);
}

int CLightCache::SetTopLight(CTopLight *light)
{
	toplight=light;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

CLightGroupElement::CLightGroupElement(int _numlights, CLight **_lights, CTopLight *toplight)
{
	prev=next=NULL;

	numlights=_numlights;

	if(numlights==0) lights=NULL;
	else
	{
		int f[3];
		f[0]=_lights[0]->px;
		f[1]=_lights[0]->py;
		f[2]=_lights[0]->pz;

		lights=new CLight[numlights];

		for(int i=0;i<numlights; i++)
		{
			lights[i] =  *_lights[i]; //copy lights
				lights[i].px-=f[0];lights[i].py-=f[1];lights[i].pz-=f[2];
		}
	}
	if(toplight)
		toplightID = toplight->id;
	else
		toplight=0;
}

CLightGroupElement::~CLightGroupElement()
{
	RemoveFromList();

	SAFE_DELETE_ARRAY(lights);
}

int CLightGroupElement::RemoveFromList()
{
	if(prev&&prev!=this) //not head
	{
		prev->next=next;
		next->prev=prev;

		prev=next=NULL;
		return 1;
	}
	return 0;
}

void CLightGroupElement::Add(CLightGroupElement *el)
{
	assert(next);
	el->prev=this;
	el->next=next;
	next->prev=el;
	next=el;
}

int CLightGroupElement::Compare(int _numlights, CLight **_lights, UI _toplightID)
{

	int f[3];

	if(numlights!=_numlights) return 0;
	if(toplightID!=_toplightID) return 0;

	if(numlights)
	{
		f[0]=_lights[0]->px;
		f[1]=_lights[0]->py;
		f[2]=_lights[0]->pz;

		for(int i=0;i<numlights;i++)
		{
			if(lights[i].Compare(*_lights[i],f)!=0) return 0;
		}
	}

	return 1;
}


CLightGroupHash::CLightGroupHash(int sz)
{
	size=sz;
	table = new CLightGroupElement*[sz];

	for(int i=0;i<size;i++)
	{
		table[i]= new CLightGroupElement();
	}

}

CLightGroupHash::~CLightGroupHash()
{
	CLightGroupElement* el;
	for(int i=0;i<size;i++)
	{
		while((el=table[i]->GetNext()))
		{
			delete el;
		}

		SAFE_DELETE(table[i]);
	}
	SAFE_DELETE_ARRAY(table);
}

int CLightGroupHash::FindLightGroup(int numlights, CLight **lights, CLightGroupElement** lge, CTopLight *toplight)
{

	int hv=CmpHash(numlights,lights);

	CLightGroupElement* el=table[hv]->GetNext();

	UI toplightID;
	if(toplight)
		toplightID=toplight->id;
	else
		toplightID=0;

	while(el)
	{
		if(el->Compare(numlights,lights,toplightID)) {
			*lge=el;
			return 1;
		}
		el=el->GetNext();
	}

	//not found create new

	el=new CLightGroupElement(numlights,lights, toplight);

	table[hv]->Add(el);

	*lge=el;

	return 2;
}


int CLightGroupHash::CmpHash(int numlights, CLight **lights)
{
	unsigned int hv;

    hv=numlights;
	CLight *l;

	if(numlights)
	{
		int fx,fy,fz;

		fx=lights[0]->px;fy=lights[0]->py;fz=lights[0]->pz;

		hv^=lights[0]->radius;
		hv^=lights[0]->ir;hv^=lights[0]->ig;hv^=lights[0]->ib;
				
		for(int i=1;i<numlights;i++)
		{
			l=lights[i];

			hv^=l->px-fx;
			hv^=l->py-fy;
			hv^=l->pz-fz;

			hv^=l->radius;
			hv^=l->ir;hv^=l->ig;hv^=l->ib;
		}
	}

	hv&=(size-1);

	return hv;
}


////////////////////////////////////////////////////////////////////////////////////////////////


CShadowGroupElement::CShadowGroupElement(int _numshadows, CShadowGridInstance **_shadows)
{
	prev=next=NULL;

	numshadows=_numshadows;

	if(numshadows==0) shadows=NULL;
	else
	{
		shadows = new CShadowInstance[numshadows];
		for(int i=0;i<numshadows;i++)
		{
			shadows[i].shadowtex = _shadows[i]->shadowtex;
			shadows[i].dx = _shadows[i]->dx;
			shadows[i].dy = _shadows[i]->dy;
			
		}
	}

}

CShadowGroupElement::~CShadowGroupElement()
{
	RemoveFromList();
	SAFE_DELETE_ARRAY(shadows);
}

int CShadowGroupElement::RemoveFromList()
{
	if(prev&&prev!=this) //not head
	{
		prev->next=next;
		next->prev=prev;

		prev=next=NULL;
		return 1;
	}
	return 0;
}

void CShadowGroupElement::Add(CShadowGroupElement *el)
{
	assert(next);
	el->prev=this;
	el->next=next;
	next->prev=el;
	next=el;
}

int CShadowGroupElement::Compare(int _numshadows, CShadowGridInstance **_shadows)
{


	if(numshadows!=_numshadows) return 0;

	if(numshadows)
	{
		for(int i=0;i<numshadows;i++)
		{
			if(_shadows[i]->Cmp(shadows[i])!=0) return 0;
		}
	}

	return 1;
}


CShadowGroupHash::CShadowGroupHash(int sz)
{
	size=sz;
	table = new CShadowGroupElement*[sz];

	for(int i=0;i<size;i++)
	{
		table[i]= new CShadowGroupElement();
	}

}

CShadowGroupHash::~CShadowGroupHash()
{
	CShadowGroupElement* el;
	for(int i=0;i<size;i++)
	{
		while((el=table[i]->GetNext()))
		{
			delete el;
		}

		SAFE_DELETE(table[i]);
	}
	SAFE_DELETE_ARRAY(table);
}

int CShadowGroupHash::FindShadowGroup(int numshadows, CShadowGridInstance **shadows, CShadowGroupElement** sge )
{

	int hv=CmpHash(numshadows,shadows);

	CShadowGroupElement* el=table[hv]->GetNext();

	while(el)
	{
		if(el->Compare(numshadows,shadows)) {
			*sge=el;
			return 1;
		}
		el=el->GetNext();
	}

	//not found create new

	el=new CShadowGroupElement(numshadows,shadows);

	table[hv]->Add(el);

	*sge=el;

	return 2;
}


int CShadowGroupHash::CmpHash(int numshadows, CShadowGridInstance **shadows)
{
	unsigned int hv;

    hv=numshadows;

	for(int i=0;i<numshadows;i++)
	{
		hv^=(int)shadows[i]->shadowtex;
		hv^=shadows[i]->dx;hv^=shadows[i]->dy;
	}

	hv&=(size-1);

	return hv;
}

////////////////////////////////////////////////////////////////////////////////////////////////

CLightMap::CLightMap(int _minx,int _miny,int _maxx,int _maxy, int _sqsx, int _sqsy)
{
	sqsx=_sqsx;sqsy=_sqsy;

	minx=_minx;miny=_miny;

    numsqx=(_maxx-_minx)/sqsx + 1;
	numsqy=(_maxy-_miny)/sqsy + 1;

	maxx=minx + numsqx*sqsx;
	maxy=miny + numsqy*sqsy;

	map=new CPLightList[numsqx*numsqy];
}

CLightMap::~CLightMap()
{
	SAFE_DELETE_ARRAY(map);
}

int CLightMap::AddLight(CLight *light)
{
	int x1,x2,y1,y2,z1,z2;
	int sqx1,sqx2,sqy1,sqy2;
	
	CPLightList *pl;
	int ady;
	int rv;

	light->GetBBox(x1,y1,z1,x2,y2,z2);
	
	if(x1<minx || y1<miny || x2>=maxx || y2>=maxy)
	{
		Resize(x1,y1,x2,y2);
	}

	rv=CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);
	assert(rv);

	pl=&map[sqx1 + sqy1 * numsqx];
	ady=numsqx - (sqx2-sqx1);

	int xx,yy,xxs;

	yy=sqy1*sqsy+miny;
	xxs=sqx1*sqsx+minx;
	for(int y=sqy1;y<sqy2;y++)
	{
		xx=xxs;
		for(int x=sqx1;x<sqx2;x++)
		{
			if(light->TestLight(xx,yy,z1,xx+sqsx,yy+sqsy,z2))
				pl->Add(light);

			pl++;
			xx+=sqsx;
		}
		pl+=ady;
		yy+=sqsy;
	}
	

	return 1;
}

int CLightMap::DeleteLight(CLight *light)
{
	int x1,x2,y1,y2,z1,z2;
	int sqx1,sqx2,sqy1,sqy2;
	
	CPLightList *pl;
	int ady;
	int rv;

	light->GetBBox(x1,y1,z1,x2,y2,z2);

	rv=CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);
	assert(rv);

	pl=&map[sqx1 + sqy1 * numsqx];
	ady=numsqx - (sqx2-sqx1);

	for(int y=sqy1;y<sqy2;y++)
	{
		for(int x=sqx1;x<sqx2;x++)
		{
			pl->Delete(light);
			pl++;
		}
		pl+=ady;
	}

	return 1;
}

int CLightMap::GetLights(CGEnElement *el, int maxlights, int &numlights, CLight **lights)
{
	int sqx1,sqx2,sqy1,sqy2;
	CPLightList *pl;
	CPLightListIter it;
	int ady;

	int x1,x2,y1,y2,z1,z2;

	x1=(int)(el->zx + el->ElSource()->minx);
	y1=(int)(el->zy + el->ElSource()->miny);
	x2=(int)(el->zx + el->ElSource()->maxx);
	y2=(int)(el->zy + el->ElSource()->maxy);
	z1=(int)(el->zz);
	z2=(int)(el->zz + el->ElSource()->zdz);

	CmpSQ(x1,y1,x2,y2,sqx1,sqy1,sqx2,sqy2);

	int pltex=0;
	CLightTex *tex=el->GetLightTex();
	int c=tex->GetLightComponents();
	if((c&(LgC_NORMAL|LgC_ZDEPTH))==0)
		pltex=1;

	numlights=0;

	pl=&map[sqx1 + sqy1 * numsqx];
	ady=numsqx - (sqx2-sqx1);

	for(int y=sqy1;y<sqy2;y++)
	{
		for(int x=sqx1;x<sqx2;x++)
		{
			it.SetList(pl);
			while( !it.End() )
			{
				if(! it->tmtag )
				{

					if((pltex?it->TestLightPlane(x1,y1,x2,y2,z1):it->TestLight(x1,y1,z1,x2,y2,z2)) && numlights<maxlights)
					{
						it->tmtag = 1;
						lights[numlights]=it;
						numlights++;
					}
				}
				it++;
			}
			pl++;
		}
		pl+=ady;
	}

	// tmtag se shodi v SortLights (stejne se tam prochazi vsechny svetla)
	// for(int l=0;l<numlights;l++) lights[l]->tmtag=0;

	SortLights(numlights,lights);

	return 1;
}


void CLightMap::SortLights(int numlight, CLight **lights)
{
	int nl=0,k,l;
	CLight *tm;

	if(!numlight) return;
	
	lights[0]->tmtag=0;
	for(l=1;l<numlight;l++)
	{
		lights[l]->tmtag=0;
		for(k=l-1;k>=0;k--)
		{
			if(!lights[k]) {
				lights[k]=lights[k+1];
				lights[k+1]=NULL;
			}else
			if(lights[k+1]->Compare(*lights[k])==-1){ //svetlo 'k+1' je mensi nez 'k'
				tm=lights[k+1];  //vymenim
				lights[k+1]=lights[k];
				lights[k]=tm;
			}else
				break;
		}
	}

	numlight=nl;
}


int CLightMap::CmpSQ(int x1, int y1, int x2, int y2, int &sqx1, int &sqy1, int &sqx2, int &sqy2)
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

int CLightMap::Resize(int x1,int y1,int x2,int y2)
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

	CPLightList *nmap=new CPLightList[ssx*ssy];

	int x,y;

	CPLightList *oldl,*newl;

	oldl=map;
	newl=&nmap[ xsq1 + ysq1*ssx ];

	for(y=0;y<numsqy;y++)
	{
		for(x=0;x<numsqx;x++)
		{
			newl->el=oldl->el;
			oldl->el=0;

			newl++;
			oldl++;
		}

		newl+=ssx-numsqx;
	}


	numsqx=ssx;
	numsqy=ssy;
	
	delete[] map;
	map=nmap;

	return 1;
}