////////////////////////////////////////////////////////////////////////////////
//
//		GEnResource
//
//		resource, ktery se ukladaji do packagu
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////

/*

	resource, ktery se ukladaji do packagu

	snadno lze pridat dalsi typ resourcu:
	1) pridej ty do eResourceType
	2) odded si novou tridu od CGEnResource
	3) implementu metody GetType, Load, Save
	4) pridej konstrukci nove tridy do CGEnResource::Create v genresource.cpp


*/


#ifndef GENRESOURCE_H
#define GENRESOURCE_H

#include "types.h"

//type resourcu:
enum eResourceType { resNone=0, resElSource=1, resLightTex=2, resAnim=3, resTex=4, resMusicModule=5, resMusicSample=6 };

class CGEnPackage;


class CGEnResource: public CRefCnt {

public:

	virtual ~CGEnResource(){};

	virtual eResourceType GetType()=0; //vraci typ resourcu

	virtual int Save( char *name, CGEnPackage *package )=0; //ulozi resource do souboru - package se pta na indexy resourcu, ktere potrebuje
	virtual int Load( char *name, CGEnPackage *package, int lazy=0, int myindex=-1 )=0; //nacte resource ze souboru - package se pta na indexy resourcu, ktere potrebuje

	virtual int ForceLoad(){return 1;} //nahraje resource, ktery byl lazy loadovan
	virtual int CanLazyLoad(){return 0;} //vraci, jestli se resource umi lazyloadovat

	static CGEnResource* CreateResource(eResourceType type); //staticka metoda - vytvori instanci spravneho typu (kvuli loadovani - je potreba vytvorit objekt a zavolat jeho Load)
};

typedef CListK<CGEnResource*> CPGEnResourceList; //seznam resourcu


#endif