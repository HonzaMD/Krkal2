/////////////////////////////////////////////////////////////////////////////
///
///		K E R   N A M E S  
///
///		Definice jmen, zavislosti. Vyhodnocovani mnozinovych operaci.
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////////

/*
	Pouziti
	-------
	Vytvorte objekt CKerNamesMain. Ten si zapamatujte a drste si ho jako Interface na konkretni oblast jmen
	Pak muzete vytvaret jmena (CKerName) a pridavat a rusit zavislosti mezi nimi.
	Spoctete Matici (nepovinne) KerNamesMain->CreateMatrix();
	A tec muzete jmena porovnavat (Compare) Uz nemente strukturu jmen! Matice
	by se pocitala znova = neefektivni!
	Zruste objekt KerNamesMain (Zrusi se i vsechna jmena)
*/


#ifndef KERNAMES_H
#define KERNAMES_H

#include "types.h"
#include "KerConstants.h"

#define KER_NAMES_HTS 512

class CKerName;
class CKerNameList;
class CKerCalculatePHT;
class CKerAuto;
class CKerAutosMain;
class CComOBObject;





/////////////////////////////////////////////////////////////////////
///
///		C K e r N a m e s M a i n
///
/////////////////////////////////////////////////////////////////////

// Hlavni Objekt Pro jmena. Je treba ho vytvorit pred praci se jmeny
// Ma u sebe seznam vsech jmen a hashovaci tabulku na jmena
// Zrusenim objektu KerNamesMain se zrusi i vsechna jmena
class CKerNamesMain {
friend CKerName;
friend CKerCalculatePHT;
friend CKerAutosMain;
public:
	CKerNamesMain();
	~CKerNamesMain();
	void CreateMatrix();  // je treba zavolat pred provadenim porovnavani (ale nemusi, vola se i automaticky)
	int FindMethodsAndParams(); // najdi vsechny jmena metod a parametru a oznac je
	int Compare(CKerName *name1,CKerName *name2); //0 - neporovnatelne, 1 - n1 je otcem n2, 2 - n1 je synem n2, 3 - cyklus, identifikace poradovymi cisly
	CKerName * GetNamePointer(char *name) {return (CKerName*)HashTable->Member(name);} // 0 - error nezname jmeno
	int TestPointerValidity(CKerName *name); // vrati 1 pokud je pointer platny, 0 pokud je neplatny nebo NULL. (funkce prochazi vsechny jmena)
	CKerNameList *FindLayer(CKerName *From, int (*FilterFce)(CKerName *name) = 0, int Type = -1, int direction = 1 ); //Vytvori pole jmen, ktere jsou nejvyse pod From (defaultni direction, direction 0 jede nahoru).
			// funkce vytvori seznam, ktery je treba zrusit, Zadejt From nula, kdyz chces prohledavat od "korenu". Je mozne zadat FilterFci, ktera vrati jedna, jestlize je jmeno pro vystup zajimave, jinak nula. 
			// zadanim typu se budou hledat jen jmena zadaneho type (-1 vse)
	CKerNameList *FindSet(CKerName *From, int (*FilterFce)(CKerName *name) = 0, int Type = -1, int direction = 1 ); // Najde mnozinu jmen. Tzn vsechny jmena, ktera jsou pod zadanym jmenem (defaultni direction). / Nad zadanym jmenem - direction = 0. Je mozno specifikovat typ
			// from nastaveno na nulu znamena ze vyhledava vse. Vrati seznam jmen, ktery je treba zrusit
	CKerName *NameInCycle; // Promennou nastravi CreateMatrix, pokud najde cyklus. Kdyz neni cyklus, je tu nula
	CKerNameList *Names;  // seznam vsech jmen. Pristupuj pouze ReadOnly Modu! :)
private:
	int GetNumber() {  // prideluje nove poradove cislo
		Numbers++;
		return Numbers-1;
	}
	void ClearMatrix();
	void FindComponent(CKerName *n, CKerName *root, int &size); // funkce pro tvorbu matic
	void SearchDown(CKerName *n, char * Matrix);				// funkce pro tvorbu matic
	void SearchUp(CKerName *n, char * Matrix);					// funkce pro tvorbu matic
	int Numbers;     // citac pridelovanych poradovych cisel
	int MatrixesCalculated;  // zda je vypocitana matice a zda je aktualni
	int MatrixSize; // velikost matice (pocet sloupcu) nektere sloupce nemusi byt vyuzity
	char **Matrix;  // Matice = Pole o velikosti MatrixSize (=Numbers+1) pointeru na ruzne velka pole charu
					// Slouzi k rychlemu porovnani dvou jmen
					// Kdyz jsou jmena ve stejnych komponentach, tak informace o jejich vztahu bude v
					// Matrix[jm1->numer][jm2->MatrixPos]. Viz Funkce Compare
	CHashTable *HashTable; // Hash Table na vyhledavani cesel nebo pointeru na jmena podle stringu
	void FindLayerOutput(CKerName *name,CKerNameList **out , int direction ); // prida jmeno do vystupniho seznamu
};

////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////
///
///		C K e r N a m e L i s t   -   S e z n a m   j m e n
///
///////////////////////////////////////////////////////////////////////////

class CKerNameList {
public:
	void DeleteAll() {
		CKerNameList *l1,*l2;
		l1 = next;
		while (l1) {
			l2=l1;
			l1=l1->next;
			delete l2;
		}
		delete this;
	}
	CKerNameList(CKerName * Name, CKerNameList *Next) {
		name=Name;
		next=Next;
	}

	CKerName *name;
	CKerNameList *next;
};


////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
///
///		C K e r N a m e
///
///////////////////////////////////////////////////////////////////////

class CKerName : public CHashElem {
friend CKerNamesMain;
public:
	CKerName(char *Name, CKerNamesMain *NamesMain){
		KerNamesMain = NamesMain;
		SetName(Name);
		childs=0;
		parents=0;
		Component = 0; KerPHTpos = 0; MatrixPos = 0; matrixsize=0;
		KerNamesMain->Names = new CKerNameList(this,KerNamesMain->Names);
		AddToHashTable(KerNamesMain->HashTable);
		number = KerNamesMain->GetNumber();
		KerNamesMain->MatrixesCalculated=0;
		Type = eKerNTvoid; TypeInfo = 0;
		UserName=0; Comment=0;
	}
	virtual ~CKerName();
		
	int IsChild(CKerName *n) { return IsEdge(n,childs);}
	void AddChild(CKerName *n) { // prida hranu k synovi
		if (IsEdge(n,childs)) return;
		childs = new CKerNameList(n,childs);
		n->parents = new CKerNameList(this,n->parents);
		KerNamesMain->MatrixesCalculated=0;
	}
	int IsParent(CKerName *n) { return IsEdge(n,parents);}
	void AddParent(CKerName *n) { // prida hranu na otce
		if (IsEdge(n,parents)) return;
		parents = new CKerNameList(n,parents);
		n->childs = new CKerNameList(this,n->childs);
		KerNamesMain->MatrixesCalculated=0;
	} 
	void DeleteChild(CKerName *n) { // zrusi hranu k synovi
		FindAndDelete(&childs,n);
		FindAndDelete(&n->parents,this);
		KerNamesMain->MatrixesCalculated=0;
	}
	void DeleteParent(CKerName *n) { // zrusi hranu od otce
		FindAndDelete(&parents,n);
		FindAndDelete(&n->childs,this);
		KerNamesMain->MatrixesCalculated=0;
	}
	void DeleteAllEdges(); // zrusi vsechny hrany z tohoto a do tohoto jmena
	int Compare(CKerName *n); // 0 - neporovnatelne, 1 - n je otcem this, 2 - n je synem this, 3 - cyklus
	
	eKerNameType Type; // Typ jmena.
	union {								// Dodatecna informace ke jmenu zavisla podle typu jmena
		int TypeInfo;					// info - pro neznamy typ. Nevyuzivano, nastaveno na 0
		int KerPHTpos;					// pozice v perfektnich hashovacich tabulkach. pro metody a parametry
		struct CKerObjectT *ObjectType;	// Typ objektu. Pro objekty
		CKerAuto *Auto;					// informace o automatismu
		CComOBObject* objBrowserLink;	// Link na data object browseru (v editoru skriptu)
		struct CKeyStruct *KeyStruct;	// Info o skenkodu klavesy, prirazene tomuto jmenu
		class CMusicSample *Sound;		// zvuk
	};
	char * GetNameString() { return name; }
	char * GetShortNameString();  // vrati kratke jmeno, zbavene verzi, _KSID_ ... Fce pro string pouziva svuj buffer, nedealokovat
	char * GetUserName() { if (UserName) return UserName; else return GetShortNameString(); } // vrati uzivatelsky jmeno, pokud neni vrati kratke jmeno. vraceny string nemenit, nedealokovat
	char * UserName; // uzivatelske jmeno. zatim nedealokuju. Vlastni to objekty (pouze)
	char * Comment; // komentar. zatim nedealokuju. Vlastni to objekty (pouze)
	int HasChilds() { return childs!=0; }	// zda ma jmeno nejake potomky
	int HasParents() { return parents!=0; } // zda ma jmeno nejake predky
private:
	CKerNamesMain *KerNamesMain; // prostor jmen, do ktereho toto jmeno patri
	int IsEdge(CKerName *n,CKerNameList *p); // je v seznamu p jmeno n?
	void FindAndDelete(CKerNameList **p2,CKerName *name); // najde jmeno v seznamu a odstrani ho
	int number; // pridelene cislo sloupce matice
	int matrixsize; // nastaveno jen i roota a jen kdyz je matice spocitana (pres Component) - pocet sloupcu (velikost componenty)
	int MatrixPos; // pridelene cislo radku matice (nastaveno jen kdyz je matice spocitana)
	CKerName *Component; // pointr na root jmeno komponenty (vsechny jmena ze stejny komponenty maji stejneho roota) (Nastaveno jen kdyz je matice spocitana)
	CKerNameList *childs; //seznam synu
	CKerNameList *parents; //seznam otcu
};




#endif
