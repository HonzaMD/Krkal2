////////////////////////////////////////////////////////////////////////////////
//
//		GEnAnim
//
//		animace
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "genanim.h"

#include "genpackage.h"
#include "register.h"

CGEnAnim::CGEnAnim()
{
	numFrames=0;
	frame = 0;
	frameTime = 0;

	repeatflag = 0;

	totalTime=0;
}

CGEnAnim::~CGEnAnim()
{
	for(int i=0;i<numFrames;i++)
		SAFE_RELEASE(frame[i]);
	SAFE_DELETE_ARRAY(frame);
	SAFE_DELETE_ARRAY(frameTime);
}

int CGEnAnim::SetNumFrames(int numtex) //nastavi pocet framu - zrusi vsechny drivejsi (nekopiruje stary veci do novyho pole)
{
	if(numtex<=0) return 0;

	for(int i=0;i<numFrames;i++)
		SAFE_RELEASE(frame[i]);
	SAFE_DELETE_ARRAY(frame);
	SAFE_DELETE_ARRAY(frameTime);

	numFrames=numtex;

	frame = new CLightTex*[numFrames];
	frameTime = new UI[numFrames];
	for(int i=0;i<numFrames;i++)
		frame[i]=NULL;

	totalTime=0;

	return 1;
}


int CGEnAnim::Load( char *name, CGEnPackage *package, int lazy, int myindex ) //ulozi animaci do souboru
{

	if(!package) return 0; //soubor musi byt v packagi

	CFSRegister reg(name,"KRKAL_ANIM"); //otevre register
	if(reg.GetOpenError()!=1) return 0;  //chyba pri otvirani

	CFSRegKey *k,*k2;
	int ind;
	int ok=1;
	CGEnResource *res;

	k = reg.FindKey("numFrm"); //precte pocet framu
	if( !SetNumFrames( k->readi() ) ) return 0; //nastavi pocet framu
	
	k = reg.FindKey("repeat"); //precte repeatflag
	repeatflag = k -> readi();

	k = reg.FindKey("Frames"); //indexy textur
	k2= reg.FindKey("Times"); //casy framu

	for(int i=0;i<numFrames;i++)
	{
		ind = k->readi(); //precte index obrazku
		
		res = package->LoadResource(ind); //nacte obrazek

		if(!res || res->GetType()!=resLightTex) {ok=0; break;} //chyba - bud vubec neexistuje nebo to neni textura

		frame[i]=(CLightTex*) res; //nastavi frame
		frame[i]->AddRef();
		frameTime[i] = k2->readi();
	}

	if(!ok){
		numFrames=0; //pri chybe zrusim vse
		for(int i=0;i<numFrames;i++)
			SAFE_RELEASE(frame[i]);
		SAFE_DELETE_ARRAY(frame);
		SAFE_DELETE_ARRAY(frameTime);
	}

	totalTime=0;
	GetTotalTime();

	return ok;
}

int CGEnAnim::ForceLoad()
{
	for(int i=0;i<numFrames;i++)
	{
		if( !frame[i]->ForceLoad() ) return 0;
	}
	return 0;
}

int CGEnAnim::Save( char *name, CGEnPackage *package ) //ulozi animaci do souboru
{
	if(!package || numFrames<1 ) return 0; //uklada pouze do packagu; animace musi mit aspon 1 obrazek

	int *pkgind=new int[numFrames]; //sem se budou ukladat indexy na textury
	int i;
	int ok=1;


	for(i=0;i<numFrames;i++) //zjistim indexy vsech textur
	{
		pkgind[i] = package->GetResourceIndex(frame[i]);
		if(pkgind[i]==-1) {ok=0;break;} //chyba - textura neni v packagi
	}

	if(ok)
	{
		CFSRegister reg(name,"KRKAL_ANIM",1); //vytvorim registr

		CFSRegKey *k,*k2;

		k = reg.AddKey("numFrm", FSRTint); //pocet framu
		k -> writei(numFrames);

		k = reg.AddKey("repeat", FSRTint); //repeatflag
		k -> writei(repeatflag);

		k = reg.AddKey("Frames",FSRTint); //indexy textur
		k2= reg.AddKey("Times",FSRTint); //casy

		for(i=0;i<numFrames;i++)
		{
			k->writei(pkgind[i]);
			k2->writei(frameTime[i]);
		}

		reg.WriteFile(); //zapisu registr

	}

	delete[] pkgind;

	return ok;
}

UI CGEnAnim::GetTotalTime()
{
	if(totalTime==0){
		for(int i=0;i<numFrames;i++)
            totalTime+=frameTime[i];	
	}
	
	return totalTime;

}