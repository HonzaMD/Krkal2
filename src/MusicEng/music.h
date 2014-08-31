/////////////////////////////////////////////////////////////////////////////
//
// music.h
//
// prehravani hudby a zvuku
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#ifndef KRKALMUSIC_H
#define KRKALMUSIC_H

#include "audiere.h"

class CMusicModule;
class CMusicSample;

class CMusicEngine{
public:
	CMusicEngine();
	~CMusicEngine();

	int Play(CMusicModule *mm, int fadeout=0, float fadetime=2); //zacne prehravat hudbu, pokud je fadeout=1, tak se stara hudba fadne, fadetime je v sekundach
	CMusicModule* Play(char *musicfile, int fadeout=0, float fadetime=2);  //nahraje a zacne prehravat hudbu, vraci pointr na musicmodule (pri chybe null) - pokud chcete pointr pouzivat zvyste nejprve pocet referenci
	int Stop();    

	CMusicModule* GetCurMusic(){return curmusic;} 

	int Fade(float destvol, float time);
	int IsFading(){return fade;}

	int PlaySample( class CKerName *soundname, int x, int y, double volume=1, float pitch=1); //prehraje zvuk, volume je 0..1, x,y souradnice, kde ma zvuk hrat, pitch=zrychleni 0.5-2
    CMusicSample* GetMusicSample( class CKerName *soundname ); //vrati zvuk

	int RegisterKernelSound();

	int Timer(float timediff);

	audiere::AudioDevicePtr GetAudiereDevice(){return device;}
private:
	audiere::AudioDevicePtr device;

	CMusicModule *curmusic,*nextmusic;

	int fade;
	float volIncr, destVol;

	class CSoundCfg* soundcfg;

};

extern CMusicEngine* MusicEngine;
#define ME MusicEngine

struct CSoundStruct{
	CSoundStruct(char *_name, char *_filename )
	{
		name = _name;
		filename = _filename;
	}
	~CSoundStruct(){
		SAFE_DELETE_ARRAY(name);
		SAFE_DELETE_ARRAY(filename);
	}

	char *name,*filename;
};

class CSoundCfg
{
public:
	CSoundCfg();
	~CSoundCfg();

	int ReadCfg( char *filename = "$DATA$/sound.cfg" );

	int RegisterKernelSound();

protected:
	CListK<CSoundStruct*> soundlist;
};

#endif