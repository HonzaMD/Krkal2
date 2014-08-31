////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lighttex.h
//
// Textura pro vypocet osvetleni
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIGHTTEX_H
#define LIGHTTEX_H

#include "types.h"
#include "lights.h"
#include "texture.h"

#include "genresource.h"

class CGEnPackage;

//slozky svetelne textury
enum eLightComponets{
	LgC_ALPHA=1, //alpha
	LgC_AMBIENT=2, //ambientni
	LgC_DIFFUSE=4, //difusni
	LgC_SPECULAR=8, //specularni
	LgC_SPEC_H=16, //exponent specularni slozky (Phong)
	LgC_NORMAL=32, //normala
	LgC_ZDEPTH=64, //vyska (z souradnice)
	LgC_ATMOSPHERE=128, //atmosphere (vlastne ambient, ale nenasobi se amb. osvetlenim)
	LgC_MAX=128
};


//vrzeny stin
struct CShadowInstance
{
	class CLightTex *shadowtex; //textura stinu
	int dx,dy; //posunuti stinu vuci elementu
//	int zmax; 

};

typedef CListK<CShadowInstance*> CPShadowInstanceList;
typedef CListKElem<CShadowInstance*> CPShadowInstanceListElem;

//svetelna textura
class CLightTex: public CGEnResource
{
	friend class CElemGrid;
	friend class CShadowGrid;
	friend class CLightCache;
public:
	CLightTex(int sizex, int sizey, unsigned int LightComponets); //vytvori texturu o velikosti sizex*sizey, ktera bude mit slozky LightComponents
	CLightTex(); //vytvori prazdou texturu (pouziva se pri loadovani)

	int Create(int sizex, int sizey, unsigned int LightComponets); //vytvori texturu o velikosti sizex*sizey, ktera bude mit slozky LightComponents

	int CopyFromTex(CLightTex *tex);

	~CLightTex();

	// funkce pro nacteni slozky textury ze souboru
	// LightTex musi jiz mit nastavenou velikost a vsechny svetelne komponenty
	// musi odpovidat rozmery obrazku - jinak error
	int SetAlpha(const char *fname); //nastavi alpha slozku
	int SetAmbient(const char *fname); //nastavi ambientni slozku
	int SetDiffuse(const char *fname); //nastavi difusni slozku
	int SetSpecular(const char *fname); //nastavi specularni slozku
	int SetSpec_H(const char *fname); //nastavi exponent specularni slozky
	int SetNormal(const char *fname); //nastavi normaly
	int SetAtmosphere(const char *fname); //nastavi atmosphere
	int SetZDepth(const char *fname); //nastavi vysky

	CTexture* GetComponentTex(eLightComponets comp); //vrati texturu komponenty (NULL pokud neni)
	CTexture* GetShadowTex(); //vrati texturu stinu

	UC GetPixelAlpha(int x,int y); //vrati alphu zadaneho pixelu

	int SetShadow(const char *fname, int dx, int dy); //nastavi vrzeny stin, (dx,dy) je posun vuci zakladne; obrazek se stine orizne
	void SetShadowPos(int dx,int dy){shadow_dx=dx;shadow_dy=dy;} //nastavi posunuti stinu

	// spocita nasvicenou texturu
	int CmpLight(void* buf, int pitch, int numlight, CLight **lights, CTopLight *toplight, class CGEnElement *gel=NULL, int numshadows=0, CShadowInstance *shadows=NULL);
	int CmpLight(CTexture *tex, int numlight, CLight **lights, CTopLight *toplight, class CGEnElement *gel=NULL, int numshadows=0, CShadowInstance *shadows=NULL);

	int CmpTex(CTexture **tex, int effect=0, DWORD color=0xFFFFFFFF); //spocita nasvicenou texturu; tex se alokuje -> volat release; pokud je effect=1 -> prevede ji na cernobilou a prenasobi zadanou barvou

	// nastavi uplnou pruhlednot pixelum, ktere vycuhuji mimo zakladnu
	int ClearOutPixels();

	virtual int Save(char *fname, CGEnPackage *package); //ulozi texturu do souboru v packagi
	virtual int Load(char *fname, CGEnPackage *package, int lazy=0, int myindex=-1 ); //nacte texturu ze souboru v packagi
	virtual int ForceLoad(); //nahraje resource, ktery byl lazy loadovan
	virtual int CanLazyLoad(){return 1;} //vraci, ze se umi lazyloadovat
	virtual eResourceType GetType(){return resLightTex;} //vrati typ resourcu

	UINT GetSX(){return sx;} //vrati xovou velikost
	UINT GetSY(){return sy;} //vrati yovou velikost
	void GetSize(UINT &x, UINT &y){x=sx;y=sy;} //vrati rozmery textury

	int IsShadow(){if(shadowimg) return 1; else return 0;} //vraci 1 pokud textura vrha stin
	int GetShadowInfo(int &sx, int &sy, int &dx, int &dy) //vraci info o stinu
		{if(!shadowimg) return 0; sx=shadow_sx;sy=shadow_sy;dx=shadow_dx;dy=shadow_dy; return 1;}

	class CGEnElSource* GetElSource(){return ElSource;} //vrati elsource
	int SetElSource(class CGEnElSource* els);  //nastavi elsource

	int Lazy(){return lazy_pkg?1:0;} //vrati jestli je textura lazy

	UI GetTexID(){return texID;}
	void ChangeID(){texID=lastID++;}

	int GetLightComponents(){return LightComponets;}

protected:

	class CGEnElSource* ElSource; //elsource - body zakladny + dalsi info

	int LoadAndLockTex(const char *fname, CTexture **tex, void **bits, int &pitch, int tsx=-1,int tsy=-1); //nahraje texturu (obrazek) ze souboru a lockne ji
	int UnlockAndReleaseTex(CTexture *tex); //odemkne texturu a zrusi ji

	int GetComponentSize(unsigned int LightComponent) const //vraci velikost zadane komponenty (1 pixelu)
	{
		switch(LightComponent)
		{
		case LgC_AMBIENT:
		case LgC_DIFFUSE:
		case LgC_SPECULAR:
		case LgC_NORMAL:
		case LgC_ATMOSPHERE:
			return 3;
		case LgC_ALPHA:
		case LgC_ZDEPTH:
			return 1;
		default:
			return 0;
		}
	}

	int GetElementSize() const //vraci velikost vsech komponent (1 pixelu)
	{
		int sz=0;
		for(int i=1;i<=LgC_MAX;i<<=1) if(LightComponets&i) sz+=GetComponentSize(i);
		return sz;
	}

	unsigned int LightComponets; //bitflags
    
	UC *data; //vsechny komponenty pohromade (pro 1 pixel jsou vsechny slozky u sebe); neni tam stin
	UI sx,sy; //rozmery

	UC *shadowimg; //stin; =NULL pokud objekt nehazi stin shora
	int shadow_sx, shadow_sy; //rozmery stin. obr.
	int shadow_dx,shadow_dy; //posunuti stinu vuci zakladne

	//lazy load:
	CGEnPackage *lazy_pkg; //pkg, kde je textura
	int lazy_myindex; //muj index v pkg

	UI texID; //ID textury
	static UI lastID; //posledni pridelene ID
};

typedef CListK<CLightTex*> CPLightTexList;
typedef CListKElem<CLightTex*> CPLightTexListElem;


#endif