#include "stdafx.h"
#include "ComSemantic.h"
#include "ComKerServices.h"
#include "fs.h"
#include "ks.h"

CComSemantic::CComSemantic(CComObjectBrowser* ob, int _queueSize, CListK<CComOBAttribute*>* _inhAttrList) 
: CComPart(ob, _queueSize)
{
	look = old = cUnknown;
	exps[0] = &es.e1;
	exps[1] = &es.e2;
	exps[2] = &es.e3;
	inhAttrList = _inhAttrList;
	assert(inhAttrList);

	defaultVersion = findAny;
	doingParams = false;
	postPassCache = 0;
}

CComSemantic::~CComSemantic()
{
	SAFE_DELETE(postPassCache);
}

void CComSemantic::DoPart()
{
	index = 0;
	postPassInput = 0;
	if(look == cUnknown)
		Step(false);

	while(look != lexEOF)
	    DoTopDecl();

	DoPostPass();
	Put(lexEOF);
}

void CComSemantic::Step(bool put)
{ 
	old = look; 
	if(put)
		Put(look);

	if(postPassInput)
		look = postPassCache->GetToken();
	else
	{
		if(look == lexEOF)
			throw CComError(errUnexpectedEOF);
		look = GetParent(); 
		index++;
	}
}

void CComSemantic::DoTopDecl()
{
	int gid;
	switch(look)
	{
	case synGlobalDefinition:
		Step(false);		// pryc se synGlobalDefinition
		gid = look;
		Step();				// pryc s id globalni promenne
		while(look == kwEdit)
		{
			CComOBGlobal* glb = (CComOBGlobal*)gTab->Get(gid);		
			DoDataEditTags(glb);
		}
		break;
	case synStartObjectDecl:
		DoObjectDecl();
		break;
	default:
		assert(false);
		Step();
	}
}

void CComSemantic::DoObjectDecl()
{
	Step();				// pryc s synStartObjectDecl
	assert(IsObjectToken(look));
	inObjID = look;
	inObj = (CComOBObject*)oTab->Get(inObjID = look);
	Step();				// pryc s id objektu
	
	while(look != synEndObjectDecl)
	{
		switch(look)
		{
		case kwScripted:
			DoScripted();
			break;

		case synAttributeDecl:
			DoAttributeDecl();
			break;
		case synMethodDecl:
			DoMethodDecl();
			break;
		default:
			assert(false);
		}
	}
	inObj = 0;
	assert(look == synEndObjectDecl);
	Step();			// pryc se synEndObjectDecl
}

void CComSemantic::DoScripted()
{
	int id;
	CComOBAttribute* atr;

	assert(look == kwScripted);
	Step();			// pryc s kwScripted

	assert(IsDataToken(look));
	id = look - tokDataFirst;
	atr = (CComOBAttribute*)inObj->scrList[id];
	Step();

	assert(look == kwEdit);
	Step();
	assert(look == tokDataFirst + kwScripted);
	Step();

	DoExpressionMain();
	es.Load1();
	if(!IsNameToken(res.id))
		throw CComError(errExpectedMethodName, noteNone);
	
	atr->knownName = res.id;
	while(look == kwEdit)
		DoDataEditTags(atr);
}

void CComSemantic::DoAttributeDecl()
{
	assert(inObj);
	Step(false);				// pryc s synAttributeDecl
	CComOBAttribute* attr = (CComOBAttribute*)inObj->aTab->Get(look);
	Step(false);				// pryc s id atributu
	if(attr->dType==kwVoid && attr->pointer==0)
		throw CComError(errInvalidType,noteVoidData,attr->name);
	while(look == kwEdit)
		DoDataEditTags(attr);
}

void CComSemantic::DoMethodDecl()
{
	CComOBArgument* arg;
	assert(inObj);
	Step();				// pryc s synMethodDecl

	inMethod = (CComOBMethod*)inObj->mTab->Get(look);
	Step();				// pryc s id metody
	
	while(IsArgumentToken(look))
	{
		arg = (CComOBArgument*)inMethod->aTab->Get(look);
		Step();			// pryc s id argumentu
		while(look == kwEdit)
			DoDataEditTags(arg);
	}
	Put(semMethodBody, false);
	
	DoMethodBody();
}

void CComSemantic::DoMethodBody()
{
	inMethod->mem = index-1;
	returnsValue = false;
	blockDepth = currentBlock = totalBlockNr = 0;
	DoCommand();
	assert(blockDepth==0 && currentBlock==0);

	if((inMethod->dType!=kwVoid || inMethod->pointer) && !returnsValue)
		throw CComError(errShouldReturnValue,noteNone,inMethod->name);

	assert(blockDepth == 0);
	inMethod = 0;
}

void CComSemantic::DoCommand(bool putStartBlock)
{
	int i, oldBlock;

	es.Reset();
	switch(look)
	{
	case synStartBlock:
		oldBlock = currentBlock;
		blockDepth++;
		totalBlockNr++;
		currentBlock = totalBlockNr;
		Step(putStartBlock);			// pryc s synStartBlock
		while(look != synEndBlock)
			DoCommand();

		blockDepth--;
		currentBlock = oldBlock;
		Step(putStartBlock);			// pryc s synEndBlock
		break;

	case synStartExpression:
		DoExpressionMain();
		break;

	case kwBreak:			case kwContinue:		
	case lexSeparator:
		Step();
		break;

	case kwIf:
		Step();			// pryc s kwIf
		DoExpressionMain();
		DoCommand();
		if(look == kwElse)
		{
			Step();
			DoCommand();
		}
		break;

	case kwWhile:
		Step();			// pryc s kwWhile
		DoExpressionMain();
		DoCommand();
		break;

	case kwDo:
		Step();			// pryc s kwDo
		DoCommand();
		assert(look == kwWhile);
		Step();			// pryc s kwWhile
		DoExpressionMain();
		break;

	case kwFor:
		Step();			// pryc s kwFor
		DoExpressionMain();
		DoExpressionMain();
		DoExpressionMain();
		DoCommand();
		break;

	case kwSwitch:
		Step();			// pryc s kwSwitch
		Put(synCaseCount, false);
		DoExpressionMain();
		i = 0;
		while(look == kwCase)
		{
			i++;
			Step();		// pryc s kwCase
			DoExpressionMain();
			while(look != synEndCase)
				DoCommand();
			Step();		// pryc se synEndCase
		}
		if(look == kwDefault)
		{
			Step();		// pryc s kwDefault		
			while(look != synEndCase)
				DoCommand();
			Step();		// pryc se synEndCase
		}
		QueueTightSubstitute(synCaseCount, tokDataFirst+i);
		break;

	case kwReturn:
		DoReturn();
		break;

	case kwWith:
		Step(false);
		i = defaultVersion;
		defaultVersion = look;
		Step(false);
		DoCommand(false);
		defaultVersion = i;
		break;

	case synLocalDefinition:	
		Step();			
		Step();	
		Put(tokDataFirst+pos.line);
		break;

	default:
		assert(false);
	}
}

void CComSemantic::DoReturn()
{
	CComOBConstant* c;

	assert(look==kwReturn);
	Step();				// pryc s kwReturn
	if(look == synStartExpression)
	{
		DoExpressionMain();
	
		returnsValue = true;
		if(Convert(res.type,res.pointer,inMethod->dType,inMethod->pointer)==cUnknown)
		{
			if(IsConstantToken(res.id))
			{
				c = (CComOBConstant*)cTab->Get(res.id);
				if(c->dType==kwInt && c->pointer==0 && c->value.intData == 0)
					return;
			}	
			char buf[2*cMaxIdentifierLn+5];	
			sprintf(buf,"%s, %s",GetTypeStr(res.type,res.pointer),GetTypeStr(inMethod->dType,inMethod->pointer));
			throw CComError(errReturnTypeMismatch,noteNone,buf);
		}
	} else
	{
		assert(look == kwVoid);
		if(inMethod->dType!=kwVoid || inMethod->pointer)
			throw CComError(errReturnTypeMismatch,noteNone);

		Step();
		es.Push(CComExpression(cUnknown,false,false,false,kwVoid,0));
	}
}


void CComSemantic::DoNew()
{
	int type, pointer;
	CListKElem<CComExpression>* stackPos;

	assert(look == kwNew);
	Step();			// pryc s new
	Put(tokDataFirst + pos.line, false);

	if(look != kwVarType)
	{
		type = look;
		Step();			// pryc s typem
		assert(IsDataToken(look));
		pointer = look - tokDataFirst;
		Step();			// pryc s pointerem

		if(type==kwObjptr && pointer==1)
			throw CComError(errExpectedObjectType, noteNone, GetTokenStr(kwObjptr));
	} else
	{
		type = kwObjptr;
		pointer = 1;
		Step();
		stackPos = es.GetPos();
		DoExpressionMain();
		es.Load1();
		if(!IsNameType(res.type) || res.pointer != 1)
			throw CComError(errExpectedName, noteNone);
		es.RestorePos(stackPos);
	}

	if(look == opBracketStart)
	{
		Step();								// pryc s hranatou zavorkou
		
		stackPos = es.GetPos();
		DoExpressionMain();
		if(Convert(&res, kwInt, 0) == cUnknown)
			throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(res.type, res.pointer, kwInt, 0));
		es.RestorePos(stackPos);

		pointer++;
	} else
		// new na objekt, dynamicke pole nebo jmeno -> nezvysuju pointer, jinak zvysuju
		if(!IsObjectType(type) && !IsDynamicArrayToken(type) && !IsNameType(type) || pointer != 1) 
			pointer++;

	es.Push(CComExpression(cUnknown, false, false, false, type, pointer));
}

void CComSemantic::DoDelete()
{
	assert(look == kwDelete);
	Step();			// pryc s delete
	Put(tokDataFirst + pos.line, false);

	es.Load1();

	if(es.e1.type == cUnknown)
		throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(es.e1.id));

	if(!IsGoodOldPointer(&es.e1) && !(IsObjectType(es.e1.type) && es.e1.pointer==1))
		throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(es.e1.type, es.e1.pointer));
	
	Put(es.e1.type, false);
	Put(tokDataFirst + es.e1.pointer, false);

	es.Push(CComExpression(cUnknown, false, false, false, kwVoid, 0));
}

void CComSemantic::DoSender()
{
	assert(look == kwSender);
	Step();			// pryc se sender

	es.Push(CComExpression(kwObjptr, false, false, false, kwObjptr, 1));
}

void CComSemantic::DoThis()
{
	assert(look == kwThis);
	Step();			// pryc s this

	assert(inObjID != cUnknown);
	es.Push(CComExpression(inObjID, false, false, false, inObjID, 1));
}

void CComSemantic::DoAssigned()
{
	assert(look == kwAssigned);
	Step();			// pryc s assigned
	
	if(inMethod->storage == kwDirect)
		throw CComError(errDirectiveNotAllowedInDirect, noteNone, GetTokenStr(kwAssigned));

	assert(IsUnknownToken(look));
	const char* idName = uTab->Get(look)->name;
	int id;

	Step(false);	// pryc s argumentem
	if((id = inMethod->aTab->Find(idName)) == cUnknown)
		throw CComError(errExpectedArgument, noteNone, idName);

	Put(tokDataFirst + id - tokArgumentFirst, false);
	es.Push(CComExpression(cUnknown, false, false, false, kwInt, 0));
}

void CComSemantic::DoTypeOf()
{
	assert(look == kwTypeOf);
	Step();			// pryc s typeof

	DoExpressionMain();
	es.Load1();
	if(!IsObjectType(es.e1.type) || es.e1.pointer != 1)
		throw CComError(errExpectedObjectType, noteNone, GetCollisionTypesStr(es.e1.type, es.e1.pointer));
	
	es.Push(CComExpression(cUnknown, false, false, false, kwName, 1));
}

void CComSemantic::DoExpressionMain()
{
	doingParamsList.DeleteAll();
	doingParams = false;
	nestedCalls = callDepth = 0;
	DoExpression();
}

void CComSemantic::DoExpression()
{
	assert(look == synStartExpression);
	Step();				// pryc s synStartExpression

	Put(tokDataFirst + pos.line, false);

	while(look != synEndExpression)
	{
		switch(look)
		{
		case synStartExpression:		DoExpression();			break;
		case kwNew:						DoNew();				break;
		case kwDelete:					DoDelete();				break;
		case kwSender:					DoSender();				break;
		case kwThis:					DoThis();				break;
		case kwAssigned:				DoAssigned();			break;
		case kwTypeOf:					DoTypeOf();				break;
		
		case opParentStart:
			DoMethodCall();
			Put(res.type, false);
			Put(tokDataFirst + res.pointer, false);
			Put(tokDataFirst + (res.isLValue ? 1 : 0), false);
			doingParams = doingParamsList.RemoveHead();
			callDepth--;
			break;

		case synArgsEnd:
			es.Push(CComExpression(look,false,false,false,cUnknown,cUnknown));
			Step();
			doingParamsList.Add(doingParams);
			doingParams = true;
			nestedCalls++;
			callDepth++;
			break;

		case synLocalizedArgument:
			Step();
			es.Load1();
			es.e1.data1 = synLocalizedArgument;
			es.Push(es.e1);
			break;

		case synNextParameter:
			Step();
			doingParams = true;
			break;

		case opDoubleDot:
			Step();				// pryc s dvojteckou
			assert(IsDataToken(look));
			if(look-tokDataFirst != kwFirst)
			{
				es.Load1();
				if(!es.e1.isLValue)
					throw CComError(errExpectedLValue, noteNone);
				es.Push(es.e1);
			}
		
			Step();				// pryc s retem
			DoSafeCallCheck(0, 0);
			break;

		case synSafeArgument:
			Step();
			doingParams = false;
			break;

		case synAssigning:		case synAddressing:
		case synLogicOr:		case synLogicAnd:
		case synConditional1:	case synConditional2:
			Step();
			break;

		default:
			if(IsOperatorToken(look))
			{
				DoOp(look);
				Step();
				Put(res.type,false);
				Put(tokDataFirst + res.pointer,false);
				Put(tokDataFirst + (res.isLValue ? 1 : 0),false);
			} else
				if(IsKernelService(look))
				{
					es.Push(CComExpression(look,false,false,false,cUnknown,cUnknown,&pos));
					Step();
					if(look != synArgsEnd)
						throw CComError(errExpectedMethodCall, noteNone);
				} else
					if(IsKernelConstant(look))
					{
						es.Push(CComExpression(look,false,false,false,kwInt,0,&pos));
						Step();
					} else
						DoPrimary();
		}
	}
	Step();			// pryc s synEndExpression
	res = es.expStack.el->data;
}

void CComSemantic::DoPrimary()
{
	int type, pointer, outID, id, id2, data1, data2, version;
	bool isConstant, isLValue, hasSideEffect, ambiguity, foundNothing;
	const char* idName;
	CComOBBaseData* d = 0;
	CComShortPosition versionPos;

	isConstant = hasSideEffect = ambiguity = foundNothing = false;
	isLValue = true;
	version = defaultVersion;
    
	type = id = data1 = data2 = cUnknown;
	pointer = 0;
	outID = look;

	Step(false);							// pryc s identifikatorem
	
	if(IsConstantToken(old))
	{
		isConstant = true;	
		isLValue = false;
		d = (CComOBBaseData*)cTab->Get(old);
	} else
	{
		if(IsUnknownToken(old))	
		{
			idName = DoIdentifier(outID, data1, data2, version, versionPos, foundNothing);
			if(version == findAny)
				version = defaultVersion;
			
			if(!foundNothing || doingParams)
			{
				type = IsDirectNameToken(outID) ? kwDirect : kwName;
				pointer = 1;
				isLValue = false;
			} 
		} else
			if(IsKnownNameAtrToken(old))
				idName = KerVarUsesNames[old-knFirst];
			else
				if(IsKnownNameToken(old))
				{
					idName = KnownNames[old-knSecond].namestr+6;

					if(!foundNothing || doingParams)
					{
						type = kwName;
						pointer = 1;
					}
				} else
					throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(old));

		if(look != synLocalizedArgument)
			// Lokalni promenna? Muze jich byt vic stejneho jmena (v ruznych blocich metody).
			if(inMethod && (id2 = inMethod->lTab->FindFirst(idName)) != cUnknown) 
			{
				// Hledam promennou ve stejnem nebo nejblizsim vyssim bloku.
				int fid = id2;
				int deepest = 0;
				CComOBLocal* loc;
				while(id2 != cUnknown)
				{
					loc = (CComOBLocal*)inMethod->lTab->Get(id2);
					if(loc->blockDepth > deepest && loc->blockDepth < blockDepth || // nejblizsi vyssi
						loc->blockNr == currentBlock)								// nebo stejny
						if(loc->posDefined.line < pos.line || 
							loc->posDefined.line == pos.line && loc->posDefined.column <= pos.column ||
							postPassInput)		// u zdedenych nekontroluju pozici
						{
							deepest = loc->blockDepth;
							d = loc;
							id = id2;
						} 
					id2 = inMethod->lTab->FindNext(idName);
				}
			} else
				if(inMethod && (id = inMethod->aTab->FindVersion(idName, version)) != cUnknown)	// Argument?
				{
					d = (CComOBBaseData*) inMethod->aTab->Get(id);
					if(IsStringType(d->dType) && inMethod->storage == kwDirect)
						isLValue = false;
				} else
					if((id = inObj->aTab->FindVersion(idName, version, &ambiguity)) != cUnknown)	// Atribut objektu?
					{
						CComOBAttribute* atr;
						atr = (CComOBAttribute*)inObj->aTab->Get(id);
						
						id2 = inObj->aTab->FindFirst(atr->name, version);
						while(id2 != cUnknown)
						{
							atr = (CComOBAttribute*)inObj->aTab->Get(id2);

							if(atr->objDef == inObj)
								break;

							id2 = inObj->aTab->FindNext(atr->name, version);
						}
						
						if(id2 != cUnknown)
							id = id2;

						atr = (CComOBAttribute*)inObj->aTab->Get(id);

						if(inMethod)		// Nemusi byt v metode - napr. chybne zadana metoda skriptovane promenne
						{
							atr->accessMethods.AddUnique(inMethod->id);	// pridam ID metody do seznamu pristupujicich metod u atributu
							inMethod->usedAttributes.AddUnique(id);		// a ID atributu do seznamu pouzitych u metody
						}
						d = atr;
					} else
						if((id = gTab->FindVersion(idName, version, &ambiguity)) != cUnknown)	// Globalni promenna?
							d = (CComOBBaseData*) gTab->Get(id);
						else
							if((id = oTab->FindVersion(idName, version, &ambiguity)) != cUnknown)
							{
								id = ((CComOBObject*)oTab->Get(id))->kerNameID;
								type = kwObjectName;
								pointer = 1;
								isLValue = false;
							} else
							{
								if((id = mTab->FindVersion(idName, version, &ambiguity)) != cUnknown)
								{
									id = ((CComOBBaseData*)mTab->Get(id))->kerNameID;
									type = kwMethodName;
									pointer = 1;
									isLValue = false;
								} else
									if((id = aTab->FindVersion(idName, version, &ambiguity)) != cUnknown)
									{
										id = ((CComOBBaseData*)aTab->Get(id))->kerNameID;
										type = kwParamName;
										pointer = 1;
									}
							}
	}

	if(ambiguity && version == findAny)
		throw CComError(errMissingVersion, noteNone, idName, &versionPos);

	if(id != cUnknown)
		outID = id;
	else
		if(version != findAny && foundNothing)
			throw CComError(errWrongVersion, noteNone, uTab->Get(version)->name, &versionPos);

	if(d)
	{
		type = d->dType;
		pointer = d->pointer;
		isLValue = true;
	}

	CComExpression exp(outID,isConstant,isLValue,hasSideEffect,type,pointer,&pos, d ? &(d->array) : 0);
	exp.data1 = data1;
	exp.data2 = data2;
	es.Push(exp);
	
	Put(outID, false);
} 

const char* CComSemantic::DoIdentifier(int& outID, int& data1, int& data2, int& version, CComShortPosition& versionPos, bool& foundNothing)
{
	CListK<CComNSEEntry>* nList;
	CComNSEEntry nse;
	int id;

	ident.DeleteAll();
	
	id = old;
	if(look == lexEditorStringSeparator) 
		DoReadVersion(version); 
	else
		version = findAny;

	ident.AddTail(CComNSENode(id, version));

	while(look == opMember4Dot)
	{
		Step(false);					// pryc s ::
		assert(IsUnknownToken(look));
		id = look;
		Step(false);					// pryc s identifikatorem
		
		if(look == lexEditorStringSeparator) 
			DoReadVersion(version); 
		else
			version = findAny;

		ident.AddTail(CComNSENode(id, version));
	}
	nList = nameSearch->FindRec(&ident);
	versionPos = pos;
	if(foundNothing = !nList || nList->num == 0)
		if(ident.num == 1)
		{
			data1 = data2 = cUnknown;
			return uTab->Get(ident.el->data.id)->name;
		} else
			throw CComError(errUnknownIdentifier, noteNone, GetLongNameStr(&ident));
	
	if(nList->num == 1)									// jednoznacny identifikator
		nse = nList->el->data;
	else
		if(IsDirectNameToken(nList->el->data.nameID))	// vice direct metod (volani bez udani objektu]
			nse = ResolveDirectMethodAmbiguity(nList);	
		else
			nse = ResolveVersionAmbiguity(nList, version);	
		
	outID = nse.nameID;
	data1 = nse.name->objID;
	
	if((data2 = nse.name->metID) != cUnknown && data1 != inObjID)
	{
		int id = inObj->mTab->Find(nse.name->name);
		if(id != cUnknown)
		{
			data1 = inObjID;
			data2 = id;
		}
	}
	return nse.name->name;
}

CComOBConstant* CComSemantic::DoReadVersion(int& version)
{
	assert(look == lexEditorStringSeparator);

	CComShortPosition versionPos = pos;
	Step(false);			// pryc s @
	assert(IsConstantToken(look));
	CComOBConstant* c = (CComOBConstant*)cTab->Get(look);
	Step(false);			// pryc s id konstanty

	version = uTab->Find(c->name);
	if(version == cUnknown)
		if(!_strcmpi(c->name, cStrThisVersion))
			version = pos.version;
		else
			throw CComError(errUnknownVersion, noteNone, c->name, &versionPos);

	return c;
}

CComNSEEntry CComSemantic::ResolveDirectMethodAmbiguity(CListK<CComNSEEntry>* nList)
{
	CListKElem<CComNSEEntry>* p = nList->el;
	while(p)
	{
		if(p->data.name->objID == inObjID)
			break;
		p = p->next;
	}
	
	if(p)
		return p->data;

	p = nList->el;
	while(p)
	{
		if(!IsDirectNameToken(p->data.nameID))
			break;
		p = p->next;
	}
	assert(p);
	return p->data;
}


CComNSEEntry CComSemantic::ResolveVersionAmbiguity(CListK<CComNSEEntry>* nList, int& version)
{
	assert(nList && nList->el);

	CComNSEEntry* ret = CheckVeryFirst(nList);
	if(ret && inObj->version == ret->name->version)
		return *ret;

	CComShortPosition versionPos = pos;

	if(version == findAny)
		if(defaultVersion != findAny)
			version = defaultVersion;
		else
			throw CComError(errMissingVersion, noteNone, nList->el->data.name->name);

	CListKElem<CComNSEEntry>* p = nList->el;
	while(p)
	{
		if(p->data.name->version == version)
			return p->data;

		p = p->next;
	}

	throw CComError(errWrongVersion, noteNone, uTab->Get(version)->name, &versionPos);

	return nList->el->data;
}

CComNSEEntry* CComSemantic::CheckVeryFirst(CListK<CComNSEEntry>* nList)
{
	int maxVersion = cUnknown;
	CListKElem<CComNSEEntry> *p, *q;

	p = nList->el;
	q = 0;
	while(p)
	{
		if(p->data.name->version > maxVersion)
		{
			maxVersion = p->data.name->version;
			q = p;
		}

		p = p->next;
	}
	return inObj->version == maxVersion ? &(q->data) : 0;
}


void CComSemantic::DoMethodCall()
{
	// Na zasobniku mam parametry jako vyrazy, synArgsEnd, metodu a nakonec objekt
	CListK<CComExpression> params;
	CComOBObject* o;
	CComOBMethod* m;
	int id;
	const char* idName;
	int time = cUnknown;
	CComShortPosition posTime, posName;

	assert(look == opParentStart);
	Step();									// pryc s operatorem volani
	Put(tokDataFirst+pos.line, false);		// radka volani pro kompilovane skripty

	if(look == synTimeDecl)
	{
		Step();							// pryc se synTimeDecl
		time = look;
		posTime = pos;
		Step();							// pryc s casem
	}

	if(time==kwCallend || time==kwTimed)
	{
		posName = pos;
		CListKElem<CComExpression>* stackPos = es.GetPos();
		DoExpression();
		es.RestorePos(stackPos);
	}

	if(time==kwCallend)
		if(Convert(res.type,res.pointer,kwObjptr,1) == cUnknown)
			throw CComError(errExpectedObjectType, noteNone);
	
	if(time==kwTimed)
		if(Convert(res.type,res.pointer,kwInt,0) == cUnknown)
			throw CComError(errExpectedTime, noteNone);

	es.Load1();
	while(es.e1.id != synArgsEnd)		// nactu zpracovane parametry do seznamu
	{
		params.Add(es.e1);
		es.Load1();
	}

	es.Load2();		// v es.e1 mam ID objektu, v es.e2 ID metody

	if(IsKernelService(es.e2.id))
		return DoKernelServiceCall(&params);

	// Objekt muze byt jiz znamy (atribut, lok. prom. atd., resp. this), nerozhodnutelny (sender) a nebo neznamy (polozka struktury)

	id = es.e1.type;
	if(IsObjectToken(id))
		o = (CComOBObject*)oTab->Get(id);
	else
		if(id == kwObjptr)	
			o = 0;
		else
			if(IsDynamicArrayToken(id))
			{
				Put(semDynamicArrayCall, false);
				return DoDynamicArrayCall(&params);
			} else
				throw CComError(errExpectedObjectType, noteNone);

		if(IsNameToken(es.e2.id))
			idName = nTab->Get(es.e2.id)->name;
		else
			if(IsDirectNameToken(es.e2.id))
				idName = dnTab->Get(es.e2.id)->name;
			else
				if(IsKnownNameToken(es.e2.id))
					idName = KnownNames[es.e2.id - knSecond].namestr+6;
				else
					if(IsUnknownToken(es.e2.id))
						idName = uTab->Get(es.e2.id)->name;
					else 
						if(IsNameType(es.e2.type))
						{
							assert(es.e2.data1 == cUnknown);
							Put(semNameCall, false);
							es.Push(res = CComExpression(cUnknown,false,false,false,kwVoid,0));
							CheckMethodParams(0, time, &params, posName, posTime);
							return;
						} else
							throw CComError(errUnexpectedSymbol, noteNone, GetTokenStr(es.e2.id));
	
	CComOBObject* o2;
	bool implicitSpec;

	if(implicitSpec = es.e2.data1 == cUnknown)
	{
		o2 = o;
		implicitSpec = true;
		Put(semImplicitSpec, false);
	} else
	{
		assert(IsObjectToken(es.e2.data1));
		o2 = (CComOBObject*)oTab->Get(es.e2.data1);
		Put(es.e2.data1, false);
	}

	// implicitni specifikace objektu -> globalni safe
	// explicitni specifikace objektu -> direct nebo lokalni safe
	
	if(implicitSpec)
	{
		if((id = mTab->Find(idName)) == cUnknown)						// neni globalni
			throw CComError(errUnknownIdentifier, noteNone, idName);	// -> chyba
		Put(tokDataFirst + id, false);

		m = (CComOBMethod*)mTab->Get(id);
		
		if(!m->global)
			throw CComError(errExpectedObjectType, noteNone);
	} else
	{	
		assert(IsMethodToken(es.e2.data2));
		m = (CComOBMethod*)o2->mTab->Get(id = es.e2.data2);

		Put(tokDataFirst + es.e2.data2, false);

		if(m->inObj != o2 && !implicitSpec)
			throw CComError(errUnknownIdentifier, noteHasntSuchMember, m->name);

		if(!m->objDef)
			if(o)
				throw CComError(errMethodBodyNotDefined, noteNone, m->name);
			else
				throw CComError(errNotDeclared, noteNone, m->name);
	}

	CheckMethodParams(m, time, &params, posName, posTime);

	if(m->storage == kwDirect)
		inMethod->dangerousCalls.AddUnique(m);

	if(time == cUnknown)	// volani
		if(m->storage == kwDirect)
			es.Push(res = CComExpression(cUnknown,false,false,false,m->dType,m->pointer));
		else
			es.Push(res = CComExpression(cUnknown,false,false,false,m->groupType,m->groupPointer));
	else					// zprava
		es.Push(res = CComExpression(cUnknown,false,false,false,kwVoid,0));
}

void CComSemantic::CheckMethodParams(CComOBMethod* m, int time, CListK<CComExpression>* params, CComShortPosition posName, CComShortPosition posTime)
{
	CListKElem<CComExpression> *p, *q;
	int numArg, i, id;
	CComOBArgument* arg;
	const char* idName;

	if(m && m->storage == kwDirect)		// U direct metod muzu zkontrolovat parametry:
	{
		if(time != cUnknown && posTime.line != cUnknown)
			throw CComError(errUnexpectedKeyword, noteTimeAndNotSafe, GetTokenStr(time), &posTime);

		if(posName.line != cUnknown)
			throw CComError(errUnexpectedSymbol, noteNone, 0, &posName);
		
		numArg = m->aTab->GetCount();
		if(params->num > numArg)
			throw CComError(errTooMuchArguments);

		p = params->el;	
		i = 0;
		while(p)
		{
			if(i == m->aTab->GetCount())
				throw CComError(errTooMuchArguments, noteNone);

			if(p->data.type == cUnknown)
				throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(p->data.id), p->data.pos);

			arg = m->GetArgument(i);

			if(Convert(p->data.type, p->data.pointer, arg->dType, arg->pointer) == cUnknown)
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(p->data.type, p->data.pointer, arg->dType, arg->pointer, arg->name), p->data.pos);
			p = p->next;	
			i++;
		}
		if(i < m->aTab->GetCount())
			throw CComError(errTooFewArguments, noteNone);
	} else
	{
		p = params->el;
		bool check = true;
		while(p)
		{
			if(check)
			{
				DoSafeCallCheck(&p->data, m);
				if(!IsKnownNameToken(p->data.id))
				{
					if(IsUnknownToken(p->data.id))
						idName =  uTab->Get(p->data.id)->name;
					else
						if(IsNameToken(p->data.id))
							idName = nTab->Get(p->data.id)->name;
						else
							if(IsArgumentToken(p->data.id))
								idName = inMethod->aTab->Get(p->data.id)->name;			// rekurze
							else
								assert(false);

					CComOBBaseData* d;
					if(p->data.data1 == synLocalizedArgument)
					{
						if(!m || (id = m->aTab->Find(idName)) == cUnknown)
							throw CComError(errUnknownIdentifier, noteNone, idName);
						d = (CComOBBaseData*)m->aTab->Get(id);
					} else
					{
						if((id = aTab->Find(idName)) == cUnknown)
							throw CComError(errUnknownIdentifier, noteNone, idName);
						d = (CComOBBaseData*)aTab->Get(id);
					}
					if(p->data.id != d->kerNameID)
						QueueBackAndForwardSubstitute(synArgsEnd, nestedCalls - callDepth + 1, p->data.id, d->kerNameID);
				}
			} else
				if(p->data.type == cUnknown)
					throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(p->data.id), p->data.pos);

			check = !check;
			q = p;
			p = p->next;	
			if(!check && !p)	// ocekava se, ze bude nasledovat hodnota, ale neni nic...
			{
				CComShortPosition tmpPos = *q->data.pos;
				tmpPos.column++;			
				throw CComError(errExpectedIdentifier, noteNone, 0, &tmpPos);
			}
		}
	}
}

void CComSemantic::DoSafeCallCheck(CComExpression* exp, CComOBMethod* callMethod)
{
	const char* idName;

	if(!exp)
	{
		es.Touch2();			// es.e1 - jmeno argumentu, es.e2 - hodnota, es.e3
		exp = &es.e1;
	}

	if(IsNameType(exp->type) && exp->pointer == 1)
		return;

	bool localized = exp->data1 == synLocalizedArgument;

	if(IsNameToken(exp->id))
		idName = nTab->Get(exp->id)->name;	
	else
		if(IsUnknownToken(exp->id))
			idName = uTab->Get(exp->id)->name;	
		else
			if(IsKnownNameAtrToken(exp->id))
				idName = KerVarUsesNames[exp->id-knFirst];
			else
				if(IsKnownNameToken(exp->id))
					idName = KnownNames[exp->id-knSecond].namestr+6;
				else
					if(IsArgumentToken(exp->id))
						idName = inMethod->aTab->Get(exp->id)->name;			// rekurze
					else
						throw CComError(errExpectedName, noteNone);

	if(localized)
	{
		if(!callMethod || callMethod->aTab->Find(idName) == cUnknown)
			throw CComError(errExpectedName, noteNone, idName);
	} else
		if(aTab->Find(idName) == cUnknown)
			throw CComError(errExpectedName, noteNone, idName);
}

void CComSemantic::DoDynamicArrayCall(CListK<CComExpression>* params)
{
	assert(IsDynamicArrayToken(es.e1.type));

	if(!IsUnknownToken(es.e2.id))
	{
		int badID = es.e2.id;
		es.e2.id = uTab->Find(GetTab(es.e2.id)->Get(es.e2.id)->name);
		QueueBackAndForwardSubstitute(synArgsEnd, badID, es.e2.id, 1);
	}

	if(es.e2.id == cTab->getcountID)
	{
		if(params->num)
			throw CComError(errTooMuchArguments, noteNone);
		es.Push(res = CComExpression(cUnknown, false, false, false, kwInt, 0));
	} else
		if(es.e2.id == cTab->setcountID)
		{
			if(params->num > 1)
				throw CComError(errTooMuchArguments, noteNone);
			else
				if(params->num == 0)
					throw CComError(errTooFewArguments, noteNone);
			
			if(Convert(&params->el->data, kwInt, 0) == cUnknown)
				throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(params->el->data.type, params->el->data.pointer, kwInt, 0));

			es.Push(res = CComExpression(cUnknown, false, false, false, kwVoid, 0));
		} else
			throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(es.e2.id));
}

void CComSemantic::DoKernelServiceCall(CListK<CComExpression>* params)
{
	int service, i;
	CComExpression e;

	service = es.e2.id - ksFirst;

	if(!IsObjectToken(es.e1.id) || oTab->Get(es.e1.id) != inObj)
		throw CComError(errUnknownIdentifier, noteHasntSuchMember, cKerServices[service].name);

	i = 0;


	while(!params->IsEmpty())
	{
		e = params->RemoveHead();
		
		if(e.type == cUnknown)
			throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(e.id), e.pos);

		while(i < cKerServices[service].paramsNr && cKerServiceParams[service][i].specUse)
			i++;

		if(i >= cKerServices[service].paramsNr)
			throw CComError(errTooMuchArguments, noteNone);

		if(KSConvert(&e, cKerServiceParams[service][i].type, cKerServiceParams[service][i].pointer) == cUnknown)
		{
			char buf[256];
			sprintf(buf, "param %d", i+1);
			throw CComError(errTypeMismatch, noteNone, GetCollisionTypesStr(e.type, e.pointer, cKerServiceParams[service][i].type, cKerServiceParams[service][i].pointer,buf));
		}
		i++;
	}

	if(i < cKerServices[service].paramsNr)			// Nebyly zadane nektere parametry.
		if(!cKerServiceParams[service][i].isDefault)	// - pokud nejsou defaultni -> chyba.
			throw CComError(errTooFewArguments, noteNone);

	es.Push(res = CComExpression(cUnknown, false, false, false, cKerServices[service].returnValue.type, cKerServices[service].returnValue.pointer));
}

void CComSemantic::DoPostCall(char* name, bool returnsValue)
{
	if(look==synTimeDecl)
	{
		Step();			// pryc se synTimeDecl
		if(look != kwMessage && returnsValue)
			throw CComError(errMessagesCantReturnValue,noteCheckTime,name);
		Step();	// pryc s casem
	}
	if(look == synPassingName)
	{
		Step();			// pryc se synPassingName
		
		DoExpression();

		if(!IsObjectType(res.type))
			throw CComError(errExpectedObjectType, noteNone);
	}
}

void CComSemantic::DoNextParameter(int& kind, char* name, int& type, int& pointer)
{
		Step();			// pryc se synNextParameter
		if(look==synSpecialCall)
		{
			if(kind == kwDirect)
				throw CComError(errBadParameterFormat,noteExpectedDirectFormat);
			kind = kwSafe;
			Step();		// pryc se synSpecialCall
			assert(look == synStartExpression);
			Step();		// pryc se synStartExpression
			if(!IsNameToken(look))
				throw CComError(errExpectedName, noteNone, GetTokenStr(look));
			strcpy(name,nTab->Get(look)->name);
			Step();		// pryc se jmenem
			assert(look == synEndExpression);
			Step();		// pryc se synEndExpression
		} else
		{
			if(kind == kwSafe)
				throw CComError(errBadParameterFormat,noteExpectedSafeFormat);
			kind = kwDirect;
			name[0] = '\0';
		}
		DoExpression();
		type = res.type;
		pointer = res.pointer;
		es.Load1();				// aby se mi tam nehromadily vysledky jednotlivych parametru
}

void CComSemantic::DoOp(int op)
{
	CComOBStructure* s;
	CComOBBaseData* d;
	const char* idName;
	int id, type;

	int arity = GetOpArity(op);

	switch(arity)
	{
	case 1:		es.Load1();		break;
	case 2:		es.Load2();		break;
	case 3:		es.Load3();		break;
	default:
		assert(false);
	}

	if(es.e1.type == cUnknown)
		if(IsKnownNameToken(es.e1.id))
			es.e1 = CComExpression(es.e1.id, true, false, false, kwName, 1);
		else
			if(op != opMember4Dot) 
				throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(es.e1.id), es.e1.pos);

	if(es.e2.type == cUnknown && arity >= 2)
		if(IsKnownNameToken(es.e2.id))
			es.e2 = CComExpression(es.e2.id, true, false, false, kwName, 1);
		else
			if(op!=opMemberDot && op!=opMemberArrow && op != opMember4Dot && op!= opAssign)
				throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(es.e2.id), es.e2.pos);

	if(es.e3.type == cUnknown && arity == 3)
		if(IsKnownNameToken(es.e3.id))
			es.e3 = CComExpression(es.e3.id, true, false, false, kwName, 1);
		else
			throw CComError(errUnknownIdentifier, noteNone, GetTokenStr(es.e3.id), es.e3.pos);

	switch(op)
	{
	// binarni, char/int/double/pointer&int
	case opPlus:
		// data + data?
		if(Convert(&es.e1, kwDouble, 0) != cUnknown &&
		   Convert(&es.e2, kwDouble, 0) != cUnknown)
			Result2();				
		else
			// pointer + int? 
			if(es.e1.pointer && IsGoodOldPointer(&es.e1) && Convert(&es.e2, kwInt, 0) != cUnknown)
				Result2()->isLValue = true;			
			else
				// int + pointer?
				if(es.e2.pointer && IsGoodOldPointer(&es.e1) && Convert(&es.e1, kwInt, 0) != cUnknown)
				{
					es.e3 = es.e1;		// prohodim operandy
					es.e1 = es.e2;
					es.e2 = es.e3;
					Result2()->isLValue = true;
				} else
					if(IsStringType(es.e1.type) && IsStringType(es.e2.type) && !es.e1.pointer && !es.e2.pointer)
						Result2(kwStringLast);
					else
						InvalidTypes(noteCheckOpSemantic);
		break;

	// binarni, char/int/double/pointer&int/pointer&pointer 
	case opMinus:	
		// data - data?
		if(Convert(&es.e1, kwDouble, 0) != cUnknown &&
		   Convert(&es.e2, kwDouble, 0) != cUnknown)
			Result2();				
		else
			// pointer - int? 
			if(es.e1.pointer && IsGoodOldPointer(&es.e1) && Convert(&es.e2, kwInt, 0) != cUnknown)
				Result2()->isLValue = true;
			else
				// pointer - pointer?
				if(es.e1.pointer && IsGoodOldPointer(&es.e1) && es.e2.pointer==es.e1.pointer && es.e2.type==es.e1.type)
				{
					Result2(kwInt)->isLValue = true;
					resPtr->pointer = 0;
				} else
					InvalidTypes(noteCheckOpSemantic);
		break;
	
	// binarni, char/int/double
	case opMultiply:	case opDivide:	
		// data * data?
		if(Convert(&es.e1, kwDouble, 0) != cUnknown &&
		   Convert(&es.e2, kwDouble, 0) != cUnknown)
			Result2();
		else
			InvalidTypes(noteCheckOpSemantic);
		break;
	
	// binarni, char/int
	case opModulo:	
	case opLeftShift:	case opRightShift:
	case opBitAnd:		case opBitOr:		case opBitXor:
		// intdata % intdata
		if(Convert(&es.e1, kwInt, 0) != cUnknown &&
		   Convert(&es.e2, kwInt, 0) != cUnknown)
			Result2();
		else
			InvalidTypes(noteCheckOpSemantic);
		break;
		
	// binarni, cokoliv, result->int
	case opLogicAnd:	case opLogicOr:
		Result2(kwInt);
		break;

	// binarni, cokoliv, ale oboji stejne, result->int
	case opLess:		case opLessEqual:	case opGreater:		case opGreaterEqual:	
	case opEqual:		case opNotEqual:
		if((type = Convert(&es.e2, es.e1.type, es.e1.pointer)) == cUnknown)
			if((type = Convert(&es.e1, es.e2.type, es.e2.pointer)) == cUnknown)
				InvalidTypes(noteCheckOpSemantic);

		if(IsStringType(es.e1.type) && !es.e1.pointer && es.e1.id == cUnknown ||
			IsStringType(es.e2.type) && !es.e2.pointer && es.e2.id == cUnknown)
		   InvalidTypes(noteMustBeLValue); 

		Result2(kwInt);
		break;

	// binarni, cokoliv, ale oboji stejne, levy lvalue
	case opAssign:		
		if(!es.e1.isLValue)
			InvalidTypes(noteMustBeLValue);

		if(Convert(&es.e2, es.e1.type, es.e1.pointer) != cUnknown)
		{
			bool test;
			if(es.e1.array.GetCount() == es.e2.array.GetCount())
			{
				test = true;
				for(int i=0; i<es.e1.array.GetCount(); i++)
					if(es.e1.array[i] != es.e2.array[i])
						test = false;
			} else 
				test = false;

			if(test)
				Result2();
			else
				InvalidTypes(noteNone);
		} else
			InvalidTypes(noteNone);
				
		break;
	
	// unarni, char/int/double/ptr (=cokoliv krome name, stringu a objektu), lvalue
	case opPreIncrement:	case opPreDecrement:
	case opPostIncrement:	case opPostDecrement:
		if(!es.e1.isLValue)
			InvalidTypes(noteMustBeLValue);

		if(Convert(&es.e1, kwDouble, 0) != cUnknown || IsGoodOldPointer(&es.e1))
			Result1()->hasSideEffect = true;
		else
			InvalidTypes(noteCheckOpSemantic);
		break;

	// unarni, char/int
	case opBitNot:
		if(Convert(&es.e1, kwInt, 0) != cUnknown)
			Result1();
		else
			InvalidTypes(noteCheckOpSemantic);
		break;

	// unarni, cokoliv 
	case opLogicNot:
		Result1();
		break;
	
	// unarni, lvalue, result->pointer++
	case opAddress:
		if(!es.e1.isLValue)
			InvalidTypes(noteMustBeLValue);

		Result1()->pointer++;
		break;

	// unarni, pointer, result->pointer--,lvalue
	case opIndirection:
		if(IsGoodOldPointer(&es.e1) && es.e1.type!=kwVoid)
		{
			Result1()->pointer--;
			resPtr->isLValue = true;
		} else
			InvalidTypes(es.e1.pointer ? noteCannotDereferenceVoid : noteNone);
		break;
	
	// binarni, cokoliv, vyhodnoti se prvni, pak druhy a vysledek je ten druhy
	case opComma:
		es.Push(es.e2);
		resPtr = &(es.expStack.el->data);
		break;

	// unarni, char/int/double
	case opUnaryPlus:	case opUnaryMinus:
		if(Convert(&es.e1, kwDouble, 0) != cUnknown)
			Result1();
		else
			InvalidTypes(noteCheckOpSemantic);
		break;

	// ternarni, pozor - mam nahrany jen prvni operand!
	case opConditional:
		// prvni muze byt cokoliv - mam ho v es.e3
		// dalsi dva operandy: cokoliv, ale oba kompatibilni
		type = Convert(&es.e1, es.e2.type, es.e2.pointer);
		if(type == cUnknown)
			type = Convert(&es.e2, es.e1.type, es.e1.pointer);
		if(type != cUnknown)
			Result2(type);
		else
			InvalidTypes(noteCheckOpSemantic);
		break;

	// binarni, index musi byt int, indexovana vec musi byt pointer 
	case opBracketStart:
		// es.e1 je indexovana promenna, es.e2 je index
		if(Convert(es.e2.type, es.e2.pointer, kwInt, 0) == cUnknown) 
			throw CComError(errInvalidIndexType, noteNone);

		if(!es.e1.pointer && !IsStringType(es.e1.type) || IsObjectType(es.e1.type) && es.e1.pointer == 1)
			InvalidTypes(noteNone);

		if(IsDynamicArrayToken(es.e1.type) && es.e1.pointer == 1)
		{
			Result2()->type--;					// typ dynamickeho pole nasleduje vzdy za prislusnym typem
			resPtr->pointer = es.e1.type==kwObjptrArray || es.e1.type==kwNameArray || es.e1.type==kwVoidArray ? 1 : 0;
		} else
			if(IsStringType(es.e1.type) && !es.e1.pointer)
				Result2(kwChar);
			else {
				Result2()->pointer--;
				if(resPtr->array.GetCount())
					resPtr->array.RemoveHead();
			}
		resPtr->isLValue = true;
		break;

	case opMemberArrow:
		if(es.e1.pointer && !es.e1.array.GetCount())
			es.e1.pointer--;
		else
			InvalidTypes(noteNone);
		// bez breaku - pokracuju dal jako opMemberDot
	case opMemberDot:
		if(es.e1.pointer || es.e1.array.GetCount())
			InvalidTypes(noteNone);

		if(!IsStructureToken(es.e1.type))
			InvalidTypes(noteNone);

		s = (CComOBStructure*) sTab->Get(es.e1.type);

		idName = GetTab(es.e2.id)->Get(es.e2.id)->name;
		if(!IsUnknownToken(es.e2.id))
			QueueTightSubstitute(es.e2.id, uTab->Find(idName));

		if((id = s->mTab->Find(idName)) == cUnknown)
			throw CComError(errCannotUseOperator, noteHasntSuchMember);
		
		d = (CComOBBaseData*) s->mTab->Get(id);
		
		es.Push(res = CComExpression(id, false, true, false, d->dType, d->pointer));
		return;

	default:
		throw CComError(errFeatureNotSupported,noteNone,GetTokenStr(op));
	}
	
	res = *resPtr;
}

CComExpression* CComSemantic::Result1()
{
	es.Push(es.e1);
	resPtr = &(es.expStack.el->data);

	resPtr->isLValue = false;
	return resPtr;
}

CComExpression* CComSemantic::Result2(int setType,bool doConversions)
{
	es.Push(es.e1);	// defaultne ulozim es.e1

	resPtr = &(es.expStack.el->data);

	resPtr->isConstant		= es.e1.isConstant && es.e2.isConstant;
	resPtr->hasSideEffect	= es.e1.hasSideEffect || es.e2.hasSideEffect;
	resPtr->isLValue		= false;
	
	if(resPtr->id != cUnknown)
	{
		if(es.e2.id != cUnknown)
			resPtr->id = cUnknown;
	} else
		resPtr->id = es.e2.id;
	
	/* Implicitni konverze:
		A) data:		char->int->double
		B) pointer:		typed*->void*
		C) objptr:		typed obj->objptr	*/

	if(setType != cUnknown)			// vim co dat za typ?
		resPtr->type = setType;
	else
		if(doConversions)			// kdyz ne, tak zkusim implicitni konverze
			if(!es.e1.pointer && !es.e2.pointer)
				// char->int->double
				if(es.e2.type==kwDouble && (es.e1.type==kwInt || es.e1.type==kwChar))	
					resPtr->type=kwDouble;
				else
					// char->int
					if(es.e2.type==kwInt && es.e1.type==kwChar)	
						resPtr->type=kwInt;
					else
						// typed*->void*
						if(es.e2.type==kwVoid && (es.e1.type==kwChar || es.e1.type==kwInt || es.e1.type==kwDouble ) && es.e1.pointer==es.e2.pointer)
							resPtr->type=kwVoid;
						else
							// typed obj->objptr
							if(es.e2.type==kwObjptr && es.e1.type>tokObjectFirst && es.e1.type<tokObjectLast)
								resPtr->type=kwObjptr;
	return resPtr;
}

void CComSemantic::InvalidTypes(int note)
{
	char buf[2*cMaxIdentifierLn+10];
	strcpy(buf,cOperators[look-opFirst-1]);
	strcat(buf," ");
	int ar = GetOpArity(look);
	char* typeStr;
	for(int i=0; i<ar; i++)
	{
		if(exps[i]->type != cUnknown)
			typeStr = GetTypeStr(exps[i]->type,exps[i]->pointer);
		else
			typeStr = newstrdup("unknown");
		assert(typeStr);
		strcat(buf,typeStr);
		SAFE_DELETE_ARRAY(typeStr);
		if(i<ar-1)
			strcat(buf,", ");
	}
	throw CComError(errCannotUseOperator,note,buf);
}

int  CComSemantic::Convert(int srcType, int srcPointer, int targetType, int targetPointer)
{
	if(srcPointer != targetPointer)
		if(IsStringType(srcType) && targetType==kwChar && srcPointer == 0 && targetPointer==1)
			return kwChar;
		else
			if(srcType==kwChar && IsStringType(targetType) && srcPointer == 1 && targetPointer==0)
				return kwString;
			else
				return cUnknown;

	if(srcType == targetType)
		return srcType;

	if(IsStringType(srcType) && IsStringType(targetType))
		return kwString;

	if(srcPointer==0)
	{
		if(targetType == kwDouble && IsIntDataType(srcType))
			return kwDouble;

		if(targetType==kwInt && srcType==kwChar || targetType==kwChar && srcType==kwInt)
			return kwInt;
	} else
		if(srcType == kwVoid)
			return kwVoid;
		else
			if(srcPointer == 1)
				if(IsObjectType(srcType) && IsObjectType(targetType))
					return kwObjptr;
				else
					if(IsNameType(srcType) && IsNameType(targetType))
						return kwName;

	return cUnknown;
}

int CComSemantic::KSConvert(CComExpression* src, int targetType, int targetPointer)
{
	int ret = Convert(src, targetType, targetPointer);

	if(ret == cUnknown)
		if(targetType == kwInt && targetPointer == 0)
			return kwInt;

	return ret;
}

void CComSemantic::DoPostPass()
{
	CComOBObject* o;
	CComOBMethod* m;
	CComOBMethod* m2;
	CComOBAttribute *a, *a1;
	int i, j, index;

	if(!postPassInput && objBrowser->generatingCode)
	{
		assert(look == lexEOF);
		parent->ReleaseOutput();
		FS->ChangeDir("$COMPFILES$");
		postPassInput = fopen(cSyntaxOutputFile,"rb");
		assert(postPassInput);
		postPassCache = new CComFileCache(postPassInput, true);
		assert(postPassCache);
	}

	if(postPassInput)
	{
		// Preklad zdedenych metod
		for(i=0; i<oTab->GetCount(); i++)		// pres vsechny objekty:
		{
			o = objBrowser->GetObject(i);		

			for(j=0; j<o->mTab->GetCount(); j++)	
			{
				m = o->GetMethod(j);		

				if(m->mem == cUnknown)					// zdedena metoda - musim jeste prelozit
					CompileInheritedMethod(o, i + tokObjectFirst, m);
			}
		}

		fclose(postPassInput);
		postPassInput = 0;
	}

	char buf[2048];
	for(i=0; i<oTab->GetCount(); i++)		// pres vsechny objekty:
	{
		o = objBrowser->GetObject(i);		
		if(o->aTab->GetCount())
		{
			int ksvgID;
			CKSKSVG *bestKSVG, *pKSVG;
			int bestFit, fit;

			ksvgID = 0;
			bestKSVG = 0;
			bestFit = cUnknown;
			while(ksvgID < KS_NUM_VG)
			{
				sprintf(buf, "_KSVG_%d_%s_%s", ksvgID, o->name, uTab->Get(o->version)->name);
				if(pKSVG = (CKSKSVG*)KSMain->KSVGs->Member(buf))
				{
					fit = o->FitInKSVG(pKSVG);
					if(fit > bestFit || fit == bestFit && pKSVG->Size < bestKSVG->Size)
					{
						bestFit = fit;
						bestKSVG = pKSVG;
					}
				}
				ksvgID++;
			} 

			o->SetKSVG(bestKSVG);
		}
	}

	// Urceni "kompilovatelnosti" metod a atributu
	for(i=0; i<oTab->GetCount(); i++)		// pres vsechny objekty:
	{
		o = objBrowser->GetObject(i);		

		// kontrola spolecnych dat
		for(j=0; j<o->mTab->GetCount(); j++)	
		{
			m = o->GetMethod(j);		

			// projdu pouzivane atributy
			CListKElem<int>* p = m->usedAttributes.el;
			while(p)
			{
				index = p->data - tokAttributeFirst;
				assert(index < o->aTab->GetCount());
				a = o->GetAttribute(index);
				
				if(m->compiled != a->compiled)	// metoda a atribut v ruznych svetech?
				{
					m->SetCompiled(a, false);	// -> oba dam do IS
					a->SetCompiled(m, false);
				}
				p = p->next;
			}	// atributy
		}		// metody

		// kontrola nebezbecnych volani
		for(j=0; j<o->mTab->GetCount(); j++)	
		{
			m = o->GetMethod(j);		

			// projdu seznam nebezpecnych volani
			CListKElem<CComOBMethod*>* p = m->dangerousCalls.el;
			while(p)
			{
				m2 = p->data;
				
				if(m->compiled != m2->compiled)	// metody v ruznych svetech?
				{
					m->SetCompiled(0, false);	// -> oba dam do IS
					m2->SetCompiled(0, false);
				}
				p = p->next;
			}	// atributy
		}		// metody

		
	}	// objekty

	// Doplneni zdedenych tagu (to co se nemohlo zdedit v synt. analyze):
	assert(inhAttrList);

	while(!inhAttrList->IsEmpty())				// nacitam dvojice: dedeny (stary) a zdedeny (novy)
	{
		a1 = inhAttrList->RemoveHead();			// stary atribut (ten, ktery se zdedil)
		assert(!inhAttrList->IsEmpty());
		a = inhAttrList->RemoveHead();			// novy (zdedeny) atribut
		assert(a && a1);

		a->edit = a1->edit;
		if(a1->dType == kwScripted)
		{
			a->knownName = a1->knownName;
			a->kerNameID = a1->kerNameID;
		}
	}
}

void CComSemantic::CompileInheritedMethod(CComOBObject* o, int objID, CComOBMethod* m)
{
	CComOBMethod* oldMet;
	int id;

	id = m->objDef->mTab->FindFirst(m->name);
	assert(id != cUnknown);
	while(id != cUnknown)
	{
		oldMet = (CComOBMethod*)m->objDef->mTab->Get(id);

		if(!strcmp(oldMet->kerKonkretniID, m->kerKonkretniID))
			break;
		
		id = m->objDef->mTab->FindNext(m->name);
	}

	postPassCache->Clear();
	fseek(postPassInput, oldMet->mem * sizeof(int), SEEK_SET);
	Step(false);
	inObj = o;
	inObjID = objID;
	inMethod = m;
	
	Put(synStartObjectDecl, false);
	Put(inObjID, false);
	Put(synMethodDecl, false);
	Put(inMethod->id, false);
	Put(semMethodBody, false);

	DoMethodBody();

	Put(synEndObjectDecl, false);
}

void CComSemantic::DoDataEditTags(CComOBBaseData* dat)
{
	int type, index;
	UComValues tmp;

	assert(look == kwEdit);
	Step();
	assert(IsDataToken(look)); 
	type = look - tokDataFirst;
	Step();

	switch(type)
	{
	case etDefaultValue:
	case etList:
		assert(IsDataToken(look));
		index = look - tokDataFirst;
		Step();
		// no break...

	case etAuto:
	case etIs:
		{
			DoExpression();
			if(!IsNameType(res.type) || res.pointer != 1)
				throw CComError(errExpectedName, noteNone);
			assert(IsNameToken(res.id));
		}
		break;
		
	default: 
		assert(false);
	}

	switch(type)
	{
	case etAuto:
		dat->edit.nameAuto = res.id;
		break;

	case etDefaultValue:
		tmp.intData = res.id - tokNameFirst;
		dat->edit.defaultValue[index] = tmp;
		dat->edit.valueType = kwInt;
		break;

	case etList:
		tmp.intData = res.id - tokNameFirst;
		dat->edit.list[index] = tmp;
		break;
	
	case etIs:
		dat->edit.isName.intData = res.id - tokNameFirst;
		break;

	default:
		assert(false);
	}	
}

void CComSemantic::QueueTightSubstitute(int find, int replace)
{
	int i = queueUp;
	
	do
		i = (i-1) % queueLn;
	while(queue[i] != find && i != queueDn);

	assert(queue[i] == find);
	queue[i] = replace;
}

void CComSemantic::QueueBackAndForwardSubstitute(int findBack, int nestedCalls, int findForward, int replace, int extraStepsBack)
{
	int i = queueUp;

	while(nestedCalls--)
	{
		do
			i = (i-1) % queueLn;
		while(queue[i] != findBack && i != queueDn);
	}
	assert(queue[i] == findBack);

	while(extraStepsBack)
	{
		i = (i-1) % queueLn;
		if(queue[i] > tokMetaLast)
			extraStepsBack--;
	}

	while(queue[i] != findForward && i != queueDn)
		i = (i+1) % queueLn;

	assert(queue[i] == findForward);
	queue[i] = replace;
}
