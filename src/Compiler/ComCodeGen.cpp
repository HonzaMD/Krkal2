/////////////////////////////////////////////////////////////////////////////
//
// ComCodeGen.cpp
//
// Implementace CComCodeGen - trida pro generovani kodu pro interpret
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComCodeGen.h"
#include "ComKerServices.h"
#include "fs.h"

#include "ident.h"

CComCodeGen::CComCodeGen(CComObjectBrowser* ob, int _queueSize) : CComPart(ob,_queueSize)
{
	look = old = cUnknown;
	code = &code1;
	breakJumpList = &defaultBJL;
	continueJumpList = &defaultCJL;
	contJump = cUnknown;
	
	SetToConstant(&expConst0, 0);

	tmpData = new CComOBBaseData("tmp", cUnknown, CComShortPosition());
	assert(tmpData);
}

CComCodeGen::~CComCodeGen()
{
	assert(breakJumpList == &defaultBJL &&  breakJumpList->IsEmpty());
	assert(continueJumpList == &defaultCJL && continueJumpList->IsEmpty());
	SAFE_DELETE(tmpData);
}

void CComCodeGen::DoPart()
{
	int i, j;
	CComOBObject* o;
	CComOBMethod* m;

	if(look == cUnknown)
	{
		Step(false);
		for(i = 0; i < sTab->GetCount(); i++)
			((CComOBStructure*)sTab->Get(tokStructureFirst+i))->ComputeMembers();
		
		#ifdef JIRKA_KING_OF_COMPILERS
		FS->ChangeDir("$COMPFILES$");
		FILE* f1 = fopen("MethodListOK.txt","wt");
		FILE* f2 = fopen("MethodListLeft.txt","wt");
		#endif

		for(i = 0; i < oTab->GetCount(); i++)
		{
			o  = objBrowser->GetObject(i);
			for(j = 0; j < o->mTab->GetCount(); j++)
			{
				m = o->GetMethod(j);
				m->mem = cUnknown;
				
				#ifdef JIRKA_KING_OF_COMPILERS
				fprintf(m->compiled ? f2 : f1, "%s::%s:\t\t\t\t%s\n",o->name,m->name, m->kerKonkretniID);
				#endif	
			}
		}
		
		#ifdef JIRKA_KING_OF_COMPILERS
		fclose(f1);
		fclose(f2);
		#endif
	}

	while(look != lexEOF)
        DoTopDecl();

	Put(lexEOF);
	DoPostPass();

#ifdef JIRKA_KING_OF_COMPILERS
	FS->ChangeDir("$COMPFILES$");
	DebugCode("code.dbg");
#endif
}

void CComCodeGen::Step(bool put)
{ 
	if(look == lexEOF)
		throw CComError(errUnexpectedEOF);
	old = look; 
	if(put)
		Put(look);

	look = GetParent(); 
}

void CComCodeGen::DoTopDecl()
{
	switch(look)
	{
	case synStartObjectDecl:
		DoObjectDecl();
		break;

	default:
		Step();
	}
}

void CComCodeGen::DoObjectDecl()
{
	Step();				// pryc s synStartObjectDecl
	assert(IsObjectToken(look));
	inObj = (CComOBObject*)oTab->Get(look);
	inObjID = look;
	Step();				// pryc s id objektu
	
	inObj->ComputeAttributes();

	while(look != synEndObjectDecl)
	{
		switch(look)
		{
		case synMethodDecl:
			DoMethodDecl();
			break;
		
		default:
			Step(false);
		}
	}
	inObj = 0;
	inObjID = cUnknown;
}

void CComCodeGen::DoMethodDecl()
{
	int toWriteStackAllocation, i;

	assert(inObj);
	Step();				// pryc s synMethodDecl
	inMethod = (CComOBMethod*)inObj->mTab->Get(look);
	
	while(look != semMethodBody)
		Step();				
	Step();				// pryc se semMethodBody

	if(!inMethod->compiled)
	{
		// inicializece generatoru pred zpracovanim dalsi metody:
		blockDepth = currentExp = totalBlockNr = 0;
		contJump = cUnknown;
		breakJumpList->DeleteAll();
		inMethod->ComputeLocals();
		inMethod->mem = GetCurrentAddr();	// vstupni bod metody
		debugMethod.Add(inMethod);

		toWriteStackAllocation = Instruction(instrRSS);

		maxStackTop = stackBottom = inMethod->localsPos + inMethod->localsSize;

		codeStart = code->GetCount();

		DoCommand();						// telo metody

		i = maxStackTop - stackBottom + inMethod->localsSize;
		if(i)
		{
			LinkAddress(toWriteStackAllocation, eLkTpInt2, i);
			LinkAddress(toWriteStackAllocation, eLkTpInt3, inMethod->localsSize);
		} else
			code->Remove(toWriteStackAllocation);

		if(inMethod->storage==kwDirect)
			Instruction(instrDRET);
		else
			Instruction(instrSRET);

		assert(blockDepth == 0);
	} else
	{
		// Kompilovana -> ignorovat telo metody
		assert(look == synStartBlock);
		blockDepth = 0;
		do
		{
			if(look == synStartBlock)
				blockDepth++;
			else
				if(look == synEndBlock)
					blockDepth--;
			Step();
		} while(blockDepth > 0);
		
	}
	inMethod = 0;
}


void CComCodeGen::DoCommand()
{
	int oldBlock;
	int oldContinueAddr = contJump;

	switch(look)
	{
	case synStartBlock:
		oldBlock = currentBlock;
		blockDepth++;
		totalBlockNr++;
		currentBlock = totalBlockNr;
		Step();			// pryc s synStartBlock
		while(look != synEndBlock)
			DoCommand();
		blockDepth--;
		currentBlock = oldBlock;
		Step();			// pryc s synEndBlock
		break;
	case synStartExpression:
		currentExp++;		// jedu dalsi expression...
		DoExpressionMain();
		break;

	case synLocalDefinition:
		Step();				// pryc se synLocalDefinition
		Step();				// pryc s id lokalni promenne
		Step();				// pryc s cislem radku
		break;
	
	case lexSeparator:
		Step();
		break;
	
	case kwIf:			DoIf();			break;
	case kwWhile:		DoWhile();		break;
	case kwDo:			DoDo();			break;
	case kwFor:			DoFor();		break;
	case kwReturn:		DoReturn();		break;
	case kwSwitch:		DoSwitch();		break;
	case kwBreak:		DoBreak();		break;
	case kwContinue:	DoContinue();	break;
	default:
		assert(false);
	}
	
	contJump = oldContinueAddr;
}

void CComCodeGen::DoIf()
{
	int toWriteCondFail, condFailAddr, toWriteOverJump;
	CIntInstructionRec instr;

/*	schema prosteho if:							schema if-else

	if(exp)			<exp>						if(exp)			<exp>
					JMPZ	fail								JMPZ fail
		cmd			<cmd>							cmd			<cmd>
					fail:										JMP over
												else			fail:
													cmd			<cmd>
																over:
*/
	assert(look == kwIf);
	Step();			// pryc s kwIf
	DoExpressionMain();
	
	toWriteCondFail = Instruction(instrJMPZ, 0, &resExp);		// jeste neznam adresu else bloku (*)
	
	DoCommand();						// Generuji kod prikazu za if

	condFailAddr = GetCurrentAddr();	// poznamenam si adresu za blokem - pokud nenasleduje else, skacu sem, jinak tady ale bude jeste skok pres else blok
	if(look == kwElse)
	{
		condFailAddr += cInstructionSize;		// musim jeste pridat JMP over instrukci
		toWriteOverJump = Instruction(instrJMP);				// (**)
		Step();							// pryc s else
		DoCommand();
		SetJumpAddress(toWriteOverJump, GetCurrentAddr());	// doplnim adresu (**)
	}
	SetJumpAddress(toWriteCondFail, condFailAddr);			// doplnim adresu (*)
}

void CComCodeGen::DoWhile()
{
	
	int toWriteCondFail, condTestAddr, toWriteLoop;
	int lastContJump;
	CListK<CComLinkElem> breakList, *lastBreakList;

	assert(look == kwWhile);
	Step();			// pryc s kwWhile
	
	lastContJump = contJump;
	contJump = condTestAddr = GetCurrentAddr();	// adresa podminky cyklu while
	
	DoExpressionMain();

	toWriteCondFail = Instruction(instrJMPZ, 0, &resExp);	// jeste neznam adresu za blokem while (*)

	lastBreakList = breakJumpList;
	breakJumpList = &breakList;

	DoCommand();						// Generuji kod prikazu za while

	toWriteLoop = Instruction(instrJMP);				// skok na opetovne vyhodnoceni vyrazu
	SetJumpAddress(toWriteLoop, condTestAddr);

	SetJumpAddress(toWriteCondFail, GetCurrentAddr());	// doplnim adresu (*)
	
	LinkJumpList(&breakList, GetCurrentAddr());				// doplnim vsechny breaky
	breakJumpList = lastBreakList;

	contJump = lastContJump;
}

void CComCodeGen::DoDo()
{
	int doStartAddr, lastContJump, toWriteLoop;
	CListK<CComLinkElem> breakList, *lastBreakList;

	assert(look == kwDo);
	Step();			// pryc s kwDo

	lastContJump = contJump;
	contJump = doStartAddr = GetCurrentAddr();			// adresa zacatku cyklu do
	
	lastBreakList = breakJumpList;
	breakJumpList = &breakList;

	DoCommand();

	assert(look == kwWhile);
	Step();			// pryc s kwWhile
	DoExpressionMain();

	toWriteLoop = Instruction(instrJMPNZ, 0, &resExp);
	SetJumpAddress(toWriteLoop, doStartAddr);			// doplnim adresu

	LinkJumpList(&breakList, GetCurrentAddr());				// doplnim vsechny breaky
	breakJumpList = lastBreakList;

	contJump = lastContJump;
}

void CComCodeGen::DoFor()
{
	CDynamicArrayV<CIntInstructionRec> code2;
	int condAddr, toWriteOverFor, lastContJump, toWriteLoop;
	CListK<CComLinkElem> breakList, *lastBreakList, contList, *lastContList;

	assert(look == kwFor);
	Step();			// pryc s kwFor

	DoExpressionMain();						// init expression
	
	condAddr = GetCurrentAddr();
	DoExpressionMain();						// cond expression

	toWriteOverFor = Instruction(instrJMPZ, 0, &resExp); // adresu skoku (za for) jeste neznam (*)

	SetAlternativeCodeOutput(&code2);
	DoExpressionMain();						// iterator expression - necham ho prelozit do code2
	SetNativeCodeOutput();

	lastBreakList = breakJumpList;
	breakJumpList = &breakList;

	lastContJump = contJump;
	contJump = cUnknown;
	lastContList = continueJumpList;
	continueJumpList= &contList;

	DoCommand();							// nyni prekladam kod prikazu for

	LinkJumpList(&contList, GetCurrentAddr());
	InsertCodeFragment(&code2);				// vlozim kod iterator expression

	toWriteLoop = Instruction(instrJMP);	// skok na zacatek for (nove testovani podminky)
	SetJumpAddress(toWriteLoop, condAddr);
	
	SetJumpAddress(toWriteOverFor, GetCurrentAddr());	// doplnim adresu skoku za for (*)
	LinkJumpList(&breakList, GetCurrentAddr());				// doplnim vsechny breaky
	breakJumpList = lastBreakList;
	continueJumpList = lastContList;
	contJump = lastContJump;
}

void CComCodeGen::DoReturn()
{
	bool returns;

	assert(look==kwReturn);
	Step();				// pryc s kwReturn
	if(returns = look != kwVoid)
	{
		DoExpressionMain();
	} else
		Step();			// pryc s kwVoid

	if(inMethod->storage == kwDirect)
		Instruction(instrDRET, returns ? &resExp : 0);
	else
		Instruction(instrSRET, returns ? &resExp : 0);
}

void CComCodeGen::DoSwitch()
{
	int count, i, toWriteCount, instrCount, toWriteDefault;
	int* tab;
	CDynamicArrayV<int> code2;
	CListK<CComLinkElem> breakList, *lastBreakList;

	assert(look == kwSwitch);
	Step();			// pryc s kwSwitch
	assert(IsDataToken(look));
	count = look - tokDataFirst;
	Step();			// pryc s poctem case navesti

	lastBreakList = breakJumpList;
	breakJumpList = &breakList;

	DoExpressionMain();
	
	CComExpression expCount, expSkip;
	SetToConstant(&expCount, count);
	SetToConstant(&expSkip, count/2 + count%2 + 1);

	toWriteCount = Instruction(instrSWITCH, &expCount, &resExp, &expSkip);

	instrCount = count/2 + count%2;
	tab = new int[instrCount];					// adresa zacatku tabulky skoku switche
	assert(tab);
	for(i=0; i < instrCount; i++)
		tab[i] = Instruction(0);				// zatim vyplnena 0 - dve hodnoty a dve adresy skoku

	toWriteDefault = Instruction(0);			// adresa default skoku

	i = 0;
	while(look == kwCase)
	{
		LinkAddress(tab[i/2], i%2 ? eLkTpInt4 : eLkTpInt3, GetCurrentAddr());
		Step();		// pryc s kwCase
		
		assert(look == synStartExpression);
		Step();		// pryc s synStartExpression
		Step();		// pryc s cislem radku
		assert(IsConstantToken(look));
		CComOBConstant* c = (CComOBConstant*)cTab->Get(look);
		assert(c && c->dType == kwInt);
		Step();
		assert(look == synEndExpression);
		Step();
		LinkAddress(tab[i/2], i%2 ? eLkTpInt2 : eLkTpInt1, c->value.intData);

		while(look != synEndCase)
			DoCommand();
		Step();		// pryc se synEndCase
		i++;
	}
	
	assert(i == count);
	LinkAddress(toWriteDefault, eLkTpInt1, GetCurrentAddr());	// adresa navesti default 

	if(look == kwDefault)
	{
		Step();		// pryc s kwDefault		
		while(look != synEndCase)
			DoCommand();
		Step();		// pryc se synEndCase
	}
	
	LinkJumpList(&breakList, GetCurrentAddr());				// doplnim vsechny breaky
	breakJumpList = lastBreakList;

	SAFE_DELETE_ARRAY(tab);
}

void CComCodeGen::LinkJumpList(CListK<CComLinkElem>* list, int toAddr)
{
	CListKElem<CComLinkElem>* p = list->el;
	while(p)
	{
		SetJumpAddress(p->data.addr, toAddr);
		p = p->next;
	}
	list->DeleteAll();
}

void CComCodeGen::DoBreak()
{
	int toWriteBreak;

	assert(look == kwBreak);
	Step();

	toWriteBreak = Instruction(instrJMP);
	breakJumpList->Add(CComLinkElem(toWriteBreak, 0));
}

void CComCodeGen::DoContinue()
{
	int toWriteCont;

	assert(look == kwContinue);
	Step();

	toWriteCont = Instruction(instrJMP);
	if(contJump == cUnknown)
        continueJumpList->Add(CComLinkElem(toWriteCont, 0));
	else
		SetJumpAddress(toWriteCont, contJump);
}


void CComCodeGen::DoNew()
{
	int type, line, pointer;
	CListKElem<CComExpression>* stackPos;

	CComExpression exp, exp2, realTarget, expLine;

	assert(look == kwNew);
	Step();			// pryc s new
	line = look-tokDataFirst;
	Step();			// pryc s cislem radku

	if(look != kwVarType)
	{
		type = look;
		Step();			// pryc s typem
		assert(IsDataToken(look));
		pointer = look - tokDataFirst;
		Step();			// pryc s pointerem
	} else
	{
		type = kwObjptr;
		pointer = 1;
		Step();

		stackPos = es.GetPos();
		DoExpression();
		es.Load1();
		exp = es.e1;						// vartype do exp
		es.RestorePos(stackPos);
	}

	CComExpression stackTmp = AllocateStack(type, pointer);
	SetToConstant(&expLine, line);

	if(look == opBracketStart)
	{
		Step();								// pryc s hranatou zavorkou

		stackPos = es.GetPos();
		DoExpression();
		es.Load1();
		es.RestorePos(stackPos);

		exp = es.e1;				
		SetToConstantT(&exp2, type, pointer);
		
		realTarget = stackTmp;
		if(IsConstantToken(exp.id))
			DoConstantOptimalization(&stackTmp, &exp, &exp2, opMultiply, 2, kwInt);
		else
			Instruction(instrMUL, &stackTmp, &exp, &exp2);
		Instruction(instrNEW, &realTarget, &stackTmp, &expLine);	
		stackTmp = realTarget;

		pointer++;
	} else
		if(pointer == 1)
		{
			if(IsObjectType(type))
			{
				if(type != kwObjptr)			// kdyz je to vartype, tak uz mam jmeno v exp
				{
					CComOBObject* o = (CComOBObject*)oTab->Get(type);
					SetToConstant(&exp, o->kerNameID - tokNameFirst);
					Instruction(instrNEWO, &stackTmp, &exp, &expLine);	// exp obsahuje ID objektu
				} else
					Instruction(instrNEWOV, &stackTmp, &exp, &expLine);	// exp obsahuje jmeno objektu
			} else
				if(IsDynamicArrayToken(type))
				{
					SetToConstant(&exp, type);
					Instruction(instrNEWA, &stackTmp, &exp);	// exp obsahuje typ pole (const)
				} else
				{
					StandardNew(&stackTmp, type, pointer, line);
					if(!IsNameType(type))
						pointer++;
				}
		} else
		{
			StandardNew(&stackTmp, type, pointer, line);
			pointer++;
		}
					
	stackTmp.pointer = pointer;

	es.Push(stackTmp);
}


void CComCodeGen::SetToConstant(CComExpression* exp, int intValue)
{
	int id;
	cTab->FindOrAddConstant(intValue, &id);
	*exp = CComExpression(id, true, false, false, kwInt, 0);
}

void CComCodeGen::SetToConstant(CComExpression* exp, double doubleValue)
{
	int id;
	cTab->FindOrAddConstant(doubleValue, &id);
	*exp = CComExpression(id, true, false, false, kwDouble, 0);
}

void CComCodeGen::SetToConstantT(CComExpression* exp, int type, int pointer)
{
	tmpData->SetData(cUnknown, type, pointer);
	SetToConstant(exp, tmpData->GetSize());		// exp obsahuje velikost
}

void CComCodeGen::StandardNew(CComExpression* target, int type, int pointer, int line)
{
	CComExpression exp, expLine;
	SetToConstantT(&exp, type, pointer);		// exp obsahuje velikost
	SetToConstant(&expLine, line);		
	Instruction(instrNEW, target, &exp, &expLine);	 
}

void CComCodeGen::DoDelete()
{
	CComExpression expLine, expType;

	assert(look == kwDelete);
	Step();			// pryc s delete
	assert(IsDataToken(look));

	SetToConstant(&expLine, look-tokDataFirst);
	Step();			// pryc s cislem radku
	es.Load1();

	if(es.e1.pointer == 1)
	{
		if(IsObjectType(es.e1.type))
			Instruction(instrDELO, &es.e1, &expLine);	
		else
			if(IsDynamicArrayToken(es.e1.type))
			{
				SetToConstant(&expType, es.e1.type);
				if(es.e1.array.GetCount())
					DoArrayAccess(&es.e1);
				Instruction(instrDELA, &es.e1, &expType);	
			} else
				Instruction(instrDEL, &es.e1, &expLine);	
	} else
		Instruction(instrDEL, &es.e1, &expLine);	

	Step();		// pryc s typem
	Step();		// pryc s pointerem

	es.Push(AllocateStack(kwVoid, 0));
}

void CComCodeGen::DoSender()
{
	assert(look == kwSender);
	Step();			// pryc se sender

	CComExpression stackTmp = AllocateStack(kwObjptr, 1);
	Instruction(instrSENDER, &stackTmp);

	es.Push(stackTmp);
}

void CComCodeGen::DoThis()
{
	assert(look == kwThis && inObjID != cUnknown);
	Step();			// pryc s this

	CComExpression stackTmp = AllocateStack(inObjID, 1);

	Instruction(instrTHIS, &stackTmp);

	es.Push(stackTmp);
}

void CComCodeGen::DoAssigned()
{
	int id;
	assert(look == kwAssigned);
	Step();			// pryc s assigned
	
	assert(IsDataToken(look));
	id = look - tokDataFirst;
	Step();		// pryc s argumentem

	CComExpression stackTmp = AllocateStack(kwInt, 0);

	CComExpression expIndex;
	SetToConstant(&expIndex, id);

	Instruction(instrASGND, &stackTmp, &expIndex);

	es.Push(stackTmp);
}

void CComCodeGen::DoTypeOf()
{
	CListKElem<CComExpression>* stackPos;

	assert(look == kwTypeOf);
	Step();			// pryc s typeof

	stackPos = es.GetPos();
	DoExpression();
	es.Load1();
	es.RestorePos(stackPos);
	
	CComExpression stackTmp = AllocateStack(kwName, 1);

	Instruction(instrTYPEOF, &stackTmp, &es.e1);

	es.Push(stackTmp);
}

void CComCodeGen::DoExpressionMain()
{
	currentExp++;
	currentStackTop = 0;
	assigning = addressing = false;
	optimizeAssign = true;
	doingParams = false;
	logicJumpList.DeleteAll();
	toPlaceParamList.DeleteAll();
	stackToPlaceParam = cUnknown;
	es.Reset();
	tmpExpStorage.RemoveAll();
	arrayAccess.RemoveAll();
	inputTokens = 0;

	DoExpression();
	
	if(resExp.array.GetCount())
		DoArrayAccess(&resExp);
	else
	{
		CComOBBaseData* d;

		if(resExp.id == cUnknown)
			d = 0;
		else
			d = (CComOBBaseData*)GetTab(resExp.id)->Get(resExp.id);

		if((d && d->array.GetCount()) || resExp.array.GetCount())
		{
			CComExpression result = AllocateStack(resExp.type, resExp.pointer);
			DoAddress(&result, &resExp);
			resExp = result;
		}
	}

}

void CComCodeGen::DoExpression(bool eval)
{
	bool wasOp;
	assert(look == synStartExpression);
	Step();			// pryc se synStartExpression
	Step();			// pryc s cislem radku
	
	while(look != synEndExpression)
	{
		inputTokens++;

		switch(look)
		{
		case synStartExpression:
			DoExpression();
			break;

		case synConditional1:
			conditionalList.Add(CComLinkElem(Instruction(instrJMPZ, 0, &(es.expStack.el->data)), es.expStack.el->data));
			Step();
			break;

		case synConditional2:
			conditionalList.Add(CComLinkElem(Instruction(instrJMP, 0, &(es.expStack.el->data)), es.expStack.el->data, GetCurrentAddr()+cInstructionSize));
			Step();
			break;

		case synLogicAnd:
			logicJumpList.Add(CComLinkElem(Instruction(instrJMPZ, 0, &(es.expStack.el->data)), es.expStack.el->data));
			Step();
			break;

		case synLogicOr:
			logicJumpList.Add(CComLinkElem(Instruction(instrJMPNZ, 0, &(es.expStack.el->data)), es.expStack.el->data));
			Step();
			break;

		case synAssigning:
			assigning = true;
			Step();
			break;

		case synAddressing:
			addressing = true;
			Step();
			break;

		case kwNew:				DoNew();			optimizeAssign = wasOp = true;		break;
		case kwDelete:			DoDelete();			optimizeAssign = wasOp = true;		break;
		case kwSender:			DoSender();			optimizeAssign = wasOp = true;		break;
		case kwThis:			DoThis();			optimizeAssign = wasOp = true;		break;
		case kwAssigned:		DoAssigned();		optimizeAssign = wasOp = true;		break;
		case kwTypeOf:			DoTypeOf();			optimizeAssign = wasOp = true;		break;
		
		case opParentStart:		
			DoMethodCall();		
			Step();					// pryc s typem
			assert(IsDataToken(look));
			Step();					// pryc s pointerem
			assert(IsDataToken(look));
			Step();					// pryc s udajem o lvalue
			stackToPlaceParam = toPlaceParamList.RemoveHead();
			if(stackToPlaceParam == cUnknown)
			{
				doingParams = false;
				optimizeAssign = wasOp = true;		
			}
			break;

		case synArgsEnd:
			es.Push(CComExpression(look,false,false,false,cUnknown,cUnknown));
			Step();
			doingParams = true;
			toPlaceParamList.Add(stackToPlaceParam);
			stackToPlaceParam = currentStackTop;
			break;

		case synLocalizedArgument:
			Step();
			break;

		case synNextParameter:
			if(doingParams)
				DoPushParamOnStack();
			Step();
			doingParams = true;
			break;

		case opDoubleDot:
			Step();				// pryc s dvojteckou
			assert(IsDataToken(look));
			if(look - tokDataFirst != kwFirst)
			{
				es.Load1();
				CComExpression stackTmp = AllocateStack(es.e1.type, es.e1.pointer+1);
				DoAddress(&stackTmp, &es.e1);
				switch(look - tokDataFirst)
				{
				case kwRet:		stackTmp.value.intData = eKTret;		break;
				case kwRetAnd:	stackTmp.value.intData = eKTretAND;		break;
				case kwRetOr:	stackTmp.value.intData = eKTretOR;		break;
				case kwRetAdd:	stackTmp.value.intData = eKTretADD;		break;
				default:
					assert(false);
				}
				
				es.Push(stackTmp);
			}
			Step();				// pryc s retem
			break;

		case synSafeArgument:
			if(doingParams && old != synNextParameter && old != synArgsEnd)
				DoPushParamOnStack();
			Step();
			break;

		default:
			if(wasOp = IsOperatorToken(look))
			{
				enmOperators op = (enmOperators)look;
				DoOp(op);
				assigning = false;
				if(op != opConditional)
					optimizeAssign = true;
			} else
				if(IsKnownNameToken(look) || IsNameToken(look))
				{
					CComExpression stackTmp = AllocateStack(kwName, 1);
					CComExpression expNameID;

					if(IsKnownNameToken(look))
						SetToConstant(&expNameID, nTab->Find(KnownNames[look-knSecond].namestr) - tokNameFirst);
					else
						SetToConstant(&expNameID, look - tokNameFirst);

					Instruction(instrNAME, &stackTmp, &expNameID);
					es.Push(stackTmp);
					wasOp = optimizeAssign = true;
					Step();
				} else
					if(IsKernelService(look))
					{
						es.Push(CComExpression(look,false,false,false,cUnknown,cUnknown,&pos));
						Step();
					} else
						if(IsKernelConstant(look))
						{
							int cid;
							cTab->FindOrAddConstant(cKerConstants[look - kcFirst].value, &cid);
							es.Push(CComExpression(cid, false, false, false, kwInt, 0));
							Step();
						} else
						{
							DoPrimary();
							if(IsGlobalToken(old)) 
								wasOp = optimizeAssign = true;
						}
		}
	}
	Step();			// pryc s synEndExpression
	resExp = es.expStack.el->data;
}

void CComCodeGen::DoPushParamOnStack()
{
	if(!es.expStack.el)
		return;							// zadny parametr
	
	es.Load1();

	CComExpression stackTmp;
	
	stackTmp = AllocateStack(es.e1.type, es.e1.pointer, !IsStringType(es.e1.type) || es.e1.pointer);	
	stackTmp.data1 = stackToPlaceParam;
	stackTmp.value.intData = es.e1.value.intData;

	tmpData->dType = stackTmp.type;
	tmpData->pointer = stackTmp.pointer;

	stackToPlaceParam += tmpData->GetSize();

	CheckImmDoubleOrString(&stackTmp, &es.e1);
	DoOpAssign(&stackTmp, &es.e1);

	es.Push(stackTmp);
}

void CComCodeGen::DoPrimary()
{
	CComOBBaseData* d;
	
	d = (CComOBBaseData*)GetTab(look)->Get(look);
	if(look == cTab->GetNNull())
	{
		es.Push(expConst0);
		es.expStack.el->data.type = kwName;
		es.expStack.el->data.pointer = 1;
	} else
		es.Push(CComExpression(look, false, false, false, d->dType, d->pointer));
	Step();
} 

void CComCodeGen::DoMethodCall()
{
	// Na zasobniku mam parametry jako vyrazy, synArgsEnd, metodu a nakonec objekt
	CListK<CComExpression> params;
	CComOBObject* o;
	CComOBMethod* m;
	int id;
	int time = cUnknown;
	CComShortPosition posTime, posName;
	CComExpression expObj, expMet, expTimeOrCallend;
	bool implicitSpec;

	assert(look == opParentStart);
	Step();									// pryc s operatorem volani
	Step();									// pryc s radkou

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
		es.Touch1();
		expTimeOrCallend = es.e1;
		es.RestorePos(stackPos);
	} else
		SetToConstant(&expTimeOrCallend, 0);

	es.Load1();
	while(es.e1.id != synArgsEnd)		// nactu zpracovane parametry do seznamu
	{
		params.Add(es.e1);
		es.Load1();
	}

	es.Load2();		// v es.e1 mam ID objektu, v es.e2 ID metody

	if(IsKernelService(es.e2.id))
		return DoKernelServiceCall(&params);

	expObj = es.e1;
	expMet = es.e2;
	id = es.e1.type;
	if(IsObjectToken(id))
		o = (CComOBObject*)oTab->Get(id);
	else
		if(id == kwObjptr)	
			o = 0;

	switch(look)
	{
	case semDynamicArrayCall:	// pryc se semDynamicArrayCall
		Step();
		assert(IsDynamicArrayToken(id));
		return DoDynamicArrayCall(&params);

	case semNameCall:
		Step();					// pryc se semNameCall
		assert(IsNameType(es.e2.type));
		return DoMethodCallSafe(&expObj, 0, &expMet, &params);

	case semImplicitSpec:
		implicitSpec = true;
		break;

	default:
		implicitSpec = false;
		assert(IsObjectToken(look));
		o = (CComOBObject*)oTab->Get(look);
	} 


	Step();			// pryc s ID objektu

	assert(IsDataToken(look));
	id = look - tokDataFirst;
	assert(IsMethodToken(id));
	
	if(implicitSpec)
		m = (CComOBMethod*)mTab->Get(id);
	else
		m = (CComOBMethod*)o->mTab->Get(id);
	
	Step();			// pryc s ID metody

	if(time == cUnknown)	// volani
		if(m->storage == kwDirect)
			DoMethodCallDirect(&expObj, m, &params);
		else
			DoMethodCallSafe(&expObj, m, &expMet, &params);
	else					// zprava
		DoMethodMessage(&expObj, &expMet, &params, time, &expTimeOrCallend);
}

void CComCodeGen::DoMethodCallDirect(CComExpression* expObj, CComOBMethod* m, CListK<CComExpression>* params)
{
	int toWrite, toWrite2;

	CComExpression stackTmp = AllocateStack(m->dType, m->pointer);

	toWrite = Instruction(instrDCALL, &stackTmp, expObj);
	toWrite2 = Instruction(0);
	
	LinkAddress(toWrite2, eLkTpInt1, GetParamsSize(params));
	LinkAddress(toWrite2, eLkTpInt2, params->num);
	LinkAddress(toWrite2, eLkTpInt3, params->el ? params->el->data.data1 : 0);

	if(m->mem == cUnknown)
		linkList.Add(CComLinkElem(toWrite, m, 0, eLkTpOperand2));
	else
		LinkAddress(toWrite, eLkTpOperand2, m->mem);

	es.Push(stackTmp);
	dataInInstructions = 1;
}

int CComCodeGen::GetParamsSize(CListK<CComExpression>* params)
{
	int ret = 0;

	CListKElem<CComExpression>* p = params->el;
	while(p)
	{
		tmpData->dType = p->data.type;
		tmpData->pointer = p->data.pointer;
		ret += tmpData->GetSize();
		p = p->next;
	}
	return ret;
}

void CComCodeGen::DoKernelServiceCall(CListK<CComExpression>* params)
{
	int service = es.e2.id - ksFirst;
	int skip = params->num/3 + (params->num%3 ? 1 : 0);
	int toWriteInfo;

	CComExpression expID, expParamCount, expSkip, stackTmp;
	SetToConstant(&expID, service);
	SetToConstant(&expParamCount, params->num);

	stackTmp = AllocateStack(cKerServices[service].returnValue.type, cKerServices[service].returnValue.pointer);
    Instruction(instrSERV, &stackTmp, &expID, &expParamCount);
	toWriteInfo = Instruction(0);

	DoParamsInInstructions(params, skip);
	skip++;
	LinkAddress(toWriteInfo, eLkTpInt1, pos.line);
	LinkAddress(toWriteInfo, eLkTpInt2, skip);

	es.Push(stackTmp);
	dataInInstructions = skip;
}

CComExpression CComCodeGen::AllocateStack(int type, int pointer, bool grow)
{
	CComExpression stackTmp(cUnknown, false, false, false, type, pointer);
	stackTmp.data1 = currentStackTop;
	if(grow)
		GrowStack(&stackTmp);
	return stackTmp;
}

void CComCodeGen::DoParamsInInstructions(CListK<CComExpression>* params, int skip)
{
	int i, toWrite;

	toWrite = code->GetCount();
	for(i = 0; i < skip; i++)
		Instruction(0);

	CListKElem<CComExpression>* p = params->el;
	for(i = 0; i < params->num; i++)
	{
		CIntInstructionRec& instr = code->Get(toWrite + i/3);
		tmpData->dType = p->data.type;
		tmpData->pointer = p->data.pointer;
		instr.instr.byteValue[1+i%3] = tmpData->GetSize();
		LinkAddress(toWrite + i/3, (enmLinkType)(i%3+1), p->data.data1);
		p = p->next;
	}
}

void CComCodeGen::DoDynamicArrayCall(CListK<CComExpression>* params)
{
	CComExpression stackTmp, expType;

	assert(IsDynamicArrayToken(es.e1.type));
	SetToConstant(&expType, es.e1.type);

	if(es.e1.array.GetCount())
		DoArrayAccess(&es.e1);

	if(es.e2.id == cTab->getcountID)
	{
		stackTmp = AllocateStack(kwInt, 0);
		Instruction(instrAGETCNT, &stackTmp, &es.e1, &expType);
	} else
		if(es.e2.id == cTab->setcountID)
		{
			assert(params->num == 1);
			stackTmp = AllocateStack(kwVoid, 0);
			Instruction(instrASETCNT, &es.e1, &(params->el->data), &expType);
		} else
			assert(false);

	es.Push(stackTmp);
}

void CComCodeGen::DoMethodCallSafe(CComExpression* expObj, CComOBMethod* m, CComExpression* expMethod, CListK<CComExpression>* params, int _type, int _pointer)
{
	int num, toWrite, toWrite2, type, pointer, blockSize, dataBlockSize, ret;

	assert(params->num % 2 == 0);
	num = params->num / 2;

	type = (m ? m->groupType : _type);
	pointer = (m ? m->groupPointer : _pointer);
	
	CComExpression stackTmp = AllocateStack(type, pointer);

	toWrite = Instruction(instrSCALL, &stackTmp, expObj, expMethod);
	toWrite2 = Instruction(0);
	
	CListK<CComExpression> values;
	DoSafeParamsInInstructions(params, num, blockSize, dataBlockSize, &values);
	DoParamsInInstructions(&values, num/3 + (num%3 ? 1 : 0));

	tmpData->dType = type;
	tmpData->pointer = pointer;

	ret = eKTret;
	if(m)
		switch(m->ret)			
		{
		case kwRetAnd:	ret = eKTretAND;	break;
		case kwRetOr:	ret = eKTretOR;		break;
		case kwRetAdd:	ret = eKTretADD;	break;
		case kwRet:		
		case cUnknown:	break;
		default:
			assert(false);
		}

	LinkAddress(toWrite2, eLkTpInt1, tmpData->GetMDType() | ret);
	LinkAddress(toWrite2, eLkTpInt2, num);
	LinkAddress(toWrite2, eLkTpInt3, blockSize);
	LinkAddress(toWrite2, eLkTpInt4, dataBlockSize);

	es.Push(stackTmp);
	dataInInstructions = 2*blockSize + dataBlockSize + 1;
}

void CComCodeGen::DoMethodMessage(CComExpression* expObj, CComExpression* expMethod, CListK<CComExpression>* params, int type, CComExpression* expTimeOrCallend)
{
	int num, toWrite, toWrite2, blockSize, dataBlockSize;

	assert(params->num % 2 == 0);
	num = params->num / 2;

	toWrite = Instruction(instrMSG, expTimeOrCallend, expObj, expMethod);
	toWrite2 = Instruction(0);
	
	CListK<CComExpression> values;
	DoSafeParamsInInstructions(params, num, blockSize, dataBlockSize, &values);
	DoParamsInInstructions(&values, num/3 + (num%3 ? 1 : 0));

	switch(type)
	{
	case kwMessage:		type = eKerCTmsg;		break;
	case kwEnd:			type = eKerCTend;		break;
	case kwNextTurn:	type = eKerCTnext;		break;
	case kwNextEnd:		type = eKerCTnextend;	break;
	case kwCallend:		type = eKerCTcallend;	break;
	case kwTimed:		type = eKerCTtimed;		break;
	default:
		assert(false);
	}

	LinkAddress(toWrite2, eLkTpInt1, type);
	LinkAddress(toWrite2, eLkTpInt2, num);
	LinkAddress(toWrite2, eLkTpInt3, blockSize);
	LinkAddress(toWrite2, eLkTpInt4, dataBlockSize);

	es.Push(AllocateStack(kwVoid, 0));
	dataInInstructions = 2*blockSize + dataBlockSize + 1;
}

void CComCodeGen::DoSafeParamsInInstructions(CListK<CComExpression>* params, int num, int& blockSize, int& dataBlockSize, CListK<CComExpression>* values)
{
	int i, toWrite1, toWrite2;	
	CListKElem<CComExpression>* p = params->el;
	
	dataBlockSize = 0;
	blockSize = num/4 + (num%4 ? 1 : 0);
	dataBlockSize = num/3 + (num%3 ? 1 : 0);

	toWrite1 = code->GetCount();
	for(i = 0; i < blockSize; i++)	Instruction(0);
	
	toWrite2 = code->GetCount();
	for(i = 0; i < blockSize; i++)	Instruction(0);
		

	for(i = 0; i < num; i++)					// list params obsahuje dvojice: jmeno, hodnota
	{
		LinkAddress(toWrite2 + i/4, (enmLinkType) (i % 4), p->data.data1); 
		p = p->next;		// dalsi: jmeno -> hodnota

		values->AddTail(p->data);
		tmpData->dType = p->data.type;
		tmpData->pointer = p->data.pointer;
		if(p->data.value.intData != cUnknown)
			LinkAddress(toWrite1 + i/4, (enmLinkType) (i % 4), tmpData->GetMDType(tmpData->dType, tmpData->pointer - 1) | p->data.value.intData); 
		else
			LinkAddress(toWrite1 + i/4, (enmLinkType) (i % 4), tmpData->GetMDType()); 
		p = p->next;		// dalsi: hodnota -> jmeno
	}
}

void CComCodeGen::DoOp(enmOperators op)
{
	int dType, pointer, arity;
	bool isDouble, isString, isLValue;

	assert(op == look);
	Step();					// pryc s operatorem

	dType = look;
	Step();					// pryc s typem
	assert(IsDataToken(look));
	pointer = look - tokDataFirst;
	Step();					// pryc s pointerem
	isLValue = look - tokDataFirst != 0;
	Step();					// pryc s udajem o lvalue

	isDouble = dType == kwDouble && pointer == 0;

	switch(arity = GetOpArity(op))
	{
	case 1:		es.Load1();		break;
	case 2:		es.Load2();		break;
	case 3:		es.Load3();		break;
	default:
		assert(false);
	}
				
	CComExpression stackTmp = CComExpression(cUnknown, false, false, false, dType, pointer);
	stackTmp.data1 = currentStackTop;

	CheckImmDoubleOrString(&stackTmp, &es.e1, op != opBracketStart);
	if(arity >= 2)	CheckImmDoubleOrString(&stackTmp, &es.e2);
	if(arity == 3)	CheckImmDoubleOrString(&stackTmp, &es.e3);

	isString = IsStringType(dType) && !pointer;
	if(!isString)
		GrowStack(&stackTmp);
	else
		if(op == opPlus)
			GrowStack(cMaxStringLn+1);

	if(arity == 2 && IsConstantToken(es.e1.id) && IsConstantToken(es.e2.id))
		DoConstantOptimalization(&stackTmp, &es.e1, &es.e2, op, arity, dType);
	else
	switch(op)
	{
	case opAssign:
		stackTmp = *DoOpAssign(&es.e1, &es.e2);
		break;

	case opPlus:
		if(isString)
		{
			CComExpression tmp1, tmp2;

			DoAddressesForStringOp(&tmp1, &tmp2, &es.e1, &es.e2);
			Instruction(instrSTRCAT, &stackTmp, &tmp1, &tmp2);
		} else
			if(es.e1.pointer)
				DoPointerAdd(&stackTmp, &es.e1, &es.e2);
			else
				if(es.e2.pointer)
					DoPointerAdd(&stackTmp, &es.e2, &es.e1);
				else
					DoAdd(&stackTmp, &es.e1, &es.e2);
		break;

	case opMinus:
		if(es.e1.pointer || es.e2.pointer)
		{
			assert(es.e1.pointer == es.e2.pointer && es.e1.type == es.e2.type);
			DoPointerSub(&stackTmp, &es.e1, &es.e2);
		} else
			DoSub(&stackTmp, &es.e1, &es.e2);
		break;

	case opMultiply:		Instruction(instrMUL, &stackTmp, &es.e1, &es.e2);		break;
	case opDivide:			Instruction(instrDIV, &stackTmp, &es.e1, &es.e2);		break;
	case opModulo:			Instruction(instrMOD, &stackTmp, &es.e1, &es.e2);		break;
	case opLeftShift:		Instruction(instrSHL, &stackTmp, &es.e1, &es.e2);		break;
	case opRightShift:		Instruction(instrSHR, &stackTmp, &es.e1, &es.e2);		break;

	case opBitAnd:			Instruction(instrAND, &stackTmp, &es.e1, &es.e2);		break;
	case opBitOr:			Instruction(instrOR, &stackTmp, &es.e1, &es.e2);		break;
	case opBitXor:			Instruction(instrXOR, &stackTmp, &es.e1, &es.e2);		break;

	case opAddress:			DoAddress(&stackTmp, &es.e1);							break;
	case opBracketStart:	DoBracket(&stackTmp, &es.e1, &es.e2);					break;

	case opLogicAnd:		
		Instruction(instrLAND, &stackTmp, &es.e1, &es.e2);		
		DoSmartLogicJump(&stackTmp);
		break;

	case opLogicOr:			
		Instruction(instrLOR, &stackTmp, &es.e1, &es.e2);		
		DoSmartLogicJump(&stackTmp);
		break;

	case opBitNot:			Instruction(instrNEG, &stackTmp, &es.e1);				break;
	case opLogicNot:		Instruction(instrNOT, &stackTmp, &es.e1);				break;

	case opUnaryPlus:		stackTmp = es.e1;										break;

	case opUnaryMinus:		
		if(IsConstantToken(es.e1.id))
		{
			CComOBConstant* c = (CComOBConstant*)cTab->Get(es.e1.id);
			SetToConstant(&stackTmp, -c->value.intData);
		} else
			DoSub(&stackTmp, &expConst0, &es.e1);	
		break;

	case opPreIncrement:	
		if(inputTokens == 2)
			Instruction(instrINC, &stackTmp, &es.e1);				
		else
			DoPreOpOnComplex(instrINC, stackTmp, es.e1);
		break;

	case opPreDecrement:	
		if(inputTokens == 2)
			Instruction(instrDEC, &stackTmp, &es.e1);				
		else
			DoPreOpOnComplex(instrDEC, stackTmp, es.e1);
		break;

	case opPostIncrement:	
		stackTmp = *DoOpAssign(&stackTmp, &es.e1);
		
		if(inputTokens == 2)
			Instruction(instrINC, &es.e1, &es.e1);
		else
			DoPostOpOnComplex(instrINC, stackTmp);
		
		break;

	case opPostDecrement:	
		stackTmp = *DoOpAssign(&stackTmp, &es.e1);
		if(inputTokens == 2)
			Instruction(instrDEC, &es.e1, &es.e1);
		else
			DoPostOpOnComplex(instrDEC, stackTmp);
		break;

	case opGreater:				case opLess:			
	case opGreaterEqual:		case opLessEqual:		
	case opEqual:				case opNotEqual:		
		DoRelation(&stackTmp, &es.e1, &es.e2, op);				
		break;


	case opIndirection:		
		if(assigning)
		{
			CComExpression expTmp;
			expTmp = stackTmp;
			stackTmp = es.e1;
			stackTmp.type = expTmp.type;
			stackTmp.pointer = expTmp.pointer;
			stackTmp.hasSideEffect = true;
		} else 
			if(!addressing)
			{
				CComExpression expSize;
				SetToConstantT(&expSize, stackTmp.type, stackTmp.pointer);
				Instruction(instrPEEK, &stackTmp, &es.e1, &expSize);
				stackTmp.data3 = tmpExpStorage.Add(es.e1) + 1;	// + 1 kvuli testu vuci 0
			} else
				stackTmp = es.e1;
		break;

	case opConditional:		
		DoConditional(&es.e2);	
		stackTmp = es.e2;
		optimizeAssign = false;
		break;

	case opComma:		
		stackTmp = es.e2;
		break;

	case opMemberDot:
		{
			CComExpression expShift;

			if(es.e1.data3 && (addressing || assigning))	// k promenne jsem se dostal dereferenci a pozaduje se jeji adresa (tu mam v tmpExpStorage)
			{
				stackTmp = tmpExpStorage[es.e1.data3-1];
				SetToConstant(&expShift, StructureAccess(&es.e1, &es.e2));
				DoAdd(&stackTmp, &stackTmp, &expShift);
				if(assigning)
				{
					stackTmp.hasSideEffect = true;
					GrowStack(&stackTmp);
				}
			} else
				{
					stackTmp = es.e1;
					stackTmp.data2 = StructureAccess(&es.e1, &es.e2);
				}
			break;
		}
	case opMemberArrow:
		{
			CComExpression expIndex;
			SetToConstant(&expIndex, StructureAccess(&es.e1, &es.e2));
			DoPointerAdd(&stackTmp, &es.e1, &expIndex, true);

			if(assigning)
			{
				stackTmp.hasSideEffect = true;
				GrowStack(&stackTmp);
			} else 
				if(!addressing)
				{
					CComExpression expSize;
					SetToConstantT(&expSize, stackTmp.type, stackTmp.pointer);
					CComExpression newTarget = AllocateStack(stackTmp.type, stackTmp.pointer);
					Instruction(instrPEEK, &newTarget, &stackTmp, &expSize);				
					stackTmp = newTarget;
				} 
			break;
		}

	default:
		assert(false);
	}

	if(!isLValue)
		inputTokens = 0;

	es.Push(stackTmp);
}


void CComCodeGen::DoPostOpOnComplex(enmInstructions op, CComExpression opResult)
{
	CIntInstructionRec instr = code->GetLast();
	CComExpression tmp = AllocateStack(opResult.type, opResult.pointer);
	CComExpression expSize;
	SetToConstantT(&expSize, tmp.type, tmp.pointer);

	Instruction(op, &tmp, &opResult);

	switch(instr.instr.opCode)
	{
	case instrPEEK:		
		{
			Instruction(instrPOKE, 0, &tmp, &expSize);
			CIntInstructionRec& pokeInstr = code->Get(code->GetCount()-1);
			pokeInstr.instr.byteValue[1] = instr.instr.byteValue[2];
			pokeInstr.res.intValue = instr.op1.intValue;
			return;
		}

	case instrAIREAD:	InstrFromInstr(instr, instrAIGET, 1, 2, 3);		break;
	case instrACREAD:	InstrFromInstr(instr, instrACGET, 1, 2, 3);		break;
	case instrADREAD:	InstrFromInstr(instr, instrADGET, 1, 2, 3);		break;
	case instrAOREAD:	InstrFromInstr(instr, instrAOGET, 1, 2, 3);		break;
	case instrAPREAD:	InstrFromInstr(instr, instrAPGET, 1, 2, 3);		break;
	case instrANREAD:	InstrFromInstr(instr, instrANGET, 1, 2, 3);		break;

	default:
		assert(false);
	}

	Instruction(instrPOKE, &opResult, &tmp, &expSize);
}

void CComCodeGen::DoPreOpOnComplex(enmInstructions op, CComExpression opResult, CComExpression opSource)
{
	CIntInstructionRec instr = code->GetLast();
	CComExpression expSize;
	SetToConstantT(&expSize, opResult.type, opResult.pointer);

	Instruction(op, &opResult, &opSource);

	switch(instr.instr.opCode)
	{
	case instrPEEK:		
		{
			Instruction(instrPOKE, 0, &opResult, &expSize);
			CIntInstructionRec& pokeInstr = code->Get(code->GetCount()-1);
			pokeInstr.instr.byteValue[1] = instr.instr.byteValue[2];
			pokeInstr.res.intValue = instr.op1.intValue;
			return;
		}

	case instrAIREAD:	InstrFromInstr(instr, instrAIGET, 1, 2, 3);		break;
	case instrACREAD:	InstrFromInstr(instr, instrACGET, 1, 2, 3);		break;
	case instrADREAD:	InstrFromInstr(instr, instrADGET, 1, 2, 3);		break;
	case instrAOREAD:	InstrFromInstr(instr, instrAOGET, 1, 2, 3);		break;
	case instrAPREAD:	InstrFromInstr(instr, instrAPGET, 1, 2, 3);		break;
	case instrANREAD:	InstrFromInstr(instr, instrANGET, 1, 2, 3);		break;

	default:
		assert(false);
	}

	Instruction(instrPOKE, &opSource, &opResult, &expSize);
}

void CComCodeGen::DoAddress(CComExpression* target, CComExpression* source)
{
	CComExpression expOffset;
	CComExpression expSel;
	int offset, sel;

	if(addressing)
	{
		addressing = false;
		DoOpAssign(target, source);
		return;
	}

	if(IsLocalToken(source->id))
	{
		offset = ((CComOBBaseData*)inMethod->lTab->Get(source->id))->mem;
		sel = eOpTpLocal;
	} else if(IsAttributeToken(source->id))
	{
		offset = ((CComOBBaseData*)inObj->aTab->Get(source->id))->mem;
		sel = eOpTpAttribute;
	} else if(IsArgumentToken(source->id))
	{
		offset = ((CComOBBaseData*)inMethod->aTab->Get(source->id))->mem;
		sel = eOpTpLocal;
	} else if(source->id == cUnknown)
	{
		offset = source->data1;
		sel = eOpTpStackTmp;
	} else if(IsGlobalToken(source->id))
	{
		offset = source->id - tokGlobalFirst;
		sel = eOpTpGlobal;
	} else 
		assert(false);
	
	SetToConstant(&expSel, sel);
	SetToConstant(&expOffset, offset);
	Instruction(instrADDR, target, &expSel, &expOffset);
}

void CComCodeGen::DoBracket(CComExpression* target, CComExpression* e1, CComExpression* e2)
{
	enmInstructions opCode;

	if(IsDynamicArrayToken(e1->type) && e1->pointer == 1)
	{
		if(e1->array.GetCount())
			DoArrayAccess(e1);

		if(assigning)
		{
			switch(e1->type)
			{
			case kwIntArray:	opCode = instrAIGET;	break;
			case kwCharArray:	opCode = instrACGET;	break;
			case kwDoubleArray:	opCode = instrADGET;	break;
			case kwObjptrArray:	opCode = instrAOGET;	break;
			case kwNameArray:	opCode = instrANGET;	break;
			case kwVoidArray:	opCode = instrAPGET;	break;
			default:
				assert(false);
			}
			Instruction(opCode, target, e1, e2);
			target->hasSideEffect = true;
		} else
		{
			switch(e1->type)
			{
			case kwIntArray:	opCode = (addressing ? instrAIGET : instrAIREAD);	break;
			case kwCharArray:	opCode = (addressing ? instrACGET : instrACREAD);	break;
			case kwDoubleArray:	opCode = (addressing ? instrADGET : instrADREAD);	break;
			case kwObjptrArray:	opCode = (addressing ? instrAOGET : instrAOREAD);	break;
			case kwNameArray:	opCode = (addressing ? instrANGET : instrANREAD);	break;
			case kwVoidArray:	opCode = (addressing ? instrAPGET : instrAPREAD);	break;
			default:
				assert(false);
			}
			Instruction(opCode, target, e1, e2);
		}
	} else
	{
		if(IsStringType(e1->type) && e1->pointer == 0)
		{
			if(IsConstantToken(e2->id))
			{
				CComOBConstant* c = (CComOBConstant*)cTab->Get(e2->id);
				*target = *e1;
				assert(c->dType == kwInt);
				target->data2 = c->value.intData;
			} else
			{
				CComExpression tmp, tmp2;
				tmp = AllocateStack(e1->type, e1->pointer+1);

				DoAddress(&tmp, e1);

				tmp2 = AllocateStack(e1->type, e1->pointer+1);

				DoPointerAdd(&tmp2, &tmp, e2, true);

				CComExpression expSize;
				SetToConstantT(&expSize, target->type, target->pointer);
				Instruction(instrPEEK, target, &tmp2, &expSize);
			}
		} else
		{
			CComOBBaseData* d;

			if(e1->id == cUnknown)
				d = 0;
			else
				d = (CComOBBaseData*)GetTab(e1->id)->Get(e1->id);

			if((d && d->array.GetCount()) || e1->array.GetCount())
			{
				int i;

				if(!e1->array.GetCount())		// Prekladam prvni pouziti []
				{
					target->array.RemoveAll();
					i = arrayAccess.Add(*e1);
					target->array.Add(i);	// - zapamatuji si id pole
				}

				// Zkopiruju do cile vsechny pripadne predchozi indexy (uplatni se u vicerozmernych poli).
				for(i = 0; i < e1->array.GetCount(); i++)
					target->array.Add(e1->array[i]);

				target->hasSideEffect = e1->hasSideEffect;

				i = arrayAccess.Add(*e2);
				target->array.Add(i);	// Zapamatuju si tento index.
			} else
				DoPointerAdd(target, e1, e2);

			if(assigning)
			{
				target->hasSideEffect = true;
				GrowStack(target);
			} else
				if(!addressing && !target->array.GetCount())
				{
					CComExpression expSize;
					SetToConstantT(&expSize, target->type, target->pointer);

					CComExpression newTarget = AllocateStack(target->type, target->pointer);
					Instruction(instrPEEK, &newTarget, target, &expSize);
					*target = newTarget;
				}
		}
	}
}

void CComCodeGen::DoArrayAccess(CComExpression *expArray)
{
	int i, j, mul;
	CComOBBaseData* d;
	CComExpression expSize, expIndex, expMul, expCurIndex, expPointer, tmp, tmp2;

	SetToConstantT(&expSize, expArray->type, 0);
	
	expPointer = arrayAccess[expArray->array[0]];
	i = expPointer.id;
	d = (CComOBBaseData*)GetTab(i)->Get(i);

	tmp2 = tmp = AllocateStack(expArray->type, expArray->pointer + 1);
	expIndex = AllocateStack(kwInt, 0);
	
	expCurIndex = arrayAccess[expArray->array[expArray->array.GetCount()-1]];
	Instruction(instrMOV, &expIndex, &expCurIndex);

	for(i = 1; i < expArray->array.GetCount() - 1; i++)
	{
		mul = 1;
		for(j = i; j < d->array.GetCount(); j++)
			mul *= d->array[j];

		SetToConstant(&expMul, mul);
		expCurIndex = arrayAccess[expArray->array[i]];

		if(IsConstantToken(expCurIndex.id))
			DoConstantOptimalization(&tmp, &expMul, &expCurIndex, opMultiply, 2, kwInt);
		else
			DoMul(&tmp, &expMul, &expCurIndex);
		DoAdd(&expIndex, &expIndex, &tmp);
		tmp = tmp2;
	}

	DoMul(&expIndex, &expSize, &expIndex);

	DoAddress(&tmp, &expPointer);		// - generuji adresu pole
	
	tmp2 = AllocateStack(expArray->type, expArray->pointer);
	DoAdd(&tmp2, &expIndex, &tmp);
    
	if(!expArray->hasSideEffect)
	{
		CComExpression newTarget = AllocateStack(tmp2.type, tmp2.pointer);
		Instruction(instrPEEK, &newTarget, &tmp2, &expSize);
		tmp2 = newTarget;
	}

	tmp2.hasSideEffect = expArray->hasSideEffect;
	*expArray = tmp2;
}

void CComCodeGen::DoRelation(CComExpression* target, CComExpression* e1, CComExpression* e2, enmOperators op)
{
	enmInstructions opCode;

	if(IsNameType(e1->type) && e1->pointer == 1 && IsNameType(e2->type) && e2->pointer == 1 && IsRelation(op))
		switch(op)
		{
		case opGreater:			case opLess:		opCode = instrNCMPEX;		break;
		case opGreaterEqual:	case opLessEqual:	opCode = instrNCMP;			break;
			break;

		default:
			assert(false);
		}
	else
		switch(op)
		{
		case opGreater:			opCode = instrTESTG;	break;
		case opGreaterEqual:	opCode = instrTESTGE;	break;
		case opLess:			opCode = instrTESTL;	break;
		case opLessEqual:		opCode = instrTESTLE;	break;
		case opEqual:			opCode = instrTESTE;	break;
		case opNotEqual:		opCode = instrTESTNE;	break;
		default: 
			assert(false);
		}
	Instruction(opCode, target, e1, e2);
}

int CComCodeGen::StructureAccess(CComExpression* e1, CComExpression* e2)
{
	CComOBStructure* s;
	CComOBBaseData* d;
	const char* idName;
	int id;

	assert(e1->type && IsUnknownToken(e2->id));
	s = (CComOBStructure*) sTab->Get(e1->type);

	idName = GetTab(e2->id)->Get(e2->id)->name;

	id = s->mTab->Find(idName);
	assert(id != cUnknown);

	d = (CComOBBaseData*) s->mTab->Get(id);
	return d->mem;
}

void CComCodeGen::DoSmartLogicJump(CComExpression* result)
{
	CComLinkElem linkElm;

    linkElm = logicJumpList.RemoveHead();
	
	// je vysledek celeho vyrazu jinde, nez vysledek prvni casti?
	if(linkElm.testExp.id != result->id || linkElm.testExp.data1 != result->data1)		
	{
		CIntInstructionRec& instr = code->Get(linkElm.addr);
		
		instr.op2.intValue = GetInfo(result, instr.instr.byteValue[3]);
	}

	LinkAddress(linkElm.addr, eLkTpJump, GetCurrentAddr());
}

void CComCodeGen::DoConditional(CComExpression* result)
{
	CComLinkElem elm1, elm2;

    elm2 = conditionalList.RemoveHead();
    elm1 = conditionalList.RemoveHead();

	CComExpression tmp(*result);
	*result = *DoOpAssign(&elm2.testExp, result);
	LinkAddress(elm1.addr, eLkTpJump, elm2.blk);
	LinkAddress(elm2.addr, eLkTpJump, GetCurrentAddr());
}

void CComCodeGen::CheckImmDoubleOrString(CComExpression* target, CComExpression* expOp, bool arrayAccessPossible)
{
	int i, j, instr, part, count, len, toWrite;
	bool constant;
	UIntInstruction data;
	CComExpression exp1, exp2;
	CComOBConstant* c;

    constant = IsConstantToken(expOp->id) && !expOp->pointer;

	if(expOp->type == kwDouble && constant)
	{
		c = (CComOBConstant*)cTab->Get(expOp->id);

		assert(target->id == cUnknown);
		target->data1 = currentStackTop;

		GrowStack(expOp);

		SetToConstant(&exp1, c->value.intArray[0]);
		SetToConstant(&exp2, c->value.intArray[1]);
		Instruction(instrPUTDC, target, &exp1, &exp2);
	} else
		if(IsStringType(expOp->type) && constant)
		{
			c = (CComOBConstant*)cTab->Get(expOp->id);
			len = c->dType - kwString;

			assert(target->id == cUnknown);
			target->data1 = currentStackTop;
			GrowStack(len+1);

			count = len / cInstructionSize;
			if(len % cInstructionSize)
				count++;

			SetToConstant(&exp1, count);
			SetToConstant(&exp2, len);
			toWrite = Instruction(instrPUTSC, target, &exp2, &exp1) + 1;

			for(i = 0; i < count; i++)
				Instruction(0);
			
			for(i = 0; i < count*4; i++)
			{
				instr = i / 4;
				part = i % 4;
			
				data.intValue = 0;
				for(j = 0; j < 4; j++)
					if(i*4 + j < len)
						data.byteValue[j] = c->name[i*4 + j];
					else
						break;
				
				LinkAddress(toWrite + instr, (enmLinkType)part, data.intValue);	
			}
		} else
			if(expOp->array.GetCount() && arrayAccessPossible)	// pole - mam pouze adresu zacatku pole (v expOp) - nyni je treba spocitat z indexu ofset
				return DoArrayAccess(expOp);
			else
				return;

	GrowStack(target);
	*expOp = *target;
}

void CComCodeGen::GrowStack(CComExpression* forWhat)
{
	tmpData->dType = forWhat->type;
	tmpData->pointer = forWhat->pointer;
	
	GrowStack(tmpData->GetSize());
}

void CComCodeGen::GrowStack(int bytes)
{
	oldStackTop = currentStackTop;
	currentStackTop += bytes;
	maxStackTop = max(maxStackTop, stackBottom + currentStackTop);
}

void CComCodeGen::DoConstantOptimalization(CComExpression* target, CComExpression* op1, CComExpression* op2, enmOperators op, int opArity, int targetType)
{
	CComOBConstant *c1, *c2;

	c1 = (CComOBConstant*)cTab->Get(op1->id);
	c2 = (CComOBConstant*)cTab->Get(op2->id);

	cs.Reset();
	cs.Push(CComExpression(c1->value.intData));
	cs.Push(CComExpression(c2->value.intData));
	cs.DoOp(op, opArity, targetType);
	cs.Load1();
	if(cs.e1.type == kwDouble)
		SetToConstant(target, cs.e1.value.doubleData);
	else
		SetToConstant(target, cs.e1.value.intData);
}

void CComCodeGen::DoPointerAdd(CComExpression* target, CComExpression* expPointer, CComExpression* expIndex, bool structAccess)
{
	CComExpression realTarget;

	tmpData->SetData(cUnknown, expPointer->type, expPointer->pointer - 1);
	
	CComExpression expConstSize;
	int size;
	SetToConstant(&expConstSize, size = (structAccess ? 1 : tmpData->GetSize()));

	realTarget = *target;
	if(IsConstantToken(expIndex->id))
		DoConstantOptimalization(target, expIndex, &expConstSize, opMultiply, 2, kwInt);
	else
		if(size > 1)
			Instruction(instrMUL, target, expIndex, &expConstSize);
		else
			*target = *expIndex;
	
	DoAdd(&realTarget, target, expPointer);
	*target = realTarget;
}

void CComCodeGen::DoPointerSub(CComExpression* target, CComExpression* expPointer1, CComExpression* expPointer2)
{
	CComExpression expConstSize;

	tmpData->SetData(cUnknown, expPointer1->type, expPointer1->pointer - 1);
	
	SetToConstant(&expConstSize, tmpData->GetSize());

	DoSub(target, expPointer1, expPointer2);
	Instruction(instrDIV, target, target, &expConstSize);
}

CComExpression* CComCodeGen::DoOpAssign(CComExpression* target, CComExpression* source)
{
	/* 
	Optimalizace - predchazejici vypocet ma vysledek na zasobniku. Je zbytecne 
	pridavat instrukci MOV cil, zasobnik. Misto toho zmenim cil posledni instrukce
	predchoziho vypoctu.
	*/
	int i;
	CIntInstructionRec  newInstr;

	CComOBBaseData* d;

	if(source->id == cUnknown)
		d = 0;
	else
		d = (CComOBBaseData*)GetTab(source->id)->Get(source->id);

	if((d && d->array.GetCount()) || source->array.GetCount())
	{
		DoAddress(target, source);
		return target;
	}

	i = code->GetCount() - 1;

	if(i == cUnknown)
	{
		DoMove(target, source);			// prvni instrukce - neni co optimalizovat
		return target;
	}

	// Hledam posledni instrukci - preskakuju DBG, ty nemaji jsou jen do poctu...
	i -= dataInInstructions;
	while(i >= codeStart && code->Get(i).instr.opCode == instrDBG)
		i--;

	if(!IsStringType(target->type) || target->pointer)
		DoMove(target, source);				// zatim vygeneruju MOV
	else
		if(target->id != source->id || target->data1 != source->data1)
		{
			CComExpression tmp1, tmp2;

			DoAddressesForStringOp(&tmp1, &tmp2, target, source);

			Instruction(instrSTRCPY, &tmp1, &tmp2);			// pripadne u stringu STRCPY
			return target;
		} else
			return target;
	
	CIntInstructionRec& lastInstr = code->Get(i);		// posledni instrukce
	newInstr = code->GetLast();

	if(	optimizeAssign &&								// Jestlize smim optimalizovat (skoro vzdy, jen ne po otazniku)
		IsAssignOptimalizable((enmInstructions)lastInstr.instr.opCode) &&	// a predchazi optimalizovatelna instrukce
		newInstr.instr.opCode == instrMOV &&								// a delam MOV 
		(getARG(lastInstr.instr.byteValue[1]) == eOpTpStackTmp) &&			// a vysledek byl ulozen na zasobnik
		lastInstr.res.intValue == newInstr.op1.intValue &&					// a tento vysledek ma nyni MOV jako zdroj
		source->id == cUnknown &&											// a opravdu to bere ze zasobniku
		(lastInstr.instr.opCode != instrPEEK || lastInstr.op2.intValue == 4))		// a neni to moc velke
	{
		lastInstr.instr.byteValue[1] = newInstr.instr.byteValue[1]; // tak zmenim cil predchozi instrukce
		lastInstr.res.intValue = newInstr.res.intValue;
		code->RemoveLast();								// a novou (MOV) zrusim, protoze neni potreba.
	}
	return target;
}

void CComCodeGen::DoAddressesForStringOp(CComExpression* tmp1, CComExpression* tmp2, CComExpression* op1, CComExpression* op2)
{
	*tmp1 = AllocateStack(op1->type, op1->pointer+1);
	*tmp2 = AllocateStack(op2->type, op2->pointer+1);

	DoAddress(tmp1, op1);
	DoAddress(tmp2, op2);
}

bool CComCodeGen::IsAssignOptimalizable(enmInstructions opCode)
{
	return opCode >= instrADD && opCode <= instrNAME 
		|| 	(opCode >= instrAIREAD && opCode <= instrAPREAD);
}


void CComCodeGen::DoMove(CComExpression* target, CComExpression* source)
{
	if(target->id != source->id || target->data1 != source->data1)
		Instruction(instrMOV, target, source);
}

void CComCodeGen::DoAdd(CComExpression* target, CComExpression* e1, CComExpression* e2)
{
	CComOBConstant *c1, *c2;
	c1 = c2 = 0;

	if(IsConstantToken(e1->id))
		c1 = (CComOBConstant*)cTab->Get(e1->id);

	if(IsConstantToken(e2->id))
		c2 = (CComOBConstant*)cTab->Get(e2->id);

	if(c1 && c1->dType == kwInt && c1->value.intData == 0)
		DoOpAssign(target, e2);
	else
		if(c2 && c2->dType == kwInt && c2->value.intData == 0)
			DoOpAssign(target, e1);
		else
			Instruction(instrADD, target, e1, e2);
}

void CComCodeGen::DoSub(CComExpression* target, CComExpression* e1, CComExpression* e2)
{
	CComOBConstant *c = 0;

	if(IsConstantToken(e2->id))
		c = (CComOBConstant*)cTab->Get(e2->id);

	if(c && c->dType == kwInt && c->value.intData == 0)
		DoOpAssign(target, e1);
	else
		Instruction(instrSUB, target, e1, e2);
}

void CComCodeGen::DoMul(CComExpression* target, CComExpression* e1, CComExpression* e2)
{
	CComOBConstant *c1, *c2;
	c1 = c2 = 0;

	if(IsConstantToken(e1->id))
		c1 = (CComOBConstant*)cTab->Get(e1->id);

	if(IsConstantToken(e2->id))
		c2 = (CComOBConstant*)cTab->Get(e2->id);

	if(c1 && c1->dType == kwInt && c1->value.intData == 1)
		DoOpAssign(target, e2);
	else
		if(c2 && c2->dType == kwInt && c2->value.intData == 1)
			DoOpAssign(target, e1);
		else
			Instruction(instrMUL, target, e1, e2);
}

int CComCodeGen::GetCurrentAddr() const
{ 
	assert(code == &code1);
	return code->GetCount() * sizeof(CIntInstructionRec); 
}

int CComCodeGen::GetCurrentCodeIndex() const
{ 
	assert(code == &code1);
	return code->GetCount(); 
}

int CComCodeGen::Instruction(int opcode, CComExpression* res, CComExpression* op1, CComExpression* op2)
{
	CIntInstructionRec instr;
	int ret;

	if(opcode)
		dataInInstructions = 0;

	CHECK_BYTE(opcode);
	instr.instr.opCode = opcode;

	CComExpression stackTmp;

	if(res)
		if(res->hasSideEffect)	
		{
			stackTmp = AllocateStack(res->type, res->pointer, false);

			instr.res.intValue = GetInfo(&stackTmp, instr.instr.byteValue[1]);
		} else
			instr.res.intValue = GetInfo(res, instr.instr.byteValue[1]);
	
	if(op1)
		instr.op1.intValue = GetInfo(op1, instr.instr.byteValue[2]);

	if(op2)
		instr.op2.intValue = GetInfo(op2, instr.instr.byteValue[3]);

	if(res && res->hasSideEffect)
	{
		if(instr.instr.opCode == instrMOV)
			stackTmp = *op1;
		else
			ret = code->Add(instr);
	} else
		ret = code->Add(instr);

	if(res && res->hasSideEffect)
	{
		CComExpression exp = *res;
		exp.hasSideEffect = false;

		CComExpression expSize;
		SetToConstantT(&expSize, exp.type, exp.pointer);
		ret = Instruction(instrPOKE, &exp, &stackTmp, &expSize);
	} 

	return ret;
}

int CComCodeGen::GetInfo(CComExpression* e, unsigned char& toWriteInfo)
{
	CComOBBaseData* d;
	enmOpType opType;
	bool isDouble, isChar;
	int ret;

	isDouble = e->type == kwDouble && e->pointer == 0;
	isChar = e->type == kwChar && e->pointer == 0;
	ret = e->data2;

	if(e->id == cUnknown)			// tmp vyraz na stackTop
	{
		ret += e->data1;
		opType = eOpTpStackTmp;
	} else
		if(IsConstantToken(e->id))
		{
			CComOBConstant* c = (CComOBConstant*)cTab->Get(e->id);
			ret += c->value.intData;
			opType = eOpTpImmediate;
		} else
			if(IsGlobalToken(e->id))
			{
				assert(ret == 0);
				opType = eOpTpGlobal;
				ret = e->id - tokGlobalFirst;
			} else
			{
				if(IsLocalToken(e->id))
				{
					d = (CComOBBaseData*)inMethod->lTab->Get(e->id);
					opType = eOpTpLocal;
				} else
					if(IsAttributeToken(e->id))
					{
						d = (CComOBBaseData*)inObj->aTab->Get(e->id);
						opType = eOpTpAttribute;
					} else
						if(IsArgumentToken(e->id))
						{
							d = (CComOBBaseData*)inMethod->aTab->Get(e->id);
							opType = eOpTpLocal;
						} else
							assert(false);
				ret += d->mem;
			}

	toWriteInfo = GetInstructionByteInfo(isDouble, isChar, opType);
	return ret;
}

int CComCodeGen::GetInstructionByteInfo(bool isDouble, bool isChar, enmOpType opType)
{
	if(isDouble)
		return opType | eOpTpDouble;
	else
		if(isChar)
			return opType | eOpTpChar;
		else
			return opType;
}

int CComCodeGen::InstrFromInstr(CIntInstructionRec instr, int opCode, int partRes, int partOp1, int partOp2)
{
	CIntInstructionRec newInstr;

	if(opCode)
		dataInInstructions = 0;

	CHECK_BYTE(opCode);
	newInstr.instr.opCode = opCode;

	if(partRes != cUnknown)
	{
		newInstr.instr.byteValue[1] = instr.instr.byteValue[partRes];
		newInstr.res.intValue = instr.GetPart(partRes);
	} else
	{
		newInstr.instr.byteValue[1] = 0;
		newInstr.res.intValue = 0;
	}

	if(partOp1 != cUnknown)
	{
		newInstr.instr.byteValue[2] = instr.instr.byteValue[partOp1];
		newInstr.op1.intValue = instr.GetPart(partOp1);
	} else
	{
		newInstr.instr.byteValue[2] = 0;
		newInstr.op1.intValue = 0;
	}

	if(partOp2 != cUnknown)
	{
		newInstr.instr.byteValue[3] = instr.instr.byteValue[partOp2];
		newInstr.op2.intValue = instr.GetPart(partOp2);
	} else
	{
		newInstr.instr.byteValue[3] = 0;
		newInstr.op2.intValue = 0;
	}

	return code->Add(newInstr);
}

void CComCodeGen::DebugToCode()
{
	if(cNoDebugInCode)
		return;

	int toWrite = Instruction(instrDBG);
	LinkAddress(toWrite, eLkTpInt2, pos.fileID);
	LinkAddress(toWrite, eLkTpInt3, pos.line);
	LinkAddress(toWrite, eLkTpInt4, pos.column);
}

void CComCodeGen::DoPostPass()
{
	CComLinkElem elm;
	while(!linkList.IsEmpty())
	{
		elm = linkList.RemoveHead();
		assert(elm.m->mem != cUnknown);
		LinkAddress(elm.addr, elm.operand, elm.m->mem);
	}
}

void CComCodeGen::LinkAddress(int toWriteAddr, enmLinkType type, int addr)
{
	CIntInstructionRec& instr = code->Get(toWriteAddr);
	switch(type)
	{
	case eLkTpInt1:	instr.instr.intValue = addr;	break;
	case eLkTpInt2:	instr.res.intValue = addr;	break;
	case eLkTpInt3:	instr.op1.intValue = addr;	break;
	case eLkTpInt4:	instr.op2.intValue = addr;	break;
	default: 
		assert(false);
	}
}

void CComCodeGen::DebugCode(const char* file)
{
	CComOBBaseData* d;

	FILE* f = fopen(file,"w");
	assert(f);

	CIntInstructionRec instr;
	int i, j, k;

	i = j = 0;
	while(i < code->GetCount())
	{
		while(j < debugMethod.GetCount() && (inMethod = debugMethod.Get(j))->mem == i * cInstructionSize)
		{
			fprintf(f, "\n");

			fprintf(f, "%s::%s:\n",inMethod->inObj->name,inMethod->name);
			stackBottom = inMethod->localsPos + inMethod->localsSize;
			j++;

			if(inMethod->aTab->GetCount())
			{
				fprintf(f, "address      param\n");
				for(k = 0; k < inMethod->aTab->GetCount(); k++)
				{
					d = inMethod->GetArgument(k);
					fprintf(f, " %6d      %s\n", d->mem, d->name);
				}
				fprintf(f, "\n");
			}

			if(inMethod->lTab->GetCount())
			{
				fprintf(f, "address      local\n");
				for(k = 0; k < inMethod->lTab->GetCount(); k++)
				{
					d = inMethod->GetLocal(k);
					fprintf(f, " %6d      %s\n", d->mem, d->name);
				}
				fprintf(f, "\n");
			}
		}
		instr = code->Get(i);
		fprintf(f, "%6d:", i * cInstructionSize);
		i += DebugInstruction(f, instr, i);
		i++;

	}
	fclose(f);
}

int CComCodeGen::DebugInstruction(FILE* f, CIntInstructionRec rec, int index)
{
	int i, j, k, addr;
	int skip = 0;
	CIntInstructionRec instr;
	UIntInstruction data;
	
	fprintf(f, "\t%s\t", cInstructions[rec.instr.opCode].name);
	
	for(i=0; i<3; i++)
		if(cInstructions[rec.instr.opCode].hasArg[i])
		{
			if(i)
				fprintf(f, ",\t");

			addr = rec.GetPart(i+1);

			switch(getARG(rec.instr.byteValue[i+1]))
			{
			case eOpTpMissing:		
				if(!addr) 
				{		
					fprintf(f, "M");	
					continue;	
				} else
					fprintf(f, "%d", addr);	
				break;

			case eOpTpImmediate:	fprintf(f, "%d", addr);		break;
			case eOpTpLocal:		fprintf(f, "l:%d", addr);	break;
			case eOpTpAttribute:	fprintf(f, "a:%d", addr);	break;
			case eOpTpStackTmp:		fprintf(f, "s:%d", addr);	break;
			case eOpTpGlobal:		fprintf(f, "g:%d", addr);	break;
			default:
				assert(false);
			}

			if(isDOUBLE(rec.instr.byteValue[i+1]))
				fprintf(f, "d");
			else
				if(isCHAR(rec.instr.byteValue[i+1]))
					fprintf(f, "c");
		}

	switch(rec.instr.opCode)
	{
	case instrDBG:		
        fprintf(f, "\t\t\tline: %5d, column: %3d, file: %3d", rec.op1.intValue, rec.op2.intValue, rec.res.intValue);	
		break;

	case instrSWITCH:	
		skip = rec.op2.intValue;
		fprintf(f, "\n");
		for(i=0; i<rec.res.intValue; i++)
		{
			instr = code->Get(index + 1 + i/2);
			if(i%2)
				fprintf(f, "\t");
			else
				fprintf(f, "%6d: ", (index + i/2 + 1) * cInstructionSize);

			fprintf(f, "case %10d at %6d\n", i%2 ? instr.res.intValue : instr.instr.intValue, i%2 ? instr.op2.intValue : instr.op1.intValue);
		}
		instr = code->Get(index + skip);
		fprintf(f, "%6d: default         at %6d", (skip + index) * cInstructionSize, instr.instr.intValue);
		break;

	case instrPUTDC:
		{
			UComValues val;
			val.intArray[0] = rec.op1.intValue;
			val.intArray[1] = rec.op2.intValue;
			fprintf(f, ",\t%f", val.doubleData);
		}
		break;

	case instrPUTSC:
			fprintf(f, "\n");
			skip = rec.op2.intValue;
			for(i = 0; i < skip; i++)
			{
				instr = code->Get(index + i + 1);	
				fprintf(f, "%6d: ", (index + i + 1)* cInstructionSize);
				for(j = 0; j < 4; j++)
				{
					data.intValue = instr.GetPart(j);
					for(k = 0; k < 4; k++)
						if(data.byteValue[k])
							fprintf(f, "%c", data.byteValue[k]);
				}
				if(i+1 < skip)
					fprintf(f, "\n");
			}
		break;

	case instrNAME:		
			fprintf(f, "\t\t; %s", nTab->Get(rec.op1.intValue+tokNameFirst)->name);		
			break;

	case instrNEWO:		
		fprintf(f, "\t\t; %s", nTab->Get(rec.op1.intValue+tokNameFirst)->name);		
		break;

	case instrNEWA:		
	case instrDELA:		
		fprintf(f, "\t\t; %s", cKeywords[rec.op1.intValue-kwFirst-1]);
		break;

	case instrAGETCNT:		
	case instrASETCNT:		
		fprintf(f, "\t\t; %s", cKeywords[rec.op2.intValue-kwFirst-1]);
		break;

	case instrADDR:
		fprintf(f, "\t\t; %s", cOpTypes[rec.op1.intValue]);
		break;

	case instrSERV:
		instr = code->Get(index + 1);
		fprintf(f, "\t; service: %s, \tparams: %d\n", cKerServices[rec.op1.intValue].name, rec.op2.intValue); 
		fprintf(f, "%6d: \tline: %d, \tskip: %d", (index+1) * cInstructionSize, instr.instr.intValue, instr.res.intValue);
		skip = instr.res.intValue;
		DebugParams(f, skip-1, index + 2, rec.op2.intValue);
		break;

	case instrDCALL:
		instr = code->Get(index + 1);
		fprintf(f, "\n%6d: \tparams: %d\t size: %d\t first at: %d", (index + 1) * cInstructionSize, instr.res.intValue, instr.instr.intValue, instr.op1.intValue);
		skip = 1;
		break;

	case instrSCALL:
	case instrMSG:
		instr = code->Get(index + 1);
		fprintf(f, "\n%6d: \t", (index + 1) * cInstructionSize); 
		if(rec.instr.opCode == instrSCALL)
		{
			fprintf(f, "returning %s ", CComExpTreeNode::GetMDTypeString(instr.instr.intValue & (eKTret-1))); 
			switch(instr.instr.intValue & eKTretMask)
			{
			case eKTretAND:		fprintf(f, " (AND)\t");		break;
			case eKTretOR:		fprintf(f, " (OR)\t");		break;
			case eKTretADD:		fprintf(f, " (ADD)\t");		break;
			default:
				fprintf(f, " \t");
			}
		} else
			fprintf(f, "type: %d\t ", instr.instr.intValue); 
		fprintf(f, "params: %d\t bsize: %d\t dbsize: %d", instr.res.intValue, instr.op1.intValue, instr.op2.intValue);
		fprintf(f, "\n\tTypes:");
		DebugBlock(f, instr.op1.intValue, index + 2, instr.res.intValue, true);
		fprintf(f, "\n\tNames:");
		DebugBlock(f, instr.op1.intValue, index + 2 + instr.op1.intValue, instr.res.intValue, false);
		fprintf(f, "\n\tValues:");
		DebugParams(f, instr.res.intValue/3 + (instr.res.intValue%3 ? 1 : 0), index + 2 + 2*instr.op1.intValue, instr.res.intValue);
		skip = 1 + 2*instr.op1.intValue + instr.op2.intValue;
		break;
	}

	fprintf(f, "\n");
	return skip;
}

void CComCodeGen::DebugBlock(FILE* f, int blockSize, int startIndex, int paramsCount, bool type)
{
	int i, j;
	CIntInstructionRec instr;

	for(i = 0; i < blockSize; i++)
	{
		fprintf(f, "\n");
		instr = code->Get(startIndex + i);
		fprintf(f, "%6d:", (startIndex + i) * cInstructionSize);
		for(j = 0; j < 4; j++)
			if(i*4 + j < paramsCount)
			{
				if(type)
				{
					fprintf(f, "\t\tparam%d: %d\t (%s)", i*4+j+1, instr.GetPart(j), CComExpTreeNode::GetMDTypeString(instr.GetPart(j) & (eKTret-1)));
					switch(instr.GetPart(j) & (eKTretMask | eKTret))
					{
					case eKTretAND:		fprintf(f, " (AND)\t");		break;
					case eKTretOR:		fprintf(f, " (OR)\t");		break;
					case eKTretADD:		fprintf(f, " (ADD)\t");		break;
					case eKTret:		fprintf(f, " (RET)\t");		break;
					default:
						fprintf(f, " \t");
					}
					fprintf(f, "\n");
				} else
					fprintf(f, "\t\tparam%d @ s:%d\n", i*4+j+1, instr.GetPart(j));
			}
	}
}

void CComCodeGen::DebugParams(FILE* f, int skip, int startIndex, int paramsCount)
{
	int i, j;
	CIntInstructionRec instr;

	for(i = 0; i < skip; i++)
	{
		fprintf(f, "\n");
		instr = code->Get(startIndex + i);
		fprintf(f, "%6d:", (startIndex + i) * cInstructionSize);
		for(j = 0; j < 3; j++)
			if(i*3 + j < paramsCount)
			{
				fprintf(f, "\t\tparam%d @ s:%d\t size: %d", i*3+j+1, instr.GetPart(j+1), instr.instr.byteValue[1+j%3]);
				fprintf(f, "\n");
			}
	}
}

void CComCodeGen::SetAlternativeCodeOutput(CDynamicArrayV<CIntInstructionRec>* code2)
{
	assert(code == &code1);
	code2->RemoveAll();
	code = code2;
}

void CComCodeGen::SetNativeCodeOutput()
{
	code = &code1;
}

void CComCodeGen::InsertCodeFragment(CDynamicArrayV<CIntInstructionRec>* code2)
{
	for(int i=0; i<code2->GetCount(); i++)	
		code->Add(code2->Get(i));
}
