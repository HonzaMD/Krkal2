////////////////////////////////////////////////////////////////////////////////////////////////
//
// FS.cpp
//
// veci okolo filesystemu
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fs.h"
#include "crc.h"

#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <direct.h>

#include <math.h>
#include <zlib.h>

CFS *FS=NULL;

enum eFSerr{
	eFSgeneral,
	eFSwrite,
	eFSread,
	eFScrc,
	eFSseek,
	eFSflush
};

#define FOPEN(file,mode) ( file[0]!=0 ? fopen(file,mode) : NULL )
#define READ(buf,size,file) if(size&&fread(buf,size,1,file)==0) throw CExc(eFS,eFSread,"Can't read from archive!")
#define WRITE(buf,size,file) if(size&&fwrite(buf,size,1,file)==0) throw CExc(eFS,eFSwrite,"Can't write to archive!")
#define SEEK(file,offset) if(fseek(file,offset,SEEK_SET)!=0) throw CExc(eFS,eFSseek,"Can't seek in archive!")
#define FLUSH(file) if(fflush(file)!=0) throw CExc(eFS,eFSflush,"Can't flush archive!")

//zvetsi na nasobek 32
#define CMP_CEIL(size) ( ( ( (size)+31 ) >>5 ) <<5 )
//pocita skutecnou zabranou velikost souboru (pricte velikost hlavicky a zvetsi na nasobek 32)
#define CMP_FILE_SPACE(filesize) CMP_CEIL(1+4+1+4+filesize)



///////////////////////////////////////////////////////////////////////////////////////////
/*
	
	Archive structure
	-----------------

	Head of archive
	---------------
	offset	size	desc.
	0		9		"KRKAL_FS"+0
	9		5		"v1.0"+0
	14		4		RootDirOffset=EmptySpaceTableOffset+EmptySpaceHeadLen+MaxEmptySpaceItems*EmptySpaceItemLen
	18		4		EmptySpaceTableOffset=30
	22		4		MaxEmptySpaceItems
	26		4		CRC

	30				EmptySpaceTable
*/
const char sFS_KRKAL[]="KRKAL_FS";
const char sFS_VER[]="v1.0";
const int defaultEmptySpaceTableOffset=30;

/*
	EmptySpaceTable
	---------------
	offset	size	desc.
	0		1		# of items.(=n)
	1		4		CRC
	
	5		4		offset to 1.EmptyBlock
	9		4		size of 1.EmptyBlock
	
	13		4		offset to 2.EmptyBlock
	17		4		size of 2.EmptyBlock
	.....
*/
const int EmptySpaceHeadLen=5;
const int EmptySpaceItemLen=8;

/*
	File/Dir Head
	-------------
	offset	size	desc.	
	0		1		file/dir (dir=1)

	File
	----
	offset	size	desc.
	0		4		size of (compressed) file = SIZE of DATA
	4		1		compressed (0=no compr.,1=zlib)
	5		4		CRC of data
	9		??		DATA
*/

/*
	Compressed File DATA
	--------------------
	offset	size	desc.
	0		4		size of uncompressed file
	4		?		DATA
*/
const int COMPRESSED_FILE_HEAD_SIZE = 4;

/*
	Dir
	---
	offset	size	desc.
	0		4		# of items
	4		4		CRC


	DirItem
	-------
	offset	size	desc.
	0		1		namelen
	1		namelen	name
	+0		1		file/dir (dir=1)
	+1		4		offset
	+5		8		date&time (FILETIME)
*/


//////////////////////////////////////////////////////////////////////////////////////////
/*

	Compressed Single File (komprimovany soubor, ktery neni v archivu)
	----------------------
	offset	size	desc.
	0		15		"KRKAL_COMPFILE"+0
	15		5		"v1.0"+0
	20		4		uncompressed len
	24		4		compressed len
	28		4		head CRC
	32		complen	compressed DATA

*/						   
const char sFS_COMPFILE[]="KRKAL_COMPFILE";
const char sFS_COMPFILE_VER[]="v1.0";
const int FS_COMPFILE_HEADLEN=sizeof(sFS_COMPFILE)+sizeof(sFS_COMPFILE_VER)+4+4+4; 

//////////////////////////////////////////////////////////////////////////////////////////



CFSEmptySpace::CFSEmptySpace(int _maxItems, CFSArchive *ar)
{

	assert(ar&&ar->file);

	maxItems =_maxItems;
	numItems = 0;

	items = new CFSEmptySpaceItem[maxItems];
	
	archive=ar;

}
CFSEmptySpace::~CFSEmptySpace()
{
	SAFE_DELETE_ARRAY(items);
}

int CFSEmptySpace::AddEmptySpace(int size, int offset)
{
	int i,pr,nx,o;

	assert(numItems<maxItems);

	if(offset+size==archive->archivesize)
	{
		//prazdny misto je na konci souboru -> zmensim soubor
		archive->archivesize-=size;
		

		i=numItems-1; //posledni
		if(items[i].offset+items[i].size==archive->archivesize)
		{ //posledni prazdne misto se dostalo na konec souboru -> zmensim soubor
			archive->archivesize-=items[i].size;
			Delete(i);
			WriteEmptySpace(archive->EmptySpaceTableOffset);
		}

		_chsize(_fileno(archive->file),archive->archivesize);

		return 1;
	}

	pr=nx=-1;


	//najde blok pred a za
	for(i=0;i<numItems;i++)
	{
		o=items[i].offset+items[i].size;
		if(o>=offset)
		{
			if(o==offset) pr=i;
			if(i+1<numItems&&items[i+1].offset==offset+size) nx=i+1;
			if(items[i].offset==offset+size) nx=i;
			break;
		}
	}

	//slouci volny bloky vedle sebe
	if(pr!=-1&&nx!=-1)
	{
		int o,s;
		o=items[pr].offset;
		s=items[pr].size+size+items[nx].size;
		
		Delete(nx);
		Change(pr,o,s);

	}else
	if(pr!=-1)
		Change(pr,items[pr].offset,items[pr].size+size);
	else
	if(nx!=-1)
		Change(nx,offset,size+items[nx].size);
	else
	{
		//prida volny blok
		Add(offset,size);
		//if(numItems==maxItems) archive->Defragment();
	}

	WriteEmptySpace(archive->EmptySpaceTableOffset);

	return 1;
}

int CFSEmptySpace::GetEmptySpace(int size,int &offset)
{
	int i,mx,mxsz=0;

	for(i=0;i<numItems;i++)
	{
		if(items[i].size==size) 
		{
			offset=items[i].offset;
			Delete(i);
			WriteEmptySpace(archive->EmptySpaceTableOffset);
			return 1;
		}else{
			if(items[i].size>mxsz) {mxsz=items[i].size;mx=i;}
		}
	}

	if(numItems)
	{
		i=mx; //vezmu nejvetsi;
		if(items[i].size>size){
			offset=items[i].offset;
			Change(i,offset+size,items[i].size-size);
			WriteEmptySpace(archive->EmptySpaceTableOffset);
			return 1;
		}
	}

	//neexistuje zadna dostatecne velka dira, pridelim misto z konce souboru
	offset = archive->archivesize;
	
	archive->archivesize+=size;
	_chsize(_fileno(archive->file),archive->archivesize);

	return 1;
}

void CFSEmptySpace::Add(int offset, int size)
{
	int i,j;

	assert(numItems<maxItems);

	i=0;
	while(items[i].offset<offset&&i<numItems) i++;

	for(j=numItems+1;j>i;j--)
		items[j]=items[j-1];

	items[j].offset=offset;
	items[j].size=size;

	numItems++;
}

void CFSEmptySpace::Delete(int i)
{
	int j;

	numItems--;
	for(j=i;j<numItems;j++)
		items[j]=items[j+1];

}
void CFSEmptySpace::Change(int i, int newoffset, int newsize)
{
	items[i].offset = newoffset;
	items[i].size = newsize;

/*	int j;
	if(newsize==items[i].size)
		items[i].offset=newoffset;
	else if(newsize<items[i].size)
	{
		j=i;
		while(j>0&&items[j].size>newsize)
		{
			items[j]=items[j-1];
			j--;
		}
		items[j].offset=newoffset;
		items[j].size=newsize;
	}else //newsize>items[i].size
	{
		j=i;
		while(j<numItems-1&&items[j].size<newsize)
		{
			items[j]=items[j+1];
			j++;
		}
		items[j].offset=newoffset;
		items[j].size=newsize;
	}*/
}

int CFSEmptySpace::WriteEmptySpace(int offset)
{
	FILE *f;
	DWORD CRC;
	assert(archive->file&&archive->readonly==0);

	f=archive->file;
	
	CRC=CmpCRC();

	SEEK(f,offset);
	WRITE(&numItems,4,f);
	WRITE(&CRC,4,f);
	for(int i=0;i<numItems;i++)
	{
		WRITE(&items[i].offset,4,f);
		WRITE(&items[i].size,4,f);
	}
	FLUSH(f);
	return 1;
}

int CFSEmptySpace::ReadEmptySpace(int offset)
{
	FILE *f;
	DWORD CRC,CRC2;
	assert(archive->file);

	f=archive->file;
	
	SEEK(f,offset);
	READ(&numItems,4,f);

	if(numItems>=maxItems) return 0;
	
	READ(&CRC,4,f);
	for(int i=0;i<numItems;i++)
	{
		READ(&items[i].offset,4,f);
		READ(&items[i].size,4,f);
	}

	CRC2=CmpCRC();

	if(CRC!=CRC2) return 0;

	return 1;
}

DWORD CFSEmptySpace::CmpCRC()
{
	CCRC32 crc;

	crc.Cmp(&numItems,4);
	for(int i=0;i<numItems;i++)
	{
		crc.Cmp(&items[i].offset,4);
		crc.Cmp(&items[i].size,4);
	}
	return crc;
}


int CFSEmptySpace::CmpOffsetAfterDefrag(int oldoffset)
{
	int esz=0;
	for(int i=0;i<numItems;i++)
	{
		if(items[i].offset>=oldoffset) break;
		esz+=items[i].size;
	}
	return oldoffset-esz;
}

int CFSEmptySpace::NeedDefrag()
{
	if(numItems==0) return 0;

	if(numItems<maxItems - 10) return 0;

	return 1;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
CFSArchive::CFSArchive()
{
	file=NULL;
	emptyspace=NULL;
	curdir=NULL;
	DirMgr=new CFSDirMgr(this);
}
CFSArchive::~CFSArchive()
{
	Close();
	SAFE_DELETE(emptyspace);
	SAFE_DELETE(DirMgr);
}

void CFSArchive::Close(){
	SAFE_DELETE(emptyspace);
	curdir=NULL;
	if(DirMgr) DirMgr->DeleteAll();
	SAFE_DELETE(emptyspace);
	if(file){fclose(file);file=NULL;}
}

int CFSArchive::Create(const char *path)
{
	try{

		Close();

		file=FOPEN(path,"w+b");
		if(!file) return 0;
		readonly=0;

		EmptySpaceTableOffset=defaultEmptySpaceTableOffset;
		MaxEmptySpaceItems=100;
		RootDirOffset=EmptySpaceTableOffset+EmptySpaceHeadLen+MaxEmptySpaceItems*EmptySpaceItemLen;

		int l=(int)strlen(path)+1;

		WriteArchiveHead();
		archivesize=RootDirOffset;
		_chsize(_fileno(file),archivesize);

		emptyspace=new CFSEmptySpace(MaxEmptySpaceItems,this);

		emptyspace->WriteEmptySpace(EmptySpaceTableOffset);


		curdir=DirMgr->CreateDir(0);
			
		assert(curdir->GetOffset()==RootDirOffset);
	}
	catch(CExc e)
	{
		if(e.errnum==eFS) return 0;
		throw;
	}

	return 1;
}

int CFSArchive::Open(const char *path, int _readonly)
{

	try
	{

	Close();

	if(_readonly)
		file=FOPEN(path,"rb");
	else
		file=FOPEN(path,"r+b");
	if(!file) return 0;
	
	readonly=_readonly;

	archivesize=_filelength(_fileno(file));

	if(!ReadArchiveHead()) 
	{
		Close();
		return 0;
	}

	emptyspace=new CFSEmptySpace(MaxEmptySpaceItems,this);

	if(!emptyspace->ReadEmptySpace(EmptySpaceTableOffset))
	{
		Close();
		return 0;
	}

	curdir=DirMgr->ReadDir(RootDirOffset);
	if(!curdir){Close();return 0;}

	}
	catch(CExc e)
	{
		if(e.errnum==eFS) return 0;
		throw;
	}
	return 1;
}

void CFSArchive::WriteArchiveHead()
{
	assert(file&&readonly==0);

	SEEK(file,0);

	WRITE(sFS_KRKAL,sizeof(sFS_KRKAL),file);
	WRITE(sFS_VER,sizeof(sFS_VER),file);

	WRITE(&RootDirOffset,sizeof(int),file); 

	WRITE(&EmptySpaceTableOffset,sizeof(int),file); 
	WRITE(&MaxEmptySpaceItems,sizeof(int),file);

	DWORD CRC=CmpArchiveHeadCRC();
	
	WRITE(&CRC,sizeof(int),file); //CRC
	FLUSH(file);
}

int CFSArchive::ReadArchiveHead()
{
	assert(file);

	SEEK(file,0);

	char buf[sizeof(sFS_KRKAL)];
	READ(buf,sizeof(sFS_KRKAL),file);
	if(strcmp(buf,sFS_KRKAL)!=0) return 0;
	READ(buf,sizeof(sFS_VER),file);
	if(strcmp(buf,sFS_VER)!=0) return 0;

	READ(&RootDirOffset,sizeof(int),file); 

	READ(&EmptySpaceTableOffset,sizeof(int),file); 
	READ(&MaxEmptySpaceItems,sizeof(int),file);

	DWORD CRC;
	
	READ(&CRC,sizeof(int),file);

	if(CRC!=CmpArchiveHeadCRC()) return 0;

	return 1;
}

DWORD CFSArchive::CmpArchiveHeadCRC()
{
	CCRC32 crc;
	crc.Cmp(&RootDirOffset,4);
	crc.Cmp(&EmptySpaceTableOffset,4);
	crc.Cmp(&MaxEmptySpaceItems,4);
	return crc;
}


int CFSArchive::ChangeDir(const char *path)
{
	if(!curdir||!file) return 0;
	
	CFSDir *d=NULL;

	int fnd;

	if(path==0||path[0]==0) 
		fnd=FindDir("/",&d);
	else
		fnd=FindDir(path,&d);

	if(fnd)
	{
		DirMgr->DeleteDir(curdir);
		curdir=d;
		return 1;
	}else
	{
		DirMgr->DeleteDir(d);
		return 0;
	}

}

int CFSArchive::ChangeDir(CFSDirRef *dirref)
{
	if(!curdir||!file) return 0;

	if(!dirref||!dirref->dir||dirref->GetArchive()!=this) return 0;

	CFSDir *d=DirMgr->ReadDir(dirref->dir->GetOffset());

	if(!d) return 0;

	DirMgr->DeleteDir(curdir);

	curdir=d;

	return 1;

}

CFSDirRef* CFSArchive::GetDir()
{
	if(!curdir||!file) return 0;
	CFSDirRef *d=new CFSDirRef(curdir);
	return d;
}



int CFSArchive::FindDir(const char *path, CFSDir **dir, int len)
{

	int cs=0,i,o;
	int ln;
	
	if(len==-1) ln=(int)strlen(path); else ln=len;
	
	if(ln==0) return 0;
	
	if(*dir) {DirMgr->DeleteDir(*dir);*dir=NULL;}

	if(path[0]=='/' || path[0]=='\\')
	{
		*dir=DirMgr->ReadDir(RootDirOffset);
		if(!*dir) return 0;

		cs=1;
		if(ln==1) return 1;
	}else{

		*dir=DirMgr->ReadDir(curdir->GetOffset());
		if(!*dir) return 0;

	}

	for(i=cs;i<=ln;i++)
	{
		if(path[i]=='/' || path[i]=='\\' || path[i]==0)
		{
			CFSDirItem **it;			

			if(!(*dir)->FindItem(path+cs,&it,i-cs))
				return 0;
			else
			{
				o=(*it)->GetOffset();
				DirMgr->DeleteDir(*dir);
				*dir=DirMgr->ReadDir(o);

				if(!*dir) return 0;
			}
            				
			cs=i+1;
		}
	}

	return 1;
}

int CFSArchive::SplitPath(const char *path, CFSDir **dir, int &fileindex)
{
	int ln=(int)strlen(path);
	int i;
	
	if(ln==0) return 0;
	
	if(*dir) {DirMgr->DeleteDir(*dir);*dir=NULL;}

	for(i=ln-1;i>=0;i--)
		if(path[i]=='/'||path[i]=='\\') break;

	fileindex=i+1;
	if(fileindex>=ln) return 0;

	if(i==-1) {
		*dir=DirMgr->ReadDir(curdir->GetOffset());
		return 1;
	}
	
	if(i==0) i++;

	if(FindDir(path,dir,i)) return 1;

	if(*dir) {DirMgr->DeleteDir(*dir);*dir=NULL;}
	
	return 0;
}

int CFSArchive::CreateDir(const char *name)
{

	if(!file||readonly) return 0;

	if(!curdir) return 0;

	CFSDir *dir=NULL;
	int fi,rr;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->CreateSubDir(name+fi);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::Delete(const char *path)
{
	if(!file||readonly) return 0;
	if(!curdir) return 0;

	CFSDir *dir=NULL;
	int fi,rr;
	if(SplitPath(path,&dir,fi))
	{
		rr=dir->Delete(path+fi);
	}

	if(dir) DirMgr->DeleteDir(dir);
	return rr;
}

int CFSArchive::WriteFile(const char *name,const char *buf, int size, unsigned char compr)
{

	if(!file||readonly) return 0;

	if(!curdir) return 0;


	if(compr!=-1&&compr!=0&&compr!=1) compr=-1;

	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->WriteFile(name+fi,buf,size,compr);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::ReadFile(const char *name, char *buf, int bufsize)
{
	if(!file) return 0;

	if(!curdir) return 0;

	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->ReadFile(name+fi,buf,bufsize);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::GetFileSize(const char *name)
{
	if(!file) return 0;

	if(!curdir) return 0;
	
	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->GetFileSize(name+fi);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::IsCompressed(const char *name)
{
	if(!file) return 0;

	if(!curdir) return 0;
	
	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->IsCompressed(name+fi);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::FileExist(const char *name)
{
	if(!file) return 0;

	if(!curdir) return 0;
	
	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->FileExist(name+fi);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}

int CFSArchive::GetTime(const char *name, FILETIME &time)
{
	if(!file) return 0;

	if(!curdir) return 0;
	
	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->GetTime(name+fi,time);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}


int CFSArchive::SetTime(const char *name, const FILETIME &time)
{
	if(!file) return 0;

	if(!curdir) return 0;
	
	int fi;
	int rr=0;
	CFSDir *dir=NULL;

	if(SplitPath(name,&dir,fi))
	{
		rr=dir->SetTime(name+fi,time);
	}
	
	if(dir) DirMgr->DeleteDir(dir);

	return rr;
}



int CFSArchive::Rename(const char *oldname, const char *newname)
{
	if(!file||readonly) return 0;
	if(!curdir) return 0;

	CFSDir *dir1=NULL,*dir2=NULL;
	int fi1,fi2;
	int rr=0;

	if(SplitPath(oldname,&dir1,fi1)&&SplitPath(newname,&dir2,fi2))
	{
		rr=dir1->Rename(oldname+fi1,newname+fi2,dir2);
	}

	if(dir1) DirMgr->DeleteDir(dir1);
	if(dir2) DirMgr->DeleteDir(dir2);

	return rr;
}

int CFSArchive::Defragment()
{
	if(!file||readonly) return 0;

	if(emptyspace->numItems==0) return 1; //neni treba defragmentovat

	int curofs=EmptySpaceTableOffset+EmptySpaceHeadLen+MaxEmptySpaceItems*EmptySpaceItemLen;
	int wrofs=curofs;
	int esindex=0;
	int esbefore=0;
	int sz,s;
	char d;
	DWORD CRC;
	char *buf;
	const int bufsz=64*1024;
	CFSDir *dir;
	int w,r;
	unsigned char compr;

	buf=new char[bufsz];
	if(!buf) return 0;

	RootDirOffset=emptyspace->CmpOffsetAfterDefrag(RootDirOffset);
	WriteArchiveHead();

	SEEK(file,curofs);

	while(curofs<archivesize)
	{
		if(esindex<emptyspace->numItems&&emptyspace->items[esindex].offset==curofs)
		{
			esbefore+=emptyspace->items[esindex].size;
			curofs+=emptyspace->items[esindex].size;
			esindex++;
		}else
		{
			SEEK(file,curofs);
			READ(&d,1,file);
			if(d)
			{
				dir=DirMgr->ReadDir(curofs);
				if(!dir)
				{
					delete[] buf;
					return 0;
				}

				sz=dir->GetSize();

				dir->DefragDir(wrofs);
				
				curofs+=sz;
				wrofs+=sz;
			}else
			{
				READ(&sz,4,file);
				READ(&compr,1,file);
				READ(&CRC,4,file);
				SEEK(file,wrofs);
				WRITE(&d,1,file);
				WRITE(&sz,4,file);
				WRITE(&compr,1,file);
				WRITE(&CRC,4,file);
				s=sz;
				w=wrofs+1+4+1+4;
				r=curofs+1+4+1+4;
				while(s>bufsz)
				{
					SEEK(file,r);
					READ(buf,bufsz,file);
					SEEK(file,w);
					WRITE(buf,bufsz,file);
					r+=bufsz;w+=bufsz;
					s-=bufsz;
				}
				if(s)
				{
					SEEK(file,r);
					READ(buf,s,file);
					SEEK(file,w);
					WRITE(buf,s,file);
				}
				sz=CMP_FILE_SPACE(sz);//zvetsi na nasobek 32
				curofs+=sz;
				wrofs+=sz;
			}
		}
	}

	emptyspace->DeleteAll();
	emptyspace->WriteEmptySpace(EmptySpaceTableOffset);

	archivesize=wrofs;
	_chsize(_fileno(file),archivesize);

	FLUSH(file);

	delete[] buf;

	return 1;
}



int CFSArchive::GetCurDirNumItems()
{
	if(!file||!curdir) return 0;
	return curdir->GetNumItems();
}

int CFSArchive::GetCurDirItem(int itnum, const char **name, int &dir)
{
	if(!file||!curdir) return 0;
	return curdir->GetItem(itnum,name,dir);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////

CFSDir::CFSDir(CFSArchive *ar)
{
	assert(ar&&ar->file);

	archive=ar;

	numitems=0;
	items=NULL;
	offset=0;
	wrsize=0;

	RefCnt=0;
	valid=0;
}

CFSDir::~CFSDir()
{
	SAFE_DELETE(items);
}

int CFSDir::Create(int _parentoffset)
{
	assert(archive->readonly==0);

	valid=0;

	SAFE_DELETE(items);

	parentoffset=_parentoffset;

	numitems=1;
	items = new CFSDirItem;
	items->Create("..",1,parentoffset);
	
	offset=0;
	wrsize=0;

	valid=1;

	Write();

	return offset;
	
}

int CFSDir::Write()
{
	assert(archive->readonly==0);
	assert(valid);

	int sz=GetSize();
	int o,res;
	int testdefrag=0;

	if(sz!=wrsize)
	{
		if(wrsize>0) //vratim stary misto
		{
			archive->emptyspace->AddEmptySpace(wrsize,offset);
			testdefrag = 1;
		}

		//alokuju novy misto
		if(!archive->emptyspace->GetEmptySpace(sz,o))
			return 0;

		if(o!=offset&&offset!=0)
		{
			//offset se zmenil musim zmenit rodice a deti
			CFSDir *dir;
				
			CFSDirItem *it=items; 
			CFSDirItem **it2;
			int oo;
			while(it)
			{
				if(it->dir)
				{
					oo=it->GetOffset();

					if(oo)
					{
						dir=archive->GetDirMgr()->ReadDir(oo);
						if(dir)
						{
							if(dir->parentoffset==offset) dir->parentoffset=o;
							if(dir->FindItem(offset,&it2))
							{
								(*it2)->offset=o;
								dir->Write();
							}
							
							archive->GetDirMgr()->DeleteDir(dir);
						}
					}else{
						//rootdir
						if(archive->RootDirOffset!=o)
						{
							archive->RootDirOffset=o;
							archive->WriteArchiveHead();
						}
					}
				}
				it=it->next;
			}

		}
		offset=o;
	}

	wrsize=sz;

	res = WriteDirData();
	if(!res) return 0;

	if(testdefrag)
	{
		if( archive->emptyspace->NeedDefrag() )
			archive->Defragment();
	}

	return res;
}

int CFSDir::WriteDirData()
{
	assert(valid);

	//vlastni zapis
	FILE *f=archive->file;
	char d=1;
	CCRC32 crc; 
	DWORD CRC;
	CFSDirItem *it=items;

	SEEK(f,offset);
	WRITE(&d,1,f);
	WRITE(&numitems,4,f);
	
	crc.Cmp(&d,1);
	crc.Cmp(&numitems,4);

	while(it){
		it->CmpCRC(crc);
		it=it->next;
	} 
	
	CRC=crc;
	WRITE(&CRC,4,f);


	it=items;
	while(it){
		it->Write(f);
		it=it->next;
	}

	FLUSH(f);

	return 1;
}

int CFSDir::Read(int _offset)
{
	valid=0;
	SAFE_DELETE(items);
	offset=0;
	wrsize=0;
	numitems=0;

	FILE *f=archive->file;

	SEEK(f,_offset);

	char d;
	DWORD CRCr;
	CCRC32 crc;
	CFSDirItem **it=&items;
	CFSDirItem *dit;


	READ(&d,1,f);
	if(d!=1) return 0;

	READ(&numitems,4,f);
	READ(&CRCr,4,f);
	for(int i=0;i<numitems;i++)
	{
		*it=new CFSDirItem;
		(*it)->Read(f);
		if(i==0&&strcmp((*it)->name,"..")==0) parentoffset=(*it)->offset;
		it=&(*it)->next;
	}
	
	crc.Cmp(&d,1);
	crc.Cmp(&numitems,4);
	dit=items;
	while(dit){
		dit->CmpCRC(crc);
		dit=dit->next;
	} 
	
	if(CRCr!=crc)
	{
		SAFE_DELETE(items);
		numitems=0;
		wrsize=0;
		return 0;
	}

	valid=1;

	wrsize=GetSize();

	offset=_offset;

	return 1;
}

int CFSDir::FindItem(const char *name,CFSDirItem ***it, int namelen)
{
	assert(valid);

	if(namelen==-1)
		namelen=(int)strlen(name);

	*it=&items;

	while(**it)
	{
		if((**it)->namelen==namelen&&
			//memcmp((**it)->name,name,namelen)==0
			_strnicmp((**it)->name,name,namelen)==0
			) return 1;
		*it=&(**it)->next;
	}

	return 0;
}

int CFSDir::FindItem(int offset,CFSDirItem ***it)
{
	assert(valid);

	*it=&items;

	while(**it)
	{
		if((**it)->offset==offset) return 1;
		*it=&(**it)->next;
	}

	return 0;
}

int CFSDir::GetSize()
{
	int sz;
	CFSDirItem *it=items;

	sz=1+4+4;

	while(it){
        sz+=it->GetSize();
		it=it->next;
	}

	//zaokrouhlim na nasobek 32
	sz=CMP_CEIL(sz);

	return sz;
}

int CFSDir::CreateSubDir(const char *name)
{
	assert(valid);
	if(archive->readonly) return 0;

	CFSDirItem **itm;
	if(FindItem(name,&itm)) return 0; //uz existuje file/dir se stejnym jmenem

	CFSDir *dir;
	dir=archive->GetDirMgr()->CreateDir(offset);
	if(!dir) return 0;
	
	int o;
	o=dir->GetOffset();

	CFSDirItem *it=new CFSDirItem;
	it->Create(name,1,o);

	assert(items);

	numitems++;

	it->next=items->next;
	items->next=it;
	
	Write();

	archive->GetDirMgr()->DeleteDir(dir);

	return 1;
}


int CFSDir::WriteFile(const char *name,const char *buf, int size, unsigned char compr)
{
	assert(valid);
	int ex=0;
	if(archive->readonly) return 0;
	int oldoffset=0;

	CFSDirItem **itm;
	if(FindItem(name,&itm)) {
		ex=1; //uz existuje file/dir se stejnym jmenem
		if((*itm)->dir) return 0; //existuje dir

		oldoffset = (*itm)->offset;

		if(compr==-1)
		{
			compr=IsCompressed(name);
		}

	}

	if(compr==-1) compr=1;

	int o;

	o=WriteFileData(buf,size,oldoffset,compr);
	if(!o) return 0;

	if(ex)
	{
		SYSTEMTIME st;
		FILETIME ft;

		GetSystemTime(&st);              // gets current time
	    SystemTimeToFileTime(&st, &ft);  // converts to file time format

		(*itm)->SetTime(ft);
		(*itm)->offset=o;
	}else
	{
		CFSDirItem *it=new CFSDirItem;
		it->Create(name,0,o);

		assert(items);

		numitems++;

		it->next=items->next;
		items->next=it;
	}

	Write();

	if( archive->emptyspace->NeedDefrag() )
		archive->Defragment();

	return 1;
}

int CFSDir::WriteFileData(const char *buf, int size, int oldoffset, unsigned char compr)
{
	//v buf jsou nekoprimovana data, size je velikost nekoprimovanych dat

	assert(valid);

	int sz,osz;
	int datasize;
	int compressedsize;
	int o;
	uLongf compbufsize;

	const char *outbuf;

	char d=0;

	assert(archive->readonly==0);


	if(compr)
	{
		compbufsize=(int)ceil(size*1.01)+12;
		outbuf=new char[compbufsize];

		compress((Bytef*)outbuf,&compbufsize,(const Bytef*)buf,size);

		compressedsize=compbufsize;
		datasize=compressedsize+COMPRESSED_FILE_HEAD_SIZE;

	}else{
		outbuf=buf;
		datasize=size;		
		compressedsize=size;
	}
	

	sz=CMP_FILE_SPACE(datasize);

	FILE *f=archive->file;


	if(oldoffset)
	{

		SEEK(f,oldoffset);

		READ(&d,1,f);
		if(d!=0){
			if(compr) delete[] outbuf;
			return 0;
		}
		READ(&osz,4,f);

		osz=CMP_FILE_SPACE(osz);

		if(osz!=sz)
		{
			archive->emptyspace->AddEmptySpace(osz,oldoffset); //vratim stary misto
			//alokuju novy misto
			if(!archive->emptyspace->GetEmptySpace(sz,o))
			{
				if(compr) delete[] outbuf;
				return 0;
			}

		}else o=oldoffset;


	}else{
		//alokuju novy misto
		if(!archive->emptyspace->GetEmptySpace(sz,o))
		{
			if(compr) delete[] outbuf;
			return 0;
		}
	}

	SEEK(f,o);
	
	CCRC32 crc;
	DWORD CRC;

	crc.Cmp(&d,1);
	crc.Cmp(&datasize,4);
	crc.Cmp(&compr,1);
	if(compr){
		crc.Cmp(&size,4); //velikost nekomprimovanych dat
	}
	crc.Cmp(outbuf,compressedsize);
	CRC=crc;

	WRITE(&d,1,f);
	WRITE(&datasize,4,f);
	WRITE(&compr,1,f);
	WRITE(&CRC,4,f);
	if(compr){
		WRITE(&size,4,f); //velikost nekomprimovanych dat
	}
	WRITE(outbuf,compressedsize,f);

	FLUSH(f);

	if(compr) delete[] outbuf;

	return o;
}

int CFSDir::ReadFile(const char *name,char *buf, int bufsize)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return 0; //neexistuje
    
	if((*itm)->dir) return 0; //je to adresar
	
	return ReadFileData((*itm)->offset,buf,bufsize);
}

int CFSDir::ReadFileData(int offset, char *buf, int bufsize)
{
	assert(valid);
	int ok=1;

	char d=1;
	unsigned char compr;
	int size;
	int uncompsize,datasize;

	char *inbuf;

	FILE *f=archive->file;

	SEEK(f,offset);
	
	CCRC32 crc;
	DWORD CRC;

	READ(&d,1,f);
	if(d!=0) return 0; 
	READ(&size,4,f);
	READ(&compr,1,f);
	READ(&CRC,4,f);
	if(compr){
		READ(&uncompsize,4,f);
		datasize=size-COMPRESSED_FILE_HEAD_SIZE;
		
		inbuf=new char[datasize];

	}else{
		if(size<=bufsize) datasize=size; else datasize=bufsize;		
		inbuf=buf;
	}

	crc.Cmp(&d,1);
	crc.Cmp(&size,4);
	crc.Cmp(&compr,1);
	if(compr)
		crc.Cmp(&uncompsize,4);

	READ(inbuf,datasize,f);

	crc.Cmp(inbuf,datasize);

	if(CRC==crc) 
	{

		if(compr)
		{
			//dekomprese

			uLongf ucs;
			
			if(uncompsize<=bufsize) ucs=uncompsize; else ucs=bufsize;

			if(uncompress((Bytef*)buf,&ucs,(Bytef*)inbuf,datasize)!=Z_OK)
				ok=0;

			if(ucs!=uncompsize) ok=2;
			
		}
	}else ok=0;

	if(compr) delete[]inbuf;
	
	return ok;

}

int CFSDir::GetFileSize(const char *name)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return 0; //neexistuje
    
	if((*itm)->dir) return 0; //je to adresar

	FILE *f=archive->file;

	SEEK(f,(*itm)->offset);
	
	char d;
	unsigned char compr;
	int size;
	int CRC;

	READ(&d,1,f);
	if(d!=0) return 0; 
	READ(&size,4,f);
	READ(&compr,1,f);
	if(!compr) return size;

	READ(&CRC,4,f);
	READ(&size,4,f);

	return size;	
}

int CFSDir::IsCompressed(const char *name)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return -1; //neexistuje
    
	if((*itm)->dir) return -1; //je to adresar

	FILE *f=archive->file;

	SEEK(f,(*itm)->offset);
	
	char d;
	unsigned char compr;
	int size;

	READ(&d,1,f);
	if(d!=0) return -1; 
	READ(&size,4,f);
	READ(&compr,1,f);

	return compr;	
}

int CFSDir::FileExist(const char *name)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return 0; //neexistuje
    
	if((*itm)->dir) return 2; //je to adresar

	return 1; //je to soubor
}

int CFSDir::GetTime(const char *name, FILETIME &item_time)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return 0; //neexistuje

	(*itm)->GetTime(item_time);
	
	return 1;
}

int CFSDir::SetTime(const char *name, const FILETIME &item_time)
{
	assert(valid);

	CFSDirItem **itm;
	if(!FindItem(name,&itm)) return 0; //neexistuje

	(*itm)->SetTime(item_time);

	Write();
	
	return 1;
}

int CFSDir::Delete(const char *name)
{
	assert(valid);

	if(archive->readonly) return 0;

	if(strcmp(name,"..")==0) return 0;

	CFSDirItem **itm,*it;
	int delall=0;

	if(strcmp(name,"*")==0)
	{
		//mazu vse
		itm=&items->next;
		it=*itm;
		delall=1;
	}else{
		if(!FindItem(name,&itm)) return 0; 
		it=*itm;
	}

	while(it)
	{

		if(it->dir)
		{
			//mazu adresar
			CFSDir *d;
			d=archive->GetDirMgr()->ReadDir(it->GetOffset());

			d->Delete("*");

			int sz=d->GetSize();
			int o=d->GetOffset();

			d->Invalidate();

			archive->emptyspace->AddEmptySpace(sz,o); //vratim stary misto

			archive->GetDirMgr()->DeleteDir(d);

		}else
		{
			//mazu soubor
			FILE *f=archive->file;

			SEEK(f,it->offset);
			
			char d;
			int size;
			READ(&d,1,f);
			if(d!=0) return 0; 
			READ(&size,4,f);

			size=CMP_FILE_SPACE(size);

			archive->emptyspace->AddEmptySpace(size,it->offset); //vratim stary misto
		}	

		*itm=it->next;
		it->next=NULL;
		delete it;
		numitems--;

		if(!delall) break;

		it=*itm;

		if( archive->emptyspace->NeedDefrag() )
			archive->Defragment();

	}

	Write();

	if( archive->emptyspace->NeedDefrag() )
		archive->Defragment();

	return 1;
}

int CFSDir::Rename(const char *oldname, const char *newname, CFSDir *newdir)
{
	assert(valid);

	if(archive->readonly) return 0;

	if(strcmp(oldname,"..")==0) return 0;
	if(strcmp(newname,"..")==0) return 0;

	CFSDirItem **itm,*it;

	if(newdir->FindItem(newname,&itm)) return 0; //uz existuje novy

	if(!FindItem(oldname,&itm)) return 0;  //neexistuje stary


	it=*itm; 

	if(it->dir) //presouvam adresar -> zkontroluju jestli ho nepresouvam do sebe
	{
		CFSDir *d=NULL;
		int po=newdir->parentoffset;

		if(newdir->offset==it->offset) return 0;
		if(po==it->offset) return 0;

		while(po!=0)
		{
			d=archive->GetDirMgr()->ReadDir(po);
			
			if(d->offset==it->offset) {
				archive->GetDirMgr()->DeleteDir(d);
				return 0;
			}

			po=d->parentoffset;
			archive->GetDirMgr()->DeleteDir(d);
		}
	}

	SAFE_DELETE_ARRAY(it->name);
    it->namelen=(int)strlen(newname);
	it->name=new char[it->namelen+1];
	strcpy(it->name,newname);

	if(newdir&&newdir->offset!=offset)
	{
		//vyhodim ze staryho adresare
		(*itm)=it->next; 
		numitems--;
		//dam do novyho
		it->next=newdir->items->next;
		newdir->items->next=it;
		newdir->numitems++;
		
		if(it->dir) //presouval jsem adresar -> musim zmenit rodice
		{
			CFSDir *dir;
			CFSDirItem **it2;
			dir=archive->GetDirMgr()->ReadDir(it->offset);
			if(dir)
			{
				dir->parentoffset=newdir->offset;
				if(dir->FindItem(offset,&it2))
				{
					(*it2)->offset=newdir->offset;
					dir->Write();
				}

				archive->GetDirMgr()->DeleteDir(dir);
			}

		}

		//zapisu novej adr.
		newdir->Write();
	}
	Write(); //zapisu starej adr.

	return 1;
}


int CFSDir::DefragDir(int newoffset)
{
	assert(valid);

	if(!archive||archive->readonly) return 0;

	CFSEmptySpace *es=archive->emptyspace;
	if(!es) return 0;

	offset=newoffset;
	parentoffset=es->CmpOffsetAfterDefrag(parentoffset);

	CFSDirItem *it=items;
	while(it)
	{
		it->offset=es->CmpOffsetAfterDefrag(it->offset);
		it=it->next;
	}

	WriteDirData();

	return 1;
}

int CFSDir::GetItem(int itnum, const char **name, int &dir)
{
	assert(valid);

	if(itnum<0||itnum>=numitems) return 0;
	
	CFSDirItem *it=items;
	
	for(int i=0;i<itnum;i++) it=it->next;

	dir=it->dir;
	*name=it->name;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int CFSDirItem::Write(FILE *f)
{
	WRITE(&namelen,1,f);
	WRITE(name,namelen,f);
	WRITE(&dir,1,f);
	WRITE(&offset,4,f);
	WRITE(&ittime,8,f);
	return 1;
}

int CFSDirItem::Read(FILE *f)
{
	SAFE_DELETE_ARRAY(name);

	READ(&namelen,1,f);
	
	name=new char[namelen+1];
	READ(name,namelen,f);
	name[namelen]=0;

	READ(&dir,1,f);
	READ(&offset,4,f);
	READ(&ittime,8,f);

	return 1+namelen+1+4+8;

}


void CFSDirItem::CmpCRC(CCRC32 &crc)
{
	crc.Cmp(&namelen,1);
	crc.Cmp(name,namelen);
	crc.Cmp(&dir,1);
	crc.Cmp(&offset,4);
	crc.Cmp(&ittime,8);
}

int CFSDirItem::GetSize()
{
	return 1+namelen+1+4+8;
}


int CFSDirItem::Create(const char *_name, char _dir, int _offset)
{
	SAFE_DELETE_ARRAY(name);

	namelen=(int)strlen(_name);
	if(namelen>256) return 0;

	name=new char[namelen+1];
	memcpy(name,_name,namelen+1);

	dir=_dir;
	offset=_offset;

    SYSTEMTIME st;

    GetSystemTime(&st);              // gets current time
    SystemTimeToFileTime(&st, &ittime);  // converts to file time format

	return 1;

}

////////////////////////////////////////////////////////////////////////////////////////////////////

CFSDirMgr::CFSDirMgr(CFSArchive *ar)
{
	archive=ar;
}

CFSDirMgr::~CFSDirMgr()
{
	DeleteAll();
}

CFSDir* CFSDirMgr::ReadDir(int offset)
{
	if(offset==0) return NULL;

	CListKElem<CFSDir*> *el;
	
	el=list.el;
	while(el)
	{
		if(el->data->offset==offset)
		{
			el->data->RefCnt++;
			return el->data;
		}
		el=el->next;
	}

	CFSDir *d=new CFSDir(archive);
	Add(d);
	d->RefCnt++;
	if(!d->Read(offset))
	{
		DeleteDir(d);
		return NULL;
	}
	return d;
}

CFSDir* CFSDirMgr::CreateDir(int parentoffset)
{
	CFSDir *d=new CFSDir(archive);
	Add(d);
	d->RefCnt++;
	if(!d->Create(parentoffset))
	{
		DeleteDir(d);
		return NULL;
	}
	return d;
}

void CFSDirMgr::DeleteDir(CFSDir *dir)
{	
	if(!dir) return;
	if(dir->RefCnt==1)
	{
		list.Delete(dir);
		SAFE_DELETE(dir);
	}else
		dir->RefCnt--;
}

void CFSDirMgr::DeleteAll()
{
	CListKElem<CFSDir*> *el=list.el;
	while(el)
	{
		SAFE_DELETE(el->data);
		el=el->next;
	}
	list.DeleteAll();
}

void CFSDirMgr::Add(CFSDir *dir)
{
	list.Add(dir);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CFSDirRef::CFSDirRef(CFSDir *d)
{
	dir=NULL;
	if(!d||!d->archive||!d->valid) return;
	dir = d->archive->GetDirMgr()->ReadDir(d->offset);
}

CFSDirRef::~CFSDirRef()
{
	if(dir)
	{
		dir->archive->GetDirMgr()->DeleteDir(dir);
		dir=NULL;
	}
}

CFSArchive* CFSDirRef::GetArchive()
{
	if(dir)
		return dir->archive;
	return NULL;
}

void CFSDirRef::Close()
{
	delete this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int CFS::FSex=0;

int CFS::InitFS()
{
	if(FSex) return 0;
	FS=new CFS();
	return 1;
}
void CFS::DoneFS()
{
	if(!FSex) return;
	SAFE_DELETE(FS);
}

CFS::CFS()
{

	assert(FSex==0);
	FSex=1;


	cfg=new CFSCfg("KRKALfs.cfg");
	
	curarchive=NULL;

	curdir=new CFSFolder;
	
	SetCurDir();

	findinfo.finddir=NULL;
	inFind=0;
	findinfo.findHandle=INVALID_HANDLE_VALUE;

}

CFS::~CFS()
{
	FindClose();

	SAFE_DELETE(curarchive);
	SAFE_DELETE(curdir);
	SAFE_DELETE(cfg);

	FSex=0;	
}

int CFS::AddFSDir(const char *key, const char *val, int newkey)
{
	if(!cfg) return 0;
	return cfg->AddKey(key,val,newkey);
}

int CFS::IsValidFilename(const char *filename)
{
	if(!filename) return 0;

	const UC *c=(UC*)filename;
	if(*c==0) return 0;
	while(*c)
	{
		if(!isprint(*c)) return 0;
		switch(*c){
			case '\\':
			case '?':
			case '|':
			case '>':
			case '<':
			case ':':
			case '/':
			case '*':
			case '"':
				return 0;
		}
		c++;
	}
	return 1;


}

int CFS::ReadFile(const char *name, char *buf, int bufsize)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	int ln,ln2;
	const char *fname;

	if(!buf) return 0;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname);
	if(ok)
	{
		//otevru soubor
		if(curarchive)
		{
			ok=curarchive->ReadFile(fname,buf,bufsize);
		}else{
			FILE *f;
			f=FOPEN(fname,"rb");
			if(!f) ok=0;
			else{
				ln=_filelength(_fileno(f));

				if(ln>bufsize) ln2=bufsize; else ln2=ln;

				if(ln>=FS_COMPFILE_HEADLEN)
				{

					int hok=0;
					char hd[FS_COMPFILE_HEADLEN];
					char *head;
					if(bufsize<FS_COMPFILE_HEADLEN) head=hd; else head=buf;

					READ(head,FS_COMPFILE_HEADLEN,f);

					CCRC32 CRC;
					
					DWORD *crc=(DWORD*)(head+FS_COMPFILE_HEADLEN-4);
					int size = * (int*)(head+sizeof(sFS_COMPFILE)+sizeof(sFS_COMPFILE_VER));
					int compsize = * (int*)(head+sizeof(sFS_COMPFILE)+sizeof(sFS_COMPFILE_VER)+4);

					if(memcmp(head,sFS_COMPFILE,sizeof(sFS_COMPFILE))==0&&memcmp(head+sizeof(sFS_COMPFILE),sFS_COMPFILE_VER,sizeof(sFS_COMPFILE_VER))==0)
						hok=1;

					CRC.Cmp(head,FS_COMPFILE_HEADLEN-4);
					
					if(*crc==CRC)
					{
						//compressed file
						char *cmpbuf=new char[compsize];
						uLongf ucs=bufsize;

						READ(cmpbuf,compsize,f);

						if(uncompress((Bytef*)buf,&ucs,(Bytef*)cmpbuf,compsize)!=Z_OK)
							ok=0;

						if(size!=ucs) ok=2;

						delete[] cmpbuf;

					}else{
						//uncompressed file
						if(bufsize<FS_COMPFILE_HEADLEN){
							memcpy(buf,head,bufsize);
						}else
						READ(buf+FS_COMPFILE_HEADLEN,ln2-FS_COMPFILE_HEADLEN,f);
					}
				}else{
					if(ln2>0) READ(buf,ln2,f);
				}

				if(ln>bufsize&&ok==1) ok=2;
			
				fclose(f);
			}


		}
	
		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::WriteFile(const char *name, char *buf, int size, int compr)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	if(!buf) return 0;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	if(compr!=-1&&compr!=0&&compr!=1) compr=-1;

	if(compr==-1)
	{
		compr=IsCompressed(parsname);
		if(compr==-1) compr=1;
	}
	
	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname);
	if(ok)
	{
		//zapisu soubor
		if(curarchive)
		{
			if(!curarchive->WriteFile(fname,buf,size,compr)) ok=0;
		}else{

			FILE *f;
			f=FOPEN(fname,"wb");
			if(!f) ok=0;
			else{

				if(compr!=1)
				{
					WRITE(buf,size,f);
				}else{

					int compsize;
					DWORD crc;CCRC32 CRC;
					char *compdata;
					uLongf compbufsize;

					compbufsize=(int)ceil(size*1.01)+12;

					compdata = new char [compbufsize];
					compress((Bytef*)compdata,&compbufsize,(const Bytef*)buf,size);

                    compsize=compbufsize;

					WRITE(sFS_COMPFILE,sizeof(sFS_COMPFILE),f);
					WRITE(sFS_COMPFILE_VER,sizeof(sFS_COMPFILE_VER),f);

					WRITE(&size,4,f);
					WRITE(&compsize,4,f);

					CRC.Cmp(sFS_COMPFILE,sizeof(sFS_COMPFILE));
					CRC.Cmp(sFS_COMPFILE_VER,sizeof(sFS_COMPFILE_VER));
					CRC.Cmp(&size,4);
					CRC.Cmp(&compsize,4);

					crc=CRC;

					WRITE(&crc,4,f);

					WRITE(compdata,compsize,f);

					delete[]compdata;

				}
				
				fclose(f);
			}
		}
	
		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::Delete(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		if(curarchive)
		{
			//jsem v archivu
			ok = curarchive->Delete(fname);
		}else{
			FILE *f;
			f=FOPEN(fname,"rb");
			if(f)
			{
				//je to soubor 
				fclose(f);

				ok = DeleteFile(fname);
			}else{
				//neni to soubor - adresar?
				ok = ChangeDir(fname);
				if(ok){
					//je to adresar
					const char *ff=NULL;
					int dir;
					int fs;

					CFSFindInfo *fi=NULL;

					if(inFind) //ulozim findinfo
					{
						fi = new CFSFindInfo(findinfo);
						inFind = 0;
						findinfo.findHandle=INVALID_HANDLE_VALUE;
						findinfo.finddir=NULL;
					}

					fs = FindFirst(&ff,dir);
					if(fs==-1) ok=0; else
					if(fs==1){
						//v adresari neco je - musim to smazat
						do{
							if(strcmp(ff,"..")!=0){
								if(dir==0 || dir == 2)
								{
									//soubor nebo archiv
									ok = DeleteFile(ff); //smazu ho
								}else{
									//adresar
									ok = Delete(ff); //rekurze...
								}
							}
							fs = FindNext(&ff,dir);
						}while(fs==1 && ok);

						if(fs==-1) ok=0;
						FindClose();

						ok &= ChangeDir(".."); //lezu nahoru
					}
					if(ok)
						ok = RemoveDirectory(fname); //smazu adresar

					if(fi) //vratim findinfo
					{
						findinfo=*fi;
						inFind=1;
						delete fi;
					}

				}
			}
		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::CopyTree(const char *sourcedir, const char *destdir, int comp)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;

	char *sn=NULL, *dn=NULL;

	tmf=new CFSFolder(*curdir);

	if( !ChangeDir(destdir) || !ChangeDir(sourcedir) ) ok=0;

	if(ok)
		ok=CopyTree2(destdir,comp);
	

	SAFE_DELETE_ARRAY(sn);
	SAFE_DELETE_ARRAY(dn);

	if(tmf)
		SetCurDir(tmf);

	return ok;
}

int CFS::CopyTree2(const char *destdir, int comp)
{
	const char *ff=NULL;
	int dir;
	int fs;
	char *buf=NULL;
	int bs=0,sz;
	char *nm;
	int dl;
	int ok=1;

	dl=(int)strlen(destdir);
	nm = new char[dl+256];
	strcpy(nm,destdir);
	nm[dl++]='/';
	nm[dl]=0;

	CFSFindInfo *fi=NULL;

	if(inFind) //ulozim findinfo
	{
		fi = new CFSFindInfo(findinfo);
		inFind = 0;
		findinfo.findHandle=INVALID_HANDLE_VALUE;
		findinfo.finddir=NULL;
	}

	fs = FindFirst(&ff,dir);
	if(fs==-1) ok=0; else
	if(fs==1){
		do{
			if(strcmp(ff,"..")!=0){
				if(dir==0)
				{
					//soubor
					sz=GetFileSize(ff);
					if(bs<sz)
					{
						SAFE_DELETE_ARRAY(buf);
						buf=new char[sz];
						bs=sz;
					}
					ok=ReadFile(ff,buf,bs);
					strcpy(nm+dl,ff);
					if(ok)
						ok=WriteFile(nm,buf,sz,comp);
				}else{
					//adresar
					strcpy(nm+dl,ff);
					ok=CreateDir(nm);
					if(ok)
						ok=ChangeDir(ff);
					if(ok)
						ok=CopyTree2(nm,comp);
					ok=ChangeDir("..");
				}
			}
			fs = FindNext(&ff,dir);
		}while(fs==1 && ok);

		if(fs==-1) ok=0;
		FindClose();
	}

	if(fi) //vratim findinfo
	{
		findinfo=*fi;
		inFind=1;
		delete fi;
	}
	SAFE_DELETE_ARRAY(buf);
	SAFE_DELETE_ARRAY(nm);
	return ok;
}


int CFS::FileExist(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		if(curarchive)
		{
			//jsem v archivu
			ok = curarchive->FileExist(fname);
		}else{
			CFSArchive ar;
			if(ar.Open(fname,1))
			{
				ar.Close(); //archiv
				ok=3;
			}else
			{
				FILE *f;
				f=FOPEN(fname,"rb");
				if(f)
				{
					ok=1; //soubor
					fclose(f);
				}
				else
				{
					if(ChangeDir(fname)) 
						ok=2; //adresar
					else 
						ok=0; //neex.		
				}
			}
		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}


int CFS::Defragment(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	int ln=0;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		ok=0;
		if(!curarchive){
			CFSArchive ar;
			if(ar.Open(fname,0))
			{
				ok = ar.Defragment();
				ar.Close();
			}
		}
		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::GetFileSize(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	int ln=0;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname);
	if(ok)
	{
		if(curarchive)
		{
			ln=curarchive->GetFileSize(fname);
		}else{
			FILE *f;
			f=FOPEN(fname,"rb");
			if(!f) ok=0;
			else{
				ln=_filelength(_fileno(f));

				int compr=0;

				char h1[sizeof(sFS_COMPFILE)];
				char h2[sizeof(sFS_COMPFILE_VER)];
				int size;
				int compsize;
				DWORD crc;
				CCRC32 CRC;

				try{
					READ(h1,sizeof(sFS_COMPFILE),f);
					READ(h2,sizeof(sFS_COMPFILE_VER),f);
		
					READ(&size,4,f);
					READ(&compsize,4,f);

					READ(&crc,4,f);

					if(memcmp(h1,sFS_COMPFILE,sizeof(sFS_COMPFILE))==0&&memcmp(h2,sFS_COMPFILE_VER,sizeof(sFS_COMPFILE_VER))==0)
					{

						CRC.Cmp(h1,sizeof(sFS_COMPFILE));
						CRC.Cmp(h2,sizeof(sFS_COMPFILE_VER));
						CRC.Cmp(&size,4);
						CRC.Cmp(&compsize,4);

						if(crc==CRC) compr=1;

					}
				}
				catch (CExc c)
				{
					if(c.errnum!=eFS) throw;
				}

				if(compr) ln=size;

				fclose(f);
			}
		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ln;
}


int CFS::GetTime(const char *name, FILETIME& time)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		if(curarchive)
		{
			ok=curarchive->GetTime(fname,time);
		}else{

			HANDLE hFile;

			hFile = CreateFile (
			    fname,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);

			if(hFile!=INVALID_HANDLE_VALUE)
			{
				GetFileTime(hFile, NULL, NULL, &time);
				CloseHandle(hFile); 
				ok=1;
			}else
				ok=0;

		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::SetTime(const char *name, const FILETIME& time)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		if(curarchive)
		{
			ok=curarchive->SetTime(fname,time);
		}else{

			HANDLE hFile;

			hFile = CreateFile (
			    fname,
				GENERIC_WRITE ,
				FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);

			if(hFile!=INVALID_HANDLE_VALUE)
			{
				if(SetFileTime(hFile, NULL, &time, &time)) ok=1; else ok=0;
				CloseHandle(hFile); 
			}else
				ok=0;

		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}


int CFS::IsCompressed(const char *name)
{
	int ok=1;
	int compr=0;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1);
	if(ok)
	{
		if(curarchive)
		{
			compr=curarchive->IsCompressed(fname);
			ok=1;
		}else{

			compr=0;

			FILE *f;

			f=FOPEN(fname,"rb");
			if(f)
			{

				char h1[sizeof(sFS_COMPFILE)];
				char h2[sizeof(sFS_COMPFILE_VER)];
				int size;
				int compsize;
				DWORD crc;
				CCRC32 CRC;

				try{
					READ(h1,sizeof(sFS_COMPFILE),f);
					READ(h2,sizeof(sFS_COMPFILE_VER),f);
		
					READ(&size,4,f);
					READ(&compsize,4,f);

					READ(&crc,4,f);

					if(memcmp(h1,sFS_COMPFILE,sizeof(sFS_COMPFILE))==0&&memcmp(h2,sFS_COMPFILE_VER,sizeof(sFS_COMPFILE_VER))==0)
					{

						CRC.Cmp(h1,sizeof(sFS_COMPFILE));
						CRC.Cmp(h2,sizeof(sFS_COMPFILE_VER));
						CRC.Cmp(&size,4);
						CRC.Cmp(&compsize,4);

						if(crc==CRC) compr=1;

					}
				}
				catch (CExc c)
				{
					if(c.errnum!=eFS) throw;
				}

				fclose(f);
				ok=1;
			}else
				ok=0;
		}

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	if(!ok) compr=-1;

	SAFE_DELETE_ARRAY(parsname);

	return compr;
}


int CFS::CreateDir(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname);
	if(ok)
	{
		//vytvorim adresar
		if(curarchive)
		{
			if(!curarchive->CreateDir(fname)) ok=0;
		}else{

			if(_mkdir(fname)) ok=0;

		}
	
		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::CreateArchive(const char *name)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(name,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname);
	if(ok)
	{
		//vytvorim archiv
		if(curarchive)
		{
			//jsem v archivu
			//nejdou delat vnoreny archivy -> chyba
			ok=0;
		}else{

			CFSArchive ar;
			if(ar.Create(fname))
				ar.Close();
			else
				ok=0;
		}
	
		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}

	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::SplitPathAndSaveCurDir(const char *name, CFSFolder **folder, const char **file, int allowarchive, int allowemplyfilename)
{

	int i=0,l=-1;
	int ok=1;
	CFSFolder *tmf=NULL;

	if(!name||!folder||!file) return 0;

	while(name[i]) //najdu posledni /
	{
		if(name[i]=='/'||name[i]=='\\') l=i;
		i++;
	}

	if(l>0)
	{
		//zmenim adresar
		char *tm=new char[l+1];
		memcpy(tm,name,l);
		tm[l]=0;
		tmf=new CFSFolder(*curdir);
		
		if(!ChangeDir(tm)) ok=0;

		delete[]tm;
	}else
	{
		SetCurDir();
	}

	if(ok&&curdir->archivedepth==0&&(!allowarchive))
	{
		//nejsem v archivu, podivam se jestli otvirany soubor neni archiv
		CFSArchive ar;
		if(ar.Open(name+l+1,1)){
			ar.Close(); //je to archiv, zakazu otevreni
			if(tmf) //zmenil jsem adresar
			{
				//vratim starej
				SAFE_DELETE(curarchive);
				SAFE_DELETE(curdir);
				curdir=tmf;
				tmf=NULL;
			}
			ok=0;
		}
	}

	if(ok)
	{
		if( ! (allowemplyfilename && name[l+1]==0) ) 
			ok = IsValidFilename(name+l+1);
	}

	if(!ok)
	{
		SAFE_DELETE(tmf);
		return 0;
	}

	*folder=tmf;
	*file=name+l+1;

	return 1;
}

int CFS::ChangeDir(const char *curpath)
{
	char *p=NULL;
	char *c,*ls;
	int end=0,err=0;
	if(!curpath) return 0;

	if(inFind && !findinfo.finddir)
	{
		findinfo.finddir = new CFSFolder(*curdir);
	}

	int mustbefullpath=0,fp=0;
	
	if(!cfg->ParseString(curpath,&p)) return 0;

	CFSFolder *dir=new CFSFolder(*curdir);

	ls=c=p;

	if(p[0]=='/'||p[0]=='\\'){
		
		if(p[1]=='\\'){
			err=1;
			/*if(p[0]!='\\') err=1;
			else{
				fp=1;
				//sitova cesta UNC: \\server\sharepoint\path
				int tt=0;
				c=&p[2]; //musim nacist server a sharepoint
				while(tt<2&&*c){
					if(*c=='\\') tt++;
					c++;
				}
				if(tt==1&&*c==0) tt++; else c--; 
				//c je ted na '\' nebo 0
				if(tt==2)
				{
					char tm=*c;
					*c=0;
					dir->SetDir(p);
					if(_chdir(p)) err=1;
					*c=tm;
					if(tm==0) end=1;
					c++;
					ls=c;
				}else err=1;

			}*/
		}else
		{
			//root
			if(!dir->path||!isalpha((UC)dir->path[0])) err=1;
			else{
				fp=1;
				char ppp[3]="c:";
				ppp[0]=dir->path[0];
				dir->SetDir(ppp);
				if(_chdir(ppp)||_chdir("\\")) err=1;
				if(p[1]==0) end=1;
				ls=c=p+1;
			}
		}
	}else{
		if(isalpha((UC)p[0])&&p[1]==':'&&(p[2]=='\\'||p[2]=='/'||p[2]==0))
		{	//zadan disk
			fp=1;
			char ppp[3]="c:";
			ppp[0]=p[0];
			dir->SetDir(ppp);
			if(_chdir(ppp)||_chdir("\\")) err=1;
			if(p[2]==0||p[3]==0) end=1;
			ls=c=p+3;
		}


	}

	if(!fp) //neni zadana cela cesta
	{
		//nastavi adresar na disku, otevre archiv
		if(!SetCurDir()) err=1; //chyba - nepodarilo se nastavit adr.
	}else{

		SAFE_DELETE(curarchive);

	}

	if(err) end=1;

	while(!end)
	{
		if(*c==0) end=1;
		if(*c=='/'||*c=='\\'||*c==0)
		{
			*c=0;
			if(dir->archivedepth==0)
			{
				//nejsem v archivu
				//zkusim zmenit adresar
				if(_chdir(ls)==0) { 
					dir->chdir(ls,0);
					if(dir->path[0]=='\\'&&_chdir(dir->path)!=0) err=1; //sitova cesta, radsi pouziju celou cestu
				}else
				{
					//chyba - neni to adresar
					//zkusim otevrit archiv
					if(!curarchive)	curarchive=new CFSArchive();
					if(!curarchive->Open(ls,0))
					{
						SAFE_DELETE(curarchive);
						err=1;break;
					}
					//je to archiv
					dir->chdir(ls,1);
				}
			}else
			{
				//jsem v archivu
				dir->chdir(ls,1);

				if(dir->archivedepth==0){
					//vylez jsem z archivu
					SAFE_DELETE(curarchive);	
				}else{
					//zmenim adresar v archivu
					if(!curarchive->ChangeDir(ls))
					{
						SAFE_DELETE(curarchive);
						err=1;break;
					}
				}
			}
			
			ls=c+1;
		}
		c++;
	}

	SAFE_DELETE_ARRAY(p);
	
	if(!err)
	{ 
		//vse ok, je nastaven novy adresar
		SAFE_DELETE(curdir);
		curdir=dir;
		return 1;
	}
	else{
		//chyba nastavim puvodni adresar
		SAFE_DELETE(curarchive);
		SAFE_DELETE(dir);
		SetCurDir();
		return 0;
	}
}

int CFS::ChangeDirByFile(const char *filepath)
{
	if(!filepath) return 0;
	int ln = (int)strlen(filepath);

    if(ln<=1) return 1;

	int ll=ln-1;
	while( ll && filepath[ll]!='/' && filepath[ll]!='\\') ll--;
	
	if(ll==0) return 1;

	char *dir = new char[ll+1];
	strncpy(dir,filepath,ll);
	dir[ll]=0;

	int r = ChangeDir(dir);

	delete[] dir;

	return r;

}

int CFS::GetCurDir(char **dir)
{
	if(!curdir) return 0;
	return curdir->GetDir(dir);
}

int CFS::GetFullPath(const char *relpath, char **fullpath)
{
	int ok=1;
	
	CFSFolder *tmf=NULL;
	const char *fname;

	char *parsname=NULL;

	if( !cfg->ParseString(relpath,&parsname) ) return 0;

	ok=SplitPathAndSaveCurDir(parsname,&tmf,&fname,1,1);
	if(ok)
	{
		ok=curdir->GetDir(fullpath,fname);

		if(tmf)
		{
			//vratim adresar
			SAFE_DELETE(curarchive);
			SAFE_DELETE(curdir);
			curdir=tmf;
		}
	}
	SAFE_DELETE_ARRAY(parsname);

	return ok;
}

int CFS::SetCurDir(CFSFolder *dir)
{
	int ok=0;
	
	if(!dir) return 0;

	SAFE_DELETE(curarchive);

	while(!ok){
		if(_chdir(dir->path)) break; //zmeni cestu na disku
		if(isalpha((UC)dir->path[0])&&dir->path[1]==':'&&dir->path[2]==0) 
			if(_chdir("\\")) break; //nastavi root

		if(dir->archivedepth) //otevre archiv
		{
			curarchive=new CFSArchive();
			if(!curarchive->Open(dir->archivename,0)) break;
			if(!curarchive->ChangeDir(dir->pathinarchive)) break;
		}
		ok=1;
	}

	if(!ok)
	{
		SAFE_DELETE(curarchive);
		SetCurDir();
	}else
	{
		if(inFind && !findinfo.finddir)
			findinfo.finddir = new CFSFolder(*curdir);
		
		SAFE_DELETE(curdir);
		curdir=dir;
	}

	return ok;
}

int CFS::SetCurDir()
{
	int ok=0;
	
	while(!ok){
		if(_chdir(curdir->path)) break; //zmeni cestu na disku
		if(isalpha((UC)curdir->path[0])&&curdir->path[1]==':'&&curdir->path[2]==0) 
			if(_chdir("\\")) break; //nastavi root
		if(curdir->archivedepth&&!curarchive) //otevre archiv
		{
			curarchive=new CFSArchive();
			if(!curarchive->Open(curdir->archivename,0)) break;
			if(!curarchive->ChangeDir(curdir->pathinarchive)) break;
		}
		ok=1;
	}

	if(!ok)
	{
		SAFE_DELETE(curarchive);
		
		//ToDo: vyhodit vyjimku

	}

	return ok;
}


int CFS::FindFirst(const char **filename, int &dir)
{
	int res;

	FindClose();

	if( !SetCurDir() ) return -1;

	if(curarchive)
	{
		res = curarchive->GetCurDirItem(0,filename,dir);

		if(!res) return 0;

		inFind = 2;
		findinfo.findArchiveLastNum = 0;

		return 1;

	}else{

		HANDLE fh;

		fh = FindFirstFile("*.*", &findinfo.FindFileData);

		if(fh == INVALID_HANDLE_VALUE ) return 0;

        if(strcmp(findinfo.FindFileData.cFileName,".")==0)
		{
			BOOL res;
			res = FindNextFile(fh,&findinfo.FindFileData);
			if(!res){
				::FindClose(fh);
				return 0;
			}
		}

		inFind = 1;
		findinfo.findHandle = fh;

		*filename = findinfo.FindFileData.cFileName;		

		if( findinfo.FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )		
 			dir = 1; //je to adresar
		else
		{

			CFSArchive ar;
			if(ar.Open(*filename,1)){
				ar.Close(); 
				dir = 2; //je to archiv
			}
			else
				dir = 0; //je to soubor
		}        

		return 1;
	}
}

int CFS::FindNext(const char **filename, int &dir)
{
	int ok;
	if(!inFind) return -1;

	CFSFolder *cd=NULL;

	if(findinfo.finddir)
	{
		cd=new CFSFolder(*curdir);
		CFSFolder *nf=new CFSFolder(*findinfo.finddir);
		SetCurDir(nf);
	}

	if(inFind==1)
	{
		BOOL res;
		res = FindNextFile(findinfo.findHandle,&findinfo.FindFileData);
		if(!res){
			FindClose();
			ok=0;
		}
		else
		{

			*filename = findinfo.FindFileData.cFileName;

			if( findinfo.FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )		
				dir = 1;
			else
			{

				CFSArchive ar;
				if(ar.Open(*filename,1)){
					ar.Close(); 
					dir = 2; //je to archiv
				}
				else
					dir = 0; //je to soubor
			}        
			ok=1;
		}
	}else{
		int res;
		findinfo.findArchiveLastNum++;
		res = curarchive->GetCurDirItem(findinfo.findArchiveLastNum,filename,dir);

		if(!res) {
			FindClose();
			ok=0;
		}else
			ok=1;
	}
	if(cd)
	{
		SetCurDir(cd);
	}
	return ok;
}

int CFS::FindClose()
{
	if(inFind==1)
	{
		::FindClose(findinfo.findHandle);
		findinfo.findHandle=INVALID_HANDLE_VALUE;
	}

	SAFE_DELETE(findinfo.finddir);

	inFind=0;

	return 1;
}

int CFS::ComparePath(const char *path1, const char *path2)
{	
	if(!path1||!path2) return 0;

	const char *c1,*c2;
	int l1,l2;

	for( c1=path1,c2=path2; *c1!=0; c1++,c2++ )
	{
		l1=*c1;l2=*c2;
		if(!l2) return 1;

		l1=tolower(l1);
		l2=tolower(l2);

		if(l1=='/'||l1=='\\'){
			if(l2=='/') continue; 
			if(l2=='\\') continue;
			return 1;
		}

		if(l1!=l2) return 1;
	}

	if(*c2) return 1;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


CFSFolder::CFSFolder()
{

	pathlen=GetCurrentDirectory(0,path);
	path=new char[pathlen];
	GetCurrentDirectory(pathlen,path);

	pathinarchive=NULL;pathinarchivelen=0;
	archivename=NULL;archivenamelen=0;

	archivedepth=0;

}

CFSFolder::CFSFolder(const CFSFolder &f)
{
	pathlen=f.pathlen;
	pathinarchivelen=f.pathinarchivelen;
	archivenamelen=f.archivenamelen;
	if(f.path)
	{
		path=new char[pathlen];
		memcpy(path,f.path,pathlen);
	}else path=NULL;
	if(f.pathinarchive)
	{
		pathinarchive=new char[pathinarchivelen];
		memcpy(pathinarchive,f.pathinarchive,pathinarchivelen);
	}else pathinarchive=NULL;
	if(f.archivename)
	{
		archivename=new char[archivenamelen];
		memcpy(archivename,f.archivename,archivenamelen);
	}else archivename=NULL;

	archivedepth=f.archivedepth;
}

CFSFolder::~CFSFolder()
{
	SAFE_DELETE(path);
	SAFE_DELETE(pathinarchive);
	SAFE_DELETE(archivename);
}

int CFSFolder::SetDir(const char *dir)
{
	if(!dir) return 0;


	int ln=(int)strlen(dir)+1;

	if(pathlen<ln)
	{
		SAFE_DELETE_ARRAY(path);
		path=new char[ln];
		pathlen=ln;
	}
	strcpy(path,dir);

	archivedepth=0;
	if(archivename) archivename[0]=0;
	if(pathinarchive) pathinarchive[0]=0;

	return 1;
}

int CFSFolder::chdir(const char *name, int archive)
{
	char *lastbs;
	char *c;
	int ln,ln2;

	if(strcmp(name,"..")==0)
	{ //lezu nahoru
        if(archivedepth)
		{
			archivedepth--;
			if(archivedepth==0) { archivename[0]=0; pathinarchive[0]=0; return 1;}
			else
				c=pathinarchive;
		}else
			c=path;

        while(*c) //najdu posledni '\'
		{
			if(*c=='/'||*c=='\\') lastbs=c;
			c++;
		}
		if(lastbs) *lastbs=0; //zkrati jmeno
		else return 0;		
	}else
	{
		ln=(int)strlen(name)+1; //+1=koncova nula
		if(archive&&archivedepth==0)
		{ //lezu do archivu
			archivedepth=1;
			if(archivenamelen<ln)
			{
				SAFE_DELETE_ARRAY(archivename);
				archivename=new char[ln];
				archivenamelen=ln;
			}
			strcpy(archivename,name);
			if(pathinarchivelen<1)
			{
				SAFE_DELETE_ARRAY(pathinarchive);
				pathinarchive=new char[1];
				pathinarchivelen=1;
			}
			pathinarchive[0]=0;
		}else
		{
			//lezu dolu
			ln++; //prida 1 na '\'

			if(archivedepth)
			{
				//lezu dolu archivem
				archivedepth++;
				ln2=(int)strlen(pathinarchive);
				if(pathinarchivelen<ln2+ln)
				{
					c=pathinarchive;
					pathinarchive=new char[ln2+ln];
					pathinarchivelen=ln2+ln;
					strcpy(pathinarchive,c);
					SAFE_DELETE_ARRAY(c);
				}
				strcat(pathinarchive,"\\");
				strcat(pathinarchive,name);
			}else
			{
				//lezu dolu po disku
				ln2=(int)strlen(path);
				if(pathlen<ln2+ln)
				{
					c=path;
					path=new char[ln2+ln];
					pathlen=ln2+ln;
					strcpy(path,c);
					SAFE_DELETE_ARRAY(c);
				}
				strcat(path,"\\");
				strcat(path,name);
			}
		}

	}
	return 1;
}

int CFSFolder::GetDir(char **dir, const char *filename)
{
	int sz=0;
	if(!dir) return 0;

	if(path) sz=(int)strlen(path);
	if(archivedepth){
		if(archivename) sz+=1+(int)strlen(archivename);
		if(pathinarchive) sz+=(int)strlen(pathinarchive);
	}
	if(filename) sz+=1+(int)strlen(filename);

	*dir=new char[sz+1];

	*dir[0]=0;

	if(path) strcat(*dir,path);	
	if(archivedepth)
	{
		if(archivename)	{
			strcat(*dir,"\\");
			strcat(*dir,archivename);
		}
		if(pathinarchive) strcat(*dir,pathinarchive);
	}
	if(filename)
	{
		strcat(*dir,"\\");
		strcat(*dir,filename);
	}
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CFSCfg::CFSCfg(const char *fname)
{
	numkeys=0;
	ReadCfg(fname);
}

CFSCfg::~CFSCfg()
{
	for(int i=0;i<numkeys;i++)
	{
		SAFE_DELETE_ARRAY(keys[i]);
		SAFE_DELETE_ARRAY(vals[i]);
	}
	numkeys=0;
}

int CFSCfg::ReadCfg(const char *fname)
{

	char *fp;
	char *buf=NULL;
	int ln;

	//nastavi $cfg$
	numkeys=1;
	keys[0]=new char[4];
	strcpy(keys[0],"cfg");

	ln=GetFullPathName(fname,0,buf,&fp);
	buf=new char[ln];
	GetFullPathName(fname,ln,buf,&fp);
	fp--;
	*fp=0;
	ln=(int)strlen(buf);

	vals[0]=new char[ln+1];
	strcpy(vals[0],buf);
	vallen[0]=ln;

	SAFE_DELETE_ARRAY(buf);

	int end=0;
	int c;
	int ls=1;
	int cmt=0;
	int mode=0;
	char key[FS_MAX_KEYLEN];
	char val[FS_MAX_VALLEN];
	int kl=0,vl=0;
	char *ttt=NULL;

	FILE *f=FOPEN(fname,"rt");
	if(f)
	{
		while(!end)
		{
			c=fgetc(f);
			if(c==EOF) {c='\n';end=1;}
			if(ls&&c=='/') cmt=1; else
			if(cmt==1){
				if(c=='/') cmt=2; else cmt=2;
			}

			if(cmt==0){
				if(mode==0&&!isspace(c)) mode=1;
				if(mode==1) {
					if(c!='='&&!isspace(c))
					{
						key[kl++]=c;
						if(kl>=FS_MAX_KEYLEN) mode=10;
					}
					else {
						key[kl]=0;mode=2;
					}
				}
				if(mode==2){
					if(c=='=') mode=3;
				}else
				if(mode==3){
					if(!isspace(c)) mode=4;
				}

				if(mode==4){
					if(!isspace(c)){						
						val[vl++]=c;
						if(vl>=FS_MAX_VALLEN) mode=10;

					}else {

						val[vl]=0;
						if(kl&&vl)
						{
							if(ParseString(val,&ttt))
							{
								
								keys[numkeys]=new char[kl+1];
								strcpy(keys[numkeys],key);								
								vals[numkeys]=ttt;

								vallen[numkeys]=(int)strlen(ttt);
								
								numkeys++;
								if(numkeys>=FS_MAX_CFGPATH) end=1;
								
							}
							ttt=NULL;

						}

						mode=5;
					}
				}
			}

			if(c=='\n') {ls=1;cmt=0;mode=0;kl=0;vl=0;} else ls=0;
		}

		fclose(f);
	}


	return 1;
}

int CFSCfg::AddKey(const char *key, const char *val, int newkey) //prida klic
{
	int keynum;

	keynum = FindKey(key);

	char *ttt=NULL;

    if(newkey){
		if(numkeys==FS_MAX_CFGPATH || keynum!=-1 ) return 0;
		keynum = numkeys;
	}else{
		if(keynum==-1) return 0;
	}

	if(!ParseString(val,&ttt)) return 0;

	if(newkey) numkeys++;

	keys[keynum]=new char[strlen(key)+1];
	strcpy(keys[keynum],key);								
	vals[keynum]=ttt;

	vallen[keynum]=(int)strlen(ttt);

	return 1;
}

int CFSCfg::ParseString(const char *instr, char **outstr)
{
	if(!instr||!outstr) return 0;

	int ln=0,ki,ln2=0;
	int ink=0;
	const char *c,*k;
	char *o,*kk;

	c=instr;
	k=NULL;

	while(*c)
	{
		if(*c=='$'){
			if(ink){
				if((ki=FindKey(k))!=-1)
					ln+=vallen[ki];				
				else return 0;
				ink=0;
			}else{
				ink=1;
				k=c;
			}
		}else 
		if(!ink) ln++;
		c++;
	}

	ln++;

	c=instr;
	k=NULL;
	ink=0;

	*outstr=new char[ln];
	o=*outstr;

	while(*c)
	{
		if(*c=='$'){
			if(ink){
				if((ki=FindKey(k))!=-1)
				{
					kk=vals[ki];
					while(*kk)
					{
						*o++=*kk++;
						ln2++;
					}
				}
				ink=0;
			}else{
				ink=1;
				k=c;
			}
		}else
		if(ink==0){			
			*o++=*c;ln2++;
		}
		c++;
	}
	*o=0;

	return 1; //smazat outstr
}

int CFSCfg::FindKey(const char *key)
{
	const char *k,*l;
	for(int i=0;i<numkeys;i++)
	{
		k=key+1;
		l=keys[i];
		while(*k==*l){ k++;l++;}
		if(*k=='$'&&*l==0) return i;
		
	}
	return -1;
}