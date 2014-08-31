/////////////////////////////////////////////////////////////////////////////
//
// refmgr.cpp
//
// Manazer objektu RefCnt
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "types.h"
#include "refmgr.h"

CRefMgr* RefMgr=NULL; 

CRefMgr::CRefMgr(int hashtablesize)
{
	hashtable = new CHashTable(hashtablesize);

}

CRefMgr::~CRefMgr()
{
	if(hashtable)
	{
		hashtable->DeleteAllMembers();
		SAFE_DELETE(hashtable);
	}
}

int CRefMgr::Add(char *name, CRefCnt *dataobject)
{
	if(!hashtable) return 0;

	if(hashtable->Member(name)) return 0;

	CRefMgrElem *el=new CRefMgrElem(name,dataobject);
	el->AddToHashTable(hashtable);

	return 1;
}

CRefCnt* CRefMgr::Find(char *name)
{
	if(!hashtable) return 0;

	CRefMgrElem *el = (CRefMgrElem*)hashtable->Member(name);

	if(!el) return 0;

	return el->GetData();
}


CRefMgrElem::CRefMgrElem(char *objectname, CRefCnt *dataobject)
{
	int ln=strlen(objectname);
	name = new char[ln+1];
	strcpy(name,objectname);

	data = dataobject;
	data->AddRef();
}

CRefMgrElem::~CRefMgrElem()
{
	if(name)
	{
		RemoveFromHashTable();
		delete[] name;
	}
	SAFE_RELEASE(data);
}