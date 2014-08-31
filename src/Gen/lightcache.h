////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lightcache.h
//
// Cache na nasviceny textury
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIGHTCACHE_H
#define LIGHTCACHE_H

#include "types.h"

#include "genmain.h"
#include "lights.h"
#include "elemgrid.h"

class CLightGroupElement: public CRefCnt{
public:

	CLightGroupElement(){prev=next=this; numlights=-1; lights=NULL;} //head constr.
	CLightGroupElement(int numlights, CLight **lights, CTopLight *toplight);

	~CLightGroupElement();

	int RemoveFromList();

	int IsHead(){return (numlights==-1);}

	CLightGroupElement* GetNext(){if(next&&!next->IsHead()) return next; else return NULL;}

	int Compare(int numlights, CLight **lights, UI toplightID); 

	void Add(CLightGroupElement *el);

	int IsPosDep(){if(numlights)return 1; else return 0;}

protected:

	int numlights; // = -1 : head of list
	CLight *lights;
	UI toplightID;

	CLightGroupElement *prev,*next;

};

class CLightGroupHash{
public:
	
	CLightGroupHash(int sz);
	~CLightGroupHash();

	int FindLightGroup(int numlights, CLight **lights, CLightGroupElement **lge, CTopLight *toplight);

protected:

	int CmpHash(int numlights, CLight **lights);

	CLightGroupElement **table;
	int size;

};


class CShadowGroupElement: public CRefCnt{
friend class CLightCache;
public:

	CShadowGroupElement(){prev=next=this; numshadows=-1; shadows=NULL;} //head constr.
	CShadowGroupElement(int numshadows, CShadowGridInstance **shadows);

	~CShadowGroupElement();

	int RemoveFromList();

	int IsHead(){return (numshadows==-1);}

	CShadowGroupElement* GetNext(){if(next&&!next->IsHead()) return next; else return NULL;}

	int Compare(int numshadows, CShadowGridInstance **shadows); 

	void Add(CShadowGroupElement *el);

protected:

	int numshadows; // = -1: head of list
	CShadowInstance *shadows;

	CShadowGroupElement *prev,*next;

};

class CShadowGroupHash{
public:
	
	CShadowGroupHash(int sz);
	~CShadowGroupHash();

	int FindShadowGroup(int numshadows, CShadowGridInstance **shadows, CShadowGroupElement **sge);

protected:

	int CmpHash(int numshadows, CShadowGridInstance **shadows);

	CShadowGroupElement **table;
	int size;

};


struct CLightID{
	CLightID(){lge=NULL;sge=NULL;texID=0;}
	~CLightID(){SAFE_RELEASE(lge);SAFE_RELEASE(sge);}

	void Create(CGEnElement *el, CLightGroupElement *_lge, CShadowGroupElement *_sge, int *pos){

		texID = el->LightTex->GetTexID();
	
		x=el->zx-pos[0];
		y=el->zy-pos[1];
		z=el->zz-pos[2];

		lge=_lge;
		sge=_sge;
			
	};

	#define ID_LIGHT_SIZE 16 //3*4 pozice + 3 intenzita + 1 radius

	int Compare(CGEnElement *el, CLightGroupElement *_lge, CShadowGroupElement *_sge, int *pos){

		if(texID != el->LightTex->GetTexID() ) 
			return 0;

		if(lge!=_lge) 
			return 0;
		if(sge!=_sge) 
			return 0;

		if( lge->IsPosDep() )
		{
			if(x!=el->zx-pos[0] ||y!=el->zy-pos[1] ||z!=el->zz-pos[2]) 
				return 0;
		}

		return 1;
	}

	float x,y,z; //pozice
	//CLightTex *tex; //textura
	UI texID; //ID textury

	CLightGroupElement *lge;
	CShadowGroupElement *sge;
};


struct CLightCacheElem{
	friend struct CLightCacheList;
	friend struct CLightCacheLRU;
	CLightCacheElem(){
		tex=NULL;
		hash_next=NULL;hash_prev=NULL;
		lru_next=NULL;lru_prev=NULL;
	}
	~CLightCacheElem(){
		SAFE_RELEASE(tex);
	}

	CLightID id;
	CTexture *tex;

	CLightCacheElem *GetHashNext(){if(hash_next->tex) return hash_next; else return NULL;}
	CLightCacheElem *GetHashPrev(){if(hash_prev->tex) return hash_prev; else return NULL;}

	CLightCacheElem *GetLRUNext(){if(lru_next->tex) return lru_next; else return NULL;}
	CLightCacheElem *GetLRUPrev(){if(lru_prev->tex) return lru_prev; else return NULL;}

	int RemoveHash(){
		if(!hash_next||!hash_prev) return 0;
		hash_prev->hash_next=hash_next;
		hash_next->hash_prev=hash_prev;
		hash_next=NULL;hash_prev=NULL;
		return 1;
	}

	int RemoveLRU(){
		if(!lru_next||!lru_prev) return 0;
		lru_prev->lru_next=lru_next;
		lru_next->lru_prev=lru_prev;
		lru_next=NULL;lru_prev=NULL;
		return 1;
	}


protected:
	CLightCacheElem *hash_next;
	CLightCacheElem *hash_prev;

	CLightCacheElem *lru_next;
	CLightCacheElem *lru_prev;
};

struct CLightCacheList{
	CLightCacheList(){head=new CLightCacheElem();head->hash_next=head;head->hash_prev=head;}
	~CLightCacheList(){SAFE_DELETE(head);}

	CLightCacheElem* GetFirst(){if(head->hash_next!=head) return head->hash_next; else return NULL;}
	CLightCacheElem* GetLast(){if(head->hash_prev!=head) return head->hash_prev; else return NULL;}

	int AddFirst(CLightCacheElem *el)
	{
		if(!el||!el->tex) return 0;
		el->hash_prev=head;
		el->hash_next=head->hash_next;
		head->hash_next->hash_prev=el;
		head->hash_next=el;
		return 1;
	}
	int AddLast(CLightCacheElem *el)
	{
		if(!el||!el->tex) return 0;
		el->hash_next=head;
		el->hash_prev=head->hash_prev;
		head->hash_prev->hash_next=el;
		head->hash_prev=el;
		return 1;
	}
	void Remove(CLightCacheElem *el)
	{
		if(!el) return;
		el->RemoveHash();
	}
	void Delete(CLightCacheElem *el)
	{
		if(!el) return;
		Remove(el);delete(el);
	}

protected:
	CLightCacheElem *head;

};

struct CLightCacheLRU{
	CLightCacheLRU(int max_elems=1000){head=new CLightCacheElem();head->lru_next=head;head->lru_prev=head;numel=0;maxel=max_elems;}
	~CLightCacheLRU(){DeleteAll(); SAFE_DELETE(head);}

	CLightCacheElem* GetFirst(){if(head->lru_next!=head) return head->lru_next; else return NULL;}
	CLightCacheElem* GetLast(){if(head->lru_prev!=head) return head->lru_prev; else return NULL;}

	int GetNumEl(){return numel;}

	int AddFirst(CLightCacheElem *el)
	{
		if(!el||!el->tex) return 0;
		el->lru_prev=head;
		el->lru_next=head->lru_next;
		head->lru_next->lru_prev=el;
		head->lru_next=el;
		numel++;
		return 1;
	}
	int AddLast(CLightCacheElem *el)
	{
		if(!el||!el->tex) return 0;
		el->lru_next=head;
		el->lru_prev=head->lru_prev;
		head->lru_prev->lru_next=el;
		head->lru_prev=el;
		numel++;
		return 1;
	}
	void Remove(CLightCacheElem *el)
	{
		if(!el) return;
		if(el->RemoveLRU()) numel--;
	}
	void Delete(CLightCacheElem *el)
	{
		if(!el) return;
		Remove(el);delete(el);
	}

	void DeleteAll()
	{
		CLightCacheElem *el,*ne;
		el=GetFirst();
		while(el){
			ne=el->GetLRUNext();
			delete el;
			el=ne;
		};
	}

	int maxel;
protected:
	CLightCacheElem *head;
	int numel;

};

//slouzi na zjisteni, ktera svetla sviti na element
class CLightMap{
public:
	CLightMap(int minx,int miny,int maxx,int maxy, int sqsx, int sqsy);
	~CLightMap();

	int AddLight(CLight *light); //prida svetlo (bodove)
	int DeleteLight(CLight *light); //odebere svetlo

	int GetLights(CGEnElement *el, int maxlights, int &numlights, CLight **lights); //vrati svetla, ktera sviti na element

protected:

	int Resize(int minx,int miny,int maxx,int maxy); //zvetsi velikost mapy
	int CmpSQ(int x1, int y1, int x2, int y2, int &sqx1, int &sqy1, int &sqx2, int &sqy2); //spocita ctverce v mape - ctverce jsou z intervalu <sqx1,sqx2) a <sqy1,sqy2)

	CPLightList *map; //mapa svetel

	int minx,miny,maxx,maxy,sqsx,sqsy; //min a max sour., velikost ctverce
	int numsqx,numsqy; //pocet ctvercu v x,y ose

	void SortLights(int numlight, CLight **lights); //setridi svetla

};


//svetelna cache
class CLightCache{
public:

	CLightCache(int sz); //sz je velikost hashovaci tabulky nasvicenych elementu (table)
	~CLightCache();
	
	int GetTex(CGEnElement *el, CTexture **outtex); //vrati nasvicenou texturu elementu - bud ji najde v cachi, nebo ji spocita

	int AddElem(CGEnElement *el){shadowgrid->AddElem(el); return elemgrid->AddElem(el);} //prida element do cache (cache musi mit seznam vsech elementu - aby je mohla zneplatnovat pri pohybu svetla, pri vrzeni stinu)
	int DeleteElem(CGEnElement *el){shadowgrid->DeleteElem(el); return elemgrid->DeleteElem(el);} //vyhodi element z cache

	int AddLight(CLight *l); //prida svetlo
	int DeleteLight(CLight *l); //vyhodi svetlo

	int SetTopLight(CTopLight *light); //nastavi svetlo shora
	CTopLight* GetTopLight(){return toplight;} //vrati svetlo shora

	int numcmptex; //pocet nasvicenych textur v cachi

protected:

	int Add(int hashval, CGEnElement *el, CLightGroupElement *lge, CShadowGroupElement *sge, int *pos, CTexture *tex); //prida nasvicenou texturu do cache

	int Find(int hashval, CGEnElement *el, CLightGroupElement *lge, CShadowGroupElement *sge, int *pos, CTexture **tex); //najde nasvicedou texturu v cachi (vraci 1 pokud najde, jinak 0)

	int CmpHash(CGEnElement *el, CLightGroupElement *lge, CShadowGroupElement *sge, int *pos) //spocte hash fci nasvicenyho elementu
	{
		unsigned int hv;
		if(lge->IsPosDep())
			hv=( ((int)el->zx-pos[0]) ^
			    ( ((int)el->zy-pos[1]) <<5)^
			    ( ((int)el->zz-pos[2]) <<8) ) <<16;
		else
			hv=333<<16; //magic constant
		
		hv^= (int)el->LightTex;
		hv^= (int)lge; 
        hv^= (int)sge;
			 		
		hv = hv % size;

		return hv;
	}

	CLightCacheList *table; //hash tabulka nasvicenych textur
	int size; //velikost hash tabulky
	CLightCacheLRU *lru; //LRU seznam nasvicenych textur (kvuli vyhazovani nejstarsich)

	CLightGroupHash *lghash; //hashtabulka skupiny svetel
	CShadowGroupHash *sghash; //hashtabulka skupiny stinu

	CLightMap *lightmap; //mapa svetel - pomoci ni se zjistuje, ktera svetla sviti na element

	CElemGrid *elemgrid; //mriz elementu - pomoci ni se zjistuje, ktere elementy jsou v dane oblasti (aby se mohli zneplatnit, napr. pri pohybu svetla)
	CShadowGrid *shadowgrid; //mriz stinu - pomoci ni se zjistuje, jaky stiny jsou vrzeny na element

	CTopLight *toplight; //svetlo shora

	CLight **tmplightarray; //pomocne pole, do ktereho se docasne ukladaji svetla svitici na element
	int tmplightarraysize; //velikost pole

	CShadowGridInstance **tmpshadowarray; //pomocne pole, do ktereho se docasne ukladaji stiny vrzene na element
	int tmpshadowarraysize; //velikost pole

};



#endif