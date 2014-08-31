/////////////////////////////////////////////////////////////////////////////
//
// ComError.cpp
//
// Implementace CComError - trida pro reprezentaci jedne chyby pri kompilaci
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <stdio.h>

#include "comerror.h"
#include "types.h"


// *** default constructor ***
CComError::CComError()
{
	errorCode = 0;
	intData = 0;
	strData = 0;
}

// *** constructor ***
CComError::CComError(int _errorCode, int _intData, const char* _strData, CComShortPosition* _pos)
{
	errorCode = _errorCode;
	intData = _intData;
	if(_strData)
	{
		strData = newstrdup(_strData);
		assert(strData);
	} else
		strData = 0;
	if(_pos)
	{
		pos.fileID = _pos->fileID;
		pos.line = _pos->line;
		pos.column = _pos->column;
	}
}

// *** copy constructor ***
CComError::CComError(CComError& e)
{
	pos = e.pos;
	
	errorCode = e.errorCode;
	intData = e.intData;
	if(e.strData)
	{
		strData = newstrdup(e.strData);
		assert(strData);
	} else
		strData = 0;
}

// *** destructor ***
CComError::~CComError()
{
	SAFE_DELETE_ARRAY(strData);
}

// *** operator = ***
CComError& CComError::operator=(const CComError& e)
{
	pos = e.pos;
	
	errorCode = e.errorCode;
	intData = e.intData;
	if(strData)
		SAFE_DELETE_ARRAY(strData);
	if(e.strData)
	{
		strData = newstrdup(e.strData);
		assert(strData);
	} else
		strData = 0;
	return *this;
}

// Do buf zapise chybove hlaseni odpovidajici kodu index, resp. errorCode pri defaultni hodnote index. Hlaseni najde v souboru errorMsgFile, ktery sama otevre a zavre. Vraci 0 pri uspechu a -1 pokud nebylo hlaseni nalezeno.
int CComError::GetErrorMsg(const char* errorMsgFile, char* buf, int index)
{
	int i = 0, j;
	int c;
	
	if(index == -1)			// default index==-1 -> pozadovana ErrorMsg pro errorCode
		index = errorCode;

	FILE* f = fopen(errorMsgFile, "r");	// otevru soubor s chybovymi hlasenimi
	if(!f)
		return -1;

	while(!feof(f))
	{
		fscanf(f,"%d",&i);	// i = cislo na zacatku radku
		c = ' ';
		while(isspace(c)) c = fgetc(f);	// preskocim whitesppace
			
		j = 0;
		buf[j++] = c;
		while(c != '\n' && !feof(f))	// plnim bufer az do konce radku, resp. souboru
			buf[j++] = c = fgetc(f);
		buf[j-1] = '\0';
		if(i==index)					// nasel jsem co potrebuju? 
		{
			fclose(f);
			return 0;	// OK, konec.
		}
	}
	fclose(f);
	return -1;	// Chyba, konec.
}


