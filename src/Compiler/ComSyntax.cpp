#include "stdafx.h"
#include "ComSyntax.h"
#include "ComFunctions.h"
#include "ks.h"
#include "ComKerServices.h"
#include "objects.h"


CComSyntax::CComSyntax(CComObjectBrowser* _objBrowser, int _queueSize) 
: CComPart(_objBrowser, _queueSize)
{
	look = old = cUnknown;

	toRemove = new CComSymbolTable(32, 0);
	toModify = new CComSymbolTable(32, 0);
	removed  = new CComSymbolTable(32, 0);
	modified = new CComSymbolTable(32, 0);

	assert(toRemove && toModify && removed && modified);

	doingParameters = attributeGroup = secondPass = false;

	maxNameLn = 4;
	nameStr = new char[maxNameLn+1];
	assert(nameStr);

	tmpArraySize = 64;
	tmpArray = new CComNSENode[tmpArraySize];
	assert(tmpArray);

	defaultEdit = new CComOBBaseData("default edit", cUnknown, CComShortPosition());
	assert(defaultEdit);
	defaultEdit->SetData(cUnknown, cUnknown, cUnknown);

	MDtestTab = new CComSymbolTable(cMethodsHTSize, 0);
	assert(MDtestTab);

	inputTokens = 0;

	removedNr = modifiedNr = 0;
}

CComSyntax::~CComSyntax()
{
	SAFE_DELETE(toRemove);
	SAFE_DELETE(toModify);
	SAFE_DELETE(removed);
	SAFE_DELETE(modified);
	inheritedAttributes.DeleteAll();
	SAFE_DELETE_ARRAY(nameStr);
	SAFE_DELETE_ARRAY(tmpArray);
	SAFE_DELETE(defaultEdit);
	SAFE_DELETE(MDtestTab);
}

void CComSyntax::DoPart()
{
	if(look == cUnknown)
		Step();

	while(look != lexEOF)
		DoTopDecl();

	Put(lexEOF);
	DoPostPass();
}


void CComSyntax::Step()
{ 
	if(look == lexEOF)
		throw CComError(errUnexpectedEOF);
	old = look; 
    if(inputTokens)
	{
		look = inputTokens->RemoveHead();
		if(!inputTokens->num)
			inputTokens = 0;
	} else
		look = GetParent();
}

void CComSyntax::RedirectInput(CListK<int>* tokens) 
{ 
	tokens->AddTail(look);
	look = tokens->RemoveHead(); 
	inputTokens = tokens; 
}

void CComSyntax::MatchLexical(int token, int note)
{
	if(look != token)
		Expected(token,errExpectedSymbol,note);
	Step();
}

void CComSyntax::MatchKeyword(int token, int note)
{
	if(look != token)
		Expected(token,errExpectedKeyword,note);
	Step();
}

void CComSyntax::MatchIdentifier(CComOBBase** ute, int note)
{
	CComOBBase* ste;
	identPos = pos;

	if(IsUnknownToken(look))
		ste = uTab->Get(look);
	else
		if(IsKnownNameToken(look))
		{
			const char* idName = KnownNames[look-knSecond].namestr+6;
			int id = uTab->FindOrAdd(idName, new CComOBBase(idName, pos.version, pos));
			ste = uTab->Get(id);
		} else
			Expected(look,errExpectedIdentifier,note);

	if(ute)
		*ute = ste;
	identPos.column -= strlen(ste->name)-1;
	Step();
}

CComOBConstant* CComSyntax::MatchConstant(int type, int note, int* cid)
{
	constType = type;
	constConstant = 0;
	constID = cUnknown;
	reqType = type;
	cs.Reset();
	DoConstExpression();
	cs.Load1();

	if(!constConstant)
	{
		switch(constType)
		{
		case kwInt:		
			constConstant = cTab->FindOrAddConstant(cs.e1.value.intData, &constID);	
			break;

		case kwDouble:
			constConstant = cTab->FindOrAddConstant(cs.e1.value.doubleData, &constID);	
			break;

		default:
			assert(false);
		}
	}

	if(cid)
		*cid = constID;

	return constConstant;
}

void CComSyntax::Expected(int token, int error, int note)
{
	throw CComError(error,note,GetTokenStr(token));
}

void CComSyntax::LocalizeIdent(CListK<CComNSENode>* what, CListK<CComNSENode>* towhich)
{
	int i = 0;
	
	CComNSENode* tmp = towhich->num > tmpArraySize ? new CComNSENode[towhich->num] : tmpArray;
	assert(tmp);

    CListKElem<CComNSENode>* p = towhich->el;
	while(p)
	{
		tmp[i++] = p->data;
		p = p->next;
	}

	while(i--)
		what->Add(tmp[i]);

	if(towhich->num > tmpArraySize)
		SAFE_DELETE_ARRAY(tmp);
}

void CComSyntax::SetIdent(CListK<CComNSENode>* what, CListK<CComNSENode>* towhich)
{
	int i = 0;
	what->DeleteAll();
	LocalizeIdent(what, towhich);
}

void CComSyntax::DoPanicPickDirties()
{
	try
	{
		while(look != lexEOF)
			switch(look)
			{
			case kwRemove:		DoModifyOrRemove(dotRemove);		break;
			case kwModify:		DoModifyOrRemove(dotModify);		break;
			default:
				Step();
			}
	} catch(CComError e)
	{

	}
}

void CComSyntax::DoTopDecl()
{
	//	look je nastaveny na zacatku toplevel deklarace (object,name,...)
	switch(look)
	{
	case kwObject:
		DoObjectDecl();
		break;
	
	case kwVoidName:
	case kwObjectName:
	case kwMethodName:
	case kwParamName:
		DoNameDef();
		break;
	
	case kwDepends:
		Step();
		MatchLexical(lexStartBlock, noteNone);
		while(look != lexEndBlock)
		{
			DoNameDepend();
			MatchLexical(lexSeparator, noteForgetSeparator);
		}
		Step();		// pryc s koncem bloku
		break;
	
	case kwDepend:
		Step();
		DoNameDepend();
		break;

	case kwRemove:		DoModifyOrRemove(dotRemove);		break;

	case kwModify:		DoModifyOrRemove(dotModify);		break;
	
	case kwInherit:		DoModifyOrRemove(dotInherit);		break;
	
	default:
		DoGlobalDef();
	}
	if(old != lexEndBlock && old != kwEndModify)
		MatchLexical(lexSeparator, noteForgetSeparator);
	
	while(look == lexSeparator)
		Step();
}

void CComSyntax::DoObjectDecl()
{
	CComOBBase* n;
	CComOBObject* oldObj;
	CComOBObject* newObj;
	const char* objName;
	int id;

	assert(!inObj);
	assert(look==kwObject); 	// look je nastaven na klicove slovo 'object'
	Step();

	Put(synStartObjectDecl);	// Zacinam deklaraci objektu 
	if(IsKnownNameToken(look))
	{
		int knid;
		knid = look - knSecond;
		Step();					// pryc se znamym jmenem
		objName = KnownNames[knid].namestr+6;
		if(KnownNames[knid].type != eKerNTobject)
			throw CComError(errExpectedObjectType, noteNone, objName);

		if((id = oTab->FindVersion(objName,findNewest)) == cUnknown)
		{
			int kerNameID;
			bool success;
			CKerName* newName;
			CComOBName* n;

			CComShortPosition posDefined = oldPos;
			posDefined.column -= strlen(objName+4);
			CComOBObject* obj =	new CComOBObject(objName, pos.version, posDefined, 0);
			id = oTab->Add(obj);

			n = new CComOBName(obj->name, obj->kerName, pos.version, uTab->Get(pos.version)->name, identPos, kwObjectName);
			CheckGlobalName(n->kerName, 0);

			kerNameID = nTab->FindOrAdd(n->kerName, n, &success);
			if(success)
				n = (CComOBName*)nTab->Get(kerNameID);

			newName = new CKerName(obj->kerName, names);
			assert(newName);
			newName->Type = eKerNTobject;

			newName->objBrowserLink = obj;
			obj->kerNameID = kerNameID;
		}
	} else
	{
		DoNameCompleteParse(&n, &id);
		assert(n);
		objName = n->name;
		id = oTab->FindVersion(objName,findNewest);
	}
	
	// hledam objekt stejneho jmena v nejnovejsi verzi -> dodeklarace
	
	if(id != cUnknown)									// dodeklarace
	{
		oldObj = (CComOBObject*)oTab->Get(id);			// oldObj = starsi verze objektu
		assert(oldObj->version!=cUnknown);

		if(oldObj->version == pos.version)	// objekt byl jiz definovan v teto verzi? 
			newObj = oldObj;
		else								// mam pouze objekt starsi verze -> novy zaznam
		{
			int kerNameID;
			bool success;
			CKerName* newName;
			CComOBName* n;

			newObj = new CComOBObject(*oldObj);	// vytvorim presnou kopii stare verze
			assert(newObj);
			newObj->posDefined = oldPos;			// a zmenim pozici deklarace a verzi
			newObj->version = pos.version;
			newObj->RecreateKerName(uTab);

			n = new CComOBName(newObj->name, newObj->kerName, newObj->version, uTab->Get(newObj->version)->name, newObj->posDefined, kwObjectName);
			kerNameID = nTab->FindOrAdd(n->kerName, n, &success);
			if(success)
				n = (CComOBName*)nTab->Get(kerNameID);

			if(!names->GetNamePointer(newObj->kerName))
			{
				newName = new CKerName(n->kerName, names);
				assert(newName);
				newName->Type = eKerNTobject;
				newName->objBrowserLink = newObj;
			}

			names->GetNamePointer(newObj->kerName)->objBrowserLink = newObj;
			
			newObj->kerNameID = kerNameID;
			id = oTab->Add(newObj->name, newObj);
			nameSearch->Add(&ident, n, kerNameID);
		}
	} else
		throw CComError(errNotDeclared, noteNone, objName);

	inObj = newObj;				// kontext: nyni budu zpracovavat TENTO objekt
	SetIdent(&inObjIdent, &ident);

	inObjID = id;
	Put(id, false);
	
	if(inheritAll = look == kwInherit)
		Step();

	MatchLexical(lexStartBlock,noteCheckObject);

	if(look == kwEdit)
		DoObjectEditTags();

	while(look != lexEndBlock)
		DoInObject();

	if(attributeGroup)
		throw CComError(errIncompleteAttributeGroup);

	MatchLexical(lexEndBlock,noteForgetEndBlock);
	Put(synEndObjectDecl,false);
	inObj = 0;
	inObjID = cUnknown;
}

void CComSyntax::DoObjectEditTags()
{
	assert(look == kwEdit);
	Step();				// pryc s kwEdit

	const char* testStr;
	CComOBPreprocessor* pre;
	CComOBConstant* c;

	MatchLexical(lexStartBlock, noteNone);
	while(look != lexEndBlock)
	{
		if(!IsUnknownToken(look))
			throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(look));

		testStr = GetTokenStr(look);
		Step();				// pryc s tagem

		pre = (CComOBPreprocessor*)pTab->FindRec(testStr);
		if(!pre)
			throw CComError(errUnknownEditTag, noteNone, testStr);
		
		switch(pre->intData)
		{
		case etUserName:
			MatchLexical(opAssign, noteNone);
			c = MatchConstant(kwString,noteNone);
			inObj->edit.SetUserName(c->value.strData);
			break;

		case etComment:
			MatchLexical(opAssign, noteNone);
			c = MatchConstant(kwString,noteNone);
			inObj->edit.SetComment(c->value.strData);
			break;

		case etInMap:
			inObj->edit.objET |= etbInMap;
			inObj->edit.objET |= etbOutMap;
			inObj->edit.objET |= etbPodu;
			break;

		case etOutMap:
			inObj->edit.objET |= etbOutMap;
			inObj->edit.objET |= etbPodu;
			break;

		case etEditor:
			inObj->edit.objET |= etbPodu;
			break;
		
		case etNoGrid:
			inObj->edit.objET |= etbNoGrid + etbInMap;
			inObj->edit.objET |= etbPodu;
			break;

		case etCollReplace:
			inObj->edit.objET |= (inObj->edit.objET & etbCollision & 0);
			break;

		case etCollDontPlace:
			inObj->edit.objET |= (inObj->edit.objET & etbCollision & 1);
			break;

		case etCollIgnore:
			inObj->edit.objET |= (inObj->edit.objET & etbCollision & 2);
			break;

		default:
			throw CComError(errInvalidEditTag, noteNone, testStr);
		}

		if(look == opComma)
			Step();
	}			// while look != lexEndBlock
	Step();		// pryc s koncem bloku	

	while(look == lexSeparator)
		Step();
}

void CComSyntax::DoInObject()
{
	int storage, dType, ret;
	bool decl, inherit;

	noSeparator = false;

	DoType(dType,&storage,&ret,&decl,&inherit);

	if(dType == kwScripted)
	{
		if(storage != cUnknown || ret != cUnknown || decl)
			throw CComError(errInvalidType);

		return DoScripted(inherit || inheritAll);
	}

	if(IsAttributeGroupToken(dType))
	{
		if(storage != cUnknown || ret != cUnknown || decl)
			throw CComError(errInvalidType);

		if(attributeGroup)
			throw CComError(errNestedAttributeGroup);

		switch(dType)
		{
		case kwPoint2D:	case kwCell2D:		attributeGroup = 2;		break;
		case kwPoint3D:	case kwCell3D:		attributeGroup = 3;		break;
		case kwArea2D:						attributeGroup = 4;		break;
		case kwArea3D:						attributeGroup = 6;		break;
		default:
			assert(false);
		}
		
		CComOBAttribute* grpAtr;
		int version;

		if(IsUnknownToken(look))
		{
			DoNameParts(&nameStr, maxNameLn, &version);
			CheckMethodAttributeName(nameStr);
			grpAtr = new CComOBAttribute(nameStr, version, pos, uTab, inObj, cUnknown);
		} else
		{
			grpAtr = new CComOBAttribute("voidAtr", pos.version, pos, uTab, inObj, cUnknown);
			version = pos.version;
		}

		assert(grpAtr);

		CComOBAttributeGroup* newAtrGrp = new CComOBAttributeGroup(dType, grpAtr);
		assert(newAtrGrp);
		inObj->gList.Add(newAtrGrp);
		
		defaultEdit->SetData(cUnknown, cUnknown, cUnknown);

		groupInherit = inheritAll;
		grpAtr->inherit = (groupInherit || inherit) ? inhInheritable : inhNotInheritable;
		inheritAll = false;

		if(look == kwEdit)
		{
			defaultEdit->SetData(cUnknown, kwInt, 0);
			DoDataEditTags(defaultEdit);
			grpAtr->edit.userName = defaultEdit->edit.userName;
			grpAtr->edit.comment = defaultEdit->edit.comment;
			defaultEdit->edit.userName = defaultEdit->edit.comment = 0;
		}

		MatchLexical(lexStartBlock);
		DoType(dType,&storage,&ret,&decl,&inherit);
	}

	if(dType == kwStruct || dType == kwUnion)
		DoStruct(dType);
	else
		DoMemberDecl(storage,dType,decl,inherit || inheritAll,ret);

	if(old != lexEndBlock && !noSeparator)
		MatchLexical(lexSeparator,noteForgetSeparator);

	while(look == lexSeparator)
		Step();

	if(attributeGroup == cUnknown)
	{
		attributeGroup = 0;
		MatchLexical(lexEndBlock);
		defaultEdit->SetData(cUnknown, cUnknown, cUnknown);
		inheritAll = groupInherit;
	}

}

void CComSyntax::DoScripted(bool inherit)
{
	CComOBAttribute* scrAtr;
	int version, id;

	Put(kwScripted, false);

	DoNameParts(&nameStr, maxNameLn, &version);
	CheckMethodAttributeName(nameStr);
	scrAtr = new CComOBAttribute(nameStr, version, pos, uTab, inObj, cUnknown);
    scrAtr->dType = kwScripted;
	scrAtr->inherit = inherit ? inhInheritable : inhNotInheritable;

	id = inObj->scrList.Add(scrAtr);
	Put(tokDataFirst+id);

	MatchKeyword(kwConstructor);
	DoNameLater(kwScripted,cUnknown);

	id = cUnknown;
	if(look == opParentStart)
	{
		MatchLexical(opParentStart);
		if(look != opParentEnd)
			MatchConstant(kwInt, noteNone, &id);
		MatchLexical(opParentEnd);
	}

	scrAtr->kerNameID = id;
	if(look == kwEdit)
		DoDataEditTags(scrAtr);
	MatchLexical(lexSeparator);
	noSeparator = true;
}

void CComSyntax::DoType(int& type, int* storage, int* ret, bool* decl, bool *inherit)
{
	CComOBBase* ste;
	bool dontstep = false;
	int ln;

	type =  cUnknown;

	if(storage)
		*storage = cUnknown;

	if(ret)
		*ret = cUnknown;

	if(decl)
		*decl = false;

	if(inherit)
		*inherit = false;

	if(!IsKeywordToken(look))
	{
		int version;
		const char* idName;
		bool wasKN;

		if(wasKN = IsKnownNameToken(look))
		{
			idName = KnownNames[look-knSecond].namestr+6;
			Step();
		} else
		{
			DoNameParts(&nameStr, maxNameLn, &version);
			idName = nameStr;
		}

		type = oTab->Find(idName);
		if(type == cUnknown && inObj)
			type = sTab->Find(nameStr);
		
		if(type == cUnknown)
			throw CComError(errUnknownType, noteNone, wasKN ? idName : GetLongNameStr(&ident));
	}

	while(IsKeywordToken(look))
	{
        switch(look)
		{
		case kwEdit:				// kvuli editu u nepojmenovane skupiny
			if(type == cUnknown)
				throw CComError(errMissingType,noteNone);
			return;			

		case kwRet:		case kwRetAnd:		case kwRetOr:		case kwRetAdd:
			if(!ret)
				throw CComError(errUnexpectedKeyword, noteNone, GetTokenStr(look));

			if(*ret != cUnknown)
				throw CComError(errAlreadyDeclared, noteNone, GetTokenStr(look));
			*ret = look;
			break;

		case kwDecl:
			if(!decl)
				throw CComError(errUnexpectedKeyword, noteNone, GetTokenStr(look));

			if(*decl)
				throw CComError(errAlreadyDeclared, noteNone, GetTokenStr(look));
			*decl = true;
			break;

		case kwInherit:
			if(!inherit)
				throw CComError(errUnexpectedKeyword, noteNone, GetTokenStr(look));

			if(*inherit)
				throw CComError(errAlreadyDeclared, noteNone, GetTokenStr(look));
			*inherit = true;
			break;

		case kwDirect:		case kwSafe:
			if(!storage)
				throw CComError(errUnexpectedKeyword, noteNone, GetTokenStr(look));

			if(*storage != cUnknown)
				throw CComError(errAlreadyDeclared, noteNone, GetTokenStr(look));

			*storage = look;
			break;

		case kwConstructor: case kwDestructor:		case kwLConstructor: 	case kwCConstructor:	case kwUConstructor:
			identPos = pos;
			identPos.column -= strlen(cKeywords[look-kwFirst-1])-1;
			// no break

		case kwName:		case kwObjectName:		case kwParamName:		case kwMethodName:
		case kwChar: 		case kwInt:				case kwDouble:			case kwVoid: 
		case kwCharArray: 	case kwIntArray:		case kwDoubleArray: 	case kwNameArray:	case kwObjptrArray:	case kwVoidArray:
		case kwObjptr:

		case kwPoint2D:		case kwCell2D:	
		case kwPoint3D:		case kwCell3D:	 
		case kwArea2D:		case kwArea3D:
		case kwCellArea2D:	case kwCellArea3D:

		case kwScripted:
			BaseTypeTest(look,type,errMultipleType);
			break;

		case kwString:
			Step();
			if(look == opBracketStart)			// Je zadana delka stringu?
			{
				Step();
				ln = MatchConstant(kwInt)->value.intData;
				if(ln < 0 || ln > cMaxStringLn)
					throw CComError(errInvalidStringLn, noteNone);
				MatchLexical(opBracketEnd, noteNone);
			} else
				ln = cMaxStringLn;
			
			if(ln < 0 || ln > cMaxStringLn)
				throw CComError(errInvalidStringLn, noteNone);
			dontstep = true;

			BaseTypeTest(kwString + ln, type, errMultipleType);
			type = kwString + ln;
			break;

		case kwStruct: 
		case kwUnion: 
			Step();						// pryc se struct
			if(!IsUnknownToken(look))	
				throw CComError(errExpectedIdentifier, noteNone, GetTokenStr(look));
			ste = uTab->Get(look);
			type = sTab->Find(ste->name);
			if(type == cUnknown)	// typ neni znamy -> bude to mozna definice struktury
			{
				type = old;
				dontstep = true;
			}
			break;

		default:
			throw CComError(errUnexpectedKeyword,noteCheckDeclRules,GetTokenStr(look));
		}
		if(!dontstep)
			Step();
		else
			dontstep = false;
	} 

	if(type == cUnknown)
		throw CComError(errMissingType,noteNone);
}

void CComSyntax::DoPointer(int& pointer, int type)
{
	pointer = 0;
	while(look == opMultiply)
	{
		Step();					// pryc s hvezdickou
		pointer++;
	}

	if(IsObjectType(type) || IsDynamicArrayToken(type) || IsNameType(type))
		pointer++;
}

void CComSyntax::BaseTypeTest(int val,int& comp,int error)
{
	char buf[64];
	if(comp != cUnknown)
	{
		sprintf(buf,"%s,%s",cKeywords[comp-kwFirst-1],cKeywords[val-kwFirst-1]);
		throw CComError(error,noteNone,buf);
	}
	comp = val;
}

void CComSyntax::DoStruct(int dType)
{
	CComOBBase* ute;
	bool success;
	int  type, pointer;

	MatchIdentifier(&ute, noteNone);

	char* versionStr = uTab->Get(pos.version)->name;
	assert(versionStr);
	
	CComOBStructure* newStruct = new CComOBStructure(ute->name, inObj, versionStr, pos);
	assert(newStruct);
	newStruct->dType = dType;
	sTab->FindOrAdd(newStruct->name, newStruct, &success);

	if(success)
		throw CComError(errAlreadyDefined, noteNone, newStruct->name);

	if(IsConstantToken(look))
	{
		CComOBConstant* c = (CComOBConstant*)cTab->Get(look);
		sprintf(newStruct->kerName, "_KSVS_%d_%s_%s", c->value.intData, newStruct->name, versionStr);

		Step();			// pryc s konstantou
	}

	MatchLexical(lexStartBlock, noteNone);

	while(look != lexEndBlock)
	{
		DoType(type);

		while(look != lexSeparator)
		{
			DoPointer(pointer, type);

			MatchIdentifier(&ute, noteNone);

			if(look!=opComma && look!=lexSeparator)
				throw CComError(errUnexpectedSymbol, noteForgetSeparator, GetTokenStr(look));

			DoStructMember(newStruct, ute, type, pointer);
			
			if(look == opComma)
				Step();			// pryc s carkou
		}
		Step();		// pryc se strednikem
	}
	Step();			// pryc s lexEndBlock
}

void CComSyntax::DoStructMember(CComOBStructure* newStruct, CComOBBase* ute, int type, int pointer)
{
	assert(newStruct && ute && ute->name);
	assert(look == lexSeparator || look == opComma);
	
	// Najde se stejnojmennou polozku? Ano -> chyba, kolize jmen.
	if(newStruct->mTab->Find(ute->name) != cUnknown)
		throw CComError(errAlreadyDeclared,noteAlreadyDeclaredStructure,ute->name);

	// Tady vim, ze se bud nic nenaslo -> muzu bez obav deklarovat novou polozku.
	CComOBBaseData* newMember = new CComOBBaseData(ute->name,inObj->version,oldPos);
	assert(newMember);
	newMember->SetData(cUnknown, type, pointer);
	newStruct->mTab->Add(newMember);	// pridam polozku do tabulky struktury
}

void CComSyntax::DoMemberDecl(int storage,int type, bool decl, bool inherit, int ret)
{
	CComOBBase* ste = 0;
	CComOBBase tmp;
	int pointer, knownName;
	bool global = true;
	int version;

	knownName = version = cUnknown;
	// pr.: int* f() {} ; 
	// look je nastaven bud na *, pokud tam byla, nebo na identifikator (f)
	DoPointer(pointer, type);

	if(IsSpecialType(type))
	{
		if(look != opParentStart)							
			throw CComError(errExpectedSymbol,noteNone,"()",&oldPos);
		tmp = CComOBBase(cKeywords[type-kwFirst-1],pos.version,pos);
		ste = &tmp;
	} else
		if(IsKnownNameAtrToken(look))
		{
			identPos = pos;
			knownName = look;
			Step();					// pryc s knownName
			tmp = CComOBBase(KerVarUsesNames[knownName-knFirst],pos.version,identPos);
			ste = &tmp;
			identPos.column -= strlen(ste->name)-4;	// -4 = -4 (delka _KN_), +1 (@), -1(vzdycky)
		} else
			if(IsKnownNameToken(look))
			{
				identPos = pos;
				knownName = look;
				Step();					// pryc s knownName
				tmp = CComOBBase(KnownNames[knownName-knSecond].namestr+6,cUnknown,identPos);
				ste = &tmp;
				identPos.column -= strlen(ste->name)-4;	// -4 = -4 (delka _KN_), +1 (@), -1(vzdycky)
			} else
			{
				if(look == opMember4Dot)
				{
					global = false;
					Step();				// pryc se ctyrteckou
				} 
				// ted by mel byt kazdopadne na identifikatoru:

				DoNameParts(&nameStr, maxNameLn, &version);
				tmp = CComOBBase(nameStr,version,identPos);
				ste = &tmp;
			}

	switch(look) 
	{
	case opParentStart:			// funkce
		if(storage == cUnknown)
			storage = kwSafe;

		if(knownName != cUnknown)
		{
			if(IsKnownNameAtrToken(knownName))
				throw CComError(errExpectedAttributeDecl,noteNone,ste->name,&identPos);

			if(KnownNames[knownName-knSecond].type != eKerNTmethod)
				throw CComError(errExpectedMethodKnownName,noteNone,0,&identPos);
		}
		DoMethod(ste->name,storage,type,pointer,knownName, global, decl, inherit, ret,version);	// zpracuji metodu
		break;
	case opComma:				// atribut
	case lexSeparator:
	case kwEdit:
	case opBracketStart:
		if(storage != cUnknown)							// diky specifikovani storage class
			throw CComError(errInvalidDataStorage,noteNone,cKeywords[storage-kwFirst-1],&identPos);

		if(!global)
			throw CComError(errInvalidMemberSpec,noteNone,ste->name);

		if(IsKnownNameToken(knownName))
			throw CComError(errExpectedMethodDecl,noteNone);

		DoAttribute(ste,storage,type,pointer,knownName,inherit,version);	// zpracuji atribut
		while(look == opComma)						// dalsi?
		{
			Step();									// pryc s carkou
			DoMemberDecl(storage,type,false,inherit,ret);		// zpracuji atribut, atd.
		}
		break;
	default:
		throw CComError(errUnexpectedSymbol,noteNotMethodNorAttribute,GetTokenStr(look));
	} 
}

void CComSyntax::DoMethod(const char* idName, int storage, int type, int pointer, int knownName, bool global, bool decl, bool inherit, int ret, int version)
{
	int id, gid;

	// Typ mam, jmeno taky a look ukazuje uz za jmeno na '('
	assert(inObj && idName);
	assert(look == opParentStart);

	if(version == cUnknown)
		version = pos.version;

	const char* versionStr = uTab->Get(version)->name;
	CComOBMethod* mlc = 0;

	if(type == kwUConstructor)
	{
		int ref = PutReference();
		DoMethod("constructor", storage, kwConstructor, pointer, knownName, global, decl, inherit, ret, version);
		CComOBMethod* mc = (CComOBMethod*) inObj->GetMethod(inObj->mTab->GetCount()-1);

		mlc = new CComOBMethod(*mc);
		assert(mlc);
		SAFE_DELETE_ARRAY(mlc->name);
		SAFE_DELETE_ARRAY(mlc->kerName);
		SAFE_DELETE_ARRAY(mlc->kerKonkretniID);
		mlc->name = newstrdup("lconstructor");
		mlc->kerName = newstrdup("_KSID_LoadConstructor");
		mlc->kerKonkretniID = new char[strlen(versionStr)+50];
		assert(mlc->kerKonkretniID);
		strcpy(mlc->kerKonkretniID, "_KSM_");
		int i = 0;
		while(mc->kerKonkretniID[i+5] != '_')	// zkopiruju cislo, 5 == strlen("_KSM_")
		{
			assert(mc->kerKonkretniID[i+5]);
			mlc->kerKonkretniID[i+5] = mc->kerKonkretniID[i+5];
			i++;
		}
		mlc->kerKonkretniID[i+5] = '\0';
		char* p = strcat(mlc->kerKonkretniID+i+5, "_LoadConstructor");
		// 15 == strlen("LoadConstructor"), 11 == strlen("Constructor"), 
		strcat(p + 15, mc->kerKonkretniID+i+6+11);	

		int mlcID = inObj->mTab->Add(mlc);
		inMethod = mlc;
		CopyQueueFromPos(ref);
		RenameMethodOnPos(ref, mlcID);
		inMethod = 0;
		return;
	}

	// Podivam se, jestli objekt nezna neco, co se jmenuje stejne jako metoda, kterou mam deklarovat.
	// Najde se stejnojmenny atribut v jakekoliv verzi? Ano -> chyba, kolize jmen.
	if(inObj->aTab->Find(idName) != cUnknown)
		throw CComError(errAlreadyDeclared,noteAlreadyDeclaredAttribute,idName);

	// Vytvorim novou metodu a hned ji pridam do tabulky objektu - aby to pri vyjimce neskoncilo memory leakem.
	CComOBMethod* newMethod = new CComOBMethod(idName,version,identPos,versionStr,inObj,storage,IsSpecialType(type) ? type : knownName, global);
	assert(newMethod);

	bool metRemoved, metModified;
	CComOBModifyRec* mdfr;
	CComOBBase* b;
	int count;

	if(metModified = ((mdfr = (CComOBModifyRec*)toModify->FindLastRec(newMethod->kerKonkretniID, &count)) != 0 && !inputTokens))		
	{
		RedirectInput(mdfr->tokens);
		DoInObject();
		noSeparator = true;
		modified->Add(newMethod->kerKonkretniID, new CComOBBase());
		modifiedNr += count;
	}
	
	if(metRemoved = ((b = toRemove->FindLastRec(newMethod->kerKonkretniID, &count)) && !noSeparator))	
	{	// - ano -> nepridam si ji do tabulky
		removed->Add(newMethod->kerKonkretniID, new CComOBBase());
		removedNr += count;
		if(b->version == kwInherit)
		{	
			newMethod->inherit = inhInheritable;
			metRemoved = false;
		}
	} 
	
	if(metRemoved || metModified)
	{
		SAFE_DELETE(newMethod);
		SkipMethodFromArgumentList();
		return;
	}

	id = inObj->mTab->Add(newMethod);	
	newMethod->id = id;
	newMethod->inherit = inherit ? inhInheritable : inhNotInheritable;
	newMethod->global = storage == kwSafe ? global : false;
	if(knownName == cUnknown)
		newMethod->ret = ret == cUnknown ? kwRet : ret;
	else
	{
		switch(KnownNames[knownName - knSecond].MethodType & eKTretMask)
		{
		case eKTretAND:	newMethod->ret = kwRetAnd;	break;
		case eKTretADD:	newMethod->ret = kwRetAdd;	break;
		case eKTretOR:	newMethod->ret = kwRetOr;	break;
		case eKTret:	
		case 0:			newMethod->ret = kwRet;	break;
		default:
			assert(false);
		}

		if((KnownNames[knownName - knSecond].MethodType & (eKTret-1))!= newMethod->GetMDType(type, pointer))
			throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(type, pointer));
	}

	if(IsSpecialType(type))
	{
		if(pointer)
			throw CComError(errConstructorOrDestructorReturns,noteNone);
		newMethod->SetData(kwSafe,kwVoid,0);
		newMethod->knownName = type;
	} else
		newMethod->SetData(storage,type,pointer);

	if(newMethod->ret != kwRet && (newMethod->dType!=kwInt && newMethod->dType!=kwChar || newMethod->pointer))
		throw CComError(errInvalidType, noteNone, GetTokenStr(newMethod->ret));

	if(newMethod->storage == kwDirect && ident.num != 1)
		throw CComError(errBadDirectMethodName, noteNone, GetLongNameStr(&ident));

	CComOBMethod* oldMethod = 0;
	// Bylo nutne metodu predem deklarovat?
	if(storage==kwSafe && !IsSpecialType(type))
		if((gid = mTab->Find(idName)) == cUnknown)
			if(!decl && knownName==cUnknown && global)	// - ano a nebyla -> chyba.
				throw CComError(errNotDeclared, noteNone, idName);
			else		// - ano a nebyla, ale mam prikaz k explicitni deklaraci -> deklaruju nyni.
			{
				oldMethod = new CComOBMethod(*newMethod);
				char* underName;

				if(global)
					underName = newMethod->name;
				else
				{
					underName = new char[strlen(inObj->kerName) + strlen(newMethod->name) + 20];
					sprintf(underName, "%s__M_%s", inObj->kerName+6, newMethod->name);
				}
				mTab->Add(underName, oldMethod);

				if(!global)
					SAFE_DELETE_ARRAY(underName);

				if(!names->GetNamePointer(newMethod->kerName))
				{
					CKerName* newName = new CKerName(newMethod->kerName, names);
					assert(newName);
					newName->Type = eKerNTmethod;
				}
			}
		else			// - ano a byla -> doplnim info o definici
		{
			oldMethod = (CComOBMethod*)mTab->Get(gid);
			oldMethod->objDef = inObj;
		} 
	
	CComOBName* newName;

	if(oldMethod && (oldMethod->dType != newMethod->dType || oldMethod->pointer != newMethod->pointer))
		throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(newMethod->dType, newMethod->pointer, oldMethod->dType, oldMethod->pointer));
		
	if(!IsSpecialType(type))
	{
		newName = new CComOBName(newMethod->name, newMethod->kerName, pos.version, uTab->Get(pos.version)->name, pos,kwMethodName);
		assert(newName);
		newName->objID = inObjID;
		newName->metID = newMethod->id;
		
		bool success;
		newMethod->kerNameID = ((newMethod->storage == kwSafe) ? nTab : dnTab)->FindOrAdd(newMethod->kerName, newName, &success);
		
		if(knownName == cUnknown)
		{
			if(success)
				newName = (CComOBName*)((newMethod->storage == kwSafe) ? nTab : dnTab)->Get(newMethod->kerNameID);
		
			// pridam metodu do nameSearch...
			if(!global)
				LocalizeIdent(&ident, &inObjIdent);
			nameSearch->Add(&ident, newName, newMethod->kerNameID);

			if(newMethod->storage == kwDirect)
			{
				LocalizeIdent(&ident, &inObjIdent);
				nameSearch->Add(&ident, newName, newMethod->kerNameID);
			}
		}

		if(newMethod->storage == kwSafe && !names->GetNamePointer(newMethod->kerName))
		{
			CKerName* newName = new CKerName(newMethod->kerName, names);
			assert(newName);
			newName->Type = eKerNTmethod;
		}
	}
	
	// Kontext: nyni budu zpracovavat TUTO metodu.
	inMethod = newMethod;			
	inMethodID = inMethod->id;
	SetIdent(&inMethodIdent, &ident);

	Put(synMethodDecl,false);
	Put(inMethodID,true,&(newMethod->posDefined));	// id metody poslu na vystup

	DoArgumentList();		// Zpracuju argumenty a zjistim bezpecnost.

	if(IsConstantToken(look))
	{
		CComOBConstant* c = (CComOBConstant*)cTab->Get(look);
		char* versionStr = uTab->Get(inMethod->version)->name;
		if(inMethod->knownName != cUnknown)
			sprintf(inMethod->kerKonkretniID,"_KSM_%d_%s_%s",c->value.intData,KnownNames[inMethod->GetMDKnownName()].namestr+6,versionStr);
		else
			sprintf(inMethod->kerKonkretniID,"_KSM_%d_%s_%s",c->value.intData,inMethod->name,versionStr);
		inMethod->canBeCompiled = inMethod->storage==kwSafe ? KSMain->KSMs->Member(inMethod->kerKonkretniID)!=0 : 
															  KSMain->KSDMs->Member(inMethod->kerName)!=0;
		inMethod->compiled = inMethod->canBeCompiled;
		
		if(MDtestTab->Find(inMethod->kerKonkretniID) != cUnknown)
			throw CComError(errAlreadyDefined, noteNone, "same method number!!!");

		Step();			// pryc s konstantou
	}

	// Nyni projdu vsechny stejnojmenne metody v objektu:
	gid = inObj->mTab->FindFirst(idName);
	CComOBMethod* m;
	do
	{
		assert(gid != cUnknown);
		m = (CComOBMethod*)inObj->mTab->Get(gid);

		// Pridavanou metodu ignoruju a rozlisuji lokalizovane a globalni metody.
		if(m != newMethod && m->global == newMethod->global)	
		{
			if(newMethod->returnsValue)
				if(m->returnsValue && (m->ret == kwRet && newMethod->ret == kwRet || m->dType != newMethod->dType || m->ret != newMethod->ret))
					throw CComError(errCannotDoMultipleMethodDecl, noteMethodReturnsValue, idName, &(newMethod->posDefined));
				else
				{
					m->groupType = newMethod->dType;
					m->groupPointer = newMethod->pointer;
				}
		
			if(newMethod->storage==kwDirect || m->storage==kwDirect)
				throw CComError(errCannotDoMultipleMethodDecl,noteCheckSafeCallRules,idName,&(newMethod->posDefined));
			else
				if(storage==kwDestructor)
					throw CComError(errMultipleDestructors,noteNone,idName,&(newMethod->posDefined));

		}
		gid = inObj->mTab->FindNext(idName);
	} while(gid != cUnknown);
	if(newMethod->returnsValue)
	{
		newMethod->groupType  = newMethod->dType;
		newMethod->groupPointer = newMethod->pointer;
		if(oldMethod && newMethod->global)
		{
			oldMethod->groupType  = newMethod->dType;
			oldMethod->groupPointer = newMethod->pointer;
		}
	} else
		if(oldMethod && newMethod->global)
		{
			newMethod->groupType = oldMethod->groupType;
			newMethod->groupPointer = oldMethod->groupPointer;
		}

	MDtestTab->Add(new CComOBBase(newMethod->kerKonkretniID, pos.version, pos));
	DoMethodBody();
	inMethod = 0;
}

void CComSyntax::SkipMethodFromArgumentList()
{
	// Najdu zacatek tela metody:
	while(look != lexStartBlock)		
		Step();

	Step();

	int depth = 0;
	while(look != lexEndBlock || depth)
	{
		if(look == lexStartBlock)
			depth++;
		else
			if(look == lexEndBlock)
				depth--;
		Step();
	}
	Step();
}

void CComSyntax::DoArgumentList()
{
	// look stale ukazuje na '('
	assert(inObj && inMethod);
	assert(look == opParentStart);
	MatchLexical(opParentStart);			// pryc s oteviraci zavorkou

	int ret, type, pointer, version, knownName, argNr, id;
	bool global, decl;
	bool safeCall = true;
	char* name;
	
	argNr = 0;
	while(look != opParentEnd)
	{
		if(argNr++ >= cMaxParameterNr)
			throw CComError(errTooMuchArguments, noteNone, inMethod->name);
		// zjistim jednotlive udaje o 1 argumentu
		DoOneArgument(&ret,type,pointer,&name,version,knownName,global, &decl);		
		
		if(!IsSafeType(type) || (pointer && (!IsObjectType(type) && !IsDynamicArrayToken(type) && !IsNameType(type) || pointer > 1)))
			if(inMethod->storage == kwSafe)
                throw CComError(errArgumentViolatesSafeCallRules,noteCheckSafeCallRules,name,&oldPos);
			else
				safeCall = false;

		// Nemam uz stejnojmenny argument deklarovan?
		const char* argName = name;
		if(inMethod->aTab->Find(argName) != cUnknown)		
			throw CComError(errAlreadyDeclared,noteAlreadyDeclaredArgument,argName,&oldPos);

		// U direct metody nepripoustim lokalizovane argumenty
		if(!global && inMethod->storage == kwDirect)
			throw CComError(errCannotLocalizeParamToDirect, noteNone, argName);

		// Vytvorim novy argument a pridam ho do tabulky metody:
		CComOBArgument* newArg = new CComOBArgument(argName,version,oldPos,(inMethod->storage==kwDirect)? 0 : uTab->Get(version)->name,inMethod,knownName,global);
		assert(newArg);
		newArg->SetData(ret,type,pointer);	

		id = inMethod->aTab->Add(newArg);	// pridani do tabulky metody

		if(look == opAssign)
		{
			if(inMethod->storage!=kwSafe)
				throw CComError(errDefaultArgWithinDirect,noteCheckDefaultArgs,nameStr,&oldPos);
			Put(id, false);
			DoDataETDefaultValue(newArg);
		}

		DoSafeArgumentCheck(newArg);

		if(inMethod->storage == kwSafe)
		{
			CComOBName* newName = new CComOBName(newArg->name, newArg->kerName, pos.version, uTab->Get(pos.version)->name, pos, kwParamName);
			assert(newName);
			newName->objID = inObjID;
			newName->metID = inMethodID;
			bool success;
			newArg->kerNameID = nTab->FindOrAdd(newArg->kerName, newName, &success);

			if(knownName == cUnknown)
			{
				if(success)
					newName = (CComOBName*)nTab->Get(newArg->kerNameID);

				// pridam argument do nameSearch...
				if(!global)
					LocalizeIdent(&ident, &inMethodIdent);
				nameSearch->Add(&ident, newName, newArg->kerNameID);
			}

			if(!names->GetNamePointer(newArg->kerName))
			{
				CKerName* newName = new CKerName(newArg->kerName, names);
				assert(newName);
				newName->Type = eKerNTparam;
			}
		}

		// Bylo treba argument globalne deklarovat?
		int aid;
		if(knownName==cUnknown && inMethod->storage==kwSafe && global)
			if((aid = aTab->Find(argName)) == cUnknown)
				if(!decl)	// - ano a nebyl -> chyba.
                	throw CComError(errNotDeclared, noteNone, argName);
				else		// - ano a nebyl, ale mam prikaz k explicitni deklaraci -> deklaruju nyni.
				{
					aTab->Add(new CComOBArgument(*newArg));
					if(!names->GetNamePointer(newArg->kerName))
					{
						CKerName* newName = new CKerName(newArg->kerName, names);
						assert(newName);
						newName->Type = eKerNTparam;
					}
				}
			else
			{
				CComOBArgument* oldArg = (CComOBArgument*)aTab->Get(aid);
				if(oldArg->dType == cUnknown)
				{
					oldArg->dType = newArg->dType;
					oldArg->pointer = newArg->pointer;
				}

			}

		if(knownName==cUnknown && inMethod->storage==kwSafe && inMethod->global && !global)
		{
			CComOBMethod* gMet = (CComOBMethod*) mTab->FindRec(inMethod->name);
			
			if(gMet->aTab->Find(argName) == cUnknown)		
				gMet->aTab->Add(new CComOBArgument(*newArg));
		}

		if(look == opComma)
			Step();								// pryc s oddelovaci carkou
	}
	MatchLexical(opParentEnd);					// pryc s ukoncovaci zavorkou
	inMethod->hasSafeArgs = safeCall;
}

void CComSyntax::DoOneArgument(int* ret,int& type,int& pointer,char** name, int& version, int& knownName, bool& global, bool* decl)
{
	// look ukazuje za zavorku
	type = knownName = cUnknown;

	DoType(type, 0, ret, decl);

	DoPointer(pointer, type);	

	if(look == opMember4Dot)
	{
		global = false;
		Step();				// pryc se ctyrteckou
	} else
		global = true;

	if(IsKnownNameToken(look))
	{
		knownName = look;
		if(KnownNames[look-knSecond].type != eKerNTparam)
			throw CComError(errExpectedParameterKnownName,noteNone, KnownNames[look-knSecond].namestr);
		*name = KnownNames[look-knSecond].namestr + 6;
		version = pos.version;
		Step();
	} else
		DoNameParts(name, maxNameLn, &version);
}

void CComSyntax::DoSafeArgumentCheck(CComOBArgument* newArg)
{
	int id, aid;
	CComOBArgument* arg;
	CComOBMethod* m;

	switch(newArg->ret)
	{
	case cUnknown:
		return;
	
	case kwRetAnd:
	case kwRetOr:
	case kwRetAdd:
		if(newArg->dType != kwInt && newArg->dType != kwChar || newArg->pointer)
			throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(newArg->dType, newArg->pointer, kwInt, 0, newArg->name));
	}

	id = inObj->mTab->FindFirst(inMethod->name);
	do
	{
		assert(id != cUnknown);
		m = (CComOBMethod*)inObj->mTab->Get(id);

		// Aktualni metodu ignoruju a rozlisuji lokalizovane a globalni metody.
		if(m != inMethod && m->global == inMethod->global)	
		{
			if((aid = m->aTab->Find(newArg->name)) != cUnknown)
			{
				arg = (CComOBArgument*)m->aTab->Get(aid);

				if(arg->ret == kwRet || arg->ret != newArg->ret)
					throw CComError(errMultipleRetInParam, noteNone, newArg->name);
			}
		}
		id = inObj->mTab->FindNext(inMethod->name);
	} while(id != cUnknown);
}

void CComSyntax::DoAttribute(CComOBBase* foundRec, int storage, int type, int pointer, int knownName, bool inherit, int version)
{
	CComOBConstant* c;
	int id;

	// Typ mam, jmeno taky ('foundRec') a look ukazuje uz za jmeno (na , nebo ;)
	assert(inObj && foundRec && foundRec->name);
	assert(look == lexSeparator || look == opComma || look == kwEdit || look == opBracketStart);

	if(knownName != cUnknown)
	{
		int t1, t2;
		t1 = KerVarUsesTypes[knownName - knFirst];
		t2 = inMethod->GetMDType(type, pointer);
		if(t1 != t2 && (t1 != eKTobject && t2 != eKTname))
			throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(type, pointer, cUnknown, cUnknown, foundRec->name));
	}

	// Podivam se, jestli objekt nezna neco, co se jmenuje stejne jako atribut, ktery mam deklarovat.

	CheckMethodAttributeName(foundRec->name);
	// Tady vim, ze se bud nic nenaslo a nebo to nebyl konflikt jmen -> muzu bez obav deklarovat novy atribut.
	if(version == cUnknown)
		version = pos.version;
	CComOBAttribute* newAttribute = new CComOBAttribute(foundRec->name,version,identPos,uTab,inObj,knownName);
	assert(newAttribute);
	newAttribute->SetData(storage,type,pointer);
	newAttribute->inherit = inherit ? inhInheritable : inhNotInheritable;;

	while(look == opBracketStart)					// pole jako atribut
	{
		Step();										// pryc s hranatou zavorkou
		c = MatchConstant(kwInt, noteNone);
		newAttribute->array.Add(c->value.intData);
		newAttribute->pointer++;

		MatchLexical(opBracketEnd, noteNone);
	}
	
	if(attributeGroup == cUnknown)
		throw CComError(errTooMuchMembersInAttributeGroup);

	if(attributeGroup)
	{
		if(newAttribute->pointer || newAttribute->dType!=kwInt)
			throw CComError(errInvalidMemberInAttributeGroup);
		
		inObj->gList[inObj->gList.GetCount()-1]->members.Add(newAttribute);

		if(--attributeGroup == 0)
			attributeGroup = -1;		// semafor pro ukonceni skupiny atributu - bude odchyceno v DoInObject

		if(defaultEdit->dType != cUnknown)
		{
			newAttribute->edit = defaultEdit->edit;
			newAttribute->use = defaultEdit->use;
		}
	}

	bool atrRemoved, atrModified;
	CComOBModifyRec* mdfr;
	CComOBBase* b;
	int count;

	if(atrModified = ((mdfr = (CComOBModifyRec*)toModify->FindLastRec(newAttribute->kerKonkretniID, &count)) != 0 && !inputTokens))		
	{
		RedirectInput(mdfr->tokens);
		DoInObject();
		noSeparator = true;
		modified->Add(newAttribute->kerKonkretniID, new CComOBBase());
		modifiedNr += count;
	}

	// Neni ale atribut urcen k odstraneni?
	if(atrRemoved = ((b = toRemove->FindLastRec(newAttribute->kerKonkretniID, &count)) && !noSeparator))	
	{	// - ano -> nepridam si ho do tabulky
		removed->Add(newAttribute->kerKonkretniID, new CComOBBase());
		removedNr += count;
		if(b->version == kwInherit && count == 1)
		{	
			newAttribute->inherit = inhInheritable;
			atrRemoved = false;
		}
	} 

	if(atrRemoved || atrModified)
	{
		SAFE_DELETE(newAttribute);
	} else
	{
		id = inObj->aTab->Add(newAttribute);	// pridam atribut do tabulky objektu

		Put(synAttributeDecl,false);
		Put(id,true,&(newAttribute->posDefined));	// id atributu poslu na vystup
		if(look == kwEdit)
			DoDataEditTags(newAttribute);
	}

}

void CComSyntax::CheckMethodAttributeName(const char* name)
{
	// Najde se stejnojmenny atribut v teto verzi? Ano -> chyba, kolize jmen.
	if(inObj->aTab->FindVersion(name, inObj->version) != cUnknown)
		throw CComError(errAlreadyDeclared,noteAlreadyDeclaredAttribute,name);

	// totez pro pripadne metody 
	if(inObj->mTab->Find(name) != cUnknown)
		throw CComError(errAlreadyDeclared,noteAlreadyDeclaredMethod,name);
}

void CComSyntax::DoDataEditTags(CComOBBaseData* dat)
{
	const char* testStr;
	CComOBPreprocessor* pre;
	CComOBConstant* c;
	CComShortPosition sp;
	int type;
	
	bool noLevelLoad = false;
	
	assert(look == kwEdit);
	Step();			// pryc s kwEdit

	MatchLexical(lexStartBlock, noteNone);
	while(look != lexEndBlock)
	{
		if(!IsUnknownToken(look))
			throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(look));

		testStr = GetTokenStr(look);
		Step();				// pryc s tagem

		pre = (CComOBPreprocessor*)pTab->FindRec(testStr);
		if(!pre)
			throw CComError(errUnknownEditTag, noteNone, testStr);

		switch(pre->intData)
		{
		case etLevelLoad:
			dat->use |= etbDataLevelLoad;	// 1. bit
			break;

		case etNoLevelLoad:
			dat->use |= etbDataLevelLoad;	// odnastaveni 1. bitu
			dat->use -= etbDataLevelLoad;
			noLevelLoad = true;
			break;

		case etAuto:
			sp = dat->posDefined;
			if(!IsNameType(dat->dType) || dat->pointer != 1)
				throw CComError(errExpectedName, noteNone, 0, &sp);

			dat->use |= etbDataAuto;	// 2. bit
			MatchLexical(opAssign, noteNone);
			DoNameLater(etAuto, cUnknown);
			break;

		case etEditable:
			dat->use |= etbDataEditable;
			dat->use |= etbDataLevelLoad;
			break;

		case etUserName:
			MatchLexical(opAssign, noteNone);
			c = MatchConstant(kwString,noteNone);
			dat->edit.SetUserName(c->value.strData);
			break;

		case etComment:
			MatchLexical(opAssign, noteNone);
			c = MatchConstant(kwString,noteNone);
			dat->edit.SetComment(c->value.strData);
			break;

		case etDefaultValue:
			DoDataETDefaultValue(dat);
			break;

		case etDefaultMember:
			if(!IsDynamicArrayToken(dat->dType) || dat->pointer != 1)
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(dat->dType, dat->pointer, cUnknown, cUnknown, dat->name));

			MatchLexical(opAssign, noteNone);
			c = MatchConstant(cUnknown);
			if(c->dType == dat->dType-1 || c->dType == kwInt && dat->dType == kwCharArray)
			{
				dat->edit.DefaultMember = c->value;
				dat->edit.hasDefaultMember = true;

				if(c->dType != dat->dType-1)
					dat->edit.DefaultMember.doubleData = dat->edit.DefaultMember.intData;
			} else
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(c->dType, c->pointer, kwInt, 0, dat->name));
			break;

		case etInterval:
			MatchLexical(opAssign, noteNone);
			MatchLexical(lexStartBlock, noteNone);
			dat->edit.limits = etInterval;

			type = IsDynamicArrayToken(dat->dType) ? dat->dType-1 : dat->dType;
			if(type!=kwDouble && type!=kwInt && type!=kwChar)
					throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(type, 0, cUnknown, cUnknown, dat->name));

			c = MatchConstant(type,noteNone);
			dat->edit.interval[0] = c->value;
			if(type==kwDouble && c->dType == kwInt)
                dat->edit.interval[0].doubleData = c->value.intData;

			MatchLexical(opComma, noteNone);

			c = MatchConstant(type,noteNone);
			dat->edit.interval[1] = c->value;
			if(type==kwDouble && c->dType == kwInt)
                dat->edit.interval[1].doubleData = c->value.intData;

			MatchLexical(lexEndBlock, noteNone);
			dat->edit.valueType = type;
			break;

		case etList:
			DoDataETList(dat);
			break;

		case etIs:
			DoDataETIs(dat);
			break;

		case etEditType:
			DoDataETEditType(dat);
			break;		// from etEditType...

		case etInMap:
			if(!IsObjectType(dat->dType) && dat->dType!=kwObjptrArray || dat->pointer != 1)
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(dat->dType, dat->pointer, cUnknown, cUnknown, dat->name));
			dat->use |= eKVUBobjInMap;
			break;

		case etOutMap:
			if(!IsObjectType(dat->dType) && dat->dType!=kwObjptrArray || dat->pointer != 1)
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(dat->dType, dat->pointer, cUnknown, cUnknown, dat->name));

			dat->use |= eKVUBobjOutMap;
			break;

		case etExclusive:
			dat->use |= eKVUBexclusive;
			break;

		case etIncludeNull:
			dat->use |= eKVUBincludeNull;
			break;

		case etPlanarNames:
			dat->use |= eKVUBplannarNames;
			break;

		default:
			throw CComError(errInvalidEditTag, noteNone, testStr);
		}

		// Pokud je to editTag pro editor -> nastavim automaticky editable
		switch(pre->intData)
		{
		case etLevelLoad:
		case etNoLevelLoad:
		case etAuto:
			break;

		default:
			dat->use |= etbDataEditable;
			if(!noLevelLoad)
				dat->use |= etbDataLevelLoad;
		}

		if(look == opComma)
			Step();
	}			// while look != lexEndBlock
	Step();		// pryc s lexEndBlock
}

void CComSyntax::DoDataETDefaultValue(CComOBBaseData* dat)
{
	int num = -1;
	int index;
	CComOBConstant* c;
	UComValues tmp;

	tmp.intData = 0;
	dat->edit.defaultValue.RemoveAll();

	MatchLexical(opAssign, noteNone);
	if(IsDynamicArrayToken(dat->dType))					// pocatecni polozka v poli
	{
		dat->edit.defaultValue.Add(tmp);
		num++;

		if(look != lexStartBlock)						// array = anull
		{
			c = MatchConstant(dat->dType, noteNone);	// NOTE!
			assert(c->value.intData == 0);	
			dat->edit.valueType = dat->dType;
			return;
		} else
			Step();
	}

	do
	{
		num++;
		if((IsNameType(dat->dType) || dat->dType == kwNameArray) && !IsKnownNameToken(look))
		{
			index = dat->edit.defaultValue.Add(tmp);
			DoNameLater(etDefaultValue, index);
		} else
		{
			if(IsDynamicArrayToken(dat->dType))
			{
				c = MatchConstant(cUnknown,noteNone);		

				if(c->dType == dat->dType-1 || 	// typ polozky pole
					c->dType == kwInt && dat->dType-1 == kwChar || // char->int
					c->dType == kwInt && dat->dType == kwNameArray)  // int->name
				{
					dat->edit.valueType = dat->dType-1;
					dat->edit.defaultValue.Add(c->value);
				} else
					throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(c->dType, c->pointer, cUnknown, cUnknown, dat->name));
			} else
			{
				c = MatchConstant(dat->dType,noteNone);
				dat->edit.SetDefaultValue(c->value, IsStringType(c->dType) ? kwString : c->dType);
			}
		}
	
		if(IsDynamicArrayToken(dat->dType) && look==opComma)
			Step();
		else
			break;
	} while(true);

	if(IsDynamicArrayToken(dat->dType) && num > 0)
		MatchLexical(lexEndBlock, noteNone);
}

void CComSyntax::DoDataETList(CComOBBaseData* dat)
{
	CComOBConstant* c;
	int index;
	UComValues tmp;

	tmp.intData = 0;

	MatchLexical(opAssign, noteNone);
	MatchLexical(lexStartBlock, noteNone);
	dat->edit.limits = etList;

	while(look != lexEndBlock)
	{
		if((IsNameType(dat->dType) || dat->dType == kwNameArray) && !IsKnownNameToken(look))
		{
			index = dat->edit.list.Add(tmp);
			DoNameLater(etList, index);
		} else
		{
			if(IsDynamicArrayToken(dat->dType))
			{
				c = MatchConstant(cUnknown,noteNone);		

				if(c->dType == dat->dType-1 || 	// typ polozky pole
					c->dType == kwInt && dat->dType-1 == kwChar || // char->int
					c->dType == kwInt && dat->dType == kwNameArray)  // int->name
				{
					dat->edit.valueType = dat->dType-1;
					dat->edit.defaultValue.Add(c->value);
				} else
					throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(c->dType, c->pointer, cUnknown, cUnknown, dat->name));
			} else
			{
				c = MatchConstant(dat->dType,noteNone);
				dat->edit.list.Add(c->value);
				dat->edit.valueType = dat->dType;
			}
		}

		if(look!=opComma && look!=lexEndBlock)
			throw CComError(errUnexpectedSymbol,noteForgetSeparator,GetTokenStr(look));
		else
			if(look == opComma)
				Step();	// pryc s carkou
	}
	Step();				// pryc s lexEndBlock
}

void CComSyntax::DoDataETEditType(CComOBBaseData* dat)
{
	int type, nameMask;
	bool wasOr;

	type = eKETdefault;
	nameMask = 0;
	wasOr = false;

	MatchLexical(opAssign, noteNone);
	do
	{
		switch(look)
		{
		case kwString:		type = eKETstring;		break;
		case kwVoid:		nameMask |= etnmVoid;	break;
		case kwObject:		nameMask |= etnmObject;	break;

		default:
			if(IsUnknownToken(look))
			{
				CComOBPreprocessor* pre;
				pre = (CComOBPreprocessor*)pTab->FindRec(uTab->Get(look)->name);
				if(pre && pre->type == pteEditTag)
				{
					switch(pre->intData)
					{
					case etNumber:		type = eKETnumeric;		break;
					case etLetter:		type = eKETcharacter;	break;
					case etBool:		type = eKETboolean;		break;
					case etMethod:		nameMask |= etnmMethod;	break;
					case etParam:		nameMask |= etnmParam;	break;
					case etAutomatism:	nameMask |= etnmAutomatism; break;
					case etAnyName:		nameMask |= etnmAnyName;	break;
					case etObjectshadow:nameMask |= etnmObjectShadow;	break;
					case etKey:			nameMask |= etnmKey;	break;
					case etSound:		nameMask |= etnmSound;	break;
					case etObjVoid:		nameMask |= etnmObjVoid;	break;
					case etAutoVoid:	nameMask |= etnmAutoVoid;	break;
					default:
						throw CComError(errExpectedEditorType, noteNone);
					}
					break;
				}

			}
			throw CComError(errExpectedEditorType, noteNone, GetTokenStr(look));
		}
		Step();						// pryc s typem
		if(look == opBitOr)
			if(type == eKETdefault)
			{
				Step();
				wasOr = true;
			} else
				throw CComError(errCannotUseOperator);

	} while(old == opBitOr);

	if(wasOr && type != eKETdefault)
		throw CComError(errCannotUseOperator);

	// case etEditType:			nyni mam v type prislusny typ

	if(dat->pointer && dat->dType!=kwCharArray && dat->dType!=kwIntArray && !IsNameType(dat->dType))
		throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(dat->dType, dat->pointer, cUnknown, cUnknown, dat->name));

	switch(dat->dType)
	{
	case kwChar:
		if(type!=eKETnumeric && type!=eKETcharacter && type != eKETboolean)
			throw CComError(errTypeMismatch);
		break;

	case kwInt:
		if(type!=eKETnumeric && type!=eKETboolean)
			throw CComError(errTypeMismatch);
		break;

	case kwCharArray:
		if(type!=eKETstring && type!=eKETnumeric && type!=eKETcharacter && type!=eKETboolean)
			throw CComError(errTypeMismatch);
		break;

	case kwIntArray:
		if(type!=eKETnumeric && type!=eKETcharacter && type!=eKETboolean)
			throw CComError(errTypeMismatch);
		break;

	case kwName:	case kwObjectName:	case kwParamName:	case kwMethodName:
		if(type!=eKETdefault)
			throw CComError(errTypeMismatch);
		break;

	default:
		throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(dat->dType, dat->pointer, cUnknown, cUnknown, dat->name));
	}
	dat->edit.editType = type;
	dat->edit.nameMask = nameMask;
}

void CComSyntax::DoDataETIs(CComOBBaseData* dat)
{
	CComOBConstant* c;
	int type, pointer;

	if(IsDynamicArrayToken(dat->dType))
	{
		type = dat->dType-1;
		pointer = 1;
	} else
	{
		type = dat->dType;
		pointer = IsObjectType(type) || IsNameType(type) ? 1 : 0;
	}

	if(IsStringType(type) || pointer != dat->pointer)
		throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(type, pointer, cUnknown, cUnknown, dat->name));

	switch(look)
	{
	case opGreater:
	case opLess:
		dat->use |= eKVUBexclusive;
		// no break...
	case opGreaterEqual:
	case opLessEqual:
		dat->edit.limits = etIs;
		dat->edit.isOp = look;
		Step();						// pryc s operaci

		if((IsNameType(dat->dType) || dat->dType == kwNameArray || IsObjectType(dat->dType) || dat->dType == kwObjptrArray) && !IsKnownNameToken(look))
			DoNameLater(etIs, cUnknown);
		else
			if(IsDynamicArrayToken(dat->dType))
			{
				c = MatchConstant(cUnknown,noteNone);		

				if(c->dType == dat->dType-1 || 	// typ polozky pole
					c->dType == kwInt && dat->dType-1 == kwChar || // char->int
					c->dType == kwInt && dat->dType == kwNameArray)  // int->name
				
					dat->edit.isName = c->value;
				else
					throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(c->dType, c->pointer, cUnknown, cUnknown, dat->name));
			} else
			{
				c = MatchConstant(dat->dType,noteNone);
				dat->edit.isName = c->value;
			}
		break;

	default:
		throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(look));
	}
}

void CComSyntax::DoNameLater(int type, int index)
{
	Put(kwEdit, false);
	Put(tokDataFirst + type, false);
	if(index != cUnknown)
		Put(tokDataFirst + index, false);

	Put(synStartExpression, false);
	DoPrimaryMain();
	Put(synEndExpression, false);
}

void CComSyntax::DoMethodBody()
{
	assert(inObj && inMethod);
	// look by mel byt na oteviraci slozene zavorce
	if(look != lexStartBlock)
		throw CComError(errMissingFunctionBody,noteCheckObject,inMethod->name);
	
	blockDepth = currentBlock = totalBlockNr = breakDepth = continueDepth = 0;
	switchBlock = cUnknown;
	DoCommand();
	assert(blockDepth==0 && currentBlock==0);
}

void CComSyntax::DoCommand()
{
	CComOBBase* ste;
	int oldBlock;
	int type = cUnknown;

	switch(look)
	{
	case lexStartBlock:
		oldBlock = currentBlock;
		blockDepth++;
		totalBlockNr++;
		currentBlock = totalBlockNr;
		Put(synStartBlock,false);
		Step();						// pryc se zacatkem bloku
		
		singleCommand = false;
		while(look != lexEndBlock && look!=lexEOF)
			DoCommand();
		
		if(look == lexEOF)
			throw CComError(errMissingEndBlock,noteForgetEndBlock);

		blockDepth--;
		currentBlock = oldBlock;

		Put(synEndBlock,false);
		Step();						// pryc s koncem bloku
		break;

	case lexEndBlock:
		throw CComError(errEndBlockWithoutStart);
		break;

	case lexSeparator:
		Step();						// pryc s oddelovacem
		if(singleCommand)
			Put(lexSeparator, false);
		break;

	default:
		if(IsKeywordToken(look))
			switch(look)
			{
			case kwIf:			DoIf();			break;
			case kwWhile:		DoWhile();		break;
			case kwDo:			DoDo();			break;
			case kwFor:			DoFor();		break;
			case kwSwitch:		DoSwitch();		break;
			case kwReturn:		DoReturn();		break;
			case kwBreak:		DoBreak();		break;
			case kwContinue:	DoContinue();	break;
			case kwWith:		DoWith();		break;
			
			case kwNew:			case kwDelete:			case kwSender:			case kwThis:		
			case kwAssigned:	case kwTypeOf:			case kwSizeof:
			case kwNull:		case kwNNull:			case kwONull:
			case kwANullChar:	case kwANullDouble:		case kwANullInt:		
			case kwANullObjptr:	case kwANullName:		case kwANullVoid:
				DoExpression();	
				break;
			default:
				DoLocalDef();
				MatchLexical(lexSeparator,noteForgetSeparator);
			}
		else
		{
			// Vyraz nebo definice lokalni promenne?
			// (Problem, kdyz zacinam jmenem typu struktury nebo objektu.)
			if(IsUnknownToken(look))
			{
				ste = uTab->Get(look);
				if(type == cUnknown)
					type = sTab->Find(ste->name);
			}

			if(type == cUnknown)
				DoExpression();
			else
				DoLocalDef();

			MatchLexical(lexSeparator,noteForgetSeparator);
		}
	}
}

void CComSyntax::DoIf()
{
	assert(look == kwIf);
	Put(kwIf);						// pryc s ifem
	Step();
	MatchLexical(opParentStart,noteCheckIf);
	DoExpression();
	MatchLexical(opParentEnd,noteCheckIf);
	singleCommand = true;
	DoCommand();
	while(look == lexSeparator)
		Step();
	if(look == kwElse)
	{
		Put(kwElse);
		Step();
		DoCommand();
	}
}

void CComSyntax::DoWhile()
{
	assert(look == kwWhile);
	Step();							// pryc s whilem
	Put(kwWhile);
	MatchLexical(opParentStart,noteCheckWhile);
	DoExpression();
	MatchLexical(opParentEnd,noteCheckWhile);

	breakDepth++;	continueDepth++;
	singleCommand = true;
	DoCommand();
	breakDepth--;	continueDepth--;
}

void CComSyntax::DoDo()
{
	assert(look == kwDo);
	Step();							// pryc s do
	Put(kwDo);
	
	breakDepth++;	continueDepth++;
	singleCommand = true;
	DoCommand();
	breakDepth--;	continueDepth--;

	MatchKeyword(kwWhile,noteCheckDo);
	Put(kwWhile);
	MatchLexical(opParentStart,noteCheckDo);
	DoExpression();
	MatchLexical(opParentEnd,noteCheckDo);
}

void CComSyntax::DoFor()
{
	assert(look == kwFor);
	Step();							// pryc s forkem
	Put(kwFor);
	MatchLexical(opParentStart,noteCheckFor);
	DoExpression();
	MatchLexical(lexSeparator,noteCheckFor);
	DoExpression();
	MatchLexical(lexSeparator,noteCheckFor);
	DoExpression();
	MatchLexical(opParentEnd,noteCheckFor);

	breakDepth++;	continueDepth++;
	singleCommand = true;
	DoCommand();
	breakDepth--;	continueDepth--;
}

void CComSyntax::DoSwitch()
{
	CComOBConstant* c;
	CListK<int> caseList;
	int id;

	assert(look == kwSwitch);
	Step();							// pryc se switchem
	Put(kwSwitch, false);
	
	MatchLexical(opParentStart);
	DoExpression();
	MatchLexical(opParentEnd);
	
	MatchLexical(lexStartBlock,noteNone);

	breakDepth++;
	switchBlock = blockDepth;
	while(look != lexEndBlock && look != kwDefault)
	{
		switch(look)
		{
		case kwCase:
			Step();					// pryc s case
			Put(kwCase, false);
			c = MatchConstant(kwInt, noteNone, &id);
			if(!caseList.AddUnique(c->value.intData))
			{
				char buf[128];
				_itoa(c->value.intData, buf, 10);
				throw CComError(errSameCase, noteNone, buf);
			}
			Put(synStartExpression, false);
			Put(id, false);
			Put(synEndExpression, false);

			MatchLexical(opDoubleDot, noteNone);

			while(look != kwCase && look != lexEndBlock && look != kwDefault)
				DoCommand();

			Put(synEndCase, false);
			break;

		default:
			throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(look));
		}
	}

	if(look == kwDefault)
	{
		Put(kwDefault, false);
		Step();					// pryc s defaultem
		MatchLexical(opDoubleDot, noteNone);
		while(look != lexEndBlock)
			DoCommand();
		Put(synEndCase, false);
	}

	Step();			// pryc s koncem switch-bloku
	switchBlock = cUnknown;
	breakDepth--;
}

void CComSyntax::DoReturn()
{
	assert(look == kwReturn);
	Step();							// pryc s returnem
	Put(kwReturn);
	if(look != lexSeparator)
		DoExpression();
	else
		Put(kwVoid, false);
	MatchLexical(lexSeparator,noteForgetSeparator);
}

void CComSyntax::DoBreak()
{
	assert(look == kwBreak);
	if(!breakDepth)
		throw CComError(errIllegalBreak, noteNone);

	Step();							// pryc s breakem

	Put(kwBreak);
	
	MatchLexical(lexSeparator,noteForgetSeparator);
}

void CComSyntax::DoContinue()
{
	assert(look == kwContinue);
	if(!continueDepth)
		throw CComError(errIllegalContinue, noteNone);

	Step();							// pryc s continue

	Put(kwContinue);
	MatchLexical(lexSeparator,noteForgetSeparator);
}

void CComSyntax::DoWith()
{
	assert(look == kwWith);
	Step();							// pryc s with
	
	Put(kwWith);

	int defaultVersion;

	MatchLexical(opParentStart, noteNone);
	DoReadVersion(defaultVersion);
	MatchLexical(opParentEnd, noteNone);
	Put(defaultVersion, false);

    DoCommand();
}

void CComSyntax::DoExpression(bool putStartExp)
{
	if(putStartExp)
		Put(synStartExpression,false);
	DoAssign();
	while(look == opComma)
	{
		CComShortPosition opPos = pos;
		Step();
		DoAssign();
		Put(opComma,true,&opPos);
	} 
	if(putStartExp)
		Put(synEndExpression,false);
}

void CComSyntax::DoAssign()
{
	int refQueuePos = PutReference();
	DoConditional();

	if(IsAssign(look))
	{
		SetLastOpState(synAssigning);
		int old = look;
		CComShortPosition opPos = pos;
		Step();
		// Idea: slozite operatory prirazeni nahradim obycejnym = a prislusnym operatorem 
		int vop;
		switch(old)
		{
		case opAssign:				vop = cUnknown;			break;
		case opAssignPlus:			vop = opPlus;			break;
		case opAssignMinus:			vop = opMinus;			break;
		case opAssignMultiply:		vop = opMultiply;		break;
		case opAssignDivide:		vop = opDivide;			break;
		case opAssignModulo:		vop = opModulo;			break;
		case opAssignLeftShift:		vop = opLeftShift;		break;
		case opAssignRightShift:	vop = opRightShift;		break;
		case opAssignAnd:			vop = opBitAnd;			break;
		case opAssignOr:			vop = opBitOr;			break;
		case opAssignXor:			vop = opBitXor;			break;
		default:
			assert(false);
		}
		
		if(vop != cUnknown)
			CopyQueueFromPos(refQueuePos);			// zopakuji lvalue 
		
		DoAssign();								// provedu cast za =
		
		if(vop != cUnknown)
			Put(vop,true,&opPos);				// pridam nahradni operator

		Put(opAssign,true,&opPos);				// a kazdopadne obycejne =
	} 
}

void CComSyntax::DoConditional()
{
	DoLogicOr();
	if(look == opConditional)
	{
		Step();
		CComShortPosition opPos = pos;
		Put(synConditional1,false);
		DoExpression(false);
		MatchLexical(opDoubleDot,noteCheckConditional);
		Put(synConditional2,false);
		DoExpression(false);
		Put(opConditional,true,&opPos);
	}
}


void CComSyntax::DoLogicOr()
{
	
	DoLogicAnd();
	if(look == opLogicOr)
	{
		int op = look;
		CComShortPosition opPos = pos;
		Step();
		Put(synLogicOr,false);
		DoLogicOr();
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoConstExpression()
{
	DoConstLogicAnd();
	if(look == opLogicOr)
	{
		CComShortPosition opPos = pos;
		Step();
		DoConstExpression();
		DoOp(opLogicOr, opPos);
	}
}

void CComSyntax::DoLogicAnd()
{
	DoBitOr();
	if(look == opLogicAnd)
	{
		CComShortPosition opPos = pos;
		Step();
		Put(synLogicAnd,false);
		DoLogicAnd();
		Put(opLogicAnd,true,&opPos);
	}
}

void CComSyntax::DoConstLogicAnd()
{
	DoConstBitOr();
	if(look == opLogicAnd)
	{
		CComShortPosition opPos = pos;
		Step();
		DoConstLogicAnd();
		DoOp(opLogicAnd, opPos);
	}
}

void CComSyntax::DoBitOr()
{
	DoBitXor();
	if(look == opBitOr)
	{
		CComShortPosition opPos = pos;
		Step();
		DoBitOr();
		Put(opBitOr,true,&opPos);
	}
}

void CComSyntax::DoConstBitOr()
{
	DoConstBitXor();
	if(look == opBitOr)
	{
		CComShortPosition opPos = pos;
		Step();
		DoConstBitOr();
		DoOp(opBitOr, opPos);
	}
}

void CComSyntax::DoBitXor()
{
	DoBitAnd();
	if(look == opBitXor)
	{
		CComShortPosition opPos = pos;
		Step();
		DoBitXor();
		Put(opBitXor,true,&opPos);
	}
}

void CComSyntax::DoConstBitXor()
{
	DoConstBitAnd();
	if(look == opBitXor)
	{
		CComShortPosition opPos = pos;
		Step();
		DoConstBitXor();
		DoOp(opBitXor, opPos);
	}
}

void CComSyntax::DoBitAnd()
{
	DoEqual();
	if(look == opBitAnd)
	{
		CComShortPosition opPos = pos;
		Step();
		DoBitAnd();
		Put(opBitAnd,true,&opPos);
	}
}

void CComSyntax::DoConstBitAnd()
{
	DoConstEqual();
	if(look == opBitAnd)
	{
		CComShortPosition opPos = pos;
		Step();
		DoConstBitAnd();
		DoOp(opBitAnd, opPos);
	}
}

void CComSyntax::DoEqual()
{
	DoRelation();
	if(IsEqual(look))
	{
		int op = look;
		CComShortPosition opPos = pos;
		Step();
		DoEqual();
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoConstEqual()
{
	DoConstRelation();
	if(IsEqual(look))
	{
		enmOperators op = (enmOperators)look;
		CComShortPosition opPos = pos;
		Step();
		DoConstEqual();
		DoOp(op, opPos);
	}
}

void CComSyntax::DoRelation()
{
	DoShift();
	if(IsRelation(look))
	{
		int op = look;
		CComShortPosition opPos = pos;
		Step();
		DoRelation();
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoConstRelation()
{
	DoConstShift();
	if(IsRelation(look))
	{
		enmOperators op = (enmOperators)look;
		CComShortPosition opPos = pos;
		Step();
		DoConstRelation();
		DoOp(op, pos);
	}
}

void CComSyntax::DoShift()
{
	DoAdd();
	if(IsShift(look))
	{
		int op = look;
		CComShortPosition opPos = pos;
		Step();
		DoShift();
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoConstShift()
{
	DoConstAdd();
	if(IsShift(look))
	{
		enmOperators op = (enmOperators)look;
		CComShortPosition opPos = pos;
		Step();
		DoConstShift();
		DoOp(op, opPos);
	}
}

void CComSyntax::DoAdd()
{
	DoSub();
	if(look == opPlus)
	{
		CComShortPosition opPos = pos;
		Step();
		DoAdd();
		Put(opPlus,true,&opPos);
	}
}

void CComSyntax::DoSub()
{
	DoMultiply();
	if(look == opMinus)
	{
		CComShortPosition opPos = pos;
		Step();
		DoSub();
		Put(opMinus,true,&opPos);
	}
}

void CComSyntax::DoConstAdd()
{
	DoConstMultiply();
	if(IsAdd(look))
	{
		enmOperators op = (enmOperators)look;
		CComShortPosition opPos = pos;
		Step();
		DoConstAdd();
		DoOp(op, opPos);
	}
}

void CComSyntax::DoMultiply()
{
	DoDivide();
	if(look == opMultiply)
	{
		CComShortPosition opPos = pos;
		Step();
		DoMultiply();
		Put(opMultiply,true,&opPos);
	}
}

void CComSyntax::DoDivide()
{
	DoUnary();
	if(look == opDivide || look == opModulo)
	{
		int op = look;
		CComShortPosition opPos = pos;
		Step();
		DoDivide();
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoConstMultiply()
{
	DoConstUnary();
	if(IsMultiply(look))
	{
		enmOperators op = (enmOperators)look;
		CComShortPosition opPos = pos;
		Step();
		DoConstMultiply();
		DoOp(op, opPos);
	}
}

void CComSyntax::DoUnary()
{
	if(IsKeywordToken(look))
		switch(look)
		{
		case kwNew:
			DoAlloc();
			return;
		case kwDelete:
			DoDealloc();	
			return;
		case kwSizeof:
			DoSizeof();	
			return;
		case kwTypeOf:
			DoTypeOf();
			return;
		}

	DoPrefixPostfix();
}

void CComSyntax::DoConstUnary()
{
	int op = cUnknown;
	CComPosition opPos = pos;

	if(IsUnaryPrefix(look))
		DoConstPrefix(op);

	DoConstPrimary();

	if(op != cUnknown)
		DoOp((enmOperators)op, opPos);
}

void CComSyntax::DoPrefixPostfix()
{
	bool hasPrefix;
	int  op;
	CComShortPosition opPos;
	if(hasPrefix = IsUnaryPrefix(look))
	{
		opPos = pos;
		DoPrefix(op);
	}
	
	if(IsUnaryPrefix(look))
		DoPrefixPostfix();
	else
	{
		if(look == kwAssigned)
			DoAssigned();	
		else
			DoPostfix();
	}
	
	if(hasPrefix)
	{
		if(op == opAddress)
			SetLastOpState(synAddressing);
		Put(op,true,&opPos);
	}
}

void CComSyntax::DoPrefix(int& op)
{
	assert(IsUnaryPrefix(look));
	op = look;
	Step();						// pryc s prefixem

	switch(op)
	{
	case opPlus:
		op = opUnaryPlus;
		break;

	case opMinus:
		op = opUnaryMinus;
		break;

	case opMultiply:
		op = opIndirection;
		break;

	case opBitAnd:
		op = opAddress;
		break;

	case opPreIncrement:
	case opPreDecrement:
	case opBitNot:
	case opLogicNot:
		break;

	default:
		throw CComError(errFeatureNotSupported,noteNone,"unknown prefix");
	}
}

void CComSyntax::DoConstPrefix(int& op)
{
	assert(IsUnaryPrefix(look));
	Step();						// pryc s prefixem

	switch(old)
	{
	case opPlus:
		op = opUnaryPlus;
		break;

	case opMinus:
		op = opUnaryMinus;
		break;

	case opBitNot:
	case opLogicNot:
		op = old;
		break;

	default:
		throw CComError(errInvalidOperator, noteNone, GetTokenStr(look));
	}
}

void CComSyntax::DoSizeof()
{
	throw CComError(errFeatureNotSupported,noteNone,"operator sizeof");			
}

void CComSyntax::DoAssigned()
{
	assert(look == kwAssigned);
	Put(look);
	Step();
	MatchLexical(opParentStart, noteNone);
	Put(look);
	Step();
	MatchLexical(opParentEnd, noteNone);
}

void CComSyntax::DoTypeOf()
{
	assert(look == kwTypeOf);
	Put(look);
	Step();
	MatchLexical(opParentStart, noteNone);
	DoExpression();
	MatchLexical(opParentEnd, noteNone);
}


void CComSyntax::DoAlloc()
{
	int type, pointer;

	assert(look == kwNew);

	Put(kwNew);
	Step();									// pryc s new

	if(look != kwVarType)
	{
		DoType(type);							// nactu typ
		DoPointer(pointer, type);				// pointer

		Put(type, false);
		Put(tokDataFirst + pointer, false);
	} else
	{
		Put(kwVarType);
		Step();								// pryc s name
		Put(synStartExpression, false);
		DoPostfix();
		Put(synEndExpression, false);
	}

	if(look == opBracketStart)
	{
		Step();								// pryc s hranatou zavorkou
		Put(opBracketStart, false);
		DoExpression();
		MatchLexical(opBracketEnd, noteNone);
	}
}

void CComSyntax::DoDealloc()
{
	assert(look == kwDelete);

	Step();						// pryc s delete	

	DoExpression(false);
	Put(kwDelete);
}

bool CComSyntax::DoPostfix(int chainedMemberAccess, CComShortPosition* chainedPos)
{
	int implicitThis, op;
	CComShortPosition opPos;
	bool wasFunctionCall;

	if(old == opMemberArrow)							// bud jsem za sipkou
		implicitThis = cUnknown;
	else
		implicitThis = PutReference(kwThis, false);		// nebo vkladam implicitni this->

	DoPrimary();
	
	opPos = pos;
	op = look;

	wasFunctionCall = (op == opParentStart);
	if(!wasFunctionCall && implicitThis!=cUnknown)
		implicitThis = RemoveReference(implicitThis, kwThis);

	if(chainedMemberAccess != cUnknown && !wasFunctionCall)
		Put(chainedMemberAccess, true, chainedPos);

	switch(op)
	{
	case opPreIncrement:
		Step();				// pryc s operatorem
		op = opPostIncrement;
		break;

	case opPreDecrement:
		Step();				// pryc s operatorem
		op = opPostDecrement;
		break;

	case opBracketStart:
		while(look == opBracketStart)
		{
			Step();						// pryc s operatorem
			DoExpression(false);		// vyraz v indexu
			MatchLexical(opBracketEnd, noteNone);
			Put(op,true,&opPos);
			opPos = pos;
		}

		switch(look)
		{
		case opMemberArrow:
		case opMemberDot:
			Step();
			if(look == opParentStart)
				throw CComError(errExpectedIdentifier, noteNone, GetTokenStr(look));
			return DoPostfix(op, &opPos);

		case opPreIncrement:
			op = opPostIncrement;
			opPos = pos;
			Step();
			break;

		case opPreDecrement:
			op = opPostDecrement;
			opPos = pos;
			Step();
			break;
		
		default:
			return false;
		}
		break;
	
	case opMemberDot:
	case opMemberArrow:
		Step();				// pryc s . nebo ->
		// Identifikator za operatorem bylo volani funkce? -> sipku dal neposilam 
		return DoPostfix(op, &opPos);

	case opParentStart:
		DoMethodCall(&opPos);	
		if(look == opMemberArrow || look == opMemberDot)
		{
			Step();
			return DoPostfix(op, &opPos);
		} else
			return true;				

	default:
		return false;
	}

	Put(op,true,&opPos);
	return false;
}

void CComSyntax::DoConstPrimary()
{
	bool isNull = false;
	int id;
	const char* idName;

	if(look == opParentStart)				// zavorka? -> znovu cele DoConstExpression()
	{
		Step();								// pryc se zavorkou
		DoConstExpression();
		MatchLexical(opParentEnd,noteNone);
	} else
	{
		if(IsConstantToken(look))
		{
			constConstant = (CComOBConstant*) cTab->Get(look);
			constID = look;
			if(constConstant->dType == kwInt && constType == kwDouble)
			{
				char buf[32];
				sprintf(buf, "%d.0", constConstant->value.intData);

				constConstant = new CComOBConstant(buf, kwDouble);
				assert(constConstant);
				constID = cTab->FindOrAdd(constConstant->name, constConstant);
			} 
		} else
			if(IsKernelConstant(look))
				constConstant = cTab->FindOrAddConstant(cKerConstants[look - kcFirst].value, &constID);
			else
				if(IsKnownNameToken(look))
				{
					idName = KnownNames[look - knSecond].namestr;


					DefineKnownName(KnownNames[look-knSecond].type, idName);
					id = nTab->Find(idName);
					assert(id != cUnknown);
					id  -= tokNameFirst;

					cTab->FindOrAddConstant(id, &constID);		
				} else
					if(IsNullToken(look))
					{
						constID = cTab->FindNull(look);
						constConstant = (CComOBConstant*)cTab->Get(constID);
						isNull = true;
					} else
						throw CComError(errExpectedConstant,noteNone);
				
		constConstant = (CComOBConstant*)cTab->Get(constID);
		assert(constConstant);
		Step();		// pryc s konstantou

		if(constType == cUnknown || constType == kwInt && reqType == cUnknown && constConstant->dType == kwDouble)
			constType = constConstant->dType;

		if(constType != constConstant->dType)
			if(!IsStringType(constType) || !IsStringType(constConstant->dType))	// string jako string...
				if(!IsNameType(constType) || constConstant->dType != kwInt)	// index jmena ziskam jako int
					throw CComError(errTypeMismatch,noteNone, GetCollisionTypesStr(constConstant->dType, constConstant->pointer));
			

		switch(constType)
		{
		case kwChar:		case kwInt:			
			cs.Push(CComExpression(constConstant->value.intData));
			break;

		case kwName:		case kwMethodName:		case kwObjectName:		case kwParamName:
			if(isNull)
			{
				constConstant = cTab->FindOrAddConstant(cUnknown, &constID);
				cs.Push(CComExpression(cUnknown));
			} else
				cs.Push(CComExpression(constConstant->value.intData));
			break;

		case kwDouble:
			cs.Push(CComExpression(constConstant->value.doubleData));
			break;

		default:
			if(IsStringType(constType))
				cs.Push(CComExpression(constConstant->value.strData));
			else
				if(IsObjectType(constType) || IsDynamicArrayToken(constType))
				{
					assert(isNull);				
					cs.Push(CComExpression(0));
				} else
					assert(false);
		}
	}
}

void CComSyntax::DefineKnownName(int type, const char* idName)
{
	switch(type)
	{
	case eKerNTvoid:	type = kwName;			break;
	case eKerNTobject:	type = kwObjectName;	break;
	case eKerNTmethod:	type = kwMethodName;	break;
	case eKerNTparam:	type = kwParamName;	break;
	default:
		assert(false);
	}

	CComOBName* n = new CComOBName(idName, idName, pos.version, uTab->Get(pos.version)->name, pos, type);
	assert(n);
	nTab->FindOrAdd(idName, n);
}

void CComSyntax::DoPrimary()
{
	if(look == opParentStart)				// zavorka? -> znovu cele DoExpression()
	{
		Step();								// pryc se zavorkou
		DoExpression();
		MatchLexical(opParentEnd,noteNone);
	} else
		if(look==kwSender || look==kwThis || IsConstantToken(look))
		{
			Put(look);
			Step();
		} else
			if(IsUnknownToken(look) || IsKnownNameAtrToken(look) || IsKernelService(look) || IsKernelConstant(look) || look == opMember4Dot)
				DoPrimaryMain();
			else
				if(IsKnownNameToken(look))
				{
					DefineKnownName(KnownNames[look-knSecond].type, KnownNames[look-knSecond].namestr);
					DoPrimaryMain();
				} else
					if(IsNullToken(look))
					{	
						Put(cTab->FindNull(look));
						Step();
					} else
						throw CComError(errExpectedIdentifier,noteNone);
}

void CComSyntax::DoPrimaryMain()
{
	bool was4Dot;
	int i;

	if(was4Dot = (look == opMember4Dot))
	{
		CListKElem<CComNSENode>* p;
		
		p = inObjIdent.el;
		assert(p);

		if(doingParameters)
			refObjID.RemoveAll();

		while(p)
		{
			if(doingParameters)
			{
				i = PutReference(p->data.id, false);
				refObjID.Add(i);
				i = PutReference(opMember4Dot, false);
				refObjID.Add(i);
			} else
			{
				Put(p->data.id, false);
				Put(opMember4Dot, false);
			}
			p = p->next;
		}
		Step();

		if(!IsUnknownToken(look))
			throw CComError(errExpectedIdentifier, noteNone, GetTokenStr(look));
	} 

	DoIdentifier();

	while(look == opMember4Dot)
	{
		Put(opMember4Dot);
		Step();
		DoIdentifier();
	}
}

void CComSyntax::DoIdentifier()
{
	Put(look);
	Step();

	if(look == lexEditorStringSeparator)
	{
		int id;
		Step();				// pryc s @
		Put(lexEditorStringSeparator, false);
		MatchConstant(kwString, noteNone, &id);
		Put(id);
	}	
}



void CComSyntax::DoMethodCall(CComShortPosition* opPos)
{
	assert(look == opParentStart);
	Step();								// pryc se zavorkou

	Put(synArgsEnd, false);
	while(look != opParentEnd)
	{
		DoNameArg();
		Put(synNextParameter, false);
		if(look != opParentEnd)
			MatchLexical(opComma,noteCantContinueParamList);
	}
	Step();								// pryc s koncovou zavorkou

	Put(opParentStart, true, opPos);

	switch(look)
	{
	case kwMessage:
	case kwEnd:
	case kwNextTurn:
	case kwNextEnd:
	case kwCallend:
	case kwTimed:
		Put(synTimeDecl,false);
		Put(look);
		Step();							// pryc s casem
	}

	if(old == kwCallend)
		if(IsUnknownToken(look) || look == kwThis || look == kwSender || look == kwNew)
			DoExpression();
		else
			throw CComError(errUnexpectedSymbol, noteForgetSeparator, GetTokenStr(look));

	if(old == kwTimed)
		DoExpression();

}

void CComSyntax::DoNameArg()
{
	bool didParameters = doingParameters;
		
	// DoExpression zde nelze pouzit, protoze by mi pozralo carky a vsechny parametry.
	CComShortPosition opPos;
	int retType = kwFirst;	// cUnknown == -1 nemuzu poslat jako data
	if(look==kwRet || look==kwRetAnd || look==kwRetOr || look==kwRetAdd)
	{
		retType = look;
		Step();				// pryc s ret
	}
	doingParameters = true;
	
	int refSafeArg = PutReference(synSafeArgument, false);
	bool was4Dot = look == opMember4Dot;
	DoAssign();						
	if(look == opDoubleDot)
	{
		if(doingParameters && was4Dot)	// predchazela :: a neni to volani lokalizovane funkce
		{
			for(int i=0; i<refObjID.GetCount(); i++)
				RemoveReference(refObjID[i]);
			Put(synLocalizedArgument, false);
		}

		RemoveReference(refSafeArg, synSafeArgument);
		Put(synSafeArgument, false);
		opPos = pos;
		Step();							// pryc s dvojteckou
		doingParameters = false;
		DoAssign();						
		Put(opDoubleDot, true, &opPos);
		Put(tokDataFirst + retType, false);
	} else
		if(retType != kwFirst)
			throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(retType));

	doingParameters = didParameters;
}

void CComSyntax::DoLocalDef()
{
	int type;

	if(switchBlock == currentBlock)
		throw CComError(errCannotDeclareLocalInSwitch, noteNone);

	DoType(type);

	DoOneLocal(type);
	while(look == opComma)
	{
		Step();						// pryc s carkou
		DoOneLocal(type);
	}
}

void CComSyntax::DoOneLocal(int type)
{
	int pointer, id, fid;
	CComOBConstant* c;
	CComOBBase* ste;
	CComOBLocal* loc;

	DoPointer(pointer, type);			// pointer


	if(!IsUnknownToken(fid = look))
		throw CComError(errExpectedIdentifier,noteCheckLocalDefRules,GetTokenStr(look));

	ste = uTab->Get(look);		// najdu si definovane jmeno

	// Nebyla uz ve stejnem bloku definovana stejnojmena lok. promenna?

    id = inMethod->lTab->FindFirst(ste->name);
	while(id != cUnknown)
	{
		loc = (CComOBLocal*)inMethod->lTab->Get(id);
		if(loc->blockNr == currentBlock)		// ve stejnem bloku -> chyba
			throw CComError(errAlreadyDefined,noteAlreadyDefinedLocal,loc->name);
	    id = inMethod->lTab->FindNext(ste->name);
	}

	// Vytvorim novy zaznam v tabulce metody
	CComOBLocal* newLocal = new CComOBLocal(ste->name,pos.version,pos);
	assert(newLocal);
	newLocal->SetData(type,pointer,blockDepth,currentBlock);
	id = inMethod->lTab->Add(newLocal);

	Put(synLocalDefinition);
	Put(id,false);
	Step();									// pryc se jmenem lokalni promenne

	while(look == opBracketStart)
	{
		Step();				// pryc s hranatou zavorkou
		c = MatchConstant(kwInt, noteNone);
		newLocal->array.Add(c->value.intData);
		newLocal->pointer++;

		MatchLexical(opBracketEnd, noteNone);
	}

	if(look == opAssign)
	{
		Step();				// pryc s rovnitkem
		Put(synStartExpression, false);
		Put(fid, false);
		DoAssign();
		Put(opAssign, false);
		Put(synEndExpression, false);
	}
}

void CComSyntax::DoNameDef()
{
	int type = look;
	Step();							// pryc se jmenem typu
	
	DoOneName(type);
	while(look == opComma)
	{
		Step();						// pryc s carkou
		DoOneName(type);
	}
}

void CComSyntax::DoOneName(int type)
{
	CComOBBase* b;
	int objID, kerNameID, version;
	bool success;

	eKerNameType MDtype;

	char* nameStr;
	const char* versionStr;
	CComOBName* n;
	CKerName* newName;
	
	DoNameParts(&nameStr, maxNameLn, &version);
	versionStr = uTab->Get(version)->name;	

	switch(type)
	{
	case kwVoidName:
		n = new CComOBName(nameStr, 0, version, versionStr, identPos, kwName);
		CheckGlobalName(n->kerName, 0);

		kerNameID = nTab->FindOrAdd(n->kerName, n, &success);
		if(success)
			n = (CComOBName*)nTab->Get(kerNameID);

		newName = new CKerName(n->kerName, names);
		assert(newName);
		newName->Type = eKerNTvoid;
		nameSearch->Add(&ident, n, kerNameID);
		return;


	case kwMethodName:
		{
			CComOBMethod* m;
			int type, pointer, ret;

			if(look == kwReturns)
			{
				Step();
				DoType(type, 0, &ret);
				DoPointer(pointer, type);
				if(ret == cUnknown)
					ret = kwRet;
			} else
			{
				type = kwVoid;
				pointer = 0;
				ret = kwRet;
			}

			if(ret != kwRet && (type!=kwInt && type!=kwChar || pointer))
				throw CComError(errInvalidType, noteNone, GetTokenStr(ret));

			mTab->Add(b = m = new CComOBMethod(nameStr, pos.version, identPos, versionStr, 0, kwSafe, cUnknown, true));
			m->SetData(kwSafe, type, pointer);
			m->groupType = type;
			m->groupPointer = pointer;
			m->ret = ret;
			MDtype = eKerNTmethod;
		}
		break;

	case kwParamName:
		aTab->Add(b = new CComOBArgument(nameStr, pos.version, identPos, versionStr, 0, cUnknown, true));
		MDtype = eKerNTparam;
		break;

	case kwObjectName:
		objID = oTab->Add(b = new CComOBObject(nameStr, pos.version, identPos, versionStr));
		MDtype = eKerNTobject;
		break;

	default:
		assert(false);
	}

	assert(b->kerName);
	n = new CComOBName(b->name, b->kerName, pos.version, versionStr, identPos, type);
	CheckGlobalName(n->kerName, 0);

	kerNameID = nTab->FindOrAdd(n->kerName, n, &success);
	if(success)
		n = (CComOBName*)nTab->Get(kerNameID);

	newName = new CKerName(b->kerName, names);
	assert(newName);
	newName->Type = MDtype;

	if(type == kwObjectName)
	{
		newName->objBrowserLink = (CComOBObject*)b;
		CComOBObject* o = (CComOBObject*)oTab->Get(objID);
		o->kerNameID = kerNameID;
	} else
		if(type != kwVoidName)
			((CComOBBaseData*)b)->kerNameID = kerNameID;

	nameSearch->Add(&ident, n, kerNameID);
}


/* DoNameParts	
 *	- parsuje jmeno, jehoz casti jsou oddelene ::
 *	- vraci _KSID string noveho jmena, verzi a plni seznam ident id jednotlivych casti jmena
*/

void CComSyntax::DoNameParts(char** name, int& maxNameLn, int* version)
{
	CComOBBase* ste;
	CListK<CComNSEEntry>* nList;
	int curNameLn = 0;
	bool nextPart;
	int i, localVersion;
	CComShortPosition versionPos;

	ident.DeleteAll();

	i = 0;
	do
	{
		ident.AddTail(CComNSENode(look, findAny));
		MatchIdentifier(&ste, noteCheckNameDef);		// pryc se jmenem
		
		if(name)
			i += strlen(ste->name) + 20 + 5 + 1;		// 5 = misto na __M_, 20 = delka verze

		versionPos = pos;
		if(look == lexEditorStringSeparator)
		{
			Step();			// pryc s @
			DoReadVersion(localVersion);
		} else
			localVersion = cUnknown;

		if(nextPart = (look == opMember4Dot))
			Step();						// pryc s ::
		
		if((nList = nameSearch->FindRec(&ident)) == 0 || nList->num == 0)// || !nextPart)
			ident.ellast->data.version = pos.version;
		else
			if(nList->num == 1)
				ident.ellast->data.version = nList->el->data.name->version;
			else
				ident.ellast->data.version = ResolveVersionAmbiguity(nList, localVersion, &versionPos);
		
	} while(nextPart);

	if(!name)
		return;

	if(i > maxNameLn)	
	{
		SAFE_DELETE_ARRAY(nameStr);
		maxNameLn = max(i, 2*maxNameLn);
		nameStr = new char[maxNameLn];
		curNameLn = i;
	}

	assert(nameStr);
	nameStr[0] = '\0';

	CListKElem<CComNSENode>* p = ident.el;
	CListKElem<CComNSENode>* q = p;
	while(p)
	{
		strcat(nameStr, IsUnknownToken(p->data.id) ? uTab->Get(p->data.id)->name : KnownNames[p->data.id-knSecond].namestr+6);
		if(p->next)
		{
			strcat(nameStr, "_");
			strcat(nameStr, uTab->Get(p->data.version)->name);
		}

		if(!p->next)
			*version = p->data.version;

		p = p->next;

		if(p)
			strcat(nameStr, "__M_");
	}

	*name = nameStr;
}

void CComSyntax::DoReadVersion(int& version)
{
	CComShortPosition versionPos = pos;
	CComOBConstant* c = MatchConstant(kwString);

	version = uTab->Find(c->name);
	if(version == cUnknown)
		if(!_strcmpi(c->name, cStrThisVersion))
			version = pos.version;
		else
			throw CComError(errUnknownVersion, noteNone, c->name, &versionPos);
}

int CComSyntax::ResolveVersionAmbiguity(CListK<CComNSEEntry>* nList, int localVersion, CComShortPosition* versionPos)
{
	assert(nList && nList->el);

	if(localVersion == cUnknown)
		throw CComError(errUnknownVersion, noteNone, 0, versionPos);

	CListKElem<CComNSEEntry>* p = nList->el;
	while(p)
	{
		if(p->data.name->version == localVersion)
			return localVersion;

		p = p->next;
	}

	throw CComError(errWrongVersion, noteNone, uTab->Get(localVersion)->name, versionPos);

	return cUnknown;
}

void CComSyntax::DoNameCompleteParse(CComOBBase** name, int* nameID)
{
	CListK<CComNSEEntry>* nList;
	CListKElem<CComNSENode>* p;
	CListK<CComNSENode> ident2;
	CComOBName *n;
	int id, version;

	assert(name && nameID);
	*name = 0;

	DoNameParts(0, maxNameLn, &version);

	SetIdent(&ident2, &ident);
	p = ident2.el;
	while(p)
	{
		p->data.version = findAny;
		p = p->next;
	}

	nList = nameSearch->FindRec(&ident2);
	n = 0;

	if(nList && nList->num)
	{
		n = nList->el->data.name;
		id = nList->el->data.nameID;
	}

	if(!n)
		if(!nList || !nList->num)
		{
			if(ident.num == 1)
				if(IsUnknownToken(ident.el->data.id))	// Destruktor a spol.
				{
					n = (CComOBName*)GetDefinedName(uTab->Get(ident.el->data.id)->name, id);
					nameSearch->Add(&ident, n, id);						// Priste uz to najdu rychle.
				} 
			
			if(!n)
				if(!IsUnknownToken(ident.el->data.id))
					throw CComError(errUnknownIdentifier, noteNone, KnownNames[ident.el->data.id-knSecond].namestr);
				else
					throw CComError(errUnknownIdentifier, noteNone, GetLongNameStr(&ident));
		} else
			assert(false);

	*name = n;
	*nameID = id;
}

void CComSyntax::CheckGlobalName(const char* name, const char* nameToShow)
{
	if(nTab->Find(name) != cUnknown)
		throw CComError(errAlreadyDeclared, noteNone, nameToShow ? nameToShow : GetLongNameStr(&ident));

}

void CComSyntax::DoNameDepend()
{
	CListK<CComNameDepData> left, right, *src, *dst;
	CListKElem<CComNameDepData> *srcPtr, *dstPtr;
	CComOBBaseData* d;
	CKerName *n, *n1;
	int op, id;

	if(look == lexStartBlock)
	{
		Step();			// pryc se zacatkem bloku
		while(look != lexEndBlock)
		{
			DoOneDepend(&d, &n, id);
			if(look!=opComma && look!= lexEndBlock)
				throw CComError(errUnexpectedSymbol, noteForgetSeparator, GetTokenStr(look));

			left.Add(CComNameDepData(d, n, id));

			if(look == opComma)
				Step();			// pryc s carkou
		}
		Step();			// pryc s koncem bloku
	} else
	{
		DoOneDepend(&d, &n, id);
		left.Add(CComNameDepData(d, n, id));
	}

	if(look!=opRightShift && look!=opLeftShift)
		throw CComError(errExpectedNameSetOperator, noteNone);
	do
	{
		op = look;
		Step();

		if(look == lexStartBlock)
		{
			Step();			// pryc se zacatkem bloku
			while(look != lexEndBlock)
			{
				DoOneDepend(&d, &n, id);
				if(look!=opComma && look!=lexEndBlock)
					throw CComError(errUnexpectedSymbol, noteForgetSeparator, GetTokenStr(look));

				right.Add(CComNameDepData(d, n, id));

				if(look == opComma)
					Step();			// pryc s carkou
			}
			Step();			// pryc s koncem bloku
		} else
		{
			DoOneDepend(&d, &n, id);
			right.Add(CComNameDepData(d, n, id));
		}

		if(op == opRightShift)
		{
			src = &left;		
			dst = &right;
		} else
		{
			dst = &left;		
			src = &right;
		}

		srcPtr = src->el;
		while(srcPtr)
		{
			n = srcPtr->data.n;
			dstPtr = dst->el;
			while(dstPtr)
			{
				n1 = dstPtr->data.n;
				if(!n->IsParent(n1))
				{
					bool depRemoved = false;
					const char* str1 = n->GetNameString();
					const char* str2 = n1->GetNameString();
					CComOBBase* b = toRemove->FindRec(str2);
					depRemoved = (b && !strcmp(b->name, str1));

					if(!depRemoved)
					{
						n->AddParent(n1);
						n1->AddChild(n);

						dTab->Add(new CComOBNameDepend(n->GetNameString(), n1->GetNameString(), pos.version, pos, dstPtr->data.id, srcPtr->data.id));
						
						if(slowMode)
							if(names->Compare(n, n1) == 3)
							{
								char buf[2*cMaxIdentifierLn+10];		
								sprintf(buf, "%s, %s", n->GetNameString(), n1->GetNameString());
								throw CComError(errCycleInNameDependencies, noteNone, buf);
							}
					} else
					{
						removed->FindOrAdd(str1, new CComOBBase(str2, b->version, pos));
						removedNr++;
					}
				}
				dstPtr = dstPtr->next;
			}
			srcPtr = srcPtr->next;
		}
		if(look==opRightShift || look==opLeftShift)
		{
			left.DeleteAll();
			while(!right.IsEmpty())
                left.Add(right.RemoveHead());
		}
	} while(look==opRightShift || look==opLeftShift);
}

void CComSyntax::DoOneDepend(CComOBBaseData** d, CKerName** n, int& id)
{
	CComOBBase* b;
	assert(n && d);
	
	DoNameCompleteParse(&b, &id);
	*d = (CComOBBaseData*)b;

	*n = names->GetNamePointer((*d)->kerName);
	assert(n);
}

CComOBBaseData* CComSyntax::GetDefinedName(const char* name, int& id)
{
/*
	if((id = nTab->Find(name)) != cUnknown)
		return (CComOBBaseData*)nTab->Get(id);
*/
	if((id = oTab->Find(name)) != cUnknown)
	{
		CComOBObject* obj = (CComOBObject*)oTab->Get(id);
		id = nTab->Find(obj->kerName);
		return (CComOBBaseData*)nTab->Get(id);
	}

	if((id = nTab->Find(name)) != cUnknown)
	{
		return (CComOBBaseData*)nTab->Get(id);
	}


	// specialni typ?
	char buf[64];

	assert(strlen(name) < 50);
	strcpy(buf, "_KSID_");
	strcat(buf, name);

	id = nTab->Find(buf);
	return (id != cUnknown) ? (CComOBBaseData*)nTab->Get(id) : 0;
}

void CComSyntax::DoGlobalDef()
{
	int type;

	DoType(type);
    
	DoOneGlobal(type);
	while(look == opComma)
	{
		Step();						// pryc s carkou
		DoOneGlobal(type);
	}
}

void CComSyntax::DoOneGlobal(int type)
{
	int pointer, id, version;

	DoPointer(pointer, type);		// pointer

	DoNameParts(&nameStr, maxNameLn, &version);

	// Nebyla uz definovana stejnojmena glob. promenna?
	if(gTab->FindVersion(nameStr, version) != cUnknown)
		throw CComError(errAlreadyDefined, noteNone, GetLongNameStr(&ident));

	// Vytvorim novy zaznam pro glob. promennou
	CComOBGlobal* newGlobal = new CComOBGlobal(nameStr,version,identPos,uTab);
	assert(newGlobal);
	newGlobal->SetData(type,pointer);
	// Pridam zaznam do tabulky object browseru
	id = gTab->Add(newGlobal);

	CComOBName* newName = new CComOBName(newGlobal->name, newGlobal->kerName, pos.version, uTab->Get(pos.version)->name, pos,kwThis);
	assert(newName);	
	int newNameID = gnTab->Add(newGlobal->kerName, newName);
	nameSearch->Add(&ident, newName, newNameID);

	Put(synGlobalDefinition);	
	Put(id,false);

	if(look == kwEdit)
		DoDataEditTags(newGlobal);
}

void CComSyntax::DoModifyOrRemove(enmDirtyOpType type)
{
	CComOBConstant* c;
	CComOBConstant* c1;
	CListK<int>* tokens;
	int cid;
	int versionData;

	Step();

	CComPosition idPos = pos;
	c = MatchConstant(kwString);

	switch(type)
	{
	case dotModify:
		tokens = new CListK<int>;
		if(look == lexSeparator)
			throw CComError(errMissingType, noteNone);

		while(look != kwEndModify && look != lexEOF)
		{
			tokens->AddTail(look);
			Step();
		}

        if(look == lexEOF)
			throw CComError(errExpectedSymbol, noteNone, GetTokenStr(kwEndModify));
		Step();
		if(secondPass)
			toModify->FindOrAdd(c->name, new CComOBModifyRec(c->name, tokens, idPos));
		else
			toModify->Add(c->name, new CComOBModifyRec(c->name, tokens, idPos));
		break;

	case dotRemove:
	case dotInherit:
		if(IsConstantToken(look) && type == dotRemove)
		{
			c1 = MatchConstant(kwString, noteNone, &cid);
			versionData = cid;
		} else
		{
			c1 = c;
			versionData = (type == dotRemove ? cUnknown : kwInherit);
		}

		if(secondPass)
			toRemove->FindOrAdd(c->name, new CComOBBase(c1->name, versionData, idPos));
		else
			toRemove->Add(c->name, new CComOBBase(c1->name, versionData, idPos));
		break;

	default:
		assert(false);
	}
}

void CComSyntax::DoOp(enmOperators op, CComShortPosition pos)
{
	constConstant = 0;
	try
	{
		cs.DoOp(op, GetOpArity(op), constType);
	} catch(CComError e)
	{
		e.pos.column = pos.column;
		e.pos.line = pos.line;
		e.pos.fileID = pos.fileID;
		throw e;
	}
}

void CComSyntax::DoPostPass()
{
	CComOBObject* o;
	CComOBMethod* m;
	CComOBAttribute* a;
	CComOBAttributeGroup* ag;
	int i, j;
	
	for(i=0; i<oTab->GetCount(); i++)
	{
		o = objBrowser->GetObject(i);			
		
		for(j=0; j<o->mTab->GetCount(); j++)
		{
			m = o->GetMethod(j);

			if(m->inherit)
				Inherit(o, m, 0);
		}

		for(j=0; j<o->aTab->GetCount(); j++)
		{
			a = o->GetAttribute(j);			

			if(a->inherit)
				Inherit(o, a, 0, false);
		}

		for(j=0; j<o->gList.GetCount(); j++)
		{
			ag = o->gList[j];

			if(ag->grpAtr->inherit)
				Inherit(o, ag, 0);
		}

		for(j=0; j<o->scrList.GetCount(); j++)
		{
			a = o->scrList[j];

			if(a->inherit)
				Inherit(o, a, 0, true);
		}
	}

	names->CreateMatrix();
	if(names->NameInCycle)
		throw CComError(errCycleInNameDependencies);
}

void CComSyntax::Inherit(CComOBObject* o, CComOBMethod* m, int fromIndex)
{
	CComOBObject* o1;
	CComOBMethod* oldMet;
	int i, j;
	bool alreadyInherited;

	for(i=fromIndex; i<oTab->GetCount(); i++)
	{
		o1 = objBrowser->GetObject(i);			
		if(names->Compare(names->GetNamePointer(o->kerName), names->GetNamePointer(o1->kerName)) == 1)
		{
			CComOBMethod* newMet = new CComOBMethod(*m);		
			newMet->inherit = (m->inherit == inhInheritable) ? inhInherited : inhNotInheritable;
			newMet->usedAttributes.DeleteAll();
			newMet->inObj = o1;

			alreadyInherited = false;
			for(j=0; j<o1->mTab->GetCount(); j++)
			{
				oldMet = o1->GetMethod(j);
				
				if(oldMet->inObj == o1 && !strcmp(oldMet->kerKonkretniID, newMet->kerKonkretniID))
				{
					alreadyInherited = true;
					break;
				}
			}

			if(!alreadyInherited)
				newMet->id = o1->mTab->Add(newMet);
			else
				SAFE_DELETE(newMet);
		}
	}

}

void CComSyntax::Inherit(CComOBObject* o, CComOBAttribute* a, int fromIndex, bool scripted)
{
	CComOBObject* o1;
	CComOBAttribute* oldAtr;
	int i, j;
	bool alreadyInherited;

	for(i=fromIndex; i<oTab->GetCount(); i++)
	{
		o1 = objBrowser->GetObject(i);			
		if(names->Compare(names->GetNamePointer(o->kerName), names->GetNamePointer(o1->kerName)) == 1)
		{
			CComOBAttribute* newAtr = new CComOBAttribute(*a);		
			newAtr->inherit = (a->inherit == inhInheritable) ? inhInherited : inhNotInheritable;
			newAtr->accessMethods.DeleteAll();
			newAtr->inObj = o1;

			alreadyInherited = false;
			
			for(j=0; j< (scripted ? o1->scrList.GetCount() : o1->aTab->GetCount()); j++)
			{
				oldAtr = (scripted ? o1->scrList[j] : o1->GetAttribute(j));

				if((oldAtr->inObj == o1 || scripted) && !strcmp(oldAtr->kerName, newAtr->kerName))
					alreadyInherited = true;
			}

			if(!alreadyInherited)
			{
				if(scripted)
					o1->scrList.Add(newAtr);
				else
					o1->aTab->Add(newAtr);

				inheritedAttributes.Add(newAtr);
				inheritedAttributes.Add(a);
			} else
				SAFE_DELETE(newAtr);
		}
	}
}

void CComSyntax::Inherit(CComOBObject* o, CComOBAttributeGroup* ag, int fromIndex)
{
	CComOBObject* o1;
	CComOBAttributeGroup* oldAtrGrp;
	int i, j;
	bool alreadyInherited;

	for(i=fromIndex; i<oTab->GetCount(); i++)
	{
		o1 = objBrowser->GetObject(i);			
		if(names->Compare(names->GetNamePointer(o->kerName), names->GetNamePointer(o1->kerName)) == 1)
		{
			CComOBAttributeGroup* newAtrGrp = new CComOBAttributeGroup(*ag);	
			assert(newAtrGrp->grpAtr);
			newAtrGrp->grpAtr->inherit = (ag->grpAtr->inherit == inhInheritable) ? inhInherited : inhNotInheritable;
			newAtrGrp->grpAtr->accessMethods.DeleteAll();
			newAtrGrp->grpAtr->inObj = o1;

			alreadyInherited = false;
			
			for(j=0; j<o1->gList.GetCount(); j++)
			{
				oldAtrGrp = o1->gList[j];

				if(oldAtrGrp->grpAtr->inObj == o1 && !strcmp(oldAtrGrp->grpAtr->kerName, newAtrGrp->grpAtr->kerName))
					alreadyInherited = true;
			}

			if(!alreadyInherited)
			{
				o1->gList.Add(newAtrGrp);
				for(j=0; j<ag->members.GetCount(); j++)
					Inherit(o, ag->members[j], fromIndex, false);
			} else
				SAFE_DELETE(newAtrGrp);
		}
	}
}


void CComSyntax::CopyQueueFromPos(int ref)
{
	int i = refs[ref];
	int j = queueUp;

	while(i != j)
	{
		while((queue[i] == tokVoidToken || queue[i] == synAssigning)&& i != j)
			i = (i+1) % queueLn;
		
		if(i == j)
			break;

		QueuePut(queue[i]);
		i = (i+1) % queueLn;
	}
}

void CComSyntax::RenameMethodOnPos(int ref, int newID)
{
	int i = refs[ref];
	assert(queue[i] == synMethodDecl);
	while(!IsMethodToken(queue[i]))
		i = (i+1) % queueLn;

	queue[i] = newID;
}

void CComSyntax::SetLastOpState(enmTokens state)
{
	int i;
	i = queueUp;
	do
	{
		i = (i-1) % queueLn;
	} while(!IsOperatorToken(queue[i]) && queue[i] != synStartExpression && queue[i] != synNextParameter && queue[i] != synArgsEnd && queue[i] != synSafeArgument);
	
	if(!IsOperatorToken(queue[i]))
		return;

	i = queueUp;
	do
	{
		queue[i] = queue[(i-1) % queueLn];
		i = (i-1) % queueLn;
	} while(!IsOperatorToken(queue[i]));

	queue[i] = state;
	queueUp = (queueUp + 1) % queueLn;

	if(QueueEmpty())
		QueueExpand();
}
