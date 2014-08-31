/////////////////////////////////////////////////////////////////////////////
//
// ComObjectBrowser.h
//
// Interface CComObjectBrowser a mnoha dalsich  
//	- hierarchicky usporadane tridy, ktere dohromady tvori objectBrowser
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ComSymbolTable.h"
#include "ComConstants.h"
#include "names.h"

#define BOOLEANSTR(b)	(b)?"true":"false"

class CComObjectBrowser;
class CComOBObject; 
class CComOBMethod; 
class CComOBName;
class CKSKSVG;

// zaznam predavany pri provadeni neciste operace modify
class CComOBModifyRec : public CComOBBase
{
public:
	CComOBModifyRec(const char* _name, CListK<int>* _tokens, CComPosition _pos) : CComOBBase(_name, cUnknown, _pos), tokens(_tokens) { }
	CComOBModifyRec(CComOBModifyRec& mdfr) { assert(false); }
	~CComOBModifyRec() { SAFE_DELETE(tokens); }

	CListK<int>* tokens;		// tokeny, kterymi se ma nahradit modifikovany clen objektu pri pristi kompilaci
};

// zaznam zavislosti jmen
class CComOBNameDepend : public CComOBBase
{
public:
	CComOBNameDepend(const char* _sonName, const char* _fatherName, int _version, CComShortPosition _posDefined, int _father, int _son);
	CComOBNameDepend(CComOBNameDepend& obnd);
	~CComOBNameDepend();

	char* sonName;	// jmeno syna
	int father;		// index otce v uTab
	int son;		// index syna v uTab
	
	void DebugToFile(FILE* f);
};

// zaznam polozky vyhledavaci struktury CComNameSearch
class CComNSEEntry
{
public:
	CComNSEEntry(CComOBName* _name = 0, int _nameID = cUnknown) { name = _name; nameID = _nameID;}
	CComNSEEntry(CComNSEEntry& nsee) { name = nsee.name; nameID = nsee.nameID; }
	~CComNSEEntry() { }

	CComOBName* name;	// kerneli jmeno
	int nameID;			// ID kerneliho jmena
};

// zaznam uzlu vyhledavaci struktury CComNameSearch
class CComNSENode
{
public:
	CComNSENode(int _id = cUnknown, int _version = cUnknown) { id = _id; version = _version; }
	CComNSENode(CComNSENode& nsen) { id = nsen.id; version = nsen.version; }
	~CComNSENode() {}
	
	CComNSENode& operator=(const CComNSENode& nsen) { id = nsen.id; version = nsen.version; return *this; } 
	
	int id;			// Id nerozpoznaneho identifikatoru (index do uTab). Identifikator reprezentuje jednu cast slozeneho jmena.
	int version;	// Verze prislusejici teto casti jmena.
};

// jeden prvek hierarchii vyhledavacich zaznamu
class CComNameSearchElem
{
public:
	CComNameSearchElem() { id = cUnknown; }
	CComNameSearchElem(CListKElem<CComNSENode>* idLeft, CComOBName* name, int nameID);
	~CComNameSearchElem();

	CComNameSearchElem* Add(CListKElem<CComNSENode>* ident, CComOBName* name, int nameID);
	void AddIfNew(CComOBName* name, int nameID);

	CComNameSearchElem* FindLast(CListKElem<CComNSENode>* ident, CListKElem<CComNSENode>** idLeft, CComNameSearchElem** firstEntry);
	CListK<CComNSEEntry>*  FindRec(CListKElem<CComNSENode>* ident);

	int id;				// Id nerozpoznaneho identifikatoru (index do uTab). Identifikator reprezentuje jednu cast slozeneho jmena.
	int version;		// Verze prislusejici teto casti jmena.
	CListK<CComNSEEntry> nList;	// Seznam jmen, ktera odpovidaji prefixu, jenz tento objekt reprezentuje.

private:
	CListK<CComNameSearchElem*> deeper;
};

// Trida pro vyhledavani a spravu slozenych jmen
class CComNameSearch
{
public:
	CComNameSearch();
	~CComNameSearch() { }

	void Add(CListK<CComNSENode>* ident, CComOBName* name, int nameID); 
	CListK<CComNSEEntry>*  FindRec(CListK<CComNSENode>* ident);

private:	
	CComNameSearchElem searchRoot;	// koren vyhledavaci struktury
	CComNameSearchElem* fastSearch[cFastNameSearchSize][cFastNameSearchMaxVersions];	// tabulka odkazu do struktury pro zrychlene vyhledavani
};

// zaznam tabulky preprocesoru
class CComOBPreprocessor : public CComOBBase
{
public:
	CComOBPreprocessor(const char* _name, int _type, int _intData = cUnknown, const char* _strData = 0);
	CComOBPreprocessor(CComOBPreprocessor& obp);
	~CComOBPreprocessor();

	int type;		// typ zaznamu {konstant pteXXX)
	int intData;	// celociselna data
	char* strData;	// stringova data
};


union UComValues
{
	int		intData;
	int		intArray[2];
	double	doubleData;
	char*	strData;
	void*	voidPtrData;
};


// trida reprezentujici editTagy
class CComOBEditTags
{
public:

	char* userName;		// uzivatelske jmeno
	char* comment;		// komentar

	int valueType;		// typ defaultni hodnoty
	CDynamicArrayV<UComValues,16> defaultValue;		// defaultni hodnota

	int objET;		// editTagy objektu
	int limits;		// typ limitu
	int nameAuto;	// ID jmena editTagu Auto
	int editType;	// typ editoru
	int	nameMask;	// maska jmena
	bool hasDefaultMember;	// true, pokud je platna polozka DefaultMember
	int isOp;		// typ operace Is
	UComValues isName;	// hodnota, se kterou je porovnavan atribut operaci Is

	UComValues interval[2];	// hodnoty intervalu - horni a dolni mez
	UComValues DefaultMember;	// hodnota DefaultMember urcujici hodnotu neinicializovane polozky pole
	CDynamicArrayV<UComValues,16> list;	// seznam pripustnych hodnot pro atribut (editTag LIST)

public:
	CComOBEditTags();
	~CComOBEditTags();

	CComOBEditTags(CComOBEditTags& et);
	CComOBEditTags& operator=(CComOBEditTags& et);

	void SetUserName(const char* _userName);
	void SetComment(const char* _comment);
	void SetDefaultValue(UComValues _value, int _valueType);

	void DebugToFile(FILE* f);
};

// Trida reprezentujici kazdou entitu, ktera ma datovou podstatu
class CComOBBaseData : public CComOBBase
{
public:
	CComOBBaseData(CComOBBase& obb) : CComOBBase(obb) { storage = dType = pointer = cUnknown; kerName = 0; }
	CComOBBaseData(const char* _name, int _version, CComShortPosition _posDefined);
	CComOBBaseData(CComOBBaseData& obbd);
	virtual ~CComOBBaseData() {}
	int storage;	// typ ulozeni (napr. u metod: direct/safe)
	int dType;		// datovy typ
	int pointer;	// pointer
	int kerNameID;	// ID prislusejiciho jmena kernelu, pokud nejake takove je, jinak cUnknown

	int	 GetSize() const;	// vrati velikost datove polozky v B
	int GetMDType(int overType = cUnknown, int overPointer = cUnknown);	// vrati typ ve vyjadreni, ktere pouziva M.D.
	int GetPointer(int overType = cUnknown, int overPointer = cUnknown); // vrati pointer, pokud je to objekt, tak o 1 mensi (kdyz nechci mit za OPointer *)

	static int GetMyType(eKerTypes mdType);		// prevede MD type na muj typ
	static int GetMyPointer(eKerTypes mdType);	// z MD type rozhodne, zda je muj pointer 0 nebo 1

	void SetData(int _storage, int _dType, int _pointer);	// nastavi storage, dType a pointer
	void DebugToFile(FILE* f, int tabIndent);

	bool compiled;	// je atribut/metoda kompilovana
	int use;		// hodnota use (pro zapis do registru)
	
	CComOBEditTags edit;	// hodnoty editTagu

	CDynamicArrayV<int,2> array;	// seznam rozmeru pole ve vsech dimenzich (pouze u poli)

	static CComObjectBrowser* objBrowser;	// odkaz na objectBrowser
// udaje pro codegen:
	int mem;			// offset v kodu/pameti, kde je entita ulozena
};

// Trida reprezentujici konstantu
class CComOBConstant : public CComOBBaseData
{
public:
	CComOBConstant(const char* _name, int _type, const char* _dispName = 0);
	CComOBConstant(CComOBConstant& obc);
	~CComOBConstant();

	UComValues value;	// ciselna hodnota konstanty
private:
	void Eval();		// spocita ciselnou hodnotu konstanty
};

// Trida reprezentujici kerneli jmeno
class CComOBName : public CComOBBaseData
{
public:
	CComOBName(const char* _name, const char* _kerName, int _version, const char* versionStr, CComShortPosition _posDefined, int _dType);
	CComOBName(CComOBName& obn);
	~CComOBName();

	void DebugToFile(FILE* f);

	int objID;	// ID objektu, pokud jde o jmeno objektu
	int metID;	// ID metody, pokud jde o jmeno metody
};

// Trida reprezentujici globalni promennou
class CComOBGlobal : public CComOBBaseData
{
public:
	CComOBGlobal(const char* _name, int _version, CComShortPosition _posDefined,CComSymbolTable* uTab);
	CComOBGlobal(CComOBGlobal& obg);
	~CComOBGlobal();	

	void SetData(int _dType, int _pointer);
	void DebugToFile(FILE* f);
};

// Trida reprezentujici atribut
class CComOBAttribute : public CComOBBaseData
{
public:
	CComOBAttribute(CComOBBaseData& obd) : CComOBBaseData(obd) { inObj = objDef = 0; kerKonkretniID = 0; }
	CComOBAttribute(const char* _name, int _version, CComShortPosition _posDefined, CComSymbolTable* uTab, CComOBObject* _obj, int _knownName);
	CComOBAttribute(CComOBAttribute& oba);
	~CComOBAttribute();

	CComOBObject* objDef;	// objekt, ve kterem byl atribut definovan
	CComOBObject* inObj;	// objekt, ve kterem je atribut umisten
	CListK<int> accessMethods;	// seznam metod tohoto objektu, ktere k atributu pristupuji
	char* kerKonkretniID;	// ID kerneliho jmena atributu
	enmInherit inherit;		// zda se ma atribut dedit nebo ne
	int  knownName;			// ID znameho jmena, pokud je atribut zname jmeno, jinak cUnknown

	void SetCompiled(CComOBMethod* sender, bool shouldBeCompiled);	// urceni kompilovatelnosti metod a atributu 
	void DebugToFile(FILE* f);
	void RecreateKerName(CComSymbolTable* uTab, CComOBObject* newObj);	// znovu vytvori kerneli identifikaci atributu
};

// Trida reprezentujici argument
class CComOBArgument : public CComOBBaseData
{
public:
	CComOBArgument(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr, CComOBMethod* inMethod, int _knownName, bool global);
	CComOBArgument(CComOBArgument& oba);
	~CComOBArgument();	

	int ret;			// modifikator ret nebo cUnknown, pokud se argument nema vracet hodnotou
	int  knownName;		// ID znameho jmena nebo cUnknown, pokud neni argument zname jmeno

	void SetData(int _ret, int _dType, int _pointer);
	int GetMDRet();		// vrati ret ve vyjadreni srozumitelnem kernelu

	void DebugToFile(FILE* f);
};

// Trida reprezentujici lokalni promennou
class CComOBLocal : public CComOBBaseData
{
public:
	int blockDepth;		// hloubka bloku, v nemz je promenna definovana
	int blockNr;		// index bloku, v nemz je promenna definovana

	CComOBLocal(const char* _name, int _version, CComShortPosition _posDefined);
	CComOBLocal(CComOBLocal& obl);
	~CComOBLocal();	

	void SetData(int _dType, int _pointer, int _blockDepth, int _blockNr);	// nastavi typ, pointer, hloubku a index bloku
	void DebugToFile(FILE* f);
};

// Trida reprezentujici metodu
class CComOBMethod : public CComOBBaseData
{
public:
	CComOBMethod(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr, CComOBObject* _obj, int storage, int _knownName, bool global);
	CComOBMethod(CComOBMethod& obm);
	~CComOBMethod();

	bool returnsValue;		// true, pokud metoda vraci hodnotu
	bool hasSafeArgs;		// true, pokud ma metoda bezpecny seznam argumentu
	bool canBeCompiled;		// true, pokud tato metoda existuje v kompilovanych skriptech
	int  localsSize;		// soucet velikosti lokalnich promennych teto metody
	int  localsPos;			// umisteni lokalnich promennych na zasobniku (= velikost bloku parametru)
	int  knownName;			// ID znameho jmena nebo cUnknown, pokud metoda neni znameho jmena
	int  id;				// id metody
	enmInherit inherit;		// udani zda se ma metoda dedit
	int  groupType;			// spolecny typ skupiny metod
	int	 groupPointer;		// spolecny pointer skupiny metod
	bool global;			// true, pokud je metoda globalni
	int  ret;				// modifikator retu metody

	CComOBObject* objDef;	// objekt, ve kterem byla metoda definovana
	CComOBObject* inObj;	// objekt, ve kterem je metoda umistena
	CComSymbolTable* aTab;	// tabulka argumentu metody
	CComSymbolTable* lTab;	// tabulka lokalnich promennych metody

	CListK<CComOBMethod*> dangerousCalls;	// seznam metod, ktere tato metoda nebezpecne vola
	CListK<int> usedAttributes;	// seznam atributu, ke kterym tato metoda pristupuje
	
	char* kerKonkretniID;	// kerneli jmeno tela metody

	int GetMDKnownName();	// ID znameho jmena metody (pokud je znameho jmena), srozumitelne kernelu

	void SetData(int _storage, int _dType, int _pointer);	// nastavi storage, dType a pointer
	void SetCompiled(CComOBAttribute* sender, bool shouldBeCompiled);	// urceni kompilovatelnosti - rekurzivni volani
	void ComputeLocals();	// spocita pametove rozmisteni parametru a lokalnich promennych teto metody
	void DebugToFile(FILE* f);

	CComOBArgument* GetArgument(int index) { return (CComOBArgument*)(aTab->Get(index+tokArgumentFirst)); }	// vrati prislusny argument metody
	CComOBLocal*    GetLocal   (int index) { return (CComOBLocal*)   (lTab->Get(index+tokLocalFirst)); }	// vrati prislusnou lokalni promennou

};

// Trida reprezentujici strukturu
class CComOBStructure : public CComOBBaseData
{
public:
	CComOBStructure(const char* _name, CComOBObject* inObj, const char* versionStr, CComShortPosition _posDefined);
	CComOBStructure(CComOBStructure& obs);
	~CComOBStructure();

	int GetStructureSize();		// vrati velikost struktury v B
	void ComputeMembers();		// spocita pametove umisteni clenu struktury

	CComSymbolTable* mTab;		// tabulka clenu strukturu
	int size;					// velikost struktury - aby se nemusela pokazde pocitat znovu

	void DebugToFile(FILE* f);
};

// Trida reprezentujici skupinu atributu
class CComOBAttributeGroup
{
public:
	CComOBAttributeGroup(int _type = cUnknown, CComOBAttribute* _grpAtr = 0) : type(_type), grpAtr(_grpAtr) { }
	CComOBAttributeGroup(CComOBAttributeGroup& ag);
	~CComOBAttributeGroup() { members.RemoveAll(); SAFE_DELETE(grpAtr); }

	int type;						// typ skupiny
	CComOBAttribute* grpAtr;		// pseudo atribut, ktery obsahuje udaje o skupine
	CDynamicArrayV<CComOBAttribute*, 6> members;	// atributy v teto skupine
};

// Trida reprezentujici objekt
class CComOBObject : public CComOBBase
{
public:
	CComOBObject(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr);
	CComOBObject(CComOBObject& obo);
	~CComOBObject();

	CComSymbolTable* aTab;		// tabulka atributu tohoto objektu
	CComSymbolTable* mTab;		// tabulka metod tohoto objektu
	CDynamicArrayV<CComOBAttributeGroup*, 8> gList;	// seznam skupin atributu v tomto objektu
	CDynamicArrayV<CComOBAttribute*, 8> scrList;	// seznam skriptovanych promennych v tomto objektu

	int isdSize;		// velikost interpretovanych dat
	CKSKSVG* ksvg;		// KSVG objektu
	int kerNameID;		// ID kerneliho jmena objektu
	bool knownName;		// ID znameho jmena - pokud se jedna o objekt znameho jmena
	CComOBEditTags edit;	// edit-tagy tohoto objektu

	CComOBAttribute* GetAttribute(int index)	// vrati prislusny atribut
	{
		return (CComOBAttribute*)(aTab->Get(index+tokAttributeFirst)); 
	}	
	
	CComOBMethod* GetMethod(int index)			// vrati prislusnou metodu
	{ 
		return (CComOBMethod*)(mTab->Get(index+tokMethodFirst)); 
	}

	void RecreateKerName(CComSymbolTable* uTab);	// znovu vytvori kerneli identifikaci objektu
	void ComputeAttributes();		// spocita pametove rozmisteni atributu objektu

	int  FitInKSVG(CKSKSVG* pKSVG);	// vrati pocet atributu, ktere lze umistit do prislusne KSVG
	bool FitAllInKSVG(CKSKSVG* pKSVG);	// vrati true, pokud lze vsechny atributy umistit do prislusne KSVG
	void SetKSVG(CKSKSVG* pKSVG);	// priradi objektu prislusne KSVG a nastavi kompilovatelnost u atributu

	void DebugToFile(FILE* f);
};

// Trida reprezentujici object browser
class CComObjectBrowser 
{
public:
	CComObjectBrowser(CComSymbolTable* _uTab, CComConstantTable* _cTab, CComSymbolTable* _oTab, CComSymbolTable* _nTab,
		CComSymbolTable* _dTab, CComSymbolTable* _gTab, CComSymbolTable* _pTab, 
		CComSymbolTable* _mTab, CComSymbolTable* _aTab, CComSymbolTable* _dnTab, CComSymbolTable* _gnTab, CComSymbolTable* _sTab,
		CKerNamesMain* _names, CComNameSearch* _nameSearch);
	~CComObjectBrowser();

	// tabulky kompilatoru:
	CComSymbolTable* uTab;
	CComConstantTable* cTab;
	CComSymbolTable* oTab;
	CComSymbolTable* nTab;
	CComSymbolTable* dTab;
	CComSymbolTable* gTab;
	CComSymbolTable* pTab;

	CComSymbolTable* mTab;
	CComSymbolTable* aTab;

	CComSymbolTable* dnTab;
	CComSymbolTable* gnTab;
	CComSymbolTable* sTab;

	CKerNamesMain*	names;
	CComNameSearch* nameSearch;

	bool generatingCode;	// true, pokud se ma generovat kod a kompilovane skripty
	
	CComOBObject* GetObject(int index)	// vraci prislusny objekt
	{ 
		return (CComOBObject*)(oTab->Get(index+tokObjectFirst)); 
	}

	CComOBGlobal* GetGlobal(int index) // vraci prislusnou globalni promennou
	{ 
		return (CComOBGlobal*)(gTab->Get(index+tokGlobalFirst)); 
	}

	CComOBName*	  GetName  (int index) // vraci prislusne jmeno
	{ 
		return (CComOBName*)  (nTab->Get(index+tokNameFirst)); 
	}

	CComOBNameDepend*  GetDepend(int index) // vraci prislusnou zavislost
	{ 
		return (CComOBNameDepend*) (dTab->Get(index+tokDependFirst)); 
	}

	CComOBStructure* GetStructure(int index) // vraci prislusnou strukturu
	{
		return (CComOBStructure*)(sTab->Get(index+tokStructureFirst)); 
	}

	const char* GetVersionStr(int index) // vraci string verze, identifikovane indexem
	{ 
		return uTab->Get(index)->name; 
	}

	int DebugToFile(const char* fileName);
};
