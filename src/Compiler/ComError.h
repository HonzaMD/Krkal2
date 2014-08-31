/////////////////////////////////////////////////////////////////////////////
//
// ComError.h
//
// Interface CComError - trida pro reprezentaci jedne chyby pri kompilaci
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ComPosition.h"
#include "ComConstants.h"

class CComError
{
public:
	CComError();
	CComError(int _errcode, int _intdata=0, const char* _strdata=0, CComShortPosition* _pos=0);
	CComError(CComError& e);

	virtual ~CComError();

	CComError& operator=(const CComError& e);

	int GetErrorMsg(const char* errorMsgFile, char* buf, int index = -1);	
	// Do buf zapise chybove hlaseni odpovidajici kodu index, resp. errorCode pri defaultni hodnote index. Hlaseni najde v souboru errorMsgFile, ktery sama otevre a zavre. Vraci 0 pri uspechu a -1 pokud nebylo hlaseni nalezeno.

	bool IsWarning() const { return errorCode > errWarningFirst; }
public:
	CComPosition pos;	// pozice, kde nastala chyba

	int errorCode;	// kod chyby
	int intData;	// data typu int - podle typu chyby
	char* strData;	// data typu char* - podle typu chyby
};
