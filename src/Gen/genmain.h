/////////////////////////////////////////////////////////////////////////////////
///
///		G A M E   G R A P H I C   E N G I N E   -   G E n
///
///		Main Objeckt, viditelnostni trideni, seznamy obrazku
///		A: Honza M.D. Krcek (viditelnostni trideni), Petr Altman (svetla)
///
/////////////////////////////////////////////////////////////////////////////////


#ifndef GENMAIN_H
#define GENMAIN_H

#include "dxbliter.h"
#include "types.h"
#include "input.h"
#include "vector.h"

#include "genresource.h"
#include "genpackage.h"
#include "genelement.h"
#include "genanim.h"

#define EPSILON 0.00014f

CVec2 RotateVL(const CVec2 &v); // rotace o 90 stupnu doleva
CVec2 RotateVR(const CVec2 &v); // rotace o 90 stupnu doprava

#define SORT_HTSIZE 1069
struct CSortResult
{
	CBltElem *e1,*e2;
	CSortResult *next;
};

class CSortResultHashTable{
public:
	CSortResultHashTable();
	~CSortResultHashTable();
	void EmptyTable();

	int FindResult(int hashval,CBltElem *e1, CBltElem *e2)
	{
		CSortResult *sr=ht[hashval];
		while(sr)
		{
			if(sr->e1==e1&&sr->e2==e2) return 1;
			if(sr->e1==e2&&sr->e2==e1) return 1;			
			sr=sr->next;
		}
		return 0;
	}
	void AddResult(int hashval,CBltElem *e1, CBltElem *e2)
	{
		CSortResult *sr;
		if(SRstack)
		{
			sr=SRstack;
			SRstack=sr->next;
		}else
			sr = new CSortResult;
		sr->e1=e1;sr->e2=e2;
		sr->next=ht[hashval];
		ht[hashval]=sr;

		if(ptrs[hashval]==-1)
		{
			ptrs[hashval]=fp;
			fp=hashval;
		}
	}

	int CmpHash(CBltElem *e1, CBltElem *e2)
	{
		return ((int)e1+(int)e2)%SORT_HTSIZE;
	}

private:
	CSortResult* ht[SORT_HTSIZE];
	int ptrs[SORT_HTSIZE];
	int fp;

	CSortResult* SRstack;
};

////////////////////////////////////////////////////////////////////////
///
///		C G E n   M a i n
///		Hlavni objekt Herniho Graficke Enginu
///
//////////////////////////////////////////////////////////////////

class CMusicModule;
class CMusicSample;

class CGEnMain {
friend class CLightTex;
friend class CLightCache;
public:
	CGEnMain(CBltWindow *_GameWindow);
	~CGEnMain ();

	//----------------------------------------------------------------------------------------------
	// vytvareni elementu
	
	// Pridani a vykresleni nove instance grafickeho objektu (obrazku)
	CGEnElement* DrawElement(float x, float y, float z, CLightTex *tex); //vytvori a zobrazi element; pro mazani elementu pouzijte funkci DeleteElement (ne delete)
	CGEnElement* DrawElement(float x, float y, float z, CGEnAnim *anim, int frame_in_starttime=-1, UI starttime=0); //vytvori a zobrazi element, spusti animaci (viz. SetAnim); pro mazani elementu pouzijte funkci DeleteElement (ne delete)
	
	CGEnElement* DrawElement(OPointer objptr, float x, float y, float z, CGEnResource *anim, int frame_in_starttime=-1, UI starttime=0); //vytvori a zobrazi element, spusti animaci (viz. SetAnim); pro mazani elementu pouzijte funkci DeleteElement (ne delete)
	int SetAnimFinishAck(CGEnElement *el, int anim=1, int move=1); //po ukonceni animace nebo pohybu zavola fci kernelu GEnProcessEnded, parametr anim (resp. move) urcuje, jestli se ma volat fce po skonceni animace (resp. po skonceni pohybu); vraci: 0=fce se nezavola, 1=fce se zavola po skonceni move, 2=fce se zavola po skonceni animace, 3=fce se zavola po skonceni animace i move (2x)

	CGEnLine* DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, DWORD col, int TopFlag=0); //vytvori a zobrazi caru; pro mazani elementu pouzijte funkci DeleteLine (ne delete)
	int DrawLine(CGEnLine *ln); //zobrazi caru; pro mazani elementu pouzijte funkci DeleteLine (ne delete)

	//kurzor
	CGEnCursor* CreateCursor(int type, float _sx, float _sy, float _sz, DWORD color=0xFFFFFFFF, DWORD color2=0x40FFFFFF); //vytvori kurzor; pro mazani elementu pouzijte funkci DeleteCursor (ne delete)
	int DrawCursor(CGEnCursor *cursor, float x, float y, float z); //zobrazi kurzor

	//----------------------------------------------------------------------------------------------
	// ruseni elementu

	int DeleteElement(CGEnElement* el); //zrusi element (deletuje el)
	int DeleteLine(CGEnLine* ln); //zrusi caru (deletuje ln)
	int DeleteCursor(CGEnCursor* cur); //zrusi cursor (deletuje cur)

	int RemoveLine(CGEnLine *ln); //vyhodi caru z enginu - caru uz pak GEn sam nedeletuje
	
	void DeleteAllElements(); //zrusi vsechny elementy(i cary) a kurzory
	void DeleteAllLights(); //zrusi vsechny svetla
	void DeleteAllCursors(); //zrusi vsechny kurzory
	void DeleteAll(); //zrusi vsechny elementy, svetla, kurzory

	//----------------------------------------------------------------------------------------------
	// modifikace existujicich elementu

	// pohyb elementu
	void MoveElem(CGEnElement *el, float startx, float starty, float startz, float destx, float desty, float destz, UI time);
	int  MoveElem(CGEnElement *MoveDest, CGEnElement *MoveSource); //zkopiruje pohyb od MoveSourcu do MoveDest; pokud se MoveSource nepohybuje posune se MoveDest na pozici MoveSource

	// spusti animaci
	// pokud je frame_in_starttime=-1 -> spusti animaci hned od prvniho obrazku
	// jinak -> frame_in_starttime je cislo obrazku v case starttime
	int SetAnim(CGEnElement *el, CGEnAnim *anim, int frame_in_starttime=-1, UI starttime=0);

	// Zmeni texturu u existujiciho elementu
	int SetLightTex(CGEnElement *el, CLightTex *tex){if(!el) return 0;return el->SetLightTex(tex);}

	// zmeni obrazek existujiciho elementu; res muze byt LightTex nebo animace
	// nezrusi move, zrusi predchozi animaci
	int SetPicture(CGEnElement *el, CGEnResource *res, int frame_in_starttime=-1, UI starttime=0);

	// vidtelnost kurzoru
	int SetVisible(CGEnCursor *cursor, int visible); 

	//----------------------------------------------------------------------------------------------
	// package

	int SetLazyLoad(int lazy){PackageLazyLoad=lazy;} //nastavi lazy load
	int GetLazyLoad(){return PackageLazyLoad;} //vrati nastaveni lazy loadu

	// Nacteni resourcu z package
	CGEnElSource* LoadElSource(char *name, CGEnPackage **pkg=NULL); //nahraje ElSource
	CLightTex* LoadLightTex(char *name, CGEnPackage **pkg=NULL); //nahraje LightTex (nahraje i potrebny ElSource)
	CGEnAnim* LoadAnim(char *name, CGEnPackage **pkg=NULL); //nahraje Animace (nahraje i potrebne Textury a ElSource)
	CTexture* LoadTexture(char *name, CGEnPackage **pkg=NULL); //nahraje texturu
	
	CMusicModule* LoadMusicModule(char *name, CGEnPackage **pkg=NULL); //nahraje MusicModule
	CMusicModule* LoadMusicModuleDirect(char *name); //nahraje MusicModule primo bez pouziti packagi, POZOR! po ukonceni prace s modulem zavolat Release! (lze jiz po zavolani Play)
	
	CMusicSample* LoadMusicSample(char *name, CGEnPackage **pkg=NULL); //nahraje MusicSample

	CGEnResource* LoadPicture(char *name); //naloduje animaci nebo lighttex

	eResourceType GetResourceType(char *name, CGEnPackage **pkg=NULL); //vrati typ resourcu

	//deletuje vsechny nepouzity resource; deletuje package, ktery nemaji zadny pouzity resource
	void UnloadResources() { PackageMgr.UnloadResources(); }

	//vrati package manager
	CPackageMgr* GetPackageMgr(){return &PackageMgr;}

	//----------------------------------------------------------------------------------------------
	// ovladani okna - pozice okna, scrolovani
	
	int GetWindowPos(float &x1, float &y1, float &x2, float &y2); //vrati polohu okna v souradnicich obrazovky
	int SetWindowPos(float x1, float y1, float x2, float y2); //nastavi polohu okna

	int GetWindowSize(float &x, float &y); //vrati velikost okna

	int AddToWindow( CBltWindow *wnd, int top=1 ); //presune herni okno do zadaneho okna

	//nascroluje herni okno, absolutni souradnice
	void SetScroll(float x, float y); 

	//vrati posunuti okna
	void GetScroll(float &x, float &y);

	void Scroll(float startx, float starty, float destx, float desty, UI time); //nastavi plynuly scroling
		// (startx, starty) je abs. pozice na zacatku
		// (destx,desty) je abs. pozice v konc. case			
		// time je celkova doba pohybu (v ms)

	void SetScrollShift(float x, float y) //nastavi posunuti nascrolovani (resetuje se nastavenim velikosti okna)
		{
			scrollshiftX=x;scrollshiftY=y;
			GetScroll(x,y);SetScroll(x,y);
		}


	//----------------------------------------------------------------------------------------------
	// svetla

	int AddLight(int x, int y, int z, UC ir, UC ig, UC ib, int radius ); //(x,y,z) pozice, (ir,ig,ib) intenzita, radius polomer
	void DeleteLight(int lightPtr);

	void SetTopLightIntenzity(UC r, UC g, UC b);
	void GetTopLightIntenzity(UC *r, UC *g, UC *b);
	
	void SetTopLightDefault();

	CTopLight* GetTopLight(){return toplight;}

	//----------------------------------------------------------------------------------------------
	// viditelnostni trideni

	CGEnElement* FindTopElement(float x, float y); //najde element na pozici x,y (podle viditelnosti); x,y je v souradnicich herniho okna

	int CompareTwo(CGEnElement *el1,CGEnElement *el2); //porovnani 2 elementu podle viditelnosti
	int CompareTwo(CGEnElement *el1,CGEnLine *el2); //porovnani 2 elementu podle viditelnosti

	void SortIt(); //setridi zneplatnene obrazky
	void BuiltSortGraph(CListK<CBltElem*> *elemlist);

	// Funkce pro vraceni Perspektivniho vektoru. Vektor vede ve smru osy z (Po transformaci do 2D)
	float GetXZTransform() {return ztransform.x;}
	float GetYZTransform() {return ztransform.y;}
	CVec2 GetZTransform() {return ztransform;}

	//----------------------------------------------------------------------------------------------
	// ruzne

	int Update(UI curtime); //upadate sceny - hejbe elementama, provadi animace, scroluje
	void SetKernelTime(UI time){kernelTime=time;} //nastavi cas

	CLightCache *lightcache; //svetelna cache
	void DeleteMovingElem(CGEnMoveElem *mel); //smaze moving element


	//----------------------------------------------------------------------------------------------
	// testy
	/*
	void ComeOnBaby();
	void CreateTest();
	void CreateTexFiles();
	CGEnElement *ty;
	int tydx, tydy, tydz;
	int tyx,tyy,tyz;
	*/

private:

	void dfs(CGEnSortElement *el); //DFS na trideni elementu

	CBltWindow *GameWindow;  // Herni Okno
		
	CVec2 ztransform;  // Perspektivni vektor
	
	CPLightList lights; //seznam svetel
	CTopLight *toplight; //svetlo shora

	CListK<CGEnCursor*> cursors; //kurzory

	UI kernelTime; //cas kernelu

	//promenne pro scrolling
	float scrollX, scrollY; //konecny vektor nascrolovani (po ukonceni plyn. scrolingu)
	float scrollXnow,scrollYnow; //aktualni vektor nascrolovani
	float scrollshiftX,scrollshiftY; //posunuti nascrolovani
	int scroll;
	float scrollDX, scrollDY;
	UI scrollStartTime,scrollTime;
	float scrollStartX,scrollStartY;

	UI lastupdatetime; //posledni cas updatu

	CPGEnMoveElemList MovingElems; //seznam pohybujich se elementu (a animaci)

	CPackageMgr PackageMgr; //spravce Packagu

	CGEnSortElement *SElements;  // Spojak Elementu, ktere bude nutno setridit a vykreslit

	CGEnSortElement *SElemsStack; // zasobnich starejch SElementu (aby se nemuseli newovat)
	CGEnSortElement* GetNewSortElem(CBltElem *bltelem);
	CGEnGraph *GGraphStack; //zasobnik starejch GenGraphu
	CGEnGraph* GetNewGGraph();
	CSortResultHashTable sortResultHT;

};


/////////////////////////////////////////////////////////////////////////

extern CGEnMain *GEnMain; /// Pointr na hlavni (jediny) Objekt GEn 

#endif