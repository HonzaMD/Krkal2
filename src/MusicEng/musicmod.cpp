/////////////////////////////////////////////////////////////////////////////
//
// musicmod.cpp
//
// music module
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "musicmod.h"
#include "music.h"
#include "filereader.h"

#include "ident.h"

#include "types.h"
#include "fs.h"
#include "krkal.h"


using namespace audiere;

#define KRKALMUSIC "KRKALmusic"

////////////////////////////////////////////////////////

 bool end_is(const char* begin, const char* ext) {
    const char* end = begin + strlen(begin);
    int ext_length = strlen(ext);
    if (ext_length > end - begin) {
      return false;
    } else {
      return (_stricmp(end - ext_length, ext) == 0);
    }
  }


FileFormat GuessFormat(const char* filename) {
    if (end_is(filename, ".aiff")) {
      return FF_AIFF;
    } else if (end_is(filename, ".wav")) {
      return FF_WAV;
    } else if (end_is(filename, ".ogg")) {
      return FF_OGG;
    } else if (end_is(filename, ".flac")) {
      return FF_FLAC;
    } else if (end_is(filename, ".mp3")) {
      return FF_MP3;
    } else if (end_is(filename, ".it") ||
               end_is(filename, ".xm") ||
               end_is(filename, ".s3m") ||
               end_is(filename, ".mod")) {
      return FF_MOD;
    } else 
      return FF_AUTODETECT;
    
  }


////////////////////////////////////////////////////////

CMusicModule::CMusicModule()
{
	Filename=0;
	stream=0;
}

CMusicModule::~CMusicModule()
{
	SAFE_DELETE_ARRAY(Filename);
}

int CMusicModule::Load( char *name, CGEnPackage *package, int lazy, int myindex  )
{
	return LoadFromFile(name);
}

int CMusicModule::LoadFromFile( char *filename )
{

	FileFormat ff = GuessFormat(filename);

	if(ff==FF_AUTODETECT) {
		if(!end_is(filename,".mm"))
			return 0;
	}

	FilePtr file = new CKrkalFile(filename, ff);
	stream = OpenSound(ME->GetAudiereDevice(),file,true,ff);

	if(!stream)
		return 0;

	SAFE_DELETE_ARRAY(Filename);
	Filename = newstrdup(filename);

	return 1;
}

int CMusicModule::SaveFromFile(char *pkgname, char *filename, CGEnPackage *pkg, int compr)
{
	int mm=0,ok;
	FileFormat ff = GuessFormat(filename);
	if(ff == FF_AUTODETECT) {
		if(end_is(filename,".mm"))
			mm=1;
		else
			return 0;
	}

	int sz = FS->GetFileSize(filename);
	if(sz<=0) return 0;
	int szmm=sz;
	int headsz=strlen(KRKALMUSIC);
	if(!mm)
		szmm+=headsz+sizeof(FileFormat);
	char *buf=new char [szmm];
	char *b=buf;
	if(!mm)
	{
		memcpy(b,KRKALMUSIC,headsz);
		b+=headsz;
		memcpy(b,&ff,sizeof(FileFormat));
		b+=sizeof(FileFormat);
	}
	ok = FS->ReadFile(filename,b,sz);

	if(ok&&mm)
		if(memcmp(b,KRKALMUSIC,headsz)!=0) ok=0;
	
	if(ok)
	{
		if(ff==FF_OGG||ff==FF_MP3) compr=0;
		ok = FS->WriteFile(pkgname,buf,szmm,compr);
	}

    delete[] buf;

	return ok;

}

int CMusicModule::Play()
{
	if(!stream) return 0;

	SetVolume(1);

	if(KRKAL->cfg.music)
		stream->play();
	SetRepeat();

	return 1;
}
int CMusicModule::Stop()
{
	if(!stream) return 0;
	stream->stop();
	return 1;
}
int CMusicModule::Reset()
{
	if(!stream) return 0;
	stream->reset();
	return 1;
}

void CMusicModule::SetRepeat(bool repeat)
{
	if(!stream) return;
	stream->setRepeat(repeat);
}

void CMusicModule::SetVolume(float volume)
{
	if(!stream) return;
	volume *= KRKAL->cfg.musicvolume;
	stream->setVolume(volume);
}
float CMusicModule::GetVolume()
{
	if(!stream) return 0;
	float volume = stream->getVolume();
	if(KRKAL->cfg.musicvolume>0)
		volume *= 1/KRKAL->cfg.musicvolume;
	return volume;
}

void CMusicModule::SetPan(float pan)
{
	if(!stream) return;
	stream->setPan(pan);
}
float CMusicModule::GetPan()
{
	if(!stream) return 0;
	return stream->getPan();
}
/////////////////////////////////////////////////////

CMusicSample::CMusicSample()
{
	for(int i=0;i<maxSOUND;i++)
		stream[i]=0;
}

CMusicSample::~CMusicSample()
{
}

int CMusicSample::Load( char *name, CGEnPackage *package, int lazy, int myindex  )
{
	return LoadFromFile(name);
}
int CMusicSample::LoadFromFile( char *filename )
{
	FileFormat ff = GuessFormat(filename);

	if(ff==FF_AUTODETECT) {
		if(!end_is(filename,".ms"))
			return 0;
	}

	FilePtr file = new CKrkalFile(filename,ff);

/*	
	soundeffect = OpenSoundEffect(ME->GetAudiereDevice(),file,MULTIPLE,ff);
	if(!soundeffect)
		return 0;
*/

	SampleSource* source = OpenSampleSource(file, ff);
	sb = CreateSampleBuffer(source);
	if(!sb) return 0;
	for(int i=0;i<maxSOUND;i++)
		stream[i] = ME->GetAudiereDevice()->openStream(sb->openStream());
    //    return (sb ? new MultipleSoundEffect(device, sb) : 0);


	return 1;
}

int CMusicSample::SaveFromFile(char *pkgname, char *filename, CGEnPackage *pkg, int compr)
{
	int mm=0,ok;
	FileFormat ff = GuessFormat(filename);
	if(ff == FF_AUTODETECT) {
		if(end_is(filename,".ms"))
			mm=1;
		else
			return 0;
	}

	int sz = FS->GetFileSize(filename);
	if(sz<=0) return 0;
	int szmm=sz;
	int headsz=strlen(KRKALMUSIC);
	if(!mm)
		szmm+=headsz+sizeof(FileFormat);
	char *buf=new char [szmm];
	char *b=buf;
	if(!mm)
	{
		memcpy(b,KRKALMUSIC,headsz);
		b+=headsz;
		memcpy(b,&ff,sizeof(FileFormat));
		b+=sizeof(FileFormat);
	}
	ok = FS->ReadFile(filename,b,sz);

	if(ok&&mm)
		if(memcmp(b,KRKALMUSIC,headsz)!=0) ok=0;
	
	if(ok)
	{
		if(ff==FF_OGG||ff==FF_MP3) compr=0;
		ok = FS->WriteFile(pkgname,buf,szmm,compr);
	}

    delete[] buf;

	return ok;

}


int CMusicSample::Play(float volume,float pan,float pitch)
{
	if(!sb) return 0;

	OutputStreamPtr s=0;
	int index=-1;
	float minvol=10;

	volume *= KRKAL->cfg.soundvolume;

	for(int i=0;i<maxSOUND;i++)
	{
		if(!stream[i]||!stream[i]->isPlaying())
		{
			index=i;
			break;
		}
		if(stream[i]->getVolume()<=minvol){
			index=i;
			minvol = stream[i]->getVolume();
		}
	}

	if(index==-1) return 0;

	if(!stream[index])
	{
		s = ME->GetAudiereDevice()->openStream(sb->openStream());
		if (!s) {
			return 0;
		}
		stream[index]=s;
	}else{
		s=stream[index];
		s->reset();
	}

	s->setVolume(volume);
	s->setPan(pan);
	s->setPitchShift(pitch);
	s->play();
	s=0;

	return 1;

/*	if(!soundeffect) return 0;
	if(KRKAL->cfg.sound)
		soundeffect->play();*/
	return 1;
}
/*
int CMusicSample::Stop()
{
	if(!soundeffect) return 0;
	soundeffect->stop();
	return 1;
}

void CMusicSample::SetVolume(float volume)
{
	if(!soundeffect) return;
	soundeffect->setVolume(volume);
}
float CMusicSample::GetVolume()
{
	if(!soundeffect) return 0;
	return soundeffect->getVolume();
}

void CMusicSample::SetPan(float pan)
{
	if(!soundeffect) return;
	soundeffect->setPan(pan);
}
float CMusicSample::GetPan()
{
	if(!soundeffect) return 0;
	return soundeffect->getPan();
}
void CMusicSample::SetPitch(float pitch)
{
	if(!soundeffect) return;
	soundeffect->setPitchShift(pitch);
}
*/
