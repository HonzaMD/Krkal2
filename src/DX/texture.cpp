//////////////////////////////////////////////////////////////////////
//
// texture.cpp
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "dx.h"

#include "texture.h"
#include "fs.h"
#include "krkal.h"

int CTexture::CreateFromFile(const char *filename, DWORD filter)
{

	D3DXIMAGE_INFO SrcInfo[3];
	
	if(pTexture) {KRKAL->DeleteTextureLater(pTexture);pTexture=NULL;}

	int ln;
	int ok=1;
	char *buf;

	ln=FS->GetFileSize(filename);
	if(!ln) return 0;

	buf=new char[ln];
	
	if(!FS->ReadFile(filename,buf,ln)) ok=0;
	else
	if(D3DXCreateTextureFromFileInMemoryEx(g_pDXapp->GetD3DDevice(),buf,ln,0,0,0, 0 ,
		D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,
		filter,D3DX_FILTER_NONE ,0,&SrcInfo[1],NULL,&pTexture)==D3D_OK)
	{
		
		 D3DSURFACE_DESC Desc;

		 pTexture->GetLevelDesc(0,&Desc);

		 imgsx=SrcInfo[1].Width;
		 imgsy=SrcInfo[1].Height;

		 texsx=Desc.Width;
		 texsy=Desc.Height;

		 texu=(float)texsx/imgsx;
		 texv=(float)texsy/imgsy;

	}else
	{ pTexture=NULL;ok=0; }

	delete[] buf;
	return ok;
}

int CTexture::Create(UINT sx, UINT sy, DWORD usage, D3DFORMAT fmt, D3DPOOL pool)
{

	if (pTexture) { KRKAL->DeleteTextureLater(pTexture); pTexture = NULL; }

    imgsx=sx;
    imgsy=sy;

	texsx=1; while(texsx<sx) texsx<<=1;
	texsy=1; while(texsy<sy) texsy<<=1;
	


	if(g_pDXapp->GetD3DDevice()->CreateTexture(texsx,texsy,1,usage,fmt,pool,&pTexture)==D3D_OK)
	{

		 texu=(float)texsx/imgsx;
		 texv=(float)texsy/imgsy;

		return 1;
	}

	pTexture=0;

	return 0;
}

CTexture::CTexture()
{
	pTexture=NULL;
}

CTexture::~CTexture()
{
	if(pTexture)
	{
		KRKAL->DeleteTextureLater(pTexture); pTexture = NULL;
	}
}

int CTexture::Lock(void **data, int &pitch, DWORD flags, const RECT *rect)
{
	if(!pTexture||!data) return 0;

	D3DLOCKED_RECT r;
	
	if(pTexture->LockRect(0,&r,rect,flags)==D3D_OK) {
		*data=r.pBits;
		pitch=r.Pitch;
		return 1; 
	}
	else return 0;

}

int CTexture::Unlock()
{
	if(!pTexture) return 0;

	if(pTexture->UnlockRect(0)==D3D_OK)	return 1; else return 0;
}