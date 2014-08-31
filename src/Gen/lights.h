////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lights.h
//
// Definice svetel
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIGHTS_H
#define LIGHTS_H

#include "types.h"

//svetlo shora (rovnobezne paprsky)
class CTopLight{
public:
	CTopLight(float dx,float dy,float dz,float ir,float ig,float ib);

	char dx,dy,dz; //smer paprsku
	UC ir,ig,ib; //intenzita

	void SetIntenzity(UC r, UC g, UC b){ir=r;ig=g;ib=b;CmpID();} //nastavi intenzitu

	void CmpID(){ id=(ir<<16)+(ig<<8)+ib; } //prepocita ID
	UI id; //ID svetla

};

//bodove svetlo
class CLight{
public:

	CLight(){tmtag=0;}
	CLight(float px,float py,float pz, float ir, float ig, float ib, float radius); //(px,py,pz) - stred, (ir,ig,ib) - intenzita, radius - polomer
	~CLight(){}

	void PointLight(float px,float py,float pz, float ir, float ig, float ib, float radius); //(px,py,pz) - stred, (ir,ig,ib) - intenzita, radius - polomer
	void PointLightInt(int px,int py,int pz, UC ir, UC ig, UC ib, int radius); //(px,py,pz) - stred, (ir,ig,ib) - intenzita, radius - polomer

	int TestLight(int x1, int y1, int z1,int x2, int y2, int z2); //testuje jestli svetlo sviti na ctverec
	int TestLightPlane(int x1, int y1, int x2, int y2, int z); //testuje jestli svetlo sviti na rovinu

	int Compare(CLight &l, int *pos=NULL); //porovna 2 svetla: vraci 0=stejny, 1=jsem vetsi nez l, -1=jsem mensi nez l

	int GetBBox(int &x1, int &y1, int &z1, int &x2, int &y2, int &z2); //vrati box, kam sviti svetlo

	int px,py,pz; //pozice svetla
	UC ir,ig,ib; //intenzita
	UC imax; //maximalni intenzita

	int r; //radius
	int radius; // 1/(r^2) shiftnuty doleva o 24bitu

	UC tmtag; //pomocny flag (pouziva se pri zjistovani vsech svetel, ktere sviti na element - aby se 1 svetlo nepridalo 2x)

};

typedef CListK<CLight> CLightList;
typedef CListIterator<CLight> CLightListIter;

typedef CListK<CLight*> CPLightList;
typedef CListIterator<CLight*> CPLightListIter;


#endif