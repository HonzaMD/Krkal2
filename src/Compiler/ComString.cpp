/////////////////////////////////////////////////////////////////////////////
//
// ComString.cpp
//
// Implementace CComString - trida pro praci se stringy v kompilovanych skriptech
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComString.h"
#include "interpret.h"

CComString::CComString(int _ln, int line)
{
	assert(_ln > 0 && _ln <= csMaxStringLn);
	ln = _ln;
	data = (char*)KerInterpret->ISMalloc(line, ln + 1);
	data[0] = data[ln] = '\0';
	owner = true;
	lineContext = line;
}

CComString::CComString(const char* str, int line)
{
	assert(str);
	ln = strlen(str);
	assert(ln <= csMaxStringLn);
	data = (char*)KerInterpret->ISMalloc(line, ln+1);
	strcpy(data, str);
	owner = true;
	lineContext = line;
}

CComString::CComString(int _ln, void* mem, int line)
{
	ln = _ln;
	assert(_ln > 0 && _ln <= csMaxStringLn);
	data = (char*)mem;
	owner = false;
	lineContext = line;
}

CComString::CComString(CComString& str)
{
	ln = str.GetLn();
	strcpy(data, str);
	lineContext = str.lineContext;
}

CComString::~CComString()
{
	SafeDelete(csUnknown);
}

void CComString::SafeDelete(int line)
{
	if(owner)
		KerInterpret->ISFree(line, data);
}

CComString& CComString::operator=(CComString str)
{
	assert((int)strlen(str) <= ln);
	strcpy(data, str.data);
	return *this;
}

char CComString::operator[](int n)
{
	assert(n < ln);
	return data[n];
}

CComString CComString::operator+(CComString& str1)
{
	int newLen = strlen(data) + strlen(str1.data);
	assert(newLen <= csMaxStringLn);
	
	char buf[csMaxStringLn];
	
	strcpy(buf, data);
	strcat(buf, str1.data);

	return CComString(buf, lineContext);
}

char* CComString::operator+(int i)
{
	assert(i < ln);
	return data + i;
}

