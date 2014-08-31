/////////////////////////////////////////////////////////////////////////////
//
// ComExpression.h
//
// Interface CComExpression - reprezentace vyrazu
// Interface CComExpStack - zasobnik pro praci s vyrazy
// Interface CComConstStack - zasobnik pro praci s konstantnimi vyrazy
// Interface CComExpTree - reprezentace stromu vyrazu
// Interface CComExpTreeNode - reprezentace uzlu ve stromu vyrazu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "types.h"
#include "ComConstants.h"
#include "ComPart.h"

class CComExpression
{
public:
	CComExpression();
	CComExpression(int _value)    : pos(0), id(cUnknown), pointer(0) { value.intData = _value; type = kwInt; }
	CComExpression(double _value) : pos(0), id(cUnknown), pointer(0) { value.doubleData = _value; type = kwDouble; }
	CComExpression(char* _value)  : pos(0), id(cUnknown), pointer(0) { value.strData = _value; type = kwString; }
	CComExpression(int id, bool _isConstant, bool _isLValue, bool _hasSideEffect, int _type, int _pointer, CComPosition* _pos = 0, CDynamicArrayV<int,2>* _array = 0);
	CComExpression(CComExpression& e);
	~CComExpression();

	CComExpression& operator=(const CComExpression& e);

public:
	bool isConstant;		// true, pokud je vyraz konstantni
	bool isLValue;			// true, pokud je vyraz LValue
	bool hasSideEffect;		// true, pokud ma vyraz sideeffect
	int	 type;				// typ vyrazu
	int  pointer;			// pointer vyrazu
	int  id;				// id vyrazu, pokud je to identifikator (vysledky operaci maji id == cUnknown)
	int  data1, data2, data3;	// data ruzneho charakteru
	UComValues value;		// vycislena hodnota vyrazu
	CComShortPosition* pos;	// pozice vyrazu, defaultne 0
	CDynamicArrayV<int, 2> array;	// seznam velikosti jednotlivych dimenzi pole (pokud vyraz reprezentuje pole)
};

class CComExpStack
{
public:
	CComExpStack() { };
	virtual ~CComExpStack() { expStack.DeleteAll(); }

	void Push(CComExpression e) { expStack.Add(e); }		// ulozi vyraz na zasobnik
	void Load1();	// nahraje ze zasobniku jeden vyraz, k dispozici v e1
	void Load2();	// nahraje ze zasobniku dva vyrazy, k dispozici v e1 a e2
	void Load3();	// nahraje ze zasobniku tri vyraz, k dispozici v e1, e2 a e3
	void Touch1();	// zkopiruje ze zasobniku jeden vyraz, k dispozici v e1
	void Touch2();	// zkopiruje ze zasobniku dva vyrazy, k dispozici v e1 a e2
	void Reset() { expStack.DeleteAll(); }	// vymaze zasobnik

	CListKElem<CComExpression>* GetPos() { return expStack.el; }	// vrati pozici vrcholu zasobniku
	void RestorePos(CListKElem<CComExpression>* pos);	// obnovi zasobnik do stavu v dobe volani GetPos, kterym jsme ziskali parametr pos
public:
	CListK<CComExpression> expStack;			// zasobnik
	CComExpression e1, e2, e3;				// pracovni zaznamy vyrazu
};

class CComConstStack : public CComExpStack
{
public:
	CComConstStack() { };
	~CComConstStack() { }

	// konstantni zasobnik navic umi pocitat hodnoty vyrazu:
	void DoOp(int op, int opArity, int constType);	
};


// Uzel ve stromu vyrazu - reprezentuje jeden podvyraz
class CComExpTreeNode
{
public:
	enum enmFlushWhat {flushType = 1, flushName = 2, flushValue = 4, flushEverything = 7};
	CComExpression data;			// typ podvyrazu (hlavne rozlisuje mezi operatory a identifikatory)
	CComExpTreeNode *left;			// levy syn (unarni a vyssi operatory)
	CComExpTreeNode *right;			// pravy syn (binarni a vyssi operatory)
	CComExpTreeNode *middle;		// prostredni syn (ternarni operatory)

	CComExpTreeNode(CComExpression _data) : data(_data) { left = right = middle = 0; }
	~CComExpTreeNode() { SAFE_DELETE(left); SAFE_DELETE(right); SAFE_DELETE(middle); }

	void Flush(int arrayCount = 0);		// obecny pokyn k vystupu do souboru
	void FlushMethodItem();				// vypise volani metody
	void FlushDynamicArray();			// vypise volani metody dynamickeho pole
	void FlushKernelService();			// vypise volani sluzby kernelu
	void FlushItem(int arrayCount = 0);		// vypise datovou polozku (atribut, lokalni promennou...)
	void FlushParamsSafe(enmFlushWhat what);	// vypise parametry pri volani safe metody
		const static char* GetMDTypeString(int type);

	int FlushParamsDirect(int param, int service, int line); // vypise parametry pri volani direct metody
	void FlushNew();		// vypise volani new
	void FlushDelete();		// vypise volani delete
	void FlushTypeOf();		// vypise volani typeof
	void FlushStringRelation();		// vypise porovnavani stringu
	void FlushStringAssign(bool wasAssigning, bool wasStringOp);	// vypise prirazovani stringu
	void FlushStandardArray();	// vypise pristup k poli
	void FlushNamesRelation();	// vypise porovnani jmen

	// pomocne funkce
	int GetParamCount(bool safe);		// vraci pocet parametru v podstromu volani metody
	int GetPointer();					// vraci pointer podvyrazu

	static bool assigning;			// true, pokud vypisuji vlevo od operace prirazeni
	static bool stringOp;			// true, pokud se provadi operace s retezci
	static CComPart* p;				// odkaz na cast kompilatoru - obcas je potreba zavolat nejakou nestatickou funkci
	static CComOBObject* inObj;		// odkaz na objekt, v nemz je tento vyraz (v nektere metode)
	static CComOBMethod* inMethod;	// odkaz na metodu, v niz je tento vyraz
	static FILE* f;					// vystupni soubor - sem se generuji kompilovane skripty
	static int lastString;			// delka posledniho stringu (pri zpracovani parametru kernelich sluzeb)
};

class CComExpTree
{
public:
	CComExpTree() { root = 0; };
	~CComExpTree() { SAFE_DELETE(root); }

	void CreateFromStack(CComExpStack* stack);		// vytvori ze zasobniku strom
		CComExpTreeNode* CreateNode(CComExpStack* stack);	// vytvori ze zaznamu zasobniku uzel stromu

	void FlushInfixToFile(CComPart* p, CComOBObject* inObj, CComOBMethod* inMethod, FILE* f);	// vypise vyraz ulozeny ve stromu do vystupniho souboru

private:
	CComExpTreeNode* root;		// koren stromu vyrazu
};