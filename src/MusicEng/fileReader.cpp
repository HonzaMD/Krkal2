/////////////////////////////////////////////////////////////////////////////
//
// fileReader.h
//
// fileReader pro Audiere
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fileReader.h"

#include "types.h"
#include "fs.h"

using namespace audiere;

#define KRKALMUSIC "KRKALmusic"

CKrkalFile::CKrkalFile(char *filename, FileFormat &ff)
{
	filedata=0;filesize=0;filepos=0;
	filebuf=0;
	
	int sz = FS->GetFileSize(filename);

	if(sz>0){
		filesize = sz;
		filebuf = new char[sz];
		FS->ReadFile(filename,filebuf,filesize);

		filedata = filebuf;

		if(ff==FF_AUTODETECT)
		{
			int ll=strlen(KRKALMUSIC);
			if(memcmp(filebuf,KRKALMUSIC,ll)==0)
			{
				filedata+=ll;
				FileFormat *fp=(FileFormat*)filedata;
				filedata+=sizeof(FileFormat);
				filesize-=ll+sizeof(FileFormat);
				ff=*fp;
			}
		}

	}

}

CKrkalFile::~CKrkalFile()
{
	SAFE_DELETE_ARRAY(filebuf);
}
