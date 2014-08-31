/////////////////////////////////////////////////////////////////////////
///
///		K S . H 
///
///		Pristup ke kompilovanym skriptum, prirazovani jmen
///		A: Honza M.D. Krcek
///
/////////////////////////////////////////////////////////////////////////


#ifndef KOMPILED_SCRIPTS_H
#define KOMPILED_SCRIPTS_H

#include "ks_version.h"
#include "types.h"

class CKSMain;
class CKSKSVG;

// Funkce ktere provadeji prerazovani jmen:
extern void KSNamesAssign();
extern void KSAssignKSVGs();
extern void KSAssignKSGs();
extern void KSAssignKSMs();
extern void KSAssignKSDMs();
extern void KSAssignKSOVs();
extern void KSAssignKSIDs();

// Objekt ktery si v hashovacich tabulkach pamatuje prirazeni jmen
// Zkonstruovat jen jednou pri startu Krkala. Pred Kernelem!
extern CKSMain *KSMain;

class CKSMain {
friend CKSKSVG;
public:
	CKSMain();
	~CKSMain();
	CHashTable *KSVGs;
	CHashTable *KSGs;
	CHashTable *KSMs;
	CHashTable *KSDMs;
	CHashTable *KSOVs;
	CHashTable *KSIDs;
};


// Informace o tom jak alokovat KSVG skupiny objektovych promennych.
// A jak pro ne nastavovat pristup.
class CKSKSVG : public CHashElem {
public:
	CKSKSVG(char *Name,	void *(*_AllocKSVG)(), void (*_SetObjectVar)(void *KSVG), int size);
	~CKSKSVG();

	void *(*AllocKSVG)();  // alokuje KSVG
	void (*SetObjectVar)(void *KSVG); // Zajisti pristup k polozkam KSVG
	void AddAttribute(char* name);

	int Size; // velikost KSVG
	CHashTable* attributes;	// jmena atributu teto ksvg
};


// Informace o umisteni globalnich promennych prislusnych jmen.
class CKSKSG : public CHashElem {
public:
	CKSKSG(char *Name, void **variable) {
		name = Name; Variable=variable;
		AddToHashTable(KSMain->KSGs);
	}
	void **Variable;
};


// Informace o umisteni pointeru na objektove promenne prislusnych jmen.
class CKSKSOV : public CHashElem {
public:
	CKSKSOV(char *Name, void **variable) {
		name = Name; Variable=variable;
		AddToHashTable(KSMain->KSOVs);
	}
	void **Variable;
};


// Informace o umisteni kodu kompilovanych metod
class CKSKSM : public CHashElem {
public:
	CKSKSM(char *Name, void (*fce)()) {
		name = Name; Fce=fce;
		AddToHashTable(KSMain->KSMs);
	}
	void (*Fce)();
};


// Zda je jmeno uz kompilovano??
class CKSKSID : public CHashElem {
public:
	CKSKSID(char *Name) {
		name = Name;
		AddToHashTable(KSMain->KSIDs);
	}
};


// Kde lezi pointer na direct call metodu?
class CKSKSDM : public CHashElem {
public:
	CKSKSDM(char *Name, void (**fce)()) {
		name = Name; Fce=fce;
		AddToHashTable(KSMain->KSDMs);
	}
	void (**Fce)();
};



#endif
