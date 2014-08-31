/////////////////////////////////////////////////////////////////////////////
//
// ComSymbolTable.h
//
// Interface CComSymbolTable - trida reprezentujici hashovaci tabulku symbolu
// Interface CComConstantTable - trida reprezentujici hashovaci tabulku konstant
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"
#include "ComPosition.h"

class CComHTE :	public CHashElem // CHashElem viz types.h - CHashTable by M.D.
{
public:
	CComHTE(const char* _name, int _index);
	~CComHTE();

	CComHTE& operator=(CComHTE& hte);
	int index;		// index do pole zaznamu symbolTable
};

// nadstavba nad hashovaci tabulkou kernelu
class CComHashTable : public CHashTable
{
public:
	CComHashTable(int hashSize) : CHashTable(hashSize) {}

	int  GetHashTableSize() const { return HashSize; }
};

// zakladni trida object browseru - musim ji vsak definovat jiz zde, protoze zde ji pouzivam
class CComOBBase
{
public:
	CComOBBase() : name(0), kerName(0), version(cUnknown) { }
	CComOBBase(const char* _name, int _version, CComShortPosition _posDefined);
	CComOBBase(CComOBBase& obb);
	CComOBBase& operator=(CComOBBase& obb);
	virtual ~CComOBBase();

	char* name;				// jmeno
	char* kerName;			// kerneli jmeno
	int version;			// verze
	CComShortPosition posDefined;	// pozice identifikatoru

	void DebugToFile(FILE* f, int tabIndent);
};


// Trida reprezentujici tabulku symbolu
class CComSymbolTable 
{
public:
	CComSymbolTable(int _htSize, int _reserved);
	CComSymbolTable(CComSymbolTable& st);
	~CComSymbolTable();
	
	void Clear();		// vymaze obsah tabulky
	
	int Find(const char* name);		// vyhleda pomoci hashTabulky polozku se jmenem name, vrati jeji index
	int FindFirst(const char* name, int version = findAny);	// vyhleda prvni vyskyt polozky se jmenem name, ktera splnuje podminku na verzi, vrati jeji index
	int FindNext(const char* name, int version = findAny);	// vyhleda dalsi vyskyt polozky se jmenem name, ktera splnuje podminku na verzi, vrati jeji index
	int FindVersion(const char* name, int version, bool* ambiguity = 0); // vyhleda prvek podle jmena a verze, vrati jeho index, pripadne nastavi, zda existuje vice polozek stejneho jmena a ruznych verzi (ambiguity)
	int FindOrAdd (const char* name, CComOBBase* itemToAdd, bool* success = 0);	// bud prida do tabulky novy zaznam (itemToAdd), nebo jej zrusi - v pripade, ze uz v tabulce je. Vrati index nalezene/pridane polozky.
	int Add(CComOBBase* itemToAdd) { return Add(itemToAdd->name, itemToAdd); }	// prida do tabulky novy zaznam, vrati jeho index
	int Add(const char* name, CComOBBase* itemToAdd);	// prida do tabulky novy zaznam, vrati jeji index

	int AddExclusive(CComOBBase* itemToAdd);	// prida do tabulky novy zaznam, vrati jeho index, selze, pokud tam jiz je

	CComOBBase* FindRec(const char* name)	// vrati pointer na zaznam jmena name
	{ 
		return symbolTable[Find(name)-reserved]; 
	}

	CComOBBase* FindLastRec(const char* name, int* count); // vrati pointer na posledni nalezeny zaznam jmena name

	CComOBBase* Get(int id);	// vrati prvek identifikovany parametrem id 

	CComOBBase* operator[](int id) { return Get(id); }
	
	int GetCount()	// vrati pocet prvku v tabulce
	{ 
		return symbolTable.GetCount(); 
	}

private:
	int reserved;			// index prvku v poli + reserved = index prvku v tabulce
	CHashList* searching;	// kontext minuleho hledani, pokud vyhledavam pomoci FindFirst a FindNext
	CComHashTable* hashTable;	// hashovaci tabulka
	CDynamicArray<CComOBBase> symbolTable;	// pole zaznamu
};

// Tabulka konstant
class CComOBConstant;

class CComConstantTable : public CComSymbolTable
{
public:
	CComConstantTable(int _htSize, int _reserved) : CComSymbolTable(_htSize, _reserved) { nullID = nnullID = onullID = anullcharID = anulldoubleID = anullintID = anullnameID = anullobjptrID = anullvoidID = getcountID = setcountID = 0; }
	CComConstantTable(CComSymbolTable& st) : CComSymbolTable(st) { }
	~CComConstantTable() {}

	CComOBConstant* FindOrAddConstant(int value, int* cid = 0);		// pridani konstanty, jejiz hodnotu udava parametr value
	CComOBConstant* FindOrAddConstant(double value, int* cid = 0);	// pridani konstanty, jejiz hodnotu udava parametr value
	CComOBConstant* FindOrAddConstant(const char* value, int* cid = 0);	// pridani konstanty, jejiz hodnotu udava parametr value

	int FindNull(int nullToken); // vrati index konstanty typu null; typy nullu se rozlisi pomoci parametru nullToken
	CComOBConstant* GetNull(int nullToken)	// vrati zaznam konstanty, ktera reprezentuje prislusny null token
	{ 
		return (CComOBConstant*)Get(FindNull(nullToken)); 
	}
	void FillNullTokens();	// vyplni tabulku null tokeny

	inline int GetNNull() const { return nnullID; }

	int getcountID, setcountID;

private:
	int nullID, nnullID, onullID, anullcharID, anulldoubleID, anullintID, anullnameID, anullobjptrID, anullvoidID;
};


