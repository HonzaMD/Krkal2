//////////////////////////////////////////////////////////////////////////////
///
///		Kernel - O B J E C T S
///
///		Informace o Bezicich objektech. Pristup k jejich Metodam a Datum.
///		A: Honza M.D. Krcek
///
///////////////////////////////////////////////////////////////////////////////

#ifndef KEROBJECTS_H
#define KEROBJECTS_H

#include "types.h"
#include "KerConstants.h"

// velikost hashovaci tabulky pro zive objekty
#define OBJECT_HT_SIZE 16384
#define OBJECT_HT_AND (OBJECT_HT_SIZE-1)

typedef CDynamicArrayV<OPointer,32> tKerLoadedObjects; // Promenne pole OPointeru




class CKerObjs;
class CGEnElement;
struct CKerAutoConnect;
class CKerName;
class CKerNamesMain;
class CFSRegister;
class CFSRegKey;
class CTexture;
class CKerMain;



/////////////////////////////////////
// Hodnota promenne zakladniho typu:
struct CKerValue {
	CKerValue() { Dint=0; typ=eKTvoid; }
	~CKerValue(); 
	union {		
		int Dint;
		unsigned char Dchar;
		double Ddouble;
		CKerName *Dname;
		OPointer Dobject;
		char *Dstring;
		CKerArrChar *Achar;
		CKerArrDouble *Adouble;
		CKerArrInt *Aint;
		CKerArrName *Aname;
		CKerArrObject *Aobject;
	};
	int typ;
};


///////////////////////////////////////////////////////////
///
///		POPIS TYPU OBJEKTU
///
///////////////////////////////////////////////////////////

// Popis argumentu konkretni metody (u direct call argumenty nejsou popsany)
struct CKerParam {
	CKerParam() { Name=0; Type=0; Offset=0;}
	CKerName *Name;		// KSID jmeno
	int Type;			// typ
	int Offset;			// Kde je argument umisten na zasobniku. pocita se relativne vzhledem k KerContext->Params
	CKerValue DefaultValue;
};


// KOnkretni metoda konkretniho typu objektu
struct CKerMethod {
	CKerMethod() { Compiled=0; Safe=1; Function=0; Name=0; Jump=0; Params=0; NumP=0; ParamCL=0; PCLnames=0; ReturnType = eKTvoid; ParamSize=0; MethodName = 0; PCLstart = 0; PCLstop = 0; ParentObj=0;}
	~CKerMethod() { 
		int f;
		SAFE_DELETE_ARRAY(Params);
		SAFE_DELETE_ARRAY(MethodName);
		if (ParamCL) { 
			for (f=0;f<PCLstop;f++) SAFE_DELETE_ARRAY(ParamCL[f]);
			SAFE_DELETE_ARRAY(ParamCL);
			SAFE_DELETE_ARRAY(PCLnames);
		}
	}
	char *MethodName;	// textove jmeno metody (KSM)
	char Compiled;		// Kompilovana?
	char Safe;			// Safe=1. Direct=2
	CKerName *Name;		// KSID jmeno
	CKerName *ParentObj; // odkud byla metoda zdedena?
	void (*Function)(); // Pointr na fci. Nenula pro kompilovane
	int Jump;			// index do pameti interpretu. Pro interpretovane
	int NumP;			// Pocet argumentu
	int ReturnType;		// typ navratove hodnoty
	int ParamSize; // Velikost Navratove hodnoty + Parametru na zasobniku (Nepocita se vel infa o parametrech)
	CKerParam ***ParamCL;	// seznam seznamu parametru, na ktere se prevede konkretni KSID jmeno (podle HT)
	CKerName **PCLnames;	// Perfektni Hashovaci tabulka. 0-nic, KSID jmeno - na teto pozici je toto jmeno
	int PCLstart, PCLstop;	// start - posud indexace v Hash tabulce (na jakem indexu HT zacina); stop - velikost HT. pokud jmeno je mimo meze, neni v HT
	CKerParam *Params;  // seznam argumentu
};


// Popis promenne o kterou se zajima kernal (editor)
struct CKerOVar {
	CKerOVar() { Offset=0; KSVar=0; Type=0; Use=0; Name=0; NameStr=0; UserName=0; Comment=0; LimintsCfg=0; LimitsList=0; DefaultValue=0; DefaultMember=0; LimitsListCount=0; EditType=0; NamesMask=0xFFFFFFFF; ArraySize=1;}
	~CKerOVar() {
		SAFE_DELETE_ARRAY(NameStr);
		SAFE_DELETE_ARRAY(UserName);
		SAFE_DELETE_ARRAY(Comment);
		SAFE_DELETE_ARRAY(LimitsList);
		SAFE_DELETE(DefaultValue);
		SAFE_DELETE(DefaultMember);
	}
	union {
		int Offset;		// offset pro interpretovanou prom
		void *_KSVar;	// Pokud je to promenna zadana skripty za behu: KSVar ukazuje sem, Tohle ukazije na data
	};
	void **KSVar;	// pointer na adresu promenne v KS, 0-prom je v IS
					// pouziti pro globalni promenne: adresa ukazuje na polozku v sekci GLOBALS
	int Type;		// typ. Pokud je typ void, jde o abstraktni polozku. Jeji vyznam je upresnen v EditType.
	int ArraySize;	// pocet prvku, pokud je promenna pole (u nepoli ja ArraySize nastavena na 1)
	int Use;		// popis promenne: 1.bit – loaduje se z levlu; 2.bit – prom. ovlivòující automatismus
							// další bity – význam pro obj mapa, umísování do levlu ...
							// podle eKerVarUses
	char *NameStr;	// Jmeno, jak se to jmenuje ve skriptu
	CKerName *Name; // jmeno pro typ objektu (pro Globalni objekt) nebo jmeno promenny ovladajici automatismus
					// nebo jmeno konstruktoru pro scripted polozku

	// Tagy Pro Editor:
	int EditType;	// upresneni typu
	int NamesMask;  // Ktere typy jmen me zajimaji?
	char *UserName; // uzivatelske jmeno (nepovinne)
	char *Comment;  // komentar (nepovinne)
	int LimintsCfg;	// konfigurace pouziti limitu - 0 - zadne limity
	CKerValue *DefaultValue; // default cele promenne
	CKerValue *DefaultMember; // default pro jeden field v poli
	CKerValue *LimitsList;	// popis limitu, bud horni a dolni mez, nebo jen jedna mez, nebo vycet povolenych hodnot
	union {
		int LimitsListCount;    // pocet polozek v poli
		int ItemID;				// ID predavane konstruktoru Skriptovatelne polozky
	};
};


// Popis objektoveho TYpu
struct CKerObjectT {
	CKerObjectT() { AllocKSVG=0; SetObjectVar=0; ISDataSize=0; Name=0; Methods=0; NumM=0; MethodCL=0; MCLnames=0; MCLstart=0; MCLstop=0; OVars=0; NumVars=0; EditTag=0; SpecificKnownVars = 0; KSVGsize = 0; DefAuto = 0; EditTex=0; AutoVars = 0; NumAVars = 0; AName=0; UserName=0; Comment=0;}
	~CKerObjectT() { 
		int f;
		SAFE_DELETE_ARRAY(Methods);
		SAFE_DELETE_ARRAY(UserName);
		SAFE_DELETE_ARRAY(Comment);
		if (MethodCL) { 
			for (f=0;f<MCLstop;f++) SAFE_DELETE_ARRAY(MethodCL[f]);
			SAFE_DELETE_ARRAY(MethodCL);
			SAFE_DELETE_ARRAY(MCLnames);
		}
		SAFE_DELETE_ARRAY(OVars);
		SAFE_DELETE_ARRAY(SpecificKnownVars);
		SAFE_DELETE_ARRAY(AutoVars);
	}
	CKerName *Name;			// Jmeno typu
	CKerName *AName;		// Bratrske jmeno pro automatismy (_KSAD_) - tvoreno automaticky
	char *UserName;			// uzivatelske jmeno (nepovinne)
	char *Comment;			// komentar (nepovinne)
	int EditTag;			// Edit tag - editovatelny, umistitelny, ...
	CTexture *EditTex;		// Muze tu byt textura pro ikonu v editoru
	void *(*AllocKSVG)();	// Funkce pro alokaci KSVG - tedy objektovych promennych v kompilovanem svete
	void (*SetObjectVar)(void *KSVG);  // zpristupni promenne z KSVG
	int ISDataSize;			// Velikost dat objektu v Interpretovanem svete
	int KSVGsize;			// Velikost dat objektu v Compilovanem svete
	int NumM;				// Pocet metod
	CKerMethod ***MethodCL;	// seznam seznamu metod, na ktere se prevede konkretni KSID jmeno (podle HT)
	CKerName **MCLnames;	// Perfektni Hashovaci tabulka. 0-nic, KSID jmeno - na teto pozici je toto jmeno
	int MCLstart, MCLstop;	// start - posud indexace v Hash tabulce (na jakem indexu HT zacina); stop - velikost HT. pokud jmeno je mimo meze, neni v HT
	CKerMethod *Methods;    // pole metod objektu
	int NumVars;			// pocet promennych zakladniho typu, ke kterym ma kernel/editor pistup
	CKerOVar *OVars;		// pole promennych zakladniho typu
	CKerOVar **SpecificKnownVars;	// pole pointru na OVars, jejichz vyznamy Kernel zna - pole muze byt prazdne nebo ma velikost KERVARUSESSIZE, undexuje se pomoci eKarVarUses
	CKerOVar **AutoVars;	// pointry na promenne ovlivnujici automatismy
	int NumAVars;			// pocet promennych ovlivnujici automatismy
	CKerName *DefAuto;		// Defaultni nastaveni automatismu

	void FindAutoVars();	// Inicializuje AutoVars
};



struct CKerOSRPointers {
	CKerOSRPointers(CFSRegKey *_key, int _pos, OPointer _obj, CKerOSRPointers *_next) {
		next = _next; pos = _pos; key = _key; obj = _obj;
	}
	CKerOSRPointers *next;
	CFSRegKey *key;
	OPointer obj;	// objekt u ktereho pointer je. Nula znamena globalni promennou
	int pos;
};


struct CKerObjSaveRec {
	CKerObjSaveRec();
	~CKerObjSaveRec();
	void AddPointer(CFSRegKey *key, int pos, OPointer obj) {	// prida pozici dalsiho pointeru na tento objekt.
		pointers = new CKerOSRPointers(key,pos,obj,pointers);
	}
	CKerArrObject *SaveGraph;  // graf konstruovany pri sejvovani levlu. (pole hran, Null znamena, ze objekt neni zarazen do grafu)
	CFSRegister *reg;			// sem se objekt sejvne
	int Tag;
	CKerOSRPointers *pointers;  // informace o tom kam se sejvly pointery na tento objekt.
	char *GlobalVar;
	char *shortcut;		// text s popiskem shortcutu (pro GameLoad) - nevlastnim to
};




//////////////////////////////////////////////////////////////////////////
///
///		BEZICI INSTANCE OBJEKTU
///
//////////////////////////////////////////////////////////////////////////

struct CKerObject {
friend CKerObjs;
friend CKerMain;
	CKerObject(CKerName *type, OPointer ForceObjPtr=0);
	~CKerObject();

	CKerObjectT *Type;		// Typ objektu - vzdy nastaveno (alespon na DefaultObject
	void *KSVG;				// Data v KS, 0-kdyz objekt v KS zadna data nema
	UC *ISData;				// Data v IS, 0-kdyz objekt v IS zadna data nema
	OPointer thisO;			// this
	CGEnElement **elements;	// Pokud je objekt umisten tak je tam nenulovy GenElement, jinak 0
	int Tag;				// znackovaci tag pro ruzne ucely
	CKerObjSaveRec *SaveRec; // info pro sejvovani
	struct CKerPlacedInfo *PlacedInfo;  // zapamatovany stav souradnic a kolizni cfg pri umisteni
private:
	CKerObject *next;		// dalsi objekt na teto pozici v HT zivych objektu
	void DeleteFromHT();
};





//////////////////////////////////////////////////////////////////////////
///
///		VYPOCET PERFEKTNI HASHOVACI TABULKY
///
//////////////////////////////////////////////////////////////////////////


/// Spojak seznamu metod (seznamy jsou v tomto spojaku poze docasne v prubehu vypoctu)
struct CKerMList {
	CKerMList(CKerMList *Next, int Pos, CKerMethod **Methods, CKerName *Name) {
		next=Next; pos=Pos; methods=Methods; name=Name;
	}
	int pos;			// pridelena pozice
	CKerName *name;		// jmeno (klic)
	CKerMList *next;
	CKerMethod **methods;  // seznam volanych metod
};


/// Spojak seznamu parametru (seznamy jsou v tomto spojaku poze docasne v prubehu vypoctu)
struct CKerPList {
	CKerPList(CKerPList *Next, int Pos, CKerParam **Params, CKerName *Name) {
		next=Next; pos=Pos; params=Params; name=Name;
	}
	int pos;			// pridelena pozice
	CKerName *name;		// jmeno (klic)
	CKerPList *next;
	CKerParam **params; // seznam parametru, kam budu predavat
};



///////////////////////////////////////////////////////////////////////////////
///		Trida, ktera pocita perfektni hashovani pro metody a parametry
///		Algoritmus: vezmu jmeno metody, ke kazdymu objektu vygeneruju
///		seznam metod KSM, ktere se pod timto jmenem maji zavolat (seznam muze byt prazdny)
///		Tyto seznamy hashuju (klic je KSID jmeno metody)
///		Hashovani: Najdu prvni pozici, kde jeste zadny objekt nema dany seznam,
///		na tuto pozici seznam ulozim, u jmena si zapomatuju pozici.
class CKerCalculatePHT {
friend CKerObjs;
private:
	CKerCalculatePHT();		// inicializace pomocnych promennych
	~CKerCalculatePHT();	// dinicializace
	void find_pos_o(int &pos);	// najdi kandidata na volnou pozici (zacina se od nuly (zadat), kdyz se pozice po druhem volani nezmeni, je volna
	void place_at_pos_o(int pos, CKerName *name);  // proda seznamy tohoto jmena na danou pozici (kazdy objekt siseznamy nyni pamatuje ve spojaku olist)
	CKerMethod ** CreateOMList(CKerName *name, CKerObjectT *o); // najdu na jake metody se jmeno pro dany objekt prevede. Funkce vraci pole s pointry na metodu, zakoncene nulou
	void calc_o();			// provede vypocet PHT pro metody objektu
	void find_pos_m(int &pos);  // pro parametry - na hledani pozice
	void place_at_pos_m(int pos, CKerName *name);  // umisteni na pozici, zarazeni do spojaku
	CKerParam ** CreateMPList(CKerName *name, CKerMethod *m);  // Nalezeni seznamu parametru, do kterych se dany parametr preda
	void calc_m();			// provede vypocet PHT pro parametry metod

	CKerMethod **ms;		// vsechny metody (vsech objektu)
	int NumM;				// pocet metod
	int *mstart, *mstop;	// pro kazdou metodu: Pozice prvniho prvku ze spojaku, velikost spojaku, kdyby byl v poli (pozice posledniho - pozice prvniho + 1)
	CKerPList **mlist;		// spojak pro kazdou metodu
	CKerParam ***mplist;	// Do tohoto pole prubezne pocitam seznamy parametru, na ktere se jmeno parametru prevadi
	int NumO;				// pocet objektu
	CKerObjectT *os;		// vsechny objekty
	int *ostart, *ostop;	// pro kazdy objekt: Pozice prvniho prvku ze spojaku, velikost spojaku, kdyby byl v poli (pozice posledniho - pozice prvniho + 1)
	CKerMList **olist;		// pro kazdy objekt jeho spojak
	CKerMethod ***omlist;   // Do tohoto pole prubezne pocitam seznamy metod, na ktere se jmeno metody prevadi
};




//////////////////////////////////////////////////////////////////////
///
///		HLAVNI OBJEKT PRO PRACI S OBJEKTY
///
//////////////////////////////////////////////////////////////////////


class CKerObjs {
friend CKerObject;
friend CKerCalculatePHT;
friend CKerMain;
public:
	CKerObjs();
	~CKerObjs() {
		// zmazu HT a vsechny objekty v ni. estruktory se nevolaji
		int f;
		for (f=0;f<OBJECT_HT_SIZE;f++) while (HT[f]) delete HT[f];
		SAFE_DELETE_ARRAY(ObjectTypes);
	}
	int LoadObjects(CFSRegister *o);	// nahraje info o typech objektu
	void CalculatePHT() {				// spocita perfektni hashovaci tabulky
		CKerCalculatePHT *calc = new CKerCalculatePHT();
		calc->calc_o();
		calc->calc_m();
		delete calc;
	}
	CKerObject *GetObject(OPointer optr);  // Prevede OPointer na CKerObject*

	tKerLoadedObjects LoadedObjects;	// Promenne pole pro naloadovane objekty (OPointry). Napred jsou ty staticke.
										// Kdyz je objekt zrusen OPointer v poli zustava
	void LOShakeOff();	// vyradi z LoadedObjects nepozivane objekty, pole setrese
	int LOShakeOffSize;	// velikost pole po setreseni. Dosahne/li pole dvojnasobne velikosti, melo by se na nej zase zavolat LOShakeOff
	CKerObjectT *ObjectTypes;	// Pole vsech typu objektu
	int NumObjectT;				// Pocet typu Objektu
	int LoadOVar(CFSRegKey *_km, CKerOVar *OV, CKerOVar ***SpecKnownVars, int GLOBAL=0);/// Nahraje informace o jedne promenne
	int CheckVarGroups(CKerOVar **OVars, int NumOV);  // otestuje spravnost skupinovych promennych
	int GetCounter() { return Counter; }  // vrati stav citace
	void WalkThrowAllObjs(void (*Fce)(CKerObject *ko)); // zavola funkci postupe na vsechny objekty. Objekty je mozno zabijet
	void KernelDump(char *file = "KernelDump");
private:
	CKerObjectT *DefaultObjectT;	// Defaultni typ. Vzdy je.
	CKerName *CKerObjs::DuplicateObjName(CKerName *name); // Vytvori Dvojnika objektoveho jmena pro automatismy
	int LoadObjectsLD(CFSRegKey *l, CKerValue *P, int Type);	// pomocn fce, nahrava defaultni hodnoty argumentu
	OPointer Counter;			// Citac OPointeru. Pri kazdem tvoreni objektu je prideleno nove cislo, citac je zvysen. Objek nikdy nedostane cislo, ktere uz bylo.
	int GetNewObjectPointer() { return Counter++; }  // Prirazeni noveho cisla
	CKerObject *HT[OBJECT_HT_SIZE];	// Hashovaci tabulka zivych objektu. Hashuje se podle OPointeru
};

#endif
