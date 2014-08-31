/////////////////////////////////////////////////////////////////////////////////
///
///		GEnElement
///
///		obrazek pro GEn: CGEnElement + CGEnElSource
///		A: Petr Altman + Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////////////


#ifndef GENELEMENT_H
#define GENELEMENT_H

#include "dxbliter.h"
#include "types.h"
#include "vector.h"
#include "lighttex.h"

#include "genresource.h"

#include "genanim.h"

#include "kerconstants.h"

///////////////////////////////////////////////////////////////////////
///
///		C G E n   E l S o u r c e
///
//////////////////////////////////////////////////////////////////////

// Typy Grafickych Objektu, info nezavisle na konkretni vykreslene instanci
class CGEnElSource: public CGEnResource {
public:
	CGEnElSource() { BasePoints=0; BasePointsCount=0; DerRampaFlag=0; DerFlugFlag=0; }
	~CGEnElSource() {
		SAFE_DELETE_ARRAY(BasePoints);
	}

	virtual int Save(char *name, CGEnPackage *pkg); //ulozi elsource do souboru v packagi
	virtual int Load(char *name, CGEnPackage *pkg, int lazy=0, int myindex=-1 ); //nacte elsource ze souboru v packagi
	virtual eResourceType GetType(){return resElSource;} //vrati ty resorcu

	int SetParams( float _pbx, float _pby, float _pbz, float _zdz, int _rampaflag, int _flugflag); //nastavi parametry elsourcu
	int SetNumPoints( int pointsnum ); //nastavi pocet bodu

	void CalculateBorderPoints(); // napred je treba zadat body zakladny / z tech se spocita LPoint, RPoint

	CVec2 *BasePoints;  // Body zakladny
	int BasePointsCount;  // jejich pocet
	int LPoint, RPoint; // Indexy Krajnich bodu zakladny pro viditelnostni trideni

	float zdz; // vyska
	float pbx,pby,pbz; // posun bitmapy

	int DerRampaFlag; // kdyz nastaveno vec bude dole v pripade viditelnostni kolize
	int DerFlugFlag; // kdyz nastaveno (na nenulu) a nastaveno na stejnou hodnotu, tak se bude tridit podle unikatnich cisel u elementu = to jest podle pointeru na elementy

/* obsolete: (urcite to nebude tady, kdyztak v lighttex)
	int UpLight; // zda na obr sviti svetla seshora
	int UpShadow; // zda jsou na onr vrhany stiny seshora 
	int LocalLight; // zda na obr sviti lokalni svetla
	float ShadowIntensity; // jak moc obr pohlcuje svetlo (generuje stin) 
*/
	
	void CmpBBox(); //spocte ohranicujici obdelnik (z nepocita)
	float minx,maxx,miny,maxy; //bbox (z ~ 0..zdz)

};

typedef CListK<CGEnElSource*> CPGEnElSourceList;
typedef CListIterator<CGEnElSource*> CPGEnElSourceListIt;

///////////////////////////////////////////////////////////////////////
//
//		CGEnMoveElem  
//
//		info o pohybu a animaci
//
///////////////////////////////////////////////////////////////////

struct CGEnMoveElem
{
	CGEnMoveElem(CGEnElement *_el) //nastavi element, zadny pohyb ani animace
	{
		el=_el;
		type=0;
		ack=0;
		anim=NULL;
	}
	~CGEnMoveElem(){
		SAFE_RELEASE(anim);
	}

	//rovnomerny primocary pohyb
	// el: pohybujici se element
	// (_startx,_starty,_startz): pocatecni poloha elementu
	// (_endx,_endy,_endz): koncova poloha elementu
	// _starttime: cas zacatku pohybu
	// _time: celkovy cas pohybu (v ms)
	CGEnMoveElem(CGEnElement *_el, float _startx, float _starty, float _startz, float _endx, float _endy, float _endz, UI _starttime, UI _time)
	{
		el=_el;
		type=0;anim=NULL;ack=0;
		StartMove(_startx,_starty,_startz,_endx,_endy,_endz,_starttime,_time);
	}

	//animace
	// el: element, ktery se bude animovat
	// _anim: animace
	// _animstarttime: cas zacatku animace
	CGEnMoveElem(CGEnElement *_el,CGEnAnim *_anim, int _frame_in_starttime, UI _starttime)
	{
		el=_el;
		type=0;anim=NULL;ack=0;
		StartAnim(_anim,_frame_in_starttime,_starttime);
	}

	//nastavi rovn. prim. pohyb
	void StartMove(float _startx, float _starty, float _startz, float _endx, float _endy, float _endz, UI _starttime, UI _time)
	{
		type|=1; ack=0;
		startx=_startx;starty=_starty;startz=_startz;
		endx=_endx;endy=_endy;endz=_endz;
		starttime=_starttime; time=_time;
		dx=(endx-startx)/time;dy=(endy-starty)/time;dz=(endz-startz)/time;
	}

	//nestavi animaci
	void StartAnim(CGEnAnim *_anim, int _frame_in_starttime, UI _starttime)
	{
		type|=2; ack=0;
		SAFE_RELEASE(anim);
		anim=_anim;
		anim->AddRef();

		nextframetime=_starttime;
		nextframeindex = _frame_in_starttime % anim->GetNumFrames();;
	}
	int SetAck(int ackanim, int ackmove){
		if( !(type&1) ) ackmove=0;
		if( !(type&2) ) ackanim=0;
		if(ackmove) ackmove=1;
		if(ackanim) ackanim=2;
		ack = ackmove|ackanim;
		return ack;
	}

	void StopAnim()
	{
		type&=1; //shodim anim flag
	}

	int type; //0=nic 1=move 2=anim 3=move+anim

	CGEnElement *el; //element

	//veci pro pohyb:
	float startx,starty,startz; //poc. poloha elementu
	float endx,endy,endz; //konc. poloha
	UI time; //celkovy cas pohybu
	UI starttime; //zacatek
	float dx,dy,dz; //posun za 1 ms

	//veci pro animaci:
	CGEnAnim *anim; //animacni sekvence
	UI nextframetime; //cas, kdy se zobrazi dalsi frame
	int nextframeindex; //index dalsiho framu

	int ack; //urcuje jestli se po skonceni ma zavolat fce krenelu
};

typedef CListK<CGEnMoveElem*> CPGEnMoveElemList;
typedef CListKElem<CGEnMoveElem*> CPGEnMoveElemListEl;
typedef CListIterator<CGEnMoveElem*> CPGEnMoveElemIter;


///////////////////////////////////////////////////////////////////////
///
///		C G E n   E l e m e n t
///
///////////////////////////////////////////////////////////////////

class CGEnGraph;

enum eGEnType {eGEnElement,eGEnLine};

class CGEnSortElement
{
public:
	CGEnSortElement *next;  // Spojak Obrazku, ktere se maji tridit (Spojak vrcholu grafu)
	CGEnGraph *gr;  //  Spojak Hran grafu z tohoto vrcholu
	UCHAR obarveni; // obarveni vrcholu v grafu 0 - jeste jsem v nem nebyl(bily), 1 - provadim(sedy), 2 - opustil jsem ho(cerny)
	CBltElem *bltEl;
};

///  Spojak Hran grafu
class CGEnGraph {
public:
	CGEnSortElement *el;
	CGEnGraph *next;
};

/// Konkretni vykreslena instance Grafickeho objektu (obrazku)
class CGEnElement: public CBltRect {

friend class CGEnMain;
//kvuli rychlejsimu pristupu mohou objekty cache pristupovat k polozkam primo:
friend class CLightTex;
friend class CLightCache;
friend class CLightMap;
friend struct CLightID;
friend class CLight;
friend class CElemGrid;
friend class CShadowGrid;

public:
	void GEnMove(float dx, float dy, float dz, int cancelmove=1);  // Pohni vykreslenym objektem o dany pocet bodu
	void GEnSetPos(float x, float y, float z, int cancelmove=1); //posune element na zadanou pozici

	void GetPos(float &x, float &y, float &z) {x=zx; y=zy; z=zz;} //vrati pozici elementu

	virtual int CmpTex(); //spocte nasvicenou texturu
	virtual int IsCmpTex(){return 1;} //flag, ze textura je pocitana
	virtual void Render(int drawall=1); //vykresli ctverec
	virtual dxBltType GetType(){return dxGEnRect;} //vrati typ elementu

	int SetLightTex(CLightTex *LightTex); //nastavi texturu
	CLightTex* GetLightTex(){return LightTex;} //vrati texturu

	CGEnElSource* ElSource(){return LightTex->GetElSource();}  // vrati Info o Grafickem Objektu (nezavisle na konkretni instanci)

	virtual ~CGEnElement();

	OPointer ObjPtr; //pointr na objekt kernelu

	int DasTepichFlag; //"kobercovy" flag - element s vyssim cislem zakryva elementy s nizsimi cislem

private:
	CGEnElement(float x, float y, float z, CLightTex *tex, OPointer ptr=0);
	float zx,zy,zz;  // umisteni obrazku ve svete 
		// Vypocet souradnic Textury:
		// x1 = zx+pbx+xztransform*zz
		// y1 = zy+pby+yztransform*zz

	CLightTex *LightTex;  //svetelna textura

	CGEnMoveElem *mel; //info o pohybu a animaci

	CGEnSortElement *sortel; //pro vidit. trideni

};


class CGEnLine: public CBltLine {

friend class CGEnMain;
friend class CGEnCursor;

public:
	CGEnLine(float x1, float y1, float z1, float x2, float y2, float z2, DWORD color, int _TopFlag=0);

	void GEnMove(float dx, float dy, float dz);  // Pohni vykreslenym objektem o dany pocet bodu
	void GEnSetPos(float x1, float y1, float z1,float x2, float y2, float z2); //posune element na zadanou pozici
	void SetCol(DWORD c){CBltLine::SetCol(c);}

	virtual dxBltType GetType(){return dxGEnLine;} //vrati typ elementu

private:
	float zx1,zy1,zz1;  // umisteni obrazku ve svete  - plati zz1<=zz2
	float zx2,zy2,zz2;
		// Vypocet souradnic:
		// x1 = zx+xztransform*zz
		// y1 = zy+yztransform*zz

	int TopFlag; //cim vetsi, tim je vic nahore (cary s vetsim cislem jsou nad tema s mesim)

	CGEnSortElement *sortel; //pro vidit. trideni
};


// kurzor kolem bunky mapy (tvoreny carama)
// zatim je podporovana jenom kostka
class CGEnCursor{
	friend class CGEnMain;

public:
	
	int MoveCursor(float dx, float dy, float dz);
	int SetCursorPos(float x, float y, float z);

	int SetVisible(int visible);
	int SetColor(DWORD color, DWORD color2);

	~CGEnCursor();

protected:
	CGEnCursor();
	int CreateCursor(int _type, float _sx, float _sy, float _sz, DWORD color, DWORD color2);
	int DeleteCursor();

	int AddToGEn(float x,float y,float z);

	int inengine;
	int type;
	float sx,sy,sz;
	float px,py,pz;

	int numlines;
	CGEnLine **lines;
};

#endif