/////////////////////////////////////////////////////////////////////////////
//
// ComCompiler.cpp
//
// Implementace CComCompiler - hlavni trida kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ComCompiler.h"
#include "ComFunctions.h"
#include "register.h"
#include "ks.h"
#include "KerConstants.h"
#include <time.h>
#include "dialogs.h"
#include "ComKerServices.h"
#include "scriptedit.h"


#include "ComString.h"
#include "interpret.h"
#include "ident.h"


CComCompiler* compiler = 0;

CComCompiler::CComCompiler()
{
	uTab = oTab = nTab = dTab = gTab = pTab = 0;
	objBrowser = 0;

	lexical = 0;
	syntax = 0;
	semantic = 0;
	codegen = 0;
	csgen = 0;

	warningTable = 0;
	slowMode = false;

	toRemove = toModify = 0;

	Reset(true);
}


CComCompiler::~CComCompiler()
{
	SAFE_DELETE(lexical);
	SAFE_DELETE(syntax);
	SAFE_DELETE(semantic);
	SAFE_DELETE(codegen);
	SAFE_DELETE(csgen);

	SAFE_DELETE(objBrowser);

	SAFE_DELETE_ARRAY(warningTable);
}

void CComCompiler::Reset(bool resetUTab)
{
	CComSymbolTable* uTmp = uTab;
	CComConstantTable* cTmp = cTab;
	
	if(!resetUTab)
	{
		uTab = objBrowser->uTab = 0;
		cTab = objBrowser->cTab = 0;
	}

	SAFE_DELETE(objBrowser);

	if(resetUTab)
	{
		uTab = new CComSymbolTable(cUnknownsHTSize,tokUnknownFirst);
		cTab = new CComConstantTable(cConstantsHTSize,tokConstantFirst);
	} else
	{
		uTab = uTmp;
		cTab = cTmp;
	}

	oTab = new CComSymbolTable(cObjectsHTSize,tokObjectFirst);
	nTab = new CComSymbolTable(cNameHTSize,tokNameFirst);
	dTab = new CComSymbolTable(cDependHTSize,tokDependFirst);
	gTab = new CComSymbolTable(cGlobalHTSize,tokGlobalFirst);
	pTab = new CComSymbolTable(cPreprocessorHTSize,0);

	mTab = new CComSymbolTable(cGMethodsHTSize,tokMethodFirst);
	aTab = new CComSymbolTable(cGArgumentsHTSize,tokArgumentFirst);

	dnTab = new CComSymbolTable(cNameHTSize,tokDirectNameFirst);
	gnTab = new CComSymbolTable(cGlobalHTSize,tokGlobalNameFirst);
	sTab = new CComSymbolTable(cStructuresHTSize,tokStructureFirst);

	names = new CKerNamesMain();
	nameSearch = new CComNameSearch();
	objBrowser = new CComObjectBrowser(uTab,cTab,oTab,nTab,dTab,gTab,pTab,mTab,aTab,dnTab,gnTab,sTab,names,nameSearch);
	assert(objBrowser);

	SAFE_DELETE(lexical);
	SAFE_DELETE(syntax);
	SAFE_DELETE(semantic);
	SAFE_DELETE(codegen);
	SAFE_DELETE(csgen);

	lexical = new CComLexical(objBrowser, cLexicalQueueLn);
	assert(lexical);
	syntax = new CComSyntax(objBrowser, cSyntaxQueueLn);
	assert(syntax);
	syntax->slowMode = slowMode;
	if(toRemove && toModify)
	{
		SAFE_DELETE(syntax->toRemove);
		SAFE_DELETE(syntax->toModify);
		syntax->toRemove = toRemove;
		syntax->toModify = toModify;
	}
	semantic = new CComSemantic(objBrowser, cSemanticQueueLn, &(syntax->inheritedAttributes));
	assert(semantic);
	codegen = new CComCodeGen(objBrowser, cCodeGenQueueLn);
	assert(codegen);
	csgen = new CComCSGenerator(objBrowser, 1);

	warningNr = 0;
	SAFE_DELETE_ARRAY(warningTable);
	FillTabs();
}

CComError CComCompiler::GetWarning(int i)
{
	assert(i>=0 && i<GetWarningNr());
	return warningTable[i];
}

void CComCompiler::FillTabs()
{
	int i;
	char buf[cMaxIdentifierLn+4+1];

	for(i=dirFirst+1; i<dirLast; i++)
	{
		CComOBPreprocessor* newDir = new CComOBPreprocessor(cDirectives[i-dirFirst-1],pteDirective,i);
		assert(newDir);
		pTab->AddExclusive(newDir);
	}

	for(i=kwFirst+1; i<=kwString; i++)
	{
		CComOBPreprocessor* newKw = new CComOBPreprocessor(cKeywords[i-kwFirst-1],pteKeyword,i);
		assert(newKw);
		pTab->AddExclusive(newKw);
	}

	for(i=0; i<KERVARUSESSIZE; i++)
	{
		CComOBPreprocessor* newKn = new CComOBPreprocessor(KerVarUsesNames[i],pteKnownName,i+knFirst);
		assert(newKn);
		pTab->AddExclusive(newKn);
	}

	for(i=eKKNdefaultObject; i<MAXKNOWNNAMES; i++)
	{
		CComOBPreprocessor* newKn = new CComOBPreprocessor(KnownNames[i].namestr+6,pteKnownName,i+knSecond);
		assert(newKn);
		pTab->AddExclusive(newKn);
	}

	for(i=0; i<etLast; i++)
	{
		CComOBPreprocessor* newEt = new CComOBPreprocessor(cEditTags[i],pteEditTag,i);
		assert(newEt);
		pTab->AddExclusive(newEt);
	}

	for(i=0; i<cKerServicesNr; i++)
	{
		sprintf(buf, "_KN_%s", cKerServices[i].name);
		CComOBPreprocessor* newKS = new CComOBPreprocessor(buf,pteKernelService,i+ksFirst);
		assert(newKS);
		pTab->AddExclusive(newKS);
	}

	for(i=0; i<cKerConstantsNr; i++)
	{
		CComOBPreprocessor* newKC = new CComOBPreprocessor(cKerConstants[i].name,pteKernelConstant,i+kcFirst);
		assert(newKC);
		pTab->AddExclusive(newKC);
	}

	int nameID;
	for(i=eKKNconstructor; i<=eKKNdestructor; i++)
	{
		CComOBName* newName = new CComOBName(KnownNames[i].namestr, KnownNames[i].namestr, cUnknown, 0, CComShortPosition(), kwMethodName);
		assert(newName);

		nameID = nTab->Add(newName);
		if(i == eKKNdestructor)
		{
			int destructorID = uTab->Add("Destructor", new CComOBBase("Destructor", cUnknown, CComShortPosition()));
			CListK<CComNSENode> ident;
			ident.Add(destructorID);
			nameSearch->Add(&ident, newName, nameID);
			CComOBMethod* newMethod = new CComOBMethod("_KSID_Destructor", cUnknown, CComShortPosition(), "", 0, kwSafe, kwDestructor, true);
			newMethod->SetData(kwSafe, kwVoid, 0);
			assert(newMethod);
			mTab->Add(newMethod);
		}

		CKerName* newCKerName = new CKerName(KnownNames[i].namestr, names);
		assert(newCKerName);
		newCKerName->Type = KnownNames[i].type;
	}

	cTab->FillNullTokens();
	
	cTab->getcountID = uTab->FindOrAdd("GetCount", new CComOBBase("GetCount", cUnknown, CComShortPosition()));
	cTab->setcountID = uTab->FindOrAdd("SetCount", new CComOBBase("SetCount", cUnknown, CComShortPosition()));
}

void CComCompiler::CreateScripts(const char* source)
{
	char buf[cMaxFileNameLn];
	CFSRegister *r;
	int i, j, k, c;

	FS->ChangeDir("$SCRIPTS$");
	const char* versionStr = lexical->GetVersionStr();
	assert(versionStr);

	sprintf(buf, "%s.code", versionStr);
	r = new CFSRegister(buf,"KRKAL SCRIPT CODE",FSREGCLEARIT);
	assert(r);
	r->SetRegisterToBeCompressed();

	r->AddKey("Scripts Version",FSRTstring)->stringwrite(versionStr);

	_int64 version64 = 0;
	_int64 base = 1;
	for(i=(int)strlen(versionStr); i>0; i--)
	{
		c = *(versionStr+i-1);
		if(c != '_')
		{
			version64 += (isdigit(c) ? c-'0' : 10+toupper(c)-'A') * base;
			base *= 16;
		}
	}

	r->AddKey("Scripts Version 64",FSRTint64)->write64(version64);
	r->AddKey("Scripts Name",FSRTstring)->stringwrite(source);
	r->AddKey("Author",FSRTstring)->stringwrite(lexical->author);
	r->AddKey("Game",FSRTstring)->stringwrite(lexical->game);
	r->AddKey("Compiled Scripts Version",FSRTint)->writei(KS_VERSION);

	r->AddKey("Number of Ker Names",FSRTint)->writei(nTab->GetCount());
	CFSRegKey* p = r->AddKey("Ker Names",FSRTstring); 
	CComOBName* name;
	for(i=0; i<nTab->GetCount(); i++)
	{
		name = objBrowser->GetName(i);
		assert(name);
		p->stringwrite(name->kerName);
	}

	p = r->AddKey("Dependencies",FSRTint);
	for(i=0; i<dTab->GetCount(); i++)
	{
		CComOBNameDepend* dep = (CComOBNameDepend*)dTab->Get(tokDependFirst+i);
		assert(dep->father-tokNameFirst > 0);
		assert(dep->son-tokNameFirst > 0);
		p->writei(dep->father-tokNameFirst);
		p->writei(dep->son-tokNameFirst);
	}

	c = codegen->code->GetCount();

	CIntInstructionRec* codeBuf = new CIntInstructionRec[c];
	assert(codeBuf);
	for(i=0; i<c; i++)
		*(codeBuf+i) = codegen->code->Get(i);

	r->AddKey("Code",FSRTchar)->blockwrite(codeBuf,c*sizeof(CIntInstructionRec));
	SAFE_DELETE_ARRAY(codeBuf);

	CFSRegister *ko, *m, *km, *pr, *kpr, *ka, *ka1;
	CComOBObject* obj;
	CComOBMethod* met;
	CComOBArgument* arg;
	CComOBAttribute* atr;
	CComOBConstant* constant;

	// Objekty:
	CFSRegister *o = r->AddKey("Objects",FSRTregister)->GetSubRegister();
	for(i=0; i<oTab->GetCount(); i++)
	{
		obj = objBrowser->GetObject(i);
		ko = o->AddKey(obj->kerName,FSRTregister)->GetSubRegister();
		c = obj->kerNameID - tokNameFirst;
		assert(c>=0 && c<nTab->GetCount());
		ko->AddKey("Name",FSRTint)->writei(c);
		ko->AddKey("Edit Tag",FSRTint)->writei(obj->edit.objET);
		ko->AddKey("IS D Size",FSRTint)->writei(obj->isdSize);
		
		// edit tags objektu:
		if(obj->edit.userName)
			ko->AddKey("User Name", FSRTstring)->stringwrite(obj->edit.userName);
		if(obj->edit.comment)
			ko->AddKey("Comment", FSRTstring)->stringwrite(obj->edit.comment);

		if(obj->aTab->GetCount())
			if(obj->ksvg)
				ko->AddKey("KSVG",FSRTstring)->stringwrite(obj->ksvg->name); 

		// Data:
		ka = ko->AddKey("Data",FSRTregister)->GetSubRegister();
		k = 0;

		for(j=0; j<obj->aTab->GetCount(); j++)
		{
			atr = obj->GetAttribute(j);			assert(atr);
			if(atr->GetMDType() == cUnknown)
				continue;
			
			if(k<obj->gList.GetCount() && (atr==obj->gList[k]->members[0] || !strcmp(atr->kerName, obj->gList[k]->members[0]->kerName)))		// skupinove promenne
			{
				CComOBAttribute* grpAtr = obj->gList[k]->grpAtr;
				char* tmpStr;

				if(grpAtr->dType == cUnknown)
					tmpStr = newstrdup(cKeywords[obj->gList[k]->type-kwFirst-1]);	
				else
					tmpStr = obj->gList[k]->grpAtr->kerName;

				ka1 = ka->AddKey(tmpStr,FSRTregister)->GetSubRegister();

				if(grpAtr->dType == cUnknown)
					SAFE_DELETE_ARRAY(tmpStr);

				ka1->AddKey("Type", FSRTint)->writei(eKTvoid);
				switch(obj->gList[k]->type)
				{
				case kwPoint2D:		c = eKET2Dpoint;	break;
				case kwPoint3D:		c = eKET3Dpoint;	break;
				case kwCell2D:		c = eKET2Dcell;		break;
				case kwCell3D:		c = eKET3Dcell;		break;
				case kwArea2D:		c = eKET2Darea;		break;
				case kwArea3D:		c = eKET3Darea;		break;
				case kwCellArea2D:	c = eKET2DcellArea;	break;
				case kwCellArea3D:	c = eKET3DcellArea;	break;
				default:
					assert(false);
				}
				ka1->AddKey("Edit Type", FSRTint)->writei(c);
				if(grpAtr->edit.userName)
					ka1->AddKey("User Name", FSRTstring)->stringwrite(grpAtr->edit.userName);
				if(grpAtr->edit.comment)
					ka1->AddKey("Comment", FSRTstring)->stringwrite(grpAtr->edit.comment);
				k++; 
			}
			DoData(atr, ka);
		}

		// Skriptovane atributy:
		for(j = 0; j < obj->scrList.GetCount(); j++)
		{
			atr = obj->scrList[j];
			ka1 = ka->AddKey(atr->kerName,FSRTregister)->GetSubRegister();
			ka1->AddKey("Type", FSRTint)->writei(eKTvoid);
			ka1->AddKey("Edit Type", FSRTint)->writei(eKETscripted);
			if(atr->edit.userName)
				ka1->AddKey("User Name", FSRTstring)->stringwrite(atr->edit.userName);
			if(atr->edit.comment)
				ka1->AddKey("Comment", FSRTstring)->stringwrite(atr->edit.comment);

			assert(atr->knownName != cUnknown);
			ka1->AddKey("Method", FSRTint)->writei(atr->knownName-tokNameFirst);
			if(atr->kerNameID != cUnknown)
			{
				constant = (CComOBConstant*)cTab->Get(atr->kerNameID);
				ka1->AddKey("Param", FSRTint)->writei(constant->value.intData);
			}
		}

		// Metody:
		m = ko->AddKey("Methods",FSRTregister)->GetSubRegister();
		for(j=0; j< obj->mTab->GetCount(); j++)
		{
			met = obj->GetMethod(j);

			km = m->AddKey(met->kerKonkretniID,FSRTregister)->GetSubRegister();

			km->AddKey("Parent Object", FSRTstring)->stringwrite(met->objDef->kerName);

			if(met->storage == kwSafe)
			{
				c = nTab->Find(met->kerName)-tokNameFirst;
				assert(c>=0 && c<nTab->GetCount());
				km->AddKey("Name",FSRTint)->writei(c);
			} else
				km->AddKey("DirectName",FSRTstring)->stringwrite(met->kerName);
				
			km->AddKey("Compiled",FSRTchar)->writec(met->compiled ? 1 : 0); 
			km->AddKey("Safe",FSRTchar)->writec(met->storage==kwSafe ? 1 : 0);
            
			if(!met->compiled)
				km->AddKey("Jump",FSRTint)->writei(met->mem);

			if(met->returnsValue)
				km->AddKey("ReturnType",FSRTint)->writei(met->GetMDType());

			// Parametry:
			if(met->aTab->GetCount()==0 || met->storage==kwDirect)
				continue;
			pr = km->AddKey("Params",FSRTregister)->GetSubRegister();
			for(k=0; k<met->aTab->GetCount(); k++)
			{
				arg = met->GetArgument(k);
				
				sprintf(buf,"%d",k);
				kpr = pr->AddKey(buf,FSRTregister)->GetSubRegister();

				c = arg->kerNameID-tokNameFirst;
				assert(c>=0 && c<nTab->GetCount());

				kpr->AddKey("Name",FSRTint)->writei(c);
				kpr->AddKey("Type",FSRTint)->writei(arg->GetMDType() | arg->GetMDRet());
				DoDataDefaults(arg, kpr);
			}
		}
	}

	// Globals:
	CFSRegister *g = r->AddKey("Globals",FSRTregister)->GetSubRegister();
	CComOBGlobal* glb;
	for(i=0; i<gTab->GetCount(); i++)
	{
		glb = objBrowser->GetGlobal(i);
		assert(glb);
		if(glb->GetMDType() == cUnknown)
			continue;
		m = DoData(glb, g);

		if(CComPart::IsObjectToken(glb->dType))
		{
			obj = (CComOBObject*)oTab->Get(glb->dType);
			assert(obj);
			c = obj->kerNameID-tokNameFirst;
			assert(c>=0 && c<nTab->GetCount());
			m->AddKey("Object", FSRTint)->writei(c);
		}
	}

	r->WriteFile();
	delete r;
}

CFSRegister* CComCompiler::DoData(CComOBBaseData* d, CFSRegister* ka)
{
	CFSRegister *ka1, *e;
	CFSRegKey *e1;
	int k, c;

	ka1 = ka->AddKey(d->kerName,FSRTregister)->GetSubRegister();
	if(!d->compiled && d->mem != cUnknown)
		ka1->AddKey("Offset",FSRTint)->writei(d->mem);

	ka1->AddKey("Type",FSRTint)->writei(d->GetMDType());
	
	if(d->array.GetCount())
	{
		int arraySize = d->array[0];
		for(k=1; k < d->array.GetCount(); k++)
		{
			arraySize *= d->array[k];
		}

		ka1->AddKey("Array Size",FSRTint)->writei(arraySize);
	}

	if(d->edit.nameAuto != cUnknown)
	{
		c = d->edit.nameAuto - tokNameFirst;
		assert(c >= 0 && c < nTab->GetCount());
		ka1->AddKey("Auto Control Name", FSRTint)->writei(c);
	}

	if(d->use)
		ka1->AddKey("Use",FSRTint)->writei(d->use);

	e = ka1;
	if(d->edit.userName)
		e->AddKey("User Name", FSRTstring)->stringwrite(d->edit.userName);
	if(d->edit.comment)
		e->AddKey("Comment", FSRTstring)->stringwrite(d->edit.comment);

	DoDataDefaults(d, e);

	switch(d->edit.limits)
	{
	case etInterval:
		e->AddKey("Limits", FSRTint)->writei(eKLCinterval);
		switch(d->edit.valueType)
		{
		case kwChar:
			e1 = e->AddKey("Interval", FSRTchar);
			e1->writec(d->edit.interval[0].intData);
			e1->writec(d->edit.interval[1].intData);
			break;
		case kwInt:			
			e1 = e->AddKey("Interval", FSRTint);
			e1->writei(d->edit.interval[0].intData);
			e1->writei(d->edit.interval[1].intData);
			break;
		case kwDouble:
			e1 = e->AddKey("Interval", FSRTdouble);
			e1->writed(d->edit.interval[0].doubleData);
			e1->writed(d->edit.interval[1].doubleData);
			break;
		default:
			assert(false);
		}
		break;

	case etList:
		e->AddKey("Limits", FSRTint)->writei(eKLClist);
		switch(d->edit.valueType)
		{
		case kwChar:
			e1 = e->AddKey("List", FSRTchar);
			for(k=0; k<d->edit.list.GetCount(); k++)
				e1->writec(d->edit.list[k].intData);
			break;
		case kwInt:			
		case kwName:			
			e1 = e->AddKey("List", FSRTint);
			for(k=0; k<d->edit.list.GetCount(); k++)
				e1->writei(d->edit.list[k].intData);
			break;
		case kwDouble:
			e1 = e->AddKey("List", FSRTdouble);
			for(k=0; k<d->edit.list.GetCount(); k++)
				e1->writed(d->edit.list[k].doubleData);
			break;
		default:
			assert(false);
		}
		break;

	case etIs:
        switch(d->edit.isOp)
		{
		case opGreaterEqual:	case opGreater:			c = eKLCup;			break;
		case opLessEqual:		case opLess:			c = eKLCdown;		break;
		default:
			assert(false);
		}
		e->AddKey("Limits", FSRTint)->writei(c);
		switch(d->dType)
		{
		case kwDouble:	case kwDoubleArray:
			e->AddKey("Limit Name", FSRTdouble)->writed(d->edit.isName.doubleData);
			break;
		case kwChar:	case kwCharArray:
			e->AddKey("Limit Name", FSRTchar)->writec(d->edit.isName.intData);
			break;
		default:
			e->AddKey("Limit Name", FSRTint)->writei(d->edit.isName.intData);
		}
		break;

	default:
		;
	}
	if(d->edit.editType)
		e->AddKey("Edit Type", FSRTint)->writei(d->edit.editType);
	if(d->edit.nameMask)
		e->AddKey("Names Mask", FSRTint)->writei(d->edit.nameMask);
	if(d->edit.hasDefaultMember)
		switch(d->dType)
		{	
		case kwDoubleArray:
			e->AddKey("Default Member", FSRTdouble)->writed(d->edit.DefaultMember.doubleData);
			break;
		case kwCharArray:
			e->AddKey("Default Member", FSRTchar)->writec(d->edit.DefaultMember.intData);
			break;
		default:
			e->AddKey("Default Member", FSRTint)->writei(d->edit.DefaultMember.intData);
		}

	return ka1;
}

void CComCompiler::DoDataDefaults(CComOBBaseData* d, CFSRegister* e)
{
	int k, c;
	CFSRegKey *e1;

	switch(d->edit.valueType)
	{
	case kwChar:
		e1 = e->AddKey("Default", FSRTchar);
		for(k=0; k<d->edit.defaultValue.GetCount(); k++)
			e1->writec(d->edit.defaultValue[k].intData);
		break;
	case kwInt:		
		e1 = e->AddKey("Default", FSRTint);
		for(k=0; k<d->edit.defaultValue.GetCount(); k++)
			e1->writei(d->edit.defaultValue[k].intData);
		break;
	case kwDouble:
		e1 = e->AddKey("Default", FSRTdouble);
		e1->writed(0);
		for(k=1; k<d->edit.defaultValue.GetCount(); k++)
			e1->writed(d->edit.defaultValue[k].doubleData);
		break;
	case kwString:
		e1 = e->AddKey("Default", FSRTstring);
		for(k=0; k<d->edit.defaultValue.GetCount(); k++)
			e1->stringwrite(d->edit.defaultValue[k].strData);
		break;

	case kwName:		case kwMethodName:		case kwObjectName:		case kwParamName:
		if(d->edit.defaultValue[0].intData == cUnknown)
			e->AddKey("Default", FSRTint)->writei(cUnknown);
		else
		{
			e1 = e->AddKey("Default", FSRTint);
			for(k=0; k<d->edit.defaultValue.GetCount(); k++)
			{
				c = d->edit.defaultValue[k].intData;
				assert(c>=0 && c<nTab->GetCount());
				e1->writei(c);
			}
		}
		break;

	case kwObjptr:
		assert(d->edit.defaultValue[0].intData == 0);
		e->AddKey("Default", FSRTint)->writei(0);
		break;

	case cUnknown:
		break;


	default:
		if(CComPart::IsDynamicArrayToken(d->edit.valueType))
		{
			assert(d->edit.defaultValue[0].intData == 0);
			e->AddKey("Default", FSRTint)->writei(0);
		} else
			assert(false);
	}
}

bool CComCompiler::IsHighlight(const char* word, int* type)
{
	CComOBPreprocessor* p = (CComOBPreprocessor*) pTab->FindRec(word);
	if(!p && word[0] == '@')
	{
		char buf[cMaxFileNameLn+5];

		strcpy(buf, "_KN_");
		strcat(buf, word+1);
		p = (CComOBPreprocessor*) pTab->FindRec(buf);
	}

	if(type && p)
		*type = p->type;

	return p != 0;
}

int CComCompiler::ParseHead(char* buf, int bufSize, SComHeadInfo* info, char** codeStart)	// 0 = chyba
{
	Reset(true);

	if(lexical->StartFromMemory((const unsigned char*)buf, bufSize, "script editor window") == cUnknown)
		return false;

	try
	{
		lexical->DoReadHead(info);
	} catch (CComError e)
	{
		return 0;
	}

	*codeStart = buf + info->behindHeadIndex;
	return 1;
}

const char* CComCompiler::GetFileName(int tabID, char* buf, int bufSize, int* minBufSize)
{
	assert(buf);
	CComOBPreprocessor* p = (CComOBPreprocessor*)pTab->Get(tabID);
	assert(p && p->type == pteFileName);

	int minLen = strlen(lexical->GetStartDir()) + 1 + strlen(p->name) + 1;
	if(minLen >= bufSize)
	{
		if(minBufSize)
			*minBufSize = minLen;
		return 0;
	}

	strcpy(buf, lexical->GetStartDir());
	if(buf[0] && buf[strlen(buf)-1] != '\\')
		strcat(buf, "\\");
	strcat(buf, p->name);
	return buf;
}

int  CComCompiler::SECompile(CSEdScriptEdit *script, bool createCompiledScripts, bool showMsgBoxInfo, CComError* error, int* warningNr, CComError** warnings, bool slowMode)
{
	toRemove = toModify = 0;
	CComPart* last = SetupForOB(script, createCompiledScripts, true);
	try
	{
		if(lexical->StartFromWindow(script) == cUnknown)
			throw CComError(errCantOpenIncludeFile, noteNone);
		last->Run();
	} catch(CComError e)
	{
		syntax->DoPanicPickDirties();
		if(DirtyOpTest())
			e = DoSecondPassOB(script, createCompiledScripts);

		if(e.errorCode == errCycleInNameDependencies && !slowMode)
			return SECompile(script, createCompiledScripts, showMsgBoxInfo, error, warningNr, warnings, true);

		if(e.errorCode != errNone)
		{
			if(showMsgBoxInfo)
				DoError(e);
			if(error)
				*error = e;
			if(warningNr)
				*warningNr = 0;
			return 0;
		}
	}

	if(DirtyOpTest())
	{
		CComError e;
		e = DoSecondPassOB(script, createCompiledScripts);
		if(e.errorCode != errNone)
		{
			if(showMsgBoxInfo)
				DoError(e);
			if(error)
				*error = e;
			if(warningNr)
				*warningNr = 0;
			return 0;
		}
	} else
		if(showMsgBoxInfo)
			Success(script->GetFileName());

	if(warningNr)
		warningNr = 0;

	lexical->ReleaseOutput();
	syntax->ReleaseOutput();
	semantic->ReleaseOutput();
	codegen->ReleaseOutput();

	CreateScripts(script->GetFileName());

#ifdef CREATE_COMPILED_SCRIPTS
	if(createCompiledScripts)
		csgen->CreateCompiledScripts();
#endif

	return 1;
}

CComPart* CComCompiler::SetupForOB(CSEdScriptEdit *script, bool createCompiledScripts, bool resetUTab)
{
	Reset(resetUTab);

	const char* dir = script->GetPath();
	assert(dir);
	lexical->SetStartDir(dir);

	syntax->StartPart(lexical);
	semantic->StartPart(syntax);

	objBrowser->generatingCode = createCompiledScripts;

#ifdef JIRKA_KING_OF_COMPILERS
	FS->ChangeDir("$COMPFILES$");
	lexical->SetDebug("lexical.dbg", true);
	syntax->SetDebug("syntax.dbg", true);
	semantic->SetDebug("semantic.dbg", true);
	codegen->SetDebug("codegen.dbg", true);
#endif

	if(createCompiledScripts)
	{
		codegen->StartPart(semantic); 

		FS->ChangeDir("$COMPFILES$");
		syntax->SetOutput(cSyntaxOutputFile);
		semantic->SetOutput(cSemanticOutputFile);
		return codegen;
	} else
		return semantic;
}

CComError CComCompiler::DoSecondPassOB(CSEdScriptEdit *script, bool createCompiledScripts)
{
	SetupForSecondPass();
	CComPart* last = SetupForOB(script, createCompiledScripts, false);
	syntax->secondPass = true;
	try
	{
		if(lexical->StartFromWindow(script) == cUnknown)
			throw CComError(errCantOpenIncludeFile, noteNone);
		last->Run();
	} catch(CComError e)
	{
		if(DirtyOpTest())
			return GetUnresolvedDirtyOp();
		else
			return e;
	}

	if(DirtyOpTest())
		return GetUnresolvedDirtyOp();
	else
		return CComError(errNone);
}


int CComCompiler::TestComp(const char* source, bool slowMode)
{
	toRemove = toModify = 0;
	CComPart* last = SetupForComplete(source, true);
	try
	{
		last->Run();
	} catch(CComError e)
	{
		syntax->DoPanicPickDirties();

		if(DirtyOpTest())
			e = DoSecondPassComplete(source);

		if(e.errorCode == errCycleInNameDependencies && !slowMode)
			return TestComp(source, true);

		if(e.errorCode != errNone)
		{
			DoError(e);
			return cError;
		}
	}

	if(DirtyOpTest())
	{
		CComError e = DoSecondPassComplete(source);

		if(e.errorCode != errNone)
		{
			DoError(e);
			return cError;
		}
	} else
		Success(source);

	lexical->ReleaseOutput();
	syntax->ReleaseOutput();
	semantic->ReleaseOutput();
	codegen->ReleaseOutput();

	#ifdef JIRKA_KING_OF_COMPILERS
	FS->ChangeDir("$COMPFILES$");
	objBrowser->DebugToFile("objbrowser.dbg");
	#endif

	CreateScripts(source);
#ifdef CREATE_COMPILED_SCRIPTS
	csgen->CreateCompiledScripts();
#endif

	return cOK;
}

void CComCompiler::Success(const char* source)
{
	char *buff = new char[strlen(source) + 64];
	sprintf(buff, "Compiled successfuly!\n%s", source);
	GUIMessageBox("OK", buff);
	SAFE_DELETE_ARRAY(buff);
}

bool CComCompiler::DirtyOpTest()
{
	return syntax->toRemove->GetCount() != syntax->removedNr || 
			syntax->toModify->GetCount() != syntax->modifiedNr;
}

CComError CComCompiler::GetUnresolvedDirtyOp()
{
	CComOBBase* bp;
	CComSymbolTable *tab1, *tab2;

	if(syntax->toRemove->GetCount() != syntax->removedNr)
	{
		tab1 = syntax->toRemove;
		tab2 = syntax->removed;
	}
	else
	{
		tab1 = syntax->toModify;
		tab2 = syntax->modified;
	}

	for(int i = 0; i<tab1->GetCount(); i++)
	{
		bp = tab1->Get(i);

		if(CComPart::IsConstantToken(bp->version))
		{
			CComOBBase* bp2;
			if((bp2 = tab2->FindRec(bp->name)) == 0)
				break;
			if(bp2->version != bp->version)
				break;
		} else
			if(tab2->Find(bp->name) == cUnknown)
				break;
	}
	CComError e = CComError(errUnresolvedDirtyOp, noteNone, 0, &(bp->posDefined));
	e.pos.file = newstrdup(pTab->Get(e.pos.fileID)->name);
	return e;
}

CComPart* CComCompiler::SetupForComplete(const char* source, bool resetUTab)
{
	char file[cMaxFileNameLn], path[cMaxFileNameLn];

	Reset(resetUTab);
	getFileParts(source,file,path);

	FS->ChangeDir("$GAME$");
	FS->ChangeDir(path);

	lexical->SetStartDir(path);
	if(lexical->StartFile(source) == cUnknown)
		throw CComError(errCantOpenIncludeFile, noteNone, source);

	FS->ChangeDir("$COMPFILES$");
	syntax->SetOutput(cSyntaxOutputFile);
	syntax->StartPart(lexical);
	semantic->SetOutput(cSemanticOutputFile);
	semantic->StartPart(syntax);
	codegen->StartPart(semantic); 

	objBrowser->generatingCode = true;
	CComPart* last = codegen;

#ifdef JIRKA_KING_OF_COMPILERS
	FS->ChangeDir("$COMPFILES$");
	lexical->SetDebug("lexical.dbg", true);
	syntax->SetDebug("syntax.dbg", true);
	semantic->SetDebug("semantic.dbg", true);
	codegen->SetDebug("codegen.dbg", true);
#endif

	return last;
}

CComError CComCompiler::DoSecondPassComplete(const char* source)
{
	SetupForSecondPass();
	CComPart* last = SetupForComplete(source, false);
	syntax->secondPass = true;
	try
	{
		last->Run();
	} catch(CComError e)
	{
		if(DirtyOpTest())
			return GetUnresolvedDirtyOp();
		else
			return e;
	}

	if(DirtyOpTest())
		return GetUnresolvedDirtyOp();
	else
		return CComError(errNone);
}

void CComCompiler::SetupForSecondPass()
{
	toRemove = syntax->toRemove;
	toModify = syntax->toModify;
	syntax->toRemove = 0;
	syntax->toModify = 0;
}


void CComCompiler::DoError(CComError e)
{
	if(e.errorCode == errNone)
		return;

	char buf[cMaxErrorMsgLn+1024], msg[cMaxErrorMsgLn], note[cMaxErrorMsgLn];

	FS->ChangeDir("$COMPILER$");
	if(e.GetErrorMsg("errors.eng", msg, e.errorCode) == cError)
		assert(false);

	sprintf(buf,"Error %d at line %d, column %d in file %s\n%s ",
			e.errorCode,e.pos.line,e.pos.column,e.pos.file,msg);
	
	if(e.intData != noteNone) 
	{
		FS->ChangeDir("$COMPILER$");
		e.GetErrorMsg("notes.eng",msg,e.intData);
		sprintf(note,"\nNote: %s",msg);
		strcat(buf,note);
	}

	if(e.strData)
	{
		strcat(buf,"\n");
		strcat(buf,e.strData);
	}

	if(e.errorCode<errWarningFirst)
		GUIMessageBox("Error found", buf);
	else
		GUIMessageBox("Warning", buf);
}