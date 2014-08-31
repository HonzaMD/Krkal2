////////////////////////////////////////////////////////////////////////////////////////////////////
//
// elemgrid.h
//
// Mriz GEn elementu (pouziva se v lightcache)
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ELEMGRID_H
#define ELEMGRID_H

#include "types.h"

#include "genelement.h"


typedef CListK<CGEnElement*> CPGEnElemList;
typedef CListKElem<CGEnElement*> CPGEnElemListElem;

//CElemGrid je 3D mriz GEnElemntu
class CElemGrid{
public:
	CElemGrid(int minx,int miny,int minz,int maxx,int maxy,int maxz, int sqsx, int sqsy, int sqsz);
	~CElemGrid();

	int AddElem(CGEnElement *el, int addingoutelem=0); //prida element
	int DeleteElem(CGEnElement *el); //vyhodi element

	int Resize(int minx,int miny,int minz,int maxx,int maxy,int maxz); //zvetsi pole podle zadanych udaju
	int Resize(); //zvetsi pole na potrebnou velikost

	int InvalidElems(int minx,int miny,int minz,int maxx,int maxy, int maxz); //zneplatni elementy uvnitr boxu


protected:

	int CmpSQ(int x1, int y1, int z1, int x2, int y2, int z2, int &sqx1, int &sqy1, int &sqz1, int &sqx2, int &sqy2, int &sqz2); //spocita ctverce cache, do kterych zasahuje box - ctverce cache jsou v rozsahu sqx1 <= x < sqx2, sqy1<= y <sqy2, sqz1<= z <sqz2

	CPGEnElemList *map; //pole seznamu elementu

	CPGEnElemList elemsout; //elementy, ktere zasahuji mimo vymezenou oblast

	int maxelemsout; //maximalni pocet elementu mimo - pri prekroceni tohoto poctu se pole zvetsi (vola se Resize())
	int numelemsout; //pocet elementu mimo

	int minx,miny,maxx,maxy,minz,maxz; //oblast mrize
	int sqsx,sqsy,sqsz; //velikost jedne bunky mrize
	int numsqx,numsqy,numsqz; //pocet bunek v x,y,z

};

//vrzeny stin ulozeny v mrizi
struct CShadowGridInstance: public CRefCnt
{
	CShadowGridInstance(){tmtag=0;}
		
	class CLightTex *shadowtex; //textura stinu
	int x1,y1,x2,y2; //souradnice stinu ve vysce 0
	int zmax; //maximalni z-souradnice

	CGEnElement *shadow_element; //element vrhajici stin

	int dx,dy; //posunuti

	int tmtag; //pomocny flag

	int Cmp(CShadowInstance &s){if(shadowtex!=s.shadowtex||dx!=s.dx||dy!=s.dy/*||zmax!=s.zmax*/) return 1; else return 0;}
	int Cmp(CShadowGridInstance *s)
	{
		if(shadowtex < s->shadowtex) return 2;
		if(shadowtex > s->shadowtex) return 1;
		if(dx < s->dx) return 2;
		if(dx > s->dx) return 1;
		if(dy < s->dy) return 2;
		if(dy > s->dy) return 2;
		return 0;
	}

};

typedef CListK<CShadowGridInstance*> CPShGrInstList;
typedef CListKElem<CShadowGridInstance*> CPShGrInstListElem;

//CShadowGrid je 2D mriz, pomoci jiz se urcuje, jaky stiny jsou kde vrzeny
class CShadowGrid
{
public:
	CShadowGrid(int minx,int miny,int maxx,int maxy, int sqsx, int sqsy);
	~CShadowGrid();

	int AddElem(CGEnElement *el); //prida element vrhajici stin
	int DeleteElem(CGEnElement *el); //vyhodi element

	int Resize(); //zvetsi mriz, aby pokryla vsechny stiny

	void GetShadows(int maxshadows, int &numshadows, CShadowGridInstance **shadows, CGEnElement *element); //vrati vsechny stiny vrzeny na element

protected:

	int Resize(int minx,int miny,int maxx,int maxy); //zvetsi mriz
	int CmpSQ(int x1, int y1, int x2, int y2, int &sqx1, int &sqy1, int &sqx2, int &sqy2); //spocte ctverce cache

	int CmpElemSquare(CGEnElement *el, int &sqx1, int &sqy1, int &sqx2, int &sqy2); //spocte ctverce cache
	void CmpElemBRect(CGEnElement *el, int &x1, int &y1, int &x2, int &y2); //vrati obalovy cvterec kolem elementu
	void CmpElemBRect2(CGEnElement *el, int &x1, int &y1, int &x2, int &y2, int &bx1, int &by1, int &bx2, int &by2); //vrati obalovy cvterec kolem elementu + ctverec zakladny
	int TestShadow(CShadowGridInstance *shgi, CGEnElement *el,int x1,int y1,int x2,int y2,int bx1,int by1,int bx2,int by2); //zjistuje jestli se stin shgi hazi na element el (x1,y1,... jsou jiz drive spocitany souradnice el)
	int TestShadow(CShadowGridInstance *shgi, CGEnElement *el); //zjistuje jestli se stin shgi hazi na element el

	void AddOutElem(CGEnElement *el); //vezme elementy, ktere zasahuji mimo mriz, a zatridi je do mrize
	void AddOutShadow(CShadowGridInstance *shgi); //vezme stiny, ktere zasahuji mimo mriz, a zatridi je do mrize

	CPShGrInstList *shadows; //mriz stinu
	CPGEnElemList *elems; //mriz elementu

	CPShGrInstList shadowsout; //elementy zasahujici mimo mriz
	CPGEnElemList elemsout; //stiny zasahujici mimo mriz

	int numshadowsout,maxshadowsout;
	int numelemsout,maxelemsout;

	int minx,miny,maxx,maxy; //rozsah mrize
	int sqsx,sqsy; //velikost ctvercu
	int numsqx,numsqy; //pocet ctvercu v x,y
};


#endif