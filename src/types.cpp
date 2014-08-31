/////////////////////////////////////////////////////////////////////////////
//
// types.cpp
//
// typy a makra pouzivane v KRKALovi
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "types.h"
#include "fs.h"

#include <stdio.h> 
#include <stdarg.h>

//#define suprdebuk
void MyDebug(char* file, char* str, int i, int info)
{
#ifdef suprdebuk
	FS->ChangeDir("$COMPFILES$");
	FILE* f = fopen(file, "a");
	fprintf(f, "%s  %d", str, i);
	if(info != -1)
		fprintf(f, "   (%d)", info);
	fprintf(f, "\n");
	fclose(f);
#endif
}

void MyDebugTab(char* file, int tabs)
{
#ifdef suprdebuk
	FS->ChangeDir("$COMPFILES$");
	FILE* f = fopen(file, "a");
	while(tabs--)
		fprintf(f, "\t");
	fclose(f);
#endif
}

void MyDebugStr(char* file, char* str)
{
#ifdef suprdebuk
	FS->ChangeDir("$COMPFILES$");
	FILE* f = fopen(file, "a");
	fprintf(f, "%s", str);
	fclose(f);
#endif
}

char *newstrdup(const char *str)
{
	char *s;

	if (!str) return 0;

	if (s = new char[strlen(str) + 1] )
			return strcpy(s,str);

	return 0;
}

CExc::CExc(int etype, int par, char *format, ... )
{

	va_list list;
	
	va_start( list, format ); 
	_vsnprintf( errstr,sizeof(errstr), format, list );
	va_end( list );

	errnum = etype;
	param=par;

}




////////////////////////////////////////////////////////////////////////////////////////
///
///  HASH TABLE
///
///  A: M.D.
///
////////////////////////////////////////////////////////////////////////////////////////


// Vyhleda jmeno name v seznamu p2, kdyz tam je, tak ho ze seznamu odstrani
void CHashElem::HFindAndDelete(CHashList **p2,CHashElem *name) { 		
	CHashList *p3;
	while (*p2 && (**p2).name!=name) p2 = &(**p2).next;
	if (*p2) {
		p3=*p2;
		*p2=p3->next;
		delete p3;
	}
}



// Hashovaci funkce ze stringu
int CHashTable::HashFunction(const char *name) {
	int f=0,a=0;
	while (name[f]) {
		a+=name[f];
		f++;
	}
	return a & HashSize;
}



// Vyhledani podle jmena
CHashElem *CHashTable::Member(const char *name) {
	CHashList *p = HashTable[HashFunction(name)];
	int f;
	while (p) {
		f=0;
		while(name[f]==p->name->name[f]&&name[f]) f++; // porovnani stringu
		if (name[f]==p->name->name[f]) return p->name;
		p=p->next;
	}
	return 0;
}

// Vyhledani podle jmena - Vyhleda prvni vyskyt
CHashElem *CHashTable::MemberFirst(const char *name,CHashList **ptr) {
	CHashList *p = HashTable[HashFunction(name)];
	int f;
	while (p) {
		f=0;
		while(name[f]==p->name->name[f]&&name[f]) f++; // porovnani stringu
		if (name[f]==p->name->name[f]) {*ptr=p->next; return p->name;}
		p=p->next;
	}
	return 0;
}

// Vyhledani podle jmena - Vyhleda Nasledujici vyskyt
CHashElem *CHashTable::MemberNext(const char *name,CHashList **ptr) {
	CHashList *p = *ptr;
	int f;
	while (p) {
		f=0;
		while(name[f]==p->name->name[f]&&name[f]) f++; // porovnani stringu
		if (name[f]==p->name->name[f]) {*ptr=p->next; return p->name;}
		p=p->next;
	}
	return 0;
}



// Vyhledani podle jmena - Vyhleda posledni vyskyt
CHashElem *CHashTable::MemberLast(const char *name,CHashList **ptr) {
	*ptr = 0;
	return MemberPrev(name,ptr);
}

// Vyhledani podle jmena - Vyhleda Predchozi vyskyt
CHashElem *CHashTable::MemberPrev(const char *name,CHashList **ptr) {
	CHashList *p = HashTable[HashFunction(name)];
	CHashList *stop = *ptr;
	int f;
	while (p != stop) {
		f=0;
		while(name[f]==p->name->name[f]&&name[f]) f++; // porovnani stringu
		if (name[f]==p->name->name[f]) {*ptr=p;}
		p=p->next;
	}
	if (*ptr!=stop) return (**ptr).name; else return 0;
}



// Odebere Polozky z tabulky a tabulku zrusi
CHashTable::~CHashTable() {
	int f;
	CHashList *p,*p2;
	for (f=0;f<=HashSize;f++) {
		p=HashTable[f];
		while(p) {
			p2=p;
			p=p->next;
			p2->name->HashTable=0;
			delete p2;
		}
	}
	delete[] HashTable;
}



void CHashTable::DeleteAllMembers() {
	int f;
	CHashList *p,*p2;
	for (f=0;f<=HashSize;f++) {
		p=HashTable[f];
		while(p) {
			p2=p;
			p=p->next;
			p2->name->HashTable=0;
			delete p2->name;
			delete p2;
		}
		HashTable[f]=0;
	}
}