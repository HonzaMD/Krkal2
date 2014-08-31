////////////////////////////////////////////////////////////////////////////////////////////////
//
// FS.h
//
// veci okolo filesystemu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////

/*

Co umi filesystem?
------------------
Umoznuje balit soubory a adresare do jednoho souboru archivu.
Archiv se chova stejne jako adresar. Je tedy mozne normalne prochazet adresarovou
strukturou i dovnitr archivu.

Filesystem umoznuje kompresi.
Neumoznuje uzivatelska prava - musi si to zaridit sam uzivatel.

Hlavni objekty:
---------------

class CFS
	hlavni trida FileSystemu
	zastinuje archivy, takze je mozne do nich normalne lizt
	pri startu je vytvorena instance CFS - pointr na ni je v globalni prom. FS
			
class CFSArchive
	reprezentuje jeden archiv
	struktura souboru je popsana na zacatku fs.cpp

*/


#ifndef FS_H
#define FS_H

#include <stdio.h>
#include "types.h"
#include "crc.h"


//forward deklarace
class CFSFolder;
class CFSArchive;

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFS
//
// "filesystem" - umoznuje pracovat s archivy (CFSArchive) jako s adresari
//
// muze byt vytvorena jen jedna instance CFS
//
// pouzivejte globalni objekt FS !!!
//
////////////////////////////////////////////////////////////////////////////////////////////////

extern class CFS *FS; //globalni objekt FS

class CFS
{
public:
	static int InitFS(); //initace - vola se z KRKAL.cpp
	static void DoneFS(); //deinitace - dtto

	int ChangeDir(const char *path); //zmeni akt. adr., vraci 0=err 1=OK
	int ChangeDirByFile(const char *filepath); //nastavi akt. adr. do adresare, kde je zadany soubor (umaze jmeno za poslednim lomitkem), vraci 0=err, 1=OK

	int GetFileSize(const char *path); //vraci velikost souboru, pokud je chyba vraci 0
	int ReadFile(const char *name, char *buf, int bufsize); //nacte soubor do bufferu, vraci 0=err 1=OK; bufsize je velikost buferu -> pokud se soubor cely nevejde nacte se zacatek a funkce vrati 2
	int WriteFile(const char *name, char *buf, int size, int compression=-1);  //zapise soubor, vraci 0=err 1=OK, compression: 0=no compr, 1=zlib, -1=stejne jako existujici soubor (pokud neex.=1)
	int CreateDir(const char *name); //vytvori adresar, vraci 0=err 1=OK

	int Delete(const char *name); //smaze soubor/adresar/archiv, vcetne vnorenych souboru a adresaru!!! (tak bacha ;-) (lze mazat i v nadrazenych adresarich - potom ale neni platny akt. adresar - musi se pak zavolat changedir s abs. cestou!), vraci 0=err 1=OK

	int CreateArchive(const char *name); //vytvori archiv, vraci 0=err 1=OK

	int GetTime(const char *name, FILETIME &time); //vrati datum a cas souboru/adresare
	int SetTime(const char *name, const FILETIME &time); //nastavi datum a cas souboru/adresare

	int IsCompressed(const char *name); //vraci jestli je soubor komprimovany

	int FileExist(const char *name); //testuje jestli existuje soubor/adresar; vraci 0=neex. 1=soubor 2=adresar 3=archiv

	int GetCurDir(char **dir); //do dir ulozi aktualni abs. cestu, dir se alokuje, vola new; vraci 0=err 1=OK
	int GetFullPath(const char *relpath, char **fullpath); //vrati celou cestu, vola new na fullpath; vraci 0=err 1=OK

	int AddFSDir(const char *key, const char *val, int newkey=1); //prida adresar ( nastavi $key$ = val )

	//Hledani vsech souboru(a adresaru) v akt. adresari
	//navratove hodnoty: -1=error, 0=neexistuje zadny dalsi soubor 1=Ok
	//dir: 0=soubor 1=adresar 2=archiv
	//filename: po volani si nekam zkopirujte, nedeletovat
	//findclose neni nutne volat
	int FindFirst(const char **filename, int &dir); 
	int FindNext(const char **filename, int &dir);
	int FindClose();

	int CopyTree(const char *sourcedir, const char *destdir, int comp=-1); //zkopiruje vse z adr. sourcedir do destdir

	int IsValidFilename(const char *filename); //testuje jestli 'filename' je platny nazev souboru (tj. neobsahuje / a dalsi znaky)

	int Defragment(const char *archvename); //defragmentuje archiv

	int ComparePath(const char *path1, const char *path2); //porovna 2 cesty jestli jsou stejny, vraci 0 kdyz jsou stejny, vlastne dela case insensitive cmp, navic '\'='/'

private:
	CFS();
	~CFS();

	CFSFolder *curdir; //aktualni adresar

	CFSArchive *curarchive; //archiv, ve kterym je akt. adresar; archiv nemusi byt otevreny

	int SetCurDir(CFSFolder *dir); //nastavi akt. adresar (zmeni curdir na disku, otevre archiv a nastavi v nem adresar); pokud nastane chyba necha stary curdir
	int SetCurDir(); //zmeni akt. adr. na disku, pokud neni otevreny zadny archiv, tak ho otevre a nastavi v nem dir

	int SplitPathAndSaveCurDir(const char *path, CFSFolder **folder, const char **file, int allowarchive=0, int allowemplyfilename=0); //rozdeli cestu na adresar a soubor, nastavi akt. adresar, puvodni adresar ulozi do folder(vola new), jmeno souboru je v file - je to je pointr do path

	static int FSex; //1=jiz je vytvorena instance CFS

	class CFSCfg *cfg;

	int inFind;
	
	struct CFSFindInfo{
		CFSFolder *finddir; //adresar, ve kterem se hledaji soubory; pokud je =NULL je stejny jako curdir
		int findArchiveLastNum;
		HANDLE findHandle;
		WIN32_FIND_DATA FindFileData;
	} findinfo;

	int CopyTree2(const char *destdir, int comp); //zkopiruje vse z akt. adr. do destdir

};


////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSFolder
//
// adresar FS, muze byt na disku, nebo v archivu CFSArchive
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSFolder
{
public:
	CFSFolder();
	~CFSFolder();

	CFSFolder(const CFSFolder &f);

	// cesta:  'path'+'/'+archivename+'pathinarchive'
	char *path; //absolutni cesta na disku k akt. adresari, pokud je aktualni adresar v archivu, tak je to jen cesta k archivu, zbytek cesy je v 'pathinarchive'
	char *archivename; //jmeno archivu (bez cesty)
	char *pathinarchive; //cesta v aktualnim archivu

	int archivedepth; //hloubka zanoreni v archivu: 0=neni v archivu, 1=rootdir, ...

	int chdir(const char *name, int archive=0); //zmeni adresar; nastavit archive na 1, kdyz se leze do archivu
												//name muze byt jen jednoduchy (bez '/','\'), podporuje jen '..'

	int SetDir(const char *dir); //nastavi adr, dir je absolutni cesta, neni v archivu

	int GetDir(char **dir, const char *filename=NULL); //do dir ulozi aktualni abs. cestu, dir se alokuje, vola new; vraci 0=err 1=OK

private:
	int pathlen,pathinarchivelen,archivenamelen; //velikost bufferu
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSCfg
//
// definice adresarovych zkratek: $neco$
// nacita se ze souboru (KRKALfs.cfg)
//
////////////////////////////////////////////////////////////////////////////////////////////////

#define FS_MAX_CFGPATH 100
#define FS_MAX_KEYLEN 25
#define FS_MAX_VALLEN 1024

class CFSCfg
{
public:
	CFSCfg(const char *fname); //nacte konfiguraci
	~CFSCfg();

	int ParseString(const char *instr, char **outstr); //vezme vstupni string a nahradi v nem $...$, vysledek ulozi do outstr, alokuje outstr

	int AddKey(const char *key, const char *val, int newkey=1); //prida klic

	int ReadCfg(const char *fname); //nacte konfiguraci

private:
	
	char *keys[FS_MAX_CFGPATH];
	char *vals[FS_MAX_CFGPATH];
	int vallen[FS_MAX_CFGPATH];
	int numkeys;

	int FindKey(const char *key);

};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSDirItem
//
// jedna polozka adresare archivu
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSDirItem
{
	friend class CFSDir;
public:

	CFSDirItem(){name=NULL;dir=0;offset=0; next=NULL;}
	~CFSDirItem(){SAFE_DELETE_ARRAY(name); SAFE_DELETE(next); }

	int Create(const char *name, char dir, int offset);
	int Read(FILE *f);
	
	void CmpCRC(CCRC32 &crc);
	int GetSize();	

	int GetOffset(){return offset;}
	void GetTime(FILETIME &tm){tm=ittime;}
	void SetTime(const FILETIME &tm) {ittime=tm;}

protected:

	int Write(FILE *f);

	char *name;
	unsigned char namelen;
	char dir;
	int offset;
	FILETIME ittime;
	CFSDirItem *next;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSDir
//
// adresar archivu
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSDir
{
	friend class CFSDirMgr;
	friend class CFSDirRef;
public:

	int Create(int parentoffset);
	int Read(int offset);

	int CreateSubDir(const char *name);

	int WriteFile(const char *name, const char *buf, int size, unsigned char compr);
	int GetFileSize(const char *name);
	int ReadFile(const char *name, char *buf, int bufsize);

	int IsCompressed(const char *name);
	int GetTime(const char *name, FILETIME &item_time);
	int SetTime(const char *name, const FILETIME &item_time);

	int GetSize();
	int GetOffset(){return offset;}
	int GetParentOffset(){return parentoffset;}

	int FileExist(const char *name);

	int FindItem(const char *name,CFSDirItem*** it, int namelen=-1);
	int FindItem(int offset,CFSDirItem*** it);

	int Rename(const char *oldname, const char *newname, CFSDir *newdir);
	int Delete(const char *name);

	int DefragDir(int newoffset); //pouziva se pri defragmentaci

	int GetNumItems(){return numitems;}
	int GetItem(int itnum, const char **name, int &dir);

	int IsValid(){return valid;}

private:
	CFSDir(CFSArchive *archive);
	~CFSDir();

	int Write();
	int WriteDirData();
	int WriteFileData(const char *buf, int size, int oldoffset, unsigned char compr);
	int ReadFileData(int offset, char *buf, int bufsize);

	CFSDirItem *items;
	int numitems;
	
	int offset;
	int wrsize;
	int parentoffset;
	
	int valid;
	void Invalidate(){valid=0;}

	int RefCnt;

	CFSArchive *archive;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSDirMgr
//
// "manazer" adresaru - pocita reference na adresar, zajistuje aktualnost CFSDir
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSDirMgr
{
public:
	CFSDirMgr(CFSArchive *archive);
	~CFSDirMgr();

	CFSDir* ReadDir(int offset);
	CFSDir* CreateDir(int parrentoffset);

	void DeleteDir(CFSDir *dir);
	void DeleteAll();

private:
	CFSDir* FindDir(int offset);

	void Add(CFSDir *dir);

	CListK<CFSDir*> list;
	CFSArchive *archive;
};


////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSDirRef
//
// umoznuje zapamatovat si aktualni adresar archivu
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSDirRef
{
	friend class CFSArchive;
public:
	void Close();

private:

	CFSDirRef(CFSDir *dir);
	~CFSDirRef();

	CFSArchive* GetArchive();

	CFSDir *dir;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSEmptySpace, CFSEmptySpaceItem
//
// volne misto uvnitr archivu
//
////////////////////////////////////////////////////////////////////////////////////////////////


struct CFSEmptySpaceItem
{
	int offset;
	int size;
};

class CFSEmptySpace
{
	friend CFSArchive;
public:
	CFSEmptySpace(int maxItems, CFSArchive *archive);
	~CFSEmptySpace();

	int AddEmptySpace(int size, int offset);
	int GetEmptySpace(int size, int &offset);

	int WriteEmptySpace(int offset);
	int ReadEmptySpace(int offset);

	int CmpOffsetAfterDefrag(int oldoffset);

	void DeleteAll(){numItems=0;}

	int NeedDefrag();

private:
	int numItems;
	int maxItems;
	CFSEmptySpaceItem *items;
	CFSArchive *archive;

	void Add(int offset, int size);
	void Delete(int i);
	void Change(int i, int newoffset, int newsize);

	DWORD CmpCRC();
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFSArchive
//
// Archiv souboru
//
////////////////////////////////////////////////////////////////////////////////////////////////

class CFSArchive
{
	friend class CFSEmptySpace;
	friend class CFSDir;
public:

	CFSArchive();
	~CFSArchive();

	int Create(const char *path);
	int Open(const char *path, int _readonly);

	void Close();

	int Defragment();

	int ChangeDir(const char *path);
	CFSDirRef* GetDir();
	int ChangeDir(CFSDirRef* dir);

	int CreateDir(const char *name);

	int Delete(const char *path);

	int WriteFile(const char *name, const char *buf, int size, unsigned char compr=-1);
	int GetFileSize(const char *name);
	int ReadFile(const char *name, char *buf, int bufsize);

	int IsCompressed(const char *name);

	int GetTime(const char *name,FILETIME &time);
	int SetTime(const char *name,const FILETIME &time);

	int Rename(const char *oldname, const char *newname);

	int FileExist(const char *fname); //vraci 0-neex. 1-soubor 2-adresar

	CFSDirMgr* GetDirMgr(){return DirMgr;}

	int GetCurDirNumItems();
	int GetCurDirItem(int itnum, const char **name, int &dir);

private:

	int SplitPath(const char *path, CFSDir **dir, int &fileindex);

	int FindDir(const char *path, CFSDir **dir, int pathlen=-1);

	void WriteArchiveHead();
	int ReadArchiveHead();
	DWORD CmpArchiveHeadCRC();

	FILE *file;
	int readonly;
	int archivesize;

	int RootDirOffset;
	int EmptySpaceTableOffset;
	int MaxEmptySpaceItems;
	CFSEmptySpace *emptyspace;

	CFSDir *curdir;

	CFSDirMgr *DirMgr;

};


#endif