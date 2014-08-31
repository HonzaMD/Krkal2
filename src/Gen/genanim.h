////////////////////////////////////////////////////////////////////////////////
//
//		GEnAnim
//
//		animace
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////


#ifndef GENANIM_H
#define GENANIM_H

#include "genresource.h"
#include "lighttex.h"

class CGEnAnim: public CGEnResource{
public:

	CGEnAnim();
	~CGEnAnim();

	virtual eResourceType GetType(){return resAnim;} //vraci typ resourcu

	virtual int Save( char *name, CGEnPackage *package ); //ulozi animaci do souboru
	virtual int Load( char *name, CGEnPackage *package, int lazy=0, int myindex=-1 ); //nacte animaci ze souboru

	virtual int ForceLoad(); //animace se neloaduje lazy, ale nahraje lazy textury

	int SetNumFrames( int numframes ); //nastavi pocet freamu
	int GetNumFrames(){return numFrames;} //vrati pocet framu

	int SetFrame( int index, CLightTex *frametex, UI frametime) //nastavi jeden frame (frametime je cas v ms, jak dlouho bude frametex zobrazen)
	{
		if(index<0||index>=numFrames) return 0;
		SAFE_RELEASE(frame[index]);
		frame[index]=frametex; frameTime[index]=frametime;
		frame[index]->AddRef();
		totalTime=0;
		return 1;
	}

	CLightTex *GetFrameTex(int index){return frame[index];} //vrati texturu zadaneho framu
	UI GetFrameTime(int index){return frameTime[index];} //vrati cas zadaneho framu

	int SetFrameTex(int index, CLightTex *frametex) //nastavi texturu zadaneho framu
	{
		if(index<0||index>=numFrames) return 0;
		SAFE_RELEASE(frame[index]);
		frame[index]=frametex;
		frame[index]->AddRef();
		return 1;
	}

	int SetFrameTime(int index, UI frametime) //nastavi cas zadaneho framu
	{
		if(index<0||index>=numFrames) return 0;
		frameTime[index]=frametime;
		totalTime=0;
		return 1;
	}

	UI GetTotalTime(); //vrati celkovy cas animace (jednoho cyklu)

	int repeatflag;

protected:
	int numFrames;
	CLightTex **frame;
	UI *frameTime;
	UI totalTime;

};

#endif