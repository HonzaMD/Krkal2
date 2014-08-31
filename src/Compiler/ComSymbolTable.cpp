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

#include "stdafx.h"
#include "ComSymbolTable.h"
#include "ComObjectBrowser.h"

/******************************************************************************
 *
 *	CComHTE
 *
 ******************************************************************************/

CComHTE::CComHTE(const char* _name, int _index)
{
	assert(_name);
	name = newstrdup(_name);
	assert(name);
	index = _index;
}

CComHTE::~CComHTE()
{ 
	SAFE_DELETE_ARRAY(name); 
}

CComHTE& CComHTE::operator=(CComHTE& hte)
{
	SAFE_DELETE_ARRAY(name); 
	name = newstrdup(hte.name);
	assert(name);
	index = hte.index;
	return *this;
}


/******************************************************************************
 *
 *	CComOBBase
 *
 ******************************************************************************/

CComOBBase::CComOBBase(const char* _name, int _version, CComShortPosition _posDefined)
{
	assert(_name);
	name = newstrdup(_name);
	assert(name);
	version = _version;
	posDefined = _posDefined;
	kerName = 0;
}

CComOBBase::CComOBBase(CComOBBase& obb)
{
	name = newstrdup(obb.name);
	assert(name);
	if(obb.kerName)
	{
		kerName = newstrdup(obb.kerName);
		assert(kerName);
	} else
		kerName = 0;
	version = obb.version;
	posDefined = obb.posDefined;
}

CComOBBase& CComOBBase::operator=(CComOBBase& obb)
{
	SAFE_DELETE_ARRAY(name);
	name = newstrdup(obb.name);
	assert(name);
	if(obb.kerName)
	{
		SAFE_DELETE_ARRAY(kerName);
		kerName = newstrdup(obb.kerName);
		assert(kerName);
	} else
		kerName = 0;
	version = obb.version;
	posDefined = obb.posDefined;
	return *this;
}

CComOBBase::~CComOBBase()
{
	SAFE_DELETE_ARRAY(name);
	SAFE_DELETE_ARRAY(kerName);
}

void CComOBBase::DebugToFile(FILE* f, int tabIndent)
{
	assert(f);
	assert(tabIndent >= 0);
	char buf[cMaxOBTabIndent+1];
	int i;
	for(i=0; i< tabIndent; i++)
		buf[i] = '\t';
	buf[i] = '\0';

	fprintf(f,"%s<Name>              = %s\n", buf, name);
	if(kerName)
		fprintf(f,"%s<Kernel name>       = %s\n", buf, kerName);
	fprintf(f,"%s<Version>           = %d\n", buf, version);
	fprintf(f,"%s<Position>          = {%d,%d,%d}\t\t// {fileID,line,column}\n", buf, posDefined.fileID, posDefined.line, posDefined.column);
}

/******************************************************************************
 *
 *	CComSymbolTable
 *
 ******************************************************************************/


CComSymbolTable::CComSymbolTable(int _htSize, int _reserved)
{
	assert(_htSize>0 && _reserved>=0);
	hashTable = new CComHashTable(_htSize);
	assert(hashTable);
	reserved = _reserved;
	searching = 0;
}

CComSymbolTable::CComSymbolTable(CComSymbolTable& st)
{
	assert(false);
}

CComSymbolTable::~CComSymbolTable()
{
	hashTable->DeleteAllMembers();
	SAFE_DELETE(hashTable);
	symbolTable.RemoveAll();
}

void CComSymbolTable::Clear()
{
	hashTable->DeleteAllMembers();
	symbolTable.RemoveAll();
	searching = 0;
}

int	CComSymbolTable::Find(const char* name)
{
	assert(name);
	CComHTE* hte = (CComHTE*)hashTable->Member(name);
	return hte ? hte->index + reserved : cUnknown;
}

int CComSymbolTable::FindFirst(const char* name, int version)
{
	CComHTE* hte;
	assert(name);
	hte = (CComHTE*)hashTable->MemberFirst(name,&searching);

	while(version != findAny && hte && symbolTable[hte->index]->version != version)
		hte = (CComHTE*)hashTable->MemberNext(name,&searching);

	return hte ? hte->index + reserved : cUnknown;
}

int CComSymbolTable::FindNext(const char* name, int version)
{
	CComHTE* hte;
	assert(name);
	if(!searching)
		return cUnknown;
	do
		hte = (CComHTE*)hashTable->MemberNext(name,&searching);
	while(version != findAny && hte && symbolTable[hte->index]->version != version);

	if(!hte)
		searching = 0;
	return hte ? hte->index + reserved : cUnknown;
}

int CComSymbolTable::FindVersion(const char* name, int version, bool* ambiguity)
{
	int minVersion = tokUnknownLast;
	int minID = tokUnknownLast;
	int id = FindFirst(name);
	int i = 1;

	if(id == cUnknown)
		return cUnknown;
	CComOBBase* rec = Get(id);

	while(id!=cUnknown && version!=findAny && version!=rec->version)
	{
		i++;
		if(rec->version < minVersion)
		{
			minVersion = rec->version;
			minID = id;
		}
		if((id = FindNext(name))== cUnknown)
			break;
		rec = Get(id);
	}

	while(ambiguity && version==findAny && id != cUnknown && FindNext(name) != cUnknown)
		i++;

	if(ambiguity)
		*ambiguity = i > 1;

	if(version==findNewest)
		return minID == tokUnknownLast ? cUnknown : minID;
	return id;
}

int CComSymbolTable::FindOrAdd (const char* name, CComOBBase* itemToAdd, bool* success)
{
	int index;

	assert(name);
	CComHTE* hte = (CComHTE*)hashTable->Member(name);

	if(success)
		*success = hte != 0;
	
	if(hte)
	{
		index = hte->index;
		SAFE_DELETE(itemToAdd);
	} else
	{
		index = symbolTable.Add(itemToAdd);
		hte = new CComHTE(name, index);
		assert(hte);
		hte->AddToHashTable(hashTable);
	}

	return index + reserved;
}

int CComSymbolTable::Add(const char* name, CComOBBase* itemToAdd)
{
	assert(itemToAdd);
	int index = symbolTable.Add(itemToAdd);
	CComHTE* hte = new CComHTE(name, index);
	assert(hte);
	hte->AddToHashTable(hashTable);
	
	return index + reserved;
}

int CComSymbolTable::AddExclusive(CComOBBase* itemToAdd)
{
	bool success;
	int ret;
	
	assert(itemToAdd);
		
	ret = FindOrAdd(itemToAdd->name, itemToAdd, &success);
	assert(!success);		// Pokousis se pridat dve stejna jmena hochu :-) Koukni na itemToAdd->name o radku vys, ale musis to zachytit uz o radku vys, protoze ted je itemToAdd uz vydeletovana!
	return ret;
}

CComOBBase* CComSymbolTable::FindLastRec(const char* name, int* count)
{
	int id, lastID;

	assert(name && count);
	*count = 0;
	lastID = id = FindFirst(name);
	while(id != cUnknown)
	{
		(*count)++;
		id = FindNext(name);
	}
	return lastID != cUnknown ? Get(lastID) : 0;
}

CComOBBase* CComSymbolTable::Get(int id)
{
	assert(id >= reserved);
	return symbolTable.Get(id - reserved);
}


CComOBConstant* CComConstantTable::FindOrAddConstant(int value, int* cid)
{
	char buf[15];
	sprintf(buf, "%d", value);

	CComOBConstant* c;
	int id;

	id = Find(buf);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)Get(id);
		assert(c);
		if(c->dType == kwInt)
		{
			if(cid)
				*cid = id;
			return c;
		}
		id = FindNext(buf);			
	}
	CComOBConstant* newConst = new CComOBConstant(buf, kwInt);
	assert(newConst);
	id = Add(newConst);
	c = (CComOBConstant*)Get(id);
	if(cid)
		*cid = id;
	return c;
}

CComOBConstant* CComConstantTable::FindOrAddConstant(double value, int* cid)
{
	char buf[30];

	sprintf(buf, "%f", value);

	CComOBConstant* c;
	int id;

	id = Find(buf);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)Get(id);
		assert(c);
		if(c->dType == kwDouble)
		{
			if(cid)
				*cid = id;
			return c;
		}
		id = FindNext(buf);			
	}
	CComOBConstant* newConst = new CComOBConstant(buf, kwDouble);
	assert(newConst);
	id = Add(newConst);
	c = (CComOBConstant*)Get(id);
	if(cid)
		*cid = id;
	return c;
}

CComOBConstant* CComConstantTable::FindOrAddConstant(const char* value, int* cid)
{
	CComOBConstant* c;
	int id;

	id = Find(value);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)Get(id);
		assert(c);
		if(c->dType == kwString)
		{
			if(cid)
				*cid = id;
			return c;
		}
		id = FindNext(value);			
	}
	CComOBConstant* newConst = new CComOBConstant(value, kwInt);
	assert(newConst);
	id = Add(newConst);
	c = (CComOBConstant*)Get(id);
	if(cid)
		*cid = id;
	return c;
}

int CComConstantTable::FindNull(int nullToken)
{
	switch(nullToken)
	{
	case kwNull:	return nullID;
	case kwNNull:	return nnullID;
	case kwONull:	return onullID;
	case kwANullChar:	return anullcharID;
	case kwANullInt:	return anullintID;
	case kwANullDouble:	return anulldoubleID;
	case kwANullObjptr:	return anullobjptrID;
	case kwANullName:	return anullnameID;
	case kwANullVoid:	return anullvoidID;
	default:
		assert(false);
		return cUnknown;
	}
}

void CComConstantTable::FillNullTokens()
{
	CComOBConstant* c;
	
	// null
	c = new CComOBConstant("null", cUnknown, "0");		
	assert(c);
	c->dType = kwVoid;
	c->pointer = 1;
	c->value.intData = 0;
	nullID = Add(c);

	// nnull
	c = new CComOBConstant("nnull", cUnknown, "0");		
	assert(c);
	c->dType = kwName;
	c->pointer = 1;
	c->value.intData = cUnknown;
	nnullID = Add(c);

	// anullchar
	c = new CComOBConstant("anullchar", cUnknown, "0");		
	assert(c);
	c->dType = kwCharArray;
	c->pointer = 1;
	c->value.intData = 0;
	anullcharID = Add(c);

	// anullint
	c = new CComOBConstant("anullint", cUnknown, "0");		
	assert(c);
	c->dType = kwIntArray;
	c->pointer = 1;
	c->value.intData = 0;
	anullintID = Add(c);

	// anulldouble
	c = new CComOBConstant("anulldouble", cUnknown, "0");		
	assert(c);
	c->dType = kwDoubleArray;
	c->pointer = 1;
	c->value.intData = 0;
	anulldoubleID = Add(c);

	// anullobjptr
	c = new CComOBConstant("anullobjptr", cUnknown, "0");		
	assert(c);
	c->dType = kwObjptrArray;
	c->pointer = 1;
	c->value.intData = 0;
	anullobjptrID = Add(c);

	// anullname
	c = new CComOBConstant("anullname", cUnknown, "0");		
	assert(c);
	c->dType = kwNameArray;
	c->pointer = 1;
	c->value.intData = 0;
	anullnameID = Add(c);

	// anullvoid
	c = new CComOBConstant("anullvoid", cUnknown, "0");		
	assert(c);
	c->dType = kwVoidArray;
	c->pointer = 1;
	c->value.intData = 0;
	anullvoidID = Add(c);


	// onull
	c = new CComOBConstant("onull", cUnknown, "0");		
	assert(c);
	c->dType = kwObjptr;
	c->pointer = 1;
	c->value.intData = 0;
	onullID = Add(c);
}
