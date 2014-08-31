//////////////////////////////////////////////////////////////////////
//
// texture.h
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////
#ifndef TEXTURE_H
#define TEXTURE_H

#include "types.h"
#include "genresource.h"

class CTexture: public CGEnResource{

protected:

	LPDIRECT3DTEXTURE8 pTexture;

	UINT imgsx, imgsy;
	UINT texsx,texsy;

	float texu,texv;

public:

	CTexture();
	~CTexture();

	int CreateFromFile(const char *filename, DWORD filter = D3DX_FILTER_NONE); //vytvori texturu ze souboru
	int Create(UINT sx, UINT sy, DWORD usage, D3DFORMAT fmt, D3DPOOL pool=D3DPOOL_MANAGED); //vytvori texturu

	LPDIRECT3DTEXTURE8 GetTexture(){return pTexture;} //vrati texturu DirectX

	void GetTextureSize(UINT *sx, UINT *sy) {*sx=texsx;*sy=texsy;} //vrati velikost textury (mocnina 2)
	void GetImageSize(UINT *sx, UINT *sy)   {*sx=imgsx;*sy=imgsy;} //vrati velikost obrazku
	void GetTexUV(float *u, float *v) {*u=texu;*v=texv;}

	void GetTexture(LPDIRECT3DTEXTURE8 *pTex){*pTex=pTexture;} //vrati pointr na directi texturu

	int Lock(void **data, int &pitch, DWORD flags=0, const RECT *rect=NULL); //zamkne texturu a vrati pointr na data (po zamknuti urychlene naplnit a odemknout!!!)
	int Unlock(); //odemkne texturu

	virtual eResourceType GetType(){return resTex;} //vraci typ resourcu

	virtual int Save( char *name, CGEnPackage *package ){return 0;} //DX neumi ukladat textury -> shit
	virtual int Load( char *name, CGEnPackage *package, int lazy=0, int myindex=-1  ){return CreateFromFile(name);} //nacte resource ze souboru - package se pta na indexy resourcu, ktere potrebuje


};

#endif