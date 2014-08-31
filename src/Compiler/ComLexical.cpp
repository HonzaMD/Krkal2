/////////////////////////////////////////////////////////////////////////////
//
// ComLexical.cpp
//
// Implementace CComLexical - trida pro lexikalni analyzu zdrojoveho kodu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComLexical.h"
#include "ComFunctions.h"
#include "ComKerServices.h"
#include "fs.h"

CComLexical::CComLexical(CComObjectBrowser* ob, int queueSize) 
: CComPart(ob, queueSize)
{
	look = ' ';
	defined = true;
	strcpy(author, "Unknown author");
	strcpy(game, "Unknown game");
}

CComLexical::~CComLexical()
{
}

void CComLexical::DoPart()
{
		int ret;
		char buf[4];

		ret = cUnknown;
		while(ret != lexEOF)
		{
		ret = cUnknown;
		// hlavni smycka lexikalniho analyzatoru
		while(ret == cUnknown || !defined)
		{
			look = SkipWhite(); 

			while(pos.startingFile)			// na zacatku souboru musim zpracovat hlavicku
			{
				pos.startingFile = false;
				DoHead();
				look = SkipWhite();
			}

			// testuji dalsi znak ze vstupu
			if((isalpha(look) || look == '_') && look < 128)	// pismeno -> jedu identifikator
				ret = DoIdentifier();
			else if(isdigit(look))			// cislice -> jedu ciselnou konstantu
				ret = DoNumber();
			else if(isoperator(look))		// operator -> nactu operator (nemusi byt jednoznakovy)
				ret = DoOperator();
			else if(look == '@')			// zavinac -> nactu zname jmeno
				ret = DoKnownName();
			else
				switch(look)				
				{
				case '\'':					// apostrof -> jedu znakovou konstantu
					ret = DoChar();
					break;
				case '"':					// uvozovka -> jedu retezcovou konstantu
					ret = DoString();		
					break;
				case ';':					// strednik 
					ret = lexSeparator;		
					Step();
					break;
				case '{':					// otviraci prikazova zavorka
					ret = lexStartBlock;	
					Step();
					break;
				case '}':					// uzaviraci prikazova zavorka
					ret = lexEndBlock; 
					Step();
					break;
				case '#':					// mriz -> jedu preprocesor
					DoPreprocessor(); 
					ret = cUnknown;					// a potom znovu cely cyklus, jeste nemam vystup
					break;
				case EOF:					// konec souboru
					ret = lexEOF; 
					if(!defined)
						throw CComError(errUnexpectedEOF,noteMissingEndIf);
					break;
				default:
					sprintf(buf,"%c",look);
					throw CComError(errUnexpectedCharacter,0,buf);
				}
		}
		Put(ret);		// nalezeny vystup pridam na konec fronty
		}
}

void CComLexical::DoReadHead(SComHeadInfo* info)
{
	DoHead();

	if(!posStack.IsEmpty())
	{
		info->importedFiles.Add(newstrdup(pos.file));
		pos = posStack.RemoveHead();
		SkipWhite();
		SAFE_DELETE_ARRAY(pos.openFile);
	}

	info->behindHeadIndex = pos.index-1;
	info->headLineCount = pos.line-1;

	strcpy(info->version, GetVersionStr());
	strcpy(info->author, author);
	strcpy(info->game, game);
	for(int i=0; i<includes.GetCount(); i++)
        info->importedFiles.Add(newstrdup(includes[i].file));
}

int CComLexical::SkipWhite(bool canIncLine)
{
	while(isspace(look) && look != EOF && (canIncLine || look != '\n' && look != '\r'))
		look = GetParent();
	return look;
}

void CComLexical::Step()
{ 
	if(look == EOF)
		throw CComError(errUnexpectedEOF,noteNone);
	look = GetParent(); 
}

int CComLexical::DoIdentifier()
{
	char buf[cMaxIdentifierLn+1];
	CComOBPreprocessor* pre;

	assert(isalpha(look) || look == '_');

	ReadTokenE(buf,cMaxIdentifierLn,isidentifier,false,errIdentifierTooLong,buf);

	if(strstr(buf,"_KN")==buf || strstr(buf,"__M"))
		throw CComError(errIdentifierBadFormat,noteCheckIdentifierRules,buf);
	
	pre = (CComOBPreprocessor*)pTab->FindRec(buf);
	if(pre)
		if(pre->type == pteDefinedSymbol && pre->strData && pre->intData==dsymDefined)
		{
			look = StartStr(pre->strData, (int)strlen(pre->name), look);
			return cUnknown;
		} else
			if(pre->type == pteKeyword)
				return pre->intData;

	CComOBBase* newUnknown = new CComOBBase(buf,pos.version,pos);
	assert(newUnknown);
	return  uTab->FindOrAdd(buf,newUnknown);
}


int CComLexical::DoKnownName()
{
	char buf[cMaxIdentifierLn+4+1];
	CComOBPreprocessor* pre;

	assert(look == '@');
	Step();

	SkipWhite();
	if(look == '\"')
		return lexEditorStringSeparator;

	strcpy(buf, "_KN_");
	ReadTokenE(buf+4,cMaxIdentifierLn,isidentifier,false,errIdentifierTooLong,buf);

	pre = (CComOBPreprocessor*)pTab->FindRec(buf);
	if(pre)
	{
		assert(pre->type == pteKnownName || pre->type == pteKernelConstant || pre->type == pteKernelService);
		return pre->intData;
	} else
		throw CComError(errUnknownKnownName,noteNone,buf+4);
}

int CComLexical::DoNumber()
{
	char buf[cMaxNumberConstantLn+1], buf2[cMaxNumberConstantLn+1];

	assert(isdigit(look));

	bool startZero = look == '0';
	enum {error, unknown, dec, hexa, octal, real};
	int type = unknown, probtype = unknown;

	int i = 0;
	while(i < cMaxNumberConstantLn && isnumber(look) && type <= unknown)
	{
		buf[i++] = look;
		buf[i] = '\0';
		if(startZero && i>1)
		{
			if(i==2 && toupper(look) == 'X')
			{
				probtype = hexa;
				Step();
				if(!ReadToken(buf+i,cMaxNumberConstantLn-i,ishexa))
					i = cMaxNumberConstantLn;
				else
					type = hexa;
			} else
				if(isoctal(look))
				{
					probtype = octal;
					Step();
					if(!ReadToken(buf+i,cMaxNumberConstantLn-i,isoctal))
						i = cMaxNumberConstantLn;
					else
						type = octal;
				} else
					if(isdigit(look))
						type = error;		// >7 v oktalovem cisle 
		}
		if(type == unknown)
			switch(toupper(look))
			{
			case '.':
				probtype = real;
				Step();
				if(!ReadToken(buf2,cMaxNumberConstantLn,isdigit))
				{
					i = cMaxNumberConstantLn;
					break;
				} else
				{
					strcat(buf,buf2);
					i+=(int)strlen(buf2);
					if(toupper(look)!='E')
					{
						type = real;
						break;
					} else
						buf[i++] = look;			// E
				}
			case 'E':
				probtype = real;
				look = GetParent();
				if(look=='+' || look=='-' || isdigit(look))
				{
					buf[i++] = look;
					Step();
				} else
				{
					type = error;
					break;
				}
				if(!ReadToken(buf+i,cMaxNumberConstantLn-i,isdigit))
					i = cMaxNumberConstantLn;
				else
					type = real;
				break;
			default:
				if(!isdigit(look))
					type = error;
			}

		if(type == unknown)
			Step();
	}

	if(i==cMaxNumberConstantLn)
		throw CComError(errIdentifierTooLong,noteNone,buf);

	if(type == error || isidentifier(look))
	{
		if(look == '_')		// verze?
			return DoVersion(buf,strlen(buf));

		sprintf(buf2,"'%c'",look);
		throw CComError(errUnexpectedCharacter,noteNumberSyntaxError,buf2);
	}

	CComOBConstant* newConst = new CComOBConstant(buf,(type==real) ? kwDouble : kwInt);
	CComOBConstant* c;
	int id;

	id = cTab->Find(buf);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)cTab->Get(id);
		assert(c);
		if(c->dType == (type==real) ? kwDouble : kwInt)
		{
			delete newConst;
			return id;
		}
		id = cTab->FindNext(buf);			
	}
	return cTab->Add(newConst);
}

int CComLexical::DoVersion(char* buf, int i)
{
	while(ishexa(look) || look == '_' && i<20)
	{
		buf[i++] = look;
		Step();
	}
	buf[i] = '\0';
	CheckVersion(buf);
	int id = uTab->Find(buf);
	if(id==cUnknown)
		throw CComError(errUnknownVersion,noteNone,buf);
	return id;
}


int CComLexical::DoString()
{
	char buf[cMaxStringLnMD+2];
	int i;

	assert(look == '\"');
	Step();					// pryc s uvozovkami
	i = 0;
	while(look != '\"' && i<cMaxStringLnMD)
		switch(look)
		{
		case EOF:
			throw CComError(errUnexpectedEOF,noteForgetEndString);
		case '\\':			// escape sequence?
			buf[i++] = DoEscape();
			break;		
		default:
			buf[i++] = look;
			Step();				// pryc se znakem
		}
	buf[i] = '\0';
	if(i == cMaxStringLnMD)
		throw CComError(errStingTooLong, noteCheckString,buf);

	Step();					// pryc s ukoncujicimi uvozovkami
	CComOBConstant* newConstant = new CComOBConstant(buf,kwString);
	CComOBConstant* c;
	int id;

	id = cTab->Find(buf);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)cTab->Get(id);
		assert(c);
		if(c->dType == kwString)
		{
			delete newConstant;
			return id;
		}
		id = cTab->FindNext(buf);			
	}
	return cTab->Add(newConstant);
}

int CComLexical::DoChar()
{
	char buf[2];

	assert(look == '\'');
	Step();					// pryc s apostrofem
	if(look == '\\')		// escape sequence?
		buf[0] = DoEscape();
	else
	{
		buf[0] = look;
		Step();				// pryc se znakem
	}

	if(look != '\'')
		throw CComError(errExpectedSymbol,noteCheckChar,"\'");

	Step();					// pryc s unkoncovacim apostrofem

	buf[1] = '\0';
	
	CComOBConstant* newConstant = new CComOBConstant(buf,kwChar);
	CComOBConstant* c;
	int id;

	id = cTab->Find(buf);
	while(id != cUnknown)
	{
		c = (CComOBConstant*)cTab->Get(id);
		assert(c);
		if(c->dType == kwChar)
		{
			delete newConstant;
			return id;
		}
		id = cTab->FindNext(buf);			
	}
	return cTab->Add(newConstant);
}

int CComLexical::DoEscape()
{
	int c;
	int code;
	CComOBConstant* cPtr;

	assert(look == '\\');
	Step();				// pryc s basckslashem
	switch(look)
	{
	case 'a':	c = '\a';	Step(); break;
	case 'b':	c = '\b';	Step(); break;
	case 'f':	c = '\f';	Step(); break;
	case 'n':	c = '\n';	Step(); break;
	case 'r':	c = '\r';	Step(); break;
	case 't':	c = '\t';	Step(); break;
	case 'v':	c = '\v';	Step(); break;
	case '\\':	c = '\\';	Step(); break;
	case '\'':	c = '\'';	Step(); break;
	case '\"':	c = '\"';	Step(); break;
	case '?':	c = '\?';	Step(); break;
	default:	// nasleduje kod znaku
		if(!isdigit(look))
			throw CComError(errEscapeSequence,noteCheckEscape);
		code = DoNumber();
		cPtr = (CComOBConstant*)cTab->Get(code);
		if(cPtr->dType != kwInt)
			throw CComError(errEscapeSequence,noteCheckEscape);
		c = cPtr->value.intData;
	}
	return c;
}

int CComLexical::DoOperator()
{
	assert(isoperator(look));

	int old = look;
	Step();

	switch(old)
	{
	case '~': return opBitNot;
	case '(': return opParentStart;
	case ')': return opParentEnd;
	case '[': return opBracketStart;
	case ']': return opBracketEnd;
	case '?': return opConditional;
	case '.': return opMemberDot;
	case ',': return opComma;

	case ':': 
		if(look == ':')
		{
			Step();
			return opMember4Dot;
		} else
			return opDoubleDot;

	case '%':
		if(look == '=')
		{
			Step();
			return opAssignModulo;
		} else
			return opModulo;

	case '^':
		if(look == '=')
		{
			Step();
			return opAssignXor;
		} else
			return opBitXor;

	case '!':
		if(look == '=')
		{
			Step();
			return opNotEqual;
		} else
			return opLogicNot;

	case '=':
		if(look == '=')
		{
			Step();
			return opEqual;
		} else
			return opAssign;

	case '+':
		switch(look)
		{
		case '+': Step(); return opPreIncrement;
		case '=': Step(); return opAssignPlus;
		default: return opPlus;
		}

	case '*':
		switch(look)
		{
		case '/': 
			throw CComError(errInvalidOperator, noteMayBeEndOfComment, "*/");
		case '=': Step(); return opAssignMultiply;
		default: return opMultiply;
		}

	case '&':
		switch(look)
		{
		case '&': Step(); return opLogicAnd;
		case '=': Step(); return opAssignAnd;
		default: return opBitAnd;
		}

	case '|':
		switch(look)
		{
		case '|': Step(); return opLogicOr;
		case '=': Step(); return opAssignOr;
		default: return opBitOr;
		}

	case '/':
		switch(look)
		{
		case '*': Step(); DoComment("/*"); return -1;
		case '/': Step(); DoComment("//"); return -1;
		case '=': Step(); return opAssignDivide;
		default: return opDivide;
		}

	case '-':
		switch(look)
		{
		case '-': Step(); return opPreDecrement;
		case '=': Step(); return opAssignMinus;
		case '>': Step(); return opMemberArrow;
		default: return opMinus;
		}

	case '>':
		switch(look)
		{
		case '>': 
			Step();
			if(look == '=')
			{
				Step(); 
				return opAssignRightShift;
			} else
				return opRightShift;
		case '=': 
			Step(); 
			return opGreaterEqual;
		default: return opGreater;
		}

	case '<':
		switch(look)
		{
		case '<': 
			Step();
			if(look == '=')
			{
				Step(); 
				return opAssignLeftShift;
			} else
				return opLeftShift;
		case '=': 
			Step(); 
			return opLessEqual;
		default: return opLess;
		}
	default:
		char buf[2];
		sprintf(buf,"%c",old);
		throw CComError(errInvalidOperator, noteNone,buf);
	}
}

void CComLexical::DoComment(const char* buf)
{
	assert(buf[0]=='/');
	if(buf[1]=='/')								// komentar	do konce radku
		while(look != '\n' && look != EOF)	// hledam konec radku
			look = GetParent();
	else
		if(buf[1]=='*')							// viceradkovy komentar - podporuji vnoreni
		{
			int old = ' ', num = 1;
			while(num && look != EOF)
			{
				if(look=='/' && old=='*')	// konec komentare
				{		
					num--;
					look=' ';	// kvuli /*/* apod.
				}
				if(look=='*' && old=='/')	// vnoreny komentar
				{
					num++;
					look=' ';
				}
				old = look;
				look = GetParent();
			}
			if(look == ' ')
				Step();
		}
}


bool CComLexical::ReadToken(char* buf, int maxlen, int(*test)(int),bool negate)
{
	int i = 0;
	while((negate ? !test(look) : test(look)) && i<maxlen && look != EOF)
	{
		buf[i++] = look;
		look = GetParent();
	}
	bool ret = i<maxlen;
	if(i >= maxlen)
		i = maxlen-1;
	buf[i] = '\0';
	return ret;
}

void CComLexical::ReadTokenE(char* buf, int maxlen, int(*test)(int),bool negate,int error,char* lookat)
{
	if(!ReadToken(buf,maxlen,test,negate))
		throw CComError(error,0,lookat);
}

void CComLexical::DoPreprocessor()
{
	char buf[cMaxDirectiveLn+1];
	CComOBPreprocessor* pre;

	assert(look == '#');
	buf[0] = look;
	Step();
	ReadTokenE(buf+1,cMaxDirectiveLn,isidentifier,false,errDirectiveTooLong,buf);
	pre = (CComOBPreprocessor*)pTab->FindRec(buf);
	if(!pre)
		throw CComError(errInvalidDirective,noteNone,buf);

	switch(pre->intData)
	{
	case dirDefine:
		DoDefine(); 
		break;
	case dirUndef:
		DoUndef(); 
		break;
	case dirIfdef:
		DoIfdef(noteCheckIfdef); 
		break;
	case dirIfndef:
		DoIfdef(noteCheckIfndef); 
		defined = !defined; 
		break;
	case dirEndif:
		DoEndif(); 
		break;
	case dirEmpty:
		break;
	default:
		throw CComError(errInvalidDirective, noteNone, pre->name);
	}	
}

void CComLexical::DoSymbolName(CComOBBase** ste, int note, char* name)
{
	char buf[cMaxFileNameLn+1],buf2[4];
	try
	{
		look = SkipWhite(false);
		if(isspace(look) || look == EOF)
			throw CComError(errMissingSymbolName);
		if(!isalpha(look))
			throw CComError(errExpectedIdentifier);
		ReadTokenE(buf,cMaxIdentifierLn,isidentifier,false,errDirectiveArgTooLong,buf);
		if(!isspace(look) && look != EOF)
		{
			sprintf(buf2,"'%c'",look);
			throw CComError(errUnexpectedCharacter,noteNone,buf2);
		}
		*ste = pTab->FindRec(buf);
		if(name)
			strcpy(name,buf);
	} catch(CComError e)
	{
		e.intData = note;
		throw e;
	}
}

void CComLexical::DoDefine()
{
	char buf[cMaxFileNameLn+1], name[cMaxFileNameLn+1];
	CComOBBase* ste;
	bool value = false;

	DoSymbolName(&ste,noteCheckDefine,name);

	look = SkipWhite(false);
	if(!isspace(look) && look != EOF)
	{
		value = true;
		ReadTokenE(buf,cMaxFileNameLn,isstring,false,errDirectiveArgTooLong,buf);
	} 

	if(!ste)
	{
		CComOBPreprocessor* newSym = new CComOBPreprocessor(name,pteDefinedSymbol,dsymDefined,value ? buf : 0);	
		pTab->Add(newSym);
	}
}

void CComLexical::DoUndef()
{
	CComOBBase* ste;

	DoSymbolName(&ste,noteCheckUndef);

	CComOBPreprocessor* pre = (CComOBPreprocessor*)ste;
	if(pre)
		pre->intData = dsymUndefined;
}

void CComLexical::DoIfdef(int note)
{
	CComOBBase* ste ;
	DoSymbolName(&ste,note);
	ifdefStack.Add(defined);
	CComOBPreprocessor* pre = (CComOBPreprocessor*)ste;
	defined = ste!=0 && pre->intData==dsymDefined;
}

void CComLexical::DoEndif()
{
	if(ifdefStack.IsEmpty())
		throw CComError(errEndifWithoutIf);
	defined = ifdefStack.RemoveHead();
}

void CComLexical::DoHead()
{
	char cat[cMaxDirectiveLn+1], val[cMaxFileNameLn+1];
	bool hasVersion, included;
	const char* valPtr;

	SkipWhite();			
	ReadTokenE(cat,cMaxDirectiveLn,isspace,true,errDirectiveArgTooLong,cat);
	if(strcmp(cat, "#head"))
		throw CComError(errMissingHead);

	hasVersion = included = false;

	do
	{
		look = SkipWhite();
		if(look == EOF)	throw CComError(errUnexpectedEOF, noteMissingEndHead);
		if(look == '#') break;
		ReadTokenE(cat,cMaxDirectiveLn,isspace,true,errDirectiveArgTooLong,cat);

		look = SkipWhite(false);
		if(look == EOF)	throw CComError(errUnexpectedEOF, noteMissingEndHead);
		if(look == '#')	break;
		ReadTokenE(val,cMaxFileNameLn,isspace,true,errDirectiveArgTooLong,val);

		if(look == EOF)	throw CComError(errUnexpectedEOF, noteMissingEndHead);

		if(!strcmp(cat,"version"))
		{
			CheckVersion(val);
			CComOBBase* newVersion = new CComOBBase(val,cUnknown,pos);
			int id = uTab->FindOrAdd(val,newVersion);
			Put(tokMetaVersion-id, false);
			hasVersion = true;
			pos.version = id;
		} else
			if(!strcmp(cat,"author"))
				strcpy(author, val);
			else
				if(!strcmp(cat,"game"))
					strcpy(game, val);
				else
					if(!strcmp(cat,"include"))
					{
						valPtr = val;					
						includes.Add(CComPosition(valPtr));
						included = true;
					}
	}	while(look != '#');

	ReadTokenE(cat,cMaxDirectiveLn,isspace,true,errDirectiveTooLong,cat);
	if(strcmp(cat,"#endhead"))
		throw CComError(errUnexpectedDirective,noteCheckHead,cat);
	
	if(!hasVersion)
		throw CComError(errMissingVersion,noteCheckHead);

	if(included)
	{
		CComPosition tmpPos = includes.RemoveHead();	
		StartFile(tmpPos.file, &tmpPos);
	}
}

void CComLexical::CheckVersion(const char* versionStr)
{

}
