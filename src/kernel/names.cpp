/////////////////////////////////////////////////////////////////////////////
///
///		K E R   N A M E S  
///
///		Definice jmen, zavislosti. Vyhodnocovani mnozinovych operaci.
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "names.h"
#include "KerServices.h"

#include "musicmod.h"


//////////////////////////////////////////////////////////////////////
///
///		M e t o d y   O b j e k t u   C K e r N a m e
///
//////////////////////////////////////////////////////////////////////


CKerName::~CKerName()
{
	DeleteAllEdges();
	FindAndDelete(&KerNamesMain->Names,this);
	RemoveFromHashTable();
	KerNamesMain->MatrixesCalculated=0;
	SAFE_DELETE_ARRAY(name);
	if(Type==eKerNTsound)
		SAFE_RELEASE(Sound);
}


// Vyhleda jmeno name v seznamu p2, kdyz tam je, tak ho ze seznamu odstrani
void CKerName::FindAndDelete(CKerNameList **p2,CKerName *name) { 		
	CKerNameList *p3;
	while (*p2 && (**p2).name!=name) p2 = &(**p2).next;
	if (*p2) {
		p3=*p2;
		*p2=p3->next;
		delete p3;
	}
}



// vrati kratke jmeno, zbavene verzi, _KSID_ ... String je nove alokovan, je nutne destruovat.
char * CKerName::GetShortNameString() {
	 return KerServices.ConverNameToShort(name); 
}


// Odstrani vsechny hrany
void CKerName::DeleteAllEdges() {
	CKerNameList *p,*p3;
	CKerName *n;
	p = childs;  // zrusim odkazy mezi sebou u detmi (predpokladam jeden nebo zadny odkaz(nehlasim chybu))
	childs=0;
	while(p) {
		n=p->name;
		p3=p;
		p=p->next;
		delete p3;
		FindAndDelete(&n->parents,this);
	}
	p = parents;  // zrusim odkazy mezi sebou a predky
	parents=0;
	while(p) {
		n=p->name;
		p3=p;
		p=p->next;
		delete p3;
		FindAndDelete(&n->childs,this);
	}
	KerNamesMain->MatrixesCalculated=0;
}


// Je jmeno n v seznamu p?
int CKerName::IsEdge(CKerName *n,CKerNameList *p) {
	while (p) {
		if (p->name==n) return 1;
		p=p->next;
	}
	return 0;
}


// Porovnani
int CKerName::Compare(CKerName *n) { // 0 - neporovnatelne, 1 - this je otcem n, 2 - this je synem n, 3 - cyklus
	if (KerNamesMain->MatrixesCalculated==0) KerNamesMain->CreateMatrix();
	if (Component!=n->Component) return 0;
	return KerNamesMain->Matrix[number][n->MatrixPos];
}






////////////////////////////////////////////////////////////////////////
///
///		M e t o d y   O b j e k t u   C K e r N a m e s M a i n
///
///////////////////////////////////////////////////////////////////////

// Porovnani
int CKerNamesMain::Compare(CKerName *name1,CKerName *name2) {
	//0 - neporovnatelne, 1 - n1 je otcem n2, 2 - n1 je synem n2, 3 - cyklus, identifikace poradovymi cisly
	if (MatrixesCalculated==0) CreateMatrix();
	if (name1->Component!=name2->Component) return 0;
	return Matrix[name1->number][name2->MatrixPos];
}



// Zrusi Matici
void CKerNamesMain::ClearMatrix() {
	int f;
	if (Matrix) {
		for (f=0;f<MatrixSize;f++) SAFE_DELETE_ARRAY(Matrix[f]);
		SAFE_DELETE_ARRAY(Matrix);
	}
	MatrixesCalculated=0;
}



// Destruktor
CKerNamesMain::~CKerNamesMain() {
	CKerNameList *p=Names, *p2;
	ClearMatrix();
	while(p) {
		p2=p;
		p=p->next;
		delete p2->name;
	}
	delete HashTable;
}



// Konstruktor
CKerNamesMain::CKerNamesMain() {
	Names = 0;
	NameInCycle = 0;
	MatrixesCalculated = 0;
	Numbers = 0;
	Matrix = 0;
	MatrixSize = 0;
	HashTable = new CHashTable(KER_NAMES_HTS);
}



// Najde souvislou komponentu (hrany chape oboustrane) z vrcholu n
// Spocita jeji velikost (do size)
// V ramci komponenty prideli vrcholum poradova cisla (MatrixPos)
// Vsem vrcholum nastavi Component na root
void CKerNamesMain::FindComponent(CKerName *n, CKerName *root, int &size){
	CKerNameList *p;
	n->MatrixPos = size;
	size++;
	n->Component = root;
	p =  n->parents;
	while (p) {
		if (p->name->Component==0) FindComponent(p->name,root,size);
		p=p->next;
	}
	p =  n->childs;
	while (p) {
		if (p->name->Component==0) FindComponent(p->name,root,size);
		p=p->next;
	}
}



// Hleda vsechny potomky (Pro vypocet hodnot do matice)
void CKerNamesMain::SearchDown(CKerName *n, char * Matrix) { // +1
	CKerNameList *p= n->childs;
	int mp;
	while (p) {
		mp = p->name->MatrixPos;
		if (Matrix[mp]==0||Matrix[mp]==2) {
			if (Matrix[mp]==2) {
				if (KerMain) KerMain->Errors->LogError(eKRTEcyclusInKSID,0,p->name->GetNameString());
				NameInCycle = p->name;
			}
			Matrix[mp]++;
			SearchDown(p->name,Matrix);
		}
		p=p->next;
	}
}



// Hleda vsechny predky (Pro vypocet hodnot do matice)
void CKerNamesMain::SearchUp(CKerName *n, char * Matrix) {  // +2
	CKerNameList *p= n->parents;
	int mp;
	while (p) {
		mp = p->name->MatrixPos;
		if (Matrix[mp]==0||Matrix[mp]==1) {
			if (Matrix[mp]==1) {
				if (KerMain) KerMain->Errors->LogError(eKRTEcyclusInKSID,0,p->name->GetNameString());
				NameInCycle = p->name;
			}
			Matrix[mp]+=2;
			SearchUp(p->name,Matrix);
		}
		p=p->next;
	}
}



// Vytvori Matici
// Matice = Pole o velikosti MatrixSize (=Numbers+1) pointeru na ruzne velka pole charu
// Slouzi k rychlemu porovnani dvou jmen
// Kdyz jsou jmena ve stejnych komponentach, tak informace o jejich vztahu bude v
// Matrix[jm1->numer][jm2->MatrixPos]. Viz Funkce Compare
void CKerNamesMain::CreateMatrix() {
	int f;
	CKerNameList *p;
	CKerName *root;
	int size;
	ClearMatrix();
	MatrixSize=Numbers+1;
	// vytvorim prvni rozmer matice
	Matrix = new char*[MatrixSize];
	for (f=0;f<MatrixSize;f++) Matrix[f]=0;
	// Smazu informaci o komponente u vsech jmen
	p=Names;
	while (p) {
		p->name->Component=0;
		p=p->next;
	}
	p=Names;
	// pro vsechny jmena
	while (p) {
		root = p->name;
		if (root->Component==0) {
			// jmeno dosud nebylo v zadne komponente. Najdu tuto komponenu.
			// spocitam jeji velikost, jmenum priradim poradova cisla a pridam je do komponenty
			size = 0;
			FindComponent(root,root,size);
			root->matrixsize=size; // nastavim velikost komponenty
		} else size = root->Component->matrixsize; // prectu si velikost komponenty
		Matrix[root->number] = new char[size]; // pridam sloupec do matice
		for (f=0;f<size;f++) Matrix[root->number][f] = 0;
		SearchDown(root,Matrix[root->number]);
		SearchUp(root,Matrix[root->number]); // a spocitam jeho hodnoty
		p=p->next;
	}
	MatrixesCalculated = 1;
	FindMethodsAndParams();
}



// Najde vsechny jmena Metod a Parametru a spravne je oznaci
int CKerNamesMain::FindMethodsAndParams() {
	CKerNameList *n = Names;
	int err=1;
	if (!MatrixesCalculated) CreateMatrix();
	// Oznacim stred komponenty
	while (n) {
		if (n->name->Type==eKerNTmethod) if (n->name->Component->Type==eKerNTvoid) n->name->Component->Type = eKerNTmethod;
		if (n->name->Type==eKerNTparam) if (n->name->Component->Type==eKerNTvoid) n->name->Component->Type = eKerNTparam;
		if (n->name->Type==eKerNTobject) if (n->name->Component->Type==eKerNTvoid) n->name->Component->Type = eKerNTobjectVoid;
		n=n->next;
	}
	n = Names;
	// Oznacim celou komponentu, podle oznaceni stredu
	while (n) {
		if (n->name->Component->Type==eKerNTmethod) if (n->name->Type==eKerNTvoid||n->name->Type==eKerNTmethod) n->name->Type = eKerNTmethod; else err=0;
		if (n->name->Component->Type==eKerNTparam) if (n->name->Type==eKerNTvoid||n->name->Type==eKerNTparam) n->name->Type = eKerNTparam; else err=0;
		if (n->name->Component->Type==eKerNTobject || n->name->Component->Type==eKerNTobjectVoid) {
			if (n->name->Type==eKerNTvoid) n->name->Type = eKerNTobjectVoid;
			else if (n->name->Type!=eKerNTobject && n->name->Type!=eKerNTobjectVoid) err=0;
		}
		n=n->next;
	}
	return err;
}



////////////////////////////////////////////////////////////
// prida jmeno do vystupniho seznamu
void CKerNamesMain::FindLayerOutput(CKerName *name,CKerNameList **out , int direction ) {
	CKerNameList *l;
	while (*out) {
		l = (*out);
		if (direction==1&&(name->Compare(l->name)==2||name==l->name)) return;
		if (direction==0&&(name->Compare(l->name)==1||name==l->name)) return;
		if (name->Compare(l->name)||name==l->name) {
			(*out) = l->next;
			delete l;
		} else out = &((**out).next);
	}
	(*out) = new CKerNameList(name,(*out));
}


////////////////////////////////////////////////////////////////////
// Vytvori pole jmen, ktere jsou nejvyse pod From (defaultni direction, direction 0 jede nahoru).
// funkce vytvori seznam, ktery je treba zrusit, Zadejt From nula, kdyz chces prohledavat od "korenu". Je mozne zadat FilterFci, ktera vrati jedna, jestlize je jmeno pro vystup zajimave, jinak nula. 
// zadanim typu se budou hledat jen jmena zadaneho type (-1 vse)
CKerNameList *CKerNamesMain::FindLayer(CKerName *From, int (*FilterFce)(CKerName *name), int Type , int direction  ) {
	CKerNameList *list = 0;
	CKerNameList *out = 0;
	CKerNameList *l, *l2;
	#define TEST(_name) ((Type==-1||_name->Type==Type) && (FilterFce==0 || FilterFce(_name)))
	if (From) {
		list = new CKerNameList(From,list);
	} else {
		l = Names;
		while (l) {
			if ((direction==1 && l->name->parents==0) || (direction==0 && l->name->childs==0)) {
				if (TEST(l->name)) FindLayerOutput(l->name,&out,direction);
				else list = new CKerNameList(l->name,list);
			}
			l = l->next;
		}
	}
	while (list) {
		l2 = list;
		list = list->next;
		if (direction==1) {
			l = l2->name->childs; 
			if (l2->name == KnownNames[eKKNeverything].Name) l = Names;
		} else l = l2->name->parents;
		while (l) {
			if (l->name != KnownNames[eKKNeverything].Name) {
				if (TEST(l->name)) FindLayerOutput(l->name,&out,direction);
				else list = new CKerNameList(l->name,list);
			}
			l = l->next;
		}
		delete l2;
	}
	return out;
}



////////////////////////////////////////////////////////////////////////
// Najde mnozinu jmen. Tzn vsechny jmena, ktera jsou pod zadanym jmenem (defaultni direction).
// Nad zadanym jmenem - direction = 0. Je mozno specifikovat typ
// from nastaveno na nulu znamena ze vyhledava vse.
// Vrati seznam jmen, ktery je treba zrusit
CKerNameList *CKerNamesMain::FindSet(CKerName *From, int (*FilterFce)(CKerName *name), int Type, int direction ) {
	CKerNameList *list = Names;
	CKerNameList *out = 0;
	while (list) {
		if (!From || (direction==1 && KerServices.IsFMemberOfS(list->name,From)) || (direction != 1 && KerServices.IsFMemberOfS(From,list->name))) {
			if ((Type==-1 || list->name->Type==Type) && (!FilterFce || FilterFce(list->name))) out = new CKerNameList(list->name,out);
		}
		list = list->next;
	}
	return out;
}



/////////////////////////////////////////////////////////////////////
// vrati 1 pokud je pointer platny, 0 pokud je neplatny nebo NULL. 
// (funkce prochazi vsechny jmena)
int CKerNamesMain::TestPointerValidity(CKerName *name) {
	if (!name) return 0;
	CKerNameList *l = Names;
	while (l) {
		if (l->name == name) return 1;
		l = l->next;
	}
	return 0;
}





