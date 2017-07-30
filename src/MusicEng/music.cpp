/////////////////////////////////////////////////////////////////////////////
//
// music.cpp
//
// prehravani hudby a zvuku
//
// A: Petr Altman
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "types.h"

#include "music.h"
#include "musicmod.h"

#include "fs.h"

#include "genmain.h"

#include "kernel.h"
#include "names.h"


using namespace audiere;

CMusicEngine* MusicEngine = NULL;

CMusicEngine::CMusicEngine()
{
	SAFE_DELETE(MusicEngine);

	curmusic = 0;
	nextmusic = 0;

	device = OpenDevice();
	if (!device) {
	    // failure
	}

	fade=0;

	MusicEngine = this;

	soundcfg = new CSoundCfg();
	soundcfg->ReadCfg();
}

CMusicEngine::~CMusicEngine()
{
	if(curmusic) curmusic->Stop();

	SAFE_RELEASE(curmusic);
	SAFE_RELEASE(nextmusic);

	MusicEngine = NULL;

	SAFE_DELETE(soundcfg);
}

int CMusicEngine::RegisterKernelSound()
{
	if(!soundcfg) return 0;
	return soundcfg->RegisterKernelSound();
}

int CMusicEngine::Fade(float destvol, float time)
{
	if(!curmusic) return 0;
	fade=1;
	float vol = curmusic->GetVolume();
	destVol = destvol;
	volIncr = (destvol-vol)/time;
	if(volIncr==0) fade=0;
	return 1;
}

int CMusicEngine::Timer(float timediff)
{
	if(!fade) return 0;
	if(!curmusic) {fade=0;return 0;}

	float vol = curmusic->GetVolume();
	vol+=volIncr*timediff;
	if(volIncr<0)
	{
		if(vol<=destVol) {vol=destVol;fade=0;}
	}else{
		if(vol>=destVol) {vol=destVol;fade=0;}
	}
	curmusic->SetVolume(vol);
	if(fade==0 && nextmusic){
		curmusic->Stop();
		curmusic->Release();
		curmusic=nextmusic;
		nextmusic=0;
		curmusic->Reset();
		curmusic->Play();
	}
	return 1;
}

int CMusicEngine::Play(CMusicModule *mm, int fadeout, float fadetime)
{
	if(!mm) return 0;

	fade=0;
	SAFE_RELEASE(nextmusic);

	if(curmusic&&fadetime>0){
		Fade(0,fadetime);

		nextmusic=mm;
		nextmusic->AddRef();
	}else{
		if(curmusic){
			curmusic->Stop();
			curmusic->Release();
		}
		curmusic = mm;
		curmusic->AddRef();
		curmusic->Reset();
		curmusic->Play();
	}

	return 1;
}

CMusicModule* CMusicEngine::Play(char *musicfile, int fadeout, float fadetime)
{
	if(curmusic)
	{
		char *fn=curmusic->GetFilename();
		if(fn)
		{
			if(FS->ComparePath(fn,musicfile)==0) return curmusic;
		}
	}


	CMusicModule *mm = new CMusicModule;
	if(!mm->LoadFromFile(musicfile))
	{
		mm->Release();
		return 0;
	}

	int r=Play(mm,fadeout,fadetime);
	mm->Release();
	if(r) return mm;
	return 0;
}

int CMusicEngine::Stop()
{
	if(!curmusic) return 0;
	if(curmusic){
		curmusic->Stop();
		SAFE_RELEASE(curmusic);
	}
	SAFE_RELEASE(nextmusic);
	fade=0;
	return 1;
}

CMusicSample* CMusicEngine::GetMusicSample( CKerName *soundname )
{
	if(!soundname || soundname->Type != eKerNTsound ) {
		KerMain->Errors->LogError(eKRTESoundNameExpected);
		return 0;
	}

	CMusicSample *ms=soundname->Sound;

	return ms;
}

int CMusicEngine::PlaySample( CKerName *soundname, int x, int y, double volume, float pitch)
{
	if(!soundname || soundname->Type != eKerNTsound ) {
		KerMain->Errors->LogError(eKRTESoundNameExpected);
		return 0;
	}

	float sx,sy,scx,scy;
	GEnMain->GetWindowSize(sx,sy);
	GEnMain->GetScroll(scx,scy);
	sx=sx/2;sy=sy/2;
	scx+=sx; //spocitam stred herni obrazovky
	scy+=sy;

	float px = x-scx;
	x=abs(x-(int)scx);y=abs(y-(int)scy);
    double vol=(x+y)/((sx+sy) * 1.4);
	if(vol<0) vol=0; else
	if(vol>1) vol=1;
	volume*=(1-vol);
	if(volume==0) return 1;

	CMusicSample *ms=soundname->Sound;
	if(!ms) return 0;


	px=px/sx;
	if(px<-1) px=-1; else
	if(px>1) px=1;

/*	ms->SetVolume((float)volume);
	ms->SetPan(px);*/
	ms->Play((float)volume,px);

	return 1;
}

//////////////////////////////////////////////////////////////////////////

CSoundCfg::CSoundCfg()
{
}

CSoundCfg::~CSoundCfg()
{
	DeleteListElems(soundlist);
}

int CSoundCfg::ReadCfg(char *filename)
{
	int sz = FS->GetFileSize(filename);

	if(sz<=0) return 0;
	sz++;

	char *buf = new char[sz];

	if(!FS->ReadFile(filename,buf,sz))
	{
		delete[] buf;
		return 0;
	}

	buf[sz-1] = 10; //pridam konec posledni radky

	int incoment = 0;
	UC *ch=(UC*)buf;
	int mode = 0;
	char *token;
	char *name=0,*sndfilename=0;

	int err=0;
	UC tmch;
	CSoundStruct *ss;

	for( ; sz && !err ; sz--,ch++ )
	{
        if(incoment && (*ch==13||*ch==10) ) { 
			incoment=0;
		}
		if(!incoment)
		{
			if(ch[0]=='/' && ch[1]=='/') incoment=1;
			else{
				switch(mode)
				{
				case 0:
					if(isspace(*ch)) break;
					mode = 1;
					token = (char*)ch;
					name=0;sndfilename=0;
				case 1:
					if(isspace(*ch)||*ch=='=')
					{
						tmch=*ch;
						*ch=0;
						name = newstrdup(token);
						*ch=tmch;
						mode=2;
					}else
						break;
				case 2:
					if(isspace(*ch)) break;
					mode=3;
				case 3:
					if(*ch!='=') err=1;
					mode = 4;
					break;
				case 4:
					if(isspace(*ch)) break;
					mode = 5;
				case 5:
					if(*ch!='"') err=1;
					mode = 6;
					token = (char*)ch+1;
					break;
				case 6:
					if(*ch!='"') break;
					tmch=*ch;
					*ch=0;
					sndfilename = newstrdup(token);
					*ch=tmch;
					mode=7;
					break;
				}

				if(*ch==13||*ch==10){
					if(mode>0)
					{
						if(mode<7) {err=1; break;}
						ss = new CSoundStruct(name,sndfilename);
						soundlist.Add(ss);
					}

					name = 0;
					sndfilename = 0;
					mode = 0;
				}

			}
		}
	}

	if(err)
	{
		SAFE_DELETE(name);
		SAFE_DELETE(sndfilename);
	}else
	{
		assert(!name);
		assert(!sndfilename);
	}

	delete[] buf;

	return !err;
}

int CSoundCfg::RegisterKernelSound()
{
	if(!KerMain) return 0;
	CKerNamesMain *kn = KerMain->KerNamesMain;
	if(!kn) return 0;

	CKerName *name;

	CListIterator<CSoundStruct*> it(soundlist);
	while(!it.End())
	{
		name = kn->GetNamePointer(it->name);
		if(name)
		{
			if(name->Type==eKerNTvoid)
			{
				CMusicSample *ms = GEnMain->LoadMusicSample(it->filename);
				if(ms)
				{	
					ms->AddRef();
					name->Type = eKerNTsound;
					name->Sound = ms;					
				}else
				KerMain->Errors->LogError(eKRTEbadSoundName,0,it->name);
			}else{
				KerMain->Errors->LogError(eKRTEbadSoundName,0,it->name);
			}
		}
		it++;
	}
	
	return 1;
}