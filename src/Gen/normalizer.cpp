////////////////////////////////////////////////////////////////////////////////////////////////////
//
// normalizer.cpp
//
// Normalizace 3D vektoru; ZShift
//
// A: Petr Altman
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "normalizer.h"

#include "genmain.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CNormalizer 
// umoznuje rychlou normalizaci vektoru pomoci predpocitanych tabulek
////////////////////////////////////////////////////////////////////////////////////////////////////

CNormalizer normalizer;
CLightZShift *zshift=NULL;

CNormalizer::CNormalizer()
{
	int x=128;
	double ln;
	for(int y=0;y<128;y++)
		for(int z=0;z<128;z++)
		{
           ln=sqrt((double)(x*x+y*y+z*z)); 
		   nvec[y][z][0]=(int)((x/ln)*127+.5);
		   nvec[y][z][1]=(int)((y/ln)*127+.5);
		   nvec[y][z][2]=(int)((z/ln)*127+.5);		   
		}
}

void CNormalizer::CmpNormal(int &nx,int &ny, int &nz)
{


	int ax=abs(nx),ay=abs(ny),az=abs(nz);

	int max=ax,maxax=0;

	int xx,yy,zz;

	if(ay>max) {max=ay;maxax=1;}
	if(az>max) {max=az;maxax=2;}

	int i1,i2;

	switch(maxax) //case podle nejvetsi osy
	{
	case 0: //x

		if(ax)
		{
		i1=(int)(((float)ay/ax)*127+.5f);
		i2=(int)(((float)az/ax)*127+.5f);
		}else{
			i1=i2=0;
		}

		xx=nvec[i1][i2][0];
		yy=nvec[i1][i2][1];
		zz=nvec[i1][i2][2];
		break;
	case 1: //y
		
		i1=(int)(((float)ax/ay)*127+.5f);
		i2=(int)(((float)az/ay)*127+.5f);

		yy=nvec[i1][i2][0];
		xx=nvec[i1][i2][1];
		zz=nvec[i1][i2][2];
		break;
	case 2: //z

		i1=(int)(((float)ay/az)*127+.5f);
		i2=(int)(((float)ax/az)*127+.5f);

		zz=nvec[i1][i2][0];
		yy=nvec[i1][i2][1];
		xx=nvec[i1][i2][2];
		break;
	}

	if(nx>0) nx=xx; else nx=-xx;
	if(ny>0) ny=yy; else ny=-yy;
	if(nz>0) nz=zz; else nz=-zz;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// CLightZShift
////////////////////////////////////////////////////////////////////////////////////////////////////


CLightZShift::CLightZShift(float ztransformx, float ztransformy, float toplight_zx, float toplight_zy)
{
	Init(ztransformx,ztransformy,toplight_zx,toplight_zy);
}

void CLightZShift::Init(float ztransformx, float ztransformy, float toplight_zx, float toplight_zy)
{

	zx = ztransformx;
	zy = ztransformy;

	shadow_zx = -toplight_zx;
	shadow_zy = toplight_zy;

	for(int i=0;i<256;i++)
	{
		xyshift[i].x = (int)floor(ztransformx * i + .5f);
		xyshift[i].y = (int)floor(ztransformy * i + .5f);

		xyshift[i].tl_x = (int)floor(shadow_zx * i + .5f);
		xyshift[i].tl_y = (int)floor(shadow_zy * i + .5f);
	}
}

void CLightZShift::CmpShadowShift(int z, int &x, int &y)
{
	x=(int)floor(shadow_zx*z+.5f);
	y=(int)floor(shadow_zy*z+.5f);
}

void CLightZShift::CmpShift(int z, float &x, float &y)
{
	x=zx*z;
	y=zy*z;
}