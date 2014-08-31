////////////////////////////////////////////////////////////////////////////////////
//
// SEdOpenedFiles.h
//
// udrzuje prehled o otevrenych souborech
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "sedopenedfiles.h"
#include "scriptedit.h"

#include "gui.h"
#include "window.h"

#include "sedobjectbrowser.h"
#include "namebrowser.h"
#include "SEdDesktop.h"

#include "comcompiler.h"

CSEdOpenedFiles* SEdOpenedFiles=NULL; 

CSEdOpenedFiles::CSEdOpenedFiles(int hashtablesize)
{
	hashtable = new CHashTable(hashtablesize);

	compfile = NULL;

	sthchng = 1;
	OBnotex = 0;
}

CSEdOpenedFiles::~CSEdOpenedFiles()
{
	if(hashtable)
	{
		hashtable->DeleteAllMembers();
		SAFE_DELETE(hashtable);
	}
}

int CSEdOpenedFiles::Add(char *name, typeID id)
{
	if(!hashtable) return 0;

	char *n = NormalizePath(name);

	if(hashtable->Member(n)) 
	{
		delete[]n;
		return 0;
	}

	CSEdOFElem *el=new CSEdOFElem(n,id);
	el->AddToHashTable(hashtable);

	openFilesList.Add(el);

	delete[]n;

	return 1;
}

typeID CSEdOpenedFiles::Find(char *name)
{
	if(!hashtable) return 0;

	char *n = NormalizePath(name);

	CSEdOFElem *el = (CSEdOFElem*)hashtable->Member(n);

	delete[]n;

	if(!el) return 0;
	return el->GetData();
}

CSEdScriptEdit* CSEdOpenedFiles::FindScripEdit(char *name)
{
	if(!hashtable) return 0;

	char *n = NormalizePath(name);

	CSEdOFElem *el = (CSEdOFElem*)hashtable->Member(n);

	delete[]n;

	if(!el) return 0;
	return el->GetScriptEdit();
}

int CSEdOpenedFiles::Delete(char *name)
{
	if(!hashtable) return 0;

	char *n = NormalizePath(name);

	CSEdOFElem *el = (CSEdOFElem*)hashtable->Member(n);
    
	delete[]n;

	if(!el) return 0;

	if( el->GetScriptEdit() == compfile )
		SetCompileFile(NULL);

	openFilesList.Delete(el);

	delete el;

	return 1;
}

typeID CSEdOpenedFiles::FocusWnd(char *name)
{
	typeID id = Find(name);
	if(!id) return 0;

	CGUIWindow* el = dynamic_cast<CGUIWindow*>(nameServer->ResolveID(id));

	if(el) el->FocusMe();
	
	return id;
}

char *CSEdOpenedFiles::NormalizePath(char *path)
{
	char *p = newstrdup(path);
	UC *c=(UC*)p;

	while(*c){
		if(*c=='\\') 
			*c='/';
		else
			*c = tolower(*c);
		c++;
	}

	return p;
}

void CSEdOpenedFiles::SetCompileFile(CSEdScriptEdit* cf)
{
	if(compfile==cf) return;

	SetChange();

	compfile = cf;

	CSEdOpenFilesListIt iter(GetOpenedFiles());
	CSEdScriptEdit *se;

	while(!iter.End())
	{
        se = iter->GetScriptEdit();
		if(se)
			se->ShowCurCompFile();
		
		iter++;
	}

	CompileForOB();
}

int CSEdOpenedFiles::CompileForOB()
{
	int ok=0;

	if(!SEdDesktop) return 0;
	if(!sthchng) return 0;

	CSEdObjectBrowser* ob = SEdDesktop->GetObjectBrowser();
	CNameBrowser* nb = SEdDesktop->GetNameBrowser();

	if(!ob&&!nb) return 0;

	if(ob) sthchng = 0;

	if(compfile)
	{
		if(compiler && compiler->SECompile(compfile,false,false))
			ok = 1;
	}

	if(ob) 
	{
		if(ok) 
			ob->UpdateOB(); 
		else
			ob->CompileError();
	}

	if(nb) nb->UpdateNB();

	return 1;
}

//----------------------------------------------------------------

CSEdOFElem::CSEdOFElem(char *_name, typeID id)
{
	int ln=strlen(_name);
	name = new char[ln+1];
	strcpy(name,_name);

	data = id;
}

CSEdOFElem::~CSEdOFElem()
{
	if(name)
	{
		RemoveFromHashTable();
		delete[] name;
	}
}

CSEdScriptEdit* CSEdOFElem::GetScriptEdit()
{
	CSEdScriptEdit *se = dynamic_cast<CSEdScriptEdit*>(nameServer->ResolveID(data));

	return se;
}