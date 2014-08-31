/////////////////////////////////////////////////////////////////////////////
//
// ComPosition.cpp
//
// Implementace CComPosition - tøída pro reprezentaci pozice ve  zdrojovem programu pri kompilaci
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComPosition.h"
#include "ComConstants.h"
#include "types.h"

CComPosition::CComPosition()
{
	fileID = cUnknown;
	file = 0;
}

CComPosition::CComPosition(const char* _file, int _line, int _column)
{
	if(_file)
	{
		file = newstrdup(_file);
		assert(file);
	} else
		file = 0;
	line = _line;
	column = _column;
	plusLine = plusColumn = 0;
	openFile = 0;
	fileID = version = cUnknown;
	index = size = 0;
	scriptEditorWindow = false;
	startingFile = true;
}

CComPosition::CComPosition(CComPosition& p)
{
	if(p.file)
	{
		file = newstrdup(p.file);
		assert(file);
	} else
		file = 0;
	line = p.line;
	column = p.column;
	plusLine = p.plusLine;
	plusColumn = p.plusColumn;
	openFile = p.openFile;
	fileID = p.fileID;
	index = p.index;
	size = p.size;
	version = p.version;
	scriptEditorWindow = p.scriptEditorWindow;
	startingFile = p.startingFile;
}

CComPosition& CComPosition::operator=(const CComPosition& p)
{
	SAFE_DELETE_ARRAY(file);
	if(p.file)
	{
		file = newstrdup(p.file);
		assert(file);
	} else
		file = 0;
	line = p.line;
	column = p.column;
	plusLine = p.plusLine;
	plusColumn = p.plusColumn;
	openFile = p.openFile;
	fileID = p.fileID;
	index = p.index;
	size = p.size;
	version = p.version;
	scriptEditorWindow = p.scriptEditorWindow;
	startingFile = p.startingFile;
	return *this;
}

CComPosition::~CComPosition()
{
	SAFE_DELETE_ARRAY(file);
}

bool CComPosition::TopTheBill()
{
	bool ret = false;
	if(plusLine)
	{
		line += plusLine;
		plusLine = 0;
		column = plusColumn;
		ret = true;
	} else
		column += plusColumn;

	plusColumn = 0;
	return ret;
}


CComShortPosition::CComShortPosition()
{
	fileID = line = column = cUnknown;
}

CComShortPosition::~CComShortPosition()
{
}

CComShortPosition::CComShortPosition(CComShortPosition& p)
{
	fileID = p.fileID;
	line = p.line;
	column = p.column;
}

CComShortPosition::CComShortPosition(CComPosition& p)
{
	fileID = p.fileID;
	line = p.line;
	column = p.column;
}
	
CComShortPosition& CComShortPosition::operator=(const CComShortPosition& p)
{
	fileID = p.fileID;
	line = p.line;
	column = p.column;
	return *this;
}

CComShortPosition& CComShortPosition::operator=(const CComPosition& p)
{
	fileID = p.fileID;
	line = p.line;
	column = p.column;
	return *this;
}
