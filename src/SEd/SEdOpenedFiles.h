////////////////////////////////////////////////////////////////////////////////////
//
// SEdOpenedFiles.h
//
// udrzuje prehled o otevrenych souborech
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef SEDOPENEDFILES_H
#define SEDOPENEDFILES_H

//////////////////////////////////////////////////////////////////////
// CSEdOpenedFiles
//////////////////////////////////////////////////////////////////////

#include "types.h"
#include "GUIconstants.h"

class CSEdScriptEdit;

typedef CListK<class CSEdOFElem*> CSEdOpenFilesList;
typedef CListIterator<class CSEdOFElem*> CSEdOpenFilesListIt;

class CSEdOpenedFiles
{
public:
	CSEdOpenedFiles(int hashtablesize=128);
	~CSEdOpenedFiles();

	int Add(char *name, typeID id); //pokud jiz existuje otevreny soubor se stejnym jmenem vraci 0, jinak 1
	typeID Find(char *name);
	int Delete(char *name);

	CSEdScriptEdit* FindScripEdit(char *name);
	
	CSEdScriptEdit* GetCompileFile(){return compfile;}
	void SetCompileFile(CSEdScriptEdit* cf);	

	typeID FocusWnd(char *name); 

	CSEdOpenFilesList* GetOpenedFiles(){return &openFilesList;}

	int CompileForOB();
	void SetChange(){sthchng=1;}

protected:

	char *NormalizePath(char *path);

	CSEdScriptEdit *compfile;

	CHashTable *hashtable;
	CSEdOpenFilesList openFilesList;

	int sthchng,OBnotex;
};

extern CSEdOpenedFiles* SEdOpenedFiles; //globalni objekt

class CSEdOFElem: public CHashElem
{
public:
	CSEdOFElem(char *name, typeID);
	~CSEdOFElem();

	CSEdScriptEdit* GetScriptEdit();

	typeID GetData(){return data;}
protected:
	typeID data;
};

#endif