//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - A u t o s
///
///		Prace s grafickymy automatismy, vyber grafiky pro objekty, graficke styly ..
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KERAUTOS_H
#define KERAUTOS_H

#include "types.h"
#include "KerConstants.h"

class CKerName;
class CLightTex;
class CGEnResource;
class CKerAutosMain;
class CTexture;
struct CKerObject;
struct CKerObjectT;

class CKerArrObject;


// typy automatismu (typy uzlu)
enum eKerAutoTypes {
	eKATpicture,	// koncova grafika - koncovy uzel
	eKATconnect,	// navaznost
	eKATvar,		// rozhodnuti podle promenne
	eKATrandom,		// rozhodnuti podle randomu
	eKATpattern,	// vzor - rozhodnuti podle mista vyskytu
	eKATname,		// zavedeni jmena (pro polozky v zdrojovem registru)
	eKATexternName,	// pouziti externiho jmena (pro polozky v zdrojovem registru)
	eKATvarAdd,		// pridani dalsich vetvi do var auta
};




/////////////////////////////////////////////////////////////////////
///
///		C K e r A u t o C o n n e c t
///		Popis Rozhodovani podle navaznosti
///
/////////////////////////////////////////////////////////////////////

struct CKerACState {
	CKerACState() { GLook=0; ko=0; 
	//QEnd=0; stav=1; TrayTest=0;
	}
	int GLook; // id toho co jsem nasel na policku (0 znamena nic)
	CKerObject *ko; // nalezeny objekt
//	int stav; // 0 navazuje, 1 nenavazuje, 2 testovano navazuje, 3 testovano nenavazuje
//	CKerObject *QEnd; // konec testovaci fronty, ktera zacina objektem ko.
//	int TrayTest;	// zda mam zkusit testovat
};

struct CKerAutoConnect {
	CKerAutoConnect(int sonsnum, int gridsize);
	~CKerAutoConnect();
	CKerName *Process(CKerObject *ko, UI NoConnectMask, CKerObject **QueueEnd=0, CKerObject **QueueStart=0, int Reset=0, CKerObject *Test=0); 
		// provede automatismus (nad objektem ko, Reset 0 - sireni, Reset 1 - Oprava navaznosti), Reset 2 - test
	CKerObject *IsObjInArray(CKerName* name, CKerArrObject *objs, CKerObject *Me, int relx, int rely, int relz);  // zjisti zda je hledany objekt v seznamu
	void CalculateFastGoDown(int SHIFT);  // vypocita dabulku rychleho sestupu. Diky tomu se da automatismus vyresit jednim dotazem
	DWORD mask; //	8 4 7 
				//	1 9 3	- stredove patro (prvnich devet bitu)	
				//  5 2 6
				//  bity 10 az 27 jsou horni a dolni patro. Viz CACellPositions
	int GridCfg; // 0-mrizka zjistovana z masky, 1 - z relativnich pozic v bunkach, 2 - z rel poz. v pixelech - brano s presnosti na pixel, 4,5,6 - brano s presnosti na bunku
	int Agresivity; // 1 - velka, podlahova agresivita, 0 - mala, stenova
	int GridSize; // pocet prohledavanych pozic
	int *GX,*GY,*GZ; // pole popisijici mrizku, muzou byt prazdna
	CKerName ***GFNames; // pole poli pointeru na vyhledavana jmena. Hlavni pole ma velikost GridSize, Dalsi pole maji promennou velikost, konci Nullem
	int *GFShift; // jak mam informaci z bunky shiftnou pri skladani vysledku
	int *FGoDown;  // rychly pristup dolu pres shiftovani. Jen pro pole do velikosti 64kB Je to poradi do synu
	UC *NoErrorMask;     // bitova maska - 1 - odpovidajici FGoDown je bez chyb
	int SonsNum;
	int **MaskLook;		// vzhled masky u jednotlivych synu
	CKerName **GoDown;	// na syna
};



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/// popis rozhodovani podle promenne
struct CKerAutoVar {
	CKerAutoVar() {a=0; GoDown=0; }
	CKerName *a;   // interval
	CKerName *GoDown;
};

/// popis rozhodovani podle randomu
struct CKerAutoRandom {
	CKerAutoRandom() {pst=1; GoDown=0; }
	double pst;   // horni, dolni mez intervalu
	CKerName *GoDown;
};

/// popis koncoveho uzlu s obrazky
struct CKerAutoPicture {
	CKerAutoPicture() { texture=0; StartTime=-1; EndAction=0; EndTimeOut=0; TimeOutRandom=0;}
	~CKerAutoPicture();
	CGEnResource *texture;
	int StartTime;  // absolutni cas spusteni animace, -1 -- animace se pousti od okamziku zmeny automatismu
	int EndAction; // 0 - none, 1 - reset auto, 2 - load anim
	int EndTimeOut;  // po jak dlouhe dobe se provede ukoncovaci akce
	int TimeOutRandom;  // na cas se hodi rovnomerny random od ETO - TOR do ETO + TOR, 0-zadny random
};




///////////////////////////////////////////////////////////////////////
///
///		C K e r A u t o
///		Popis automaitickeho uzlu
///
///////////////////////////////////////////////////////////////////////


class CKerAuto {
friend CKerAutosMain;
public:
	CKerAuto(int type, int sonsnum);
	~CKerAuto();
	int Type;				// typ uzlu
	CTexture *EditTex;		// zde muze byt textura, kterou editor vyuzije jako ikonu
	int SonsNum;			// pocet synu
	union {
		struct {  // variable
			CKerName *VarName; // jmeno promenne
			CKerAutoVar *intervals;
			CKerName *Default; // default, kdyz vsechny moznost selzou
		};
				 // random
		CKerAutoRandom *psti;
				 // connect
		CKerAutoConnect *Connect;  
		struct { // pattern
			CKerName **GoDown;
			int patA, patB, patC;
		};
				 // picture;
		CKerAutoPicture *textures;
	};
	CKerAuto *next;
};





/////////////////////////////////////////////////////////////////////
///
///		C K e r A u t o s M a i n
///		Hlavni objekt pro praci s automatismy
///
/////////////////////////////////////////////////////////////////////



class CKerAutosMain {
friend CKerAuto;
friend CKerObject;
public:
	CKerAutosMain() {
		autos = 0;
		ToDo = 0;
		SaveReg=0; SaveVer=0;
	}
	~CKerAutosMain();

	int OpenFileToSaveAutos(const char *file, const char *version); // Je treba volat drive, nez se zacnou za behu pridavat nove automatismy. Otevreni existujiciho souboru - verzi nastav na NULL. vytvoreni noveho souboru - Verzi mi predej. Vrati 1 OK, 0 - chyba. Pokud je uz neco otevreny, zmeny budou zapomenuty!
	int GetSaveAutosState() { return (int)SaveReg; }
	int SaveChangedAutos(); // Prida do souboru zmeny. 1 OK, 0 chyba
	void AddEasyAuto(CKerObjectT *object, CKerName *Auto); // prida k objektu existujici automatismus. Je treba mit otevreny soubor, kam se bude sejvovat
	void AddEasyAuto(CKerObjectT *object, char *path); // prida k objektu automatismus, ktery vytvori ze zadaneho obrazku nebo animace (.tex, .ani) Je treba mit otevreny soubor, kam se bude sejvovat
	CKerName *AddEasyAuto(char *path); // prida automatismus, ktery vytvori ze zadaneho obrazku nebo animace (.tex, .ani) Je treba mit otevreny soubor, kam se bude sejvovat

	void LoadAllAutos();  // nahraje vsechny soubory s atomatismama po ceste do rootu
	int LoadAutos(const char *file, int ChangeDirToFile=1);  // nahraje konkretni soubor s automatismama
	void CheckAutosIntegrity();		// overi spravnost automatismu
	void AssignAutosToObjs();		// prideli dafaultni automatismy k objektum

	CKerName *ProcessAuto(CKerObject *kobj, CKerObject **QueueEnd=0, CKerObject **QueueStart=0, int Reset=0, CKerObject *Test=0, int RandomReset=0); //pozor, rusi kontext!
		// provede automatismus (nad objektem ko, Reset 0 - sireni, Reset 1 - Oprava navaznosti), Reset 2 - test
	void ProcessAuto2(CKerObject *ko, int Remove=0,int ConnectionReset=1,int RandomReset=0);
	void ProcessToDoAutos();	// spocita vsechny automatismy v ToDo fronte
	void ChangeAktivAuto(CKerObject *ko, CKerName *_OldA, CKerName *_NewA); // zmeni grafiku podle toho, jak se zmenily automatismy
	void GEnProcessEnded(OPointer obj, int ProcessID);
private:
	void StornoQueue(CKerObject *OldQend);
	CKerObject *ToDo;		// seznam automatismu k provedeni
	CKerAuto *autos;		// seznam vsech nahranych rozhodovacich uzlu
	class CFSRegister *SaveReg;
	char *SaveVer;
};



#endif
