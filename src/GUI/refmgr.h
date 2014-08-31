/////////////////////////////////////////////////////////////////////////////
//
// refmgr.h
//
// Manazer objektu RefCnt (vyhledavani objektu podle jmena)
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#ifndef REFMGR_H
#define REFMGR_H

#include "types.h"

class CRefMgr
{
public:
	CRefMgr(int hashtablesize=256);
	~CRefMgr();

	int Add(char *name, CRefCnt *dataobject); //pokud jiz existuje objekt se stajnym jmenem vraci 0, jinak 1 a zvysi # referenci u dataobjectu
	CRefCnt* Find(char *name); //najde objekt, pri chybe vraci NULL, nezvysuje # referenci

protected:
	CHashTable *hashtable;
};

extern CRefMgr* RefMgr; //globalni objekt

class CRefMgrElem: public CHashElem
{
public:
	CRefMgrElem(char *objectname, CRefCnt *dataobject);
	~CRefMgrElem();

	CRefCnt* GetData(){return data;}
protected:
	CRefCnt *data;
};

#endif