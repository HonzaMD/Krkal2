/////////////////////////////////////////////////////////////////////////
///
///		K S  
///
///		Pristup ke kompilovanym skriptum, prirazovani jmen
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ks.h"


//////////////////////////////////////////////////
////	C K S M A I N

// Konstruktor. Vytvori a vyplni hashovaci tabulky. Priradi informace ke jmenum.
CKSMain::CKSMain() {
	if (KSMain) throw CExc(eKernel,0,"Error - Pokus Vytvorit Objekt pro kompilovane skripty KSMain dvakrat!");
	KSMain=this;
	KSVGs = new CHashTable(KS_KSVGs_HTS);
	KSAssignKSVGs();
	KSGs = new CHashTable(KS_KSGs_HTS);
	KSAssignKSGs();
	KSMs = new CHashTable(KS_KSMs_HTS);
	KSAssignKSMs();
	KSDMs = new CHashTable(KS_KSDMs_HTS);
	KSAssignKSDMs();
	KSOVs = new CHashTable(KS_KSOVs_HTS);
	KSAssignKSOVs();
	KSIDs = new CHashTable(KS_KSIDs_HTS);
	KSAssignKSIDs();
}


// Destruktor - Smaze hashovaci tabulky
CKSMain::~CKSMain(){
	if (KSVGs) KSVGs->DeleteAllMembers();
	SAFE_DELETE(KSVGs);
	if (KSGs) KSGs->DeleteAllMembers();
	SAFE_DELETE(KSGs);
	if (KSMs) KSMs->DeleteAllMembers();
	SAFE_DELETE(KSMs);
	if (KSDMs) KSDMs->DeleteAllMembers();
	SAFE_DELETE(KSDMs);
	if (KSOVs) KSOVs->DeleteAllMembers();
	SAFE_DELETE(KSOVs);
	if (KSIDs) KSIDs->DeleteAllMembers();
	SAFE_DELETE(KSIDs);
	KSMain=0;
}


/////////////////////////////////////////////////////////////////////////////////////
///		CKSKSVG
// konstruktor
CKSKSVG::CKSKSVG(char *Name, void *(*_AllocKSVG)(), void (*_SetObjectVar)(void *KSVG), int size){
	name = Name;
	AddToHashTable(KSMain->KSVGs);
	AllocKSVG = _AllocKSVG;
	SetObjectVar = _SetObjectVar;
	Size = size;
	attributes = new CHashTable(8);
	assert(attributes);
}

CKSKSVG::~CKSKSVG()
{
	attributes->DeleteAllMembers();
	SAFE_DELETE(attributes);
}

void CKSKSVG::AddAttribute(char* name)
{
	CHashElem* p = new CHashElem();
	assert(p);
	
	p->name = name;
	p->AddToHashTable(attributes);
}

