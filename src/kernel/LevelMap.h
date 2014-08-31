//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - L e v e l M a p
///
///		Sluzby Kernelu pro praci s Mapou a pro spolupraci sobjektem Mapa
///		Sluzby pro praci s umistitelnyma objektama, umistovani, pohyby, kolize
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KERLEVELMAP_H
#define KERLEVELMAP_H

#include "types.h"
#include "KerArray.h"
#include "kernel.h"


///////////////////////////////////////////////////////////////////////////
///
///		CKerMoveTimer
///
///		struktura, ktera ridi plynuly pohyb. Pokud se umisteny objekt pohybuje,
///		je pro nej tato struktura vytvorena a objekt a mover jsou vzajemne propojeny
///		Kazdy takt je pohyb aktualizovan v obj Mapa. GEn provadi na kernelu nezavysly plynuly pohyb.
///		Pohyb je zrusen, kdyz se sam ukonci a vyvolanim jineho pohybu (at uz plynuleho ci okamziteho)
///		Objekt je informovan o ukonceni pohybu zpravou (pokud pohyb zkoncil prirozene nebo diky kolizi)
///
///////////////////////////////////////////////////////////////////////////

struct CKerMoveTimer {
	CKerMoveTimer(CKerObject *ko, int startX, int startY, int startZ, int endX, int endY, int endZ, UI time);
	CKerMoveTimer(CKerObject *ko, CKerMoveTimer *mover); // okopiruju pohyb z jineho moveru
	void GSaveMover(CFSRegKey *k);
	~CKerMoveTimer();
	void DoMove();  // muze se deletovat

	int startx, starty, startz; // pocatcni soradnice
	int endx, endy, endz;		// koncove souradnice
	UI endTime;					// cas konce
	UI startTime;				// cas zacatku
	CKerObject *kMovingObj;		// pohybovany objekt

	int DeleteMe;				// nastavim kdyz mam pohyb vydeletevat a poslat o tom objektu zpravu
	CKerMoveTimer *next, *prev; // obousmerny spojak vsech existujicich Moveru
};






///////////////////////////////////////////////////////////////////////////
//
//		Struktury kam se nahravaji promenne z objektu, ovlivnujici umistovani a kolize
//		Dale jsou tu vsechny dalsi promenne potrebne pro umistene objekty
//		Umisteny objekt k temto promennym nema primy pristup. 
//		Takze napriklad nasilna zmena souradnice x se neprojevi
//
////////////////////////////////////////////////////////////////////////////

struct CKerCollisionInfo {
	CKerCollisionInfo();
	void LoadObjectInfo(OPointer obj);
	void CalculateCube();

	int x,y,z;		// souradnice objektu
	UC CollCfg;		// jakym zpusobem se urcuje, co objekt zabira
	int CellX, CellY, CellZ, CellR;	// pocty zabranych bunek
	int CubeX1, CubeY1, CubeZ1;		// kolizni krichle
	int CubeX2, CubeY2, CubeZ2;
	CKerName *APicture;		// zvoleny automatismus
};

struct CKerPlacedInfo : CKerCollisionInfo {
	CKerPlacedInfo() {
		AutoPrev=0; AutoNext = 0; AktivAuto=0; Mover=0; AktivAuto2=0; Tag=0; 
	}
	~CKerPlacedInfo() {
		SAFE_DELETE(Mover);
	}
	void GSave(CFSRegKey *k);	// ulozi Placed Info do registru
	int GLoad(CFSRegKey *k);	// nahraje Placed Info z registru
	CKerArrObject ConnectedObjs; // objekty na ktere navazuji.
	CKerName *AktivAuto;	// Prave zvoleny prvek a - to co se prave ted kresli
	CKerName *AktivAuto2;   // Vysledny Auto po provedeni testu. Pokud to nebude zamitnuto, na tohle se automatismus zmeni.
	int Tag;				// 0 netestovano, 1 je nutne prepocitat, 3 testovano, 2 - Byl vybran navaznostni Automatismus
	CKerObject *AutoNext;	// Spojak menenych objektu, diky automatismu.
	CKerObject *AutoPrev;	// Spojak menenych objektu, diky automatismu.
	CKerMoveTimer *Mover;	// pokud objektem hejbe mover
	CKerArrObject MConnectedObjs;  // pole objektu, ktere se budou hejbat zaroven se mnou. Je v nem i tento objekt
};


/////////////////////////////////////////////////////////

struct CKerMapTrigger {
	CKerMapTrigger(OPointer Triger, CKerMapTrigger *Next) { triger = Triger; next = Next; }
	CKerMapTrigger *next;
	OPointer triger;
};



/////////////////////////////////////////////////////////////////////////////
///
///		CKerMapInfo
///
///		Interface pro praci s mapou, funkce pro praci s umistenyma objektama,
///		umistovani, pohyby, detekce kolizi
///
//////////////////////////////////////////////////////////////////////////////


class CKerMapInfo {
	friend CKerMoveTimer;
public:
	CKerMapInfo();
	~CKerMapInfo();
	int CellType;		// Typ Bunky podle eKerCellTypes
	int leftx, lefty;	// souradnice leveho horniho rohu mapy
	int rightx, righty; // souradnice praveho dolniho rohu mapy
	int lowerlevel, upperlevel;	// cislo nejspodnejsiho a nejhorejsiho patra bunek
	int FirstCellx, FirstCelly; // cislo prvni bunky (leve horni)
	int NumCellsx, NumCellsy;	// pocet bunek
	int CellSizeX, CellSizeY, CellSizeZ;	// Velikost bunky v ixove, ypsilonove a Zetove souradnici;
	int registered;		// zda byla mapa registrovana (pro chybovou kontrolu)
	OPointer OMapa;		// pointer na objekt mapa 

	void RegisterMap(int _CellType, int _leftx, int _lefty, int _rightx, int _righty, int _lowerlevel, int _upperlevel, int cellsizeX, int cellsizeY, int cellsizeZ);	//  Vola objekt mapa ve svem konstruktoru.
	int GLoadMapInfo(CFSRegKey *k); // nehraje Map Info z ulozeny hry
	void GSaveMapInfo(CFSRegKey *k); // ulozi Map Info
	int GLoadMover(CFSRegKey *k, CKerObject *ko); // nahraje informace o plynulem pohybu
	void GetNumberOfCells(int *sizex, int *sizey, int *startx, int *starty) { *sizex = NumCellsx; *sizey = NumCellsy; *startx = FirstCellx; *starty = FirstCelly; }	// vrati  indexy prvni bunky a pocty bunek. vse jen v osach x a y
	void ResizeMap(); // volat pote, co se zmeni vlastnosti mapy. Dojde k Reregistraci. znovu se umisti objekty (trigery), do mapy. Pokud je Obj mimo, bude zabit
	void RMPlaceObjAgain(CKerObject *ko); // znovu umisti objekty (triger), do mapy. Pokud je Obj mimo, bude zabit

	void SetScrollCenter(int x, int y); // nascrolluje tak, aby bod x,y bul ve stredu
	void WindowScroll(int dx, int dy,UI time); // zaplne plynule scrollovani okna
	// Funkce pro scrolling se zasekavaji tak, aby nebylo videt za kraj mapy
	void CalcScrollDistance(int x, int y, int *dx, int *dy); // Funkce zmensi vzdalenost dx a dy, pokud uz neni treba scrollovat.(jsem zarazen o okraj)
	void RefreshScroll() {SetScrollCenter(ScrollX, ScrollY);} // naskrolluje tam kde to ma kernel ulozeno

	CKerArrInt * CellColisionB(OPointer Obj, CKerArrInt *OutPutBuff = 0, int dx=0, int dy=0, int dz=0); // vrati pole trojic souradnic x y z bunek do kterych objekt zasahuje. Funkce bud vyuzije OutPutBuff, nebo sveje vzdy stejne pole v tompripade nedealokovat
	void DeleteOutOfMapCalls(CKerArrInt &Buff); // z pole odstrani bunky, ktere jsou mimo mapu
	int IsCellInMap(int cx, int cy, int cz); // 1 Ano, 0 Ne
	int IsPixelInMap(int x, int y, int z) { return FindCell(x,y,z,x,y,z); } // 1 Ano, 0 Ne
	int IsObjInCollision(int Codeline, OPointer obj, int dx=0, int dy=0, int dz=0);  // vrati 1 pokud objekt koliduje, jinak 0. dx dy dz je relativni posun souradnic objektu., vrati 1 take pokud je obj mimo mapu
    CKerArrObject *FindCollidingObjs(int CodeLine, OPointer obj, CKerArrObject *ret=0, int dx=0, int dy=0, int dz=0); // Vrati seznam objektu se kterymi objekt koliduje. dx dy dz je relativni posun souradnic objektu. V promenne ret muzes urcit, kam se bude vracet, jinak fce pole alokuje

	CKerArrObject *GetObjsFromCell(int CodeLine, int cx, int cy, int cz, CKerArrObject *RetObjs=0);  // fce se zepta mapy, jake objekty jsou na dane bunce. Fce muze ale nemusi(!) vyuzit pole RetObjs pro vraceceni objektu. V pripade ze fce pole naalokuje nekde jinde je treba ho dealokovat
	CKerArrObject *GetObjsFromCellSet(int CodeLine, CKerArrInt *CellsArray, CKerArrObject *RetObjs=0); // fce se zepta mapy, jake objekty jsou na danych bunkach. Pole ma tvar B1x,B1y,B1z, B2x,B2y,B2z, ... Fce muze ale nemusi(!) vyuzit pole RetObjs pro vraceceni objektu. V pripade ze fce pole naalokuje nekde jinde je treba ho dealokovat

	int FindCell(int x, int y, int z, int &cx, int &cy, int &cz); // zjisti do ktere bunky bod x,y,z patri. Vrati 0, jestlize jsi mimo mapu, jinak 1
    void FindCellPosition(int cx, int cy, int cz, int &x, int &y, int &z);  // Najde centralni pozici bunky

	void ReadColisionGroups(CKerObject *ko, CKerName **AddGr, CKerName **SubGr, CKerName **FceGr);	// vrati mnoziny pro vypocet kolizi, je treba volat getaccesstoobject
	void ReadObjCoords(CKerObject *kObj, int &x, int &y, int &z); // precte z objektu souradnice ma-li je (jinak vraci nuly). Napred je treba zavolat GetAccessToObj
	void WriteObjCoords(CKerObject *kObj, int x, int y, int z);	  // zapise do objektu souradnice ma-li je.  Napred je treba zavolat GetAccessToObj
	void ChangeObjCoords(int CodeLine, OPointer obj, int x, int y, int z=0);	  // zmeni souradnice objektu, objekt bude pripadne premisten.	
	int FindObjCell(CKerObject *kObj, int &cx, int &cy, int &cz); // ze souradnic zjisti primarni bunku objektu. Napred je treba zavolat GetAccessToObj, vrati nulu v pripade chyby out of map
	int IsObjInMap(OPointer ObjPtr) // zjisti zda je objekt umisten v mape
		{ CKerObject *kobj=KerMain->Objs->GetObject(ObjPtr); if (kobj&&kobj->elements) return 1; else return 0;}

	void PlaceObjToMap(int CodeLine, OPointer obj);	// umisti objekt do mapy
	int GPlaceObjToMap(CKerObject *ko, CFSRegKey *k); // umisti objekt do mapy pri nahravani hry (nahraje PlacedInfo a vytvori rafiku)
	void PlaceObjToMapWithKill(int CodeLine, OPointer obj);	// pred umistenim znici vsechny kolidujici objekty. umisti objekt do mapy
	void RemoveObjFromMap(int CodeLine, OPointer obj); // odebere objekt z mapy
	void MoveObjTo(int CodeLine, OPointer obj, int x, int y, int z);  // okamzite posune objekt na nove souradnice.
	void MoveObjRel(int CodeLine, OPointer obj, int relx, int rely, int relz);  // okamzite posune objekt relativne.

	CKerArrObject *GetTriggers(CKerArrInt *CellsArray, CKerArrObject *ObjectArray=0); // najde vsechny triggery v oblasti. Druhym parametrem se da specifukovat pole, kam funkce bude vracet. Jinak je pole alokovano

	int DownDivide(int a, int b) { // provede a / b. S tim ze zaokrouhli vzdy dolu. B musi byt kladne.
		return a<0 ? (a-b+1)/b : a/b;
	}
	void MoveMovingObjs();  // U vsech plynule pohybujicich se objektu se aktualizuji jejich pozice v mape. tuto fci vola RunTurn na acatku kazdeho tahu
private:
	void MoveObjTo2(int CodeLine, CKerObject *ko, OPointer obj, int x, int y, int z, int MoveGEnElemToo=1);  // okamzite posune objekt na nove souradnice. MoveGEnElemToo vyuziva kontroler plynuleho pohybu, nenastavovat
	void GetNumberOfCells2();	// spocita pocty bunek a prvni bunku
	CKerArrInt CollBuffB;		// pole vyuzivane fci CellCollisonB
	CKerMoveTimer *Movers;		// seznam kontroleru pro plynule pohyby
	CKerMapTrigger **Triggers;	// Mapa trigeru

	void DeleteTriggers(int Kill=0);
	void NewTriggers() {
		int f,count = NumCellsx*NumCellsy*(upperlevel-lowerlevel+1);
		Triggers = new CKerMapTrigger*[count];
		for (f=0; f<count; f++) Triggers[f] = 0;
	}
	void PlaceTriggerToMap(int CodeLine, CKerObject *ko, CKerArrInt *CellsBuff);	// umisti trigger do mapy
	void RemoveTriggerFromMap(CKerObject *ko); // odebere trigger z mapy
	void PlaceTriggerToMap2(CKerObject *ko, CKerArrInt *CellsBuff); // umisti cast triggeru do mapy (pro MoveTo)
	void RemoveTriggerFromMap2(CKerObject *ko, CKerArrInt *CellsBuff); // odebere cast triggeru z mapy
	int TestTrigger(CKerObject *kTrigger, CKerObject *ko); // otestuje kolizi trigeru a objektu

	void CalcScrollWindowXY(int x, int y);
	int ScrollX, ScrollY;
	int ScrollWX, ScrollWY;
};


#endif



