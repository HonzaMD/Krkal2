////////////////////////////////////////////////////////////////////////////////////////////////////
//
// lighttex.cpp
//
// Textura pro vypocet osvetleni
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "lighttex.h"

#include "texture.h"
#include "register.h"
#include "normalizer.h"
#include "genmain.h"

#include "genpackage.h"

UI CLightTex::lastID = 1;

////////////////////////////////////////////////////////////////////////////////////////////////

CLightTex::CLightTex(int sizex, int sizey, unsigned int _LightComponets)
{

	ChangeID();

	sx=sizex;
	sy=sizey;
	
	LightComponets=_LightComponets;
    
	int s=GetElementSize();

    data=new UC[s*sx*sy];

	shadowimg = NULL;

	ElSource =NULL;

	lazy_pkg=NULL;

}

int CLightTex::Create(int sizex, int sizey, unsigned int _LightComponets)
{
	sx=sizex;
	sy=sizey;
	
	LightComponets=_LightComponets;
	
	if(LightComponets&LgC_SPECULAR) LightComponets^=LgC_SPECULAR; //!!! specular zatim nepodporuji
	if(LightComponets&LgC_SPEC_H) LightComponets^=LgC_SPEC_H; //!!! specular zatim nepodporuji
    
	int s=GetElementSize();

	SAFE_DELETE_ARRAY(data);
    data=new UC[s*sx*sy];

	SAFE_DELETE_ARRAY(shadowimg);
	shadowimg = NULL;

	lazy_pkg=NULL;
	
	return 1;
}

CLightTex::CLightTex()
{
	ChangeID();
	sx=0;
	sy=0;
	LightComponets=0;
    data=0;

	shadowimg = NULL;
	ElSource=NULL;

	lazy_pkg=NULL;
}

CLightTex::~CLightTex()
{
	SAFE_RELEASE(ElSource);
	SAFE_DELETE_ARRAY(data);
	SAFE_DELETE_ARRAY(shadowimg);
}

int CLightTex::CopyFromTex(CLightTex *tex)
{
	if(!tex) return 0;

	if(!tex->ForceLoad()) return 0;

	SAFE_RELEASE(ElSource);
	SAFE_DELETE_ARRAY(data);
	SAFE_DELETE_ARRAY(shadowimg);
	
	int s=tex->GetElementSize();
	LightComponets=tex->LightComponets;
	sx=tex->sx;sy=tex->sy;
    
	data=new UC[s*sx*sy];
	memcpy(data,tex->data,s*sx*sy);

	ElSource=tex->ElSource;
	ElSource->AddRef();

	shadow_dx=tex->shadow_dx;shadow_dy=tex->shadow_dy;
	shadow_sx=tex->shadow_sx;shadow_sy=tex->shadow_sy;
	if(tex->shadowimg)
	{
		shadowimg = new UC[shadow_sx*shadow_sy];
		memcpy(shadowimg,tex->shadowimg,shadow_sx*shadow_sy);
	}

	lazy_pkg=NULL;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////

int CLightTex::SetAlpha(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_ALPHA)==0) return 0;

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC a;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_ALPHA;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(LgC_ALPHA);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			texdata+=3; //bgr
			a=*(texdata++); //a
			*(datap++)=a;

			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetAmbient(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_AMBIENT)==0) return 0;
	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC r,g,b;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_AMBIENT;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(LgC_AMBIENT);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			b=*(texdata++);g=*(texdata++);r=*(texdata++);
			*(datap++)=r;//r
			*(datap++)=g;//g
			*(datap++)=b;//b
			texdata++; //a

			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetDiffuse(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_DIFFUSE)==0) return 0;

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC r,g,b;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_DIFFUSE;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(LgC_DIFFUSE);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			b=*(texdata++);g=*(texdata++);r=*(texdata++);
			*(datap++)=r;//r
			*(datap++)=g;//g
			*(datap++)=b;//b
			texdata++; //a

			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetSpecular(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_SPECULAR)==0) return 0;
	return 0;
}
int CLightTex::SetSpec_H(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_SPEC_H)==0) return 0;
	return 0;
}

int CLightTex::SetNormal(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_NORMAL)==0) return 0;

	CTexture *tex;
	UC *texdata;
	char *datap;
	UC nx,ny,nz;
	int pitch,dp,dpp;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_NORMAL;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	datap=(char*)data+sp;
	dpp=sz-GetComponentSize(LgC_NORMAL);

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			nz=*(texdata++);ny=*(texdata++);nx=*(texdata++);
			texdata++; //a
			*(datap++)=(char)(nx-128);//x
			*(datap++)=(char)(ny-128);//y
			*(datap++)=(char)(nz-128);//z

			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetZDepth(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_ZDEPTH)==0) return 0;

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC z;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_ZDEPTH;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(LgC_ZDEPTH);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			z=*(texdata++); //b
			texdata+=3; //gra
			
			*(datap++)=z;
			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetAtmosphere(const char *fname)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&LgC_ATMOSPHERE)==0) return 0;
	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC r,g,b;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch,sx,sy)) return 0;

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<LgC_ATMOSPHERE;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(LgC_ATMOSPHERE);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			b=*(texdata++);g=*(texdata++);r=*(texdata++);
			*(datap++)=r;//r
			*(datap++)=g;//g
			*(datap++)=b;//b
			texdata++; //a

			datap+=dpp;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	return 1;
}

int CLightTex::SetShadow(const char *fname, int dx, int dy)
{
	if(!ForceLoad()) return 0;
	
	SAFE_DELETE_ARRAY(shadowimg);

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp;
	UC a;

	UI sx,sy;
	UI x,y;

	if(!LoadAndLockTex(fname,&tex,(void**)&texdata,pitch)) return 0;
	
	tex->GetImageSize(&sx,&sy);

	dp=pitch-sx*4;

	shadowimg = new UC[sx*sy];
	datap=shadowimg;

	for(y=0;y<sy;y++)
	{
		for(x=0;x<sx;x++)
		{
			texdata+=3; //bgr
			a=*(texdata++); //a
			*(datap++)=255-a;
		}
		texdata+=dp;
	}

	UnlockAndReleaseTex(tex);

	//orezu texturu se stinem:

	datap=shadowimg;

	//nejdriv zjistim, kde je stin
	UI xmin=sx-1,ymin=sy-1,xmax=0,ymax=0;
	int sh=0;

	for(y=0;y<sy;y++)
		for(x=0;x<sx;x++)
		{
			if(*datap != 255)
			{
				xmin = MIN(xmin,x);
				ymin = MIN(ymin,y);
				xmax = MAX(xmax,x);
				ymax = MAX(ymax,y);
				sh=1;
			}
			datap++;
		}

	if(!sh) //zadny stin
	{
		delete[] shadowimg;
		shadowimg = NULL;
		return 1;
	}

	if(xmin!=0||ymin!=0||xmax!=sx-1||ymax!=sy-1) //stin neni pres cely obrazek - orezu ho
	{
		UI sx2,sy2;
		sx2 = xmax-xmin+1; sy2 = ymax-ymin+1;

		UC *shimg=new UC[sx2*sy2];
		UC *datap2=shimg;

		datap = shadowimg + sx*ymin + xmin;

		for(y=0;y<sy2;y++)
		{
			memcpy(datap2,datap,sx2);

			datap+=sx;
			datap2+=sx2;
		}

		delete[] shadowimg;
		shadowimg = shimg;

		shadow_sx=sx2;shadow_sy=sy2;
		shadow_dx=dx+xmin;shadow_dy=dy+ymin; //je potreba oriznuty stin posunout
	}
	else
	{//stin je pres celou texturu
		shadow_sx=sx;shadow_sy=sy; 
		shadow_dx=dx;shadow_dy=dy;
	}

	return 1;
}

int CLightTex::SetElSource(CGEnElSource *els)
{
	if(!ForceLoad()) return 0;

	if(!els) return 0;
	SAFE_RELEASE(ElSource);
	ElSource=els;
	ElSource->AddRef();
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////

CTexture* CLightTex::GetComponentTex(eLightComponets comp)
{
	if(!ForceLoad()) return 0;

	if((LightComponets&comp)==0) return 0;

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp,dpp;
	UC a,r,g,b;

	tex = new CTexture();
	if(!tex->Create(sx,sy,0,D3DFMT_A8R8G8B8) || !tex->Lock((void**)&texdata,pitch))
	{
		tex->Release();
        return 0;
	}

	int sp=0,sz=GetElementSize();
	for(int ss=1;ss<comp;ss<<=1) if(LightComponets&ss) sp+=GetComponentSize(ss);

	dp=pitch-sx*4;
	dpp=sz-GetComponentSize(comp);
	datap=data+sp;

	for(UINT y=0;y<sy;y++)
	{
		for(UINT x=0;x<sx;x++)
		{
			switch(comp)
			{
			case LgC_ALPHA:
			case LgC_SPEC_H:
			case LgC_ZDEPTH:
				r=g=b= *(datap++);
				a=0xFF;
				break;				
			case LgC_AMBIENT:
			case LgC_DIFFUSE:
			case LgC_SPECULAR:
			case LgC_ATMOSPHERE:
				r=*(datap++);
				g=*(datap++);
				b=*(datap++);
				a=0xff;
				break;
			case LgC_NORMAL:
				r= *(char*)(datap++)+128;
				g= *(char*)(datap++)+128;
				b= *(char*)(datap++)+128;
				a=0xff;
				break;
			default:
				r=g=b=0;
				a=0xFF;
			}
			
			*(texdata++) = b; //b
			*(texdata++) = g; //g
			*(texdata++) = r; //r
			*(texdata++) = a; //a

			datap+=dpp;
		}
		texdata+=dp;
	}

	tex->Unlock();

	return tex;
}

CTexture* CLightTex::GetShadowTex()
{
	if(!ForceLoad()) return 0;

	if(!IsShadow()) return 0;

	CTexture *tex;
	UC *texdata, *datap;
	int pitch,dp;
	UC a;

	tex = new CTexture();
	if(!tex->Create(shadow_sx,shadow_sy,0,D3DFMT_A8R8G8B8) || !tex->Lock((void**)&texdata,pitch))
	{
		tex->Release();
        return 0;
	}

	dp=pitch-shadow_sx*4;
	datap=shadowimg;

	for(int y=0;y<shadow_sy;y++)
	{
		for(int x=0;x<shadow_sx;x++)
		{
			a=*(datap++);
			*(texdata++) = a; //b
			*(texdata++) = a; //g
			*(texdata++) = a; //r
			*(texdata++) = 0xff; //a
		}
		texdata+=dp;
	}

	tex->Unlock();

	return tex;
}


///////////////////////////////////////////////////////////////////////////////////////////

int CLightTex::Save(char *fname, CGEnPackage *package)
{
	if(!ForceLoad()) return 0;

	int ElsIndex;

	if(!package) return 0;
	
	ElsIndex = package->GetResourceIndex(ElSource);
	if(ElsIndex==-1) return 0; //ElSource neni v package


	CFSRegister reg(fname,"KRKAL_LIGHTTEX");

	reg.DeleteAllKeys();

	CFSRegKey *k;
	k = reg.AddKey("channels", FSRTint);
	k -> writei(LightComponets);

	k = reg.AddKey("sx", FSRTint);
	k -> writei(sx);

	k = reg.AddKey("sy", FSRTint);
	k -> writei(sy);

	int csz=GetElementSize();
	UC *d;

	k = reg.AddKey("data", FSRTchar);
	for(int c=0;c<csz;c++)
	{
		d=data+c;
		for(UI y=0;y<sy;y++)
			for(UI x=0;x<sx;x++)
			{
				k->writec(*d);
				d+=csz;
			}
	}

	if(shadowimg){

		k = reg.AddKey("shadow_sx", FSRTint);
		k -> writei(shadow_sx);
		k = reg.AddKey("shadow_sy", FSRTint);
		k -> writei(shadow_sy);
		k = reg.AddKey("shadow_dx", FSRTint);
		k -> writei(shadow_dx);
		k = reg.AddKey("shadow_dy", FSRTint);
		k -> writei(shadow_dy);

		k = reg.AddKey("shadowimg", FSRTchar);
		k -> blockwrite(shadowimg,shadow_sx*shadow_sy);

	}

	k = reg.AddKey("ElSourceIndex",FSRTint);
	k -> writei(ElsIndex);

	reg.WriteFile();

	return 1;

}

int CLightTex::Load(char *fname, CGEnPackage *package, int lazy, int myindex )
{

	if(!package) return 0;


	if(lazy)
	{
		lazy_pkg=package;
		lazy_myindex=myindex;
	}
	else
	{
		CFSRegister reg(fname,"KRKAL_LIGHTTEX");
		if(reg.GetOpenError()!=1) return 0;

		CFSRegKey *k;	
		k = reg.FindKey("channels");
		LightComponets = k -> readi();

		k = reg.FindKey("sx");
		sx = k -> readi();

		k = reg.FindKey("sy");
		sy = k -> readi();


		SAFE_DELETE_ARRAY(shadowimg);
		SAFE_DELETE_ARRAY(data);
		int csz=GetElementSize();
		int sz=sx*sy*csz;
		UC *d;

		data=new UC[sz];

		k = reg.FindKey("data");
		for(int c=0;c<csz;c++)
		{
			d=data+c;
			for(UI y=0;y<sy;y++)
				for(UI x=0;x<sx;x++)
				{
					*d=k->readc();
					d+=csz;
				}
		}

		//shadow image:
		k = reg.FindKey("shadow_sx");
		if(k)
		{
			shadow_sx = k -> readi();
			k = reg.FindKey("shadow_sy");
			shadow_sy = k -> readi();
			k = reg.FindKey("shadow_dx");
			shadow_dx = k -> readi();
			k = reg.FindKey("shadow_dy");
			shadow_dy = k -> readi();

			shadowimg = new UC[shadow_sx*shadow_sy];

			k = reg.FindKey("shadowimg");
			k -> blockread(shadowimg,shadow_sx*shadow_sy);
		}

		ElSource=0;
		if(package)
		{
			k = reg.FindKey("ElSourceIndex");
			int els = k -> readi();

			CGEnResource *res = package->LoadResource(els);

			if(!res || res->GetType()!=resElSource) return 0;		

			ElSource = (CGEnElSource*)res;

			ElSource->AddRef();
			
		}

		lazy_pkg=NULL;

	}

	return 1;
}

int CLightTex::ForceLoad()
{
	if(!lazy_pkg) return 1;

	return lazy_pkg->ForceLoad(lazy_myindex);
}


////////////////////////////////////////////////////////////////////////////////////////////////

int CLightTex::LoadAndLockTex(const char *fname, CTexture **tex, void **bits, int &pitch, int tsx,int tsy)
{

	if(tex==NULL||bits==NULL) return 0;

	*tex=new CTexture();

	if(!(*tex)->CreateFromFile(fname)) {(*tex)->Release(); *tex=NULL; return 0;}

	if(tsx!=-1)
	{
		UINT xx,yy;
		(*tex)->GetImageSize(&xx,&yy);
		if(xx!=tsx||yy!=tsy) {
			(*tex)->Release(); *tex=NULL; return 0;
		}
	}
	
	if(!(*tex)->Lock(bits,pitch))
	{
		(*tex)->Release(); *tex=NULL; return 0;
	}

	return 1;

}

int CLightTex::UnlockAndReleaseTex(CTexture *tex)
{
	if(!tex) return 0;
	
	tex->Unlock();
	tex->Release();

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////

int CLightTex::CmpLight(void* vbuf, int pitch, int numlight, CLight **lights, CTopLight *toplight, CGEnElement *gel, int numshadows, CShadowInstance *shadows)
{
	assert(lazy_pkg==0);

	int dp=pitch-sx*4;
	UC *buf=(UC*)vbuf;
	UC *lt=data;

	UC dr,dg,db;
	UC amr,amg,amb;
	int nx=0,ny=0,nz=127;
	int llx,lly,llz;
	UI cr,cg,cb;
	UI r,g,b;
	UC a=0xff;
	UC z=0;
	int zshiftx=0,zshifty=0;

	int xx,yy,zz;
	int xxs,yys;
	int px,py,pz;

	CLight *l;

	int nl;
	int ll,li;
	int lg;

	int isalpha=LightComponets&LgC_ALPHA;
	int isambient=LightComponets&LgC_AMBIENT;
	int iszdepth=LightComponets&LgC_ZDEPTH;
	int isnormal=LightComponets&LgC_NORMAL;

	if(gel){
		xxs = (int)(gel->zx + ElSource->pbx);
		yys = (int)(gel->zy + ElSource->pby);	
		zz  = (int)(gel->zz);
	}else{
		xxs = (int)(ElSource->pbx);
		yys = (int)(ElSource->pby);	
		zz  = 0;
	}

	int shx,shy,shxb,shyb;
	CShadowInstance *shadow;
	CLightTex *shadowtex;
	int shadow_intenzity=255;
	int shadow_intall;

	int ar,ag,ab; //ambient koefs.
	double avrgtoplight = ((double)toplight->ir + (double)toplight->ig + (double)toplight->ib) / 3;
	ar = (int)(.12*(toplight->ir + avrgtoplight));
	ag = (int)(.12*(toplight->ig + avrgtoplight));
	ab = (int)(.12*(toplight->ib + avrgtoplight));
//	ar=ag=ab=(int)(.3*(1<<7));

	//vypocet nasvicene textury
	#include "lighttexcase.h"

	return 1;
}

int CLightTex::CmpLight(CTexture *tex, int numlight, CLight **lights, CTopLight *toplight, CGEnElement *gel, int numshadows, CShadowInstance *shadows)
{
	assert(lazy_pkg==0);

	void *buf;
	int pitch;

	if(!tex) return 0;

	if(!tex->Lock(&buf,pitch)) return 0;

	CmpLight(buf,pitch,numlight,lights,toplight,gel,numshadows,shadows);

	tex->Unlock();

	return 1;
}


int CLightTex::ClearOutPixels()
{
	if(!ForceLoad()) return 0;

	if((LightComponets&(LgC_ZDEPTH|LgC_ALPHA))==0) return 0;
	int sz=GetElementSize();
	int pa=0,pz=0,dp;
	for(int ss=1;ss<LgC_ALPHA;ss<<=1) if(LightComponets&ss) pa+=GetComponentSize(ss);
	for(int ss=1;ss<LgC_ZDEPTH;ss<<=1) if(LightComponets&ss) pz+=GetComponentSize(ss);

	pz=pz-pa;

	dp=sz - pz - 1;

	UC *datap=data + pa;

	int z;
	float zx,zy;
	float xx,yy;
	UC *a;

	for(int y=0;y<(int)sy;y++)
		for(int x=0;x<(int)sx;x++)
		{
			a = datap;
			datap += pz;
			z = *datap++;
			datap+=dp;

			zshift->CmpShift(z,zx,zy);
			xx=x-14-zx;yy=y-14-zy;

			if(ceil(xx)<0 || floor(xx)>=40 || ceil(yy)<0 || floor(yy)>=40) *a=0;

		}

	return 1;
}

UC CLightTex::GetPixelAlpha(int x,int y)
{
	assert(lazy_pkg==0);

	if(x<0||y<0||x>=(int)sx||y>=(int)sy) return 0;

	if((LightComponets & LgC_ALPHA)==0) return 0xFF;

	int sz = GetElementSize();

	return data[(x+y*sx)*sz];
}

int CLightTex::CmpTex(CTexture **tex, int effect, DWORD color)
{
	void *buf;
	int pitch;

	if(!tex) return 0;

	if(!ForceLoad()) return 0;

	*tex = new CTexture;
 	(*tex)->Create(sx,sy,0,D3DFMT_A8R8G8B8);

	if(!(*tex)->Lock(&buf,pitch)) return 0;

	CmpLight(buf,pitch,0,0,GEnMain->GetTopLight());

	UI x,y;
	int dpp = pitch-4*sx;
	UC *d = (UC*) buf;
	UC fa,fr,fg,fb;
	fa=(UC)((color>>24)&0xFF);
	fr=(UC)((color>>16)&0xFF);
	fg=(UC)((color>>8)&0xFF);
	fb=(UC)((color)&0xFF);
	int Y;
	if(effect==1){
		for(y=0;y<sy;y++)
		{
			for(x=0;x<sx;x++)
			{
				Y = (int) (.299 * d[2]/*r*/ + 0.587 * d[1]/*g*/ + 0.114 * d[0]/*b*/);				
				d[0]=(UC)(Y*fb/255);
				d[1]=(UC)(Y*fg/255);
				d[2]=(UC)(Y*fr/255);
				d[3]=(UC)(d[3]*fa/255);
				d+=4;
				
			}
			d+=dpp;
		}
	}

	(*tex)->Unlock();

	return 1;
}