/////////////////////////////////////////////////////////////////////////////
//
// ComExpression.h
//
// Implementace CComExpression - reprezentace vyrazu
// Implementace CComExpStack - zasobnik pro praci s vyrazy
// Implementace CComConstStack - zasobnik pro praci s konstantnimi vyrazy
// Implementace CComExpTree - reprezentace stromu vyrazu
// Implementace CComExpTreeNode - reprezentace uzlu ve stromu vyrazu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComExpression.h"
#include "ComError.h"
#include "ComKerServices.h"
#include "objects.h"

CComExpression::CComExpression()
{
	pos = 0;
}

CComExpression::CComExpression(int _id, bool _isConstant, bool _isLValue, bool _hasSideEffect, int _type, int _pointer, CComPosition* _pos, CDynamicArrayV<int,2>* _array)
{
	id			  = _id;
	isConstant	  = _isConstant;
	isLValue	  = _isLValue;
	hasSideEffect = _hasSideEffect;
	type		  = _type;
	pointer		  = _pointer;
	if(_pos)
	{
		pos	= new CComShortPosition(*_pos);
		assert(pos);
	} else
		pos = 0;

	if(_array)
		for(int i=0; i<_array->GetCount(); i++)
			array.Add(_array->Get(i));

	value.intData = cUnknown;
	data1 = data2 = data3 = 0;
}

CComExpression::~CComExpression()
{
	SAFE_DELETE(pos);
}


CComExpression::CComExpression(CComExpression& e)
{
	id			  = e.id;
	isConstant	  = e.isConstant;
	isLValue	  = e.isLValue;
	hasSideEffect = e.hasSideEffect;
	type		  = e.type;
	pointer		  = e.pointer;
	value		  = e.value;
	data1		  = e.data1;
	data2		  = e.data2;
	data3		  = e.data3;
	if(e.pos)
	{
		pos	= new CComShortPosition(*e.pos);
		assert(pos);
	} else
		pos = 0;
	for(int i=0; i<e.array.GetCount(); i++)
		array.Add(e.array[i]);
}

CComExpression& CComExpression::operator=(const CComExpression& e)
{
	id			  = e.id;
	isConstant	  = e.isConstant;
	isLValue	  = e.isLValue;
	hasSideEffect = e.hasSideEffect;
	type		  = e.type;
	pointer		  = e.pointer;
	value		  = e.value;
	data1		  = e.data1;
	data2		  = e.data2;
	data3		  = e.data3;
	SAFE_DELETE(pos);
	if(e.pos)
	{
		pos	= new CComShortPosition(*e.pos);
		assert(pos);
	} else
		pos = 0;
	array.RemoveAll();

	for(int i=0; i<e.array.GetCount(); i++)
		array.Add(e.array[i]);

	return *this;
}


void CComExpStack::Load1()
{
	e1 = expStack.RemoveHead();
}

void CComExpStack::Load2()
{
	e2 = expStack.RemoveHead();			// postfix -> prvni operand byl ulozen drive a je
    e1 = expStack.RemoveHead();			// tedy az druhy v poradi
}

void CComExpStack::Load3()
{
	e2 = expStack.RemoveHead();			
	e1 = expStack.RemoveHead();			
    e3 = expStack.RemoveHead();			// prvni chci mit extra v e3
}

void CComExpStack::Touch1()
{
	assert(expStack.el);
	e1 = expStack.el->data;
}

void CComExpStack::Touch2()
{
	assert(expStack.el && expStack.el->next);
	e2 = expStack.el->data;
	e1 = expStack.el->next->data;
}

void CComExpStack::RestorePos(CListKElem<CComExpression>* pos)
{
	CListKElem<CComExpression>* p = expStack.el;
	CListKElem<CComExpression>* q;
	while(p && p != pos)
	{
		q = p;
		p = p->next;
		delete q;
	}
	expStack.el = p;
}

void CComConstStack::DoOp(int op, int opArity, int constType)
{
	switch(opArity)
	{
	case 1:		
		Load1();	
		if(e1.type == kwInt && constType == kwDouble)
			e1.value.doubleData = e1.value.intData;
		break;
	case 2:		
		Load2();	
		if(e1.type == kwInt && constType == kwDouble)
			e1.value.doubleData = e1.value.intData;
		if(e2.type == kwInt && constType == kwDouble)
			e2.value.doubleData = e2.value.intData;
		break;
	default:
		assert(false);
	}

	switch(op)
	{
	case opPlus:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData + e2.value.intData));
		else
			Push(CComExpression(e1.value.doubleData + e2.value.doubleData));
		break;

	case opMinus:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData - e2.value.intData));
		else
			Push(CComExpression(e1.value.doubleData - e2.value.doubleData));
		break;

	case opMultiply:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData * e2.value.intData));
		else
			Push(CComExpression(e1.value.doubleData * e2.value.doubleData));
		break;

	case opDivide:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData / e2.value.intData));
		else
			Push(CComExpression(e1.value.doubleData / e2.value.doubleData));
		break;

	case opModulo:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData % e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opRightShift:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData >> e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opLeftShift:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData << e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opBitAnd:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData & e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opBitOr:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData | e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opBitXor:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData ^ e2.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opBitNot:
		if(constType == kwInt)
			Push(CComExpression(~e1.value.intData));
		else
			throw CComError(errCannotUseOperator,noteNone);
		break;

	case opComma:
		if(constType == kwInt)
			Push(CComExpression(e2.value.intData));
		else
			Push(CComExpression(e2.value.doubleData));
		break;

	case opUnaryPlus:
		if(constType == kwInt)
			Push(CComExpression(e1.value.intData));
		else
			Push(CComExpression(e1.value.doubleData));
		break;

	case opUnaryMinus:
		if(constType == kwInt)
			Push(CComExpression(-e1.value.intData));
		else
			Push(CComExpression(-e1.value.doubleData));
		break;


	default:
		assert(false);
	}
}


bool				CComExpTreeNode::assigning	= false;
bool				CComExpTreeNode::stringOp	= false;
CComPart*			CComExpTreeNode::p			= 0;
CComOBObject*		CComExpTreeNode::inObj		= 0;
CComOBMethod*		CComExpTreeNode::inMethod	= 0;
FILE*				CComExpTreeNode::f			= 0;
int					CComExpTreeNode::lastString = cUnknown;

void CComExpTreeNode::Flush(int arrayCount)
{
	if(middle)	// ternarni operator ( ? nebo volani funkce )
	{
		if(data.id == opParentStart)
			FlushMethodItem();
		else
			if(data.id == kwIntArray)
				FlushDynamicArray();
			else
			{	// otaznik vyresim primo zde:
				assert(data.id==opConditional);
				middle->Flush();
				fprintf(f, " ? ");
				assert(right);
				right->Flush();
				fprintf(f, " : ");
				assert(left);
				left->Flush();
			}
	} else
		if(right)	// binarni operator
		{
			bool wasAssigning = assigning;
			bool wasStringOp = stringOp;
			assert(left);

			switch(data.id)
			{
			case opMember4Dot:	
				return left->Flush();
			}

			if(CComPart::IsRelation(data.id) || CComPart::IsEqual(data.id))
				if(CComPart::IsStringType(left->data.type) && left->GetPointer() == 0)
					return FlushStringRelation();
	
			if(CComPart::IsRelation(data.id) && CComPart::IsNameType(left->data.type) && left->GetPointer() == 1)
				return FlushNamesRelation();

			if(data.id == opBracketStart)
				return FlushStandardArray();

			stringOp = wasStringOp || (data.id == opPlus || data.id == opAssign)&& CComPart::IsStringType(data.type);
			assigning = wasAssigning || CComPart::IsAssign(data.id);

			if(data.id == opAssign && stringOp && 
				(CComPart::IsArgumentToken(right->data.id) || CComPart::IsGlobalToken(right->data.id) || CComPart::IsAttributeToken(right->data.id)))
					return FlushStringAssign(wasAssigning, wasStringOp);

			right->Flush();
			assigning = wasAssigning;

			if(CComPart::IsAdd(data.id) || CComPart::IsMultiply(data.id) || CComPart::IsAssign(data.id) ||
				CComPart::IsShift(data.id) || CComPart::IsRelation(data.id) || CComPart::IsEqual(data.id))
				fprintf(f, " %s ", cOperators[data.id-opFirst-1]);
			else
				fprintf(f, "%s", cOperators[data.id-opFirst-1]);
			left->Flush();
		
			stringOp = wasStringOp;
		}
		else
			if(left)	// unarni operator
			{
				switch(data.id)
				{
				case opParentStart:
                    fprintf(f, "(");
					break;

				case kwNew:			
					return FlushNew();

				case kwDelete:
					return FlushDelete();

				case kwTypeOf:
					return FlushTypeOf();
				}

				if(CComPart::IsKernelService(data.id))
					return FlushKernelService();

				if(CComPart::IsUnaryPrefix(data.id))
					fprintf(f, "%s", cOperators[data.id-opFirst-1]);
				
				left->Flush();
				
				if(!CComPart::IsUnaryPrefix(data.id) && data.id!=opParentStart)
					fprintf(f, "%s", cOperators[data.id-opFirst-1]);

				if(data.id == opParentStart)
					fprintf(f, ")");
			} else
				FlushItem(arrayCount);		// operand
}

void CComExpTreeNode::FlushItem(int arrayCount)
{
	CComOBBaseData* d;
	int i;

	if(data.id == cUnknown)
		return;					// prazdny list - nic nevypisovat

	if(CComPart::IsKeywordToken(data.id))
	{
		switch(data.id)
		{
		case kwThis:		fprintf(f, "KerContext->KCthis");	break;
		case kwSender:		fprintf(f, "KerContext->Sender");	break;
		case kwAssigned:	fprintf(f, "KerContext->ParamsInfo[%d]", data.type); break;
		default:
			assert(false);
		}
		return;
	}

	if(CComPart::IsKernelConstant(data.id))
	{
		fprintf(f, cKerConstants[data.id - kcFirst].name + 4);	// bez _KN_
		return;
	} else
		if(CComPart::IsKnownNameToken(data.id))
		{
			fprintf(f, KnownNames[data.id - knSecond].namestr);	
			return;
		} 

	d = (CComOBBaseData*)p->GetTab(data.id)->Get(data.id);
	assert(d);

	if(CComPart::IsArgumentToken(data.id))	
		if(inMethod->storage==kwSafe)
		{
			if(stringOp && !assigning)
				fprintf(f, "CComString(%d, ", d->dType - kwString);
			
			fprintf(f, "KER_ARGUMENT(%s", p->GetTypeString(d->dType));
			for(i=0; i<d->GetPointer(); i++)
				fprintf(f, "*");
			fprintf(f, ", %d)", data.id - tokArgumentFirst);
			
			if(stringOp && !assigning)
				fprintf(f, ", %d)", data.pos->line);
		} else
			if(stringOp && !assigning)
				fprintf(f, "CComString(%d, %s, %d)", d->dType - kwString, d->kerName, data.pos->line);
			else
				fprintf(f, d->kerName);
	else
		if(CComPart::IsAttributeToken(data.id) || CComPart::IsGlobalToken(data.id))
		{
			if(stringOp && !assigning)
				fprintf(f, "CComString(%d, ", d->dType - kwString);

			if(stringOp)
				fprintf(f, "((char*");
			else
			{
				int pointer = d->GetPointer();
				int array = d->array.GetCount();
				bool deref;

				fprintf(f, "(");
				if(deref = (arrayCount < pointer-array || (arrayCount == 0 && (pointer != array || !pointer))))
					fprintf(f, "*");

				const char* tmp = p->GetTypeString(d->dType);
                fprintf(f, "(%s*", tmp);
				
				for(i=0; i<pointer - array; i++)
					fprintf(f, "*");
			}
			fprintf(f, ")%s)", d->kerName);

			if(stringOp && !assigning)
				fprintf(f, ", %d)", data.pos->line);
		} else
			if(CComPart::IsLocalToken(data.id) || (CComPart::IsArgumentToken(data.id) && !data.hasSideEffect))
				fprintf(f, d->kerName);
			else
				if(CComPart::IsConstantToken(data.id))
				{
					CComOBConstant* c = (CComOBConstant*)p->cTab->Get(data.id);
					if(CComPart::IsStringType(c->dType))
						if(!stringOp)
							fprintf(f, "\"%s\"", c->kerName);
						else
							fprintf(f, "CComString(\"%s\", %d)", c->kerName, data.pos->line);
					 else
						if(c->dType == kwChar)
							fprintf(f, "'%s'", c->kerName);
						else
							fprintf(f, c->kerName);
				} else
					if(CComPart::IsNameType(data.type))
					{
						CComOBName* n = (CComOBName*)p->GetTab(data.id)->Get(data.id);
						assert(n);
						fprintf(f, n->kerName);
					} else
						fprintf(f, d->name);
}

void CComExpTreeNode::FlushMethodItem()
{
	CComExpTreeNode* method;
	CComExpTreeNode* callend;
	CComExpTreeNode* params;
	int time, i;
	bool intCall = false;

	assert(left && middle && right && right->data.id==opComma && right->left && right->right);
	
	method = right->right;
	callend = right->left;
	params = left;
	time = method->data.type;						// message - data.type obsahuje cas volani

	assert(method->data.id==kwMethodName);
	CComOBMethod* m = (CComOBMethod*)method->data.value.voidPtrData;

	if(!m)
		if(time!= cUnknown)				
			fprintf(f, "KerMain->message");
		else
			fprintf(f, "KerMain->call");
	else
		if(m->storage == kwDirect)
			fprintf(f, m->kerName);	
		else
		{			// look == kwSafe
			if(time!= cUnknown)				// message
				fprintf(f, "KerMain->message");
			else
				if(m->groupPointer)
				{
					fprintf(f, "(%s", p->GetTypeString(m->groupType));
					for(i=0; i<m->GetPointer(m->groupType, m->groupPointer); i++)
						fprintf(f, "*");
					fprintf(f, ")KerMain->callint");
					intCall = true;
				} else
					switch(m->groupType)
					{
					case kwVoid:	fprintf(f, "KerMain->call");			break;
					case kwChar:	fprintf(f, "KerMain->callchar");		break;
					case kwDouble:	fprintf(f, "KerMain->calldouble");		break;
					case kwInt:		fprintf(f, "KerMain->callint");			intCall = true;		break;
					default:
						fprintf(f, "(%s)KerMain->callint", p->GetTypeString(m->groupType));
						intCall = true;
					}
		}
	fprintf(f, "(%d, ", method->data.pos->line);

	if(middle->data.id == kwThis)
		fprintf(f, "KerContext->KCthis");
	else
		middle->Flush();

	if(!m || m->storage == kwSafe)
	{
		if(m)
			fprintf(f, ", %s", m->kerName);
		else
		{
			fprintf(f, ", ");
			assert(method->left);
			method->left->Flush();
		}

		if(time != cUnknown)
		{
			if(time == kwCallend)
			{
				fprintf(f, ", eKerCTcallend, ");
				callend->Flush();
			} else
			{
				switch(time)
				{
				case kwMessage:		fprintf(f, ", eKerCTmsg, 0");	break;
				case kwEnd:			fprintf(f, ", eKerCTend, 0");	break;
				case kwNextTurn:	fprintf(f, ", eKerCTnext, 0");	break;
				case kwNextEnd:		fprintf(f, ", eKerCTnextend, 0");	break;
				case kwTimed:		
					fprintf(f, ", eKerCTtimed, ");	
					callend->Flush();	
					break;
				default:
					assert(false);
				}
			}
		}

		if(intCall)
			fprintf(f, ", %d", m->GetMDType(m->groupType, m->groupPointer));		// u callint se jeste zadava typ navratove hodnoty

		if(m)
			switch(m->ret)			// Ma funkce specifikovan ret?
			{
			case kwRetAnd:	fprintf(f, " | eKTretAND");	break;
			case kwRetOr:	fprintf(f, " | eKTretOR");	break;
			case kwRetAdd:	fprintf(f, " | eKTretADD");	break;
			case kwRet:
			case cUnknown:	break;
			default:
				assert(false);
			}

		
		fprintf(f, ", %d", params->GetParamCount(true));
		if(time != cUnknown)
		{
			params->FlushParamsSafe(flushType);
			params->FlushParamsSafe(flushName);
			params->FlushParamsSafe(flushValue);
		} else
			params->FlushParamsSafe(flushEverything);

	} else
	{		// storage == kwDirect
		if(params->GetParamCount(false))
			fprintf(f, ", ");
		params->FlushParamsDirect(0, cUnknown, cUnknown);
	}

	fprintf(f, ")");
}

void CComExpTreeNode::FlushDynamicArray()
{
	// left: id 1 -> GetCount() id 2 -> SetCount(), middle: pole, right: parametr
	assert(left && right && middle);
	middle->Flush();
	fprintf(f, "->");
	switch(left->data.id)
	{
	case 1:
		fprintf(f, "GetCount()");
		break;

	case 2:
		fprintf(f, "SetCount(");
		right->Flush();
		fprintf(f, ")");
		break;

	default:
		assert(false);
	}
}

void CComExpTreeNode::FlushKernelService()
{
	assert(left && CComPart::IsKernelService(data.id));	// parametry
	int service = data.id - ksFirst;
	
	fprintf(f, "%s(", cKerServices[service].compileToStr);

	left->FlushParamsDirect(0, service, data.value.intData);	
	fprintf(f, ")");
}


void CComExpTreeNode::FlushParamsSafe(enmFlushWhat what)
{
	int type, pointer;

	if(data.id==cUnknown)		// prazdny parametr -> pryc
		return;

	assert(data.pointer);

	switch(data.id)
	{
	case opComma:				// carka oddelujici parametry -> rekurze na pravy a levy
		assert(left && right && !middle);
		right->FlushParamsSafe(what);
		left->FlushParamsSafe(what);
		break;

	case opDoubleDot:			// dvojtecka oddelujici jmeno a hodnotu:
		assert(left && right && !middle);
		if(CComPart::IsObjectType(left->data.type) || CComPart::IsDynamicArrayToken(left->data.type) || CComPart::IsNameType(left->data.type))
            pointer = 1;
		else
			pointer = 0;

		switch(left->data.type)
		{
		case kwSender:		
			type = eKTobject;		
			break;

		default:
			type = inMethod->GetMDType(left->data.type, pointer);
			assert(type != cUnknown);
		}
			

		if(what & flushType)
		{
			const char* MDstr;

			fprintf(f, ", ");
			if(type >= eKTstring)
			{
				fprintf(f, "(eKTstring + %d)", type - eKTstring);
			} else
				if(MDstr = GetMDTypeString(type))
					fprintf(f, "%s", MDstr);
				else
					assert(false);
			

			switch(data.value.intData)			// Byl specifikovan ret?
			{
			case kwRet:		fprintf(f, " | eKTret");	break;
			case kwRetAnd:	fprintf(f, " | eKTretAND");	break;
			case kwRetOr:	fprintf(f, " | eKTretOR");	break;
			case kwRetAdd:	fprintf(f, " | eKTretADD");	break;
			case cUnknown:	break;
			}
		}

		if(what & flushName)
		{
			fprintf(f, ", ");							// jmeno
			right->Flush();
		}

		if(what & flushValue)
		{
			fprintf(f, ", ");
			if(data.value.intData != cUnknown)		// Byl specifikovan ret?
				fprintf(f, "&(");
			left->Flush();				// hodnota
			if(data.value.intData != cUnknown)		
				fprintf(f, ")");
		}
		break;

	case opParentStart:			// podvyraz
		assert(left);
		fprintf(f, "(");
		left->Flush();
		fprintf(f, ")");
		break;

	default:
		assert(false);
	}
}

const char* CComExpTreeNode::GetMDTypeString(int type)
{
	switch(type)
	{
	case eKTchar:		return "eKTchar";	
	case eKTdouble:		return "eKTdouble";
	case eKTint:		return "eKTint";
	case eKTpointer:	return "eKTpointer";
	case eKTobject:		return "eKTobject";
	case eKTname:		return "eKTname";
	case eKTvoid:		return "eKTvoid";
	case eKTarrChar:	return "eKTarrChar";
	case eKTarrDouble:	return "eKTarrDouble";
	case eKTarrInt:		return "eKTarrInt";
	case eKTarrPointer:	return "eKTarrPointer";
	case eKTarrObject:	return "eKTarrObject";
	case eKTarrName:	return "eKTarrName";
	default:
		if(type >= eKTstring && type <= eKTstring + cMaxStringLn)
			return "eKTstring";
		else
			return 0;	
	}
	return 0;
}

int CComExpTreeNode::FlushParamsDirect(int param, int service, int line)
{
	if(data.id==cUnknown)		// prazdny parametr -> pryc
		return param;

	if(service != cUnknown)
	{
		while(cKerServiceParams[service][param].specUse && param < cKerServices[service].paramsNr)
		{
			if(param)
				fprintf(f, ", ");
			switch(cKerServiceParams[service][param].specUse)
			{
			case 1:			fprintf(f, "%d", line);					break;
			case 2:			fprintf(f, "KerContext->KCthis");		break;
			case 3:
				assert(lastString != cUnknown);
				fprintf(f, "%d", lastString);
				break;

			default:
				assert(false);
			}
			param++;
		}
	}

	switch(data.id)
	{
	case opComma:				// carka oddelujici parametry -> rekurze na pravy a levy
		assert(left && right && !middle && data.pointer==2);
		param = right->FlushParamsDirect(param, service, line);
		param = left->FlushParamsDirect(param, service, line);
		break;

	default:
		if(param)
			fprintf(f, ", ");

		bool typecast = service != cUnknown && cKerServiceParams[service][param].type == kwDouble &&
			cKerServiceParams[service][param].pointer == 0 && data.type != kwDouble;
			
		if(typecast)
			fprintf(f, "(double)(");

		Flush();

		if(typecast)
			fprintf(f, ")");

		if(CComPart::IsStringType(data.type))
			lastString = eKTstring + data.type - kwString;
		param++;
	}
	return param;
}


int CComExpTreeNode::GetParamCount(bool safe)
{
	assert(!middle);

	int ret = 0;

	if(data.pointer)				// operator - carka, dvojtecka nebo zavorka
		switch(data.id)
		{
		case opComma:				// - carka -> rekurze na oba podstromy
			assert(left && right);
			ret += left->GetParamCount(safe);
			ret += right->GetParamCount(safe);
			break;
		
		case opDoubleDot:			// - dvojtecka -> mam jeden parametr
			assert(safe);
			ret = 1;
			break;

		default:
			ret = 1;
		}
	else
	{
		assert(!left && !right);	// parametr - nesmi mit zadne podstromy
		if(data.type != cUnknown)	// prazdny parametr se nepocita
			ret = 1;
	}
	return ret;
}

int CComExpTreeNode::GetPointer()
{
	if(CComPart::IsOperatorToken(data.id))
		if(data.id == opParentStart && data.pointer == 3)
		{
			CComOBMethod* m = (CComOBMethod*)right->right->data.value.voidPtrData;
			assert(m);
			return m->GetPointer();
		} else
			return left->GetPointer();

	CComOBBaseData* d = (CComOBBaseData*)p->GetTab(data.id)->Get(data.id);
	assert(d);
	return d->GetPointer();
}

void CComExpTreeNode::FlushNew()
{
	int line = data.pos->line;
	int pointer = data.value.intData;

	if(CComPart::IsObjectType(data.type) && pointer == 1)		// new na objekt
		fprintf(f, "KerMain->NewObject(%d , %s)", line, p->oTab->Get(data.type)->kerName); 
	else
		if(data.type == cUnknown)
		{
			fprintf(f, "KerMain->NewObject(%d , ", line);
			left->Flush();
			fprintf(f, ")");
		} else
			if(CComPart::IsDynamicArrayToken(data.type) && pointer == 1)	// new na objekt
				fprintf(f, "new %s()", p->GetTypeString(data.type));
			else		// new na data
			{
				fprintf(f, "KER_NEW(%d, %s", line, p->GetTypeString(data.type));

				for(int i=0; i<inMethod->GetPointer(data.type, pointer); i++)		
					fprintf(f, "*");

				fprintf(f, ", ");
				assert(left);
				if(left->data.id != cUnknown)			// zbyva vypsat pocet alokovanych polozek
					left->Flush();
				else
					fprintf(f, "1");
				fprintf(f, ")");
			}
}

void CComExpTreeNode::FlushDelete()
{
	int line = data.data1;

	assert(left);
	if(CComPart::IsObjectType(data.type))	// delete na objekt
		fprintf(f, "KerMain->DeleteObject(%d, ", line);
	else								
		if(CComPart::IsDynamicArrayToken(data.type))
			fprintf(f, "delete ");
		else	// delete na data
			fprintf(f, "KerInterpret->ISFree(%d, ", line);			

	left->Flush();

	if(!CComPart::IsDynamicArrayToken(data.type))
		fprintf(f, ")");
}

void CComExpTreeNode::FlushTypeOf()
{
	assert(left && !right);
	fprintf(f, "KerMain->GetObjType(");
	left->Flush();
	fprintf(f, ")");
}

void CComExpTreeNode::FlushStringRelation()
{
	fprintf(f, "strcmp(");
	right->Flush();
	fprintf(f, ", ");
	left->Flush();

	switch(data.id)
	{
	case opGreater:			fprintf(f, ") > 0");		break;
	case opGreaterEqual:	fprintf(f, ") >= 0");		break;
	case opLess:			fprintf(f, ") < 0");		break;
	case opLessEqual:		fprintf(f, ") <= 0");		break;
	case opEqual:			fprintf(f, ") == 0");		break;
	case opNotEqual:		fprintf(f, ")");			break;
	default:
		assert(false);
	}
}

void CComExpTreeNode::FlushStringAssign(bool wasAssigning, bool wasStringOp)
{
	fprintf(f, "strcpy(");
	right->Flush();

	assigning = wasAssigning;

	fprintf(f, ", ");
	left->Flush();
	fprintf(f, ")");

	stringOp = wasStringOp;
}

void CComExpTreeNode::FlushStandardArray()
{
	CDynamicArrayV<CComExpTreeNode*, 8> indexes;
	CComExpTreeNode *q, *r;
	CComExpTreeNode* kerArray;
	CComOBBaseData* d;
	int i, j, k;

	d = 0;	
	kerArray = 0;
	q = this;
	k = 0;
	while(q)
	{
		if(q->data.id == opBracketStart)
			if(q->right && q->data.type == q->right->data.type - 1)
				kerArray = q->left;
			else
			{
				indexes.Add(q->left);
				k++;
			}
		else
		{
			q->Flush(k);
			r = q;

			if(r->data.id == opMemberArrow)
			{
				assert(CComPart::IsStructureToken(r->right->data.type));
				CComOBStructure* s = (CComOBStructure*)p->sTab->Get(r->right->data.type);
				const char* idName = p->uTab->Get(r->left->data.id)->name;
				d = (CComOBBaseData*)s->mTab->FindRec(idName);
			} else
				d = (CComOBBaseData*)p->GetTab(r->data.id)->Get(r->data.id);
 			break;
		}
		q = q->right;
	}
	assert(d);

	if(d->array.GetCount())
	{
		if(k)
			fprintf(f, "[");

		for(i=0; i<k; i++)
		{
			q = indexes.Get(k-i-1);
			if(q->left)
				fprintf(f, "(");
			q->Flush();
			if(q->left)
				fprintf(f, ")");
			if(i+1 < k)
				fprintf(f, "*");
			for(j=i+1; j<k; j++)
			{
				fprintf(f, "%d", d->array[j]);
				if(j+1 < k)
					fprintf(f, "*");
			}
			if(i+1 < k)
				fprintf(f, " + ");
		}
		if(k)
			fprintf(f, "]");
	} else
		for(i=0; i<k; i++)
		{
			fprintf(f, "[");
			q = indexes.Get(k-i-1);
			q->Flush();
			fprintf(f, "]");
		}

	if(kerArray)
	{
		if(assigning)
			fprintf(f, "->Get(");
		else
			fprintf(f, "->Read(");

		kerArray->Flush();
		fprintf(f, ")");
	}
}

void CComExpTreeNode::FlushNamesRelation()
{
	if(data.id == opGreater || data.id == opLess)
		fprintf(f, "KerServices.IsFMemberOfSExcl(");
	else
		fprintf(f, "KerServices.IsFMemberOfS(");


	switch(data.id)
	{
	case opGreater:			
	case opGreaterEqual:		
		left->Flush();
		fprintf(f, ", ");
		right->Flush();
		break;
		
	case opLess:				
	case opLessEqual:			
		right->Flush();
		fprintf(f, ", ");
		left->Flush();
		break;

	default:
		assert(false);
	}
	
	fprintf(f, ")");
}

void CComExpTree::CreateFromStack(CComExpStack* stack)
{
	SAFE_DELETE(root);
	root = CreateNode(stack);
}

CComExpTreeNode* CComExpTree::CreateNode(CComExpStack* stack)
{
	if(stack->expStack.IsEmpty())
		return 0;

	stack->Load1();
	CComExpTreeNode* newNode = new CComExpTreeNode(stack->e1);
	assert(newNode);


	newNode->left = newNode->right = newNode->middle = 0;
	
	int opArity = stack->e1.pointer;		// arita operatoru

	if(opArity) 
	{
		// alespon unarni operator
		newNode->left = CreateNode(stack);
		
		if(opArity > 1)
		{
			// alespon binarni operator
			newNode->right = CreateNode(stack);
			
			if(opArity == 3)
			{
				// otaznik
				newNode->middle = CreateNode(stack);
			}
		}
	}
	
	return newNode;
}

void CComExpTree::FlushInfixToFile(CComPart* p, CComOBObject* inObj, CComOBMethod* inMethod, FILE* f)
{
	CComExpTreeNode::p = p;
	CComExpTreeNode::inObj = inObj;
	CComExpTreeNode::inMethod = inMethod;
	CComExpTreeNode::f = f;
	root->Flush();
}

