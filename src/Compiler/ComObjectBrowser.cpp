/////////////////////////////////////////////////////////////////////////////
//
// ComObjectBrowser.cpp
//
// Implementace CComObjectBrowser a mnoha dalsich  
//	- hierarchicky usporadane tridy, ktere dohromady tvori objectBrowser
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComObjectBrowser.h"
#include "ComConstants.h"
#include "ComPart.h"
#include "ks.h"

/******************************************************************************
 *
 *	CComOBNameDepend
 *
 ******************************************************************************/

CComOBNameDepend::CComOBNameDepend(const char* _sonName, const char* _fatherName, int _version, CComShortPosition _posDefined, int _father, int _son) :
	CComOBBase(_fatherName,_version,_posDefined)
{
	sonName = newstrdup(_sonName);
	assert(sonName);
	assert(son != cUnknown && father != cUnknown);
	father = _father;
	son = _son;
}

CComOBNameDepend::CComOBNameDepend(CComOBNameDepend& obnd) : CComOBBase(obnd)
{
	sonName = newstrdup(obnd.sonName);
	assert(sonName);
	father = obnd.father;
	son = obnd.son;
}

CComOBNameDepend::~CComOBNameDepend()
{
	SAFE_DELETE_ARRAY(sonName);
}

void CComOBNameDepend::DebugToFile(FILE* f)
{
	fprintf(f,"\t<Name>              = %s -> %s\n", sonName, name);
	fprintf(f,"\t<Version>           = %d\n", version);
	fprintf(f,"\t<Position>          = {%d,%d,%d}\t\t// {fileID,line,column}\n", posDefined.fileID, posDefined.line, posDefined.column);
	fprintf(f,"\t<Son>               = %d\n",son);
	fprintf(f,"\t<Father>            = %d\n",father);
}

/******************************************************************************
 *
 *	CComNameSearchElem
 *
 ******************************************************************************/

CComNameSearchElem::~CComNameSearchElem() 
{ 
	CListKElem<CComNameSearchElem*>* p = deeper.el;
	while(p)
	{
		SAFE_DELETE(p->data);
		p = p->next;
	}
	deeper.DeleteAll(); 
	nList.DeleteAll(); 
}

CComNameSearchElem::CComNameSearchElem(CListKElem<CComNSENode>* idLeft, CComOBName* name, int nameID)
{
	assert(idLeft);
	if(idLeft->next)
	{
		CComNameSearchElem* newElem = new CComNameSearchElem(idLeft->next, name, nameID);
		assert(newElem);
		deeper.Add(newElem);
	} else
		nList.Add(CComNSEEntry(name, nameID));

	id = idLeft->data.id;
	version = idLeft->data.version;
}

CComNameSearchElem* CComNameSearchElem::Add(CListKElem<CComNSENode>* ident, CComOBName* name, int nameID)
{
	CListKElem<CComNSENode>* idLeft;
	CComNameSearchElem* first;

	first = 0;
	CComNameSearchElem* last = FindLast(ident, &idLeft, &first);
	
	if(!idLeft)
		last->AddIfNew(name, nameID);
	else
	{
		CComNameSearchElem* newElem = new CComNameSearchElem(idLeft, name, nameID);
		assert(newElem);
		last->deeper.Add(newElem);
	}
	
	if(!first)
		first = deeper.el->data;

	return first;
}


void CComNameSearchElem::AddIfNew(CComOBName* name, int nameID)
{
	CListKElem<CComNSEEntry>* p = nList.el;
	while(p)
	{
		if(p->data.nameID == nameID)
			break;
		p = p->next;
	}

	if(!p)
		nList.Add(CComNSEEntry(name, nameID));
}

CComNameSearchElem* CComNameSearchElem::FindLast(CListKElem<CComNSENode>* ident, CListKElem<CComNSENode>** idLeft, CComNameSearchElem** firstEntry)
{
	CComNameSearchElem* ret;

	if(!ident)
	{
		*idLeft = 0;
		return this;
	}

	CListKElem<CComNameSearchElem*>* p = deeper.el;
	while(p)
	{
		if(p->data->id == ident->data.id)
		{
			if(firstEntry && *firstEntry == 0)
				*firstEntry = p->data;

			if(p->data->version == ident->data.version)
				return p->data->FindLast(ident->next, idLeft, firstEntry);
			else
				if(ident->data.version == findAny)
				{
                    ret = p->data->FindLast(ident->next, idLeft, firstEntry);
					if(!(*idLeft))
						return ret;
					else
						if(firstEntry)
							*firstEntry = 0;
				}
		}	

		p = p->next;
	}
	
	*idLeft = ident;
	return this;
}

CListK<CComNSEEntry>* CComNameSearchElem::FindRec(CListKElem<CComNSENode>* ident)
{
	CListKElem<CComNSENode>* idLeft;

	CComNameSearchElem* last = FindLast(ident, &idLeft, 0);
	
	if(!idLeft)
		return &(last->nList);
	else
		return 0;
}

CComNameSearch::CComNameSearch()
{
	int i, j;
	for(i=0; i<cFastNameSearchSize; i++)
		for(j=0; j<cFastNameSearchMaxVersions; j++)
			fastSearch[i][j] = 0;
}

void CComNameSearch::Add(CListK<CComNSENode>* ident, CComOBName* name, int nameID)
{ 
	CComNameSearchElem* ns = searchRoot.Add(ident->el, name, nameID);
	int i1 = ident->el->data.id - tokUnknownFirst;
	int i2;

	if(i1 < cFastNameSearchSize)
	{
		i2 = 0;
		while(i2 < cFastNameSearchMaxVersions && fastSearch[i1][i2])
			i2++;
		
		if(i2 < cFastNameSearchMaxVersions)
			fastSearch[i1][i2] = ns;
	}
}

CListK<CComNSEEntry>*  CComNameSearch::FindRec(CListK<CComNSENode>* ident) 
{ 
	int i1 = ident->el->data.id - CComPart::IsUnknownToken(ident->el->data.id) ? tokUnknownFirst : knSecond;
	int i2 = 0;

	if(i1 < cFastNameSearchSize)
	{
		while(i2 < cFastNameSearchMaxVersions && fastSearch[i1][i2] && fastSearch[i1][i2]->version != ident->el->data.version)
			i2++;

		if(i2 < cFastNameSearchMaxVersions && fastSearch[i1][i2])
			if(ident->el->next)
				return fastSearch[i1][i2]->FindRec(ident->el->next);
			else
				return &(fastSearch[i1][i2]->nList);
	} 
	return searchRoot.FindRec(ident->el); 
}

/******************************************************************************
 *
 *	CComOBPreprocessor
 *
 ******************************************************************************/
	
CComOBPreprocessor::CComOBPreprocessor(const char* _name, int _type, int _intData, const char* _strData) : 
	CComOBBase(_name,cUnknown,CComShortPosition())
{
	type = _type;
	intData = _intData;
	if(_strData)
	{
		strData = newstrdup(_strData);
		assert(strData);
	} else
		strData = 0;
}

CComOBPreprocessor::CComOBPreprocessor(CComOBPreprocessor& obp) : CComOBBase(obp)
{
	assert(obp.strData);
	type = obp.type;
	intData = obp.intData;
	if(obp.strData)
	{
		strData = newstrdup(obp.strData);
		assert(strData);
	} else
		strData = 0;
}

CComOBPreprocessor::~CComOBPreprocessor()
{
	SAFE_DELETE_ARRAY(strData);
}


/******************************************************************************
 *
 *	CComOBEditTags
 *
 ******************************************************************************/

CComOBEditTags::CComOBEditTags()
{
	userName = comment = 0;
	valueType = cUnknown;
	limits = cUnknown;
	objET = 0;
	nameAuto = cUnknown;
	editType = eKETdefault;
	nameMask = 0;
	hasDefaultMember = false;
	isOp = cUnknown;
}

CComOBEditTags::~CComOBEditTags()
{
	SAFE_DELETE_ARRAY(userName);
	SAFE_DELETE_ARRAY(comment);
	if(valueType == kwString)
		SAFE_DELETE_ARRAY(defaultValue[0].strData);
}

CComOBEditTags::CComOBEditTags(CComOBEditTags& et)
{
	int i;
	userName= newstrdup(et.userName);
	assert(userName || ! et.userName);
	comment = newstrdup(et.comment);
	assert(comment || ! et.comment);

	valueType = et.valueType;
	if(et.valueType == kwString)
		defaultValue[0].strData = newstrdup(et.defaultValue[0].strData);
	else
		for(i=0; i<et.defaultValue.GetCount(); i++)
			defaultValue.Add(et.defaultValue[i]);

	objET = et.objET;
	limits = et.limits;
	interval[0] = et.interval[0];
	interval[1] = et.interval[1];
	DefaultMember	= et.DefaultMember;
	for(i=0; i<et.list.GetCount(); i++)
		list.Add(et.list[i]);

	nameAuto = et.nameAuto;
	editType = et.editType;
	nameMask = et.nameMask;
	hasDefaultMember = et.hasDefaultMember;
	isOp = et.isOp;
	isName = et.isName;
}

CComOBEditTags& CComOBEditTags::operator=(CComOBEditTags& et)
{
	int i;
	SAFE_DELETE_ARRAY(userName);
	SAFE_DELETE_ARRAY(comment);

	userName= newstrdup(et.userName);
	assert(userName || ! et.userName);
	comment = newstrdup(et.comment);
	assert(comment || ! et.comment);

	if(valueType == kwString)
		SAFE_DELETE_ARRAY(defaultValue[0].strData);
	defaultValue.RemoveAll();

	valueType = et.valueType;
	if(et.valueType == kwString)
	{
		defaultValue.Add(et.defaultValue[0]);
		defaultValue[0].strData = newstrdup(et.defaultValue[0].strData);
	} else
		for(i=0; i<et.defaultValue.GetCount(); i++)
			defaultValue.Add(et.defaultValue[i]);

	objET = et.objET;
	limits = et.limits;
	interval[0] = et.interval[0];
	interval[1] = et.interval[1];
	DefaultMember	= et.DefaultMember;
	list.RemoveAll();
	for(i=0; i<et.list.GetCount(); i++)
		list.Add(et.list[i]);
	
	nameAuto = et.nameAuto;
	editType = et.editType;
	nameMask = et.nameMask;
	hasDefaultMember = et.hasDefaultMember;
	isOp = et.isOp;
	isName = et.isName;

	return *this;
}

void CComOBEditTags::SetUserName(const char* _userName)
{
	SAFE_DELETE_ARRAY(userName);
	userName= newstrdup(_userName);
	assert(userName || _userName);
}

void CComOBEditTags::SetComment(const char* _comment)
{
	SAFE_DELETE_ARRAY(comment);
	comment = newstrdup(_comment);
	assert(comment || _comment);
}

void CComOBEditTags::SetDefaultValue(UComValues _value, int _valueType)
{
	if(valueType == kwString)
		SAFE_DELETE_ARRAY(defaultValue[0].strData);

	valueType = _valueType;
	if(_valueType == kwString)
	{
		UComValues tmp;
		tmp.strData = newstrdup(_value.strData);
		defaultValue.Add(tmp); 
	} else
		defaultValue.Add(_value);
}

void CComOBEditTags::DebugToFile(FILE* f)
{
	fprintf(f,"\t\t[Edit tags]\n");
	if(userName)
		fprintf(f,"\t\t\tUser name     \t %s\n",userName);
	if(comment)
		fprintf(f,"\t\t\tComment       \t %s\n",comment);
	if(valueType != cUnknown)
		fprintf(f,"\t\t\tDefault value \t %d\n", valueType);
	switch(limits)
	{
	case etInterval:
		fprintf(f,"\t\t\tInterval limit\n");
		break;
	case etList:
		fprintf(f,"\t\t\tList limit (%d values)\n", list.GetCount());
		break;
	}
}


/******************************************************************************
 *
 *	CComOBBaseData
 *
 ******************************************************************************/

CComObjectBrowser* CComOBBaseData::objBrowser = 0;

CComOBBaseData::CComOBBaseData(const char* _name, int _version, CComShortPosition _posDefined) : CComOBBase(_name, _version, _posDefined)
{
	storage = dType = kerNameID = cUnknown;
	pointer = 0;
	
	mem = cUnknown;

	compiled = false;
	use = 0;

	kerName = 0;
}

CComOBBaseData::CComOBBaseData(CComOBBaseData& obbd) : CComOBBase(obbd)
{
	storage = obbd.storage;
	dType	= obbd.dType;
	pointer = obbd.pointer;
	kerNameID = obbd.kerNameID;

	mem = obbd.mem;

	compiled = obbd.compiled;
	use = obbd.use;
	edit = obbd.edit;

	int i;
	for(i = 0; i < obbd.array.GetCount(); i++)	
		array.Add(obbd.array[i]);
}

void CComOBBaseData::SetData(int _storage, int _dType, int _pointer)
{
	storage = _storage;
	dType	= _dType;
	pointer = _pointer;
}

int CComOBBaseData::GetSize() const 
{
	int i, mul, ret;

	mul = 1;
	for(i = 0; i < array.GetCount(); i++)
		mul *= array[i];

	if(pointer)
		ret = 4;
	else
		switch(dType)
		{
		case kwChar:	ret = 1;	break;
		case kwDouble:	ret = 8;	break;
		case kwVoid:	ret = 0;	break;
		default:
			if(CComPart::IsStringType(dType))
				ret = dType-kwString+1;
			else
				if(CComPart::IsStructureToken(dType))
					ret = ((CComOBStructure*)objBrowser->sTab->Get(dType))->GetStructureSize();
				else
					ret = 4;
		}

	return ret * mul;
}

int CComOBBaseData::GetMDType(int overType, int overPointer)
{
	int testType, testPointer;

	if(overType != cUnknown)
	{	
		testType = overType;
		testPointer = overPointer;
	} else
	{
		testType = dType;
		testPointer = pointer;

	}
	if((testType>=tokObjectFirst && testType<tokObjectLast || testType==kwObjptr)&& testPointer == 1)
		return eKTobject;

	if(testPointer == 1 || (overType == cUnknown && testPointer - array.GetCount() == 1))
		switch(testType)
		{
		case kwCharArray:	return eKTarrChar;		
		case kwDoubleArray:	return eKTarrDouble;
		case kwIntArray:	return eKTarrInt;
		case kwObjptrArray:	return eKTarrObject;
		case kwNameArray:	return eKTarrName;
		case kwVoidArray:	return eKTarrPointer;
		
		case kwName:	
		case kwObjectName:	
		case kwMethodName:	
		case kwParamName:	
				return eKTname;	
		}

	if(testPointer && (overType != cUnknown || testPointer != array.GetCount()))	
		return eKTpointer;

	switch(testType)
	{
	case kwChar:	return eKTchar;
	case kwDouble:	return eKTdouble;
	case kwInt:		return eKTint;		
	case kwVoid:	return eKTvoid;	
	
	default:
		if(testType>=kwString && testType<=kwStringLast)
			return testType == kwString ? eKTstring + 250 : eKTstring+testType-kwString;
		
		if(testType>=tokStructureFirst && testType < tokStructureLast)
			return eKTstruct;
	}

	return cUnknown;
}

int CComOBBaseData::GetPointer(int overType, int overPointer)
{
	int testType, testPointer;

	if(overType != cUnknown)
	{	
		testType = overType;
		testPointer = overPointer;
	} else
	{
		testType = dType;
		testPointer = pointer;
	}
	if(testType>=tokObjectFirst && testType<tokObjectLast || testType==kwObjptr)
		return testPointer-1;
	else
		return testPointer;
}

int CComOBBaseData::GetMyType(eKerTypes mdType)
{
	switch(mdType)
	{
	case eKTchar:		return kwChar;	
	case eKTdouble:		return kwDouble;
	case eKTint:		
	case eKTpointer:	return kwInt;
	case eKTobject:		return kwObjptr;
	case eKTname:		return kwName;
	case eKTvoid:		return kwVoid;
	case eKTarrChar:	return kwCharArray;
	case eKTarrDouble:	return kwDoubleArray;
	case eKTarrInt:		return kwIntArray;
	case eKTarrPointer:	return kwVoidArray;
	case eKTarrObject:	return kwObjptrArray;
	case eKTarrName:	return kwNameArray;
	case eKTstruct:		return kwStruct;
	}
	assert(false);
	return cUnknown;
}

int CComOBBaseData::GetMyPointer(eKerTypes mdType)
{
	switch(mdType)
	{
	case eKTchar:		case eKTdouble:			case eKTint:		case eKTvoid:		
		return 0;

	case eKTarrChar:	case eKTarrDouble:		case eKTarrInt:		case eKTarrPointer:	
	case eKTarrObject:	case eKTarrName:		case eKTpointer:	case eKTobject:		
	case eKTname:		
		return 1;
	}
	assert(false);
	return cUnknown;
}


void CComOBBaseData::DebugToFile(FILE* f, int tabIndent)
{
	CComOBBase::DebugToFile(f,tabIndent);
	char buf[cMaxOBTabIndent+1];
	int i;
	for(i=0; i< tabIndent; i++)
		buf[i] = '\t';
	buf[i] = '\0';

	fprintf(f,"%s<Data type>         = %d\n", buf,dType);
	fprintf(f,"%s<Storage class>     = %d\n", buf,storage);
	fprintf(f,"%s<Pointer>           = %d\n", buf,pointer);
	fprintf(f,"%s<Memory address>    = %d\n", buf,mem);
	fprintf(f,"%s<Compiled>          = %s\n", buf,BOOLEANSTR(compiled));
	fprintf(f,"%s<Use>               = %d\n", buf,use);
	edit.DebugToFile(f);
	if(array.GetCount())
	{
		fprintf(f, "%s<Array dimensions>\t", buf);
		for(i = 0; i < array.GetCount(); i++)	
			fprintf(f, " %d", array[i]);
		fprintf(f, "\n");
	}
}

/******************************************************************************
 *
 *	CComOBConstant
 *
 ******************************************************************************/

CComOBConstant::CComOBConstant(const char* _name, int _type, const char* _dispName) : 
	CComOBBaseData(_name,cUnknown,CComShortPosition())
{
	SetData(cUnknown,_type,0);
	if(_type != cUnknown)
		Eval();
	
	kerName = _dispName ? newstrdup(_dispName) : newstrdup(_name);
	assert(kerName);
}

CComOBConstant::CComOBConstant(CComOBConstant& obc) : CComOBBaseData(obc)
{
	value = obc.value;
}

CComOBConstant::~CComOBConstant()
{

}

void CComOBConstant::Eval()
{
	int val;
	char* stop;
	int radix;

	switch(dType)
	{
	case kwChar:		value.intData = name[0];			break;
	case kwDouble:		value.doubleData = atof(name);		break;
	case kwString:		value.strData = name;				dType = strlen(name)<250? kwString + strlen(name):249;	break;
	case kwInt: 	
	case kwName:
		if(!(isdigit(name[0]) || name[0] == '-' && isdigit(name[1])) )
			return;
		radix = 10;
		if(name[0] == '0')
			radix = name[1]=='x' ? 16 : 8;
		val = (int)strtoul(name, &stop, radix);
		assert(!stop || *stop=='\0');
		value.intData = val;	
		break;
	default:
		assert(false);
	}
}

/******************************************************************************
 *
 *	CComOBName
 *
 ******************************************************************************/
CComOBName::CComOBName(const char* _name, const char* _kerName, int _version, const char* versionStr, CComShortPosition _posDefined, int _dType) :
	CComOBBaseData(_name,_version,_posDefined)
{
	SetData(cUnknown,_dType,1);
	if(_kerName)
		kerName = newstrdup(_kerName);
	else
	{
		kerName = new char[strlen(_name) + cStrVersionLn + 1 + 7];	// 7 = _KSID_ _
		assert(kerName);

		sprintf(kerName, "_KSID_%s_%s", _name, versionStr);
	}

	objID = metID = cUnknown;
}


CComOBName::CComOBName(CComOBName& obn) : CComOBBaseData(obn)
{
	assert(false);
}

CComOBName::~CComOBName()
{
}

void CComOBName::DebugToFile(FILE* f)
{
	CComOBBase::DebugToFile(f,1);
	fprintf(f,"\t<Type>              = %d\n",dType);
}

/******************************************************************************
 *
 *	CComOBGlobal
 *
 ******************************************************************************/

CComOBGlobal::CComOBGlobal(const char* _name, int _version, CComShortPosition _posDefined,CComSymbolTable* uTab) :
CComOBBaseData(_name,_version,_posDefined)
{
	assert(uTab);
	char* versionStr = uTab->Get(_version)->name;
	kerName = new char[strlen(name)+cStrVersionLn+6+1];	// 6 = 3(KSG) + 3x _
	assert(kerName);
	sprintf(kerName,"_KSG_%s_%s",name,versionStr);
}

CComOBGlobal::CComOBGlobal(CComOBGlobal& obg) : CComOBBaseData(obg)
{
}

CComOBGlobal::~CComOBGlobal()
{
}

void CComOBGlobal::SetData(int _dType, int _pointer)
{
	CComOBBaseData::SetData(cUnknown,_dType, _pointer);
}

void CComOBGlobal::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,1);
}

/******************************************************************************
 *
 *	CComOBAttribute
 *
 ******************************************************************************/

CComOBAttribute::CComOBAttribute(const char* _name, int _version, CComShortPosition _posDefined, CComSymbolTable* uTab, CComOBObject* _obj, int _knownName) 
: CComOBBaseData (_name, _version, _posDefined)
{
	compiled = true;
	use = 0;
	inherit = inhNotInheritable;

	if(_knownName != cUnknown)
		use += (_knownName - knFirst) << KERVARUSESSHIFT;

	knownName = _knownName;
	kerKonkretniID = 0;

	if(!_obj)
		return;

	inObj = objDef = _obj;

	char* versionStr = uTab->Get(_version)->name;
	char* objVersionStr = uTab->Get(inObj->version)->name;

	kerName = new char[strlen(inObj->name)+strlen(objVersionStr)+strlen(name)+cStrVersionLn+8+4+1];	// 4 = KSOV, 8 = 7x _ + M
	assert(kerName);
	if(inObj->knownName)
		sprintf(kerName,"_KSOV_%s__M_%s_%s",inObj->name,name,versionStr);
	else
		sprintf(kerName,"_KSOV_%s_%s__M_%s_%s",inObj->name,objVersionStr,name,versionStr);

	kerKonkretniID = new char[cStrVersionLn+30]; // hruby odhad i pro cislo radku a sloupce
	assert(kerKonkretniID);
	sprintf(kerKonkretniID,"_KSOVA_%d_%d_%s",_posDefined.line,_posDefined.column, versionStr);
}

CComOBAttribute::CComOBAttribute(CComOBAttribute& oba) : CComOBBaseData (oba)
{
	objDef = oba.objDef;
	inObj  = oba.inObj;
	compiled = oba.compiled;
	use = oba.use;
	inherit = oba.inherit;
	knownName = oba.knownName;

	CListKElem<int>* p = oba.accessMethods.el;
	while(p)
	{
		accessMethods.Add(p->data);
		p = p->next;
	}
	kerKonkretniID = newstrdup(oba.kerKonkretniID);
	assert(kerKonkretniID);
}

CComOBAttribute::~CComOBAttribute()
{
	accessMethods.DeleteAll();
	SAFE_DELETE_ARRAY(kerKonkretniID);
}

void CComOBAttribute::SetCompiled(CComOBMethod* sender, bool shouldBeCompiled)
{
	int m;
	CComOBMethod* met;
	assert(inObj);
	compiled = shouldBeCompiled;

	CListKElem<int>* p = accessMethods.el;
	while(p)
	{
		m = p->data - tokMethodFirst;

		assert(m < inObj->mTab->GetCount());
		met = inObj->GetMethod(m);
		assert(met);
		if(met!=sender && met->compiled!=shouldBeCompiled)
			met->SetCompiled(this, shouldBeCompiled);
		p = p->next;
	}
}

void CComOBAttribute::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,2);
	fprintf(f,"\t\t<Inheritance status>= %d\n", inherit);
	fprintf(f,"\t\t<Is compiled>       = %s\n", BOOLEANSTR(compiled));
	fprintf(f,"\t\t<Use flags>         = %d\n", use);
	if(kerKonkretniID)
		fprintf(f,"\t\t<Konkretni ID>      = %s\n", kerKonkretniID);
	fprintf(f,"\t\t<# of acc. methods> = %d\n",accessMethods.num);
	if(accessMethods.num)
	{
		fprintf(f,"\t\t[Accessing methods]\n");
		CListKElem<int>* p = accessMethods.el;
		while(p)
		{
			fprintf(f,"\t\t\tMethod %d\t %s\n",p->data, inObj->mTab->Get(p->data)->name);
			p = p->next;
		}
	}
}

void CComOBAttribute::RecreateKerName(CComSymbolTable* uTab, CComOBObject* newObj)
{
	inObj = newObj;
	SAFE_DELETE_ARRAY(kerName);
	char* versionStr = uTab->Get(version)->name;
	char* objVersionStr = uTab->Get(inObj->version)->name;

	kerName = new char[strlen(inObj->name)+strlen(objVersionStr)+strlen(name)+cStrVersionLn+8+4+1];	// 4 = KSOV, 8 = 7x _ + M
	assert(kerName);
	sprintf(kerName,"_KSOV_%s_%s__M_%s_%s",inObj->name,objVersionStr,name,versionStr);
}

/******************************************************************************
 *
 *	CComOBArgument
 *
 ******************************************************************************/

CComOBArgument::CComOBArgument(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr, CComOBMethod* inMethod, int _knownName, bool global) 
: CComOBBaseData (_name, _version, _posDefined)
{
	assert(inMethod || _knownName==cUnknown && global);
	knownName = _knownName;
	if(versionStr)		// != 0 u safe metod
	{
		if(knownName != cUnknown)
		{
			kerName = new char[strlen(name)+7+1];	// 7 = _KSID_ + _ 
			assert(kerName);
			sprintf(kerName,"_KSID_%s",name);
		} else
			if(global)
			{
				kerName = new char[strlen(name)+cStrVersionLn+7+1];	// 7 = _KSID_ + _ 
				assert(kerName);
				sprintf(kerName,"_KSID_%s_%s",name,versionStr);
			} else
			{
				kerName = new char[strlen(inMethod->kerName)+strlen(name)+cStrVersionLn+5+1];	// 5 = __M_ + _ 
				assert(kerName);
				if(inMethod->knownName != cUnknown)
					sprintf(kerName,"%s__M_%s",inMethod->kerName,name);
				else
					sprintf(kerName,"%s__M_%s_%s",inMethod->kerName,name,versionStr);
			}
	} else			// == 0 u direct metod
	{
		kerName = new char[strlen(name)+5+1];	// 5 = 3(KSL) + 2x _
		assert(kerName);
		sprintf(kerName,"_KSL_%s",name);
	}
}

CComOBArgument::CComOBArgument(CComOBArgument& oba) : CComOBBaseData (oba)
{
	ret = oba.ret;
	knownName = oba.knownName;
}

CComOBArgument::~CComOBArgument()
{
}

void CComOBArgument::SetData(int _ret, int _dType, int _pointer)
{
	CComOBBaseData::SetData(cUnknown,_dType,_pointer);
	ret = _ret;
}

int CComOBArgument::GetMDRet()
{
	switch(ret)
	{
	case cUnknown:	return eKTretNormal;
	case kwRet:		return eKTret;
	case kwRetAnd:	return eKTretAND;
	case kwRetOr:	return eKTretOR;
	case kwRetAdd:	return eKTretADD;
	default:
		assert(false);
	}
	return cUnknown;
}

void CComOBArgument::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,3);
	fprintf(f,"\t\t\t<Return by value>   = %d\n",ret);
}

/******************************************************************************
 *
 *	CComOBLocal
 *
 ******************************************************************************/

CComOBLocal::CComOBLocal(const char* _name, int _version, CComShortPosition _posDefined) 
: CComOBBaseData (_name, _version, _posDefined)
{
	kerName = new char[strlen(name)+5+1];	// 5 = 3(KSL) + 2x _
	assert(kerName);
	sprintf(kerName,"_KSL_%s",name);
}

CComOBLocal::CComOBLocal(CComOBLocal& obl) : CComOBBaseData (obl)
{
	blockNr = obl.blockNr;
	blockDepth = obl.blockDepth;
}

CComOBLocal::~CComOBLocal()
{
}

void CComOBLocal::SetData(int _dType, int _pointer, int _blockDepth, int _blockNr)
{
	CComOBBaseData::SetData(cUnknown,_dType,_pointer);
	blockDepth = _blockDepth;
	blockNr = _blockNr;
}


void CComOBLocal::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,3);
	fprintf(f,"\t\t\t<Block number>      = %d\n", blockNr);
	fprintf(f,"\t\t\t<Block depth>       = %d\n", blockDepth);
}

/******************************************************************************
 *
 *	CComOBMethod
 *
 ******************************************************************************/

CComOBMethod::CComOBMethod(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr, CComOBObject* _obj, int storage, int _knownName, bool global) 
: CComOBBaseData (_name, _version,  _posDefined)
{
	aTab = new CComSymbolTable(cArgumentsHTSize,tokArgumentFirst);
	assert(aTab);

	lTab = new CComSymbolTable(cLocalsHTSize,tokLocalFirst);
	assert(lTab);

	assert(inObj || _knownName==cUnknown && global && storage==kwSafe);

	objDef = inObj = _obj;
	id = cUnknown;

	knownName = _knownName;
	if(knownName != cUnknown)
	{
		kerName = new char[1+strlen(KnownNames[GetMDKnownName()].namestr)];
		assert(kerName);
		strcpy(kerName, KnownNames[GetMDKnownName()].namestr);

		kerKonkretniID = new char[strlen(KnownNames[GetMDKnownName()].namestr)+cStrVersionLn+30]; // hruby odhad i pro cislo radku
		assert(kerKonkretniID);
		sprintf(kerKonkretniID,"_KSM_%d_%s_%s",_posDefined.line,KnownNames[GetMDKnownName()].namestr+6,versionStr);
	} else
	{
		if(storage == kwSafe)		
			if(!global)
			{
				kerName = new char[strlen(inObj->kerName)+strlen(name)+cStrVersionLn+5+1];	// 5 = __M_ + _ 
				assert(kerName);
				if(inObj->knownName)
					sprintf(kerName,"%s__M_%s",inObj->kerName,name);
				else
					sprintf(kerName,"%s__M_%s_%s",inObj->kerName,name,versionStr);
			} else
			{
				kerName = new char[strlen(name)+cStrVersionLn+7+1];	// 7 = _KSID_ + _ 
				assert(kerName);
				sprintf(kerName,"_KSID_%s_%s",name,versionStr);
			}
		else			
		{
			kerName = new char[strlen(name)+strlen(inObj->name)+2*cStrVersionLn+12+1];	// 12 = 4(KSDM) + 7x _ + M
			assert(kerName);
			sprintf(kerName,"_KSDM_%s_%s__M_%s_%s",inObj->name,versionStr,name,versionStr);
		}

		kerKonkretniID = new char[strlen(name)+cStrVersionLn+30]; // hruby odhad i pro cislo radku
		assert(kerKonkretniID);
		sprintf(kerKonkretniID,"_KSM_%d_%s_%s",_posDefined.line,name,versionStr);
	}

	compiled = canBeCompiled = (storage == kwSafe) ? KSMain->KSMs->Member(kerKonkretniID)!=0 :
													 KSMain->KSDMs->Member(kerName)!=0;
	CComOBMethod::global = global;
	returnsValue = hasSafeArgs = false;
	localsSize = localsPos = 0;
	inherit = inhNotInheritable;
	groupType = kwVoid;
	groupPointer = 0;
	ret = cUnknown;
}

CComOBMethod::CComOBMethod(CComOBMethod& obm) 
: CComOBBaseData (obm)
{
	objDef		  = obm.objDef;
	inObj		  = obm.inObj;
	returnsValue  = obm.returnsValue;
	hasSafeArgs	  = obm.hasSafeArgs;
	canBeCompiled = obm.canBeCompiled;
	localsSize	  = obm.localsSize;
	localsPos	  = obm.localsPos;
	id			  = obm.id;
	inherit		  = obm.inherit;
	groupType	  = obm.groupType;
	groupPointer  = obm.groupPointer;
	global		  = obm.global;
	ret			  = obm.ret;

	CListKElem<CComOBMethod*>* p = obm.dangerousCalls.el;
	while(p)
	{
		dangerousCalls.Add(p->data);
		p = p->next;
	}

	CListKElem<int>* q = obm.usedAttributes.el;
	while(q)
	{
		usedAttributes.Add(q->data);
		q = q->next;
	}

	aTab = new CComSymbolTable(cArgumentsHTSize,tokArgumentFirst);
	assert(aTab);
	int i;
	for(i=0; i<obm.aTab->GetCount(); i++)
		aTab->Add(new CComOBArgument(*obm.GetArgument(i)));
	
	lTab = new CComSymbolTable(cLocalsHTSize,tokLocalFirst);
	assert(lTab);
	for(i=0; i<obm.lTab->GetCount(); i++)
		lTab->Add(new CComOBLocal(*obm.GetLocal(i)));

	kerKonkretniID = newstrdup(obm.kerKonkretniID);
	assert(kerKonkretniID);
	knownName = obm.knownName;
}

CComOBMethod::~CComOBMethod()
{
	SAFE_DELETE(aTab);
	SAFE_DELETE(lTab);
	dangerousCalls.DeleteAll();
	SAFE_DELETE_ARRAY(kerKonkretniID);
}


int CComOBMethod::GetMDKnownName()
{
	switch(knownName)
	{
	case kwConstructor:		return eKKNconstructor;
	case kwDestructor:		return eKKNdestructor;
	case kwLConstructor:	return eKKNloadConstructor;
	case kwCConstructor:	return eKKNcopyConstructor;
	default: 
		return knownName-knSecond;
	}
	return cUnknown;
}

void CComOBMethod::SetData(int _storage, int _dType, int _pointer)
{
	CComOBBaseData::SetData(_storage,_dType,_pointer);
	returnsValue = _dType != kwVoid || pointer;
}

void CComOBMethod::SetCompiled(CComOBAttribute* sender, bool shouldBeCompiled)
{
	int a;
	CComOBAttribute* atr;
	assert(inObj);
	compiled = shouldBeCompiled;

	CListKElem<int>* p = usedAttributes.el;
	while(p)
	{
		a = p->data - tokAttributeFirst;
		assert(a < inObj->aTab->GetCount());
		atr = inObj->GetAttribute(a);
		assert(atr);
		if(atr!=sender && atr->compiled!=shouldBeCompiled)
			atr->SetCompiled(this, shouldBeCompiled);
		p = p->next;
	}
}

void CComOBMethod::ComputeLocals()
{
	CComOBBaseData* p;

	localsPos = 0;
	int i;
	for(i=0; i<aTab->GetCount(); i++)
	{
		p = GetArgument(i);
		p->mem = localsPos;
		localsPos += p->GetSize();
	}
	assert(!compiled);
	localsSize = 0;
	if(storage == kwSafe)
		localsPos += aTab->GetCount();

	for(i=0; i<lTab->GetCount(); i++)
	{
		p = GetLocal(i);
		p->mem = localsPos + localsSize;
		localsSize += p->GetSize();
	}
}

void CComOBMethod::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,2);
	if(kerKonkretniID)
		fprintf(f,"\t\t<Konkretni ID>      = %s\n", kerKonkretniID);
	fprintf(f,"\t\t<Returns value>     = %s\n", BOOLEANSTR(returnsValue));
	fprintf(f,"\t\t<Safe arg list>     = %s\n", BOOLEANSTR(hasSafeArgs));
	fprintf(f,"\t\t<Can be compiled>   = %s\n", BOOLEANSTR(canBeCompiled));
	fprintf(f,"\t\t<Is compiled>       = %s\n", BOOLEANSTR(compiled));
	fprintf(f,"\t\t<Locals size>       = %d\n", localsSize);
	fprintf(f,"\t\t<Locals pos>        = %d\n", localsPos);
	fprintf(f,"\t\t<ID>                = %d\n", id);
	fprintf(f,"\t\t<Group type>        = %d\n", groupType);
	fprintf(f,"\t\t<Group pointer>     = %d\n", groupPointer);
	fprintf(f,"\t\t<Is inheritable>    = %s\n", BOOLEANSTR(inherit));
	fprintf(f,"\t\t<Global>            = %s\n", BOOLEANSTR(global));
	fprintf(f,"\t\t<Ret status>        = %d\n", ret);
	if(knownName != cUnknown)
		fprintf(f,"\t\t<Known name>        = %d\n", knownName);

	fprintf(f,"\t\t<# of arguments>    = %d\n", aTab->GetCount());
	if(aTab->GetCount())
		for(int i=0; i<aTab->GetCount(); i++)
		{
			fprintf(f,"\t\t[Argument %d]\n",i);
			GetArgument(i)->DebugToFile(f);
		}

	fprintf(f,"\t\t<# of locals>       = %d\n", lTab->GetCount());
	if(lTab->GetCount())
		for(int i=0; i<lTab->GetCount(); i++)
		{
			fprintf(f,"\t\t[Local %d]\n",i);
			GetLocal(i)->DebugToFile(f);
		}

	fprintf(f,"\t\t<# of dang. calls>  = %d\n", dangerousCalls.num);
	if(dangerousCalls.num)
	{
		fprintf(f,"\t\t[Dangerous calls]\n");
		CListKElem<CComOBMethod*>* p = dangerousCalls.el;
		while(p)
		{
			fprintf(f,"\t\t\tMethod %s\n", p->data->kerName);
			p = p->next;
		}
	}

	fprintf(f,"\t\t<# of used attrib.> = %d\n", usedAttributes.num);
	if(usedAttributes.num)
	{
		fprintf(f,"\t\t[Used attributes]\n");
		CListKElem<int>* p = usedAttributes.el;
		while(p)
		{
			fprintf(f,"\t\t\tAttribute %d \t %s\n", p->data, inObj->aTab->Get(p->data)->name);
			p = p->next;
		}
	}
}

/******************************************************************************
 *
 *	CComOBStructure
 *
 ******************************************************************************/

CComOBStructure::CComOBStructure(const char* _name, CComOBObject* inObj, const char* versionStr, CComShortPosition _posDefined) 
: CComOBBaseData(_name, inObj->version, _posDefined)
{
	kerName = new char[strlen(_name)+cStrVersionLn+30];		// 30 - horni odhad i pro cislo radku
	assert(kerName);
	sprintf(kerName, "_KSVS_%d_%s_%s", _posDefined.line, _name, versionStr);

	mTab = new CComSymbolTable(cStructureMembersHTSize, 0);
	assert(mTab);

	size = cUnknown;
}

CComOBStructure::CComOBStructure(CComOBStructure& obs)
: CComOBBaseData(obs)
{
	mTab = new CComSymbolTable(cStructureMembersHTSize,0);
	assert(mTab);
	for(int i=0; i<obs.mTab->GetCount(); i++)
		mTab->Add(new CComOBBaseData(*(CComOBBaseData*)obs.mTab->Get(i)));

	size = obs.size;
}

CComOBStructure::~CComOBStructure()
{
	SAFE_DELETE(mTab);
}

int CComOBStructure::GetStructureSize()
{
	if(size != cUnknown)
		return size;
	else
	{
		int i;

		size = 0;
		for(i = 0; i < mTab->GetCount(); i++)
			size += ((CComOBBaseData*)mTab->Get(i))->GetSize();

		return size;
	}
}

void CComOBStructure::ComputeMembers()
{
	int offset, i;
	CComOBBaseData* d;

	offset = 0;
	for(i = 0; i < mTab->GetCount(); i++)
	{
		d = (CComOBBaseData*)mTab->Get(i);
		d->mem = offset;
		offset += d->GetSize();
	}
	
}

void CComOBStructure::DebugToFile(FILE* f)
{
	CComOBBaseData::DebugToFile(f,2);

	CComOBBaseData* d;
	fprintf(f,"\t\t<# of members>      = %d\n", mTab->GetCount());
	for(int i=0; i<mTab->GetCount(); i++)
	{
		fprintf(f,"\t\t[Member %d]\n", i);
		d = (CComOBBaseData*)mTab->Get(i);
		assert(d);
		d->DebugToFile(f, 3);
	}
}

CComOBAttributeGroup::CComOBAttributeGroup(CComOBAttributeGroup& ag)
{ 
	type	= ag.type;
	grpAtr	= new CComOBAttribute(*ag.grpAtr);
	assert(grpAtr);

	members.RemoveAll();  
	for(int i=0; i<ag.members.GetCount(); i++)
		members.Add(ag.members[i]);
}

/******************************************************************************
 *
 *	CComOBObject
 *
 ******************************************************************************/

CComOBObject::CComOBObject(const char* _name, int _version, CComShortPosition _posDefined, const char* versionStr) 
: CComOBBase (_name, _version, _posDefined)
{
	aTab = new CComSymbolTable(cAttributesHTSize,tokAttributeFirst);
	assert(aTab);
	mTab = new CComSymbolTable(cMethodsHTSize,tokMethodFirst);
	assert(mTab);
	
	kerName = new char[strlen(name)+cStrVersionLn+7+1];	// 7 = 4(KSID) + 3x _
	assert(kerName);
	if(versionStr)
		sprintf(kerName,"_KSID_%s_%s",name,versionStr);
	else
		sprintf(kerName,"_KSID_%s",name);					// pro objekty znamych jmen

	isdSize = 0;
	ksvg = 0;
	kerNameID = cUnknown;
	knownName = versionStr == 0;
}

CComOBObject::CComOBObject(CComOBObject& obo) 
: CComOBBase(obo)
{
	int i;

	aTab = new CComSymbolTable(cAttributesHTSize,tokAttributeFirst);
	assert(aTab);

	for(i=0; i<obo.aTab->GetCount(); i++)
		aTab->Add(new CComOBAttribute(*obo.GetAttribute(i)));

	mTab = new CComSymbolTable(cMethodsHTSize,tokMethodFirst);
	assert(mTab);
	for(i=0; i<obo.mTab->GetCount(); i++)
		mTab->Add(new CComOBMethod(*obo.GetMethod(i)));

	isdSize		= obo.isdSize;
	ksvg		= obo.ksvg;
	kerNameID	= obo.kerNameID;
	edit		= obo.edit;
	knownName	= obo.knownName;

	CComOBAttribute* atr;
	for(i=0; i<obo.scrList.GetCount(); i++)
	{
		atr = new CComOBAttribute(*(obo.scrList[i]));
		scrList.Add(atr);
	}
}

CComOBObject::~CComOBObject()
{
	SAFE_DELETE(aTab);
	SAFE_DELETE(mTab);
	
	CComOBAttributeGroup* atrGrp;
	while(gList.GetCount())
	{
		atrGrp = gList.RemoveHead();
		SAFE_DELETE(atrGrp);
	}
	
	CComOBAttribute* atr;
	while(scrList.GetCount())
	{
		atr = scrList.RemoveHead();
		SAFE_DELETE(atr);
	}
}

void CComOBObject::RecreateKerName(CComSymbolTable* uTab)
{
	SAFE_DELETE_ARRAY(kerName);
	assert(uTab);
	char* versionStr = uTab->Get(version)->name;
	kerName = new char[strlen(name)+cStrVersionLn+7+1];	// 7 = 4(KSID) + 3x _
	assert(kerName);
	sprintf(kerName,"_KSID_%s_%s",name,versionStr);
	kerNameID = cUnknown;

	for(int i=0; i<aTab->GetCount(); i++)
		GetAttribute(i)->RecreateKerName(uTab, this);
}

void CComOBObject::ComputeAttributes()
{
	CComOBAttribute* attr;
	isdSize = 0;
	for(int i=0; i<aTab->GetCount(); i++)
	{
		attr = GetAttribute(i);
		if(!attr->compiled)
		{
			attr->mem = isdSize;		
			isdSize += attr->GetSize();
		} else
		{
			// TODO:
			//	ksvg++;
		}
	}
}

int CComOBObject::FitInKSVG(CKSKSVG* pKSVG)
{
	int i, ret;
	CComOBAttribute* a;

	ret = 0;
	for(i = 0; i < aTab->GetCount(); i++)
	{
		a = GetAttribute(i);
		if(pKSVG->attributes->Member(a->kerName))
			ret++;
	}

	return ret;
}

bool CComOBObject::FitAllInKSVG(CKSKSVG* pKSVG)
{
	int i;
	CComOBAttribute* a;

	for(i = 0; i < aTab->GetCount(); i++)
	{
		a = GetAttribute(i);
		if(!pKSVG->attributes->Member(a->kerName))
			return false;
	}

	return true;
}

void CComOBObject::SetKSVG(CKSKSVG* pKSVG)
{
	int i;
	CComOBAttribute* a;

	ksvg = pKSVG;
	for(i = 0; i < aTab->GetCount(); i++)
	{
		a = GetAttribute(i);
		a->compiled = ksvg && ksvg->attributes->Member(a->kerName) != 0;
	}
}


void CComOBObject::DebugToFile(FILE* f)
{
	CComOBBase::DebugToFile(f,1);
	fprintf(f,"\t<Size in IS>        = %d\n", isdSize);
	if(ksvg)
		fprintf(f,"\t<KSVG>              = %s\n", ksvg->name);
	edit.DebugToFile(f);

	fprintf(f,"\t<# of attributes>   = %d\n\n", aTab->GetCount());
	int i;
	for(i=0; i<aTab->GetCount(); i++)
	{
		fprintf(f,"\t[Attribute %d]\n",i);
		GetAttribute(i)->DebugToFile(f);
	}
	fprintf(f,"\n\t<# of methods> = %d\n\n", mTab->GetCount());
	for(i=0; i<mTab->GetCount(); i++)
	{
		fprintf(f,"\t[Method %d]\n",i);
		GetMethod(i)->DebugToFile(f);
	}
}



/******************************************************************************
 *
 *	CComObjectBrowser
 *
 ******************************************************************************/

CComObjectBrowser::CComObjectBrowser(CComSymbolTable* _uTab, CComConstantTable* _cTab, CComSymbolTable* _oTab, CComSymbolTable* _nTab, CComSymbolTable* _dTab, CComSymbolTable* _gTab, CComSymbolTable* _pTab, 
	CComSymbolTable* _mTab, CComSymbolTable* _aTab,	CComSymbolTable* _dnTab, CComSymbolTable* _gnTab, CComSymbolTable* _sTab,
	CKerNamesMain* _names, CComNameSearch* _nameSearch)
{
	assert(_uTab && _cTab && _oTab && _nTab && _dTab && _gTab && _pTab && _mTab && _aTab && _dnTab && _gnTab && _sTab && _names && _nameSearch);
	uTab = _uTab;
	cTab = _cTab;
	oTab = _oTab;
	nTab = _nTab;
	dTab = _dTab;
	gTab = _gTab;
	pTab = _pTab;
	
	mTab = _mTab;
	aTab = _aTab;

	dnTab = _dnTab;
	gnTab = _gnTab;
	sTab = _sTab;

	names = _names;
	nameSearch = _nameSearch;

	CComOBBaseData::objBrowser = this;
	generatingCode = false;
}

CComObjectBrowser::~CComObjectBrowser()
{
	SAFE_DELETE(uTab);
	SAFE_DELETE(cTab);
	SAFE_DELETE(oTab);
	SAFE_DELETE(nTab);
	SAFE_DELETE(dTab);
	SAFE_DELETE(gTab);
	SAFE_DELETE(pTab);

	SAFE_DELETE(mTab);
	SAFE_DELETE(aTab);

	SAFE_DELETE(dnTab);
	SAFE_DELETE(gnTab);
	SAFE_DELETE(sTab);

	SAFE_DELETE(names);
	SAFE_DELETE(nameSearch);
}

int CComObjectBrowser::DebugToFile(const char* fileName)
{
	FILE* f = fopen(fileName,"w");
	if(!f)
		return cError;
	
	fprintf(f,"-- This file was auto-generated by the object browser. --\n\n");
	fprintf(f,"<# of objects> = %d\n\n", oTab->GetCount());
	for(int i=0; i<oTab->GetCount(); i++)
	{
		fprintf(f,"[Object %d]\n",i);
		GetObject(i)->DebugToFile(f);
	}

	fprintf(f,"\n<# of names>  = %d\n\n", nTab->GetCount());
	for(int i=0; i<nTab->GetCount(); i++)
	{
		fprintf(f,"[Name %d]\n",i);
		GetName(i)->DebugToFile(f);
	}

	fprintf(f,"\n<# of name dependencies> = %d\n\n", dTab->GetCount());
	for(int i=0; i<dTab->GetCount(); i++)
	{
		fprintf(f,"[Dependency %d]\n",i);
		GetDepend(i)->DebugToFile(f);
	}

	fprintf(f,"\n<# of globals>           = %d\n\n", gTab->GetCount());
	for(int i=0; i<gTab->GetCount(); i++)
	{
		fprintf(f,"[Global %d]\n",i);
		GetGlobal(i)->DebugToFile(f);
	}

	fprintf(f,"\n\t<# of structures> = %d\n\n", sTab->GetCount());
	for(int i=0; i<sTab->GetCount(); i++)
	{
		fprintf(f,"\t[Structure %d]\n",i);
		GetStructure(i)->DebugToFile(f);
	}
	fclose(f);
	return cOK;
}