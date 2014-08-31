/////////////////////////////////////////////////////////////////////////////
//
// music.h
//
// music module
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////

#ifndef MUSICMOD_H
#define MUSICMOD_H

#include "audiere.h"

#include "genresource.h"

class CMusicModule;

class CMusicModule: public CGEnResource{
	friend class CMusicEngine;
public:
	CMusicModule();
	~CMusicModule();

	int Play();
	int Stop();
	int Reset();
	void SetRepeat(bool repeat=true);

	void SetVolume(float volume); //nastavi hlasitost - volume: 0=min 1=max
	float GetVolume();

	void SetPan(float pan=0); //nastavi panning - pan: -1=left 0=center 1=right
	float GetPan();

	virtual eResourceType GetType(){return resMusicModule;}
	virtual int Save( char *name, CGEnPackage *package ){return 0;}
	virtual int Load( char *name, CGEnPackage *package, int lazy=0, int myindex=-1  );
	static int SaveFromFile( char *pkgname, char *filename, CGEnPackage *package, int compr );

	int LoadFromFile( char *filename );

	char* GetFilename(){return Filename;}

protected:
	audiere::OutputStreamPtr stream;
	char *Filename;
};

class CMusicSample: public CGEnResource{
	friend class CMusicEngine;
public:
	CMusicSample();
	~CMusicSample();

	int Play(float volume=1,float pan=0,float pitch=1);
/*	int Stop();

	void SetVolume(float volume); //nastavi hlasitost - volume: 0=min 1=max
	float GetVolume();
	void SetPan(float pan=0); //nastavi panning - pan: -1=left 0=center 1=right
	float GetPan();
	void SetPitch(float pitch=1); //zrychleni/zpomaleni prehravani, pitch: 0.5-2
	float GetPitch();
*/

	virtual eResourceType GetType(){return resMusicSample;}
	virtual int Save( char *name, CGEnPackage *package ){return 0;}
	virtual int Load( char *name, CGEnPackage *package, int lazy=0, int myindex=-1  );
	static int SaveFromFile( char *pkgname, char *filename, CGEnPackage *package, int compr );

	int LoadFromFile( char *filename );

protected:
	//audiere::SoundEffectPtr soundeffect;
	audiere::SampleBufferPtr sb;

#define maxSOUND 3

	audiere::OutputStreamPtr stream[maxSOUND];


};


#endif