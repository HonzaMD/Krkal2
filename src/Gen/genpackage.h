////////////////////////////////////////////////////////////////////////////////
//
//		GEnPackage
//
//		balicek obrazku(CLightTex), ElSourcu a animaci pro GEn
//		A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////

/*
	Do packagi se ukladaji objekty odvozene od CGEnResource (viz. genresource.h)

	Pro loadovani pouzivejte funkce GEnu: LoadElSource, LoadLightTex, LoadAnim

	Pri loadovani nejakeho resourcu se nejdriv zjisti v jakem packagi je. Pokud jeste neni package
	naloadovany, tak se nahraje seznam resourcu v tomto balicku. Resource si sam nahraje 
	ostatni resource, ktere potrebuje. Nahrane resource se registruji a pri opetovnem pokusu o nahrani
	se pouzije jiz nahrany.


	Package se chovaji jako adresare. Uvnitr package jsou resourci ulozeny jako soubory. Uvnit package
	nemohou byt adresare (tj. ani vnorene package). Kazdy package obsahuje navic soubor 'index', ve kterem
	je seznam resourcu package.

*/


#ifndef GENPACKAGE_H
#define GENPACKAGE_H

#include "types.h"
#include "genelement.h"
#include "lighttex.h"

extern int PackageLazyLoad;

enum GEnPkgType { pkgElSource, pkgLightTex, pkgAnim };

//Package
class CGEnPackage{
public:
	CGEnPackage();
	~CGEnPackage();

	int LoadPackage(char *name); //nahraje seznam resourcu (vraci 0=err 1=ok)

	int CreatePackage(char *name, int createarchive=0); //vytvori novy package (pokud jiz existuje - error) (vraci 0=err 1=ok)

	int AddResource( char *name, CGEnResource *res, int newresource=1 ); //prida resource do package 
		//resource, ktere potrebuje jiz musi byt v packagi - jinak error
		//pokud je newresource=1 -> kontroluje jmeno, pokud uz je resource se stejnym jmenem v packagi, tak vraci error
		//pokud je newresource=0 -> prepise stary resource, pokud neexistoval->chyba, pokud nesouhlasi typ noveho a stareho resourcu->chyba
		//vraci 0=err 1=ok
	int AddResourceFromFile( char *name, char *filename, eResourceType type, int newresource=1); //prida do package soubor

	CGEnResource* LoadResource( int index ); //nahraje resource (podle cisla) (vraci NULL=chyba)
	CGEnResource* LoadResource( char *name ); //nahraje resource (podle jmena souboru) (vraci NULL=chyba)
	int ForceLoad(int index); //nahraje resource, ktery byl lazy loadovan

	int DeleteResource( char *name ); //smaze resource z pakaze

	int GetResourceIndex( CGEnResource *res ); //vrati cislo resorce ( -1: chyba(resource neni v packagi), jinak vraci cislo >=0)
	int GetResourceIndex( char *name ); //vrati cislo resorce ( -1: chyba(resource neni v packagi), jinak vraci cislo >=0)

	eResourceType GetResourceType( int index ); //vrati typ resourcu
	eResourceType GetResourceType( char *name ); //vrati typ resourcu

	const char* GetResourceName( int index ); //vrati jmeno resourcu
	int GetNumResources(){return numResources;} //vrati pocet resourcu

	int IsValidResourceName( char *name); //vraci: 0=neplatne jmeno, 1=ok, 2=platne jmeno, ale uz je v packagi

	void DestroyPackage(); //zavre balicek (nerusi nahrane resource, rusi jen seznam resourcu)

	char *GetName(){return PackageName;} //vrati jmeno balicku (cela cesta)

	int UnloadResources(); //zrusi nepouzivane resourcy; vraci 1 pokud v packagi zadne pouzivane resourcy nejsou

protected:

	int WriteIndex(); //zapise seznam resourcu
	int ReadIndex(); //nacte seznam resourcu

	char *PackageName; //jmeno package (cela cesta)
	
	int numResources; //pocet resourcu v packagi
	char* *ResourceNames; //jmena resourcu (jmena souboru; uvnit package nejsou zadne adresare)
	eResourceType *ResourceTypes; //typ resourcu
	CGEnResource* *Resources; //pointry na jiz naloadovane resource
	
};

typedef CListK<CGEnPackage*> CPGEnPackageList; //seznam Packagi
typedef CListIterator<CGEnPackage*> CPGEnPackageListIter;


//zaznam hash tabulky, reprezentujici package
class CPackageHashEl: public CHashElem {
public:
	CPackageHashEl(CGEnPackage *package);

	CGEnPackage* GetPackage(){return package;}

protected:
	CGEnPackage *package;
};

//spravce packagi - pomoci nej se loaduji package a resource
class CPackageMgr{
public:
	CPackageMgr();
	~CPackageMgr();	

	CGEnResource* LoadResource( char *resourcename, eResourceType type, CGEnPackage **pkg=NULL ); //nahraje resource
		//zadava se cesta k resourci (package se chovaji jako adresare)
		//pokud neni package nahrany, tak ho nahraje
		//zkontroluje typ - pokud nesouhlasi vraci NULL
		//pri chybe vraci NULL

	int AddResource( char *resourcename, CGEnResource *res, int newresource=1 );
	int AddResourceFromFile( char *resourcename, char *filename, eResourceType type, int newresource=1);

	int DeleteResource( char *resourcename ); //smaze resource z pakaze
	int DeletePackage( char *packagename ); //smaze package

	eResourceType GetResourceType( char *resourcename, CGEnPackage **pkg=NULL ); //vrati typ resourcu
	eResourceType GetResourceType( char *packagename, char *resname, CGEnPackage **pkg=NULL ); //vrati typ resourcu

	void UnloadResources(); //zrusi vsechny nepouzivane resource; zrusi nahrane package, ktere nemaji zadny pouzivany resource

	CGEnPackage* GetPackage(char *packagename); //nahraje package (pokud je jiz nahrany, tak ho znovu nenahrava - pouziva hashtabulku) (vraci NULL pri chybe); packagename musi byt cela cesta k packagi

protected:

	void UnloadAllPackages(); //zrusi vsechny package (deletuje nepouzivane resourcy); zrusi vsechny package ikdyz je v nich naky pouzivany resource

	int SplitResource(char *resourcefullpath, char **resourcename); //rozdeli celou cestu k resourcu na cestu k pakagi a resourcu; nic nealokuje; (posledniho lomitko prepise na 0)

	CHashTable *PackageHashTable; //hash tabulka nahranuch Packagu
	CPGEnPackageList PackageList; //seznam nahranych Packagu

};

#endif