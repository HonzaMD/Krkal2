/////////////////////////////////////////////////////////////////////////////
//
// ComFunctions.cpp
//
// Implementace globalnich funkci pouzivanych v kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComFunctions.h"
#include "ComConstants.h"
#include <assert.h>

int isoperator(int c)
{
	switch(c)
	{
	case '+': case '-': case '*': case '/':
	case '>': case '<': case '=': 
	case '&': case '|': case '^': case '!': case '~':
	case '(': case ')': case '[': case ']':
	case '?': case '%': case '.': case ',': case ':':
		return 1;
	default:
		return 0;
	}
}

int endsoperator(int c)
{
	switch(c)
	{
	case '~':	case '(': case ')': case '[': case ']':
	case '?': case '.': case ',':
		return 1;
	default:
		return 0;
	}
}

int isidentifier(int c)
{
	return (isalnum(c) || c == '_') && c < 128;
}

int isstring(int c)
{
	return c != '\n' && c != '\r';
}

int ishexa(int c)
{
	return isdigit(c) || (toupper(c)>='A' && toupper(c)<='F');
}

int isoctal(int c)
{
	return c>='0' && c<'8';
}

int isnumber(int c)
{
	return ishexa(c) || c=='.' || toupper(c)=='E' || toupper(c)=='X';
}

int getFileParts(const char* arg,char* file,char* path)
{
	assert(arg);
	assert(file);
	assert(path);
	int last = (int)strlen(arg) - 1;
	int i, j;

	if(last == 0)
		return cError;

	i = j = 0;
	file[0] = path[0] = '\0';
	while(i<=last)
	{
		file[j++] = arg[i];
		file[j] = '\0';
		if(arg[i] == '\\')
		{
			strcat(path, file);
			j = 0;
			file[0] = '\0';
		}
		i++;
	}
	return cOK;
}

