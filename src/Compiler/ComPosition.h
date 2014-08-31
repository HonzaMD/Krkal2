/////////////////////////////////////////////////////////////////////////////
//
// ComPosition.h
//
// Interface CComPosition - tøída pro reprezentaci pozice ve  zdrojovem programu pri kompilaci
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "ComConstants.h"

#pragma once

class CComPosition
{
public:
	CComPosition();
	CComPosition(const char* _file, int line=1, int column=0);
	CComPosition(CComPosition& p);

	CComPosition& operator=(const CComPosition& p);
	virtual ~CComPosition();

	bool TopTheBill();

public:
	char* file;		// soubor
	int line;		// radka
	int column;		// sloupec
	int plusLine;
	int plusColumn;		
	unsigned char* openFile;	// handle otevreneho souboru nebo 0
	int	fileID;		// index do tabulky symbolu preprocesoru - polozka obsahuje jmeno souboru
	int index;		// index na aktualni znak v souboru
	int size;		// velikost souboru
	int version;	// verze souboru
	bool scriptEditorWindow;	// neni to soubor z disku, ale okno editoru skriptu (pretypovany pointer v openFile)
	bool startingFile;
};

struct CComShortPosition
{
	CComShortPosition();
	~CComShortPosition();

	CComShortPosition(CComShortPosition& p);
	CComShortPosition(CComPosition& p);
	
	CComShortPosition& operator=(const CComShortPosition& p);
	CComShortPosition& operator=(const CComPosition& p);

	int fileID;		// ID souboru - index do tabulky symbolu preprocesoru
	int line;		// radka
	int column;		// sloupec
};