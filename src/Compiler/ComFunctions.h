/////////////////////////////////////////////////////////////////////////////
//
// ComFunctions.h
//
// Interface globalnich funkci pouzivanych v kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#if !defined COM_COMFUNCTIONS_H__INCLUDED_

#define COM_COMFUNCTIONS_H__INCLUDED_


int isoperator(int c);		// vraci nenulovou hodnotu, pokud je c operator
int endsoperator(int c);	// vraci nenulovou hodnotu, pokud je c znak, ktery ukoncuje viceznakovy operator
int isidentifier(int c);	// vraci nenulovou hodnotu, pokud je c platny znak identifikatoru
int isstring(int c);		// vraci nenulovou hodnotu, pokud je c platny znak retezce
int ishexa(int c);			// vraci nenulovou hodnotu, pokud je c hexadecimalni cislice
int isoctal(int c);			// vraci nenulovou hodnotu, pokud je c oktalova cislice
int isnumber(int c);		// vraci nenulovou hodnotu, pokud je c platny znak desitkove cislice

int getFileParts(const char* arg,char* file,char* path);	// rozdeli jmeno souboru arg na cestu (path) a samotne jmeno souboru (file)

#endif
