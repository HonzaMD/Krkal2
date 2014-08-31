/////////////////////////////////////////////////////////////////////////////
//
// ComCSGenerator.h
//
// Implementace CComCSGenerator - trida pro generovani kompilovanych skriptu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComCSGenerator.h"
#include "ident.h"
#include "fs.h"
#include "ks.h"
#include "KerConstants.h"
#include "ComKerServices.h"
#include "time.h"
#include "ComString.h"


CComCSGenerator::CComCSGenerator(CComObjectBrowser* ob, int _queueSize)
: CComPart(ob, _queueSize)
{
	look = cUnknown;
}

CComCSGenerator::~CComCSGenerator()
{
}

void CComCSGenerator::CreateCompiledScripts()
{
	int i, j, k, l, ksvg, firstFreeKSVG, scriptNr;
	CComOBBaseData* d;
	CComOBBaseData* d2;
	CComOBObject* o;
	CComOBMethod* m;
	CComOBAttribute* atr;
	CComOBArgument* arg;
	CComOBStructure* str;
	char ksvgstr[1024];
	char* tmpStr;
	FILE* fo;
	FILE* fo2;
	
	CComHashTable* tmpTable = new CComHashTable(256);
	CComHTE* hte;

	scriptNr = KS_VERSION+1;
	FS->ChangeDir("$KRKAL$\\KScripts");
	FS->GetCurDir(&tmpStr);
	sprintf(ksvgstr, "%s\\script%d.cpp", tmpStr, scriptNr);
	FILE* f = fopen(ksvgstr, "w+t");
	assert(f);
	SAFE_DELETE(tmpStr);


	fprintf(f,"\n////////////////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///		Compiled Scripts - File %d", scriptNr);
	fprintf(f,"\n///");
	fprintf(f,"\n///		A: Generated automatically");
	fprintf(f,"\n///");
	fprintf(f,"\n////////////////////////////////////////////////////////////////////////////////");

	fprintf(f,"\n\n\n\n////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///		 Ker Names");
	fprintf(f,"\n///");
	fprintf(f,"\n////////////////////////////////////////////////////////////////////\n\n");

	fo = StartFile("assign_names.cpp");
	fo2 = StartFile("assign_KSIDs.cpp");

	for(i=0; i < nTab->GetCount(); i++)
	{
		d = (CComOBBaseData*) nTab->Get(tokNameFirst+i);
		if(KSMain->KSIDs->Member(d->kerName)==0 && d->dType != kwVoidName)
		{
			fprintf(f, "CKerName* %s = 0;\n", d->kerName);
			fprintf(fo, "\tif (name = KerMain->KerNamesMain->GetNamePointer(\"%s\")) %s = name;\n", d->kerName, d->kerName);

			fprintf(fo2, "\tnew CKSKSID(\"%s\");\n", d->kerName);
		}
	}

	FinishFile(fo);
	FinishFile(fo2);

	fprintf(f,"\n\n\n////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///  VNITRNI STRUKTURY OBJEKTU");

	fprintf(f,"\n\n// typy se budou dat menit jen jako celky\n\n");

	for(j=0; j<sTab->GetCount(); j++)
	{
		str = objBrowser->GetStructure(j);		
		
		if(tmpTable->Member(str->kerName))
			continue;

		fprintf(f,"struct %s {\n", str->kerName);
		for(k=0; k<str->mTab->GetCount(); k++)
		{
			d2 = (CComOBBaseData*)str->mTab->Get(k);
		
			fprintf(f, "\t%s", GetTypeString(d2->dType));

			for(l=0; l<d2->GetPointer(); l++)
				fprintf(f,"*");
			fprintf(f," %s;\n", d2->name);
		}
		fprintf(f,"};\n");

		hte = new CComHTE(str->kerName, 0);
		hte->AddToHashTable(tmpTable);
	}


	fprintf(f,"\n\n\n////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///			Globals");
	fprintf(f,"\n///");
	fprintf(f,"\n////////////////////////////////////////////////////////////////////\n\n");

	fo = StartFile("assign_KSGs.cpp");

	for(i=0; i < gTab->GetCount(); i++)
	{
		d = (CComOBBaseData*) gTab->Get(tokGlobalFirst+i);
		if(KSMain->KSGs->Member(d->kerName) == 0)
		{
			fprintf(f, "void* %s = 0;\n", d->kerName);
			fprintf(fo, "\tnew CKSKSG(\"%s\",&%s);\n",d->kerName,d->kerName);
		}
	}

	FinishFile(fo);

	fprintf(f,"\n\n\n////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///		 Object Variables");
	fprintf(f,"\n///");
	fprintf(f,"\n////////////////////////////////////////////////////////////////////");

	fo = StartFile("assign_KSOVs.cpp");

	fprintf(f,"\n\n\n///////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n// Pointers to Object Variables\n\n\n");

	for(i=0; i<oTab->GetCount(); i++)
	{
		inObj = o = objBrowser->GetObject(i);		
		
		for(j=0; j<o->aTab->GetCount(); j++)
		{
			atr = o->GetAttribute(j);

			if(KSMain->KSOVs->Member(atr->kerName) ||  tmpTable->Member(atr->kerName))
				continue;

			fprintf(f, "void* %s;\n", atr->kerName);
			fprintf(fo, "\tnew CKSKSOV(\"%s\",&%s);\n", atr->kerName, atr->kerName);
			hte = new CComHTE(atr->kerName, 0);
			hte->AddToHashTable(tmpTable);
		}
	}
	FinishFile(fo);

	fprintf(f,"\n\n\n//////////////////////////////////////////////////////////////////////");
	fprintf(f,"\n// KSVGs\n\n\n");

	fo = StartFile("assign_KSVGs.cpp");

	firstFreeKSVG = KS_NUM_VG;

	for(i=0; i<oTab->GetCount(); i++)
	{
		ksvg = FindKSVG(o);
		if(ksvg == cUnknown)
			ksvg = firstFreeKSVG++;

		inObj = o = objBrowser->GetObject(i);		

		if(o->aTab->GetCount() == 0)
			continue;
		sprintf(ksvgstr,"_KSVG_%d_%s_%s", ksvg, o->name, uTab->Get(o->version)->name);

		if(KSMain->KSVGs->Member(ksvgstr))
			continue;

		fprintf(f,"struct %s {\n", ksvgstr);
		for(j=0; j<o->aTab->GetCount(); j++)
		{
			atr = o->GetAttribute(j);
			
			if(IsStringType(atr->dType))
				fprintf(f, "\tchar %s[%d]", atr->kerName, atr->dType-kwString+1);
			else
			{
				fprintf(f, "\t%s", GetTypeString(atr->dType));

				for(k=0; k < atr->GetPointer()-atr->array.GetCount(); k++)
					fprintf(f, "*");
				fprintf(f, " %s", atr->kerName);
			}
			
			for(k=0; k < atr->array.GetCount(); k++)
				fprintf(f, "[%d]", atr->array[k]);

			fprintf(f, ";\n");
		}
		fprintf(f,"};\n\n");

		fprintf(f,"void* KSAlloc%s() {\n", ksvgstr);
		fprintf(f,"\treturn new %s;\n", ksvgstr);
		fprintf(f,"}\n\n");

		fprintf(f,"void KSSetV%s(void* KSVG) {\n", ksvgstr);
		fprintf(fo, "\ttmp = new CKSKSVG(\"%s\",&KSAlloc%s,&KSSetV%s,sizeof(%s));\n", ksvgstr, ksvgstr, ksvgstr, ksvgstr);
		for(j=0; j<o->aTab->GetCount(); j++)
		{
			atr = o->GetAttribute(j);
			fprintf(f, "\t%s = &( (%s*)KSVG)->%s;\n", atr->kerName, ksvgstr, atr->kerName);
			fprintf(fo, "\ttmp->AddAttribute(\"%s\");\n", atr->kerName);
		}
		fprintf(fo, "\n");
		fprintf(f,"}\n\n");

	}

	FinishFile(fo);

	fprintf(f,"\n\n\n///////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///		Pointers To Direct Call Methods");
	fprintf(f,"\n///");
	fprintf(f,"\n///////////////////////////////////////////////////////////////////\n\n");

	fo = StartFile("assign_KSDMs.cpp");

	tmpTable->DeleteAllMembers();

	for(i=0; i<oTab->GetCount(); i++)
	{
		inObj = o = objBrowser->GetObject(i);		
		for(j=0; j<o->mTab->GetCount(); j++)
		{
			m = o->GetMethod(j);			
			if(KSMain->KSDMs->Member(m->kerName) || tmpTable->Member(m->kerName))
				continue;
			if(m->storage == kwDirect)
			{
				fprintf(f, "%s", GetTypeString(m->dType));
				for(l=0; l<m->GetPointer(); l++)
					fprintf(f,"*");
				fprintf(f, " (*%s)(int __CodeLine, OPointer __thisO", m->kerName);
				
				for(k=0; k<m->aTab->GetCount(); k++)
				{
					arg = m->GetArgument(k);			
					fprintf(f, ", %s", GetTypeString(arg->dType));
	
					for(l=0; l<arg->GetPointer(); l++)
						fprintf(f,"*");

					fprintf(f, " %s", arg->kerName);
				}
				fprintf(f, ") = 0;\n");

				fprintf(fo,"\tnew CKSKSDM(\"%s\",(void (**)())&%s);\n", m->kerName, m->kerName);
				
				hte = new CComHTE(m->kerName, 0);
				hte->AddToHashTable(tmpTable);
			}
		}
	}

	FinishFile(fo);

	fprintf(f,"\n\n\n///////////////////////////////////////////////////////////////////");
	fprintf(f,"\n///");
	fprintf(f,"\n///		 Object Methods");
	fprintf(f,"\n///");
	fprintf(f,"\n////////////////////////////////////////////////////////////////////\n\n");

	fo = StartFile("assign_KSMs.cpp");

	for(i=0; i<oTab->GetCount(); i++)
	{
		inObj = o = objBrowser->GetObject(i);		
		for(j=0; j<o->mTab->GetCount(); j++)
		{
			m = o->GetMethod(j);			
			if(m->storage==kwSafe ? KSMain->KSMs->Member(m->kerKonkretniID) : KSMain->KSDMs->Member(m->kerName))
				continue;

			if(m->inherit == inhInherited)		// zdedene metody znovu nevypisuji
				continue;

			fprintf(f,"//%s::%s\n", o->kerName, m->kerName);
			if(m->storage == kwDirect)
			{
				fprintf(f, "%s", GetTypeString(m->dType));
				for(l=0; l<m->GetPointer(); l++)
					fprintf(f,"*");
				fprintf(f, " %s(", m->kerKonkretniID);
			
				fprintf(f, "int __CodeLine, OPointer __thisO");
				for(k=0; k<m->aTab->GetCount(); k++)
				{
					arg = m->GetArgument(k);			
					fprintf(f, ", %s", GetTypeString(arg->dType));
					for(l=0; l<arg->GetPointer(); l++)
						fprintf(f,"*");

					fprintf(f, " %s", arg->kerName);
				}
				fprintf(f, ") {\n");
			} else
				fprintf(f, "void %s() {\n", m->kerKonkretniID);

			if(m->storage == kwDirect)
			{
				fprintf(f, "\tDIRECT_CALL_BEGIN(%s,\"%s\",", o->kerName, m->kerKonkretniID);
				if(!m->returnsValue)
					fprintf(f, ";)\n");
				else
					fprintf(f, "0)\n");
			}

			DoMethodBody(o, m, f);
			// ... kod metody

			if(m->storage == kwDirect)
				fprintf(f, "\tDIRECT_CALL_END\n");

			if(m->storage == kwDirect)
				fprintf(fo, "\tnew CKSKSM(\"%s\",(void(*)())%s);\n", m->kerKonkretniID, m->kerKonkretniID);
			else
				fprintf(fo, "\tnew CKSKSM(\"%s\",%s);\n", m->kerKonkretniID, m->kerKonkretniID);

			fprintf(f, "}\n\n");
		}
	}

	fclose(f);		// scriptXXX.cpp
	FinishFile(fo);

	// allscripts:
	FS->ChangeDir("$KRKAL$\\KScripts");
	fo = StartFile("allscripts.cpp",&fo2,false);
	fseek(fo, -(int)strlen("// STOP TAG: }"), SEEK_CUR);
	sprintf(ksvgstr,"\n#include \"script%d.cpp\"\n", scriptNr);
	fprintf(fo, ksvgstr);
	fprintf(fo, "// STOP TAG: }\n");
	FinishFile(fo, fo2);

	// ks_version:
	FS->ChangeDir("$KRKAL$\\KScripts\\New");
	fo = fopen("ks_version.h", "wt");
	assert(fo);

	fprintf(fo,"///////////////////////////////////////////////////////////////////////////////////\n");
	fprintf(fo,"///\n");
	fprintf(fo,"///		Version of Compilated Scripts and \n");
	fprintf(fo,"///		Number of Variable Groups\n");
	fprintf(fo,"///\n");
	fprintf(fo,"///		A: Generated automatically\n");
	fprintf(fo,"///\n");
	fprintf(fo,"///////////////////////////////////////////////////////////////////////////////////\n\n\n");

	fprintf(fo,"#define KS_VERSION %d\n", KS_VERSION+1);
	fprintf(fo,"#define KS_NUM_VG %d\n", firstFreeKSVG);
	fprintf(fo,"#define KS_KSVGs_HTS %d\n", 128);
	fprintf(fo,"#define KS_KSGs_HTS %d\n",64);
	fprintf(fo,"#define KS_KSDMs_HTS %d\n",16);
	fprintf(fo,"#define KS_KSMs_HTS %d\n",256);
	fprintf(fo,"#define KS_KSOVs_HTS %d\n",256);
	fprintf(fo,"#define KS_KSIDs_HTS %d\n",256);
	fclose(fo);

	tmpTable->DeleteAllMembers();
	SAFE_DELETE(tmpTable);

	CreateKernelServiceHandlerProc();
}

void CComCSGenerator::CreateKernelServiceHandlerProc()
{
	int i, j, k, l, firstDefault, paramCount, specParams, type, pointer; 
	FS->ChangeDir("$COMPFILES$");

	FILE* f = fopen("kernelserv.txt", "wt");

	fprintf(f, "\tswitch(kerServiceID)\n");
	fprintf(f, "\t{\n");


	for(i = 0; i < cKerServicesNr; i++)
	{
		fprintf(f, "\tcase %d: \t\t //%s\n", i, cKerServices[i].name);
		
		firstDefault = paramCount = cKerServices[i].paramsNr;
		specParams = 0;

		for(j = 0; j < paramCount; j++)
			if(cKerServiceParams[i][j].isDefault)
				firstDefault = min(firstDefault, j);
			else
				if(cKerServiceParams[i][j].specUse)
					specParams++;

		
		fprintf(f, "\t\t");
		for(j = firstDefault; j <= paramCount; j++)
		{
			fprintf(f, "if(paramCount == %d)\n", j - specParams);

			type = cKerServices[i].returnValue.type;
			pointer = cKerServices[i].returnValue.pointer;
			
			fprintf(f, "\t\t\t");

			if(pointer)
				if(type == kwObjptr)
					fprintf(f, "op[0].intData = ");
				else
					fprintf(f, "op[0].voidPtrData = ");
			else
				if(type == kwDouble)
					fprintf(f, "op[0].doubleData = ");
				else
					if(type != kwVoid)
						fprintf(f, "op[0].intData = ");


			fprintf(f, "%s(", cKerServices[i].compileToStr);

			for(k = 0; k < j; k++)
			{
				type = cKerServiceParams[i][k].type;
				pointer = cKerServiceParams[i][k].pointer;

				if(pointer)
				{
					fprintf(f, "(%s", GetTypeString(type, isUCParam(i, k)));
					if(!IsStringType(type))
						for(l = 0; l < pointer - (type == kwObjptr ? 1 : 0); l++)
							fprintf(f, "*");
					fprintf(f, ")");
				}
				fprintf(f, "op[%d].", k);

				if(type == kwDouble && pointer == 0)
					fprintf(f, "doubleData");
				else
					if(pointer > 1 || pointer == 1 && (type == kwName || IsDynamicArrayToken(type)))
						fprintf(f, "voidPtrData");
					else
						fprintf(f, "intData");
			
				if(k + 1 < j)
					fprintf(f, ", ");
			}

			fprintf(f, ");\n");		// konec volani sluzby
			fprintf(f, "\t\telse ");
		}
		fprintf(f, "\n\t\t\tKerMain->Errors->LogError(eKRTECorruptedInstr);\n");
		
		fprintf(f, "\t\tbreak;\n\n");
	}

	fprintf(f, "\tdefault:\n");
	fprintf(f, "\t\tKerMain->Errors->LogError(eKRTECorruptedInstr);\n");
	fprintf(f, "\t}\n");		// konec switche

	fclose(f);
}

FILE* CComCSGenerator::StartFile(const char* inFile, FILE** fIn, bool log)
{
	int i;
	char tbuf[32], dbuf[32], tmp[cMaxFileNameLn+1];

	FILE* fi = fopen(inFile, "rt");
	sprintf(tmp, "New\\%s", inFile);
	FILE* fo = fopen(tmp, "wt");
	assert(fi && fo);

	while((i = fgetc(fi)) != '}')
	{
		assert(i != EOF);	
		fputc(i, fo);
	}
	
	if(fIn)
		*fIn = fi;
	else
		fclose(fi);

	if(log)
	{
		_strdate(dbuf);
		_strtime(tbuf);
		fprintf(fo, "\n\t// New lines added %s at %s.\n\n", dbuf, tbuf);
	}
	return fo;
}

void CComCSGenerator::FinishFile(FILE* f, FILE* fIn)
{
	int i;
	if(fIn)
		while((i = fgetc(fIn)) != EOF)
			fputc(i, f);
	else
		fprintf(f, "}\n");
	fclose(f);
}

int CComCSGenerator::FindKSVG(CComOBObject* o)
{
	int ret, ksvgID;
	CKSKSVG *bestKSVG, *pKSVG;
	char buf[2048];

	ksvgID = 0;
	bestKSVG = 0;
	ret = cUnknown;
	while(ksvgID < KS_NUM_VG)
	{
		sprintf(buf, "_KSVG_%d_%s_%s", ksvgID, o->name, uTab->Get(o->version)->name);
		if(pKSVG = (CKSKSVG*)KSMain->KSVGs->Member(buf))
			if(o->FitAllInKSVG(pKSVG) && (!bestKSVG || pKSVG->Size < bestKSVG->Size))
			{
				bestKSVG = pKSVG;
				ret = ksvgID;
			}
		ksvgID++;
	} 


	return ret;
}

void CComCSGenerator::DoMethodBody(CComOBObject* o, CComOBMethod* m, FILE* f)
{
	FS->ChangeDir("$COMPFILES$");

	FILE* src = fopen(cSemanticOutputFile,"rb");
	assert(src);

	inObj = o;
	inMethod = m;
	tab = 0;
	of = f;
	inputFile = src;
	inObjID = oTab->Find(inObj->name);
	if(SeekToMethod(o, m, src))
	{
		// Preskocim pripadne nastaveni defaultnich parametru az k oteveraci zavorce tela funkce:
		while(look != synStartBlock)			
			Step();				

		Step();			// oteviraci slozena zavorka tam uz je
		while(look != synEndBlock)
			DoCommand();
		fclose(src);
	}
}

bool CComCSGenerator::SeekToMethod(CComOBObject* o, CComOBMethod* m, FILE* f)
{
	bool oFound, mFound;

	assert(f);
	oFound = mFound = false;

	rewind(f);

	while(!oFound)
	{
		while(!oFound)
		{
			while(look!=synStartObjectDecl && look!=lexEOF)
				Step();
			
			if(look == lexEOF)
				return false;
			else
				Step();

			assert(IsObjectToken(look));
			oFound = (oTab->Get(look) == o);
		}
		

		while(!mFound && look!=lexEOF)
		{
			while(look != synMethodDecl && look != synStartObjectDecl)
				Step();

			if(look == synStartObjectDecl)
			{
				oFound = false;			
				break;
			}
			Step();
			assert(IsMethodToken(look));
			mFound = (o->mTab->Get(look) == m);
		}
	}

	if(mFound)
		Step();

	return mFound;
}

void CComCSGenerator::Step()
{
	int test = fread(&look, sizeof(int), 1, inputFile);
	assert(test == 1);
}

void CComCSGenerator::DoCommand()
{
	bool shift = look!=synStartBlock;
	if(shift)
		tab++;

	switch(look)
	{
	case synStartBlock:
		Step();
		Tab(); fprintf(of, "{\n"); 

		while(look != synEndBlock)
			DoCommand();

		Tab(); fprintf(of, "}\n");
		Step();			// pryc s koncem bloku
		break;

	case synEndBlock:
		assert(false);

	case lexSeparator:
		Step();
		Tab(); fprintf(of,";\n"); 
		break;

	case kwIf:
		Step();
		Nl(); Tab(); fprintf(of,"if("); DoExpressionMain(); fprintf(of,")\n");
		DoCommand();

		if(look == kwElse)
		{
			Step();
			Tab(); fprintf(of,"else\n");
			DoCommand();
		}
		break;	

	case kwWhile:
		Step();
		Nl(); Tab(); fprintf(of,"while("); DoExpressionMain(); fprintf(of,")\n");
		DoCommand();
		break;

	case kwDo:
		Step();
		Nl(); Tab(); fprintf(of,"do\n"); 
		DoCommand();
		assert(look == kwWhile);
		Step();
		Tab(); fprintf(of,"while(");
		DoExpressionMain(); fprintf(of,");\n");
		break;

	case kwFor:
		Step();
		Nl(); Tab(); fprintf(of,"for("); DoExpressionMain();
		fprintf(of," ;"); DoExpressionMain();
		fprintf(of," ;"); DoExpressionMain(); fprintf(of, ")\n");
		DoCommand();
		break;

	case kwSwitch:
		Step();			// pryc s kwSwitch
		Step();			// pryc s poctem case
		Nl(); Tab(); fprintf(of,"switch("); DoExpressionMain(); fprintf(of, ")\n");
		Tab(); fprintf(of, "{\n");

		while(look == kwCase)
		{
			Tab(); fprintf(of, "case ");
			Step();		// pryc s kwCase
			DoExpressionMain();
			fprintf(of, ":\n");
			while(look != synEndCase)
				DoCommand();
			Step();		// pryc se synEndCase
		}

		if(look == kwDefault)
		{
			Step();		// pryc s kwDefault		
			Tab(); fprintf(of, "default:\n");
			while(look != synEndCase)
				DoCommand();
			Step();		// pryc se synEndCase
		}
		
		Tab(); fprintf(of, "}\n");
		break;

	case kwReturn:
		DoReturn();
		break;

	case kwBreak:
		Step();
		Tab();	fprintf(of, "break;\n");
		break;
	
	case kwContinue:
		Step();
		Tab();	fprintf(of, "continue;\n");
		break;

	default:
		Tab();
		if(look == synLocalDefinition)
			DoLocalDef();
		else
		{
			DoExpressionMain();
			if(look == lexSeparator)
				Step();
		}
		fprintf(of,";\n");
	}


	if(shift)
		tab--;
}

void CComCSGenerator::DoReturn()
{
	int i;
	Step();
	Tab(); 
	if(inMethod->storage == kwDirect)
	{
		if(look != kwVoid)
		{
			fprintf(of, "{\n");
			Tab(); fprintf(of, "\t%s", GetTypeString(inMethod->dType));
			for(i=0; i<inMethod->GetPointer(); i++)
                fprintf(of,"*");
			fprintf(of, " _KSTMP_RET = ");
			DoExpressionMain();
			fprintf(of, ";\n");
			Tab();	fprintf(of, "\tDIRECT_CALL_END\n");
            Tab();	fprintf(of, "\treturn _KSTMP_RET;\n");
		} else
		{
			Step();
			fprintf(of, "{\n");
			Tab();	fprintf(of, "\tDIRECT_CALL_END\n");
			Tab();	fprintf(of, "\treturn; \n"); 
		}
		Tab();	fprintf(of, "}\n"); 
	} else
	{	// safe
		if(look == kwVoid)
		{
			Step();
			fprintf(of,"return;\n"); 
		} else
		{
			fprintf(of,"{ KER_RETARG(%s",GetTypeString(inMethod->dType));
			for(i=0; i<inMethod->GetPointer(); i++)
                fprintf(of,"*");
			fprintf(of,") = ");
			DoExpressionMain();
			fprintf(of, "; return; }\n");
		}
	}	
}


void CComCSGenerator::DoExpressionMain()
{
	es.Reset();		
	lastObject = cUnknown;
	DoExpression();
	if(!es.expStack.IsEmpty())
	{
		tree.CreateFromStack(&es);
		tree.FlushInfixToFile(this, inObj, inMethod, of);
	}
}

void CComCSGenerator::DoLocalDef()
{
	int i, line, old;
	CComOBBaseData* d;

	assert(look == synLocalDefinition);
	Step();
	old = look;
	Step();
	assert(IsDataToken(look));
	line = look - tokDataFirst;
	Step();

	d = (CComOBBaseData*)GetTab(old)->Get(old);

	if(IsStringType(d->dType) && !d->pointer)
		fprintf(of, "CComString %s(%d, %d)", d->kerName, d->dType - kwString, line);
	else
	{
		fprintf(of, GetTypeString(d->dType));

		for(i=0; i<d->GetPointer()-d->array.GetCount(); i++)
			fprintf(of, "*");
	
		fprintf(of, " %s", d->kerName);
		for(i=0; i<d->array.GetCount(); i++)
			fprintf(of, "[%d]", d->array[i]);
	}
}

void CComCSGenerator::DoExpression()
{
	int old, type;
	CComSymbolTable* tab;

	assert(look==synStartExpression);
	Step();							// pryc se synStartExpression
	assert(IsDataToken(look));
	pos.line = look - tokDataFirst;
	Step();							// pryc s radkou

	while(look != synEndExpression)
	{
		old = look;	
		switch(old)
		{
		case synStartExpression:
			DoExpression();
			// zavorka jako unarni operator - vyraz v zavorce
			es.Push(CComExpression(opParentStart,false,false,false,es.expStack.el->data.type,1));	
			break;

		case synLocalizedArgument:		case synAssigning:		case synAddressing:
		case synLogicOr:				case synLogicAnd:
		case synConditional1:			case synConditional2:
		case synSafeArgument:
			Step();
			break;

		case opParentStart:
			DoMethodCall();
			break;

		case kwNew:
			DoNew();
			break;

		case kwDelete:
			DoDelete();
			break;

		case kwThis:
			Step();
			es.Push(CComExpression(kwThis,false,false,false, inObjID,0));
			break;

		case kwSender:
			Step();
			es.Push(CComExpression(kwSender,false,false,false, kwSender,0));
			break;

		case kwAssigned:
			DoAssigned();
			break;

		case kwTypeOf:
			Step();
			DoExpression();
			es.Push(CComExpression(kwTypeOf,false,false,false,kwName,1));
			break;

		default:
			type = cUnknown;
			tab = GetTab(look);
			if(tab)
				type = ((CComOBBaseData*)tab->Get(look))->dType;
			else
				if(IsKnownNameToken(look))
					type = kwName;

			es.Push(CComExpression(look,false,false,false,type,IsOperatorToken(look) ? GetOpArity(look) : 0, &pos));
			Step();
		}

		if(IsOperatorToken(old))	// vypusteni ciloveho typu a pointeru
			if(old!=opDoubleDot)
			{
				es.expStack.el->data.type = look;			// predani typu u operatoru
				Step();
				Step();							// pryc s pointerem
				Step();							// pryc s udajem o lvalue
			} else
			{
				look -= tokDataFirst;
				if(look == kwFirst)
					look = cUnknown;
				es.expStack.el->data.value.intData = look;	// predani informace o typu retu
				Step();				// pryc s typem retu
			}

	}
	Step();		// pryc se synEndExpression
}

void CComCSGenerator::DoNew()
{
	int line, type, pointer, id;
	bool vartype;

	assert(look == kwNew);
	Step();			// pryc s new
	assert(IsDataToken(look));
	line = look - tokDataFirst;
	Step();			// pryc s radkou
	
	id = type = pointer = cUnknown;
	if(look != kwVarType)
	{
		type = look;
		Step();			// pryc s typem
		assert(IsDataToken(look));
		pointer = look - tokDataFirst;
		Step();			// pryc s pointerem
		vartype = false;
	} else
	{
		Step();			// pryc s vartype
		DoExpression();	// zpracuji vartype
		vartype = true;
	}

	if(look == opBracketStart)
	{
		Step();								// pryc s hranatou zavorkou
		DoExpression();
	} else
		// Zadne hranate zavorky -> prazdny operand.
		if(!vartype)
			es.Push(CComExpression(cUnknown,false,false,false,cUnknown,0));	

	// new jako unarni operator - vyraz v hranatych zavorkach nebo vartype (resp. prazdny operand)
	CComExpression newExp(kwNew,false,false,false,type,1);
	newExp.value.intData = pointer;
	newExp.pos = new CComShortPosition();
	assert(newExp.pos);
	newExp.pos->line = line;
	newExp.pos->column = id;
	es.Push(newExp);		
}

void CComCSGenerator::DoDelete()
{
	int line, type, pointer;

	assert(look == kwDelete);
	Step();						// pryc s delete
	assert(IsDataToken(look));
	line = look - tokDataFirst;
	Step();
	type = look;
	Step();			// pryc s typem
	assert(IsDataToken(look));
	pointer = look - tokDataFirst;
	Step();			// pryc s pointerem

	// Delete jako unarni operator; v isConstant predavam informaci o tom, ze rusim objekt
	CComExpression exp(kwDelete,false,false,false,type,1);
	exp.data1 = line;
	es.Push(exp);		
}

void CComCSGenerator::DoAssigned()
{
	int i;
	assert(look == kwAssigned);
	Step();						// pryc s assigned
	assert(IsDataToken(look));
	i = look - tokDataFirst;
	Step();
	
	es.Push(CComExpression(kwAssigned, false, false, false, i, 0));	
}

void CComCSGenerator::DoMethodCall()
{
	int id;
	CComOBObject* o;
	CComOBMethod* m; 
	CListK<CComExpression> params;
	CListK<CComExpression> callend;
	int time = cUnknown;
	int line;
	bool implicitSpec, namecall;
	CComExpression expObj;

	assert(look == opParentStart);
	Step();					// pryc s opParentStart
	line = look-tokDataFirst;			
	Step();					// pryc s radkou

	if(look == synTimeDecl)
	{
		Step();							// pryc se synTimeDecl
		time = look;
		Step();							// pryc s casem
	}

	if(time==kwCallend || time==kwTimed)
	{
		CListKElem<CComExpression>* stackPos = es.GetPos();
		DoExpression();
		CListKElem<CComExpression>* p = es.expStack.el;
		while(p && p!=stackPos)
		{
			callend.Add(p->data);
			p = p->next;
		}
        es.RestorePos(stackPos);
	} 

    es.Load1();
	while(es.e1.id != synArgsEnd)		// nactu zpracovane parametry do seznamu
	{
		params.Add(es.e1);
		es.Load1();
	}

	es.Load2();		// v es.e1 mam ID objektu, v es.e2 ID metody

	if(IsKernelService(es.e2.id))
		return DoKernelServiceCall(&params, line);

	expObj = es.e1;
	id = es.e1.type;
	if(IsObjectToken(id))
		o = (CComOBObject*)oTab->Get(id);
	else
		if(id == kwObjptr)	
			o = 0;

	implicitSpec = namecall = false;
	switch(look)
	{
	case semDynamicArrayCall:
		Step();
		assert(IsDynamicArrayToken(id));
		return DoDynamicArrayCall(&params);

	case semNameCall:
		assert(IsNameType(es.e2.type));
		namecall = true;
		break;

	case semImplicitSpec:
		implicitSpec = true;
		break;

	default:
		assert(IsObjectToken(look));
		o = (CComOBObject*)oTab->Get(look);
	} 

	Step();			// pryc s ID objektu

	if(!namecall)
	{
		assert(IsDataToken(look));
		id = look - tokDataFirst;
		assert(IsMethodToken(id));
		
		if(implicitSpec)
			m = (CComOBMethod*)mTab->Get(id);
		else
			m = (CComOBMethod*)o->mTab->Get(id);
		
		Step();			// pryc s ID metody
	}

	int objArity;
	switch(expObj.id)
	{
	case opMemberArrow:
	case opMemberDot:
	case opBracketStart:
		objArity = 2;
		break;

	case opParentStart:
		objArity = 3;
		break;

	default:
		objArity = 0;
	}
	es.Push(CComExpression(expObj.id,false,false,false,expObj.type,objArity));		// volany objekt

	CComPosition pos;
	pos.line = line;
	if(namecall)
		es.Push(es.e2);
	CComExpression e(kwMethodName,false,false,false,time,namecall ? 1 : 0,&pos);	// volana metoda - cas zpravy v type
	
	if(namecall)
		e.value.voidPtrData = 0;
	else
		e.value.voidPtrData = m;

	es.Push(e);

	if(callend.IsEmpty())		// zadny callEnd objekt -> prazdny list ve stromu
		es.Push(CComExpression(cUnknown,false,false,false,cUnknown,0));	

	while(!callend.IsEmpty())
		es.Push(callend.RemoveHead());

	es.Push(CComExpression(opComma,false,false,false,cUnknown,2));	// pro metodu a callend
	
	if(params.IsEmpty())		// prazdny seznam parametru -> prazdny list ve stromu
		es.Push(CComExpression(cUnknown,false,false,false,cUnknown,0));	

	int count = 0;
	while(!params.IsEmpty())
	{
		e = params.RemoveHead();
		if(e.id == synNextParameter)	// Dalsi vyraz?
		{
			// - kdyz ano a neni to posledni parametr, tak oddelim parametry carkou.
			if(++count >= 2)	
				es.Push(CComExpression(opComma,false,false,false,cUnknown,2));
		} else
			es.Push(e);			// - kdyz ne, tak to ulozim.
	}
	// zavorka jako ternarni operator - objekt, metoda, argumenty
	es.Push(CComExpression(opParentStart,false,false,false,cUnknown,3));	
}

void CComCSGenerator::DoDynamicArrayCall(CListK<CComExpression>* params)
{
	const char* idName;
	
	idName = GetTab(es.e2.id)->Get(es.e2.id)->name;

	// Pole, na kterem volam jednu z funkci:
	es.Push(es.e1);		
	if(!strcmp(idName, "GetCount"))
	{
		assert(params->num == 0);
		es.Push(CComExpression(cUnknown,false,false,false,cUnknown,0));	 // prazdny parametr
		es.Push(CComExpression(1, false, false, false, cUnknown, 0));	 // GetCount
	} else
		if(!strcmp(idName, "SetCount"))
		{
			CComExpression e;
			while(!params->IsEmpty())
			{
				e = params->RemoveHead();
				if(e.id != synNextParameter)	
					es.Push(e);
			}
			es.Push(CComExpression(2, false, false, false, cUnknown, 0));	// SetCount
		} else
			assert(false);
	
	// Typ dynamickeho pole jako ternarni operator - pole, parametr a volana metoda
	es.Push(CComExpression(kwIntArray, false, false, false, cUnknown, 3));
}

void CComCSGenerator::DoKernelServiceCall(CListK<CComExpression>* params, int line)
{
	CComExpression e;
	int count = 0;

	assert(es.e1.id == kwThis);
	
	if(params->IsEmpty())		// prazdny seznam parametru -> prazdny list ve stromu
		es.Push(CComExpression(cUnknown,false,false,false,cUnknown,0));	

	while(!params->IsEmpty())
	{
		e = params->RemoveHead();
		if(e.id == synNextParameter)	// Dalsi vyraz?
		{
			// - kdyz ano a neni to posledni parametr, tak oddelim parametry carkou.
			if(++count >= 2)
				es.Push(CComExpression(opComma,false,false,false,cUnknown,2));
		} else
			es.Push(e);			// - kdyz ne, tak to ulozim.
	}
	
	// ksFirst jako unarni operator - argumenty; typ sluzby je v type, line ve value.intData
	CComExpression eService = CComExpression(es.e2.id,false,false,false,cUnknown,1);
	eService.value.intData = line;
	es.Push(eService);	
}