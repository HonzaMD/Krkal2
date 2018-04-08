/////////////////////////////////////////////////////////////////////////////
//
// ComPart.cpp
//
// Implementace CComPart - spolecny predek vsech casti kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ComPart.h"
#include "ComFunctions.h"
#include "fs.h"
#include "ComKerServices.h"
#include "scriptedit.h"
#include "SEdOpenedFiles.h"


CComFileCache::CComFileCache(FILE* _f, bool _in)
{ 
	f = _f;
	in = _in;
	Clear();
}

CComFileCache::~CComFileCache() 
{
	if(!in)
		Flush();	
}

int CComFileCache::GetToken()
{
	assert(in);
	if(index == size)
		Fetch();
	return cache[index++];
}

void CComFileCache::PutToken(int token)
{
	assert(!in);
	cache[index++] = token;
	if(index == size)
		Flush();
}

void CComFileCache::Flush()
{
	assert(!in);	
	fwrite(cache, sizeof(int), index, f);
	index = 0;
}

void CComFileCache::Fetch()
{
	size = fread(cache, sizeof(int), cFileCacheSize, f);
	index = 0;
}

void CComFileCache::Clear()
{
	index = 0;
	size = in ? 0 : cFileCacheSize;
}

// *** Constructor ***
CComPart::CComPart(CComObjectBrowser* ob, int queueSize)
{
	assert(ob);
	queueLn = queueSize;	
	queueUp = queueDn = 0;	
	queue = new int [queueSize];
	assert(queue);
	inputFile = 0;
	debugFile = outputFile = 0;
	inputStr = 0;
	parent = 0;
	scriptWindow = 0;
	inputType = inpNone;
	debugPos = false;
	indexStr = 0;
	uTab = ob->uTab;
	cTab = ob->cTab;
	oTab = ob->oTab;
	nTab = ob->nTab;
	dTab = ob->dTab;
	gTab = ob->gTab;
	pTab = ob->pTab;

	dnTab = ob->dnTab;
	gnTab = ob->gnTab;
	sTab = ob->sTab;

	mTab = ob->mTab;
	aTab = ob->aTab;

	names = ob->names;
	nameSearch = ob->nameSearch;

	objBrowser = ob;

	assert(uTab && cTab && oTab && nTab && dTab && gTab && pTab && mTab && aTab && dnTab && gnTab && names && nameSearch);
	inObj = 0;
	inMethod = 0;
	inObjID = cUnknown;
	startDir = 0;
	tmpStr = 0;
	outputCache = 0;
}

// *** Destructor ***
CComPart::~CComPart()
{
	SAFE_DELETE_ARRAY(queue);
	if(debugFile) 	
		fclose(debugFile);

	if(inputFile) 	
		SAFE_DELETE_ARRAY(inputFile);

	if(outputFile) 
	{
		outputCache->Flush();
		fclose(outputFile);
	}

	if(inputStr)	
		SAFE_DELETE_ARRAY(inputStr);

	CComPosition tmpPos;
	while(!posStack.IsEmpty())
	{
		tmpPos = posStack.RemoveHead();
		if(!tmpPos.scriptEditorWindow)
			 SAFE_DELETE_ARRAY(tmpPos.openFile);
	}

	while(includes.GetCount())
	{
		tmpPos = includes.RemoveHead();
		SAFE_DELETE_ARRAY(tmpPos.openFile);
	}
	SAFE_DELETE_ARRAY(startDir);
	SAFE_DELETE_ARRAY(tmpStr);
	SAFE_DELETE(outputCache);
}

void CComPart::ReleaseOutput()
{
	if(debugFile) 	
		fclose(debugFile);
	if(outputFile) 	
	{
		outputCache->Flush();
		fclose(outputFile);
	}
	debugFile = outputFile = 0;
}

int CComPart::Get()
{ 
	try
	{
		if(QueueEmpty())
			DoPart();
		return QueueGet();
	} 
	catch(CComError e)
	{
		if(e.pos.fileID == cUnknown)
			e.pos = pos;
		if(!e.pos.file)
		{
			e.pos.file = newstrdup(pTab->Get(e.pos.fileID)->name);
			assert(e.pos.file);
		}
		throw e;
	}
}

int CComPart::GetParent()
{
	assert(inputType != inpNone);

	int ret; 

	oldPos = pos;
	switch(inputType)
	{
	case inpWindow:
	case inpFile:

		if(inputType == inpFile)
		{
			if(pos.index == pos.size)
				ret = EOF;
			else
				ret = pos.openFile[pos.index++];
		} else
		{
			ret = scriptWindow->GetNextChar();
			pos.index++;
		}
		space = isspace(ret);

		switch(ret)
		{
		case '\n':
			pos.plusLine++;
			pos.plusColumn = 0;
			break;
		case '\t':
			pos.plusColumn += cTabSize;
			break;
		case ' ':
			pos.plusColumn++;
			break;
		case EOF:
			if(FinishFile() == cOK)		// 0, pokud je to opravdu posledni soubor -> return EOF
				ret = GetParent();  // jinak po FinishFile() vratim dalsi znak z 'nadrazeneho' souboru
			else
			{
				if(pos.TopTheBill())
					lineChange = true;
				if(!pos.column)
					pos.column++;
				space = true;
			}
			break;
		default:
			if(pos.TopTheBill())
				lineChange = true;
			pos.column++;
		}
		break;
	
	case inpString:
			ret = inputStr[indexStr++];
			if(!ret)
			{
				FinishStr();
				ret = GetParent();
			}
			pos.column++;
			break;

	case inpPart:
			ret = parent->Get();
			while(ret < tokMetaLast)
			{
				if(ret <= tokMetaVersion)
					pos.version = -ret+tokMetaVersion;	
				else
					if(ret <= tokMetaFile)
					{
						pos.fileID = -ret+tokMetaFile;	
						if(pos.fileID != oldPos.fileID)
							fileChange = true;
					} else
						if(ret <= tokMetaLineFirst)
						{
							pos.line = -ret+tokMetaLineFirst+1;
							if(pos.line != oldPos.line)
								lineChange = true;
						} else
							if(ret <= tokMetaColumnFirst)
								pos.column = -ret+tokMetaColumnFirst+1;
							else
								assert(false);
				ret = parent->Get();
			}
			break;

	default:
			assert(false);
	}
	return ret;
}

void CComPart::DebugString(int token, char* buf)
{
	if(token <= tokMetaVersion)
		sprintf(buf,"Version: \t%10d\t%s", -token+tokMetaVersion,uTab->Get(-token+tokMetaVersion)->name);	
	else if(token <= tokMetaFile)
		sprintf(buf,"File:    \t%10d\t%s", -token+tokMetaFile,pTab->Get(-token+tokMetaFile)->name);	
	else if(token <= tokMetaLineFirst)
		sprintf(buf,"  Line:  \t%10d", -token+tokMetaLineFirst+1);
	else if(token <= tokMetaColumnFirst)
		sprintf(buf,"  Column:\t%10d", -token+tokMetaColumnFirst+1);
	else if(IsLexicalToken(token))
		sprintf(buf,"Lexical: \t%10d\t%s", token, cLexicalSymbols[token-lexFirst-1]);
	else if(IsSyntaxToken(token))
		sprintf(buf,"Syntax: \t%10d\t%s",token,cSyntaxSymbols[token-synFirst-1]);
	else if(IsSemanticToken(token))
		sprintf(buf,"Semantic: \t%10d\t%s",token,cSemanticSymbols[token-semFirst-1]);
	else if(IsKeywordToken(token))
		sprintf(buf,"Keyword:\t%10d\t%s",token,cKeywords[min(token-kwFirst-1,kwString-kwFirst-1)]);
	else if(IsKnownNameToken(token) || IsKnownNameAtrToken(token) || IsKernelConstant(token))
		sprintf(buf,"Known name:\t%10d", token);
	else if(IsKernelService(token))
		sprintf(buf,"Kerservice:\t%10d", token);
	else if(IsOperatorToken(token))
		sprintf(buf,"Operator:\t%10d\t%s",token,cOperators[token-opFirst-1]);
	else if(IsUnknownToken(token))
		sprintf(buf,"Unknown: \t%10d\t%s",token,uTab->Get(token)->name);
	else if(IsConstantToken(token))
		sprintf(buf,"Constant: \t%10d\t%s",token,cTab->Get(token)->name);
	else if(IsObjectToken(token))
		sprintf(buf,"Object: \t%10d\t%s",token,oTab->Get(token)->name);
	else if(IsStructureToken(token))
		sprintf(buf,"Struct: \t%10d\t%s",token,sTab->Get(token)->name);
	else if(IsNameToken(token))
		sprintf(buf,"Name:   \t%10d\t%s",token,nTab->Get(token)->name);
	else if(IsDirectNameToken(token))
		sprintf(buf,"Dir.name:\t%10d\t%s",token,dnTab->Get(token)->name);
	else if(IsGlobalNameToken(token))
		sprintf(buf,"Glob.name:\t%10d\t%s",token,dnTab->Get(token)->name);
	else if(IsGlobalToken(token))
		sprintf(buf,"Global: \t%10d\t%s",token,gTab->Get(token)->name);
	else if(IsMethodToken(token))
	{
		if(inObj)
			sprintf(buf,"Method: \t%10d\t%s",token,inObj->mTab->Get(token)->name);
		else
			sprintf(buf,"Method: \t%10d\t%s",token,mTab->Get(token)->name);
	} else if(IsAttributeToken(token))
	{
		assert(inObj);
		sprintf(buf,"Attrib: \t%10d\t%s",token,inObj->aTab->Get(token)->name);
	} else if(IsLocalToken(token))
	{
		assert(inMethod);
		sprintf(buf,"Local:  \t%10d\t%s",token,inMethod->lTab->Get(token)->name);
	} else if(IsArgumentToken(token))
	{
		if(inMethod)
			sprintf(buf,"Argument:\t%10d\t%s",token,inMethod->aTab->Get(token)->name);
		else
			sprintf(buf,"Argument:\t%10d\t%s",token,aTab->Get(token)->name);
	} else if(token >= tokDataFirst)
		sprintf(buf,"Data:    \t%10d\t", token-tokDataFirst);
	else
		assert(false);
}	

const char* CComPart::GetTokenStr(int token)
{
	if(token == cUnknown)
		return cStrEmpty;
	else if(IsLexicalToken(token))
		return cLexicalSymbols[token-lexFirst-1];
	else if(IsSyntaxToken(token))
		return cSyntaxSymbols[token-synFirst-1];
	else if(IsKeywordToken(token))
		return cKeywords[min(token-kwFirst-1,kwString-kwFirst-1)];
	else if(IsKnownNameToken(token))
		return KnownNames[token-knSecond].namestr+6;
	else if(IsKnownNameAtrToken(token))
		return KerVarUsesNames[token-knFirst];
	else if(IsKernelService(token))
		return cKerServices[token-ksFirst].name;
	else if(IsOperatorToken(token))
		return cOperators[token-opFirst-1];
	else 
		return GetTab(token)->Get(token)->name;
	
	return 0;
}

void CComPart::Put(int token, bool putPosition, CComShortPosition* alterPosition)
{
	if(putPosition)
	{
		if(fileChange)
		{
			QueuePut(tokMetaFile-(alterPosition ? alterPosition->fileID : pos.fileID));
			fileChange = false;
		}

		if(lineChange || alterPosition)
		{
			QueuePut(tokMetaLineFirst-(alterPosition ? alterPosition->line : pos.line)+1);
			lineChange = false;
		}

		if(inputType == inpFile || inputType == inpWindow)
			QueuePut(tokMetaColumnFirst-(alterPosition ? alterPosition->column : pos.column-(space ? 0 : 1))+1);
		else
			QueuePut(tokMetaColumnFirst-(alterPosition ? alterPosition->column : pos.column)+1);
	}
	QueuePut(token);
}

int  CComPart::PutReference(int token, bool putPosition)
{
	int i;

	if(token != cUnknown)
	{
		Put(token,putPosition);
		i = (queueUp+queueLn-1)%queueLn;
	} else
		i = queueUp;
	return refs.Add(i);
}

int CComPart::RemoveReference(int ref, int testVal)
{
	int old = queue[refs[ref]];
	if(testVal != cUnknown)
		assert(testVal == old);
	queue[refs[ref]] = tokVoidToken;

	if(debugFile)
		fprintf(debugFile, "*** removed token: %d ***\n", old); 

	//refs.Remove(ref);
	return cUnknown;
}

CComSymbolTable* CComPart::GetTab(int token)
{
	if(IsUnknownToken(token)) 
		return uTab;
	else if(IsConstantToken(token)) 
		return cTab;
	else if(IsObjectToken(token))
		return oTab;
	else if(IsNameToken(token))
		return nTab;
	else if(IsDirectNameToken(token))
		return dnTab;
	else if(IsGlobalToken(token))
		return gTab;
	else if(IsMethodToken(token))
	{
		assert(inObj);
		return inObj->mTab;
	} else if(IsAttributeToken(token))
	{
		assert(inObj);
		return inObj->aTab;
	} else if(IsLocalToken(token))
	{
		assert(inMethod);
		return inMethod->lTab;
	} else if(IsArgumentToken(token))
	{
		assert(inMethod);
		return inMethod->aTab;
	} else if(IsStructureToken(token))
		return sTab;
	return 0;
}

int CComPart::GetOpArity(int opIndex)
{
	assert(opIndex>opFirst && opIndex<opLast);
	if(opIndex>=opPlus && opIndex<=opAssignXor)
		return 2;
	else
		if(opIndex>=opSizeof && opIndex<=opUnaryMinus)
			return 1;
		else
			if(opIndex>=opMemberDot && opIndex<=opBracketStart)
				return 2;
			else
				if(opIndex==opConditional)
					return 3;
				else
					if(opIndex == opParentStart)
						return 1;
					else
						assert(false);
	return cUnknown;
}

char* CComPart::GetTypeStr(int type, int pointer)
{
	char* ret = new char[cMaxIdentifierLn+pointer+1];
	assert(ret);

	if(IsKeywordToken(type))
		strcpy(ret,cKeywords[min(type-kwFirst-1,kwString-kwFirst-1)]);
	else
		if(IsObjectToken(type))
			strcpy(ret,oTab->Get(type)->name);
		else
			if(IsStructureToken(type))
				strcpy(ret, sTab->Get(type)->name);
	
	if(type == kwObjptr)
		pointer--;

	while(pointer--)	
		strcat(ret,"*");
	return ret;
}


int CComPart::GetTypeSize(int type, int pointer)
{
	if(pointer)
		return 4;

	switch(type)
	{
	case kwChar:	return 1;
	case kwDouble:	return 8;
	default:
		return 4;
	}
}

const char* CComPart::GetTypeString(int type, bool considerUnsigned)
{
	if(IsStructureToken(type))
		return sTab->Get(type)->kerName;
	else
		if(IsObjectType(type))
			return cStrOPointer;
		else
			if(IsNameType(type))
				return cStrCKerName;
    		else
				if(IsStringType(type))
					return cStrString;
				else
					if(IsDynamicArrayToken(type))
					{
						CComOBBaseData tmp("tmp",cUnknown,CComShortPosition());			
						tmp.SetData(cUnknown, type, 1);
						return cDynamicArrayNames[tmp.GetMDType()-eKTarrChar];
					} else
						if(type == kwChar)
							if(considerUnsigned)
								return cStrUnsignedChar;
							else
								return GetTokenStr(type);
						else
							return	GetTokenStr(type);
}

const char* CComPart::GetLongNameStr(CListK<CComNSENode>* ident)
{
	int ln;
	CListKElem<CComNSENode>* p;

	ln = 1;
	p = ident->el;
	while(p)
	{
		ln += strlen(uTab->Get(p->data.id)->name) + 2;			// 2 == strlen("::")
		p = p->next;
	}

	SAFE_DELETE_ARRAY(tmpStr);
	tmpStr = new char[ln];
	assert(tmpStr);
	tmpStr[0] = '\0';

	p = ident->el;
	while(p)
	{
		if(p != ident->el)
			strcat(tmpStr, "::");
		strcat(tmpStr, uTab->Get(p->data.id)->name);
		p = p->next;
	}
	return tmpStr;
}

const char* CComPart::GetCollisionTypesStr(int type1, int pointer1, int type2, int pointer2, const char* addStr) 
{ 	
	SAFE_DELETE_ARRAY(tmpStr);
	char *str1, *str2;

	str1 = GetTypeStr(type1, pointer1);
	str2 = (type2 != cUnknown) ? GetTypeStr(type2, pointer2) : 0;

	tmpStr = new char[strlen(str1) + (str2 ? strlen(str2) : 0) + (addStr ? strlen(addStr) : 0) + 20];
	assert(tmpStr);
	sprintf(tmpStr, "%s", str1); 

	if(type2 != cUnknown)
	{
		strcat(tmpStr, ", ");
		strcat(tmpStr, str2);
	}

	if(addStr)
	{
		strcat(tmpStr, " (");
		strcat(tmpStr, addStr);
		strcat(tmpStr, ")");
	}

	SAFE_DELETE_ARRAY(str1);
	SAFE_DELETE_ARRAY(str2);
	
	return tmpStr;
}

int CComPart::StartFile(const char* fileName, CComPosition* inclPos)
{
	int fileSize;
	CComOBPreprocessor* newRec;
	const char* tmp;
	char* fullName;
	CSEdScriptEdit* edit;

	assert(fileName);

	int recID = pTab->Find(fileName);
	if (included.Find(recID) != cUnknown)
	{
		newRec = (CComOBPreprocessor*)pTab->Get(recID);
		if (newRec->type == pteFileName)
			return cError;
	}

	if(inclPos)
	{
		tmp = inclPos->file;
		fullName = new char[strlen(tmp) + strlen(startDir) + 10];
		assert(fullName);
		
		strcpy(fullName, startDir);
		if(fullName[0] && fullName[strlen(fullName)-1] != '\\')
			strcat(fullName, "\\");
		strcat(fullName, tmp);

		if(SEdOpenedFiles && (edit = SEdOpenedFiles->FindScripEdit(fullName)))
		{
			SAFE_DELETE_ARRAY(fullName);
			int ret = StartFromWindow(edit);
			included.Add(pos.fileID);
			return ret;
		}

		SAFE_DELETE_ARRAY(fullName);
	}

	FS->ChangeDir("$GAME$");
	FS->ChangeDir(startDir);

	CComPosition newPos(fileName);

	newRec = new CComOBPreprocessor(fileName,pteFileName,0,0);
	newPos.fileID = pTab->FindOrAdd(fileName,newRec);
	
	fileSize = FS->GetFileSize(fileName);
	newPos.openFile = new unsigned char[fileSize+10];
	assert(newPos.openFile);

	newPos.size = fileSize;

	if(FS->ReadFile(fileName, (char*)newPos.openFile, fileSize) != 1)
	{
		SAFE_DELETE_ARRAY(newPos.openFile);	
		throw CComError(errCantOpenIncludeFile,noteNone,fileName);
	}
	
	if(pos.file)
		posStack.Add(pos);	// zaloha pozice v aktualnim souboru

	newPos.startingFile = true;

	included.Add(newPos.fileID);

	oldPos = pos;
	pos = newPos;	
	SetSource(newPos.openFile);
	fileChange = true;
	return 0;
}

int CComPart::FinishFile()
{
	assert(inputType == inpFile || inputType == inpWindow);
	
	SAFE_DELETE_ARRAY(inputFile);
	if(!includes.GetCount())
		if(posStack.el)
		{
			pos = posStack.RemoveHead();
			if(pos.scriptEditorWindow)		
				SetSource((CSEdScriptEdit*)pos.openFile); // navrat do okna editoru
			else
				SetSource(pos.openFile);	// soubor
		} else
			return cError;
	else
	{
		int res;
		pos = posStack.RemoveHead();
		CComPosition tmpPos;

		do
		{
			if(includes.GetCount() == 0)
				return cError;
			
			tmpPos = includes.RemoveHead();
			res = StartFile(tmpPos.file, &tmpPos);
		} while(res == cError);
	}
	fileChange = true;
	if(pos.version != cUnknown)
		Put(tokMetaVersion-pos.version, false);

	return cOK;
}

int CComPart::StartStr(const char* _inputStr, int rewind, int lookToSave)
{
	assert(_inputStr);
	assert(inputType == inpFile);
	assert(rewind <= pos.column);
	strRewind = rewind - (int)strlen(_inputStr) - 1;
	pos.column -= rewind+1;
	SetSource(_inputStr, lookToSave);
	indexStr = 0;
	return ' ';
}

int CComPart::FinishStr()
{
	assert(inputType == inpString && inputStr);
	SAFE_DELETE_ARRAY(inputStr);
	SetSource(pos.openFile);
	pos.column += strRewind;
	return 0;
}

int CComPart::StartPart(CComPart* _parent)
{
	SetSource(_parent);
	return 0;
}

int CComPart::StartFromMemory(const unsigned char* data, int size, const char* fileName)
{
	assert(inputType == inpNone);
	assert(data && size > 0);

	CComPosition newPos(fileName);

	CComOBPreprocessor* newRec = new CComOBPreprocessor(fileName,pteFileName,0,0);
	newPos.fileID = pTab->FindOrAdd(fileName,newRec);

	newPos.openFile = new unsigned char[size];
	assert(newPos.openFile);

	newPos.size = size;

	memcpy(newPos.openFile, data, size); 
	
	if(pos.file)
		posStack.Add(pos);	// zaloha pozice v aktualnim souboru

	oldPos = pos;
	pos = newPos;	
	SetSource(newPos.openFile);
	fileChange = true;
	return 0;
}

int CComPart::StartFromWindow(CSEdScriptEdit* script)
{
	SetSource(script);
	int test = script->SeekToStart();
	assert(test);
	const char* fileName = script->GetFileName();

	if(pos.file)
		posStack.Add(pos);	// zaloha pozice v aktualnim souboru

	pos = CComPosition(fileName);

	CComOBPreprocessor* newRec = new CComOBPreprocessor(fileName,pteFileName,0,0);
	pos.fileID = pTab->FindOrAdd(fileName,newRec);

	pos.openFile = (unsigned char*)script;
	pos.scriptEditorWindow = true;
	fileChange = true;
	inputFile = 0;
	return 0;
}

void CComPart::SetWarning(CComError e)
{
	assert(e.IsWarning());
	if(e.pos.fileID == cUnknown)
		e.pos = pos;
	if(!e.pos.file)
	{
		e.pos.file = newstrdup(pTab->Get(e.pos.fileID)->name);
		assert(e.pos.file);
	}
	warningList.Add(e);
}

void CComPart::SetSource(CComPart* _parent)
{
	assert(_parent);
	parent = _parent;
	inputType = inpPart;
}

void CComPart::SetSource(CSEdScriptEdit *script)
{
	assert(script);
	scriptWindow = script;
	inputType = inpWindow;
}

void CComPart::SetSource(const unsigned char* _inputFile)
{
	assert(_inputFile);
	inputFile = _inputFile;
	inputType = inpFile;
}

void CComPart::SetSource(const char* _inputStr, int lookToSave)
{
	assert(_inputStr);
	int len = (int)strlen(_inputStr);
	inputStr = new char[len+2];
	assert(inputStr);
	strcpy(inputStr,_inputStr);
	inputStr[len] = lookToSave;
	inputStr[len+1] = '\0';
	inputType = inpString;
}

void CComPart::SetDebug(const char* fileName, bool _debugPos)
{
	if(debugFile)
		fclose(debugFile);
	if(fileName)
	{
		debugFile = fopen(fileName,"w");
		assert(debugFile);
		debugPos = _debugPos;
	} else
		debugFile = 0;
}

void CComPart::SetOutput(const char* fileName)
{
	if(outputFile)
		fclose(outputFile);
	if(fileName)
	{
		outputFile = fopen(fileName,"wb");
		assert(outputFile);

		SAFE_DELETE(outputCache);
		outputCache	= new CComFileCache(outputFile, false);
		assert(outputCache);
	} else
		outputFile = 0; 
}

void CComPart::SetStartDir(const char* _startDir)
{
	SAFE_DELETE_ARRAY(startDir);
	assert(_startDir);
	startDir = newstrdup(_startDir);
	assert(startDir);
}

int CComPart::QueueGet()
{
	int ret;
	do
	{
		assert(!QueueEmpty());
		ret = queue[queueDn];
		queueDn = (queueDn + 1) % queueLn;
	} while(ret == tokVoidToken);

	if(outputFile && ret > tokMetaLast)
		outputCache->PutToken(ret);
		//fwrite(&ret,sizeof(ret),1,outputFile);

	return ret;
}

void CComPart::QueuePut(int token)
{
	queue[queueUp] = token;
	queueUp = (queueUp + 1) % queueLn;

	if(QueueEmpty())	// zde to naopak znamena QueueFull
		QueueExpand();

	if(debugFile && (debugPos || token > tokMetaLast))
	{
		char buf[cMaxDebugStringLn];
		DebugString(token, buf);
		fprintf(debugFile, "%s\n", buf); 
	}
}

void CComPart::QueueExpand(int size)
{
	if(size == -1)
		size = 2 * queueLn;
	int* newQueue = new int[size];
	assert(newQueue);

	int i, j;

	for(i=queueDn; i<queueLn; i++)
		newQueue[i-queueDn] = queue[i];
	
	for(j=0; j<queueDn; j++)
		newQueue[i+j] = queue[j];
	

	for(i=0; i<refs.GetCount(); i++)
		if(refs[i] < queueDn)
			refs[i] += queueDn;
		else
			refs[i] -= queueDn;

	queueDn = 0;
	queueUp = queueLn;
	queueLn = size;
	
	delete[] queue;
	queue = newQueue;
}

